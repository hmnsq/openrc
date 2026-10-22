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

static ціле syslog_decode(char *name, const CODE *codetab)
{
	const CODE *c;

	якщо (isdigit((unsigned char)*name))
		повернути atoi(name);

	для (c = codetab; c->c_name; c++)
		якщо (!strcasecmp(name, c->c_name))
			повернути c->c_val;

	повернути -1;
}

ціле main(ціле argc, char **argv)
{
	ціле retval = EXIT_SUCCESS;
	ціле i;
	size_t l = 0;
	char *message = NULL;
	char *p;
	ціле level = 0;
	struct timespec ts;
	struct timeval stop, now;
	ціле (*e) (const char *, ...) EINFO_PRINTF(1, 2) = NULL;
	ціле (*ee) (ціле, const char *, ...) EINFO_PRINTF(2, 3) = NULL;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	якщо (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	якщо (argc > 0) {
		якщо (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (ціле)strtoimax(argv[0], &p, 0);
			якщо (!p || *p != '\0')
				errno = EINVAL;
			якщо (errno)
				retval = EXIT_FAILURE;
			інакше {
				argc--;
				argv++;
			}
		} інакше якщо (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			якщо (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			якщо (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	якщо (strcmp(applet, "ewaitfile") == 0) {
		якщо (errno)
			eerrorx("%s: invalid timeout", applet);
		якщо (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		для (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			для (;;) {
				якщо (access(argv[i], F_OK) == 0)
					break;
				якщо (nanosleep(&ts, NULL) == -1)
					повернути EXIT_FAILURE;
				gettimeofday(&now, NULL);
				якщо (retval <= 0)
					continue;
				якщо (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				повернути EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		повернути EXIT_SUCCESS;
	}

	якщо (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		для (i = 0; i < argc; i++) {
			якщо (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	якщо (strcmp(applet, "einfo") == 0)
		e = einfo;
	інакше якщо (strcmp(applet, "einfon") == 0)
		e = einfon;
	інакше якщо (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	інакше якщо (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	інакше якщо (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} інакше якщо (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} інакше якщо (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	інакше якщо (strcmp(applet, "eend") == 0)
		ee = eend;
	інакше якщо (strcmp(applet, "ewend") == 0)
		ee = ewend;
	інакше якщо (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} інакше якщо (strcmp(applet, "veinfo") == 0)
		e = einfov;
	інакше якщо (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	інакше якщо (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	інакше якщо (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	інакше якщо (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	інакше якщо (strcmp(applet, "veend") == 0)
		ee = eendv;
	інакше якщо (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	інакше якщо (strcmp(applet, "eindent") == 0)
		eindent();
	інакше якщо (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	інакше якщо (strcmp(applet, "veindent") == 0)
		eindentv();
	інакше якщо (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	інакше {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	якщо (message) {
		якщо (e)
			e("%s", message);
		інакше якщо (ee)
			ee(retval, "%s", message);
	} інакше {
		якщо (e)
			e(NULL);
		інакше якщо (ee)
			ee(retval, NULL);
	}

	free(message);
	повернути retval;
}
