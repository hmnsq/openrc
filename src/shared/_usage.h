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

#تعریف_کرو getoptstring_COMMON "ChqVvU"

#تعریف_کرو longopts_COMMON							      \
	{ "help",           0, تہی, 'h'},				      \
	{ "nocolor",        0, تہی, 'C'},				      \
	{ "version",        0, تہی, 'V'},				      \
	{ "verbose",        0, تہی, 'v'},				      \
	{ "quiet",          0, تہی, 'q'},				      \
	{ "user",           0, تہی, 'U'},				      \
	{ تہی,             0, تہی,  0 }

#تعریف_کرو longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#تعریف_کرو case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#تعریف_کرو case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#تعریف_کرو case_RC_COMMON_getopt_case_V  اگر (argc == 2) show_نسخہ();
#تعریف_کرو case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#تعریف_کرو case_RC_COMMON_getopt_case_q  سیٹ_quiet_options();
#تعریف_کرو case_RC_COMMON_getopt_case_U  rc_سیٹ_user();
#تعریف_کرو case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#تعریف_کرو case_RC_COMMON_GETOPT						      \
	حالت 'C': case_RC_COMMON_getopt_case_C; توڑو;			      \
	حالت 'h': case_RC_COMMON_getopt_case_h; توڑو;			      \
	حالت 'V': case_RC_COMMON_getopt_case_V; توڑو;			      \
	حالت 'v': case_RC_COMMON_getopt_case_v; توڑو;			      \
	حالت 'q': case_RC_COMMON_getopt_case_q; توڑو;			      \
	حالت 'U': case_RC_COMMON_getopt_case_U; توڑو;			      \
	طےشدہ:  case_RC_COMMON_getopt_default; توڑو;

خارجی ثابت حرف *applet;
خارجی ثابت حرف *extraopts;
خارجی ثابت حرف getoptstring[];
خارجی ثابت ساخت option longopts[];
خارجی ثابت حرف * ثابت longopts_help[];
خارجی ثابت حرف *usagestring;

خالی سیٹ_quiet_options(خالی);
خالی show_نسخہ(خالی);
خالی usage(صحیح exit_status);
