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

#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "librc.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getoptstring_COMMON "ChqVvU"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ longopts_COMMON							      \
	{ "help",           0, ਨਿਰਾ, 'h'},				      \
	{ "nocolor",        0, ਨਿਰਾ, 'C'},				      \
	{ "version",        0, ਨਿਰਾ, 'V'},				      \
	{ "verbose",        0, ਨਿਰਾ, 'v'},				      \
	{ "quiet",          0, ਨਿਰਾ, 'q'},				      \
	{ "user",           0, ਨਿਰਾ, 'U'},				      \
	{ ਨਿਰਾ,             0, ਨਿਰਾ,  0 }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ case_RC_COMMON_getopt_case_V  ਜੇ (argc == 2) show_ਸੰਸਕਰਣ();
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ case_RC_COMMON_getopt_case_q  ਸੈੱਟ_quiet_options();
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ case_RC_COMMON_getopt_case_U  rc_ਸੈੱਟ_user();
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ case_RC_COMMON_GETOPT						      \
	ਹਾਲਤ 'C': case_RC_COMMON_getopt_case_C; ਰੋਕੋ;			      \
	ਹਾਲਤ 'h': case_RC_COMMON_getopt_case_h; ਰੋਕੋ;			      \
	ਹਾਲਤ 'V': case_RC_COMMON_getopt_case_V; ਰੋਕੋ;			      \
	ਹਾਲਤ 'v': case_RC_COMMON_getopt_case_v; ਰੋਕੋ;			      \
	ਹਾਲਤ 'q': case_RC_COMMON_getopt_case_q; ਰੋਕੋ;			      \
	ਹਾਲਤ 'U': case_RC_COMMON_getopt_case_U; ਰੋਕੋ;			      \
	ਮੂਲ:  case_RC_COMMON_getopt_default; ਰੋਕੋ;

ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *applet;
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *extraopts;
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ getoptstring[];
ਬਾਹਰੀ ਅਚਲ ਬਣਤਰ option longopts[];
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[];
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *usagestring;

ਖਾਲੀ ਸੈੱਟ_quiet_options(ਖਾਲੀ);
ਖਾਲੀ show_ਸੰਸਕਰਣ(ਖਾਲੀ);
ਖਾਲੀ usage(ਪੂਰਨ exit_status);
