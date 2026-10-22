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


#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <paths.h>
#అన్తర్భావ <pwd.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/sysmacros.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/utsname.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <utmpx.h>

#అన్తర్భావ "broadcast.h"
#అన్తర్భావ "helpers.h"

#నిర్వచితం_కాకపోతే _PATH_DEV
# define _PATH_DEV	"/dev/"
#అయితే_ముగింపు


స్థిర శూన్య getuidtty(అక్షరం **userp, అక్షరం **ttyp)
{
	నిర్మాణం passwd 		*pwd;
	uid_t			uid;
	అక్షరం			*tty;
	స్థిర అక్షరం		uidbuf[32];
	అక్షరం		*ttynm = ఖాళీ;

	uid = getuid();
	ఒకవేళ ((pwd = getpwuid(uid)) != ఖాళీ) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, పరిమాణం(uidbuf) - 1);
	} లేకపోతే {
		ఒకవేళ (uid)
			sprintf(uidbuf, "uid %d", (పూర్ణాంకం) uid);
		లేకపోతే
			sprintf(uidbuf, "root");
	}

	ఒకవేళ ((tty = ttyname(0)) != ఖాళీ) {
		స్థిరరాశి size_t plen = అక్షరశ్రేణి_పొడవు(_PATH_DEV);
		ఒకవేళ (అక్షరశ్రేణి_పోలిక(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			ఒకవేళ (tty[0] == '/')
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
స్థిర పూర్ణాంకం file_isatty(స్థిరరాశి అక్షరం *fname)
{
	నిర్మాణం stat		st;
	పూర్ణాంకం			major;

	ఒకవేళ (stat(fname, &st) < 0)
		తిరిగి 0;

	ఒకవేళ (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		తిరిగి 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	ఒకవేళ (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		తిరిగి 0;
	తిరిగి 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
శూన్య ప్రసారం(అక్షరం *పాఠ్యం)
{
	అక్షరం *tty;
	అక్షరం *user;
	నిర్మాణం utsname name;
	time_t t;
	అక్షరం *date;
	అక్షరం *p;
	అక్షరం *పంక్తి = ఖాళీ;
	పూర్ణాంకం len;
	పూర్ణాంకం fd;
	అక్షరం *పదం = ఖాళీ;
	నిర్మాణం utmpx *utmp;

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
	ఒకవేళ (p)
		*p = 0;

	len = xasprintf(&పంక్తి, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, పాఠ్యం);
	విడుదల(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	ఒకవేళ (fork() != 0)
		తిరిగి;

	setutxent();

	అయితే ((utmp = getutxent()) != ఖాళీ) {
		ఒకవేళ (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			కొనసాగు;
		ఒకవేళ (అక్షరశ్రేణి_పోలిక(utmp->ut_line, _PATH_DEV, అక్షరశ్రేణి_పొడవు(_PATH_DEV)) == 0)
			xasprintf(&పదం, "%s", utmp->ut_line);
		లేకపోతే
			xasprintf(&పదం, "%s%s", _PATH_DEV, utmp->ut_line);
		ఒకవేళ (అక్షరశ్రేణి_శోధన(పదం, "/../")) {
			విడుదల(పదం);
			కొనసాగు;
		}

		/* Open it non-delay */
		ఒకవేళ (file_isatty(పదం) && (fd = open(పదం, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			ఒకవేళ (isatty(fd))
				లిఖ(fd, పంక్తి, len);
			close(fd);
		}
		విడుదల(పదం);
	}
	endutxent();
	విడుదల(పంక్తి);
	నిష్క్రమించు(0);
}
