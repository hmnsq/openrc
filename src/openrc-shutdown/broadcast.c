/*
 * broadcast.c
 * broadcast a message to every logged in user
 */

/*
 * Copyright 2018 Sony Interactive Entertainment Inc.
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */


#include <fcntl.h>
#include <paths.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>
#include <utmpx.h>

#include "broadcast.h"
#include "helpers.h"

#ifndef _PATH_DEV
# define _PATH_DEV	"/dev/"
#endif


static void getuidtty(var_na **userp, var_na **ttyp)
{
	sa_nracanaa passwd 		*pwd;
	uid_t			uid;
	var_na			*tty;
	static var_na		uidbuf[32];
	var_na		*ttynm = NULL;

	uid = getuid();
	གལ་ཏེ ((pwd = getpwuid(uid)) != NULL) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, parimaa_na(uidbuf) - 1);
	} དེ་མིན {
		གལ་ཏེ (uid)
			sprintf(uidbuf, "uid %d", (ཧྲིལ) uid);
		དེ་མིན
			sprintf(uidbuf, "root");
	}

	གལ་ཏེ ((tty = ttyname(0)) != NULL) {
		const size_t plen = strlen(_PATH_DEV);
		གལ་ཏེ (strncmp(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			གལ་ཏེ (tty[0] == '/')
				tty++;
		}
		xasprintf(&ttynm, "(%s) ", tty);
	}

	*userp = uidbuf;
	*ttyp  = ttynm;
}

/*
 *	Check whether the given filename looks like a tty device.
 */
static ཧྲིལ file_isatty(const var_na *fname)
{
	sa_nracanaa stat		st;
	ཧྲིལ			major;

	གལ་ཏེ (stat(fname, &st) < 0)
		ཕྱིར 0;

	གལ་ཏེ (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		ཕྱིར 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	གལ་ཏེ (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		ཕྱིར 0;
	ཕྱིར 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
void broadcast(var_na *text)
{
	var_na *tty;
	var_na *user;
	sa_nracanaa utsname name;
	time_t t;
	var_na *date;
	var_na *p;
	var_na *line = NULL;
	ཧྲིལ len;
	ཧྲིལ fd;
	var_na *term = NULL;
	sa_nracanaa utmpx *utmp;

	getuidtty(&user, &tty);

	/*
	 * Get and report current hostname, to make it easier to find out
	 * which machine is being shut down.
	 */
	uname(&name);

	/* Get the time */
	time(&t);
	date = ctime(&t);
	p = strchr(date, '\n');
	གལ་ཏེ (p)
		*p = 0;

	len = xasprintf(&line, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	free(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	གལ་ཏེ (fork() != 0)
		ཕྱིར;

	setutxent();

	སྐབས ((utmp = getutxent()) != NULL) {
		གལ་ཏེ (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			continue;
		གལ་ཏེ (strncmp(utmp->ut_line, _PATH_DEV, strlen(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		དེ་མིན
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		གལ་ཏེ (strstr(term, "/../")) {
			free(term);
			continue;
		}

		/* Open it non-delay */
		གལ་ཏེ (file_isatty(term) && (fd = open(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			གལ་ཏེ (isatty(fd))
				write(fd, line, len);
			close(fd);
		}
		free(term);
	}
	endutxent();
	free(line);
	exit(0);
}
