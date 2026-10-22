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

#ଅନ୍ତର୍ଭାଵ <sys/time.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <utime.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"


ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *extraopts = "file";
ଅଚଳ ଅକ୍ଷର getoptstring[] = "sw" getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "save", 0, ଖାଲି, 's' },
	{ "warn", 0, ଖାଲି, 'w' },
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
ଅଚଳ ଅକ୍ଷର *usagestring = ଖାଲି;

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt, sflag = 0, wflag = 0;
	ଅଚଳ ଅକ୍ଷର *file = ଖାଲି;
	ସଂରଚନା stat sb;
	ସଂରଚନା timeval tv;

	applet = basename_c(argv[0]);
	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) 0)) != -1)
	{
		ବାଛ (opt) {
		ସ୍ଥିତି 's':
			sflag = 1;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'w':
			wflag = 1;
			ଭାଙ୍ଗ;
		case_RC_COMMON_GETOPT
		}
	}

	ଯଦି (optind < argc)
		file = argv[optind++];
	ଅନ୍ୟଥା
		eerrorx("swclock: Reference file was not specified");

	ଯଦି (sflag) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = open(file, O_WRONLY | O_CREAT, 0644);
		ଯଦି (fd == -1)
			eerrorx("swclock: open: %s", strerror(errno));
		ଯଦି (futimens(fd, ଖାଲି) == -1)
			eerrorx("swclock: futimens: %s", strerror(errno));
		close(fd);
		ଫେରନ୍ତୁ 0;
	}

	ଯଦି (stat(file, &sb) == -1) {
		ଯଦି (wflag != 0 && errno == ENOENT)
			ewarn("swclock: `%s': %s", file, strerror(errno));
		ଅନ୍ୟଥା
			eerrorx("swclock: `%s': %s", file, strerror(errno));
		ଫେରନ୍ତୁ 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	ଯଦି (settimeofday(&tv, ଖାଲି) == -1)
		eerrorx("swclock: settimeofday: %s", strerror(errno));

	ଫେରନ୍ତୁ 0;
}
