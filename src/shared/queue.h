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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ	SYS_ಸರತಿ_H
#ವ್ಯಾಖ್ಯಾನಿಸು	SYS_ಸರತಿ_H

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __NetBSD__
#ಅನ್ತರ್ಭಾವ <sys/null.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(QUEUEDEBUG)
# ಆದರೆ defined(_KERNEL)
#  define QUEUEDEBUG_ABORT(...) panic(__VA_ARGS__)
# ಇಲ್ಲದಿದ್ದರೆ
#  include <err.h>
#  define QUEUEDEBUG_ABORT(...) err(1, __VA_ARGS__)
# endif
#ಆದರೆ_ಅಂತ್ಯ

/*
 * Singly-linked List definitions.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_ತಲೆ(name, type)						\
ರಚನೆ name {								\
	ರಚನೆ type *slh_ಮೊದಲ;	/* first element */			\
}

#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_ತಲೆ_INITIALIZER(ತಲೆ)					\
	{ ಖಾಲಿ }

#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_ನಮೂದು(type)						\
ರಚನೆ {								\
	ರಚನೆ type *sle_ಮುಂದಿನ;	/* next element */			\
}

/*
 * Singly-linked List access methods.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_ಮೊದಲ(ತಲೆ)	((ತಲೆ)->slh_ಮೊದಲ)
#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_ಕೊನೆ(ತಲೆ)		ಖಾಲಿ
#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_ಖಾಲಿಗಳು(ತಲೆ)	((ತಲೆ)->slh_ಮೊದಲ == ಖಾಲಿ)
#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_ಮುಂದಿನ(elm, ಕ್ಷೇತ್ರ)	((elm)->ಕ್ಷೇತ್ರ.sle_ಮುಂದಿನ)

#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_FOREACH(var, ತಲೆ, ಕ್ಷೇತ್ರ)					\
	ಗಾಗಿ((var) = (ತಲೆ)->slh_ಮೊದಲ;					\
	    (var) != SLIST_ಕೊನೆ(ತಲೆ);					\
	    (var) = (var)->ಕ್ಷೇತ್ರ.sle_ಮುಂದಿನ)

#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_FOREACH_ಸುರಕ್ಷಿತ(var, ತಲೆ, ಕ್ಷೇತ್ರ, tvar)			\
	ಗಾಗಿ ((var) = SLIST_ಮೊದಲ((ತಲೆ));				\
	    (var) != SLIST_ಕೊನೆ(ತಲೆ) &&					\
	    ((tvar) = SLIST_ಮುಂದಿನ((var), ಕ್ಷೇತ್ರ), 1);			\
	    (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_INIT(ತಲೆ) ಮಾಡು {						\
	(ತಲೆ)->slh_ಮೊದಲ = SLIST_ಕೊನೆ(ತಲೆ);				\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_ಸೇರಿಸು_AFTER(slistelm, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	(elm)->ಕ್ಷೇತ್ರ.sle_ಮುಂದಿನ = (slistelm)->ಕ್ಷೇತ್ರ.sle_ಮುಂದಿನ;		\
	(slistelm)->ಕ್ಷೇತ್ರ.sle_ಮುಂದಿನ = (elm);				\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_ಸೇರಿಸು_ತಲೆ(ತಲೆ, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	(elm)->ಕ್ಷೇತ್ರ.sle_ಮುಂದಿನ = (ತಲೆ)->slh_ಮೊದಲ;			\
	(ತಲೆ)->slh_ಮೊದಲ = (elm);					\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_REMOVE_AFTER(slistelm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	(slistelm)->ಕ್ಷೇತ್ರ.sle_ಮುಂದಿನ =					\
	    SLIST_ಮುಂದಿನ(SLIST_ಮುಂದಿನ((slistelm), ಕ್ಷೇತ್ರ), ಕ್ಷೇತ್ರ);		\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_REMOVE_ತಲೆ(ತಲೆ, ಕ್ಷೇತ್ರ) ಮಾಡು {				\
	(ತಲೆ)->slh_ಮೊದಲ = (ತಲೆ)->slh_ಮೊದಲ->ಕ್ಷೇತ್ರ.sle_ಮುಂದಿನ;		\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	SLIST_REMOVE(ತಲೆ, elm, type, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	ಆದರೆ ((ತಲೆ)->slh_ಮೊದಲ == (elm)) {				\
		SLIST_REMOVE_ತಲೆ((ತಲೆ), ಕ್ಷೇತ್ರ);			\
	}								\
	ಇಲ್ಲದಿದ್ದರೆ {								\
		ರಚನೆ type *curelm = (ತಲೆ)->slh_ಮೊದಲ;		\
		ಆಗ(curelm->ಕ್ಷೇತ್ರ.sle_ಮುಂದಿನ != (elm))			\
			curelm = curelm->ಕ್ಷೇತ್ರ.sle_ಮುಂದಿನ;		\
		curelm->ಕ್ಷೇತ್ರ.sle_ಮುಂದಿನ =				\
		    curelm->ಕ್ಷೇತ್ರ.sle_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.sle_ಮುಂದಿನ;		\
	}								\
} ಆಗ (/*CONSTCOND*/0)


/*
 * List definitions.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_ತಲೆ(name, type)						\
ರಚನೆ name {								\
	ರಚನೆ type *lh_ಮೊದಲ;	/* first element */			\
}

#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_ತಲೆ_INITIALIZER(ತಲೆ)					\
	{ ಖಾಲಿ }

#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_ನಮೂದು(type)						\
ರಚನೆ {								\
	ರಚನೆ type *le_ಮುಂದಿನ;	/* next element */			\
	ರಚನೆ type **le_prev;	/* address of previous next element */	\
}

/*
 * List access methods.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_ಮೊದಲ(ತಲೆ)		((ತಲೆ)->lh_ಮೊದಲ)
#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_ಕೊನೆ(ತಲೆ)			ಖಾಲಿ
#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_ಖಾಲಿಗಳು(ತಲೆ)		((ತಲೆ)->lh_ಮೊದಲ == LIST_ಕೊನೆ(ತಲೆ))
#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_ಮುಂದಿನ(elm, ಕ್ಷೇತ್ರ)		((elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ)

#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_FOREACH(var, ತಲೆ, ಕ್ಷೇತ್ರ)					\
	ಗಾಗಿ ((var) = ((ತಲೆ)->lh_ಮೊದಲ);				\
	    (var) != LIST_ಕೊನೆ(ತಲೆ);					\
	    (var) = ((var)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ))

#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_FOREACH_ಸುರಕ್ಷಿತ(var, ತಲೆ, ಕ್ಷೇತ್ರ, tvar)			\
	ಗಾಗಿ ((var) = LIST_ಮೊದಲ((ತಲೆ));				\
	    (var) != LIST_ಕೊನೆ(ತಲೆ) &&					\
	    ((tvar) = LIST_ಮುಂದಿನ((var), ಕ್ಷೇತ್ರ), 1);			\
	    (var) = (tvar))

#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_MOVE(head1, head2) ಮಾಡು {					\
	LIST_INIT((head2));						\
	ಆದರೆ (!LIST_ಖಾಲಿಗಳು((head1))) {					\
		(head2)->lh_ಮೊದಲ = (head1)->lh_ಮೊದಲ;			\
		LIST_INIT((head1));					\
	}								\
} ಆಗ (/*CONSTCOND*/0)

/*
 * List functions.
 */
#ಆದರೆ defined(QUEUEDEBUG)
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_LIST_ಸೇರಿಸು_ತಲೆ(ತಲೆ, elm, ಕ್ಷೇತ್ರ)			\
	ಆದರೆ ((ತಲೆ)->lh_ಮೊದಲ &&						\
	    (ತಲೆ)->lh_ಮೊದಲ->ಕ್ಷೇತ್ರ.le_prev != &(ತಲೆ)->lh_ಮೊದಲ)	\
		QUEUEDEBUG_ABORT("LIST_INSERT_HEAD %p %s:%d", (ತಲೆ),	\
		    __FILE__, __LINE__);
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_LIST_OP(elm, ಕ್ಷೇತ್ರ)					\
	ಆದರೆ ((elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ &&					\
	    (elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.le_prev !=			\
	    &(elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ)					\
		QUEUEDEBUG_ABORT("LIST_* forw %p %s:%d", (elm),		\
		    __FILE__, __LINE__);				\
	ಆದರೆ (*(elm)->ಕ್ಷೇತ್ರ.le_prev != (elm))				\
		QUEUEDEBUG_ABORT("LIST_* back %p %s:%d", (elm),		\
		    __FILE__, __LINE__);
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_LIST_POSTREMOVE(elm, ಕ್ಷೇತ್ರ)				\
	(elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ = (ಶೂನ್ಯ *)1L;				\
	(elm)->ಕ್ಷೇತ್ರ.le_prev = (ಶೂನ್ಯ *)1L;
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_LIST_ಸೇರಿಸು_ತಲೆ(ತಲೆ, elm, ಕ್ಷೇತ್ರ)
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_LIST_OP(elm, ಕ್ಷೇತ್ರ)
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_LIST_POSTREMOVE(elm, ಕ್ಷೇತ್ರ)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_INIT(ತಲೆ) ಮಾಡು {						\
	(ತಲೆ)->lh_ಮೊದಲ = LIST_ಕೊನೆ(ತಲೆ);				\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_ಸೇರಿಸು_AFTER(listelm, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	QUEUEDEBUG_LIST_OP((listelm), ಕ್ಷೇತ್ರ)				\
	ಆದರೆ (((elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ = (listelm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ) != 	\
	    LIST_ಕೊನೆ(ತಲೆ))						\
		(listelm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.le_prev =		\
		    &(elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ;				\
	(listelm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ = (elm);				\
	(elm)->ಕ್ಷೇತ್ರ.le_prev = &(listelm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ;		\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_ಸೇರಿಸು_BEFORE(listelm, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	QUEUEDEBUG_LIST_OP((listelm), ಕ್ಷೇತ್ರ)				\
	(elm)->ಕ್ಷೇತ್ರ.le_prev = (listelm)->ಕ್ಷೇತ್ರ.le_prev;		\
	(elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ = (listelm);				\
	*(listelm)->ಕ್ಷೇತ್ರ.le_prev = (elm);				\
	(listelm)->ಕ್ಷೇತ್ರ.le_prev = &(elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ;		\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_ಸೇರಿಸು_ತಲೆ(ತಲೆ, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {				\
	QUEUEDEBUG_LIST_ಸೇರಿಸು_ತಲೆ((ತಲೆ), (elm), ಕ್ಷೇತ್ರ)		\
	ಆದರೆ (((elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ = (ತಲೆ)->lh_ಮೊದಲ) != LIST_ಕೊನೆ(ತಲೆ))\
		(ತಲೆ)->lh_ಮೊದಲ->ಕ್ಷೇತ್ರ.le_prev = &(elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ;\
	(ತಲೆ)->lh_ಮೊದಲ = (elm);					\
	(elm)->ಕ್ಷೇತ್ರ.le_prev = &(ತಲೆ)->lh_ಮೊದಲ;			\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	LIST_REMOVE(elm, ಕ್ಷೇತ್ರ) ಮಾಡು {					\
	QUEUEDEBUG_LIST_OP((elm), ಕ್ಷೇತ್ರ)				\
	ಆದರೆ ((elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ != ಖಾಲಿ)				\
		(elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.le_prev = 			\
		    (elm)->ಕ್ಷೇತ್ರ.le_prev;				\
	*(elm)->ಕ್ಷೇತ್ರ.le_prev = (elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ;			\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), ಕ್ಷೇತ್ರ)			\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು LIST_REPLACE(elm, elm2, ಕ್ಷೇತ್ರ) ಮಾಡು {				\
	ಆದರೆ (((elm2)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ = (elm)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ) != ಖಾಲಿ)	\
		(elm2)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.le_prev =			\
		    &(elm2)->ಕ್ಷೇತ್ರ.le_ಮುಂದಿನ;				\
	(elm2)->ಕ್ಷೇತ್ರ.le_prev = (elm)->ಕ್ಷೇತ್ರ.le_prev;			\
	*(elm2)->ಕ್ಷೇತ್ರ.le_prev = (elm2);				\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), ಕ್ಷೇತ್ರ)			\
} ಆಗ (/*CONSTCOND*/0)

/*
 * Simple queue definitions.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_ತಲೆ(name, type)					\
ರಚನೆ name {								\
	ರಚನೆ type *sqh_ಮೊದಲ;	/* first element */			\
	ರಚನೆ type **sqh_ಕೊನೆಯ;	/* addr of last next element */		\
}

#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_ತಲೆ_INITIALIZER(ತಲೆ)					\
	{ ಖಾಲಿ, &(ತಲೆ).sqh_ಮೊದಲ }

#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_ನಮೂದು(type)						\
ರಚನೆ {								\
	ರಚನೆ type *sqe_ಮುಂದಿನ;	/* next element */			\
}

/*
 * Simple queue access methods.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_ಮೊದಲ(ತಲೆ)		((ತಲೆ)->sqh_ಮೊದಲ)
#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_ಕೊನೆ(ತಲೆ)		ಖಾಲಿ
#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_ಖಾಲಿಗಳು(ತಲೆ)		((ತಲೆ)->sqh_ಮೊದಲ == SIMPLEQ_ಕೊನೆ(ತಲೆ))
#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_ಮುಂದಿನ(elm, ಕ್ಷೇತ್ರ)	((elm)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ)

#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_FOREACH(var, ತಲೆ, ಕ್ಷೇತ್ರ)				\
	ಗಾಗಿ ((var) = ((ತಲೆ)->sqh_ಮೊದಲ);				\
	    (var) != SIMPLEQ_ಕೊನೆ(ತಲೆ);					\
	    (var) = ((var)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ))

#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_FOREACH_ಸುರಕ್ಷಿತ(var, ತಲೆ, ಕ್ಷೇತ್ರ, ಮುಂದಿನ)			\
	ಗಾಗಿ ((var) = ((ತಲೆ)->sqh_ಮೊದಲ);				\
	    (var) != SIMPLEQ_ಕೊನೆ(ತಲೆ) &&				\
	    ((ಮುಂದಿನ = ((var)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ)), 1);			\
	    (var) = (ಮುಂದಿನ))

/*
 * Simple queue functions.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_INIT(ತಲೆ) ಮಾಡು {						\
	(ತಲೆ)->sqh_ಮೊದಲ = ಖಾಲಿ;					\
	(ತಲೆ)->sqh_ಕೊನೆಯ = &(ತಲೆ)->sqh_ಮೊದಲ;				\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_ಸೇರಿಸು_ತಲೆ(ತಲೆ, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	ಆದರೆ (((elm)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ = (ತಲೆ)->sqh_ಮೊದಲ) == ಖಾಲಿ)	\
		(ತಲೆ)->sqh_ಕೊನೆಯ = &(elm)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ;		\
	(ತಲೆ)->sqh_ಮೊದಲ = (elm);					\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_ಸೇರಿಸು_ಬಾಲ(ತಲೆ, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	(elm)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ = ಖಾಲಿ;					\
	*(ತಲೆ)->sqh_ಕೊನೆಯ = (elm);					\
	(ತಲೆ)->sqh_ಕೊನೆಯ = &(elm)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ;			\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_ಸೇರಿಸು_AFTER(ತಲೆ, listelm, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {		\
	ಆದರೆ (((elm)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ = (listelm)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ) == ಖಾಲಿ)\
		(ತಲೆ)->sqh_ಕೊನೆಯ = &(elm)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ;		\
	(listelm)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ = (elm);				\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_REMOVE_ತಲೆ(ತಲೆ, ಕ್ಷೇತ್ರ) ಮಾಡು {				\
	ಆದರೆ (((ತಲೆ)->sqh_ಮೊದಲ = (ತಲೆ)->sqh_ಮೊದಲ->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ) == ಖಾಲಿ) \
		(ತಲೆ)->sqh_ಕೊನೆಯ = &(ತಲೆ)->sqh_ಮೊದಲ;			\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು SIMPLEQ_REMOVE_AFTER(ತಲೆ, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	ಆದರೆ (((elm)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ = (elm)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ) \
	    == ಖಾಲಿ)							\
		(ತಲೆ)->sqh_ಕೊನೆಯ = &(elm)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ;		\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_REMOVE(ತಲೆ, elm, type, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	ಆದರೆ ((ತಲೆ)->sqh_ಮೊದಲ == (elm)) {				\
		SIMPLEQ_REMOVE_ತಲೆ((ತಲೆ), ಕ್ಷೇತ್ರ);			\
	} ಇಲ್ಲದಿದ್ದರೆ {							\
		ರಚನೆ type *curelm = (ತಲೆ)->sqh_ಮೊದಲ;		\
		ಆಗ (curelm->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ != (elm))			\
			curelm = curelm->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ;		\
		ಆದರೆ ((curelm->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ =				\
			curelm->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ) == ಖಾಲಿ) \
			    (ತಲೆ)->sqh_ಕೊನೆಯ = &(curelm)->ಕ್ಷೇತ್ರ.sqe_ಮುಂದಿನ; \
	}								\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_CONCAT(head1, head2) ಮಾಡು {				\
	ಆದರೆ (!SIMPLEQ_ಖಾಲಿಗಳು((head2))) {					\
		*(head1)->sqh_ಕೊನೆಯ = (head2)->sqh_ಮೊದಲ;		\
		(head1)->sqh_ಕೊನೆಯ = (head2)->sqh_ಕೊನೆಯ;		\
		SIMPLEQ_INIT((head2));					\
	}								\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	SIMPLEQ_ಕೊನೆಯ(ತಲೆ, type, ಕ್ಷೇತ್ರ)					\
	(SIMPLEQ_ಖಾಲಿಗಳು((ತಲೆ)) ?						\
		ಖಾಲಿ :							\
	        ((ರಚನೆ type *)(ಶೂನ್ಯ *)				\
		((ಅಕ್ಷರ *)((ತಲೆ)->sqh_ಕೊನೆಯ) - offsetof(ರಚನೆ type, ಕ್ಷೇತ್ರ))))

/*
 * Tail queue definitions.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_ತಲೆ_2(name, type, qual)					\
ರಚನೆ name {								\
	qual type *tqh_ಮೊದಲ;		/* first element */		\
	qual type *qual *tqh_ಕೊನೆಯ;	/* addr of last next element */	\
}
#ವ್ಯಾಖ್ಯಾನಿಸು TAILQ_ತಲೆ(name, type)	TAILQ_ತಲೆ_2(name, ರಚನೆ type,)

#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_ತಲೆ_INITIALIZER(ತಲೆ)					\
	{ TAILQ_ಕೊನೆ(ತಲೆ), &(ತಲೆ).tqh_ಮೊದಲ }

#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_ನಮೂದು_2(type, qual)					\
ರಚನೆ {								\
	qual type *tqe_ಮುಂದಿನ;		/* next element */		\
	qual type *qual *tqe_prev;	/* address of previous next element */\
}
#ವ್ಯಾಖ್ಯಾನಿಸು TAILQ_ನಮೂದು(type)	TAILQ_ನಮೂದು_2(ರಚನೆ type,)

/*
 * Tail queue access methods.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_ಮೊದಲ(ತಲೆ)		((ತಲೆ)->tqh_ಮೊದಲ)
#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_ಕೊನೆ(ತಲೆ)			(ಖಾಲಿ)
#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_ಮುಂದಿನ(elm, ಕ್ಷೇತ್ರ)		((elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ)
#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_ಕೊನೆಯ(ತಲೆ, headname) \
	(*(((ರಚನೆ headname *)((ತಲೆ)->tqh_ಕೊನೆಯ))->tqh_ಕೊನೆಯ))
#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_PREV(elm, headname, ಕ್ಷೇತ್ರ) \
	(*(((ರಚನೆ headname *)((elm)->ಕ್ಷೇತ್ರ.tqe_prev))->tqh_ಕೊನೆಯ))
#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_ಖಾಲಿಗಳು(ತಲೆ)		(TAILQ_ಮೊದಲ(ತಲೆ) == TAILQ_ಕೊನೆ(ತಲೆ))


#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_FOREACH(var, ತಲೆ, ಕ್ಷೇತ್ರ)					\
	ಗಾಗಿ ((var) = ((ತಲೆ)->tqh_ಮೊದಲ);				\
	    (var) != TAILQ_ಕೊನೆ(ತಲೆ);					\
	    (var) = ((var)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ))

#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_FOREACH_ಸುರಕ್ಷಿತ(var, ತಲೆ, ಕ್ಷೇತ್ರ, ಮುಂದಿನ)			\
	ಗಾಗಿ ((var) = ((ತಲೆ)->tqh_ಮೊದಲ);				\
	    (var) != TAILQ_ಕೊನೆ(ತಲೆ) &&					\
	    ((ಮುಂದಿನ) = TAILQ_ಮುಂದಿನ(var, ಕ್ಷೇತ್ರ), 1); (var) = (ಮುಂದಿನ))

#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_FOREACH_ಹಿಮ್ಮುಖ(var, ತಲೆ, headname, ಕ್ಷೇತ್ರ)		\
	ಗಾಗಿ ((var) = (*(((ರಚನೆ headname *)((ತಲೆ)->tqh_ಕೊನೆಯ))->tqh_ಕೊನೆಯ));\
	    (var) != TAILQ_ಕೊನೆ(ತಲೆ);					\
	    (var) = (*(((ರಚನೆ headname *)((var)->ಕ್ಷೇತ್ರ.tqe_prev))->tqh_ಕೊನೆಯ)))

#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_FOREACH_ಹಿಮ್ಮುಖ_ಸುರಕ್ಷಿತ(var, ತಲೆ, headname, ಕ್ಷೇತ್ರ, prev)	\
	ಗಾಗಿ ((var) = TAILQ_ಕೊನೆಯ((ತಲೆ), headname);			\
	    (var) != TAILQ_ಕೊನೆ(ತಲೆ) && 				\
	    ((prev) = TAILQ_PREV((var), headname, ಕ್ಷೇತ್ರ), 1); (var) = (prev))

/*
 * Tail queue functions.
 */
#ಆದರೆ defined(QUEUEDEBUG)
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_TAILQ_ಸೇರಿಸು_ತಲೆ(ತಲೆ, elm, ಕ್ಷೇತ್ರ)			\
	ಆದರೆ ((ತಲೆ)->tqh_ಮೊದಲ &&					\
	    (ತಲೆ)->tqh_ಮೊದಲ->ಕ್ಷೇತ್ರ.tqe_prev != &(ತಲೆ)->tqh_ಮೊದಲ)	\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_HEAD %p %s:%d", (ತಲೆ),	\
		    __FILE__, __LINE__);
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_TAILQ_ಸೇರಿಸು_ಬಾಲ(ತಲೆ, elm, ಕ್ಷೇತ್ರ)			\
	ಆದರೆ (*(ತಲೆ)->tqh_ಕೊನೆಯ != ಖಾಲಿ)					\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_TAIL %p %s:%d", (ತಲೆ),	\
		    __FILE__, __LINE__);
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_TAILQ_OP(elm, ಕ್ಷೇತ್ರ)					\
	ಆದರೆ ((elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ &&					\
	    (elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.tqe_prev !=			\
	    &(elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ)					\
		QUEUEDEBUG_ABORT("TAILQ_* forw %p %s:%d", (elm),	\
		    __FILE__, __LINE__);				\
	ಆದರೆ (*(elm)->ಕ್ಷೇತ್ರ.tqe_prev != (elm))				\
		QUEUEDEBUG_ABORT("TAILQ_* back %p %s:%d", (elm),	\
		    __FILE__, __LINE__);
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_TAILQ_PREREMOVE(ತಲೆ, elm, ಕ್ಷೇತ್ರ)			\
	ಆದರೆ ((elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ == ಖಾಲಿ &&				\
	    (ತಲೆ)->tqh_ಕೊನೆಯ != &(elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ)			\
		QUEUEDEBUG_ABORT("TAILQ_PREREMOVE head %p elm %p %s:%d",\
		    (ತಲೆ), (elm), __FILE__, __LINE__);
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, ಕ್ಷೇತ್ರ)				\
	(elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ = (ಶೂನ್ಯ *)1L;				\
	(elm)->ಕ್ಷೇತ್ರ.tqe_prev = (ಶೂನ್ಯ *)1L;
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_TAILQ_ಸೇರಿಸು_ತಲೆ(ತಲೆ, elm, ಕ್ಷೇತ್ರ)
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_TAILQ_ಸೇರಿಸು_ಬಾಲ(ತಲೆ, elm, ಕ್ಷೇತ್ರ)
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_TAILQ_OP(elm, ಕ್ಷೇತ್ರ)
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_TAILQ_PREREMOVE(ತಲೆ, elm, ಕ್ಷೇತ್ರ)
#ವ್ಯಾಖ್ಯಾನಿಸು	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, ಕ್ಷೇತ್ರ)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_INIT(ತಲೆ) ಮಾಡು {						\
	(ತಲೆ)->tqh_ಮೊದಲ = TAILQ_ಕೊನೆ(ತಲೆ);				\
	(ತಲೆ)->tqh_ಕೊನೆಯ = &(ತಲೆ)->tqh_ಮೊದಲ;				\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_ಸೇರಿಸು_ತಲೆ(ತಲೆ, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	QUEUEDEBUG_TAILQ_ಸೇರಿಸು_ತಲೆ((ತಲೆ), (elm), ಕ್ಷೇತ್ರ)		\
	ಆದರೆ (((elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ = (ತಲೆ)->tqh_ಮೊದಲ) != TAILQ_ಕೊನೆ(ತಲೆ))\
		(ತಲೆ)->tqh_ಮೊದಲ->ಕ್ಷೇತ್ರ.tqe_prev =			\
		    &(elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ;				\
	ಇಲ್ಲದಿದ್ದರೆ								\
		(ತಲೆ)->tqh_ಕೊನೆಯ = &(elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ;		\
	(ತಲೆ)->tqh_ಮೊದಲ = (elm);					\
	(elm)->ಕ್ಷೇತ್ರ.tqe_prev = &(ತಲೆ)->tqh_ಮೊದಲ;			\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_ಸೇರಿಸು_ಬಾಲ(ತಲೆ, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	QUEUEDEBUG_TAILQ_ಸೇರಿಸು_ಬಾಲ((ತಲೆ), (elm), ಕ್ಷೇತ್ರ)		\
	(elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ = TAILQ_ಕೊನೆ(ತಲೆ);			\
	(elm)->ಕ್ಷೇತ್ರ.tqe_prev = (ತಲೆ)->tqh_ಕೊನೆಯ;			\
	*(ತಲೆ)->tqh_ಕೊನೆಯ = (elm);					\
	(ತಲೆ)->tqh_ಕೊನೆಯ = &(elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ;			\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_ಸೇರಿಸು_AFTER(ತಲೆ, listelm, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {		\
	QUEUEDEBUG_TAILQ_OP((listelm), ಕ್ಷೇತ್ರ)				\
	ಆದರೆ (((elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ = (listelm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ) != 	\
	    TAILQ_ಕೊನೆ(ತಲೆ))						\
		(elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.tqe_prev = 		\
		    &(elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ;				\
	ಇಲ್ಲದಿದ್ದರೆ								\
		(ತಲೆ)->tqh_ಕೊನೆಯ = &(elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ;		\
	(listelm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ = (elm);				\
	(elm)->ಕ್ಷೇತ್ರ.tqe_prev = &(listelm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ;		\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_ಸೇರಿಸು_BEFORE(listelm, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	QUEUEDEBUG_TAILQ_OP((listelm), ಕ್ಷೇತ್ರ)				\
	(elm)->ಕ್ಷೇತ್ರ.tqe_prev = (listelm)->ಕ್ಷೇತ್ರ.tqe_prev;		\
	(elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ = (listelm);				\
	*(listelm)->ಕ್ಷೇತ್ರ.tqe_prev = (elm);				\
	(listelm)->ಕ್ಷೇತ್ರ.tqe_prev = &(elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ;		\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_REMOVE(ತಲೆ, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {				\
	QUEUEDEBUG_TAILQ_PREREMOVE((ತಲೆ), (elm), ಕ್ಷೇತ್ರ)		\
	QUEUEDEBUG_TAILQ_OP((elm), ಕ್ಷೇತ್ರ)				\
	ಆದರೆ (((elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ) != TAILQ_ಕೊನೆ(ತಲೆ))			\
		(elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.tqe_prev = 		\
		    (elm)->ಕ್ಷೇತ್ರ.tqe_prev;				\
	ಇಲ್ಲದಿದ್ದರೆ								\
		(ತಲೆ)->tqh_ಕೊನೆಯ = (elm)->ಕ್ಷೇತ್ರ.tqe_prev;		\
	*(elm)->ಕ್ಷೇತ್ರ.tqe_prev = (elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ;			\
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), ಕ್ಷೇತ್ರ);			\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು TAILQ_REPLACE(ತಲೆ, elm, elm2, ಕ್ಷೇತ್ರ) ಮಾಡು {                      \
        ಆದರೆ (((elm2)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ = (elm)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ) != 	\
	    TAILQ_ಕೊನೆ(ತಲೆ))   						\
                (elm2)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.tqe_prev =                \
                    &(elm2)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ;                            \
        ಇಲ್ಲದಿದ್ದರೆ                                                            \
                (ತಲೆ)->tqh_ಕೊನೆಯ = &(elm2)->ಕ್ಷೇತ್ರ.tqe_ಮುಂದಿನ;             \
        (elm2)->ಕ್ಷೇತ್ರ.tqe_prev = (elm)->ಕ್ಷೇತ್ರ.tqe_prev;                 \
        *(elm2)->ಕ್ಷೇತ್ರ.tqe_prev = (elm2);                               \
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), ಕ್ಷೇತ್ರ);			\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	TAILQ_CONCAT(head1, head2, ಕ್ಷೇತ್ರ) ಮಾಡು {				\
	ಆದರೆ (!TAILQ_ಖಾಲಿಗಳು(head2)) {					\
		*(head1)->tqh_ಕೊನೆಯ = (head2)->tqh_ಮೊದಲ;		\
		(head2)->tqh_ಮೊದಲ->ಕ್ಷೇತ್ರ.tqe_prev = (head1)->tqh_ಕೊನೆಯ;	\
		(head1)->tqh_ಕೊನೆಯ = (head2)->tqh_ಕೊನೆಯ;			\
		TAILQ_INIT((head2));					\
	}								\
} ಆಗ (/*CONSTCOND*/0)

/*
 * Singly-linked Tail queue declarations.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_ತಲೆ(name, type)						\
ರಚನೆ name {								\
	ರಚನೆ type *stqh_ಮೊದಲ;	/* first element */		\
	ರಚನೆ type **stqh_ಕೊನೆಯ;	/* addr of last next element */	\
}

#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_ತಲೆ_INITIALIZER(ತಲೆ)					\
	{ ಖಾಲಿ, &(ತಲೆ).stqh_ಮೊದಲ }

#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_ನಮೂದು(type)						\
ರಚನೆ {								\
	ರಚನೆ type *stqe_ಮುಂದಿನ;	/* next element */			\
}

/*
 * Singly-linked Tail queue access methods.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_ಮೊದಲ(ತಲೆ)	((ತಲೆ)->stqh_ಮೊದಲ)
#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_ಕೊನೆ(ತಲೆ)	ಖಾಲಿ
#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_ಮುಂದಿನ(elm, ಕ್ಷೇತ್ರ)	((elm)->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ)
#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_ಖಾಲಿಗಳು(ತಲೆ)	(STAILQ_ಮೊದಲ(ತಲೆ) == STAILQ_ಕೊನೆ(ತಲೆ))

/*
 * Singly-linked Tail queue functions.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_INIT(ತಲೆ) ಮಾಡು {						\
	(ತಲೆ)->stqh_ಮೊದಲ = ಖಾಲಿ;					\
	(ತಲೆ)->stqh_ಕೊನೆಯ = &(ತಲೆ)->stqh_ಮೊದಲ;				\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_ಸೇರಿಸು_ತಲೆ(ತಲೆ, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	ಆದರೆ (((elm)->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ = (ತಲೆ)->stqh_ಮೊದಲ) == ಖಾಲಿ)	\
		(ತಲೆ)->stqh_ಕೊನೆಯ = &(elm)->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ;		\
	(ತಲೆ)->stqh_ಮೊದಲ = (elm);					\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_ಸೇರಿಸು_ಬಾಲ(ತಲೆ, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	(elm)->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ = ಖಾಲಿ;					\
	*(ತಲೆ)->stqh_ಕೊನೆಯ = (elm);					\
	(ತಲೆ)->stqh_ಕೊನೆಯ = &(elm)->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ;			\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_ಸೇರಿಸು_AFTER(ತಲೆ, listelm, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {		\
	ಆದರೆ (((elm)->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ = (listelm)->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ) == ಖಾಲಿ)\
		(ತಲೆ)->stqh_ಕೊನೆಯ = &(elm)->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ;		\
	(listelm)->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ = (elm);				\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_REMOVE_ತಲೆ(ತಲೆ, ಕ್ಷೇತ್ರ) ಮಾಡು {				\
	ಆದರೆ (((ತಲೆ)->stqh_ಮೊದಲ = (ತಲೆ)->stqh_ಮೊದಲ->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ) == ಖಾಲಿ) \
		(ತಲೆ)->stqh_ಕೊನೆಯ = &(ತಲೆ)->stqh_ಮೊದಲ;			\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_REMOVE(ತಲೆ, elm, type, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	ಆದರೆ ((ತಲೆ)->stqh_ಮೊದಲ == (elm)) {				\
		STAILQ_REMOVE_ತಲೆ((ತಲೆ), ಕ್ಷೇತ್ರ);			\
	} ಇಲ್ಲದಿದ್ದರೆ {							\
		ರಚನೆ type *curelm = (ತಲೆ)->stqh_ಮೊದಲ;		\
		ಆಗ (curelm->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ != (elm))			\
			curelm = curelm->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ;		\
		ಆದರೆ ((curelm->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ =				\
			curelm->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ) == ಖಾಲಿ) \
			    (ತಲೆ)->stqh_ಕೊನೆಯ = &(curelm)->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ; \
	}								\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_FOREACH(var, ತಲೆ, ಕ್ಷೇತ್ರ)				\
	ಗಾಗಿ ((var) = ((ತಲೆ)->stqh_ಮೊದಲ);				\
		(var);							\
		(var) = ((var)->ಕ್ಷೇತ್ರ.stqe_ಮುಂದಿನ))

#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_FOREACH_ಸುರಕ್ಷಿತ(var, ತಲೆ, ಕ್ಷೇತ್ರ, tvar)			\
	ಗಾಗಿ ((var) = STAILQ_ಮೊದಲ((ತಲೆ));				\
	    (var) && ((tvar) = STAILQ_ಮುಂದಿನ((var), ಕ್ಷೇತ್ರ), 1);		\
	    (var) = (tvar))

#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_CONCAT(head1, head2) ಮಾಡು {				\
	ಆದರೆ (!STAILQ_ಖಾಲಿಗಳು((head2))) {					\
		*(head1)->stqh_ಕೊನೆಯ = (head2)->stqh_ಮೊದಲ;		\
		(head1)->stqh_ಕೊನೆಯ = (head2)->stqh_ಕೊನೆಯ;		\
		STAILQ_INIT((head2));					\
	}								\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	STAILQ_ಕೊನೆಯ(ತಲೆ, type, ಕ್ಷೇತ್ರ)					\
	(STAILQ_ಖಾಲಿಗಳು((ತಲೆ)) ?						\
		ಖಾಲಿ :							\
	        ((ರಚನೆ type *)(ಶೂನ್ಯ *)				\
		((ಅಕ್ಷರ *)((ತಲೆ)->stqh_ಕೊನೆಯ) - offsetof(ರಚನೆ type, ಕ್ಷೇತ್ರ))))


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _KERNEL
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
ಸ್ಥಿರ __inline ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ * __launder_type(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *);
ಸ್ಥಿರ __inline ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *
__launder_type(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *__x)
{
	__asm __volatile("" : "+r" (__x));
	ಹಿಂತಿರುಗಿ __x;
}

#ಆದರೆ defined(QUEUEDEBUG)
#ವ್ಯಾಖ್ಯಾನಿಸು QUEUEDEBUG_CIRCLEQ_ತಲೆ(ತಲೆ, ಕ್ಷೇತ್ರ)				\
	ಆದರೆ ((ತಲೆ)->cqh_ಮೊದಲ != CIRCLEQ_ENDC(ತಲೆ) &&			\
	    (ತಲೆ)->cqh_ಮೊದಲ->ಕ್ಷೇತ್ರ.cqe_prev != CIRCLEQ_ENDC(ತಲೆ))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head forw %p %s:%d", (ತಲೆ),	\
		      __FILE__, __LINE__);				\
	ಆದರೆ ((ತಲೆ)->cqh_ಕೊನೆಯ != CIRCLEQ_ENDC(ತಲೆ) &&			\
	    (ತಲೆ)->cqh_ಕೊನೆಯ->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ != CIRCLEQ_ENDC(ತಲೆ))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head back %p %s:%d", (ತಲೆ),	\
		      __FILE__, __LINE__);
#ವ್ಯಾಖ್ಯಾನಿಸು QUEUEDEBUG_CIRCLEQ_ELM(ತಲೆ, elm, ಕ್ಷೇತ್ರ)			\
	ಆದರೆ ((elm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ == CIRCLEQ_ENDC(ತಲೆ)) {		\
		ಆದರೆ ((ತಲೆ)->cqh_ಕೊನೆಯ != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm last %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} ಇಲ್ಲದಿದ್ದರೆ {							\
		ಆದರೆ ((elm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.cqe_prev != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm forw %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}								\
	ಆದರೆ ((elm)->ಕ್ಷೇತ್ರ.cqe_prev == CIRCLEQ_ENDC(ತಲೆ)) {		\
		ಆದರೆ ((ತಲೆ)->cqh_ಮೊದಲ != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm first %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} ಇಲ್ಲದಿದ್ದರೆ {							\
		ಆದರೆ ((elm)->ಕ್ಷೇತ್ರ.cqe_prev->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm prev %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}
#ವ್ಯಾಖ್ಯಾನಿಸು QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, ಕ್ಷೇತ್ರ)			\
	(elm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ = (ಶೂನ್ಯ *)1L;				\
	(elm)->ಕ್ಷೇತ್ರ.cqe_prev = (ಶೂನ್ಯ *)1L;
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು QUEUEDEBUG_CIRCLEQ_ತಲೆ(ತಲೆ, ಕ್ಷೇತ್ರ)
#ವ್ಯಾಖ್ಯಾನಿಸು QUEUEDEBUG_CIRCLEQ_ELM(ತಲೆ, elm, ಕ್ಷೇತ್ರ)
#ವ್ಯಾಖ್ಯಾನಿಸು QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, ಕ್ಷೇತ್ರ)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_ತಲೆ(name, type)					\
ರಚನೆ name {								\
	ರಚನೆ type *cqh_ಮೊದಲ;		/* first element */		\
	ರಚನೆ type *cqh_ಕೊನೆಯ;		/* last element */		\
}

#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_ತಲೆ_INITIALIZER(ತಲೆ)					\
	{ CIRCLEQ_ಕೊನೆ(&ತಲೆ), CIRCLEQ_ಕೊನೆ(&ತಲೆ) }

#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_ನಮೂದು(type)						\
ರಚನೆ {								\
	ರಚನೆ type *cqe_ಮುಂದಿನ;		/* next element */		\
	ರಚನೆ type *cqe_prev;		/* previous element */		\
}

/*
 * Circular queue functions.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_INIT(ತಲೆ) ಮಾಡು {						\
	(ತಲೆ)->cqh_ಮೊದಲ = CIRCLEQ_ಕೊನೆ(ತಲೆ);				\
	(ತಲೆ)->cqh_ಕೊನೆಯ = CIRCLEQ_ಕೊನೆ(ತಲೆ);				\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_ಸೇರಿಸು_AFTER(ತಲೆ, listelm, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {		\
	QUEUEDEBUG_CIRCLEQ_ತಲೆ((ತಲೆ), ಕ್ಷೇತ್ರ)				\
	QUEUEDEBUG_CIRCLEQ_ELM((ತಲೆ), (listelm), ಕ್ಷೇತ್ರ)		\
	(elm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ = (listelm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ;		\
	(elm)->ಕ್ಷೇತ್ರ.cqe_prev = (listelm);				\
	ಆದರೆ ((listelm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ == CIRCLEQ_ENDC(ತಲೆ))		\
		(ತಲೆ)->cqh_ಕೊನೆಯ = (elm);				\
	ಇಲ್ಲದಿದ್ದರೆ								\
		(listelm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.cqe_prev = (elm);	\
	(listelm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ = (elm);				\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_ಸೇರಿಸು_BEFORE(ತಲೆ, listelm, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {		\
	QUEUEDEBUG_CIRCLEQ_ತಲೆ((ತಲೆ), ಕ್ಷೇತ್ರ)				\
	QUEUEDEBUG_CIRCLEQ_ELM((ತಲೆ), (listelm), ಕ್ಷೇತ್ರ)		\
	(elm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ = (listelm);				\
	(elm)->ಕ್ಷೇತ್ರ.cqe_prev = (listelm)->ಕ್ಷೇತ್ರ.cqe_prev;		\
	ಆದರೆ ((listelm)->ಕ್ಷೇತ್ರ.cqe_prev == CIRCLEQ_ENDC(ತಲೆ))		\
		(ತಲೆ)->cqh_ಮೊದಲ = (elm);				\
	ಇಲ್ಲದಿದ್ದರೆ								\
		(listelm)->ಕ್ಷೇತ್ರ.cqe_prev->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ = (elm);	\
	(listelm)->ಕ್ಷೇತ್ರ.cqe_prev = (elm);				\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_ಸೇರಿಸು_ತಲೆ(ತಲೆ, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	QUEUEDEBUG_CIRCLEQ_ತಲೆ((ತಲೆ), ಕ್ಷೇತ್ರ)				\
	(elm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ = (ತಲೆ)->cqh_ಮೊದಲ;			\
	(elm)->ಕ್ಷೇತ್ರ.cqe_prev = CIRCLEQ_ಕೊನೆ(ತಲೆ);			\
	ಆದರೆ ((ತಲೆ)->cqh_ಕೊನೆಯ == CIRCLEQ_ENDC(ತಲೆ))			\
		(ತಲೆ)->cqh_ಕೊನೆಯ = (elm);				\
	ಇಲ್ಲದಿದ್ದರೆ								\
		(ತಲೆ)->cqh_ಮೊದಲ->ಕ್ಷೇತ್ರ.cqe_prev = (elm);		\
	(ತಲೆ)->cqh_ಮೊದಲ = (elm);					\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_ಸೇರಿಸು_ಬಾಲ(ತಲೆ, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {			\
	QUEUEDEBUG_CIRCLEQ_ತಲೆ((ತಲೆ), ಕ್ಷೇತ್ರ)				\
	(elm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ = CIRCLEQ_ಕೊನೆ(ತಲೆ);			\
	(elm)->ಕ್ಷೇತ್ರ.cqe_prev = (ತಲೆ)->cqh_ಕೊನೆಯ;			\
	ಆದರೆ ((ತಲೆ)->cqh_ಮೊದಲ == CIRCLEQ_ENDC(ತಲೆ))			\
		(ತಲೆ)->cqh_ಮೊದಲ = (elm);				\
	ಇಲ್ಲದಿದ್ದರೆ								\
		(ತಲೆ)->cqh_ಕೊನೆಯ->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ = (elm);		\
	(ತಲೆ)->cqh_ಕೊನೆಯ = (elm);					\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_REMOVE(ತಲೆ, elm, ಕ್ಷೇತ್ರ) ಮಾಡು {				\
	QUEUEDEBUG_CIRCLEQ_ತಲೆ((ತಲೆ), ಕ್ಷೇತ್ರ)				\
	QUEUEDEBUG_CIRCLEQ_ELM((ತಲೆ), (elm), ಕ್ಷೇತ್ರ)			\
	ಆದರೆ ((elm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ == CIRCLEQ_ENDC(ತಲೆ))		\
		(ತಲೆ)->cqh_ಕೊನೆಯ = (elm)->ಕ್ಷೇತ್ರ.cqe_prev;		\
	ಇಲ್ಲದಿದ್ದರೆ								\
		(elm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ->ಕ್ಷೇತ್ರ.cqe_prev =			\
		    (elm)->ಕ್ಷೇತ್ರ.cqe_prev;				\
	ಆದರೆ ((elm)->ಕ್ಷೇತ್ರ.cqe_prev == CIRCLEQ_ENDC(ತಲೆ))		\
		(ತಲೆ)->cqh_ಮೊದಲ = (elm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ;		\
	ಇಲ್ಲದಿದ್ದರೆ								\
		(elm)->ಕ್ಷೇತ್ರ.cqe_prev->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ =			\
		    (elm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ;				\
	QUEUEDEBUG_CIRCLEQ_POSTREMOVE((elm), ಕ್ಷೇತ್ರ)			\
} ಆಗ (/*CONSTCOND*/0)

#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_FOREACH(var, ತಲೆ, ಕ್ಷೇತ್ರ)				\
	ಗಾಗಿ ((var) = ((ತಲೆ)->cqh_ಮೊದಲ);				\
		(var) != CIRCLEQ_ENDC(ತಲೆ);				\
		(var) = ((var)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ))

#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_FOREACH_ಹಿಮ್ಮುಖ(var, ತಲೆ, ಕ್ಷೇತ್ರ)			\
	ಗಾಗಿ ((var) = ((ತಲೆ)->cqh_ಕೊನೆಯ);				\
		(var) != CIRCLEQ_ENDC(ತಲೆ);				\
		(var) = ((var)->ಕ್ಷೇತ್ರ.cqe_prev))

/*
 * Circular queue access methods.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_ಮೊದಲ(ತಲೆ)		((ತಲೆ)->cqh_ಮೊದಲ)
#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_ಕೊನೆಯ(ತಲೆ)		((ತಲೆ)->cqh_ಕೊನೆಯ)
/* For comparisons */
#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_ENDC(ತಲೆ)		(__launder_type(ತಲೆ))
/* For assignments */
#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_ಕೊನೆ(ತಲೆ)		((ಶೂನ್ಯ *)(ತಲೆ))
#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_ಮುಂದಿನ(elm, ಕ್ಷೇತ್ರ)	((elm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ)
#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_PREV(elm, ಕ್ಷೇತ್ರ)	((elm)->ಕ್ಷೇತ್ರ.cqe_prev)
#ವ್ಯಾಖ್ಯಾನಿಸು	CIRCLEQ_ಖಾಲಿಗಳು(ತಲೆ)						\
    (CIRCLEQ_ಮೊದಲ(ತಲೆ) == CIRCLEQ_ENDC(ತಲೆ))

#ವ್ಯಾಖ್ಯಾನಿಸು CIRCLEQ_LOOP_ಮುಂದಿನ(ತಲೆ, elm, ಕ್ಷೇತ್ರ)				\
	(((elm)->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ == CIRCLEQ_ENDC(ತಲೆ))			\
	    ? ((ತಲೆ)->cqh_ಮೊದಲ)					\
	    : (elm->ಕ್ಷೇತ್ರ.cqe_ಮುಂದಿನ))
#ವ್ಯಾಖ್ಯಾನಿಸು CIRCLEQ_LOOP_PREV(ತಲೆ, elm, ಕ್ಷೇತ್ರ)				\
	(((elm)->ಕ್ಷೇತ್ರ.cqe_prev == CIRCLEQ_ENDC(ತಲೆ))			\
	    ? ((ತಲೆ)->cqh_ಕೊನೆಯ)					\
	    : (elm->ಕ್ಷೇತ್ರ.cqe_prev))
#ಆದರೆ_ಅಂತ್ಯ /* !_KERNEL */

#ಆದರೆ_ಅಂತ್ಯ	/* !_SYS_QUEUE_H_ */
