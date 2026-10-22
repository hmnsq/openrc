/*
 * Copyright (c) 2016 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "rc.h"

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (argc < 3)
		തിരികെ EXIT_FAILURE;

	/* This test is correct as it's not present in baselayout */
	വേണ്ടി (i = 2; i < argc; ++i)
		എങ്കിൽ (!rc_newer_than(argv[1], argv[i], ശൂന്യ, ശൂന്യ))
			തിരികെ EXIT_FAILURE;

	തിരികെ EXIT_SUCCESS;
}
