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

#समावेश <sys/time.h>
#समावेश <sys/stat.h>
#समावेश <errno.h>
#समावेश <fcntl.h>
#समावेश <getopt.h>
#समावेश <unistd.h>
#समावेश <utime.h>
#समावेश <string.h>

#समावेश "einfo.h"
#समावेश "_usage.h"
#समावेश "helpers.h"


स्थिर अक्षर *applet = शून्य;
स्थिर अक्षर *extraopts = "file";
स्थिर अक्षर getoptstring[] = "sw" getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "save", 0, शून्य, 's' },
	{ "warn", 0, शून्य, 'w' },
	longopts_COMMON
};
स्थिर अक्षर * स्थिर longopts_help[] = {
	"saves the time",
	"no error if no reference file",
	longopts_help_COMMON
};
स्थिर अक्षर *usagestring = शून्य;

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक opt, sflag = 0, wflag = 0;
	स्थिर अक्षर *file = शून्य;
	काष्ठा stat sb;
	काष्ठा timeval tv;

	applet = basename_c(argv[0]);
	जबतक ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		चयन (opt) {
		हाल 's':
			sflag = 1;
			अवरोध;
		हाल 'w':
			wflag = 1;
			अवरोध;
		case_RC_COMMON_GETOPT
		}
	}

	अगर (optind < argc)
		file = argv[optind++];
	अन्यथा
		eerrorx("swclock: Reference file was not specified");

	अगर (sflag) {
		पूर्णांक fd = खोलो(file, O_WRONLY | O_CREAT, 0644);
		अगर (fd == -1)
			eerrorx("swclock: open: %s", म_त्रुटि(त्रुटि_सं));
		अगर (futimens(fd, शून्य) == -1)
			eerrorx("swclock: futimens: %s", म_त्रुटि(त्रुटि_सं));
		बंद(fd);
		वापस 0;
	}

	अगर (stat(file, &sb) == -1) {
		अगर (wflag != 0 && त्रुटि_सं == ENOENT)
			ewarn("swclock: `%s': %s", file, म_त्रुटि(त्रुटि_सं));
		अन्यथा
			eerrorx("swclock: `%s': %s", file, म_त्रुटि(त्रुटि_सं));
		वापस 0;
	}

	tv.tv_sec = sb.st_mtime;
	tv.tv_usec = 0;

	अगर (settimeofday(&tv, शून्य) == -1)
		eerrorx("swclock: settimeofday: %s", म_त्रुटि(त्रुटि_सं));

	वापस 0;
}
