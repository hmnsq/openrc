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

static 정수 syslog_decode(char *name, const CODE *codetab)
{
	const CODE *c;

	만약 (isdigit((unsigned char)*name))
		반환 atoi(name);

	위해 (c = codetab; c->c_name; c++)
		만약 (!strcasecmp(name, c->c_name))
			반환 c->c_val;

	반환 -1;
}

정수 main(정수 argc, char **argv)
{
	정수 retval = EXIT_SUCCESS;
	정수 i;
	size_t l = 0;
	char *message = NULL;
	char *p;
	정수 level = 0;
	struct timespec ts;
	struct timeval stop, now;
	정수 (*e) (const char *, ...) EINFO_PRINTF(1, 2) = NULL;
	정수 (*ee) (정수, const char *, ...) EINFO_PRINTF(2, 3) = NULL;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	만약 (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	만약 (argc > 0) {
		만약 (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (정수)strtoimax(argv[0], &p, 0);
			만약 (!p || *p != '\0')
				errno = EINVAL;
			만약 (errno)
				retval = EXIT_FAILURE;
			아니면 {
				argc--;
				argv++;
			}
		} 아니면 만약 (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			만약 (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			만약 (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	만약 (strcmp(applet, "ewaitfile") == 0) {
		만약 (errno)
			eerrorx("%s: invalid timeout", applet);
		만약 (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		위해 (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			위해 (;;) {
				만약 (access(argv[i], F_OK) == 0)
					break;
				만약 (nanosleep(&ts, NULL) == -1)
					반환 EXIT_FAILURE;
				gettimeofday(&now, NULL);
				만약 (retval <= 0)
					continue;
				만약 (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				반환 EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		반환 EXIT_SUCCESS;
	}

	만약 (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		위해 (i = 0; i < argc; i++) {
			만약 (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	만약 (strcmp(applet, "einfo") == 0)
		e = einfo;
	아니면 만약 (strcmp(applet, "einfon") == 0)
		e = einfon;
	아니면 만약 (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	아니면 만약 (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	아니면 만약 (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} 아니면 만약 (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} 아니면 만약 (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	아니면 만약 (strcmp(applet, "eend") == 0)
		ee = eend;
	아니면 만약 (strcmp(applet, "ewend") == 0)
		ee = ewend;
	아니면 만약 (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} 아니면 만약 (strcmp(applet, "veinfo") == 0)
		e = einfov;
	아니면 만약 (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	아니면 만약 (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	아니면 만약 (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	아니면 만약 (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	아니면 만약 (strcmp(applet, "veend") == 0)
		ee = eendv;
	아니면 만약 (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	아니면 만약 (strcmp(applet, "eindent") == 0)
		eindent();
	아니면 만약 (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	아니면 만약 (strcmp(applet, "veindent") == 0)
		eindentv();
	아니면 만약 (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	아니면 {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	만약 (message) {
		만약 (e)
			e("%s", message);
		아니면 만약 (ee)
			ee(retval, "%s", message);
	} 아니면 {
		만약 (e)
			e(NULL);
		아니면 만약 (ee)
			ee(retval, NULL);
	}

	free(message);
	반환 retval;
}
