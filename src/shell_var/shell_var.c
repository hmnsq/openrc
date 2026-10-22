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

#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ i;
	അക്ഷരം *p;
	പൂർണ്ണസംഖ്യ c;

	വേണ്ടി (i = 1; i < argc; i++) {
		p = argv[i];
		എങ്കിൽ (i != 1)
			putchar(' ');
		ആകുമ്പോൾ (*p) {
			c = (unsigned അക്ഷരം)*p++;
			എങ്കിൽ (!isalnum(c))
				c = '_';
			putchar(c);
		}
	}
	putchar('\n');
	തിരികെ EXIT_SUCCESS;
}
