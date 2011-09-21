/*
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "wanipc.h"
#include "netconf.h"
#include "bcmnvram.h"
#include "nvparse.h"
#include "mapmgr.h"


#if	FD_SETSIZE < 200
#error "FD_SETSIZE is too small.  Must be >= 200"
#endif

typedef struct map_set {
    fd_set map;
    int count;
} map_set_t;


static int get_nat_list(netconf_nat_t **plist, int *nrules);
static void add_nat_entry(netconf_nat_t *nat);
static void delete_nat_entry(netconf_nat_t *pnat);
static bool find_matching_entry(const netconf_nat_t *e, netconf_nat_t *result);
static bool nat_equal(const netconf_nat_t *e1, const netconf_nat_t *e2);

static map_set_t ports;
static map_set_t ranges;


void mapmgr_update()
{
    int i;
    netconf_nat_t m;

    memset(&ports, 0, sizeof(ports));
    memset(&ranges, 0, sizeof(ranges));

    for (i = 0; i < NFDBITS; i++) {
	if (get_forward_port(i, &m)) {
	    if (m.match.dst.ports[0] == m.match.dst.ports[1]) {
		FD_SET(i, &ports.map);
		ports.count++;
	    } else {
		FD_SET(i, &ranges.map);
		ranges.count++;
	    }
	}
    }
}


/* Get the n'th port mapping entry from nvram.  This routine maintains
   the illusion that port mappings are stored in a compact, sequential
   list, no matter what the interlying storage really is.  */
static bool mapmgr_get_map(map_set_t *pset, int n, mapping_t *m)
{
    bool foundit = FALSE;
    int i;

    for (i = 0; i < NFDBITS; i++) {
	if (FD_ISSET(i, &pset->map)) {
	    if (n-- == 0) {
		foundit = get_forward_port(i, (netconf_nat_t*)m);
		break;
	    }
	} 
    }
    
    return foundit;
}

/* Write the n'th port mapping entry from nvram.  This routine maintains
   the illusion that port mappings are stored in a compact, sequential
   list, no matter what the interlying storage really is.  */
static bool mapmgr_add_map(map_set_t *pset, mapping_t *m)
{
    bool foundit = FALSE;
    int i;

    foundit = FALSE;
    for (i = 0; i < NFDBITS; i++) {
	if (!FD_ISSET(i, &ports.map) && !FD_ISSET(i, &ranges.map) ) {
	    foundit = set_forward_port(i, (netconf_nat_t*)m);
	    req_nvram_commit(0);
	    FD_SET(i, &pset->map);
	    pset->count++;
	    break;
	}
    }

    return foundit;
}


/* Deletes the i'th forward_port%d entry from nvram and moves all subsequent entries up. */
static bool mapmgr_delete_map(map_set_t *pset, int n)
{
    bool foundit = FALSE;
    int i;

    for (i = 0; i < NFDBITS; i++) {
	if (FD_ISSET(i, &pset->map)) {	
	    if (n-- == 0) {
		FD_CLR(i, &pset->map);
		pset->count--;
		foundit = del_forward_port(i);
		req_nvram_commit(0);
		break;
	    }
	}
    }

    return foundit;
}



/* Get the n'th port mapping entry from nvram.  This routine maintains
   the illusion that port mappings are stored in a compact, sequential
   list, no matter what the interlying storage really is.  */
bool mapmgr_get_port_map(int n, mapping_t *m)
{
    return mapmgr_get_map(&ports, n, m);
}

/* Write the n'th port mapping entry from nvram.  This routine maintains
   the illusion that port mappings are stored in a compact, sequential
   list, no matter what the interlying storage really is.  */
bool mapmgr_add_port_map(mapping_t *m)
{
    bool success;

    if ((success = mapmgr_add_map(&ports, m)) != FALSE  && !(m->match.flags & NETCONF_DISABLED)) {
	add_nat_entry((netconf_nat_t *) m);
	bump_generation();
    }

    return success;
}


/* Deletes the i'th forward_port%d entry from nvram. */
bool mapmgr_delete_port_map(int n)
{
    bool success = FALSE;
    mapping_t m;
    mapping_t result;


    if (mapmgr_get_map(&ports, n, &m)) {
	if (find_matching_entry(&m, &result)) {
	    delete_nat_entry((netconf_nat_t *) &result);
	}
	success = mapmgr_delete_map(&ports, n);
	bump_generation();
    }
    return success;
}




/* Get the n'th port mapping entry from nvram.  This routine maintains
   the illusion that port mappings are stored in a compact, sequential
   list, no matter what the interlying storage really is.  */
bool mapmgr_get_range_map(int n, mapping_t *m)
{
    return mapmgr_get_map(&ranges, n, m);
}

/* Write the n'th port mapping entry from nvram.  This routine maintains
   the illusion that port mappings are stored in a compact, sequential
   list, no matter what the interlying storage really is.  */
bool mapmgr_add_range_map(mapping_t *m)
{
    return mapmgr_add_map(&ranges, m);
}


/* Deletes the i'th forward_port%d entry from nvram and moves all subsequent entries up. */
bool mapmgr_delete_range_map(int n)
{
    return mapmgr_delete_map(&ranges, n);
}



int mapmgr_port_map_count()
{
    return ports.count;
}


int mapmgr_range_map_count()
{
    return ranges.count;
}


/* get the current list of static NAT mappings. 
   Return 0 on success, non-zero on failure.
*/
static int get_nat_list(netconf_nat_t **plist, int *nrules)
{
    int needlen = 0, listlen;
    netconf_nat_t *nat_list = 0;

    netconf_get_nat(NULL, &needlen);
    if (needlen > 0) {
	nat_list = (netconf_nat_t *) malloc(needlen);
	if (nat_list) {
	    memset(nat_list, 0, needlen);
	    listlen = needlen;
	    if (netconf_get_nat(nat_list, &listlen) == 0 && needlen == listlen) {
		*nrules = needlen/sizeof(netconf_nat_t);
		*plist = nat_list;
		return 0;
	    }
	    free(nat_list);
	}
	return 1;
    } else {
	*nrules = 0;
	*plist = NULL;
	return 0;
    }
}


/* Add port forward and a matching ACCEPT rule to the FORWARD table */
static void
add_nat_entry(netconf_nat_t *entry)
{
    int dir = NETCONF_FORWARD;
    int log_level = atoi(nvram_safe_get("log_level"));
    int target = (log_level & 2) ? NETCONF_LOG_ACCEPT : NETCONF_ACCEPT;
    netconf_filter_t filter;
    struct in_addr netmask = { 0xffffffff };
    netconf_nat_t nat = *entry;
    
    if (entry->ipaddr.s_addr == 0xffffffff) {
	inet_aton(nvram_safe_get("lan_ipaddr"), &nat.ipaddr);
	inet_aton(nvram_safe_get("lan_netmask"), &netmask);
	nat.ipaddr.s_addr &= netmask.s_addr;
	nat.ipaddr.s_addr |= (0xffffffff & ~netmask.s_addr);
    }

    /* Set up LAN side match */
    memset(&filter, 0, sizeof(filter));
    filter.match.ipproto = nat.match.ipproto;
    filter.match.src.ports[1] = nat.match.src.ports[1];
    filter.match.dst.ipaddr.s_addr = nat.ipaddr.s_addr;
    filter.match.dst.netmask.s_addr = netmask.s_addr;
    filter.match.dst.ports[0] = nat.ports[0];
    filter.match.dst.ports[1] = nat.ports[1];
    strncpy(filter.match.in.name, nat.match.in.name, IFNAMSIZ);

    /* Accept connection */
    filter.target = target;
    filter.dir = dir;

    /* Do it */
    netconf_add_nat(&nat);
    netconf_add_filter(&filter);
}



/* Combination PREROUTING DNAT and FORWARD ACCEPT */
static void 
delete_nat_entry(netconf_nat_t *entry)
{
    int dir = NETCONF_FORWARD;
    int log_level = atoi(nvram_safe_get("log_level"));
    int target = (log_level & 2) ? NETCONF_LOG_ACCEPT : NETCONF_ACCEPT;
    netconf_filter_t filter;
    struct in_addr netmask = { 0xffffffff };
    netconf_nat_t nat = *entry;
    
    if (entry->ipaddr.s_addr == 0xffffffff) {
	inet_aton(nvram_safe_get("lan_ipaddr"), &nat.ipaddr);
	inet_aton(nvram_safe_get("lan_netmask"), &netmask);
	nat.ipaddr.s_addr &= netmask.s_addr;
	nat.ipaddr.s_addr |= (0xffffffff & ~netmask.s_addr);
    }

    /* Set up LAN side match */
    memset(&filter, 0, sizeof(filter));
    filter.match.ipproto = nat.match.ipproto;
    filter.match.src.ports[1] = nat.match.src.ports[1];
    filter.match.dst.ipaddr.s_addr = nat.ipaddr.s_addr;
    filter.match.dst.netmask.s_addr = netmask.s_addr;
    filter.match.dst.ports[0] = nat.ports[0];
    filter.match.dst.ports[1] = nat.ports[1];
    strncpy(filter.match.in.name, nat.match.in.name, IFNAMSIZ);

    /* Accept connection */
    filter.target = target;
    filter.dir = dir;

    /* Do it */
    errno = netconf_del_nat(&nat);
    if (errno)
	UPNP_ERROR(("netconf_del_nat returned error %d\n", errno));

    errno = netconf_del_filter(&filter);
    if (errno)
	UPNP_ERROR(("netconf_del_filter returned error %d\n", errno));
}

static bool find_matching_entry(const netconf_nat_t *e, netconf_nat_t *result)
{
    bool foundit = FALSE;
    netconf_nat_t *rule = NULL;
    int nrules, rulenum;
    netconf_nat_t *nat_list = NULL;

    if (!get_nat_list(&nat_list, &nrules)) {
	for ( rulenum = 0; rulenum < nrules; rulenum++, rule = NULL ) {
	    rule = &nat_list[rulenum];
	    // Only match DNAT entries 
	    // (there may be a MASQUERADE entry that we don't want.)
	    if (rule->target != NETCONF_DNAT) {
		continue;
	    }

	    // does the source information match?
	    if (nat_equal(e, rule)) {

		// copy the matched entry into our results.
		*result = *rule;

		// initialize the next and prev pointers so 
		// this entry looks like a single element list.
		netconf_list_init((netconf_fw_t *) result);

		// indicate that we found a matching entry.
		foundit = TRUE;
		break;
	    } 
	}
	free(nat_list);
    }
	
    return foundit;
}

static bool nat_equal(const netconf_nat_t *e1, const netconf_nat_t *e2)
{
    bool matched = FALSE;

    do {
	if (e1->match.ipproto != e2->match.ipproto) {
	    continue;
	}

	if (e1->match.dst.ipaddr.s_addr && 
	    ( e1->match.dst.netmask.s_addr != e2->match.dst.netmask.s_addr 
	      || e1->match.dst.ipaddr.s_addr != e2->match.dst.ipaddr.s_addr) ) {
	    continue;
	}
	
	if (e1->match.dst.ports[0] != 0 
	    && e1->match.dst.ports[0] != e2->match.dst.ports[0]) {
	    continue;
	}
	if (e1->match.dst.ports[1] 
	    && (e1->match.dst.ports[1] != e2->match.dst.ports[1])) {
	    continue;
	}
	matched = TRUE;
    } while (0);

    return matched;
}
