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

صحيح main(صحيح argc, حرف **argv)
{
	صحيح i;

	𐡄𐡍 (argc < 3)
		أرجع EXIT_FAILURE;

	/* This test is perverted - historically the baselayout function
	 * returns 0 on *failure*, which is plain wrong */
	لكل (i = 2; i < argc; ++i)
		𐡄𐡍 (!rc_newer_than(argv[1], argv[i], عدم, عدم))
			أرجع EXIT_SUCCESS;

	أرجع EXIT_FAILURE;
}
