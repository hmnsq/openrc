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

#ضمن <sys/time.h>
#ضمن <sys/stat.h>
#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <getopt.h>
#ضمن <unistd.h>
#ضمن <utime.h>
#ضمن <string.h>

#ضمن "einfo.h"
#ضمن "_usage.h"
#ضمن "helpers.h"


ثابت حرف *applet = عدم;
ثابت حرف *extraopts = "file";
ثابت حرف getoptstring[] = "sw" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "save", 0, عدم, 's' },
	{ "warn", 0, عدم, 'w' },
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
ثابت حرف *usagestring = عدم;

صحيح الرئيسية(صحيح argc, حرف **argv)
{
	صحيح opt, sflag = 0, wflag = 0;
	ثابت حرف *file = عدم;
	بنية stat sb;
	بنية timeval tv;

	applet = basename_c(argv[0]);
	بينما ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
		حالة 's':
			sflag = 1;
			اكسر;
		حالة 'w':
			wflag = 1;
			اكسر;
		case_RC_COMMON_GETOPT
		}
	}

	إذا (optind < argc)
		file = argv[optind++];
	وإلا
		eerrorx("swclock: Reference file was not specified");

	إذا (sflag) {
		صحيح fd = open(file, O_WRONLY | O_CREAT, 0644);
		إذا (fd == -1)
			eerrorx("swclock: open: %s", strerror(errno));
		إذا (futimens(fd, عدم) == -1)
			eerrorx("swclock: futimens: %s", strerror(errno));
		close(fd);
		إرجاع 0;
	}

	إذا (stat(file, &sb) == -1) {
		إذا (wflag != 0 && errno == ENOENT)
			ewarn("swclock: `%s': %s", file, strerror(errno));
		وإلا
			eerrorx("swclock: `%s': %s", file, strerror(errno));
		إرجاع 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	إذا (settimeofday(&tv, عدم) == -1)
		eerrorx("swclock: settimeofday: %s", strerror(errno));

	إرجاع 0;
}
