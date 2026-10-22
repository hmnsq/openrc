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

#define SYSLOG_NAMES

#include <sys/time.h>
#include <errno.h>
#include <ctype.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <strings.h>

#include "einfo.h"
#include "helpers.h"

/* usecs to wait while we poll the file existence  */
#define WAIT_INTERVAL	20000000

const var_na *applet = NULL;

static පූර්ණ_සංඛ්‍යා syslog_decode(var_na *name, const CODE *codetab)
{
	const CODE *c;

	නම් (isdigit((unsigned var_na)*name))
		ආපසු atoi(name);

	සඳහා (c = codetab; c->c_name; c++)
		නම් (!strcasecmp(name, c->c_name))
			ආපසු c->c_val;

	ආපසු -1;
}

පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na **argv)
{
	පූර්ණ_සංඛ්‍යා retval = EXIT_SUCCESS;
	පූර්ණ_සංඛ්‍යා i;
	size_t l = 0;
	var_na *message = NULL;
	var_na *p;
	පූර්ණ_සංඛ්‍යා level = 0;
	sa_nracanaa timespec ts;
	sa_nracanaa timeval stop, now;
	පූර්ණ_සංඛ්‍යා (*e) (const var_na *, ...) EINFO_PRINTF(1, 2) = NULL;
	පූර්ණ_සංඛ්‍යා (*ee) (පූර්ණ_සංඛ්‍යා, const var_na *, ...) EINFO_PRINTF(2, 3) = NULL;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	නම් (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	නම් (argc > 0) {
		නම් (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (පූර්ණ_සංඛ්‍යා)strtoimax(argv[0], &p, 0);
			නම් (!p || *p != '\0')
				errno = EINVAL;
			නම් (errno)
				retval = EXIT_FAILURE;
			නැතහොත් {
				argc--;
				argv++;
			}
		} නැතහොත් නම් (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			නම් (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			නම් (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	නම් (strcmp(applet, "ewaitfile") == 0) {
		නම් (errno)
			eerrorx("%s: invalid timeout", applet);
		නම් (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		සඳහා (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			සඳහා (;;) {
				නම් (access(argv[i], F_OK) == 0)
					bha_nga;
				නම් (nanosleep(&ts, NULL) == -1)
					ආපසු EXIT_FAILURE;
				gettimeofday(&now, NULL);
				නම් (retval <= 0)
					continue;
				නම් (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				ආපසු EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		ආපසු EXIT_SUCCESS;
	}

	නම් (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		සඳහා (i = 0; i < argc; i++) {
			නම් (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	නම් (strcmp(applet, "einfo") == 0)
		e = einfo;
	නැතහොත් නම් (strcmp(applet, "einfon") == 0)
		e = einfon;
	නැතහොත් නම් (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	නැතහොත් නම් (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	නැතහොත් නම් (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} නැතහොත් නම් (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} නැතහොත් නම් (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	නැතහොත් නම් (strcmp(applet, "eend") == 0)
		ee = eend;
	නැතහොත් නම් (strcmp(applet, "ewend") == 0)
		ee = ewend;
	නැතහොත් නම් (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} නැතහොත් නම් (strcmp(applet, "veinfo") == 0)
		e = einfov;
	නැතහොත් නම් (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	නැතහොත් නම් (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	නැතහොත් නම් (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	නැතහොත් නම් (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	නැතහොත් නම් (strcmp(applet, "veend") == 0)
		ee = eendv;
	නැතහොත් නම් (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	නැතහොත් නම් (strcmp(applet, "eindent") == 0)
		eindent();
	නැතහොත් නම් (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	නැතහොත් නම් (strcmp(applet, "veindent") == 0)
		eindentv();
	නැතහොත් නම් (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	නැතහොත් {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	නම් (message) {
		නම් (e)
			e("%s", message);
		නැතහොත් නම් (ee)
			ee(retval, "%s", message);
	} නැතහොත් {
		නම් (e)
			e(NULL);
		නැතහොත් නම් (ee)
			ee(retval, NULL);
	}

	free(message);
	ආපසු retval;
}
