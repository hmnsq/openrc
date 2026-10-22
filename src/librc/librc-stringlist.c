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

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "librc.h"
#include "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(void)
{
	RC_STRINGLIST *l = xmalloc(sizeof(*l));
	TAILQ_INIT(l);
	trả_về l;
}

RC_STRING *
rc_stringlist_add(RC_STRINGLIST *list, const char *value)
{
	RC_STRING *s = xmalloc(sizeof(*s));

	s->value = xstrdup(value);
	TAILQ_INSERT_TAIL(list, s, entries);
	trả_về s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, const char *value)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    nếu (strcmp(s->value, value) == 0) {
		    errno = EEXIST;
		    trả_về NULL;
	    }

	trả_về rc_stringlist_add(list, value);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, const char *value)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    nếu (strcmp(s->value, value) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    free(s->value);
		    free(s);
		    trả_về true;
	    }

	errno = EEXIST;
	trả_về false;
}

RC_STRING *
rc_stringlist_find(RC_STRINGLIST *list, const char *value)
{
	RC_STRING *s;

	nếu (list) {
		TAILQ_FOREACH(s, list, entries)
		    nếu (strcmp(s->value, value) == 0)
			    trả_về s;
	}
	trả_về NULL;
}

RC_STRINGLIST *
rc_stringlist_split(const char *value, const char *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	char *d = xstrdup(value);
	char *p = d, *token;

	trong_khi ((token = strsep(&p, sep)))
		rc_stringlist_add(list, token);
	free(d);

	trả_về list;
}

void
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
		last = NULL;
		TAILQ_FOREACH(n, new, entries) {
			nếu (strcmp(s->value, n->value) < 0)
				break;
			last = n;
		}
		nếu (last)
			TAILQ_INSERT_AFTER(new, last, s, entries);
		không_thì
			TAILQ_INSERT_HEAD(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	free(l);
	*list = new;
}

void
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	nếu (!list)
		trả_về;

	s1 = TAILQ_FIRST(list);
	trong_khi (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		free(s1->value);
		free(s1);
		s1 = s2;
	}
	free(list);
}
