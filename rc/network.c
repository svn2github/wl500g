/*
 * Network services
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
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <signal.h>
typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <wlutils.h>
#include <nvparse.h>
#include <rc.h>
#include <bcmutils.h>
#include <etioctl.h>
#include <bcmparams.h>
#include <semaphore.h>
#include <fcntl.h>

void lan_up(char *lan_ifname);
int wait_for_ifup( char * prefix, char * wan_ifname, struct ifreq * ifr );

#if !defined(__UCLIBC_MAJOR__) \
 || __UCLIBC_MAJOR__ > 0 \
 || __UCLIBC_MINOR__ > 9 \
 || (__UCLIBC_MINOR__ == 9 && __UCLIBC_SUBLEVEL__ >= 32)

#define HOTPLUG_DEV_START
sem_t * hotplug_sem;
void
hotplug_sem_open()
{
	hotplug_sem = sem_open( "/hotplug_sem", O_CREAT | O_EXCL, S_IRWXU | S_IRWXG, 1 );
	if( hotplug_sem == SEM_FAILED ){
#ifdef DEBUG
		if(errno) dprintf( "%p, %s", hotplug_sem, strerror(errno) );
#endif
		hotplug_sem = sem_open( "/hotplug_sem", 0 );
#ifdef DEBUG
		if(errno) dprintf( "%p, %s", hotplug_sem, strerror(errno) );
#endif
	}
}

void
hotplug_sem_close()
{
	if(hotplug_sem) sem_close( hotplug_sem );
}

void
hotplug_sem_lock()
{
	if(hotplug_sem) sem_wait( hotplug_sem );
}

void
hotplug_sem_unlock()
{
	if(hotplug_sem) sem_post( hotplug_sem );
}
#else
#define hotplug_sem_open()
#define hotplug_sem_close()
#define hotplug_sem_lock()
#define hotplug_sem_unlock()
#endif
static int
add_routes(char *prefix, char *var, char *ifname)
{
	int err, m;
	char word[80], *next;
	char *ipaddr, *netmask, *gateway, *metric;
	char tmp[100];

	foreach(word, nvram_safe_get(strcat_r(prefix, var, tmp)), next) {

		netmask = word;
		ipaddr = strsep(&netmask, ":");
		if (!ipaddr || !netmask)
			continue;
		gateway = netmask;
		netmask = strsep(&gateway, ":");
		if (!netmask || !gateway)
			continue;
		metric = gateway;
		gateway = strsep(&metric, ":");
		if (!gateway || !metric)
			continue;
			
		if (inet_addr_(gateway) == INADDR_ANY) 
			gateway = nvram_safe_get("wanx_gateway");

		m = atoi(metric) + 1;
		dprintf("\n\n\nadd %s %d %s %s %s\n\n\n", ifname, m, ipaddr, gateway, netmask);
		
		if ((err = route_add(ifname, m, ipaddr, gateway, netmask))) {
			logmessage("route", "add failed(%d) '%s': %s dst %s mask %s gw %s metric %d",
			err, strerror(err),
			ifname, ipaddr, gateway, netmask, m);
		}
	}

	return 0;
}

static void
add_wanx_routes(char *prefix, char *ifname, int metric)
{
	char *routes, *tmp;
	char buf[30];
	struct in_addr mask;
	char *ipaddr, *gateway;
	int bits;

	char netmask[] = "255.255.255.255";

	if (!nvram_match("dr_enable_x", "1"))
		return;

	/* classful static routes */
	routes = strdup(nvram_safe_get(strcat_r(prefix, "routes", buf)));
	for (tmp = routes; tmp && *tmp; )
	{
		ipaddr  = strsep(&tmp, " ");
		gateway = strsep(&tmp, " ");

		if (gateway && strcmp(ipaddr, "0.0.0.0"))
			route_add(ifname, metric + 1, ipaddr, gateway, netmask);
	}
	free(routes);

	/* rfc classless static routes*/
	/* skip it like windows clients do
	routes = strdup(nvram_safe_get(strcat_r(prefix, "routes_rfc", buf))); */

	/* ms classless static routes*/
	routes = strdup(nvram_safe_get(strcat_r(prefix, "routes_ms", buf)));
	for (tmp = routes; tmp && *tmp; )
	{
		ipaddr  = strsep(&tmp, "/");
		bits    = atoi(strsep(&tmp, " "));
		gateway = strsep(&tmp, " ");

		if (gateway && bits > 0 && bits <= 32)
		{
			mask.s_addr = htonl(0xffffffff << (32 - bits));
			strcpy(netmask, inet_ntoa(mask));
			route_add(ifname, metric + 1, ipaddr, gateway, netmask);
		}
	}
	free(routes);

	return;
}

static int
del_routes(char *prefix, char *var, char *ifname)
{
	char word[80], *next;
	char *ipaddr, *netmask, *gateway, *metric;
	char tmp[100];
	
	foreach(word, nvram_safe_get(strcat_r(prefix, var, tmp)), next) {
		dprintf("add %s\n", word);
		
		netmask = word;
		ipaddr = strsep(&netmask, ":");
		if (!ipaddr || !netmask)
			continue;
		gateway = netmask;
		netmask = strsep(&gateway, ":");
		if (!netmask || !gateway)
			continue;
		metric = gateway;
		gateway = strsep(&metric, ":");
		if (!gateway || !metric)
			continue;
			
		if (inet_addr_(gateway) == INADDR_ANY) 
			gateway = nvram_safe_get("wanx_gateway");
		
		dprintf("add %s\n", ifname);
		
		route_del(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
	}

	return 0;
}

static int
add_lan_routes(char *lan_ifname)
{
	return add_routes("lan_", "route", lan_ifname);
}

static int
del_lan_routes(char *lan_ifname)
{
	return del_routes("lan_", "route", lan_ifname);
}

static void
start_igmpproxy(char *wan_ifname)
{
	static char *igmpproxy_conf = "/etc/igmpproxy.conf";
	struct stat	st_buf;
	FILE 		*fp;

	if (atoi(nvram_safe_get("udpxy_enable_x"))) {
		if (nvram_invmatch("udpxy_wan_x", "0"))
			eval("/usr/sbin/udpxy",
				"-m", wan_ifname, "-p", nvram_get("udpxy_enable_x"));
		else
			eval("/usr/sbin/udpxy", "-a", nvram_get("lan_ifname") ? : "br0",
				"-m", wan_ifname, "-p", nvram_get("udpxy_enable_x"));
	}
	
	if (!nvram_match("mr_enable_x", "1"))
		return;
	
	if (stat(igmpproxy_conf, &st_buf) != 0) 
	{
		if ((fp = fopen(igmpproxy_conf, "w")) == NULL) {
			perror(igmpproxy_conf);
			return;
		}
		
		fprintf(fp, "# automagically generated from web settings\n"
			"quickleave\n\n"
			"phyint %s upstream\n"
			"\taltnet %s\n\n"
			"phyint %s downstream ratelimit 0\n\n", 
			wan_ifname, 
			nvram_get("mr_altnet_x") ? : "0.0.0.0/0", 
			nvram_get("lan_ifname") ? : "br0");
			
		fclose(fp);
	}
	
	eval("/usr/sbin/igmpproxy", igmpproxy_conf);
}

void
start_lan(void)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");
	char name[80], *next;
	int s;
	struct ifreq ifr;

	dprintf("%s\n", lan_ifname);

 	/* Bring up bridged interfaces */
	if (strncmp(lan_ifname, "br", 2) == 0) {
		eval("brctl", "addbr", lan_ifname);
		eval("brctl", "setfd", lan_ifname, "0");
		if (nvram_match("router_disable", "1") || nvram_match("lan_stp", "0"))
			eval("brctl", "stp", lan_ifname, "dis");
#ifdef ASUS_EXT
		foreach(name, nvram_safe_get("lan_ifnames_t"), next) {
#else
		foreach(name, nvram_safe_get("lan_ifnames"), next) {
#endif
			/* Bring up interface */
			ifconfig(name, IFUP, NULL, NULL);
			/* Set the logical bridge address to that of the first interface */
			if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
				continue;
			strncpy(ifr.ifr_name, lan_ifname, IFNAMSIZ);
			if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0 &&
			    memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", ETHER_ADDR_LEN) == 0) {
				strncpy(ifr.ifr_name, name, IFNAMSIZ);
				if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0) {
					strncpy(ifr.ifr_name, lan_ifname, IFNAMSIZ);
					ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
					ioctl(s, SIOCSIFHWADDR, &ifr);
				}
			}
			close(s);
			/* If not a wl i/f then simply add it to the bridge */
			if (eval("wlconf", name, "up"))
			{
				eval("brctl", "addif", lan_ifname, name);
			}
			else 
			{
				/* get the instance number of the wl i/f */
				char wl_name[] = "wlXXXXXXXXXX_mode";
				int unit;
#ifdef ASUS_EXT
				/* do not play srom games, let asus firmware do everything */
				/* mac offset == 72 for sroms 1, 2, == 76 for srom 3 */
				/* sync_mac(name, nvram_safe_get("et0macaddr")); */
#endif
				wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit));
				snprintf(wl_name, sizeof(wl_name), "wl%d_mode", unit);
				/* Receive all multicast frames in WET mode */
				if (nvram_match(wl_name, "wet"))
					ifconfig(name, IFUP | IFF_ALLMULTI, NULL, NULL);
				/* Do not attach the main wl i/f if in wds mode */
				if (nvram_invmatch(wl_name, "wds"))
					eval("brctl", "addif",lan_ifname,name);
			}
		}
	}
	/* specific non-bridged lan i/f */
	else if (strcmp(lan_ifname, "")) {
		/* Bring up interface */
		ifconfig(lan_ifname, IFUP, NULL, NULL);
		/* config wireless i/f */
		if (!eval("wlconf", lan_ifname, "up")) {
			char tmp[100], prefix[] = "wanXXXXXXXXXX_";
			int unit;
			/* get the instance number of the wl i/f */
			wl_ioctl(lan_ifname, WLC_GET_INSTANCE, &unit, sizeof(unit));
			snprintf(prefix, sizeof(prefix), "wl%d_", unit);
			/* Receive all multicast frames in WET mode */
			if (nvram_match(strcat_r(prefix, "mode", tmp), "wet"))
				ifconfig(lan_ifname, IFUP | IFF_ALLMULTI, NULL, NULL);
		}
	}
	/* Get current LAN hardware address */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
		char eabuf[32];
		strncpy(ifr.ifr_name, lan_ifname, IFNAMSIZ);
		if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0)
			nvram_set("lan_hwaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
		close(s);
	}

#ifdef WPA2_WMM
	/* Set QoS mode */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
		int i, qos;
		caddr_t ifrdata;
		struct ethtool_drvinfo info;

		qos = (strcmp(nvram_safe_get("wl_wme"), "on")) ? 0 : 1;
		for (i = 1; i <= DEV_NUMIFS; i ++) {
			ifr.ifr_ifindex = i;
			if (ioctl(s, SIOCGIFNAME, &ifr))
				continue;
			if (ioctl(s, SIOCGIFHWADDR, &ifr))
				continue;
			if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER)
				continue;
			/* get flags */
			if (ioctl(s, SIOCGIFFLAGS, &ifr))
				continue;
			/* if up(wan not up yet at this point) */
			if (ifr.ifr_flags & IFF_UP) {
				ifrdata = ifr.ifr_data;
				memset(&info, 0, sizeof(info));
				info.cmd = ETHTOOL_GDRVINFO;
				ifr.ifr_data = (caddr_t)&info;
				if (ioctl(s, SIOCETHTOOL, &ifr) >= 0) {
					/* currently only need to set QoS to et devices */
					if (!strncmp(info.driver, "et", 2)) {
						ifr.ifr_data = (caddr_t)&qos;
						ioctl(s, SIOCSETCQOS, &ifr);
					}
				}
				ifr.ifr_data = ifrdata;
			}
		}
	}
	close(s);
#endif

#ifdef ASUS_EXT
	/* 
	* Configure DHCP connection. The DHCP client will run 
	* 'udhcpc bound'/'udhcpc deconfig' upon finishing IP address 
	* renew and release.
	*/
	if (nvram_match("router_disable", "1"))
	{
		if (nvram_match("lan_proto_x", "1")) 
		{
			char *dhcp_argv[] = { "udhcpc",
					      "-i", lan_ifname,
					      "-p", "/var/run/udhcpc_lan.pid",
					      "-s", "/tmp/landhcpc",
					      NULL
			};
			pid_t pid;


			/* Bring up and configure LAN interface */
			ifconfig(lan_ifname, IFUP,
		 		nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));

			symlink("/sbin/rc", "/tmp/landhcpc");

			/* Start dhcp daemon */
			_eval(dhcp_argv, NULL, 0, &pid);
		}
		else
		{
			/* Bring up and configure LAN interface */
			ifconfig(lan_ifname, IFUP,
		 		nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));
			lan_up(lan_ifname);

			update_lan_status(1);
		}
	}
	else
	{
		/* Bring up and configure LAN interface */
		ifconfig(lan_ifname, IFUP,
		 	nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));
		/* Install lan specific static routes */
		add_lan_routes(lan_ifname);

		update_lan_status(1);
	}
#else
	/* Bring up and configure LAN interface */
	ifconfig(lan_ifname, IFUP,
		 nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));

	/* Install lan specific static routes */
	add_lan_routes(lan_ifname);
#endif

#ifndef ASUS_EXT
	/* Start syslogd if either log_ipaddr or log_ram_enable is set */
	if (nvram_invmatch("log_ipaddr", "") || nvram_match("log_ram_enable", "1")) {
		char *argv[] = {
			"syslogd",
			NULL, 		/* -C */
			NULL, NULL,	/* -R host */
			NULL
		};
		int pid;
		int argc = 1;
		
		if (nvram_match("log_ram_enable", "1")) {
			argv[argc++] = "-C";
		}
		else if (!nvram_match("log_ram_enable", "0")) {
			nvram_set("log_ram_enable", "0");
		}
				
		if (nvram_invmatch("log_ipaddr", "")) {
			argv[argc++] = "-R";
			argv[argc++] = nvram_get("log_ipaddr");
		}

		_eval(argv, NULL, 0, &pid);
	}
#endif

	dprintf("%s %s\n",
		nvram_safe_get("lan_ipaddr"),
		nvram_safe_get("lan_netmask"));

#ifdef __CONFIG_IPV6__
	/* IPv6 address config */
	if (nvram_invmatch("ipv6_proto", ""))
	{
		char *ip6_addr = nvram_safe_get("ipv6_lan_addr");
		char *ip6_size = nvram_safe_get("ipv6_lan_netsize");
		if (*ip6_addr && *ip6_size)
		{
			char *ip6_net = (char*) malloc(INET6_ADDRSTRLEN);
			strcpy( ip6_net, ip6_addr );
			strcat( ip6_net, "/" );
			strcat( ip6_net, ip6_size );

			eval( "ip", "-6", "addr", "add", ip6_net, "dev", nvram_safe_get("lan_ifname") );

			free( ip6_net );
		}
	}
#endif

}

void
stop_lan(void)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");
	char name[80], *next;

	dprintf("%s\n", lan_ifname);

#ifndef ASUS_EXT
	/* Stop the syslogd daemon */
	eval("killall", "syslogd");
#endif

	/* Remove static routes */
	del_lan_routes(lan_ifname);

	/* Bring down LAN interface */
	ifconfig(lan_ifname, 0, NULL, NULL);

	/* Bring down bridged interfaces */
	if (strncmp(lan_ifname, "br", 2) == 0) {
#ifdef ASUS_EXT
		foreach(name, nvram_safe_get("lan_ifnames_t"), next) {
#else
		foreach(name, nvram_safe_get("lan_ifnames"), next) {
#endif
			eval("wlconf", name, "down");
			ifconfig(name, 0, NULL, NULL);
			eval("brctl", "delif", lan_ifname, name);
		}
		eval("brctl", "delbr", lan_ifname);
	}
	/* Bring down specific interface */
	else if (strcmp(lan_ifname, ""))
		eval("wlconf", lan_ifname, "down");
	
	dprintf("done\n");
}

static int
wan_prefix(char *ifname, char *prefix)
{
	int unit;
	
	if ((unit = wan_ifunit(ifname)) < 0)
		return -1;

	sprintf(prefix, "wan%d_", unit);
	return 0;
}

static int
add_wan_routes(char *wan_ifname)
{
	char prefix[] = "wanXXXXXXXXXX_";

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return -1;

	return add_routes(prefix, "route", wan_ifname);
}

static int
del_wan_routes(char *wan_ifname)
{
	char prefix[] = "wanXXXXXXXXXX_";

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return -1;

	return del_routes(prefix, "route", wan_ifname);
}

int
wan_valid(char *ifname)
{
	char name[80], *next;

	foreach(name, nvram_safe_get("wan_ifnames"), next)
		if (ifname && !strcmp(ifname, name))
			return 1;

	if (nvram_invmatch("wl_mode_ex", "ap")) {
		return nvram_match("wl0_ifname", ifname);
	}

	return 0;
}

void
start_wan(void)
{
	char *wan_ifname;
	char *wan_proto;
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char eabuf[32];
	int s;
	struct ifreq ifr;
	pid_t pid;

	/* check if we need to setup WAN */
	if (nvram_match("router_disable", "1"))
		return;

#if defined(__CONFIG_MADWIMAX__) || defined( __CONFIG_MODEM__)
	hotplug_sem_open();
#endif

#ifdef ASUS_EXT
	update_wan_status(0);
	/* start connection independent firewall */
	/* start_firewall(); */
#else
	/* start connection independent firewall */
	start_firewall();
#endif

	/* Create links */
	mkdir("/tmp/ppp", 0777);
	symlink("/sbin/rc", "/tmp/ppp/ip-up");
	symlink("/sbin/rc", "/tmp/ppp/ip-down");
	symlink("/sbin/rc", "/tmp/udhcpc");

#ifdef __CONFIG_MODEM__
	/* ppp contents */
//	eval("cp", "-dpR", "/usr/ppp", "/tmp");	
	mkdir("/tmp/ppp/peers", 0777);
#endif

	//symlink("/dev/null", "/tmp/ppp/connect-errors");

	/* Start each configured and enabled wan connection and its undelying i/f */
	for (unit = 0; unit < MAX_NVPARSE; unit ++) 
	{
#ifdef ASUS_EXT // Only multiple pppoe is allowed 
		if (unit>0 && nvram_invmatch("wan_proto", "pppoe")) break;
#endif

		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		/* make sure the connection exists and is enabled */ 
		wan_ifname = nvram_get(strcat_r(prefix, "ifname", tmp));
		if (!wan_ifname)
			continue;

		wan_proto = nvram_get(strcat_r(prefix, "proto", tmp));
		if (!wan_proto || !strcmp(wan_proto, "disabled"))
			continue;

		dprintf("%s %s\n\n\n\n\n", wan_ifname, wan_proto);

		/* disable the connection if the i/f is not in wan_ifnames */
		if (!wan_valid(wan_ifname)) {
			nvram_set(strcat_r(prefix, "proto", tmp), "disabled");
			continue;
		}
#ifdef __CONFIG_MADWIMAX__
		if (strcmp(wan_proto, "wimax") != 0)
		{
#endif
		/* Set i/f hardware address before bringing it up */
		if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
			continue;

		strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);

		/* Since WAN interface may be already turned up (by vlan.c),
		   if WAN hardware address is specified (and different than the current one),
		   we need to make it down for synchronizing hwaddr. */
		if (ioctl(s, SIOCGIFHWADDR, &ifr)) {
			close(s);
			continue;
		}

		ether_atoe(nvram_safe_get(strcat_r(prefix, "hwaddr", tmp)), eabuf);
		if (bcmp(eabuf, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN))
		{
			/* current hardware address is different than user specified */
			ifconfig(wan_ifname, 0, NULL, NULL);
		}

		/* Configure i/f only once, specially for wireless i/f shared by multiple connections */
		if (ioctl(s, SIOCGIFFLAGS, &ifr)) {
			close(s);
			continue;
		}
		else if ( !(ifr.ifr_flags & IFF_UP) )
		{
			/* Sync connection nvram address and i/f hardware address */
			memset(ifr.ifr_hwaddr.sa_data, 0, ETHER_ADDR_LEN);

			if (!nvram_invmatch(strcat_r(prefix, "hwaddr", tmp), "") ||
			    !ether_atoe(nvram_safe_get(strcat_r(prefix, "hwaddr", tmp)), ifr.ifr_hwaddr.sa_data) ||
			    !memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", ETHER_ADDR_LEN)) {
				if (ioctl(s, SIOCGIFHWADDR, &ifr)) {
					close(s);
					continue;
				}
				nvram_set(strcat_r(prefix, "hwaddr", tmp), ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
			}
			else {
				ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
				ioctl(s, SIOCSIFHWADDR, &ifr);
			}

			/* Bring up i/f */
			ifconfig(wan_ifname, IFUP, NULL, NULL);

			/* do wireless specific config */
			if (!eval("wlconf", wan_ifname, "up")) {
				/* Kick wl to join network */
				if (nvram_match("wl0_mode", "wet") || nvram_match("wl0_mode", "sta"))
					system(nvram_safe_get("wl0_join"));
			}
		}
		close(s);

#ifdef ASUS_EXT
		if (unit==0)
		{
			setup_ethernet(nvram_safe_get("wan_ifname"));
			start_pppoe_relay(wan_ifname);
		}
#endif

#ifdef __CONFIG_MADWIMAX__
		}
#endif

#ifdef ASUS_EXT
		if (unit==0)
		{
			FILE *fp;

			/* Enable Forwarding */
			if ((fp = fopen("/proc/sys/net/ipv4/ip_forward", "r+")))
			{
				fputc('1', fp);
				fclose(fp);
			} else
				perror("/proc/sys/net/ipv4/ip_forward");

#ifdef __CONFIG_IPV6__
			/* Enable IPv6 Forwarding */
			if (nvram_invmatch("ipv6_proto", ""))
			{
				if ((fp = fopen("/proc/sys/net/ipv6/conf/all/forwarding", "r+")))
				{
					fputc('1', fp);
					fclose(fp);
				} else
					perror("/proc/sys/net/ipv6/conf/all/forwarding");
			}
#endif
		}

		/* 
		* Configure PPPoE connection. The PPPoE client will run 
		* ip-up/ip-down scripts upon link's connect/disconnect.
		*/
		if (strcmp(wan_proto, "pppoe") == 0 || strcmp(wan_proto, "pptp") == 0 ||
		    strcmp(wan_proto, "l2tp") == 0) 
		{
			int demand = atoi(nvram_safe_get(strcat_r(prefix, "pppoe_idletime", tmp))) &&
			    strcmp(wan_proto, "l2tp") /* L2TP does not support idling */;
			
			/* update demand option */
			nvram_set(strcat_r(prefix, "pppoe_demand", tmp), demand ? "1" : "0");

			/* Bring up WAN interface */
			ifconfig(wan_ifname, IFUP, 
				nvram_get(strcat_r(prefix, "pppoe_ipaddr", tmp)),
				nvram_get(strcat_r(prefix, "pppoe_netmask", tmp)));

			/* start firewall */
			start_firewall_ex(nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp)),
				"0.0.0.0", "br0", nvram_safe_get("lan_ipaddr"));

		 	/* launch dhcp client and wait for lease forawhile */
		 	if (nvram_match(strcat_r(prefix, "pppoe_ipaddr", tmp), "0.0.0.0")) 
		 	{
				char *wan_hostname = nvram_get(strcat_r(prefix, "hostname", tmp));
				char *dhcp_argv[] = { "udhcpc",
					      "-i", wan_ifname,
					      "-p", (sprintf(tmp, "/var/run/udhcpc%d.pid", unit), tmp),
					      "-s", "/tmp/udhcpc",
					      "-b",
					      wan_hostname && *wan_hostname ? "-H" : NULL,
					      wan_hostname && *wan_hostname ? wan_hostname : NULL,
					      NULL
				};
				/* Start dhcp daemon */
				_eval(dhcp_argv, NULL, 0, NULL);
		 	} else {
			 	/* setup static wan routes via physical device */
				add_routes("wan_", "route", wan_ifname);
				/* and set default route if specified with metric 1 */
				if (inet_addr_(nvram_safe_get(strcat_r(prefix, "pppoe_gateway", tmp))) &&
				    !nvram_match("wan_heartbeat_x", ""))
					route_add(wan_ifname, 2, "0.0.0.0", 
						nvram_safe_get(strcat_r(prefix, "pppoe_gateway", tmp)), "0.0.0.0");
				/* start multicast router */
				start_igmpproxy(wan_ifname);
			}
			
			/* launch pppoe client daemon */
			start_pppd(prefix);

			/* ppp interface name is referenced from this point on */
			wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
			
			/* Pretend that the WAN interface is up */
			if (demand) 
			{
				if( ! wait_for_ifup( prefix, wan_ifname, &ifr ) ) continue;
			}
#ifdef ASUS_EXT
			nvram_set("wan_ifname_t", wan_ifname);
#endif
		}
#endif

#ifdef __CONFIG_MADWIMAX__
		else if (strcmp(wan_proto, "wimax") == 0){
			// wait for usb-device initializing
			sleep(1);
			/* launch wimax daemon */
			start_wimax(prefix);

			/* wimax interface name is referenced from this point on */
			wan_ifname = nvram_safe_get(strcat_r(prefix, "wimax_ifname", tmp));
#ifdef ASUS_EXT
			nvram_set("wan_ifname_t", wan_ifname);
#endif
			continue;
		}
#endif
		/* 
		* Configure DHCP connection. The DHCP client will run 
		* 'udhcpc bound'/'udhcpc deconfig' upon finishing IP address 
		* renew and release.
		*/
		else if (strcmp(wan_proto, "dhcp") == 0 ||
			 strcmp(wan_proto, "bigpond") == 0 )
		{
			char *wan_hostname = nvram_get(strcat_r(prefix, "hostname", tmp));
			char *dhcp_argv[] = { "udhcpc",
					      "-i", wan_ifname,
					      "-p", (sprintf(tmp, "/var/run/udhcpc%d.pid", unit), tmp),
					      "-s", "/tmp/udhcpc",
					      wan_hostname && *wan_hostname ? "-H" : NULL,
					      wan_hostname && *wan_hostname ? wan_hostname : NULL,
					      NULL
			};
			/* start firewall */
			start_firewall_ex(wan_ifname, "0.0.0.0", "br0", nvram_safe_get("lan_ipaddr"));
			/* Start dhcp daemon */
			_eval(dhcp_argv, NULL, 0, &pid);
			/* Update wan information for null DNS server */
			update_wan_status(1);
#ifdef ASUS_EXT
			wanmessage("Can not get IP from server");
			nvram_set("wan_ifname_t", wan_ifname);
#endif
		}
		/* Configure static IP connection. */
		else if (strcmp(wan_proto, "static") == 0 )
		{
			/* Assign static IP address to i/f */
			ifconfig(wan_ifname, IFUP,
				 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
				 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
			/* We are done configuration */
			wan_up(wan_ifname);
#ifdef ASUS_EXT
			nvram_set("wan_ifname_t", wan_ifname);
#endif
		}
#ifdef __CONFIG_MODEM__
		/* 
		* Configure PPP connection. The PPP client will run 
		* ip-up/ip-down scripts upon link's connect/disconnect.
		*/
		else if (strcmp(wan_proto, "usbmodem") == 0 )
		{
			int demand = atoi(nvram_safe_get(strcat_r(prefix, "modem_idletime", tmp)));
			
			/* update demand option */
			nvram_set(strcat_r(prefix, "modem_demand", tmp), demand ? "1" : "0");

			/* Bring up WAN interface */
			ifconfig(wan_ifname, IFUP, 
				nvram_get(strcat_r(prefix, "ipaddr", tmp)),
				nvram_get(strcat_r(prefix, "netmask", tmp)));

			/* start firewall */
			start_firewall_ex(nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp)),
				"0.0.0.0", "br0", nvram_safe_get("lan_ipaddr"));

			hotplug_sem_lock();
			nvram_set( strcat_r(prefix, "prepared", tmp), "1" );
			
			if( nvram_match( strcat_r(prefix, "dial_enabled", tmp), "1" ) )
			{
				/* launch ppp client daemon */
				start_modem_dial(prefix);
			} else
			{
				demand=0;
				nvram_set( strcat_r(prefix, "dial_enabled", tmp), "1" );
			}
			hotplug_sem_unlock();

			/* ppp interface name is referenced from this point on */
			wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
			
			/* Pretend that the WAN interface is up */
			if (demand)
			{
				if( ! wait_for_ifup( prefix, wan_ifname, &ifr ) ) continue;
			}
		}
#endif

#ifndef ASUS_EXT
		/* Start connection dependent firewall */
		start_firewall2(wan_ifname);
#endif

		dprintf("%s %s\n",
			nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
			nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
	}

#ifdef __CONFIG_IPV6__
	/* IPv6 address config */
	if (nvram_invmatch("ipv6_proto", ""))
	{
		char *ip6_ifname = "sixtun";
		char ip6_relay[INET6_ADDRSTRLEN] = "::";

		char *ip6_addr = nvram_safe_get("ipv6_wan_addr");
		char *ip6_size = nvram_safe_get("ipv6_wan_netsize");
		char *ip6_router = nvram_safe_get("ipv6_wan_router");

		/* Native IPv6 address */
		if (nvram_match("ipv6_proto", "native"))
		{
			ip6_ifname = nvram_safe_get("wan0_ifname");

		} else

		/* Tunnel 6in4 & 6to4 */
		if (nvram_match("ipv6_proto", "tun6in4") || nvram_match("ipv6_proto", "tun6to4"))
		{
			char *sit_local = "0.0.0.0";
			char *sit_remote = "any";

			/* Tunnel 6in4 */
			if (nvram_match("ipv6_proto", "tun6in4"))
				sit_remote = nvram_safe_get("ipv6_sit_remote");
			else

			/* Tunnel 6to4 */
			if (nvram_match("ipv6_proto", "tun6to4"))
			{
				sit_local = nvram_safe_get("ipv6_sit_local");
				strcat(ip6_relay, nvram_safe_get("ipv6_sit_relay"));
				ip6_router = ip6_relay;
			}

			/* Instantiate tunnel */
			eval("ip", "tunnel", "add", ip6_ifname, "mode", "sit",
				"local", sit_local, "remote", sit_remote,
				"ttl", nvram_safe_get("ipv6_sit_ttl"));
			/* Set MTU value */
			eval("ip", "link", "set", "mtu", nvram_safe_get("ipv6_sit_mtu"), "dev", ip6_ifname);
			/* Enable tunnel */
			eval("ip", "link", "set", ip6_ifname, "up");
		}

		/* Configurate wanif IPv6 address */
		if (*ip6_addr && *ip6_size)
		{
			char ip6_net[INET6_ADDRSTRLEN];

			sprintf(ip6_net, "%s/%s", ip6_addr, ip6_size);
        		eval("ip", "-6", "addr", "add", ip6_net, "dev", ip6_ifname);
        	}

        	/* Configurate remote IPv6 address (default gateway) */
		if (*ip6_router)
		{
			int size;

			if (nvram_match("ipv6_proto", "tun6to4"))
				eval("ip", "-6", "route", "add", "2002::/16", "dev", ip6_ifname);
			else
			if ((sscanf(ip6_router, "%[^/]/%d", ip6_relay, &size) == 2) && (size < 128)) {
				ip6_router = ip6_relay;
				eval("ip", "-6", "route", "add", ip6_router, "dev", ip6_ifname);
			}

			eval("ip", "-6", "route", "add", "default",
				"via", ip6_router, "dev", ip6_ifname);
		}

		/* IPv6 router advertisement daemon */
		if (nvram_match("ipv6_radvd_enable", "1"))
		{
			FILE *radvdconf = fopen( "/etc/radvd.conf", "w" );
			if (radvdconf != NULL)
			{
				int netsize = atoi(nvram_safe_get("ipv6_lan_netsize")), i = 15, n = netsize;
				struct in6_addr addr;
				char final[INET6_ADDRSTRLEN];

				/* Convert for easy manipulation */
				inet_pton(AF_INET6, nvram_safe_get("ipv6_lan_addr"), &addr);

				/* Clean complete bytes, starting from the end */
				while (n >= 8)
				{
					addr.s6_addr[i--] = 0;
					n -= 8;
				}

				/* Clear remaining bits */
				if (i > 0)
					addr.s6_addr[i] &= (255 << n);

				/* Convert back to string representation */
				inet_ntop(AF_INET6, &addr, final, INET6_ADDRSTRLEN);

				/* And write out to config file */
				fprintf( radvdconf,
					"interface %s {\n"
					" AdvSendAdvert on; prefix %s/%d { AdvOnLink on; AdvAutonomous on; };\n"
					"};",
					nvram_safe_get("lan_ifname"), final, netsize);
				fclose(radvdconf);

				/* Then start the radvd */
				eval("radvd");
			}
		}
	}
#endif

#if defined(__CONFIG_MADWIMAX__) || defined( __CONFIG_MODEM__)
	hotplug_sem_close();
#endif

}


#if defined(__CONFIG_MADWIMAX__) || defined(__CONFIG_MODEM__)
int 
stop_usb_communication_devices(void)
{
  	char *wan_ifname;
	char *wan_proto;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit;

	/* Start each configured and enabled wan connection and its undelying i/f */
	for( unit=0; unit<MAX_NVPARSE; unit++) 
	{
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		/* make sure the connection exists and is enabled */ 
		wan_ifname = nvram_get(strcat_r(prefix, "ifname", tmp));
		if (!wan_ifname)
			continue;

		wan_proto = nvram_get(strcat_r(prefix, "proto", tmp));
		if (!wan_proto || !strcmp(wan_proto, "disabled"))
			continue;

#ifdef __CONFIG_MADWIMAX__
		if( !strcmp(wan_proto, "wimax")) stop_wimax(prefix);
		else
#else
		{}
#endif
#ifdef __CONFIG_MODEM__
		if( !strcmp(wan_proto, "usbmodem")){
		    nvram_unset( strcat_r(prefix, "prepared", tmp));
		     stop_modem_dial(prefix); 
		}
#endif
	}
	return 0;
}
#endif


void
stop_wan(char *ifname)
{
	char name[80], *next, signal[] = "XXXX";
	
	eval("killall", "ntpclient");

	/* Shutdown and kill all possible tasks */
	eval("killall", "ip-up");
	eval("killall", "ip-down");
#ifdef __CONFIG_XL2TPD__
	eval("killall", "xl2tpd");
#else
	eval("killall", "l2tpd");
#endif
	eval("killall", "pppd");

#if defined(__CONFIG_MADWIMAX__) || defined(__CONFIG_MODEM__)
	stop_usb_communication_devices();
#endif
	snprintf(signal, sizeof(signal), "-%d", SIGUSR2);
	eval("killall", signal, "udhcpc");
	eval("killall", "udhcpc");
	eval("killall", "igmpproxy");
	eval("killall", "udpxy");
	eval("killall", "pppoe-relay");

	if (ifname)
	{
		wan_down(ifname);
	} else
	{
		/* Bring down WAN interfaces */
		foreach(name, nvram_safe_get("wan_ifnames"), next)
		{
			ifconfig(name, 0, NULL, NULL);
		}
	}

	/* Remove dynamically created links */
	unlink("/tmp/udhcpc");

	unlink("/tmp/ppp/ip-up");
	unlink("/tmp/ppp/ip-down");
	rmdir("/tmp/ppp");

#ifdef ASUS_EXT
	update_wan_status(0);
#endif

	dprintf("done\n");
}

static int 
update_resolvconf(char *ifname, int metric, int up)
{
	FILE *fp;
	char word[100], *next;

	dprintf("%s %d %d\n", ifname, metric, up);

	/* check if auto dns enabled */
	if (nvram_invmatch("wan_dnsenable_x", "1"))
		return 0;

	if (!(fp = fopen("/tmp/resolv.conf", "w+"))) {
		perror("/tmp/resolv.conf");
		return errno;
	}

	foreach(word, (nvram_get("wan0_dns") ? : 
		nvram_safe_get("wanx_dns")), next) 
	{
		fprintf(fp, "nameserver %s\n", word);
		dprintf( "nameserver %s\n", word );
	}

#ifdef __CONFIG_IPV6__
	if (nvram_invmatch("ipv6_proto", "") && nvram_invmatch("ipv6_dns1_x", ""))
 	{
 		next = nvram_safe_get("ipv6_dns1_x");
 		fprintf(fp, "nameserver %s\n", next);
		dprintf( "nameserver %s\n", next );
	}
#endif

	fclose(fp);

	/* Notify dnsmasq of change */
	eval("killall", "-1", "dnsmasq");

	return 0;
}

void
wan_up(char *wan_ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_proto, *gateway;
	int metric;

#ifdef DEBUG
	int r;
	r = wan_prefix(wan_ifname, prefix);
	dprintf("wan_up: %s %s %d\n", wan_ifname, prefix, r);
#endif

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0) 
	{
		/* called for dhcp+ppp */
		if (!nvram_match("wan0_ifname", wan_ifname))
			return;

		/* re-start firewall with old ppp0 address or 0.0.0.0 */
		start_firewall_ex("ppp0", nvram_safe_get("wan0_ipaddr"),
			"br0", nvram_safe_get("lan_ipaddr"));

	 	/* setup static wan routes via physical device */
		add_routes("wan_", "route", wan_ifname);
		/* and one supplied via DHCP */
		add_wanx_routes("wanx_", wan_ifname, 0);
		
		gateway = inet_addr_(nvram_safe_get("wan_gateway")) != INADDR_ANY ?
			nvram_get("wan_gateway") : nvram_safe_get("wanx_gateway");

		/* and default route with metric 1 */
		if (inet_addr_(gateway) != INADDR_ANY)
		{
			char word[100], *next;

			route_add(wan_ifname, 2, "0.0.0.0", gateway, "0.0.0.0");

			/* ... and to dns servers as well for demand ppp to work */
			if (nvram_match("wan_dnsenable_x", "1"))
				foreach(word, nvram_safe_get("wanx_dns"), next) 
			{
				in_addr_t mask = inet_addr(nvram_safe_get("wanx_netmask"));
				if ((inet_addr(word) & mask) != (inet_addr(nvram_safe_get("wanx_ipaddr")) & mask))
					route_add(wan_ifname, 2, word, gateway, "255.255.255.255");
			}
		}

		/* start multicast router */
		start_igmpproxy(wan_ifname);
		
		update_resolvconf(wan_ifname,2,1);
		
		return;
	}

	wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));

	dprintf("%s %s\n", wan_ifname, wan_proto);
	metric = atoi(nvram_safe_get(strcat_r(prefix, "priority", tmp)));

	/* Set default route to gateway if specified */
	if (nvram_match(strcat_r(prefix, "primary", tmp), "1")) 
	{
		if (strcmp(wan_proto, "dhcp") == 0 || 
		    strcmp(wan_proto, "static") == 0 
#ifdef __CONFIG_MADWIMAX__
		 || strcmp(wan_proto, "wimax") == 0
#endif
		){
			/* the gateway is in the local network */
			route_add(wan_ifname, 0, nvram_safe_get(strcat_r(prefix, "gateway", tmp)),
				NULL, "255.255.255.255");
		}
		/* default route via default gateway */
		
		dprintf("metric %s - %d\n",nvram_safe_get(strcat_r(prefix, "priority", tmp)),metric);
		route_add(wan_ifname, metric, "0.0.0.0", 
			nvram_safe_get(strcat_r(prefix, "gateway", tmp)), "0.0.0.0");
		/* hack: avoid routing cycles, when both peer and server has the same IP */
		if (strcmp(wan_proto, "pptp") == 0 || strcmp(wan_proto, "l2tp") == 0) {
			/* delete gateway route as it's no longer needed */
			route_del(wan_ifname, 0, nvram_safe_get(strcat_r(prefix, "gateway", tmp)),
				"0.0.0.0", "255.255.255.255");
		}
	}

	/* Install interface dependent static routes */
	add_wan_routes(wan_ifname);

 	/* setup static wan routes via physical device */
	if (    strcmp(wan_proto, "dhcp") == 0 
	     || strcmp(wan_proto, "static") == 0 
#ifdef __CONFIG_MADWIMAX__
	     || strcmp(wan_proto, "wimax") == 0 
#endif
	){
		nvram_set("wanx_gateway", nvram_safe_get(strcat_r(prefix, "gateway", tmp)));
		add_routes("wan_", "route", wan_ifname);
	}

	/* and one supplied via DHCP */
	if (    strcmp(wan_proto, "dhcp") == 0 
#ifdef __CONFIG_MADWIMAX__
	     || strcmp(wan_proto, "wimax") == 0 
#endif
	){
		add_wanx_routes(prefix, wan_ifname, 0);
	}

	/* Add dns servers to resolv.conf */
	update_resolvconf(wan_ifname, metric, 1 );

	/* Sync time */
	//start_ntpc();

#ifdef ASUS_EXT
	update_wan_status(1);
	start_firewall_ex(wan_ifname, nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), "br0", nvram_safe_get("lan_ipaddr"));
	start_ddns();
	stop_upnp();
	start_upnp();		
	if (strcmp(wan_proto, "bigpond")==0) {
		stop_bpalogin();
		start_bpalogin();
	}
#endif
	
#ifdef QOS
	// start qos related 
	start_qos(nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)));
#endif

	/* start multicast router */
	if (   strcmp(wan_proto, "dhcp") == 0
	    || strcmp(wan_proto, "bigpond") == 0
	    || strcmp(wan_proto, "static") == 0 
#ifdef __CONFIG_MADWIMAX__
	    || strcmp(wan_proto, "wimax") == 0
#endif
	){
		start_igmpproxy(wan_ifname);
	}

	dprintf("done\n");
}

void
wan_down(char *wan_ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_proto;
	int metric;

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return;

	wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));
	
	dprintf("%s %s\n", wan_ifname, wan_proto);

	/* Remove default route to gateway if specified */
	metric = atoi(nvram_safe_get(strcat_r(prefix, "priority", tmp)));
	if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
		route_del(wan_ifname, metric, "0.0.0.0", 
			nvram_safe_get(strcat_r(prefix, "gateway", tmp)),
			"0.0.0.0");

	/* Remove interface dependent static routes */
	del_wan_routes(wan_ifname);

	/* Update resolv.conf -- leave as is if no dns servers left for demand to work */
	if (*nvram_safe_get("wanx_dns"))
		nvram_unset(strcat_r(prefix, "dns", tmp));
	update_resolvconf(wan_ifname, metric, 0);

#ifdef ASUS_EXT
	update_wan_status(0);

	if (strcmp(wan_proto, "bigpond")==0) stop_bpalogin();
#endif

	dprintf("done\n");
}

#ifdef ASUS_EXT
void
lan_up(char *lan_ifname)
{
	FILE *fp;
	char word[100], *next;

	/* Set default route to gateway if specified */
	route_add(lan_ifname, 0, "0.0.0.0", 
			nvram_safe_get("lan_gateway"),
			"0.0.0.0");

	/* Open resolv.conf to read */
	if (!(fp = fopen("/tmp/resolv.conf", "w"))) {
		perror("/tmp/resolv.conf");
		return;
	}

	/*if (nvram_invmatch("lan_gateway", ""))
		fprintf(fp, "nameserver %s\n", nvram_safe_get("lan_gateway"));*/

	foreach(word, nvram_safe_get("lan_dns"), next)
	{
		fprintf(fp, "nameserver %s\n", word);
	}

#ifdef __CONFIG_IPV6__
	if (nvram_invmatch("ipv6_proto", "") && nvram_invmatch("ipv6_dns1_x", ""))
 	{
 		fprintf(fp, "nameserver %s\n", nvram_safe_get("ipv6_dns1_x"));
	}
#endif

	fclose(fp);

	/* Notify dnsmasq of change */
	eval("killall", "-1", "dnsmasq");
	
	/* Sync time */
	//start_ntpc();
}

#if 0
void
lan_down(char *lan_ifname)
{
	/* Remove default route to gateway if specified */
	route_del(lan_ifname, 0, "0.0.0.0", 
			nvram_safe_get("lan_gateway"),
			"0.0.0.0");

	/* remove resolv.conf */
	unlink("/tmp/resolv.conf");
}
#endif

void
lan_up_ex(char *lan_ifname)
{
	FILE *fp;
	char word[100], *next;

	/* Set default route to gateway if specified */
	route_add(lan_ifname, 0, "0.0.0.0", 
			nvram_safe_get("lan_gateway_t"),
			"0.0.0.0");

	/* Open resolv.conf to read */
	if (!(fp = fopen("/tmp/resolv.conf", "w"))) {
		perror("/tmp/resolv.conf");
		return;
	}

	/*if (nvram_invmatch("lan_gateway_t", ""))
		fprintf(fp, "nameserver %s\n", nvram_safe_get("lan_gateway_t"))*/;

	foreach(word, nvram_safe_get("lan_dns_t"), next)
	{
		fprintf(fp, "nameserver %s\n", word);
	}

#ifdef __CONFIG_IPV6__
	if (nvram_invmatch("ipv6_proto", "") && nvram_invmatch("ipv6_dns1_x", ""))
 	{
 		fprintf(fp, "nameserver %s\n", nvram_safe_get("ipv6_dns1_x"));
	}
#endif

	fclose(fp);

	/* Notify dnsmasq of change */
	eval("killall", "-1", "dnsmasq");
	
	/* Sync time */
	//start_ntpc();
	//update_lan_status(1);
}

void
lan_down_ex(char *lan_ifname)
{
	/* Remove default route to gateway if specified */
	route_del(lan_ifname, 0, "0.0.0.0", 
			nvram_safe_get("lan_gateway_t"),
			"0.0.0.0");

	/* remove resolv.conf */
	unlink("/tmp/resolv.conf");

	update_lan_status(0);
}



#endif

static int
notify_nas(char *type, char *ifname, char *action)
{
	char *argv[] = {"nas4not", type, ifname, action, 
			NULL,	/* role */
			NULL,	/* crypto */
			NULL,	/* auth */
			NULL,	/* passphrase */
			NULL,	/* ssid */
			NULL};
	char *str = NULL;
	int retries = 10;
	char tmp[100], prefix[] = "wlXXXXXXXXXX_";
	int unit;
	char remote[ETHER_ADDR_LEN];
	char ssid[48], pass[80], auth[16], crypto[16], role[8];
	int i;

	/* the wireless interface must be configured to run NAS */
	wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit));
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
#ifdef WPA2_WMM
	if (nvram_match(strcat_r(prefix, "akm", tmp), "") &&
	    nvram_match(strcat_r(prefix, "auth_mode", tmp), "none"))
#else
	if (nvram_match(strcat_r(prefix, "auth_mode", tmp), "open") ||
	    nvram_match(strcat_r(prefix, "auth_mode", tmp), "shared"))
#endif
		return 0;

	/* find WDS link configuration */
	wl_ioctl(ifname, WLC_WDS_GET_REMOTE_HWADDR, remote, ETHER_ADDR_LEN);
	for (i = 0; i < MAX_NVPARSE; i ++) {
		char mac[ETHER_ADDR_STR_LEN];
		uint8 ea[ETHER_ADDR_LEN];

		if (get_wds_wsec(unit, i, mac, role, crypto, auth, ssid, pass) &&
		    ether_atoe(mac, ea) && !bcmp(ea, remote, ETHER_ADDR_LEN)) {
			argv[4] = role;
			argv[5] = crypto;
			argv[6] = auth;
			argv[7] = pass;
			argv[8] = ssid;
			break;
		}
	}

	/* did not find WDS link configuration, use wireless' */
	if (i == MAX_NVPARSE) {
		/* role */
		argv[4] = "auto";
		/* crypto */
		argv[5] = nvram_safe_get(strcat_r(prefix, "crypto", tmp));
		/* auth mode */
#ifdef WPA2_WMM
		argv[6] = nvram_safe_get(strcat_r(prefix, "akm", tmp));
#else
		argv[6] = nvram_safe_get(strcat_r(prefix, "auth_mode", tmp));
#endif
		/* passphrase */
		argv[7] = nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp));
		/* ssid */
		argv[8] = nvram_safe_get(strcat_r(prefix, "ssid", tmp));
	}

	/* wait till nas is started */
	while (retries -- > 0 && !(str = file2str("/tmp/nas.lan.pid")))
		sleep(1);
	if (str) {
		int pid;
		free(str);
		return _eval(argv, ">/dev/console", 0, &pid);
	}
	return -1;
}

int
hotplug_net(void)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");
	char *interface, *action;

	if (!(interface = getenv("INTERFACE")) ||
	    !(action = getenv("ACTION")))
		return EINVAL;

	if (strncmp(interface, "wds", 3))
		return 0;

	if (!strcmp(action, "register")) {
		/* Bring up the interface and add to the bridge */
		ifconfig(interface, IFUP, NULL, NULL);
		
		/* Bridge WDS interfaces */
		if (!strncmp(lan_ifname, "br", 2) && 
		    eval("brctl", "addif", lan_ifname, interface))
		    return 0;

		/* Notify NAS of adding the interface */
		notify_nas("lan", interface, "up");
	}
	return 0;
}


int
wan_ifunit(char *wan_ifname)
{
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	if ((unit = ppp_ifunit(wan_ifname)) >= 0)
		return unit;
#ifdef __CONFIG_MADWIMAX__
	else
	if ((unit = wimax_ifunit(wan_ifname)) >= 0)
		return unit;
#endif
	else {
		for (unit = 0; unit < MAX_NVPARSE; unit ++) {
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname) &&
			    (nvram_match(strcat_r(prefix, "proto", tmp), "dhcp") ||
			     nvram_match(strcat_r(prefix, "proto", tmp), "bigpond") ||
			     nvram_match(strcat_r(prefix, "proto", tmp), "static")))
				return unit;
		}
	}
	return -1;
}

int
preset_wan_routes(char *wan_ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";


	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return -1;

	/* Set default route to gateway if specified */
	if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
	{
		route_add(wan_ifname, 0, "0.0.0.0", "0.0.0.0", "0.0.0.0");
	}

	/* Install interface dependent static routes */
	add_wan_routes(wan_ifname);
	return 0;
}

int
wan_primary_ifunit(void)
{
	int unit;
	
	for (unit = 0; unit < MAX_NVPARSE; unit ++) {
		char tmp[100], prefix[] = "wanXXXXXXXXXX_";
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
			return unit;
	}

	return 0;
}

// return 0 if failed
int wait_for_ifup( char * prefix, char * wan_ifname, struct ifreq * ifr )
{
	int timeout = 5;
	int s, pid;
	char tmp[200];

	char *ping_argv[] = { "ping", "-c1", "", NULL};

	/* Wait for pppx to be created */
	while (ifconfig(wan_ifname, IFUP, NULL, NULL) && timeout--)
		sleep(1);

	/* Retrieve IP info */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return 0;
	strncpy(ifr->ifr_name, wan_ifname, IFNAMSIZ);

	/* Set temporary IP address */
	if (ioctl(s, SIOCGIFADDR, ifr) < 0)
		perror(wan_ifname);
	nvram_set(strcat_r(prefix, "ipaddr", tmp), inet_ntoa(sin_addr(&ifr->ifr_addr)));
	nvram_set(strcat_r(prefix, "netmask", tmp), "255.255.255.255");

	/* Set temporary P-t-P address */
	if (ioctl(s, SIOCGIFDSTADDR, ifr) < 0)
		perror(wan_ifname);
	nvram_set(strcat_r(prefix, "gateway", tmp), inet_ntoa(sin_addr(&ifr->ifr_dstaddr)));

	close(s);

	/* 
	* Preset routes so that traffic can be sent to proper pppx even before 
	* the link is brought up.
	*/

	preset_wan_routes(wan_ifname);

	/* Stimulate link up */
	ping_argv[2] = nvram_safe_get(strcat_r(prefix, "gateway", tmp));
	_eval(ping_argv, NULL, 0, &pid);
	return 1;
}

#if defined(__CONFIG_MADWIMAX__) || defined(__CONFIG_MODEM)
void hotplug_network_device( char * interface, char * action, char * product )
{
	char *wan_ifname;
	char *wan_proto;
	char *device;
	int unit;
	int found=0;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	dprintf( "%s %s %s", interface, action, product );

	int action_add = (strcmp(action, "add") == 0);

	hotplug_sem_open();

	/* Start each configured and enabled wan connection and its undelying i/f */
	for (unit = 0; unit < MAX_NVPARSE; unit ++) 
	{
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		/* make sure the connection exists and is enabled */ 
		wan_ifname = nvram_get(strcat_r(prefix, "ifname", tmp));
		if (!wan_ifname)
			continue;

		wan_proto = nvram_get(strcat_r(prefix, "proto", tmp));
		if (!wan_proto || !strcmp(wan_proto, "disabled"))
			continue;

		dprintf("%s %s \n\n\n\n\n", wan_ifname, wan_proto);

		if( !found ){
#ifdef __CONFIG_MADWIMAX__
		    if( hotplug_check_wimax( interface, product, prefix ) ){
			found = 1;
		    } else 
#endif
#ifdef __CONFIG_MODEM__
		    if( hotplug_check_modem( interface, product, prefix ) ){
			found = 2;
		    }
#else
		    {}
#endif
		}
		if( found )
		{
		    hotplug_sem_lock();
		    if ( action_add )
		    {
			device = nvram_get( strcat_r(prefix, "usb_device", tmp) );
			if ( !device || !*device )
			{
#ifdef __CONFIG_MADWIMAX__
			    if ( found==1 && strcmp(wan_proto, "wimax") == 0 )
			    {
				nvram_set(strcat_r(prefix, "usb_device", tmp), product );
#ifdef HOTPLUG_DEV_START
				start_wimax( prefix );
#endif
			    } else
#endif
#ifdef __CONFIG_MODEM__
			    if ( found==2 && strcmp(wan_proto, "usbmodem") == 0 )
			    {
				nvram_set(strcat_r(prefix, "usb_device", tmp), product );
#ifdef HOTPLUG_DEV_START
				start_modem_dial( prefix );
#endif
			    }
#else
			    {}
#endif
			}
		    } else {
#ifdef __CONFIG_MADWIMAX__
			if ( found==1 && strcmp(wan_proto, "wimax") == 0 )
			{
			    nvram_unset(strcat_r(prefix, "usb_device", tmp) );
			} else
#endif
#ifdef __CONFIG_MODEM__
			if ( found==2 && strcmp(wan_proto, "usbmodem") == 0 )
			{
			    nvram_unset(strcat_r(prefix, "usb_device", tmp) );
			    stop_modem_dial(prefix);
			}
#else
			{}
#endif
		    }

		    hotplug_sem_unlock();

		    break;
		}
	}
	hotplug_sem_close();

	dprintf("done");
};
#endif
