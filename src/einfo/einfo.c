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

	ᮚᮦᮔ᮪ (isdigit((unsigned var_na)*name))
		ᮝᮜᮤ atoi(name);

	for (c = codetab; c->c_name; c++)
		ᮚᮦᮔ᮪ (!strcasecmp(name, c->c_name))
			ᮝᮜᮤ c->c_val;

	ᮝᮜᮤ -1;
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

	ᮚᮦᮔ᮪ (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	ᮚᮦᮔ᮪ (argc > 0) {
		ᮚᮦᮔ᮪ (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (puur_naa_nka)strtoimax(argv[0], &p, 0);
			ᮚᮦᮔ᮪ (!p || *p != '\0')
				errno = EINVAL;
			ᮚᮦᮔ᮪ (errno)
				retval = EXIT_FAILURE;
			ᮜᮤᮚ {
				argc--;
				argv++;
			}
		} ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			ᮚᮦᮔ᮪ (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			ᮚᮦᮔ᮪ (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	ᮚᮦᮔ᮪ (strcmp(applet, "ewaitfile") == 0) {
		ᮚᮦᮔ᮪ (errno)
			eerrorx("%s: invalid timeout", applet);
		ᮚᮦᮔ᮪ (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		for (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			for (;;) {
				ᮚᮦᮔ᮪ (access(argv[i], F_OK) == 0)
					bha_nga;
				ᮚᮦᮔ᮪ (nanosleep(&ts, NULL) == -1)
					ᮝᮜᮤ EXIT_FAILURE;
				gettimeofday(&now, NULL);
				ᮚᮦᮔ᮪ (retval <= 0)
					continue;
				ᮚᮦᮔ᮪ (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				ᮝᮜᮤ EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		ᮝᮜᮤ EXIT_SUCCESS;
	}

	ᮚᮦᮔ᮪ (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		for (i = 0; i < argc; i++) {
			ᮚᮦᮔ᮪ (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	ᮚᮦᮔ᮪ (strcmp(applet, "einfo") == 0)
		e = einfo;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "einfon") == 0)
		e = einfon;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "eend") == 0)
		ee = eend;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "ewend") == 0)
		ee = ewend;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "veinfo") == 0)
		e = einfov;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "veend") == 0)
		ee = eendv;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "eindent") == 0)
		eindent();
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "veindent") == 0)
		eindentv();
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	ᮜᮤᮚ {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	ᮚᮦᮔ᮪ (message) {
		ᮚᮦᮔ᮪ (e)
			e("%s", message);
		ᮜᮤᮚ ᮚᮦᮔ᮪ (ee)
			ee(retval, "%s", message);
	} ᮜᮤᮚ {
		ᮚᮦᮔ᮪ (e)
			e(NULL);
		ᮜᮤᮚ ᮚᮦᮔ᮪ (ee)
			ee(retval, NULL);
	}

	free(message);
	ᮝᮜᮤ retval;
}
