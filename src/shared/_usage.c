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

#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "_usage.h"
#അന്തര്ഭാവ "version.h"
#അന്തര്ഭാവ "helpers.h"


ശൂന്യം set_quiet_options(ശൂന്യം)
{
	സ്ഥിതം പൂർണ്ണസംഖ്യ qcount = 0;

	qcount ++;
	തിരഞ്ഞെടുപ്പ് (qcount) {
	അവസ്ഥ 1:
		setenv ("EINFO_QUIET", "YES", 1);
		നിർത്തുക;
	അവസ്ഥ 2:
		setenv ("EERROR_QUIET", "YES", 1);
		നിർത്തുക;
	}
}

RC_NORETURN ശൂന്യം show_version(ശൂന്യം)
{
	സ്ഥിരം അക്ഷരം *systype = ശൂന്യ;

	printf("%s (OpenRC", applet);
	എങ്കിൽ ((systype = rc_sys()))
		printf(" [%s]", systype);
	printf(") %s", VERSION);
#ifdef BRANDING
	printf(" (%s)", BRANDING);
#endif
	printf("\n");
	exit(EXIT_SUCCESS);
}

RC_NORETURN ശൂന്യം usage(പൂർണ്ണസംഖ്യ exit_status)
{
	സ്ഥിരം അക്ഷരം * സ്ഥിരം has_arg[] = { "", "<arg>", "[arg]" };
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ len;
	അക്ഷരം *lo;
	അക്ഷരം *p;
	അക്ഷരം *token;
	അക്ഷരം val[4] = "-?,";

	എങ്കിൽ (usagestring)
		printf("%s", usagestring);
	അല്ലെങ്കിൽ
		printf("Usage: %s [options] ", applet);

	എങ്കിൽ (extraopts)
		printf("%s", extraopts);

	printf("\n\nOptions: [ %s ]\n", getoptstring);
	വേണ്ടി (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = printf("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		ആകുമ്പോൾ ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			എങ്കിൽ (len > 0)
				printf("%*s", len, "");
			puts(token);
			len = 0;
		}
		സ്വതന്ത്രം(lo);
	}
	exit(exit_status);
}
