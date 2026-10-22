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

#यदि_अपरिभाषितम्	_SYS_QUEUE_H_
#परिभाषय	_SYS_QUEUE_H_

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
#यदि_परिभाषितम् __NetBSD__
#अन्तर्भाव <sys/null.h>
#यदि_अन्तः

#यदि defined(QUEUEDEBUG)
# यदि defined(_KERNEL)
#  define QUEUEDEBUG_ABORT(...) panic(__VA_ARGS__)
# अन्यथा
#  include <err.h>
#  define QUEUEDEBUG_ABORT(...) err(1, __VA_ARGS__)
# endif
#यदि_अन्तः

/*
 * Singly-linked List definitions.
 */
#परिभाषय	SLIST_शीर्ष(name, type)						\
संरचना name {								\
	संरचना type *slh_first;	/* first element */			\
}

#परिभाषय	SLIST_शीर्ष_INITIALIZER(शीर्ष)					\
	{ रिक्त }

#परिभाषय	SLIST_प्रविष्टिः(type)						\
संरचना {								\
	संरचना type *sle_अग्र;	/* next element */			\
}

/*
 * Singly-linked List access methods.
 */
#परिभाषय	SLIST_FIRST(शीर्ष)	((शीर्ष)->slh_first)
#परिभाषय	SLIST_अन्तः(शीर्ष)		रिक्त
#परिभाषय	SLIST_रिक्तानि(शीर्ष)	((शीर्ष)->slh_first == रिक्त)
#परिभाषय	SLIST_अग्र(elm, क्षेत्रम्)	((elm)->क्षेत्रम्.sle_अग्र)

#परिभाषय	SLIST_FOREACH(var, शीर्ष, क्षेत्रम्)					\
	प्रति((var) = (शीर्ष)->slh_first;					\
	    (var) != SLIST_अन्तः(शीर्ष);					\
	    (var) = (var)->क्षेत्रम्.sle_अग्र)

#परिभाषय	SLIST_FOREACH_SAFE(var, शीर्ष, क्षेत्रम्, tvar)			\
	प्रति ((var) = SLIST_FIRST((शीर्ष));				\
	    (var) != SLIST_अन्तः(शीर्ष) &&					\
	    ((tvar) = SLIST_अग्र((var), क्षेत्रम्), 1);			\
	    (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#परिभाषय	SLIST_INIT(शीर्ष) कुरु {						\
	(शीर्ष)->slh_first = SLIST_अन्तः(शीर्ष);				\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	SLIST_निवेशय_AFTER(slistelm, elm, क्षेत्रम्) कुरु {			\
	(elm)->क्षेत्रम्.sle_अग्र = (slistelm)->क्षेत्रम्.sle_अग्र;		\
	(slistelm)->क्षेत्रम्.sle_अग्र = (elm);				\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	SLIST_निवेशय_शीर्ष(शीर्ष, elm, क्षेत्रम्) कुरु {			\
	(elm)->क्षेत्रम्.sle_अग्र = (शीर्ष)->slh_first;			\
	(शीर्ष)->slh_first = (elm);					\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	SLIST_REMOVE_AFTER(slistelm, क्षेत्रम्) कुरु {			\
	(slistelm)->क्षेत्रम्.sle_अग्र =					\
	    SLIST_अग्र(SLIST_अग्र((slistelm), क्षेत्रम्), क्षेत्रम्);		\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	SLIST_REMOVE_शीर्ष(शीर्ष, क्षेत्रम्) कुरु {				\
	(शीर्ष)->slh_first = (शीर्ष)->slh_first->क्षेत्रम्.sle_अग्र;		\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	SLIST_REMOVE(शीर्ष, elm, type, क्षेत्रम्) कुरु {			\
	यदि ((शीर्ष)->slh_first == (elm)) {				\
		SLIST_REMOVE_शीर्ष((शीर्ष), क्षेत्रम्);			\
	}								\
	अन्यथा {								\
		संरचना type *curelm = (शीर्ष)->slh_first;		\
		यावत्(curelm->क्षेत्रम्.sle_अग्र != (elm))			\
			curelm = curelm->क्षेत्रम्.sle_अग्र;		\
		curelm->क्षेत्रम्.sle_अग्र =				\
		    curelm->क्षेत्रम्.sle_अग्र->क्षेत्रम्.sle_अग्र;		\
	}								\
} यावत् (/*CONSTCOND*/0)


/*
 * List definitions.
 */
#परिभाषय	LIST_शीर्ष(name, type)						\
संरचना name {								\
	संरचना type *lh_first;	/* first element */			\
}

#परिभाषय	LIST_शीर्ष_INITIALIZER(शीर्ष)					\
	{ रिक्त }

#परिभाषय	LIST_प्रविष्टिः(type)						\
संरचना {								\
	संरचना type *le_अग्र;	/* next element */			\
	संरचना type **le_पूर्व;	/* address of previous next element */	\
}

/*
 * List access methods.
 */
#परिभाषय	LIST_FIRST(शीर्ष)		((शीर्ष)->lh_first)
#परिभाषय	LIST_अन्तः(शीर्ष)			रिक्त
#परिभाषय	LIST_रिक्तानि(शीर्ष)		((शीर्ष)->lh_first == LIST_अन्तः(शीर्ष))
#परिभाषय	LIST_अग्र(elm, क्षेत्रम्)		((elm)->क्षेत्रम्.le_अग्र)

#परिभाषय	LIST_FOREACH(var, शीर्ष, क्षेत्रम्)					\
	प्रति ((var) = ((शीर्ष)->lh_first);				\
	    (var) != LIST_अन्तः(शीर्ष);					\
	    (var) = ((var)->क्षेत्रम्.le_अग्र))

#परिभाषय	LIST_FOREACH_SAFE(var, शीर्ष, क्षेत्रम्, tvar)			\
	प्रति ((var) = LIST_FIRST((शीर्ष));				\
	    (var) != LIST_अन्तः(शीर्ष) &&					\
	    ((tvar) = LIST_अग्र((var), क्षेत्रम्), 1);			\
	    (var) = (tvar))

#परिभाषय	LIST_MOVE(head1, head2) कुरु {					\
	LIST_INIT((head2));						\
	यदि (!LIST_रिक्तानि((head1))) {					\
		(head2)->lh_first = (head1)->lh_first;			\
		LIST_INIT((head1));					\
	}								\
} यावत् (/*CONSTCOND*/0)

/*
 * List functions.
 */
#यदि defined(QUEUEDEBUG)
#परिभाषय	QUEUEDEBUG_LIST_निवेशय_शीर्ष(शीर्ष, elm, क्षेत्रम्)			\
	यदि ((शीर्ष)->lh_first &&						\
	    (शीर्ष)->lh_first->क्षेत्रम्.le_पूर्व != &(शीर्ष)->lh_first)	\
		QUEUEDEBUG_ABORT("LIST_INSERT_HEAD %p %s:%d", (शीर्ष),	\
		    __FILE__, __LINE__);
#परिभाषय	QUEUEDEBUG_LIST_OP(elm, क्षेत्रम्)					\
	यदि ((elm)->क्षेत्रम्.le_अग्र &&					\
	    (elm)->क्षेत्रम्.le_अग्र->क्षेत्रम्.le_पूर्व !=			\
	    &(elm)->क्षेत्रम्.le_अग्र)					\
		QUEUEDEBUG_ABORT("LIST_* forw %p %s:%d", (elm),		\
		    __FILE__, __LINE__);				\
	यदि (*(elm)->क्षेत्रम्.le_पूर्व != (elm))				\
		QUEUEDEBUG_ABORT("LIST_* back %p %s:%d", (elm),		\
		    __FILE__, __LINE__);
#परिभाषय	QUEUEDEBUG_LIST_POSTREMOVE(elm, क्षेत्रम्)				\
	(elm)->क्षेत्रम्.le_अग्र = (शून्य *)1L;				\
	(elm)->क्षेत्रम्.le_पूर्व = (शून्य *)1L;
#अन्यथा
#परिभाषय	QUEUEDEBUG_LIST_निवेशय_शीर्ष(शीर्ष, elm, क्षेत्रम्)
#परिभाषय	QUEUEDEBUG_LIST_OP(elm, क्षेत्रम्)
#परिभाषय	QUEUEDEBUG_LIST_POSTREMOVE(elm, क्षेत्रम्)
#यदि_अन्तः

#परिभाषय	LIST_INIT(शीर्ष) कुरु {						\
	(शीर्ष)->lh_first = LIST_अन्तः(शीर्ष);				\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	LIST_निवेशय_AFTER(listelm, elm, क्षेत्रम्) कुरु {			\
	QUEUEDEBUG_LIST_OP((listelm), क्षेत्रम्)				\
	यदि (((elm)->क्षेत्रम्.le_अग्र = (listelm)->क्षेत्रम्.le_अग्र) != 	\
	    LIST_अन्तः(शीर्ष))						\
		(listelm)->क्षेत्रम्.le_अग्र->क्षेत्रम्.le_पूर्व =		\
		    &(elm)->क्षेत्रम्.le_अग्र;				\
	(listelm)->क्षेत्रम्.le_अग्र = (elm);				\
	(elm)->क्षेत्रम्.le_पूर्व = &(listelm)->क्षेत्रम्.le_अग्र;		\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	LIST_निवेशय_BEFORE(listelm, elm, क्षेत्रम्) कुरु {			\
	QUEUEDEBUG_LIST_OP((listelm), क्षेत्रम्)				\
	(elm)->क्षेत्रम्.le_पूर्व = (listelm)->क्षेत्रम्.le_पूर्व;		\
	(elm)->क्षेत्रम्.le_अग्र = (listelm);				\
	*(listelm)->क्षेत्रम्.le_पूर्व = (elm);				\
	(listelm)->क्षेत्रम्.le_पूर्व = &(elm)->क्षेत्रम्.le_अग्र;		\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	LIST_निवेशय_शीर्ष(शीर्ष, elm, क्षेत्रम्) कुरु {				\
	QUEUEDEBUG_LIST_निवेशय_शीर्ष((शीर्ष), (elm), क्षेत्रम्)		\
	यदि (((elm)->क्षेत्रम्.le_अग्र = (शीर्ष)->lh_first) != LIST_अन्तः(शीर्ष))\
		(शीर्ष)->lh_first->क्षेत्रम्.le_पूर्व = &(elm)->क्षेत्रम्.le_अग्र;\
	(शीर्ष)->lh_first = (elm);					\
	(elm)->क्षेत्रम्.le_पूर्व = &(शीर्ष)->lh_first;			\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	LIST_REMOVE(elm, क्षेत्रम्) कुरु {					\
	QUEUEDEBUG_LIST_OP((elm), क्षेत्रम्)				\
	यदि ((elm)->क्षेत्रम्.le_अग्र != रिक्त)				\
		(elm)->क्षेत्रम्.le_अग्र->क्षेत्रम्.le_पूर्व = 			\
		    (elm)->क्षेत्रम्.le_पूर्व;				\
	*(elm)->क्षेत्रम्.le_पूर्व = (elm)->क्षेत्रम्.le_अग्र;			\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), क्षेत्रम्)			\
} यावत् (/*CONSTCOND*/0)

#परिभाषय LIST_REPLACE(elm, elm2, क्षेत्रम्) कुरु {				\
	यदि (((elm2)->क्षेत्रम्.le_अग्र = (elm)->क्षेत्रम्.le_अग्र) != रिक्त)	\
		(elm2)->क्षेत्रम्.le_अग्र->क्षेत्रम्.le_पूर्व =			\
		    &(elm2)->क्षेत्रम्.le_अग्र;				\
	(elm2)->क्षेत्रम्.le_पूर्व = (elm)->क्षेत्रम्.le_पूर्व;			\
	*(elm2)->क्षेत्रम्.le_पूर्व = (elm2);				\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), क्षेत्रम्)			\
} यावत् (/*CONSTCOND*/0)

/*
 * Simple queue definitions.
 */
#परिभाषय	SIMPLEQ_शीर्ष(name, type)					\
संरचना name {								\
	संरचना type *sqh_first;	/* first element */			\
	संरचना type **sqh_चरमः;	/* addr of last next element */		\
}

#परिभाषय	SIMPLEQ_शीर्ष_INITIALIZER(शीर्ष)					\
	{ रिक्त, &(शीर्ष).sqh_first }

#परिभाषय	SIMPLEQ_प्रविष्टिः(type)						\
संरचना {								\
	संरचना type *sqe_अग्र;	/* next element */			\
}

/*
 * Simple queue access methods.
 */
#परिभाषय	SIMPLEQ_FIRST(शीर्ष)		((शीर्ष)->sqh_first)
#परिभाषय	SIMPLEQ_अन्तः(शीर्ष)		रिक्त
#परिभाषय	SIMPLEQ_रिक्तानि(शीर्ष)		((शीर्ष)->sqh_first == SIMPLEQ_अन्तः(शीर्ष))
#परिभाषय	SIMPLEQ_अग्र(elm, क्षेत्रम्)	((elm)->क्षेत्रम्.sqe_अग्र)

#परिभाषय	SIMPLEQ_FOREACH(var, शीर्ष, क्षेत्रम्)				\
	प्रति ((var) = ((शीर्ष)->sqh_first);				\
	    (var) != SIMPLEQ_अन्तः(शीर्ष);					\
	    (var) = ((var)->क्षेत्रम्.sqe_अग्र))

#परिभाषय	SIMPLEQ_FOREACH_SAFE(var, शीर्ष, क्षेत्रम्, अग्र)			\
	प्रति ((var) = ((शीर्ष)->sqh_first);				\
	    (var) != SIMPLEQ_अन्तः(शीर्ष) &&				\
	    ((अग्र = ((var)->क्षेत्रम्.sqe_अग्र)), 1);			\
	    (var) = (अग्र))

/*
 * Simple queue functions.
 */
#परिभाषय	SIMPLEQ_INIT(शीर्ष) कुरु {						\
	(शीर्ष)->sqh_first = रिक्त;					\
	(शीर्ष)->sqh_चरमः = &(शीर्ष)->sqh_first;				\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	SIMPLEQ_निवेशय_शीर्ष(शीर्ष, elm, क्षेत्रम्) कुरु {			\
	यदि (((elm)->क्षेत्रम्.sqe_अग्र = (शीर्ष)->sqh_first) == रिक्त)	\
		(शीर्ष)->sqh_चरमः = &(elm)->क्षेत्रम्.sqe_अग्र;		\
	(शीर्ष)->sqh_first = (elm);					\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	SIMPLEQ_निवेशय_पुच्छ(शीर्ष, elm, क्षेत्रम्) कुरु {			\
	(elm)->क्षेत्रम्.sqe_अग्र = रिक्त;					\
	*(शीर्ष)->sqh_चरमः = (elm);					\
	(शीर्ष)->sqh_चरमः = &(elm)->क्षेत्रम्.sqe_अग्र;			\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	SIMPLEQ_निवेशय_AFTER(शीर्ष, listelm, elm, क्षेत्रम्) कुरु {		\
	यदि (((elm)->क्षेत्रम्.sqe_अग्र = (listelm)->क्षेत्रम्.sqe_अग्र) == रिक्त)\
		(शीर्ष)->sqh_चरमः = &(elm)->क्षेत्रम्.sqe_अग्र;		\
	(listelm)->क्षेत्रम्.sqe_अग्र = (elm);				\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	SIMPLEQ_REMOVE_शीर्ष(शीर्ष, क्षेत्रम्) कुरु {				\
	यदि (((शीर्ष)->sqh_first = (शीर्ष)->sqh_first->क्षेत्रम्.sqe_अग्र) == रिक्त) \
		(शीर्ष)->sqh_चरमः = &(शीर्ष)->sqh_first;			\
} यावत् (/*CONSTCOND*/0)

#परिभाषय SIMPLEQ_REMOVE_AFTER(शीर्ष, elm, क्षेत्रम्) कुरु {			\
	यदि (((elm)->क्षेत्रम्.sqe_अग्र = (elm)->क्षेत्रम्.sqe_अग्र->क्षेत्रम्.sqe_अग्र) \
	    == रिक्त)							\
		(शीर्ष)->sqh_चरमः = &(elm)->क्षेत्रम्.sqe_अग्र;		\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	SIMPLEQ_REMOVE(शीर्ष, elm, type, क्षेत्रम्) कुरु {			\
	यदि ((शीर्ष)->sqh_first == (elm)) {				\
		SIMPLEQ_REMOVE_शीर्ष((शीर्ष), क्षेत्रम्);			\
	} अन्यथा {							\
		संरचना type *curelm = (शीर्ष)->sqh_first;		\
		यावत् (curelm->क्षेत्रम्.sqe_अग्र != (elm))			\
			curelm = curelm->क्षेत्रम्.sqe_अग्र;		\
		यदि ((curelm->क्षेत्रम्.sqe_अग्र =				\
			curelm->क्षेत्रम्.sqe_अग्र->क्षेत्रम्.sqe_अग्र) == रिक्त) \
			    (शीर्ष)->sqh_चरमः = &(curelm)->क्षेत्रम्.sqe_अग्र; \
	}								\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	SIMPLEQ_CONCAT(head1, head2) कुरु {				\
	यदि (!SIMPLEQ_रिक्तानि((head2))) {					\
		*(head1)->sqh_चरमः = (head2)->sqh_first;		\
		(head1)->sqh_चरमः = (head2)->sqh_चरमः;		\
		SIMPLEQ_INIT((head2));					\
	}								\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	SIMPLEQ_चरमः(शीर्ष, type, क्षेत्रम्)					\
	(SIMPLEQ_रिक्तानि((शीर्ष)) ?						\
		रिक्त :							\
	        ((संरचना type *)(शून्य *)				\
		((वर्ण *)((शीर्ष)->sqh_चरमः) - offsetof(संरचना type, क्षेत्रम्))))

/*
 * Tail queue definitions.
 */
#परिभाषय	TAILQ_शीर्ष_2(name, type, qual)					\
संरचना name {								\
	qual type *tqh_first;		/* first element */		\
	qual type *qual *tqh_चरमः;	/* addr of last next element */	\
}
#परिभाषय TAILQ_शीर्ष(name, type)	TAILQ_शीर्ष_2(name, संरचना type,)

#परिभाषय	TAILQ_शीर्ष_INITIALIZER(शीर्ष)					\
	{ TAILQ_अन्तः(शीर्ष), &(शीर्ष).tqh_first }

#परिभाषय	TAILQ_प्रविष्टिः_2(type, qual)					\
संरचना {								\
	qual type *tqe_अग्र;		/* next element */		\
	qual type *qual *tqe_पूर्व;	/* address of previous next element */\
}
#परिभाषय TAILQ_प्रविष्टिः(type)	TAILQ_प्रविष्टिः_2(संरचना type,)

/*
 * Tail queue access methods.
 */
#परिभाषय	TAILQ_FIRST(शीर्ष)		((शीर्ष)->tqh_first)
#परिभाषय	TAILQ_अन्तः(शीर्ष)			(रिक्त)
#परिभाषय	TAILQ_अग्र(elm, क्षेत्रम्)		((elm)->क्षेत्रम्.tqe_अग्र)
#परिभाषय	TAILQ_चरमः(शीर्ष, headname) \
	(*(((संरचना headname *)((शीर्ष)->tqh_चरमः))->tqh_चरमः))
#परिभाषय	TAILQ_पूर्व(elm, headname, क्षेत्रम्) \
	(*(((संरचना headname *)((elm)->क्षेत्रम्.tqe_पूर्व))->tqh_चरमः))
#परिभाषय	TAILQ_रिक्तानि(शीर्ष)		(TAILQ_FIRST(शीर्ष) == TAILQ_अन्तः(शीर्ष))


#परिभाषय	TAILQ_FOREACH(var, शीर्ष, क्षेत्रम्)					\
	प्रति ((var) = ((शीर्ष)->tqh_first);				\
	    (var) != TAILQ_अन्तः(शीर्ष);					\
	    (var) = ((var)->क्षेत्रम्.tqe_अग्र))

#परिभाषय	TAILQ_FOREACH_SAFE(var, शीर्ष, क्षेत्रम्, अग्र)			\
	प्रति ((var) = ((शीर्ष)->tqh_first);				\
	    (var) != TAILQ_अन्तः(शीर्ष) &&					\
	    ((अग्र) = TAILQ_अग्र(var, क्षेत्रम्), 1); (var) = (अग्र))

#परिभाषय	TAILQ_FOREACH_REVERSE(var, शीर्ष, headname, क्षेत्रम्)		\
	प्रति ((var) = (*(((संरचना headname *)((शीर्ष)->tqh_चरमः))->tqh_चरमः));\
	    (var) != TAILQ_अन्तः(शीर्ष);					\
	    (var) = (*(((संरचना headname *)((var)->क्षेत्रम्.tqe_पूर्व))->tqh_चरमः)))

#परिभाषय	TAILQ_FOREACH_REVERSE_SAFE(var, शीर्ष, headname, क्षेत्रम्, पूर्व)	\
	प्रति ((var) = TAILQ_चरमः((शीर्ष), headname);			\
	    (var) != TAILQ_अन्तः(शीर्ष) && 				\
	    ((पूर्व) = TAILQ_पूर्व((var), headname, क्षेत्रम्), 1); (var) = (पूर्व))

/*
 * Tail queue functions.
 */
#यदि defined(QUEUEDEBUG)
#परिभाषय	QUEUEDEBUG_TAILQ_निवेशय_शीर्ष(शीर्ष, elm, क्षेत्रम्)			\
	यदि ((शीर्ष)->tqh_first &&					\
	    (शीर्ष)->tqh_first->क्षेत्रम्.tqe_पूर्व != &(शीर्ष)->tqh_first)	\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_HEAD %p %s:%d", (शीर्ष),	\
		    __FILE__, __LINE__);
#परिभाषय	QUEUEDEBUG_TAILQ_निवेशय_पुच्छ(शीर्ष, elm, क्षेत्रम्)			\
	यदि (*(शीर्ष)->tqh_चरमः != रिक्त)					\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_TAIL %p %s:%d", (शीर्ष),	\
		    __FILE__, __LINE__);
#परिभाषय	QUEUEDEBUG_TAILQ_OP(elm, क्षेत्रम्)					\
	यदि ((elm)->क्षेत्रम्.tqe_अग्र &&					\
	    (elm)->क्षेत्रम्.tqe_अग्र->क्षेत्रम्.tqe_पूर्व !=			\
	    &(elm)->क्षेत्रम्.tqe_अग्र)					\
		QUEUEDEBUG_ABORT("TAILQ_* forw %p %s:%d", (elm),	\
		    __FILE__, __LINE__);				\
	यदि (*(elm)->क्षेत्रम्.tqe_पूर्व != (elm))				\
		QUEUEDEBUG_ABORT("TAILQ_* back %p %s:%d", (elm),	\
		    __FILE__, __LINE__);
#परिभाषय	QUEUEDEBUG_TAILQ_PREREMOVE(शीर्ष, elm, क्षेत्रम्)			\
	यदि ((elm)->क्षेत्रम्.tqe_अग्र == रिक्त &&				\
	    (शीर्ष)->tqh_चरमः != &(elm)->क्षेत्रम्.tqe_अग्र)			\
		QUEUEDEBUG_ABORT("TAILQ_PREREMOVE head %p elm %p %s:%d",\
		    (शीर्ष), (elm), __FILE__, __LINE__);
#परिभाषय	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, क्षेत्रम्)				\
	(elm)->क्षेत्रम्.tqe_अग्र = (शून्य *)1L;				\
	(elm)->क्षेत्रम्.tqe_पूर्व = (शून्य *)1L;
#अन्यथा
#परिभाषय	QUEUEDEBUG_TAILQ_निवेशय_शीर्ष(शीर्ष, elm, क्षेत्रम्)
#परिभाषय	QUEUEDEBUG_TAILQ_निवेशय_पुच्छ(शीर्ष, elm, क्षेत्रम्)
#परिभाषय	QUEUEDEBUG_TAILQ_OP(elm, क्षेत्रम्)
#परिभाषय	QUEUEDEBUG_TAILQ_PREREMOVE(शीर्ष, elm, क्षेत्रम्)
#परिभाषय	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, क्षेत्रम्)
#यदि_अन्तः

#परिभाषय	TAILQ_INIT(शीर्ष) कुरु {						\
	(शीर्ष)->tqh_first = TAILQ_अन्तः(शीर्ष);				\
	(शीर्ष)->tqh_चरमः = &(शीर्ष)->tqh_first;				\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	TAILQ_निवेशय_शीर्ष(शीर्ष, elm, क्षेत्रम्) कुरु {			\
	QUEUEDEBUG_TAILQ_निवेशय_शीर्ष((शीर्ष), (elm), क्षेत्रम्)		\
	यदि (((elm)->क्षेत्रम्.tqe_अग्र = (शीर्ष)->tqh_first) != TAILQ_अन्तः(शीर्ष))\
		(शीर्ष)->tqh_first->क्षेत्रम्.tqe_पूर्व =			\
		    &(elm)->क्षेत्रम्.tqe_अग्र;				\
	अन्यथा								\
		(शीर्ष)->tqh_चरमः = &(elm)->क्षेत्रम्.tqe_अग्र;		\
	(शीर्ष)->tqh_first = (elm);					\
	(elm)->क्षेत्रम्.tqe_पूर्व = &(शीर्ष)->tqh_first;			\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	TAILQ_निवेशय_पुच्छ(शीर्ष, elm, क्षेत्रम्) कुरु {			\
	QUEUEDEBUG_TAILQ_निवेशय_पुच्छ((शीर्ष), (elm), क्षेत्रम्)		\
	(elm)->क्षेत्रम्.tqe_अग्र = TAILQ_अन्तः(शीर्ष);			\
	(elm)->क्षेत्रम्.tqe_पूर्व = (शीर्ष)->tqh_चरमः;			\
	*(शीर्ष)->tqh_चरमः = (elm);					\
	(शीर्ष)->tqh_चरमः = &(elm)->क्षेत्रम्.tqe_अग्र;			\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	TAILQ_निवेशय_AFTER(शीर्ष, listelm, elm, क्षेत्रम्) कुरु {		\
	QUEUEDEBUG_TAILQ_OP((listelm), क्षेत्रम्)				\
	यदि (((elm)->क्षेत्रम्.tqe_अग्र = (listelm)->क्षेत्रम्.tqe_अग्र) != 	\
	    TAILQ_अन्तः(शीर्ष))						\
		(elm)->क्षेत्रम्.tqe_अग्र->क्षेत्रम्.tqe_पूर्व = 		\
		    &(elm)->क्षेत्रम्.tqe_अग्र;				\
	अन्यथा								\
		(शीर्ष)->tqh_चरमः = &(elm)->क्षेत्रम्.tqe_अग्र;		\
	(listelm)->क्षेत्रम्.tqe_अग्र = (elm);				\
	(elm)->क्षेत्रम्.tqe_पूर्व = &(listelm)->क्षेत्रम्.tqe_अग्र;		\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	TAILQ_निवेशय_BEFORE(listelm, elm, क्षेत्रम्) कुरु {			\
	QUEUEDEBUG_TAILQ_OP((listelm), क्षेत्रम्)				\
	(elm)->क्षेत्रम्.tqe_पूर्व = (listelm)->क्षेत्रम्.tqe_पूर्व;		\
	(elm)->क्षेत्रम्.tqe_अग्र = (listelm);				\
	*(listelm)->क्षेत्रम्.tqe_पूर्व = (elm);				\
	(listelm)->क्षेत्रम्.tqe_पूर्व = &(elm)->क्षेत्रम्.tqe_अग्र;		\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	TAILQ_REMOVE(शीर्ष, elm, क्षेत्रम्) कुरु {				\
	QUEUEDEBUG_TAILQ_PREREMOVE((शीर्ष), (elm), क्षेत्रम्)		\
	QUEUEDEBUG_TAILQ_OP((elm), क्षेत्रम्)				\
	यदि (((elm)->क्षेत्रम्.tqe_अग्र) != TAILQ_अन्तः(शीर्ष))			\
		(elm)->क्षेत्रम्.tqe_अग्र->क्षेत्रम्.tqe_पूर्व = 		\
		    (elm)->क्षेत्रम्.tqe_पूर्व;				\
	अन्यथा								\
		(शीर्ष)->tqh_चरमः = (elm)->क्षेत्रम्.tqe_पूर्व;		\
	*(elm)->क्षेत्रम्.tqe_पूर्व = (elm)->क्षेत्रम्.tqe_अग्र;			\
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), क्षेत्रम्);			\
} यावत् (/*CONSTCOND*/0)

#परिभाषय TAILQ_REPLACE(शीर्ष, elm, elm2, क्षेत्रम्) कुरु {                      \
        यदि (((elm2)->क्षेत्रम्.tqe_अग्र = (elm)->क्षेत्रम्.tqe_अग्र) != 	\
	    TAILQ_अन्तः(शीर्ष))   						\
                (elm2)->क्षेत्रम्.tqe_अग्र->क्षेत्रम्.tqe_पूर्व =                \
                    &(elm2)->क्षेत्रम्.tqe_अग्र;                            \
        अन्यथा                                                            \
                (शीर्ष)->tqh_चरमः = &(elm2)->क्षेत्रम्.tqe_अग्र;             \
        (elm2)->क्षेत्रम्.tqe_पूर्व = (elm)->क्षेत्रम्.tqe_पूर्व;                 \
        *(elm2)->क्षेत्रम्.tqe_पूर्व = (elm2);                               \
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), क्षेत्रम्);			\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	TAILQ_CONCAT(head1, head2, क्षेत्रम्) कुरु {				\
	यदि (!TAILQ_रिक्तानि(head2)) {					\
		*(head1)->tqh_चरमः = (head2)->tqh_first;		\
		(head2)->tqh_first->क्षेत्रम्.tqe_पूर्व = (head1)->tqh_चरमः;	\
		(head1)->tqh_चरमः = (head2)->tqh_चरमः;			\
		TAILQ_INIT((head2));					\
	}								\
} यावत् (/*CONSTCOND*/0)

/*
 * Singly-linked Tail queue declarations.
 */
#परिभाषय	STAILQ_शीर्ष(name, type)						\
संरचना name {								\
	संरचना type *stqh_first;	/* first element */		\
	संरचना type **stqh_चरमः;	/* addr of last next element */	\
}

#परिभाषय	STAILQ_शीर्ष_INITIALIZER(शीर्ष)					\
	{ रिक्त, &(शीर्ष).stqh_first }

#परिभाषय	STAILQ_प्रविष्टिः(type)						\
संरचना {								\
	संरचना type *stqe_अग्र;	/* next element */			\
}

/*
 * Singly-linked Tail queue access methods.
 */
#परिभाषय	STAILQ_FIRST(शीर्ष)	((शीर्ष)->stqh_first)
#परिभाषय	STAILQ_अन्तः(शीर्ष)	रिक्त
#परिभाषय	STAILQ_अग्र(elm, क्षेत्रम्)	((elm)->क्षेत्रम्.stqe_अग्र)
#परिभाषय	STAILQ_रिक्तानि(शीर्ष)	(STAILQ_FIRST(शीर्ष) == STAILQ_अन्तः(शीर्ष))

/*
 * Singly-linked Tail queue functions.
 */
#परिभाषय	STAILQ_INIT(शीर्ष) कुरु {						\
	(शीर्ष)->stqh_first = रिक्त;					\
	(शीर्ष)->stqh_चरमः = &(शीर्ष)->stqh_first;				\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	STAILQ_निवेशय_शीर्ष(शीर्ष, elm, क्षेत्रम्) कुरु {			\
	यदि (((elm)->क्षेत्रम्.stqe_अग्र = (शीर्ष)->stqh_first) == रिक्त)	\
		(शीर्ष)->stqh_चरमः = &(elm)->क्षेत्रम्.stqe_अग्र;		\
	(शीर्ष)->stqh_first = (elm);					\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	STAILQ_निवेशय_पुच्छ(शीर्ष, elm, क्षेत्रम्) कुरु {			\
	(elm)->क्षेत्रम्.stqe_अग्र = रिक्त;					\
	*(शीर्ष)->stqh_चरमः = (elm);					\
	(शीर्ष)->stqh_चरमः = &(elm)->क्षेत्रम्.stqe_अग्र;			\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	STAILQ_निवेशय_AFTER(शीर्ष, listelm, elm, क्षेत्रम्) कुरु {		\
	यदि (((elm)->क्षेत्रम्.stqe_अग्र = (listelm)->क्षेत्रम्.stqe_अग्र) == रिक्त)\
		(शीर्ष)->stqh_चरमः = &(elm)->क्षेत्रम्.stqe_अग्र;		\
	(listelm)->क्षेत्रम्.stqe_अग्र = (elm);				\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	STAILQ_REMOVE_शीर्ष(शीर्ष, क्षेत्रम्) कुरु {				\
	यदि (((शीर्ष)->stqh_first = (शीर्ष)->stqh_first->क्षेत्रम्.stqe_अग्र) == रिक्त) \
		(शीर्ष)->stqh_चरमः = &(शीर्ष)->stqh_first;			\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	STAILQ_REMOVE(शीर्ष, elm, type, क्षेत्रम्) कुरु {			\
	यदि ((शीर्ष)->stqh_first == (elm)) {				\
		STAILQ_REMOVE_शीर्ष((शीर्ष), क्षेत्रम्);			\
	} अन्यथा {							\
		संरचना type *curelm = (शीर्ष)->stqh_first;		\
		यावत् (curelm->क्षेत्रम्.stqe_अग्र != (elm))			\
			curelm = curelm->क्षेत्रम्.stqe_अग्र;		\
		यदि ((curelm->क्षेत्रम्.stqe_अग्र =				\
			curelm->क्षेत्रम्.stqe_अग्र->क्षेत्रम्.stqe_अग्र) == रिक्त) \
			    (शीर्ष)->stqh_चरमः = &(curelm)->क्षेत्रम्.stqe_अग्र; \
	}								\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	STAILQ_FOREACH(var, शीर्ष, क्षेत्रम्)				\
	प्रति ((var) = ((शीर्ष)->stqh_first);				\
		(var);							\
		(var) = ((var)->क्षेत्रम्.stqe_अग्र))

#परिभाषय	STAILQ_FOREACH_SAFE(var, शीर्ष, क्षेत्रम्, tvar)			\
	प्रति ((var) = STAILQ_FIRST((शीर्ष));				\
	    (var) && ((tvar) = STAILQ_अग्र((var), क्षेत्रम्), 1);		\
	    (var) = (tvar))

#परिभाषय	STAILQ_CONCAT(head1, head2) कुरु {				\
	यदि (!STAILQ_रिक्तानि((head2))) {					\
		*(head1)->stqh_चरमः = (head2)->stqh_first;		\
		(head1)->stqh_चरमः = (head2)->stqh_चरमः;		\
		STAILQ_INIT((head2));					\
	}								\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	STAILQ_चरमः(शीर्ष, type, क्षेत्रम्)					\
	(STAILQ_रिक्तानि((शीर्ष)) ?						\
		रिक्त :							\
	        ((संरचना type *)(शून्य *)				\
		((वर्ण *)((शीर्ष)->stqh_चरमः) - offsetof(संरचना type, क्षेत्रम्))))


#यदि_अपरिभाषितम् _KERNEL
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
स्थिर __inline ध्रुव शून्य * __launder_type(ध्रुव शून्य *);
स्थिर __inline ध्रुव शून्य *
__launder_type(ध्रुव शून्य *__x)
{
	__asm __volatile("" : "+r" (__x));
	प्रत्यागम __x;
}

#यदि defined(QUEUEDEBUG)
#परिभाषय QUEUEDEBUG_CIRCLEQ_शीर्ष(शीर्ष, क्षेत्रम्)				\
	यदि ((शीर्ष)->cqh_first != CIRCLEQ_ENDC(शीर्ष) &&			\
	    (शीर्ष)->cqh_first->क्षेत्रम्.cqe_पूर्व != CIRCLEQ_ENDC(शीर्ष))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head forw %p %s:%d", (शीर्ष),	\
		      __FILE__, __LINE__);				\
	यदि ((शीर्ष)->cqh_चरमः != CIRCLEQ_ENDC(शीर्ष) &&			\
	    (शीर्ष)->cqh_चरमः->क्षेत्रम्.cqe_अग्र != CIRCLEQ_ENDC(शीर्ष))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head back %p %s:%d", (शीर्ष),	\
		      __FILE__, __LINE__);
#परिभाषय QUEUEDEBUG_CIRCLEQ_ELM(शीर्ष, elm, क्षेत्रम्)			\
	यदि ((elm)->क्षेत्रम्.cqe_अग्र == CIRCLEQ_ENDC(शीर्ष)) {		\
		यदि ((शीर्ष)->cqh_चरमः != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm last %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} अन्यथा {							\
		यदि ((elm)->क्षेत्रम्.cqe_अग्र->क्षेत्रम्.cqe_पूर्व != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm forw %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}								\
	यदि ((elm)->क्षेत्रम्.cqe_पूर्व == CIRCLEQ_ENDC(शीर्ष)) {		\
		यदि ((शीर्ष)->cqh_first != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm first %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} अन्यथा {							\
		यदि ((elm)->क्षेत्रम्.cqe_पूर्व->क्षेत्रम्.cqe_अग्र != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm prev %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}
#परिभाषय QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, क्षेत्रम्)			\
	(elm)->क्षेत्रम्.cqe_अग्र = (शून्य *)1L;				\
	(elm)->क्षेत्रम्.cqe_पूर्व = (शून्य *)1L;
#अन्यथा
#परिभाषय QUEUEDEBUG_CIRCLEQ_शीर्ष(शीर्ष, क्षेत्रम्)
#परिभाषय QUEUEDEBUG_CIRCLEQ_ELM(शीर्ष, elm, क्षेत्रम्)
#परिभाषय QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, क्षेत्रम्)
#यदि_अन्तः

#परिभाषय	CIRCLEQ_शीर्ष(name, type)					\
संरचना name {								\
	संरचना type *cqh_first;		/* first element */		\
	संरचना type *cqh_चरमः;		/* last element */		\
}

#परिभाषय	CIRCLEQ_शीर्ष_INITIALIZER(शीर्ष)					\
	{ CIRCLEQ_अन्तः(&शीर्ष), CIRCLEQ_अन्तः(&शीर्ष) }

#परिभाषय	CIRCLEQ_प्रविष्टिः(type)						\
संरचना {								\
	संरचना type *cqe_अग्र;		/* next element */		\
	संरचना type *cqe_पूर्व;		/* previous element */		\
}

/*
 * Circular queue functions.
 */
#परिभाषय	CIRCLEQ_INIT(शीर्ष) कुरु {						\
	(शीर्ष)->cqh_first = CIRCLEQ_अन्तः(शीर्ष);				\
	(शीर्ष)->cqh_चरमः = CIRCLEQ_अन्तः(शीर्ष);				\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	CIRCLEQ_निवेशय_AFTER(शीर्ष, listelm, elm, क्षेत्रम्) कुरु {		\
	QUEUEDEBUG_CIRCLEQ_शीर्ष((शीर्ष), क्षेत्रम्)				\
	QUEUEDEBUG_CIRCLEQ_ELM((शीर्ष), (listelm), क्षेत्रम्)		\
	(elm)->क्षेत्रम्.cqe_अग्र = (listelm)->क्षेत्रम्.cqe_अग्र;		\
	(elm)->क्षेत्रम्.cqe_पूर्व = (listelm);				\
	यदि ((listelm)->क्षेत्रम्.cqe_अग्र == CIRCLEQ_ENDC(शीर्ष))		\
		(शीर्ष)->cqh_चरमः = (elm);				\
	अन्यथा								\
		(listelm)->क्षेत्रम्.cqe_अग्र->क्षेत्रम्.cqe_पूर्व = (elm);	\
	(listelm)->क्षेत्रम्.cqe_अग्र = (elm);				\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	CIRCLEQ_निवेशय_BEFORE(शीर्ष, listelm, elm, क्षेत्रम्) कुरु {		\
	QUEUEDEBUG_CIRCLEQ_शीर्ष((शीर्ष), क्षेत्रम्)				\
	QUEUEDEBUG_CIRCLEQ_ELM((शीर्ष), (listelm), क्षेत्रम्)		\
	(elm)->क्षेत्रम्.cqe_अग्र = (listelm);				\
	(elm)->क्षेत्रम्.cqe_पूर्व = (listelm)->क्षेत्रम्.cqe_पूर्व;		\
	यदि ((listelm)->क्षेत्रम्.cqe_पूर्व == CIRCLEQ_ENDC(शीर्ष))		\
		(शीर्ष)->cqh_first = (elm);				\
	अन्यथा								\
		(listelm)->क्षेत्रम्.cqe_पूर्व->क्षेत्रम्.cqe_अग्र = (elm);	\
	(listelm)->क्षेत्रम्.cqe_पूर्व = (elm);				\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	CIRCLEQ_निवेशय_शीर्ष(शीर्ष, elm, क्षेत्रम्) कुरु {			\
	QUEUEDEBUG_CIRCLEQ_शीर्ष((शीर्ष), क्षेत्रम्)				\
	(elm)->क्षेत्रम्.cqe_अग्र = (शीर्ष)->cqh_first;			\
	(elm)->क्षेत्रम्.cqe_पूर्व = CIRCLEQ_अन्तः(शीर्ष);			\
	यदि ((शीर्ष)->cqh_चरमः == CIRCLEQ_ENDC(शीर्ष))			\
		(शीर्ष)->cqh_चरमः = (elm);				\
	अन्यथा								\
		(शीर्ष)->cqh_first->क्षेत्रम्.cqe_पूर्व = (elm);		\
	(शीर्ष)->cqh_first = (elm);					\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	CIRCLEQ_निवेशय_पुच्छ(शीर्ष, elm, क्षेत्रम्) कुरु {			\
	QUEUEDEBUG_CIRCLEQ_शीर्ष((शीर्ष), क्षेत्रम्)				\
	(elm)->क्षेत्रम्.cqe_अग्र = CIRCLEQ_अन्तः(शीर्ष);			\
	(elm)->क्षेत्रम्.cqe_पूर्व = (शीर्ष)->cqh_चरमः;			\
	यदि ((शीर्ष)->cqh_first == CIRCLEQ_ENDC(शीर्ष))			\
		(शीर्ष)->cqh_first = (elm);				\
	अन्यथा								\
		(शीर्ष)->cqh_चरमः->क्षेत्रम्.cqe_अग्र = (elm);		\
	(शीर्ष)->cqh_चरमः = (elm);					\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	CIRCLEQ_REMOVE(शीर्ष, elm, क्षेत्रम्) कुरु {				\
	QUEUEDEBUG_CIRCLEQ_शीर्ष((शीर्ष), क्षेत्रम्)				\
	QUEUEDEBUG_CIRCLEQ_ELM((शीर्ष), (elm), क्षेत्रम्)			\
	यदि ((elm)->क्षेत्रम्.cqe_अग्र == CIRCLEQ_ENDC(शीर्ष))		\
		(शीर्ष)->cqh_चरमः = (elm)->क्षेत्रम्.cqe_पूर्व;		\
	अन्यथा								\
		(elm)->क्षेत्रम्.cqe_अग्र->क्षेत्रम्.cqe_पूर्व =			\
		    (elm)->क्षेत्रम्.cqe_पूर्व;				\
	यदि ((elm)->क्षेत्रम्.cqe_पूर्व == CIRCLEQ_ENDC(शीर्ष))		\
		(शीर्ष)->cqh_first = (elm)->क्षेत्रम्.cqe_अग्र;		\
	अन्यथा								\
		(elm)->क्षेत्रम्.cqe_पूर्व->क्षेत्रम्.cqe_अग्र =			\
		    (elm)->क्षेत्रम्.cqe_अग्र;				\
	QUEUEDEBUG_CIRCLEQ_POSTREMOVE((elm), क्षेत्रम्)			\
} यावत् (/*CONSTCOND*/0)

#परिभाषय	CIRCLEQ_FOREACH(var, शीर्ष, क्षेत्रम्)				\
	प्रति ((var) = ((शीर्ष)->cqh_first);				\
		(var) != CIRCLEQ_ENDC(शीर्ष);				\
		(var) = ((var)->क्षेत्रम्.cqe_अग्र))

#परिभाषय	CIRCLEQ_FOREACH_REVERSE(var, शीर्ष, क्षेत्रम्)			\
	प्रति ((var) = ((शीर्ष)->cqh_चरमः);				\
		(var) != CIRCLEQ_ENDC(शीर्ष);				\
		(var) = ((var)->क्षेत्रम्.cqe_पूर्व))

/*
 * Circular queue access methods.
 */
#परिभाषय	CIRCLEQ_FIRST(शीर्ष)		((शीर्ष)->cqh_first)
#परिभाषय	CIRCLEQ_चरमः(शीर्ष)		((शीर्ष)->cqh_चरमः)
/* For comparisons */
#परिभाषय	CIRCLEQ_ENDC(शीर्ष)		(__launder_type(शीर्ष))
/* For assignments */
#परिभाषय	CIRCLEQ_अन्तः(शीर्ष)		((शून्य *)(शीर्ष))
#परिभाषय	CIRCLEQ_अग्र(elm, क्षेत्रम्)	((elm)->क्षेत्रम्.cqe_अग्र)
#परिभाषय	CIRCLEQ_पूर्व(elm, क्षेत्रम्)	((elm)->क्षेत्रम्.cqe_पूर्व)
#परिभाषय	CIRCLEQ_रिक्तानि(शीर्ष)						\
    (CIRCLEQ_FIRST(शीर्ष) == CIRCLEQ_ENDC(शीर्ष))

#परिभाषय CIRCLEQ_LOOP_अग्र(शीर्ष, elm, क्षेत्रम्)				\
	(((elm)->क्षेत्रम्.cqe_अग्र == CIRCLEQ_ENDC(शीर्ष))			\
	    ? ((शीर्ष)->cqh_first)					\
	    : (elm->क्षेत्रम्.cqe_अग्र))
#परिभाषय CIRCLEQ_LOOP_पूर्व(शीर्ष, elm, क्षेत्रम्)				\
	(((elm)->क्षेत्रम्.cqe_पूर्व == CIRCLEQ_ENDC(शीर्ष))			\
	    ? ((शीर्ष)->cqh_चरमः)					\
	    : (elm->क्षेत्रम्.cqe_पूर्व))
#यदि_अन्तः /* !_KERNEL */

#यदि_अन्तः	/* !_SYS_QUEUE_H_ */
