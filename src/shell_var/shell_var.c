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

පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na **argv)
{
	පූර්ණ_සංඛ්‍යා i;
	var_na *p;
	පූර්ණ_සංඛ්‍යා c;

	සඳහා (i = 1; i < argc; i++) {
		p = argv[i];
		නම් (i != 1)
			putchar(' ');
		අතරතුර (*p) {
			c = (unsigned var_na)*p++;
			නම් (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	ආපසු EXIT_SUCCESS;
}
