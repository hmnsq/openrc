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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ	SYS_ਕਤਾਰ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SYS_ਕਤਾਰ_H

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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __NetBSD__
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/null.h>
#ਜੇ_ਅੰਤ

#ਜੇ defined(QUEUEDEBUG)
# ਜੇ defined(_KERNEL)
#  define QUEUEDEBUG_ABORT(...) panic(__VA_ARGS__)
# ਨਹੀਂਤਾਂ
#  include <err.h>
#  define QUEUEDEBUG_ABORT(...) err(1, __VA_ARGS__)
# endif
#ਜੇ_ਅੰਤ

/*
 * Singly-linked List definitions.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_ਸਿਰ(name, type)						\
ਬਣਤਰ name {								\
	ਬਣਤਰ type *slh_ਪਹਿਲਾ;	/* first element */			\
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_ਸਿਰ_INITIALIZER(ਸਿਰ)					\
	{ ਨਿਰਾ }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_ਇੰਦਰਾਜ(type)						\
ਬਣਤਰ {								\
	ਬਣਤਰ type *sle_ਅਗਲਾ;	/* next element */			\
}

/*
 * Singly-linked List access methods.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_ਪਹਿਲਾ(ਸਿਰ)	((ਸਿਰ)->slh_ਪਹਿਲਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_ਸਿਰਾ(ਸਿਰ)		ਨਿਰਾ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_ਖਾਲੀ_ਥਾਵਾਂ(ਸਿਰ)	((ਸਿਰ)->slh_ਪਹਿਲਾ == ਨਿਰਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_ਅਗਲਾ(elm, ਖੇਤਰ)	((elm)->ਖੇਤਰ.sle_ਅਗਲਾ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_FOREACH(var, ਸਿਰ, ਖੇਤਰ)					\
	ਲਈ((var) = (ਸਿਰ)->slh_ਪਹਿਲਾ;					\
	    (var) != SLIST_ਸਿਰਾ(ਸਿਰ);					\
	    (var) = (var)->ਖੇਤਰ.sle_ਅਗਲਾ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_FOREACH_ਸੁਰੱਖਿਅਤ(var, ਸਿਰ, ਖੇਤਰ, tvar)			\
	ਲਈ ((var) = SLIST_ਪਹਿਲਾ((ਸਿਰ));				\
	    (var) != SLIST_ਸਿਰਾ(ਸਿਰ) &&					\
	    ((tvar) = SLIST_ਅਗਲਾ((var), ਖੇਤਰ), 1);			\
	    (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_INIT(ਸਿਰ) ਕਰੋ {						\
	(ਸਿਰ)->slh_ਪਹਿਲਾ = SLIST_ਸਿਰਾ(ਸਿਰ);				\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_ਪਾਓ_AFTER(slistelm, elm, ਖੇਤਰ) ਕਰੋ {			\
	(elm)->ਖੇਤਰ.sle_ਅਗਲਾ = (slistelm)->ਖੇਤਰ.sle_ਅਗਲਾ;		\
	(slistelm)->ਖੇਤਰ.sle_ਅਗਲਾ = (elm);				\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_ਪਾਓ_ਸਿਰ(ਸਿਰ, elm, ਖੇਤਰ) ਕਰੋ {			\
	(elm)->ਖੇਤਰ.sle_ਅਗਲਾ = (ਸਿਰ)->slh_ਪਹਿਲਾ;			\
	(ਸਿਰ)->slh_ਪਹਿਲਾ = (elm);					\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_REMOVE_AFTER(slistelm, ਖੇਤਰ) ਕਰੋ {			\
	(slistelm)->ਖੇਤਰ.sle_ਅਗਲਾ =					\
	    SLIST_ਅਗਲਾ(SLIST_ਅਗਲਾ((slistelm), ਖੇਤਰ), ਖੇਤਰ);		\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_REMOVE_ਸਿਰ(ਸਿਰ, ਖੇਤਰ) ਕਰੋ {				\
	(ਸਿਰ)->slh_ਪਹਿਲਾ = (ਸਿਰ)->slh_ਪਹਿਲਾ->ਖੇਤਰ.sle_ਅਗਲਾ;		\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SLIST_REMOVE(ਸਿਰ, elm, type, ਖੇਤਰ) ਕਰੋ {			\
	ਜੇ ((ਸਿਰ)->slh_ਪਹਿਲਾ == (elm)) {				\
		SLIST_REMOVE_ਸਿਰ((ਸਿਰ), ਖੇਤਰ);			\
	}								\
	ਨਹੀਂਤਾਂ {								\
		ਬਣਤਰ type *curelm = (ਸਿਰ)->slh_ਪਹਿਲਾ;		\
		ਜਦੋਂ(curelm->ਖੇਤਰ.sle_ਅਗਲਾ != (elm))			\
			curelm = curelm->ਖੇਤਰ.sle_ਅਗਲਾ;		\
		curelm->ਖੇਤਰ.sle_ਅਗਲਾ =				\
		    curelm->ਖੇਤਰ.sle_ਅਗਲਾ->ਖੇਤਰ.sle_ਅਗਲਾ;		\
	}								\
} ਜਦੋਂ (/*CONSTCOND*/0)


/*
 * List definitions.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_ਸਿਰ(name, type)						\
ਬਣਤਰ name {								\
	ਬਣਤਰ type *lh_ਪਹਿਲਾ;	/* first element */			\
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_ਸਿਰ_INITIALIZER(ਸਿਰ)					\
	{ ਨਿਰਾ }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_ਇੰਦਰਾਜ(type)						\
ਬਣਤਰ {								\
	ਬਣਤਰ type *le_ਅਗਲਾ;	/* next element */			\
	ਬਣਤਰ type **le_prev;	/* address of previous next element */	\
}

/*
 * List access methods.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_ਪਹਿਲਾ(ਸਿਰ)		((ਸਿਰ)->lh_ਪਹਿਲਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_ਸਿਰਾ(ਸਿਰ)			ਨਿਰਾ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_ਖਾਲੀ_ਥਾਵਾਂ(ਸਿਰ)		((ਸਿਰ)->lh_ਪਹਿਲਾ == LIST_ਸਿਰਾ(ਸਿਰ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_ਅਗਲਾ(elm, ਖੇਤਰ)		((elm)->ਖੇਤਰ.le_ਅਗਲਾ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_FOREACH(var, ਸਿਰ, ਖੇਤਰ)					\
	ਲਈ ((var) = ((ਸਿਰ)->lh_ਪਹਿਲਾ);				\
	    (var) != LIST_ਸਿਰਾ(ਸਿਰ);					\
	    (var) = ((var)->ਖੇਤਰ.le_ਅਗਲਾ))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_FOREACH_ਸੁਰੱਖਿਅਤ(var, ਸਿਰ, ਖੇਤਰ, tvar)			\
	ਲਈ ((var) = LIST_ਪਹਿਲਾ((ਸਿਰ));				\
	    (var) != LIST_ਸਿਰਾ(ਸਿਰ) &&					\
	    ((tvar) = LIST_ਅਗਲਾ((var), ਖੇਤਰ), 1);			\
	    (var) = (tvar))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_MOVE(head1, head2) ਕਰੋ {					\
	LIST_INIT((head2));						\
	ਜੇ (!LIST_ਖਾਲੀ_ਥਾਵਾਂ((head1))) {					\
		(head2)->lh_ਪਹਿਲਾ = (head1)->lh_ਪਹਿਲਾ;			\
		LIST_INIT((head1));					\
	}								\
} ਜਦੋਂ (/*CONSTCOND*/0)

/*
 * List functions.
 */
#ਜੇ defined(QUEUEDEBUG)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_LIST_ਪਾਓ_ਸਿਰ(ਸਿਰ, elm, ਖੇਤਰ)			\
	ਜੇ ((ਸਿਰ)->lh_ਪਹਿਲਾ &&						\
	    (ਸਿਰ)->lh_ਪਹਿਲਾ->ਖੇਤਰ.le_prev != &(ਸਿਰ)->lh_ਪਹਿਲਾ)	\
		QUEUEDEBUG_ABORT("LIST_INSERT_HEAD %p %s:%d", (ਸਿਰ),	\
		    __FILE__, __LINE__);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_LIST_OP(elm, ਖੇਤਰ)					\
	ਜੇ ((elm)->ਖੇਤਰ.le_ਅਗਲਾ &&					\
	    (elm)->ਖੇਤਰ.le_ਅਗਲਾ->ਖੇਤਰ.le_prev !=			\
	    &(elm)->ਖੇਤਰ.le_ਅਗਲਾ)					\
		QUEUEDEBUG_ABORT("LIST_* forw %p %s:%d", (elm),		\
		    __FILE__, __LINE__);				\
	ਜੇ (*(elm)->ਖੇਤਰ.le_prev != (elm))				\
		QUEUEDEBUG_ABORT("LIST_* back %p %s:%d", (elm),		\
		    __FILE__, __LINE__);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_LIST_POSTREMOVE(elm, ਖੇਤਰ)				\
	(elm)->ਖੇਤਰ.le_ਅਗਲਾ = (ਖਾਲੀ *)1L;				\
	(elm)->ਖੇਤਰ.le_prev = (ਖਾਲੀ *)1L;
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_LIST_ਪਾਓ_ਸਿਰ(ਸਿਰ, elm, ਖੇਤਰ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_LIST_OP(elm, ਖੇਤਰ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_LIST_POSTREMOVE(elm, ਖੇਤਰ)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_INIT(ਸਿਰ) ਕਰੋ {						\
	(ਸਿਰ)->lh_ਪਹਿਲਾ = LIST_ਸਿਰਾ(ਸਿਰ);				\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_ਪਾਓ_AFTER(listelm, elm, ਖੇਤਰ) ਕਰੋ {			\
	QUEUEDEBUG_LIST_OP((listelm), ਖੇਤਰ)				\
	ਜੇ (((elm)->ਖੇਤਰ.le_ਅਗਲਾ = (listelm)->ਖੇਤਰ.le_ਅਗਲਾ) != 	\
	    LIST_ਸਿਰਾ(ਸਿਰ))						\
		(listelm)->ਖੇਤਰ.le_ਅਗਲਾ->ਖੇਤਰ.le_prev =		\
		    &(elm)->ਖੇਤਰ.le_ਅਗਲਾ;				\
	(listelm)->ਖੇਤਰ.le_ਅਗਲਾ = (elm);				\
	(elm)->ਖੇਤਰ.le_prev = &(listelm)->ਖੇਤਰ.le_ਅਗਲਾ;		\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_ਪਾਓ_BEFORE(listelm, elm, ਖੇਤਰ) ਕਰੋ {			\
	QUEUEDEBUG_LIST_OP((listelm), ਖੇਤਰ)				\
	(elm)->ਖੇਤਰ.le_prev = (listelm)->ਖੇਤਰ.le_prev;		\
	(elm)->ਖੇਤਰ.le_ਅਗਲਾ = (listelm);				\
	*(listelm)->ਖੇਤਰ.le_prev = (elm);				\
	(listelm)->ਖੇਤਰ.le_prev = &(elm)->ਖੇਤਰ.le_ਅਗਲਾ;		\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_ਪਾਓ_ਸਿਰ(ਸਿਰ, elm, ਖੇਤਰ) ਕਰੋ {				\
	QUEUEDEBUG_LIST_ਪਾਓ_ਸਿਰ((ਸਿਰ), (elm), ਖੇਤਰ)		\
	ਜੇ (((elm)->ਖੇਤਰ.le_ਅਗਲਾ = (ਸਿਰ)->lh_ਪਹਿਲਾ) != LIST_ਸਿਰਾ(ਸਿਰ))\
		(ਸਿਰ)->lh_ਪਹਿਲਾ->ਖੇਤਰ.le_prev = &(elm)->ਖੇਤਰ.le_ਅਗਲਾ;\
	(ਸਿਰ)->lh_ਪਹਿਲਾ = (elm);					\
	(elm)->ਖੇਤਰ.le_prev = &(ਸਿਰ)->lh_ਪਹਿਲਾ;			\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LIST_REMOVE(elm, ਖੇਤਰ) ਕਰੋ {					\
	QUEUEDEBUG_LIST_OP((elm), ਖੇਤਰ)				\
	ਜੇ ((elm)->ਖੇਤਰ.le_ਅਗਲਾ != ਨਿਰਾ)				\
		(elm)->ਖੇਤਰ.le_ਅਗਲਾ->ਖੇਤਰ.le_prev = 			\
		    (elm)->ਖੇਤਰ.le_prev;				\
	*(elm)->ਖੇਤਰ.le_prev = (elm)->ਖੇਤਰ.le_ਅਗਲਾ;			\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), ਖੇਤਰ)			\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIST_REPLACE(elm, elm2, ਖੇਤਰ) ਕਰੋ {				\
	ਜੇ (((elm2)->ਖੇਤਰ.le_ਅਗਲਾ = (elm)->ਖੇਤਰ.le_ਅਗਲਾ) != ਨਿਰਾ)	\
		(elm2)->ਖੇਤਰ.le_ਅਗਲਾ->ਖੇਤਰ.le_prev =			\
		    &(elm2)->ਖੇਤਰ.le_ਅਗਲਾ;				\
	(elm2)->ਖੇਤਰ.le_prev = (elm)->ਖੇਤਰ.le_prev;			\
	*(elm2)->ਖੇਤਰ.le_prev = (elm2);				\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), ਖੇਤਰ)			\
} ਜਦੋਂ (/*CONSTCOND*/0)

/*
 * Simple queue definitions.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_ਸਿਰ(name, type)					\
ਬਣਤਰ name {								\
	ਬਣਤਰ type *sqh_ਪਹਿਲਾ;	/* first element */			\
	ਬਣਤਰ type **sqh_ਆਖਰੀ;	/* addr of last next element */		\
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_ਸਿਰ_INITIALIZER(ਸਿਰ)					\
	{ ਨਿਰਾ, &(ਸਿਰ).sqh_ਪਹਿਲਾ }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_ਇੰਦਰਾਜ(type)						\
ਬਣਤਰ {								\
	ਬਣਤਰ type *sqe_ਅਗਲਾ;	/* next element */			\
}

/*
 * Simple queue access methods.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_ਪਹਿਲਾ(ਸਿਰ)		((ਸਿਰ)->sqh_ਪਹਿਲਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_ਸਿਰਾ(ਸਿਰ)		ਨਿਰਾ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_ਖਾਲੀ_ਥਾਵਾਂ(ਸਿਰ)		((ਸਿਰ)->sqh_ਪਹਿਲਾ == SIMPLEQ_ਸਿਰਾ(ਸਿਰ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_ਅਗਲਾ(elm, ਖੇਤਰ)	((elm)->ਖੇਤਰ.sqe_ਅਗਲਾ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_FOREACH(var, ਸਿਰ, ਖੇਤਰ)				\
	ਲਈ ((var) = ((ਸਿਰ)->sqh_ਪਹਿਲਾ);				\
	    (var) != SIMPLEQ_ਸਿਰਾ(ਸਿਰ);					\
	    (var) = ((var)->ਖੇਤਰ.sqe_ਅਗਲਾ))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_FOREACH_ਸੁਰੱਖਿਅਤ(var, ਸਿਰ, ਖੇਤਰ, ਅਗਲਾ)			\
	ਲਈ ((var) = ((ਸਿਰ)->sqh_ਪਹਿਲਾ);				\
	    (var) != SIMPLEQ_ਸਿਰਾ(ਸਿਰ) &&				\
	    ((ਅਗਲਾ = ((var)->ਖੇਤਰ.sqe_ਅਗਲਾ)), 1);			\
	    (var) = (ਅਗਲਾ))

/*
 * Simple queue functions.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_INIT(ਸਿਰ) ਕਰੋ {						\
	(ਸਿਰ)->sqh_ਪਹਿਲਾ = ਨਿਰਾ;					\
	(ਸਿਰ)->sqh_ਆਖਰੀ = &(ਸਿਰ)->sqh_ਪਹਿਲਾ;				\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_ਪਾਓ_ਸਿਰ(ਸਿਰ, elm, ਖੇਤਰ) ਕਰੋ {			\
	ਜੇ (((elm)->ਖੇਤਰ.sqe_ਅਗਲਾ = (ਸਿਰ)->sqh_ਪਹਿਲਾ) == ਨਿਰਾ)	\
		(ਸਿਰ)->sqh_ਆਖਰੀ = &(elm)->ਖੇਤਰ.sqe_ਅਗਲਾ;		\
	(ਸਿਰ)->sqh_ਪਹਿਲਾ = (elm);					\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_ਪਾਓ_ਪੂਛ(ਸਿਰ, elm, ਖੇਤਰ) ਕਰੋ {			\
	(elm)->ਖੇਤਰ.sqe_ਅਗਲਾ = ਨਿਰਾ;					\
	*(ਸਿਰ)->sqh_ਆਖਰੀ = (elm);					\
	(ਸਿਰ)->sqh_ਆਖਰੀ = &(elm)->ਖੇਤਰ.sqe_ਅਗਲਾ;			\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_ਪਾਓ_AFTER(ਸਿਰ, listelm, elm, ਖੇਤਰ) ਕਰੋ {		\
	ਜੇ (((elm)->ਖੇਤਰ.sqe_ਅਗਲਾ = (listelm)->ਖੇਤਰ.sqe_ਅਗਲਾ) == ਨਿਰਾ)\
		(ਸਿਰ)->sqh_ਆਖਰੀ = &(elm)->ਖੇਤਰ.sqe_ਅਗਲਾ;		\
	(listelm)->ਖੇਤਰ.sqe_ਅਗਲਾ = (elm);				\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_REMOVE_ਸਿਰ(ਸਿਰ, ਖੇਤਰ) ਕਰੋ {				\
	ਜੇ (((ਸਿਰ)->sqh_ਪਹਿਲਾ = (ਸਿਰ)->sqh_ਪਹਿਲਾ->ਖੇਤਰ.sqe_ਅਗਲਾ) == ਨਿਰਾ) \
		(ਸਿਰ)->sqh_ਆਖਰੀ = &(ਸਿਰ)->sqh_ਪਹਿਲਾ;			\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIMPLEQ_REMOVE_AFTER(ਸਿਰ, elm, ਖੇਤਰ) ਕਰੋ {			\
	ਜੇ (((elm)->ਖੇਤਰ.sqe_ਅਗਲਾ = (elm)->ਖੇਤਰ.sqe_ਅਗਲਾ->ਖੇਤਰ.sqe_ਅਗਲਾ) \
	    == ਨਿਰਾ)							\
		(ਸਿਰ)->sqh_ਆਖਰੀ = &(elm)->ਖੇਤਰ.sqe_ਅਗਲਾ;		\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_REMOVE(ਸਿਰ, elm, type, ਖੇਤਰ) ਕਰੋ {			\
	ਜੇ ((ਸਿਰ)->sqh_ਪਹਿਲਾ == (elm)) {				\
		SIMPLEQ_REMOVE_ਸਿਰ((ਸਿਰ), ਖੇਤਰ);			\
	} ਨਹੀਂਤਾਂ {							\
		ਬਣਤਰ type *curelm = (ਸਿਰ)->sqh_ਪਹਿਲਾ;		\
		ਜਦੋਂ (curelm->ਖੇਤਰ.sqe_ਅਗਲਾ != (elm))			\
			curelm = curelm->ਖੇਤਰ.sqe_ਅਗਲਾ;		\
		ਜੇ ((curelm->ਖੇਤਰ.sqe_ਅਗਲਾ =				\
			curelm->ਖੇਤਰ.sqe_ਅਗਲਾ->ਖੇਤਰ.sqe_ਅਗਲਾ) == ਨਿਰਾ) \
			    (ਸਿਰ)->sqh_ਆਖਰੀ = &(curelm)->ਖੇਤਰ.sqe_ਅਗਲਾ; \
	}								\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_CONCAT(head1, head2) ਕਰੋ {				\
	ਜੇ (!SIMPLEQ_ਖਾਲੀ_ਥਾਵਾਂ((head2))) {					\
		*(head1)->sqh_ਆਖਰੀ = (head2)->sqh_ਪਹਿਲਾ;		\
		(head1)->sqh_ਆਖਰੀ = (head2)->sqh_ਆਖਰੀ;		\
		SIMPLEQ_INIT((head2));					\
	}								\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SIMPLEQ_ਆਖਰੀ(ਸਿਰ, type, ਖੇਤਰ)					\
	(SIMPLEQ_ਖਾਲੀ_ਥਾਵਾਂ((ਸਿਰ)) ?						\
		ਨਿਰਾ :							\
	        ((ਬਣਤਰ type *)(ਖਾਲੀ *)				\
		((ਅੱਖਰ *)((ਸਿਰ)->sqh_ਆਖਰੀ) - offsetof(ਬਣਤਰ type, ਖੇਤਰ))))

/*
 * Tail queue definitions.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_ਸਿਰ_2(name, type, qual)					\
ਬਣਤਰ name {								\
	qual type *tqh_ਪਹਿਲਾ;		/* first element */		\
	qual type *qual *tqh_ਆਖਰੀ;	/* addr of last next element */	\
}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TAILQ_ਸਿਰ(name, type)	TAILQ_ਸਿਰ_2(name, ਬਣਤਰ type,)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_ਸਿਰ_INITIALIZER(ਸਿਰ)					\
	{ TAILQ_ਸਿਰਾ(ਸਿਰ), &(ਸਿਰ).tqh_ਪਹਿਲਾ }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_ਇੰਦਰਾਜ_2(type, qual)					\
ਬਣਤਰ {								\
	qual type *tqe_ਅਗਲਾ;		/* next element */		\
	qual type *qual *tqe_prev;	/* address of previous next element */\
}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TAILQ_ਇੰਦਰਾਜ(type)	TAILQ_ਇੰਦਰਾਜ_2(ਬਣਤਰ type,)

/*
 * Tail queue access methods.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_ਪਹਿਲਾ(ਸਿਰ)		((ਸਿਰ)->tqh_ਪਹਿਲਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_ਸਿਰਾ(ਸਿਰ)			(ਨਿਰਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_ਅਗਲਾ(elm, ਖੇਤਰ)		((elm)->ਖੇਤਰ.tqe_ਅਗਲਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_ਆਖਰੀ(ਸਿਰ, headname) \
	(*(((ਬਣਤਰ headname *)((ਸਿਰ)->tqh_ਆਖਰੀ))->tqh_ਆਖਰੀ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_PREV(elm, headname, ਖੇਤਰ) \
	(*(((ਬਣਤਰ headname *)((elm)->ਖੇਤਰ.tqe_prev))->tqh_ਆਖਰੀ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_ਖਾਲੀ_ਥਾਵਾਂ(ਸਿਰ)		(TAILQ_ਪਹਿਲਾ(ਸਿਰ) == TAILQ_ਸਿਰਾ(ਸਿਰ))


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_FOREACH(var, ਸਿਰ, ਖੇਤਰ)					\
	ਲਈ ((var) = ((ਸਿਰ)->tqh_ਪਹਿਲਾ);				\
	    (var) != TAILQ_ਸਿਰਾ(ਸਿਰ);					\
	    (var) = ((var)->ਖੇਤਰ.tqe_ਅਗਲਾ))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(var, ਸਿਰ, ਖੇਤਰ, ਅਗਲਾ)			\
	ਲਈ ((var) = ((ਸਿਰ)->tqh_ਪਹਿਲਾ);				\
	    (var) != TAILQ_ਸਿਰਾ(ਸਿਰ) &&					\
	    ((ਅਗਲਾ) = TAILQ_ਅਗਲਾ(var, ਖੇਤਰ), 1); (var) = (ਅਗਲਾ))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_FOREACH_ਉਲਟ(var, ਸਿਰ, headname, ਖੇਤਰ)		\
	ਲਈ ((var) = (*(((ਬਣਤਰ headname *)((ਸਿਰ)->tqh_ਆਖਰੀ))->tqh_ਆਖਰੀ));\
	    (var) != TAILQ_ਸਿਰਾ(ਸਿਰ);					\
	    (var) = (*(((ਬਣਤਰ headname *)((var)->ਖੇਤਰ.tqe_prev))->tqh_ਆਖਰੀ)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_FOREACH_ਉਲਟ_ਸੁਰੱਖਿਅਤ(var, ਸਿਰ, headname, ਖੇਤਰ, prev)	\
	ਲਈ ((var) = TAILQ_ਆਖਰੀ((ਸਿਰ), headname);			\
	    (var) != TAILQ_ਸਿਰਾ(ਸਿਰ) && 				\
	    ((prev) = TAILQ_PREV((var), headname, ਖੇਤਰ), 1); (var) = (prev))

/*
 * Tail queue functions.
 */
#ਜੇ defined(QUEUEDEBUG)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_TAILQ_ਪਾਓ_ਸਿਰ(ਸਿਰ, elm, ਖੇਤਰ)			\
	ਜੇ ((ਸਿਰ)->tqh_ਪਹਿਲਾ &&					\
	    (ਸਿਰ)->tqh_ਪਹਿਲਾ->ਖੇਤਰ.tqe_prev != &(ਸਿਰ)->tqh_ਪਹਿਲਾ)	\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_HEAD %p %s:%d", (ਸਿਰ),	\
		    __FILE__, __LINE__);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_TAILQ_ਪਾਓ_ਪੂਛ(ਸਿਰ, elm, ਖੇਤਰ)			\
	ਜੇ (*(ਸਿਰ)->tqh_ਆਖਰੀ != ਨਿਰਾ)					\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_TAIL %p %s:%d", (ਸਿਰ),	\
		    __FILE__, __LINE__);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_TAILQ_OP(elm, ਖੇਤਰ)					\
	ਜੇ ((elm)->ਖੇਤਰ.tqe_ਅਗਲਾ &&					\
	    (elm)->ਖੇਤਰ.tqe_ਅਗਲਾ->ਖੇਤਰ.tqe_prev !=			\
	    &(elm)->ਖੇਤਰ.tqe_ਅਗਲਾ)					\
		QUEUEDEBUG_ABORT("TAILQ_* forw %p %s:%d", (elm),	\
		    __FILE__, __LINE__);				\
	ਜੇ (*(elm)->ਖੇਤਰ.tqe_prev != (elm))				\
		QUEUEDEBUG_ABORT("TAILQ_* back %p %s:%d", (elm),	\
		    __FILE__, __LINE__);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_TAILQ_PREREMOVE(ਸਿਰ, elm, ਖੇਤਰ)			\
	ਜੇ ((elm)->ਖੇਤਰ.tqe_ਅਗਲਾ == ਨਿਰਾ &&				\
	    (ਸਿਰ)->tqh_ਆਖਰੀ != &(elm)->ਖੇਤਰ.tqe_ਅਗਲਾ)			\
		QUEUEDEBUG_ABORT("TAILQ_PREREMOVE head %p elm %p %s:%d",\
		    (ਸਿਰ), (elm), __FILE__, __LINE__);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, ਖੇਤਰ)				\
	(elm)->ਖੇਤਰ.tqe_ਅਗਲਾ = (ਖਾਲੀ *)1L;				\
	(elm)->ਖੇਤਰ.tqe_prev = (ਖਾਲੀ *)1L;
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_TAILQ_ਪਾਓ_ਸਿਰ(ਸਿਰ, elm, ਖੇਤਰ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_TAILQ_ਪਾਓ_ਪੂਛ(ਸਿਰ, elm, ਖੇਤਰ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_TAILQ_OP(elm, ਖੇਤਰ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_TAILQ_PREREMOVE(ਸਿਰ, elm, ਖੇਤਰ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, ਖੇਤਰ)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_INIT(ਸਿਰ) ਕਰੋ {						\
	(ਸਿਰ)->tqh_ਪਹਿਲਾ = TAILQ_ਸਿਰਾ(ਸਿਰ);				\
	(ਸਿਰ)->tqh_ਆਖਰੀ = &(ਸਿਰ)->tqh_ਪਹਿਲਾ;				\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_ਪਾਓ_ਸਿਰ(ਸਿਰ, elm, ਖੇਤਰ) ਕਰੋ {			\
	QUEUEDEBUG_TAILQ_ਪਾਓ_ਸਿਰ((ਸਿਰ), (elm), ਖੇਤਰ)		\
	ਜੇ (((elm)->ਖੇਤਰ.tqe_ਅਗਲਾ = (ਸਿਰ)->tqh_ਪਹਿਲਾ) != TAILQ_ਸਿਰਾ(ਸਿਰ))\
		(ਸਿਰ)->tqh_ਪਹਿਲਾ->ਖੇਤਰ.tqe_prev =			\
		    &(elm)->ਖੇਤਰ.tqe_ਅਗਲਾ;				\
	ਨਹੀਂਤਾਂ								\
		(ਸਿਰ)->tqh_ਆਖਰੀ = &(elm)->ਖੇਤਰ.tqe_ਅਗਲਾ;		\
	(ਸਿਰ)->tqh_ਪਹਿਲਾ = (elm);					\
	(elm)->ਖੇਤਰ.tqe_prev = &(ਸਿਰ)->tqh_ਪਹਿਲਾ;			\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_ਪਾਓ_ਪੂਛ(ਸਿਰ, elm, ਖੇਤਰ) ਕਰੋ {			\
	QUEUEDEBUG_TAILQ_ਪਾਓ_ਪੂਛ((ਸਿਰ), (elm), ਖੇਤਰ)		\
	(elm)->ਖੇਤਰ.tqe_ਅਗਲਾ = TAILQ_ਸਿਰਾ(ਸਿਰ);			\
	(elm)->ਖੇਤਰ.tqe_prev = (ਸਿਰ)->tqh_ਆਖਰੀ;			\
	*(ਸਿਰ)->tqh_ਆਖਰੀ = (elm);					\
	(ਸਿਰ)->tqh_ਆਖਰੀ = &(elm)->ਖੇਤਰ.tqe_ਅਗਲਾ;			\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_ਪਾਓ_AFTER(ਸਿਰ, listelm, elm, ਖੇਤਰ) ਕਰੋ {		\
	QUEUEDEBUG_TAILQ_OP((listelm), ਖੇਤਰ)				\
	ਜੇ (((elm)->ਖੇਤਰ.tqe_ਅਗਲਾ = (listelm)->ਖੇਤਰ.tqe_ਅਗਲਾ) != 	\
	    TAILQ_ਸਿਰਾ(ਸਿਰ))						\
		(elm)->ਖੇਤਰ.tqe_ਅਗਲਾ->ਖੇਤਰ.tqe_prev = 		\
		    &(elm)->ਖੇਤਰ.tqe_ਅਗਲਾ;				\
	ਨਹੀਂਤਾਂ								\
		(ਸਿਰ)->tqh_ਆਖਰੀ = &(elm)->ਖੇਤਰ.tqe_ਅਗਲਾ;		\
	(listelm)->ਖੇਤਰ.tqe_ਅਗਲਾ = (elm);				\
	(elm)->ਖੇਤਰ.tqe_prev = &(listelm)->ਖੇਤਰ.tqe_ਅਗਲਾ;		\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_ਪਾਓ_BEFORE(listelm, elm, ਖੇਤਰ) ਕਰੋ {			\
	QUEUEDEBUG_TAILQ_OP((listelm), ਖੇਤਰ)				\
	(elm)->ਖੇਤਰ.tqe_prev = (listelm)->ਖੇਤਰ.tqe_prev;		\
	(elm)->ਖੇਤਰ.tqe_ਅਗਲਾ = (listelm);				\
	*(listelm)->ਖੇਤਰ.tqe_prev = (elm);				\
	(listelm)->ਖੇਤਰ.tqe_prev = &(elm)->ਖੇਤਰ.tqe_ਅਗਲਾ;		\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_REMOVE(ਸਿਰ, elm, ਖੇਤਰ) ਕਰੋ {				\
	QUEUEDEBUG_TAILQ_PREREMOVE((ਸਿਰ), (elm), ਖੇਤਰ)		\
	QUEUEDEBUG_TAILQ_OP((elm), ਖੇਤਰ)				\
	ਜੇ (((elm)->ਖੇਤਰ.tqe_ਅਗਲਾ) != TAILQ_ਸਿਰਾ(ਸਿਰ))			\
		(elm)->ਖੇਤਰ.tqe_ਅਗਲਾ->ਖੇਤਰ.tqe_prev = 		\
		    (elm)->ਖੇਤਰ.tqe_prev;				\
	ਨਹੀਂਤਾਂ								\
		(ਸਿਰ)->tqh_ਆਖਰੀ = (elm)->ਖੇਤਰ.tqe_prev;		\
	*(elm)->ਖੇਤਰ.tqe_prev = (elm)->ਖੇਤਰ.tqe_ਅਗਲਾ;			\
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), ਖੇਤਰ);			\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TAILQ_REPLACE(ਸਿਰ, elm, elm2, ਖੇਤਰ) ਕਰੋ {                      \
        ਜੇ (((elm2)->ਖੇਤਰ.tqe_ਅਗਲਾ = (elm)->ਖੇਤਰ.tqe_ਅਗਲਾ) != 	\
	    TAILQ_ਸਿਰਾ(ਸਿਰ))   						\
                (elm2)->ਖੇਤਰ.tqe_ਅਗਲਾ->ਖੇਤਰ.tqe_prev =                \
                    &(elm2)->ਖੇਤਰ.tqe_ਅਗਲਾ;                            \
        ਨਹੀਂਤਾਂ                                                            \
                (ਸਿਰ)->tqh_ਆਖਰੀ = &(elm2)->ਖੇਤਰ.tqe_ਅਗਲਾ;             \
        (elm2)->ਖੇਤਰ.tqe_prev = (elm)->ਖੇਤਰ.tqe_prev;                 \
        *(elm2)->ਖੇਤਰ.tqe_prev = (elm2);                               \
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), ਖੇਤਰ);			\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	TAILQ_CONCAT(head1, head2, ਖੇਤਰ) ਕਰੋ {				\
	ਜੇ (!TAILQ_ਖਾਲੀ_ਥਾਵਾਂ(head2)) {					\
		*(head1)->tqh_ਆਖਰੀ = (head2)->tqh_ਪਹਿਲਾ;		\
		(head2)->tqh_ਪਹਿਲਾ->ਖੇਤਰ.tqe_prev = (head1)->tqh_ਆਖਰੀ;	\
		(head1)->tqh_ਆਖਰੀ = (head2)->tqh_ਆਖਰੀ;			\
		TAILQ_INIT((head2));					\
	}								\
} ਜਦੋਂ (/*CONSTCOND*/0)

/*
 * Singly-linked Tail queue declarations.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_ਸਿਰ(name, type)						\
ਬਣਤਰ name {								\
	ਬਣਤਰ type *stqh_ਪਹਿਲਾ;	/* first element */		\
	ਬਣਤਰ type **stqh_ਆਖਰੀ;	/* addr of last next element */	\
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_ਸਿਰ_INITIALIZER(ਸਿਰ)					\
	{ ਨਿਰਾ, &(ਸਿਰ).stqh_ਪਹਿਲਾ }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_ਇੰਦਰਾਜ(type)						\
ਬਣਤਰ {								\
	ਬਣਤਰ type *stqe_ਅਗਲਾ;	/* next element */			\
}

/*
 * Singly-linked Tail queue access methods.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_ਪਹਿਲਾ(ਸਿਰ)	((ਸਿਰ)->stqh_ਪਹਿਲਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_ਸਿਰਾ(ਸਿਰ)	ਨਿਰਾ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_ਅਗਲਾ(elm, ਖੇਤਰ)	((elm)->ਖੇਤਰ.stqe_ਅਗਲਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_ਖਾਲੀ_ਥਾਵਾਂ(ਸਿਰ)	(STAILQ_ਪਹਿਲਾ(ਸਿਰ) == STAILQ_ਸਿਰਾ(ਸਿਰ))

/*
 * Singly-linked Tail queue functions.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_INIT(ਸਿਰ) ਕਰੋ {						\
	(ਸਿਰ)->stqh_ਪਹਿਲਾ = ਨਿਰਾ;					\
	(ਸਿਰ)->stqh_ਆਖਰੀ = &(ਸਿਰ)->stqh_ਪਹਿਲਾ;				\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_ਪਾਓ_ਸਿਰ(ਸਿਰ, elm, ਖੇਤਰ) ਕਰੋ {			\
	ਜੇ (((elm)->ਖੇਤਰ.stqe_ਅਗਲਾ = (ਸਿਰ)->stqh_ਪਹਿਲਾ) == ਨਿਰਾ)	\
		(ਸਿਰ)->stqh_ਆਖਰੀ = &(elm)->ਖੇਤਰ.stqe_ਅਗਲਾ;		\
	(ਸਿਰ)->stqh_ਪਹਿਲਾ = (elm);					\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_ਪਾਓ_ਪੂਛ(ਸਿਰ, elm, ਖੇਤਰ) ਕਰੋ {			\
	(elm)->ਖੇਤਰ.stqe_ਅਗਲਾ = ਨਿਰਾ;					\
	*(ਸਿਰ)->stqh_ਆਖਰੀ = (elm);					\
	(ਸਿਰ)->stqh_ਆਖਰੀ = &(elm)->ਖੇਤਰ.stqe_ਅਗਲਾ;			\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_ਪਾਓ_AFTER(ਸਿਰ, listelm, elm, ਖੇਤਰ) ਕਰੋ {		\
	ਜੇ (((elm)->ਖੇਤਰ.stqe_ਅਗਲਾ = (listelm)->ਖੇਤਰ.stqe_ਅਗਲਾ) == ਨਿਰਾ)\
		(ਸਿਰ)->stqh_ਆਖਰੀ = &(elm)->ਖੇਤਰ.stqe_ਅਗਲਾ;		\
	(listelm)->ਖੇਤਰ.stqe_ਅਗਲਾ = (elm);				\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_REMOVE_ਸਿਰ(ਸਿਰ, ਖੇਤਰ) ਕਰੋ {				\
	ਜੇ (((ਸਿਰ)->stqh_ਪਹਿਲਾ = (ਸਿਰ)->stqh_ਪਹਿਲਾ->ਖੇਤਰ.stqe_ਅਗਲਾ) == ਨਿਰਾ) \
		(ਸਿਰ)->stqh_ਆਖਰੀ = &(ਸਿਰ)->stqh_ਪਹਿਲਾ;			\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_REMOVE(ਸਿਰ, elm, type, ਖੇਤਰ) ਕਰੋ {			\
	ਜੇ ((ਸਿਰ)->stqh_ਪਹਿਲਾ == (elm)) {				\
		STAILQ_REMOVE_ਸਿਰ((ਸਿਰ), ਖੇਤਰ);			\
	} ਨਹੀਂਤਾਂ {							\
		ਬਣਤਰ type *curelm = (ਸਿਰ)->stqh_ਪਹਿਲਾ;		\
		ਜਦੋਂ (curelm->ਖੇਤਰ.stqe_ਅਗਲਾ != (elm))			\
			curelm = curelm->ਖੇਤਰ.stqe_ਅਗਲਾ;		\
		ਜੇ ((curelm->ਖੇਤਰ.stqe_ਅਗਲਾ =				\
			curelm->ਖੇਤਰ.stqe_ਅਗਲਾ->ਖੇਤਰ.stqe_ਅਗਲਾ) == ਨਿਰਾ) \
			    (ਸਿਰ)->stqh_ਆਖਰੀ = &(curelm)->ਖੇਤਰ.stqe_ਅਗਲਾ; \
	}								\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_FOREACH(var, ਸਿਰ, ਖੇਤਰ)				\
	ਲਈ ((var) = ((ਸਿਰ)->stqh_ਪਹਿਲਾ);				\
		(var);							\
		(var) = ((var)->ਖੇਤਰ.stqe_ਅਗਲਾ))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(var, ਸਿਰ, ਖੇਤਰ, tvar)			\
	ਲਈ ((var) = STAILQ_ਪਹਿਲਾ((ਸਿਰ));				\
	    (var) && ((tvar) = STAILQ_ਅਗਲਾ((var), ਖੇਤਰ), 1);		\
	    (var) = (tvar))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_CONCAT(head1, head2) ਕਰੋ {				\
	ਜੇ (!STAILQ_ਖਾਲੀ_ਥਾਵਾਂ((head2))) {					\
		*(head1)->stqh_ਆਖਰੀ = (head2)->stqh_ਪਹਿਲਾ;		\
		(head1)->stqh_ਆਖਰੀ = (head2)->stqh_ਆਖਰੀ;		\
		STAILQ_INIT((head2));					\
	}								\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STAILQ_ਆਖਰੀ(ਸਿਰ, type, ਖੇਤਰ)					\
	(STAILQ_ਖਾਲੀ_ਥਾਵਾਂ((ਸਿਰ)) ?						\
		ਨਿਰਾ :							\
	        ((ਬਣਤਰ type *)(ਖਾਲੀ *)				\
		((ਅੱਖਰ *)((ਸਿਰ)->stqh_ਆਖਰੀ) - offsetof(ਬਣਤਰ type, ਖੇਤਰ))))


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _KERNEL
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
ਸਥਿਰ __inline ਅਚਲ ਖਾਲੀ * __launder_type(ਅਚਲ ਖਾਲੀ *);
ਸਥਿਰ __inline ਅਚਲ ਖਾਲੀ *
__launder_type(ਅਚਲ ਖਾਲੀ *__x)
{
	__asm __volatile("" : "+r" (__x));
	ਵਾਪਸ __x;
}

#ਜੇ defined(QUEUEDEBUG)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QUEUEDEBUG_CIRCLEQ_ਸਿਰ(ਸਿਰ, ਖੇਤਰ)				\
	ਜੇ ((ਸਿਰ)->cqh_ਪਹਿਲਾ != CIRCLEQ_ENDC(ਸਿਰ) &&			\
	    (ਸਿਰ)->cqh_ਪਹਿਲਾ->ਖੇਤਰ.cqe_prev != CIRCLEQ_ENDC(ਸਿਰ))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head forw %p %s:%d", (ਸਿਰ),	\
		      __FILE__, __LINE__);				\
	ਜੇ ((ਸਿਰ)->cqh_ਆਖਰੀ != CIRCLEQ_ENDC(ਸਿਰ) &&			\
	    (ਸਿਰ)->cqh_ਆਖਰੀ->ਖੇਤਰ.cqe_ਅਗਲਾ != CIRCLEQ_ENDC(ਸਿਰ))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head back %p %s:%d", (ਸਿਰ),	\
		      __FILE__, __LINE__);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QUEUEDEBUG_CIRCLEQ_ELM(ਸਿਰ, elm, ਖੇਤਰ)			\
	ਜੇ ((elm)->ਖੇਤਰ.cqe_ਅਗਲਾ == CIRCLEQ_ENDC(ਸਿਰ)) {		\
		ਜੇ ((ਸਿਰ)->cqh_ਆਖਰੀ != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm last %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} ਨਹੀਂਤਾਂ {							\
		ਜੇ ((elm)->ਖੇਤਰ.cqe_ਅਗਲਾ->ਖੇਤਰ.cqe_prev != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm forw %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}								\
	ਜੇ ((elm)->ਖੇਤਰ.cqe_prev == CIRCLEQ_ENDC(ਸਿਰ)) {		\
		ਜੇ ((ਸਿਰ)->cqh_ਪਹਿਲਾ != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm first %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} ਨਹੀਂਤਾਂ {							\
		ਜੇ ((elm)->ਖੇਤਰ.cqe_prev->ਖੇਤਰ.cqe_ਅਗਲਾ != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm prev %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, ਖੇਤਰ)			\
	(elm)->ਖੇਤਰ.cqe_ਅਗਲਾ = (ਖਾਲੀ *)1L;				\
	(elm)->ਖੇਤਰ.cqe_prev = (ਖਾਲੀ *)1L;
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QUEUEDEBUG_CIRCLEQ_ਸਿਰ(ਸਿਰ, ਖੇਤਰ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QUEUEDEBUG_CIRCLEQ_ELM(ਸਿਰ, elm, ਖੇਤਰ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, ਖੇਤਰ)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_ਸਿਰ(name, type)					\
ਬਣਤਰ name {								\
	ਬਣਤਰ type *cqh_ਪਹਿਲਾ;		/* first element */		\
	ਬਣਤਰ type *cqh_ਆਖਰੀ;		/* last element */		\
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_ਸਿਰ_INITIALIZER(ਸਿਰ)					\
	{ CIRCLEQ_ਸਿਰਾ(&ਸਿਰ), CIRCLEQ_ਸਿਰਾ(&ਸਿਰ) }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_ਇੰਦਰਾਜ(type)						\
ਬਣਤਰ {								\
	ਬਣਤਰ type *cqe_ਅਗਲਾ;		/* next element */		\
	ਬਣਤਰ type *cqe_prev;		/* previous element */		\
}

/*
 * Circular queue functions.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_INIT(ਸਿਰ) ਕਰੋ {						\
	(ਸਿਰ)->cqh_ਪਹਿਲਾ = CIRCLEQ_ਸਿਰਾ(ਸਿਰ);				\
	(ਸਿਰ)->cqh_ਆਖਰੀ = CIRCLEQ_ਸਿਰਾ(ਸਿਰ);				\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_ਪਾਓ_AFTER(ਸਿਰ, listelm, elm, ਖੇਤਰ) ਕਰੋ {		\
	QUEUEDEBUG_CIRCLEQ_ਸਿਰ((ਸਿਰ), ਖੇਤਰ)				\
	QUEUEDEBUG_CIRCLEQ_ELM((ਸਿਰ), (listelm), ਖੇਤਰ)		\
	(elm)->ਖੇਤਰ.cqe_ਅਗਲਾ = (listelm)->ਖੇਤਰ.cqe_ਅਗਲਾ;		\
	(elm)->ਖੇਤਰ.cqe_prev = (listelm);				\
	ਜੇ ((listelm)->ਖੇਤਰ.cqe_ਅਗਲਾ == CIRCLEQ_ENDC(ਸਿਰ))		\
		(ਸਿਰ)->cqh_ਆਖਰੀ = (elm);				\
	ਨਹੀਂਤਾਂ								\
		(listelm)->ਖੇਤਰ.cqe_ਅਗਲਾ->ਖੇਤਰ.cqe_prev = (elm);	\
	(listelm)->ਖੇਤਰ.cqe_ਅਗਲਾ = (elm);				\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_ਪਾਓ_BEFORE(ਸਿਰ, listelm, elm, ਖੇਤਰ) ਕਰੋ {		\
	QUEUEDEBUG_CIRCLEQ_ਸਿਰ((ਸਿਰ), ਖੇਤਰ)				\
	QUEUEDEBUG_CIRCLEQ_ELM((ਸਿਰ), (listelm), ਖੇਤਰ)		\
	(elm)->ਖੇਤਰ.cqe_ਅਗਲਾ = (listelm);				\
	(elm)->ਖੇਤਰ.cqe_prev = (listelm)->ਖੇਤਰ.cqe_prev;		\
	ਜੇ ((listelm)->ਖੇਤਰ.cqe_prev == CIRCLEQ_ENDC(ਸਿਰ))		\
		(ਸਿਰ)->cqh_ਪਹਿਲਾ = (elm);				\
	ਨਹੀਂਤਾਂ								\
		(listelm)->ਖੇਤਰ.cqe_prev->ਖੇਤਰ.cqe_ਅਗਲਾ = (elm);	\
	(listelm)->ਖੇਤਰ.cqe_prev = (elm);				\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_ਪਾਓ_ਸਿਰ(ਸਿਰ, elm, ਖੇਤਰ) ਕਰੋ {			\
	QUEUEDEBUG_CIRCLEQ_ਸਿਰ((ਸਿਰ), ਖੇਤਰ)				\
	(elm)->ਖੇਤਰ.cqe_ਅਗਲਾ = (ਸਿਰ)->cqh_ਪਹਿਲਾ;			\
	(elm)->ਖੇਤਰ.cqe_prev = CIRCLEQ_ਸਿਰਾ(ਸਿਰ);			\
	ਜੇ ((ਸਿਰ)->cqh_ਆਖਰੀ == CIRCLEQ_ENDC(ਸਿਰ))			\
		(ਸਿਰ)->cqh_ਆਖਰੀ = (elm);				\
	ਨਹੀਂਤਾਂ								\
		(ਸਿਰ)->cqh_ਪਹਿਲਾ->ਖੇਤਰ.cqe_prev = (elm);		\
	(ਸਿਰ)->cqh_ਪਹਿਲਾ = (elm);					\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_ਪਾਓ_ਪੂਛ(ਸਿਰ, elm, ਖੇਤਰ) ਕਰੋ {			\
	QUEUEDEBUG_CIRCLEQ_ਸਿਰ((ਸਿਰ), ਖੇਤਰ)				\
	(elm)->ਖੇਤਰ.cqe_ਅਗਲਾ = CIRCLEQ_ਸਿਰਾ(ਸਿਰ);			\
	(elm)->ਖੇਤਰ.cqe_prev = (ਸਿਰ)->cqh_ਆਖਰੀ;			\
	ਜੇ ((ਸਿਰ)->cqh_ਪਹਿਲਾ == CIRCLEQ_ENDC(ਸਿਰ))			\
		(ਸਿਰ)->cqh_ਪਹਿਲਾ = (elm);				\
	ਨਹੀਂਤਾਂ								\
		(ਸਿਰ)->cqh_ਆਖਰੀ->ਖੇਤਰ.cqe_ਅਗਲਾ = (elm);		\
	(ਸਿਰ)->cqh_ਆਖਰੀ = (elm);					\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_REMOVE(ਸਿਰ, elm, ਖੇਤਰ) ਕਰੋ {				\
	QUEUEDEBUG_CIRCLEQ_ਸਿਰ((ਸਿਰ), ਖੇਤਰ)				\
	QUEUEDEBUG_CIRCLEQ_ELM((ਸਿਰ), (elm), ਖੇਤਰ)			\
	ਜੇ ((elm)->ਖੇਤਰ.cqe_ਅਗਲਾ == CIRCLEQ_ENDC(ਸਿਰ))		\
		(ਸਿਰ)->cqh_ਆਖਰੀ = (elm)->ਖੇਤਰ.cqe_prev;		\
	ਨਹੀਂਤਾਂ								\
		(elm)->ਖੇਤਰ.cqe_ਅਗਲਾ->ਖੇਤਰ.cqe_prev =			\
		    (elm)->ਖੇਤਰ.cqe_prev;				\
	ਜੇ ((elm)->ਖੇਤਰ.cqe_prev == CIRCLEQ_ENDC(ਸਿਰ))		\
		(ਸਿਰ)->cqh_ਪਹਿਲਾ = (elm)->ਖੇਤਰ.cqe_ਅਗਲਾ;		\
	ਨਹੀਂਤਾਂ								\
		(elm)->ਖੇਤਰ.cqe_prev->ਖੇਤਰ.cqe_ਅਗਲਾ =			\
		    (elm)->ਖੇਤਰ.cqe_ਅਗਲਾ;				\
	QUEUEDEBUG_CIRCLEQ_POSTREMOVE((elm), ਖੇਤਰ)			\
} ਜਦੋਂ (/*CONSTCOND*/0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_FOREACH(var, ਸਿਰ, ਖੇਤਰ)				\
	ਲਈ ((var) = ((ਸਿਰ)->cqh_ਪਹਿਲਾ);				\
		(var) != CIRCLEQ_ENDC(ਸਿਰ);				\
		(var) = ((var)->ਖੇਤਰ.cqe_ਅਗਲਾ))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_FOREACH_ਉਲਟ(var, ਸਿਰ, ਖੇਤਰ)			\
	ਲਈ ((var) = ((ਸਿਰ)->cqh_ਆਖਰੀ);				\
		(var) != CIRCLEQ_ENDC(ਸਿਰ);				\
		(var) = ((var)->ਖੇਤਰ.cqe_prev))

/*
 * Circular queue access methods.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_ਪਹਿਲਾ(ਸਿਰ)		((ਸਿਰ)->cqh_ਪਹਿਲਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_ਆਖਰੀ(ਸਿਰ)		((ਸਿਰ)->cqh_ਆਖਰੀ)
/* For comparisons */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_ENDC(ਸਿਰ)		(__launder_type(ਸਿਰ))
/* For assignments */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_ਸਿਰਾ(ਸਿਰ)		((ਖਾਲੀ *)(ਸਿਰ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_ਅਗਲਾ(elm, ਖੇਤਰ)	((elm)->ਖੇਤਰ.cqe_ਅਗਲਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_PREV(elm, ਖੇਤਰ)	((elm)->ਖੇਤਰ.cqe_prev)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CIRCLEQ_ਖਾਲੀ_ਥਾਵਾਂ(ਸਿਰ)						\
    (CIRCLEQ_ਪਹਿਲਾ(ਸਿਰ) == CIRCLEQ_ENDC(ਸਿਰ))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIRCLEQ_LOOP_ਅਗਲਾ(ਸਿਰ, elm, ਖੇਤਰ)				\
	(((elm)->ਖੇਤਰ.cqe_ਅਗਲਾ == CIRCLEQ_ENDC(ਸਿਰ))			\
	    ? ((ਸਿਰ)->cqh_ਪਹਿਲਾ)					\
	    : (elm->ਖੇਤਰ.cqe_ਅਗਲਾ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIRCLEQ_LOOP_PREV(ਸਿਰ, elm, ਖੇਤਰ)				\
	(((elm)->ਖੇਤਰ.cqe_prev == CIRCLEQ_ENDC(ਸਿਰ))			\
	    ? ((ਸਿਰ)->cqh_ਆਖਰੀ)					\
	    : (elm->ਖੇਤਰ.cqe_prev))
#ਜੇ_ਅੰਤ /* !_KERNEL */

#ਜੇ_ਅੰਤ	/* !_SYS_QUEUE_H_ */
