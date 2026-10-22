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

정수 main(정수 argc, char **argv)
{
	정수 i;
	char *p;
	정수 c;

	위해 (i = 1; i < argc; i++) {
		p = argv[i];
		만약 (i != 1)
			putchar(' ');
		동안 (*p) {
			c = (unsigned char)*p++;
			만약 (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	반환 EXIT_SUCCESS;
}
