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

#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ "librc.h"

#define getoptstring_COMMON "ChqVvU"

#define longopts_COMMON							      \
	{ "help",           0, ખાલી, 'h'},				      \
	{ "nocolor",        0, ખાલી, 'C'},				      \
	{ "version",        0, ખાલી, 'V'},				      \
	{ "verbose",        0, ખાલી, 'v'},				      \
	{ "quiet",          0, ખાલી, 'q'},				      \
	{ "user",           0, ખાલી, 'U'},				      \
	{ ખાલી,             0, ખાલી,  0 }

#define longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#define case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#define case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#define case_RC_COMMON_getopt_case_V  જો (argc == 2) show_version();
#define case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#define case_RC_COMMON_getopt_case_q  set_quiet_options();
#define case_RC_COMMON_getopt_case_U  rc_set_user();
#define case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#define case_RC_COMMON_GETOPT						      \
	સ્થિતિ 'C': case_RC_COMMON_getopt_case_C; તોડો;			      \
	સ્થિતિ 'h': case_RC_COMMON_getopt_case_h; તોડો;			      \
	સ્થિતિ 'V': case_RC_COMMON_getopt_case_V; તોડો;			      \
	સ્થિતિ 'v': case_RC_COMMON_getopt_case_v; તોડો;			      \
	સ્થિતિ 'q': case_RC_COMMON_getopt_case_q; તોડો;			      \
	સ્થિતિ 'U': case_RC_COMMON_getopt_case_U; તોડો;			      \
	મૂળભૂત:  case_RC_COMMON_getopt_default; તોડો;

બાહ્ય અચલ અક્ષર *applet;
બાહ્ય અચલ અક્ષર *extraopts;
બાહ્ય અચલ અક્ષર getoptstring[];
બાહ્ય અચલ સંરચના option longopts[];
બાહ્ય અચલ અક્ષર * અચલ longopts_help[];
બાહ્ય અચલ અક્ષર *usagestring;

શૂન્ય set_quiet_options(શૂન્ય);
શૂન્ય show_version(શૂન્ય);
શૂન્ય usage(પૂર્ણાંક exit_status);
