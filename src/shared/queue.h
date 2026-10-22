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

#ifndef	_SYS_QUEUE_H_
#define	_SYS_QUEUE_H_

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
#ifdef __NetBSD__
#অন্তর্ভাব <sys/null.h>
#endif

#যদি defined(QUEUEDEBUG)
# যদি defined(_KERNEL)
#  define QUEUEDEBUG_ABORT(...) panic(__VA_ARGS__)
# অন্যথা
#  include <err.h>
#  define QUEUEDEBUG_ABORT(...) err(1, __VA_ARGS__)
# endif
#endif

/*
 * Singly-linked List definitions.
 */
#define	SLIST_শীর্ষ(name, type)						\
কাঠামো name {								\
	কাঠামো type *slh_first;	/* first element */			\
}

#define	SLIST_শীর্ষ_INITIALIZER(শীর্ষ_2)					\
	{ ফাঁকা }

#define	SLIST_ENTRY(type)						\
কাঠামো {								\
	কাঠামো type *sle_পরবর্তী;	/* next element */			\
}

/*
 * Singly-linked List access methods.
 */
#define	SLIST_FIRST(শীর্ষ_2)	((শীর্ষ_2)->slh_first)
#define	SLIST_END(শীর্ষ_2)		ফাঁকা
#define	SLIST_EMPTY(শীর্ষ_2)	((শীর্ষ_2)->slh_first == ফাঁকা)
#define	SLIST_পরবর্তী(elm, field)	((elm)->field.sle_পরবর্তী)

#define	SLIST_FOREACH(var, শীর্ষ_2, field)					\
	জন্য((var) = (শীর্ষ_2)->slh_first;					\
	    (var) != SLIST_END(শীর্ষ_2);					\
	    (var) = (var)->field.sle_পরবর্তী)

#define	SLIST_FOREACH_SAFE(var, শীর্ষ_2, field, tvar)			\
	জন্য ((var) = SLIST_FIRST((শীর্ষ_2));				\
	    (var) != SLIST_END(শীর্ষ_2) &&					\
	    ((tvar) = SLIST_পরবর্তী((var), field), 1);			\
	    (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#define	SLIST_INIT(শীর্ষ_2) করো {						\
	(শীর্ষ_2)->slh_first = SLIST_END(শীর্ষ_2);				\
} যখন (/*CONSTCOND*/0)

#define	SLIST_ঢোকাও_AFTER(slistelm, elm, field) করো {			\
	(elm)->field.sle_পরবর্তী = (slistelm)->field.sle_পরবর্তী;		\
	(slistelm)->field.sle_পরবর্তী = (elm);				\
} যখন (/*CONSTCOND*/0)

#define	SLIST_ঢোকাও_শীর্ষ(শীর্ষ_2, elm, field) করো {			\
	(elm)->field.sle_পরবর্তী = (শীর্ষ_2)->slh_first;			\
	(শীর্ষ_2)->slh_first = (elm);					\
} যখন (/*CONSTCOND*/0)

#define	SLIST_REMOVE_AFTER(slistelm, field) করো {			\
	(slistelm)->field.sle_পরবর্তী =					\
	    SLIST_পরবর্তী(SLIST_পরবর্তী((slistelm), field), field);		\
} যখন (/*CONSTCOND*/0)

#define	SLIST_REMOVE_শীর্ষ(শীর্ষ_2, field) করো {				\
	(শীর্ষ_2)->slh_first = (শীর্ষ_2)->slh_first->field.sle_পরবর্তী;		\
} যখন (/*CONSTCOND*/0)

#define	SLIST_REMOVE(শীর্ষ_2, elm, type, field) করো {			\
	যদি ((শীর্ষ_2)->slh_first == (elm)) {				\
		SLIST_REMOVE_শীর্ষ((শীর্ষ_2), field);			\
	}								\
	অন্যথা {								\
		কাঠামো type *curelm = (শীর্ষ_2)->slh_first;		\
		যখন(curelm->field.sle_পরবর্তী != (elm))			\
			curelm = curelm->field.sle_পরবর্তী;		\
		curelm->field.sle_পরবর্তী =				\
		    curelm->field.sle_পরবর্তী->field.sle_পরবর্তী;		\
	}								\
} যখন (/*CONSTCOND*/0)


/*
 * List definitions.
 */
#define	LIST_শীর্ষ(name, type)						\
কাঠামো name {								\
	কাঠামো type *lh_first;	/* first element */			\
}

#define	LIST_শীর্ষ_INITIALIZER(শীর্ষ_2)					\
	{ ফাঁকা }

#define	LIST_ENTRY(type)						\
কাঠামো {								\
	কাঠামো type *le_পরবর্তী;	/* next element */			\
	কাঠামো type **le_পূর্ব;	/* address of previous next element */	\
}

/*
 * List access methods.
 */
#define	LIST_FIRST(শীর্ষ_2)		((শীর্ষ_2)->lh_first)
#define	LIST_END(শীর্ষ_2)			ফাঁকা
#define	LIST_EMPTY(শীর্ষ_2)		((শীর্ষ_2)->lh_first == LIST_END(শীর্ষ_2))
#define	LIST_পরবর্তী(elm, field)		((elm)->field.le_পরবর্তী)

#define	LIST_FOREACH(var, শীর্ষ_2, field)					\
	জন্য ((var) = ((শীর্ষ_2)->lh_first);				\
	    (var) != LIST_END(শীর্ষ_2);					\
	    (var) = ((var)->field.le_পরবর্তী))

#define	LIST_FOREACH_SAFE(var, শীর্ষ_2, field, tvar)			\
	জন্য ((var) = LIST_FIRST((শীর্ষ_2));				\
	    (var) != LIST_END(শীর্ষ_2) &&					\
	    ((tvar) = LIST_পরবর্তী((var), field), 1);			\
	    (var) = (tvar))

#define	LIST_MOVE(head1, head2) করো {					\
	LIST_INIT((head2));						\
	যদি (!LIST_EMPTY((head1))) {					\
		(head2)->lh_first = (head1)->lh_first;			\
		LIST_INIT((head1));					\
	}								\
} যখন (/*CONSTCOND*/0)

/*
 * List functions.
 */
#যদি defined(QUEUEDEBUG)
#define	QUEUEDEBUG_LIST_ঢোকাও_শীর্ষ(শীর্ষ_2, elm, field)			\
	যদি ((শীর্ষ_2)->lh_first &&						\
	    (শীর্ষ_2)->lh_first->field.le_পূর্ব != &(শীর্ষ_2)->lh_first)	\
		QUEUEDEBUG_ABORT("LIST_INSERT_HEAD %p %s:%d", (শীর্ষ_2),	\
		    __FILE__, __LINE__);
#define	QUEUEDEBUG_LIST_OP(elm, field)					\
	যদি ((elm)->field.le_পরবর্তী &&					\
	    (elm)->field.le_পরবর্তী->field.le_পূর্ব !=			\
	    &(elm)->field.le_পরবর্তী)					\
		QUEUEDEBUG_ABORT("LIST_* forw %p %s:%d", (elm),		\
		    __FILE__, __LINE__);				\
	যদি (*(elm)->field.le_পূর্ব != (elm))				\
		QUEUEDEBUG_ABORT("LIST_* back %p %s:%d", (elm),		\
		    __FILE__, __LINE__);
#define	QUEUEDEBUG_LIST_POSTREMOVE(elm, field)				\
	(elm)->field.le_পরবর্তী = (শূন্য *)1L;				\
	(elm)->field.le_পূর্ব = (শূন্য *)1L;
#অন্যথা
#define	QUEUEDEBUG_LIST_ঢোকাও_শীর্ষ(শীর্ষ_2, elm, field)
#define	QUEUEDEBUG_LIST_OP(elm, field)
#define	QUEUEDEBUG_LIST_POSTREMOVE(elm, field)
#endif

#define	LIST_INIT(শীর্ষ_2) করো {						\
	(শীর্ষ_2)->lh_first = LIST_END(শীর্ষ_2);				\
} যখন (/*CONSTCOND*/0)

#define	LIST_ঢোকাও_AFTER(listelm, elm, field) করো {			\
	QUEUEDEBUG_LIST_OP((listelm), field)				\
	যদি (((elm)->field.le_পরবর্তী = (listelm)->field.le_পরবর্তী) != 	\
	    LIST_END(শীর্ষ_2))						\
		(listelm)->field.le_পরবর্তী->field.le_পূর্ব =		\
		    &(elm)->field.le_পরবর্তী;				\
	(listelm)->field.le_পরবর্তী = (elm);				\
	(elm)->field.le_পূর্ব = &(listelm)->field.le_পরবর্তী;		\
} যখন (/*CONSTCOND*/0)

#define	LIST_ঢোকাও_BEFORE(listelm, elm, field) করো {			\
	QUEUEDEBUG_LIST_OP((listelm), field)				\
	(elm)->field.le_পূর্ব = (listelm)->field.le_পূর্ব;		\
	(elm)->field.le_পরবর্তী = (listelm);				\
	*(listelm)->field.le_পূর্ব = (elm);				\
	(listelm)->field.le_পূর্ব = &(elm)->field.le_পরবর্তী;		\
} যখন (/*CONSTCOND*/0)

#define	LIST_ঢোকাও_শীর্ষ(শীর্ষ_2, elm, field) করো {				\
	QUEUEDEBUG_LIST_ঢোকাও_শীর্ষ((শীর্ষ_2), (elm), field)		\
	যদি (((elm)->field.le_পরবর্তী = (শীর্ষ_2)->lh_first) != LIST_END(শীর্ষ_2))\
		(শীর্ষ_2)->lh_first->field.le_পূর্ব = &(elm)->field.le_পরবর্তী;\
	(শীর্ষ_2)->lh_first = (elm);					\
	(elm)->field.le_পূর্ব = &(শীর্ষ_2)->lh_first;			\
} যখন (/*CONSTCOND*/0)

#define	LIST_REMOVE(elm, field) করো {					\
	QUEUEDEBUG_LIST_OP((elm), field)				\
	যদি ((elm)->field.le_পরবর্তী != ফাঁকা)				\
		(elm)->field.le_পরবর্তী->field.le_পূর্ব = 			\
		    (elm)->field.le_পূর্ব;				\
	*(elm)->field.le_পূর্ব = (elm)->field.le_পরবর্তী;			\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), field)			\
} যখন (/*CONSTCOND*/0)

#define LIST_REPLACE(elm, elm2, field) করো {				\
	যদি (((elm2)->field.le_পরবর্তী = (elm)->field.le_পরবর্তী) != ফাঁকা)	\
		(elm2)->field.le_পরবর্তী->field.le_পূর্ব =			\
		    &(elm2)->field.le_পরবর্তী;				\
	(elm2)->field.le_পূর্ব = (elm)->field.le_পূর্ব;			\
	*(elm2)->field.le_পূর্ব = (elm2);				\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), field)			\
} যখন (/*CONSTCOND*/0)

/*
 * Simple queue definitions.
 */
#define	SIMPLEQ_শীর্ষ(name, type)					\
কাঠামো name {								\
	কাঠামো type *sqh_first;	/* first element */			\
	কাঠামো type **sqh_last;	/* addr of last next element */		\
}

#define	SIMPLEQ_শীর্ষ_INITIALIZER(শীর্ষ_2)					\
	{ ফাঁকা, &(শীর্ষ_2).sqh_first }

#define	SIMPLEQ_ENTRY(type)						\
কাঠামো {								\
	কাঠামো type *sqe_পরবর্তী;	/* next element */			\
}

/*
 * Simple queue access methods.
 */
#define	SIMPLEQ_FIRST(শীর্ষ_2)		((শীর্ষ_2)->sqh_first)
#define	SIMPLEQ_END(শীর্ষ_2)		ফাঁকা
#define	SIMPLEQ_EMPTY(শীর্ষ_2)		((শীর্ষ_2)->sqh_first == SIMPLEQ_END(শীর্ষ_2))
#define	SIMPLEQ_পরবর্তী(elm, field)	((elm)->field.sqe_পরবর্তী)

#define	SIMPLEQ_FOREACH(var, শীর্ষ_2, field)				\
	জন্য ((var) = ((শীর্ষ_2)->sqh_first);				\
	    (var) != SIMPLEQ_END(শীর্ষ_2);					\
	    (var) = ((var)->field.sqe_পরবর্তী))

#define	SIMPLEQ_FOREACH_SAFE(var, শীর্ষ_2, field, পরবর্তী)			\
	জন্য ((var) = ((শীর্ষ_2)->sqh_first);				\
	    (var) != SIMPLEQ_END(শীর্ষ_2) &&				\
	    ((পরবর্তী = ((var)->field.sqe_পরবর্তী)), 1);			\
	    (var) = (পরবর্তী))

/*
 * Simple queue functions.
 */
#define	SIMPLEQ_INIT(শীর্ষ_2) করো {						\
	(শীর্ষ_2)->sqh_first = ফাঁকা;					\
	(শীর্ষ_2)->sqh_last = &(শীর্ষ_2)->sqh_first;				\
} যখন (/*CONSTCOND*/0)

#define	SIMPLEQ_ঢোকাও_শীর্ষ(শীর্ষ_2, elm, field) করো {			\
	যদি (((elm)->field.sqe_পরবর্তী = (শীর্ষ_2)->sqh_first) == ফাঁকা)	\
		(শীর্ষ_2)->sqh_last = &(elm)->field.sqe_পরবর্তী;		\
	(শীর্ষ_2)->sqh_first = (elm);					\
} যখন (/*CONSTCOND*/0)

#define	SIMPLEQ_ঢোকাও_লেজ(শীর্ষ_2, elm, field) করো {			\
	(elm)->field.sqe_পরবর্তী = ফাঁকা;					\
	*(শীর্ষ_2)->sqh_last = (elm);					\
	(শীর্ষ_2)->sqh_last = &(elm)->field.sqe_পরবর্তী;			\
} যখন (/*CONSTCOND*/0)

#define	SIMPLEQ_ঢোকাও_AFTER(শীর্ষ_2, listelm, elm, field) করো {		\
	যদি (((elm)->field.sqe_পরবর্তী = (listelm)->field.sqe_পরবর্তী) == ফাঁকা)\
		(শীর্ষ_2)->sqh_last = &(elm)->field.sqe_পরবর্তী;		\
	(listelm)->field.sqe_পরবর্তী = (elm);				\
} যখন (/*CONSTCOND*/0)

#define	SIMPLEQ_REMOVE_শীর্ষ(শীর্ষ_2, field) করো {				\
	যদি (((শীর্ষ_2)->sqh_first = (শীর্ষ_2)->sqh_first->field.sqe_পরবর্তী) == ফাঁকা) \
		(শীর্ষ_2)->sqh_last = &(শীর্ষ_2)->sqh_first;			\
} যখন (/*CONSTCOND*/0)

#define SIMPLEQ_REMOVE_AFTER(শীর্ষ_2, elm, field) করো {			\
	যদি (((elm)->field.sqe_পরবর্তী = (elm)->field.sqe_পরবর্তী->field.sqe_পরবর্তী) \
	    == ফাঁকা)							\
		(শীর্ষ_2)->sqh_last = &(elm)->field.sqe_পরবর্তী;		\
} যখন (/*CONSTCOND*/0)

#define	SIMPLEQ_REMOVE(শীর্ষ_2, elm, type, field) করো {			\
	যদি ((শীর্ষ_2)->sqh_first == (elm)) {				\
		SIMPLEQ_REMOVE_শীর্ষ((শীর্ষ_2), field);			\
	} অন্যথা {							\
		কাঠামো type *curelm = (শীর্ষ_2)->sqh_first;		\
		যখন (curelm->field.sqe_পরবর্তী != (elm))			\
			curelm = curelm->field.sqe_পরবর্তী;		\
		যদি ((curelm->field.sqe_পরবর্তী =				\
			curelm->field.sqe_পরবর্তী->field.sqe_পরবর্তী) == ফাঁকা) \
			    (শীর্ষ_2)->sqh_last = &(curelm)->field.sqe_পরবর্তী; \
	}								\
} যখন (/*CONSTCOND*/0)

#define	SIMPLEQ_CONCAT(head1, head2) করো {				\
	যদি (!SIMPLEQ_EMPTY((head2))) {					\
		*(head1)->sqh_last = (head2)->sqh_first;		\
		(head1)->sqh_last = (head2)->sqh_last;		\
		SIMPLEQ_INIT((head2));					\
	}								\
} যখন (/*CONSTCOND*/0)

#define	SIMPLEQ_LAST(শীর্ষ_2, type, field)					\
	(SIMPLEQ_EMPTY((শীর্ষ_2)) ?						\
		ফাঁকা :							\
	        ((কাঠামো type *)(শূন্য *)				\
		((অক্ষর *)((শীর্ষ_2)->sqh_last) - offsetof(কাঠামো type, field))))

/*
 * Tail queue definitions.
 */
#define	TAILQ_শীর্ষ_2(name, type, qual)					\
কাঠামো name {								\
	qual type *tqh_first;		/* first element */		\
	qual type *qual *tqh_last;	/* addr of last next element */	\
}
#define TAILQ_শীর্ষ(name, type)	TAILQ_শীর্ষ_2(name, কাঠামো type,)

#define	TAILQ_শীর্ষ_INITIALIZER(শীর্ষ_2)					\
	{ TAILQ_END(শীর্ষ_2), &(শীর্ষ_2).tqh_first }

#define	_TAILQ_ENTRY(type, qual)					\
কাঠামো {								\
	qual type *tqe_পরবর্তী;		/* next element */		\
	qual type *qual *tqe_পূর্ব;	/* address of previous next element */\
}
#define TAILQ_ENTRY(type)	_TAILQ_ENTRY(কাঠামো type,)

/*
 * Tail queue access methods.
 */
#define	TAILQ_FIRST(শীর্ষ_2)		((শীর্ষ_2)->tqh_first)
#define	TAILQ_END(শীর্ষ_2)			(ফাঁকা)
#define	TAILQ_পরবর্তী(elm, field)		((elm)->field.tqe_পরবর্তী)
#define	TAILQ_LAST(শীর্ষ_2, headname) \
	(*(((কাঠামো headname *)((শীর্ষ_2)->tqh_last))->tqh_last))
#define	TAILQ_পূর্ব(elm, headname, field) \
	(*(((কাঠামো headname *)((elm)->field.tqe_পূর্ব))->tqh_last))
#define	TAILQ_EMPTY(শীর্ষ_2)		(TAILQ_FIRST(শীর্ষ_2) == TAILQ_END(শীর্ষ_2))


#define	TAILQ_FOREACH(var, শীর্ষ_2, field)					\
	জন্য ((var) = ((শীর্ষ_2)->tqh_first);				\
	    (var) != TAILQ_END(শীর্ষ_2);					\
	    (var) = ((var)->field.tqe_পরবর্তী))

#define	TAILQ_FOREACH_SAFE(var, শীর্ষ_2, field, পরবর্তী)			\
	জন্য ((var) = ((শীর্ষ_2)->tqh_first);				\
	    (var) != TAILQ_END(শীর্ষ_2) &&					\
	    ((পরবর্তী) = TAILQ_পরবর্তী(var, field), 1); (var) = (পরবর্তী))

#define	TAILQ_FOREACH_REVERSE(var, শীর্ষ_2, headname, field)		\
	জন্য ((var) = (*(((কাঠামো headname *)((শীর্ষ_2)->tqh_last))->tqh_last));\
	    (var) != TAILQ_END(শীর্ষ_2);					\
	    (var) = (*(((কাঠামো headname *)((var)->field.tqe_পূর্ব))->tqh_last)))

#define	TAILQ_FOREACH_REVERSE_SAFE(var, শীর্ষ_2, headname, field, পূর্ব)	\
	জন্য ((var) = TAILQ_LAST((শীর্ষ_2), headname);			\
	    (var) != TAILQ_END(শীর্ষ_2) && 				\
	    ((পূর্ব) = TAILQ_পূর্ব((var), headname, field), 1); (var) = (পূর্ব))

/*
 * Tail queue functions.
 */
#যদি defined(QUEUEDEBUG)
#define	QUEUEDEBUG_TAILQ_ঢোকাও_শীর্ষ(শীর্ষ_2, elm, field)			\
	যদি ((শীর্ষ_2)->tqh_first &&					\
	    (শীর্ষ_2)->tqh_first->field.tqe_পূর্ব != &(শীর্ষ_2)->tqh_first)	\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_HEAD %p %s:%d", (শীর্ষ_2),	\
		    __FILE__, __LINE__);
#define	QUEUEDEBUG_TAILQ_ঢোকাও_লেজ(শীর্ষ_2, elm, field)			\
	যদি (*(শীর্ষ_2)->tqh_last != ফাঁকা)					\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_TAIL %p %s:%d", (শীর্ষ_2),	\
		    __FILE__, __LINE__);
#define	QUEUEDEBUG_TAILQ_OP(elm, field)					\
	যদি ((elm)->field.tqe_পরবর্তী &&					\
	    (elm)->field.tqe_পরবর্তী->field.tqe_পূর্ব !=			\
	    &(elm)->field.tqe_পরবর্তী)					\
		QUEUEDEBUG_ABORT("TAILQ_* forw %p %s:%d", (elm),	\
		    __FILE__, __LINE__);				\
	যদি (*(elm)->field.tqe_পূর্ব != (elm))				\
		QUEUEDEBUG_ABORT("TAILQ_* back %p %s:%d", (elm),	\
		    __FILE__, __LINE__);
#define	QUEUEDEBUG_TAILQ_PREREMOVE(শীর্ষ_2, elm, field)			\
	যদি ((elm)->field.tqe_পরবর্তী == ফাঁকা &&				\
	    (শীর্ষ_2)->tqh_last != &(elm)->field.tqe_পরবর্তী)			\
		QUEUEDEBUG_ABORT("TAILQ_PREREMOVE head %p elm %p %s:%d",\
		    (শীর্ষ_2), (elm), __FILE__, __LINE__);
#define	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, field)				\
	(elm)->field.tqe_পরবর্তী = (শূন্য *)1L;				\
	(elm)->field.tqe_পূর্ব = (শূন্য *)1L;
#অন্যথা
#define	QUEUEDEBUG_TAILQ_ঢোকাও_শীর্ষ(শীর্ষ_2, elm, field)
#define	QUEUEDEBUG_TAILQ_ঢোকাও_লেজ(শীর্ষ_2, elm, field)
#define	QUEUEDEBUG_TAILQ_OP(elm, field)
#define	QUEUEDEBUG_TAILQ_PREREMOVE(শীর্ষ_2, elm, field)
#define	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, field)
#endif

#define	TAILQ_INIT(শীর্ষ_2) করো {						\
	(শীর্ষ_2)->tqh_first = TAILQ_END(শীর্ষ_2);				\
	(শীর্ষ_2)->tqh_last = &(শীর্ষ_2)->tqh_first;				\
} যখন (/*CONSTCOND*/0)

#define	TAILQ_ঢোকাও_শীর্ষ(শীর্ষ_2, elm, field) করো {			\
	QUEUEDEBUG_TAILQ_ঢোকাও_শীর্ষ((শীর্ষ_2), (elm), field)		\
	যদি (((elm)->field.tqe_পরবর্তী = (শীর্ষ_2)->tqh_first) != TAILQ_END(শীর্ষ_2))\
		(শীর্ষ_2)->tqh_first->field.tqe_পূর্ব =			\
		    &(elm)->field.tqe_পরবর্তী;				\
	অন্যথা								\
		(শীর্ষ_2)->tqh_last = &(elm)->field.tqe_পরবর্তী;		\
	(শীর্ষ_2)->tqh_first = (elm);					\
	(elm)->field.tqe_পূর্ব = &(শীর্ষ_2)->tqh_first;			\
} যখন (/*CONSTCOND*/0)

#define	TAILQ_ঢোকাও_লেজ(শীর্ষ_2, elm, field) করো {			\
	QUEUEDEBUG_TAILQ_ঢোকাও_লেজ((শীর্ষ_2), (elm), field)		\
	(elm)->field.tqe_পরবর্তী = TAILQ_END(শীর্ষ_2);			\
	(elm)->field.tqe_পূর্ব = (শীর্ষ_2)->tqh_last;			\
	*(শীর্ষ_2)->tqh_last = (elm);					\
	(শীর্ষ_2)->tqh_last = &(elm)->field.tqe_পরবর্তী;			\
} যখন (/*CONSTCOND*/0)

#define	TAILQ_ঢোকাও_AFTER(শীর্ষ_2, listelm, elm, field) করো {		\
	QUEUEDEBUG_TAILQ_OP((listelm), field)				\
	যদি (((elm)->field.tqe_পরবর্তী = (listelm)->field.tqe_পরবর্তী) != 	\
	    TAILQ_END(শীর্ষ_2))						\
		(elm)->field.tqe_পরবর্তী->field.tqe_পূর্ব = 		\
		    &(elm)->field.tqe_পরবর্তী;				\
	অন্যথা								\
		(শীর্ষ_2)->tqh_last = &(elm)->field.tqe_পরবর্তী;		\
	(listelm)->field.tqe_পরবর্তী = (elm);				\
	(elm)->field.tqe_পূর্ব = &(listelm)->field.tqe_পরবর্তী;		\
} যখন (/*CONSTCOND*/0)

#define	TAILQ_ঢোকাও_BEFORE(listelm, elm, field) করো {			\
	QUEUEDEBUG_TAILQ_OP((listelm), field)				\
	(elm)->field.tqe_পূর্ব = (listelm)->field.tqe_পূর্ব;		\
	(elm)->field.tqe_পরবর্তী = (listelm);				\
	*(listelm)->field.tqe_পূর্ব = (elm);				\
	(listelm)->field.tqe_পূর্ব = &(elm)->field.tqe_পরবর্তী;		\
} যখন (/*CONSTCOND*/0)

#define	TAILQ_REMOVE(শীর্ষ_2, elm, field) করো {				\
	QUEUEDEBUG_TAILQ_PREREMOVE((শীর্ষ_2), (elm), field)		\
	QUEUEDEBUG_TAILQ_OP((elm), field)				\
	যদি (((elm)->field.tqe_পরবর্তী) != TAILQ_END(শীর্ষ_2))			\
		(elm)->field.tqe_পরবর্তী->field.tqe_পূর্ব = 		\
		    (elm)->field.tqe_পূর্ব;				\
	অন্যথা								\
		(শীর্ষ_2)->tqh_last = (elm)->field.tqe_পূর্ব;		\
	*(elm)->field.tqe_পূর্ব = (elm)->field.tqe_পরবর্তী;			\
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), field);			\
} যখন (/*CONSTCOND*/0)

#define TAILQ_REPLACE(শীর্ষ_2, elm, elm2, field) করো {                      \
        যদি (((elm2)->field.tqe_পরবর্তী = (elm)->field.tqe_পরবর্তী) != 	\
	    TAILQ_END(শীর্ষ_2))   						\
                (elm2)->field.tqe_পরবর্তী->field.tqe_পূর্ব =                \
                    &(elm2)->field.tqe_পরবর্তী;                            \
        অন্যথা                                                            \
                (শীর্ষ_2)->tqh_last = &(elm2)->field.tqe_পরবর্তী;             \
        (elm2)->field.tqe_পূর্ব = (elm)->field.tqe_পূর্ব;                 \
        *(elm2)->field.tqe_পূর্ব = (elm2);                               \
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), field);			\
} যখন (/*CONSTCOND*/0)

#define	TAILQ_CONCAT(head1, head2, field) করো {				\
	যদি (!TAILQ_EMPTY(head2)) {					\
		*(head1)->tqh_last = (head2)->tqh_first;		\
		(head2)->tqh_first->field.tqe_পূর্ব = (head1)->tqh_last;	\
		(head1)->tqh_last = (head2)->tqh_last;			\
		TAILQ_INIT((head2));					\
	}								\
} যখন (/*CONSTCOND*/0)

/*
 * Singly-linked Tail queue declarations.
 */
#define	STAILQ_শীর্ষ(name, type)						\
কাঠামো name {								\
	কাঠামো type *stqh_first;	/* first element */		\
	কাঠামো type **stqh_last;	/* addr of last next element */	\
}

#define	STAILQ_শীর্ষ_INITIALIZER(শীর্ষ_2)					\
	{ ফাঁকা, &(শীর্ষ_2).stqh_first }

#define	STAILQ_ENTRY(type)						\
কাঠামো {								\
	কাঠামো type *stqe_পরবর্তী;	/* next element */			\
}

/*
 * Singly-linked Tail queue access methods.
 */
#define	STAILQ_FIRST(শীর্ষ_2)	((শীর্ষ_2)->stqh_first)
#define	STAILQ_END(শীর্ষ_2)	ফাঁকা
#define	STAILQ_পরবর্তী(elm, field)	((elm)->field.stqe_পরবর্তী)
#define	STAILQ_EMPTY(শীর্ষ_2)	(STAILQ_FIRST(শীর্ষ_2) == STAILQ_END(শীর্ষ_2))

/*
 * Singly-linked Tail queue functions.
 */
#define	STAILQ_INIT(শীর্ষ_2) করো {						\
	(শীর্ষ_2)->stqh_first = ফাঁকা;					\
	(শীর্ষ_2)->stqh_last = &(শীর্ষ_2)->stqh_first;				\
} যখন (/*CONSTCOND*/0)

#define	STAILQ_ঢোকাও_শীর্ষ(শীর্ষ_2, elm, field) করো {			\
	যদি (((elm)->field.stqe_পরবর্তী = (শীর্ষ_2)->stqh_first) == ফাঁকা)	\
		(শীর্ষ_2)->stqh_last = &(elm)->field.stqe_পরবর্তী;		\
	(শীর্ষ_2)->stqh_first = (elm);					\
} যখন (/*CONSTCOND*/0)

#define	STAILQ_ঢোকাও_লেজ(শীর্ষ_2, elm, field) করো {			\
	(elm)->field.stqe_পরবর্তী = ফাঁকা;					\
	*(শীর্ষ_2)->stqh_last = (elm);					\
	(শীর্ষ_2)->stqh_last = &(elm)->field.stqe_পরবর্তী;			\
} যখন (/*CONSTCOND*/0)

#define	STAILQ_ঢোকাও_AFTER(শীর্ষ_2, listelm, elm, field) করো {		\
	যদি (((elm)->field.stqe_পরবর্তী = (listelm)->field.stqe_পরবর্তী) == ফাঁকা)\
		(শীর্ষ_2)->stqh_last = &(elm)->field.stqe_পরবর্তী;		\
	(listelm)->field.stqe_পরবর্তী = (elm);				\
} যখন (/*CONSTCOND*/0)

#define	STAILQ_REMOVE_শীর্ষ(শীর্ষ_2, field) করো {				\
	যদি (((শীর্ষ_2)->stqh_first = (শীর্ষ_2)->stqh_first->field.stqe_পরবর্তী) == ফাঁকা) \
		(শীর্ষ_2)->stqh_last = &(শীর্ষ_2)->stqh_first;			\
} যখন (/*CONSTCOND*/0)

#define	STAILQ_REMOVE(শীর্ষ_2, elm, type, field) করো {			\
	যদি ((শীর্ষ_2)->stqh_first == (elm)) {				\
		STAILQ_REMOVE_শীর্ষ((শীর্ষ_2), field);			\
	} অন্যথা {							\
		কাঠামো type *curelm = (শীর্ষ_2)->stqh_first;		\
		যখন (curelm->field.stqe_পরবর্তী != (elm))			\
			curelm = curelm->field.stqe_পরবর্তী;		\
		যদি ((curelm->field.stqe_পরবর্তী =				\
			curelm->field.stqe_পরবর্তী->field.stqe_পরবর্তী) == ফাঁকা) \
			    (শীর্ষ_2)->stqh_last = &(curelm)->field.stqe_পরবর্তী; \
	}								\
} যখন (/*CONSTCOND*/0)

#define	STAILQ_FOREACH(var, শীর্ষ_2, field)				\
	জন্য ((var) = ((শীর্ষ_2)->stqh_first);				\
		(var);							\
		(var) = ((var)->field.stqe_পরবর্তী))

#define	STAILQ_FOREACH_SAFE(var, শীর্ষ_2, field, tvar)			\
	জন্য ((var) = STAILQ_FIRST((শীর্ষ_2));				\
	    (var) && ((tvar) = STAILQ_পরবর্তী((var), field), 1);		\
	    (var) = (tvar))

#define	STAILQ_CONCAT(head1, head2) করো {				\
	যদি (!STAILQ_EMPTY((head2))) {					\
		*(head1)->stqh_last = (head2)->stqh_first;		\
		(head1)->stqh_last = (head2)->stqh_last;		\
		STAILQ_INIT((head2));					\
	}								\
} যখন (/*CONSTCOND*/0)

#define	STAILQ_LAST(শীর্ষ_2, type, field)					\
	(STAILQ_EMPTY((শীর্ষ_2)) ?						\
		ফাঁকা :							\
	        ((কাঠামো type *)(শূন্য *)				\
		((অক্ষর *)((শীর্ষ_2)->stqh_last) - offsetof(কাঠামো type, field))))


#ifndef _KERNEL
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
স্থির __inline ধ্রুবক শূন্য * __launder_type(ধ্রুবক শূন্য *);
স্থির __inline ধ্রুবক শূন্য *
__launder_type(ধ্রুবক শূন্য *__x)
{
	__asm __volatile("" : "+r" (__x));
	ফেরত __x;
}

#যদি defined(QUEUEDEBUG)
#define QUEUEDEBUG_CIRCLEQ_শীর্ষ(শীর্ষ_2, field)				\
	যদি ((শীর্ষ_2)->cqh_first != CIRCLEQ_ENDC(শীর্ষ_2) &&			\
	    (শীর্ষ_2)->cqh_first->field.cqe_পূর্ব != CIRCLEQ_ENDC(শীর্ষ_2))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head forw %p %s:%d", (শীর্ষ_2),	\
		      __FILE__, __LINE__);				\
	যদি ((শীর্ষ_2)->cqh_last != CIRCLEQ_ENDC(শীর্ষ_2) &&			\
	    (শীর্ষ_2)->cqh_last->field.cqe_পরবর্তী != CIRCLEQ_ENDC(শীর্ষ_2))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head back %p %s:%d", (শীর্ষ_2),	\
		      __FILE__, __LINE__);
#define QUEUEDEBUG_CIRCLEQ_ELM(শীর্ষ_2, elm, field)			\
	যদি ((elm)->field.cqe_পরবর্তী == CIRCLEQ_ENDC(শীর্ষ_2)) {		\
		যদি ((শীর্ষ_2)->cqh_last != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm last %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} অন্যথা {							\
		যদি ((elm)->field.cqe_পরবর্তী->field.cqe_পূর্ব != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm forw %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}								\
	যদি ((elm)->field.cqe_পূর্ব == CIRCLEQ_ENDC(শীর্ষ_2)) {		\
		যদি ((শীর্ষ_2)->cqh_first != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm first %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} অন্যথা {							\
		যদি ((elm)->field.cqe_পূর্ব->field.cqe_পরবর্তী != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm prev %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}
#define QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, field)			\
	(elm)->field.cqe_পরবর্তী = (শূন্য *)1L;				\
	(elm)->field.cqe_পূর্ব = (শূন্য *)1L;
#অন্যথা
#define QUEUEDEBUG_CIRCLEQ_শীর্ষ(শীর্ষ_2, field)
#define QUEUEDEBUG_CIRCLEQ_ELM(শীর্ষ_2, elm, field)
#define QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, field)
#endif

#define	CIRCLEQ_শীর্ষ(name, type)					\
কাঠামো name {								\
	কাঠামো type *cqh_first;		/* first element */		\
	কাঠামো type *cqh_last;		/* last element */		\
}

#define	CIRCLEQ_শীর্ষ_INITIALIZER(শীর্ষ_2)					\
	{ CIRCLEQ_END(&শীর্ষ_2), CIRCLEQ_END(&শীর্ষ_2) }

#define	CIRCLEQ_ENTRY(type)						\
কাঠামো {								\
	কাঠামো type *cqe_পরবর্তী;		/* next element */		\
	কাঠামো type *cqe_পূর্ব;		/* previous element */		\
}

/*
 * Circular queue functions.
 */
#define	CIRCLEQ_INIT(শীর্ষ_2) করো {						\
	(শীর্ষ_2)->cqh_first = CIRCLEQ_END(শীর্ষ_2);				\
	(শীর্ষ_2)->cqh_last = CIRCLEQ_END(শীর্ষ_2);				\
} যখন (/*CONSTCOND*/0)

#define	CIRCLEQ_ঢোকাও_AFTER(শীর্ষ_2, listelm, elm, field) করো {		\
	QUEUEDEBUG_CIRCLEQ_শীর্ষ((শীর্ষ_2), field)				\
	QUEUEDEBUG_CIRCLEQ_ELM((শীর্ষ_2), (listelm), field)		\
	(elm)->field.cqe_পরবর্তী = (listelm)->field.cqe_পরবর্তী;		\
	(elm)->field.cqe_পূর্ব = (listelm);				\
	যদি ((listelm)->field.cqe_পরবর্তী == CIRCLEQ_ENDC(শীর্ষ_2))		\
		(শীর্ষ_2)->cqh_last = (elm);				\
	অন্যথা								\
		(listelm)->field.cqe_পরবর্তী->field.cqe_পূর্ব = (elm);	\
	(listelm)->field.cqe_পরবর্তী = (elm);				\
} যখন (/*CONSTCOND*/0)

#define	CIRCLEQ_ঢোকাও_BEFORE(শীর্ষ_2, listelm, elm, field) করো {		\
	QUEUEDEBUG_CIRCLEQ_শীর্ষ((শীর্ষ_2), field)				\
	QUEUEDEBUG_CIRCLEQ_ELM((শীর্ষ_2), (listelm), field)		\
	(elm)->field.cqe_পরবর্তী = (listelm);				\
	(elm)->field.cqe_পূর্ব = (listelm)->field.cqe_পূর্ব;		\
	যদি ((listelm)->field.cqe_পূর্ব == CIRCLEQ_ENDC(শীর্ষ_2))		\
		(শীর্ষ_2)->cqh_first = (elm);				\
	অন্যথা								\
		(listelm)->field.cqe_পূর্ব->field.cqe_পরবর্তী = (elm);	\
	(listelm)->field.cqe_পূর্ব = (elm);				\
} যখন (/*CONSTCOND*/0)

#define	CIRCLEQ_ঢোকাও_শীর্ষ(শীর্ষ_2, elm, field) করো {			\
	QUEUEDEBUG_CIRCLEQ_শীর্ষ((শীর্ষ_2), field)				\
	(elm)->field.cqe_পরবর্তী = (শীর্ষ_2)->cqh_first;			\
	(elm)->field.cqe_পূর্ব = CIRCLEQ_END(শীর্ষ_2);			\
	যদি ((শীর্ষ_2)->cqh_last == CIRCLEQ_ENDC(শীর্ষ_2))			\
		(শীর্ষ_2)->cqh_last = (elm);				\
	অন্যথা								\
		(শীর্ষ_2)->cqh_first->field.cqe_পূর্ব = (elm);		\
	(শীর্ষ_2)->cqh_first = (elm);					\
} যখন (/*CONSTCOND*/0)

#define	CIRCLEQ_ঢোকাও_লেজ(শীর্ষ_2, elm, field) করো {			\
	QUEUEDEBUG_CIRCLEQ_শীর্ষ((শীর্ষ_2), field)				\
	(elm)->field.cqe_পরবর্তী = CIRCLEQ_END(শীর্ষ_2);			\
	(elm)->field.cqe_পূর্ব = (শীর্ষ_2)->cqh_last;			\
	যদি ((শীর্ষ_2)->cqh_first == CIRCLEQ_ENDC(শীর্ষ_2))			\
		(শীর্ষ_2)->cqh_first = (elm);				\
	অন্যথা								\
		(শীর্ষ_2)->cqh_last->field.cqe_পরবর্তী = (elm);		\
	(শীর্ষ_2)->cqh_last = (elm);					\
} যখন (/*CONSTCOND*/0)

#define	CIRCLEQ_REMOVE(শীর্ষ_2, elm, field) করো {				\
	QUEUEDEBUG_CIRCLEQ_শীর্ষ((শীর্ষ_2), field)				\
	QUEUEDEBUG_CIRCLEQ_ELM((শীর্ষ_2), (elm), field)			\
	যদি ((elm)->field.cqe_পরবর্তী == CIRCLEQ_ENDC(শীর্ষ_2))		\
		(শীর্ষ_2)->cqh_last = (elm)->field.cqe_পূর্ব;		\
	অন্যথা								\
		(elm)->field.cqe_পরবর্তী->field.cqe_পূর্ব =			\
		    (elm)->field.cqe_পূর্ব;				\
	যদি ((elm)->field.cqe_পূর্ব == CIRCLEQ_ENDC(শীর্ষ_2))		\
		(শীর্ষ_2)->cqh_first = (elm)->field.cqe_পরবর্তী;		\
	অন্যথা								\
		(elm)->field.cqe_পূর্ব->field.cqe_পরবর্তী =			\
		    (elm)->field.cqe_পরবর্তী;				\
	QUEUEDEBUG_CIRCLEQ_POSTREMOVE((elm), field)			\
} যখন (/*CONSTCOND*/0)

#define	CIRCLEQ_FOREACH(var, শীর্ষ_2, field)				\
	জন্য ((var) = ((শীর্ষ_2)->cqh_first);				\
		(var) != CIRCLEQ_ENDC(শীর্ষ_2);				\
		(var) = ((var)->field.cqe_পরবর্তী))

#define	CIRCLEQ_FOREACH_REVERSE(var, শীর্ষ_2, field)			\
	জন্য ((var) = ((শীর্ষ_2)->cqh_last);				\
		(var) != CIRCLEQ_ENDC(শীর্ষ_2);				\
		(var) = ((var)->field.cqe_পূর্ব))

/*
 * Circular queue access methods.
 */
#define	CIRCLEQ_FIRST(শীর্ষ_2)		((শীর্ষ_2)->cqh_first)
#define	CIRCLEQ_LAST(শীর্ষ_2)		((শীর্ষ_2)->cqh_last)
/* For comparisons */
#define	CIRCLEQ_ENDC(শীর্ষ_2)		(__launder_type(শীর্ষ_2))
/* For assignments */
#define	CIRCLEQ_END(শীর্ষ_2)		((শূন্য *)(শীর্ষ_2))
#define	CIRCLEQ_পরবর্তী(elm, field)	((elm)->field.cqe_পরবর্তী)
#define	CIRCLEQ_পূর্ব(elm, field)	((elm)->field.cqe_পূর্ব)
#define	CIRCLEQ_EMPTY(শীর্ষ_2)						\
    (CIRCLEQ_FIRST(শীর্ষ_2) == CIRCLEQ_ENDC(শীর্ষ_2))

#define CIRCLEQ_LOOP_পরবর্তী(শীর্ষ_2, elm, field)				\
	(((elm)->field.cqe_পরবর্তী == CIRCLEQ_ENDC(শীর্ষ_2))			\
	    ? ((শীর্ষ_2)->cqh_first)					\
	    : (elm->field.cqe_পরবর্তী))
#define CIRCLEQ_LOOP_পূর্ব(শীর্ষ_2, elm, field)				\
	(((elm)->field.cqe_পূর্ব == CIRCLEQ_ENDC(শীর্ষ_2))			\
	    ? ((শীর্ষ_2)->cqh_last)					\
	    : (elm->field.cqe_পূর্ব))
#endif /* !_KERNEL */

#endif	/* !_SYS_QUEUE_H_ */
