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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "librc.h"
#અન્તર્ભાવ "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(શૂન્ય)
{
	RC_STRINGLIST *l = xmalloc(કદ(*l));
	TAILQ_INIT(l);
	પાછા l;
}

RC_STRING *
rc_stringlist_add(RC_STRINGLIST *list, અચલ અક્ષર *કિંમત)
{
	RC_STRING *s = xmalloc(કદ(*s));

	s->કિંમત = xstrdup(કિંમત);
	TAILQ_ઉમેરો_TAIL(list, s, entries);
	પાછા s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, અચલ અક્ષર *કિંમત)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    જો (strcmp(s->કિંમત, કિંમત) == 0) {
		    errno = EEXIST;
		    પાછા ખાલી;
	    }

	પાછા rc_stringlist_add(list, કિંમત);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, અચલ અક્ષર *કિંમત)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    જો (strcmp(s->કિંમત, કિંમત) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    મુક્ત(s->કિંમત);
		    મુક્ત(s);
		    પાછા true;
	    }

	errno = EEXIST;
	પાછા false;
}

RC_STRING *
rc_stringlist_find(RC_STRINGLIST *list, અચલ અક્ષર *કિંમત)
{
	RC_STRING *s;

	જો (list) {
		TAILQ_FOREACH(s, list, entries)
		    જો (strcmp(s->કિંમત, કિંમત) == 0)
			    પાછા s;
	}
	પાછા ખાલી;
}

RC_STRINGLIST *
rc_stringlist_split(અચલ અક્ષર *કિંમત, અચલ અક્ષર *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	અક્ષર *d = xstrdup(કિંમત);
	અક્ષર *p = d, *token;

	જ્યારે ((token = strsep(&p, sep)))
		rc_stringlist_add(list, token);
	મુક્ત(d);

	પાછા list;
}

શૂન્ય
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
		last = ખાલી;
		TAILQ_FOREACH(n, new, entries) {
			જો (strcmp(s->કિંમત, n->કિંમત) < 0)
				તોડો;
			last = n;
		}
		જો (last)
			TAILQ_ઉમેરો_AFTER(new, last, s, entries);
		નહીંતર
			TAILQ_ઉમેરો_HEAD(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	મુક્ત(l);
	*list = new;
}

શૂન્ય
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	જો (!list)
		પાછા;

	s1 = TAILQ_FIRST(list);
	જ્યારે (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		મુક્ત(s1->કિંમત);
		મુક્ત(s1);
		s1 = s2;
	}
	મુક્ત(list);
}
