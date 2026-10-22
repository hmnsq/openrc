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

#அந்தர்பாவ <sys/time.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <inttypes.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <syslog.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <strings.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "helpers.h"

/* usecs to wait while we poll the file existence  */
#define WAIT_INTERVAL	20000000

மாறிலி எழுத்து *applet = வெற்று;

நிலையான முழு syslog_decode(எழுத்து *name, மாறிலி CODE *codetab)
{
	மாறிலி CODE *c;

	எனில் (isdigit((unsigned எழுத்து)*name))
		திரும்பு atoi(name);

	க்கு (c = codetab; c->c_name; c++)
		எனில் (!strcasecmp(name, c->c_name))
			திரும்பு c->c_val;

	திரும்பு -1;
}

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	முழு retval = EXIT_SUCCESS;
	முழு i;
	size_t l = 0;
	எழுத்து *message = வெற்று;
	எழுத்து *p;
	முழு level = 0;
	கட்டமைப்பு timespec ts;
	கட்டமைப்பு timeval stop, now;
	முழு (*e) (மாறிலி எழுத்து *, ...) EINFO_PRINTF(1, 2) = வெற்று;
	முழு (*ee) (முழு, மாறிலி எழுத்து *, ...) EINFO_PRINTF(2, 3) = வெற்று;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	எனில் (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	எனில் (argc > 0) {
		எனில் (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (முழு)strtoimax(argv[0], &p, 0);
			எனில் (!p || *p != '\0')
				errno = EINVAL;
			எனில் (errno)
				retval = EXIT_FAILURE;
			இல்லையெனில் {
				argc--;
				argv++;
			}
		} இல்லையெனில் எனில் (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			எனில் (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			எனில் (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	எனில் (strcmp(applet, "ewaitfile") == 0) {
		எனில் (errno)
			eerrorx("%s: invalid timeout", applet);
		எனில் (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, வெற்று);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		க்கு (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			க்கு (;;) {
				எனில் (access(argv[i], F_OK) == 0)
					நிறுத்து;
				எனில் (nanosleep(&ts, வெற்று) == -1)
					திரும்பு EXIT_FAILURE;
				gettimeofday(&now, வெற்று);
				எனில் (retval <= 0)
					தொடர்;
				எனில் (timercmp(&now, &stop, <))
					தொடர்;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				திரும்பு EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, வெற்று);
		}
		திரும்பு EXIT_SUCCESS;
	}

	எனில் (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		க்கு (i = 0; i < argc; i++) {
			எனில் (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	எனில் (strcmp(applet, "einfo") == 0)
		e = einfo;
	இல்லையெனில் எனில் (strcmp(applet, "einfon") == 0)
		e = einfon;
	இல்லையெனில் எனில் (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	இல்லையெனில் எனில் (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	இல்லையெனில் எனில் (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} இல்லையெனில் எனில் (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} இல்லையெனில் எனில் (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	இல்லையெனில் எனில் (strcmp(applet, "eend") == 0)
		ee = eend;
	இல்லையெனில் எனில் (strcmp(applet, "ewend") == 0)
		ee = ewend;
	இல்லையெனில் எனில் (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} இல்லையெனில் எனில் (strcmp(applet, "veinfo") == 0)
		e = einfov;
	இல்லையெனில் எனில் (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	இல்லையெனில் எனில் (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	இல்லையெனில் எனில் (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	இல்லையெனில் எனில் (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	இல்லையெனில் எனில் (strcmp(applet, "veend") == 0)
		ee = eendv;
	இல்லையெனில் எனில் (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	இல்லையெனில் எனில் (strcmp(applet, "eindent") == 0)
		eindent();
	இல்லையெனில் எனில் (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	இல்லையெனில் எனில் (strcmp(applet, "veindent") == 0)
		eindentv();
	இல்லையெனில் எனில் (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	இல்லையெனில் {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	எனில் (message) {
		எனில் (e)
			e("%s", message);
		இல்லையெனில் எனில் (ee)
			ee(retval, "%s", message);
	} இல்லையெனில் {
		எனில் (e)
			e(வெற்று);
		இல்லையெனில் எனில் (ee)
			ee(retval, வெற்று);
	}

	விடுவி(message);
	திரும்பு retval;
}
