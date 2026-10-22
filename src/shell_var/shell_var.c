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

puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	puur_naa_nka i;
	var_na *p;
	puur_naa_nka c;

	for (i = 1; i < argc; i++) {
		p = argv[i];
		ᨀᨘ (i != 1)
			putchar(' ');
		ᨀᨛ (*p) {
			c = (unsigned var_na)*p++;
			ᨀᨘ (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	ᨀᨘᨒ EXIT_SUCCESS;
}
