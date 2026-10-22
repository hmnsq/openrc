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

#અન્તર્ભાવ <sys/time.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <syslog.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <strings.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "helpers.h"

/* usecs to wait while we poll the file existence  */
#define WAIT_INTERVAL	20000000

અચલ અક્ષર *applet = ખાલી;

સ્થિર પૂર્ણાંક syslog_decode(અક્ષર *name, અચલ CODE *codetab)
{
	અચલ CODE *c;

	જો (isdigit((unsigned અક્ષર)*name))
		પાછા atoi(name);

	માટે (c = codetab; c->c_name; c++)
		જો (!strcasecmp(name, c->c_name))
			પાછા c->c_val;

	પાછા -1;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	પૂર્ણાંક retval = EXIT_SUCCESS;
	પૂર્ણાંક i;
	size_t l = 0;
	અક્ષર *message = ખાલી;
	અક્ષર *p;
	પૂર્ણાંક level = 0;
	સંરચના timespec ts;
	સંરચના timeval stop, now;
	પૂર્ણાંક (*e) (અચલ અક્ષર *, ...) EINFO_PRINTF(1, 2) = ખાલી;
	પૂર્ણાંક (*ee) (પૂર્ણાંક, અચલ અક્ષર *, ...) EINFO_PRINTF(2, 3) = ખાલી;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	જો (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	જો (argc > 0) {
		જો (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (પૂર્ણાંક)strtoimax(argv[0], &p, 0);
			જો (!p || *p != '\0')
				errno = EINVAL;
			જો (errno)
				retval = EXIT_FAILURE;
			નહીંતર {
				argc--;
				argv++;
			}
		} નહીંતર જો (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			જો (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			જો (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	જો (strcmp(applet, "ewaitfile") == 0) {
		જો (errno)
			eerrorx("%s: invalid timeout", applet);
		જો (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, ખાલી);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		માટે (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			માટે (;;) {
				જો (access(argv[i], F_OK) == 0)
					તોડો;
				જો (nanosleep(&ts, ખાલી) == -1)
					પાછા EXIT_FAILURE;
				gettimeofday(&now, ખાલી);
				જો (retval <= 0)
					ચાલુ;
				જો (timercmp(&now, &stop, <))
					ચાલુ;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				પાછા EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, ખાલી);
		}
		પાછા EXIT_SUCCESS;
	}

	જો (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		માટે (i = 0; i < argc; i++) {
			જો (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	જો (strcmp(applet, "einfo") == 0)
		e = einfo;
	નહીંતર જો (strcmp(applet, "einfon") == 0)
		e = einfon;
	નહીંતર જો (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	નહીંતર જો (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	નહીંતર જો (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} નહીંતર જો (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} નહીંતર જો (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	નહીંતર જો (strcmp(applet, "eend") == 0)
		ee = eend;
	નહીંતર જો (strcmp(applet, "ewend") == 0)
		ee = ewend;
	નહીંતર જો (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} નહીંતર જો (strcmp(applet, "veinfo") == 0)
		e = einfov;
	નહીંતર જો (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	નહીંતર જો (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	નહીંતર જો (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	નહીંતર જો (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	નહીંતર જો (strcmp(applet, "veend") == 0)
		ee = eendv;
	નહીંતર જો (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	નહીંતર જો (strcmp(applet, "eindent") == 0)
		eindent();
	નહીંતર જો (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	નહીંતર જો (strcmp(applet, "veindent") == 0)
		eindentv();
	નહીંતર જો (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	નહીંતર {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	જો (message) {
		જો (e)
			e("%s", message);
		નહીંતર જો (ee)
			ee(retval, "%s", message);
	} નહીંતર {
		જો (e)
			e(ખાલી);
		નહીંતર જો (ee)
			ee(retval, ખાલી);
	}

	મુક્ત(message);
	પાછા retval;
}
