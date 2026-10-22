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


#شامل <fcntl.h>
#شامل <paths.h>
#شامل <pwd.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/stat.h>
#شامل <sys/sysmacros.h>
#شامل <sys/types.h>
#شامل <sys/utsname.h>
#شامل <time.h>
#شامل <unistd.h>
#شامل <utmpx.h>

#شامل "broadcast.h"
#شامل "helpers.h"

#ifndef _PATH_DEV
# define _PATH_DEV	"/dev/"
#endif


ایستا تهی getuidtty(نویسه **userp, نویسه **ttyp)
{
	ساختار passwd 		*pwd;
	uid_t			uid;
	نویسه			*tty;
	ایستا نویسه		uidbuf[32];
	نویسه		*ttynm = پوچ;

	uid = getuid();
	اگر ((pwd = getpwuid(uid)) != پوچ) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, اندازه(uidbuf) - 1);
	} وگرنه {
		اگر (uid)
			sprintf(uidbuf, "uid %d", (صحیح) uid);
		وگرنه
			sprintf(uidbuf, "root");
	}

	اگر ((tty = ttyname(0)) != پوچ) {
		ثابت size_t plen = strlen(_PATH_DEV);
		اگر (strncmp(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			اگر (tty[0] == '/')
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
ایستا صحیح file_isatty(ثابت نویسه *fname)
{
	ساختار stat		st;
	صحیح			major;

	اگر (stat(fname, &st) < 0)
		بازگشت 0;

	اگر (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		بازگشت 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	اگر (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		بازگشت 0;
	بازگشت 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
تهی broadcast(نویسه *text)
{
	نویسه *tty;
	نویسه *user;
	ساختار utsname name;
	time_t t;
	نویسه *date;
	نویسه *p;
	نویسه *سطر = پوچ;
	صحیح len;
	صحیح fd;
	نویسه *term = پوچ;
	ساختار utmpx *utmp;

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
	اگر (p)
		*p = 0;

	len = xasprintf(&سطر, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	آزاد(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	اگر (fork() != 0)
		بازگشت;

	setutxent();

	وقتی ((utmp = getutxent()) != پوچ) {
		اگر (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			ادامه;
		اگر (strncmp(utmp->ut_line, _PATH_DEV, strlen(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		وگرنه
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		اگر (strstr(term, "/../")) {
			آزاد(term);
			ادامه;
		}

		/* Open it non-delay */
		اگر (file_isatty(term) && (fd = open(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			اگر (isatty(fd))
				اكتب(fd, سطر, len);
			close(fd);
		}
		آزاد(term);
	}
	endutxent();
	آزاد(سطر);
	exit(0);
}
