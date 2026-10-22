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

#घोषणा SYSLOG_NAMES

#समावेश <sys/time.h>
#समावेश <errno.h>
#समावेश <ctype.h>
#समावेश <inttypes.h>
#समावेश <stddef.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <syslog.h>
#समावेश <time.h>
#समावेश <strings.h>

#समावेश "einfo.h"
#समावेश "helpers.h"

/* usecs to wait while we poll the file existence  */
#घोषणा WAIT_INTERVAL	20000000

स्थिर अक्षर *applet = शून्य;

अटल पूर्णांक syslog_decode(अक्षर *name, स्थिर CODE *codetab)
{
	स्थिर CODE *c;

	अगर (है_अंक((अचिन्हित अक्षर)*name))
		वापस म_से_प(name);

	क्रम (c = codetab; c->c_name; c++)
		अगर (!strcasecmp(name, c->c_name))
			वापस c->c_val;

	वापस -1;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक retval = निकास_सफल;
	पूर्णांक i;
	माप_प्रकार l = 0;
	अक्षर *message = शून्य;
	अक्षर *p;
	पूर्णांक level = 0;
	काष्ठा timespec ts;
	काष्ठा timeval stop, now;
	पूर्णांक (*e) (स्थिर अक्षर *, ...) EINFO_PRINTF(1, 2) = शून्य;
	पूर्णांक (*ee) (पूर्णांक, स्थिर अक्षर *, ...) EINFO_PRINTF(2, 3) = शून्य;

	applet = basename_c(argv[0]);
	argc--;
	argv++;

	अगर (म_भेद(applet, "eval_ecolors") == 0) {
		म_लिखो("GOOD='%s'\nWARN='%s'\nBAD='%s'\nHILITE='%s'\nBRACKET='%s'\nNORMAL='%s'\n",
		    ecolor(ECOLOR_GOOD),
		    ecolor(ECOLOR_WARN),
		    ecolor(ECOLOR_BAD),
		    ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_BRACKET),
		    ecolor(ECOLOR_NORMAL));
		निकास(निकास_सफल);
	}

	अगर (argc > 0) {
		अगर (म_भेद(applet, "eend") == 0 ||
		    म_भेद(applet, "ewend") == 0 ||
		    म_भेद(applet, "veend") == 0 ||
		    म_भेद(applet, "vewend") == 0 ||
		    म_भेद(applet, "ewaitfile") == 0)
		{
			त्रुटि_सं = 0;
			retval = (पूर्णांक)strtoimax(argv[0], &p, 0);
			अगर (!p || *p != '\0')
				त्रुटि_सं = EINVAL;
			अगर (त्रुटि_सं)
				retval = निकास_त्रुटि;
			अन्यथा {
				argc--;
				argv++;
			}
		} अन्यथा अगर (म_भेद(applet, "esyslog") == 0 ||
		    म_भेद(applet, "elog") == 0) {
			p = म_अक्षर(argv[0], '.');
			अगर (!p ||
			    (level = syslog_decode(p + 1, prioritynames)) == -1)
				eerrorx("%s: invalid log level `%s'", applet, argv[0]);

			अगर (argc < 3)
				eerrorx("%s: not enough arguments", applet);

			unsetenv("EINFO_LOG");
			setenv("EINFO_LOG", argv[1], 1);

			argc -= 2;
			argv += 2;
		}
	}

	अगर (म_भेद(applet, "ewaitfile") == 0) {
		अगर (त्रुटि_सं)
			eerrorx("%s: invalid timeout", applet);
		अगर (argc == 0)
			eerrorx("%s: not enough arguments", applet);

		gettimeofday(&stop, शून्य);
		/* retval stores the timeout */
		stop.tv_sec += retval;
		ts.tv_sec = 0;
		ts.tv_nsec = WAIT_INTERVAL;
		क्रम (i = 0; i < argc; i++) {
			ebeginv("Waiting for %s", argv[i]);
			क्रम (;;) {
				अगर (access(argv[i], F_OK) == 0)
					अवरोध;
				अगर (nanosleep(&ts, शून्य) == -1)
					वापस निकास_त्रुटि;
				gettimeofday(&now, शून्य);
				अगर (retval <= 0)
					जारी;
				अगर (timercmp(&now, &stop, <))
					जारी;
				eendv(निकास_त्रुटि,
				    "timed out waiting for %s", argv[i]);
				वापस निकास_त्रुटि;
			}
			eendv(निकास_सफल, शून्य);
		}
		वापस निकास_सफल;
	}

	अगर (argc > 0) {
		खाता *mem = xopen_memstream(&message, &l);
		क्रम (i = 0; i < argc; i++) {
			अगर (i > 0)
				ख_अक्षर_दो(' ', mem);
			ख_माला_दो(argv[i], mem);
		}
		xclose_memstream(mem);
	}

	अगर (म_भेद(applet, "einfo") == 0)
		e = einfo;
	अन्यथा अगर (म_भेद(applet, "einfon") == 0)
		e = einfon;
	अन्यथा अगर (म_भेद(applet, "ewarn") == 0)
		e = ewarn;
	अन्यथा अगर (म_भेद(applet, "ewarnn") == 0)
		e = ewarnn;
	अन्यथा अगर (म_भेद(applet, "eerror") == 0) {
		e = eerror;
		retval = 1;
	} अन्यथा अगर (म_भेद(applet, "eerrorn") == 0) {
		e = eerrorn;
		retval = 1;
	} अन्यथा अगर (म_भेद(applet, "ebegin") == 0)
		e = ebegin;
	अन्यथा अगर (म_भेद(applet, "eend") == 0)
		ee = eend;
	अन्यथा अगर (म_भेद(applet, "ewend") == 0)
		ee = ewend;
	अन्यथा अगर (म_भेद(applet, "esyslog") == 0) {
		elog(retval, "%s", message);
		retval = 0;
	} अन्यथा अगर (म_भेद(applet, "veinfo") == 0)
		e = einfov;
	अन्यथा अगर (म_भेद(applet, "veinfon") == 0)
		e = einfovn;
	अन्यथा अगर (म_भेद(applet, "vewarn") == 0)
		e = ewarnv;
	अन्यथा अगर (म_भेद(applet, "vewarnn") == 0)
		e = ewarnvn;
	अन्यथा अगर (म_भेद(applet, "vebegin") == 0)
		e = ebeginv;
	अन्यथा अगर (म_भेद(applet, "veend") == 0)
		ee = eendv;
	अन्यथा अगर (म_भेद(applet, "vewend") == 0)
		ee = ewendv;
	अन्यथा अगर (म_भेद(applet, "eindent") == 0)
		eindent();
	अन्यथा अगर (म_भेद(applet, "eoutdent") == 0)
		eoutdent();
	अन्यथा अगर (म_भेद(applet, "veindent") == 0)
		eindentv();
	अन्यथा अगर (म_भेद(applet, "veoutdent") == 0)
		eoutdentv();
	अन्यथा {
		eerror("%s: unknown applet", applet);
		retval = निकास_त्रुटि;
	}

	अगर (message) {
		अगर (e)
			e("%s", message);
		अन्यथा अगर (ee)
			ee(retval, "%s", message);
	} अन्यथा {
		अगर (e)
			e(शून्य);
		अन्यथा अगर (ee)
			ee(retval, शून्य);
	}

	मुक्त(message);
	वापस retval;
}
