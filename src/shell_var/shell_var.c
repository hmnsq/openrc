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

#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
{
	পূর্ণাঙ্ক i;
	ৱর্ণ *p;
	পূর্ণাঙ্ক c;

	বাবে (i = 1; i < argc; i++) {
		p = argv[i];
		যদি (i != 1)
			putchar(' ');
		যেতিয়ালৈ (*p) {
			c = (unsigned ৱর্ণ)*p++;
			যদি (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	ঘূৰাই EXIT_SUCCESS;
}
