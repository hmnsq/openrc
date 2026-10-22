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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਪੂਰਨ i;

	ਜੇ (argc < 3)
		ਵਾਪਸ EXIT_FAILURE;

	/* This test is correct as it's not present in baselayout */
	ਲਈ (i = 2; i < argc; ++i)
		ਜੇ (!rc_newer_than(argv[1], argv[i], ਨਿਰਾ, ਨਿਰਾ))
			ਵਾਪਸ EXIT_FAILURE;

	ਵਾਪਸ EXIT_SUCCESS;
}
