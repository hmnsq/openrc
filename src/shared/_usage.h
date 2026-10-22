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

#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ "librc.h"

#ವ್ಯಾಖ್ಯಾನಿಸು getoptstring_COMMON "ChqVvU"

#ವ್ಯಾಖ್ಯಾನಿಸು longopts_COMMON							      \
	{ "help",           0, ಖಾಲಿ, 'h'},				      \
	{ "nocolor",        0, ಖಾಲಿ, 'C'},				      \
	{ "version",        0, ಖಾಲಿ, 'V'},				      \
	{ "verbose",        0, ಖಾಲಿ, 'v'},				      \
	{ "quiet",          0, ಖಾಲಿ, 'q'},				      \
	{ "user",           0, ಖಾಲಿ, 'U'},				      \
	{ ಖಾಲಿ,             0, ಖಾಲಿ,  0 }

#ವ್ಯಾಖ್ಯಾನಿಸು longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#ವ್ಯಾಖ್ಯಾನಿಸು case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#ವ್ಯಾಖ್ಯಾನಿಸು case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#ವ್ಯಾಖ್ಯಾನಿಸು case_RC_COMMON_getopt_case_V  ಆದರೆ (argc == 2) show_ಆವೃತ್ತಿ();
#ವ್ಯಾಖ್ಯಾನಿಸು case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#ವ್ಯಾಖ್ಯಾನಿಸು case_RC_COMMON_getopt_case_q  ಗಣ_quiet_options();
#ವ್ಯಾಖ್ಯಾನಿಸು case_RC_COMMON_getopt_case_U  rc_ಗಣ_user();
#ವ್ಯಾಖ್ಯಾನಿಸು case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#ವ್ಯಾಖ್ಯಾನಿಸು case_RC_COMMON_GETOPT						      \
	ಸ್ಥಿತಿ 'C': case_RC_COMMON_getopt_case_C; ನಿಲ್ಲಿಸು;			      \
	ಸ್ಥಿತಿ 'h': case_RC_COMMON_getopt_case_h; ನಿಲ್ಲಿಸು;			      \
	ಸ್ಥಿತಿ 'V': case_RC_COMMON_getopt_case_V; ನಿಲ್ಲಿಸು;			      \
	ಸ್ಥಿತಿ 'v': case_RC_COMMON_getopt_case_v; ನಿಲ್ಲಿಸು;			      \
	ಸ್ಥಿತಿ 'q': case_RC_COMMON_getopt_case_q; ನಿಲ್ಲಿಸು;			      \
	ಸ್ಥಿತಿ 'U': case_RC_COMMON_getopt_case_U; ನಿಲ್ಲಿಸು;			      \
	ಪೂರ್ವನಿಯೋಜಿತ:  case_RC_COMMON_getopt_default; ನಿಲ್ಲಿಸು;

ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring;

ಶೂನ್ಯ ಗಣ_quiet_options(ಶೂನ್ಯ);
ಶೂನ್ಯ show_ಆವೃತ್ತಿ(ಶೂನ್ಯ);
ಶೂನ್ಯ usage(ಪೂರ್ಣಾಂಕ exit_status);
