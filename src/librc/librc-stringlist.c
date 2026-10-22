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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "librc.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(ಶೂನ್ಯ)
{
	RC_STRINGLIST *l = xmalloc(ಪರಿಮಾಣ(*l));
	TAILQ_INIT(l);
	ಪ್ರತ್ಯಾಗಮ l;
}

RC_STRING *
rc_stringlist_add(RC_STRINGLIST *list, ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	RC_STRING *s = xmalloc(ಪರಿಮಾಣ(*s));

	s->ಮೌಲ್ಯ = xstrdup(ಮೌಲ್ಯ);
	TAILQ_ಸೇರಿಸು_TAIL(list, s, entries);
	ಪ್ರತ್ಯಾಗಮ s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    ಯದಿ (strcmp(s->ಮೌಲ್ಯ, ಮೌಲ್ಯ) == 0) {
		    errno = EEXIST;
		    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	    }

	ಪ್ರತ್ಯಾಗಮ rc_stringlist_add(list, ಮೌಲ್ಯ);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    ಯದಿ (strcmp(s->ಮೌಲ್ಯ, ಮೌಲ್ಯ) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    ಮುಕ್ತ(s->ಮೌಲ್ಯ);
		    ಮುಕ್ತ(s);
		    ಪ್ರತ್ಯಾಗಮ true;
	    }

	errno = EEXIST;
	ಪ್ರತ್ಯಾಗಮ false;
}

RC_STRING *
rc_stringlist_find(RC_STRINGLIST *list, ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	RC_STRING *s;

	ಯದಿ (list) {
		TAILQ_FOREACH(s, list, entries)
		    ಯದಿ (strcmp(s->ಮೌಲ್ಯ, ಮೌಲ್ಯ) == 0)
			    ಪ್ರತ್ಯಾಗಮ s;
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

RC_STRINGLIST *
rc_stringlist_split(ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ, ಧ್ರುವ ವರ್ಣ *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	ವರ್ಣ *d = xstrdup(ಮೌಲ್ಯ);
	ವರ್ಣ *p = d, *token;

	ಯಾವತ್ ((token = strsep(&p, sep)))
		rc_stringlist_add(list, token);
	ಮುಕ್ತ(d);

	ಪ್ರತ್ಯಾಗಮ list;
}

ಶೂನ್ಯ
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
		last = ರಿಕ್ತ;
		TAILQ_FOREACH(n, new, entries) {
			ಯದಿ (strcmp(s->ಮೌಲ್ಯ, n->ಮೌಲ್ಯ) < 0)
				ಭಙ್ಗ;
			last = n;
		}
		ಯದಿ (last)
			TAILQ_ಸೇರಿಸು_AFTER(new, last, s, entries);
		ಅನ್ಯಥಾ
			TAILQ_ಸೇರಿಸು_HEAD(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	ಮುಕ್ತ(l);
	*list = new;
}

ಶೂನ್ಯ
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	ಯದಿ (!list)
		ಪ್ರತ್ಯಾಗಮ;

	s1 = TAILQ_FIRST(list);
	ಯಾವತ್ (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		ಮುಕ್ತ(s1->ಮೌಲ್ಯ);
		ಮುಕ್ತ(s1);
		s1 = s2;
	}
	ಮುಕ್ತ(list);
}
