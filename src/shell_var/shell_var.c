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

整数 メインページ(整数 argc, char **argv)
{
	整数 i;
	char *p;
	整数 c;

	ために (i = 1; i < argc; i++) {
		p = argv[i];
		もし (i != 1)
			putchar(' ');
		ながら (*p) {
			c = (unsigned char)*p++;
			もし (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	戻る EXIT_SUCCESS;
}
