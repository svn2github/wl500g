/*
 * Miscellaneous services
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
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>

#ifndef ASUS_EXT
int
start_dhcpd(void)
{
	FILE *fp;
	char name[100];

	if (nvram_match("router_disable", "1") || nvram_invmatch("lan_proto", "dhcp"))
		return 0;

	dprintf("%s %s %s %s\n",
		nvram_safe_get("lan_ifname"),
		nvram_safe_get("dhcp_start"),
		nvram_safe_get("dhcp_end"),
		nvram_safe_get("lan_lease"));

	/* Touch leases file */
	if (!(fp = fopen("/tmp/udhcpd.leases", "a"))) {
		perror("/tmp/udhcpd.leases");
		return errno;
	}
	fclose(fp);

	/* Write configuration file based on current information */
	if (!(fp = fopen("/tmp/udhcpd.conf", "w"))) {
		perror("/tmp/udhcpd.conf");
		return errno;
	}
	fprintf(fp, "pidfile /var/run/udhcpd.pid\n");
	fprintf(fp, "start %s\n", nvram_safe_get("dhcp_start"));
	fprintf(fp, "end %s\n", nvram_safe_get("dhcp_end"));
	fprintf(fp, "interface %s\n", nvram_safe_get("lan_ifname"));
	fprintf(fp, "remaining yes\n");
	fprintf(fp, "lease_file /tmp/udhcpd.leases\n");
	fprintf(fp, "option subnet %s\n", nvram_safe_get("lan_netmask"));
	fprintf(fp, "option router %s\n", nvram_safe_get("lan_ipaddr"));
	fprintf(fp, "option dns %s\n", nvram_safe_get("lan_ipaddr"));
	fprintf(fp, "option lease %s\n", nvram_safe_get("lan_lease"));
	snprintf(name, sizeof(name), "%s_wins", nvram_safe_get("dhcp_wins"));
	if (nvram_invmatch(name, ""))
		fprintf(fp, "option wins %s\n", nvram_get(name));
	snprintf(name, sizeof(name), "%s_domain", nvram_safe_get("dhcp_domain"));
	if (nvram_invmatch(name, ""))
		fprintf(fp, "option domain %s\n", nvram_get(name));
	fclose(fp);

	eval("udhcpd", "/tmp/udhcpd.conf");

	dprintf("done\n");
	return 0;
}

int
stop_dhcpd(void)
{
	char sigusr1[] = "-XX";
	int ret;

/*
* Process udhcpd handles two signals - SIGTERM and SIGUSR1
*
*  - SIGUSR1 saves all leases in /tmp/udhcpd.leases
*  - SIGTERM causes the process to be killed
*
* The SIGUSR1+SIGTERM behavior is what we like so that all current client
* leases will be honorred when the dhcpd restarts and all clients can extend
* their leases and continue their current IP addresses. Otherwise clients
* would get NAK'd when they try to extend/rebind their leases and they 
* would have to release current IP and to request a new one which causes 
* a no-IP gap in between.
*/
	sprintf(sigusr1, "-%d", SIGUSR1);
	eval("killall", sigusr1, "udhcpd");
	ret = eval("killall", "udhcpd");

	dprintf("done\n");
	return ret;
}

int
start_dns(void)
{
	int ret;
	FILE *fp;
	
	if (nvram_match("router_disable", "1"))
		return 0;

	/* Create resolv.conf with empty nameserver list */
	if (!(fp = fopen("/tmp/resolv.conf", "w"))) {
		perror("/tmp/resolv.conf");
		return errno;
	}
	fclose(fp);

	/* launch dns relay */
	ret = eval("dnsmasq", "-h",
		   "-i", nvram_safe_get("lan_ifname"),
		   "-r", "/tmp/resolv.conf");

	dprintf("done\n");
	return ret;
}	

int
stop_dns(void)
{
	int ret = eval("killall", "dnsmasq");
	
	/* Remove resolv.conf */
	unlink("/tmp/resolv.conf");

	dprintf("done\n");
	return ret;
}	
#endif

int
start_telnetd(void)
{
	int ret;
	
	/* telnet is enabled by default */
	if (nvram_invmatch("telnet_enable", "1"))
		return 0;

	char *argv[] = {"telnetd", NULL};
	pid_t pid;

	ret = _eval(argv, NULL, 0, &pid);

	dprintf("done\n");
	return ret;
}

int
stop_telnetd(void)
{
	int ret = eval("killall", "telnetd");

	dprintf("done\n");
	return ret;
}

int
start_dropbear(void)
{
	if (nvram_match("ssh_enable", "0"))
		return 0;

	int ret = eval("dropbearstart");

	dprintf("done\n");
	return ret;
}

int
stop_dropbear(void)
{
	int ret = eval("killall", "dropbear");

	dprintf("done\n");
	return ret;
}

int
start_snmpd(void)
{
	static char *snmpd_conf = "/tmp/snmpd.conf";
	
	int ret;
	FILE *fp;
	
	if (!nvram_match("snmp_enable", "1"))
		return 0;
		
	/* create snmpd.conf  */
	if ((fp = fopen(snmpd_conf, "w")) == NULL) {
		perror(snmpd_conf);
		return 1;
	}

	fprintf(fp, "# automagically generated from web settings\n");

	fprintf(fp, "community %s\n", nvram_get("snmp_community") ? : "public");
	fprintf(fp, "syscontact %s\n", nvram_get("snmp_contact") ? : "Administrator");
	fprintf(fp, "syslocation %s\n", nvram_get("snmp_location") ? : "Unknown");

	fclose(fp);
	
	ret = eval("snmpd", "-c", snmpd_conf);

	dprintf("done\n");
	return ret;
}

int
stop_snmpd(void)
{
	int ret = eval("killall", "snmpd");

	dprintf("done\n");
	return ret;
}

int
start_httpd(void)
{
	int ret;

	chdir("/www");
#ifdef ASUS_EXT
	if (nvram_invmatch("router_disable", "1"))
		ret = eval("httpd", nvram_safe_get("wan0_ifname"));
	else
#endif
	ret = eval("httpd");

	chdir("/");

	dprintf("done\n");
	return ret;
}

int
stop_httpd(void)
{
	int ret = eval("killall", "httpd");

	dprintf("done\n");
	return ret;
}

int
start_upnp(void)
{
	char *wan_ifname, *wan_proto;
	int ret;
	char var[100], prefix[] = "wanXXXXXXXXXX_";

	if (!nvram_invmatch("upnp_enable", "0") || nvram_match("router_disable", "1"))
		return 0;
	
	ret = eval("killall", "-SIGUSR1", "upnp");
	if (ret != 0) {
	    snprintf(prefix, sizeof(prefix), "wan%d_", wan_primary_ifunit());
	    wan_proto = nvram_safe_get(strcat_r(prefix, "proto", var));
	    wan_ifname = nvram_match("upnp_enable", "1") && (strcmp(wan_proto, "pppoe") == 0 || 
	    	strcmp(wan_proto, "pptp") ==0 || strcmp(wan_proto, "l2tp") == 0) ? 
					nvram_safe_get(strcat_r(prefix, "pppoe_ifname", var)) :
					nvram_safe_get(strcat_r(prefix, "ifname", var));
	    ret = eval("upnp", "-D",
		       "-L", nvram_safe_get("lan_ifname"),
		       "-W", wan_ifname);

	}
	dprintf("done\n");
	return ret;
}

int
stop_upnp(void)
{
	int ret = 0;

	if (nvram_invmatch("upnp_enable", "0"))
	    ret = eval("killall", "upnp");

	dprintf("done\n");
	return ret;
}

int
start_nas(char *type)
{
	char cfgfile[64];
	char pidfile[64];
	if (!type || !*type)
		type = "lan";
	snprintf(cfgfile, sizeof(cfgfile), "/tmp/nas.%s.conf", type);
	snprintf(pidfile, sizeof(pidfile), "/tmp/nas.%s.pid", type);
	{
		char *argv[] = {"nas", cfgfile, pidfile, type, NULL};
		pid_t pid;

		_eval(argv, NULL, 0, &pid);
		dprintf("done\n");
	}
	return 0;
}

int
stop_nas(void)
{
	int ret = eval("killall", "nas");

	dprintf("done\n");
	return ret;
}

int
start_ntpc(void)
{
	pid_t pid;
	char *servers = nvram_safe_get("ntp_servers");
	
	if (strlen(servers)) {
		char *ntp_argv[] = {"ntpclient", "-h", servers, "-i", "3", "-c", "1", "-lst", NULL};
		_eval(ntp_argv, NULL, 0, &pid);
	}
	
	return pid;
}

int
stop_ntpc(void)
{
	int ret = eval("killall", "ntpclient");

	return ret;
}

int
start_services(void)
{
	start_telnetd();
	start_dropbear();
	start_httpd();
	start_dns();
	start_dhcpd();
#ifdef ASUS_EXT
	start_logger();
#endif
	start_snmpd();
	start_upnp();
	start_nas("lan");

#ifdef ASUS_EXT
	start_usb();
#endif

	dprintf("done\n");
	return 0;
}

int
stop_services(void)
{
	preshutdown_system();

#ifdef ASUS_EXT
	stop_usb();
#endif
	stop_nas();
	stop_upnp();
	stop_snmpd();
#ifdef ASUS_EXT
	stop_logger();
#endif
	stop_dhcpd();
	stop_dns();
	stop_httpd();
	stop_dropbear();
	stop_telnetd();

	dprintf("done\n");
	return 0;
}
