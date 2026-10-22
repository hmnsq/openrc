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

ចំនួនគត់ main(ចំនួនគត់ argc, var_na **argv)
{
	ចំនួនគត់ i;

	ប្រសិនបើ (argc < 3)
		ត្រឡប់ EXIT_FAILURE;

	/* This test is perverted - historically the baselayout function
	 * returns 0 on *failure*, which is plain wrong */
	សម្រាប់ (i = 2; i < argc; ++i)
		ប្រសិនបើ (!rc_newer_than(argv[1], argv[i], NULL, NULL))
			ត្រឡប់ EXIT_SUCCESS;

	ត្រឡប់ EXIT_FAILURE;
}
