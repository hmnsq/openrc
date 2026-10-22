/*
 * swclock.c
 * Sets the system time from the mtime of the given file.
 * This is useful for systems who do not have a working RTC and rely on ntp.
 * OpenRC relies on the correct system time for a lot of operations
 * so this is needed quite early.
 */

/*
 * Copyright (c) 2007-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ಅನ್ತರ್ಭಾವ <sys/time.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <utime.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"


ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *extraopts = "file";
ಧ್ರುವ ವರ್ಣ getoptstring[] = "sw" getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{ "save", 0, ರಿಕ್ತ, 's' },
	{ "warn", 0, ರಿಕ್ತ, 'w' },
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
ಧ್ರುವ ವರ್ಣ *usagestring = ರಿಕ್ತ;

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ opt, sflag = 0, wflag = 0;
	ಧ್ರುವ ವರ್ಣ *file = ರಿಕ್ತ;
	ಸಂರಚನಾ stat sb;
	ಸಂರಚನಾ timeval tv;

	applet = basename_c(argv[0]);
	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಙ್ಕ *) 0)) != -1)
	{
		ವಿಕಲ್ಪ (opt) {
		ಪಕ್ಷ 's':
			sflag = 1;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'w':
			wflag = 1;
			ಭಙ್ಗ;
		case_RC_COMMON_GETOPT
		}
	}

	ಯದಿ (optind < argc)
		file = argv[optind++];
	ಅನ್ಯಥಾ
		eerrorx("swclock: Reference file was not specified");

	ಯದಿ (sflag) {
		ಪೂರ್ಣಾಙ್ಕ fd = open(file, O_WRONLY | O_CREAT, 0644);
		ಯದಿ (fd == -1)
			eerrorx("swclock: open: %s", strerror(errno));
		ಯದಿ (futimens(fd, ರಿಕ್ತ) == -1)
			eerrorx("swclock: futimens: %s", strerror(errno));
		close(fd);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಯದಿ (stat(file, &sb) == -1) {
		ಯದಿ (wflag != 0 && errno == ENOENT)
			ewarn("swclock: `%s': %s", file, strerror(errno));
		ಅನ್ಯಥಾ
			eerrorx("swclock: `%s': %s", file, strerror(errno));
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	ಯದಿ (settimeofday(&tv, ರಿಕ್ತ) == -1)
		eerrorx("swclock: settimeofday: %s", strerror(errno));

	ಪ್ರತ್ಯಾಗಮ 0;
}
