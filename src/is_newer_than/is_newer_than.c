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

#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "rc.h"

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	పూర్ణాంకం i;

	ఒకవేళ (argc < 3)
		తిరిగి EXIT_FAILURE;

	/* This test is correct as it's not present in baselayout */
	కొరకు (i = 2; i < argc; ++i)
		ఒకవేళ (!rc_newer_than(argv[1], argv[i], ఖాళీ, ఖాళీ))
			తిరిగి EXIT_FAILURE;

	తిరిగి EXIT_SUCCESS;
}
