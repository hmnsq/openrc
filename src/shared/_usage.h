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

#شامل <getopt.h>
#شامل <stdlib.h>
#شامل "librc.h"

#define getoptstring_COMMON "ChqVvU"

#define longopts_COMMON							      \
	{ "help",           0, پوچ, 'h'},				      \
	{ "nocolor",        0, پوچ, 'C'},				      \
	{ "version",        0, پوچ, 'V'},				      \
	{ "verbose",        0, پوچ, 'v'},				      \
	{ "quiet",          0, پوچ, 'q'},				      \
	{ "user",           0, پوچ, 'U'},				      \
	{ پوچ,             0, پوچ,  0 }

#define longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#define case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#define case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#define case_RC_COMMON_getopt_case_V  اگر (argc == 2) show_version();
#define case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#define case_RC_COMMON_getopt_case_q  set_quiet_options();
#define case_RC_COMMON_getopt_case_U  rc_set_user();
#define case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#define case_RC_COMMON_GETOPT						      \
	حالت 'C': case_RC_COMMON_getopt_case_C; بشکن;			      \
	حالت 'h': case_RC_COMMON_getopt_case_h; بشکن;			      \
	حالت 'V': case_RC_COMMON_getopt_case_V; بشکن;			      \
	حالت 'v': case_RC_COMMON_getopt_case_v; بشکن;			      \
	حالت 'q': case_RC_COMMON_getopt_case_q; بشکن;			      \
	حالت 'U': case_RC_COMMON_getopt_case_U; بشکن;			      \
	پیشفرض:  case_RC_COMMON_getopt_default; بشکن;

بیرونی ثابت نویسه *applet;
بیرونی ثابت نویسه *extraopts;
بیرونی ثابت نویسه getoptstring[];
بیرونی ثابت ساختار option longopts[];
بیرونی ثابت نویسه * ثابت longopts_help[];
بیرونی ثابت نویسه *usagestring;

تهی set_quiet_options(تهی);
تهی show_version(تهی);
تهی usage(صحیح exit_status);
