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

จำนวนเต็ม หลัก(จำนวนเต็ม argc, var_na **argv)
{
	จำนวนเต็ม i;
	var_na *p;
	จำนวนเต็ม c;

	สำหรับ (i = 1; i < argc; i++) {
		p = argv[i];
		ถ้า (i != 1)
			putchar(' ');
		ขณะ (*p) {
			c = (unsigned var_na)*p++;
			ถ้า (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	คืน EXIT_SUCCESS;
}
