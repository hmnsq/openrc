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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _PATH_DEV
# define _PATH_DEV	"/dev/"
#ಆದರೆ_ಅಂತ್ಯ


ಸ್ಥಿರ ಶೂನ್ಯ getuidtty(ಅಕ್ಷರ **userp, ಅಕ್ಷರ **ttyp)
{
	ರಚನೆ passwd 		*pwd;
	uid_t			uid;
	ಅಕ್ಷರ			*tty;
	ಸ್ಥಿರ ಅಕ್ಷರ		uidbuf[32];
	ಅಕ್ಷರ		*ttynm = ಖಾಲಿ;

	uid = getuid();
	ಆದರೆ ((pwd = getpwuid(uid)) != ಖಾಲಿ) {
		uidbuf[0] = 0;
		strncat(uidbuf, pwd->pw_name, ಗಾತ್ರ(uidbuf) - 1);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (uid)
			sprintf(uidbuf, "uid %d", (ಪೂರ್ಣಾಂಕ) uid);
		ಇಲ್ಲದಿದ್ದರೆ
			sprintf(uidbuf, "root");
	}

	ಆದರೆ ((tty = ttyname(0)) != ಖಾಲಿ) {
		ಸ್ಥಿರಾಂಕ size_t plen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(_PATH_DEV);
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(tty, _PATH_DEV, plen) == 0) {
			tty += plen;
			ಆದರೆ (tty[0] == '/')
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ file_isatty(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
	ರಚನೆ stat		st;
	ಪೂರ್ಣಾಂಕ			major;

	ಆದರೆ (stat(fname, &st) < 0)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (st.st_nlink != 1 || !S_ISCHR(st.st_mode))
		ಹಿಂತಿರುಗಿ 0;

	/*
	 *	It would be an impossible task to list all major/minors
	 *	of tty devices here, so we just exclude the obvious
	 *	majors of which just opening has side-effects:
	 *	printers and tapes.
	 */
	major = major(st.st_dev);
	ಆದರೆ (major == 1 || major == 2 || major == 6 || major == 9 ||
	    major == 12 || major == 16 || major == 21 || major == 27 ||
	    major == 37 || major == 96 || major == 97 || major == 206 ||
	    major == 230)
		ಹಿಂತಿರುಗಿ 0;
	ಹಿಂತಿರುಗಿ 1;
}

/*
 *	broadcast function.
 *
 *	NB: Not multithread safe.
 */
ಶೂನ್ಯ ಪ್ರಸಾರ(ಅಕ್ಷರ *ಪಠ್ಯ)
{
	ಅಕ್ಷರ *tty;
	ಅಕ್ಷರ *user;
	ರಚನೆ utsname name;
	time_t t;
	ಅಕ್ಷರ *date;
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *ಸಾಲು = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ len;
	ಪೂರ್ಣಾಂಕ fd;
	ಅಕ್ಷರ *ಪದ = ಖಾಲಿ;
	ರಚನೆ utmpx *utmp;

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
	ಆದರೆ (p)
		*p = 0;

	len = xasprintf(&ಸಾಲು, "\007\r\nBroadcast message from %s@%s %s(%s):\r\n\r\n%s",
			user, name.nodename, tty, date, ಪಠ್ಯ);
	ಬಿಡುಗಡೆ(tty);

	/*
	 *	Fork to avoid hanging in a write()
	 */
	ಆದರೆ (fork() != 0)
		ಹಿಂತಿರುಗಿ;

	setutxent();

	ಆಗ ((utmp = getutxent()) != ಖಾಲಿ) {
		ಆದರೆ (utmp->ut_type != USER_PROCESS || utmp->ut_user[0] == 0)
			ಮುಂದುವರಿ;
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(utmp->ut_line, _PATH_DEV, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(_PATH_DEV)) == 0)
			xasprintf(&ಪದ, "%s", utmp->ut_line);
		ಇಲ್ಲದಿದ್ದರೆ
			xasprintf(&ಪದ, "%s%s", _PATH_DEV, utmp->ut_line);
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(ಪದ, "/../")) {
			ಬಿಡುಗಡೆ(ಪದ);
			ಮುಂದುವರಿ;
		}

		/* Open it non-delay */
		ಆದರೆ (file_isatty(ಪದ) && (fd = open(ಪದ, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0) {
			ಆದರೆ (isatty(fd))
				ಲಿಖ(fd, ಸಾಲು, len);
			close(fd);
		}
		ಬಿಡುಗಡೆ(ಪದ);
	}
	endutxent();
	ಬಿಡುಗಡೆ(ಸಾಲು);
	ನಿರ್ಗಮಿಸು(0);
}
