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

#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব "librc.h"

#define getoptstring_COMMON "ChqVvU"

#define longopts_COMMON							      \
	{ "help",           0, রিক্ত, 'h'},				      \
	{ "nocolor",        0, রিক্ত, 'C'},				      \
	{ "version",        0, রিক্ত, 'V'},				      \
	{ "verbose",        0, রিক্ত, 'v'},				      \
	{ "quiet",          0, রিক্ত, 'q'},				      \
	{ "user",           0, রিক্ত, 'U'},				      \
	{ রিক্ত,             0, রিক্ত,  0 }

#define longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#define case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#define case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#define case_RC_COMMON_getopt_case_V  যদি (argc == 2) show_version();
#define case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#define case_RC_COMMON_getopt_case_q  set_quiet_options();
#define case_RC_COMMON_getopt_case_U  rc_set_user();
#define case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#define case_RC_COMMON_GETOPT						      \
	পক্ষ 'C': case_RC_COMMON_getopt_case_C; ভঙ্গ;			      \
	পক্ষ 'h': case_RC_COMMON_getopt_case_h; ভঙ্গ;			      \
	পক্ষ 'V': case_RC_COMMON_getopt_case_V; ভঙ্গ;			      \
	পক্ষ 'v': case_RC_COMMON_getopt_case_v; ভঙ্গ;			      \
	পক্ষ 'q': case_RC_COMMON_getopt_case_q; ভঙ্গ;			      \
	পক্ষ 'U': case_RC_COMMON_getopt_case_U; ভঙ্গ;			      \
	স্বভাব:  case_RC_COMMON_getopt_default; ভঙ্গ;

বাহ্য ধ্রুব বর্ণ *applet;
বাহ্য ধ্রুব বর্ণ *extraopts;
বাহ্য ধ্রুব বর্ণ getoptstring[];
বাহ্য ধ্রুব সংরচনা option longopts[];
বাহ্য ধ্রুব বর্ণ * ধ্রুব longopts_help[];
বাহ্য ধ্রুব বর্ণ *usagestring;

শূন্য set_quiet_options(শূন্য);
শূন্য show_version(শূন্য);
শূন্য usage(পূর্ণাঙ্ক exit_status);
