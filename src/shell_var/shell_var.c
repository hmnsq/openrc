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

#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	পূর্ণাঙ্ক i;
	বর্ণ *p;
	পূর্ণাঙ্ক c;

	প্রতি (i = 1; i < argc; i++) {
		p = argv[i];
		যদি (i != 1)
			putchar(' ');
		যাবত্ (*p) {
			c = (unsigned বর্ণ)*p++;
			যদি (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	প্রত্যাগম EXIT_SUCCESS;
}
