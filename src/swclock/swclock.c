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

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <utime.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"


ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ *extraopts = "file";
ਅਚਲ ਅੱਖਰ getoptstring[] = "sw" getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "save", 0, ਨਿਰਾ, 's' },
	{ "warn", 0, ਨਿਰਾ, 'w' },
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
ਅਚਲ ਅੱਖਰ *usagestring = ਨਿਰਾ;

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਪੂਰਨ opt, sflag = 0, wflag = 0;
	ਅਚਲ ਅੱਖਰ *file = ਨਿਰਾ;
	ਬਣਤਰ stat sb;
	ਬਣਤਰ timeval tv;

	applet = basename_c(argv[0]);
	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ਪੂਰਨ *) 0)) != -1)
	{
		ਚੋਣ (opt) {
		ਹਾਲਤ 's':
			sflag = 1;
			ਰੋਕੋ;
		ਹਾਲਤ 'w':
			wflag = 1;
			ਰੋਕੋ;
		case_RC_COMMON_GETOPT
		}
	}

	ਜੇ (optind < argc)
		file = argv[optind++];
	ਨਹੀਂਤਾਂ
		eerrorx("swclock: Reference file was not specified");

	ਜੇ (sflag) {
		ਪੂਰਨ fd = open(file, O_WRONLY | O_CREAT, 0644);
		ਜੇ (fd == -1)
			eerrorx("swclock: open: %s", strerror(errno));
		ਜੇ (futimens(fd, ਨਿਰਾ) == -1)
			eerrorx("swclock: futimens: %s", strerror(errno));
		close(fd);
		ਵਾਪਸ 0;
	}

	ਜੇ (stat(file, &sb) == -1) {
		ਜੇ (wflag != 0 && errno == ENOENT)
			ewarn("swclock: `%s': %s", file, strerror(errno));
		ਨਹੀਂਤਾਂ
			eerrorx("swclock: `%s': %s", file, strerror(errno));
		ਵਾਪਸ 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	ਜੇ (settimeofday(&tv, ਨਿਰਾ) == -1)
		eerrorx("swclock: settimeofday: %s", strerror(errno));

	ਵਾਪਸ 0;
}
