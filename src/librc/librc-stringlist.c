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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "librc.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(ಶೂನ್ಯ)
{
	RC_STRINGLIST *l = xmalloc(ಗಾತ್ರ(*l));
	TAILQ_INIT(l);
	ಹಿಂತಿರುಗಿ l;
}

RC_STRING *
rc_stringlist_ಕೂಡಿಸು(RC_STRINGLIST *list, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	RC_STRING *s = xmalloc(ಗಾತ್ರ(*s));

	s->ಮೌಲ್ಯ = xstrdup(ಮೌಲ್ಯ);
	TAILQ_ಸೇರಿಸು_ಬಾಲ(list, s, entries);
	ಹಿಂತಿರುಗಿ s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    ಆದರೆ (strcmp(s->ಮೌಲ್ಯ, ಮೌಲ್ಯ) == 0) {
		    errno = EEXIST;
		    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	    }

	ಹಿಂತಿರುಗಿ rc_stringlist_ಕೂಡಿಸು(list, ಮೌಲ್ಯ);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    ಆದರೆ (strcmp(s->ಮೌಲ್ಯ, ಮೌಲ್ಯ) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    ಬಿಡುಗಡೆ(s->ಮೌಲ್ಯ);
		    ಬಿಡುಗಡೆ(s);
		    ಹಿಂತಿರುಗಿ true;
	    }

	errno = EEXIST;
	ಹಿಂತಿರುಗಿ false;
}

RC_STRING *
rc_stringlist_ಕಂಡುಹಿಡಿ(RC_STRINGLIST *list, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	RC_STRING *s;

	ಆದರೆ (list) {
		TAILQ_FOREACH(s, list, entries)
		    ಆದರೆ (strcmp(s->ಮೌಲ್ಯ, ಮೌಲ್ಯ) == 0)
			    ಹಿಂತಿರುಗಿ s;
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

RC_STRINGLIST *
rc_stringlist_ವಿಭಜಿಸು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	ಅಕ್ಷರ *d = xstrdup(ಮೌಲ್ಯ);
	ಅಕ್ಷರ *p = d, *token;

	ಆಗ ((token = strsep(&p, sep)))
		rc_stringlist_ಕೂಡಿಸು(list, token);
	ಬಿಡುಗಡೆ(d);

	ಹಿಂತಿರುಗಿ list;
}

ಶೂನ್ಯ
rc_stringlist_ವಿಂಗಡಿಸು(RC_STRINGLIST **list)
{
	RC_STRINGLIST *l = *list;
	RC_STRINGLIST *new = rc_stringlist_new();
	RC_STRING *s;
	RC_STRING *sn;
	RC_STRING *n;
	RC_STRING *ಕೊನೆಯ;

	TAILQ_FOREACH_ಸುರಕ್ಷಿತ(s, l, entries, sn) {
		TAILQ_REMOVE(l, s, entries);
		ಕೊನೆಯ = ಖಾಲಿ;
		TAILQ_FOREACH(n, new, entries) {
			ಆದರೆ (strcmp(s->ಮೌಲ್ಯ, n->ಮೌಲ್ಯ) < 0)
				ನಿಲ್ಲಿಸು;
			ಕೊನೆಯ = n;
		}
		ಆದರೆ (ಕೊನೆಯ)
			TAILQ_ಸೇರಿಸು_AFTER(new, ಕೊನೆಯ, s, entries);
		ಇಲ್ಲದಿದ್ದರೆ
			TAILQ_ಸೇರಿಸು_ತಲೆ(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	ಬಿಡುಗಡೆ(l);
	*list = new;
}

ಶೂನ್ಯ
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	ಆದರೆ (!list)
		ಹಿಂತಿರುಗಿ;

	s1 = TAILQ_ಮೊದಲ(list);
	ಆಗ (s1) {
		s2 = TAILQ_ಮುಂದಿನ(s1, entries);
		ಬಿಡುಗಡೆ(s1->ಮೌಲ್ಯ);
		ಬಿಡುಗಡೆ(s1);
		s1 = s2;
	}
	ಬಿಡುಗಡೆ(list);
}
