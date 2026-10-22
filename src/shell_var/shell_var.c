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

#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	முழு i;
	எழுத்து *p;
	முழு c;

	க்கு (i = 1; i < argc; i++) {
		p = argv[i];
		எனில் (i != 1)
			putchar(' ');
		போது (*p) {
			c = (unsigned எழுத்து)*p++;
			எனில் (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	திரும்பு EXIT_SUCCESS;
}
