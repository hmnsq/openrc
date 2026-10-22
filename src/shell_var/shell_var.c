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

#ضمن <ctype.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>

صحيح main(صحيح argc, حرف **argv)
{
	صحيح i;
	حرف *p;
	صحيح c;

	لكل (i = 1; i < argc; i++) {
		p = argv[i];
		ܐܢ (i != 1)
			putchar(' ');
		ܟܡܐ (*p) {
			c = (unsigned حرف)*p++;
			ܐܢ (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	ܦܢܐ EXIT_SUCCESS;
}
