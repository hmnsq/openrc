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


#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <paths.h>
#ଅନ୍ତର୍ଭାଵ <pwd.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/sysmacros.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/utsname.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <utmpx.h>

#ଅନ୍ତର୍ଭାଵ "broadcast.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

#ifndef _PATH_DEV
# define _PATH_DEV	"/dev/"
#endif


ସ୍ଥିର ଶୂନ୍ୟ getuidtty(ଅକ୍ଷର **userp, ଅକ୍ଷର **ttyp)
{
	ସଂରଚନା passwd 		*pwd;
	uid_t			uid;
	ଅକ୍ଷର			*tty;
	ସ୍ଥିର ଅକ୍ଷର		uidbuf[32];
	ଅକ୍ଷର		*ttynm = ଖାଲି;

	uid = getuid();
	ଯଦି ((pwd = getpwuid(uid)) != ଖାଲି) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, ଆକାର(uidbuf) - 1);
	} ଅନ୍ୟଥା {
		ଯଦି (uid)
			sprintf(uidbuf, "uid %d", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) uid);
		ଅନ୍ୟଥା
			sprintf(uidbuf, "root");
	}

	ଯଦି ((tty = ttyname(0)) != ଖାଲି) {
		ଅଚଳ size_t plen = strlen(_PATH_DEV);
		ଯଦି (strncmp(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			ଯଦି (tty[0] == '/')
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା file_isatty(ଅଚଳ ଅକ୍ଷର *fname)
{
	ସଂରଚନା stat		st;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା			major;

	ଯଦି (stat(fname, &st) < 0)
		ଫେରନ୍ତୁ 0;

	ଯଦି (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		ଫେରନ୍ତୁ 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	ଯଦି (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		ଫେରନ୍ତୁ 0;
	ଫେରନ୍ତୁ 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
ଶୂନ୍ୟ broadcast(ଅକ୍ଷର *text)
{
	ଅକ୍ଷର *tty;
	ଅକ୍ଷର *user;
	ସଂରଚନା utsname name;
	time_t t;
	ଅକ୍ଷର *date;
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *ଧାଡ଼ି = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
	ଅକ୍ଷର *term = ଖାଲି;
	ସଂରଚନା utmpx *utmp;

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
	ଯଦି (p)
		*p = 0;

	len = xasprintf(&ଧାଡ଼ି, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	ମୁକ୍ତ(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	ଯଦି (fork() != 0)
		ଫେରନ୍ତୁ;

	setutxent();

	ଯେତେବେଳେ ((utmp = getutxent()) != ଖାଲି) {
		ଯଦି (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			ଜାରି;
		ଯଦି (strncmp(utmp->ut_line, _PATH_DEV, strlen(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		ଅନ୍ୟଥା
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		ଯଦି (strstr(term, "/../")) {
			ମୁକ୍ତ(term);
			ଜାରି;
		}

		/* Open it non-delay */
		ଯଦି (file_isatty(term) && (fd = open(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			ଯଦି (isatty(fd))
				ଲିଖ(fd, ଧାଡ଼ି, len);
			close(fd);
		}
		ମୁକ୍ତ(term);
	}
	endutxent();
	ମୁକ୍ତ(ଧାଡ଼ି);
	exit(0);
}
