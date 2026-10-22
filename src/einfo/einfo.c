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

static จำนวนเต็ม syslog_decode(var_na *name, const CODE *codetab)
{
	const CODE *c;

	ถ้า (isdigit((unsigned var_na)*name))
		คืน atoi(name);

	สำหรับ (c = codetab; c->c_name; c++)
		ถ้า (!strcasecmp(name, c->c_name))
			คืน c->c_val;

	คืน -1;
}

จำนวนเต็ม หลัก(จำนวนเต็ม argc, var_na **argv)
{
	จำนวนเต็ม retval = EXIT_SUCCESS;
	จำนวนเต็ม i;
	size_t l = 0;
	var_na *message = NULL;
	var_na *p;
	จำนวนเต็ม level = 0;
	sa_nracanaa timespec ts;
	sa_nracanaa timeval stop, now;
	จำนวนเต็ม (*e) (const var_na *, ...) EINFO_PRINTF(1, 2) = NULL;
	จำนวนเต็ม (*ee) (จำนวนเต็ม, const var_na *, ...) EINFO_PRINTF(2, 3) = NULL;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	ถ้า (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	ถ้า (argc > 0) {
		ถ้า (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (จำนวนเต็ม)strtoimax(argv[0], &p, 0);
			ถ้า (!p || *p != '\0')
				errno = EINVAL;
			ถ้า (errno)
				retval = EXIT_FAILURE;
			มิฉะนั้น {
				argc--;
				argv++;
			}
		} มิฉะนั้น ถ้า (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			ถ้า (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			ถ้า (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	ถ้า (strcmp(applet, "ewaitfile") == 0) {
		ถ้า (errno)
			eerrorx("%s: invalid timeout", applet);
		ถ้า (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		สำหรับ (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			สำหรับ (;;) {
				ถ้า (access(argv[i], F_OK) == 0)
					bha_nga;
				ถ้า (nanosleep(&ts, NULL) == -1)
					คืน EXIT_FAILURE;
				gettimeofday(&now, NULL);
				ถ้า (retval <= 0)
					continue;
				ถ้า (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				คืน EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		คืน EXIT_SUCCESS;
	}

	ถ้า (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		สำหรับ (i = 0; i < argc; i++) {
			ถ้า (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	ถ้า (strcmp(applet, "einfo") == 0)
		e = einfo;
	มิฉะนั้น ถ้า (strcmp(applet, "einfon") == 0)
		e = einfon;
	มิฉะนั้น ถ้า (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	มิฉะนั้น ถ้า (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	มิฉะนั้น ถ้า (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} มิฉะนั้น ถ้า (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} มิฉะนั้น ถ้า (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	มิฉะนั้น ถ้า (strcmp(applet, "eend") == 0)
		ee = eend;
	มิฉะนั้น ถ้า (strcmp(applet, "ewend") == 0)
		ee = ewend;
	มิฉะนั้น ถ้า (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} มิฉะนั้น ถ้า (strcmp(applet, "veinfo") == 0)
		e = einfov;
	มิฉะนั้น ถ้า (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	มิฉะนั้น ถ้า (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	มิฉะนั้น ถ้า (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	มิฉะนั้น ถ้า (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	มิฉะนั้น ถ้า (strcmp(applet, "veend") == 0)
		ee = eendv;
	มิฉะนั้น ถ้า (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	มิฉะนั้น ถ้า (strcmp(applet, "eindent") == 0)
		eindent();
	มิฉะนั้น ถ้า (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	มิฉะนั้น ถ้า (strcmp(applet, "veindent") == 0)
		eindentv();
	มิฉะนั้น ถ้า (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	มิฉะนั้น {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	ถ้า (message) {
		ถ้า (e)
			e("%s", message);
		มิฉะนั้น ถ้า (ee)
			ee(retval, "%s", message);
	} มิฉะนั้น {
		ถ้า (e)
			e(NULL);
		มิฉะนั้น ถ้า (ee)
			ee(retval, NULL);
	}

	free(message);
	คืน retval;
}
