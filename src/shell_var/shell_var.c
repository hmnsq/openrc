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

#ضمن <ctype.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>

صحيح اصلی(صحيح argc, حرف **argv)
{
	صحيح i;
	حرف *p;
	صحيح c;

	لكل (i = 1; i < argc; i++) {
		p = argv[i];
		إذا (i != 1)
			حرف_چاپ_کن(' ');
		طالما (*p) {
			c = (بیعلامت حرف)*p++;
			إذا (!آیا_حرف_یا_رقم(c))
				c = '_';
			حرف_چاپ_کن(c);
		}
	}
	حرف_چاپ_کن('\n');
	أرجع EXIT_SUCCESS;
}
