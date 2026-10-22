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

#تعریف_کرو SYSLOG_نام

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
#تعریف_کرو WAIT_INTERVAL	20000000

ثابت حرف *applet = تہی;

ساکن صحیح syslog_decode(حرف *name, ثابت CODE *codetab)
{
	ثابت CODE *c;

	اگر (ہندسہ_ہے((غیرعلامتی حرف)*name))
		واپس atoi(name);

	برائے (c = codetab; c->c_name; c++)
		اگر (!strcasecmp(name, c->c_name))
			واپس c->c_val;

	واپس -1;
}

صحیح مین(صحیح argc, حرف **argv)
{
	صحیح retval = EXIT_SUCCESS;
	صحیح i;
	size_t l = 0;
	حرف *پیغام = تہی;
	حرف *p;
	صحیح level = 0;
	ساخت timespec ts;
	ساخت timeval stop, now;
	صحیح (*e) (ثابت حرف *, ...) EINFO_PRINTF(1, 2) = تہی;
	صحیح (*ee) (صحیح, ثابت حرف *, ...) EINFO_PRINTF(2, 3) = تہی;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	اگر (strcmp(applet, "eval_ecolors") == 0) {
		لکھو("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_معمول));
		باہر_نکلو(EXIT_SUCCESS);
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
			ورنہ {
				argc--;
				argv++;
			}
		} ورنہ اگر (strcmp(applet, "esyslog") == 0 ||
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

		gettimeofday(&stop, تہی);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		برائے (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			برائے (;;) {
				اگر (access(argv[i], F_OK) == 0)
					توڑو;
				اگر (nanosleep(&ts, تہی) == -1)
					واپس EXIT_FAILURE;
				gettimeofday(&now, تہی);
				اگر (retval <= 0)
					جاری;
				اگر (timercmp(&now, &stop, <))
					جاری;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				واپس EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, تہی);
		}
		واپس EXIT_SUCCESS;
	}

	اگر (argc > 0) {
		FILE *mem = xopen_memstream(&پیغام, &l);
		برائے (i = 0; i < argc; i++) {
			اگر (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	اگر (strcmp(applet, "einfo") == 0)
		e = einfo;
	ورنہ اگر (strcmp(applet, "einfon") == 0)
		e = einfon;
	ورنہ اگر (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	ورنہ اگر (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	ورنہ اگر (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} ورنہ اگر (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} ورنہ اگر (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	ورنہ اگر (strcmp(applet, "eend") == 0)
		ee = eend;
	ورنہ اگر (strcmp(applet, "ewend") == 0)
		ee = ewend;
	ورنہ اگر (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", پیغام);
		retval = 0;
	} ورنہ اگر (strcmp(applet, "veinfo") == 0)
		e = einfov;
	ورنہ اگر (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	ورنہ اگر (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	ورنہ اگر (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	ورنہ اگر (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	ورنہ اگر (strcmp(applet, "veend") == 0)
		ee = eendv;
	ورنہ اگر (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	ورنہ اگر (strcmp(applet, "eindent") == 0)
		eindent();
	ورنہ اگر (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	ورنہ اگر (strcmp(applet, "veindent") == 0)
		eindentv();
	ورنہ اگر (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	ورنہ {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	اگر (پیغام) {
		اگر (e)
			e("%s", پیغام);
		ورنہ اگر (ee)
			ee(retval, "%s", پیغام);
	} ورنہ {
		اگر (e)
			e(تہی);
		ورنہ اگر (ee)
			ee(retval, تہی);
	}

	آزاد(پیغام);
	واپس retval;
}
