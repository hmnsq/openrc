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

#اگر_تعریف_نشده	SYS_صف_H
#تعریف_کن	SYS_صف_H

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
#اگر_تعریف_شده __NetBSD__
#ضمن <sys/null.h>
#پایان_اگر

#إذا defined(QUEUEDEBUG)
# إذا defined(_KERNEL)
#  define QUEUEDEBUG_ABORT(...) panic(__VA_ARGS__)
# وإلا
#  include <err.h>
#  define QUEUEDEBUG_ABORT(...) err(1, __VA_ARGS__)
# endif
#پایان_اگر

/*
 * Singly-linked List definitions.
 */
#تعریف_کن	SLIST_سر(name, type)						\
بنية name {								\
	بنية type *slh_اولی;	/* first element */			\
}

#تعریف_کن	SLIST_سر_INITIALIZER(سر)					\
	{ عدم }

#تعریف_کن	SLIST_مدخل(type)						\
بنية {								\
	بنية type *sle_بعدی;	/* next element */			\
}

/*
 * Singly-linked List access methods.
 */
#تعریف_کن	SLIST_اولی(سر)	((سر)->slh_اولی)
#تعریف_کن	SLIST_انتها(سر)		عدم
#تعریف_کن	SLIST_خالی(سر)	((سر)->slh_اولی == عدم)
#تعریف_کن	SLIST_بعدی(elm, فیلد)	((elm)->فیلد.sle_بعدی)

#تعریف_کن	SLIST_FOREACH(var, سر, فیلد)					\
	لكل((var) = (سر)->slh_اولی;					\
	    (var) != SLIST_انتها(سر);					\
	    (var) = (var)->فیلد.sle_بعدی)

#تعریف_کن	SLIST_FOREACH_امن(var, سر, فیلد, tvar)			\
	لكل ((var) = SLIST_اولی((سر));				\
	    (var) != SLIST_انتها(سر) &&					\
	    ((tvar) = SLIST_بعدی((var), فیلد), 1);			\
	    (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#تعریف_کن	SLIST_INIT(سر) افعل {						\
	(سر)->slh_اولی = SLIST_انتها(سر);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	SLIST_درج_AFTER(slistelm, elm, فیلد) افعل {			\
	(elm)->فیلد.sle_بعدی = (slistelm)->فیلد.sle_بعدی;		\
	(slistelm)->فیلد.sle_بعدی = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	SLIST_درج_سر(سر, elm, فیلد) افعل {			\
	(elm)->فیلد.sle_بعدی = (سر)->slh_اولی;			\
	(سر)->slh_اولی = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	SLIST_REMOVE_AFTER(slistelm, فیلد) افعل {			\
	(slistelm)->فیلد.sle_بعدی =					\
	    SLIST_بعدی(SLIST_بعدی((slistelm), فیلد), فیلد);		\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	SLIST_REMOVE_سر(سر, فیلد) افعل {				\
	(سر)->slh_اولی = (سر)->slh_اولی->فیلد.sle_بعدی;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	SLIST_REMOVE(سر, elm, type, فیلد) افعل {			\
	إذا ((سر)->slh_اولی == (elm)) {				\
		SLIST_REMOVE_سر((سر), فیلد);			\
	}								\
	وإلا {								\
		بنية type *curelm = (سر)->slh_اولی;		\
		طالما(curelm->فیلد.sle_بعدی != (elm))			\
			curelm = curelm->فیلد.sle_بعدی;		\
		curelm->فیلد.sle_بعدی =				\
		    curelm->فیلد.sle_بعدی->فیلد.sle_بعدی;		\
	}								\
} طالما (/*CONSTCOND*/0)


/*
 * List definitions.
 */
#تعریف_کن	LIST_سر(name, type)						\
بنية name {								\
	بنية type *lh_اولی;	/* first element */			\
}

#تعریف_کن	LIST_سر_INITIALIZER(سر)					\
	{ عدم }

#تعریف_کن	LIST_مدخل(type)						\
بنية {								\
	بنية type *le_بعدی;	/* next element */			\
	بنية type **le_prev;	/* address of previous next element */	\
}

/*
 * List access methods.
 */
#تعریف_کن	LIST_اولی(سر)		((سر)->lh_اولی)
#تعریف_کن	LIST_انتها(سر)			عدم
#تعریف_کن	LIST_خالی(سر)		((سر)->lh_اولی == LIST_انتها(سر))
#تعریف_کن	LIST_بعدی(elm, فیلد)		((elm)->فیلد.le_بعدی)

#تعریف_کن	LIST_FOREACH(var, سر, فیلد)					\
	لكل ((var) = ((سر)->lh_اولی);				\
	    (var) != LIST_انتها(سر);					\
	    (var) = ((var)->فیلد.le_بعدی))

#تعریف_کن	LIST_FOREACH_امن(var, سر, فیلد, tvar)			\
	لكل ((var) = LIST_اولی((سر));				\
	    (var) != LIST_انتها(سر) &&					\
	    ((tvar) = LIST_بعدی((var), فیلد), 1);			\
	    (var) = (tvar))

#تعریف_کن	LIST_MOVE(head1, head2) افعل {					\
	LIST_INIT((head2));						\
	إذا (!LIST_خالی((head1))) {					\
		(head2)->lh_اولی = (head1)->lh_اولی;			\
		LIST_INIT((head1));					\
	}								\
} طالما (/*CONSTCOND*/0)

/*
 * List functions.
 */
#إذا defined(QUEUEDEBUG)
#تعریف_کن	QUEUEDEBUG_LIST_درج_سر(سر, elm, فیلد)			\
	إذا ((سر)->lh_اولی &&						\
	    (سر)->lh_اولی->فیلد.le_prev != &(سر)->lh_اولی)	\
		QUEUEDEBUG_ABORT("LIST_INSERT_HEAD %p %s:%d", (سر),	\
		    __FILE__, __LINE__);
#تعریف_کن	QUEUEDEBUG_LIST_OP(elm, فیلد)					\
	إذا ((elm)->فیلد.le_بعدی &&					\
	    (elm)->فیلد.le_بعدی->فیلد.le_prev !=			\
	    &(elm)->فیلد.le_بعدی)					\
		QUEUEDEBUG_ABORT("LIST_* forw %p %s:%d", (elm),		\
		    __FILE__, __LINE__);				\
	إذا (*(elm)->فیلد.le_prev != (elm))				\
		QUEUEDEBUG_ABORT("LIST_* back %p %s:%d", (elm),		\
		    __FILE__, __LINE__);
#تعریف_کن	QUEUEDEBUG_LIST_POSTREMOVE(elm, فیلد)				\
	(elm)->فیلد.le_بعدی = (فراغ *)1L;				\
	(elm)->فیلد.le_prev = (فراغ *)1L;
#وإلا
#تعریف_کن	QUEUEDEBUG_LIST_درج_سر(سر, elm, فیلد)
#تعریف_کن	QUEUEDEBUG_LIST_OP(elm, فیلد)
#تعریف_کن	QUEUEDEBUG_LIST_POSTREMOVE(elm, فیلد)
#پایان_اگر

#تعریف_کن	LIST_INIT(سر) افعل {						\
	(سر)->lh_اولی = LIST_انتها(سر);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	LIST_درج_AFTER(listelm, elm, فیلد) افعل {			\
	QUEUEDEBUG_LIST_OP((listelm), فیلد)				\
	إذا (((elm)->فیلد.le_بعدی = (listelm)->فیلد.le_بعدی) != 	\
	    LIST_انتها(سر))						\
		(listelm)->فیلد.le_بعدی->فیلد.le_prev =		\
		    &(elm)->فیلد.le_بعدی;				\
	(listelm)->فیلد.le_بعدی = (elm);				\
	(elm)->فیلد.le_prev = &(listelm)->فیلد.le_بعدی;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	LIST_درج_BEFORE(listelm, elm, فیلد) افعل {			\
	QUEUEDEBUG_LIST_OP((listelm), فیلد)				\
	(elm)->فیلد.le_prev = (listelm)->فیلد.le_prev;		\
	(elm)->فیلد.le_بعدی = (listelm);				\
	*(listelm)->فیلد.le_prev = (elm);				\
	(listelm)->فیلد.le_prev = &(elm)->فیلد.le_بعدی;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	LIST_درج_سر(سر, elm, فیلد) افعل {				\
	QUEUEDEBUG_LIST_درج_سر((سر), (elm), فیلد)		\
	إذا (((elm)->فیلد.le_بعدی = (سر)->lh_اولی) != LIST_انتها(سر))\
		(سر)->lh_اولی->فیلد.le_prev = &(elm)->فیلد.le_بعدی;\
	(سر)->lh_اولی = (elm);					\
	(elm)->فیلد.le_prev = &(سر)->lh_اولی;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	LIST_REMOVE(elm, فیلد) افعل {					\
	QUEUEDEBUG_LIST_OP((elm), فیلد)				\
	إذا ((elm)->فیلد.le_بعدی != عدم)				\
		(elm)->فیلد.le_بعدی->فیلد.le_prev = 			\
		    (elm)->فیلد.le_prev;				\
	*(elm)->فیلد.le_prev = (elm)->فیلد.le_بعدی;			\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), فیلد)			\
} طالما (/*CONSTCOND*/0)

#تعریف_کن LIST_REPLACE(elm, elm2, فیلد) افعل {				\
	إذا (((elm2)->فیلد.le_بعدی = (elm)->فیلد.le_بعدی) != عدم)	\
		(elm2)->فیلد.le_بعدی->فیلد.le_prev =			\
		    &(elm2)->فیلد.le_بعدی;				\
	(elm2)->فیلد.le_prev = (elm)->فیلد.le_prev;			\
	*(elm2)->فیلد.le_prev = (elm2);				\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), فیلد)			\
} طالما (/*CONSTCOND*/0)

/*
 * Simple queue definitions.
 */
#تعریف_کن	SIMPLEQ_سر(name, type)					\
بنية name {								\
	بنية type *sqh_اولی;	/* first element */			\
	بنية type **sqh_آخرین;	/* addr of last next element */		\
}

#تعریف_کن	SIMPLEQ_سر_INITIALIZER(سر)					\
	{ عدم, &(سر).sqh_اولی }

#تعریف_کن	SIMPLEQ_مدخل(type)						\
بنية {								\
	بنية type *sqe_بعدی;	/* next element */			\
}

/*
 * Simple queue access methods.
 */
#تعریف_کن	SIMPLEQ_اولی(سر)		((سر)->sqh_اولی)
#تعریف_کن	SIMPLEQ_انتها(سر)		عدم
#تعریف_کن	SIMPLEQ_خالی(سر)		((سر)->sqh_اولی == SIMPLEQ_انتها(سر))
#تعریف_کن	SIMPLEQ_بعدی(elm, فیلد)	((elm)->فیلد.sqe_بعدی)

#تعریف_کن	SIMPLEQ_FOREACH(var, سر, فیلد)				\
	لكل ((var) = ((سر)->sqh_اولی);				\
	    (var) != SIMPLEQ_انتها(سر);					\
	    (var) = ((var)->فیلد.sqe_بعدی))

#تعریف_کن	SIMPLEQ_FOREACH_امن(var, سر, فیلد, بعدی)			\
	لكل ((var) = ((سر)->sqh_اولی);				\
	    (var) != SIMPLEQ_انتها(سر) &&				\
	    ((بعدی = ((var)->فیلد.sqe_بعدی)), 1);			\
	    (var) = (بعدی))

/*
 * Simple queue functions.
 */
#تعریف_کن	SIMPLEQ_INIT(سر) افعل {						\
	(سر)->sqh_اولی = عدم;					\
	(سر)->sqh_آخرین = &(سر)->sqh_اولی;				\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	SIMPLEQ_درج_سر(سر, elm, فیلد) افعل {			\
	إذا (((elm)->فیلد.sqe_بعدی = (سر)->sqh_اولی) == عدم)	\
		(سر)->sqh_آخرین = &(elm)->فیلد.sqe_بعدی;		\
	(سر)->sqh_اولی = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	SIMPLEQ_درج_دم(سر, elm, فیلد) افعل {			\
	(elm)->فیلد.sqe_بعدی = عدم;					\
	*(سر)->sqh_آخرین = (elm);					\
	(سر)->sqh_آخرین = &(elm)->فیلد.sqe_بعدی;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	SIMPLEQ_درج_AFTER(سر, listelm, elm, فیلد) افعل {		\
	إذا (((elm)->فیلد.sqe_بعدی = (listelm)->فیلد.sqe_بعدی) == عدم)\
		(سر)->sqh_آخرین = &(elm)->فیلد.sqe_بعدی;		\
	(listelm)->فیلد.sqe_بعدی = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	SIMPLEQ_REMOVE_سر(سر, فیلد) افعل {				\
	إذا (((سر)->sqh_اولی = (سر)->sqh_اولی->فیلد.sqe_بعدی) == عدم) \
		(سر)->sqh_آخرین = &(سر)->sqh_اولی;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کن SIMPLEQ_REMOVE_AFTER(سر, elm, فیلد) افعل {			\
	إذا (((elm)->فیلد.sqe_بعدی = (elm)->فیلد.sqe_بعدی->فیلد.sqe_بعدی) \
	    == عدم)							\
		(سر)->sqh_آخرین = &(elm)->فیلد.sqe_بعدی;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	SIMPLEQ_REMOVE(سر, elm, type, فیلد) افعل {			\
	إذا ((سر)->sqh_اولی == (elm)) {				\
		SIMPLEQ_REMOVE_سر((سر), فیلد);			\
	} وإلا {							\
		بنية type *curelm = (سر)->sqh_اولی;		\
		طالما (curelm->فیلد.sqe_بعدی != (elm))			\
			curelm = curelm->فیلد.sqe_بعدی;		\
		إذا ((curelm->فیلد.sqe_بعدی =				\
			curelm->فیلد.sqe_بعدی->فیلد.sqe_بعدی) == عدم) \
			    (سر)->sqh_آخرین = &(curelm)->فیلد.sqe_بعدی; \
	}								\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	SIMPLEQ_CONCAT(head1, head2) افعل {				\
	إذا (!SIMPLEQ_خالی((head2))) {					\
		*(head1)->sqh_آخرین = (head2)->sqh_اولی;		\
		(head1)->sqh_آخرین = (head2)->sqh_آخرین;		\
		SIMPLEQ_INIT((head2));					\
	}								\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	SIMPLEQ_آخرین(سر, type, فیلد)					\
	(SIMPLEQ_خالی((سر)) ?						\
		عدم :							\
	        ((بنية type *)(فراغ *)				\
		((حرف *)((سر)->sqh_آخرین) - offsetof(بنية type, فیلد))))

/*
 * Tail queue definitions.
 */
#تعریف_کن	TAILQ_سر_2(name, type, qual)					\
بنية name {								\
	qual type *tqh_اولی;		/* first element */		\
	qual type *qual *tqh_آخرین;	/* addr of last next element */	\
}
#تعریف_کن TAILQ_سر(name, type)	TAILQ_سر_2(name, بنية type,)

#تعریف_کن	TAILQ_سر_INITIALIZER(سر)					\
	{ TAILQ_انتها(سر), &(سر).tqh_اولی }

#تعریف_کن	TAILQ_مدخل_2(type, qual)					\
بنية {								\
	qual type *tqe_بعدی;		/* next element */		\
	qual type *qual *tqe_prev;	/* address of previous next element */\
}
#تعریف_کن TAILQ_مدخل(type)	TAILQ_مدخل_2(بنية type,)

/*
 * Tail queue access methods.
 */
#تعریف_کن	TAILQ_اولی(سر)		((سر)->tqh_اولی)
#تعریف_کن	TAILQ_انتها(سر)			(عدم)
#تعریف_کن	TAILQ_بعدی(elm, فیلد)		((elm)->فیلد.tqe_بعدی)
#تعریف_کن	TAILQ_آخرین(سر, headname) \
	(*(((بنية headname *)((سر)->tqh_آخرین))->tqh_آخرین))
#تعریف_کن	TAILQ_PREV(elm, headname, فیلد) \
	(*(((بنية headname *)((elm)->فیلد.tqe_prev))->tqh_آخرین))
#تعریف_کن	TAILQ_خالی(سر)		(TAILQ_اولی(سر) == TAILQ_انتها(سر))


#تعریف_کن	TAILQ_FOREACH(var, سر, فیلد)					\
	لكل ((var) = ((سر)->tqh_اولی);				\
	    (var) != TAILQ_انتها(سر);					\
	    (var) = ((var)->فیلد.tqe_بعدی))

#تعریف_کن	TAILQ_FOREACH_امن(var, سر, فیلد, بعدی)			\
	لكل ((var) = ((سر)->tqh_اولی);				\
	    (var) != TAILQ_انتها(سر) &&					\
	    ((بعدی) = TAILQ_بعدی(var, فیلد), 1); (var) = (بعدی))

#تعریف_کن	TAILQ_FOREACH_معکوس(var, سر, headname, فیلد)		\
	لكل ((var) = (*(((بنية headname *)((سر)->tqh_آخرین))->tqh_آخرین));\
	    (var) != TAILQ_انتها(سر);					\
	    (var) = (*(((بنية headname *)((var)->فیلد.tqe_prev))->tqh_آخرین)))

#تعریف_کن	TAILQ_FOREACH_معکوس_امن(var, سر, headname, فیلد, prev)	\
	لكل ((var) = TAILQ_آخرین((سر), headname);			\
	    (var) != TAILQ_انتها(سر) && 				\
	    ((prev) = TAILQ_PREV((var), headname, فیلد), 1); (var) = (prev))

/*
 * Tail queue functions.
 */
#إذا defined(QUEUEDEBUG)
#تعریف_کن	QUEUEDEBUG_TAILQ_درج_سر(سر, elm, فیلد)			\
	إذا ((سر)->tqh_اولی &&					\
	    (سر)->tqh_اولی->فیلد.tqe_prev != &(سر)->tqh_اولی)	\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_HEAD %p %s:%d", (سر),	\
		    __FILE__, __LINE__);
#تعریف_کن	QUEUEDEBUG_TAILQ_درج_دم(سر, elm, فیلد)			\
	إذا (*(سر)->tqh_آخرین != عدم)					\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_TAIL %p %s:%d", (سر),	\
		    __FILE__, __LINE__);
#تعریف_کن	QUEUEDEBUG_TAILQ_OP(elm, فیلد)					\
	إذا ((elm)->فیلد.tqe_بعدی &&					\
	    (elm)->فیلد.tqe_بعدی->فیلد.tqe_prev !=			\
	    &(elm)->فیلد.tqe_بعدی)					\
		QUEUEDEBUG_ABORT("TAILQ_* forw %p %s:%d", (elm),	\
		    __FILE__, __LINE__);				\
	إذا (*(elm)->فیلد.tqe_prev != (elm))				\
		QUEUEDEBUG_ABORT("TAILQ_* back %p %s:%d", (elm),	\
		    __FILE__, __LINE__);
#تعریف_کن	QUEUEDEBUG_TAILQ_PREREMOVE(سر, elm, فیلد)			\
	إذا ((elm)->فیلد.tqe_بعدی == عدم &&				\
	    (سر)->tqh_آخرین != &(elm)->فیلد.tqe_بعدی)			\
		QUEUEDEBUG_ABORT("TAILQ_PREREMOVE head %p elm %p %s:%d",\
		    (سر), (elm), __FILE__, __LINE__);
#تعریف_کن	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, فیلد)				\
	(elm)->فیلد.tqe_بعدی = (فراغ *)1L;				\
	(elm)->فیلد.tqe_prev = (فراغ *)1L;
#وإلا
#تعریف_کن	QUEUEDEBUG_TAILQ_درج_سر(سر, elm, فیلد)
#تعریف_کن	QUEUEDEBUG_TAILQ_درج_دم(سر, elm, فیلد)
#تعریف_کن	QUEUEDEBUG_TAILQ_OP(elm, فیلد)
#تعریف_کن	QUEUEDEBUG_TAILQ_PREREMOVE(سر, elm, فیلد)
#تعریف_کن	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, فیلد)
#پایان_اگر

#تعریف_کن	TAILQ_INIT(سر) افعل {						\
	(سر)->tqh_اولی = TAILQ_انتها(سر);				\
	(سر)->tqh_آخرین = &(سر)->tqh_اولی;				\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	TAILQ_درج_سر(سر, elm, فیلد) افعل {			\
	QUEUEDEBUG_TAILQ_درج_سر((سر), (elm), فیلد)		\
	إذا (((elm)->فیلد.tqe_بعدی = (سر)->tqh_اولی) != TAILQ_انتها(سر))\
		(سر)->tqh_اولی->فیلد.tqe_prev =			\
		    &(elm)->فیلد.tqe_بعدی;				\
	وإلا								\
		(سر)->tqh_آخرین = &(elm)->فیلد.tqe_بعدی;		\
	(سر)->tqh_اولی = (elm);					\
	(elm)->فیلد.tqe_prev = &(سر)->tqh_اولی;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	TAILQ_درج_دم(سر, elm, فیلد) افعل {			\
	QUEUEDEBUG_TAILQ_درج_دم((سر), (elm), فیلد)		\
	(elm)->فیلد.tqe_بعدی = TAILQ_انتها(سر);			\
	(elm)->فیلد.tqe_prev = (سر)->tqh_آخرین;			\
	*(سر)->tqh_آخرین = (elm);					\
	(سر)->tqh_آخرین = &(elm)->فیلد.tqe_بعدی;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	TAILQ_درج_AFTER(سر, listelm, elm, فیلد) افعل {		\
	QUEUEDEBUG_TAILQ_OP((listelm), فیلد)				\
	إذا (((elm)->فیلد.tqe_بعدی = (listelm)->فیلد.tqe_بعدی) != 	\
	    TAILQ_انتها(سر))						\
		(elm)->فیلد.tqe_بعدی->فیلد.tqe_prev = 		\
		    &(elm)->فیلد.tqe_بعدی;				\
	وإلا								\
		(سر)->tqh_آخرین = &(elm)->فیلد.tqe_بعدی;		\
	(listelm)->فیلد.tqe_بعدی = (elm);				\
	(elm)->فیلد.tqe_prev = &(listelm)->فیلد.tqe_بعدی;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	TAILQ_درج_BEFORE(listelm, elm, فیلد) افعل {			\
	QUEUEDEBUG_TAILQ_OP((listelm), فیلد)				\
	(elm)->فیلد.tqe_prev = (listelm)->فیلد.tqe_prev;		\
	(elm)->فیلد.tqe_بعدی = (listelm);				\
	*(listelm)->فیلد.tqe_prev = (elm);				\
	(listelm)->فیلد.tqe_prev = &(elm)->فیلد.tqe_بعدی;		\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	TAILQ_REMOVE(سر, elm, فیلد) افعل {				\
	QUEUEDEBUG_TAILQ_PREREMOVE((سر), (elm), فیلد)		\
	QUEUEDEBUG_TAILQ_OP((elm), فیلد)				\
	إذا (((elm)->فیلد.tqe_بعدی) != TAILQ_انتها(سر))			\
		(elm)->فیلد.tqe_بعدی->فیلد.tqe_prev = 		\
		    (elm)->فیلد.tqe_prev;				\
	وإلا								\
		(سر)->tqh_آخرین = (elm)->فیلد.tqe_prev;		\
	*(elm)->فیلد.tqe_prev = (elm)->فیلد.tqe_بعدی;			\
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), فیلد);			\
} طالما (/*CONSTCOND*/0)

#تعریف_کن TAILQ_REPLACE(سر, elm, elm2, فیلد) افعل {                      \
        إذا (((elm2)->فیلد.tqe_بعدی = (elm)->فیلد.tqe_بعدی) != 	\
	    TAILQ_انتها(سر))   						\
                (elm2)->فیلد.tqe_بعدی->فیلد.tqe_prev =                \
                    &(elm2)->فیلد.tqe_بعدی;                            \
        وإلا                                                            \
                (سر)->tqh_آخرین = &(elm2)->فیلد.tqe_بعدی;             \
        (elm2)->فیلد.tqe_prev = (elm)->فیلد.tqe_prev;                 \
        *(elm2)->فیلد.tqe_prev = (elm2);                               \
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), فیلد);			\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	TAILQ_CONCAT(head1, head2, فیلد) افعل {				\
	إذا (!TAILQ_خالی(head2)) {					\
		*(head1)->tqh_آخرین = (head2)->tqh_اولی;		\
		(head2)->tqh_اولی->فیلد.tqe_prev = (head1)->tqh_آخرین;	\
		(head1)->tqh_آخرین = (head2)->tqh_آخرین;			\
		TAILQ_INIT((head2));					\
	}								\
} طالما (/*CONSTCOND*/0)

/*
 * Singly-linked Tail queue declarations.
 */
#تعریف_کن	STAILQ_سر(name, type)						\
بنية name {								\
	بنية type *stqh_اولی;	/* first element */		\
	بنية type **stqh_آخرین;	/* addr of last next element */	\
}

#تعریف_کن	STAILQ_سر_INITIALIZER(سر)					\
	{ عدم, &(سر).stqh_اولی }

#تعریف_کن	STAILQ_مدخل(type)						\
بنية {								\
	بنية type *stqe_بعدی;	/* next element */			\
}

/*
 * Singly-linked Tail queue access methods.
 */
#تعریف_کن	STAILQ_اولی(سر)	((سر)->stqh_اولی)
#تعریف_کن	STAILQ_انتها(سر)	عدم
#تعریف_کن	STAILQ_بعدی(elm, فیلد)	((elm)->فیلد.stqe_بعدی)
#تعریف_کن	STAILQ_خالی(سر)	(STAILQ_اولی(سر) == STAILQ_انتها(سر))

/*
 * Singly-linked Tail queue functions.
 */
#تعریف_کن	STAILQ_INIT(سر) افعل {						\
	(سر)->stqh_اولی = عدم;					\
	(سر)->stqh_آخرین = &(سر)->stqh_اولی;				\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	STAILQ_درج_سر(سر, elm, فیلد) افعل {			\
	إذا (((elm)->فیلد.stqe_بعدی = (سر)->stqh_اولی) == عدم)	\
		(سر)->stqh_آخرین = &(elm)->فیلد.stqe_بعدی;		\
	(سر)->stqh_اولی = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	STAILQ_درج_دم(سر, elm, فیلد) افعل {			\
	(elm)->فیلد.stqe_بعدی = عدم;					\
	*(سر)->stqh_آخرین = (elm);					\
	(سر)->stqh_آخرین = &(elm)->فیلد.stqe_بعدی;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	STAILQ_درج_AFTER(سر, listelm, elm, فیلد) افعل {		\
	إذا (((elm)->فیلد.stqe_بعدی = (listelm)->فیلد.stqe_بعدی) == عدم)\
		(سر)->stqh_آخرین = &(elm)->فیلد.stqe_بعدی;		\
	(listelm)->فیلد.stqe_بعدی = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	STAILQ_REMOVE_سر(سر, فیلد) افعل {				\
	إذا (((سر)->stqh_اولی = (سر)->stqh_اولی->فیلد.stqe_بعدی) == عدم) \
		(سر)->stqh_آخرین = &(سر)->stqh_اولی;			\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	STAILQ_REMOVE(سر, elm, type, فیلد) افعل {			\
	إذا ((سر)->stqh_اولی == (elm)) {				\
		STAILQ_REMOVE_سر((سر), فیلد);			\
	} وإلا {							\
		بنية type *curelm = (سر)->stqh_اولی;		\
		طالما (curelm->فیلد.stqe_بعدی != (elm))			\
			curelm = curelm->فیلد.stqe_بعدی;		\
		إذا ((curelm->فیلد.stqe_بعدی =				\
			curelm->فیلد.stqe_بعدی->فیلد.stqe_بعدی) == عدم) \
			    (سر)->stqh_آخرین = &(curelm)->فیلد.stqe_بعدی; \
	}								\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	STAILQ_FOREACH(var, سر, فیلد)				\
	لكل ((var) = ((سر)->stqh_اولی);				\
		(var);							\
		(var) = ((var)->فیلد.stqe_بعدی))

#تعریف_کن	STAILQ_FOREACH_امن(var, سر, فیلد, tvar)			\
	لكل ((var) = STAILQ_اولی((سر));				\
	    (var) && ((tvar) = STAILQ_بعدی((var), فیلد), 1);		\
	    (var) = (tvar))

#تعریف_کن	STAILQ_CONCAT(head1, head2) افعل {				\
	إذا (!STAILQ_خالی((head2))) {					\
		*(head1)->stqh_آخرین = (head2)->stqh_اولی;		\
		(head1)->stqh_آخرین = (head2)->stqh_آخرین;		\
		STAILQ_INIT((head2));					\
	}								\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	STAILQ_آخرین(سر, type, فیلد)					\
	(STAILQ_خالی((سر)) ?						\
		عدم :							\
	        ((بنية type *)(فراغ *)				\
		((حرف *)((سر)->stqh_آخرین) - offsetof(بنية type, فیلد))))


#اگر_تعریف_نشده _KERNEL
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
#تعریف_کن QUEUEDEBUG_CIRCLEQ_سر(سر, فیلد)				\
	إذا ((سر)->cqh_اولی != CIRCLEQ_ENDC(سر) &&			\
	    (سر)->cqh_اولی->فیلد.cqe_prev != CIRCLEQ_ENDC(سر))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head forw %p %s:%d", (سر),	\
		      __FILE__, __LINE__);				\
	إذا ((سر)->cqh_آخرین != CIRCLEQ_ENDC(سر) &&			\
	    (سر)->cqh_آخرین->فیلد.cqe_بعدی != CIRCLEQ_ENDC(سر))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head back %p %s:%d", (سر),	\
		      __FILE__, __LINE__);
#تعریف_کن QUEUEDEBUG_CIRCLEQ_ELM(سر, elm, فیلد)			\
	إذا ((elm)->فیلد.cqe_بعدی == CIRCLEQ_ENDC(سر)) {		\
		إذا ((سر)->cqh_آخرین != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm last %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} وإلا {							\
		إذا ((elm)->فیلد.cqe_بعدی->فیلد.cqe_prev != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm forw %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}								\
	إذا ((elm)->فیلد.cqe_prev == CIRCLEQ_ENDC(سر)) {		\
		إذا ((سر)->cqh_اولی != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm first %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} وإلا {							\
		إذا ((elm)->فیلد.cqe_prev->فیلد.cqe_بعدی != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm prev %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}
#تعریف_کن QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, فیلد)			\
	(elm)->فیلد.cqe_بعدی = (فراغ *)1L;				\
	(elm)->فیلد.cqe_prev = (فراغ *)1L;
#وإلا
#تعریف_کن QUEUEDEBUG_CIRCLEQ_سر(سر, فیلد)
#تعریف_کن QUEUEDEBUG_CIRCLEQ_ELM(سر, elm, فیلد)
#تعریف_کن QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, فیلد)
#پایان_اگر

#تعریف_کن	CIRCLEQ_سر(name, type)					\
بنية name {								\
	بنية type *cqh_اولی;		/* first element */		\
	بنية type *cqh_آخرین;		/* last element */		\
}

#تعریف_کن	CIRCLEQ_سر_INITIALIZER(سر)					\
	{ CIRCLEQ_انتها(&سر), CIRCLEQ_انتها(&سر) }

#تعریف_کن	CIRCLEQ_مدخل(type)						\
بنية {								\
	بنية type *cqe_بعدی;		/* next element */		\
	بنية type *cqe_prev;		/* previous element */		\
}

/*
 * Circular queue functions.
 */
#تعریف_کن	CIRCLEQ_INIT(سر) افعل {						\
	(سر)->cqh_اولی = CIRCLEQ_انتها(سر);				\
	(سر)->cqh_آخرین = CIRCLEQ_انتها(سر);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	CIRCLEQ_درج_AFTER(سر, listelm, elm, فیلد) افعل {		\
	QUEUEDEBUG_CIRCLEQ_سر((سر), فیلد)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر), (listelm), فیلد)		\
	(elm)->فیلد.cqe_بعدی = (listelm)->فیلد.cqe_بعدی;		\
	(elm)->فیلد.cqe_prev = (listelm);				\
	إذا ((listelm)->فیلد.cqe_بعدی == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_آخرین = (elm);				\
	وإلا								\
		(listelm)->فیلد.cqe_بعدی->فیلد.cqe_prev = (elm);	\
	(listelm)->فیلد.cqe_بعدی = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	CIRCLEQ_درج_BEFORE(سر, listelm, elm, فیلد) افعل {		\
	QUEUEDEBUG_CIRCLEQ_سر((سر), فیلد)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر), (listelm), فیلد)		\
	(elm)->فیلد.cqe_بعدی = (listelm);				\
	(elm)->فیلد.cqe_prev = (listelm)->فیلد.cqe_prev;		\
	إذا ((listelm)->فیلد.cqe_prev == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_اولی = (elm);				\
	وإلا								\
		(listelm)->فیلد.cqe_prev->فیلد.cqe_بعدی = (elm);	\
	(listelm)->فیلد.cqe_prev = (elm);				\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	CIRCLEQ_درج_سر(سر, elm, فیلد) افعل {			\
	QUEUEDEBUG_CIRCLEQ_سر((سر), فیلد)				\
	(elm)->فیلد.cqe_بعدی = (سر)->cqh_اولی;			\
	(elm)->فیلد.cqe_prev = CIRCLEQ_انتها(سر);			\
	إذا ((سر)->cqh_آخرین == CIRCLEQ_ENDC(سر))			\
		(سر)->cqh_آخرین = (elm);				\
	وإلا								\
		(سر)->cqh_اولی->فیلد.cqe_prev = (elm);		\
	(سر)->cqh_اولی = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	CIRCLEQ_درج_دم(سر, elm, فیلد) افعل {			\
	QUEUEDEBUG_CIRCLEQ_سر((سر), فیلد)				\
	(elm)->فیلد.cqe_بعدی = CIRCLEQ_انتها(سر);			\
	(elm)->فیلد.cqe_prev = (سر)->cqh_آخرین;			\
	إذا ((سر)->cqh_اولی == CIRCLEQ_ENDC(سر))			\
		(سر)->cqh_اولی = (elm);				\
	وإلا								\
		(سر)->cqh_آخرین->فیلد.cqe_بعدی = (elm);		\
	(سر)->cqh_آخرین = (elm);					\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	CIRCLEQ_REMOVE(سر, elm, فیلد) افعل {				\
	QUEUEDEBUG_CIRCLEQ_سر((سر), فیلد)				\
	QUEUEDEBUG_CIRCLEQ_ELM((سر), (elm), فیلد)			\
	إذا ((elm)->فیلد.cqe_بعدی == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_آخرین = (elm)->فیلد.cqe_prev;		\
	وإلا								\
		(elm)->فیلد.cqe_بعدی->فیلد.cqe_prev =			\
		    (elm)->فیلد.cqe_prev;				\
	إذا ((elm)->فیلد.cqe_prev == CIRCLEQ_ENDC(سر))		\
		(سر)->cqh_اولی = (elm)->فیلد.cqe_بعدی;		\
	وإلا								\
		(elm)->فیلد.cqe_prev->فیلد.cqe_بعدی =			\
		    (elm)->فیلد.cqe_بعدی;				\
	QUEUEDEBUG_CIRCLEQ_POSTREMOVE((elm), فیلد)			\
} طالما (/*CONSTCOND*/0)

#تعریف_کن	CIRCLEQ_FOREACH(var, سر, فیلد)				\
	لكل ((var) = ((سر)->cqh_اولی);				\
		(var) != CIRCLEQ_ENDC(سر);				\
		(var) = ((var)->فیلد.cqe_بعدی))

#تعریف_کن	CIRCLEQ_FOREACH_معکوس(var, سر, فیلد)			\
	لكل ((var) = ((سر)->cqh_آخرین);				\
		(var) != CIRCLEQ_ENDC(سر);				\
		(var) = ((var)->فیلد.cqe_prev))

/*
 * Circular queue access methods.
 */
#تعریف_کن	CIRCLEQ_اولی(سر)		((سر)->cqh_اولی)
#تعریف_کن	CIRCLEQ_آخرین(سر)		((سر)->cqh_آخرین)
/* For comparisons */
#تعریف_کن	CIRCLEQ_ENDC(سر)		(__launder_type(سر))
/* For assignments */
#تعریف_کن	CIRCLEQ_انتها(سر)		((فراغ *)(سر))
#تعریف_کن	CIRCLEQ_بعدی(elm, فیلد)	((elm)->فیلد.cqe_بعدی)
#تعریف_کن	CIRCLEQ_PREV(elm, فیلد)	((elm)->فیلد.cqe_prev)
#تعریف_کن	CIRCLEQ_خالی(سر)						\
    (CIRCLEQ_اولی(سر) == CIRCLEQ_ENDC(سر))

#تعریف_کن CIRCLEQ_LOOP_بعدی(سر, elm, فیلد)				\
	(((elm)->فیلد.cqe_بعدی == CIRCLEQ_ENDC(سر))			\
	    ? ((سر)->cqh_اولی)					\
	    : (elm->فیلد.cqe_بعدی))
#تعریف_کن CIRCLEQ_LOOP_PREV(سر, elm, فیلد)				\
	(((elm)->فیلد.cqe_prev == CIRCLEQ_ENDC(سر))			\
	    ? ((سر)->cqh_آخرین)					\
	    : (elm->فیلد.cqe_prev))
#پایان_اگر /* !_KERNEL */

#پایان_اگر	/* !_SYS_QUEUE_H_ */
