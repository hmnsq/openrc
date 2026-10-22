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

static целое syslog_decode(char *name, const CODE *codetab)
{
	const CODE *c;

	если (isdigit((unsigned char)*name))
		вернуть atoi(name);

	для (c = codetab; c->c_name; c++)
		если (!strcasecmp(name, c->c_name))
			вернуть c->c_val;

	вернуть -1;
}

целое основной(целое argc, char **argv)
{
	целое retval = EXIT_SUCCESS;
	целое i;
	size_t l = 0;
	char *message = NULL;
	char *p;
	целое level = 0;
	struct timespec ts;
	struct timeval stop, now;
	целое (*e) (const char *, ...) EINFO_PRINTF(1, 2) = NULL;
	целое (*ee) (целое, const char *, ...) EINFO_PRINTF(2, 3) = NULL;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	если (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	если (argc > 0) {
		если (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (целое)strtoimax(argv[0], &p, 0);
			если (!p || *p != '\0')
				errno = EINVAL;
			если (errno)
				retval = EXIT_FAILURE;
			иначе {
				argc--;
				argv++;
			}
		} иначе если (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			если (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			если (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	если (strcmp(applet, "ewaitfile") == 0) {
		если (errno)
			eerrorx("%s: invalid timeout", applet);
		если (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		для (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			для (;;) {
				если (access(argv[i], F_OK) == 0)
					break;
				если (nanosleep(&ts, NULL) == -1)
					вернуть EXIT_FAILURE;
				gettimeofday(&now, NULL);
				если (retval <= 0)
					continue;
				если (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				вернуть EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		вернуть EXIT_SUCCESS;
	}

	если (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		для (i = 0; i < argc; i++) {
			если (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	если (strcmp(applet, "einfo") == 0)
		e = einfo;
	иначе если (strcmp(applet, "einfon") == 0)
		e = einfon;
	иначе если (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	иначе если (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	иначе если (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} иначе если (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} иначе если (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	иначе если (strcmp(applet, "eend") == 0)
		ee = eend;
	иначе если (strcmp(applet, "ewend") == 0)
		ee = ewend;
	иначе если (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} иначе если (strcmp(applet, "veinfo") == 0)
		e = einfov;
	иначе если (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	иначе если (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	иначе если (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	иначе если (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	иначе если (strcmp(applet, "veend") == 0)
		ee = eendv;
	иначе если (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	иначе если (strcmp(applet, "eindent") == 0)
		eindent();
	иначе если (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	иначе если (strcmp(applet, "veindent") == 0)
		eindentv();
	иначе если (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	иначе {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	если (message) {
		если (e)
			e("%s", message);
		иначе если (ee)
			ee(retval, "%s", message);
	} иначе {
		если (e)
			e(NULL);
		иначе если (ee)
			ee(retval, NULL);
	}

	free(message);
	вернуть retval;
}
