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

#அந்தர்பாவ <sys/time.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <utime.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "helpers.h"


மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *extraopts = "file";
மாறிலி எழுத்து getoptstring[] = "sw" getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "save", 0, வெற்று, 's' },
	{ "warn", 0, வெற்று, 'w' },
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
மாறிலி எழுத்து *usagestring = வெற்று;

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	முழு opt, sflag = 0, wflag = 0;
	மாறிலி எழுத்து *file = வெற்று;
	கட்டமைப்பு stat sb;
	கட்டமைப்பு timeval tv;

	applet = basename_c(argv[0]);
	போது ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (முழு *) 0)) != -1)
	{
		தேர்வு (opt) {
		நிலை 's':
			sflag = 1;
			நிறுத்து;
		நிலை 'w':
			wflag = 1;
			நிறுத்து;
		case_RC_COMMON_GETOPT
		}
	}

	எனில் (optind < argc)
		file = argv[optind++];
	இல்லையெனில்
		eerrorx("swclock: Reference file was not specified");

	எனில் (sflag) {
		முழு fd = open(file, O_WRONLY | O_CREAT, 0644);
		எனில் (fd == -1)
			eerrorx("swclock: open: %s", strerror(errno));
		எனில் (futimens(fd, வெற்று) == -1)
			eerrorx("swclock: futimens: %s", strerror(errno));
		close(fd);
		திரும்பு 0;
	}

	எனில் (stat(file, &sb) == -1) {
		எனில் (wflag != 0 && errno == ENOENT)
			ewarn("swclock: `%s': %s", file, strerror(errno));
		இல்லையெனில்
			eerrorx("swclock: `%s': %s", file, strerror(errno));
		திரும்பு 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	எனில் (settimeofday(&tv, வெற்று) == -1)
		eerrorx("swclock: settimeofday: %s", strerror(errno));

	திரும்பு 0;
}
