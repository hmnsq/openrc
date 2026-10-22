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

#جيڪڏهن_وصف_نه_ٿيل	SYS_قطار_H
#وصف_ڪريو	SYS_قطار_H

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
#جيڪڏهن_وصف_ٿيل __NetBSD__
#ضمن <sys/null.h>
#جيڪڏهن_پورو

#إذا defined(QUEUEDEBUG)
# إذا defined(_KERNEL)
#  define QUEUEDEBUG_ABORT(...) panic(__VA_ARGS__)
# وإلا
#  include <err.h>
#  define QUEUEDEBUG_ABORT(...) err(1, __VA_ARGS__)
# endif
#جيڪڏهن_پورو

/*
 * Singly-linked List definitions.
 */
#وصف_ڪريو	SLIST_سر(name, type)						\
بنية name {								\
	بنية type *slh_پهريون;	/* first element */			\
}

#وصف_ڪريو	SLIST_سر_INITIALIZER(سر)					\
	{ عدم }

#وصف_ڪريو	SLIST_داخلا(type)						\
بنية {								\
	بنية type *sle_ايندڙ;	/* next element */			\
}

/*
 * Singly-linked List access methods.
 */
#وصف_ڪريو	SLIST_پهريون(سر)	((سر)->slh_پهريون)
#وصف_ڪريو	SLIST_پڇاڙي(سر)		عدم
#وصف_ڪريو	SLIST_خالي_جايون(سر)	((سر)->slh_پهريون == عدم)
#وصف_ڪريو	SLIST_ايندڙ(elm, ميدان)	((elm)->ميدان.sle_ايندڙ)

#وصف_ڪريو	SLIST_FOREACH(var, سر, ميدان)					\
	لكل((var) = (سر)->slh_پهريون;					\
	    (var) != SLIST_پڇاڙي(سر);					\
	    (var) = (var)->ميدان.sle_ايندڙ)

#وصف_ڪريو	SLIST_FOREACH_محفوظ(var, سر, ميدان, tvar)			\
	لكل ((var) = SLIST_پهريون((سر));				\
	    (var) != SLIST_پڇاڙي(سر) &&					\
	    ((tvar) = SLIST_ايندڙ((var), ميدان), 1);			\
	    (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#وصف_ڪريو	SLIST_INIT(سر) افعل {						\
	(سر)->slh_پهريون = SLIST_پڇاڙي(سر);				\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	SLIST_وجهو_AFTER(slistelm, elm, ميدان) افعل {			\
	(elm)->ميدان.sle_ايندڙ = (slistelm)->ميدان.sle_ايندڙ;		\
	(slistelm)->ميدان.sle_ايندڙ = (elm);				\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	SLIST_وجهو_سر(سر, elm, ميدان) افعل {			\
	(elm)->ميدان.sle_ايندڙ = (سر)->slh_پهريون;			\
	(سر)->slh_پهريون = (elm);					\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	SLIST_REMOVE_AFTER(slistelm, ميدان) افعل {			\
	(slistelm)->ميدان.sle_ايندڙ =					\
	    SLIST_ايندڙ(SLIST_ايندڙ((slistelm), ميدان), ميدان);		\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	SLIST_REMOVE_سر(سر, ميدان) افعل {				\
	(سر)->slh_پهريون = (سر)->slh_پهريون->ميدان.sle_ايندڙ;		\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	SLIST_REMOVE(سر, elm, type, ميدان) افعل {			\
	إذا ((سر)->slh_پهريون == (elm)) {				\
		SLIST_REMOVE_سر((سر), ميدان);			\
	}								\
	وإلا {								\
		بنية type *curelm = (سر)->slh_پهريون;		\
		طالما(curelm->ميدان.sle_ايندڙ != (elm))			\
			curelm = curelm->ميدان.sle_ايندڙ;		\
		curelm->ميدان.sle_ايندڙ =				\
		    curelm->ميدان.sle_ايندڙ->ميدان.sle_ايندڙ;		\
	}								\
} طالما (/*CONSTCOND*/0)


/*
 * List definitions.
 */
#وصف_ڪريو	LIST_سر(name, type)						\
بنية name {								\
	بنية type *lh_پهريون;	/* first element */			\
}

#وصف_ڪريو	LIST_سر_INITIALIZER(سر)					\
	{ عدم }

#وصف_ڪريو	LIST_داخلا(type)						\
بنية {								\
	بنية type *le_ايندڙ;	/* next element */			\
	بنية type **le_prev;	/* address of previous next element */	\
}

/*
 * List access methods.
 */
#وصف_ڪريو	LIST_پهريون(سر)		((سر)->lh_پهريون)
#وصف_ڪريو	LIST_پڇاڙي(سر)			عدم
#وصف_ڪريو	LIST_خالي_جايون(سر)		((سر)->lh_پهريون == LIST_پڇاڙي(سر))
#وصف_ڪريو	LIST_ايندڙ(elm, ميدان)		((elm)->ميدان.le_ايندڙ)

#وصف_ڪريو	LIST_FOREACH(var, سر, ميدان)					\
	لكل ((var) = ((سر)->lh_پهريون);				\
	    (var) != LIST_پڇاڙي(سر);					\
	    (var) = ((var)->ميدان.le_ايندڙ))

#وصف_ڪريو	LIST_FOREACH_محفوظ(var, سر, ميدان, tvar)			\
	لكل ((var) = LIST_پهريون((سر));				\
	    (var) != LIST_پڇاڙي(سر) &&					\
	    ((tvar) = LIST_ايندڙ((var), ميدان), 1);			\
	    (var) = (tvar))

#وصف_ڪريو	LIST_MOVE(head1, head2) افعل {					\
	LIST_INIT((head2));						\
	إذا (!LIST_خالي_جايون((head1))) {					\
		(head2)->lh_پهريون = (head1)->lh_پهريون;			\
		LIST_INIT((head1));					\
	}								\
} طالما (/*CONSTCOND*/0)

/*
 * List functions.
 */
#إذا defined(QUEUEDEBUG)
#وصف_ڪريو	QUEUEDEBUG_LIST_وجهو_سر(سر, elm, ميدان)			\
	إذا ((سر)->lh_پهريون &&						\
	    (سر)->lh_پهريون->ميدان.le_prev != &(سر)->lh_پهريون)	\
		QUEUEDEBUG_ABORT("LIST_INSERT_HEAD %p %s:%d", (سر),	\
		    __FILE__, __LINE__);
#وصف_ڪريو	QUEUEDEBUG_LIST_OP(elm, ميدان)					\
	إذا ((elm)->ميدان.le_ايندڙ &&					\
	    (elm)->ميدان.le_ايندڙ->ميدان.le_prev !=			\
	    &(elm)->ميدان.le_ايندڙ)					\
		QUEUEDEBUG_ABORT("LIST_* forw %p %s:%d", (elm),		\
		    __FILE__, __LINE__);				\
	إذا (*(elm)->ميدان.le_prev != (elm))				\
		QUEUEDEBUG_ABORT("LIST_* back %p %s:%d", (elm),		\
		    __FILE__, __LINE__);
#وصف_ڪريو	QUEUEDEBUG_LIST_POSTREMOVE(elm, ميدان)				\
	(elm)->ميدان.le_ايندڙ = (فراغ *)1L;				\
	(elm)->ميدان.le_prev = (فراغ *)1L;
#وإلا
#وصف_ڪريو	QUEUEDEBUG_LIST_وجهو_سر(سر, elm, ميدان)
#وصف_ڪريو	QUEUEDEBUG_LIST_OP(elm, ميدان)
#وصف_ڪريو	QUEUEDEBUG_LIST_POSTREMOVE(elm, ميدان)
#جيڪڏهن_پورو

#وصف_ڪريو	LIST_INIT(سر) افعل {						\
	(سر)->lh_پهريون = LIST_پڇاڙي(سر);				\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	LIST_وجهو_AFTER(listelm, elm, ميدان) افعل {			\
	QUEUEDEBUG_LIST_OP((listelm), ميدان)				\
	إذا (((elm)->ميدان.le_ايندڙ = (listelm)->ميدان.le_ايندڙ) != 	\
	    LIST_پڇاڙي(سر))						\
		(listelm)->ميدان.le_ايندڙ->ميدان.le_prev =		\
		    &(elm)->ميدان.le_ايندڙ;				\
	(listelm)->ميدان.le_ايندڙ = (elm);				\
	(elm)->ميدان.le_prev = &(listelm)->ميدان.le_ايندڙ;		\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	LIST_وجهو_BEFORE(listelm, elm, ميدان) افعل {			\
	QUEUEDEBUG_LIST_OP((listelm), ميدان)				\
	(elm)->ميدان.le_prev = (listelm)->ميدان.le_prev;		\
	(elm)->ميدان.le_ايندڙ = (listelm);				\
	*(listelm)->ميدان.le_prev = (elm);				\
	(listelm)->ميدان.le_prev = &(elm)->ميدان.le_ايندڙ;		\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	LIST_وجهو_سر(سر, elm, ميدان) افعل {				\
	QUEUEDEBUG_LIST_وجهو_سر((سر), (elm), ميدان)		\
	إذا (((elm)->ميدان.le_ايندڙ = (سر)->lh_پهريون) != LIST_پڇاڙي(سر))\
		(سر)->lh_پهريون->ميدان.le_prev = &(elm)->ميدان.le_ايندڙ;\
	(سر)->lh_پهريون = (elm);					\
	(elm)->ميدان.le_prev = &(سر)->lh_پهريون;			\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	LIST_REMOVE(elm, ميدان) افعل {					\
	QUEUEDEBUG_LIST_OP((elm), ميدان)				\
	إذا ((elm)->ميدان.le_ايندڙ != عدم)				\
		(elm)->ميدان.le_ايندڙ->ميدان.le_prev = 			\
		    (elm)->ميدان.le_prev;				\
	*(elm)->ميدان.le_prev = (elm)->ميدان.le_ايندڙ;			\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), ميدان)			\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو LIST_REPLACE(elm, elm2, ميدان) افعل {				\
	إذا (((elm2)->ميدان.le_ايندڙ = (elm)->ميدان.le_ايندڙ) != عدم)	\
		(elm2)->ميدان.le_ايندڙ->ميدان.le_prev =			\
		    &(elm2)->ميدان.le_ايندڙ;				\
	(elm2)->ميدان.le_prev = (elm)->ميدان.le_prev;			\
	*(elm2)->ميدان.le_prev = (elm2);				\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), ميدان)			\
} طالما (/*CONSTCOND*/0)

/*
 * Simple queue definitions.
 */
#وصف_ڪريو	SIMPLEQ_سر(name, type)					\
بنية name {								\
	بنية type *sqh_پهريون;	/* first element */			\
	بنية type **sqh_آخري;	/* addr of last next element */		\
}

#وصف_ڪريو	SIMPLEQ_سر_INITIALIZER(سر)					\
	{ عدم, &(سر).sqh_پهريون }

#وصف_ڪريو	SIMPLEQ_داخلا(type)						\
بنية {								\
	بنية type *sqe_ايندڙ;	/* next element */			\
}

/*
 * Simple queue access methods.
 */
#وصف_ڪريو	SIMPLEQ_پهريون(سر)		((سر)->sqh_پهريون)
#وصف_ڪريو	SIMPLEQ_پڇاڙي(سر)		عدم
#وصف_ڪريو	SIMPLEQ_خالي_جايون(سر)		((سر)->sqh_پهريون == SIMPLEQ_پڇاڙي(سر))
#وصف_ڪريو	SIMPLEQ_ايندڙ(elm, ميدان)	((elm)->ميدان.sqe_ايندڙ)

#وصف_ڪريو	SIMPLEQ_FOREACH(var, سر, ميدان)				\
	لكل ((var) = ((سر)->sqh_پهريون);				\
	    (var) != SIMPLEQ_پڇاڙي(سر);					\
	    (var) = ((var)->ميدان.sqe_ايندڙ))

#وصف_ڪريو	SIMPLEQ_FOREACH_محفوظ(var, سر, ميدان, ايندڙ)			\
	لكل ((var) = ((سر)->sqh_پهريون);				\
	    (var) != SIMPLEQ_پڇاڙي(سر) &&				\
	    ((ايندڙ = ((var)->ميدان.sqe_ايندڙ)), 1);			\
	    (var) = (ايندڙ))

/*
 * Simple queue functions.
 */
#وصف_ڪريو	SIMPLEQ_INIT(سر) افعل {						\
	(سر)->sqh_پهريون = عدم;					\
	(سر)->sqh_آخري = &(سر)->sqh_پهريون;				\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	SIMPLEQ_وجهو_سر(سر, elm, ميدان) افعل {			\
	إذا (((elm)->ميدان.sqe_ايندڙ = (سر)->sqh_پهريون) == عدم)	\
		(سر)->sqh_آخري = &(elm)->ميدان.sqe_ايندڙ;		\
	(سر)->sqh_پهريون = (elm);					\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	SIMPLEQ_وجهو_پڇ(سر, elm, ميدان) افعل {			\
	(elm)->ميدان.sqe_ايندڙ = عدم;					\
	*(سر)->sqh_آخري = (elm);					\
	(سر)->sqh_آخري = &(elm)->ميدان.sqe_ايندڙ;			\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	SIMPLEQ_وجهو_AFTER(سر, listelm, elm, ميدان) افعل {		\
	إذا (((elm)->ميدان.sqe_ايندڙ = (listelm)->ميدان.sqe_ايندڙ) == عدم)\
		(سر)->sqh_آخري = &(elm)->ميدان.sqe_ايندڙ;		\
	(listelm)->ميدان.sqe_ايندڙ = (elm);				\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	SIMPLEQ_REMOVE_سر(سر, ميدان) افعل {				\
	إذا (((سر)->sqh_پهريون = (سر)->sqh_پهريون->ميدان.sqe_ايندڙ) == عدم) \
		(سر)->sqh_آخري = &(سر)->sqh_پهريون;			\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو SIMPLEQ_REMOVE_AFTER(سر, elm, ميدان) افعل {			\
	إذا (((elm)->ميدان.sqe_ايندڙ = (elm)->ميدان.sqe_ايندڙ->ميدان.sqe_ايندڙ) \
	    == عدم)							\
		(سر)->sqh_آخري = &(elm)->ميدان.sqe_ايندڙ;		\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	SIMPLEQ_REMOVE(سر, elm, type, ميدان) افعل {			\
	إذا ((سر)->sqh_پهريون == (elm)) {				\
		SIMPLEQ_REMOVE_سر((سر), ميدان);			\
	} وإلا {							\
		بنية type *curelm = (سر)->sqh_پهريون;		\
		طالما (curelm->ميدان.sqe_ايندڙ != (elm))			\
			curelm = curelm->ميدان.sqe_ايندڙ;		\
		إذا ((curelm->ميدان.sqe_ايندڙ =				\
			curelm->ميدان.sqe_ايندڙ->ميدان.sqe_ايندڙ) == عدم) \
			    (سر)->sqh_آخري = &(curelm)->ميدان.sqe_ايندڙ; \
	}								\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	SIMPLEQ_CONCAT(head1, head2) افعل {				\
	إذا (!SIMPLEQ_خالي_جايون((head2))) {					\
		*(head1)->sqh_آخري = (head2)->sqh_پهريون;		\
		(head1)->sqh_آخري = (head2)->sqh_آخري;		\
		SIMPLEQ_INIT((head2));					\
	}								\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	SIMPLEQ_آخري(سر, type, ميدان)					\
	(SIMPLEQ_خالي_جايون((سر)) ?						\
		عدم :							\
	        ((بنية type *)(فراغ *)				\
		((حرف *)((سر)->sqh_آخري) - offsetof(بنية type, ميدان))))

/*
 * Tail queue definitions.
 */
#وصف_ڪريو	TAILQ_سر_2(name, type, qual)					\
بنية name {								\
	qual type *tqh_پهريون;		/* first element */		\
	qual type *qual *tqh_آخري;	/* addr of last next element */	\
}
#وصف_ڪريو TAILQ_سر(name, type)	TAILQ_سر_2(name, بنية type,)

#وصف_ڪريو	TAILQ_سر_INITIALIZER(سر)					\
	{ TAILQ_پڇاڙي(سر), &(سر).tqh_پهريون }

#وصف_ڪريو	TAILQ_داخلا_2(type, qual)					\
بنية {								\
	qual type *tqe_ايندڙ;		/* next element */		\
	qual type *qual *tqe_prev;	/* address of previous next element */\
}
#وصف_ڪريو TAILQ_داخلا(type)	TAILQ_داخلا_2(بنية type,)

/*
 * Tail queue access methods.
 */
#وصف_ڪريو	TAILQ_پهريون(سر)		((سر)->tqh_پهريون)
#وصف_ڪريو	TAILQ_پڇاڙي(سر)			(عدم)
#وصف_ڪريو	TAILQ_ايندڙ(elm, ميدان)		((elm)->ميدان.tqe_ايندڙ)
#وصف_ڪريو	TAILQ_آخري(سر, headname) \
	(*(((بنية headname *)((سر)->tqh_آخري))->tqh_آخري))
#وصف_ڪريو	TAILQ_PREV(elm, headname, ميدان) \
	(*(((بنية headname *)((elm)->ميدان.tqe_prev))->tqh_آخري))
#وصف_ڪريو	TAILQ_خالي_جايون(سر)		(TAILQ_پهريون(سر) == TAILQ_پڇاڙي(سر))


#وصف_ڪريو	TAILQ_FOREACH(var, سر, ميدان)					\
	لكل ((var) = ((سر)->tqh_پهريون);				\
	    (var) != TAILQ_پڇاڙي(سر);					\
	    (var) = ((var)->ميدان.tqe_ايندڙ))

#وصف_ڪريو	TAILQ_FOREACH_محفوظ(var, سر, ميدان, ايندڙ)			\
	لكل ((var) = ((سر)->tqh_پهريون);				\
	    (var) != TAILQ_پڇاڙي(سر) &&					\
	    ((ايندڙ) = TAILQ_ايندڙ(var, ميدان), 1); (var) = (ايندڙ))

#وصف_ڪريو	TAILQ_FOREACH_ابتو(var, سر, headname, ميدان)		\
	لكل ((var) = (*(((بنية headname *)((سر)->tqh_آخري))->tqh_آخري));\
	    (var) != TAILQ_پڇاڙي(سر);					\
	    (var) = (*(((بنية headname *)((var)->ميدان.tqe_prev))->tqh_آخري)))

#وصف_ڪريو	TAILQ_FOREACH_ابتو_محفوظ(var, سر, headname, ميدان, prev)	\
	لكل ((var) = TAILQ_آخري((سر), headname);			\
	    (var) != TAILQ_پڇاڙي(سر) && 				\
	    ((prev) = TAILQ_PREV((var), headname, ميدان), 1); (var) = (prev))

/*
 * Tail queue functions.
 */
#إذا defined(QUEUEDEBUG)
#وصف_ڪريو	QUEUEDEBUG_TAILQ_وجهو_سر(سر, elm, ميدان)			\
	إذا ((سر)->tqh_پهريون &&					\
	    (سر)->tqh_پهريون->ميدان.tqe_prev != &(سر)->tqh_پهريون)	\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_HEAD %p %s:%d", (سر),	\
		    __FILE__, __LINE__);
#وصف_ڪريو	QUEUEDEBUG_TAILQ_وجهو_پڇ(سر, elm, ميدان)			\
	إذا (*(سر)->tqh_آخري != عدم)					\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_TAIL %p %s:%d", (سر),	\
		    __FILE__, __LINE__);
#وصف_ڪريو	QUEUEDEBUG_TAILQ_OP(elm, ميدان)					\
	إذا ((elm)->ميدان.tqe_ايندڙ &&					\
	    (elm)->ميدان.tqe_ايندڙ->ميدان.tqe_prev !=			\
	    &(elm)->ميدان.tqe_ايندڙ)					\
		QUEUEDEBUG_ABORT("TAILQ_* forw %p %s:%d", (elm),	\
		    __FILE__, __LINE__);				\
	إذا (*(elm)->ميدان.tqe_prev != (elm))				\
		QUEUEDEBUG_ABORT("TAILQ_* back %p %s:%d", (elm),	\
		    __FILE__, __LINE__);
#وصف_ڪريو	QUEUEDEBUG_TAILQ_PREREMOVE(سر, elm, ميدان)			\
	إذا ((elm)->ميدان.tqe_ايندڙ == عدم &&				\
	    (سر)->tqh_آخري != &(elm)->ميدان.tqe_ايندڙ)			\
		QUEUEDEBUG_ABORT("TAILQ_PREREMOVE head %p elm %p %s:%d",\
		    (سر), (elm), __FILE__, __LINE__);
#وصف_ڪريو	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, ميدان)				\
	(elm)->ميدان.tqe_ايندڙ = (فراغ *)1L;				\
	(elm)->ميدان.tqe_prev = (فراغ *)1L;
#وإلا
#وصف_ڪريو	QUEUEDEBUG_TAILQ_وجهو_سر(سر, elm, ميدان)
#وصف_ڪريو	QUEUEDEBUG_TAILQ_وجهو_پڇ(سر, elm, ميدان)
#وصف_ڪريو	QUEUEDEBUG_TAILQ_OP(elm, ميدان)
#وصف_ڪريو	QUEUEDEBUG_TAILQ_PREREMOVE(سر, elm, ميدان)
#وصف_ڪريو	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, ميدان)
#جيڪڏهن_پورو

#وصف_ڪريو	TAILQ_INIT(سر) افعل {						\
	(سر)->tqh_پهريون = TAILQ_پڇاڙي(سر);				\
	(سر)->tqh_آخري = &(سر)->tqh_پهريون;				\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	TAILQ_وجهو_سر(سر, elm, ميدان) افعل {			\
	QUEUEDEBUG_TAILQ_وجهو_سر((سر), (elm), ميدان)		\
	إذا (((elm)->ميدان.tqe_ايندڙ = (سر)->tqh_پهريون) != TAILQ_پڇاڙي(سر))\
		(سر)->tqh_پهريون->ميدان.tqe_prev =			\
		    &(elm)->ميدان.tqe_ايندڙ;				\
	وإلا								\
		(سر)->tqh_آخري = &(elm)->ميدان.tqe_ايندڙ;		\
	(سر)->tqh_پهريون = (elm);					\
	(elm)->ميدان.tqe_prev = &(سر)->tqh_پهريون;			\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	TAILQ_وجهو_پڇ(سر, elm, ميدان) افعل {			\
	QUEUEDEBUG_TAILQ_وجهو_پڇ((سر), (elm), ميدان)		\
	(elm)->ميدان.tqe_ايندڙ = TAILQ_پڇاڙي(سر);			\
	(elm)->ميدان.tqe_prev = (سر)->tqh_آخري;			\
	*(سر)->tqh_آخري = (elm);					\
	(سر)->tqh_آخري = &(elm)->ميدان.tqe_ايندڙ;			\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	TAILQ_وجهو_AFTER(سر, listelm, elm, ميدان) افعل {		\
	QUEUEDEBUG_TAILQ_OP((listelm), ميدان)				\
	إذا (((elm)->ميدان.tqe_ايندڙ = (listelm)->ميدان.tqe_ايندڙ) != 	\
	    TAILQ_پڇاڙي(سر))						\
		(elm)->ميدان.tqe_ايندڙ->ميدان.tqe_prev = 		\
		    &(elm)->ميدان.tqe_ايندڙ;				\
	وإلا								\
		(سر)->tqh_آخري = &(elm)->ميدان.tqe_ايندڙ;		\
	(listelm)->ميدان.tqe_ايندڙ = (elm);				\
	(elm)->ميدان.tqe_prev = &(listelm)->ميدان.tqe_ايندڙ;		\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	TAILQ_وجهو_BEFORE(listelm, elm, ميدان) افعل {			\
	QUEUEDEBUG_TAILQ_OP((listelm), ميدان)				\
	(elm)->ميدان.tqe_prev = (listelm)->ميدان.tqe_prev;		\
	(elm)->ميدان.tqe_ايندڙ = (listelm);				\
	*(listelm)->ميدان.tqe_prev = (elm);				\
	(listelm)->ميدان.tqe_prev = &(elm)->ميدان.tqe_ايندڙ;		\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	TAILQ_REMOVE(سر, elm, ميدان) افعل {				\
	QUEUEDEBUG_TAILQ_PREREMOVE((سر), (elm), ميدان)		\
	QUEUEDEBUG_TAILQ_OP((elm), ميدان)				\
	إذا (((elm)->ميدان.tqe_ايندڙ) != TAILQ_پڇاڙي(سر))			\
		(elm)->ميدان.tqe_ايندڙ->ميدان.tqe_prev = 		\
		    (elm)->ميدان.tqe_prev;				\
	وإلا								\
		(سر)->tqh_آخري = (elm)->ميدان.tqe_prev;		\
	*(elm)->ميدان.tqe_prev = (elm)->ميدان.tqe_ايندڙ;			\
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), ميدان);			\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو TAILQ_REPLACE(سر, elm, elm2, ميدان) افعل {                      \
        إذا (((elm2)->ميدان.tqe_ايندڙ = (elm)->ميدان.tqe_ايندڙ) != 	\
	    TAILQ_پڇاڙي(سر))   						\
                (elm2)->ميدان.tqe_ايندڙ->ميدان.tqe_prev =                \
                    &(elm2)->ميدان.tqe_ايندڙ;                            \
        وإلا                                                            \
                (سر)->tqh_آخري = &(elm2)->ميدان.tqe_ايندڙ;             \
        (elm2)->ميدان.tqe_prev = (elm)->ميدان.tqe_prev;                 \
        *(elm2)->ميدان.tqe_prev = (elm2);                               \
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), ميدان);			\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	TAILQ_CONCAT(head1, head2, ميدان) افعل {				\
	إذا (!TAILQ_خالي_جايون(head2)) {					\
		*(head1)->tqh_آخري = (head2)->tqh_پهريون;		\
		(head2)->tqh_پهريون->ميدان.tqe_prev = (head1)->tqh_آخري;	\
		(head1)->tqh_آخري = (head2)->tqh_آخري;			\
		TAILQ_INIT((head2));					\
	}								\
} طالما (/*CONSTCOND*/0)

/*
 * Singly-linked Tail queue declarations.
 */
#وصف_ڪريو	STAILQ_سر(name, type)						\
بنية name {								\
	بنية type *stqh_پهريون;	/* first element */		\
	بنية type **stqh_آخري;	/* addr of last next element */	\
}

#وصف_ڪريو	STAILQ_سر_INITIALIZER(سر)					\
	{ عدم, &(سر).stqh_پهريون }

#وصف_ڪريو	STAILQ_داخلا(type)						\
بنية {								\
	بنية type *stqe_ايندڙ;	/* next element */			\
}

/*
 * Singly-linked Tail queue access methods.
 */
#وصف_ڪريو	STAILQ_پهريون(سر)	((سر)->stqh_پهريون)
#وصف_ڪريو	STAILQ_پڇاڙي(سر)	عدم
#وصف_ڪريو	STAILQ_ايندڙ(elm, ميدان)	((elm)->ميدان.stqe_ايندڙ)
#وصف_ڪريو	STAILQ_خالي_جايون(سر)	(STAILQ_پهريون(سر) == STAILQ_پڇاڙي(سر))

/*
 * Singly-linked Tail queue functions.
 */
#وصف_ڪريو	STAILQ_INIT(سر) افعل {						\
	(سر)->stqh_پهريون = عدم;					\
	(سر)->stqh_آخري = &(سر)->stqh_پهريون;				\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	STAILQ_وجهو_سر(سر, elm, ميدان) افعل {			\
	إذا (((elm)->ميدان.stqe_ايندڙ = (سر)->stqh_پهريون) == عدم)	\
		(سر)->stqh_آخري = &(elm)->ميدان.stqe_ايندڙ;		\
	(سر)->stqh_پهريون = (elm);					\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	STAILQ_وجهو_پڇ(سر, elm, ميدان) افعل {			\
	(elm)->ميدان.stqe_ايندڙ = عدم;					\
	*(سر)->stqh_آخري = (elm);					\
	(سر)->stqh_آخري = &(elm)->ميدان.stqe_ايندڙ;			\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	STAILQ_وجهو_AFTER(سر, listelm, elm, ميدان) افعل {		\
	إذا (((elm)->ميدان.stqe_ايندڙ = (listelm)->ميدان.stqe_ايندڙ) == عدم)\
		(سر)->stqh_آخري = &(elm)->ميدان.stqe_ايندڙ;		\
	(listelm)->ميدان.stqe_ايندڙ = (elm);				\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	STAILQ_REMOVE_سر(سر, ميدان) افعل {				\
	إذا (((سر)->stqh_پهريون = (سر)->stqh_پهريون->ميدان.stqe_ايندڙ) == عدم) \
		(سر)->stqh_آخري = &(سر)->stqh_پهريون;			\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	STAILQ_REMOVE(سر, elm, type, ميدان) افعل {			\
	إذا ((سر)->stqh_پهريون == (elm)) {				\
		STAILQ_REMOVE_سر((سر), ميدان);			\
	} وإلا {							\
		بنية type *curelm = (سر)->stqh_پهريون;		\
		طالما (curelm->ميدان.stqe_ايندڙ != (elm))			\
			curelm = curelm->ميدان.stqe_ايندڙ;		\
		إذا ((curelm->ميدان.stqe_ايندڙ =				\
			curelm->ميدان.stqe_ايندڙ->ميدان.stqe_ايندڙ) == عدم) \
			    (سر)->stqh_آخري = &(curelm)->ميدان.stqe_ايندڙ; \
	}								\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	STAILQ_FOREACH(var, سر, ميدان)				\
	لكل ((var) = ((سر)->stqh_پهريون);				\
		(var);							\
		(var) = ((var)->ميدان.stqe_ايندڙ))

#وصف_ڪريو	STAILQ_FOREACH_محفوظ(var, سر, ميدان, tvar)			\
	لكل ((var) = STAILQ_پهريون((سر));				\
	    (var) && ((tvar) = STAILQ_ايندڙ((var), ميدان), 1);		\
	    (var) = (tvar))

#وصف_ڪريو	STAILQ_CONCAT(head1, head2) افعل {				\
	إذا (!STAILQ_خالي_جايون((head2))) {					\
		*(head1)->stqh_آخري = (head2)->stqh_پهريون;		\
		(head1)->stqh_آخري = (head2)->stqh_آخري;		\
		STAILQ_INIT((head2));					\
	}								\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	STAILQ_آخري(سر, type, ميدان)					\
	(STAILQ_خالي_جايون((سر)) ?						\
		عدم :							\
	        ((بنية type *)(فراغ *)				\
		((حرف *)((سر)->stqh_آخري) - offsetof(بنية type, ميدان))))


#جيڪڏهن_وصف_نه_ٿيل _KERNEL
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
#وصف_ڪريو QUEUEDEBUG_CIRCLEQ_سر(سر, ميدان)				\
	إذا ((سر)->cqh_پهريون != CIRCLEQ_ENDC(سر) &&			\
	    (سر)->cqh_پهريون->ميدان.cqe_prev != CIRCLEQ_ENDC(سر))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head forw %p %s:%d", (سر),	\
		      __FILE__, __LINE__);				\
	إذا ((سر)->cqh_آخري != CIRCLEQ_ENDC(سر) &&			\
	    (سر)->cqh_آخري->ميدان.cqe_ايندڙ != CIRCLEQ_ENDC(سر))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head back %p %s:%d", (سر),	\
		      __FILE__, __LINE__);
#وصف_ڪريو QUEUEDEBUG_CIRCLEQ_ELM(سر, elm, ميدان)			\
	إذا ((elm)->ميدان.cqe_ايندڙ == CIRCLEQ_ENDC(سر)) {		\
		إذا ((سر)->cqh_آخري != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm last %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} وإلا {							\
		إذا ((elm)->ميدان.cqe_ايندڙ->ميدان.cqe_prev != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm forw %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}								\
	إذا ((elm)->ميدان.cqe_prev == CIRCLEQ_ENDC(سر)) {		\
		إذا ((سر)->cqh_پهريون != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm first %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} وإلا {							\
		إذا ((elm)->ميدان.cqe_prev->ميدان.cqe_ايندڙ != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm prev %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}
#وصف_ڪريو QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, ميدان)			\
	(elm)->ميدان.cqe_ايندڙ = (فراغ *)1L;				\
	(elm)->ميدان.cqe_prev = (فراغ *)1L;
#وإلا
#وصف_ڪريو QUEUEDEBUG_CIRCLEQ_سر(سر, ميدان)
#وصف_ڪريو QUEUEDEBUG_CIRCLEQ_ELM(سر, elm, ميدان)
#وصف_ڪريو QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, ميدان)
#جيڪڏهن_پورو

#وصف_ڪريو	CIRCLEQ_سر(name, type)					\
بنية name {								\
	بنية type *cqh_پهريون;		/* first element */		\
	بنية type *cqh_آخري;		/* last element */		\
}

#وصف_ڪريو	CIRCLEQ_سر_INITIALIZER(سر)					\
	{ CIRCLEQ_پڇاڙي(&سر), CIRCLEQ_پڇاڙي(&سر) }

#وصف_ڪريو	CIRCLEQ_داخلا(type)						\
بنية {								\
	بنية type *cqe_ايندڙ;		/* next element */		\
	بنية type *cqe_prev;		/* previous element */		\
}

/*
 * Circular queue functions.
 */
#وصف_ڪريو	CIRCLEQ_INIT(سر) افعل {						\
	(سر)->cqh_پهريون = CIRCLEQ_پڇاڙي(سر);				\
	(سر)->cqh_آخري = CIRCLEQ_پڇاڙي(سر);				\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	CIRCLEQ_وجهو_AFTER(سر, listelm, elm, ميدان) افعل {		\
	QUEUEDEBUG_CIRCLEQ_سر((سر), ميدان)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر), (listelm), ميدان)		\
	(elm)->ميدان.cqe_ايندڙ = (listelm)->ميدان.cqe_ايندڙ;		\
	(elm)->ميدان.cqe_prev = (listelm);				\
	إذا ((listelm)->ميدان.cqe_ايندڙ == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_آخري = (elm);				\
	وإلا								\
		(listelm)->ميدان.cqe_ايندڙ->ميدان.cqe_prev = (elm);	\
	(listelm)->ميدان.cqe_ايندڙ = (elm);				\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	CIRCLEQ_وجهو_BEFORE(سر, listelm, elm, ميدان) افعل {		\
	QUEUEDEBUG_CIRCLEQ_سر((سر), ميدان)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر), (listelm), ميدان)		\
	(elm)->ميدان.cqe_ايندڙ = (listelm);				\
	(elm)->ميدان.cqe_prev = (listelm)->ميدان.cqe_prev;		\
	إذا ((listelm)->ميدان.cqe_prev == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_پهريون = (elm);				\
	وإلا								\
		(listelm)->ميدان.cqe_prev->ميدان.cqe_ايندڙ = (elm);	\
	(listelm)->ميدان.cqe_prev = (elm);				\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	CIRCLEQ_وجهو_سر(سر, elm, ميدان) افعل {			\
	QUEUEDEBUG_CIRCLEQ_سر((سر), ميدان)				\
	(elm)->ميدان.cqe_ايندڙ = (سر)->cqh_پهريون;			\
	(elm)->ميدان.cqe_prev = CIRCLEQ_پڇاڙي(سر);			\
	إذا ((سر)->cqh_آخري == CIRCLEQ_ENDC(سر))			\
		(سر)->cqh_آخري = (elm);				\
	وإلا								\
		(سر)->cqh_پهريون->ميدان.cqe_prev = (elm);		\
	(سر)->cqh_پهريون = (elm);					\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	CIRCLEQ_وجهو_پڇ(سر, elm, ميدان) افعل {			\
	QUEUEDEBUG_CIRCLEQ_سر((سر), ميدان)				\
	(elm)->ميدان.cqe_ايندڙ = CIRCLEQ_پڇاڙي(سر);			\
	(elm)->ميدان.cqe_prev = (سر)->cqh_آخري;			\
	إذا ((سر)->cqh_پهريون == CIRCLEQ_ENDC(سر))			\
		(سر)->cqh_پهريون = (elm);				\
	وإلا								\
		(سر)->cqh_آخري->ميدان.cqe_ايندڙ = (elm);		\
	(سر)->cqh_آخري = (elm);					\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	CIRCLEQ_REMOVE(سر, elm, ميدان) افعل {				\
	QUEUEDEBUG_CIRCLEQ_سر((سر), ميدان)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر), (elm), ميدان)			\
	إذا ((elm)->ميدان.cqe_ايندڙ == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_آخري = (elm)->ميدان.cqe_prev;		\
	وإلا								\
		(elm)->ميدان.cqe_ايندڙ->ميدان.cqe_prev =			\
		    (elm)->ميدان.cqe_prev;				\
	إذا ((elm)->ميدان.cqe_prev == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_پهريون = (elm)->ميدان.cqe_ايندڙ;		\
	وإلا								\
		(elm)->ميدان.cqe_prev->ميدان.cqe_ايندڙ =			\
		    (elm)->ميدان.cqe_ايندڙ;				\
	QUEUEDEBUG_CIRCLEQ_POSTREMOVE((elm), ميدان)			\
} طالما (/*CONSTCOND*/0)

#وصف_ڪريو	CIRCLEQ_FOREACH(var, سر, ميدان)				\
	لكل ((var) = ((سر)->cqh_پهريون);				\
		(var) != CIRCLEQ_ENDC(سر);				\
		(var) = ((var)->ميدان.cqe_ايندڙ))

#وصف_ڪريو	CIRCLEQ_FOREACH_ابتو(var, سر, ميدان)			\
	لكل ((var) = ((سر)->cqh_آخري);				\
		(var) != CIRCLEQ_ENDC(سر);				\
		(var) = ((var)->ميدان.cqe_prev))

/*
 * Circular queue access methods.
 */
#وصف_ڪريو	CIRCLEQ_پهريون(سر)		((سر)->cqh_پهريون)
#وصف_ڪريو	CIRCLEQ_آخري(سر)		((سر)->cqh_آخري)
/* For comparisons */
#وصف_ڪريو	CIRCLEQ_ENDC(سر)		(__launder_type(سر))
/* For assignments */
#وصف_ڪريو	CIRCLEQ_پڇاڙي(سر)		((فراغ *)(سر))
#وصف_ڪريو	CIRCLEQ_ايندڙ(elm, ميدان)	((elm)->ميدان.cqe_ايندڙ)
#وصف_ڪريو	CIRCLEQ_PREV(elm, ميدان)	((elm)->ميدان.cqe_prev)
#وصف_ڪريو	CIRCLEQ_خالي_جايون(سر)						\
    (CIRCLEQ_پهريون(سر) == CIRCLEQ_ENDC(سر))

#وصف_ڪريو CIRCLEQ_LOOP_ايندڙ(سر, elm, ميدان)				\
	(((elm)->ميدان.cqe_ايندڙ == CIRCLEQ_ENDC(سر))			\
	    ? ((سر)->cqh_پهريون)					\
	    : (elm->ميدان.cqe_ايندڙ))
#وصف_ڪريو CIRCLEQ_LOOP_PREV(سر, elm, ميدان)				\
	(((elm)->ميدان.cqe_prev == CIRCLEQ_ENDC(سر))			\
	    ? ((سر)->cqh_آخري)					\
	    : (elm->ميدان.cqe_prev))
#جيڪڏهن_پورو /* !_KERNEL */

#جيڪڏهن_پورو	/* !_SYS_QUEUE_H_ */
