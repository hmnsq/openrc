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

	ᯀᯮᯝ (isdigit((unsigned var_na)*name))
		ᯀᯮᯞ atoi(name);

	for (c = codetab; c->c_name; c++)
		ᯀᯮᯝ (!strcasecmp(name, c->c_name))
			ᯀᯮᯞ c->c_val;

	ᯀᯮᯞ -1;
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

	ᯀᯮᯝ (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	ᯀᯮᯝ (argc > 0) {
		ᯀᯮᯝ (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (puur_naa_nka)strtoimax(argv[0], &p, 0);
			ᯀᯮᯝ (!p || *p != '\0')
				errno = EINVAL;
			ᯀᯮᯝ (errno)
				retval = EXIT_FAILURE;
			ᯀᯮᯝᯂᯪ {
				argc--;
				argv++;
			}
		} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			ᯀᯮᯝ (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			ᯀᯮᯝ (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	ᯀᯮᯝ (strcmp(applet, "ewaitfile") == 0) {
		ᯀᯮᯝ (errno)
			eerrorx("%s: invalid timeout", applet);
		ᯀᯮᯝ (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		for (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			for (;;) {
				ᯀᯮᯝ (access(argv[i], F_OK) == 0)
					bha_nga;
				ᯀᯮᯝ (nanosleep(&ts, NULL) == -1)
					ᯀᯮᯞ EXIT_FAILURE;
				gettimeofday(&now, NULL);
				ᯀᯮᯝ (retval <= 0)
					continue;
				ᯀᯮᯝ (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				ᯀᯮᯞ EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		ᯀᯮᯞ EXIT_SUCCESS;
	}

	ᯀᯮᯝ (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		for (i = 0; i < argc; i++) {
			ᯀᯮᯝ (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	ᯀᯮᯝ (strcmp(applet, "einfo") == 0)
		e = einfo;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "einfon") == 0)
		e = einfon;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "eend") == 0)
		ee = eend;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "ewend") == 0)
		ee = ewend;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "veinfo") == 0)
		e = einfov;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "veend") == 0)
		ee = eendv;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "eindent") == 0)
		eindent();
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "veindent") == 0)
		eindentv();
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	ᯀᯮᯝᯂᯪ {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	ᯀᯮᯝ (message) {
		ᯀᯮᯝ (e)
			e("%s", message);
		ᯀᯮᯝᯂᯪ ᯀᯮᯝ (ee)
			ee(retval, "%s", message);
	} ᯀᯮᯝᯂᯪ {
		ᯀᯮᯝ (e)
			e(NULL);
		ᯀᯮᯝᯂᯪ ᯀᯮᯝ (ee)
			ee(retval, NULL);
	}

	free(message);
	ᯀᯮᯞ retval;
}
