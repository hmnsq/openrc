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

#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ "librc.h"

#define getoptstring_COMMON "ChqVvU"

#define longopts_COMMON							      \
	{ "help",           0, வெற்று, 'h'},				      \
	{ "nocolor",        0, வெற்று, 'C'},				      \
	{ "version",        0, வெற்று, 'V'},				      \
	{ "verbose",        0, வெற்று, 'v'},				      \
	{ "quiet",          0, வெற்று, 'q'},				      \
	{ "user",           0, வெற்று, 'U'},				      \
	{ வெற்று,             0, வெற்று,  0 }

#define longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#define case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#define case_RC_COMMON_getopt_case_h  usage (EXIT_SUCCESS);
#define case_RC_COMMON_getopt_case_V  எனில் (argc == 2) show_version();
#define case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#define case_RC_COMMON_getopt_case_q  set_quiet_options();
#define case_RC_COMMON_getopt_case_U  rc_set_user();
#define case_RC_COMMON_getopt_default usage (EXIT_FAILURE);

#define case_RC_COMMON_GETOPT						      \
	நிலை 'C': case_RC_COMMON_getopt_case_C; நிறுத்து;			      \
	நிலை 'h': case_RC_COMMON_getopt_case_h; நிறுத்து;			      \
	நிலை 'V': case_RC_COMMON_getopt_case_V; நிறுத்து;			      \
	நிலை 'v': case_RC_COMMON_getopt_case_v; நிறுத்து;			      \
	நிலை 'q': case_RC_COMMON_getopt_case_q; நிறுத்து;			      \
	நிலை 'U': case_RC_COMMON_getopt_case_U; நிறுத்து;			      \
	இயல்பு:  case_RC_COMMON_getopt_default; நிறுத்து;

வெளி மாறிலி எழுத்து *applet;
வெளி மாறிலி எழுத்து *extraopts;
வெளி மாறிலி எழுத்து getoptstring[];
வெளி மாறிலி கட்டமைப்பு option longopts[];
வெளி மாறிலி எழுத்து * மாறிலி longopts_help[];
வெளி மாறிலி எழுத்து *usagestring;

வெறுமை set_quiet_options(வெறுமை);
வெறுமை show_version(வெறுமை);
வெறுமை usage(முழு exit_status);
