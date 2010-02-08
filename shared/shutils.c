/*
 * Shell-like utility functions
 *
 * Copyright 2005, Broadcom Corporation
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
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include <bcmnvram.h>
#include <shutils.h>

/* Linux specific headers */
#ifdef linux
#include <error.h>
#include <termios.h>
#include <sys/time.h>
#include <net/ethernet.h>
#endif /* linux */

#define MAX_NVPARSE 255

#ifdef linux

mode_t 
exists(const char *path)
{
	struct stat buf;
	
	return stat(path, &buf) ? 0 : buf.st_mode;
}


/*
 * Reads file and returns contents
 * @param	fd	file descriptor
 * @return	contents of file or NULL if an error occurred
 */
char *
fd2str(int fd)
{
	char *buf = NULL;
	size_t count = 0, n;

	do {
		buf = realloc(buf, count + 512);
		n = read(fd, buf + count, 512);
		if (n < 0) {
			free(buf);
			buf = NULL;
		}
		count += n;
	} while (n == 512);

	close(fd);
	if (buf)
		buf[count] = '\0';
	return buf;
}

/*
 * Reads file and returns contents
 * @param	path	path to file
 * @return	contents of file or NULL if an error occurred
 */
char *
file2str(const char *path)
{
	int fd;

	if ((fd = open(path, O_RDONLY)) == -1) {
		perror(path);
		return NULL;
	}

	return fd2str(fd);
}

/* 
 * Waits for a file descriptor to change status or unblocked signal
 * @param	fd	file descriptor
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @return	1 if descriptor changed status or 0 if timed out or -1 on error
 */
int
waitfor(int fd, int timeout)
{
	fd_set rfds;
	struct timeval tv = { timeout, 0 };

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	return select(fd + 1, &rfds, NULL, NULL, (timeout > 0) ? &tv : NULL);
}

/* 
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param	argv	argument list
 * @param	path	NULL, ">output", or ">>output"
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @param	ppid	NULL to wait for child termination or pointer to pid
 * @return	return value of executed command or errno
 */
int
_eval(char *const argv[], char *path, int timeout, int *ppid)
{
	sigset_t set, omask;
	pid_t pid, ret;
	int status;
	int fd;
	int flags;
	int sig;

	/* Block SIGCHLD signal to avoid interaction with its handler */
	if (ppid == NULL) {
		sigemptyset(&set);
		sigaddset(&set, SIGCHLD);
		sigprocmask(SIG_BLOCK, &set, &omask);
	}

	switch (pid = fork()) {
	case -1:	/* error */
		perror("fork");
		return errno;
	case 0:		/* child */
		/* Reset signal handlers set for parent process */
		for (sig = 0; sig < (_NSIG-1); sig++)
			signal(sig, SIG_DFL);

		/* Unblock signals if called from signal handler */
		sigemptyset(&set);
		sigprocmask(SIG_SETMASK, &set, NULL);

		/* Clean up */
		ioctl(0, TIOCNOTTY, 0);
		close(STDIN_FILENO);
		setsid();

		fd = open("/dev/null", O_RDWR); /* stdin */

		/* Redirect stdout to <path> */
		if (path) {
			flags = O_WRONLY | O_CREAT;
			if (!strncmp(path, ">>", 2)) {
				/* append to <path> */
				flags |= O_APPEND;
				path += 2;
			} else if (!strncmp(path, ">", 1)) {
				/* overwrite <path> */
				flags |= O_TRUNC;
				path += 1;
			}
			if ((fd = open(path, flags, 0644)) < 0)
				perror(path);
			else {
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
				close(fd);
			}
		} else {
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
		}

		/* execute command */
		dprintf("%s\n", argv[0]);
		setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin", 1);
		alarm(timeout);
		execvp(argv[0], argv);
		perror(argv[0]);
		exit(errno);
	default:	/* parent */
		if (ppid != NULL) {
			*ppid = pid;
			return 0;
		} else {
			do
				ret = waitpid(pid, &status, 0);
			while ((ret == -1) && (errno = EINTR));

			// Restore signals, errno should be preserved
			flags = errno;
			sigprocmask(SIG_SETMASK, &omask, NULL);
			errno = flags;

			if (ret != pid) {
				perror("waitpid");
				return errno;
			}
			if (WIFEXITED(status))
				return WEXITSTATUS(status);
			else
				return status;
		}
	}
}

/* 
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param	argv	argument list
 * @return	stdout of executed command or NULL if an error occurred
 */
char *
_backtick(char *const argv[])
{
	int filedes[2];
	pid_t pid;
	int status;
	char *buf = NULL;

	/* create pipe */
	if (pipe(filedes) == -1) {
		perror(argv[0]);
		return NULL;
	}

	switch (pid = fork()) {
	case -1:	/* error */
		return NULL;
	case 0:		/* child */
		close(filedes[0]);	/* close read end of pipe */
		dup2(filedes[1], 1);	/* redirect stdout to write end of pipe */
		close(filedes[1]);	/* close write end of pipe */
		execvp(argv[0], argv);
		exit(errno);
		break;
	default:	/* parent */
		close(filedes[1]);	/* close write end of pipe */
		buf = fd2str(filedes[0]);
		waitpid(pid, &status, 0);
		break;
	}
	
	return buf;
}

/*
 * Kills process whose PID is stored in plaintext in pidfile
 * @param       pidfile PID file, signal
 * @return      0 on success and errno on failure
 */
int inline 
kill_pidfile(char *pidfile)
{
        return kill_pidfile_s(pidfile, SIGTERM);
}

int
kill_pidfile_s(char *pidfile, int sig)
{
	FILE *fp = fopen(pidfile, "r");
	char buf[256];

	if (fp && fgets(buf, sizeof(buf), fp)) {
		pid_t pid = strtoul(buf, NULL, 0);
		fclose(fp);
		if (pid <= 0)
			return -ESRCH;
		else
			return kill(pid, sig);
	} else
		return errno;
}

/*
 * fread() with automatic retry on syscall interrupt
 * @param	ptr	location to store to
 * @param	size	size of each element of data
 * @param	nmemb	number of elements
 * @param	stream	file stream
 * @return	number of items successfully read
 */
int
safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret = 0;

	do {
		clearerr(stream);
		ret += fread((char *)ptr + (ret * size), size, nmemb - ret, stream);
	} while (ret < nmemb && ferror(stream) && errno == EINTR);

	return ret;
}

/*
 * fwrite() with automatic retry on syscall interrupt
 * @param	ptr	location to read from
 * @param	size	size of each element of data
 * @param	nmemb	number of elements
 * @param	stream	file stream
 * @return	number of items successfully written
 */
int
safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret = 0;

	do {
		clearerr(stream);
		ret += fwrite((char *)ptr + (ret * size), size, nmemb - ret, stream);
	} while (ret < nmemb && ferror(stream) && errno == EINTR);

	return ret;
}

#endif /* linux */
/*
 * Convert Ethernet address string representation to binary data
 * @param	a	string in xx:xx:xx:xx:xx:xx notation
 * @param	e	binary data
 * @return	TRUE if conversion was successful and FALSE otherwise
 */
int
ether_atoe(const char *a, unsigned char *e)
{
	char *c = (char *) a;
	int i = 0;

	memset(e, 0, ETHER_ADDR_LEN);
	for (;;) {
		e[i++] = (unsigned char) strtoul(c, &c, 16);
		if (!*c++ || i == ETHER_ADDR_LEN)
			break;
	}
	return (i == ETHER_ADDR_LEN);
}

/*
 * Convert Ethernet address binary data to string representation
 * @param	e	binary data
 * @param	a	string in xx:xx:xx:xx:xx:xx notation
 * @return	a
 */
char *
ether_etoa(const unsigned char *e, char *a)
{
	char *c = a;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", e[i] & 0xff);
	}
	return a;
}

/*
 * Get the ip configuration index if it exists given the 
 * eth name.
 * 
 * @param	wl_ifname 	pointer to eth interface name
 * @return	index or -1 if not found
 */ 
int 
get_ipconfig_index(char *eth_ifname)
{
	char varname[64];
	char varval[64];
	char *ptr;
	char wl_ifname[64];
	int index;
	
	/* Bail if we get a NULL or empty string */
	
	if (!eth_ifname) return -1;
	if (!*eth_ifname) return -1;
	
	/* Look up wl name from the eth name */
	if( osifname_to_nvifname( eth_ifname, wl_ifname, sizeof(wl_ifname)) != 0 )
		return -1;
		
	snprintf(varname,sizeof(varname),"%s_ipconfig_index",wl_ifname);

	ptr = nvram_get(varname);
	
	if (ptr){
	/* Check ipconfig_index pointer to see if it is still pointing 
	   the correct lan config block */
		if (*ptr) {
			int index;
			char *ifname;
			char buf[64];
			index = atoi(ptr);
			
			snprintf(buf,sizeof(buf),"lan%d_ifname",index);
			
			ifname = nvram_get(buf);
			
			if (ifname) {
				if  (!(strcmp(ifname,wl_ifname)))
					return index;
			}
			nvram_unset(varname);
		}
	}
	
	/* The index pointer may not have been configured if the
	 * user enters the variables manually. Do a brute force search 
	 *  of the lanXX_ifname variables
	 */
	for (index=0 ; index < MAX_NVPARSE; index++){
		snprintf(varname,sizeof(varname),"lan%d_ifname",index);
		if ( nvram_match(varname,wl_ifname)){
			/* if a match is found set up a corresponding index pointer for wlXX */
			snprintf(varname,sizeof(varname),"%s_ipconfig_index",wl_ifname);
			snprintf(varval,sizeof(varval),"%d",index);
			nvram_set(varname,varval);
			nvram_commit();
			return index;
		};
	}
	return -1;
}
	
/*
 * Set the ip configuration index given the eth name
 * Updates both wlXX_ipconfig_index and lanYY_ifname.
 * 
 * @param	eth_ifname 	pointer to eth interface name
 * @return	0 if successful -1 if not.
 */
int 
set_ipconfig_index(char *eth_ifname,int index)
{
	char varname[255];
	char varval[16];
	char wl_ifname[64];
	
	/* Bail if we get a NULL or empty string */
	
	if (!eth_ifname) return -1;
	if (!*eth_ifname) return -1;
	
	if (index >= MAX_NVPARSE) return -1;
	
	/* Look up wl name from the eth name only if the name contains
	   eth
	*/

	if( osifname_to_nvifname( eth_ifname, wl_ifname, sizeof(wl_ifname)) != 0 )
		return -1;
	
	snprintf(varname,sizeof(varname),"%s_ipconfig_index",wl_ifname);
	snprintf(varval,sizeof(varval),"%d",index);
	nvram_set(varname,varval);
	
	snprintf(varname,sizeof(varname),"lan%d_ifname",index);
	nvram_set(varname,wl_ifname);
	
	nvram_commit();
	
	return 0;
}
	
/*
 * Get interfaces belonging to a specific bridge.
 * 
 * @param	bridge_name 	pointer to bridge interface name
 * @return	list of interfaces belonging to the bridge or NULL
 *              if not found/empty
 */	
char *
get_bridged_interfaces(char *bridge_name)
{
	static char interfaces[255] ;	
	char *ifnames=NULL;
	char bridge[64];
	
	if (!bridge_name) return NULL;
	
	memset(interfaces,0,sizeof(interfaces));
	snprintf(bridge,sizeof(bridge),"%s_ifnames",bridge_name);
	
	ifnames=nvram_get(bridge);
	
	if (ifnames) 
		strncpy(interfaces,ifnames,sizeof(interfaces));
	else
		return NULL;
		
	return  interfaces;
		
}

/*
 * Search a string backwards for a set of characters
 * This is the reverse version of strspn()
 *
 * @param	s	string to search backwards
 * @param	accept	set of chars for which to search
 * @return	number of characters in the trailing segment of s 
 *		which consist only of characters from accept.
 */
static size_t
sh_strrspn(const char *s, const char *accept)
{
	const char *p;
	size_t accept_len = strlen(accept);
	int i;

	
	if (s[0] == '\0')
		return 0;
	
	p = s + (strlen(s) - 1);
	i = 0;
	
	do {
		if (memchr(accept, *p, accept_len) == NULL)
			break;
		p--; i++;
	} while (p != s);

	return i;
}

/*
 * Parse the unit and subunit from an interface string such as wlXX or wlXX.YY
 *
 * @param	ifname	interface string to parse
 * @param	unit	pointer to return the unit number, may pass NULL
 * @param	subunit	pointer to return the subunit number, may pass NULL
 * @return	Returns 0 if the string ends with digits or digits.digits, -1 otherwise.
 *		If ifname ends in digits.digits, then unit and subuint are set
 *		to the first and second values respectively. If ifname ends 
 *		in just digits, unit is set to the value, and subunit is set
 *		to -1. On error both unit and subunit are -1. NULL may be passed
 *		for unit and/or subuint to ignore the value.
 */
int
get_ifname_unit(const char* ifname, int *unit, int *subunit)
{
	const char digits[] = "0123456789";
	char str[64];
	char *p;
	size_t ifname_len = strlen(ifname);
	size_t len;
	long val;

	if (unit)
		*unit = -1;
	if (subunit)
		*subunit = -1;
	
	if (ifname_len + 1 > sizeof(str)) 
		return -1;

	strcpy(str, ifname);

	/* find the trailing digit chars */
	len = sh_strrspn(str, digits);
	
	/* fail if there were no trailing digits */
	if (len == 0)
		return -1;

	/* point to the beginning of the last integer and convert */
	p = str + (ifname_len - len);
	val = strtol(p, NULL, 10);
	
	/* if we are at the beginning of the string, or the previous
	 * character is not a '.', then we have the unit number and
	 * we are done parsing
	 */
	if (p == str || p[-1] != '.') {
		if (unit)
			*unit = val;
		return 0;
	} else {
		if (subunit)
			*subunit = val;
	}

	/* chop off the '.NNN' and get the unit number */
	p--;
	p[0] = '\0';
	
	/* find the trailing digit chars */
	len = sh_strrspn(str, digits);

	/* fail if there were no trailing digits */
	if (len == 0)
		return -1;

	/* point to the beginning of the last integer and convert */
	p = p - len;
	val = strtol(p, NULL, 10);

	/* save the unit number */
	if (unit)
		*unit = val;

	return 0;
}

/** 
		remove_from_list
		Remove the specified word from the list.

		@param name word to be removed from the list
		@param list Space separated list to modify
		@param listsize Max size the list can occupy

		@return	error code
*/
int remove_from_list( char *name, char *list, int listsize )
{
	int listlen = 0;
	int namelen = 0;
	char *occurrence = list;
	
	if( !list || !name || (listsize <= 0) )
		return EINVAL;

	listlen = strlen( list );
	namelen = strlen( name );

	while( occurrence != NULL && ( occurrence - list < listlen ))
	{
		occurrence = strstr( occurrence, name );
		
		if( !occurrence )
			return EINVAL;
		
		/* last item in list? */
		if( occurrence[namelen] == 0 )
		{
			/* only item in list? */
			if( occurrence != list )
				occurrence--;
			occurrence[0] = 0;
			break;
		}
		else if( occurrence[namelen] == ' ' )
		{
			strncpy( occurrence, &occurrence[namelen+1/*space*/], 
							 strlen( &occurrence[namelen+1/*space*/]) +1/*terminate*/ );
			break;
		}
		occurrence++;
	}

	return 0;
}

/** 
		add_to_list
		Add the specified interface(string) to the list as long as
		it will fit in the space left in the list.

		NOTE: If item is already in list, it won't be added again.

		@param name Name of interface to be added to the list
		@param list List to modify
		@param listsize Max size the list can occupy

		@return	error code
*/
int add_to_list( char *name, char *list, int listsize )
{
	int listlen = 0;
	int namelen = 0;
	char *temp = NULL;

	if( !list || !name || (listsize <= 0) )
		return EINVAL;

	listlen = strlen( list );
	namelen = strlen( name );

	/* is the item already in the list? */
	temp = strstr( list, name );
	if( temp && ( temp[namelen] == ' ' || temp[namelen] == 0))
		return 0;


	if( listsize <= listlen + namelen + 1/*space*/ )
		return EMSGSIZE;

	/* add a space if the list isn't empty */
	if( list[0] != 0 )
	{
		list[listlen++] = 0x20;
	}
	
	strncpy( &list[listlen], name, namelen+1/*terminate*/ );

	return 0;
}


#define WLMBSS_DEV_NAME	"wlmbss"
#define WL_DEV_NAME "wl"
#define WDS_DEV_NAME	"wds"

#if defined(linux)
/**
	 nvifname_to_osifname() 
	 The intent here is to provide a conversion between the OS interface name
	 and the device name that we keep in NVRAM.  
	 This should eventually be placed in a Linux specific file with other 
	 OS abstraction functions.

	 @param nvifname pointer to ifname to be converted
	 @param osifname_buf storage for the converted osifname
	 @param osifname_buf_len length of storage for osifname_buf
*/
int 
nvifname_to_osifname( const char *nvifname, char *osifname_buf, 
											int osifname_buf_len )
{
	char varname[NVRAM_MAX_PARAM_LEN];
	char *ptr;
	
	memset( osifname_buf, 0, osifname_buf_len );

	/* Bail if we get a NULL or empty string */
	if((!nvifname) || (!*nvifname) || (!osifname_buf)){
		return -1;
	}
	
	if (strstr(nvifname,"eth") || strstr(nvifname,".")){
		strncpy( osifname_buf, nvifname, osifname_buf_len);
		return 0;
	}	
	
	snprintf( varname, sizeof(varname), "%s_ifname", nvifname);
	ptr = nvram_get(varname);
	if (ptr){
		/* Bail if the string is empty */
		if (!*ptr) return -1;
		strncpy( osifname_buf, ptr, osifname_buf_len);
		return 0;
	}

	return -1;
}


/* osifname_to_nvifname()

   Convert the OS interface name to the name we use internally(NVRAM,GUI,etc.)

	 This is the Linux version of this function 

	 @param osifname pointer to osifname to be converted
	 @param nvifname_buf storage for the converted ifname
	 @param nvifname_buf_len length of storage for nvifname_buf
*/

int
osifname_to_nvifname( const char *osifname, char *nvifname_buf, 
											int nvifname_buf_len )
{
	char varname[NVRAM_MAX_PARAM_LEN];
	int pri,sec;
	
	/* Bail if we get a NULL or empty string */
	
	if((!osifname) || (!*osifname) || (!nvifname_buf))
	{
		return -1;
	}
	
	memset(nvifname_buf, 0, nvifname_buf_len);
	
	if (strstr(osifname,"wl")){
		strncpy(nvifname_buf,osifname,nvifname_buf_len);
		return 0;
	}	
	
	/* look for interface name on the primary interfaces first */
	for (pri=0;pri < MAX_NVPARSE; pri++){
		snprintf(varname,sizeof(varname),
					"wl%d_ifname",pri);
		if (nvram_match(varname,osifname)) {
					snprintf(nvifname_buf,nvifname_buf_len,"wl%d",pri);
					return 0;
				}
	}
	
	/* look for interface name on the multi-instance interfaces */
	for (pri=0;pri < MAX_NVPARSE; pri++)
		for (sec=0;sec< MAX_NVPARSE; sec++){
			snprintf(varname,sizeof(varname),
					"wl%d.%d_ifname",pri,sec);
			if (nvram_match(varname,osifname)) {
					snprintf(nvifname_buf,nvifname_buf_len,"wl%d.%d",pri,sec);
					return 0;
				}
		}
	
	return -1;

}

#endif
