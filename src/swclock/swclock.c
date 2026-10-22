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

#અન્તર્ભાવ <sys/time.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <utime.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "_usage.h"
#અન્તર્ભાવ "helpers.h"


અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *extraopts = "file";
અચલ અક્ષર getoptstring[] = "sw" getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{ "save", 0, ખાલી, 's' },
	{ "warn", 0, ખાલી, 'w' },
	longopts_COMMON
};
અચલ અક્ષર * અચલ longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
અચલ અક્ષર *usagestring = ખાલી;

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	પૂર્ણાંક opt, sflag = 0, wflag = 0;
	અચલ અક્ષર *file = ખાલી;
	સંરચના stat sb;
	સંરચના timeval tv;

	applet = basename_c(argv[0]);
	જ્યારે ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (પૂર્ણાંક *) 0)) != -1)
	{
		પસંદગી (opt) {
		સ્થિતિ 's':
			sflag = 1;
			તોડો;
		સ્થિતિ 'w':
			wflag = 1;
			તોડો;
		case_RC_COMMON_GETOPT
		}
	}

	જો (optind < argc)
		file = argv[optind++];
	નહીંતર
		eerrorx("swclock: Reference file was not specified");

	જો (sflag) {
		પૂર્ણાંક fd = open(file, O_WRONLY | O_CREAT, 0644);
		જો (fd == -1)
			eerrorx("swclock: open: %s", strerror(errno));
		જો (futimens(fd, ખાલી) == -1)
			eerrorx("swclock: futimens: %s", strerror(errno));
		close(fd);
		પાછા 0;
	}

	જો (stat(file, &sb) == -1) {
		જો (wflag != 0 && errno == ENOENT)
			ewarn("swclock: `%s': %s", file, strerror(errno));
		નહીંતર
			eerrorx("swclock: `%s': %s", file, strerror(errno));
		પાછા 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	જો (settimeofday(&tv, ખાલી) == -1)
		eerrorx("swclock: settimeofday: %s", strerror(errno));

	પાછા 0;
}
