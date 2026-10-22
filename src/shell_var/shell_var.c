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

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ i;
	ಅಕ್ಷರ *p;
	ಪೂರ್ಣಾಂಕ c;

	ಗಾಗಿ (i = 1; i < argc; i++) {
		p = argv[i];
		ಆದರೆ (i != 1)
			ಅಕ್ಷರ_ಬರೆ(' ');
		ಆಗ (*p) {
			c = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*p++;
			ಆದರೆ (!ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(c))
				c = '_';
			ಅಕ್ಷರ_ಬರೆ(c);
		}
	}
	ಅಕ್ಷರ_ಬರೆ('\n');
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
