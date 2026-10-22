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

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (argc < 3)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;

	/* This test is perverted - historically the baselayout function
	 * returns 0 on *failure*, which is plain wrong */
	ಗಾಗಿ (i = 2; i < argc; ++i)
		ಆದರೆ (!rc_newer_than(argv[1], argv[i], ಖಾಲಿ, ಖಾಲಿ))
			ಹಿಂತಿರುಗಿ EXIT_SUCCESS;

	ಹಿಂತಿರುಗಿ EXIT_FAILURE;
}
