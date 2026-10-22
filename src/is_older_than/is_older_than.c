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

#include <stdlib.h>

#include "rc.h"

số_nguyên main(số_nguyên argc, char **argv)
{
	số_nguyên i;

	nếu (argc < 3)
		trả_về EXIT_FAILURE;

	/* This test is perverted - historically the baselayout function
	 * returns 0 on *failure*, which is plain wrong */
	cho (i = 2; i < argc; ++i)
		nếu (!rc_newer_than(argv[1], argv[i], NULL, NULL))
			trả_về EXIT_SUCCESS;

	trả_về EXIT_FAILURE;
}
