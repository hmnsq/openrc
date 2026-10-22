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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "librc.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(ଶୂନ୍ୟ)
{
	RC_STRINGLIST *l = xmalloc(ଆକାର(*l));
	TAILQ_INIT(l);
	ଫେରନ୍ତୁ l;
}

RC_STRING *
rc_stringlist_add(RC_STRINGLIST *list, ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	RC_STRING *s = xmalloc(ଆକାର(*s));

	s->ମୂଲ୍ୟ = xstrdup(ମୂଲ୍ୟ);
	TAILQ_ଭର୍ତ୍ତି_TAIL(list, s, entries);
	ଫେରନ୍ତୁ s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    ଯଦି (strcmp(s->ମୂଲ୍ୟ, ମୂଲ୍ୟ) == 0) {
		    errno = EEXIST;
		    ଫେରନ୍ତୁ ଖାଲି;
	    }

	ଫେରନ୍ତୁ rc_stringlist_add(list, ମୂଲ୍ୟ);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    ଯଦି (strcmp(s->ମୂଲ୍ୟ, ମୂଲ୍ୟ) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    ମୁକ୍ତ(s->ମୂଲ୍ୟ);
		    ମୁକ୍ତ(s);
		    ଫେରନ୍ତୁ true;
	    }

	errno = EEXIST;
	ଫେରନ୍ତୁ false;
}

RC_STRING *
rc_stringlist_find(RC_STRINGLIST *list, ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	RC_STRING *s;

	ଯଦି (list) {
		TAILQ_FOREACH(s, list, entries)
		    ଯଦି (strcmp(s->ମୂଲ୍ୟ, ମୂଲ୍ୟ) == 0)
			    ଫେରନ୍ତୁ s;
	}
	ଫେରନ୍ତୁ ଖାଲି;
}

RC_STRINGLIST *
rc_stringlist_split(ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ, ଅଚଳ ଅକ୍ଷର *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	ଅକ୍ଷର *d = xstrdup(ମୂଲ୍ୟ);
	ଅକ୍ଷର *p = d, *token;

	ଯେତେବେଳେ ((token = strsep(&p, sep)))
		rc_stringlist_add(list, token);
	ମୁକ୍ତ(d);

	ଫେରନ୍ତୁ list;
}

ଶୂନ୍ୟ
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
		last = ଖାଲି;
		TAILQ_FOREACH(n, new, entries) {
			ଯଦି (strcmp(s->ମୂଲ୍ୟ, n->ମୂଲ୍ୟ) < 0)
				ଭାଙ୍ଗ;
			last = n;
		}
		ଯଦି (last)
			TAILQ_ଭର୍ତ୍ତି_AFTER(new, last, s, entries);
		ଅନ୍ୟଥା
			TAILQ_ଭର୍ତ୍ତି_HEAD(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	ମୁକ୍ତ(l);
	*list = new;
}

ଶୂନ୍ୟ
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	ଯଦି (!list)
		ଫେରନ୍ତୁ;

	s1 = TAILQ_FIRST(list);
	ଯେତେବେଳେ (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		ମୁକ୍ତ(s1->ମୂଲ୍ୟ);
		ମୁକ୍ତ(s1);
		s1 = s2;
	}
	ମୁକ୍ତ(list);
}
