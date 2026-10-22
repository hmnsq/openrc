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

#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "_usage.h"
#અન્તર્ભાવ "version.h"
#અન્તર્ભાવ "helpers.h"


શૂન્ય set_quiet_options(શૂન્ય)
{
	સ્થિર પૂર્ણાંક qcount = 0;

	qcount ++;
	પસંદગી (qcount) {
	સ્થિતિ 1:
		setenv ("EINFO_QUIET", "YES", 1);
		તોડો;
	સ્થિતિ 2:
		setenv ("EERROR_QUIET", "YES", 1);
		તોડો;
	}
}

RC_NORETURN શૂન્ય show_version(શૂન્ય)
{
	અચલ અક્ષર *systype = ખાલી;

	printf("%s (OpenRC", applet);
	જો ((systype = rc_sys()))
		printf(" [%s]", systype);
	printf(") %s", VERSION);
#ifdef BRANDING
	printf(" (%s)", BRANDING);
#endif
	printf("\n");
	exit(EXIT_SUCCESS);
}

RC_NORETURN શૂન્ય usage(પૂર્ણાંક exit_status)
{
	અચલ અક્ષર * અચલ has_arg[] = { "", "<arg>", "[arg]" };
	પૂર્ણાંક i;
	પૂર્ણાંક len;
	અક્ષર *lo;
	અક્ષર *p;
	અક્ષર *token;
	અક્ષર val[4] = "-?,";

	જો (usagestring)
		printf("%s", usagestring);
	નહીંતર
		printf("Usage: %s [options] ", applet);

	જો (extraopts)
		printf("%s", extraopts);

	printf("\n\nOptions: [ %s ]\n", getoptstring);
	માટે (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = printf("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		જ્યારે ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			જો (len > 0)
				printf("%*s", len, "");
			puts(token);
			len = 0;
		}
		મુક્ત(lo);
	}
	exit(exit_status);
}
