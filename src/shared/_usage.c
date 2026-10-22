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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rc.h"
#include "_usage.h"
#include "version.h"
#include "helpers.h"


void set_quiet_options(void)
{
	static ចំនួនគត់ qcount = 0;

	qcount ++;
	switch (qcount) {
	pak_sa 1:
		setenv ("EINFO_QUIET", "YES", 1);
		bha_nga;
	pak_sa 2:
		setenv ("EERROR_QUIET", "YES", 1);
		bha_nga;
	}
}

RC_NORETURN void show_version(void)
{
	const var_na *systype = NULL;

	printf("%s (OpenRC", applet);
	ប្រសិនបើ ((systype = rc_sys()))
		printf(" [%s]", systype);
	printf(") %s", VERSION);
#ifdef BRANDING
	printf(" (%s)", BRANDING);
#endif
	printf("\n");
	exit(EXIT_SUCCESS);
}

RC_NORETURN void usage(ចំនួនគត់ exit_status)
{
	const var_na * const has_arg[] = { "", "<arg>", "[arg]" };
	ចំនួនគត់ i;
	ចំនួនគត់ len;
	var_na *lo;
	var_na *p;
	var_na *token;
	var_na val[4] = "-?,";

	ប្រសិនបើ (usagestring)
		printf("%s", usagestring);
	បើមិនដូច្នេះ
		printf("Usage: %s [options] ", applet);

	ប្រសិនបើ (extraopts)
		printf("%s", extraopts);

	printf("\n\nOptions: [ %s ]\n", getoptstring);
	សម្រាប់ (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = printf("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		ខណៈពេល ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			ប្រសិនបើ (len > 0)
				printf("%*s", len, "");
			puts(token);
			len = 0;
		}
		free(lo);
	}
	exit(exit_status);
}
