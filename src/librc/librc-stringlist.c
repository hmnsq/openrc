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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "queue.h"
#अन्तर्भाव "librc.h"
#अन्तर्भाव "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(शून्य)
{
	RC_STRINGLIST *l = xmalloc(परिमाण(*l));
	TAILQ_INIT(l);
	फर्काऊ l;
}

RC_STRING *
rc_stringlist_add(RC_STRINGLIST *list, अचर वर्ण *मान)
{
	RC_STRING *s = xmalloc(परिमाण(*s));

	s->मान = xstrdup(मान);
	TAILQ_घुसाउ_TAIL(list, s, entries);
	फर्काऊ s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, अचर वर्ण *मान)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    यदि (म_भेद(s->मान, मान) == 0) {
		    त्रुटि_सं = EEXIST;
		    फर्काऊ रिक्त;
	    }

	फर्काऊ rc_stringlist_add(list, मान);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, अचर वर्ण *मान)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    यदि (म_भेद(s->मान, मान) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    मुक्त(s->मान);
		    मुक्त(s);
		    फर्काऊ true;
	    }

	त्रुटि_सं = EEXIST;
	फर्काऊ false;
}

RC_STRING *
rc_stringlist_find(RC_STRINGLIST *list, अचर वर्ण *मान)
{
	RC_STRING *s;

	यदि (list) {
		TAILQ_FOREACH(s, list, entries)
		    यदि (म_भेद(s->मान, मान) == 0)
			    फर्काऊ s;
	}
	फर्काऊ रिक्त;
}

RC_STRINGLIST *
rc_stringlist_split(अचर वर्ण *मान, अचर वर्ण *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	वर्ण *d = xstrdup(मान);
	वर्ण *p = d, *token;

	जबसम्म ((token = strsep(&p, sep)))
		rc_stringlist_add(list, token);
	मुक्त(d);

	फर्काऊ list;
}

शून्य
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
		last = रिक्त;
		TAILQ_FOREACH(n, new, entries) {
			यदि (म_भेद(s->मान, n->मान) < 0)
				रोक;
			last = n;
		}
		यदि (last)
			TAILQ_घुसाउ_AFTER(new, last, s, entries);
		अन्यथा
			TAILQ_घुसाउ_HEAD(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	मुक्त(l);
	*list = new;
}

शून्य
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	यदि (!list)
		फर्काऊ;

	s1 = TAILQ_FIRST(list);
	जबसम्म (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		मुक्त(s1->मान);
		मुक्त(s1);
		s1 = s2;
	}
	मुक्त(list);
}
