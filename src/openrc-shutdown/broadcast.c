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


#समावेश <fcntl.h>
#समावेश <paths.h>
#समावेश <pwd.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <sys/stat.h>
#समावेश <sys/sysmacros.h>
#समावेश <sys/types.h>
#समावेश <sys/utsname.h>
#समावेश <time.h>
#समावेश <unistd.h>
#समावेश <utmpx.h>

#समावेश "broadcast.h"
#समावेश "helpers.h"

#अगर_अघोषित _PATH_DEV
# define _PATH_DEV	"/dev/"
#पूर्ण_अगर


अटल व्योम getuidtty(अक्षर **userp, अक्षर **ttyp)
{
	काष्ठा passwd 		*pwd;
	uid_t			uid;
	अक्षर			*tty;
	अटल अक्षर		uidbuf[32];
	अक्षर		*ttynm = शून्य;

	uid = getuid();
	अगर ((pwd = getpwuid(uid)) != शून्य) {
		uidbuf[0] = 0;
		म_जोड़न(uidbuf, pwd->pw_name, माप(uidbuf) - 1);
	} अन्यथा {
		अगर (uid)
			प्र_लिखो(uidbuf, "uid %d", (पूर्णांक) uid);
		अन्यथा
			प्र_लिखो(uidbuf, "root");
	}

	अगर ((tty = ttyname(0)) != शून्य) {
		स्थिर माप_प्रकार plen = म_माप(_PATH_DEV);
		अगर (म_भेदन(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			अगर (tty[0] == '/')
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
अटल पूर्णांक file_isatty(स्थिर अक्षर *fname)
{
	काष्ठा stat		st;
	पूर्णांक			major;

	अगर (stat(fname, &st) < 0)
		वापस 0;

	अगर (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		वापस 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	अगर (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		वापस 0;
	वापस 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
व्योम broadcast(अक्षर *text)
{
	अक्षर *tty;
	अक्षर *user;
	काष्ठा utsname name;
	समय_प्रकार t;
	अक्षर *date;
	अक्षर *p;
	अक्षर *पंक्ति = शून्य;
	पूर्णांक len;
	पूर्णांक fd;
	अक्षर *term = शून्य;
	काष्ठा utmpx *utmp;

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
	अगर (p)
		*p = 0;

	len = xasprintf(&पंक्ति, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	मुक्त(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	अगर (विभाजन() != 0)
		वापस;

	setutxent();

	जबतक ((utmp = getutxent()) != शून्य) {
		अगर (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			जारी;
		अगर (म_भेदन(utmp->ut_line, _PATH_DEV, म_माप(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		अन्यथा
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		अगर (म_माला(term, "/../")) {
			मुक्त(term);
			जारी;
		}

		/* Open it non-delay */
		अगर (file_isatty(term) && (fd = खोलो(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			अगर (isatty(fd))
				ग_लिखो(fd, पंक्ति, len);
			बंद(fd);
		}
		मुक्त(term);
	}
	endutxent();
	मुक्त(पंक्ति);
	निकास(0);
}
