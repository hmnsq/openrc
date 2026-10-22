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


#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <paths.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/sysmacros.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/utsname.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <utmpx.h>

#ಅನ್ತರ್ಭಾವ "broadcast.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

#ifndef _PATH_DEV
# define _PATH_DEV	"/dev/"
#endif


ಸ್ಥಿರ ಶೂನ್ಯ getuidtty(ವರ್ಣ **userp, ವರ್ಣ **ttyp)
{
	ಸಂರಚನಾ passwd 		*pwd;
	uid_t			uid;
	ವರ್ಣ			*tty;
	ಸ್ಥಿರ ವರ್ಣ		uidbuf[32];
	ವರ್ಣ		*ttynm = ರಿಕ್ತ;

	uid = getuid();
	ಯದಿ ((pwd = getpwuid(uid)) != ರಿಕ್ತ) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, ಪರಿಮಾಣ(uidbuf) - 1);
	} ಅನ್ಯಥಾ {
		ಯದಿ (uid)
			sprintf(uidbuf, "uid %d", (ಪೂರ್ಣಾಙ್ಕ) uid);
		ಅನ್ಯಥಾ
			sprintf(uidbuf, "root");
	}

	ಯದಿ ((tty = ttyname(0)) != ರಿಕ್ತ) {
		ಧ್ರುವ size_t plen = strlen(_PATH_DEV);
		ಯದಿ (strncmp(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			ಯದಿ (tty[0] == '/')
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ file_isatty(ಧ್ರುವ ವರ್ಣ *fname)
{
	ಸಂರಚನಾ stat		st;
	ಪೂರ್ಣಾಙ್ಕ			major;

	ಯದಿ (stat(fname, &st) < 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		ಪ್ರತ್ಯಾಗಮ 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	ಯದಿ (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಪ್ರತ್ಯಾಗಮ 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
ಶೂನ್ಯ broadcast(ವರ್ಣ *text)
{
	ವರ್ಣ *tty;
	ವರ್ಣ *user;
	ಸಂರಚನಾ utsname name;
	time_t t;
	ವರ್ಣ *date;
	ವರ್ಣ *p;
	ವರ್ಣ *ಸಾಲು = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ len;
	ಪೂರ್ಣಾಙ್ಕ fd;
	ವರ್ಣ *term = ರಿಕ್ತ;
	ಸಂರಚನಾ utmpx *utmp;

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
	ಯದಿ (p)
		*p = 0;

	len = xasprintf(&ಸಾಲು, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	ಮುಕ್ತ(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	ಯದಿ (fork() != 0)
		ಪ್ರತ್ಯಾಗಮ;

	setutxent();

	ಯಾವತ್ ((utmp = getutxent()) != ರಿಕ್ತ) {
		ಯದಿ (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			ಅನುವರ್ತ;
		ಯದಿ (strncmp(utmp->ut_line, _PATH_DEV, strlen(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		ಅನ್ಯಥಾ
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		ಯದಿ (strstr(term, "/../")) {
			ಮುಕ್ತ(term);
			ಅನುವರ್ತ;
		}

		/* Open it non-delay */
		ಯದಿ (file_isatty(term) && (fd = open(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			ಯದಿ (isatty(fd))
				ಲಿಖ(fd, ಸಾಲು, len);
			close(fd);
		}
		ಮುಕ್ತ(term);
	}
	endutxent();
	ಮುಕ್ತ(ಸಾಲು);
	exit(0);
}
