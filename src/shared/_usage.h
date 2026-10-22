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

#ضمن <getopt.h>
#ضمن <stdlib.h>
#ضمن "librc.h"

#وصف_ڪريو getoptstring_COMMON "ChqVvU"

#وصف_ڪريو longopts_COMMON							      \
	{ "help",           0, عدم, 'h'},				      \
	{ "nocolor",        0, عدم, 'C'},				      \
	{ "version",        0, عدم, 'V'},				      \
	{ "verbose",        0, عدم, 'v'},				      \
	{ "quiet",          0, عدم, 'q'},				      \
	{ "user",           0, عدم, 'U'},				      \
	{ عدم,             0, عدم,  0 }

#وصف_ڪريو longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#وصف_ڪريو case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#وصف_ڪريو case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#وصف_ڪريو case_RC_COMMON_getopt_case_V  إذا (argc == 2) show_نسخو();
#وصف_ڪريو case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#وصف_ڪريو case_RC_COMMON_getopt_case_q  سيٽ_quiet_options();
#وصف_ڪريو case_RC_COMMON_getopt_case_U  rc_سيٽ_user();
#وصف_ڪريو case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#وصف_ڪريو case_RC_COMMON_GETOPT						      \
	حالة 'C': case_RC_COMMON_getopt_case_C; اكسر;			      \
	حالة 'h': case_RC_COMMON_getopt_case_h; اكسر;			      \
	حالة 'V': case_RC_COMMON_getopt_case_V; اكسر;			      \
	حالة 'v': case_RC_COMMON_getopt_case_v; اكسر;			      \
	حالة 'q': case_RC_COMMON_getopt_case_q; اكسر;			      \
	حالة 'U': case_RC_COMMON_getopt_case_U; اكسر;			      \
	افتراضي:  case_RC_COMMON_getopt_default; اكسر;

خارجي ثابت حرف *applet;
خارجي ثابت حرف *extraopts;
خارجي ثابت حرف getoptstring[];
خارجي ثابت بنية option longopts[];
خارجي ثابت حرف * ثابت longopts_help[];
خارجي ثابت حرف *usagestring;

فراغ سيٽ_quiet_options(فراغ);
فراغ show_نسخو(فراغ);
فراغ usage(صحيح exit_status);
