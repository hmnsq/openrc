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

#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <utime.h>
#include <string.h>

#include "einfo.h"
#include "_usage.h"
#include "helpers.h"


const char *applet = NULL;
const char *extraopts = "file";
const char getoptstring[] = "sw" getoptstring_COMMON;
const struct option longopts[] = {
	{ "save", 0, NULL, 's' },
	{ "warn", 0, NULL, 'w' },
	longopts_COMMON
};
const char * const longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
const char *usagestring = NULL;

整数 主要(整数 argc, char **argv)
{
	整数 opt, sflag = 0, wflag = 0;
	const char *file = NULL;
	struct stat sb;
	struct timeval tv;

	applet = basename_c(argv[0]);
	当 ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (整数 *) 0)) != -1)
	{
		switch (opt) {
		case 's':
			sflag = 1;
			break;
		case 'w':
			wflag = 1;
			break;
		case_RC_COMMON_GETOPT
		}
	}

	如果 (optind < argc)
		file = argv[optind++];
	否则
		eerrorx("swclock: Reference file was not specified");

	如果 (sflag) {
		整数 fd = open(file, O_WRONLY | O_CREAT, 0644);
		如果 (fd == -1)
			eerrorx("swclock: open: %s", strerror(errno));
		如果 (futimens(fd, NULL) == -1)
			eerrorx("swclock: futimens: %s", strerror(errno));
		close(fd);
		返回 0;
	}

	如果 (stat(file, &sb) == -1) {
		如果 (wflag != 0 && errno == ENOENT)
			ewarn("swclock: `%s': %s", file, strerror(errno));
		否则
			eerrorx("swclock: `%s': %s", file, strerror(errno));
		返回 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	如果 (settimeofday(&tv, NULL) == -1)
		eerrorx("swclock: settimeofday: %s", strerror(errno));

	返回 0;
}
