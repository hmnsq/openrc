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

#شامل <errno.h>
#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "queue.h"
#شامل "librc.h"
#شامل "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(تهی)
{
	RC_STRINGLIST *l = xmalloc(اندازه(*l));
	TAILQ_INIT(l);
	بازگشت l;
}

RC_STRING *
rc_stringlist_add(RC_STRINGLIST *list, ثابت نویسه *مقدار)
{
	RC_STRING *s = xmalloc(اندازه(*s));

	s->مقدار = xstrdup(مقدار);
	TAILQ_درج_TAIL(list, s, entries);
	بازگشت s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, ثابت نویسه *مقدار)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    اگر (strcmp(s->مقدار, مقدار) == 0) {
		    errno = EEXIST;
		    بازگشت پوچ;
	    }

	بازگشت rc_stringlist_add(list, مقدار);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, ثابت نویسه *مقدار)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    اگر (strcmp(s->مقدار, مقدار) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    آزاد(s->مقدار);
		    آزاد(s);
		    بازگشت true;
	    }

	errno = EEXIST;
	بازگشت false;
}

RC_STRING *
rc_stringlist_find(RC_STRINGLIST *list, ثابت نویسه *مقدار)
{
	RC_STRING *s;

	اگر (list) {
		TAILQ_FOREACH(s, list, entries)
		    اگر (strcmp(s->مقدار, مقدار) == 0)
			    بازگشت s;
	}
	بازگشت پوچ;
}

RC_STRINGLIST *
rc_stringlist_split(ثابت نویسه *مقدار, ثابت نویسه *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	نویسه *d = xstrdup(مقدار);
	نویسه *p = d, *token;

	وقتی ((token = strsep(&p, sep)))
		rc_stringlist_add(list, token);
	آزاد(d);

	بازگشت list;
}

تهی
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
		last = پوچ;
		TAILQ_FOREACH(n, new, entries) {
			اگر (strcmp(s->مقدار, n->مقدار) < 0)
				بشکن;
			last = n;
		}
		اگر (last)
			TAILQ_درج_AFTER(new, last, s, entries);
		وگرنه
			TAILQ_درج_HEAD(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	آزاد(l);
	*list = new;
}

تهی
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	اگر (!list)
		بازگشت;

	s1 = TAILQ_FIRST(list);
	وقتی (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		آزاد(s1->مقدار);
		آزاد(s1);
		s1 = s2;
	}
	آزاد(list);
}
