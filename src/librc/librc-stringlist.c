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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "librc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(ਖਾਲੀ)
{
	RC_STRINGLIST *l = xmalloc(ਆਕਾਰ(*l));
	TAILQ_INIT(l);
	ਵਾਪਸ l;
}

RC_STRING *
rc_stringlist_ਜੋੜੋ(RC_STRINGLIST *list, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	RC_STRING *s = xmalloc(ਆਕਾਰ(*s));

	s->ਮੁੱਲ = xstrdup(ਮੁੱਲ);
	TAILQ_ਪਾਓ_ਪੂਛ(list, s, entries);
	ਵਾਪਸ s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    ਜੇ (strcmp(s->ਮੁੱਲ, ਮੁੱਲ) == 0) {
		    errno = EEXIST;
		    ਵਾਪਸ ਨਿਰਾ;
	    }

	ਵਾਪਸ rc_stringlist_ਜੋੜੋ(list, ਮੁੱਲ);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    ਜੇ (strcmp(s->ਮੁੱਲ, ਮੁੱਲ) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    ਮੁਕਤ(s->ਮੁੱਲ);
		    ਮੁਕਤ(s);
		    ਵਾਪਸ true;
	    }

	errno = EEXIST;
	ਵਾਪਸ false;
}

RC_STRING *
rc_stringlist_ਲੱਭੋ(RC_STRINGLIST *list, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	RC_STRING *s;

	ਜੇ (list) {
		TAILQ_FOREACH(s, list, entries)
		    ਜੇ (strcmp(s->ਮੁੱਲ, ਮੁੱਲ) == 0)
			    ਵਾਪਸ s;
	}
	ਵਾਪਸ ਨਿਰਾ;
}

RC_STRINGLIST *
rc_stringlist_ਵੰਡੋ(ਅਚਲ ਅੱਖਰ *ਮੁੱਲ, ਅਚਲ ਅੱਖਰ *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	ਅੱਖਰ *d = xstrdup(ਮੁੱਲ);
	ਅੱਖਰ *p = d, *token;

	ਜਦੋਂ ((token = strsep(&p, sep)))
		rc_stringlist_ਜੋੜੋ(list, token);
	ਮੁਕਤ(d);

	ਵਾਪਸ list;
}

ਖਾਲੀ
rc_stringlist_ਛਾਂਟੋ(RC_STRINGLIST **list)
{
	RC_STRINGLIST *l = *list;
	RC_STRINGLIST *new = rc_stringlist_new();
	RC_STRING *s;
	RC_STRING *sn;
	RC_STRING *n;
	RC_STRING *ਆਖਰੀ;

	TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(s, l, entries, sn) {
		TAILQ_REMOVE(l, s, entries);
		ਆਖਰੀ = ਨਿਰਾ;
		TAILQ_FOREACH(n, new, entries) {
			ਜੇ (strcmp(s->ਮੁੱਲ, n->ਮੁੱਲ) < 0)
				ਰੋਕੋ;
			ਆਖਰੀ = n;
		}
		ਜੇ (ਆਖਰੀ)
			TAILQ_ਪਾਓ_AFTER(new, ਆਖਰੀ, s, entries);
		ਨਹੀਂਤਾਂ
			TAILQ_ਪਾਓ_ਸਿਰ(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	ਮੁਕਤ(l);
	*list = new;
}

ਖਾਲੀ
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	ਜੇ (!list)
		ਵਾਪਸ;

	s1 = TAILQ_ਪਹਿਲਾ(list);
	ਜਦੋਂ (s1) {
		s2 = TAILQ_ਅਗਲਾ(s1, entries);
		ਮੁਕਤ(s1->ਮੁੱਲ);
		ਮੁਕਤ(s1);
		s1 = s2;
	}
	ਮੁਕਤ(list);
}
