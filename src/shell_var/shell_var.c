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

#شامل <ctype.h>
#شامل <stdio.h>
#شامل <stdlib.h>

صحیح مین(صحیح argc, حرف **argv)
{
	صحیح i;
	حرف *p;
	صحیح c;

	برائے (i = 1; i < argc; i++) {
		p = argv[i];
		اگر (i != 1)
			حرف_لکھو(' ');
		جب (*p) {
			c = (غیرعلامتی حرف)*p++;
			اگر (!حرف_یا_ہندسہ(c))
				c = '_';
			حرف_لکھو(c);
		}
	}
	حرف_لکھو('\n');
	واپس EXIT_SUCCESS;
}
