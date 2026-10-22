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

#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ವರ್ಣ *p;
	ಪೂರ್ಣಾಙ್ಕ c;

	ಪ್ರತಿ (i = 1; i < argc; i++) {
		p = argv[i];
		ಯದಿ (i != 1)
			putchar(' ');
		ಯಾವತ್ (*p) {
			c = (unsigned ವರ್ಣ)*p++;
			ಯದಿ (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
