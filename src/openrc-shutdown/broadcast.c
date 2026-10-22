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


#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <paths.h>
#অন্তর্ভাব <pwd.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/sysmacros.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/utsname.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <utmpx.h>

#অন্তর্ভাব "broadcast.h"
#অন্তর্ভাব "helpers.h"

#ifndef _PATH_DEV
# define _PATH_DEV	"/dev/"
#endif


স্থির শূন্য getuidtty(অক্ষর **userp, অক্ষর **ttyp)
{
	কাঠামো passwd 		*pwd;
	uid_t			uid;
	অক্ষর			*tty;
	স্থির অক্ষর		uidbuf[32];
	অক্ষর		*ttynm = ফাঁকা;

	uid = getuid();
	যদি ((pwd = getpwuid(uid)) != ফাঁকা) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, আকার(uidbuf) - 1);
	} অন্যথা {
		যদি (uid)
			sprintf(uidbuf, "uid %d", (পূর্ণসংখ্যা) uid);
		অন্যথা
			sprintf(uidbuf, "root");
	}

	যদি ((tty = ttyname(0)) != ফাঁকা) {
		ধ্রুবক size_t plen = strlen(_PATH_DEV);
		যদি (strncmp(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			যদি (tty[0] == '/')
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
স্থির পূর্ণসংখ্যা file_isatty(ধ্রুবক অক্ষর *fname)
{
	কাঠামো stat		st;
	পূর্ণসংখ্যা			major;

	যদি (stat(fname, &st) < 0)
		ফেরত 0;

	যদি (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		ফেরত 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	যদি (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		ফেরত 0;
	ফেরত 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
শূন্য broadcast(অক্ষর *text)
{
	অক্ষর *tty;
	অক্ষর *user;
	কাঠামো utsname name;
	time_t t;
	অক্ষর *date;
	অক্ষর *p;
	অক্ষর *সারি = ফাঁকা;
	পূর্ণসংখ্যা len;
	পূর্ণসংখ্যা fd;
	অক্ষর *term = ফাঁকা;
	কাঠামো utmpx *utmp;

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
	যদি (p)
		*p = 0;

	len = xasprintf(&সারি, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	মুক্ত(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	যদি (fork() != 0)
		ফেরত;

	setutxent();

	যখন ((utmp = getutxent()) != ফাঁকা) {
		যদি (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			চালু;
		যদি (strncmp(utmp->ut_line, _PATH_DEV, strlen(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		অন্যথা
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		যদি (strstr(term, "/../")) {
			মুক্ত(term);
			চালু;
		}

		/* Open it non-delay */
		যদি (file_isatty(term) && (fd = open(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			যদি (isatty(fd))
				লিখ(fd, সারি, len);
			close(fd);
		}
		মুক্ত(term);
	}
	endutxent();
	মুক্ত(সারি);
	exit(0);
}
