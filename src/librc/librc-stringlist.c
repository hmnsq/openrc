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
	반환 l;
}

RC_STRING *
rc_stringlist_add(RC_STRINGLIST *list, const char *value)
{
	RC_STRING *s = xmalloc(sizeof(*s));

	s->value = xstrdup(value);
	TAILQ_INSERT_TAIL(list, s, entries);
	반환 s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, const char *value)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    만약 (strcmp(s->value, value) == 0) {
		    errno = EEXIST;
		    반환 NULL;
	    }

	반환 rc_stringlist_add(list, value);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, const char *value)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    만약 (strcmp(s->value, value) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    free(s->value);
		    free(s);
		    반환 true;
	    }

	errno = EEXIST;
	반환 false;
}

RC_STRING *
rc_stringlist_find(RC_STRINGLIST *list, const char *value)
{
	RC_STRING *s;

	만약 (list) {
		TAILQ_FOREACH(s, list, entries)
		    만약 (strcmp(s->value, value) == 0)
			    반환 s;
	}
	반환 NULL;
}

RC_STRINGLIST *
rc_stringlist_split(const char *value, const char *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	char *d = xstrdup(value);
	char *p = d, *token;

	동안 ((token = strsep(&p, sep)))
		rc_stringlist_add(list, token);
	free(d);

	반환 list;
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
			만약 (strcmp(s->value, n->value) < 0)
				break;
			last = n;
		}
		만약 (last)
			TAILQ_INSERT_AFTER(new, last, s, entries);
		아니면
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

	만약 (!list)
		반환;

	s1 = TAILQ_FIRST(list);
	동안 (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		free(s1->value);
		free(s1);
		s1 = s2;
	}
	free(list);
}
