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

#অন্তর্ভাৱ <stdlib.h>

#অন্তর্ভাৱ "rc.h"

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
{
	পূর্ণাঙ্ক i;

	যদি (argc < 3)
		ঘূৰাই EXIT_FAILURE;

	/* This test is perverted - historically the baselayout function
	 * returns 0 on *failure*, which is plain wrong */
	বাবে (i = 2; i < argc; ++i)
		যদি (!rc_newer_than(argv[1], argv[i], রিক্ত, রিক্ত))
			ঘূৰাই EXIT_SUCCESS;

	ঘূৰাই EXIT_FAILURE;
}
