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

#اگر_غیر_تعریف_شدہ	SYS_قطار_H
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
#اگر_تعریف_شدہ __NetBSD__
#شامل <sys/null.h>
#اگر_ختم

#اگر defined(QUEUEDEBUG)
# اگر defined(_KERNEL)
#  define QUEUEDEBUG_ABORT(...) panic(__VA_ARGS__)
# ورنہ
#  include <err.h>
#  define QUEUEDEBUG_ABORT(...) err(1, __VA_ARGS__)
# endif
#اگر_ختم

/*
 * Singly-linked List definitions.
 */
#تعریف_کرو	SLIST_سر(name, type)						\
ساخت name {								\
	ساخت type *slh_پہلا;	/* first element */			\
}

#تعریف_کرو	SLIST_سر_INITIALIZER(سر)					\
	{ تہی }

#تعریف_کرو	SLIST_اندراج(type)						\
ساخت {								\
	ساخت type *sle_اگلا;	/* next element */			\
}

/*
 * Singly-linked List access methods.
 */
#تعریف_کرو	SLIST_پہلا(سر)	((سر)->slh_پہلا)
#تعریف_کرو	SLIST_انتہا(سر)		تہی
#تعریف_کرو	SLIST_خالی_خانے(سر)	((سر)->slh_پہلا == تہی)
#تعریف_کرو	SLIST_اگلا(elm, میدان)	((elm)->میدان.sle_اگلا)

#تعریف_کرو	SLIST_FOREACH(var, سر, میدان)					\
	برائے((var) = (سر)->slh_پہلا;					\
	    (var) != SLIST_انتہا(سر);					\
	    (var) = (var)->میدان.sle_اگلا)

#تعریف_کرو	SLIST_FOREACH_محفوظ(var, سر, میدان, tvar)			\
	برائے ((var) = SLIST_پہلا((سر));				\
	    (var) != SLIST_انتہا(سر) &&					\
	    ((tvar) = SLIST_اگلا((var), میدان), 1);			\
	    (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#تعریف_کرو	SLIST_INIT(سر) کرو {						\
	(سر)->slh_پہلا = SLIST_انتہا(سر);				\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	SLIST_داخل_کرو_AFTER(slistelm, elm, میدان) کرو {			\
	(elm)->میدان.sle_اگلا = (slistelm)->میدان.sle_اگلا;		\
	(slistelm)->میدان.sle_اگلا = (elm);				\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	SLIST_داخل_کرو_سر(سر, elm, میدان) کرو {			\
	(elm)->میدان.sle_اگلا = (سر)->slh_پہلا;			\
	(سر)->slh_پہلا = (elm);					\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	SLIST_REMOVE_AFTER(slistelm, میدان) کرو {			\
	(slistelm)->میدان.sle_اگلا =					\
	    SLIST_اگلا(SLIST_اگلا((slistelm), میدان), میدان);		\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	SLIST_REMOVE_سر(سر, میدان) کرو {				\
	(سر)->slh_پہلا = (سر)->slh_پہلا->میدان.sle_اگلا;		\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	SLIST_REMOVE(سر, elm, type, میدان) کرو {			\
	اگر ((سر)->slh_پہلا == (elm)) {				\
		SLIST_REMOVE_سر((سر), میدان);			\
	}								\
	ورنہ {								\
		ساخت type *curelm = (سر)->slh_پہلا;		\
		جب(curelm->میدان.sle_اگلا != (elm))			\
			curelm = curelm->میدان.sle_اگلا;		\
		curelm->میدان.sle_اگلا =				\
		    curelm->میدان.sle_اگلا->میدان.sle_اگلا;		\
	}								\
} جب (/*CONSTCOND*/0)


/*
 * List definitions.
 */
#تعریف_کرو	LIST_سر(name, type)						\
ساخت name {								\
	ساخت type *lh_پہلا;	/* first element */			\
}

#تعریف_کرو	LIST_سر_INITIALIZER(سر)					\
	{ تہی }

#تعریف_کرو	LIST_اندراج(type)						\
ساخت {								\
	ساخت type *le_اگلا;	/* next element */			\
	ساخت type **le_پچھلا;	/* address of previous next element */	\
}

/*
 * List access methods.
 */
#تعریف_کرو	LIST_پہلا(سر)		((سر)->lh_پہلا)
#تعریف_کرو	LIST_انتہا(سر)			تہی
#تعریف_کرو	LIST_خالی_خانے(سر)		((سر)->lh_پہلا == LIST_انتہا(سر))
#تعریف_کرو	LIST_اگلا(elm, میدان)		((elm)->میدان.le_اگلا)

#تعریف_کرو	LIST_FOREACH(var, سر, میدان)					\
	برائے ((var) = ((سر)->lh_پہلا);				\
	    (var) != LIST_انتہا(سر);					\
	    (var) = ((var)->میدان.le_اگلا))

#تعریف_کرو	LIST_FOREACH_محفوظ(var, سر, میدان, tvar)			\
	برائے ((var) = LIST_پہلا((سر));				\
	    (var) != LIST_انتہا(سر) &&					\
	    ((tvar) = LIST_اگلا((var), میدان), 1);			\
	    (var) = (tvar))

#تعریف_کرو	LIST_MOVE(head1, head2) کرو {					\
	LIST_INIT((head2));						\
	اگر (!LIST_خالی_خانے((head1))) {					\
		(head2)->lh_پہلا = (head1)->lh_پہلا;			\
		LIST_INIT((head1));					\
	}								\
} جب (/*CONSTCOND*/0)

/*
 * List functions.
 */
#اگر defined(QUEUEDEBUG)
#تعریف_کرو	QUEUEDEBUG_LIST_داخل_کرو_سر(سر, elm, میدان)			\
	اگر ((سر)->lh_پہلا &&						\
	    (سر)->lh_پہلا->میدان.le_پچھلا != &(سر)->lh_پہلا)	\
		QUEUEDEBUG_ABORT("LIST_INSERT_HEAD %p %s:%d", (سر),	\
		    __FILE__, __LINE__);
#تعریف_کرو	QUEUEDEBUG_LIST_OP(elm, میدان)					\
	اگر ((elm)->میدان.le_اگلا &&					\
	    (elm)->میدان.le_اگلا->میدان.le_پچھلا !=			\
	    &(elm)->میدان.le_اگلا)					\
		QUEUEDEBUG_ABORT("LIST_* forw %p %s:%d", (elm),		\
		    __FILE__, __LINE__);				\
	اگر (*(elm)->میدان.le_پچھلا != (elm))				\
		QUEUEDEBUG_ABORT("LIST_* back %p %s:%d", (elm),		\
		    __FILE__, __LINE__);
#تعریف_کرو	QUEUEDEBUG_LIST_POSTREMOVE(elm, میدان)				\
	(elm)->میدان.le_اگلا = (خالی *)1L;				\
	(elm)->میدان.le_پچھلا = (خالی *)1L;
#ورنہ
#تعریف_کرو	QUEUEDEBUG_LIST_داخل_کرو_سر(سر, elm, میدان)
#تعریف_کرو	QUEUEDEBUG_LIST_OP(elm, میدان)
#تعریف_کرو	QUEUEDEBUG_LIST_POSTREMOVE(elm, میدان)
#اگر_ختم

#تعریف_کرو	LIST_INIT(سر) کرو {						\
	(سر)->lh_پہلا = LIST_انتہا(سر);				\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	LIST_داخل_کرو_AFTER(listelm, elm, میدان) کرو {			\
	QUEUEDEBUG_LIST_OP((listelm), میدان)				\
	اگر (((elm)->میدان.le_اگلا = (listelm)->میدان.le_اگلا) != 	\
	    LIST_انتہا(سر))						\
		(listelm)->میدان.le_اگلا->میدان.le_پچھلا =		\
		    &(elm)->میدان.le_اگلا;				\
	(listelm)->میدان.le_اگلا = (elm);				\
	(elm)->میدان.le_پچھلا = &(listelm)->میدان.le_اگلا;		\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	LIST_داخل_کرو_BEFORE(listelm, elm, میدان) کرو {			\
	QUEUEDEBUG_LIST_OP((listelm), میدان)				\
	(elm)->میدان.le_پچھلا = (listelm)->میدان.le_پچھلا;		\
	(elm)->میدان.le_اگلا = (listelm);				\
	*(listelm)->میدان.le_پچھلا = (elm);				\
	(listelm)->میدان.le_پچھلا = &(elm)->میدان.le_اگلا;		\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	LIST_داخل_کرو_سر(سر, elm, میدان) کرو {				\
	QUEUEDEBUG_LIST_داخل_کرو_سر((سر), (elm), میدان)		\
	اگر (((elm)->میدان.le_اگلا = (سر)->lh_پہلا) != LIST_انتہا(سر))\
		(سر)->lh_پہلا->میدان.le_پچھلا = &(elm)->میدان.le_اگلا;\
	(سر)->lh_پہلا = (elm);					\
	(elm)->میدان.le_پچھلا = &(سر)->lh_پہلا;			\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	LIST_REMOVE(elm, میدان) کرو {					\
	QUEUEDEBUG_LIST_OP((elm), میدان)				\
	اگر ((elm)->میدان.le_اگلا != تہی)				\
		(elm)->میدان.le_اگلا->میدان.le_پچھلا = 			\
		    (elm)->میدان.le_پچھلا;				\
	*(elm)->میدان.le_پچھلا = (elm)->میدان.le_اگلا;			\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), میدان)			\
} جب (/*CONSTCOND*/0)

#تعریف_کرو LIST_REPLACE(elm, elm2, میدان) کرو {				\
	اگر (((elm2)->میدان.le_اگلا = (elm)->میدان.le_اگلا) != تہی)	\
		(elm2)->میدان.le_اگلا->میدان.le_پچھلا =			\
		    &(elm2)->میدان.le_اگلا;				\
	(elm2)->میدان.le_پچھلا = (elm)->میدان.le_پچھلا;			\
	*(elm2)->میدان.le_پچھلا = (elm2);				\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), میدان)			\
} جب (/*CONSTCOND*/0)

/*
 * Simple queue definitions.
 */
#تعریف_کرو	SIMPLEQ_سر(name, type)					\
ساخت name {								\
	ساخت type *sqh_پہلا;	/* first element */			\
	ساخت type **sqh_آخری;	/* addr of last next element */		\
}

#تعریف_کرو	SIMPLEQ_سر_INITIALIZER(سر)					\
	{ تہی, &(سر).sqh_پہلا }

#تعریف_کرو	SIMPLEQ_اندراج(type)						\
ساخت {								\
	ساخت type *sqe_اگلا;	/* next element */			\
}

/*
 * Simple queue access methods.
 */
#تعریف_کرو	SIMPLEQ_پہلا(سر)		((سر)->sqh_پہلا)
#تعریف_کرو	SIMPLEQ_انتہا(سر)		تہی
#تعریف_کرو	SIMPLEQ_خالی_خانے(سر)		((سر)->sqh_پہلا == SIMPLEQ_انتہا(سر))
#تعریف_کرو	SIMPLEQ_اگلا(elm, میدان)	((elm)->میدان.sqe_اگلا)

#تعریف_کرو	SIMPLEQ_FOREACH(var, سر, میدان)				\
	برائے ((var) = ((سر)->sqh_پہلا);				\
	    (var) != SIMPLEQ_انتہا(سر);					\
	    (var) = ((var)->میدان.sqe_اگلا))

#تعریف_کرو	SIMPLEQ_FOREACH_محفوظ(var, سر, میدان, اگلا)			\
	برائے ((var) = ((سر)->sqh_پہلا);				\
	    (var) != SIMPLEQ_انتہا(سر) &&				\
	    ((اگلا = ((var)->میدان.sqe_اگلا)), 1);			\
	    (var) = (اگلا))

/*
 * Simple queue functions.
 */
#تعریف_کرو	SIMPLEQ_INIT(سر) کرو {						\
	(سر)->sqh_پہلا = تہی;					\
	(سر)->sqh_آخری = &(سر)->sqh_پہلا;				\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_داخل_کرو_سر(سر, elm, میدان) کرو {			\
	اگر (((elm)->میدان.sqe_اگلا = (سر)->sqh_پہلا) == تہی)	\
		(سر)->sqh_آخری = &(elm)->میدان.sqe_اگلا;		\
	(سر)->sqh_پہلا = (elm);					\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_داخل_کرو_دم(سر, elm, میدان) کرو {			\
	(elm)->میدان.sqe_اگلا = تہی;					\
	*(سر)->sqh_آخری = (elm);					\
	(سر)->sqh_آخری = &(elm)->میدان.sqe_اگلا;			\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_داخل_کرو_AFTER(سر, listelm, elm, میدان) کرو {		\
	اگر (((elm)->میدان.sqe_اگلا = (listelm)->میدان.sqe_اگلا) == تہی)\
		(سر)->sqh_آخری = &(elm)->میدان.sqe_اگلا;		\
	(listelm)->میدان.sqe_اگلا = (elm);				\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_REMOVE_سر(سر, میدان) کرو {				\
	اگر (((سر)->sqh_پہلا = (سر)->sqh_پہلا->میدان.sqe_اگلا) == تہی) \
		(سر)->sqh_آخری = &(سر)->sqh_پہلا;			\
} جب (/*CONSTCOND*/0)

#تعریف_کرو SIMPLEQ_REMOVE_AFTER(سر, elm, میدان) کرو {			\
	اگر (((elm)->میدان.sqe_اگلا = (elm)->میدان.sqe_اگلا->میدان.sqe_اگلا) \
	    == تہی)							\
		(سر)->sqh_آخری = &(elm)->میدان.sqe_اگلا;		\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_REMOVE(سر, elm, type, میدان) کرو {			\
	اگر ((سر)->sqh_پہلا == (elm)) {				\
		SIMPLEQ_REMOVE_سر((سر), میدان);			\
	} ورنہ {							\
		ساخت type *curelm = (سر)->sqh_پہلا;		\
		جب (curelm->میدان.sqe_اگلا != (elm))			\
			curelm = curelm->میدان.sqe_اگلا;		\
		اگر ((curelm->میدان.sqe_اگلا =				\
			curelm->میدان.sqe_اگلا->میدان.sqe_اگلا) == تہی) \
			    (سر)->sqh_آخری = &(curelm)->میدان.sqe_اگلا; \
	}								\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_CONCAT(head1, head2) کرو {				\
	اگر (!SIMPLEQ_خالی_خانے((head2))) {					\
		*(head1)->sqh_آخری = (head2)->sqh_پہلا;		\
		(head1)->sqh_آخری = (head2)->sqh_آخری;		\
		SIMPLEQ_INIT((head2));					\
	}								\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	SIMPLEQ_آخری(سر, type, میدان)					\
	(SIMPLEQ_خالی_خانے((سر)) ?						\
		تہی :							\
	        ((ساخت type *)(خالی *)				\
		((حرف *)((سر)->sqh_آخری) - offsetof(ساخت type, میدان))))

/*
 * Tail queue definitions.
 */
#تعریف_کرو	TAILQ_سر_2(name, type, qual)					\
ساخت name {								\
	qual type *tqh_پہلا;		/* first element */		\
	qual type *qual *tqh_آخری;	/* addr of last next element */	\
}
#تعریف_کرو TAILQ_سر(name, type)	TAILQ_سر_2(name, ساخت type,)

#تعریف_کرو	TAILQ_سر_INITIALIZER(سر)					\
	{ TAILQ_انتہا(سر), &(سر).tqh_پہلا }

#تعریف_کرو	TAILQ_اندراج_2(type, qual)					\
ساخت {								\
	qual type *tqe_اگلا;		/* next element */		\
	qual type *qual *tqe_پچھلا;	/* address of previous next element */\
}
#تعریف_کرو TAILQ_اندراج(type)	TAILQ_اندراج_2(ساخت type,)

/*
 * Tail queue access methods.
 */
#تعریف_کرو	TAILQ_پہلا(سر)		((سر)->tqh_پہلا)
#تعریف_کرو	TAILQ_انتہا(سر)			(تہی)
#تعریف_کرو	TAILQ_اگلا(elm, میدان)		((elm)->میدان.tqe_اگلا)
#تعریف_کرو	TAILQ_آخری(سر, headname) \
	(*(((ساخت headname *)((سر)->tqh_آخری))->tqh_آخری))
#تعریف_کرو	TAILQ_پچھلا(elm, headname, میدان) \
	(*(((ساخت headname *)((elm)->میدان.tqe_پچھلا))->tqh_آخری))
#تعریف_کرو	TAILQ_خالی_خانے(سر)		(TAILQ_پہلا(سر) == TAILQ_انتہا(سر))


#تعریف_کرو	TAILQ_FOREACH(var, سر, میدان)					\
	برائے ((var) = ((سر)->tqh_پہلا);				\
	    (var) != TAILQ_انتہا(سر);					\
	    (var) = ((var)->میدان.tqe_اگلا))

#تعریف_کرو	TAILQ_FOREACH_محفوظ(var, سر, میدان, اگلا)			\
	برائے ((var) = ((سر)->tqh_پہلا);				\
	    (var) != TAILQ_انتہا(سر) &&					\
	    ((اگلا) = TAILQ_اگلا(var, میدان), 1); (var) = (اگلا))

#تعریف_کرو	TAILQ_FOREACH_الٹ(var, سر, headname, میدان)		\
	برائے ((var) = (*(((ساخت headname *)((سر)->tqh_آخری))->tqh_آخری));\
	    (var) != TAILQ_انتہا(سر);					\
	    (var) = (*(((ساخت headname *)((var)->میدان.tqe_پچھلا))->tqh_آخری)))

#تعریف_کرو	TAILQ_FOREACH_الٹ_محفوظ(var, سر, headname, میدان, پچھلا)	\
	برائے ((var) = TAILQ_آخری((سر), headname);			\
	    (var) != TAILQ_انتہا(سر) && 				\
	    ((پچھلا) = TAILQ_پچھلا((var), headname, میدان), 1); (var) = (پچھلا))

/*
 * Tail queue functions.
 */
#اگر defined(QUEUEDEBUG)
#تعریف_کرو	QUEUEDEBUG_TAILQ_داخل_کرو_سر(سر, elm, میدان)			\
	اگر ((سر)->tqh_پہلا &&					\
	    (سر)->tqh_پہلا->میدان.tqe_پچھلا != &(سر)->tqh_پہلا)	\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_HEAD %p %s:%d", (سر),	\
		    __FILE__, __LINE__);
#تعریف_کرو	QUEUEDEBUG_TAILQ_داخل_کرو_دم(سر, elm, میدان)			\
	اگر (*(سر)->tqh_آخری != تہی)					\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_TAIL %p %s:%d", (سر),	\
		    __FILE__, __LINE__);
#تعریف_کرو	QUEUEDEBUG_TAILQ_OP(elm, میدان)					\
	اگر ((elm)->میدان.tqe_اگلا &&					\
	    (elm)->میدان.tqe_اگلا->میدان.tqe_پچھلا !=			\
	    &(elm)->میدان.tqe_اگلا)					\
		QUEUEDEBUG_ABORT("TAILQ_* forw %p %s:%d", (elm),	\
		    __FILE__, __LINE__);				\
	اگر (*(elm)->میدان.tqe_پچھلا != (elm))				\
		QUEUEDEBUG_ABORT("TAILQ_* back %p %s:%d", (elm),	\
		    __FILE__, __LINE__);
#تعریف_کرو	QUEUEDEBUG_TAILQ_PREREMOVE(سر, elm, میدان)			\
	اگر ((elm)->میدان.tqe_اگلا == تہی &&				\
	    (سر)->tqh_آخری != &(elm)->میدان.tqe_اگلا)			\
		QUEUEDEBUG_ABORT("TAILQ_PREREMOVE head %p elm %p %s:%d",\
		    (سر), (elm), __FILE__, __LINE__);
#تعریف_کرو	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, میدان)				\
	(elm)->میدان.tqe_اگلا = (خالی *)1L;				\
	(elm)->میدان.tqe_پچھلا = (خالی *)1L;
#ورنہ
#تعریف_کرو	QUEUEDEBUG_TAILQ_داخل_کرو_سر(سر, elm, میدان)
#تعریف_کرو	QUEUEDEBUG_TAILQ_داخل_کرو_دم(سر, elm, میدان)
#تعریف_کرو	QUEUEDEBUG_TAILQ_OP(elm, میدان)
#تعریف_کرو	QUEUEDEBUG_TAILQ_PREREMOVE(سر, elm, میدان)
#تعریف_کرو	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, میدان)
#اگر_ختم

#تعریف_کرو	TAILQ_INIT(سر) کرو {						\
	(سر)->tqh_پہلا = TAILQ_انتہا(سر);				\
	(سر)->tqh_آخری = &(سر)->tqh_پہلا;				\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	TAILQ_داخل_کرو_سر(سر, elm, میدان) کرو {			\
	QUEUEDEBUG_TAILQ_داخل_کرو_سر((سر), (elm), میدان)		\
	اگر (((elm)->میدان.tqe_اگلا = (سر)->tqh_پہلا) != TAILQ_انتہا(سر))\
		(سر)->tqh_پہلا->میدان.tqe_پچھلا =			\
		    &(elm)->میدان.tqe_اگلا;				\
	ورنہ								\
		(سر)->tqh_آخری = &(elm)->میدان.tqe_اگلا;		\
	(سر)->tqh_پہلا = (elm);					\
	(elm)->میدان.tqe_پچھلا = &(سر)->tqh_پہلا;			\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	TAILQ_داخل_کرو_دم(سر, elm, میدان) کرو {			\
	QUEUEDEBUG_TAILQ_داخل_کرو_دم((سر), (elm), میدان)		\
	(elm)->میدان.tqe_اگلا = TAILQ_انتہا(سر);			\
	(elm)->میدان.tqe_پچھلا = (سر)->tqh_آخری;			\
	*(سر)->tqh_آخری = (elm);					\
	(سر)->tqh_آخری = &(elm)->میدان.tqe_اگلا;			\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	TAILQ_داخل_کرو_AFTER(سر, listelm, elm, میدان) کرو {		\
	QUEUEDEBUG_TAILQ_OP((listelm), میدان)				\
	اگر (((elm)->میدان.tqe_اگلا = (listelm)->میدان.tqe_اگلا) != 	\
	    TAILQ_انتہا(سر))						\
		(elm)->میدان.tqe_اگلا->میدان.tqe_پچھلا = 		\
		    &(elm)->میدان.tqe_اگلا;				\
	ورنہ								\
		(سر)->tqh_آخری = &(elm)->میدان.tqe_اگلا;		\
	(listelm)->میدان.tqe_اگلا = (elm);				\
	(elm)->میدان.tqe_پچھلا = &(listelm)->میدان.tqe_اگلا;		\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	TAILQ_داخل_کرو_BEFORE(listelm, elm, میدان) کرو {			\
	QUEUEDEBUG_TAILQ_OP((listelm), میدان)				\
	(elm)->میدان.tqe_پچھلا = (listelm)->میدان.tqe_پچھلا;		\
	(elm)->میدان.tqe_اگلا = (listelm);				\
	*(listelm)->میدان.tqe_پچھلا = (elm);				\
	(listelm)->میدان.tqe_پچھلا = &(elm)->میدان.tqe_اگلا;		\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	TAILQ_REMOVE(سر, elm, میدان) کرو {				\
	QUEUEDEBUG_TAILQ_PREREMOVE((سر), (elm), میدان)		\
	QUEUEDEBUG_TAILQ_OP((elm), میدان)				\
	اگر (((elm)->میدان.tqe_اگلا) != TAILQ_انتہا(سر))			\
		(elm)->میدان.tqe_اگلا->میدان.tqe_پچھلا = 		\
		    (elm)->میدان.tqe_پچھلا;				\
	ورنہ								\
		(سر)->tqh_آخری = (elm)->میدان.tqe_پچھلا;		\
	*(elm)->میدان.tqe_پچھلا = (elm)->میدان.tqe_اگلا;			\
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), میدان);			\
} جب (/*CONSTCOND*/0)

#تعریف_کرو TAILQ_REPLACE(سر, elm, elm2, میدان) کرو {                      \
        اگر (((elm2)->میدان.tqe_اگلا = (elm)->میدان.tqe_اگلا) != 	\
	    TAILQ_انتہا(سر))   						\
                (elm2)->میدان.tqe_اگلا->میدان.tqe_پچھلا =                \
                    &(elm2)->میدان.tqe_اگلا;                            \
        ورنہ                                                            \
                (سر)->tqh_آخری = &(elm2)->میدان.tqe_اگلا;             \
        (elm2)->میدان.tqe_پچھلا = (elm)->میدان.tqe_پچھلا;                 \
        *(elm2)->میدان.tqe_پچھلا = (elm2);                               \
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), میدان);			\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	TAILQ_CONCAT(head1, head2, میدان) کرو {				\
	اگر (!TAILQ_خالی_خانے(head2)) {					\
		*(head1)->tqh_آخری = (head2)->tqh_پہلا;		\
		(head2)->tqh_پہلا->میدان.tqe_پچھلا = (head1)->tqh_آخری;	\
		(head1)->tqh_آخری = (head2)->tqh_آخری;			\
		TAILQ_INIT((head2));					\
	}								\
} جب (/*CONSTCOND*/0)

/*
 * Singly-linked Tail queue declarations.
 */
#تعریف_کرو	STAILQ_سر(name, type)						\
ساخت name {								\
	ساخت type *stqh_پہلا;	/* first element */		\
	ساخت type **stqh_آخری;	/* addr of last next element */	\
}

#تعریف_کرو	STAILQ_سر_INITIALIZER(سر)					\
	{ تہی, &(سر).stqh_پہلا }

#تعریف_کرو	STAILQ_اندراج(type)						\
ساخت {								\
	ساخت type *stqe_اگلا;	/* next element */			\
}

/*
 * Singly-linked Tail queue access methods.
 */
#تعریف_کرو	STAILQ_پہلا(سر)	((سر)->stqh_پہلا)
#تعریف_کرو	STAILQ_انتہا(سر)	تہی
#تعریف_کرو	STAILQ_اگلا(elm, میدان)	((elm)->میدان.stqe_اگلا)
#تعریف_کرو	STAILQ_خالی_خانے(سر)	(STAILQ_پہلا(سر) == STAILQ_انتہا(سر))

/*
 * Singly-linked Tail queue functions.
 */
#تعریف_کرو	STAILQ_INIT(سر) کرو {						\
	(سر)->stqh_پہلا = تہی;					\
	(سر)->stqh_آخری = &(سر)->stqh_پہلا;				\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_داخل_کرو_سر(سر, elm, میدان) کرو {			\
	اگر (((elm)->میدان.stqe_اگلا = (سر)->stqh_پہلا) == تہی)	\
		(سر)->stqh_آخری = &(elm)->میدان.stqe_اگلا;		\
	(سر)->stqh_پہلا = (elm);					\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_داخل_کرو_دم(سر, elm, میدان) کرو {			\
	(elm)->میدان.stqe_اگلا = تہی;					\
	*(سر)->stqh_آخری = (elm);					\
	(سر)->stqh_آخری = &(elm)->میدان.stqe_اگلا;			\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_داخل_کرو_AFTER(سر, listelm, elm, میدان) کرو {		\
	اگر (((elm)->میدان.stqe_اگلا = (listelm)->میدان.stqe_اگلا) == تہی)\
		(سر)->stqh_آخری = &(elm)->میدان.stqe_اگلا;		\
	(listelm)->میدان.stqe_اگلا = (elm);				\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_REMOVE_سر(سر, میدان) کرو {				\
	اگر (((سر)->stqh_پہلا = (سر)->stqh_پہلا->میدان.stqe_اگلا) == تہی) \
		(سر)->stqh_آخری = &(سر)->stqh_پہلا;			\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_REMOVE(سر, elm, type, میدان) کرو {			\
	اگر ((سر)->stqh_پہلا == (elm)) {				\
		STAILQ_REMOVE_سر((سر), میدان);			\
	} ورنہ {							\
		ساخت type *curelm = (سر)->stqh_پہلا;		\
		جب (curelm->میدان.stqe_اگلا != (elm))			\
			curelm = curelm->میدان.stqe_اگلا;		\
		اگر ((curelm->میدان.stqe_اگلا =				\
			curelm->میدان.stqe_اگلا->میدان.stqe_اگلا) == تہی) \
			    (سر)->stqh_آخری = &(curelm)->میدان.stqe_اگلا; \
	}								\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_FOREACH(var, سر, میدان)				\
	برائے ((var) = ((سر)->stqh_پہلا);				\
		(var);							\
		(var) = ((var)->میدان.stqe_اگلا))

#تعریف_کرو	STAILQ_FOREACH_محفوظ(var, سر, میدان, tvar)			\
	برائے ((var) = STAILQ_پہلا((سر));				\
	    (var) && ((tvar) = STAILQ_اگلا((var), میدان), 1);		\
	    (var) = (tvar))

#تعریف_کرو	STAILQ_CONCAT(head1, head2) کرو {				\
	اگر (!STAILQ_خالی_خانے((head2))) {					\
		*(head1)->stqh_آخری = (head2)->stqh_پہلا;		\
		(head1)->stqh_آخری = (head2)->stqh_آخری;		\
		STAILQ_INIT((head2));					\
	}								\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	STAILQ_آخری(سر, type, میدان)					\
	(STAILQ_خالی_خانے((سر)) ?						\
		تہی :							\
	        ((ساخت type *)(خالی *)				\
		((حرف *)((سر)->stqh_آخری) - offsetof(ساخت type, میدان))))


#اگر_غیر_تعریف_شدہ _KERNEL
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
ساکن __inline ثابت خالی * __launder_type(ثابت خالی *);
ساکن __inline ثابت خالی *
__launder_type(ثابت خالی *__x)
{
	__asm __volatile("" : "+r" (__x));
	واپس __x;
}

#اگر defined(QUEUEDEBUG)
#تعریف_کرو QUEUEDEBUG_CIRCLEQ_سر(سر, میدان)				\
	اگر ((سر)->cqh_پہلا != CIRCLEQ_ENDC(سر) &&			\
	    (سر)->cqh_پہلا->میدان.cqe_پچھلا != CIRCLEQ_ENDC(سر))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head forw %p %s:%d", (سر),	\
		      __FILE__, __LINE__);				\
	اگر ((سر)->cqh_آخری != CIRCLEQ_ENDC(سر) &&			\
	    (سر)->cqh_آخری->میدان.cqe_اگلا != CIRCLEQ_ENDC(سر))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head back %p %s:%d", (سر),	\
		      __FILE__, __LINE__);
#تعریف_کرو QUEUEDEBUG_CIRCLEQ_ELM(سر, elm, میدان)			\
	اگر ((elm)->میدان.cqe_اگلا == CIRCLEQ_ENDC(سر)) {		\
		اگر ((سر)->cqh_آخری != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm last %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} ورنہ {							\
		اگر ((elm)->میدان.cqe_اگلا->میدان.cqe_پچھلا != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm forw %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}								\
	اگر ((elm)->میدان.cqe_پچھلا == CIRCLEQ_ENDC(سر)) {		\
		اگر ((سر)->cqh_پہلا != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm first %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} ورنہ {							\
		اگر ((elm)->میدان.cqe_پچھلا->میدان.cqe_اگلا != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm prev %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}
#تعریف_کرو QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, میدان)			\
	(elm)->میدان.cqe_اگلا = (خالی *)1L;				\
	(elm)->میدان.cqe_پچھلا = (خالی *)1L;
#ورنہ
#تعریف_کرو QUEUEDEBUG_CIRCLEQ_سر(سر, میدان)
#تعریف_کرو QUEUEDEBUG_CIRCLEQ_ELM(سر, elm, میدان)
#تعریف_کرو QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, میدان)
#اگر_ختم

#تعریف_کرو	CIRCLEQ_سر(name, type)					\
ساخت name {								\
	ساخت type *cqh_پہلا;		/* first element */		\
	ساخت type *cqh_آخری;		/* last element */		\
}

#تعریف_کرو	CIRCLEQ_سر_INITIALIZER(سر)					\
	{ CIRCLEQ_انتہا(&سر), CIRCLEQ_انتہا(&سر) }

#تعریف_کرو	CIRCLEQ_اندراج(type)						\
ساخت {								\
	ساخت type *cqe_اگلا;		/* next element */		\
	ساخت type *cqe_پچھلا;		/* previous element */		\
}

/*
 * Circular queue functions.
 */
#تعریف_کرو	CIRCLEQ_INIT(سر) کرو {						\
	(سر)->cqh_پہلا = CIRCLEQ_انتہا(سر);				\
	(سر)->cqh_آخری = CIRCLEQ_انتہا(سر);				\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	CIRCLEQ_داخل_کرو_AFTER(سر, listelm, elm, میدان) کرو {		\
	QUEUEDEBUG_CIRCLEQ_سر((سر), میدان)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر), (listelm), میدان)		\
	(elm)->میدان.cqe_اگلا = (listelm)->میدان.cqe_اگلا;		\
	(elm)->میدان.cqe_پچھلا = (listelm);				\
	اگر ((listelm)->میدان.cqe_اگلا == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_آخری = (elm);				\
	ورنہ								\
		(listelm)->میدان.cqe_اگلا->میدان.cqe_پچھلا = (elm);	\
	(listelm)->میدان.cqe_اگلا = (elm);				\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	CIRCLEQ_داخل_کرو_BEFORE(سر, listelm, elm, میدان) کرو {		\
	QUEUEDEBUG_CIRCLEQ_سر((سر), میدان)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر), (listelm), میدان)		\
	(elm)->میدان.cqe_اگلا = (listelm);				\
	(elm)->میدان.cqe_پچھلا = (listelm)->میدان.cqe_پچھلا;		\
	اگر ((listelm)->میدان.cqe_پچھلا == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_پہلا = (elm);				\
	ورنہ								\
		(listelm)->میدان.cqe_پچھلا->میدان.cqe_اگلا = (elm);	\
	(listelm)->میدان.cqe_پچھلا = (elm);				\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	CIRCLEQ_داخل_کرو_سر(سر, elm, میدان) کرو {			\
	QUEUEDEBUG_CIRCLEQ_سر((سر), میدان)				\
	(elm)->میدان.cqe_اگلا = (سر)->cqh_پہلا;			\
	(elm)->میدان.cqe_پچھلا = CIRCLEQ_انتہا(سر);			\
	اگر ((سر)->cqh_آخری == CIRCLEQ_ENDC(سر))			\
		(سر)->cqh_آخری = (elm);				\
	ورنہ								\
		(سر)->cqh_پہلا->میدان.cqe_پچھلا = (elm);		\
	(سر)->cqh_پہلا = (elm);					\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	CIRCLEQ_داخل_کرو_دم(سر, elm, میدان) کرو {			\
	QUEUEDEBUG_CIRCLEQ_سر((سر), میدان)				\
	(elm)->میدان.cqe_اگلا = CIRCLEQ_انتہا(سر);			\
	(elm)->میدان.cqe_پچھلا = (سر)->cqh_آخری;			\
	اگر ((سر)->cqh_پہلا == CIRCLEQ_ENDC(سر))			\
		(سر)->cqh_پہلا = (elm);				\
	ورنہ								\
		(سر)->cqh_آخری->میدان.cqe_اگلا = (elm);		\
	(سر)->cqh_آخری = (elm);					\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	CIRCLEQ_REMOVE(سر, elm, میدان) کرو {				\
	QUEUEDEBUG_CIRCLEQ_سر((سر), میدان)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر), (elm), میدان)			\
	اگر ((elm)->میدان.cqe_اگلا == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_آخری = (elm)->میدان.cqe_پچھلا;		\
	ورنہ								\
		(elm)->میدان.cqe_اگلا->میدان.cqe_پچھلا =			\
		    (elm)->میدان.cqe_پچھلا;				\
	اگر ((elm)->میدان.cqe_پچھلا == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_پہلا = (elm)->میدان.cqe_اگلا;		\
	ورنہ								\
		(elm)->میدان.cqe_پچھلا->میدان.cqe_اگلا =			\
		    (elm)->میدان.cqe_اگلا;				\
	QUEUEDEBUG_CIRCLEQ_POSTREMOVE((elm), میدان)			\
} جب (/*CONSTCOND*/0)

#تعریف_کرو	CIRCLEQ_FOREACH(var, سر, میدان)				\
	برائے ((var) = ((سر)->cqh_پہلا);				\
		(var) != CIRCLEQ_ENDC(سر);				\
		(var) = ((var)->میدان.cqe_اگلا))

#تعریف_کرو	CIRCLEQ_FOREACH_الٹ(var, سر, میدان)			\
	برائے ((var) = ((سر)->cqh_آخری);				\
		(var) != CIRCLEQ_ENDC(سر);				\
		(var) = ((var)->میدان.cqe_پچھلا))

/*
 * Circular queue access methods.
 */
#تعریف_کرو	CIRCLEQ_پہلا(سر)		((سر)->cqh_پہلا)
#تعریف_کرو	CIRCLEQ_آخری(سر)		((سر)->cqh_آخری)
/* For comparisons */
#تعریف_کرو	CIRCLEQ_ENDC(سر)		(__launder_type(سر))
/* For assignments */
#تعریف_کرو	CIRCLEQ_انتہا(سر)		((خالی *)(سر))
#تعریف_کرو	CIRCLEQ_اگلا(elm, میدان)	((elm)->میدان.cqe_اگلا)
#تعریف_کرو	CIRCLEQ_پچھلا(elm, میدان)	((elm)->میدان.cqe_پچھلا)
#تعریف_کرو	CIRCLEQ_خالی_خانے(سر)						\
    (CIRCLEQ_پہلا(سر) == CIRCLEQ_ENDC(سر))

#تعریف_کرو CIRCLEQ_LOOP_اگلا(سر, elm, میدان)				\
	(((elm)->میدان.cqe_اگلا == CIRCLEQ_ENDC(سر))			\
	    ? ((سر)->cqh_پہلا)					\
	    : (elm->میدان.cqe_اگلا))
#تعریف_کرو CIRCLEQ_LOOP_پچھلا(سر, elm, میدان)				\
	(((elm)->میدان.cqe_پچھلا == CIRCLEQ_ENDC(سر))			\
	    ? ((سر)->cqh_آخری)					\
	    : (elm->میدان.cqe_پچھلا))
#اگر_ختم /* !_KERNEL */

#اگر_ختم	/* !_SYS_QUEUE_H_ */
