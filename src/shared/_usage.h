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

#define getoptstring_COMMON "ChqVvU"

#define longopts_COMMON							      \
	{ "help",           0, ರಿಕ್ತ, 'h'},				      \
	{ "nocolor",        0, ರಿಕ್ತ, 'C'},				      \
	{ "version",        0, ರಿಕ್ತ, 'V'},				      \
	{ "verbose",        0, ರಿಕ್ತ, 'v'},				      \
	{ "quiet",          0, ರಿಕ್ತ, 'q'},				      \
	{ "user",           0, ರಿಕ್ತ, 'U'},				      \
	{ ರಿಕ್ತ,             0, ರಿಕ್ತ,  0 }

#define longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#define case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#define case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#define case_RC_COMMON_getopt_case_V  ಯದಿ (argc == 2) show_version();
#define case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#define case_RC_COMMON_getopt_case_q  set_quiet_options();
#define case_RC_COMMON_getopt_case_U  rc_set_user();
#define case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#define case_RC_COMMON_GETOPT						      \
	ಪಕ್ಷ 'C': case_RC_COMMON_getopt_case_C; ಭಙ್ಗ;			      \
	ಪಕ್ಷ 'h': case_RC_COMMON_getopt_case_h; ಭಙ್ಗ;			      \
	ಪಕ್ಷ 'V': case_RC_COMMON_getopt_case_V; ಭಙ್ಗ;			      \
	ಪಕ್ಷ 'v': case_RC_COMMON_getopt_case_v; ಭಙ್ಗ;			      \
	ಪಕ್ಷ 'q': case_RC_COMMON_getopt_case_q; ಭಙ್ಗ;			      \
	ಪಕ್ಷ 'U': case_RC_COMMON_getopt_case_U; ಭಙ್ಗ;			      \
	ಸ್ವಭಾವ:  case_RC_COMMON_getopt_default; ಭಙ್ಗ;

ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *applet;
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *extraopts;
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ getoptstring[];
ಬಾಹ್ಯ ಧ್ರುವ ಸಂರಚನಾ option longopts[];
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[];
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *usagestring;

ಶೂನ್ಯ set_quiet_options(ಶೂನ್ಯ);
ಶೂನ್ಯ show_version(ಶೂನ್ಯ);
ಶೂನ್ಯ usage(ಪೂರ್ಣಾಙ್ಕ exit_status);
