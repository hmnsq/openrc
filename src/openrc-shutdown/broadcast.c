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


#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <paths.h>
#അന്തര്ഭാവ <pwd.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/sysmacros.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/utsname.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <utmpx.h>

#അന്തര്ഭാവ "broadcast.h"
#അന്തര്ഭാവ "helpers.h"

#ifndef _PATH_DEV
# define _PATH_DEV	"/dev/"
#endif


സ്ഥിതം ശൂന്യം getuidtty(അക്ഷരം **userp, അക്ഷരം **ttyp)
{
	ഘടന passwd 		*pwd;
	uid_t			uid;
	അക്ഷരം			*tty;
	സ്ഥിതം അക്ഷരം		uidbuf[32];
	അക്ഷരം		*ttynm = ശൂന്യ;

	uid = getuid();
	എങ്കിൽ ((pwd = getpwuid(uid)) != ശൂന്യ) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, വലുപ്പം(uidbuf) - 1);
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (uid)
			sprintf(uidbuf, "uid %d", (പൂർണ്ണസംഖ്യ) uid);
		അല്ലെങ്കിൽ
			sprintf(uidbuf, "root");
	}

	എങ്കിൽ ((tty = ttyname(0)) != ശൂന്യ) {
		സ്ഥിരം size_t plen = strlen(_PATH_DEV);
		എങ്കിൽ (strncmp(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			എങ്കിൽ (tty[0] == '/')
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
സ്ഥിതം പൂർണ്ണസംഖ്യ file_isatty(സ്ഥിരം അക്ഷരം *fname)
{
	ഘടന stat		st;
	പൂർണ്ണസംഖ്യ			major;

	എങ്കിൽ (stat(fname, &st) < 0)
		തിരികെ 0;

	എങ്കിൽ (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		തിരികെ 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	എങ്കിൽ (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		തിരികെ 0;
	തിരികെ 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
ശൂന്യം broadcast(അക്ഷരം *text)
{
	അക്ഷരം *tty;
	അക്ഷരം *user;
	ഘടന utsname name;
	time_t t;
	അക്ഷരം *date;
	അക്ഷരം *p;
	അക്ഷരം *വരി = ശൂന്യ;
	പൂർണ്ണസംഖ്യ len;
	പൂർണ്ണസംഖ്യ fd;
	അക്ഷരം *term = ശൂന്യ;
	ഘടന utmpx *utmp;

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
	എങ്കിൽ (p)
		*p = 0;

	len = xasprintf(&വരി, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	സ്വതന്ത്രം(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	എങ്കിൽ (fork() != 0)
		തിരികെ;

	setutxent();

	ആകുമ്പോൾ ((utmp = getutxent()) != ശൂന്യ) {
		എങ്കിൽ (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			തുടരുക;
		എങ്കിൽ (strncmp(utmp->ut_line, _PATH_DEV, strlen(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		അല്ലെങ്കിൽ
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		എങ്കിൽ (strstr(term, "/../")) {
			സ്വതന്ത്രം(term);
			തുടരുക;
		}

		/* Open it non-delay */
		എങ്കിൽ (file_isatty(term) && (fd = open(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			എങ്കിൽ (isatty(fd))
				ലിഖ(fd, വരി, len);
			close(fd);
		}
		സ്വതന്ത്രം(term);
	}
	endutxent();
	സ്വതന്ത്രം(വരി);
	exit(0);
}
