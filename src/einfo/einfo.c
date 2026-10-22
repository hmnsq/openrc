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

static ကိန်းပြည့် syslog_decode(var_na *name, const CODE *codetab)
{
	const CODE *c;

	အကယ်၍ (isdigit((unsigned var_na)*name))
		ပြန်ပေး atoi(name);

	အတွက် (c = codetab; c->c_name; c++)
		အကယ်၍ (!strcasecmp(name, c->c_name))
			ပြန်ပေး c->c_val;

	ပြန်ပေး -1;
}

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na **argv)
{
	ကိန်းပြည့် retval = EXIT_SUCCESS;
	ကိန်းပြည့် i;
	size_t l = 0;
	var_na *message = NULL;
	var_na *p;
	ကိန်းပြည့် level = 0;
	sa_nracanaa timespec ts;
	sa_nracanaa timeval stop, now;
	ကိန်းပြည့် (*e) (const var_na *, ...) EINFO_PRINTF(1, 2) = NULL;
	ကိန်းပြည့် (*ee) (ကိန်းပြည့်, const var_na *, ...) EINFO_PRINTF(2, 3) = NULL;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	အကယ်၍ (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	အကယ်၍ (argc > 0) {
		အကယ်၍ (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (ကိန်းပြည့်)strtoimax(argv[0], &p, 0);
			အကယ်၍ (!p || *p != '\0')
				errno = EINVAL;
			အကယ်၍ (errno)
				retval = EXIT_FAILURE;
			မဟုတ်ပါက {
				argc--;
				argv++;
			}
		} မဟုတ်ပါက အကယ်၍ (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			အကယ်၍ (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			အကယ်၍ (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	အကယ်၍ (strcmp(applet, "ewaitfile") == 0) {
		အကယ်၍ (errno)
			eerrorx("%s: invalid timeout", applet);
		အကယ်၍ (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, NULL);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		အတွက် (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			အတွက် (;;) {
				အကယ်၍ (access(argv[i], F_OK) == 0)
					bha_nga;
				အကယ်၍ (nanosleep(&ts, NULL) == -1)
					ပြန်ပေး EXIT_FAILURE;
				gettimeofday(&now, NULL);
				အကယ်၍ (retval <= 0)
					continue;
				အကယ်၍ (timercmp(&now, &stop, <))
					continue;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				ပြန်ပေး EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, NULL);
		}
		ပြန်ပေး EXIT_SUCCESS;
	}

	အကယ်၍ (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		အတွက် (i = 0; i < argc; i++) {
			အကယ်၍ (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	အကယ်၍ (strcmp(applet, "einfo") == 0)
		e = einfo;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "einfon") == 0)
		e = einfon;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} မဟုတ်ပါက အကယ်၍ (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} မဟုတ်ပါက အကယ်၍ (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "eend") == 0)
		ee = eend;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "ewend") == 0)
		ee = ewend;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} မဟုတ်ပါက အကယ်၍ (strcmp(applet, "veinfo") == 0)
		e = einfov;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "veend") == 0)
		ee = eendv;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "eindent") == 0)
		eindent();
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "veindent") == 0)
		eindentv();
	မဟုတ်ပါက အကယ်၍ (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	မဟုတ်ပါက {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	အကယ်၍ (message) {
		အကယ်၍ (e)
			e("%s", message);
		မဟုတ်ပါက အကယ်၍ (ee)
			ee(retval, "%s", message);
	} မဟုတ်ပါက {
		အကယ်၍ (e)
			e(NULL);
		မဟုတ်ပါက အကယ်၍ (ee)
			ee(retval, NULL);
	}

	free(message);
	ပြန်ပေး retval;
}
