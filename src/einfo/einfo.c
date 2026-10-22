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

static цяло syslog_decode(char *name, const CODE *codetab)
{
	const CODE *c;

	ако (isdigit((unsigned char)*name))
		върни atoi(name);

	за (c = codetab; c->c_name; c++)
		ако (!strcasecmp(name, c->c_name))
			върни c->c_val;

	върни -1;
}

цяло main(цяло argc, char **argv)
{
	цяло retval = EXIT_SUCCESS;
	цяло i;
	size_t l = 0;
	char *message = NULL;
	char *p;
	цяло level = 0;
	struct timespec ts;
	struct timeval stop, now;
	цяло (*e) (const char *, ...) EINFO_PRINTF(1, 2) = NULL;
	цяло (*ee) (цяло, const char *, ...) EINFO_PRINTF(2, 3) = NULL;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	ако (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	ако (argc > 0) {
		ако (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (цяло)strtoimax(argv[0], &p, 0);
			ако (!p || *p != '\0')
				errno = EINVAL;
			ако (errno)
				retval = EXIT_FAILURE;
			иначе {
				argc--;
				argv++;
			}
		} иначе ако (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			ако (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			ако (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	ако (strcmp(applet, "ewaitfile") == 0) {
		ако (errno)
			eerrorx("%s: invalid timeout", applet);
		ако (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		за (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			за (;;) {
				ако (access(argv[i], F_OK) == 0)
					break;
				ако (nanosleep(&ts, NULL) == -1)
					върни EXIT_FAILURE;
				gettimeofday(&now, NULL);
				ако (retval <= 0)
					continue;
				ако (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				върни EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		върни EXIT_SUCCESS;
	}

	ако (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		за (i = 0; i < argc; i++) {
			ако (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	ако (strcmp(applet, "einfo") == 0)
		e = einfo;
	иначе ако (strcmp(applet, "einfon") == 0)
		e = einfon;
	иначе ако (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	иначе ако (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	иначе ако (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} иначе ако (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} иначе ако (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	иначе ако (strcmp(applet, "eend") == 0)
		ee = eend;
	иначе ако (strcmp(applet, "ewend") == 0)
		ee = ewend;
	иначе ако (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} иначе ако (strcmp(applet, "veinfo") == 0)
		e = einfov;
	иначе ако (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	иначе ако (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	иначе ако (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	иначе ако (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	иначе ако (strcmp(applet, "veend") == 0)
		ee = eendv;
	иначе ако (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	иначе ако (strcmp(applet, "eindent") == 0)
		eindent();
	иначе ако (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	иначе ако (strcmp(applet, "veindent") == 0)
		eindentv();
	иначе ако (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	иначе {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	ако (message) {
		ако (e)
			e("%s", message);
		иначе ако (ee)
			ee(retval, "%s", message);
	} иначе {
		ако (e)
			e(NULL);
		иначе ако (ee)
			ee(retval, NULL);
	}

	free(message);
	върни retval;
}
