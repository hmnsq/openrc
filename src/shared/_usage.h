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

#समावेश <getopt.h>
#समावेश <stdlib.h>
#समावेश "librc.h"

#घोषणा getoptstring_COMMON "ChqVvU"

#घोषणा longopts_COMMON							      \
	{ "help",           0, शून्य, 'h'},				      \
	{ "nocolor",        0, शून्य, 'C'},				      \
	{ "version",        0, शून्य, 'V'},				      \
	{ "verbose",        0, शून्य, 'v'},				      \
	{ "quiet",          0, शून्य, 'q'},				      \
	{ "user",           0, शून्य, 'U'},				      \
	{ शून्य,             0, शून्य,  0 }

#घोषणा longopts_help_COMMON						      \
	"Display this help output",					      \
	"Disable color output",						      \
	"Display software version",			              \
	"Run verbosely",						      \
	"Run quietly (repeat to suppress errors)",         \
	"Run in user mode"

#घोषणा case_RC_COMMON_getopt_case_C  setenv ("EINFO_COLOR", "NO", 1);
#घोषणा case_RC_COMMON_getopt_case_h  usage (निकास_सफल);
#घोषणा case_RC_COMMON_getopt_case_V  अगर (argc == 2) show_version();
#घोषणा case_RC_COMMON_getopt_case_v  setenv ("EINFO_VERBOSE", "YES", 1);
#घोषणा case_RC_COMMON_getopt_case_q  set_quiet_options();
#घोषणा case_RC_COMMON_getopt_case_U  rc_set_user();
#घोषणा case_RC_COMMON_getopt_default usage (निकास_त्रुटि);

#घोषणा case_RC_COMMON_GETOPT						      \
	हाल 'C': case_RC_COMMON_getopt_case_C; अवरोध;			      \
	हाल 'h': case_RC_COMMON_getopt_case_h; अवरोध;			      \
	हाल 'V': case_RC_COMMON_getopt_case_V; अवरोध;			      \
	हाल 'v': case_RC_COMMON_getopt_case_v; अवरोध;			      \
	हाल 'q': case_RC_COMMON_getopt_case_q; अवरोध;			      \
	हाल 'U': case_RC_COMMON_getopt_case_U; अवरोध;			      \
	शेष:  case_RC_COMMON_getopt_default; अवरोध;

बाह्य स्थिर अक्षर *applet;
बाह्य स्थिर अक्षर *extraopts;
बाह्य स्थिर अक्षर getoptstring[];
बाह्य स्थिर काष्ठा option longopts[];
बाह्य स्थिर अक्षर * स्थिर longopts_help[];
बाह्य स्थिर अक्षर *usagestring;

व्योम set_quiet_options(व्योम);
व्योम show_version(व्योम);
व्योम usage(पूर्णांक exit_status);
