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


#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <paths.h>
#অন্তর্ভাৱ <pwd.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <sys/sysmacros.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/utsname.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <utmpx.h>

#অন্তর্ভাৱ "broadcast.h"
#অন্তর্ভাৱ "helpers.h"

#ifndef _PATH_DEV
# define _PATH_DEV	"/dev/"
#endif


স্থির শূন্য getuidtty(ৱর্ণ **userp, ৱর্ণ **ttyp)
{
	সংরচনা passwd 		*pwd;
	uid_t			uid;
	ৱর্ণ			*tty;
	স্থির ৱর্ণ		uidbuf[32];
	ৱর্ণ		*ttynm = রিক্ত;

	uid = getuid();
	যদি ((pwd = getpwuid(uid)) != রিক্ত) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, পরিমাণ(uidbuf) - 1);
	} অন্যথা {
		যদি (uid)
			sprintf(uidbuf, "uid %d", (পূর্ণাঙ্ক) uid);
		অন্যথা
			sprintf(uidbuf, "root");
	}

	যদি ((tty = ttyname(0)) != রিক্ত) {
		ধ্রুৱ size_t plen = strlen(_PATH_DEV);
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
স্থির পূর্ণাঙ্ক file_isatty(ধ্রুৱ ৱর্ণ *fname)
{
	সংরচনা stat		st;
	পূর্ণাঙ্ক			major;

	যদি (stat(fname, &st) < 0)
		ঘূৰাই 0;

	যদি (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		ঘূৰাই 0;

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
		ঘূৰাই 0;
	ঘূৰাই 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
শূন্য broadcast(ৱর্ণ *text)
{
	ৱর্ণ *tty;
	ৱর্ণ *user;
	সংরচনা utsname name;
	time_t t;
	ৱর্ণ *date;
	ৱর্ণ *p;
	ৱর্ণ *সাৰি = রিক্ত;
	পূর্ণাঙ্ক len;
	পূর্ণাঙ্ক fd;
	ৱর্ণ *term = রিক্ত;
	সংরচনা utmpx *utmp;

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

	len = xasprintf(&সাৰি, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	মুক্ত(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	যদি (fork() != 0)
		ঘূৰাই;

	setutxent();

	যেতিয়ালৈ ((utmp = getutxent()) != রিক্ত) {
		যদি (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			অনুৱর্ত;
		যদি (strncmp(utmp->ut_line, _PATH_DEV, strlen(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		অন্যথা
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		যদি (strstr(term, "/../")) {
			মুক্ত(term);
			অনুৱর্ত;
		}

		/* Open it non-delay */
		যদি (file_isatty(term) && (fd = open(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			যদি (isatty(fd))
				লিখ(fd, সাৰি, len);
			close(fd);
		}
		মুক্ত(term);
	}
	endutxent();
	মুক্ত(সাৰি);
	exit(0);
}
