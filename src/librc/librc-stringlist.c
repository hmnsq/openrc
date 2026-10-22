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
rc_stringlist_add(RC_STRINGLIST *list, ثابت حرف *value)
{
	RC_STRING *s = xmalloc(حجم(*s));

	s->value = xstrdup(value);
	TAILQ_INSERT_TAIL(list, s, entries);
	أرجع s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, ثابت حرف *value)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    𐤀𐤌 (strcmp(s->value, value) == 0) {
		    errno = EEXIST;
		    أرجع عدم;
	    }

	أرجع rc_stringlist_add(list, value);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, ثابت حرف *value)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    𐤀𐤌 (strcmp(s->value, value) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    حرر(s->value);
		    حرر(s);
		    أرجع true;
	    }

	errno = EEXIST;
	أرجع false;
}

RC_STRING *
rc_stringlist_find(RC_STRINGLIST *list, ثابت حرف *value)
{
	RC_STRING *s;

	𐤀𐤌 (list) {
		TAILQ_FOREACH(s, list, entries)
		    𐤀𐤌 (strcmp(s->value, value) == 0)
			    أرجع s;
	}
	أرجع عدم;
}

RC_STRINGLIST *
rc_stringlist_split(ثابت حرف *value, ثابت حرف *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	حرف *d = xstrdup(value);
	حرف *p = d, *token;

	طالما ((token = strsep(&p, sep)))
		rc_stringlist_add(list, token);
	حرر(d);

	أرجع list;
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
			𐤀𐤌 (strcmp(s->value, n->value) < 0)
				اكسر;
			last = n;
		}
		𐤀𐤌 (last)
			TAILQ_INSERT_AFTER(new, last, s, entries);
		وإلا
			TAILQ_INSERT_HEAD(new, s, entries);
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

	𐤀𐤌 (!list)
		أرجع;

	s1 = TAILQ_FIRST(list);
	طالما (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		حرر(s1->value);
		حرر(s1);
		s1 = s2;
	}
	حرر(list);
}
