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
	إرجاع l;
}

RC_STRING *
rc_stringlist_add(RC_STRINGLIST *list, ثابت حرف *قيمة)
{
	RC_STRING *s = xmalloc(حجم(*s));

	s->قيمة = xstrdup(قيمة);
	TAILQ_أدرج_TAIL(list, s, entries);
	إرجاع s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, ثابت حرف *قيمة)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    إذا (strcmp(s->قيمة, قيمة) == 0) {
		    errno = EEXIST;
		    إرجاع عدم;
	    }

	إرجاع rc_stringlist_add(list, قيمة);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, ثابت حرف *قيمة)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    إذا (strcmp(s->قيمة, قيمة) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    حرر(s->قيمة);
		    حرر(s);
		    إرجاع true;
	    }

	errno = EEXIST;
	إرجاع false;
}

RC_STRING *
rc_stringlist_find(RC_STRINGLIST *list, ثابت حرف *قيمة)
{
	RC_STRING *s;

	إذا (list) {
		TAILQ_FOREACH(s, list, entries)
		    إذا (strcmp(s->قيمة, قيمة) == 0)
			    إرجاع s;
	}
	إرجاع عدم;
}

RC_STRINGLIST *
rc_stringlist_split(ثابت حرف *قيمة, ثابت حرف *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	حرف *d = xstrdup(قيمة);
	حرف *p = d, *token;

	بينما ((token = strsep(&p, sep)))
		rc_stringlist_add(list, token);
	حرر(d);

	إرجاع list;
}

فراغ
rc_stringlist_sort(RC_STRINGLIST **list)
{
	RC_STRINGLIST *l = *list;
	RC_STRINGLIST *new = rc_stringlist_new();
	RC_STRING *s;
	RC_STRING *sn;
	RC_STRING *n;
	RC_STRING *last;

	TAILQ_FOREACH_SAFE(s, l, entries, sn) {
		TAILQ_REMOVE(l, s, entries);
		last = عدم;
		TAILQ_FOREACH(n, new, entries) {
			إذا (strcmp(s->قيمة, n->قيمة) < 0)
				اكسر;
			last = n;
		}
		إذا (last)
			TAILQ_أدرج_AFTER(new, last, s, entries);
		وإلا
			TAILQ_أدرج_HEAD(new, s, entries);
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
		إرجاع;

	s1 = TAILQ_FIRST(list);
	بينما (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		حرر(s1->قيمة);
		حرر(s1);
		s1 = s2;
	}
	حرر(list);
}
