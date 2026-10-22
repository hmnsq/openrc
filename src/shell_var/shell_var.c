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

#समावेश <ctype.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक i;
	अक्षर *p;
	पूर्णांक c;

	क्रम (i = 1; i < argc; i++) {
		p = argv[i];
		अगर (i != 1)
			अक्षर_दो(' ');
		जबतक (*p) {
			c = (अचिन्हित अक्षर)*p++;
			अगर (!है_अक्षर_अंक(c))
				c = '_';
			अक्षर_दो(c);
		}
	}
	अक्षर_दो('\n');
	वापस निकास_सफल;
}
