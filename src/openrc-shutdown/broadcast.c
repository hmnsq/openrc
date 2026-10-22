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


#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <paths.h>
#अन्तर्भाव <pwd.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/sysmacros.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/utsname.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <utmpx.h>

#अन्तर्भाव "broadcast.h"
#अन्तर्भाव "helpers.h"

#ifndef _PATH_DEV
# define _PATH_DEV	"/dev/"
#endif


स्थिर शून्य getuidtty(वर्ण **userp, वर्ण **ttyp)
{
	संरचना passwd 		*pwd;
	uid_t			uid;
	वर्ण			*tty;
	स्थिर वर्ण		uidbuf[32];
	वर्ण		*ttynm = रिक्त;

	uid = getuid();
	यदि ((pwd = getpwuid(uid)) != रिक्त) {
		uidbuf[0] = 0;
		म_जोड़न(uidbuf, pwd->pw_name, परिमाण(uidbuf) - 1);
	} अन्यथा {
		यदि (uid)
			प्र_लिखो(uidbuf, "uid %d", (पूर्णाङ्क) uid);
		अन्यथा
			प्र_लिखो(uidbuf, "root");
	}

	यदि ((tty = ttyname(0)) != रिक्त) {
		अचर माप_प्रकार plen = म_माप(_PATH_DEV);
		यदि (म_भेदन(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			यदि (tty[0] == '/')
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
स्थिर पूर्णाङ्क file_isatty(अचर वर्ण *fname)
{
	संरचना stat		st;
	पूर्णाङ्क			major;

	यदि (stat(fname, &st) < 0)
		फर्काऊ 0;

	यदि (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		फर्काऊ 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	यदि (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		फर्काऊ 0;
	फर्काऊ 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
शून्य broadcast(वर्ण *text)
{
	वर्ण *tty;
	वर्ण *user;
	संरचना utsname name;
	समय_प्रकार t;
	वर्ण *date;
	वर्ण *p;
	वर्ण *पङ्क्ति = रिक्त;
	पूर्णाङ्क len;
	पूर्णाङ्क fd;
	वर्ण *term = रिक्त;
	संरचना utmpx *utmp;

	getuidtty(&user, &tty);

	/*
	 * Get and report current hostname, to make it easier to find out
	 * which machine is being shut down.
	 */
	uname(&name);

	/* Get the time */
	समय(&t);
	date = स_समय(&t);
	p = म_अक्षर(date, '\n');
	यदि (p)
		*p = 0;

	len = xasprintf(&पङ्क्ति, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	मुक्त(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	यदि (विभाजन() != 0)
		फर्काऊ;

	setutxent();

	जबसम्म ((utmp = getutxent()) != रिक्त) {
		यदि (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			जारी;
		यदि (म_भेदन(utmp->ut_line, _PATH_DEV, म_माप(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		अन्यथा
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		यदि (म_माला(term, "/../")) {
			मुक्त(term);
			जारी;
		}

		/* Open it non-delay */
		यदि (file_isatty(term) && (fd = खोलो(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			यदि (isatty(fd))
				लिख(fd, पङ्क्ति, len);
			बंद(fd);
		}
		मुक्त(term);
	}
	endutxent();
	मुक्त(पङ्क्ति);
	निकास(0);
}
