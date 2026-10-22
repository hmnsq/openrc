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

#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਪੂਰਨ i;
	ਅੱਖਰ *p;
	ਪੂਰਨ c;

	ਲਈ (i = 1; i < argc; i++) {
		p = argv[i];
		ਜੇ (i != 1)
			ਅੱਖਰ_ਲਿਖੋ(' ');
		ਜਦੋਂ (*p) {
			c = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*p++;
			ਜੇ (!ਅੱਖਰ_ਜਾਂ_ਅੰਕ(c))
				c = '_';
			ਅੱਖਰ_ਲਿਖੋ(c);
		}
	}
	ਅੱਖਰ_ਲਿਖੋ('\n');
	ਵਾਪਸ EXIT_SUCCESS;
}
