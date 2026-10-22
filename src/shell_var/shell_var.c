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

#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	પૂર્ણાંક i;
	અક્ષર *p;
	પૂર્ણાંક c;

	માટે (i = 1; i < argc; i++) {
		p = argv[i];
		જો (i != 1)
			putchar(' ');
		જ્યારે (*p) {
			c = (unsigned અક્ષર)*p++;
			જો (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	પાછા EXIT_SUCCESS;
}
