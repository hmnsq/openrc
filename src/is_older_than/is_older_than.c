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

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक i;

	जर (argc < 3)
		परत निकास_त्रुटि;

	/* This test is perverted - historically the baselayout function
	 * returns 0 on *failure*, which is plain wrong */
	साठी (i = 2; i < argc; ++i)
		जर (!rc_newer_than(argv[1], argv[i], रिक्त, रिक्त))
			परत निकास_सफल;

	परत निकास_त्रुटि;
}
