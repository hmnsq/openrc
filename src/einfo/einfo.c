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

const char *applet = NULL;

static số_nguyên syslog_decode(char *name, const CODE *codetab)
{
	const CODE *c;

	nếu (isdigit((unsigned char)*name))
		trả_về atoi(name);

	cho (c = codetab; c->c_name; c++)
		nếu (!strcasecmp(name, c->c_name))
			trả_về c->c_val;

	trả_về -1;
}

số_nguyên main(số_nguyên argc, char **argv)
{
	số_nguyên retval = EXIT_SUCCESS;
	số_nguyên i;
	size_t l = 0;
	char *message = NULL;
	char *p;
	số_nguyên level = 0;
	struct timespec ts;
	struct timeval stop, now;
	số_nguyên (*e) (const char *, ...) EINFO_PRINTF(1, 2) = NULL;
	số_nguyên (*ee) (số_nguyên, const char *, ...) EINFO_PRINTF(2, 3) = NULL;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	nếu (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	nếu (argc > 0) {
		nếu (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (số_nguyên)strtoimax(argv[0], &p, 0);
			nếu (!p || *p != '\0')
				errno = EINVAL;
			nếu (errno)
				retval = EXIT_FAILURE;
			không_thì {
				argc--;
				argv++;
			}
		} không_thì nếu (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			nếu (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			nếu (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	nếu (strcmp(applet, "ewaitfile") == 0) {
		nếu (errno)
			eerrorx("%s: invalid timeout", applet);
		nếu (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		cho (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			cho (;;) {
				nếu (access(argv[i], F_OK) == 0)
					break;
				nếu (nanosleep(&ts, NULL) == -1)
					trả_về EXIT_FAILURE;
				gettimeofday(&now, NULL);
				nếu (retval <= 0)
					continue;
				nếu (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				trả_về EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		trả_về EXIT_SUCCESS;
	}

	nếu (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		cho (i = 0; i < argc; i++) {
			nếu (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	nếu (strcmp(applet, "einfo") == 0)
		e = einfo;
	không_thì nếu (strcmp(applet, "einfon") == 0)
		e = einfon;
	không_thì nếu (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	không_thì nếu (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	không_thì nếu (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} không_thì nếu (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} không_thì nếu (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	không_thì nếu (strcmp(applet, "eend") == 0)
		ee = eend;
	không_thì nếu (strcmp(applet, "ewend") == 0)
		ee = ewend;
	không_thì nếu (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} không_thì nếu (strcmp(applet, "veinfo") == 0)
		e = einfov;
	không_thì nếu (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	không_thì nếu (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	không_thì nếu (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	không_thì nếu (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	không_thì nếu (strcmp(applet, "veend") == 0)
		ee = eendv;
	không_thì nếu (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	không_thì nếu (strcmp(applet, "eindent") == 0)
		eindent();
	không_thì nếu (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	không_thì nếu (strcmp(applet, "veindent") == 0)
		eindentv();
	không_thì nếu (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	không_thì {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	nếu (message) {
		nếu (e)
			e("%s", message);
		không_thì nếu (ee)
			ee(retval, "%s", message);
	} không_thì {
		nếu (e)
			e(NULL);
		không_thì nếu (ee)
			ee(retval, NULL);
	}

	free(message);
	trả_về retval;
}
