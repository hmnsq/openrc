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

#অন্তর্ভাব <sys/time.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <utime.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "_usage.h"
#অন্তর্ভাব "helpers.h"


ধ্রুব বর্ণ *applet = রিক্ত;
ধ্রুব বর্ণ *extraopts = "file";
ধ্রুব বর্ণ getoptstring[] = "sw" getoptstring_COMMON;
ধ্রুব সংরচনা option longopts[] = {
	{ "save", 0, রিক্ত, 's' },
	{ "warn", 0, রিক্ত, 'w' },
	longopts_COMMON
};
ধ্রুব বর্ণ * ধ্রুব longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
ধ্রুব বর্ণ *usagestring = রিক্ত;

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	পূর্ণাঙ্ক opt, sflag = 0, wflag = 0;
	ধ্রুব বর্ণ *file = রিক্ত;
	সংরচনা stat sb;
	সংরচনা timeval tv;

	applet = basename_c(argv[0]);
	যাবত্ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (পূর্ণাঙ্ক *) 0)) != -1)
	{
		বিকল্প (opt) {
		পক্ষ 's':
			sflag = 1;
			ভঙ্গ;
		পক্ষ 'w':
			wflag = 1;
			ভঙ্গ;
		case_RC_COMMON_GETOPT
		}
	}

	যদি (optind < argc)
		file = argv[optind++];
	অন্যথা
		eerrorx("swclock: Reference file was not specified");

	যদি (sflag) {
		পূর্ণাঙ্ক fd = open(file, O_WRONLY | O_CREAT, 0644);
		যদি (fd == -1)
			eerrorx("swclock: open: %s", strerror(errno));
		যদি (futimens(fd, রিক্ত) == -1)
			eerrorx("swclock: futimens: %s", strerror(errno));
		close(fd);
		প্রত্যাগম 0;
	}

	যদি (stat(file, &sb) == -1) {
		যদি (wflag != 0 && errno == ENOENT)
			ewarn("swclock: `%s': %s", file, strerror(errno));
		অন্যথা
			eerrorx("swclock: `%s': %s", file, strerror(errno));
		প্রত্যাগম 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	যদি (settimeofday(&tv, রিক্ত) == -1)
		eerrorx("swclock: settimeofday: %s", strerror(errno));

	প্রত্যাগম 0;
}
