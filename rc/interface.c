/*
 * Linux network interface code
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <proto/ethernet.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmparams.h>
#include <rc.h>

in_addr_t
inet_addr_(const char *cp)
{
	struct in_addr a;

	if (!inet_aton(cp, &a))
		return INADDR_ANY;
	else
		return a.s_addr;
}

int
ifconfig(char *name, int flags, char *addr, char *netmask)
{
	int s, ret = 0;
	struct ifreq ifr;
	struct in_addr in_addr, in_netmask, in_broadaddr;

	/* Open a raw socket to the kernel */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return errno;

	/* Set interface name */
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	
	/* Set interface flags */
	ifr.ifr_flags = flags;
	if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0)
		goto err;
	
	/* Set IP address */
	if (addr && *addr) {
		inet_aton(addr, &in_addr);
		sin_addr(&ifr.ifr_addr).s_addr = in_addr.s_addr;
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFADDR, &ifr) < 0)
			goto err;
	}

	/* Set IP netmask and broadcast */
	if ((addr && *addr) && (netmask && *netmask)) {
		inet_aton(netmask, &in_netmask);
		sin_addr(&ifr.ifr_netmask).s_addr = in_netmask.s_addr;
		ifr.ifr_netmask.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFNETMASK, &ifr) < 0)
			goto err;

		in_broadaddr.s_addr = (in_addr.s_addr & in_netmask.s_addr) | ~in_netmask.s_addr;
		sin_addr(&ifr.ifr_broadaddr).s_addr = in_broadaddr.s_addr;
		ifr.ifr_broadaddr.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFBRDADDR, &ifr) < 0)
			goto err;
	}
	goto fexit;

 err:
	ret = errno;
	perror(name);
 fexit:
	close(s);
	return ret;
}

static int
route_manip(int cmd, char *name, int metric, char *dst, char *gateway, char *genmask)
{
	int s, ret = 0;
	struct rtentry rt;

	/* Open a raw socket to the kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return errno;

	/* Fill in rtentry */
	memset(&rt, 0, sizeof(rt));
	if (dst)
		inet_aton(dst, &sin_addr(&rt.rt_dst));
	if (gateway)
		inet_aton(gateway, &sin_addr(&rt.rt_gateway));
	if (genmask)
		inet_aton(genmask, &sin_addr(&rt.rt_genmask));
	rt.rt_metric = metric;
	rt.rt_flags = RTF_UP;
	if (sin_addr(&rt.rt_gateway).s_addr)
		rt.rt_flags |= RTF_GATEWAY;
	if (sin_addr(&rt.rt_genmask).s_addr == INADDR_BROADCAST)
		rt.rt_flags |= RTF_HOST;
	rt.rt_dev = name;

	/* Force address family to AF_INET */
	rt.rt_dst.sa_family = AF_INET;
	rt.rt_gateway.sa_family = AF_INET;
	rt.rt_genmask.sa_family = AF_INET;
		
	if (ioctl(s, cmd, &rt) < 0)
	{
		ret = errno;
		dprintf("route %s %s: %s\n", (cmd==SIOCADDRT)?"add":"del",
			name, strerror(ret));
	}

	close(s);
	return ret;
}

int
route_add(char *name, int metric, char *dst, char *gateway, char *genmask)
{
	return route_manip(SIOCADDRT, name, metric, dst, gateway, genmask);
}

int
route_del(char *name, int metric, char *dst, char *gateway, char *genmask)
{
	return route_manip(SIOCDELRT, name, metric, dst, gateway, genmask);
}

/* configure loopback interface */
void
config_loopback(void)
{
	/* Bring up loopback interface */
	ifconfig("lo", IFUP, "127.0.0.1", "255.0.0.0");

	/* Add to routing table */
	route_add("lo", 0, "127.0.0.0", "0.0.0.0", "255.0.0.0");
}

#ifndef CONFIG_SENTRY5
/* configure/start vlan interface(s) based on nvram settings */
int
start_vlan(void)
{
	int s;
	struct ifreq ifr;
	int i, j, vlan0tag;
	char ea[ETHER_ADDR_LEN];

	/* set vlan i/f name to style "vlan<ID>" */
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");

	/* create vlan interfaces */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return errno;

	vlan0tag = strtoul(nvram_safe_get("vlan0tag"), NULL, 0);

	for (i = 0; i <= VLAN_MAXVID; i ++) {
		char nvvar_name[16];
		char vlan_id[16];
		char *hwname, *hwaddr;
		char prio[8];
		/* get the address of the EMAC on which the VLAN sits */
		snprintf(nvvar_name, sizeof(nvvar_name), "vlan%dhwname", i);
		if (!(hwname = nvram_get(nvvar_name)))
			continue;
		snprintf(nvvar_name, sizeof(nvvar_name), "%smacaddr", hwname);
		if (!(hwaddr = nvram_get(nvvar_name)))
			continue;
		ether_atoe(hwaddr, ea);
		/* find the interface name to which the address is assigned */
		for (j = 1; j <= DEV_NUMIFS; j ++) {
			ifr.ifr_ifindex = j;
			if (ioctl(s, SIOCGIFNAME, &ifr))
				continue;
			if (ioctl(s, SIOCGIFHWADDR, &ifr))
				continue;
			if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER)
				continue;
			if (!bcmp(ifr.ifr_hwaddr.sa_data, ea, ETHER_ADDR_LEN))
				break;
		}
		if (j > DEV_NUMIFS)
			continue;
		if (ioctl(s, SIOCGIFFLAGS, &ifr))
			continue;
		if (!(ifr.ifr_flags & IFF_UP))
			ifconfig(ifr.ifr_name, IFUP, 0, 0);
		/* create the VLAN interface */
		snprintf(vlan_id, sizeof(vlan_id), "%d", i | vlan0tag);
		eval("vconfig", "add", ifr.ifr_name, vlan_id);
		/* setup ingress map (vlan->priority => skb->priority) */
		snprintf(vlan_id, sizeof(vlan_id), "vlan%d", i | vlan0tag);
		for (j = 0; j < VLAN_NUMPRIS; j ++) {
			snprintf(prio, sizeof(prio), "%d", j);
			eval("vconfig", "set_ingress_map", vlan_id, prio, prio);
		}
	}

	close(s);

	return 0;
}

/* stop/rem vlan interface(s) based on nvram settings */
int
stop_vlan(void)
{
	int i, vlan0tag;
	char nvvar_name[16];
	char vlan_id[16];
	char *hwname;

	vlan0tag = strtoul(nvram_safe_get("vlan0tag"), NULL, 0);

	for (i = 0; i <= VLAN_MAXVID; i ++) {
		/* get the address of the EMAC on which the VLAN sits */
		snprintf(nvvar_name, sizeof(nvvar_name), "vlan%dhwname", i);
		if (!(hwname = nvram_get(nvvar_name)))
			continue;

		/* remove the VLAN interface */
		snprintf(vlan_id, sizeof(vlan_id), "vlan%d", i | vlan0tag);
		eval("vconfig", "rem", vlan_id);
	}

	return 0;
}
#endif
