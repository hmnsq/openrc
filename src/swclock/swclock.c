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

#అన్తర్భావ <sys/time.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <utime.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "helpers.h"


స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *extraopts = "file";
స్థిరరాశి అక్షరం getoptstring[] = "sw" getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "save", 0, ఖాళీ, 's' },
	{ "warn", 0, ఖాళీ, 'w' },
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
స్థిరరాశి అక్షరం *usagestring = ఖాళీ;

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	పూర్ణాంకం opt, sflag = 0, wflag = 0;
	స్థిరరాశి అక్షరం *file = ఖాళీ;
	నిర్మాణం stat sb;
	నిర్మాణం timeval tv;

	applet = basename_c(argv[0]);
	అయితే ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (పూర్ణాంకం *) 0)) != -1)
	{
		ఎంపిక (opt) {
		స్థితి 's':
			sflag = 1;
			ఆపు;
		స్థితి 'w':
			wflag = 1;
			ఆపు;
		case_RC_COMMON_GETOPT
		}
	}

	ఒకవేళ (optind < argc)
		file = argv[optind++];
	లేకపోతే
		eerrorx("swclock: Reference file was not specified");

	ఒకవేళ (sflag) {
		పూర్ణాంకం fd = open(file, O_WRONLY | O_CREAT, 0644);
		ఒకవేళ (fd == -1)
			eerrorx("swclock: open: %s", strerror(errno));
		ఒకవేళ (futimens(fd, ఖాళీ) == -1)
			eerrorx("swclock: futimens: %s", strerror(errno));
		close(fd);
		తిరిగి 0;
	}

	ఒకవేళ (stat(file, &sb) == -1) {
		ఒకవేళ (wflag != 0 && errno == ENOENT)
			ewarn("swclock: `%s': %s", file, strerror(errno));
		లేకపోతే
			eerrorx("swclock: `%s': %s", file, strerror(errno));
		తిరిగి 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	ఒకవేళ (settimeofday(&tv, ఖాళీ) == -1)
		eerrorx("swclock: settimeofday: %s", strerror(errno));

	తిరిగి 0;
}
