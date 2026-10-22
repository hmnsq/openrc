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

#अन्तर्भाव <sys/time.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <utime.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"


ध्रुव वर्ण *applet = रिक्त;
ध्रुव वर्ण *extraopts = "file";
ध्रुव वर्ण getoptstring[] = "sw" getoptstring_COMMON;
ध्रुव संरचना option longopts[] = {
	{ "save", 0, रिक्त, 's' },
	{ "warn", 0, रिक्त, 'w' },
	longopts_COMMON
};
ध्रुव वर्ण * ध्रुव longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
ध्रुव वर्ण *usagestring = रिक्त;

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क opt, sflag = 0, wflag = 0;
	ध्रुव वर्ण *file = रिक्त;
	संरचना stat sb;
	संरचना timeval tv;

	applet = basename_c(argv[0]);
	यावत् ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णाङ्क *) 0)) != -1)
	{
		विकल्प (opt) {
		पक्ष 's':
			sflag = 1;
			भङ्ग;
		पक्ष 'w':
			wflag = 1;
			भङ्ग;
		case_RC_COMMON_GETOPT
		}
	}

	यदि (optind < argc)
		file = argv[optind++];
	अन्यथा
		eerrorx("swclock: Reference file was not specified");

	यदि (sflag) {
		पूर्णाङ्क fd = खोलो(file, O_WRONLY | O_CREAT, 0644);
		यदि (fd == -1)
			eerrorx("swclock: open: %s", म_त्रुटि(त्रुटि_सं));
		यदि (futimens(fd, रिक्त) == -1)
			eerrorx("swclock: futimens: %s", म_त्रुटि(त्रुटि_सं));
		बंद(fd);
		प्रत्यागम 0;
	}

	यदि (stat(file, &sb) == -1) {
		यदि (wflag != 0 && त्रुटि_सं == ENOENT)
			ewarn("swclock: `%s': %s", file, म_त्रुटि(त्रुटि_सं));
		अन्यथा
			eerrorx("swclock: `%s': %s", file, म_त्रुटि(त्रुटि_सं));
		प्रत्यागम 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	यदि (settimeofday(&tv, रिक्त) == -1)
		eerrorx("swclock: settimeofday: %s", म_त्रुटि(त्रुटि_सं));

	प्रत्यागम 0;
}
