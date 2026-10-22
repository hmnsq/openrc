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

#अन्तर्भाव <ctype.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क i;
	वर्ण *p;
	पूर्णाङ्क c;

	प्रति (i = 1; i < argc; i++) {
		p = argv[i];
		यदि (i != 1)
			अक्षरं_लिख(' ');
		यावत् (*p) {
			c = (अचिह्नित वर्ण)*p++;
			यदि (!अक्षरम्_अङ्कः_वा(c))
				c = '_';
			अक्षरं_लिख(c);
		}
	}
	अक्षरं_लिख('\n');
	प्रत्यागम EXIT_SUCCESS;
}
