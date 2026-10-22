/*
 * Copyright (c) 2007-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#అన్తర్భావ <ctype.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	పూర్ణాంకం i;
	అక్షరం *p;
	పూర్ణాంకం c;

	కొరకు (i = 1; i < argc; i++) {
		p = argv[i];
		ఒకవేళ (i != 1)
			అక్షరం_రాయి(' ');
		అయితే (*p) {
			c = (గుర్తులేని అక్షరం)*p++;
			ఒకవేళ (!అక్షరమా_అంకెనా(c))
				c = '_';
			అక్షరం_రాయి(c);
		}
	}
	అక్షరం_రాయి('\n');
	తిరిగి EXIT_SUCCESS;
}
