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

#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ "librc.h"

#define getoptstring_COMMON "ChqVvU"

#define longopts_COMMON							      \
	{ "help",           0, ଖାଲି, 'h'},				      \
	{ "nocolor",        0, ଖାଲି, 'C'},				      \
	{ "version",        0, ଖାଲି, 'V'},				      \
	{ "verbose",        0, ଖାଲି, 'v'},				      \
	{ "quiet",          0, ଖାଲି, 'q'},				      \
	{ "user",           0, ଖାଲି, 'U'},				      \
	{ ଖାଲି,             0, ଖାଲି,  0 }

#define longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#define case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#define case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#define case_RC_COMMON_getopt_case_V  ଯଦି (argc == 2) show_version();
#define case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#define case_RC_COMMON_getopt_case_q  set_quiet_options();
#define case_RC_COMMON_getopt_case_U  rc_set_user();
#define case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#define case_RC_COMMON_GETOPT						      \
	ସ୍ଥିତି 'C': case_RC_COMMON_getopt_case_C; ଭାଙ୍ଗ;			      \
	ସ୍ଥିତି 'h': case_RC_COMMON_getopt_case_h; ଭାଙ୍ଗ;			      \
	ସ୍ଥିତି 'V': case_RC_COMMON_getopt_case_V; ଭାଙ୍ଗ;			      \
	ସ୍ଥିତି 'v': case_RC_COMMON_getopt_case_v; ଭାଙ୍ଗ;			      \
	ସ୍ଥିତି 'q': case_RC_COMMON_getopt_case_q; ଭାଙ୍ଗ;			      \
	ସ୍ଥିତି 'U': case_RC_COMMON_getopt_case_U; ଭାଙ୍ଗ;			      \
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:  case_RC_COMMON_getopt_default; ଭାଙ୍ଗ;

ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *applet;
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *extraopts;
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର getoptstring[];
ବାହ୍ୟ ଅଚଳ ସଂରଚନା option longopts[];
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[];
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *usagestring;

ଶୂନ୍ୟ set_quiet_options(ଶୂନ୍ୟ);
ଶୂନ୍ୟ show_version(ଶୂନ୍ୟ);
ଶୂନ୍ୟ usage(ପୂର୍ଣ୍ଣସଂଖ୍ୟା exit_status);
