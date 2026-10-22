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

ཧྲིལ main(ཧྲིལ argc, var_na **argv)
{
	ཧྲིལ i;
	var_na *p;
	ཧྲིལ c;

	དོན་ལུ (i = 1; i < argc; i++) {
		p = argv[i];
		གལ་སྲིད (i != 1)
			putchar(' ');
		སྐབས (*p) {
			c = (unsigned var_na)*p++;
			གལ་སྲིད (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	ལོག EXIT_SUCCESS;
}
