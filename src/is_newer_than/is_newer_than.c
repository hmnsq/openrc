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

#ضمن <stdlib.h>

#ضمن "rc.h"

صحيح مکيه(صحيح argc, حرف **argv)
{
	صحيح i;

	إذا (argc < 3)
		أرجع EXIT_FAILURE;

	/* This test is correct as it's not present in baselayout */
	لكل (i = 2; i < argc; ++i)
		إذا (!rc_newer_than(argv[1], argv[i], عدم, عدم))
			أرجع EXIT_FAILURE;

	أرجع EXIT_SUCCESS;
}
