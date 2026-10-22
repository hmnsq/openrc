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

#अन्तर्भाव <getopt.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव "librc.h"

#define getoptstring_COMMON "ChqVvU"

#define longopts_COMMON							      \
	{ "help",           0, रिक्त, 'h'},				      \
	{ "nocolor",        0, रिक्त, 'C'},				      \
	{ "version",        0, रिक्त, 'V'},				      \
	{ "verbose",        0, रिक्त, 'v'},				      \
	{ "quiet",          0, रिक्त, 'q'},				      \
	{ "user",           0, रिक्त, 'U'},				      \
	{ रिक्त,             0, रिक्त,  0 }

#define longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#define case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#define case_RC_COMMON_getopt_case_h  usage (निकास_सफल);
#define case_RC_COMMON_getopt_case_V  जर (argc == 2) show_version();
#define case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#define case_RC_COMMON_getopt_case_q  set_quiet_options();
#define case_RC_COMMON_getopt_case_U  rc_set_user();
#define case_RC_COMMON_getopt_default usage (निकास_त्रुटि);

#define case_RC_COMMON_GETOPT						      \
	स्थिती 'C': case_RC_COMMON_getopt_case_C; थांबा;			      \
	स्थिती 'h': case_RC_COMMON_getopt_case_h; थांबा;			      \
	स्थिती 'V': case_RC_COMMON_getopt_case_V; थांबा;			      \
	स्थिती 'v': case_RC_COMMON_getopt_case_v; थांबा;			      \
	स्थिती 'q': case_RC_COMMON_getopt_case_q; थांबा;			      \
	स्थिती 'U': case_RC_COMMON_getopt_case_U; थांबा;			      \
	पूर्वनिर्धारित:  case_RC_COMMON_getopt_default; थांबा;

बाह्य स्थिरांक अक्षर *applet;
बाह्य स्थिरांक अक्षर *extraopts;
बाह्य स्थिरांक अक्षर getoptstring[];
बाह्य स्थिरांक संरचना option longopts[];
बाह्य स्थिरांक अक्षर * स्थिरांक longopts_help[];
बाह्य स्थिरांक अक्षर *usagestring;

शून्य set_quiet_options(शून्य);
शून्य show_version(शून्य);
शून्य usage(पूर्णांक exit_status);
