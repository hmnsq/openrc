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


स्थिर शून्य getuidtty(अक्षर **userp, अक्षर **ttyp)
{
	संरचना passwd 		*pwd;
	uid_t			uid;
	अक्षर			*tty;
	स्थिर अक्षर		uidbuf[32];
	अक्षर		*ttynm = रिक्त;

	uid = getuid();
	जर ((pwd = getpwuid(uid)) != रिक्त) {
		uidbuf[0] = 0;
		म_जोड़न(uidbuf, pwd->pw_name, आकार(uidbuf) - 1);
	} अन्यथा {
		जर (uid)
			प्र_लिखो(uidbuf, "uid %d", (पूर्णांक) uid);
		अन्यथा
			प्र_लिखो(uidbuf, "root");
	}

	जर ((tty = ttyname(0)) != रिक्त) {
		स्थिरांक माप_प्रकार plen = म_माप(_PATH_DEV);
		जर (म_भेदन(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			जर (tty[0] == '/')
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
स्थिर पूर्णांक file_isatty(स्थिरांक अक्षर *fname)
{
	संरचना stat		st;
	पूर्णांक			major;

	जर (stat(fname, &st) < 0)
		परत 0;

	जर (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		परत 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	जर (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		परत 0;
	परत 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
शून्य broadcast(अक्षर *text)
{
	अक्षर *tty;
	अक्षर *user;
	संरचना utsname name;
	समय_प्रकार t;
	अक्षर *date;
	अक्षर *p;
	अक्षर *ओळ = रिक्त;
	पूर्णांक len;
	पूर्णांक fd;
	अक्षर *term = रिक्त;
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
	जर (p)
		*p = 0;

	len = xasprintf(&ओळ, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	मुक्त(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	जर (विभाजन() != 0)
		परत;

	setutxent();

	पर्यंत ((utmp = getutxent()) != रिक्त) {
		जर (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			सुरूठेवा;
		जर (म_भेदन(utmp->ut_line, _PATH_DEV, म_माप(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		अन्यथा
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		जर (म_माला(term, "/../")) {
			मुक्त(term);
			सुरूठेवा;
		}

		/* Open it non-delay */
		जर (file_isatty(term) && (fd = खोलो(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			जर (isatty(fd))
				लिख(fd, ओळ, len);
			बंद(fd);
		}
		मुक्त(term);
	}
	endutxent();
	मुक्त(ओळ);
	निकास(0);
}
