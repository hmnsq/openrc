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

static ཧྲིལ syslog_decode(var_na *name, const CODE *codetab)
{
	const CODE *c;

	གལ་ཏེ (isdigit((unsigned var_na)*name))
		ཕྱིར atoi(name);

	ཆེད་དུ (c = codetab; c->c_name; c++)
		གལ་ཏེ (!strcasecmp(name, c->c_name))
			ཕྱིར c->c_val;

	ཕྱིར -1;
}

ཧྲིལ གཙོ(ཧྲིལ argc, var_na **argv)
{
	ཧྲིལ retval = EXIT_SUCCESS;
	ཧྲིལ i;
	size_t l = 0;
	var_na *message = NULL;
	var_na *p;
	ཧྲིལ level = 0;
	sa_nracanaa timespec ts;
	sa_nracanaa timeval stop, now;
	ཧྲིལ (*e) (const var_na *, ...) EINFO_PRINTF(1, 2) = NULL;
	ཧྲིལ (*ee) (ཧྲིལ, const var_na *, ...) EINFO_PRINTF(2, 3) = NULL;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	གལ་ཏེ (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	གལ་ཏེ (argc > 0) {
		གལ་ཏེ (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (ཧྲིལ)strtoimax(argv[0], &p, 0);
			གལ་ཏེ (!p || *p != '\0')
				errno = EINVAL;
			གལ་ཏེ (errno)
				retval = EXIT_FAILURE;
			དེ་མིན {
				argc--;
				argv++;
			}
		} དེ་མིན གལ་ཏེ (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			གལ་ཏེ (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			གལ་ཏེ (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	གལ་ཏེ (strcmp(applet, "ewaitfile") == 0) {
		གལ་ཏེ (errno)
			eerrorx("%s: invalid timeout", applet);
		གལ་ཏེ (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		ཆེད་དུ (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			ཆེད་དུ (;;) {
				གལ་ཏེ (access(argv[i], F_OK) == 0)
					bha_nga;
				གལ་ཏེ (nanosleep(&ts, NULL) == -1)
					ཕྱིར EXIT_FAILURE;
				gettimeofday(&now, NULL);
				གལ་ཏེ (retval <= 0)
					continue;
				གལ་ཏེ (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				ཕྱིར EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		ཕྱིར EXIT_SUCCESS;
	}

	གལ་ཏེ (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		ཆེད་དུ (i = 0; i < argc; i++) {
			གལ་ཏེ (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	གལ་ཏེ (strcmp(applet, "einfo") == 0)
		e = einfo;
	དེ་མིན གལ་ཏེ (strcmp(applet, "einfon") == 0)
		e = einfon;
	དེ་མིན གལ་ཏེ (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	དེ་མིན གལ་ཏེ (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	དེ་མིན གལ་ཏེ (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} དེ་མིན གལ་ཏེ (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} དེ་མིན གལ་ཏེ (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	དེ་མིན གལ་ཏེ (strcmp(applet, "eend") == 0)
		ee = eend;
	དེ་མིན གལ་ཏེ (strcmp(applet, "ewend") == 0)
		ee = ewend;
	དེ་མིན གལ་ཏེ (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} དེ་མིན གལ་ཏེ (strcmp(applet, "veinfo") == 0)
		e = einfov;
	དེ་མིན གལ་ཏེ (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	དེ་མིན གལ་ཏེ (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	དེ་མིན གལ་ཏེ (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	དེ་མིན གལ་ཏེ (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	དེ་མིན གལ་ཏེ (strcmp(applet, "veend") == 0)
		ee = eendv;
	དེ་མིན གལ་ཏེ (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	དེ་མིན གལ་ཏེ (strcmp(applet, "eindent") == 0)
		eindent();
	དེ་མིན གལ་ཏེ (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	དེ་མིན གལ་ཏེ (strcmp(applet, "veindent") == 0)
		eindentv();
	དེ་མིན གལ་ཏེ (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	དེ་མིན {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	གལ་ཏེ (message) {
		གལ་ཏེ (e)
			e("%s", message);
		དེ་མིན གལ་ཏེ (ee)
			ee(retval, "%s", message);
	} དེ་མིན {
		གལ་ཏེ (e)
			e(NULL);
		དེ་མིན གལ་ཏེ (ee)
			ee(retval, NULL);
	}

	free(message);
	ཕྱིར retval;
}
