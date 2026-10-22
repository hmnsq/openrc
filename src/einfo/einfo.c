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

#अन्तर्भाव <sys/time.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <syslog.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <strings.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "helpers.h"

/* usecs to wait while we poll the file existence  */
#define WAIT_INTERVAL	20000000

ध्रुव वर्ण *applet = रिक्त;

स्थिर पूर्णाङ्क syslog_decode(वर्ण *name, ध्रुव CODE *codetab)
{
	ध्रुव CODE *c;

	यदि (isdigit((unsigned वर्ण)*name))
		प्रत्यागम atoi(name);

	प्रति (c = codetab; c->c_name; c++)
		यदि (!strcasecmp(name, c->c_name))
			प्रत्यागम c->c_val;

	प्रत्यागम -1;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क retval = EXIT_SUCCESS;
	पूर्णाङ्क i;
	size_t l = 0;
	वर्ण *message = रिक्त;
	वर्ण *p;
	पूर्णाङ्क level = 0;
	संरचना timespec ts;
	संरचना timeval stop, now;
	पूर्णाङ्क (*e) (ध्रुव वर्ण *, ...) EINFO_PRINTF(1, 2) = रिक्त;
	पूर्णाङ्क (*ee) (पूर्णाङ्क, ध्रुव वर्ण *, ...) EINFO_PRINTF(2, 3) = रिक्त;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	यदि (strcmp(applet, "eval_ecolors") == 0) {
		printf("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		exit(EXIT_SUCCESS);
	}

	यदि (argc > 0) {
		यदि (strcmp(applet, "eend") == 0 ||
		    strcmp(applet, "ewend") == 0 ||
		    strcmp(applet, "veend") == 0 ||
		    strcmp(applet, "vewend") == 0 ||
		    strcmp(applet, "ewaitfile") == 0)
		{
			errno = 0;
			retval = (पूर्णाङ्क)strtoimax(argv[0], &p, 0);
			यदि (!p || *p != '\0')
				errno = EINVAL;
			यदि (errno)
				retval = EXIT_FAILURE;
			अन्यथा {
				argc--;
				argv++;
			}
		} अन्यथा यदि (strcmp(applet, "esyslog") == 0 ||
		    strcmp(applet, "elog") == 0) {
			p = strchr(argv[0], '.');
			यदि (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			यदि (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	यदि (strcmp(applet, "ewaitfile") == 0) {
		यदि (errno)
			eerrorx("%s: invalid timeout", applet);
		यदि (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, रिक्त);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		प्रति (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			प्रति (;;) {
				यदि (access(argv[i], F_OK) == 0)
					भङ्ग;
				यदि (nanosleep(&ts, रिक्त) == -1)
					प्रत्यागम EXIT_FAILURE;
				gettimeofday(&now, रिक्त);
				यदि (retval <= 0)
					अनुवर्त;
				यदि (timercmp(&now, &stop, <))
					अनुवर्त;
				eendv(EXIT_FAILURE,
				    "timed out waiting for %s", argv[i]);
				प्रत्यागम EXIT_FAILURE;
			}
			eendv(EXIT_SUCCESS, रिक्त);
		}
		प्रत्यागम EXIT_SUCCESS;
	}

	यदि (argc > 0) {
		FILE *mem = xopen_memstream(&message, &l);
		प्रति (i = 0; i < argc; i++) {
			यदि (i > 0)
				fputc(' ', mem);
			fputs(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	यदि (strcmp(applet, "einfo") == 0)
		e = einfo;
	अन्यथा यदि (strcmp(applet, "einfon") == 0)
		e = einfon;
	अन्यथा यदि (strcmp(applet, "ewarn") == 0)
		e = ewarn;
	अन्यथा यदि (strcmp(applet, "ewarnn") == 0)
		e = ewarnn;
	अन्यथा यदि (strcmp(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} अन्यथा यदि (strcmp(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} अन्यथा यदि (strcmp(applet, "ebegin") == 0)
		e = ebegin;
	अन्यथा यदि (strcmp(applet, "eend") == 0)
		ee = eend;
	अन्यथा यदि (strcmp(applet, "ewend") == 0)
		ee = ewend;
	अन्यथा यदि (strcmp(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} अन्यथा यदि (strcmp(applet, "veinfo") == 0)
		e = einfov;
	अन्यथा यदि (strcmp(applet, "veinfon") == 0)
		e = einfovn;
	अन्यथा यदि (strcmp(applet, "vewarn") == 0)
		e = ewarnv;
	अन्यथा यदि (strcmp(applet, "vewarnn") == 0)
		e = ewarnvn;
	अन्यथा यदि (strcmp(applet, "vebegin") == 0)
		e = ebeginv;
	अन्यथा यदि (strcmp(applet, "veend") == 0)
		ee = eendv;
	अन्यथा यदि (strcmp(applet, "vewend") == 0)
		ee = ewendv;
	अन्यथा यदि (strcmp(applet, "eindent") == 0)
		eindent();
	अन्यथा यदि (strcmp(applet, "eoutdent") == 0)
		eoutdent();
	अन्यथा यदि (strcmp(applet, "veindent") == 0)
		eindentv();
	अन्यथा यदि (strcmp(applet, "veoutdent") == 0)
		eoutdentv();
	अन्यथा {
		eerror("%s: unknown applet", applet);
		retval = EXIT_FAILURE;
	}

	यदि (message) {
		यदि (e)
			e("%s", message);
		अन्यथा यदि (ee)
			ee(retval, "%s", message);
	} अन्यथा {
		यदि (e)
			e(रिक्त);
		अन्यथा यदि (ee)
			ee(retval, रिक्त);
	}

	मुक्त(message);
	प्रत्यागम retval;
}
