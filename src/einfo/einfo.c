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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SYSLOG_ਨਾਂ

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <syslog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

/* usecs to wait while we poll the file existence  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WAIT_INTERVAL	20000000

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;

ਸਥਿਰ ਪੂਰਨ syslog_decode(ਅੱਖਰ *name, ਅਚਲ CODE *codetab)
{
	ਅਚਲ CODE *c;

	ਜੇ (ਅੰਕ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*name))
		ਵਾਪਸ atoi(name);

	ਲਈ (c = codetab; c->c_name; c++)
		ਜੇ (!strcasecmp(name, c->c_name))
			ਵਾਪਸ c->c_val;

	ਵਾਪਸ -1;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਪੂਰਨ retval = EXIT_SUCCESS;
	ਪੂਰਨ i;
	size_t l = 0;
	ਅੱਖਰ *ਸੁਨੇਹਾ = ਨਿਰਾ;
	ਅੱਖਰ *p;
	ਪੂਰਨ level = 0;
	ਬਣਤਰ timespec ts;
	ਬਣਤਰ timeval stop, now;
	ਪੂਰਨ (*e) (ਅਚਲ ਅੱਖਰ *, ...) EINFO_PRINTF(1, 2) = ਨਿਰਾ;
	ਪੂਰਨ (*ee) (ਪੂਰਨ, ਅਚਲ ਅੱਖਰ *, ...) EINFO_PRINTF(2, 3) = ਨਿਰਾ;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	ਜੇ (strcmp(applet, "eval_ecolors") == 0) {
		ਲਿਖੋ("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_ਸਾਧਾਰਨ));
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
	}

	ਜੇ (argc > 0) {
		ਜੇ (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (ਪੂਰਨ)strtoimax(argv[0], &p, 0);
			ਜੇ (!p || *p != '\0')
				errno = EINVAL;
			ਜੇ (errno)
				retval = EXIT_FAILURE;
			ਨਹੀਂਤਾਂ {
				argc--;
				argv++;
			}
		} ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			ਜੇ (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			ਜੇ (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	ਜੇ (strcmp(applet, "ewaitfile") == 0) {
		ਜੇ (errno)
			eerrorx("%s: invalid timeout", applet);
		ਜੇ (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, ਨਿਰਾ);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		ਲਈ (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			ਲਈ (;;) {
				ਜੇ (access(argv[i], F_OK) == 0)
					ਰੋਕੋ;
				ਜੇ (nanosleep(&ts, ਨਿਰਾ) == -1)
					ਵਾਪਸ EXIT_FAILURE;
				gettimeofday(&now, ਨਿਰਾ);
				ਜੇ (retval <= 0)
					ਜਾਰੀ;
				ਜੇ (timercmp(&now, &stop, <))
					ਜਾਰੀ;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				ਵਾਪਸ EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, ਨਿਰਾ);
		}
		ਵਾਪਸ EXIT_SUCCESS;
	}

	ਜੇ (argc > 0) {
		FILE *mem = xopen_memstream(&ਸੁਨੇਹਾ, &l);
		ਲਈ (i = 0; i < argc; i++) {
			ਜੇ (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	ਜੇ (strcmp(applet, "einfo") == 0)
		e = einfo;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "einfon") == 0)
		e = einfon;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "eend") == 0)
		ee = eend;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "ewend") == 0)
		ee = ewend;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", ਸੁਨੇਹਾ);
		retval = 0;
	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "veinfo") == 0)
		e = einfov;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "veend") == 0)
		ee = eendv;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "eindent") == 0)
		eindent();
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "veindent") == 0)
		eindentv();
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	ਨਹੀਂਤਾਂ {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	ਜੇ (ਸੁਨੇਹਾ) {
		ਜੇ (e)
			e("%s", ਸੁਨੇਹਾ);
		ਨਹੀਂਤਾਂ ਜੇ (ee)
			ee(retval, "%s", ਸੁਨੇਹਾ);
	} ਨਹੀਂਤਾਂ {
		ਜੇ (e)
			e(ਨਿਰਾ);
		ਨਹੀਂਤਾਂ ਜੇ (ee)
			ee(retval, ਨਿਰਾ);
	}

	ਮੁਕਤ(ਸੁਨੇਹਾ);
	ਵਾਪਸ retval;
}
