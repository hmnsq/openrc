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

#که_نه_تعریف_شوی	SYS_کتار_H
#تعریف_کړه	SYS_کتار_H

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
#که_تعریف_شوی __NetBSD__
#ضمن <sys/null.h>
#د_که_پای

#إذا defined(QUEUEDEBUG)
# إذا defined(_KERNEL)
#  define QUEUEDEBUG_ABORT(...) panic(__VA_ARGS__)
# وإلا
#  include <err.h>
#  define QUEUEDEBUG_ABORT(...) err(1, __VA_ARGS__)
# endif
#د_که_پای

/*
 * Singly-linked List definitions.
 */
#تعریف_کړه	SLIST_سر(name, type)						\
بنية name {								\
	بنية type *slh_لومړی;	/* first element */			\
}

#تعریف_کړه	SLIST_سر_INITIALIZER(سر_2)					\
	{ عدم }

#تعریف_کړه	SLIST_ثبت(type)						\
بنية {								\
	بنية type *sle_بل;	/* next element */			\
}

/*
 * Singly-linked List access methods.
 */
#تعریف_کړه	SLIST_لومړی(سر_2)	((سر_2)->slh_لومړی)
#تعریف_کړه	SLIST_پای_ټکی(سر_2)		عدم
#تعریف_کړه	SLIST_تشه(سر_2)	((سر_2)->slh_لومړی == عدم)
#تعریف_کړه	SLIST_بل(elm, ډګر)	((elm)->ډګر.sle_بل)

#تعریف_کړه	SLIST_FOREACH(var, سر_2, ډګر)					\
	لكل((var) = (سر_2)->slh_لومړی;					\
	    (var) != SLIST_پای_ټکی(سر_2);					\
	    (var) = (var)->ډګر.sle_بل)

#تعریف_کړه	SLIST_FOREACH_خوندي(var, سر_2, ډګر, tvar)			\
	لكل ((var) = SLIST_لومړی((سر_2));				\
	    (var) != SLIST_پای_ټکی(سر_2) &&					\
	    ((tvar) = SLIST_بل((var), ډګر), 1);			\
	    (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#تعریف_کړه	SLIST_INIT(سر_2) افعل {						\
	(سر_2)->slh_لومړی = SLIST_پای_ټکی(سر_2);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	SLIST_ورننباسه_AFTER(slistelm, elm, ډګر) افعل {			\
	(elm)->ډګر.sle_بل = (slistelm)->ډګر.sle_بل;		\
	(slistelm)->ډګر.sle_بل = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	SLIST_ورننباسه_سر(سر_2, elm, ډګر) افعل {			\
	(elm)->ډګر.sle_بل = (سر_2)->slh_لومړی;			\
	(سر_2)->slh_لومړی = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	SLIST_REMOVE_AFTER(slistelm, ډګر) افعل {			\
	(slistelm)->ډګر.sle_بل =					\
	    SLIST_بل(SLIST_بل((slistelm), ډګر), ډګر);		\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	SLIST_REMOVE_سر(سر_2, ډګر) افعل {				\
	(سر_2)->slh_لومړی = (سر_2)->slh_لومړی->ډګر.sle_بل;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	SLIST_REMOVE(سر_2, elm, type, ډګر) افعل {			\
	إذا ((سر_2)->slh_لومړی == (elm)) {				\
		SLIST_REMOVE_سر((سر_2), ډګر);			\
	}								\
	وإلا {								\
		بنية type *curelm = (سر_2)->slh_لومړی;		\
		طالما(curelm->ډګر.sle_بل != (elm))			\
			curelm = curelm->ډګر.sle_بل;		\
		curelm->ډګر.sle_بل =				\
		    curelm->ډګر.sle_بل->ډګر.sle_بل;		\
	}								\
} طالما (/*CONSTCOND*/0)


/*
 * List definitions.
 */
#تعریف_کړه	LIST_سر(name, type)						\
بنية name {								\
	بنية type *lh_لومړی;	/* first element */			\
}

#تعریف_کړه	LIST_سر_INITIALIZER(سر_2)					\
	{ عدم }

#تعریف_کړه	LIST_ثبت(type)						\
بنية {								\
	بنية type *le_بل;	/* next element */			\
	بنية type **le_prev;	/* address of previous next element */	\
}

/*
 * List access methods.
 */
#تعریف_کړه	LIST_لومړی(سر_2)		((سر_2)->lh_لومړی)
#تعریف_کړه	LIST_پای_ټکی(سر_2)			عدم
#تعریف_کړه	LIST_تشه(سر_2)		((سر_2)->lh_لومړی == LIST_پای_ټکی(سر_2))
#تعریف_کړه	LIST_بل(elm, ډګر)		((elm)->ډګر.le_بل)

#تعریف_کړه	LIST_FOREACH(var, سر_2, ډګر)					\
	لكل ((var) = ((سر_2)->lh_لومړی);				\
	    (var) != LIST_پای_ټکی(سر_2);					\
	    (var) = ((var)->ډګر.le_بل))

#تعریف_کړه	LIST_FOREACH_خوندي(var, سر_2, ډګر, tvar)			\
	لكل ((var) = LIST_لومړی((سر_2));				\
	    (var) != LIST_پای_ټکی(سر_2) &&					\
	    ((tvar) = LIST_بل((var), ډګر), 1);			\
	    (var) = (tvar))

#تعریف_کړه	LIST_MOVE(head1, head2) افعل {					\
	LIST_INIT((head2));						\
	إذا (!LIST_تشه((head1))) {					\
		(head2)->lh_لومړی = (head1)->lh_لومړی;			\
		LIST_INIT((head1));					\
	}								\
} طالما (/*CONSTCOND*/0)

/*
 * List functions.
 */
#إذا defined(QUEUEDEBUG)
#تعریف_کړه	QUEUEDEBUG_LIST_ورننباسه_سر(سر_2, elm, ډګر)			\
	إذا ((سر_2)->lh_لومړی &&						\
	    (سر_2)->lh_لومړی->ډګر.le_prev != &(سر_2)->lh_لومړی)	\
		QUEUEDEBUG_ABORT("LIST_INSERT_HEAD %p %s:%d", (سر_2),	\
		    __FILE__, __LINE__);
#تعریف_کړه	QUEUEDEBUG_LIST_OP(elm, ډګر)					\
	إذا ((elm)->ډګر.le_بل &&					\
	    (elm)->ډګر.le_بل->ډګر.le_prev !=			\
	    &(elm)->ډګر.le_بل)					\
		QUEUEDEBUG_ABORT("LIST_* forw %p %s:%d", (elm),		\
		    __FILE__, __LINE__);				\
	إذا (*(elm)->ډګر.le_prev != (elm))				\
		QUEUEDEBUG_ABORT("LIST_* back %p %s:%d", (elm),		\
		    __FILE__, __LINE__);
#تعریف_کړه	QUEUEDEBUG_LIST_POSTREMOVE(elm, ډګر)				\
	(elm)->ډګر.le_بل = (فراغ *)1L;				\
	(elm)->ډګر.le_prev = (فراغ *)1L;
#وإلا
#تعریف_کړه	QUEUEDEBUG_LIST_ورننباسه_سر(سر_2, elm, ډګر)
#تعریف_کړه	QUEUEDEBUG_LIST_OP(elm, ډګر)
#تعریف_کړه	QUEUEDEBUG_LIST_POSTREMOVE(elm, ډګر)
#د_که_پای

#تعریف_کړه	LIST_INIT(سر_2) افعل {						\
	(سر_2)->lh_لومړی = LIST_پای_ټکی(سر_2);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	LIST_ورننباسه_AFTER(listelm, elm, ډګر) افعل {			\
	QUEUEDEBUG_LIST_OP((listelm), ډګر)				\
	إذا (((elm)->ډګر.le_بل = (listelm)->ډګر.le_بل) != 	\
	    LIST_پای_ټکی(سر_2))						\
		(listelm)->ډګر.le_بل->ډګر.le_prev =		\
		    &(elm)->ډګر.le_بل;				\
	(listelm)->ډګر.le_بل = (elm);				\
	(elm)->ډګر.le_prev = &(listelm)->ډګر.le_بل;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	LIST_ورننباسه_BEFORE(listelm, elm, ډګر) افعل {			\
	QUEUEDEBUG_LIST_OP((listelm), ډګر)				\
	(elm)->ډګر.le_prev = (listelm)->ډګر.le_prev;		\
	(elm)->ډګر.le_بل = (listelm);				\
	*(listelm)->ډګر.le_prev = (elm);				\
	(listelm)->ډګر.le_prev = &(elm)->ډګر.le_بل;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	LIST_ورننباسه_سر(سر_2, elm, ډګر) افعل {				\
	QUEUEDEBUG_LIST_ورننباسه_سر((سر_2), (elm), ډګر)		\
	إذا (((elm)->ډګر.le_بل = (سر_2)->lh_لومړی) != LIST_پای_ټکی(سر_2))\
		(سر_2)->lh_لومړی->ډګر.le_prev = &(elm)->ډګر.le_بل;\
	(سر_2)->lh_لومړی = (elm);					\
	(elm)->ډګر.le_prev = &(سر_2)->lh_لومړی;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	LIST_REMOVE(elm, ډګر) افعل {					\
	QUEUEDEBUG_LIST_OP((elm), ډګر)				\
	إذا ((elm)->ډګر.le_بل != عدم)				\
		(elm)->ډګر.le_بل->ډګر.le_prev = 			\
		    (elm)->ډګر.le_prev;				\
	*(elm)->ډګر.le_prev = (elm)->ډګر.le_بل;			\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), ډګر)			\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه LIST_REPLACE(elm, elm2, ډګر) افعل {				\
	إذا (((elm2)->ډګر.le_بل = (elm)->ډګر.le_بل) != عدم)	\
		(elm2)->ډګر.le_بل->ډګر.le_prev =			\
		    &(elm2)->ډګر.le_بل;				\
	(elm2)->ډګر.le_prev = (elm)->ډګر.le_prev;			\
	*(elm2)->ډګر.le_prev = (elm2);				\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), ډګر)			\
} طالما (/*CONSTCOND*/0)

/*
 * Simple queue definitions.
 */
#تعریف_کړه	SIMPLEQ_سر(name, type)					\
بنية name {								\
	بنية type *sqh_لومړی;	/* first element */			\
	بنية type **sqh_وروستی;	/* addr of last next element */		\
}

#تعریف_کړه	SIMPLEQ_سر_INITIALIZER(سر_2)					\
	{ عدم, &(سر_2).sqh_لومړی }

#تعریف_کړه	SIMPLEQ_ثبت(type)						\
بنية {								\
	بنية type *sqe_بل;	/* next element */			\
}

/*
 * Simple queue access methods.
 */
#تعریف_کړه	SIMPLEQ_لومړی(سر_2)		((سر_2)->sqh_لومړی)
#تعریف_کړه	SIMPLEQ_پای_ټکی(سر_2)		عدم
#تعریف_کړه	SIMPLEQ_تشه(سر_2)		((سر_2)->sqh_لومړی == SIMPLEQ_پای_ټکی(سر_2))
#تعریف_کړه	SIMPLEQ_بل(elm, ډګر)	((elm)->ډګر.sqe_بل)

#تعریف_کړه	SIMPLEQ_FOREACH(var, سر_2, ډګر)				\
	لكل ((var) = ((سر_2)->sqh_لومړی);				\
	    (var) != SIMPLEQ_پای_ټکی(سر_2);					\
	    (var) = ((var)->ډګر.sqe_بل))

#تعریف_کړه	SIMPLEQ_FOREACH_خوندي(var, سر_2, ډګر, بل)			\
	لكل ((var) = ((سر_2)->sqh_لومړی);				\
	    (var) != SIMPLEQ_پای_ټکی(سر_2) &&				\
	    ((بل = ((var)->ډګر.sqe_بل)), 1);			\
	    (var) = (بل))

/*
 * Simple queue functions.
 */
#تعریف_کړه	SIMPLEQ_INIT(سر_2) افعل {						\
	(سر_2)->sqh_لومړی = عدم;					\
	(سر_2)->sqh_وروستی = &(سر_2)->sqh_لومړی;				\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	SIMPLEQ_ورننباسه_سر(سر_2, elm, ډګر) افعل {			\
	إذا (((elm)->ډګر.sqe_بل = (سر_2)->sqh_لومړی) == عدم)	\
		(سر_2)->sqh_وروستی = &(elm)->ډګر.sqe_بل;		\
	(سر_2)->sqh_لومړی = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	SIMPLEQ_ورننباسه_لکۍ(سر_2, elm, ډګر) افعل {			\
	(elm)->ډګر.sqe_بل = عدم;					\
	*(سر_2)->sqh_وروستی = (elm);					\
	(سر_2)->sqh_وروستی = &(elm)->ډګر.sqe_بل;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	SIMPLEQ_ورننباسه_AFTER(سر_2, listelm, elm, ډګر) افعل {		\
	إذا (((elm)->ډګر.sqe_بل = (listelm)->ډګر.sqe_بل) == عدم)\
		(سر_2)->sqh_وروستی = &(elm)->ډګر.sqe_بل;		\
	(listelm)->ډګر.sqe_بل = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	SIMPLEQ_REMOVE_سر(سر_2, ډګر) افعل {				\
	إذا (((سر_2)->sqh_لومړی = (سر_2)->sqh_لومړی->ډګر.sqe_بل) == عدم) \
		(سر_2)->sqh_وروستی = &(سر_2)->sqh_لومړی;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه SIMPLEQ_REMOVE_AFTER(سر_2, elm, ډګر) افعل {			\
	إذا (((elm)->ډګر.sqe_بل = (elm)->ډګر.sqe_بل->ډګر.sqe_بل) \
	    == عدم)							\
		(سر_2)->sqh_وروستی = &(elm)->ډګر.sqe_بل;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	SIMPLEQ_REMOVE(سر_2, elm, type, ډګر) افعل {			\
	إذا ((سر_2)->sqh_لومړی == (elm)) {				\
		SIMPLEQ_REMOVE_سر((سر_2), ډګر);			\
	} وإلا {							\
		بنية type *curelm = (سر_2)->sqh_لومړی;		\
		طالما (curelm->ډګر.sqe_بل != (elm))			\
			curelm = curelm->ډګر.sqe_بل;		\
		إذا ((curelm->ډګر.sqe_بل =				\
			curelm->ډګر.sqe_بل->ډګر.sqe_بل) == عدم) \
			    (سر_2)->sqh_وروستی = &(curelm)->ډګر.sqe_بل; \
	}								\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	SIMPLEQ_CONCAT(head1, head2) افعل {				\
	إذا (!SIMPLEQ_تشه((head2))) {					\
		*(head1)->sqh_وروستی = (head2)->sqh_لومړی;		\
		(head1)->sqh_وروستی = (head2)->sqh_وروستی;		\
		SIMPLEQ_INIT((head2));					\
	}								\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	SIMPLEQ_وروستی(سر_2, type, ډګر)					\
	(SIMPLEQ_تشه((سر_2)) ?						\
		عدم :							\
	        ((بنية type *)(فراغ *)				\
		((حرف *)((سر_2)->sqh_وروستی) - offsetof(بنية type, ډګر))))

/*
 * Tail queue definitions.
 */
#تعریف_کړه	TAILQ_سر_2(name, type, qual)					\
بنية name {								\
	qual type *tqh_لومړی;		/* first element */		\
	qual type *qual *tqh_وروستی;	/* addr of last next element */	\
}
#تعریف_کړه TAILQ_سر(name, type)	TAILQ_سر_2(name, بنية type,)

#تعریف_کړه	TAILQ_سر_INITIALIZER(سر_2)					\
	{ TAILQ_پای_ټکی(سر_2), &(سر_2).tqh_لومړی }

#تعریف_کړه	TAILQ_ثبت_2(type, qual)					\
بنية {								\
	qual type *tqe_بل;		/* next element */		\
	qual type *qual *tqe_prev;	/* address of previous next element */\
}
#تعریف_کړه TAILQ_ثبت(type)	TAILQ_ثبت_2(بنية type,)

/*
 * Tail queue access methods.
 */
#تعریف_کړه	TAILQ_لومړی(سر_2)		((سر_2)->tqh_لومړی)
#تعریف_کړه	TAILQ_پای_ټکی(سر_2)			(عدم)
#تعریف_کړه	TAILQ_بل(elm, ډګر)		((elm)->ډګر.tqe_بل)
#تعریف_کړه	TAILQ_وروستی(سر_2, headname) \
	(*(((بنية headname *)((سر_2)->tqh_وروستی))->tqh_وروستی))
#تعریف_کړه	TAILQ_PREV(elm, headname, ډګر) \
	(*(((بنية headname *)((elm)->ډګر.tqe_prev))->tqh_وروستی))
#تعریف_کړه	TAILQ_تشه(سر_2)		(TAILQ_لومړی(سر_2) == TAILQ_پای_ټکی(سر_2))


#تعریف_کړه	TAILQ_FOREACH(var, سر_2, ډګر)					\
	لكل ((var) = ((سر_2)->tqh_لومړی);				\
	    (var) != TAILQ_پای_ټکی(سر_2);					\
	    (var) = ((var)->ډګر.tqe_بل))

#تعریف_کړه	TAILQ_FOREACH_خوندي(var, سر_2, ډګر, بل)			\
	لكل ((var) = ((سر_2)->tqh_لومړی);				\
	    (var) != TAILQ_پای_ټکی(سر_2) &&					\
	    ((بل) = TAILQ_بل(var, ډګر), 1); (var) = (بل))

#تعریف_کړه	TAILQ_FOREACH_سرچپه(var, سر_2, headname, ډګر)		\
	لكل ((var) = (*(((بنية headname *)((سر_2)->tqh_وروستی))->tqh_وروستی));\
	    (var) != TAILQ_پای_ټکی(سر_2);					\
	    (var) = (*(((بنية headname *)((var)->ډګر.tqe_prev))->tqh_وروستی)))

#تعریف_کړه	TAILQ_FOREACH_سرچپه_خوندي(var, سر_2, headname, ډګر, prev)	\
	لكل ((var) = TAILQ_وروستی((سر_2), headname);			\
	    (var) != TAILQ_پای_ټکی(سر_2) && 				\
	    ((prev) = TAILQ_PREV((var), headname, ډګر), 1); (var) = (prev))

/*
 * Tail queue functions.
 */
#إذا defined(QUEUEDEBUG)
#تعریف_کړه	QUEUEDEBUG_TAILQ_ورننباسه_سر(سر_2, elm, ډګر)			\
	إذا ((سر_2)->tqh_لومړی &&					\
	    (سر_2)->tqh_لومړی->ډګر.tqe_prev != &(سر_2)->tqh_لومړی)	\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_HEAD %p %s:%d", (سر_2),	\
		    __FILE__, __LINE__);
#تعریف_کړه	QUEUEDEBUG_TAILQ_ورننباسه_لکۍ(سر_2, elm, ډګر)			\
	إذا (*(سر_2)->tqh_وروستی != عدم)					\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_TAIL %p %s:%d", (سر_2),	\
		    __FILE__, __LINE__);
#تعریف_کړه	QUEUEDEBUG_TAILQ_OP(elm, ډګر)					\
	إذا ((elm)->ډګر.tqe_بل &&					\
	    (elm)->ډګر.tqe_بل->ډګر.tqe_prev !=			\
	    &(elm)->ډګر.tqe_بل)					\
		QUEUEDEBUG_ABORT("TAILQ_* forw %p %s:%d", (elm),	\
		    __FILE__, __LINE__);				\
	إذا (*(elm)->ډګر.tqe_prev != (elm))				\
		QUEUEDEBUG_ABORT("TAILQ_* back %p %s:%d", (elm),	\
		    __FILE__, __LINE__);
#تعریف_کړه	QUEUEDEBUG_TAILQ_PREREMOVE(سر_2, elm, ډګر)			\
	إذا ((elm)->ډګر.tqe_بل == عدم &&				\
	    (سر_2)->tqh_وروستی != &(elm)->ډګر.tqe_بل)			\
		QUEUEDEBUG_ABORT("TAILQ_PREREMOVE head %p elm %p %s:%d",\
		    (سر_2), (elm), __FILE__, __LINE__);
#تعریف_کړه	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, ډګر)				\
	(elm)->ډګر.tqe_بل = (فراغ *)1L;				\
	(elm)->ډګر.tqe_prev = (فراغ *)1L;
#وإلا
#تعریف_کړه	QUEUEDEBUG_TAILQ_ورننباسه_سر(سر_2, elm, ډګر)
#تعریف_کړه	QUEUEDEBUG_TAILQ_ورننباسه_لکۍ(سر_2, elm, ډګر)
#تعریف_کړه	QUEUEDEBUG_TAILQ_OP(elm, ډګر)
#تعریف_کړه	QUEUEDEBUG_TAILQ_PREREMOVE(سر_2, elm, ډګر)
#تعریف_کړه	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, ډګر)
#د_که_پای

#تعریف_کړه	TAILQ_INIT(سر_2) افعل {						\
	(سر_2)->tqh_لومړی = TAILQ_پای_ټکی(سر_2);				\
	(سر_2)->tqh_وروستی = &(سر_2)->tqh_لومړی;				\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	TAILQ_ورننباسه_سر(سر_2, elm, ډګر) افعل {			\
	QUEUEDEBUG_TAILQ_ورننباسه_سر((سر_2), (elm), ډګر)		\
	إذا (((elm)->ډګر.tqe_بل = (سر_2)->tqh_لومړی) != TAILQ_پای_ټکی(سر_2))\
		(سر_2)->tqh_لومړی->ډګر.tqe_prev =			\
		    &(elm)->ډګر.tqe_بل;				\
	وإلا								\
		(سر_2)->tqh_وروستی = &(elm)->ډګر.tqe_بل;		\
	(سر_2)->tqh_لومړی = (elm);					\
	(elm)->ډګر.tqe_prev = &(سر_2)->tqh_لومړی;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	TAILQ_ورننباسه_لکۍ(سر_2, elm, ډګر) افعل {			\
	QUEUEDEBUG_TAILQ_ورننباسه_لکۍ((سر_2), (elm), ډګر)		\
	(elm)->ډګر.tqe_بل = TAILQ_پای_ټکی(سر_2);			\
	(elm)->ډګر.tqe_prev = (سر_2)->tqh_وروستی;			\
	*(سر_2)->tqh_وروستی = (elm);					\
	(سر_2)->tqh_وروستی = &(elm)->ډګر.tqe_بل;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	TAILQ_ورننباسه_AFTER(سر_2, listelm, elm, ډګر) افعل {		\
	QUEUEDEBUG_TAILQ_OP((listelm), ډګر)				\
	إذا (((elm)->ډګر.tqe_بل = (listelm)->ډګر.tqe_بل) != 	\
	    TAILQ_پای_ټکی(سر_2))						\
		(elm)->ډګر.tqe_بل->ډګر.tqe_prev = 		\
		    &(elm)->ډګر.tqe_بل;				\
	وإلا								\
		(سر_2)->tqh_وروستی = &(elm)->ډګر.tqe_بل;		\
	(listelm)->ډګر.tqe_بل = (elm);				\
	(elm)->ډګر.tqe_prev = &(listelm)->ډګر.tqe_بل;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	TAILQ_ورننباسه_BEFORE(listelm, elm, ډګر) افعل {			\
	QUEUEDEBUG_TAILQ_OP((listelm), ډګر)				\
	(elm)->ډګر.tqe_prev = (listelm)->ډګر.tqe_prev;		\
	(elm)->ډګر.tqe_بل = (listelm);				\
	*(listelm)->ډګر.tqe_prev = (elm);				\
	(listelm)->ډګر.tqe_prev = &(elm)->ډګر.tqe_بل;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	TAILQ_REMOVE(سر_2, elm, ډګر) افعل {				\
	QUEUEDEBUG_TAILQ_PREREMOVE((سر_2), (elm), ډګر)		\
	QUEUEDEBUG_TAILQ_OP((elm), ډګر)				\
	إذا (((elm)->ډګر.tqe_بل) != TAILQ_پای_ټکی(سر_2))			\
		(elm)->ډګر.tqe_بل->ډګر.tqe_prev = 		\
		    (elm)->ډګر.tqe_prev;				\
	وإلا								\
		(سر_2)->tqh_وروستی = (elm)->ډګر.tqe_prev;		\
	*(elm)->ډګر.tqe_prev = (elm)->ډګر.tqe_بل;			\
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), ډګر);			\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه TAILQ_REPLACE(سر_2, elm, elm2, ډګر) افعل {                      \
        إذا (((elm2)->ډګر.tqe_بل = (elm)->ډګر.tqe_بل) != 	\
	    TAILQ_پای_ټکی(سر_2))   						\
                (elm2)->ډګر.tqe_بل->ډګر.tqe_prev =                \
                    &(elm2)->ډګر.tqe_بل;                            \
        وإلا                                                            \
                (سر_2)->tqh_وروستی = &(elm2)->ډګر.tqe_بل;             \
        (elm2)->ډګر.tqe_prev = (elm)->ډګر.tqe_prev;                 \
        *(elm2)->ډګر.tqe_prev = (elm2);                               \
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), ډګر);			\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	TAILQ_CONCAT(head1, head2, ډګر) افعل {				\
	إذا (!TAILQ_تشه(head2)) {					\
		*(head1)->tqh_وروستی = (head2)->tqh_لومړی;		\
		(head2)->tqh_لومړی->ډګر.tqe_prev = (head1)->tqh_وروستی;	\
		(head1)->tqh_وروستی = (head2)->tqh_وروستی;			\
		TAILQ_INIT((head2));					\
	}								\
} طالما (/*CONSTCOND*/0)

/*
 * Singly-linked Tail queue declarations.
 */
#تعریف_کړه	STAILQ_سر(name, type)						\
بنية name {								\
	بنية type *stqh_لومړی;	/* first element */		\
	بنية type **stqh_وروستی;	/* addr of last next element */	\
}

#تعریف_کړه	STAILQ_سر_INITIALIZER(سر_2)					\
	{ عدم, &(سر_2).stqh_لومړی }

#تعریف_کړه	STAILQ_ثبت(type)						\
بنية {								\
	بنية type *stqe_بل;	/* next element */			\
}

/*
 * Singly-linked Tail queue access methods.
 */
#تعریف_کړه	STAILQ_لومړی(سر_2)	((سر_2)->stqh_لومړی)
#تعریف_کړه	STAILQ_پای_ټکی(سر_2)	عدم
#تعریف_کړه	STAILQ_بل(elm, ډګر)	((elm)->ډګر.stqe_بل)
#تعریف_کړه	STAILQ_تشه(سر_2)	(STAILQ_لومړی(سر_2) == STAILQ_پای_ټکی(سر_2))

/*
 * Singly-linked Tail queue functions.
 */
#تعریف_کړه	STAILQ_INIT(سر_2) افعل {						\
	(سر_2)->stqh_لومړی = عدم;					\
	(سر_2)->stqh_وروستی = &(سر_2)->stqh_لومړی;				\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	STAILQ_ورننباسه_سر(سر_2, elm, ډګر) افعل {			\
	إذا (((elm)->ډګر.stqe_بل = (سر_2)->stqh_لومړی) == عدم)	\
		(سر_2)->stqh_وروستی = &(elm)->ډګر.stqe_بل;		\
	(سر_2)->stqh_لومړی = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	STAILQ_ورننباسه_لکۍ(سر_2, elm, ډګر) افعل {			\
	(elm)->ډګر.stqe_بل = عدم;					\
	*(سر_2)->stqh_وروستی = (elm);					\
	(سر_2)->stqh_وروستی = &(elm)->ډګر.stqe_بل;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	STAILQ_ورننباسه_AFTER(سر_2, listelm, elm, ډګر) افعل {		\
	إذا (((elm)->ډګر.stqe_بل = (listelm)->ډګر.stqe_بل) == عدم)\
		(سر_2)->stqh_وروستی = &(elm)->ډګر.stqe_بل;		\
	(listelm)->ډګر.stqe_بل = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	STAILQ_REMOVE_سر(سر_2, ډګر) افعل {				\
	إذا (((سر_2)->stqh_لومړی = (سر_2)->stqh_لومړی->ډګر.stqe_بل) == عدم) \
		(سر_2)->stqh_وروستی = &(سر_2)->stqh_لومړی;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	STAILQ_REMOVE(سر_2, elm, type, ډګر) افعل {			\
	إذا ((سر_2)->stqh_لومړی == (elm)) {				\
		STAILQ_REMOVE_سر((سر_2), ډګر);			\
	} وإلا {							\
		بنية type *curelm = (سر_2)->stqh_لومړی;		\
		طالما (curelm->ډګر.stqe_بل != (elm))			\
			curelm = curelm->ډګر.stqe_بل;		\
		إذا ((curelm->ډګر.stqe_بل =				\
			curelm->ډګر.stqe_بل->ډګر.stqe_بل) == عدم) \
			    (سر_2)->stqh_وروستی = &(curelm)->ډګر.stqe_بل; \
	}								\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	STAILQ_FOREACH(var, سر_2, ډګر)				\
	لكل ((var) = ((سر_2)->stqh_لومړی);				\
		(var);							\
		(var) = ((var)->ډګر.stqe_بل))

#تعریف_کړه	STAILQ_FOREACH_خوندي(var, سر_2, ډګر, tvar)			\
	لكل ((var) = STAILQ_لومړی((سر_2));				\
	    (var) && ((tvar) = STAILQ_بل((var), ډګر), 1);		\
	    (var) = (tvar))

#تعریف_کړه	STAILQ_CONCAT(head1, head2) افعل {				\
	إذا (!STAILQ_تشه((head2))) {					\
		*(head1)->stqh_وروستی = (head2)->stqh_لومړی;		\
		(head1)->stqh_وروستی = (head2)->stqh_وروستی;		\
		STAILQ_INIT((head2));					\
	}								\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	STAILQ_وروستی(سر_2, type, ډګر)					\
	(STAILQ_تشه((سر_2)) ?						\
		عدم :							\
	        ((بنية type *)(فراغ *)				\
		((حرف *)((سر_2)->stqh_وروستی) - offsetof(بنية type, ډګر))))


#که_نه_تعریف_شوی _KERNEL
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
ساكن __inline ثابت فراغ * __launder_type(ثابت فراغ *);
ساكن __inline ثابت فراغ *
__launder_type(ثابت فراغ *__x)
{
	__asm __volatile("" : "+r" (__x));
	أرجع __x;
}

#إذا defined(QUEUEDEBUG)
#تعریف_کړه QUEUEDEBUG_CIRCLEQ_سر(سر_2, ډګر)				\
	إذا ((سر_2)->cqh_لومړی != CIRCLEQ_ENDC(سر_2) &&			\
	    (سر_2)->cqh_لومړی->ډګر.cqe_prev != CIRCLEQ_ENDC(سر_2))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head forw %p %s:%d", (سر_2),	\
		      __FILE__, __LINE__);				\
	إذا ((سر_2)->cqh_وروستی != CIRCLEQ_ENDC(سر_2) &&			\
	    (سر_2)->cqh_وروستی->ډګر.cqe_بل != CIRCLEQ_ENDC(سر_2))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head back %p %s:%d", (سر_2),	\
		      __FILE__, __LINE__);
#تعریف_کړه QUEUEDEBUG_CIRCLEQ_ELM(سر_2, elm, ډګر)			\
	إذا ((elm)->ډګر.cqe_بل == CIRCLEQ_ENDC(سر_2)) {		\
		إذا ((سر_2)->cqh_وروستی != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm last %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} وإلا {							\
		إذا ((elm)->ډګر.cqe_بل->ډګر.cqe_prev != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm forw %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}								\
	إذا ((elm)->ډګر.cqe_prev == CIRCLEQ_ENDC(سر_2)) {		\
		إذا ((سر_2)->cqh_لومړی != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm first %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} وإلا {							\
		إذا ((elm)->ډګر.cqe_prev->ډګر.cqe_بل != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm prev %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}
#تعریف_کړه QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, ډګر)			\
	(elm)->ډګر.cqe_بل = (فراغ *)1L;				\
	(elm)->ډګر.cqe_prev = (فراغ *)1L;
#وإلا
#تعریف_کړه QUEUEDEBUG_CIRCLEQ_سر(سر_2, ډګر)
#تعریف_کړه QUEUEDEBUG_CIRCLEQ_ELM(سر_2, elm, ډګر)
#تعریف_کړه QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, ډګر)
#د_که_پای

#تعریف_کړه	CIRCLEQ_سر(name, type)					\
بنية name {								\
	بنية type *cqh_لومړی;		/* first element */		\
	بنية type *cqh_وروستی;		/* last element */		\
}

#تعریف_کړه	CIRCLEQ_سر_INITIALIZER(سر_2)					\
	{ CIRCLEQ_پای_ټکی(&سر_2), CIRCLEQ_پای_ټکی(&سر_2) }

#تعریف_کړه	CIRCLEQ_ثبت(type)						\
بنية {								\
	بنية type *cqe_بل;		/* next element */		\
	بنية type *cqe_prev;		/* previous element */		\
}

/*
 * Circular queue functions.
 */
#تعریف_کړه	CIRCLEQ_INIT(سر_2) افعل {						\
	(سر_2)->cqh_لومړی = CIRCLEQ_پای_ټکی(سر_2);				\
	(سر_2)->cqh_وروستی = CIRCLEQ_پای_ټکی(سر_2);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	CIRCLEQ_ورننباسه_AFTER(سر_2, listelm, elm, ډګر) افعل {		\
	QUEUEDEBUG_CIRCLEQ_سر((سر_2), ډګر)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر_2), (listelm), ډګر)		\
	(elm)->ډګر.cqe_بل = (listelm)->ډګر.cqe_بل;		\
	(elm)->ډګر.cqe_prev = (listelm);				\
	إذا ((listelm)->ډګر.cqe_بل == CIRCLEQ_ENDC(سر_2))		\
		(سر_2)->cqh_وروستی = (elm);				\
	وإلا								\
		(listelm)->ډګر.cqe_بل->ډګر.cqe_prev = (elm);	\
	(listelm)->ډګر.cqe_بل = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	CIRCLEQ_ورننباسه_BEFORE(سر_2, listelm, elm, ډګر) افعل {		\
	QUEUEDEBUG_CIRCLEQ_سر((سر_2), ډګر)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر_2), (listelm), ډګر)		\
	(elm)->ډګر.cqe_بل = (listelm);				\
	(elm)->ډګر.cqe_prev = (listelm)->ډګر.cqe_prev;		\
	إذا ((listelm)->ډګر.cqe_prev == CIRCLEQ_ENDC(سر_2))		\
		(سر_2)->cqh_لومړی = (elm);				\
	وإلا								\
		(listelm)->ډګر.cqe_prev->ډګر.cqe_بل = (elm);	\
	(listelm)->ډګر.cqe_prev = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	CIRCLEQ_ورننباسه_سر(سر_2, elm, ډګر) افعل {			\
	QUEUEDEBUG_CIRCLEQ_سر((سر_2), ډګر)				\
	(elm)->ډګر.cqe_بل = (سر_2)->cqh_لومړی;			\
	(elm)->ډګر.cqe_prev = CIRCLEQ_پای_ټکی(سر_2);			\
	إذا ((سر_2)->cqh_وروستی == CIRCLEQ_ENDC(سر_2))			\
		(سر_2)->cqh_وروستی = (elm);				\
	وإلا								\
		(سر_2)->cqh_لومړی->ډګر.cqe_prev = (elm);		\
	(سر_2)->cqh_لومړی = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	CIRCLEQ_ورننباسه_لکۍ(سر_2, elm, ډګر) افعل {			\
	QUEUEDEBUG_CIRCLEQ_سر((سر_2), ډګر)				\
	(elm)->ډګر.cqe_بل = CIRCLEQ_پای_ټکی(سر_2);			\
	(elm)->ډګر.cqe_prev = (سر_2)->cqh_وروستی;			\
	إذا ((سر_2)->cqh_لومړی == CIRCLEQ_ENDC(سر_2))			\
		(سر_2)->cqh_لومړی = (elm);				\
	وإلا								\
		(سر_2)->cqh_وروستی->ډګر.cqe_بل = (elm);		\
	(سر_2)->cqh_وروستی = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	CIRCLEQ_REMOVE(سر_2, elm, ډګر) افعل {				\
	QUEUEDEBUG_CIRCLEQ_سر((سر_2), ډګر)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر_2), (elm), ډګر)			\
	إذا ((elm)->ډګر.cqe_بل == CIRCLEQ_ENDC(سر_2))		\
		(سر_2)->cqh_وروستی = (elm)->ډګر.cqe_prev;		\
	وإلا								\
		(elm)->ډګر.cqe_بل->ډګر.cqe_prev =			\
		    (elm)->ډګر.cqe_prev;				\
	إذا ((elm)->ډګر.cqe_prev == CIRCLEQ_ENDC(سر_2))		\
		(سر_2)->cqh_لومړی = (elm)->ډګر.cqe_بل;		\
	وإلا								\
		(elm)->ډګر.cqe_prev->ډګر.cqe_بل =			\
		    (elm)->ډګر.cqe_بل;				\
	QUEUEDEBUG_CIRCLEQ_POSTREMOVE((elm), ډګر)			\
} طالما (/*CONSTCOND*/0)

#تعریف_کړه	CIRCLEQ_FOREACH(var, سر_2, ډګر)				\
	لكل ((var) = ((سر_2)->cqh_لومړی);				\
		(var) != CIRCLEQ_ENDC(سر_2);				\
		(var) = ((var)->ډګر.cqe_بل))

#تعریف_کړه	CIRCLEQ_FOREACH_سرچپه(var, سر_2, ډګر)			\
	لكل ((var) = ((سر_2)->cqh_وروستی);				\
		(var) != CIRCLEQ_ENDC(سر_2);				\
		(var) = ((var)->ډګر.cqe_prev))

/*
 * Circular queue access methods.
 */
#تعریف_کړه	CIRCLEQ_لومړی(سر_2)		((سر_2)->cqh_لومړی)
#تعریف_کړه	CIRCLEQ_وروستی(سر_2)		((سر_2)->cqh_وروستی)
/* For comparisons */
#تعریف_کړه	CIRCLEQ_ENDC(سر_2)		(__launder_type(سر_2))
/* For assignments */
#تعریف_کړه	CIRCLEQ_پای_ټکی(سر_2)		((فراغ *)(سر_2))
#تعریف_کړه	CIRCLEQ_بل(elm, ډګر)	((elm)->ډګر.cqe_بل)
#تعریف_کړه	CIRCLEQ_PREV(elm, ډګر)	((elm)->ډګر.cqe_prev)
#تعریف_کړه	CIRCLEQ_تشه(سر_2)						\
    (CIRCLEQ_لومړی(سر_2) == CIRCLEQ_ENDC(سر_2))

#تعریف_کړه CIRCLEQ_LOOP_بل(سر_2, elm, ډګر)				\
	(((elm)->ډګر.cqe_بل == CIRCLEQ_ENDC(سر_2))			\
	    ? ((سر_2)->cqh_لومړی)					\
	    : (elm->ډګر.cqe_بل))
#تعریف_کړه CIRCLEQ_LOOP_PREV(سر_2, elm, ډګر)				\
	(((elm)->ډګر.cqe_prev == CIRCLEQ_ENDC(سر_2))			\
	    ? ((سر_2)->cqh_وروستی)					\
	    : (elm->ډګر.cqe_prev))
#د_که_پای /* !_KERNEL */

#د_که_پای	/* !_SYS_QUEUE_H_ */
