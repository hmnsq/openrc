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

#شامل <sys/time.h>
#شامل <sys/stat.h>
#شامل <errno.h>
#شامل <fcntl.h>
#شامل <getopt.h>
#شامل <unistd.h>
#شامل <utime.h>
#شامل <string.h>

#شامل "einfo.h"
#شامل "_usage.h"
#شامل "helpers.h"


ثابت حرف *applet = تہی;
ثابت حرف *extraopts = "file";
ثابت حرف getoptstring[] = "sw" getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{ "save", 0, تہی, 's' },
	{ "warn", 0, تہی, 'w' },
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
ثابت حرف *usagestring = تہی;

صحیح مین(صحیح argc, حرف **argv)
{
	صحیح opt, sflag = 0, wflag = 0;
	ثابت حرف *file = تہی;
	ساخت stat sb;
	ساخت timeval tv;

	applet = basename_c(argv[0]);
	جب ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
		حالت 's':
			sflag = 1;
			توڑو;
		حالت 'w':
			wflag = 1;
			توڑو;
		case_RC_COMMON_GETOPT
		}
	}

	اگر (optind < argc)
		file = argv[optind++];
	ورنہ
		eerrorx("swclock: Reference file was not specified");

	اگر (sflag) {
		صحیح fd = open(file, O_WRONLY | O_CREAT, 0644);
		اگر (fd == -1)
			eerrorx("swclock: open: %s", strerror(errno));
		اگر (futimens(fd, تہی) == -1)
			eerrorx("swclock: futimens: %s", strerror(errno));
		close(fd);
		واپس 0;
	}

	اگر (stat(file, &sb) == -1) {
		اگر (wflag != 0 && errno == ENOENT)
			ewarn("swclock: `%s': %s", file, strerror(errno));
		ورنہ
			eerrorx("swclock: `%s': %s", file, strerror(errno));
		واپس 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	اگر (settimeofday(&tv, تہی) == -1)
		eerrorx("swclock: settimeofday: %s", strerror(errno));

	واپس 0;
}
