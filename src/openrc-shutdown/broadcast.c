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


#ضمن <fcntl.h>
#ضمن <paths.h>
#ضمن <pwd.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/stat.h>
#ضمن <sys/sysmacros.h>
#ضمن <sys/types.h>
#ضمن <sys/utsname.h>
#ضمن <time.h>
#ضمن <unistd.h>
#ضمن <utmpx.h>

#ضمن "broadcast.h"
#ضمن "helpers.h"

#جے_بنا_تعریف _PATH_DEV
# define _PATH_DEV	"/dev/"
#جے_انت


ساكن فراغ getuidtty(حرف **userp, حرف **ttyp)
{
	بنية passwd 		*pwd;
	uid_t			uid;
	حرف			*tty;
	ساكن حرف		uidbuf[32];
	حرف		*ttynm = عدم;

	uid = getuid();
	إذا ((pwd = getpwuid(uid)) != عدم) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, حجم(uidbuf) - 1);
	} وإلا {
		إذا (uid)
			sprintf(uidbuf, "uid %d", (صحيح) uid);
		وإلا
			sprintf(uidbuf, "root");
	}

	إذا ((tty = ttyname(0)) != عدم) {
		ثابت size_t plen = لڑی_لمبائی(_PATH_DEV);
		إذا (لڑی_مقابلہ(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			إذا (tty[0] == '/')
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
ساكن صحيح file_isatty(ثابت حرف *fname)
{
	بنية stat		st;
	صحيح			major;

	إذا (stat(fname, &st) < 0)
		أرجع 0;

	إذا (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		أرجع 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	إذا (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		أرجع 0;
	أرجع 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
فراغ نشر(حرف *متن)
{
	حرف *tty;
	حرف *user;
	بنية utsname name;
	time_t t;
	حرف *date;
	حرف *p;
	حرف *سطر = عدم;
	صحيح len;
	صحيح fd;
	حرف *رکن = عدم;
	بنية utmpx *utmp;

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
	إذا (p)
		*p = 0;

	len = xasprintf(&سطر, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, متن);
	حرر(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	إذا (fork() != 0)
		أرجع;

	setutxent();

	طالما ((utmp = getutxent()) != عدم) {
		إذا (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			استمر;
		إذا (لڑی_مقابلہ(utmp->ut_line, _PATH_DEV, لڑی_لمبائی(_PATH_DEV)) == 0)
			xasprintf(&رکن, "%s", utmp->ut_line);
		وإلا
			xasprintf(&رکن, "%s%s", _PATH_DEV, utmp->ut_line);
		إذا (لڑی_کھوج(رکن, "/../")) {
			حرر(رکن);
			استمر;
		}

		/* Open it non-delay */
		إذا (file_isatty(رکن) && (fd = open(رکن, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			إذا (isatty(fd))
				اكتب(fd, سطر, len);
			close(fd);
		}
		حرر(رکن);
	}
	endutxent();
	حرر(سطر);
	باہر_نکلو(0);
}
