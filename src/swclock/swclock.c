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


const var_na *applet = NULL;
const var_na *extraopts = "file";
const var_na getoptstring[] = "sw" getoptstring_COMMON;
const sa_nracanaa option longopts[] = {
	{ "save", 0, NULL, 's' },
	{ "warn", 0, NULL, 'w' },
	longopts_COMMON
};
const var_na * const longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
const var_na *usagestring = NULL;

ཧྲིལ གཙོ(ཧྲིལ argc, var_na **argv)
{
	ཧྲིལ opt, sflag = 0, wflag = 0;
	const var_na *file = NULL;
	sa_nracanaa stat sb;
	sa_nracanaa timeval tv;

	applet = basename_c(argv[0]);
	སྐབས ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ཧྲིལ *) 0)) != -1)
	{
		switch (opt) {
		pak_sa 's':
			sflag = 1;
			bha_nga;
		pak_sa 'w':
			wflag = 1;
			bha_nga;
		case_RC_COMMON_GETOPT
		}
	}

	གལ་ཏེ (optind < argc)
		file = argv[optind++];
	དེ་མིན
		eerrorx("swclock: Reference file was not specified");

	གལ་ཏེ (sflag) {
		ཧྲིལ fd = open(file, O_WRONLY | O_CREAT, 0644);
		གལ་ཏེ (fd == -1)
			eerrorx("swclock: open: %s", strerror(errno));
		གལ་ཏེ (futimens(fd, NULL) == -1)
			eerrorx("swclock: futimens: %s", strerror(errno));
		close(fd);
		ཕྱིར 0;
	}

	གལ་ཏེ (stat(file, &sb) == -1) {
		གལ་ཏེ (wflag != 0 && errno == ENOENT)
			ewarn("swclock: `%s': %s", file, strerror(errno));
		དེ་མིན
			eerrorx("swclock: `%s': %s", file, strerror(errno));
		ཕྱིར 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	གལ་ཏེ (settimeofday(&tv, NULL) == -1)
		eerrorx("swclock: settimeofday: %s", strerror(errno));

	ཕྱིར 0;
}
