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

static 整数 syslog_decode(char *name, const CODE *codetab)
{
	const CODE *c;

	もし (isdigit((unsigned char)*name))
		戻る atoi(name);

	ために (c = codetab; c->c_name; c++)
		もし (!strcasecmp(name, c->c_name))
			戻る c->c_val;

	戻る -1;
}

整数 メインページ(整数 argc, char **argv)
{
	整数 retval = EXIT_SUCCESS;
	整数 i;
	size_t l = 0;
	char *message = NULL;
	char *p;
	整数 level = 0;
	struct timespec ts;
	struct timeval stop, now;
	整数 (*e) (const char *, ...) EINFO_PRINTF(1, 2) = NULL;
	整数 (*ee) (整数, const char *, ...) EINFO_PRINTF(2, 3) = NULL;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	もし (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	もし (argc > 0) {
		もし (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (整数)strtoimax(argv[0], &p, 0);
			もし (!p || *p != '\0')
				errno = EINVAL;
			もし (errno)
				retval = EXIT_FAILURE;
			否则 {
				argc--;
				argv++;
			}
		} 否则 もし (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			もし (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			もし (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	もし (strcmp(applet, "ewaitfile") == 0) {
		もし (errno)
			eerrorx("%s: invalid timeout", applet);
		もし (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		ために (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			ために (;;) {
				もし (access(argv[i], F_OK) == 0)
					break;
				もし (nanosleep(&ts, NULL) == -1)
					戻る EXIT_FAILURE;
				gettimeofday(&now, NULL);
				もし (retval <= 0)
					continue;
				もし (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				戻る EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		戻る EXIT_SUCCESS;
	}

	もし (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		ために (i = 0; i < argc; i++) {
			もし (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	もし (strcmp(applet, "einfo") == 0)
		e = einfo;
	否则 もし (strcmp(applet, "einfon") == 0)
		e = einfon;
	否则 もし (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	否则 もし (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	否则 もし (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} 否则 もし (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} 否则 もし (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	否则 もし (strcmp(applet, "eend") == 0)
		ee = eend;
	否则 もし (strcmp(applet, "ewend") == 0)
		ee = ewend;
	否则 もし (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} 否则 もし (strcmp(applet, "veinfo") == 0)
		e = einfov;
	否则 もし (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	否则 もし (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	否则 もし (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	否则 もし (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	否则 もし (strcmp(applet, "veend") == 0)
		ee = eendv;
	否则 もし (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	否则 もし (strcmp(applet, "eindent") == 0)
		eindent();
	否则 もし (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	否则 もし (strcmp(applet, "veindent") == 0)
		eindentv();
	否则 もし (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	否则 {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	もし (message) {
		もし (e)
			e("%s", message);
		否则 もし (ee)
			ee(retval, "%s", message);
	} 否则 {
		もし (e)
			e(NULL);
		否则 もし (ee)
			ee(retval, NULL);
	}

	free(message);
	戻る retval;
}
