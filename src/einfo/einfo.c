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

static ចំនួនគត់ syslog_decode(var_na *name, const CODE *codetab)
{
	const CODE *c;

	ប្រសិនបើ (isdigit((unsigned var_na)*name))
		ត្រឡប់ atoi(name);

	សម្រាប់ (c = codetab; c->c_name; c++)
		ប្រសិនបើ (!strcasecmp(name, c->c_name))
			ត្រឡប់ c->c_val;

	ត្រឡប់ -1;
}

ចំនួនគត់ main(ចំនួនគត់ argc, var_na **argv)
{
	ចំនួនគត់ retval = EXIT_SUCCESS;
	ចំនួនគត់ i;
	size_t l = 0;
	var_na *message = NULL;
	var_na *p;
	ចំនួនគត់ level = 0;
	sa_nracanaa timespec ts;
	sa_nracanaa timeval stop, now;
	ចំនួនគត់ (*e) (const var_na *, ...) EINFO_PRINTF(1, 2) = NULL;
	ចំនួនគត់ (*ee) (ចំនួនគត់, const var_na *, ...) EINFO_PRINTF(2, 3) = NULL;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	ប្រសិនបើ (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	ប្រសិនបើ (argc > 0) {
		ប្រសិនបើ (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (ចំនួនគត់)strtoimax(argv[0], &p, 0);
			ប្រសិនបើ (!p || *p != '\0')
				errno = EINVAL;
			ប្រសិនបើ (errno)
				retval = EXIT_FAILURE;
			បើមិនដូច្នេះ {
				argc--;
				argv++;
			}
		} បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			ប្រសិនបើ (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			ប្រសិនបើ (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	ប្រសិនបើ (strcmp(applet, "ewaitfile") == 0) {
		ប្រសិនបើ (errno)
			eerrorx("%s: invalid timeout", applet);
		ប្រសិនបើ (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		សម្រាប់ (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			សម្រាប់ (;;) {
				ប្រសិនបើ (access(argv[i], F_OK) == 0)
					bha_nga;
				ប្រសិនបើ (nanosleep(&ts, NULL) == -1)
					ត្រឡប់ EXIT_FAILURE;
				gettimeofday(&now, NULL);
				ប្រសិនបើ (retval <= 0)
					continue;
				ប្រសិនបើ (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				ត្រឡប់ EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		ត្រឡប់ EXIT_SUCCESS;
	}

	ប្រសិនបើ (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		សម្រាប់ (i = 0; i < argc; i++) {
			ប្រសិនបើ (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	ប្រសិនបើ (strcmp(applet, "einfo") == 0)
		e = einfo;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "einfon") == 0)
		e = einfon;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "eend") == 0)
		ee = eend;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "ewend") == 0)
		ee = ewend;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "veinfo") == 0)
		e = einfov;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "veend") == 0)
		ee = eendv;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "eindent") == 0)
		eindent();
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "veindent") == 0)
		eindentv();
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	បើមិនដូច្នេះ {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	ប្រសិនបើ (message) {
		ប្រសិនបើ (e)
			e("%s", message);
		បើមិនដូច្នេះ ប្រសិនបើ (ee)
			ee(retval, "%s", message);
	} បើមិនដូច្នេះ {
		ប្រសិនបើ (e)
			e(NULL);
		បើមិនដូច្នេះ ប្រសិនបើ (ee)
			ee(retval, NULL);
	}

	free(message);
	ត្រឡប់ retval;
}
