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


स्थिरांक अक्षर *applet = रिक्त;
स्थिरांक अक्षर *extraopts = "file";
स्थिरांक अक्षर getoptstring[] = "sw" getoptstring_COMMON;
स्थिरांक संरचना option longopts[] = {
	{ "save", 0, रिक्त, 's' },
	{ "warn", 0, रिक्त, 'w' },
	longopts_COMMON
};
स्थिरांक अक्षर * स्थिरांक longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
स्थिरांक अक्षर *usagestring = रिक्त;

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक opt, sflag = 0, wflag = 0;
	स्थिरांक अक्षर *file = रिक्त;
	संरचना stat sb;
	संरचना timeval tv;

	applet = basename_c(argv[0]);
	पर्यंत ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		निवड (opt) {
		स्थिती 's':
			sflag = 1;
			थांबा;
		स्थिती 'w':
			wflag = 1;
			थांबा;
		case_RC_COMMON_GETOPT
		}
	}

	जर (optind < argc)
		file = argv[optind++];
	अन्यथा
		eerrorx("swclock: Reference file was not specified");

	जर (sflag) {
		पूर्णांक fd = खोलो(file, O_WRONLY | O_CREAT, 0644);
		जर (fd == -1)
			eerrorx("swclock: open: %s", म_त्रुटि(त्रुटि_सं));
		जर (futimens(fd, रिक्त) == -1)
			eerrorx("swclock: futimens: %s", म_त्रुटि(त्रुटि_सं));
		बंद(fd);
		परत 0;
	}

	जर (stat(file, &sb) == -1) {
		जर (wflag != 0 && त्रुटि_सं == ENOENT)
			ewarn("swclock: `%s': %s", file, म_त्रुटि(त्रुटि_सं));
		अन्यथा
			eerrorx("swclock: `%s': %s", file, म_त्रुटि(त्रुटि_सं));
		परत 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	जर (settimeofday(&tv, रिक्त) == -1)
		eerrorx("swclock: settimeofday: %s", म_त्रुटि(त्रुटि_सं));

	परत 0;
}
