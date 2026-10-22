/*	$NetBSD: queue.h,v 1.67 2014/05/17 21:22:56 rmind Exp $	*/

/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)queue.h	8.5 (Berkeley) 8/20/94
 */

#अगर_अघोषित	_SYS_QUEUE_H_
#घोषणा	_SYS_QUEUE_H_

/*
 * This file defines five types of data structures: singly-linked lists,
 * lists, simple queues, tail queues, and circular queues.
 *
 * A singly-linked list is headed by a single forward pointer. The
 * elements are singly linked for minimum space and pointer manipulation
 * overhead at the expense of O(n) removal for arbitrary elements. New
 * elements can be added to the list after an existing element or at the
 * head of the list.  Elements being removed from the head of the list
 * should use the explicit macro for this purpose for optimum
 * efficiency. A singly-linked list may only be traversed in the forward
 * direction.  Singly-linked lists are ideal for applications with large
 * datasets and few or no removals or for implementing a LIFO queue.
 *
 * A list is headed by a single forward pointer (or an array of forward
 * pointers for a hash table header). The elements are doubly linked
 * so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before
 * or after an existing element or at the head of the list. A list
 * may only be traversed in the forward direction.
 *
 * A simple queue is headed by a pair of pointers, one the head of the
 * list and the other to the tail of the list. The elements are singly
 * linked to save space, so elements can only be removed from the
 * head of the list. New elements can be added to the list after
 * an existing element, at the head of the list, or at the end of the
 * list. A simple queue may only be traversed in the forward direction.
 *
 * A tail queue is headed by a pair of pointers, one to the head of the
 * list and the other to the tail of the list. The elements are doubly
 * linked so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before or
 * after an existing element, at the head of the list, or at the end of
 * the list. A tail queue may be traversed in either direction.
 *
 * A circle queue is headed by a pair of pointers, one to the head of the
 * list and the other to the tail of the list. The elements are doubly
 * linked so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before or after
 * an existing element, at the head of the list, or at the end of the list.
 * A circle queue may be traversed in either direction, but has a more
 * complex end of list detection.
 *
 * For details on the use of these macros, see the queue(3) manual page.
 */

/*
 * Include the definition of NULL only on NetBSD because sys/null.h
 * is not available elsewhere.  This conditional makes the header
 * portable and it can simply be dropped verbatim into any system.
 * The caveat is that on other systems some other header
 * must provide NULL before the macros can be used.
 */
#अगर_घोषित __NetBSD__
#समावेश <sys/null.h>
#पूर्ण_अगर

#अगर defined(QUEUEDEBUG)
# अगर defined(_KERNEL)
#  define QUEUEDEBUG_ABORT(...) panic(__VA_ARGS__)
# अन्यथा
#  include <err.h>
#  define QUEUEDEBUG_ABORT(...) err(1, __VA_ARGS__)
# endif
#पूर्ण_अगर

/*
 * Singly-linked List definitions.
 */
#घोषणा	SLIST_शीर्ष(name, type)						\
काष्ठा name {								\
	काष्ठा type *slh_first;	/* first element */			\
}

#घोषणा	SLIST_शीर्ष_INITIALIZER(शीर्ष_2)					\
	{ शून्य }

#घोषणा	SLIST_ENTRY(type)						\
काष्ठा {								\
	काष्ठा type *sle_अग्र;	/* next element */			\
}

/*
 * Singly-linked List access methods.
 */
#घोषणा	SLIST_FIRST(शीर्ष_2)	((शीर्ष_2)->slh_first)
#घोषणा	SLIST_END(शीर्ष_2)		शून्य
#घोषणा	SLIST_EMPTY(शीर्ष_2)	((शीर्ष_2)->slh_first == शून्य)
#घोषणा	SLIST_अग्र(elm, field)	((elm)->field.sle_अग्र)

#घोषणा	SLIST_FOREACH(var, शीर्ष_2, field)					\
	क्रम((var) = (शीर्ष_2)->slh_first;					\
	    (var) != SLIST_END(शीर्ष_2);					\
	    (var) = (var)->field.sle_अग्र)

#घोषणा	SLIST_FOREACH_SAFE(var, शीर्ष_2, field, tvar)			\
	क्रम ((var) = SLIST_FIRST((शीर्ष_2));				\
	    (var) != SLIST_END(शीर्ष_2) &&					\
	    ((tvar) = SLIST_अग्र((var), field), 1);			\
	    (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#घोषणा	SLIST_INIT(शीर्ष_2) करो {						\
	(शीर्ष_2)->slh_first = SLIST_END(शीर्ष_2);				\
} जबतक (/*CONSTCOND*/0)

#घोषणा	SLIST_जोड़ो_AFTER(slistelm, elm, field) करो {			\
	(elm)->field.sle_अग्र = (slistelm)->field.sle_अग्र;		\
	(slistelm)->field.sle_अग्र = (elm);				\
} जबतक (/*CONSTCOND*/0)

#घोषणा	SLIST_जोड़ो_शीर्ष(शीर्ष_2, elm, field) करो {			\
	(elm)->field.sle_अग्र = (शीर्ष_2)->slh_first;			\
	(शीर्ष_2)->slh_first = (elm);					\
} जबतक (/*CONSTCOND*/0)

#घोषणा	SLIST_REMOVE_AFTER(slistelm, field) करो {			\
	(slistelm)->field.sle_अग्र =					\
	    SLIST_अग्र(SLIST_अग्र((slistelm), field), field);		\
} जबतक (/*CONSTCOND*/0)

#घोषणा	SLIST_REMOVE_शीर्ष(शीर्ष_2, field) करो {				\
	(शीर्ष_2)->slh_first = (शीर्ष_2)->slh_first->field.sle_अग्र;		\
} जबतक (/*CONSTCOND*/0)

#घोषणा	SLIST_REMOVE(शीर्ष_2, elm, type, field) करो {			\
	अगर ((शीर्ष_2)->slh_first == (elm)) {				\
		SLIST_REMOVE_शीर्ष((शीर्ष_2), field);			\
	}								\
	अन्यथा {								\
		काष्ठा type *curelm = (शीर्ष_2)->slh_first;		\
		जबतक(curelm->field.sle_अग्र != (elm))			\
			curelm = curelm->field.sle_अग्र;		\
		curelm->field.sle_अग्र =				\
		    curelm->field.sle_अग्र->field.sle_अग्र;		\
	}								\
} जबतक (/*CONSTCOND*/0)


/*
 * List definitions.
 */
#घोषणा	LIST_शीर्ष(name, type)						\
काष्ठा name {								\
	काष्ठा type *lh_first;	/* first element */			\
}

#घोषणा	LIST_शीर्ष_INITIALIZER(शीर्ष_2)					\
	{ शून्य }

#घोषणा	LIST_ENTRY(type)						\
काष्ठा {								\
	काष्ठा type *le_अग्र;	/* next element */			\
	काष्ठा type **le_पूर्व;	/* address of previous next element */	\
}

/*
 * List access methods.
 */
#घोषणा	LIST_FIRST(शीर्ष_2)		((शीर्ष_2)->lh_first)
#घोषणा	LIST_END(शीर्ष_2)			शून्य
#घोषणा	LIST_EMPTY(शीर्ष_2)		((शीर्ष_2)->lh_first == LIST_END(शीर्ष_2))
#घोषणा	LIST_अग्र(elm, field)		((elm)->field.le_अग्र)

#घोषणा	LIST_FOREACH(var, शीर्ष_2, field)					\
	क्रम ((var) = ((शीर्ष_2)->lh_first);				\
	    (var) != LIST_END(शीर्ष_2);					\
	    (var) = ((var)->field.le_अग्र))

#घोषणा	LIST_FOREACH_SAFE(var, शीर्ष_2, field, tvar)			\
	क्रम ((var) = LIST_FIRST((शीर्ष_2));				\
	    (var) != LIST_END(शीर्ष_2) &&					\
	    ((tvar) = LIST_अग्र((var), field), 1);			\
	    (var) = (tvar))

#घोषणा	LIST_MOVE(head1, head2) करो {					\
	LIST_INIT((head2));						\
	अगर (!LIST_EMPTY((head1))) {					\
		(head2)->lh_first = (head1)->lh_first;			\
		LIST_INIT((head1));					\
	}								\
} जबतक (/*CONSTCOND*/0)

/*
 * List functions.
 */
#अगर defined(QUEUEDEBUG)
#घोषणा	QUEUEDEBUG_LIST_जोड़ो_शीर्ष(शीर्ष_2, elm, field)			\
	अगर ((शीर्ष_2)->lh_first &&						\
	    (शीर्ष_2)->lh_first->field.le_पूर्व != &(शीर्ष_2)->lh_first)	\
		QUEUEDEBUG_ABORT("LIST_INSERT_HEAD %p %s:%d", (शीर्ष_2),	\
		    __खाता__, __LINE__);
#घोषणा	QUEUEDEBUG_LIST_OP(elm, field)					\
	अगर ((elm)->field.le_अग्र &&					\
	    (elm)->field.le_अग्र->field.le_पूर्व !=			\
	    &(elm)->field.le_अग्र)					\
		QUEUEDEBUG_ABORT("LIST_* forw %p %s:%d", (elm),		\
		    __खाता__, __LINE__);				\
	अगर (*(elm)->field.le_पूर्व != (elm))				\
		QUEUEDEBUG_ABORT("LIST_* back %p %s:%d", (elm),		\
		    __खाता__, __LINE__);
#घोषणा	QUEUEDEBUG_LIST_POSTREMOVE(elm, field)				\
	(elm)->field.le_अग्र = (व्योम *)1L;				\
	(elm)->field.le_पूर्व = (व्योम *)1L;
#अन्यथा
#घोषणा	QUEUEDEBUG_LIST_जोड़ो_शीर्ष(शीर्ष_2, elm, field)
#घोषणा	QUEUEDEBUG_LIST_OP(elm, field)
#घोषणा	QUEUEDEBUG_LIST_POSTREMOVE(elm, field)
#पूर्ण_अगर

#घोषणा	LIST_INIT(शीर्ष_2) करो {						\
	(शीर्ष_2)->lh_first = LIST_END(शीर्ष_2);				\
} जबतक (/*CONSTCOND*/0)

#घोषणा	LIST_जोड़ो_AFTER(listelm, elm, field) करो {			\
	QUEUEDEBUG_LIST_OP((listelm), field)				\
	अगर (((elm)->field.le_अग्र = (listelm)->field.le_अग्र) != 	\
	    LIST_END(शीर्ष_2))						\
		(listelm)->field.le_अग्र->field.le_पूर्व =		\
		    &(elm)->field.le_अग्र;				\
	(listelm)->field.le_अग्र = (elm);				\
	(elm)->field.le_पूर्व = &(listelm)->field.le_अग्र;		\
} जबतक (/*CONSTCOND*/0)

#घोषणा	LIST_जोड़ो_BEFORE(listelm, elm, field) करो {			\
	QUEUEDEBUG_LIST_OP((listelm), field)				\
	(elm)->field.le_पूर्व = (listelm)->field.le_पूर्व;		\
	(elm)->field.le_अग्र = (listelm);				\
	*(listelm)->field.le_पूर्व = (elm);				\
	(listelm)->field.le_पूर्व = &(elm)->field.le_अग्र;		\
} जबतक (/*CONSTCOND*/0)

#घोषणा	LIST_जोड़ो_शीर्ष(शीर्ष_2, elm, field) करो {				\
	QUEUEDEBUG_LIST_जोड़ो_शीर्ष((शीर्ष_2), (elm), field)		\
	अगर (((elm)->field.le_अग्र = (शीर्ष_2)->lh_first) != LIST_END(शीर्ष_2))\
		(शीर्ष_2)->lh_first->field.le_पूर्व = &(elm)->field.le_अग्र;\
	(शीर्ष_2)->lh_first = (elm);					\
	(elm)->field.le_पूर्व = &(शीर्ष_2)->lh_first;			\
} जबतक (/*CONSTCOND*/0)

#घोषणा	LIST_REMOVE(elm, field) करो {					\
	QUEUEDEBUG_LIST_OP((elm), field)				\
	अगर ((elm)->field.le_अग्र != शून्य)				\
		(elm)->field.le_अग्र->field.le_पूर्व = 			\
		    (elm)->field.le_पूर्व;				\
	*(elm)->field.le_पूर्व = (elm)->field.le_अग्र;			\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), field)			\
} जबतक (/*CONSTCOND*/0)

#घोषणा LIST_REPLACE(elm, elm2, field) करो {				\
	अगर (((elm2)->field.le_अग्र = (elm)->field.le_अग्र) != शून्य)	\
		(elm2)->field.le_अग्र->field.le_पूर्व =			\
		    &(elm2)->field.le_अग्र;				\
	(elm2)->field.le_पूर्व = (elm)->field.le_पूर्व;			\
	*(elm2)->field.le_पूर्व = (elm2);				\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), field)			\
} जबतक (/*CONSTCOND*/0)

/*
 * Simple queue definitions.
 */
#घोषणा	SIMPLEQ_शीर्ष(name, type)					\
काष्ठा name {								\
	काष्ठा type *sqh_first;	/* first element */			\
	काष्ठा type **sqh_last;	/* addr of last next element */		\
}

#घोषणा	SIMPLEQ_शीर्ष_INITIALIZER(शीर्ष_2)					\
	{ शून्य, &(शीर्ष_2).sqh_first }

#घोषणा	SIMPLEQ_ENTRY(type)						\
काष्ठा {								\
	काष्ठा type *sqe_अग्र;	/* next element */			\
}

/*
 * Simple queue access methods.
 */
#घोषणा	SIMPLEQ_FIRST(शीर्ष_2)		((शीर्ष_2)->sqh_first)
#घोषणा	SIMPLEQ_END(शीर्ष_2)		शून्य
#घोषणा	SIMPLEQ_EMPTY(शीर्ष_2)		((शीर्ष_2)->sqh_first == SIMPLEQ_END(शीर्ष_2))
#घोषणा	SIMPLEQ_अग्र(elm, field)	((elm)->field.sqe_अग्र)

#घोषणा	SIMPLEQ_FOREACH(var, शीर्ष_2, field)				\
	क्रम ((var) = ((शीर्ष_2)->sqh_first);				\
	    (var) != SIMPLEQ_END(शीर्ष_2);					\
	    (var) = ((var)->field.sqe_अग्र))

#घोषणा	SIMPLEQ_FOREACH_SAFE(var, शीर्ष_2, field, अग्र)			\
	क्रम ((var) = ((शीर्ष_2)->sqh_first);				\
	    (var) != SIMPLEQ_END(शीर्ष_2) &&				\
	    ((अग्र = ((var)->field.sqe_अग्र)), 1);			\
	    (var) = (अग्र))

/*
 * Simple queue functions.
 */
#घोषणा	SIMPLEQ_INIT(शीर्ष_2) करो {						\
	(शीर्ष_2)->sqh_first = शून्य;					\
	(शीर्ष_2)->sqh_last = &(शीर्ष_2)->sqh_first;				\
} जबतक (/*CONSTCOND*/0)

#घोषणा	SIMPLEQ_जोड़ो_शीर्ष(शीर्ष_2, elm, field) करो {			\
	अगर (((elm)->field.sqe_अग्र = (शीर्ष_2)->sqh_first) == शून्य)	\
		(शीर्ष_2)->sqh_last = &(elm)->field.sqe_अग्र;		\
	(शीर्ष_2)->sqh_first = (elm);					\
} जबतक (/*CONSTCOND*/0)

#घोषणा	SIMPLEQ_जोड़ो_पुच्छ(शीर्ष_2, elm, field) करो {			\
	(elm)->field.sqe_अग्र = शून्य;					\
	*(शीर्ष_2)->sqh_last = (elm);					\
	(शीर्ष_2)->sqh_last = &(elm)->field.sqe_अग्र;			\
} जबतक (/*CONSTCOND*/0)

#घोषणा	SIMPLEQ_जोड़ो_AFTER(शीर्ष_2, listelm, elm, field) करो {		\
	अगर (((elm)->field.sqe_अग्र = (listelm)->field.sqe_अग्र) == शून्य)\
		(शीर्ष_2)->sqh_last = &(elm)->field.sqe_अग्र;		\
	(listelm)->field.sqe_अग्र = (elm);				\
} जबतक (/*CONSTCOND*/0)

#घोषणा	SIMPLEQ_REMOVE_शीर्ष(शीर्ष_2, field) करो {				\
	अगर (((शीर्ष_2)->sqh_first = (शीर्ष_2)->sqh_first->field.sqe_अग्र) == शून्य) \
		(शीर्ष_2)->sqh_last = &(शीर्ष_2)->sqh_first;			\
} जबतक (/*CONSTCOND*/0)

#घोषणा SIMPLEQ_REMOVE_AFTER(शीर्ष_2, elm, field) करो {			\
	अगर (((elm)->field.sqe_अग्र = (elm)->field.sqe_अग्र->field.sqe_अग्र) \
	    == शून्य)							\
		(शीर्ष_2)->sqh_last = &(elm)->field.sqe_अग्र;		\
} जबतक (/*CONSTCOND*/0)

#घोषणा	SIMPLEQ_REMOVE(शीर्ष_2, elm, type, field) करो {			\
	अगर ((शीर्ष_2)->sqh_first == (elm)) {				\
		SIMPLEQ_REMOVE_शीर्ष((शीर्ष_2), field);			\
	} अन्यथा {							\
		काष्ठा type *curelm = (शीर्ष_2)->sqh_first;		\
		जबतक (curelm->field.sqe_अग्र != (elm))			\
			curelm = curelm->field.sqe_अग्र;		\
		अगर ((curelm->field.sqe_अग्र =				\
			curelm->field.sqe_अग्र->field.sqe_अग्र) == शून्य) \
			    (शीर्ष_2)->sqh_last = &(curelm)->field.sqe_अग्र; \
	}								\
} जबतक (/*CONSTCOND*/0)

#घोषणा	SIMPLEQ_CONCAT(head1, head2) करो {				\
	अगर (!SIMPLEQ_EMPTY((head2))) {					\
		*(head1)->sqh_last = (head2)->sqh_first;		\
		(head1)->sqh_last = (head2)->sqh_last;		\
		SIMPLEQ_INIT((head2));					\
	}								\
} जबतक (/*CONSTCOND*/0)

#घोषणा	SIMPLEQ_LAST(शीर्ष_2, type, field)					\
	(SIMPLEQ_EMPTY((शीर्ष_2)) ?						\
		शून्य :							\
	        ((काष्ठा type *)(व्योम *)				\
		((अक्षर *)((शीर्ष_2)->sqh_last) - दुरत्व(काष्ठा type, field))))

/*
 * Tail queue definitions.
 */
#घोषणा	TAILQ_शीर्ष_2(name, type, qual)					\
काष्ठा name {								\
	qual type *tqh_first;		/* first element */		\
	qual type *qual *tqh_last;	/* addr of last next element */	\
}
#घोषणा TAILQ_शीर्ष(name, type)	TAILQ_शीर्ष_2(name, काष्ठा type,)

#घोषणा	TAILQ_शीर्ष_INITIALIZER(शीर्ष_2)					\
	{ TAILQ_END(शीर्ष_2), &(शीर्ष_2).tqh_first }

#घोषणा	_TAILQ_ENTRY(type, qual)					\
काष्ठा {								\
	qual type *tqe_अग्र;		/* next element */		\
	qual type *qual *tqe_पूर्व;	/* address of previous next element */\
}
#घोषणा TAILQ_ENTRY(type)	_TAILQ_ENTRY(काष्ठा type,)

/*
 * Tail queue access methods.
 */
#घोषणा	TAILQ_FIRST(शीर्ष_2)		((शीर्ष_2)->tqh_first)
#घोषणा	TAILQ_END(शीर्ष_2)			(शून्य)
#घोषणा	TAILQ_अग्र(elm, field)		((elm)->field.tqe_अग्र)
#घोषणा	TAILQ_LAST(शीर्ष_2, headname) \
	(*(((काष्ठा headname *)((शीर्ष_2)->tqh_last))->tqh_last))
#घोषणा	TAILQ_पूर्व(elm, headname, field) \
	(*(((काष्ठा headname *)((elm)->field.tqe_पूर्व))->tqh_last))
#घोषणा	TAILQ_EMPTY(शीर्ष_2)		(TAILQ_FIRST(शीर्ष_2) == TAILQ_END(शीर्ष_2))


#घोषणा	TAILQ_FOREACH(var, शीर्ष_2, field)					\
	क्रम ((var) = ((शीर्ष_2)->tqh_first);				\
	    (var) != TAILQ_END(शीर्ष_2);					\
	    (var) = ((var)->field.tqe_अग्र))

#घोषणा	TAILQ_FOREACH_SAFE(var, शीर्ष_2, field, अग्र)			\
	क्रम ((var) = ((शीर्ष_2)->tqh_first);				\
	    (var) != TAILQ_END(शीर्ष_2) &&					\
	    ((अग्र) = TAILQ_अग्र(var, field), 1); (var) = (अग्र))

#घोषणा	TAILQ_FOREACH_REVERSE(var, शीर्ष_2, headname, field)		\
	क्रम ((var) = (*(((काष्ठा headname *)((शीर्ष_2)->tqh_last))->tqh_last));\
	    (var) != TAILQ_END(शीर्ष_2);					\
	    (var) = (*(((काष्ठा headname *)((var)->field.tqe_पूर्व))->tqh_last)))

#घोषणा	TAILQ_FOREACH_REVERSE_SAFE(var, शीर्ष_2, headname, field, पूर्व)	\
	क्रम ((var) = TAILQ_LAST((शीर्ष_2), headname);			\
	    (var) != TAILQ_END(शीर्ष_2) && 				\
	    ((पूर्व) = TAILQ_पूर्व((var), headname, field), 1); (var) = (पूर्व))

/*
 * Tail queue functions.
 */
#अगर defined(QUEUEDEBUG)
#घोषणा	QUEUEDEBUG_TAILQ_जोड़ो_शीर्ष(शीर्ष_2, elm, field)			\
	अगर ((शीर्ष_2)->tqh_first &&					\
	    (शीर्ष_2)->tqh_first->field.tqe_पूर्व != &(शीर्ष_2)->tqh_first)	\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_HEAD %p %s:%d", (शीर्ष_2),	\
		    __खाता__, __LINE__);
#घोषणा	QUEUEDEBUG_TAILQ_जोड़ो_पुच्छ(शीर्ष_2, elm, field)			\
	अगर (*(शीर्ष_2)->tqh_last != शून्य)					\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_TAIL %p %s:%d", (शीर्ष_2),	\
		    __खाता__, __LINE__);
#घोषणा	QUEUEDEBUG_TAILQ_OP(elm, field)					\
	अगर ((elm)->field.tqe_अग्र &&					\
	    (elm)->field.tqe_अग्र->field.tqe_पूर्व !=			\
	    &(elm)->field.tqe_अग्र)					\
		QUEUEDEBUG_ABORT("TAILQ_* forw %p %s:%d", (elm),	\
		    __खाता__, __LINE__);				\
	अगर (*(elm)->field.tqe_पूर्व != (elm))				\
		QUEUEDEBUG_ABORT("TAILQ_* back %p %s:%d", (elm),	\
		    __खाता__, __LINE__);
#घोषणा	QUEUEDEBUG_TAILQ_PREREMOVE(शीर्ष_2, elm, field)			\
	अगर ((elm)->field.tqe_अग्र == शून्य &&				\
	    (शीर्ष_2)->tqh_last != &(elm)->field.tqe_अग्र)			\
		QUEUEDEBUG_ABORT("TAILQ_PREREMOVE head %p elm %p %s:%d",\
		    (शीर्ष_2), (elm), __खाता__, __LINE__);
#घोषणा	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, field)				\
	(elm)->field.tqe_अग्र = (व्योम *)1L;				\
	(elm)->field.tqe_पूर्व = (व्योम *)1L;
#अन्यथा
#घोषणा	QUEUEDEBUG_TAILQ_जोड़ो_शीर्ष(शीर्ष_2, elm, field)
#घोषणा	QUEUEDEBUG_TAILQ_जोड़ो_पुच्छ(शीर्ष_2, elm, field)
#घोषणा	QUEUEDEBUG_TAILQ_OP(elm, field)
#घोषणा	QUEUEDEBUG_TAILQ_PREREMOVE(शीर्ष_2, elm, field)
#घोषणा	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, field)
#पूर्ण_अगर

#घोषणा	TAILQ_INIT(शीर्ष_2) करो {						\
	(शीर्ष_2)->tqh_first = TAILQ_END(शीर्ष_2);				\
	(शीर्ष_2)->tqh_last = &(शीर्ष_2)->tqh_first;				\
} जबतक (/*CONSTCOND*/0)

#घोषणा	TAILQ_जोड़ो_शीर्ष(शीर्ष_2, elm, field) करो {			\
	QUEUEDEBUG_TAILQ_जोड़ो_शीर्ष((शीर्ष_2), (elm), field)		\
	अगर (((elm)->field.tqe_अग्र = (शीर्ष_2)->tqh_first) != TAILQ_END(शीर्ष_2))\
		(शीर्ष_2)->tqh_first->field.tqe_पूर्व =			\
		    &(elm)->field.tqe_अग्र;				\
	अन्यथा								\
		(शीर्ष_2)->tqh_last = &(elm)->field.tqe_अग्र;		\
	(शीर्ष_2)->tqh_first = (elm);					\
	(elm)->field.tqe_पूर्व = &(शीर्ष_2)->tqh_first;			\
} जबतक (/*CONSTCOND*/0)

#घोषणा	TAILQ_जोड़ो_पुच्छ(शीर्ष_2, elm, field) करो {			\
	QUEUEDEBUG_TAILQ_जोड़ो_पुच्छ((शीर्ष_2), (elm), field)		\
	(elm)->field.tqe_अग्र = TAILQ_END(शीर्ष_2);			\
	(elm)->field.tqe_पूर्व = (शीर्ष_2)->tqh_last;			\
	*(शीर्ष_2)->tqh_last = (elm);					\
	(शीर्ष_2)->tqh_last = &(elm)->field.tqe_अग्र;			\
} जबतक (/*CONSTCOND*/0)

#घोषणा	TAILQ_जोड़ो_AFTER(शीर्ष_2, listelm, elm, field) करो {		\
	QUEUEDEBUG_TAILQ_OP((listelm), field)				\
	अगर (((elm)->field.tqe_अग्र = (listelm)->field.tqe_अग्र) != 	\
	    TAILQ_END(शीर्ष_2))						\
		(elm)->field.tqe_अग्र->field.tqe_पूर्व = 		\
		    &(elm)->field.tqe_अग्र;				\
	अन्यथा								\
		(शीर्ष_2)->tqh_last = &(elm)->field.tqe_अग्र;		\
	(listelm)->field.tqe_अग्र = (elm);				\
	(elm)->field.tqe_पूर्व = &(listelm)->field.tqe_अग्र;		\
} जबतक (/*CONSTCOND*/0)

#घोषणा	TAILQ_जोड़ो_BEFORE(listelm, elm, field) करो {			\
	QUEUEDEBUG_TAILQ_OP((listelm), field)				\
	(elm)->field.tqe_पूर्व = (listelm)->field.tqe_पूर्व;		\
	(elm)->field.tqe_अग्र = (listelm);				\
	*(listelm)->field.tqe_पूर्व = (elm);				\
	(listelm)->field.tqe_पूर्व = &(elm)->field.tqe_अग्र;		\
} जबतक (/*CONSTCOND*/0)

#घोषणा	TAILQ_REMOVE(शीर्ष_2, elm, field) करो {				\
	QUEUEDEBUG_TAILQ_PREREMOVE((शीर्ष_2), (elm), field)		\
	QUEUEDEBUG_TAILQ_OP((elm), field)				\
	अगर (((elm)->field.tqe_अग्र) != TAILQ_END(शीर्ष_2))			\
		(elm)->field.tqe_अग्र->field.tqe_पूर्व = 		\
		    (elm)->field.tqe_पूर्व;				\
	अन्यथा								\
		(शीर्ष_2)->tqh_last = (elm)->field.tqe_पूर्व;		\
	*(elm)->field.tqe_पूर्व = (elm)->field.tqe_अग्र;			\
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), field);			\
} जबतक (/*CONSTCOND*/0)

#घोषणा TAILQ_REPLACE(शीर्ष_2, elm, elm2, field) करो {                      \
        अगर (((elm2)->field.tqe_अग्र = (elm)->field.tqe_अग्र) != 	\
	    TAILQ_END(शीर्ष_2))   						\
                (elm2)->field.tqe_अग्र->field.tqe_पूर्व =                \
                    &(elm2)->field.tqe_अग्र;                            \
        अन्यथा                                                            \
                (शीर्ष_2)->tqh_last = &(elm2)->field.tqe_अग्र;             \
        (elm2)->field.tqe_पूर्व = (elm)->field.tqe_पूर्व;                 \
        *(elm2)->field.tqe_पूर्व = (elm2);                               \
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), field);			\
} जबतक (/*CONSTCOND*/0)

#घोषणा	TAILQ_CONCAT(head1, head2, field) करो {				\
	अगर (!TAILQ_EMPTY(head2)) {					\
		*(head1)->tqh_last = (head2)->tqh_first;		\
		(head2)->tqh_first->field.tqe_पूर्व = (head1)->tqh_last;	\
		(head1)->tqh_last = (head2)->tqh_last;			\
		TAILQ_INIT((head2));					\
	}								\
} जबतक (/*CONSTCOND*/0)

/*
 * Singly-linked Tail queue declarations.
 */
#घोषणा	STAILQ_शीर्ष(name, type)						\
काष्ठा name {								\
	काष्ठा type *stqh_first;	/* first element */		\
	काष्ठा type **stqh_last;	/* addr of last next element */	\
}

#घोषणा	STAILQ_शीर्ष_INITIALIZER(शीर्ष_2)					\
	{ शून्य, &(शीर्ष_2).stqh_first }

#घोषणा	STAILQ_ENTRY(type)						\
काष्ठा {								\
	काष्ठा type *stqe_अग्र;	/* next element */			\
}

/*
 * Singly-linked Tail queue access methods.
 */
#घोषणा	STAILQ_FIRST(शीर्ष_2)	((शीर्ष_2)->stqh_first)
#घोषणा	STAILQ_END(शीर्ष_2)	शून्य
#घोषणा	STAILQ_अग्र(elm, field)	((elm)->field.stqe_अग्र)
#घोषणा	STAILQ_EMPTY(शीर्ष_2)	(STAILQ_FIRST(शीर्ष_2) == STAILQ_END(शीर्ष_2))

/*
 * Singly-linked Tail queue functions.
 */
#घोषणा	STAILQ_INIT(शीर्ष_2) करो {						\
	(शीर्ष_2)->stqh_first = शून्य;					\
	(शीर्ष_2)->stqh_last = &(शीर्ष_2)->stqh_first;				\
} जबतक (/*CONSTCOND*/0)

#घोषणा	STAILQ_जोड़ो_शीर्ष(शीर्ष_2, elm, field) करो {			\
	अगर (((elm)->field.stqe_अग्र = (शीर्ष_2)->stqh_first) == शून्य)	\
		(शीर्ष_2)->stqh_last = &(elm)->field.stqe_अग्र;		\
	(शीर्ष_2)->stqh_first = (elm);					\
} जबतक (/*CONSTCOND*/0)

#घोषणा	STAILQ_जोड़ो_पुच्छ(शीर्ष_2, elm, field) करो {			\
	(elm)->field.stqe_अग्र = शून्य;					\
	*(शीर्ष_2)->stqh_last = (elm);					\
	(शीर्ष_2)->stqh_last = &(elm)->field.stqe_अग्र;			\
} जबतक (/*CONSTCOND*/0)

#घोषणा	STAILQ_जोड़ो_AFTER(शीर्ष_2, listelm, elm, field) करो {		\
	अगर (((elm)->field.stqe_अग्र = (listelm)->field.stqe_अग्र) == शून्य)\
		(शीर्ष_2)->stqh_last = &(elm)->field.stqe_अग्र;		\
	(listelm)->field.stqe_अग्र = (elm);				\
} जबतक (/*CONSTCOND*/0)

#घोषणा	STAILQ_REMOVE_शीर्ष(शीर्ष_2, field) करो {				\
	अगर (((शीर्ष_2)->stqh_first = (शीर्ष_2)->stqh_first->field.stqe_अग्र) == शून्य) \
		(शीर्ष_2)->stqh_last = &(शीर्ष_2)->stqh_first;			\
} जबतक (/*CONSTCOND*/0)

#घोषणा	STAILQ_REMOVE(शीर्ष_2, elm, type, field) करो {			\
	अगर ((शीर्ष_2)->stqh_first == (elm)) {				\
		STAILQ_REMOVE_शीर्ष((शीर्ष_2), field);			\
	} अन्यथा {							\
		काष्ठा type *curelm = (शीर्ष_2)->stqh_first;		\
		जबतक (curelm->field.stqe_अग्र != (elm))			\
			curelm = curelm->field.stqe_अग्र;		\
		अगर ((curelm->field.stqe_अग्र =				\
			curelm->field.stqe_अग्र->field.stqe_अग्र) == शून्य) \
			    (शीर्ष_2)->stqh_last = &(curelm)->field.stqe_अग्र; \
	}								\
} जबतक (/*CONSTCOND*/0)

#घोषणा	STAILQ_FOREACH(var, शीर्ष_2, field)				\
	क्रम ((var) = ((शीर्ष_2)->stqh_first);				\
		(var);							\
		(var) = ((var)->field.stqe_अग्र))

#घोषणा	STAILQ_FOREACH_SAFE(var, शीर्ष_2, field, tvar)			\
	क्रम ((var) = STAILQ_FIRST((शीर्ष_2));				\
	    (var) && ((tvar) = STAILQ_अग्र((var), field), 1);		\
	    (var) = (tvar))

#घोषणा	STAILQ_CONCAT(head1, head2) करो {				\
	अगर (!STAILQ_EMPTY((head2))) {					\
		*(head1)->stqh_last = (head2)->stqh_first;		\
		(head1)->stqh_last = (head2)->stqh_last;		\
		STAILQ_INIT((head2));					\
	}								\
} जबतक (/*CONSTCOND*/0)

#घोषणा	STAILQ_LAST(शीर्ष_2, type, field)					\
	(STAILQ_EMPTY((शीर्ष_2)) ?						\
		शून्य :							\
	        ((काष्ठा type *)(व्योम *)				\
		((अक्षर *)((शीर्ष_2)->stqh_last) - दुरत्व(काष्ठा type, field))))


#अगर_अघोषित _KERNEL
/*
 * Circular queue definitions. Do not use. We still keep the macros
 * for compatibility but because of pointer aliasing issues their use
 * is discouraged!
 */

/*
 * __launder_type():  We use this ugly hack to work around the compiler
 * noticing that two types may not alias each other and elide tests in code.
 * We hit this in the CIRCLEQ macros when comparing 'struct name *' and
 * 'struct type *' (see CIRCLEQ_HEAD()).  Modern compilers (such as GCC
 * 4.8) declare these comparisons as always false, causing the code to
 * not run as designed.
 *
 * This hack is only to be used for comparisons and thus can be fully const.
 * Do not use for assignment.
 *
 * If we ever choose to change the ABI of the CIRCLEQ macros, we could fix
 * this by changing the head/tail sentinel values, but see the note above
 * this one.
 */
अटल __inline स्थिर व्योम * __launder_type(स्थिर व्योम *);
अटल __inline स्थिर व्योम *
__launder_type(स्थिर व्योम *__x)
{
	__asm __volatile("" : "+r" (__x));
	वापस __x;
}

#अगर defined(QUEUEDEBUG)
#घोषणा QUEUEDEBUG_CIRCLEQ_शीर्ष(शीर्ष_2, field)				\
	अगर ((शीर्ष_2)->cqh_first != CIRCLEQ_ENDC(शीर्ष_2) &&			\
	    (शीर्ष_2)->cqh_first->field.cqe_पूर्व != CIRCLEQ_ENDC(शीर्ष_2))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head forw %p %s:%d", (शीर्ष_2),	\
		      __खाता__, __LINE__);				\
	अगर ((शीर्ष_2)->cqh_last != CIRCLEQ_ENDC(शीर्ष_2) &&			\
	    (शीर्ष_2)->cqh_last->field.cqe_अग्र != CIRCLEQ_ENDC(शीर्ष_2))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head back %p %s:%d", (शीर्ष_2),	\
		      __खाता__, __LINE__);
#घोषणा QUEUEDEBUG_CIRCLEQ_ELM(शीर्ष_2, elm, field)			\
	अगर ((elm)->field.cqe_अग्र == CIRCLEQ_ENDC(शीर्ष_2)) {		\
		अगर ((शीर्ष_2)->cqh_last != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm last %p %s:%d",	\
			    (elm), __खाता__, __LINE__);			\
	} अन्यथा {							\
		अगर ((elm)->field.cqe_अग्र->field.cqe_पूर्व != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm forw %p %s:%d",	\
			    (elm), __खाता__, __LINE__);			\
	}								\
	अगर ((elm)->field.cqe_पूर्व == CIRCLEQ_ENDC(शीर्ष_2)) {		\
		अगर ((शीर्ष_2)->cqh_first != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm first %p %s:%d",	\
			    (elm), __खाता__, __LINE__);			\
	} अन्यथा {							\
		अगर ((elm)->field.cqe_पूर्व->field.cqe_अग्र != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm prev %p %s:%d",	\
			    (elm), __खाता__, __LINE__);			\
	}
#घोषणा QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, field)			\
	(elm)->field.cqe_अग्र = (व्योम *)1L;				\
	(elm)->field.cqe_पूर्व = (व्योम *)1L;
#अन्यथा
#घोषणा QUEUEDEBUG_CIRCLEQ_शीर्ष(शीर्ष_2, field)
#घोषणा QUEUEDEBUG_CIRCLEQ_ELM(शीर्ष_2, elm, field)
#घोषणा QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, field)
#पूर्ण_अगर

#घोषणा	CIRCLEQ_शीर्ष(name, type)					\
काष्ठा name {								\
	काष्ठा type *cqh_first;		/* first element */		\
	काष्ठा type *cqh_last;		/* last element */		\
}

#घोषणा	CIRCLEQ_शीर्ष_INITIALIZER(शीर्ष_2)					\
	{ CIRCLEQ_END(&शीर्ष_2), CIRCLEQ_END(&शीर्ष_2) }

#घोषणा	CIRCLEQ_ENTRY(type)						\
काष्ठा {								\
	काष्ठा type *cqe_अग्र;		/* next element */		\
	काष्ठा type *cqe_पूर्व;		/* previous element */		\
}

/*
 * Circular queue functions.
 */
#घोषणा	CIRCLEQ_INIT(शीर्ष_2) करो {						\
	(शीर्ष_2)->cqh_first = CIRCLEQ_END(शीर्ष_2);				\
	(शीर्ष_2)->cqh_last = CIRCLEQ_END(शीर्ष_2);				\
} जबतक (/*CONSTCOND*/0)

#घोषणा	CIRCLEQ_जोड़ो_AFTER(शीर्ष_2, listelm, elm, field) करो {		\
	QUEUEDEBUG_CIRCLEQ_शीर्ष((शीर्ष_2), field)				\
	QUEUEDEBUG_CIRCLEQ_ELM((शीर्ष_2), (listelm), field)		\
	(elm)->field.cqe_अग्र = (listelm)->field.cqe_अग्र;		\
	(elm)->field.cqe_पूर्व = (listelm);				\
	अगर ((listelm)->field.cqe_अग्र == CIRCLEQ_ENDC(शीर्ष_2))		\
		(शीर्ष_2)->cqh_last = (elm);				\
	अन्यथा								\
		(listelm)->field.cqe_अग्र->field.cqe_पूर्व = (elm);	\
	(listelm)->field.cqe_अग्र = (elm);				\
} जबतक (/*CONSTCOND*/0)

#घोषणा	CIRCLEQ_जोड़ो_BEFORE(शीर्ष_2, listelm, elm, field) करो {		\
	QUEUEDEBUG_CIRCLEQ_शीर्ष((शीर्ष_2), field)				\
	QUEUEDEBUG_CIRCLEQ_ELM((शीर्ष_2), (listelm), field)		\
	(elm)->field.cqe_अग्र = (listelm);				\
	(elm)->field.cqe_पूर्व = (listelm)->field.cqe_पूर्व;		\
	अगर ((listelm)->field.cqe_पूर्व == CIRCLEQ_ENDC(शीर्ष_2))		\
		(शीर्ष_2)->cqh_first = (elm);				\
	अन्यथा								\
		(listelm)->field.cqe_पूर्व->field.cqe_अग्र = (elm);	\
	(listelm)->field.cqe_पूर्व = (elm);				\
} जबतक (/*CONSTCOND*/0)

#घोषणा	CIRCLEQ_जोड़ो_शीर्ष(शीर्ष_2, elm, field) करो {			\
	QUEUEDEBUG_CIRCLEQ_शीर्ष((शीर्ष_2), field)				\
	(elm)->field.cqe_अग्र = (शीर्ष_2)->cqh_first;			\
	(elm)->field.cqe_पूर्व = CIRCLEQ_END(शीर्ष_2);			\
	अगर ((शीर्ष_2)->cqh_last == CIRCLEQ_ENDC(शीर्ष_2))			\
		(शीर्ष_2)->cqh_last = (elm);				\
	अन्यथा								\
		(शीर्ष_2)->cqh_first->field.cqe_पूर्व = (elm);		\
	(शीर्ष_2)->cqh_first = (elm);					\
} जबतक (/*CONSTCOND*/0)

#घोषणा	CIRCLEQ_जोड़ो_पुच्छ(शीर्ष_2, elm, field) करो {			\
	QUEUEDEBUG_CIRCLEQ_शीर्ष((शीर्ष_2), field)				\
	(elm)->field.cqe_अग्र = CIRCLEQ_END(शीर्ष_2);			\
	(elm)->field.cqe_पूर्व = (शीर्ष_2)->cqh_last;			\
	अगर ((शीर्ष_2)->cqh_first == CIRCLEQ_ENDC(शीर्ष_2))			\
		(शीर्ष_2)->cqh_first = (elm);				\
	अन्यथा								\
		(शीर्ष_2)->cqh_last->field.cqe_अग्र = (elm);		\
	(शीर्ष_2)->cqh_last = (elm);					\
} जबतक (/*CONSTCOND*/0)

#घोषणा	CIRCLEQ_REMOVE(शीर्ष_2, elm, field) करो {				\
	QUEUEDEBUG_CIRCLEQ_शीर्ष((शीर्ष_2), field)				\
	QUEUEDEBUG_CIRCLEQ_ELM((शीर्ष_2), (elm), field)			\
	अगर ((elm)->field.cqe_अग्र == CIRCLEQ_ENDC(शीर्ष_2))		\
		(शीर्ष_2)->cqh_last = (elm)->field.cqe_पूर्व;		\
	अन्यथा								\
		(elm)->field.cqe_अग्र->field.cqe_पूर्व =			\
		    (elm)->field.cqe_पूर्व;				\
	अगर ((elm)->field.cqe_पूर्व == CIRCLEQ_ENDC(शीर्ष_2))		\
		(शीर्ष_2)->cqh_first = (elm)->field.cqe_अग्र;		\
	अन्यथा								\
		(elm)->field.cqe_पूर्व->field.cqe_अग्र =			\
		    (elm)->field.cqe_अग्र;				\
	QUEUEDEBUG_CIRCLEQ_POSTREMOVE((elm), field)			\
} जबतक (/*CONSTCOND*/0)

#घोषणा	CIRCLEQ_FOREACH(var, शीर्ष_2, field)				\
	क्रम ((var) = ((शीर्ष_2)->cqh_first);				\
		(var) != CIRCLEQ_ENDC(शीर्ष_2);				\
		(var) = ((var)->field.cqe_अग्र))

#घोषणा	CIRCLEQ_FOREACH_REVERSE(var, शीर्ष_2, field)			\
	क्रम ((var) = ((शीर्ष_2)->cqh_last);				\
		(var) != CIRCLEQ_ENDC(शीर्ष_2);				\
		(var) = ((var)->field.cqe_पूर्व))

/*
 * Circular queue access methods.
 */
#घोषणा	CIRCLEQ_FIRST(शीर्ष_2)		((शीर्ष_2)->cqh_first)
#घोषणा	CIRCLEQ_LAST(शीर्ष_2)		((शीर्ष_2)->cqh_last)
/* For comparisons */
#घोषणा	CIRCLEQ_ENDC(शीर्ष_2)		(__launder_type(शीर्ष_2))
/* For assignments */
#घोषणा	CIRCLEQ_END(शीर्ष_2)		((व्योम *)(शीर्ष_2))
#घोषणा	CIRCLEQ_अग्र(elm, field)	((elm)->field.cqe_अग्र)
#घोषणा	CIRCLEQ_पूर्व(elm, field)	((elm)->field.cqe_पूर्व)
#घोषणा	CIRCLEQ_EMPTY(शीर्ष_2)						\
    (CIRCLEQ_FIRST(शीर्ष_2) == CIRCLEQ_ENDC(शीर्ष_2))

#घोषणा CIRCLEQ_LOOP_अग्र(शीर्ष_2, elm, field)				\
	(((elm)->field.cqe_अग्र == CIRCLEQ_ENDC(शीर्ष_2))			\
	    ? ((शीर्ष_2)->cqh_first)					\
	    : (elm->field.cqe_अग्र))
#घोषणा CIRCLEQ_LOOP_पूर्व(शीर्ष_2, elm, field)				\
	(((elm)->field.cqe_पूर्व == CIRCLEQ_ENDC(शीर्ष_2))			\
	    ? ((शीर्ष_2)->cqh_last)					\
	    : (elm->field.cqe_पूर्व))
#पूर्ण_अगर /* !_KERNEL */

#पूर्ण_अगर	/* !_SYS_QUEUE_H_ */
