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

цяло main(цяло argc, char **argv)
{
	цяло i;

	ако (argc < 3)
		върни EXIT_FAILURE;

	/* This test is correct as it's not present in baselayout */
	за (i = 2; i < argc; ++i)
		ако (!rc_newer_than(argv[1], argv[i], NULL, NULL))
			върни EXIT_FAILURE;

	върни EXIT_SUCCESS;
}
