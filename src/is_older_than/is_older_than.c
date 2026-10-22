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

#شامل <stdlib.h>

#شامل "rc.h"

صحیح اصلی(صحیح argc, نویسه **argv)
{
	صحیح i;

	اگر (argc < 3)
		بازگشت EXIT_FAILURE;

	/* This test is perverted - historically the baselayout function
	 * returns 0 on *failure*, which is plain wrong */
	برای (i = 2; i < argc; ++i)
		اگر (!rc_newer_than(argv[1], argv[i], پوچ, پوچ))
			بازگشت EXIT_SUCCESS;

	بازگشت EXIT_FAILURE;
}
