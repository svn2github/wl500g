/*
 * Router default NVRAM values
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

#include <epivers.h>
#include <string.h>
#include <ctype.h>
#include <bcmnvram.h>
#include <typedefs.h>
#include <wlioctl.h>
#include <stdio.h>
#include <ezc.h>
#include <bcmconfig.h>

#define XSTR(s) STR(s)
#define STR(s) #s

struct nvram_tuple router_defaults[] = {
	/* OS parameters */
	{ "os_name", "", 0 },			/* OS name string */
	{ "os_version", EPI_VERSION_STR, 0 },	/* OS revision */
	{ "os_date", __DATE__, 0 },		/* OS date */

	/* Miscellaneous parameters */
	{ "timer_interval", "3600", 0 },	/* Timer interval in seconds */
	{ "ntp_server0", "pool.ntp.org", 0 },	/* NTP server */
	{ "time_zone", "GMT0", 0 },		/* Time zone (GNU TZ format) */
	{ "log_level", "0", 0 },		/* Bitmask 0:off 1:denied 2:accepted */
	{ "upnp_enable", "1", 0 },		/* Start UPnP */
#ifdef __CONFIG_MINIUPNPD__
	{ "upnp_proto", "0", 0 },		/* 0: Both, 1: UPNP, 2: NAT-PMP */
#endif
	{ "ezc_enable", "1", 0 },		/* Enable EZConfig updates */
	{ "ezc_version", EZC_VERSION_STR, 0 },	/* EZConfig version */
	{ "is_default", "1", 0 },		/* is it default setting: 1:yes 0:no*/
	{ "os_server", "", 0 },			/* URL for getting upgrades */
	{ "console_loglevel", "0", 0 },		/* Kernel panics only */

	/* Big switches */
	{ "router_disable", "0", 0 },		/* lan_proto=static lan_stp=0 wan_proto=disabled */
	{ "fw_disable", "0", 0 },		/* Disable firewall (allow new connections from the WAN) */

	{ "log_ipaddr", "", 0 },		/* syslog recipient */
	
	/* LAN H/W parameters */
	{ "lan_ifname", "br0", 0 },		/* LAN interface name */
	{ "lan_ifnames", "", 0 },		/* Enslaved LAN interfaces */
	{ "lan_hwnames", "", 0 },		/* LAN driver names (e.g. et0) */
	{ "lan_hwaddr", "", 0 },		/* LAN interface MAC address */
	
	/* LAN TCP/IP parameters */
	{ "lan_dhcp", "0", 0 },			/* DHCP client [static|dhcp] */
	{ "lan_ipaddr", "192.168.1.1", 0 },	/* LAN IP address */
	{ "lan_netmask", "255.255.255.0", 0 },	/* LAN netmask */
	{ "lan_gateway", "192.168.1.1", 0 },	/* LAN gateway */
	{ "lan_proto", "dhcp", 0 },		/* DHCP server [static|dhcp] */
	{ "lan_wins", "", 0 },			/* x.x.x.x x.x.x.x ... */
	{ "lan_domain", "", 0 },		/* LAN domain name */
	{ "lan_lease", "86400", 0 },		/* LAN lease time in seconds */
	{ "lan_stp", "1", 0 },			/* LAN spanning tree protocol */
	{ "lan_route", "", 0 },			/* Static routes (ipaddr:netmask:gateway:metric:ifname ...) */

#ifdef __CONFIG_NAT__
	/* WAN H/W parameters */
	{ "wan_ifname", "", 0 },		/* WAN interface name */
	{ "wan_ifnames", "", 0 },		/* WAN interface names */
	{ "wan_hwname", "", 0 },		/* WAN driver name (e.g. et1) */
	{ "wan_hwaddr", "", 0 },		/* WAN interface MAC address */
	
	/* WAN TCP/IP parameters */
	{ "wan_proto", "dhcp", 0 },		/* [static|dhcp|pppoe|disabled] */
	{ "wan_ipaddr", "0.0.0.0", 0 },		/* WAN IP address */
	{ "wan_netmask", "0.0.0.0", 0 },	/* WAN netmask */
	{ "wan_gateway", "0.0.0.0", 0 },	/* WAN gateway */
	{ "wan_dns", "", 0 },			/* x.x.x.x x.x.x.x ... */
	{ "wan_wins", "", 0 },			/* x.x.x.x x.x.x.x ... */
	{ "wan_hostname", "", 0 },		/* WAN hostname */
	{ "wan_domain", "", 0 },		/* WAN domain name */
	{ "wan_lease", "86400", 0 },		/* WAN lease time in seconds */
	
	/* PPPoE parameters */
	{ "wan_pppoe_ifname", "", 0 },		/* PPPoE enslaved interface */
	{ "wan_pppoe_username", "", 0 },	/* PPP username */
	{ "wan_pppoe_passwd", "", 0 },		/* PPP password */
	{ "wan_pppoe_idletime", "0", 0 },	/* Dial on demand max idle time (seconds) */
	{ "wan_pppoe_keepalive", "0", 0 },	/* Restore link automatically */
	{ "wan_pppoe_demand", "0", 0 },		/* Dial on demand */
	{ "wan_pppoe_mru", "1492", 0 },		/* Negotiate MRU to this value */
	{ "wan_pppoe_mtu", "1492", 0 },		/* Negotiate MTU to the smaller of this value or the peer MRU */
	{ "wan_pppoe_service", "", 0 },		/* PPPoE service name */
	{ "wan_pppoe_ac", "", 0 },		/* PPPoE access concentrator name */

	/* Misc WAN parameters */
	{ "wan_desc", "", 0 },			/* WAN connection description */
	{ "wan_route", "", 0 },			/* Static routes (ipaddr:netmask:gateway:metric:ifname ...) */
	{ "wan_primary", "0", 0 },		/* Primary wan connection */

	{ "wan_unit", "0", 0 },			/* Last configured connection */
	
	/* Filters */
	{ "filter_maclist", "", 0 },		/* xx:xx:xx:xx:xx:xx ... */
	{ "filter_macmode", "deny", 0 },	/* "allow" only, "deny" only, or "disabled" (allow all) */

	/* Port forwards */
	{ "dmz_ipaddr", "", 0 },		/* x.x.x.x (equivalent to 0-60999>dmz_ipaddr:0-60999) */
	{ "forward_port0", "", 0 },		/* wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1[:,]proto[:,]enable[:,]desc */
	{ "autofw_port0", "", 0 },		/* out_proto:out_port,in_proto:in_port0-in_port1>to_port0-to_port1,enable,desc */

	/* DHCP server parameters */
	{ "dhcp_start", "192.168.1.101", 0 },	/* First assignable DHCP address */
	{ "dhcp_end", "192.168.1.254", 0 },	/* Last assignable DHCP address */
	{ "dhcp_lease", "86400", 0},		/* Lease time in seconds */
	{ "dhcp_domain", "wan", 0 },		/* Use WAN domain name first if available (wan|lan) */
	{ "dhcp_wins", "wan", 0 },		/* Use WAN WINS first if available (wan|lan) */
#endif	/* __CONFIG_NAT__ */

#ifdef __CONFIG_IPV6__
	/* Main parameters */
	{ "ipv6_proto", "", 0 },
	{ "ipv6_if_x", "0", 0 },

	/* LAN parameters */
	{ "ipv6_lanauto_x", "0", 0 },
	{ "ipv6_lan_addr", "", 0 },
	{ "ipv6_lan_netsize", "64", 0 },

	/* WAN parameters */
	{ "ipv6_wanauto_x", "0", 0 },
	{ "ipv6_wan_addr", "", 0 },
	{ "ipv6_wan_netsize", "64", 0 },
	{ "ipv6_wan_router", "", 0 },

	/* Tunnel parameters */
	{ "ipv6_sit_remote", "", 0 },
	{ "ipv6_sit_relay", "192.88.99.1", 0 },
	{ "ipv6_6rd_router", "", 0 },
	{ "ipv6_6rd_ip4size", "0", 0 },
	{ "ipv6_sit_mtu", "1480", 0 },
	{ "ipv6_sit_ttl", "64", 0 },

	/* DNSv6 parameters */
	{ "ipv6_dnsenable_x", "0", 0 },
	{ "ipv6_dns1_x", "", 0 },
	{ "ipv6_dns2_x", "", 0 },
	{ "ipv6_dns3_x", "", 0 },

	{ "ipv6_radvd_enable", "1", 0 },
//	{ "ipv6_radvd_lifetime", "86400", 0 },
	{ "ipv6_radvd_dns1_x", "", 0 },
#endif  /* __CONFIG_IPV6__ */

	/* Web server parameters */
	{ "http_username", "admin", 0 },	/* Username */
	{ "http_passwd", "admin", 0 },		/* Password */
	{ "http_wanport", "", 0 },		/* WAN port to listen on */
	{ "http_lanport", "80", 0 },		/* LAN port to listen on */

	/* Wireless parameters */
	{ "wl_ifname", "", 0 },			/* Interface name */
	{ "wl_hwaddr", "", 0 },			/* MAC address */
	{ "wl_phytype", "", 0 },		/* Current wireless band ("a" (5 GHz), "b" (2.4 GHz), or "g" (2.4 GHz)) */
	{ "wl_corerev", "", 0 },		/* Current core revision */
	{ "wl_phytypes", "", 0 },		/* List of supported wireless bands (e.g. "ga") */
	{ "wl_radioids", "", 0 },		/* List of radio IDs */
	{ "wl_ssid", "default", 0 },		/* Service set ID (network name) */
	{ "wl_bss_enabled", "1", 0 },		/* See "default_get" below. */
	{ "wl_country_code", "", 0 },		/* Country code (default obtained from driver) */
	{ "wl_radio", "1", 0 },			/* Enable (1) or disable (0) radio */
	{ "wl_closed", "0", 0 },		/* Closed (hidden) network */
	{ "wl_ap_isolate", "0", 0 },            /* AP isolate mode */
	{ "wl_wmf_bss_enable", "1", 0 },	/* WMF Enable/Disable */
	{ "wl_mcast_regen_bss_enable", "1", 0 },	/* MCAST REGEN Enable/Disable */
	{ "wl_rxchain_pwrsave_enable", "1", 0 },	/* Rxchain powersave enable */
	{ "wl_rxchain_pwrsave_quiet_time", "1800", 0 },	/* Quiet time for power save */
	{ "wl_rxchain_pwrsave_pps", "10", 0 },	/* Packets per second threshold for power save */
	{ "wl_radio_pwrsave_enable", "0", 0 },  /* Radio powersave enable */
	{ "wl_radio_pwrsave_quiet_time", "1800", 0 },   /* Quiet time for power save */
	{ "wl_radio_pwrsave_pps", "10", 0 },    /* Packets per second threshold for power save */
	{ "wl_radio_pwrsave_on_time", "50", 0 },        /* Radio on time for power save */
	{ "wl_mode", "ap", 0 },			/* AP mode (ap|sta|wds) */
	{ "wl_lazywds", "0", 0 },		/* Enable "lazy" WDS mode (0|1) */
	{ "wl_wds", "", 0 },			/* xx:xx:xx:xx:xx:xx ... */
	{ "wl_wds_timeout", "0", 0 },		/* WDS link detection interval */
	{ "wl_wep", "disabled", 0 },		/* WEP data encryption (enabled|disabled) */
	{ "wl_auth", "0", 0 },			/* Shared key authentication optional (0) or required (1) */
	{ "wl_key", "1", 0 },			/* Current WEP key */
	{ "wl_key1", "", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key2", "", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key3", "", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key4", "", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_maclist", "", 0 },		/* xx:xx:xx:xx:xx:xx ... */
	{ "wl_macmode", "disabled", 0 },	/* "allow" only, "deny" only, or "disabled" (allow all) */
	{ "wl_channel", "0", 0 },		/* Channel number */
	{ "wl_reg_mode", "off", 0 },		/* Regulatory: 802.11H(h)/802.11D(d)/off(off) */
	{ "wl_dfs_preism", "60", 0 },		/* 802.11H pre network CAC time */
	{ "wl_dfs_postism", "60", 0 },		/* 802.11H In Service Monitoring CAC time */
	/* Radar thrs params format: version thresh0_20 thresh1_20 thresh0_40 thresh1_40 */
	{ "wl_radarthrs", "0 0x6a8 0x6c8 0x6ac 0x6c7", 0 },
	{ "wl_rate", "0", 0 },			/* Rate (bps, 0 for auto) */
	{ "wl_mrate", "0", 0 },			/* Mcast Rate (bps, 0 for auto) */
	{ "wl_rateset", "default", 0 },		/* "default" or "all" or "12" */
	{ "wl_frag", "2346", 0 },		/* Fragmentation threshold */
	{ "wl_rts", "2347", 0 },		/* RTS threshold */
	{ "wl_dtim", "3", 0 },			/* DTIM period */
	{ "wl_bcn", "100", 0 },			/* Beacon interval */
	{ "wl_bcn_rotate", "1", 0 },		/* Beacon rotation */
	{ "wl_plcphdr", "long", 0 },		/* 802.11b PLCP preamble type */
	{ "wl_gmode", XSTR(GMODE_AUTO), 0 },	/* 54g mode */
	{ "wl_gmode_protection", "auto", 0 },	/* 802.11g RTS/CTS protection (off|auto) */
	{ "wl_afterburner", "off", 0 },		/* AfterBurner */
	{ "wl_frameburst", "off", 0 },		/* BRCM Frambursting mode (off|on) */
	{ "wl_antdiv", "-1", 0 },		/* Antenna Diversity (-1|0|1|3) */
	{ "wl_infra", "1", 0 },			/* Network Type (BSS/IBSS) */
	{ "wl_nctrlsb", "lower", 0},		/* N-CTRL SB (none/lower/upper) */
	{ "wl_nband", "2", 0},			/* N-BAND */
	{ "wl_nmcsidx", "-1", 0},		/* MCS Index for N - rate */
	{ "wl_nmode", "-1", 0},			/* N-mode */
	{ "wl_rifs_advert", "auto", 0},		/* RIFS mode advertisement */
	{ "wl_vlan_prio_mode", "off", 0},	/* VLAN Priority support */

#ifdef __CONFIG_BCMWL5__
	{ "wl_rxstreams", "0", 0},              /* 802.11n Rx Streams, 0 is invalid, WLCONF will
						 * change it to a radio appropriate default
						 */
	{ "wl_txstreams", "0", 0},              /* 802.11n Tx Streams 0, 0 is invalid, WLCONF will
						 * change it to a radio appropriate default
						 */
	{ "wl_stbc_tx", "auto", 0 },		/* Default STBC TX setting */
	{ "wl_stbc_rx", "1", 0 },		/* Default STBC RX setting */
	{ "wl_ampdu", "auto", 0 },		/* Default AMPDU setting */
	/* Default AMPDU retry limit per-tid setting */
	{ "wl_ampdu_rtylimit_tid", "7 7 7 7 7 7 7 7", 0 },
	/* Default AMPDU regular rate retry limit per-tid setting */
	{ "wl_ampdu_rr_rtylimit_tid", "3 3 3 3 3 3 3 3", 0 },
	{ "wl_amsdu", "auto", 0 },		/* Default AMSDU setting */
	{ "wl_obss_coex", "1", 0 },
#endif

	/* WPA parameters */
	{ "wl_auth_mode", "open", 0 },		/* Network authentication mode */
	{ "wl_wpa_psk", "", 0 },		/* WPA pre-shared key */
	{ "wl_wpa_mode", "0", 0},
	{ "wl_wpa_gtk_rekey", "0", 0 },		/* GTK rotation interval */
	{ "wl_radius_ipaddr", "", 0 },		/* RADIUS server IP address */
	{ "wl_radius_key", "", 0 },		/* RADIUS shared secret */
	{ "wl_radius_port", "1812", 0 },	/* RADIUS server UDP port */
	{ "wl_crypto", "tkip", 0 },		/* WPA data encryption */
	{ "wl_net_reauth", "36000", 0 },	/* Network Re-auth/PMK caching duration */
	{ "wl_akm", "", 0 },			/* Authenticated Key Management list */

	/* WME parameters */
	{ "wl_wme", "on", 0 },		/* WME mode (off|on|auto) */
	{ "wl_wme_no_ack", "off", 0},		/* WME No-Acknowledgmen mode */
	{ "wl_wme_apsd", "on", 0 },		/* WME APSD mode */

	/* EDCA parameters for STA */
#ifdef __CONFIG_BCMWL5__
	{ "wl_wme_sta_be", "15 1023 3 0 0 off off", 0 },	/* WME STA AC_BE parameters */
	{ "wl_wme_sta_bk", "15 1023 7 0 0 off off", 0 },	/* WME STA AC_BK parameters */
	{ "wl_wme_sta_vi", "7 15 2 6016 3008 off off", 0 },	/* WME STA AC_VI parameters */
	{ "wl_wme_sta_vo", "3 7 2 3264 1504 off off", 0 },	/* WME STA AC_VO parameters */
#else
	{ "wl_wme_sta_bk", "15 1023 7 0 0 off", 0 },	/* WME STA AC_BK paramters */
	{ "wl_wme_sta_be", "15 1023 3 0 0 off", 0 },	/* WME STA AC_BE paramters */
	{ "wl_wme_sta_vi", "7 15 2 6016 3008 off", 0 },	/* WME STA AC_VI paramters */
	{ "wl_wme_sta_vo", "3 7 2 3264 1504 off", 0 },	/* WME STA AC_VO paramters */
#endif

	/* EDCA parameters for AP */
#ifdef __CONFIG_BCMWL5__
	{ "wl_wme_ap_be", "15 63 3 0 0 off off", 0 },		/* WME AP AC_BE parameters */
	{ "wl_wme_ap_bk", "15 1023 7 0 0 off off", 0 },		/* WME AP AC_BK parameters */
	{ "wl_wme_ap_vi", "7 15 1 6016 3008 off off", 0 },	/* WME AP AC_VI parameters */
	{ "wl_wme_ap_vo", "3 7 1 3264 1504 off off", 0 },	/* WME AP AC_VO parameters */
#else
	{ "wl_wme_ap_bk", "15 1023 7 0 0 off", 0 },	/* WME AP AC_BK paramters */
	{ "wl_wme_ap_be", "15 63 3 0 0 off", 0 },	/* WME AP AC_BE paramters */
	{ "wl_wme_ap_vi", "7 15 1 6016 3008 off", 0 },	/* WME AP AC_VI paramters */
	{ "wl_wme_ap_vo", "3 7 1 3264 1504 off", 0 },	/* WME AP AC_VO paramters */
#endif

	/* Per AC Tx parameters */
#ifdef __CONFIG_BCMWL5__
	{ "wl_wme_txp_be", "7 3 4 2 0", 0 },	/* WME AC_BE Tx parameters */
	{ "wl_wme_txp_bk", "7 3 4 2 0", 0 },	/* WME AC_BK Tx parameters */
	{ "wl_wme_txp_vi", "7 3 4 2 0", 0 },	/* WME AC_VI Tx parameters */
	{ "wl_wme_txp_vo", "7 3 4 2 0", 0 },	/* WME AC_VO Tx parameters */
#endif

	{ "wl_wme_bss_disable", "0", 0 },	/* WME BSS disable advertising (off|on) */

	{ "wl_maxassoc", "128", 0},		/* Max associations driver could support */
	{ "wl_bss_maxassoc", "128", 0},		/* Max associations driver could support */

	{ "wl_unit", "0", 0 },			/* Last configured interface */
	{ "wl_sta_retry_time", "5", 0 },	/* Seconds between association attempts */
#ifdef BCMDBG
	{ "wl_nas_dbg", "0", 0 },		/* Enable/Disable NAS Debugging messages */
#endif

#ifdef __CONFIG_EMF__
	/* EMF defaults */
	{ "emf_entry", "", 0 },			/* Static MFDB entry (mgrp:if) */
	{ "emf_uffp_entry", "", 0 },		/* Unreg frames forwarding ports */
	{ "emf_rtport_entry", "", 0 },		/* IGMP frames forwarding ports */
	{ "emf_enable", "0", 0 },		/* Disable EMF by default */
#endif /* __CONFIG_EMF__ */

	/* Restore defaults */
	{ "restore_defaults", "0", 0 },		/* Set to 0 to not restore defaults on boot */

#include "flash.default"

	{ 0, 0, 0 }
};

/* Translates from, for example, wl0_ (or wl0.1_) to wl_. */
/* Only single digits are currently supported */

static void
fix_name(const char *name, char *fixed_name)
{
	char *pSuffix = NULL;

	/* Translate prefix wlx_ and wlx.y_ to wl_ */
	/* Expected inputs are: wld_root, wld.d_root, wld.dd_root
	 * We accept: wld + '_' anywhere
	 */
	pSuffix = strchr(name, '_');

	if ((strncmp(name, "wl", 2) == 0) && isdigit(name[2]) && (pSuffix != NULL)) {
		strcpy(fixed_name, "wl");
		strcpy(&fixed_name[2], pSuffix);
		return;
	}

	/* No match with above rules: default to input name */
	strcpy(fixed_name, name);
}


/* 
 * Find nvram param name; return pointer which should be treated as const
 * return NULL if not found.
 *
 * NOTE:  This routine special-cases the variable wl_bss_enabled.  It will
 * return the normal default value if asked for wl_ or wl0_.  But it will
 * return 0 if asked for a virtual BSS reference like wl0.1_.
 */
char *
nvram_default_get(const char *name)
{
	int idx;
	char fixed_name[NVRAM_MAX_VALUE_LEN];

	fix_name(name, fixed_name);
	if (strcmp(fixed_name, "wl_bss_enabled") == 0) {
                if (name[3] == '.' || name[4] == '.') { /* Virtual interface */
                        return "0";
                }
        }

        for (idx = 0; router_defaults[idx].name != NULL; idx++) {
                if (strcmp(router_defaults[idx].name, fixed_name) == 0) {
                        return router_defaults[idx].value;
                }
        }

        return NULL;
}

