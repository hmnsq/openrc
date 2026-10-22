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

puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	puur_naa_nka i;

	ပေႃး (argc < 3)
		ပွၵ်ႈ EXIT_FAILURE;

	/* This test is perverted - historically the baselayout function
	 * returns 0 on *failure*, which is plain wrong */
	for (i = 2; i < argc; ++i)
		ပေႃး (!rc_newer_than(argv[1], argv[i], NULL, NULL))
			ပွၵ်ႈ EXIT_SUCCESS;

	ပွၵ်ႈ EXIT_FAILURE;
}
