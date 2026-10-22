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
#define case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#define case_RC_COMMON_getopt_case_V  सचे (argc == 2) show_version();
#define case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#define case_RC_COMMON_getopt_case_q  set_quiet_options();
#define case_RC_COMMON_getopt_case_U  rc_set_user();
#define case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#define case_RC_COMMON_GETOPT						      \
	पक्ष 'C': case_RC_COMMON_getopt_case_C; भङ्ग;			      \
	पक्ष 'h': case_RC_COMMON_getopt_case_h; भङ्ग;			      \
	पक्ष 'V': case_RC_COMMON_getopt_case_V; भङ्ग;			      \
	पक्ष 'v': case_RC_COMMON_getopt_case_v; भङ्ग;			      \
	पक्ष 'q': case_RC_COMMON_getopt_case_q; भङ्ग;			      \
	पक्ष 'U': case_RC_COMMON_getopt_case_U; भङ्ग;			      \
	स्वभाव:  case_RC_COMMON_getopt_default; भङ्ग;

बाह्य ध्रुव वर्ण *applet;
बाह्य ध्रुव वर्ण *extraopts;
बाह्य ध्रुव वर्ण getoptstring[];
बाह्य ध्रुव संरचना option longopts[];
बाह्य ध्रुव वर्ण * ध्रुव longopts_help[];
बाह्य ध्रुव वर्ण *usagestring;

शून्य set_quiet_options(शून्य);
शून्य show_version(शून्य);
शून्य usage(पूर्णाङ्क exit_status);
