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

#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "rc.h"

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (argc < 3)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;

	/* This test is correct as it's not present in baselayout */
	ಪ್ರತಿ (i = 2; i < argc; ++i)
		ಯದಿ (!rc_newer_than(argv[1], argv[i], ರಿಕ್ತ, ರಿಕ್ತ))
			ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
