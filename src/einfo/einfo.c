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

#شامل <sys/time.h>
#شامل <errno.h>
#شامل <ctype.h>
#شامل <inttypes.h>
#شامل <stddef.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <syslog.h>
#شامل <time.h>
#شامل <strings.h>

#شامل "einfo.h"
#شامل "helpers.h"

/* usecs to wait while we poll the file existence  */
#define WAIT_INTERVAL	20000000

ثابت نویسه *applet = پوچ;

ایستا صحیح syslog_decode(نویسه *name, ثابت CODE *codetab)
{
	ثابت CODE *c;

	اگر (isdigit((unsigned نویسه)*name))
		بازگشت atoi(name);

	برای (c = codetab; c->c_name; c++)
		اگر (!strcasecmp(name, c->c_name))
			بازگشت c->c_val;

	بازگشت -1;
}

صحیح اصلی(صحیح argc, نویسه **argv)
{
	صحیح retval = EXIT_SUCCESS;
	صحیح i;
	size_t l = 0;
	نویسه *message = پوچ;
	نویسه *p;
	صحیح level = 0;
	ساختار timespec ts;
	ساختار timeval stop, now;
	صحیح (*e) (ثابت نویسه *, ...) EINFO_PRINTF(1, 2) = پوچ;
	صحیح (*ee) (صحیح, ثابت نویسه *, ...) EINFO_PRINTF(2, 3) = پوچ;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	اگر (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	اگر (argc > 0) {
		اگر (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (صحیح)strtoimax(argv[0], &p, 0);
			اگر (!p || *p != '\0')
				errno = EINVAL;
			اگر (errno)
				retval = EXIT_FAILURE;
			وگرنه {
				argc--;
				argv++;
			}
		} وگرنه اگر (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			اگر (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			اگر (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	اگر (strcmp(applet, "ewaitfile") == 0) {
		اگر (errno)
			eerrorx("%s: invalid timeout", applet);
		اگر (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, پوچ);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		برای (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			برای (;;) {
				اگر (access(argv[i], F_OK) == 0)
					بشکن;
				اگر (nanosleep(&ts, پوچ) == -1)
					بازگشت EXIT_FAILURE;
				gettimeofday(&now, پوچ);
				اگر (retval <= 0)
					ادامه;
				اگر (timercmp(&now, &stop, <))
					ادامه;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				بازگشت EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, پوچ);
		}
		بازگشت EXIT_SUCCESS;
	}

	اگر (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		برای (i = 0; i < argc; i++) {
			اگر (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	اگر (strcmp(applet, "einfo") == 0)
		e = einfo;
	وگرنه اگر (strcmp(applet, "einfon") == 0)
		e = einfon;
	وگرنه اگر (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	وگرنه اگر (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	وگرنه اگر (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} وگرنه اگر (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} وگرنه اگر (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	وگرنه اگر (strcmp(applet, "eend") == 0)
		ee = eend;
	وگرنه اگر (strcmp(applet, "ewend") == 0)
		ee = ewend;
	وگرنه اگر (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} وگرنه اگر (strcmp(applet, "veinfo") == 0)
		e = einfov;
	وگرنه اگر (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	وگرنه اگر (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	وگرنه اگر (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	وگرنه اگر (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	وگرنه اگر (strcmp(applet, "veend") == 0)
		ee = eendv;
	وگرنه اگر (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	وگرنه اگر (strcmp(applet, "eindent") == 0)
		eindent();
	وگرنه اگر (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	وگرنه اگر (strcmp(applet, "veindent") == 0)
		eindentv();
	وگرنه اگر (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	وگرنه {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	اگر (message) {
		اگر (e)
			e("%s", message);
		وگرنه اگر (ee)
			ee(retval, "%s", message);
	} وگرنه {
		اگر (e)
			e(پوچ);
		وگرنه اگر (ee)
			ee(retval, پوچ);
	}

	آزاد(message);
	بازگشت retval;
}
