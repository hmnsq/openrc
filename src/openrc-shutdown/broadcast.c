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


#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <paths.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pwd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/sysmacros.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/utsname.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <utmpx.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "broadcast.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _PATH_DEV
# define _PATH_DEV	"/dev/"
#ਜੇ_ਅੰਤ


ਸਥਿਰ ਖਾਲੀ getuidtty(ਅੱਖਰ **userp, ਅੱਖਰ **ttyp)
{
	ਬਣਤਰ passwd 		*pwd;
	uid_t			uid;
	ਅੱਖਰ			*tty;
	ਸਥਿਰ ਅੱਖਰ		uidbuf[32];
	ਅੱਖਰ		*ttynm = ਨਿਰਾ;

	uid = getuid();
	ਜੇ ((pwd = getpwuid(uid)) != ਨਿਰਾ) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, ਆਕਾਰ(uidbuf) - 1);
	} ਨਹੀਂਤਾਂ {
		ਜੇ (uid)
			sprintf(uidbuf, "uid %d", (ਪੂਰਨ) uid);
		ਨਹੀਂਤਾਂ
			sprintf(uidbuf, "root");
	}

	ਜੇ ((tty = ttyname(0)) != ਨਿਰਾ) {
		ਅਚਲ size_t plen = ਲੜੀ_ਲੰਬਾਈ(_PATH_DEV);
		ਜੇ (ਲੜੀ_ਤੁਲਨਾ(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			ਜੇ (tty[0] == '/')
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
ਸਥਿਰ ਪੂਰਨ file_isatty(ਅਚਲ ਅੱਖਰ *fname)
{
	ਬਣਤਰ stat		st;
	ਪੂਰਨ			major;

	ਜੇ (stat(fname, &st) < 0)
		ਵਾਪਸ 0;

	ਜੇ (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		ਵਾਪਸ 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	ਜੇ (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		ਵਾਪਸ 0;
	ਵਾਪਸ 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
ਖਾਲੀ ਪ੍ਰਸਾਰਣ(ਅੱਖਰ *ਪਾਠ)
{
	ਅੱਖਰ *tty;
	ਅੱਖਰ *user;
	ਬਣਤਰ utsname name;
	time_t t;
	ਅੱਖਰ *date;
	ਅੱਖਰ *p;
	ਅੱਖਰ *ਸਤਰ = ਨਿਰਾ;
	ਪੂਰਨ len;
	ਪੂਰਨ fd;
	ਅੱਖਰ *ਪਦ = ਨਿਰਾ;
	ਬਣਤਰ utmpx *utmp;

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
	ਜੇ (p)
		*p = 0;

	len = xasprintf(&ਸਤਰ, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, ਪਾਠ);
	ਮੁਕਤ(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	ਜੇ (fork() != 0)
		ਵਾਪਸ;

	setutxent();

	ਜਦੋਂ ((utmp = getutxent()) != ਨਿਰਾ) {
		ਜੇ (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			ਜਾਰੀ;
		ਜੇ (ਲੜੀ_ਤੁਲਨਾ(utmp->ut_line, _PATH_DEV, ਲੜੀ_ਲੰਬਾਈ(_PATH_DEV)) == 0)
			xasprintf(&ਪਦ, "%s", utmp->ut_line);
		ਨਹੀਂਤਾਂ
			xasprintf(&ਪਦ, "%s%s", _PATH_DEV, utmp->ut_line);
		ਜੇ (ਲੜੀ_ਖੋਜ(ਪਦ, "/../")) {
			ਮੁਕਤ(ਪਦ);
			ਜਾਰੀ;
		}

		/* Open it non-delay */
		ਜੇ (file_isatty(ਪਦ) && (fd = open(ਪਦ, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			ਜੇ (isatty(fd))
				ਲਿਖ(fd, ਸਤਰ, len);
			close(fd);
		}
		ਮੁਕਤ(ਪਦ);
	}
	endutxent();
	ਮੁਕਤ(ਸਤਰ);
	ਬਾਹਰ_ਨਿਕਲੋ(0);
}
