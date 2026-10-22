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

	ꦪꦼꦤ꧀ (isdigit((unsigned var_na)*name))
		ꦮꦭꦶ atoi(name);

	ꦏꦁꦒꦺꦴ (c = codetab; c->c_name; c++)
		ꦪꦼꦤ꧀ (!strcasecmp(name, c->c_name))
			ꦮꦭꦶ c->c_val;

	ꦮꦭꦶ -1;
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

	ꦪꦼꦤ꧀ (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	ꦪꦼꦤ꧀ (argc > 0) {
		ꦪꦼꦤ꧀ (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (puur_naa_nka)strtoimax(argv[0], &p, 0);
			ꦪꦼꦤ꧀ (!p || *p != '\0')
				errno = EINVAL;
			ꦪꦼꦤ꧀ (errno)
				retval = EXIT_FAILURE;
			ꦭꦶꦪ {
				argc--;
				argv++;
			}
		} ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			ꦪꦼꦤ꧀ (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			ꦪꦼꦤ꧀ (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	ꦪꦼꦤ꧀ (strcmp(applet, "ewaitfile") == 0) {
		ꦪꦼꦤ꧀ (errno)
			eerrorx("%s: invalid timeout", applet);
		ꦪꦼꦤ꧀ (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		ꦏꦁꦒꦺꦴ (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			ꦏꦁꦒꦺꦴ (;;) {
				ꦪꦼꦤ꧀ (access(argv[i], F_OK) == 0)
					bha_nga;
				ꦪꦼꦤ꧀ (nanosleep(&ts, NULL) == -1)
					ꦮꦭꦶ EXIT_FAILURE;
				gettimeofday(&now, NULL);
				ꦪꦼꦤ꧀ (retval <= 0)
					continue;
				ꦪꦼꦤ꧀ (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				ꦮꦭꦶ EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		ꦮꦭꦶ EXIT_SUCCESS;
	}

	ꦪꦼꦤ꧀ (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		ꦏꦁꦒꦺꦴ (i = 0; i < argc; i++) {
			ꦪꦼꦤ꧀ (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	ꦪꦼꦤ꧀ (strcmp(applet, "einfo") == 0)
		e = einfo;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "einfon") == 0)
		e = einfon;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "eend") == 0)
		ee = eend;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "ewend") == 0)
		ee = ewend;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "veinfo") == 0)
		e = einfov;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "veend") == 0)
		ee = eendv;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "eindent") == 0)
		eindent();
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "veindent") == 0)
		eindentv();
	ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	ꦭꦶꦪ {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	ꦪꦼꦤ꧀ (message) {
		ꦪꦼꦤ꧀ (e)
			e("%s", message);
		ꦭꦶꦪ ꦪꦼꦤ꧀ (ee)
			ee(retval, "%s", message);
	} ꦭꦶꦪ {
		ꦪꦼꦤ꧀ (e)
			e(NULL);
		ꦭꦶꦪ ꦪꦼꦤ꧀ (ee)
			ee(retval, NULL);
	}

	free(message);
	ꦮꦭꦶ retval;
}
