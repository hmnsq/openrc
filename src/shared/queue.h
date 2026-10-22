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

#جے_بنا_تعریف	SYS_قطار_H
#تعریف_کرو	SYS_قطار_H

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
#جے_تعریف_شدہ __NetBSD__
#ضمن <sys/null.h>
#جے_انت

#إذا defined(QUEUEDEBUG)
# إذا defined(_KERNEL)
#  define QUEUEDEBUG_ABORT(...) panic(__VA_ARGS__)
# وإلا
#  include <err.h>
#  define QUEUEDEBUG_ABORT(...) err(1, __VA_ARGS__)
# endif
#جے_انت

/*
 * Singly-linked List definitions.
 */
#تعریف_کرو	SLIST_سر(name, type)						\
بنية name {								\
	بنية type *slh_پہلا;	/* first element */			\
}

#تعریف_کرو	SLIST_سر_INITIALIZER(سر)					\
	{ عدم }

#تعریف_کرو	SLIST_اندراج(type)						\
بنية {								\
	بنية type *sle_اگلا;	/* next element */			\
}

/*
 * Singly-linked List access methods.
 */
#تعریف_کرو	SLIST_پہلا(سر)	((سر)->slh_پہلا)
#تعریف_کرو	SLIST_سرا(سر)		عدم
#تعریف_کرو	SLIST_خالی_تھاواں(سر)	((سر)->slh_پہلا == عدم)
#تعریف_کرو	SLIST_اگلا(elm, میدان)	((elm)->میدان.sle_اگلا)

#تعریف_کرو	SLIST_FOREACH(var, سر, میدان)					\
	لكل((var) = (سر)->slh_پہلا;					\
	    (var) != SLIST_سرا(سر);					\
	    (var) = (var)->میدان.sle_اگلا)

#تعریف_کرو	SLIST_FOREACH_محفوظ(var, سر, میدان, tvar)			\
	لكل ((var) = SLIST_پہلا((سر));				\
	    (var) != SLIST_سرا(سر) &&					\
	    ((tvar) = SLIST_اگلا((var), میدان), 1);			\
	    (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#تعریف_کرو	SLIST_INIT(سر) افعل {						\
	(سر)->slh_پہلا = SLIST_سرا(سر);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	SLIST_داخل_کرو_AFTER(slistelm, elm, میدان) افعل {			\
	(elm)->میدان.sle_اگلا = (slistelm)->میدان.sle_اگلا;		\
	(slistelm)->میدان.sle_اگلا = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	SLIST_داخل_کرو_سر(سر, elm, میدان) افعل {			\
	(elm)->میدان.sle_اگلا = (سر)->slh_پہلا;			\
	(سر)->slh_پہلا = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	SLIST_REMOVE_AFTER(slistelm, میدان) افعل {			\
	(slistelm)->میدان.sle_اگلا =					\
	    SLIST_اگلا(SLIST_اگلا((slistelm), میدان), میدان);		\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	SLIST_REMOVE_سر(سر, میدان) افعل {				\
	(سر)->slh_پہلا = (سر)->slh_پہلا->میدان.sle_اگلا;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	SLIST_REMOVE(سر, elm, type, میدان) افعل {			\
	إذا ((سر)->slh_پہلا == (elm)) {				\
		SLIST_REMOVE_سر((سر), میدان);			\
	}								\
	وإلا {								\
		بنية type *curelm = (سر)->slh_پہلا;		\
		طالما(curelm->میدان.sle_اگلا != (elm))			\
			curelm = curelm->میدان.sle_اگلا;		\
		curelm->میدان.sle_اگلا =				\
		    curelm->میدان.sle_اگلا->میدان.sle_اگلا;		\
	}								\
} طالما (/*CONSTCOND*/0)


/*
 * List definitions.
 */
#تعریف_کرو	LIST_سر(name, type)						\
بنية name {								\
	بنية type *lh_پہلا;	/* first element */			\
}

#تعریف_کرو	LIST_سر_INITIALIZER(سر)					\
	{ عدم }

#تعریف_کرو	LIST_اندراج(type)						\
بنية {								\
	بنية type *le_اگلا;	/* next element */			\
	بنية type **le_prev;	/* address of previous next element */	\
}

/*
 * List access methods.
 */
#تعریف_کرو	LIST_پہلا(سر)		((سر)->lh_پہلا)
#تعریف_کرو	LIST_سرا(سر)			عدم
#تعریف_کرو	LIST_خالی_تھاواں(سر)		((سر)->lh_پہلا == LIST_سرا(سر))
#تعریف_کرو	LIST_اگلا(elm, میدان)		((elm)->میدان.le_اگلا)

#تعریف_کرو	LIST_FOREACH(var, سر, میدان)					\
	لكل ((var) = ((سر)->lh_پہلا);				\
	    (var) != LIST_سرا(سر);					\
	    (var) = ((var)->میدان.le_اگلا))

#تعریف_کرو	LIST_FOREACH_محفوظ(var, سر, میدان, tvar)			\
	لكل ((var) = LIST_پہلا((سر));				\
	    (var) != LIST_سرا(سر) &&					\
	    ((tvar) = LIST_اگلا((var), میدان), 1);			\
	    (var) = (tvar))

#تعریف_کرو	LIST_MOVE(head1, head2) افعل {					\
	LIST_INIT((head2));						\
	إذا (!LIST_خالی_تھاواں((head1))) {					\
		(head2)->lh_پہلا = (head1)->lh_پہلا;			\
		LIST_INIT((head1));					\
	}								\
} طالما (/*CONSTCOND*/0)

/*
 * List functions.
 */
#إذا defined(QUEUEDEBUG)
#تعریف_کرو	QUEUEDEBUG_LIST_داخل_کرو_سر(سر, elm, میدان)			\
	إذا ((سر)->lh_پہلا &&						\
	    (سر)->lh_پہلا->میدان.le_prev != &(سر)->lh_پہلا)	\
		QUEUEDEBUG_ABORT("LIST_INSERT_HEAD %p %s:%d", (سر),	\
		    __FILE__, __LINE__);
#تعریف_کرو	QUEUEDEBUG_LIST_OP(elm, میدان)					\
	إذا ((elm)->میدان.le_اگلا &&					\
	    (elm)->میدان.le_اگلا->میدان.le_prev !=			\
	    &(elm)->میدان.le_اگلا)					\
		QUEUEDEBUG_ABORT("LIST_* forw %p %s:%d", (elm),		\
		    __FILE__, __LINE__);				\
	إذا (*(elm)->میدان.le_prev != (elm))				\
		QUEUEDEBUG_ABORT("LIST_* back %p %s:%d", (elm),		\
		    __FILE__, __LINE__);
#تعریف_کرو	QUEUEDEBUG_LIST_POSTREMOVE(elm, میدان)				\
	(elm)->میدان.le_اگلا = (فراغ *)1L;				\
	(elm)->میدان.le_prev = (فراغ *)1L;
#وإلا
#تعریف_کرو	QUEUEDEBUG_LIST_داخل_کرو_سر(سر, elm, میدان)
#تعریف_کرو	QUEUEDEBUG_LIST_OP(elm, میدان)
#تعریف_کرو	QUEUEDEBUG_LIST_POSTREMOVE(elm, میدان)
#جے_انت

#تعریف_کرو	LIST_INIT(سر) افعل {						\
	(سر)->lh_پہلا = LIST_سرا(سر);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	LIST_داخل_کرو_AFTER(listelm, elm, میدان) افعل {			\
	QUEUEDEBUG_LIST_OP((listelm), میدان)				\
	إذا (((elm)->میدان.le_اگلا = (listelm)->میدان.le_اگلا) != 	\
	    LIST_سرا(سر))						\
		(listelm)->میدان.le_اگلا->میدان.le_prev =		\
		    &(elm)->میدان.le_اگلا;				\
	(listelm)->میدان.le_اگلا = (elm);				\
	(elm)->میدان.le_prev = &(listelm)->میدان.le_اگلا;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	LIST_داخل_کرو_BEFORE(listelm, elm, میدان) افعل {			\
	QUEUEDEBUG_LIST_OP((listelm), میدان)				\
	(elm)->میدان.le_prev = (listelm)->میدان.le_prev;		\
	(elm)->میدان.le_اگلا = (listelm);				\
	*(listelm)->میدان.le_prev = (elm);				\
	(listelm)->میدان.le_prev = &(elm)->میدان.le_اگلا;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	LIST_داخل_کرو_سر(سر, elm, میدان) افعل {				\
	QUEUEDEBUG_LIST_داخل_کرو_سر((سر), (elm), میدان)		\
	إذا (((elm)->میدان.le_اگلا = (سر)->lh_پہلا) != LIST_سرا(سر))\
		(سر)->lh_پہلا->میدان.le_prev = &(elm)->میدان.le_اگلا;\
	(سر)->lh_پہلا = (elm);					\
	(elm)->میدان.le_prev = &(سر)->lh_پہلا;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	LIST_REMOVE(elm, میدان) افعل {					\
	QUEUEDEBUG_LIST_OP((elm), میدان)				\
	إذا ((elm)->میدان.le_اگلا != عدم)				\
		(elm)->میدان.le_اگلا->میدان.le_prev = 			\
		    (elm)->میدان.le_prev;				\
	*(elm)->میدان.le_prev = (elm)->میدان.le_اگلا;			\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), میدان)			\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو LIST_REPLACE(elm, elm2, میدان) افعل {				\
	إذا (((elm2)->میدان.le_اگلا = (elm)->میدان.le_اگلا) != عدم)	\
		(elm2)->میدان.le_اگلا->میدان.le_prev =			\
		    &(elm2)->میدان.le_اگلا;				\
	(elm2)->میدان.le_prev = (elm)->میدان.le_prev;			\
	*(elm2)->میدان.le_prev = (elm2);				\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), میدان)			\
} طالما (/*CONSTCOND*/0)

/*
 * Simple queue definitions.
 */
#تعریف_کرو	SIMPLEQ_سر(name, type)					\
بنية name {								\
	بنية type *sqh_پہلا;	/* first element */			\
	بنية type **sqh_آخری;	/* addr of last next element */		\
}

#تعریف_کرو	SIMPLEQ_سر_INITIALIZER(سر)					\
	{ عدم, &(سر).sqh_پہلا }

#تعریف_کرو	SIMPLEQ_اندراج(type)						\
بنية {								\
	بنية type *sqe_اگلا;	/* next element */			\
}

/*
 * Simple queue access methods.
 */
#تعریف_کرو	SIMPLEQ_پہلا(سر)		((سر)->sqh_پہلا)
#تعریف_کرو	SIMPLEQ_سرا(سر)		عدم
#تعریف_کرو	SIMPLEQ_خالی_تھاواں(سر)		((سر)->sqh_پہلا == SIMPLEQ_سرا(سر))
#تعریف_کرو	SIMPLEQ_اگلا(elm, میدان)	((elm)->میدان.sqe_اگلا)

#تعریف_کرو	SIMPLEQ_FOREACH(var, سر, میدان)				\
	لكل ((var) = ((سر)->sqh_پہلا);				\
	    (var) != SIMPLEQ_سرا(سر);					\
	    (var) = ((var)->میدان.sqe_اگلا))

#تعریف_کرو	SIMPLEQ_FOREACH_محفوظ(var, سر, میدان, اگلا)			\
	لكل ((var) = ((سر)->sqh_پہلا);				\
	    (var) != SIMPLEQ_سرا(سر) &&				\
	    ((اگلا = ((var)->میدان.sqe_اگلا)), 1);			\
	    (var) = (اگلا))

/*
 * Simple queue functions.
 */
#تعریف_کرو	SIMPLEQ_INIT(سر) افعل {						\
	(سر)->sqh_پہلا = عدم;					\
	(سر)->sqh_آخری = &(سر)->sqh_پہلا;				\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_داخل_کرو_سر(سر, elm, میدان) افعل {			\
	إذا (((elm)->میدان.sqe_اگلا = (سر)->sqh_پہلا) == عدم)	\
		(سر)->sqh_آخری = &(elm)->میدان.sqe_اگلا;		\
	(سر)->sqh_پہلا = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_داخل_کرو_پوچھ(سر, elm, میدان) افعل {			\
	(elm)->میدان.sqe_اگلا = عدم;					\
	*(سر)->sqh_آخری = (elm);					\
	(سر)->sqh_آخری = &(elm)->میدان.sqe_اگلا;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_داخل_کرو_AFTER(سر, listelm, elm, میدان) افعل {		\
	إذا (((elm)->میدان.sqe_اگلا = (listelm)->میدان.sqe_اگلا) == عدم)\
		(سر)->sqh_آخری = &(elm)->میدان.sqe_اگلا;		\
	(listelm)->میدان.sqe_اگلا = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_REMOVE_سر(سر, میدان) افعل {				\
	إذا (((سر)->sqh_پہلا = (سر)->sqh_پہلا->میدان.sqe_اگلا) == عدم) \
		(سر)->sqh_آخری = &(سر)->sqh_پہلا;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو SIMPLEQ_REMOVE_AFTER(سر, elm, میدان) افعل {			\
	إذا (((elm)->میدان.sqe_اگلا = (elm)->میدان.sqe_اگلا->میدان.sqe_اگلا) \
	    == عدم)							\
		(سر)->sqh_آخری = &(elm)->میدان.sqe_اگلا;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_REMOVE(سر, elm, type, میدان) افعل {			\
	إذا ((سر)->sqh_پہلا == (elm)) {				\
		SIMPLEQ_REMOVE_سر((سر), میدان);			\
	} وإلا {							\
		بنية type *curelm = (سر)->sqh_پہلا;		\
		طالما (curelm->میدان.sqe_اگلا != (elm))			\
			curelm = curelm->میدان.sqe_اگلا;		\
		إذا ((curelm->میدان.sqe_اگلا =				\
			curelm->میدان.sqe_اگلا->میدان.sqe_اگلا) == عدم) \
			    (سر)->sqh_آخری = &(curelm)->میدان.sqe_اگلا; \
	}								\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_CONCAT(head1, head2) افعل {				\
	إذا (!SIMPLEQ_خالی_تھاواں((head2))) {					\
		*(head1)->sqh_آخری = (head2)->sqh_پہلا;		\
		(head1)->sqh_آخری = (head2)->sqh_آخری;		\
		SIMPLEQ_INIT((head2));					\
	}								\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_آخری(سر, type, میدان)					\
	(SIMPLEQ_خالی_تھاواں((سر)) ?						\
		عدم :							\
	        ((بنية type *)(فراغ *)				\
		((حرف *)((سر)->sqh_آخری) - offsetof(بنية type, میدان))))

/*
 * Tail queue definitions.
 */
#تعریف_کرو	TAILQ_سر_2(name, type, qual)					\
بنية name {								\
	qual type *tqh_پہلا;		/* first element */		\
	qual type *qual *tqh_آخری;	/* addr of last next element */	\
}
#تعریف_کرو TAILQ_سر(name, type)	TAILQ_سر_2(name, بنية type,)

#تعریف_کرو	TAILQ_سر_INITIALIZER(سر)					\
	{ TAILQ_سرا(سر), &(سر).tqh_پہلا }

#تعریف_کرو	TAILQ_اندراج_2(type, qual)					\
بنية {								\
	qual type *tqe_اگلا;		/* next element */		\
	qual type *qual *tqe_prev;	/* address of previous next element */\
}
#تعریف_کرو TAILQ_اندراج(type)	TAILQ_اندراج_2(بنية type,)

/*
 * Tail queue access methods.
 */
#تعریف_کرو	TAILQ_پہلا(سر)		((سر)->tqh_پہلا)
#تعریف_کرو	TAILQ_سرا(سر)			(عدم)
#تعریف_کرو	TAILQ_اگلا(elm, میدان)		((elm)->میدان.tqe_اگلا)
#تعریف_کرو	TAILQ_آخری(سر, headname) \
	(*(((بنية headname *)((سر)->tqh_آخری))->tqh_آخری))
#تعریف_کرو	TAILQ_PREV(elm, headname, میدان) \
	(*(((بنية headname *)((elm)->میدان.tqe_prev))->tqh_آخری))
#تعریف_کرو	TAILQ_خالی_تھاواں(سر)		(TAILQ_پہلا(سر) == TAILQ_سرا(سر))


#تعریف_کرو	TAILQ_FOREACH(var, سر, میدان)					\
	لكل ((var) = ((سر)->tqh_پہلا);				\
	    (var) != TAILQ_سرا(سر);					\
	    (var) = ((var)->میدان.tqe_اگلا))

#تعریف_کرو	TAILQ_FOREACH_محفوظ(var, سر, میدان, اگلا)			\
	لكل ((var) = ((سر)->tqh_پہلا);				\
	    (var) != TAILQ_سرا(سر) &&					\
	    ((اگلا) = TAILQ_اگلا(var, میدان), 1); (var) = (اگلا))

#تعریف_کرو	TAILQ_FOREACH_الٹ(var, سر, headname, میدان)		\
	لكل ((var) = (*(((بنية headname *)((سر)->tqh_آخری))->tqh_آخری));\
	    (var) != TAILQ_سرا(سر);					\
	    (var) = (*(((بنية headname *)((var)->میدان.tqe_prev))->tqh_آخری)))

#تعریف_کرو	TAILQ_FOREACH_الٹ_محفوظ(var, سر, headname, میدان, prev)	\
	لكل ((var) = TAILQ_آخری((سر), headname);			\
	    (var) != TAILQ_سرا(سر) && 				\
	    ((prev) = TAILQ_PREV((var), headname, میدان), 1); (var) = (prev))

/*
 * Tail queue functions.
 */
#إذا defined(QUEUEDEBUG)
#تعریف_کرو	QUEUEDEBUG_TAILQ_داخل_کرو_سر(سر, elm, میدان)			\
	إذا ((سر)->tqh_پہلا &&					\
	    (سر)->tqh_پہلا->میدان.tqe_prev != &(سر)->tqh_پہلا)	\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_HEAD %p %s:%d", (سر),	\
		    __FILE__, __LINE__);
#تعریف_کرو	QUEUEDEBUG_TAILQ_داخل_کرو_پوچھ(سر, elm, میدان)			\
	إذا (*(سر)->tqh_آخری != عدم)					\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_TAIL %p %s:%d", (سر),	\
		    __FILE__, __LINE__);
#تعریف_کرو	QUEUEDEBUG_TAILQ_OP(elm, میدان)					\
	إذا ((elm)->میدان.tqe_اگلا &&					\
	    (elm)->میدان.tqe_اگلا->میدان.tqe_prev !=			\
	    &(elm)->میدان.tqe_اگلا)					\
		QUEUEDEBUG_ABORT("TAILQ_* forw %p %s:%d", (elm),	\
		    __FILE__, __LINE__);				\
	إذا (*(elm)->میدان.tqe_prev != (elm))				\
		QUEUEDEBUG_ABORT("TAILQ_* back %p %s:%d", (elm),	\
		    __FILE__, __LINE__);
#تعریف_کرو	QUEUEDEBUG_TAILQ_PREREMOVE(سر, elm, میدان)			\
	إذا ((elm)->میدان.tqe_اگلا == عدم &&				\
	    (سر)->tqh_آخری != &(elm)->میدان.tqe_اگلا)			\
		QUEUEDEBUG_ABORT("TAILQ_PREREMOVE head %p elm %p %s:%d",\
		    (سر), (elm), __FILE__, __LINE__);
#تعریف_کرو	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, میدان)				\
	(elm)->میدان.tqe_اگلا = (فراغ *)1L;				\
	(elm)->میدان.tqe_prev = (فراغ *)1L;
#وإلا
#تعریف_کرو	QUEUEDEBUG_TAILQ_داخل_کرو_سر(سر, elm, میدان)
#تعریف_کرو	QUEUEDEBUG_TAILQ_داخل_کرو_پوچھ(سر, elm, میدان)
#تعریف_کرو	QUEUEDEBUG_TAILQ_OP(elm, میدان)
#تعریف_کرو	QUEUEDEBUG_TAILQ_PREREMOVE(سر, elm, میدان)
#تعریف_کرو	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, میدان)
#جے_انت

#تعریف_کرو	TAILQ_INIT(سر) افعل {						\
	(سر)->tqh_پہلا = TAILQ_سرا(سر);				\
	(سر)->tqh_آخری = &(سر)->tqh_پہلا;				\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	TAILQ_داخل_کرو_سر(سر, elm, میدان) افعل {			\
	QUEUEDEBUG_TAILQ_داخل_کرو_سر((سر), (elm), میدان)		\
	إذا (((elm)->میدان.tqe_اگلا = (سر)->tqh_پہلا) != TAILQ_سرا(سر))\
		(سر)->tqh_پہلا->میدان.tqe_prev =			\
		    &(elm)->میدان.tqe_اگلا;				\
	وإلا								\
		(سر)->tqh_آخری = &(elm)->میدان.tqe_اگلا;		\
	(سر)->tqh_پہلا = (elm);					\
	(elm)->میدان.tqe_prev = &(سر)->tqh_پہلا;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	TAILQ_داخل_کرو_پوچھ(سر, elm, میدان) افعل {			\
	QUEUEDEBUG_TAILQ_داخل_کرو_پوچھ((سر), (elm), میدان)		\
	(elm)->میدان.tqe_اگلا = TAILQ_سرا(سر);			\
	(elm)->میدان.tqe_prev = (سر)->tqh_آخری;			\
	*(سر)->tqh_آخری = (elm);					\
	(سر)->tqh_آخری = &(elm)->میدان.tqe_اگلا;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	TAILQ_داخل_کرو_AFTER(سر, listelm, elm, میدان) افعل {		\
	QUEUEDEBUG_TAILQ_OP((listelm), میدان)				\
	إذا (((elm)->میدان.tqe_اگلا = (listelm)->میدان.tqe_اگلا) != 	\
	    TAILQ_سرا(سر))						\
		(elm)->میدان.tqe_اگلا->میدان.tqe_prev = 		\
		    &(elm)->میدان.tqe_اگلا;				\
	وإلا								\
		(سر)->tqh_آخری = &(elm)->میدان.tqe_اگلا;		\
	(listelm)->میدان.tqe_اگلا = (elm);				\
	(elm)->میدان.tqe_prev = &(listelm)->میدان.tqe_اگلا;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	TAILQ_داخل_کرو_BEFORE(listelm, elm, میدان) افعل {			\
	QUEUEDEBUG_TAILQ_OP((listelm), میدان)				\
	(elm)->میدان.tqe_prev = (listelm)->میدان.tqe_prev;		\
	(elm)->میدان.tqe_اگلا = (listelm);				\
	*(listelm)->میدان.tqe_prev = (elm);				\
	(listelm)->میدان.tqe_prev = &(elm)->میدان.tqe_اگلا;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	TAILQ_REMOVE(سر, elm, میدان) افعل {				\
	QUEUEDEBUG_TAILQ_PREREMOVE((سر), (elm), میدان)		\
	QUEUEDEBUG_TAILQ_OP((elm), میدان)				\
	إذا (((elm)->میدان.tqe_اگلا) != TAILQ_سرا(سر))			\
		(elm)->میدان.tqe_اگلا->میدان.tqe_prev = 		\
		    (elm)->میدان.tqe_prev;				\
	وإلا								\
		(سر)->tqh_آخری = (elm)->میدان.tqe_prev;		\
	*(elm)->میدان.tqe_prev = (elm)->میدان.tqe_اگلا;			\
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), میدان);			\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو TAILQ_REPLACE(سر, elm, elm2, میدان) افعل {                      \
        إذا (((elm2)->میدان.tqe_اگلا = (elm)->میدان.tqe_اگلا) != 	\
	    TAILQ_سرا(سر))   						\
                (elm2)->میدان.tqe_اگلا->میدان.tqe_prev =                \
                    &(elm2)->میدان.tqe_اگلا;                            \
        وإلا                                                            \
                (سر)->tqh_آخری = &(elm2)->میدان.tqe_اگلا;             \
        (elm2)->میدان.tqe_prev = (elm)->میدان.tqe_prev;                 \
        *(elm2)->میدان.tqe_prev = (elm2);                               \
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), میدان);			\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	TAILQ_CONCAT(head1, head2, میدان) افعل {				\
	إذا (!TAILQ_خالی_تھاواں(head2)) {					\
		*(head1)->tqh_آخری = (head2)->tqh_پہلا;		\
		(head2)->tqh_پہلا->میدان.tqe_prev = (head1)->tqh_آخری;	\
		(head1)->tqh_آخری = (head2)->tqh_آخری;			\
		TAILQ_INIT((head2));					\
	}								\
} طالما (/*CONSTCOND*/0)

/*
 * Singly-linked Tail queue declarations.
 */
#تعریف_کرو	STAILQ_سر(name, type)						\
بنية name {								\
	بنية type *stqh_پہلا;	/* first element */		\
	بنية type **stqh_آخری;	/* addr of last next element */	\
}

#تعریف_کرو	STAILQ_سر_INITIALIZER(سر)					\
	{ عدم, &(سر).stqh_پہلا }

#تعریف_کرو	STAILQ_اندراج(type)						\
بنية {								\
	بنية type *stqe_اگلا;	/* next element */			\
}

/*
 * Singly-linked Tail queue access methods.
 */
#تعریف_کرو	STAILQ_پہلا(سر)	((سر)->stqh_پہلا)
#تعریف_کرو	STAILQ_سرا(سر)	عدم
#تعریف_کرو	STAILQ_اگلا(elm, میدان)	((elm)->میدان.stqe_اگلا)
#تعریف_کرو	STAILQ_خالی_تھاواں(سر)	(STAILQ_پہلا(سر) == STAILQ_سرا(سر))

/*
 * Singly-linked Tail queue functions.
 */
#تعریف_کرو	STAILQ_INIT(سر) افعل {						\
	(سر)->stqh_پہلا = عدم;					\
	(سر)->stqh_آخری = &(سر)->stqh_پہلا;				\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_داخل_کرو_سر(سر, elm, میدان) افعل {			\
	إذا (((elm)->میدان.stqe_اگلا = (سر)->stqh_پہلا) == عدم)	\
		(سر)->stqh_آخری = &(elm)->میدان.stqe_اگلا;		\
	(سر)->stqh_پہلا = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_داخل_کرو_پوچھ(سر, elm, میدان) افعل {			\
	(elm)->میدان.stqe_اگلا = عدم;					\
	*(سر)->stqh_آخری = (elm);					\
	(سر)->stqh_آخری = &(elm)->میدان.stqe_اگلا;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_داخل_کرو_AFTER(سر, listelm, elm, میدان) افعل {		\
	إذا (((elm)->میدان.stqe_اگلا = (listelm)->میدان.stqe_اگلا) == عدم)\
		(سر)->stqh_آخری = &(elm)->میدان.stqe_اگلا;		\
	(listelm)->میدان.stqe_اگلا = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_REMOVE_سر(سر, میدان) افعل {				\
	إذا (((سر)->stqh_پہلا = (سر)->stqh_پہلا->میدان.stqe_اگلا) == عدم) \
		(سر)->stqh_آخری = &(سر)->stqh_پہلا;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_REMOVE(سر, elm, type, میدان) افعل {			\
	إذا ((سر)->stqh_پہلا == (elm)) {				\
		STAILQ_REMOVE_سر((سر), میدان);			\
	} وإلا {							\
		بنية type *curelm = (سر)->stqh_پہلا;		\
		طالما (curelm->میدان.stqe_اگلا != (elm))			\
			curelm = curelm->میدان.stqe_اگلا;		\
		إذا ((curelm->میدان.stqe_اگلا =				\
			curelm->میدان.stqe_اگلا->میدان.stqe_اگلا) == عدم) \
			    (سر)->stqh_آخری = &(curelm)->میدان.stqe_اگلا; \
	}								\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_FOREACH(var, سر, میدان)				\
	لكل ((var) = ((سر)->stqh_پہلا);				\
		(var);							\
		(var) = ((var)->میدان.stqe_اگلا))

#تعریف_کرو	STAILQ_FOREACH_محفوظ(var, سر, میدان, tvar)			\
	لكل ((var) = STAILQ_پہلا((سر));				\
	    (var) && ((tvar) = STAILQ_اگلا((var), میدان), 1);		\
	    (var) = (tvar))

#تعریف_کرو	STAILQ_CONCAT(head1, head2) افعل {				\
	إذا (!STAILQ_خالی_تھاواں((head2))) {					\
		*(head1)->stqh_آخری = (head2)->stqh_پہلا;		\
		(head1)->stqh_آخری = (head2)->stqh_آخری;		\
		STAILQ_INIT((head2));					\
	}								\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_آخری(سر, type, میدان)					\
	(STAILQ_خالی_تھاواں((سر)) ?						\
		عدم :							\
	        ((بنية type *)(فراغ *)				\
		((حرف *)((سر)->stqh_آخری) - offsetof(بنية type, میدان))))


#جے_بنا_تعریف _KERNEL
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
#تعریف_کرو QUEUEDEBUG_CIRCLEQ_سر(سر, میدان)				\
	إذا ((سر)->cqh_پہلا != CIRCLEQ_ENDC(سر) &&			\
	    (سر)->cqh_پہلا->میدان.cqe_prev != CIRCLEQ_ENDC(سر))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head forw %p %s:%d", (سر),	\
		      __FILE__, __LINE__);				\
	إذا ((سر)->cqh_آخری != CIRCLEQ_ENDC(سر) &&			\
	    (سر)->cqh_آخری->میدان.cqe_اگلا != CIRCLEQ_ENDC(سر))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head back %p %s:%d", (سر),	\
		      __FILE__, __LINE__);
#تعریف_کرو QUEUEDEBUG_CIRCLEQ_ELM(سر, elm, میدان)			\
	إذا ((elm)->میدان.cqe_اگلا == CIRCLEQ_ENDC(سر)) {		\
		إذا ((سر)->cqh_آخری != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm last %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} وإلا {							\
		إذا ((elm)->میدان.cqe_اگلا->میدان.cqe_prev != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm forw %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}								\
	إذا ((elm)->میدان.cqe_prev == CIRCLEQ_ENDC(سر)) {		\
		إذا ((سر)->cqh_پہلا != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm first %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} وإلا {							\
		إذا ((elm)->میدان.cqe_prev->میدان.cqe_اگلا != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm prev %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}
#تعریف_کرو QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, میدان)			\
	(elm)->میدان.cqe_اگلا = (فراغ *)1L;				\
	(elm)->میدان.cqe_prev = (فراغ *)1L;
#وإلا
#تعریف_کرو QUEUEDEBUG_CIRCLEQ_سر(سر, میدان)
#تعریف_کرو QUEUEDEBUG_CIRCLEQ_ELM(سر, elm, میدان)
#تعریف_کرو QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, میدان)
#جے_انت

#تعریف_کرو	CIRCLEQ_سر(name, type)					\
بنية name {								\
	بنية type *cqh_پہلا;		/* first element */		\
	بنية type *cqh_آخری;		/* last element */		\
}

#تعریف_کرو	CIRCLEQ_سر_INITIALIZER(سر)					\
	{ CIRCLEQ_سرا(&سر), CIRCLEQ_سرا(&سر) }

#تعریف_کرو	CIRCLEQ_اندراج(type)						\
بنية {								\
	بنية type *cqe_اگلا;		/* next element */		\
	بنية type *cqe_prev;		/* previous element */		\
}

/*
 * Circular queue functions.
 */
#تعریف_کرو	CIRCLEQ_INIT(سر) افعل {						\
	(سر)->cqh_پہلا = CIRCLEQ_سرا(سر);				\
	(سر)->cqh_آخری = CIRCLEQ_سرا(سر);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	CIRCLEQ_داخل_کرو_AFTER(سر, listelm, elm, میدان) افعل {		\
	QUEUEDEBUG_CIRCLEQ_سر((سر), میدان)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر), (listelm), میدان)		\
	(elm)->میدان.cqe_اگلا = (listelm)->میدان.cqe_اگلا;		\
	(elm)->میدان.cqe_prev = (listelm);				\
	إذا ((listelm)->میدان.cqe_اگلا == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_آخری = (elm);				\
	وإلا								\
		(listelm)->میدان.cqe_اگلا->میدان.cqe_prev = (elm);	\
	(listelm)->میدان.cqe_اگلا = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	CIRCLEQ_داخل_کرو_BEFORE(سر, listelm, elm, میدان) افعل {		\
	QUEUEDEBUG_CIRCLEQ_سر((سر), میدان)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر), (listelm), میدان)		\
	(elm)->میدان.cqe_اگلا = (listelm);				\
	(elm)->میدان.cqe_prev = (listelm)->میدان.cqe_prev;		\
	إذا ((listelm)->میدان.cqe_prev == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_پہلا = (elm);				\
	وإلا								\
		(listelm)->میدان.cqe_prev->میدان.cqe_اگلا = (elm);	\
	(listelm)->میدان.cqe_prev = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	CIRCLEQ_داخل_کرو_سر(سر, elm, میدان) افعل {			\
	QUEUEDEBUG_CIRCLEQ_سر((سر), میدان)				\
	(elm)->میدان.cqe_اگلا = (سر)->cqh_پہلا;			\
	(elm)->میدان.cqe_prev = CIRCLEQ_سرا(سر);			\
	إذا ((سر)->cqh_آخری == CIRCLEQ_ENDC(سر))			\
		(سر)->cqh_آخری = (elm);				\
	وإلا								\
		(سر)->cqh_پہلا->میدان.cqe_prev = (elm);		\
	(سر)->cqh_پہلا = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	CIRCLEQ_داخل_کرو_پوچھ(سر, elm, میدان) افعل {			\
	QUEUEDEBUG_CIRCLEQ_سر((سر), میدان)				\
	(elm)->میدان.cqe_اگلا = CIRCLEQ_سرا(سر);			\
	(elm)->میدان.cqe_prev = (سر)->cqh_آخری;			\
	إذا ((سر)->cqh_پہلا == CIRCLEQ_ENDC(سر))			\
		(سر)->cqh_پہلا = (elm);				\
	وإلا								\
		(سر)->cqh_آخری->میدان.cqe_اگلا = (elm);		\
	(سر)->cqh_آخری = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	CIRCLEQ_REMOVE(سر, elm, میدان) افعل {				\
	QUEUEDEBUG_CIRCLEQ_سر((سر), میدان)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر), (elm), میدان)			\
	إذا ((elm)->میدان.cqe_اگلا == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_آخری = (elm)->میدان.cqe_prev;		\
	وإلا								\
		(elm)->میدان.cqe_اگلا->میدان.cqe_prev =			\
		    (elm)->میدان.cqe_prev;				\
	إذا ((elm)->میدان.cqe_prev == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_پہلا = (elm)->میدان.cqe_اگلا;		\
	وإلا								\
		(elm)->میدان.cqe_prev->میدان.cqe_اگلا =			\
		    (elm)->میدان.cqe_اگلا;				\
	QUEUEDEBUG_CIRCLEQ_POSTREMOVE((elm), میدان)			\
} طالما (/*CONSTCOND*/0)

#تعریف_کرو	CIRCLEQ_FOREACH(var, سر, میدان)				\
	لكل ((var) = ((سر)->cqh_پہلا);				\
		(var) != CIRCLEQ_ENDC(سر);				\
		(var) = ((var)->میدان.cqe_اگلا))

#تعریف_کرو	CIRCLEQ_FOREACH_الٹ(var, سر, میدان)			\
	لكل ((var) = ((سر)->cqh_آخری);				\
		(var) != CIRCLEQ_ENDC(سر);				\
		(var) = ((var)->میدان.cqe_prev))

/*
 * Circular queue access methods.
 */
#تعریف_کرو	CIRCLEQ_پہلا(سر)		((سر)->cqh_پہلا)
#تعریف_کرو	CIRCLEQ_آخری(سر)		((سر)->cqh_آخری)
/* For comparisons */
#تعریف_کرو	CIRCLEQ_ENDC(سر)		(__launder_type(سر))
/* For assignments */
#تعریف_کرو	CIRCLEQ_سرا(سر)		((فراغ *)(سر))
#تعریف_کرو	CIRCLEQ_اگلا(elm, میدان)	((elm)->میدان.cqe_اگلا)
#تعریف_کرو	CIRCLEQ_PREV(elm, میدان)	((elm)->میدان.cqe_prev)
#تعریف_کرو	CIRCLEQ_خالی_تھاواں(سر)						\
    (CIRCLEQ_پہلا(سر) == CIRCLEQ_ENDC(سر))

#تعریف_کرو CIRCLEQ_LOOP_اگلا(سر, elm, میدان)				\
	(((elm)->میدان.cqe_اگلا == CIRCLEQ_ENDC(سر))			\
	    ? ((سر)->cqh_پہلا)					\
	    : (elm->میدان.cqe_اگلا))
#تعریف_کرو CIRCLEQ_LOOP_PREV(سر, elm, میدان)				\
	(((elm)->میدان.cqe_prev == CIRCLEQ_ENDC(سر))			\
	    ? ((سر)->cqh_آخری)					\
	    : (elm->میدان.cqe_prev))
#جے_انت /* !_KERNEL */

#جے_انت	/* !_SYS_QUEUE_H_ */
