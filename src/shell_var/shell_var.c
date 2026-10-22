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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

ціле main(ціле argc, char **argv)
{
	ціле i;
	char *p;
	ціле c;

	для (i = 1; i < argc; i++) {
		p = argv[i];
		якщо (i != 1)
			putchar(' ');
		поки (*p) {
			c = (unsigned char)*p++;
			якщо (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	повернути EXIT_SUCCESS;
}
