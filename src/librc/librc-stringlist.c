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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "librc.h"
#അന്തര്ഭാവ "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(ശൂന്യം)
{
	RC_STRINGLIST *l = xmalloc(വലുപ്പം(*l));
	TAILQ_INIT(l);
	തിരികെ l;
}

RC_STRING *
rc_stringlist_add(RC_STRINGLIST *list, സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	RC_STRING *s = xmalloc(വലുപ്പം(*s));

	s->മൂല്യം = xstrdup(മൂല്യം);
	TAILQ_തിരുകുക_TAIL(list, s, entries);
	തിരികെ s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    എങ്കിൽ (strcmp(s->മൂല്യം, മൂല്യം) == 0) {
		    errno = EEXIST;
		    തിരികെ ശൂന്യ;
	    }

	തിരികെ rc_stringlist_add(list, മൂല്യം);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    എങ്കിൽ (strcmp(s->മൂല്യം, മൂല്യം) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    സ്വതന്ത്രം(s->മൂല്യം);
		    സ്വതന്ത്രം(s);
		    തിരികെ true;
	    }

	errno = EEXIST;
	തിരികെ false;
}

RC_STRING *
rc_stringlist_find(RC_STRINGLIST *list, സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	RC_STRING *s;

	എങ്കിൽ (list) {
		TAILQ_FOREACH(s, list, entries)
		    എങ്കിൽ (strcmp(s->മൂല്യം, മൂല്യം) == 0)
			    തിരികെ s;
	}
	തിരികെ ശൂന്യ;
}

RC_STRINGLIST *
rc_stringlist_split(സ്ഥിരം അക്ഷരം *മൂല്യം, സ്ഥിരം അക്ഷരം *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	അക്ഷരം *d = xstrdup(മൂല്യം);
	അക്ഷരം *p = d, *token;

	ആകുമ്പോൾ ((token = strsep(&p, sep)))
		rc_stringlist_add(list, token);
	സ്വതന്ത്രം(d);

	തിരികെ list;
}

ശൂന്യം
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
		last = ശൂന്യ;
		TAILQ_FOREACH(n, new, entries) {
			എങ്കിൽ (strcmp(s->മൂല്യം, n->മൂല്യം) < 0)
				നിർത്തുക;
			last = n;
		}
		എങ്കിൽ (last)
			TAILQ_തിരുകുക_AFTER(new, last, s, entries);
		അല്ലെങ്കിൽ
			TAILQ_തിരുകുക_HEAD(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	സ്വതന്ത്രം(l);
	*list = new;
}

ശൂന്യം
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	എങ്കിൽ (!list)
		തിരികെ;

	s1 = TAILQ_FIRST(list);
	ആകുമ്പോൾ (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		സ്വതന്ത്രം(s1->മൂല്യം);
		സ്വതന്ത്രം(s1);
		s1 = s2;
	}
	സ്വതന്ത്രം(list);
}
