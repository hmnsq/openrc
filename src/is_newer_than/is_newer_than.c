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

ཧྲིལ གཙོ(ཧྲིལ argc, var_na **argv)
{
	ཧྲིལ i;

	གལ་ཏེ (argc < 3)
		ཕྱིར EXIT_FAILURE;

	/* This test is correct as it's not present in baselayout */
	ཆེད་དུ (i = 2; i < argc; ++i)
		གལ་ཏེ (!rc_newer_than(argv[1], argv[i], NULL, NULL))
			ཕྱིར EXIT_FAILURE;

	ཕྱིར EXIT_SUCCESS;
}
