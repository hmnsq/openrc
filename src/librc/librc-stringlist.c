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

#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "queue.h"
#অন্তর্ভাৱ "librc.h"
#অন্তর্ভাৱ "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(শূন্য)
{
	RC_STRINGLIST *l = xmalloc(পরিমাণ(*l));
	TAILQ_INIT(l);
	ঘূৰাই l;
}

RC_STRING *
rc_stringlist_add(RC_STRINGLIST *list, ধ্রুৱ ৱর্ণ *মান)
{
	RC_STRING *s = xmalloc(পরিমাণ(*s));

	s->মান = xstrdup(মান);
	TAILQ_ঢোকাও_TAIL(list, s, entries);
	ঘূৰাই s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, ধ্রুৱ ৱর্ণ *মান)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    যদি (strcmp(s->মান, মান) == 0) {
		    errno = EEXIST;
		    ঘূৰাই রিক্ত;
	    }

	ঘূৰাই rc_stringlist_add(list, মান);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, ধ্রুৱ ৱর্ণ *মান)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    যদি (strcmp(s->মান, মান) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    মুক্ত(s->মান);
		    মুক্ত(s);
		    ঘূৰাই true;
	    }

	errno = EEXIST;
	ঘূৰাই false;
}

RC_STRING *
rc_stringlist_find(RC_STRINGLIST *list, ধ্রুৱ ৱর্ণ *মান)
{
	RC_STRING *s;

	যদি (list) {
		TAILQ_FOREACH(s, list, entries)
		    যদি (strcmp(s->মান, মান) == 0)
			    ঘূৰাই s;
	}
	ঘূৰাই রিক্ত;
}

RC_STRINGLIST *
rc_stringlist_split(ধ্রুৱ ৱর্ণ *মান, ধ্রুৱ ৱর্ণ *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	ৱর্ণ *d = xstrdup(মান);
	ৱর্ণ *p = d, *token;

	যেতিয়ালৈ ((token = strsep(&p, sep)))
		rc_stringlist_add(list, token);
	মুক্ত(d);

	ঘূৰাই list;
}

শূন্য
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
		last = রিক্ত;
		TAILQ_FOREACH(n, new, entries) {
			যদি (strcmp(s->মান, n->মান) < 0)
				ভঙ্গ;
			last = n;
		}
		যদি (last)
			TAILQ_ঢোকাও_AFTER(new, last, s, entries);
		অন্যথা
			TAILQ_ঢোকাও_HEAD(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	মুক্ত(l);
	*list = new;
}

শূন্য
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	যদি (!list)
		ঘূৰাই;

	s1 = TAILQ_FIRST(list);
	যেতিয়ালৈ (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		মুক্ত(s1->মান);
		মুক্ত(s1);
		s1 = s2;
	}
	মুক্ত(list);
}
