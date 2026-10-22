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

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	পূর্ণসংখ্যা i;
	অক্ষর *p;
	পূর্ণসংখ্যা c;

	জন্য (i = 1; i < argc; i++) {
		p = argv[i];
		যদি (i != 1)
			putchar(' ');
		যখন (*p) {
			c = (unsigned অক্ষর)*p++;
			যদি (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	ফেরত EXIT_SUCCESS;
}
