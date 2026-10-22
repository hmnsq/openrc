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

#include <getopt.h>
#include <stdlib.h>
#include "librc.h"

#define getoptstring_COMMON "ChqVvU"

#define longopts_COMMON							      \
	{ "help",           0, NULL, 'h'},				      \
	{ "nocolor",        0, NULL, 'C'},				      \
	{ "version",        0, NULL, 'V'},				      \
	{ "verbose",        0, NULL, 'v'},				      \
	{ "quiet",          0, NULL, 'q'},				      \
	{ "user",           0, NULL, 'U'},				      \
	{ NULL,             0, NULL,  0 }

#define longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#define case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#define case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#define case_RC_COMMON_getopt_case_V  ꦪꦼꦤ꧀ (argc == 2) show_version();
#define case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#define case_RC_COMMON_getopt_case_q  set_quiet_options();
#define case_RC_COMMON_getopt_case_U  rc_set_user();
#define case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#define case_RC_COMMON_GETOPT						      \
	pak_sa 'C': case_RC_COMMON_getopt_case_C; bha_nga;			      \
	pak_sa 'h': case_RC_COMMON_getopt_case_h; bha_nga;			      \
	pak_sa 'V': case_RC_COMMON_getopt_case_V; bha_nga;			      \
	pak_sa 'v': case_RC_COMMON_getopt_case_v; bha_nga;			      \
	pak_sa 'q': case_RC_COMMON_getopt_case_q; bha_nga;			      \
	pak_sa 'U': case_RC_COMMON_getopt_case_U; bha_nga;			      \
	default:  case_RC_COMMON_getopt_default; bha_nga;

extern const var_na *applet;
extern const var_na *extraopts;
extern const var_na getoptstring[];
extern const sa_nracanaa option longopts[];
extern const var_na * const longopts_help[];
extern const var_na *usagestring;

void set_quiet_options(void);
void show_version(void);
void usage(puur_naa_nka exit_status);
