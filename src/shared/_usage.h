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

#అన్తర్భావ <getopt.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ "librc.h"

#నిర్వచించు getoptstring_COMMON "ChqVvU"

#నిర్వచించు longopts_COMMON							      \
	{ "help",           0, ఖాళీ, 'h'},				      \
	{ "nocolor",        0, ఖాళీ, 'C'},				      \
	{ "version",        0, ఖాళీ, 'V'},				      \
	{ "verbose",        0, ఖాళీ, 'v'},				      \
	{ "quiet",          0, ఖాళీ, 'q'},				      \
	{ "user",           0, ఖాళీ, 'U'},				      \
	{ ఖాళీ,             0, ఖాళీ,  0 }

#నిర్వచించు longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#నిర్వచించు case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#నిర్వచించు case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#నిర్వచించు case_RC_COMMON_getopt_case_V  ఒకవేళ (argc == 2) show_సంస్కరణ();
#నిర్వచించు case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#నిర్వచించు case_RC_COMMON_getopt_case_q  సమితి_quiet_options();
#నిర్వచించు case_RC_COMMON_getopt_case_U  rc_సమితి_user();
#నిర్వచించు case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#నిర్వచించు case_RC_COMMON_GETOPT						      \
	స్థితి 'C': case_RC_COMMON_getopt_case_C; ఆపు;			      \
	స్థితి 'h': case_RC_COMMON_getopt_case_h; ఆపు;			      \
	స్థితి 'V': case_RC_COMMON_getopt_case_V; ఆపు;			      \
	స్థితి 'v': case_RC_COMMON_getopt_case_v; ఆపు;			      \
	స్థితి 'q': case_RC_COMMON_getopt_case_q; ఆపు;			      \
	స్థితి 'U': case_RC_COMMON_getopt_case_U; ఆపు;			      \
	అప్రమేయ:  case_RC_COMMON_getopt_default; ఆపు;

బాహ్య స్థిరరాశి అక్షరం *applet;
బాహ్య స్థిరరాశి అక్షరం *extraopts;
బాహ్య స్థిరరాశి అక్షరం getoptstring[];
బాహ్య స్థిరరాశి నిర్మాణం option longopts[];
బాహ్య స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[];
బాహ్య స్థిరరాశి అక్షరం *usagestring;

శూన్య సమితి_quiet_options(శూన్య);
శూన్య show_సంస్కరణ(శూన్య);
శూన్య usage(పూర్ణాంకం exit_status);
