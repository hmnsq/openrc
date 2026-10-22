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

#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ "librc.h"

#define getoptstring_COMMON "ChqVvU"

#define longopts_COMMON							      \
	{ "help",           0, ശൂന്യ, 'h'},				      \
	{ "nocolor",        0, ശൂന്യ, 'C'},				      \
	{ "version",        0, ശൂന്യ, 'V'},				      \
	{ "verbose",        0, ശൂന്യ, 'v'},				      \
	{ "quiet",          0, ശൂന്യ, 'q'},				      \
	{ "user",           0, ശൂന്യ, 'U'},				      \
	{ ശൂന്യ,             0, ശൂന്യ,  0 }

#define longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#define case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#define case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#define case_RC_COMMON_getopt_case_V  എങ്കിൽ (argc == 2) show_version();
#define case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#define case_RC_COMMON_getopt_case_q  set_quiet_options();
#define case_RC_COMMON_getopt_case_U  rc_set_user();
#define case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#define case_RC_COMMON_GETOPT						      \
	അവസ്ഥ 'C': case_RC_COMMON_getopt_case_C; നിർത്തുക;			      \
	അവസ്ഥ 'h': case_RC_COMMON_getopt_case_h; നിർത്തുക;			      \
	അവസ്ഥ 'V': case_RC_COMMON_getopt_case_V; നിർത്തുക;			      \
	അവസ്ഥ 'v': case_RC_COMMON_getopt_case_v; നിർത്തുക;			      \
	അവസ്ഥ 'q': case_RC_COMMON_getopt_case_q; നിർത്തുക;			      \
	അവസ്ഥ 'U': case_RC_COMMON_getopt_case_U; നിർത്തുക;			      \
	സ്ഥിരസ്ഥിതി:  case_RC_COMMON_getopt_default; നിർത്തുക;

ബാഹ്യ സ്ഥിരം അക്ഷരം *applet;
ബാഹ്യ സ്ഥിരം അക്ഷരം *extraopts;
ബാഹ്യ സ്ഥിരം അക്ഷരം getoptstring[];
ബാഹ്യ സ്ഥിരം ഘടന option longopts[];
ബാഹ്യ സ്ഥിരം അക്ഷരം * സ്ഥിരം longopts_help[];
ബാഹ്യ സ്ഥിരം അക്ഷരം *usagestring;

ശൂന്യം set_quiet_options(ശൂന്യം);
ശൂന്യം show_version(ശൂന്യം);
ശൂന്യം usage(പൂർണ്ണസംഖ്യ exit_status);
