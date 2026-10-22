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

#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "rc.h"

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	পূর্ণাঙ্ক i;

	যদি (argc < 3)
		প্রত্যাগম EXIT_FAILURE;

	/* This test is correct as it's not present in baselayout */
	প্রতি (i = 2; i < argc; ++i)
		যদি (!rc_newer_than(argv[1], argv[i], রিক্ত, রিক্ত))
			প্রত্যাগম EXIT_FAILURE;

	প্রত্যাগম EXIT_SUCCESS;
}
