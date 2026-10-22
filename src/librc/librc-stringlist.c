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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "queue.h"
#అన్తర్భావ "librc.h"
#అన్తర్భావ "helpers.h"

RC_STRINGLIST *
rc_stringlist_new(శూన్య)
{
	RC_STRINGLIST *l = xmalloc(పరిమాణం(*l));
	TAILQ_INIT(l);
	తిరిగి l;
}

RC_STRING *
rc_stringlist_కూడు(RC_STRINGLIST *list, స్థిరరాశి అక్షరం *విలువ)
{
	RC_STRING *s = xmalloc(పరిమాణం(*s));

	s->విలువ = xstrdup(విలువ);
	TAILQ_చొప్పించు_తోక(list, s, entries);
	తిరిగి s;
}

RC_STRING *
rc_stringlist_addu(RC_STRINGLIST *list, స్థిరరాశి అక్షరం *విలువ)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    ఒకవేళ (strcmp(s->విలువ, విలువ) == 0) {
		    errno = EEXIST;
		    తిరిగి ఖాళీ;
	    }

	తిరిగి rc_stringlist_కూడు(list, విలువ);
}

bool
rc_stringlist_delete(RC_STRINGLIST *list, స్థిరరాశి అక్షరం *విలువ)
{
	RC_STRING *s;

	TAILQ_FOREACH(s, list, entries)
	    ఒకవేళ (strcmp(s->విలువ, విలువ) == 0) {
		    TAILQ_REMOVE(list, s, entries);
		    విడుదల(s->విలువ);
		    విడుదల(s);
		    తిరిగి true;
	    }

	errno = EEXIST;
	తిరిగి false;
}

RC_STRING *
rc_stringlist_కనుగొను(RC_STRINGLIST *list, స్థిరరాశి అక్షరం *విలువ)
{
	RC_STRING *s;

	ఒకవేళ (list) {
		TAILQ_FOREACH(s, list, entries)
		    ఒకవేళ (strcmp(s->విలువ, విలువ) == 0)
			    తిరిగి s;
	}
	తిరిగి ఖాళీ;
}

RC_STRINGLIST *
rc_stringlist_విభజించు(స్థిరరాశి అక్షరం *విలువ, స్థిరరాశి అక్షరం *sep)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	అక్షరం *d = xstrdup(విలువ);
	అక్షరం *p = d, *token;

	అయితే ((token = strsep(&p, sep)))
		rc_stringlist_కూడు(list, token);
	విడుదల(d);

	తిరిగి list;
}

శూన్య
rc_stringlist_క్రమీకరించు(RC_STRINGLIST **list)
{
	RC_STRINGLIST *l = *list;
	RC_STRINGLIST *new = rc_stringlist_new();
	RC_STRING *s;
	RC_STRING *sn;
	RC_STRING *n;
	RC_STRING *చివరి;

	TAILQ_FOREACH_సురక్షిత(s, l, entries, sn) {
		TAILQ_REMOVE(l, s, entries);
		చివరి = ఖాళీ;
		TAILQ_FOREACH(n, new, entries) {
			ఒకవేళ (strcmp(s->విలువ, n->విలువ) < 0)
				ఆపు;
			చివరి = n;
		}
		ఒకవేళ (చివరి)
			TAILQ_చొప్పించు_AFTER(new, చివరి, s, entries);
		లేకపోతే
			TAILQ_చొప్పించు_తల(new, s, entries);
	}

	/* Now we've sorted the list, copy across the new head */
	విడుదల(l);
	*list = new;
}

శూన్య
rc_stringlist_free(RC_STRINGLIST *list)
{
	RC_STRING *s1;
	RC_STRING *s2;

	ఒకవేళ (!list)
		తిరిగి;

	s1 = TAILQ_మొదటి(list);
	అయితే (s1) {
		s2 = TAILQ_తదుపరి(s1, entries);
		విడుదల(s1->విలువ);
		విడుదల(s1);
		s1 = s2;
	}
	విడుదల(list);
}
