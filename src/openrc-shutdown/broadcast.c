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

#اگر_غیر_تعریف_شدہ _PATH_DEV
# define _PATH_DEV	"/dev/"
#اگر_ختم


ساکن خالی getuidtty(حرف **userp, حرف **ttyp)
{
	ساخت passwd 		*pwd;
	uid_t			uid;
	حرف			*tty;
	ساکن حرف		uidbuf[32];
	حرف		*ttynm = تہی;

	uid = getuid();
	اگر ((pwd = getpwuid(uid)) != تہی) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, حجم(uidbuf) - 1);
	} ورنہ {
		اگر (uid)
			sprintf(uidbuf, "uid %d", (صحیح) uid);
		ورنہ
			sprintf(uidbuf, "root");
	}

	اگر ((tty = ttyname(0)) != تہی) {
		ثابت size_t plen = لڑی_لمبائی(_PATH_DEV);
		اگر (لڑی_موازنہ(tty, _PATH_DEV, plen) == 0) {
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
ساکن صحیح file_isatty(ثابت حرف *fname)
{
	ساخت stat		st;
	صحیح			major;

	اگر (stat(fname, &st) < 0)
		واپس 0;

	اگر (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		واپس 0;

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
		واپس 0;
	واپس 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
خالی نشر(حرف *متن)
{
	حرف *tty;
	حرف *user;
	ساخت utsname name;
	time_t t;
	حرف *date;
	حرف *p;
	حرف *سطر = تہی;
	صحیح len;
	صحیح fd;
	حرف *رکن = تہی;
	ساخت utmpx *utmp;

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
			user, name.nodename, tty, date, متن);
	آزاد(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	اگر (fork() != 0)
		واپس;

	setutxent();

	جب ((utmp = getutxent()) != تہی) {
		اگر (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			جاری;
		اگر (لڑی_موازنہ(utmp->ut_line, _PATH_DEV, لڑی_لمبائی(_PATH_DEV)) == 0)
			xasprintf(&رکن, "%s", utmp->ut_line);
		ورنہ
			xasprintf(&رکن, "%s%s", _PATH_DEV, utmp->ut_line);
		اگر (لڑی_تلاش(رکن, "/../")) {
			آزاد(رکن);
			جاری;
		}

		/* Open it non-delay */
		اگر (file_isatty(رکن) && (fd = open(رکن, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			اگر (isatty(fd))
				اكتب(fd, سطر, len);
			close(fd);
		}
		آزاد(رکن);
	}
	endutxent();
	آزاد(سطر);
	باہر_نکلو(0);
}
