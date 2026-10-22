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


#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <paths.h>
#அந்தர்பாவ <pwd.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/sysmacros.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/utsname.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <utmpx.h>

#அந்தர்பாவ "broadcast.h"
#அந்தர்பாவ "helpers.h"

#ifndef _PATH_DEV
# define _PATH_DEV	"/dev/"
#endif


நிலையான வெறுமை getuidtty(எழுத்து **userp, எழுத்து **ttyp)
{
	கட்டமைப்பு passwd 		*pwd;
	uid_t			uid;
	எழுத்து			*tty;
	நிலையான எழுத்து		uidbuf[32];
	எழுத்து		*ttynm = வெற்று;

	uid = getuid();
	எனில் ((pwd = getpwuid(uid)) != வெற்று) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, அளவு(uidbuf) - 1);
	} இல்லையெனில் {
		எனில் (uid)
			sprintf(uidbuf, "uid %d", (முழு) uid);
		இல்லையெனில்
			sprintf(uidbuf, "root");
	}

	எனில் ((tty = ttyname(0)) != வெற்று) {
		மாறிலி size_t plen = strlen(_PATH_DEV);
		எனில் (strncmp(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			எனில் (tty[0] == '/')
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
நிலையான முழு file_isatty(மாறிலி எழுத்து *fname)
{
	கட்டமைப்பு stat		st;
	முழு			major;

	எனில் (stat(fname, &st) < 0)
		திரும்பு 0;

	எனில் (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		திரும்பு 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	எனில் (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		திரும்பு 0;
	திரும்பு 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
வெறுமை broadcast(எழுத்து *text)
{
	எழுத்து *tty;
	எழுத்து *user;
	கட்டமைப்பு utsname name;
	time_t t;
	எழுத்து *date;
	எழுத்து *p;
	எழுத்து *வரி = வெற்று;
	முழு len;
	முழு fd;
	எழுத்து *term = வெற்று;
	கட்டமைப்பு utmpx *utmp;

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
	எனில் (p)
		*p = 0;

	len = xasprintf(&வரி, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, text);
	விடுவி(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	எனில் (fork() != 0)
		திரும்பு;

	setutxent();

	போது ((utmp = getutxent()) != வெற்று) {
		எனில் (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			தொடர்;
		எனில் (strncmp(utmp->ut_line, _PATH_DEV, strlen(_PATH_DEV)) == 0)
			xasprintf(&term, "%s", utmp->ut_line);
		இல்லையெனில்
			xasprintf(&term, "%s%s", _PATH_DEV, utmp->ut_line);
		எனில் (strstr(term, "/../")) {
			விடுவி(term);
			தொடர்;
		}

		/* Open it non-delay */
		எனில் (file_isatty(term) && (fd = open(term, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			எனில் (isatty(fd))
				லிக(fd, வரி, len);
			close(fd);
		}
		விடுவி(term);
	}
	endutxent();
	விடுவி(வரி);
	exit(0);
}
