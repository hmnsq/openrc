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

#నిర్వచితం_కాకపోతే	SYS_వరుస_H
#నిర్వచించు	SYS_వరుస_H

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
#నిర్వచితమైతే __NetBSD__
#అన్తర్భావ <sys/null.h>
#అయితే_ముగింపు

#ఒకవేళ defined(QUEUEDEBUG)
# ఒకవేళ defined(_KERNEL)
#  define QUEUEDEBUG_ABORT(...) panic(__VA_ARGS__)
# లేకపోతే
#  include <err.h>
#  define QUEUEDEBUG_ABORT(...) err(1, __VA_ARGS__)
# endif
#అయితే_ముగింపు

/*
 * Singly-linked List definitions.
 */
#నిర్వచించు	SLIST_తల(name, type)						\
నిర్మాణం name {								\
	నిర్మాణం type *slh_మొదటి;	/* first element */			\
}

#నిర్వచించు	SLIST_తల_INITIALIZER(తల)					\
	{ ఖాళీ }

#నిర్వచించు	SLIST_నమోదు(type)						\
నిర్మాణం {								\
	నిర్మాణం type *sle_తదుపరి;	/* next element */			\
}

/*
 * Singly-linked List access methods.
 */
#నిర్వచించు	SLIST_మొదటి(తల)	((తల)->slh_మొదటి)
#నిర్వచించు	SLIST_చివర(తల)		ఖాళీ
#నిర్వచించు	SLIST_ఖాళీలు(తల)	((తల)->slh_మొదటి == ఖాళీ)
#నిర్వచించు	SLIST_తదుపరి(elm, క్షేత్రం)	((elm)->క్షేత్రం.sle_తదుపరి)

#నిర్వచించు	SLIST_FOREACH(var, తల, క్షేత్రం)					\
	కొరకు((var) = (తల)->slh_మొదటి;					\
	    (var) != SLIST_చివర(తల);					\
	    (var) = (var)->క్షేత్రం.sle_తదుపరి)

#నిర్వచించు	SLIST_FOREACH_సురక్షిత(var, తల, క్షేత్రం, tvar)			\
	కొరకు ((var) = SLIST_మొదటి((తల));				\
	    (var) != SLIST_చివర(తల) &&					\
	    ((tvar) = SLIST_తదుపరి((var), క్షేత్రం), 1);			\
	    (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#నిర్వచించు	SLIST_INIT(తల) చేయి {						\
	(తల)->slh_మొదటి = SLIST_చివర(తల);				\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	SLIST_చొప్పించు_AFTER(slistelm, elm, క్షేత్రం) చేయి {			\
	(elm)->క్షేత్రం.sle_తదుపరి = (slistelm)->క్షేత్రం.sle_తదుపరి;		\
	(slistelm)->క్షేత్రం.sle_తదుపరి = (elm);				\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	SLIST_చొప్పించు_తల(తల, elm, క్షేత్రం) చేయి {			\
	(elm)->క్షేత్రం.sle_తదుపరి = (తల)->slh_మొదటి;			\
	(తల)->slh_మొదటి = (elm);					\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	SLIST_REMOVE_AFTER(slistelm, క్షేత్రం) చేయి {			\
	(slistelm)->క్షేత్రం.sle_తదుపరి =					\
	    SLIST_తదుపరి(SLIST_తదుపరి((slistelm), క్షేత్రం), క్షేత్రం);		\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	SLIST_REMOVE_తల(తల, క్షేత్రం) చేయి {				\
	(తల)->slh_మొదటి = (తల)->slh_మొదటి->క్షేత్రం.sle_తదుపరి;		\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	SLIST_REMOVE(తల, elm, type, క్షేత్రం) చేయి {			\
	ఒకవేళ ((తల)->slh_మొదటి == (elm)) {				\
		SLIST_REMOVE_తల((తల), క్షేత్రం);			\
	}								\
	లేకపోతే {								\
		నిర్మాణం type *curelm = (తల)->slh_మొదటి;		\
		అయితే(curelm->క్షేత్రం.sle_తదుపరి != (elm))			\
			curelm = curelm->క్షేత్రం.sle_తదుపరి;		\
		curelm->క్షేత్రం.sle_తదుపరి =				\
		    curelm->క్షేత్రం.sle_తదుపరి->క్షేత్రం.sle_తదుపరి;		\
	}								\
} అయితే (/*CONSTCOND*/0)


/*
 * List definitions.
 */
#నిర్వచించు	LIST_తల(name, type)						\
నిర్మాణం name {								\
	నిర్మాణం type *lh_మొదటి;	/* first element */			\
}

#నిర్వచించు	LIST_తల_INITIALIZER(తల)					\
	{ ఖాళీ }

#నిర్వచించు	LIST_నమోదు(type)						\
నిర్మాణం {								\
	నిర్మాణం type *le_తదుపరి;	/* next element */			\
	నిర్మాణం type **le_prev;	/* address of previous next element */	\
}

/*
 * List access methods.
 */
#నిర్వచించు	LIST_మొదటి(తల)		((తల)->lh_మొదటి)
#నిర్వచించు	LIST_చివర(తల)			ఖాళీ
#నిర్వచించు	LIST_ఖాళీలు(తల)		((తల)->lh_మొదటి == LIST_చివర(తల))
#నిర్వచించు	LIST_తదుపరి(elm, క్షేత్రం)		((elm)->క్షేత్రం.le_తదుపరి)

#నిర్వచించు	LIST_FOREACH(var, తల, క్షేత్రం)					\
	కొరకు ((var) = ((తల)->lh_మొదటి);				\
	    (var) != LIST_చివర(తల);					\
	    (var) = ((var)->క్షేత్రం.le_తదుపరి))

#నిర్వచించు	LIST_FOREACH_సురక్షిత(var, తల, క్షేత్రం, tvar)			\
	కొరకు ((var) = LIST_మొదటి((తల));				\
	    (var) != LIST_చివర(తల) &&					\
	    ((tvar) = LIST_తదుపరి((var), క్షేత్రం), 1);			\
	    (var) = (tvar))

#నిర్వచించు	LIST_MOVE(head1, head2) చేయి {					\
	LIST_INIT((head2));						\
	ఒకవేళ (!LIST_ఖాళీలు((head1))) {					\
		(head2)->lh_మొదటి = (head1)->lh_మొదటి;			\
		LIST_INIT((head1));					\
	}								\
} అయితే (/*CONSTCOND*/0)

/*
 * List functions.
 */
#ఒకవేళ defined(QUEUEDEBUG)
#నిర్వచించు	QUEUEDEBUG_LIST_చొప్పించు_తల(తల, elm, క్షేత్రం)			\
	ఒకవేళ ((తల)->lh_మొదటి &&						\
	    (తల)->lh_మొదటి->క్షేత్రం.le_prev != &(తల)->lh_మొదటి)	\
		QUEUEDEBUG_ABORT("LIST_INSERT_HEAD %p %s:%d", (తల),	\
		    __FILE__, __LINE__);
#నిర్వచించు	QUEUEDEBUG_LIST_OP(elm, క్షేత్రం)					\
	ఒకవేళ ((elm)->క్షేత్రం.le_తదుపరి &&					\
	    (elm)->క్షేత్రం.le_తదుపరి->క్షేత్రం.le_prev !=			\
	    &(elm)->క్షేత్రం.le_తదుపరి)					\
		QUEUEDEBUG_ABORT("LIST_* forw %p %s:%d", (elm),		\
		    __FILE__, __LINE__);				\
	ఒకవేళ (*(elm)->క్షేత్రం.le_prev != (elm))				\
		QUEUEDEBUG_ABORT("LIST_* back %p %s:%d", (elm),		\
		    __FILE__, __LINE__);
#నిర్వచించు	QUEUEDEBUG_LIST_POSTREMOVE(elm, క్షేత్రం)				\
	(elm)->క్షేత్రం.le_తదుపరి = (శూన్య *)1L;				\
	(elm)->క్షేత్రం.le_prev = (శూన్య *)1L;
#లేకపోతే
#నిర్వచించు	QUEUEDEBUG_LIST_చొప్పించు_తల(తల, elm, క్షేత్రం)
#నిర్వచించు	QUEUEDEBUG_LIST_OP(elm, క్షేత్రం)
#నిర్వచించు	QUEUEDEBUG_LIST_POSTREMOVE(elm, క్షేత్రం)
#అయితే_ముగింపు

#నిర్వచించు	LIST_INIT(తల) చేయి {						\
	(తల)->lh_మొదటి = LIST_చివర(తల);				\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	LIST_చొప్పించు_AFTER(listelm, elm, క్షేత్రం) చేయి {			\
	QUEUEDEBUG_LIST_OP((listelm), క్షేత్రం)				\
	ఒకవేళ (((elm)->క్షేత్రం.le_తదుపరి = (listelm)->క్షేత్రం.le_తదుపరి) != 	\
	    LIST_చివర(తల))						\
		(listelm)->క్షేత్రం.le_తదుపరి->క్షేత్రం.le_prev =		\
		    &(elm)->క్షేత్రం.le_తదుపరి;				\
	(listelm)->క్షేత్రం.le_తదుపరి = (elm);				\
	(elm)->క్షేత్రం.le_prev = &(listelm)->క్షేత్రం.le_తదుపరి;		\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	LIST_చొప్పించు_BEFORE(listelm, elm, క్షేత్రం) చేయి {			\
	QUEUEDEBUG_LIST_OP((listelm), క్షేత్రం)				\
	(elm)->క్షేత్రం.le_prev = (listelm)->క్షేత్రం.le_prev;		\
	(elm)->క్షేత్రం.le_తదుపరి = (listelm);				\
	*(listelm)->క్షేత్రం.le_prev = (elm);				\
	(listelm)->క్షేత్రం.le_prev = &(elm)->క్షేత్రం.le_తదుపరి;		\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	LIST_చొప్పించు_తల(తల, elm, క్షేత్రం) చేయి {				\
	QUEUEDEBUG_LIST_చొప్పించు_తల((తల), (elm), క్షేత్రం)		\
	ఒకవేళ (((elm)->క్షేత్రం.le_తదుపరి = (తల)->lh_మొదటి) != LIST_చివర(తల))\
		(తల)->lh_మొదటి->క్షేత్రం.le_prev = &(elm)->క్షేత్రం.le_తదుపరి;\
	(తల)->lh_మొదటి = (elm);					\
	(elm)->క్షేత్రం.le_prev = &(తల)->lh_మొదటి;			\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	LIST_REMOVE(elm, క్షేత్రం) చేయి {					\
	QUEUEDEBUG_LIST_OP((elm), క్షేత్రం)				\
	ఒకవేళ ((elm)->క్షేత్రం.le_తదుపరి != ఖాళీ)				\
		(elm)->క్షేత్రం.le_తదుపరి->క్షేత్రం.le_prev = 			\
		    (elm)->క్షేత్రం.le_prev;				\
	*(elm)->క్షేత్రం.le_prev = (elm)->క్షేత్రం.le_తదుపరి;			\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), క్షేత్రం)			\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు LIST_REPLACE(elm, elm2, క్షేత్రం) చేయి {				\
	ఒకవేళ (((elm2)->క్షేత్రం.le_తదుపరి = (elm)->క్షేత్రం.le_తదుపరి) != ఖాళీ)	\
		(elm2)->క్షేత్రం.le_తదుపరి->క్షేత్రం.le_prev =			\
		    &(elm2)->క్షేత్రం.le_తదుపరి;				\
	(elm2)->క్షేత్రం.le_prev = (elm)->క్షేత్రం.le_prev;			\
	*(elm2)->క్షేత్రం.le_prev = (elm2);				\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), క్షేత్రం)			\
} అయితే (/*CONSTCOND*/0)

/*
 * Simple queue definitions.
 */
#నిర్వచించు	SIMPLEQ_తల(name, type)					\
నిర్మాణం name {								\
	నిర్మాణం type *sqh_మొదటి;	/* first element */			\
	నిర్మాణం type **sqh_చివరి;	/* addr of last next element */		\
}

#నిర్వచించు	SIMPLEQ_తల_INITIALIZER(తల)					\
	{ ఖాళీ, &(తల).sqh_మొదటి }

#నిర్వచించు	SIMPLEQ_నమోదు(type)						\
నిర్మాణం {								\
	నిర్మాణం type *sqe_తదుపరి;	/* next element */			\
}

/*
 * Simple queue access methods.
 */
#నిర్వచించు	SIMPLEQ_మొదటి(తల)		((తల)->sqh_మొదటి)
#నిర్వచించు	SIMPLEQ_చివర(తల)		ఖాళీ
#నిర్వచించు	SIMPLEQ_ఖాళీలు(తల)		((తల)->sqh_మొదటి == SIMPLEQ_చివర(తల))
#నిర్వచించు	SIMPLEQ_తదుపరి(elm, క్షేత్రం)	((elm)->క్షేత్రం.sqe_తదుపరి)

#నిర్వచించు	SIMPLEQ_FOREACH(var, తల, క్షేత్రం)				\
	కొరకు ((var) = ((తల)->sqh_మొదటి);				\
	    (var) != SIMPLEQ_చివర(తల);					\
	    (var) = ((var)->క్షేత్రం.sqe_తదుపరి))

#నిర్వచించు	SIMPLEQ_FOREACH_సురక్షిత(var, తల, క్షేత్రం, తదుపరి)			\
	కొరకు ((var) = ((తల)->sqh_మొదటి);				\
	    (var) != SIMPLEQ_చివర(తల) &&				\
	    ((తదుపరి = ((var)->క్షేత్రం.sqe_తదుపరి)), 1);			\
	    (var) = (తదుపరి))

/*
 * Simple queue functions.
 */
#నిర్వచించు	SIMPLEQ_INIT(తల) చేయి {						\
	(తల)->sqh_మొదటి = ఖాళీ;					\
	(తల)->sqh_చివరి = &(తల)->sqh_మొదటి;				\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	SIMPLEQ_చొప్పించు_తల(తల, elm, క్షేత్రం) చేయి {			\
	ఒకవేళ (((elm)->క్షేత్రం.sqe_తదుపరి = (తల)->sqh_మొదటి) == ఖాళీ)	\
		(తల)->sqh_చివరి = &(elm)->క్షేత్రం.sqe_తదుపరి;		\
	(తల)->sqh_మొదటి = (elm);					\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	SIMPLEQ_చొప్పించు_తోక(తల, elm, క్షేత్రం) చేయి {			\
	(elm)->క్షేత్రం.sqe_తదుపరి = ఖాళీ;					\
	*(తల)->sqh_చివరి = (elm);					\
	(తల)->sqh_చివరి = &(elm)->క్షేత్రం.sqe_తదుపరి;			\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	SIMPLEQ_చొప్పించు_AFTER(తల, listelm, elm, క్షేత్రం) చేయి {		\
	ఒకవేళ (((elm)->క్షేత్రం.sqe_తదుపరి = (listelm)->క్షేత్రం.sqe_తదుపరి) == ఖాళీ)\
		(తల)->sqh_చివరి = &(elm)->క్షేత్రం.sqe_తదుపరి;		\
	(listelm)->క్షేత్రం.sqe_తదుపరి = (elm);				\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	SIMPLEQ_REMOVE_తల(తల, క్షేత్రం) చేయి {				\
	ఒకవేళ (((తల)->sqh_మొదటి = (తల)->sqh_మొదటి->క్షేత్రం.sqe_తదుపరి) == ఖాళీ) \
		(తల)->sqh_చివరి = &(తల)->sqh_మొదటి;			\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు SIMPLEQ_REMOVE_AFTER(తల, elm, క్షేత్రం) చేయి {			\
	ఒకవేళ (((elm)->క్షేత్రం.sqe_తదుపరి = (elm)->క్షేత్రం.sqe_తదుపరి->క్షేత్రం.sqe_తదుపరి) \
	    == ఖాళీ)							\
		(తల)->sqh_చివరి = &(elm)->క్షేత్రం.sqe_తదుపరి;		\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	SIMPLEQ_REMOVE(తల, elm, type, క్షేత్రం) చేయి {			\
	ఒకవేళ ((తల)->sqh_మొదటి == (elm)) {				\
		SIMPLEQ_REMOVE_తల((తల), క్షేత్రం);			\
	} లేకపోతే {							\
		నిర్మాణం type *curelm = (తల)->sqh_మొదటి;		\
		అయితే (curelm->క్షేత్రం.sqe_తదుపరి != (elm))			\
			curelm = curelm->క్షేత్రం.sqe_తదుపరి;		\
		ఒకవేళ ((curelm->క్షేత్రం.sqe_తదుపరి =				\
			curelm->క్షేత్రం.sqe_తదుపరి->క్షేత్రం.sqe_తదుపరి) == ఖాళీ) \
			    (తల)->sqh_చివరి = &(curelm)->క్షేత్రం.sqe_తదుపరి; \
	}								\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	SIMPLEQ_CONCAT(head1, head2) చేయి {				\
	ఒకవేళ (!SIMPLEQ_ఖాళీలు((head2))) {					\
		*(head1)->sqh_చివరి = (head2)->sqh_మొదటి;		\
		(head1)->sqh_చివరి = (head2)->sqh_చివరి;		\
		SIMPLEQ_INIT((head2));					\
	}								\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	SIMPLEQ_చివరి(తల, type, క్షేత్రం)					\
	(SIMPLEQ_ఖాళీలు((తల)) ?						\
		ఖాళీ :							\
	        ((నిర్మాణం type *)(శూన్య *)				\
		((అక్షరం *)((తల)->sqh_చివరి) - offsetof(నిర్మాణం type, క్షేత్రం))))

/*
 * Tail queue definitions.
 */
#నిర్వచించు	TAILQ_తల_2(name, type, qual)					\
నిర్మాణం name {								\
	qual type *tqh_మొదటి;		/* first element */		\
	qual type *qual *tqh_చివరి;	/* addr of last next element */	\
}
#నిర్వచించు TAILQ_తల(name, type)	TAILQ_తల_2(name, నిర్మాణం type,)

#నిర్వచించు	TAILQ_తల_INITIALIZER(తల)					\
	{ TAILQ_చివర(తల), &(తల).tqh_మొదటి }

#నిర్వచించు	TAILQ_నమోదు_2(type, qual)					\
నిర్మాణం {								\
	qual type *tqe_తదుపరి;		/* next element */		\
	qual type *qual *tqe_prev;	/* address of previous next element */\
}
#నిర్వచించు TAILQ_నమోదు(type)	TAILQ_నమోదు_2(నిర్మాణం type,)

/*
 * Tail queue access methods.
 */
#నిర్వచించు	TAILQ_మొదటి(తల)		((తల)->tqh_మొదటి)
#నిర్వచించు	TAILQ_చివర(తల)			(ఖాళీ)
#నిర్వచించు	TAILQ_తదుపరి(elm, క్షేత్రం)		((elm)->క్షేత్రం.tqe_తదుపరి)
#నిర్వచించు	TAILQ_చివరి(తల, headname) \
	(*(((నిర్మాణం headname *)((తల)->tqh_చివరి))->tqh_చివరి))
#నిర్వచించు	TAILQ_PREV(elm, headname, క్షేత్రం) \
	(*(((నిర్మాణం headname *)((elm)->క్షేత్రం.tqe_prev))->tqh_చివరి))
#నిర్వచించు	TAILQ_ఖాళీలు(తల)		(TAILQ_మొదటి(తల) == TAILQ_చివర(తల))


#నిర్వచించు	TAILQ_FOREACH(var, తల, క్షేత్రం)					\
	కొరకు ((var) = ((తల)->tqh_మొదటి);				\
	    (var) != TAILQ_చివర(తల);					\
	    (var) = ((var)->క్షేత్రం.tqe_తదుపరి))

#నిర్వచించు	TAILQ_FOREACH_సురక్షిత(var, తల, క్షేత్రం, తదుపరి)			\
	కొరకు ((var) = ((తల)->tqh_మొదటి);				\
	    (var) != TAILQ_చివర(తల) &&					\
	    ((తదుపరి) = TAILQ_తదుపరి(var, క్షేత్రం), 1); (var) = (తదుపరి))

#నిర్వచించు	TAILQ_FOREACH_తిరగేయి(var, తల, headname, క్షేత్రం)		\
	కొరకు ((var) = (*(((నిర్మాణం headname *)((తల)->tqh_చివరి))->tqh_చివరి));\
	    (var) != TAILQ_చివర(తల);					\
	    (var) = (*(((నిర్మాణం headname *)((var)->క్షేత్రం.tqe_prev))->tqh_చివరి)))

#నిర్వచించు	TAILQ_FOREACH_తిరగేయి_సురక్షిత(var, తల, headname, క్షేత్రం, prev)	\
	కొరకు ((var) = TAILQ_చివరి((తల), headname);			\
	    (var) != TAILQ_చివర(తల) && 				\
	    ((prev) = TAILQ_PREV((var), headname, క్షేత్రం), 1); (var) = (prev))

/*
 * Tail queue functions.
 */
#ఒకవేళ defined(QUEUEDEBUG)
#నిర్వచించు	QUEUEDEBUG_TAILQ_చొప్పించు_తల(తల, elm, క్షేత్రం)			\
	ఒకవేళ ((తల)->tqh_మొదటి &&					\
	    (తల)->tqh_మొదటి->క్షేత్రం.tqe_prev != &(తల)->tqh_మొదటి)	\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_HEAD %p %s:%d", (తల),	\
		    __FILE__, __LINE__);
#నిర్వచించు	QUEUEDEBUG_TAILQ_చొప్పించు_తోక(తల, elm, క్షేత్రం)			\
	ఒకవేళ (*(తల)->tqh_చివరి != ఖాళీ)					\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_TAIL %p %s:%d", (తల),	\
		    __FILE__, __LINE__);
#నిర్వచించు	QUEUEDEBUG_TAILQ_OP(elm, క్షేత్రం)					\
	ఒకవేళ ((elm)->క్షేత్రం.tqe_తదుపరి &&					\
	    (elm)->క్షేత్రం.tqe_తదుపరి->క్షేత్రం.tqe_prev !=			\
	    &(elm)->క్షేత్రం.tqe_తదుపరి)					\
		QUEUEDEBUG_ABORT("TAILQ_* forw %p %s:%d", (elm),	\
		    __FILE__, __LINE__);				\
	ఒకవేళ (*(elm)->క్షేత్రం.tqe_prev != (elm))				\
		QUEUEDEBUG_ABORT("TAILQ_* back %p %s:%d", (elm),	\
		    __FILE__, __LINE__);
#నిర్వచించు	QUEUEDEBUG_TAILQ_PREREMOVE(తల, elm, క్షేత్రం)			\
	ఒకవేళ ((elm)->క్షేత్రం.tqe_తదుపరి == ఖాళీ &&				\
	    (తల)->tqh_చివరి != &(elm)->క్షేత్రం.tqe_తదుపరి)			\
		QUEUEDEBUG_ABORT("TAILQ_PREREMOVE head %p elm %p %s:%d",\
		    (తల), (elm), __FILE__, __LINE__);
#నిర్వచించు	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, క్షేత్రం)				\
	(elm)->క్షేత్రం.tqe_తదుపరి = (శూన్య *)1L;				\
	(elm)->క్షేత్రం.tqe_prev = (శూన్య *)1L;
#లేకపోతే
#నిర్వచించు	QUEUEDEBUG_TAILQ_చొప్పించు_తల(తల, elm, క్షేత్రం)
#నిర్వచించు	QUEUEDEBUG_TAILQ_చొప్పించు_తోక(తల, elm, క్షేత్రం)
#నిర్వచించు	QUEUEDEBUG_TAILQ_OP(elm, క్షేత్రం)
#నిర్వచించు	QUEUEDEBUG_TAILQ_PREREMOVE(తల, elm, క్షేత్రం)
#నిర్వచించు	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, క్షేత్రం)
#అయితే_ముగింపు

#నిర్వచించు	TAILQ_INIT(తల) చేయి {						\
	(తల)->tqh_మొదటి = TAILQ_చివర(తల);				\
	(తల)->tqh_చివరి = &(తల)->tqh_మొదటి;				\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	TAILQ_చొప్పించు_తల(తల, elm, క్షేత్రం) చేయి {			\
	QUEUEDEBUG_TAILQ_చొప్పించు_తల((తల), (elm), క్షేత్రం)		\
	ఒకవేళ (((elm)->క్షేత్రం.tqe_తదుపరి = (తల)->tqh_మొదటి) != TAILQ_చివర(తల))\
		(తల)->tqh_మొదటి->క్షేత్రం.tqe_prev =			\
		    &(elm)->క్షేత్రం.tqe_తదుపరి;				\
	లేకపోతే								\
		(తల)->tqh_చివరి = &(elm)->క్షేత్రం.tqe_తదుపరి;		\
	(తల)->tqh_మొదటి = (elm);					\
	(elm)->క్షేత్రం.tqe_prev = &(తల)->tqh_మొదటి;			\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	TAILQ_చొప్పించు_తోక(తల, elm, క్షేత్రం) చేయి {			\
	QUEUEDEBUG_TAILQ_చొప్పించు_తోక((తల), (elm), క్షేత్రం)		\
	(elm)->క్షేత్రం.tqe_తదుపరి = TAILQ_చివర(తల);			\
	(elm)->క్షేత్రం.tqe_prev = (తల)->tqh_చివరి;			\
	*(తల)->tqh_చివరి = (elm);					\
	(తల)->tqh_చివరి = &(elm)->క్షేత్రం.tqe_తదుపరి;			\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	TAILQ_చొప్పించు_AFTER(తల, listelm, elm, క్షేత్రం) చేయి {		\
	QUEUEDEBUG_TAILQ_OP((listelm), క్షేత్రం)				\
	ఒకవేళ (((elm)->క్షేత్రం.tqe_తదుపరి = (listelm)->క్షేత్రం.tqe_తదుపరి) != 	\
	    TAILQ_చివర(తల))						\
		(elm)->క్షేత్రం.tqe_తదుపరి->క్షేత్రం.tqe_prev = 		\
		    &(elm)->క్షేత్రం.tqe_తదుపరి;				\
	లేకపోతే								\
		(తల)->tqh_చివరి = &(elm)->క్షేత్రం.tqe_తదుపరి;		\
	(listelm)->క్షేత్రం.tqe_తదుపరి = (elm);				\
	(elm)->క్షేత్రం.tqe_prev = &(listelm)->క్షేత్రం.tqe_తదుపరి;		\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	TAILQ_చొప్పించు_BEFORE(listelm, elm, క్షేత్రం) చేయి {			\
	QUEUEDEBUG_TAILQ_OP((listelm), క్షేత్రం)				\
	(elm)->క్షేత్రం.tqe_prev = (listelm)->క్షేత్రం.tqe_prev;		\
	(elm)->క్షేత్రం.tqe_తదుపరి = (listelm);				\
	*(listelm)->క్షేత్రం.tqe_prev = (elm);				\
	(listelm)->క్షేత్రం.tqe_prev = &(elm)->క్షేత్రం.tqe_తదుపరి;		\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	TAILQ_REMOVE(తల, elm, క్షేత్రం) చేయి {				\
	QUEUEDEBUG_TAILQ_PREREMOVE((తల), (elm), క్షేత్రం)		\
	QUEUEDEBUG_TAILQ_OP((elm), క్షేత్రం)				\
	ఒకవేళ (((elm)->క్షేత్రం.tqe_తదుపరి) != TAILQ_చివర(తల))			\
		(elm)->క్షేత్రం.tqe_తదుపరి->క్షేత్రం.tqe_prev = 		\
		    (elm)->క్షేత్రం.tqe_prev;				\
	లేకపోతే								\
		(తల)->tqh_చివరి = (elm)->క్షేత్రం.tqe_prev;		\
	*(elm)->క్షేత్రం.tqe_prev = (elm)->క్షేత్రం.tqe_తదుపరి;			\
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), క్షేత్రం);			\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు TAILQ_REPLACE(తల, elm, elm2, క్షేత్రం) చేయి {                      \
        ఒకవేళ (((elm2)->క్షేత్రం.tqe_తదుపరి = (elm)->క్షేత్రం.tqe_తదుపరి) != 	\
	    TAILQ_చివర(తల))   						\
                (elm2)->క్షేత్రం.tqe_తదుపరి->క్షేత్రం.tqe_prev =                \
                    &(elm2)->క్షేత్రం.tqe_తదుపరి;                            \
        లేకపోతే                                                            \
                (తల)->tqh_చివరి = &(elm2)->క్షేత్రం.tqe_తదుపరి;             \
        (elm2)->క్షేత్రం.tqe_prev = (elm)->క్షేత్రం.tqe_prev;                 \
        *(elm2)->క్షేత్రం.tqe_prev = (elm2);                               \
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), క్షేత్రం);			\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	TAILQ_CONCAT(head1, head2, క్షేత్రం) చేయి {				\
	ఒకవేళ (!TAILQ_ఖాళీలు(head2)) {					\
		*(head1)->tqh_చివరి = (head2)->tqh_మొదటి;		\
		(head2)->tqh_మొదటి->క్షేత్రం.tqe_prev = (head1)->tqh_చివరి;	\
		(head1)->tqh_చివరి = (head2)->tqh_చివరి;			\
		TAILQ_INIT((head2));					\
	}								\
} అయితే (/*CONSTCOND*/0)

/*
 * Singly-linked Tail queue declarations.
 */
#నిర్వచించు	STAILQ_తల(name, type)						\
నిర్మాణం name {								\
	నిర్మాణం type *stqh_మొదటి;	/* first element */		\
	నిర్మాణం type **stqh_చివరి;	/* addr of last next element */	\
}

#నిర్వచించు	STAILQ_తల_INITIALIZER(తల)					\
	{ ఖాళీ, &(తల).stqh_మొదటి }

#నిర్వచించు	STAILQ_నమోదు(type)						\
నిర్మాణం {								\
	నిర్మాణం type *stqe_తదుపరి;	/* next element */			\
}

/*
 * Singly-linked Tail queue access methods.
 */
#నిర్వచించు	STAILQ_మొదటి(తల)	((తల)->stqh_మొదటి)
#నిర్వచించు	STAILQ_చివర(తల)	ఖాళీ
#నిర్వచించు	STAILQ_తదుపరి(elm, క్షేత్రం)	((elm)->క్షేత్రం.stqe_తదుపరి)
#నిర్వచించు	STAILQ_ఖాళీలు(తల)	(STAILQ_మొదటి(తల) == STAILQ_చివర(తల))

/*
 * Singly-linked Tail queue functions.
 */
#నిర్వచించు	STAILQ_INIT(తల) చేయి {						\
	(తల)->stqh_మొదటి = ఖాళీ;					\
	(తల)->stqh_చివరి = &(తల)->stqh_మొదటి;				\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	STAILQ_చొప్పించు_తల(తల, elm, క్షేత్రం) చేయి {			\
	ఒకవేళ (((elm)->క్షేత్రం.stqe_తదుపరి = (తల)->stqh_మొదటి) == ఖాళీ)	\
		(తల)->stqh_చివరి = &(elm)->క్షేత్రం.stqe_తదుపరి;		\
	(తల)->stqh_మొదటి = (elm);					\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	STAILQ_చొప్పించు_తోక(తల, elm, క్షేత్రం) చేయి {			\
	(elm)->క్షేత్రం.stqe_తదుపరి = ఖాళీ;					\
	*(తల)->stqh_చివరి = (elm);					\
	(తల)->stqh_చివరి = &(elm)->క్షేత్రం.stqe_తదుపరి;			\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	STAILQ_చొప్పించు_AFTER(తల, listelm, elm, క్షేత్రం) చేయి {		\
	ఒకవేళ (((elm)->క్షేత్రం.stqe_తదుపరి = (listelm)->క్షేత్రం.stqe_తదుపరి) == ఖాళీ)\
		(తల)->stqh_చివరి = &(elm)->క్షేత్రం.stqe_తదుపరి;		\
	(listelm)->క్షేత్రం.stqe_తదుపరి = (elm);				\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	STAILQ_REMOVE_తల(తల, క్షేత్రం) చేయి {				\
	ఒకవేళ (((తల)->stqh_మొదటి = (తల)->stqh_మొదటి->క్షేత్రం.stqe_తదుపరి) == ఖాళీ) \
		(తల)->stqh_చివరి = &(తల)->stqh_మొదటి;			\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	STAILQ_REMOVE(తల, elm, type, క్షేత్రం) చేయి {			\
	ఒకవేళ ((తల)->stqh_మొదటి == (elm)) {				\
		STAILQ_REMOVE_తల((తల), క్షేత్రం);			\
	} లేకపోతే {							\
		నిర్మాణం type *curelm = (తల)->stqh_మొదటి;		\
		అయితే (curelm->క్షేత్రం.stqe_తదుపరి != (elm))			\
			curelm = curelm->క్షేత్రం.stqe_తదుపరి;		\
		ఒకవేళ ((curelm->క్షేత్రం.stqe_తదుపరి =				\
			curelm->క్షేత్రం.stqe_తదుపరి->క్షేత్రం.stqe_తదుపరి) == ఖాళీ) \
			    (తల)->stqh_చివరి = &(curelm)->క్షేత్రం.stqe_తదుపరి; \
	}								\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	STAILQ_FOREACH(var, తల, క్షేత్రం)				\
	కొరకు ((var) = ((తల)->stqh_మొదటి);				\
		(var);							\
		(var) = ((var)->క్షేత్రం.stqe_తదుపరి))

#నిర్వచించు	STAILQ_FOREACH_సురక్షిత(var, తల, క్షేత్రం, tvar)			\
	కొరకు ((var) = STAILQ_మొదటి((తల));				\
	    (var) && ((tvar) = STAILQ_తదుపరి((var), క్షేత్రం), 1);		\
	    (var) = (tvar))

#నిర్వచించు	STAILQ_CONCAT(head1, head2) చేయి {				\
	ఒకవేళ (!STAILQ_ఖాళీలు((head2))) {					\
		*(head1)->stqh_చివరి = (head2)->stqh_మొదటి;		\
		(head1)->stqh_చివరి = (head2)->stqh_చివరి;		\
		STAILQ_INIT((head2));					\
	}								\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	STAILQ_చివరి(తల, type, క్షేత్రం)					\
	(STAILQ_ఖాళీలు((తల)) ?						\
		ఖాళీ :							\
	        ((నిర్మాణం type *)(శూన్య *)				\
		((అక్షరం *)((తల)->stqh_చివరి) - offsetof(నిర్మాణం type, క్షేత్రం))))


#నిర్వచితం_కాకపోతే _KERNEL
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
స్థిర __inline స్థిరరాశి శూన్య * __launder_type(స్థిరరాశి శూన్య *);
స్థిర __inline స్థిరరాశి శూన్య *
__launder_type(స్థిరరాశి శూన్య *__x)
{
	__asm __volatile("" : "+r" (__x));
	తిరిగి __x;
}

#ఒకవేళ defined(QUEUEDEBUG)
#నిర్వచించు QUEUEDEBUG_CIRCLEQ_తల(తల, క్షేత్రం)				\
	ఒకవేళ ((తల)->cqh_మొదటి != CIRCLEQ_ENDC(తల) &&			\
	    (తల)->cqh_మొదటి->క్షేత్రం.cqe_prev != CIRCLEQ_ENDC(తల))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head forw %p %s:%d", (తల),	\
		      __FILE__, __LINE__);				\
	ఒకవేళ ((తల)->cqh_చివరి != CIRCLEQ_ENDC(తల) &&			\
	    (తల)->cqh_చివరి->క్షేత్రం.cqe_తదుపరి != CIRCLEQ_ENDC(తల))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head back %p %s:%d", (తల),	\
		      __FILE__, __LINE__);
#నిర్వచించు QUEUEDEBUG_CIRCLEQ_ELM(తల, elm, క్షేత్రం)			\
	ఒకవేళ ((elm)->క్షేత్రం.cqe_తదుపరి == CIRCLEQ_ENDC(తల)) {		\
		ఒకవేళ ((తల)->cqh_చివరి != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm last %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} లేకపోతే {							\
		ఒకవేళ ((elm)->క్షేత్రం.cqe_తదుపరి->క్షేత్రం.cqe_prev != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm forw %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}								\
	ఒకవేళ ((elm)->క్షేత్రం.cqe_prev == CIRCLEQ_ENDC(తల)) {		\
		ఒకవేళ ((తల)->cqh_మొదటి != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm first %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} లేకపోతే {							\
		ఒకవేళ ((elm)->క్షేత్రం.cqe_prev->క్షేత్రం.cqe_తదుపరి != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm prev %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}
#నిర్వచించు QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, క్షేత్రం)			\
	(elm)->క్షేత్రం.cqe_తదుపరి = (శూన్య *)1L;				\
	(elm)->క్షేత్రం.cqe_prev = (శూన్య *)1L;
#లేకపోతే
#నిర్వచించు QUEUEDEBUG_CIRCLEQ_తల(తల, క్షేత్రం)
#నిర్వచించు QUEUEDEBUG_CIRCLEQ_ELM(తల, elm, క్షేత్రం)
#నిర్వచించు QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, క్షేత్రం)
#అయితే_ముగింపు

#నిర్వచించు	CIRCLEQ_తల(name, type)					\
నిర్మాణం name {								\
	నిర్మాణం type *cqh_మొదటి;		/* first element */		\
	నిర్మాణం type *cqh_చివరి;		/* last element */		\
}

#నిర్వచించు	CIRCLEQ_తల_INITIALIZER(తల)					\
	{ CIRCLEQ_చివర(&తల), CIRCLEQ_చివర(&తల) }

#నిర్వచించు	CIRCLEQ_నమోదు(type)						\
నిర్మాణం {								\
	నిర్మాణం type *cqe_తదుపరి;		/* next element */		\
	నిర్మాణం type *cqe_prev;		/* previous element */		\
}

/*
 * Circular queue functions.
 */
#నిర్వచించు	CIRCLEQ_INIT(తల) చేయి {						\
	(తల)->cqh_మొదటి = CIRCLEQ_చివర(తల);				\
	(తల)->cqh_చివరి = CIRCLEQ_చివర(తల);				\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	CIRCLEQ_చొప్పించు_AFTER(తల, listelm, elm, క్షేత్రం) చేయి {		\
	QUEUEDEBUG_CIRCLEQ_తల((తల), క్షేత్రం)				\
	QUEUEDEBUG_CIRCLEQ_ELM((తల), (listelm), క్షేత్రం)		\
	(elm)->క్షేత్రం.cqe_తదుపరి = (listelm)->క్షేత్రం.cqe_తదుపరి;		\
	(elm)->క్షేత్రం.cqe_prev = (listelm);				\
	ఒకవేళ ((listelm)->క్షేత్రం.cqe_తదుపరి == CIRCLEQ_ENDC(తల))		\
		(తల)->cqh_చివరి = (elm);				\
	లేకపోతే								\
		(listelm)->క్షేత్రం.cqe_తదుపరి->క్షేత్రం.cqe_prev = (elm);	\
	(listelm)->క్షేత్రం.cqe_తదుపరి = (elm);				\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	CIRCLEQ_చొప్పించు_BEFORE(తల, listelm, elm, క్షేత్రం) చేయి {		\
	QUEUEDEBUG_CIRCLEQ_తల((తల), క్షేత్రం)				\
	QUEUEDEBUG_CIRCLEQ_ELM((తల), (listelm), క్షేత్రం)		\
	(elm)->క్షేత్రం.cqe_తదుపరి = (listelm);				\
	(elm)->క్షేత్రం.cqe_prev = (listelm)->క్షేత్రం.cqe_prev;		\
	ఒకవేళ ((listelm)->క్షేత్రం.cqe_prev == CIRCLEQ_ENDC(తల))		\
		(తల)->cqh_మొదటి = (elm);				\
	లేకపోతే								\
		(listelm)->క్షేత్రం.cqe_prev->క్షేత్రం.cqe_తదుపరి = (elm);	\
	(listelm)->క్షేత్రం.cqe_prev = (elm);				\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	CIRCLEQ_చొప్పించు_తల(తల, elm, క్షేత్రం) చేయి {			\
	QUEUEDEBUG_CIRCLEQ_తల((తల), క్షేత్రం)				\
	(elm)->క్షేత్రం.cqe_తదుపరి = (తల)->cqh_మొదటి;			\
	(elm)->క్షేత్రం.cqe_prev = CIRCLEQ_చివర(తల);			\
	ఒకవేళ ((తల)->cqh_చివరి == CIRCLEQ_ENDC(తల))			\
		(తల)->cqh_చివరి = (elm);				\
	లేకపోతే								\
		(తల)->cqh_మొదటి->క్షేత్రం.cqe_prev = (elm);		\
	(తల)->cqh_మొదటి = (elm);					\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	CIRCLEQ_చొప్పించు_తోక(తల, elm, క్షేత్రం) చేయి {			\
	QUEUEDEBUG_CIRCLEQ_తల((తల), క్షేత్రం)				\
	(elm)->క్షేత్రం.cqe_తదుపరి = CIRCLEQ_చివర(తల);			\
	(elm)->క్షేత్రం.cqe_prev = (తల)->cqh_చివరి;			\
	ఒకవేళ ((తల)->cqh_మొదటి == CIRCLEQ_ENDC(తల))			\
		(తల)->cqh_మొదటి = (elm);				\
	లేకపోతే								\
		(తల)->cqh_చివరి->క్షేత్రం.cqe_తదుపరి = (elm);		\
	(తల)->cqh_చివరి = (elm);					\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	CIRCLEQ_REMOVE(తల, elm, క్షేత్రం) చేయి {				\
	QUEUEDEBUG_CIRCLEQ_తల((తల), క్షేత్రం)				\
	QUEUEDEBUG_CIRCLEQ_ELM((తల), (elm), క్షేత్రం)			\
	ఒకవేళ ((elm)->క్షేత్రం.cqe_తదుపరి == CIRCLEQ_ENDC(తల))		\
		(తల)->cqh_చివరి = (elm)->క్షేత్రం.cqe_prev;		\
	లేకపోతే								\
		(elm)->క్షేత్రం.cqe_తదుపరి->క్షేత్రం.cqe_prev =			\
		    (elm)->క్షేత్రం.cqe_prev;				\
	ఒకవేళ ((elm)->క్షేత్రం.cqe_prev == CIRCLEQ_ENDC(తల))		\
		(తల)->cqh_మొదటి = (elm)->క్షేత్రం.cqe_తదుపరి;		\
	లేకపోతే								\
		(elm)->క్షేత్రం.cqe_prev->క్షేత్రం.cqe_తదుపరి =			\
		    (elm)->క్షేత్రం.cqe_తదుపరి;				\
	QUEUEDEBUG_CIRCLEQ_POSTREMOVE((elm), క్షేత్రం)			\
} అయితే (/*CONSTCOND*/0)

#నిర్వచించు	CIRCLEQ_FOREACH(var, తల, క్షేత్రం)				\
	కొరకు ((var) = ((తల)->cqh_మొదటి);				\
		(var) != CIRCLEQ_ENDC(తల);				\
		(var) = ((var)->క్షేత్రం.cqe_తదుపరి))

#నిర్వచించు	CIRCLEQ_FOREACH_తిరగేయి(var, తల, క్షేత్రం)			\
	కొరకు ((var) = ((తల)->cqh_చివరి);				\
		(var) != CIRCLEQ_ENDC(తల);				\
		(var) = ((var)->క్షేత్రం.cqe_prev))

/*
 * Circular queue access methods.
 */
#నిర్వచించు	CIRCLEQ_మొదటి(తల)		((తల)->cqh_మొదటి)
#నిర్వచించు	CIRCLEQ_చివరి(తల)		((తల)->cqh_చివరి)
/* For comparisons */
#నిర్వచించు	CIRCLEQ_ENDC(తల)		(__launder_type(తల))
/* For assignments */
#నిర్వచించు	CIRCLEQ_చివర(తల)		((శూన్య *)(తల))
#నిర్వచించు	CIRCLEQ_తదుపరి(elm, క్షేత్రం)	((elm)->క్షేత్రం.cqe_తదుపరి)
#నిర్వచించు	CIRCLEQ_PREV(elm, క్షేత్రం)	((elm)->క్షేత్రం.cqe_prev)
#నిర్వచించు	CIRCLEQ_ఖాళీలు(తల)						\
    (CIRCLEQ_మొదటి(తల) == CIRCLEQ_ENDC(తల))

#నిర్వచించు CIRCLEQ_LOOP_తదుపరి(తల, elm, క్షేత్రం)				\
	(((elm)->క్షేత్రం.cqe_తదుపరి == CIRCLEQ_ENDC(తల))			\
	    ? ((తల)->cqh_మొదటి)					\
	    : (elm->క్షేత్రం.cqe_తదుపరి))
#నిర్వచించు CIRCLEQ_LOOP_PREV(తల, elm, క్షేత్రం)				\
	(((elm)->క్షేత్రం.cqe_prev == CIRCLEQ_ENDC(తల))			\
	    ? ((తల)->cqh_చివరి)					\
	    : (elm->క్షేత్రం.cqe_prev))
#అయితే_ముగింపు /* !_KERNEL */

#అయితే_ముగింపు	/* !_SYS_QUEUE_H_ */
