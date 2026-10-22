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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "einfo.h"

पूर्णाङ्क main(शून्य)
{
	अचर वर्ण *p = दो_पर्या("RC_PID");
	पूर्णाङ्क pid;

	यदि (p && माला_पूछो(p, "%d", &pid) == 1) {
		यदि (समाप्त(pid, SIGUSR1) != 0)
			eerrorx("rc-abort: failed to signal parent %d: %s",
			    pid, म_त्रुटि(त्रुटि_सं));
		फर्काऊ निकास_सफल;
	}

	फर्काऊ निकास_त्रुटि;
}
