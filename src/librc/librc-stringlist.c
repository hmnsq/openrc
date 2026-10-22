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

#समावेश <errno.h>
#समावेश <stdbool.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "queue.h"
#समावेश "librc.h"
#समावेश "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(व्योम)
{
	RC_STRINGLIST *l = xmalloc(माप(*l));
	TAILQ_INIT(l);
	वापस l;
}

RC_STRING *
rc_stringlist_add(RC_STRINGLIST *list, स्थिर अक्षर *मूल्य)
{
	RC_STRING *s = xmalloc(माप(*s));

	s->मूल्य = xstrdup(मूल्य);
	TAILQ_जोड़ो_पुच्छ(list, s, entries);
	वापस s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, स्थिर अक्षर *मूल्य)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    अगर (म_भेद(s->मूल्य, मूल्य) == 0) {
		    त्रुटि_सं = EEXIST;
		    वापस शून्य;
	    }

	वापस rc_stringlist_add(list, मूल्य);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, स्थिर अक्षर *मूल्य)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    अगर (म_भेद(s->मूल्य, मूल्य) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    मुक्त(s->मूल्य);
		    मुक्त(s);
		    वापस true;
	    }

	त्रुटि_सं = EEXIST;
	वापस false;
}

RC_STRING *
rc_stringlist_find(RC_STRINGLIST *list, स्थिर अक्षर *मूल्य)
{
	RC_STRING *s;

	अगर (list) {
		TAILQ_FOREACH(s, list, entries)
		    अगर (म_भेद(s->मूल्य, मूल्य) == 0)
			    वापस s;
	}
	वापस शून्य;
}

RC_STRINGLIST *
rc_stringlist_split(स्थिर अक्षर *मूल्य, स्थिर अक्षर *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	अक्षर *d = xstrdup(मूल्य);
	अक्षर *p = d, *token;

	जबतक ((token = strsep(&p, sep)))
		rc_stringlist_add(list, token);
	मुक्त(d);

	वापस list;
}

व्योम
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
		last = शून्य;
		TAILQ_FOREACH(n, new, entries) {
			अगर (म_भेद(s->मूल्य, n->मूल्य) < 0)
				अवरोध;
			last = n;
		}
		अगर (last)
			TAILQ_जोड़ो_AFTER(new, last, s, entries);
		अन्यथा
			TAILQ_जोड़ो_शीर्ष(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	मुक्त(l);
	*list = new;
}

व्योम
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	अगर (!list)
		वापस;

	s1 = TAILQ_FIRST(list);
	जबतक (s1) {
		s2 = TAILQ_अग्र(s1, entries);
		मुक्त(s1->मूल्य);
		मुक्त(s1);
		s1 = s2;
	}
	मुक्त(list);
}
