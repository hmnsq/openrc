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


static void getuidtty(char **userp, char **ttyp)
{
	struct passwd 		*pwd;
	uid_t			uid;
	char			*tty;
	static char		uidbuf[32];
	char		*ttynm = NULL;

	uid = getuid();
	만약 ((pwd = getpwuid(uid)) != NULL) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, sizeof(uidbuf) - 1);
	} 아니면 {
		만약 (uid)
			sprintf(uidbuf, "uid %d", (정수) uid);
		아니면
			sprintf(uidbuf, "root");
	}

	만약 ((tty = ttyname(0)) != NULL) {
		const size_t plen = strlen(_PATH_DEV);
		만약 (strncmp(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			만약 (tty[0] == '/')
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
static 정수 file_isatty(const char *fname)
{
	struct stat		st;
	정수			major;

	만약 (stat(fname, &st) < 0)
		반환 0;

	만약 (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		반환 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	만약 (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		반환 0;
	반환 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
void broadcast(char *text)
{
	char *tty;
	char *user;
	struct utsname name;
	time_t t;
	char *date;
	char *p;
	char *line = NULL;
	정수 len;
	정수 fd;
	char *term = NULL;
	struct utmpx *utmp;

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
	만약 (p)
		*p = 0;

	len = xasprintf(&line, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	free(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	만약 (fork() != 0)
		반환;

	setutxent();

	동안 ((utmp = getutxent()) != NULL) {
		만약 (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			continue;
		만약 (strncmp(utmp->ut_line, _PATH_DEV, strlen(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		아니면
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		만약 (strstr(term, "/../")) {
			free(term);
			continue;
		}

		/* Open it non-delay */
		만약 (file_isatty(term) && (fd = open(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			만약 (isatty(fd))
				write(fd, line, len);
			close(fd);
		}
		free(term);
	}
	endutxent();
	free(line);
	exit(0);
}
