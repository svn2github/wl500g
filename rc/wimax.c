#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>
#include <sys/stat.h>

#include <nvparse.h>

static char *pid_fname  = "/var/run/madwimax.pid";
static char *ev_fname   = "/tmp/madwimax.events";

const char * wimax_ifname_templ = "wmx%d";

#define WIMAX_UNIT 10

void update_nvram_wmx( char * ifname, int isup );

void get_wimax_ifname( char * out, int unit ){
   sprintf( out, wimax_ifname_templ, unit  );
};

int
wimax_modem(void)
{
//	dprintf("%s\n", nvram_safe_get("wimax_device"));
	if (!nvram_safe_get("wimax_device")) 
		return 0;

	return 1;
}

int
madwimax_start(char *ifname)
{

	int ret;

	update_nvram_wmx( ifname, 0 );
	
	char *wimax_ssid = nvram_get("wimax_ssid");
	char *wmx_argv[] = {
		"/usr/sbin/madwimax", "-qof",
		"-e", ev_fname,
		"-p", pid_fname,
		"-s", "/tmp/madwimax.log",
		"--ssid", wimax_ssid && *wimax_ssid ? wimax_ssid : "@yota.ru",
		"-i", ifname,
		NULL};
	pid_t pid;

	if (nvram_match("wimax_enable","1") && (wimax_modem()) )
	{
		eval("insmod", "tun");
		ret = _eval(wmx_argv, NULL, 0, &pid);
		symlink("/sbin/rc", ev_fname);
	}
	
	return ( nvram_match("wan_proto","wimax") );
}


time_t prev_time = 0;
// called from watchdog using the "madwimax-check" symlink to rc.
int
madwimax_check(void)
{
	char tmp[100];
	// check nvram wimax_enble
	if (!nvram_match("wimax_enable","1")) {
		return 0;
	}
	if (!wimax_modem()) {
		return 0;
	}

	// check the madwimax process
	pid_t pid = 0;

        FILE *fp = fopen(pid_fname, "r");
        char buf[256];
        if (fp && fgets(buf, sizeof(buf), fp)) {
                pid = strtoul(buf, NULL, 0);
                fclose(fp);
        }

        int unit = atoi(nvram_safe_get("wimax_unit"));

	if (pid > 0 && !nvram_match("wmx_chk_con", "0" ) ) {
		// TODO check the process state
		//dprintf( "wmx_chk_con enabled, madwimad pid %d", pid );
		time_t cur_time = time(NULL);
		if(	(cur_time - prev_time) > 60 || 
			cur_time < prev_time  )
		{
			get_wimax_ifname( tmp, unit );
			char s_pid[20];
			sprintf( s_pid, "%d", pid  );
			char * wchk_argv[] = {
				"/usr/sbin/wimax_check_connection.sh",
				s_pid,
				tmp,
				nvram_safe_get( "wmx_gateway_t" ),
				NULL
			};
			int ping_pid;
			_eval( wchk_argv, NULL, 0, &ping_pid);
			prev_time = cur_time;
		
			fp = fopen(pid_fname, "r");
			if (fp && fgets(buf, sizeof(buf), fp)) {
				pid = strtoul(buf, NULL, 0);
				fclose(fp);
			}
		}
	}
	// if no madwimax, then STOP THE UDHCPC
	if (pid == 0) {
	 	// kill the udhcpc
		kill_pidfile_s((sprintf(tmp, "/var/run/udhcpc%d.pid", unit), tmp), SIGUSR2);
		kill_pidfile((sprintf(tmp, "/var/run/udhcpc%d.pid", unit), tmp));
		get_wimax_ifname( tmp, unit );
		madwimax_start( tmp );
	}
	return 0;
}

//=========================== madwimax events ==================================

// if-create
int
madwimax_create(char *ifname)
{
	char tmp[100];
	char prefix[] = "wanXXXXXXXXXX_";

	dprintf( "ifname %s", ifname );

	if (nvram_match("wan_proto","wimax"))	// wimax - the main wan interface
	{
		nvram_set("wan_ifname",ifname);
		nvram_set("wan0_ifname",ifname);
		nvram_set("wan0_priority",nvram_safe_get("wimax_priority"));
		nvram_set("wan0_proto","wimax");
		nvram_set("wan_ifnames",ifname);
//		nvram_set("wimax_prefix", "wan0_");

	}
	else // ------ for using in future ------
	{
		snprintf(prefix, sizeof(prefix), "wan%d_", WIMAX_UNIT );// ???????? update_wan_status // MAX_NVPARSE-1
		nvram_set(strcat_r(prefix, "ifname", tmp), ifname);
		nvram_set(strcat_r(prefix, "proto", tmp), "wimax");
		nvram_set(strcat_r(prefix, "primary", tmp), "1");
		nvram_set(strcat_r(prefix, "priority", tmp), nvram_safe_get("wimax_priority"));
		nvram_set(strcat_r(prefix, "dnsenable_x", tmp), "1");	//nvram_safe_get("wimax_priority"));

		snprintf(tmp, sizeof(tmp), "%d", WIMAX_UNIT);
		nvram_set("wimax_unit", tmp);
	}
	return 1;
}

//if-release
int
madwimax_release(char *ifname)
{
	char tmp[100];
	char prefix[] = "wanXXXXXXXXXX_";

	dprintf( "ifname %s", ifname );

	int unit = wan_ifunit(ifname);

	if ( unit ) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit );

		unlink(ev_fname);

		if (nvram_match("wan_proto","wimax")) {
			stop_wan();
			nvram_set("wan_ifname","");
			nvram_set("wan0_ifname","");
			nvram_set("wan_ifnames","");
		} else {
			nvram_unset(strcat_r(prefix, "ifname", tmp));
			nvram_unset(strcat_r(prefix, "proto", tmp));
			nvram_unset(strcat_r(prefix, "primary", tmp));
		}
	}

	return 1;
}

//if-up
int
madwimax_up(char *ifname)
{
	char tmp[100];
	pid_t pid;
	char prefix[] = "wanXXXXXXXXXX_";

	dprintf( "ifname %s", ifname );

	int unit = WIMAX_UNIT;

	if ( nvram_match("wan_ipaddr", "0.0.0.0") || nvram_match("wan_ipaddr", "") ){
		unit = atoi(nvram_safe_get("wimax_unit"));
		snprintf(prefix, sizeof(prefix), "wan%d_", unit );
		nvram_set(strcat_r(prefix, "proto", tmp), "wimax");

		char *dhcp_argv[] = { "udhcpc",
				      "-i", ifname,
				      "-p", (sprintf(tmp, "/var/run/udhcpc%d.pid", unit), tmp),
				      "-s", "/tmp/udhcpc",
				      NULL
		};
		/* Start dhcp daemon */
		_eval(dhcp_argv, NULL, 0, &pid);
	} else {
			ifconfig(ifname, IFUP,
				 nvram_safe_get("wan_ipaddr"), 
				 nvram_safe_get("wan_netmask") );
	}
	wan_up( ifname );
	update_nvram_wmx( ifname, 1 );
	return 1;
}

//if-down
int
madwimax_down(char *ifname)
{
	dprintf( "ifname %s", ifname );

	wan_down(ifname);
	update_nvram_wmx( ifname, 0 );
	
	char tmp[100];
	int unit = wan_ifunit(ifname);
 	// kill the udhcpc
	kill_pidfile_s((sprintf(tmp, "/var/run/udhcpc%d.pid", unit), tmp), SIGUSR2);
	kill_pidfile((sprintf(tmp, "/var/run/udhcpc%d.pid", unit), tmp));
	return 0;
}

void update_nvram_wmx( char * ifname, int isup )
{
	dprintf( "ifname: %s, is up: %d", ifname, isup );
	if( isup ){
		nvram_set("wmx_ifname_t", ifname );
		nvram_set("wmx_ipaddr_t",nvram_safe_get("wan_ipaddr_t"));
		nvram_set("wmx_netmask_t",nvram_safe_get("wan_netmask_t"));
		nvram_set("wmx_gateway_t",nvram_safe_get("wan_gateway_t"));
		nvram_set("wmx_dns_t",nvram_safe_get("wan_dns_t"));
	} else {
		nvram_set("wmx_ifname_t","");
		nvram_set("wmx_ipaddr_t","");
		nvram_set("wmx_netmask_t","");
		nvram_set("wmx_gateway_t","");
		nvram_set("wmx_dns_t","");
	}
	nvram_set("wmx_ping_t","");
};

int
madwimax_main(int argc, char **argv)
{

	dprintf("%s %s %s\n", argv[0], argv[1], argv[2]);

	if (argc != 3)
		return(0);

	if (!strncmp(argv[1], "if-create", 5 ))
		return madwimax_create(argv[2]);
	else if (!strncmp(argv[1], "if-up", 5))
		return madwimax_up(argv[2]);
	else if (!strncmp(argv[1], "if-down", 5))
		return madwimax_down(argv[2]);
	else if (!strncmp(argv[1], "if-release", 5))
		return madwimax_release(argv[2]);
	else return 0;
}
