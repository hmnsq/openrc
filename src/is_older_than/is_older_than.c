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

#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "rc.h"

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क i;

	यदि (argc < 3)
		प्रत्यागम EXIT_FAILURE;

	/* This test is perverted - historically the baselayout function
	 * returns 0 on *failure*, which is plain wrong */
	प्रति (i = 2; i < argc; ++i)
		यदि (!rc_newer_than(argv[1], argv[i], रिक्त, रिक्त))
			प्रत्यागम EXIT_SUCCESS;

	प्रत्यागम EXIT_FAILURE;
}
