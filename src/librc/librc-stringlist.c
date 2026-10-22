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
rc_stringlist_new(خالی)
{
	RC_STRINGLIST *l = xmalloc(حجم(*l));
	TAILQ_INIT(l);
	واپس l;
}

RC_STRING *
rc_stringlist_جوڑو(RC_STRINGLIST *list, ثابت حرف *قدر)
{
	RC_STRING *s = xmalloc(حجم(*s));

	s->قدر = xstrdup(قدر);
	TAILQ_داخل_کرو_دم(list, s, entries);
	واپس s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, ثابت حرف *قدر)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    اگر (strcmp(s->قدر, قدر) == 0) {
		    errno = EEXIST;
		    واپس تہی;
	    }

	واپس rc_stringlist_جوڑو(list, قدر);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, ثابت حرف *قدر)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    اگر (strcmp(s->قدر, قدر) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    آزاد(s->قدر);
		    آزاد(s);
		    واپس true;
	    }

	errno = EEXIST;
	واپس false;
}

RC_STRING *
rc_stringlist_ڈھونڈو(RC_STRINGLIST *list, ثابت حرف *قدر)
{
	RC_STRING *s;

	اگر (list) {
		TAILQ_FOREACH(s, list, entries)
		    اگر (strcmp(s->قدر, قدر) == 0)
			    واپس s;
	}
	واپس تہی;
}

RC_STRINGLIST *
rc_stringlist_بانٹو(ثابت حرف *قدر, ثابت حرف *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	حرف *d = xstrdup(قدر);
	حرف *p = d, *token;

	جب ((token = strsep(&p, sep)))
		rc_stringlist_جوڑو(list, token);
	آزاد(d);

	واپس list;
}

خالی
rc_stringlist_ترتیب_دو(RC_STRINGLIST **list)
{
	RC_STRINGLIST *l = *list;
	RC_STRINGLIST *new = rc_stringlist_new();
	RC_STRING *s;
	RC_STRING *sn;
	RC_STRING *n;
	RC_STRING *آخری;

	TAILQ_FOREACH_محفوظ(s, l, entries, sn) {
		TAILQ_REMOVE(l, s, entries);
		آخری = تہی;
		TAILQ_FOREACH(n, new, entries) {
			اگر (strcmp(s->قدر, n->قدر) < 0)
				توڑو;
			آخری = n;
		}
		اگر (آخری)
			TAILQ_داخل_کرو_AFTER(new, آخری, s, entries);
		ورنہ
			TAILQ_داخل_کرو_سر(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	آزاد(l);
	*list = new;
}

خالی
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	اگر (!list)
		واپس;

	s1 = TAILQ_پہلا(list);
	جب (s1) {
		s2 = TAILQ_اگلا(s1, entries);
		آزاد(s1->قدر);
		آزاد(s1);
		s1 = s2;
	}
	آزاد(list);
}
