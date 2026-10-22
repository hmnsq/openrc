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
	{ "help",           0, ফাঁকা, 'h'},				      \
	{ "nocolor",        0, ফাঁকা, 'C'},				      \
	{ "version",        0, ফাঁকা, 'V'},				      \
	{ "verbose",        0, ফাঁকা, 'v'},				      \
	{ "quiet",          0, ফাঁকা, 'q'},				      \
	{ "user",           0, ফাঁকা, 'U'},				      \
	{ ফাঁকা,             0, ফাঁকা,  0 }

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
	ক্ষেত্র 'C': case_RC_COMMON_getopt_case_C; বিরতি;			      \
	ক্ষেত্র 'h': case_RC_COMMON_getopt_case_h; বিরতি;			      \
	ক্ষেত্র 'V': case_RC_COMMON_getopt_case_V; বিরতি;			      \
	ক্ষেত্র 'v': case_RC_COMMON_getopt_case_v; বিরতি;			      \
	ক্ষেত্র 'q': case_RC_COMMON_getopt_case_q; বিরতি;			      \
	ক্ষেত্র 'U': case_RC_COMMON_getopt_case_U; বিরতি;			      \
	পূর্বনির্ধারিত:  case_RC_COMMON_getopt_default; বিরতি;

বাহ্যিক ধ্রুবক অক্ষর *applet;
বাহ্যিক ধ্রুবক অক্ষর *extraopts;
বাহ্যিক ধ্রুবক অক্ষর getoptstring[];
বাহ্যিক ধ্রুবক কাঠামো option longopts[];
বাহ্যিক ধ্রুবক অক্ষর * ধ্রুবক longopts_help[];
বাহ্যিক ধ্রুবক অক্ষর *usagestring;

শূন্য set_quiet_options(শূন্য);
শূন্য show_version(শূন্য);
শূন্য usage(পূর্ণসংখ্যা exit_status);
