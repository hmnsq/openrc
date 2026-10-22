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


#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <paths.h>
#અન્તર્ભાવ <pwd.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/sysmacros.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/utsname.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <utmpx.h>

#અન્તર્ભાવ "broadcast.h"
#અન્તર્ભાવ "helpers.h"

#ifndef _PATH_DEV
# define _PATH_DEV	"/dev/"
#endif


સ્થિર શૂન્ય getuidtty(અક્ષર **userp, અક્ષર **ttyp)
{
	સંરચના passwd 		*pwd;
	uid_t			uid;
	અક્ષર			*tty;
	સ્થિર અક્ષર		uidbuf[32];
	અક્ષર		*ttynm = ખાલી;

	uid = getuid();
	જો ((pwd = getpwuid(uid)) != ખાલી) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, કદ(uidbuf) - 1);
	} નહીંતર {
		જો (uid)
			sprintf(uidbuf, "uid %d", (પૂર્ણાંક) uid);
		નહીંતર
			sprintf(uidbuf, "root");
	}

	જો ((tty = ttyname(0)) != ખાલી) {
		અચલ size_t plen = strlen(_PATH_DEV);
		જો (strncmp(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			જો (tty[0] == '/')
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
સ્થિર પૂર્ણાંક file_isatty(અચલ અક્ષર *fname)
{
	સંરચના stat		st;
	પૂર્ણાંક			major;

	જો (stat(fname, &st) < 0)
		પાછા 0;

	જો (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		પાછા 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	જો (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		પાછા 0;
	પાછા 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
શૂન્ય broadcast(અક્ષર *text)
{
	અક્ષર *tty;
	અક્ષર *user;
	સંરચના utsname name;
	time_t t;
	અક્ષર *date;
	અક્ષર *p;
	અક્ષર *લીટી = ખાલી;
	પૂર્ણાંક len;
	પૂર્ણાંક fd;
	અક્ષર *term = ખાલી;
	સંરચના utmpx *utmp;

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
	જો (p)
		*p = 0;

	len = xasprintf(&લીટી, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	મુક્ત(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	જો (fork() != 0)
		પાછા;

	setutxent();

	જ્યારે ((utmp = getutxent()) != ખાલી) {
		જો (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			ચાલુ;
		જો (strncmp(utmp->ut_line, _PATH_DEV, strlen(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		નહીંતર
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		જો (strstr(term, "/../")) {
			મુક્ત(term);
			ચાલુ;
		}

		/* Open it non-delay */
		જો (file_isatty(term) && (fd = open(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			જો (isatty(fd))
				લિખ(fd, લીટી, len);
			close(fd);
		}
		મુક્ત(term);
	}
	endutxent();
	મુક્ત(લીટી);
	exit(0);
}
