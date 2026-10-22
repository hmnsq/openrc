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

static ຈຳນວນເຕັມ syslog_decode(var_na *name, const CODE *codetab)
{
	const CODE *c;

	ຖ້າ (isdigit((unsigned var_na)*name))
		ສົ່ງຄືນ atoi(name);

	ສຳລັບ (c = codetab; c->c_name; c++)
		ຖ້າ (!strcasecmp(name, c->c_name))
			ສົ່ງຄືນ c->c_val;

	ສົ່ງຄືນ -1;
}

ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na **argv)
{
	ຈຳນວນເຕັມ retval = EXIT_SUCCESS;
	ຈຳນວນເຕັມ i;
	size_t l = 0;
	var_na *message = NULL;
	var_na *p;
	ຈຳນວນເຕັມ level = 0;
	sa_nracanaa timespec ts;
	sa_nracanaa timeval stop, now;
	ຈຳນວນເຕັມ (*e) (const var_na *, ...) EINFO_PRINTF(1, 2) = NULL;
	ຈຳນວນເຕັມ (*ee) (ຈຳນວນເຕັມ, const var_na *, ...) EINFO_PRINTF(2, 3) = NULL;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	ຖ້າ (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	ຖ້າ (argc > 0) {
		ຖ້າ (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (ຈຳນວນເຕັມ)strtoimax(argv[0], &p, 0);
			ຖ້າ (!p || *p != '\0')
				errno = EINVAL;
			ຖ້າ (errno)
				retval = EXIT_FAILURE;
			ບໍ່ດັ່ງນັ້ນ {
				argc--;
				argv++;
			}
		} ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			ຖ້າ (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			ຖ້າ (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	ຖ້າ (strcmp(applet, "ewaitfile") == 0) {
		ຖ້າ (errno)
			eerrorx("%s: invalid timeout", applet);
		ຖ້າ (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		ສຳລັບ (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			ສຳລັບ (;;) {
				ຖ້າ (access(argv[i], F_OK) == 0)
					bha_nga;
				ຖ້າ (nanosleep(&ts, NULL) == -1)
					ສົ່ງຄືນ EXIT_FAILURE;
				gettimeofday(&now, NULL);
				ຖ້າ (retval <= 0)
					continue;
				ຖ້າ (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				ສົ່ງຄືນ EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		ສົ່ງຄືນ EXIT_SUCCESS;
	}

	ຖ້າ (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		ສຳລັບ (i = 0; i < argc; i++) {
			ຖ້າ (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	ຖ້າ (strcmp(applet, "einfo") == 0)
		e = einfo;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "einfon") == 0)
		e = einfon;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "eend") == 0)
		ee = eend;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "ewend") == 0)
		ee = ewend;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "veinfo") == 0)
		e = einfov;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "veend") == 0)
		ee = eendv;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "eindent") == 0)
		eindent();
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "veindent") == 0)
		eindentv();
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	ບໍ່ດັ່ງນັ້ນ {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	ຖ້າ (message) {
		ຖ້າ (e)
			e("%s", message);
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (ee)
			ee(retval, "%s", message);
	} ບໍ່ດັ່ງນັ້ນ {
		ຖ້າ (e)
			e(NULL);
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (ee)
			ee(retval, NULL);
	}

	free(message);
	ສົ່ງຄືນ retval;
}
