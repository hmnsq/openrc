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

#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "rc.h"

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	முழு i;

	எனில் (argc < 3)
		திரும்பு EXIT_FAILURE;

	/* This test is perverted - historically the baselayout function
	 * returns 0 on *failure*, which is plain wrong */
	க்கு (i = 2; i < argc; ++i)
		எனில் (!rc_newer_than(argv[1], argv[i], வெற்று, வெற்று))
			திரும்பு EXIT_SUCCESS;

	திரும்பு EXIT_FAILURE;
}
