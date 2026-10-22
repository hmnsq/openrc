/*
 * librc-strlist.h
 * String list functions to make using queue(3) a little easier.
*/

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

#ضمن <errno.h>
#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "queue.h"
#ضمن "librc.h"
#ضمن "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(فراغ)
{
	RC_STRINGLIST *l = xmalloc(حجم(*l));
	TAILQ_INIT(l);
	أرجع l;
}

RC_STRING *
rc_stringlist_جمع_کن(RC_STRINGLIST *list, ثابت حرف *مقدار)
{
	RC_STRING *s = xmalloc(حجم(*s));

	s->مقدار = xstrdup(مقدار);
	TAILQ_درج_دم(list, s, entries);
	أرجع s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, ثابت حرف *مقدار)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    إذا (strcmp(s->مقدار, مقدار) == 0) {
		    errno = EEXIST;
		    أرجع عدم;
	    }

	أرجع rc_stringlist_جمع_کن(list, مقدار);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, ثابت حرف *مقدار)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    إذا (strcmp(s->مقدار, مقدار) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    حرر(s->مقدار);
		    حرر(s);
		    أرجع true;
	    }

	errno = EEXIST;
	أرجع false;
}

RC_STRING *
rc_stringlist_پیدا_کن(RC_STRINGLIST *list, ثابت حرف *مقدار)
{
	RC_STRING *s;

	إذا (list) {
		TAILQ_FOREACH(s, list, entries)
		    إذا (strcmp(s->مقدار, مقدار) == 0)
			    أرجع s;
	}
	أرجع عدم;
}

RC_STRINGLIST *
rc_stringlist_تقسیم(ثابت حرف *مقدار, ثابت حرف *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	حرف *d = xstrdup(مقدار);
	حرف *p = d, *token;

	طالما ((token = strsep(&p, sep)))
		rc_stringlist_جمع_کن(list, token);
	حرر(d);

	أرجع list;
}

فراغ
rc_stringlist_مرتب_سازی(RC_STRINGLIST **list)
{
	RC_STRINGLIST *l = *list;
	RC_STRINGLIST *new = rc_stringlist_new();
	RC_STRING *s;
	RC_STRING *sn;
	RC_STRING *n;
	RC_STRING *آخرین;

	TAILQ_FOREACH_امن(s, l, entries, sn) {
		TAILQ_REMOVE(l, s, entries);
		آخرین = عدم;
		TAILQ_FOREACH(n, new, entries) {
			إذا (strcmp(s->مقدار, n->مقدار) < 0)
				اكسر;
			آخرین = n;
		}
		إذا (آخرین)
			TAILQ_درج_AFTER(new, آخرین, s, entries);
		وإلا
			TAILQ_درج_سر(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	حرر(l);
	*list = new;
}

فراغ
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	إذا (!list)
		أرجع;

	s1 = TAILQ_اولی(list);
	طالما (s1) {
		s2 = TAILQ_بعدی(s1, entries);
		حرر(s1->مقدار);
		حرر(s1);
		s1 = s2;
	}
	حرر(list);
}
