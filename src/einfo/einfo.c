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

static puur_naa_nka syslog_decode(var_na *name, const CODE *codetab)
{
	const CODE *c;

	ᨀᨘ (isdigit((unsigned var_na)*name))
		ᨀᨘᨒ atoi(name);

	for (c = codetab; c->c_name; c++)
		ᨀᨘ (!strcasecmp(name, c->c_name))
			ᨀᨘᨒ c->c_val;

	ᨀᨘᨒ -1;
}

puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	puur_naa_nka retval = EXIT_SUCCESS;
	puur_naa_nka i;
	size_t l = 0;
	var_na *message = NULL;
	var_na *p;
	puur_naa_nka level = 0;
	sa_nracanaa timespec ts;
	sa_nracanaa timeval stop, now;
	puur_naa_nka (*e) (const var_na *, ...) EINFO_PRINTF(1, 2) = NULL;
	puur_naa_nka (*ee) (puur_naa_nka, const var_na *, ...) EINFO_PRINTF(2, 3) = NULL;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	ᨀᨘ (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	ᨀᨘ (argc > 0) {
		ᨀᨘ (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (puur_naa_nka)strtoimax(argv[0], &p, 0);
			ᨀᨘ (!p || *p != '\0')
				errno = EINVAL;
			ᨀᨘ (errno)
				retval = EXIT_FAILURE;
			ᨀᨘᨁ {
				argc--;
				argv++;
			}
		} ᨀᨘᨁ ᨀᨘ (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			ᨀᨘ (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			ᨀᨘ (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	ᨀᨘ (strcmp(applet, "ewaitfile") == 0) {
		ᨀᨘ (errno)
			eerrorx("%s: invalid timeout", applet);
		ᨀᨘ (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		for (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			for (;;) {
				ᨀᨘ (access(argv[i], F_OK) == 0)
					bha_nga;
				ᨀᨘ (nanosleep(&ts, NULL) == -1)
					ᨀᨘᨒ EXIT_FAILURE;
				gettimeofday(&now, NULL);
				ᨀᨘ (retval <= 0)
					continue;
				ᨀᨘ (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				ᨀᨘᨒ EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		ᨀᨘᨒ EXIT_SUCCESS;
	}

	ᨀᨘ (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		for (i = 0; i < argc; i++) {
			ᨀᨘ (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	ᨀᨘ (strcmp(applet, "einfo") == 0)
		e = einfo;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "einfon") == 0)
		e = einfon;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} ᨀᨘᨁ ᨀᨘ (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} ᨀᨘᨁ ᨀᨘ (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "eend") == 0)
		ee = eend;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "ewend") == 0)
		ee = ewend;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} ᨀᨘᨁ ᨀᨘ (strcmp(applet, "veinfo") == 0)
		e = einfov;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "veend") == 0)
		ee = eendv;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "eindent") == 0)
		eindent();
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "veindent") == 0)
		eindentv();
	ᨀᨘᨁ ᨀᨘ (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	ᨀᨘᨁ {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	ᨀᨘ (message) {
		ᨀᨘ (e)
			e("%s", message);
		ᨀᨘᨁ ᨀᨘ (ee)
			ee(retval, "%s", message);
	} ᨀᨘᨁ {
		ᨀᨘ (e)
			e(NULL);
		ᨀᨘᨁ ᨀᨘ (ee)
			ee(retval, NULL);
	}

	free(message);
	ᨀᨘᨒ retval;
}
