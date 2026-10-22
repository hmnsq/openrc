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

#അന്തര്ഭാവ <sys/time.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <utime.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "_usage.h"
#അന്തര്ഭാവ "helpers.h"


സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;
സ്ഥിരം അക്ഷരം *extraopts = "file";
സ്ഥിരം അക്ഷരം getoptstring[] = "sw" getoptstring_COMMON;
സ്ഥിരം ഘടന option longopts[] = {
	{ "save", 0, ശൂന്യ, 's' },
	{ "warn", 0, ശൂന്യ, 'w' },
	longopts_COMMON
};
സ്ഥിരം അക്ഷരം * സ്ഥിരം longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
സ്ഥിരം അക്ഷരം *usagestring = ശൂന്യ;

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ opt, sflag = 0, wflag = 0;
	സ്ഥിരം അക്ഷരം *file = ശൂന്യ;
	ഘടന stat sb;
	ഘടന timeval tv;

	applet = basename_c(argv[0]);
	ആകുമ്പോൾ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (പൂർണ്ണസംഖ്യ *) 0)) != -1)
	{
		തിരഞ്ഞെടുപ്പ് (opt) {
		അവസ്ഥ 's':
			sflag = 1;
			നിർത്തുക;
		അവസ്ഥ 'w':
			wflag = 1;
			നിർത്തുക;
		case_RC_COMMON_GETOPT
		}
	}

	എങ്കിൽ (optind < argc)
		file = argv[optind++];
	അല്ലെങ്കിൽ
		eerrorx("swclock: Reference file was not specified");

	എങ്കിൽ (sflag) {
		പൂർണ്ണസംഖ്യ fd = open(file, O_WRONLY | O_CREAT, 0644);
		എങ്കിൽ (fd == -1)
			eerrorx("swclock: open: %s", strerror(errno));
		എങ്കിൽ (futimens(fd, ശൂന്യ) == -1)
			eerrorx("swclock: futimens: %s", strerror(errno));
		close(fd);
		തിരികെ 0;
	}

	എങ്കിൽ (stat(file, &sb) == -1) {
		എങ്കിൽ (wflag != 0 && errno == ENOENT)
			ewarn("swclock: `%s': %s", file, strerror(errno));
		അല്ലെങ്കിൽ
			eerrorx("swclock: `%s': %s", file, strerror(errno));
		തിരികെ 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	എങ്കിൽ (settimeofday(&tv, ശൂന്യ) == -1)
		eerrorx("swclock: settimeofday: %s", strerror(errno));

	തിരികെ 0;
}
