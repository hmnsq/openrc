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


ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts = "file";
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = "sw" getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{ "save", 0, ಖಾಲಿ, 's' },
	{ "warn", 0, ಖಾಲಿ, 'w' },
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ಖಾಲಿ;

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ opt, sflag = 0, wflag = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file = ಖಾಲಿ;
	ರಚನೆ stat sb;
	ರಚನೆ timeval tv;

	applet = basename_c(argv[0]);
	ಆಗ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಂಕ *) 0)) != -1)
	{
		ಆಯ್ಕೆ (opt) {
		ಸ್ಥಿತಿ 's':
			sflag = 1;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'w':
			wflag = 1;
			ನಿಲ್ಲಿಸು;
		case_RC_COMMON_GETOPT
		}
	}

	ಆದರೆ (optind < argc)
		file = argv[optind++];
	ಇಲ್ಲದಿದ್ದರೆ
		eerrorx("swclock: Reference file was not specified");

	ಆದರೆ (sflag) {
		ಪೂರ್ಣಾಂಕ fd = open(file, O_WRONLY | O_CREAT, 0644);
		ಆದರೆ (fd == -1)
			eerrorx("swclock: open: %s", strerror(errno));
		ಆದರೆ (futimens(fd, ಖಾಲಿ) == -1)
			eerrorx("swclock: futimens: %s", strerror(errno));
		close(fd);
		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (stat(file, &sb) == -1) {
		ಆದರೆ (wflag != 0 && errno == ENOENT)
			ewarn("swclock: `%s': %s", file, strerror(errno));
		ಇಲ್ಲದಿದ್ದರೆ
			eerrorx("swclock: `%s': %s", file, strerror(errno));
		ಹಿಂತಿರುಗಿ 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	ಆದರೆ (settimeofday(&tv, ಖಾಲಿ) == -1)
		eerrorx("swclock: settimeofday: %s", strerror(errno));

	ಹಿಂತಿರುಗಿ 0;
}
