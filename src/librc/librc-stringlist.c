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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "librc.h"
#அந்தர்பாவ "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(வெறுமை)
{
	RC_STRINGLIST *l = xmalloc(அளவு(*l));
	TAILQ_INIT(l);
	திரும்பு l;
}

RC_STRING *
rc_stringlist_add(RC_STRINGLIST *list, மாறிலி எழுத்து *மதிப்பு)
{
	RC_STRING *s = xmalloc(அளவு(*s));

	s->மதிப்பு = xstrdup(மதிப்பு);
	TAILQ_செருகு_TAIL(list, s, entries);
	திரும்பு s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, மாறிலி எழுத்து *மதிப்பு)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    எனில் (strcmp(s->மதிப்பு, மதிப்பு) == 0) {
		    errno = EEXIST;
		    திரும்பு வெற்று;
	    }

	திரும்பு rc_stringlist_add(list, மதிப்பு);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, மாறிலி எழுத்து *மதிப்பு)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    எனில் (strcmp(s->மதிப்பு, மதிப்பு) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    விடுவி(s->மதிப்பு);
		    விடுவி(s);
		    திரும்பு true;
	    }

	errno = EEXIST;
	திரும்பு false;
}

RC_STRING *
rc_stringlist_find(RC_STRINGLIST *list, மாறிலி எழுத்து *மதிப்பு)
{
	RC_STRING *s;

	எனில் (list) {
		TAILQ_FOREACH(s, list, entries)
		    எனில் (strcmp(s->மதிப்பு, மதிப்பு) == 0)
			    திரும்பு s;
	}
	திரும்பு வெற்று;
}

RC_STRINGLIST *
rc_stringlist_split(மாறிலி எழுத்து *மதிப்பு, மாறிலி எழுத்து *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	எழுத்து *d = xstrdup(மதிப்பு);
	எழுத்து *p = d, *token;

	போது ((token = strsep(&p, sep)))
		rc_stringlist_add(list, token);
	விடுவி(d);

	திரும்பு list;
}

வெறுமை
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
		last = வெற்று;
		TAILQ_FOREACH(n, new, entries) {
			எனில் (strcmp(s->மதிப்பு, n->மதிப்பு) < 0)
				நிறுத்து;
			last = n;
		}
		எனில் (last)
			TAILQ_செருகு_AFTER(new, last, s, entries);
		இல்லையெனில்
			TAILQ_செருகு_HEAD(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	விடுவி(l);
	*list = new;
}

வெறுமை
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	எனில் (!list)
		திரும்பு;

	s1 = TAILQ_FIRST(list);
	போது (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		விடுவி(s1->மதிப்பு);
		விடுவி(s1);
		s1 = s2;
	}
	விடுவி(list);
}
