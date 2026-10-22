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
	ако ((pwd = getpwuid(uid)) != NULL) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, sizeof(uidbuf) - 1);
	} иначе {
		ако (uid)
			sprintf(uidbuf, "uid %d", (цяло) uid);
		иначе
			sprintf(uidbuf, "root");
	}

	ако ((tty = ttyname(0)) != NULL) {
		const size_t plen = strlen(_PATH_DEV);
		ако (strncmp(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			ако (tty[0] == '/')
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
static цяло file_isatty(const char *fname)
{
	struct stat		st;
	цяло			major;

	ако (stat(fname, &st) < 0)
		върни 0;

	ако (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		върни 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	ако (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		върни 0;
	върни 1;
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
	цяло len;
	цяло fd;
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
	ако (p)
		*p = 0;

	len = xasprintf(&line, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	free(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	ако (fork() != 0)
		върни;

	setutxent();

	докато ((utmp = getutxent()) != NULL) {
		ако (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			continue;
		ако (strncmp(utmp->ut_line, _PATH_DEV, strlen(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		иначе
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		ако (strstr(term, "/../")) {
			free(term);
			continue;
		}

		/* Open it non-delay */
		ако (file_isatty(term) && (fd = open(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			ако (isatty(fd))
				write(fd, line, len);
			close(fd);
		}
		free(term);
	}
	endutxent();
	free(line);
	exit(0);
}
