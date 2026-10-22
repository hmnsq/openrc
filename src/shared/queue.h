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
#അന്തര്ഭാവ <sys/null.h>
#endif

#എങ്കിൽ defined(QUEUEDEBUG)
# എങ്കിൽ defined(_KERNEL)
#  define QUEUEDEBUG_ABORT(...) panic(__VA_ARGS__)
# അല്ലെങ്കിൽ
#  include <err.h>
#  define QUEUEDEBUG_ABORT(...) err(1, __VA_ARGS__)
# endif
#endif

/*
 * Singly-linked List definitions.
 */
#define	SLIST_HEAD(name, type)						\
ഘടന name {								\
	ഘടന type *slh_first;	/* first element */			\
}

#define	SLIST_HEAD_INITIALIZER(head)					\
	{ ശൂന്യ }

#define	SLIST_ENTRY(type)						\
ഘടന {								\
	ഘടന type *sle_next;	/* next element */			\
}

/*
 * Singly-linked List access methods.
 */
#define	SLIST_FIRST(head)	((head)->slh_first)
#define	SLIST_END(head)		ശൂന്യ
#define	SLIST_EMPTY(head)	((head)->slh_first == ശൂന്യ)
#define	SLIST_NEXT(elm, field)	((elm)->field.sle_next)

#define	SLIST_FOREACH(var, head, field)					\
	വേണ്ടി((var) = (head)->slh_first;					\
	    (var) != SLIST_END(head);					\
	    (var) = (var)->field.sle_next)

#define	SLIST_FOREACH_SAFE(var, head, field, tvar)			\
	വേണ്ടി ((var) = SLIST_FIRST((head));				\
	    (var) != SLIST_END(head) &&					\
	    ((tvar) = SLIST_NEXT((var), field), 1);			\
	    (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#define	SLIST_INIT(head) ചെയ്യുക {						\
	(head)->slh_first = SLIST_END(head);				\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	SLIST_തിരുകുക_AFTER(slistelm, elm, field) ചെയ്യുക {			\
	(elm)->field.sle_next = (slistelm)->field.sle_next;		\
	(slistelm)->field.sle_next = (elm);				\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	SLIST_തിരുകുക_HEAD(head, elm, field) ചെയ്യുക {			\
	(elm)->field.sle_next = (head)->slh_first;			\
	(head)->slh_first = (elm);					\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	SLIST_REMOVE_AFTER(slistelm, field) ചെയ്യുക {			\
	(slistelm)->field.sle_next =					\
	    SLIST_NEXT(SLIST_NEXT((slistelm), field), field);		\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	SLIST_REMOVE_HEAD(head, field) ചെയ്യുക {				\
	(head)->slh_first = (head)->slh_first->field.sle_next;		\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	SLIST_REMOVE(head, elm, type, field) ചെയ്യുക {			\
	എങ്കിൽ ((head)->slh_first == (elm)) {				\
		SLIST_REMOVE_HEAD((head), field);			\
	}								\
	അല്ലെങ്കിൽ {								\
		ഘടന type *curelm = (head)->slh_first;		\
		ആകുമ്പോൾ(curelm->field.sle_next != (elm))			\
			curelm = curelm->field.sle_next;		\
		curelm->field.sle_next =				\
		    curelm->field.sle_next->field.sle_next;		\
	}								\
} ആകുമ്പോൾ (/*CONSTCOND*/0)


/*
 * List definitions.
 */
#define	LIST_HEAD(name, type)						\
ഘടന name {								\
	ഘടന type *lh_first;	/* first element */			\
}

#define	LIST_HEAD_INITIALIZER(head)					\
	{ ശൂന്യ }

#define	LIST_ENTRY(type)						\
ഘടന {								\
	ഘടന type *le_next;	/* next element */			\
	ഘടന type **le_prev;	/* address of previous next element */	\
}

/*
 * List access methods.
 */
#define	LIST_FIRST(head)		((head)->lh_first)
#define	LIST_END(head)			ശൂന്യ
#define	LIST_EMPTY(head)		((head)->lh_first == LIST_END(head))
#define	LIST_NEXT(elm, field)		((elm)->field.le_next)

#define	LIST_FOREACH(var, head, field)					\
	വേണ്ടി ((var) = ((head)->lh_first);				\
	    (var) != LIST_END(head);					\
	    (var) = ((var)->field.le_next))

#define	LIST_FOREACH_SAFE(var, head, field, tvar)			\
	വേണ്ടി ((var) = LIST_FIRST((head));				\
	    (var) != LIST_END(head) &&					\
	    ((tvar) = LIST_NEXT((var), field), 1);			\
	    (var) = (tvar))

#define	LIST_MOVE(head1, head2) ചെയ്യുക {					\
	LIST_INIT((head2));						\
	എങ്കിൽ (!LIST_EMPTY((head1))) {					\
		(head2)->lh_first = (head1)->lh_first;			\
		LIST_INIT((head1));					\
	}								\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

/*
 * List functions.
 */
#എങ്കിൽ defined(QUEUEDEBUG)
#define	QUEUEDEBUG_LIST_തിരുകുക_HEAD(head, elm, field)			\
	എങ്കിൽ ((head)->lh_first &&						\
	    (head)->lh_first->field.le_prev != &(head)->lh_first)	\
		QUEUEDEBUG_ABORT("LIST_INSERT_HEAD %p %s:%d", (head),	\
		    __FILE__, __LINE__);
#define	QUEUEDEBUG_LIST_OP(elm, field)					\
	എങ്കിൽ ((elm)->field.le_next &&					\
	    (elm)->field.le_next->field.le_prev !=			\
	    &(elm)->field.le_next)					\
		QUEUEDEBUG_ABORT("LIST_* forw %p %s:%d", (elm),		\
		    __FILE__, __LINE__);				\
	എങ്കിൽ (*(elm)->field.le_prev != (elm))				\
		QUEUEDEBUG_ABORT("LIST_* back %p %s:%d", (elm),		\
		    __FILE__, __LINE__);
#define	QUEUEDEBUG_LIST_POSTREMOVE(elm, field)				\
	(elm)->field.le_next = (ശൂന്യം *)1L;				\
	(elm)->field.le_prev = (ശൂന്യം *)1L;
#അല്ലെങ്കിൽ
#define	QUEUEDEBUG_LIST_തിരുകുക_HEAD(head, elm, field)
#define	QUEUEDEBUG_LIST_OP(elm, field)
#define	QUEUEDEBUG_LIST_POSTREMOVE(elm, field)
#endif

#define	LIST_INIT(head) ചെയ്യുക {						\
	(head)->lh_first = LIST_END(head);				\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	LIST_തിരുകുക_AFTER(listelm, elm, field) ചെയ്യുക {			\
	QUEUEDEBUG_LIST_OP((listelm), field)				\
	എങ്കിൽ (((elm)->field.le_next = (listelm)->field.le_next) != 	\
	    LIST_END(head))						\
		(listelm)->field.le_next->field.le_prev =		\
		    &(elm)->field.le_next;				\
	(listelm)->field.le_next = (elm);				\
	(elm)->field.le_prev = &(listelm)->field.le_next;		\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	LIST_തിരുകുക_BEFORE(listelm, elm, field) ചെയ്യുക {			\
	QUEUEDEBUG_LIST_OP((listelm), field)				\
	(elm)->field.le_prev = (listelm)->field.le_prev;		\
	(elm)->field.le_next = (listelm);				\
	*(listelm)->field.le_prev = (elm);				\
	(listelm)->field.le_prev = &(elm)->field.le_next;		\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	LIST_തിരുകുക_HEAD(head, elm, field) ചെയ്യുക {				\
	QUEUEDEBUG_LIST_തിരുകുക_HEAD((head), (elm), field)		\
	എങ്കിൽ (((elm)->field.le_next = (head)->lh_first) != LIST_END(head))\
		(head)->lh_first->field.le_prev = &(elm)->field.le_next;\
	(head)->lh_first = (elm);					\
	(elm)->field.le_prev = &(head)->lh_first;			\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	LIST_REMOVE(elm, field) ചെയ്യുക {					\
	QUEUEDEBUG_LIST_OP((elm), field)				\
	എങ്കിൽ ((elm)->field.le_next != ശൂന്യ)				\
		(elm)->field.le_next->field.le_prev = 			\
		    (elm)->field.le_prev;				\
	*(elm)->field.le_prev = (elm)->field.le_next;			\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), field)			\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define LIST_REPLACE(elm, elm2, field) ചെയ്യുക {				\
	എങ്കിൽ (((elm2)->field.le_next = (elm)->field.le_next) != ശൂന്യ)	\
		(elm2)->field.le_next->field.le_prev =			\
		    &(elm2)->field.le_next;				\
	(elm2)->field.le_prev = (elm)->field.le_prev;			\
	*(elm2)->field.le_prev = (elm2);				\
	QUEUEDEBUG_LIST_POSTREMOVE((elm), field)			\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

/*
 * Simple queue definitions.
 */
#define	SIMPLEQ_HEAD(name, type)					\
ഘടന name {								\
	ഘടന type *sqh_first;	/* first element */			\
	ഘടന type **sqh_last;	/* addr of last next element */		\
}

#define	SIMPLEQ_HEAD_INITIALIZER(head)					\
	{ ശൂന്യ, &(head).sqh_first }

#define	SIMPLEQ_ENTRY(type)						\
ഘടന {								\
	ഘടന type *sqe_next;	/* next element */			\
}

/*
 * Simple queue access methods.
 */
#define	SIMPLEQ_FIRST(head)		((head)->sqh_first)
#define	SIMPLEQ_END(head)		ശൂന്യ
#define	SIMPLEQ_EMPTY(head)		((head)->sqh_first == SIMPLEQ_END(head))
#define	SIMPLEQ_NEXT(elm, field)	((elm)->field.sqe_next)

#define	SIMPLEQ_FOREACH(var, head, field)				\
	വേണ്ടി ((var) = ((head)->sqh_first);				\
	    (var) != SIMPLEQ_END(head);					\
	    (var) = ((var)->field.sqe_next))

#define	SIMPLEQ_FOREACH_SAFE(var, head, field, next)			\
	വേണ്ടി ((var) = ((head)->sqh_first);				\
	    (var) != SIMPLEQ_END(head) &&				\
	    ((next = ((var)->field.sqe_next)), 1);			\
	    (var) = (next))

/*
 * Simple queue functions.
 */
#define	SIMPLEQ_INIT(head) ചെയ്യുക {						\
	(head)->sqh_first = ശൂന്യ;					\
	(head)->sqh_last = &(head)->sqh_first;				\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	SIMPLEQ_തിരുകുക_HEAD(head, elm, field) ചെയ്യുക {			\
	എങ്കിൽ (((elm)->field.sqe_next = (head)->sqh_first) == ശൂന്യ)	\
		(head)->sqh_last = &(elm)->field.sqe_next;		\
	(head)->sqh_first = (elm);					\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	SIMPLEQ_തിരുകുക_TAIL(head, elm, field) ചെയ്യുക {			\
	(elm)->field.sqe_next = ശൂന്യ;					\
	*(head)->sqh_last = (elm);					\
	(head)->sqh_last = &(elm)->field.sqe_next;			\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	SIMPLEQ_തിരുകുക_AFTER(head, listelm, elm, field) ചെയ്യുക {		\
	എങ്കിൽ (((elm)->field.sqe_next = (listelm)->field.sqe_next) == ശൂന്യ)\
		(head)->sqh_last = &(elm)->field.sqe_next;		\
	(listelm)->field.sqe_next = (elm);				\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	SIMPLEQ_REMOVE_HEAD(head, field) ചെയ്യുക {				\
	എങ്കിൽ (((head)->sqh_first = (head)->sqh_first->field.sqe_next) == ശൂന്യ) \
		(head)->sqh_last = &(head)->sqh_first;			\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define SIMPLEQ_REMOVE_AFTER(head, elm, field) ചെയ്യുക {			\
	എങ്കിൽ (((elm)->field.sqe_next = (elm)->field.sqe_next->field.sqe_next) \
	    == ശൂന്യ)							\
		(head)->sqh_last = &(elm)->field.sqe_next;		\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	SIMPLEQ_REMOVE(head, elm, type, field) ചെയ്യുക {			\
	എങ്കിൽ ((head)->sqh_first == (elm)) {				\
		SIMPLEQ_REMOVE_HEAD((head), field);			\
	} അല്ലെങ്കിൽ {							\
		ഘടന type *curelm = (head)->sqh_first;		\
		ആകുമ്പോൾ (curelm->field.sqe_next != (elm))			\
			curelm = curelm->field.sqe_next;		\
		എങ്കിൽ ((curelm->field.sqe_next =				\
			curelm->field.sqe_next->field.sqe_next) == ശൂന്യ) \
			    (head)->sqh_last = &(curelm)->field.sqe_next; \
	}								\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	SIMPLEQ_CONCAT(head1, head2) ചെയ്യുക {				\
	എങ്കിൽ (!SIMPLEQ_EMPTY((head2))) {					\
		*(head1)->sqh_last = (head2)->sqh_first;		\
		(head1)->sqh_last = (head2)->sqh_last;		\
		SIMPLEQ_INIT((head2));					\
	}								\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	SIMPLEQ_LAST(head, type, field)					\
	(SIMPLEQ_EMPTY((head)) ?						\
		ശൂന്യ :							\
	        ((ഘടന type *)(ശൂന്യം *)				\
		((അക്ഷരം *)((head)->sqh_last) - offsetof(ഘടന type, field))))

/*
 * Tail queue definitions.
 */
#define	_TAILQ_HEAD(name, type, qual)					\
ഘടന name {								\
	qual type *tqh_first;		/* first element */		\
	qual type *qual *tqh_last;	/* addr of last next element */	\
}
#define TAILQ_HEAD(name, type)	_TAILQ_HEAD(name, ഘടന type,)

#define	TAILQ_HEAD_INITIALIZER(head)					\
	{ TAILQ_END(head), &(head).tqh_first }

#define	_TAILQ_ENTRY(type, qual)					\
ഘടന {								\
	qual type *tqe_next;		/* next element */		\
	qual type *qual *tqe_prev;	/* address of previous next element */\
}
#define TAILQ_ENTRY(type)	_TAILQ_ENTRY(ഘടന type,)

/*
 * Tail queue access methods.
 */
#define	TAILQ_FIRST(head)		((head)->tqh_first)
#define	TAILQ_END(head)			(ശൂന്യ)
#define	TAILQ_NEXT(elm, field)		((elm)->field.tqe_next)
#define	TAILQ_LAST(head, headname) \
	(*(((ഘടന headname *)((head)->tqh_last))->tqh_last))
#define	TAILQ_PREV(elm, headname, field) \
	(*(((ഘടന headname *)((elm)->field.tqe_prev))->tqh_last))
#define	TAILQ_EMPTY(head)		(TAILQ_FIRST(head) == TAILQ_END(head))


#define	TAILQ_FOREACH(var, head, field)					\
	വേണ്ടി ((var) = ((head)->tqh_first);				\
	    (var) != TAILQ_END(head);					\
	    (var) = ((var)->field.tqe_next))

#define	TAILQ_FOREACH_SAFE(var, head, field, next)			\
	വേണ്ടി ((var) = ((head)->tqh_first);				\
	    (var) != TAILQ_END(head) &&					\
	    ((next) = TAILQ_NEXT(var, field), 1); (var) = (next))

#define	TAILQ_FOREACH_REVERSE(var, head, headname, field)		\
	വേണ്ടി ((var) = (*(((ഘടന headname *)((head)->tqh_last))->tqh_last));\
	    (var) != TAILQ_END(head);					\
	    (var) = (*(((ഘടന headname *)((var)->field.tqe_prev))->tqh_last)))

#define	TAILQ_FOREACH_REVERSE_SAFE(var, head, headname, field, prev)	\
	വേണ്ടി ((var) = TAILQ_LAST((head), headname);			\
	    (var) != TAILQ_END(head) && 				\
	    ((prev) = TAILQ_PREV((var), headname, field), 1); (var) = (prev))

/*
 * Tail queue functions.
 */
#എങ്കിൽ defined(QUEUEDEBUG)
#define	QUEUEDEBUG_TAILQ_തിരുകുക_HEAD(head, elm, field)			\
	എങ്കിൽ ((head)->tqh_first &&					\
	    (head)->tqh_first->field.tqe_prev != &(head)->tqh_first)	\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_HEAD %p %s:%d", (head),	\
		    __FILE__, __LINE__);
#define	QUEUEDEBUG_TAILQ_തിരുകുക_TAIL(head, elm, field)			\
	എങ്കിൽ (*(head)->tqh_last != ശൂന്യ)					\
		QUEUEDEBUG_ABORT("TAILQ_INSERT_TAIL %p %s:%d", (head),	\
		    __FILE__, __LINE__);
#define	QUEUEDEBUG_TAILQ_OP(elm, field)					\
	എങ്കിൽ ((elm)->field.tqe_next &&					\
	    (elm)->field.tqe_next->field.tqe_prev !=			\
	    &(elm)->field.tqe_next)					\
		QUEUEDEBUG_ABORT("TAILQ_* forw %p %s:%d", (elm),	\
		    __FILE__, __LINE__);				\
	എങ്കിൽ (*(elm)->field.tqe_prev != (elm))				\
		QUEUEDEBUG_ABORT("TAILQ_* back %p %s:%d", (elm),	\
		    __FILE__, __LINE__);
#define	QUEUEDEBUG_TAILQ_PREREMOVE(head, elm, field)			\
	എങ്കിൽ ((elm)->field.tqe_next == ശൂന്യ &&				\
	    (head)->tqh_last != &(elm)->field.tqe_next)			\
		QUEUEDEBUG_ABORT("TAILQ_PREREMOVE head %p elm %p %s:%d",\
		    (head), (elm), __FILE__, __LINE__);
#define	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, field)				\
	(elm)->field.tqe_next = (ശൂന്യം *)1L;				\
	(elm)->field.tqe_prev = (ശൂന്യം *)1L;
#അല്ലെങ്കിൽ
#define	QUEUEDEBUG_TAILQ_തിരുകുക_HEAD(head, elm, field)
#define	QUEUEDEBUG_TAILQ_തിരുകുക_TAIL(head, elm, field)
#define	QUEUEDEBUG_TAILQ_OP(elm, field)
#define	QUEUEDEBUG_TAILQ_PREREMOVE(head, elm, field)
#define	QUEUEDEBUG_TAILQ_POSTREMOVE(elm, field)
#endif

#define	TAILQ_INIT(head) ചെയ്യുക {						\
	(head)->tqh_first = TAILQ_END(head);				\
	(head)->tqh_last = &(head)->tqh_first;				\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	TAILQ_തിരുകുക_HEAD(head, elm, field) ചെയ്യുക {			\
	QUEUEDEBUG_TAILQ_തിരുകുക_HEAD((head), (elm), field)		\
	എങ്കിൽ (((elm)->field.tqe_next = (head)->tqh_first) != TAILQ_END(head))\
		(head)->tqh_first->field.tqe_prev =			\
		    &(elm)->field.tqe_next;				\
	അല്ലെങ്കിൽ								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(head)->tqh_first = (elm);					\
	(elm)->field.tqe_prev = &(head)->tqh_first;			\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	TAILQ_തിരുകുക_TAIL(head, elm, field) ചെയ്യുക {			\
	QUEUEDEBUG_TAILQ_തിരുകുക_TAIL((head), (elm), field)		\
	(elm)->field.tqe_next = TAILQ_END(head);			\
	(elm)->field.tqe_prev = (head)->tqh_last;			\
	*(head)->tqh_last = (elm);					\
	(head)->tqh_last = &(elm)->field.tqe_next;			\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	TAILQ_തിരുകുക_AFTER(head, listelm, elm, field) ചെയ്യുക {		\
	QUEUEDEBUG_TAILQ_OP((listelm), field)				\
	എങ്കിൽ (((elm)->field.tqe_next = (listelm)->field.tqe_next) != 	\
	    TAILQ_END(head))						\
		(elm)->field.tqe_next->field.tqe_prev = 		\
		    &(elm)->field.tqe_next;				\
	അല്ലെങ്കിൽ								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(listelm)->field.tqe_next = (elm);				\
	(elm)->field.tqe_prev = &(listelm)->field.tqe_next;		\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	TAILQ_തിരുകുക_BEFORE(listelm, elm, field) ചെയ്യുക {			\
	QUEUEDEBUG_TAILQ_OP((listelm), field)				\
	(elm)->field.tqe_prev = (listelm)->field.tqe_prev;		\
	(elm)->field.tqe_next = (listelm);				\
	*(listelm)->field.tqe_prev = (elm);				\
	(listelm)->field.tqe_prev = &(elm)->field.tqe_next;		\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	TAILQ_REMOVE(head, elm, field) ചെയ്യുക {				\
	QUEUEDEBUG_TAILQ_PREREMOVE((head), (elm), field)		\
	QUEUEDEBUG_TAILQ_OP((elm), field)				\
	എങ്കിൽ (((elm)->field.tqe_next) != TAILQ_END(head))			\
		(elm)->field.tqe_next->field.tqe_prev = 		\
		    (elm)->field.tqe_prev;				\
	അല്ലെങ്കിൽ								\
		(head)->tqh_last = (elm)->field.tqe_prev;		\
	*(elm)->field.tqe_prev = (elm)->field.tqe_next;			\
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), field);			\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define TAILQ_REPLACE(head, elm, elm2, field) ചെയ്യുക {                      \
        എങ്കിൽ (((elm2)->field.tqe_next = (elm)->field.tqe_next) != 	\
	    TAILQ_END(head))   						\
                (elm2)->field.tqe_next->field.tqe_prev =                \
                    &(elm2)->field.tqe_next;                            \
        അല്ലെങ്കിൽ                                                            \
                (head)->tqh_last = &(elm2)->field.tqe_next;             \
        (elm2)->field.tqe_prev = (elm)->field.tqe_prev;                 \
        *(elm2)->field.tqe_prev = (elm2);                               \
	QUEUEDEBUG_TAILQ_POSTREMOVE((elm), field);			\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	TAILQ_CONCAT(head1, head2, field) ചെയ്യുക {				\
	എങ്കിൽ (!TAILQ_EMPTY(head2)) {					\
		*(head1)->tqh_last = (head2)->tqh_first;		\
		(head2)->tqh_first->field.tqe_prev = (head1)->tqh_last;	\
		(head1)->tqh_last = (head2)->tqh_last;			\
		TAILQ_INIT((head2));					\
	}								\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

/*
 * Singly-linked Tail queue declarations.
 */
#define	STAILQ_HEAD(name, type)						\
ഘടന name {								\
	ഘടന type *stqh_first;	/* first element */		\
	ഘടന type **stqh_last;	/* addr of last next element */	\
}

#define	STAILQ_HEAD_INITIALIZER(head)					\
	{ ശൂന്യ, &(head).stqh_first }

#define	STAILQ_ENTRY(type)						\
ഘടന {								\
	ഘടന type *stqe_next;	/* next element */			\
}

/*
 * Singly-linked Tail queue access methods.
 */
#define	STAILQ_FIRST(head)	((head)->stqh_first)
#define	STAILQ_END(head)	ശൂന്യ
#define	STAILQ_NEXT(elm, field)	((elm)->field.stqe_next)
#define	STAILQ_EMPTY(head)	(STAILQ_FIRST(head) == STAILQ_END(head))

/*
 * Singly-linked Tail queue functions.
 */
#define	STAILQ_INIT(head) ചെയ്യുക {						\
	(head)->stqh_first = ശൂന്യ;					\
	(head)->stqh_last = &(head)->stqh_first;				\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	STAILQ_തിരുകുക_HEAD(head, elm, field) ചെയ്യുക {			\
	എങ്കിൽ (((elm)->field.stqe_next = (head)->stqh_first) == ശൂന്യ)	\
		(head)->stqh_last = &(elm)->field.stqe_next;		\
	(head)->stqh_first = (elm);					\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	STAILQ_തിരുകുക_TAIL(head, elm, field) ചെയ്യുക {			\
	(elm)->field.stqe_next = ശൂന്യ;					\
	*(head)->stqh_last = (elm);					\
	(head)->stqh_last = &(elm)->field.stqe_next;			\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	STAILQ_തിരുകുക_AFTER(head, listelm, elm, field) ചെയ്യുക {		\
	എങ്കിൽ (((elm)->field.stqe_next = (listelm)->field.stqe_next) == ശൂന്യ)\
		(head)->stqh_last = &(elm)->field.stqe_next;		\
	(listelm)->field.stqe_next = (elm);				\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	STAILQ_REMOVE_HEAD(head, field) ചെയ്യുക {				\
	എങ്കിൽ (((head)->stqh_first = (head)->stqh_first->field.stqe_next) == ശൂന്യ) \
		(head)->stqh_last = &(head)->stqh_first;			\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	STAILQ_REMOVE(head, elm, type, field) ചെയ്യുക {			\
	എങ്കിൽ ((head)->stqh_first == (elm)) {				\
		STAILQ_REMOVE_HEAD((head), field);			\
	} അല്ലെങ്കിൽ {							\
		ഘടന type *curelm = (head)->stqh_first;		\
		ആകുമ്പോൾ (curelm->field.stqe_next != (elm))			\
			curelm = curelm->field.stqe_next;		\
		എങ്കിൽ ((curelm->field.stqe_next =				\
			curelm->field.stqe_next->field.stqe_next) == ശൂന്യ) \
			    (head)->stqh_last = &(curelm)->field.stqe_next; \
	}								\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	STAILQ_FOREACH(var, head, field)				\
	വേണ്ടി ((var) = ((head)->stqh_first);				\
		(var);							\
		(var) = ((var)->field.stqe_next))

#define	STAILQ_FOREACH_SAFE(var, head, field, tvar)			\
	വേണ്ടി ((var) = STAILQ_FIRST((head));				\
	    (var) && ((tvar) = STAILQ_NEXT((var), field), 1);		\
	    (var) = (tvar))

#define	STAILQ_CONCAT(head1, head2) ചെയ്യുക {				\
	എങ്കിൽ (!STAILQ_EMPTY((head2))) {					\
		*(head1)->stqh_last = (head2)->stqh_first;		\
		(head1)->stqh_last = (head2)->stqh_last;		\
		STAILQ_INIT((head2));					\
	}								\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	STAILQ_LAST(head, type, field)					\
	(STAILQ_EMPTY((head)) ?						\
		ശൂന്യ :							\
	        ((ഘടന type *)(ശൂന്യം *)				\
		((അക്ഷരം *)((head)->stqh_last) - offsetof(ഘടന type, field))))


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
സ്ഥിതം __inline സ്ഥിരം ശൂന്യം * __launder_type(സ്ഥിരം ശൂന്യം *);
സ്ഥിതം __inline സ്ഥിരം ശൂന്യം *
__launder_type(സ്ഥിരം ശൂന്യം *__x)
{
	__asm __volatile("" : "+r" (__x));
	തിരികെ __x;
}

#എങ്കിൽ defined(QUEUEDEBUG)
#define QUEUEDEBUG_CIRCLEQ_HEAD(head, field)				\
	എങ്കിൽ ((head)->cqh_first != CIRCLEQ_ENDC(head) &&			\
	    (head)->cqh_first->field.cqe_prev != CIRCLEQ_ENDC(head))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head forw %p %s:%d", (head),	\
		      __FILE__, __LINE__);				\
	എങ്കിൽ ((head)->cqh_last != CIRCLEQ_ENDC(head) &&			\
	    (head)->cqh_last->field.cqe_next != CIRCLEQ_ENDC(head))	\
		QUEUEDEBUG_ABORT("CIRCLEQ head back %p %s:%d", (head),	\
		      __FILE__, __LINE__);
#define QUEUEDEBUG_CIRCLEQ_ELM(head, elm, field)			\
	എങ്കിൽ ((elm)->field.cqe_next == CIRCLEQ_ENDC(head)) {		\
		എങ്കിൽ ((head)->cqh_last != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm last %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} അല്ലെങ്കിൽ {							\
		എങ്കിൽ ((elm)->field.cqe_next->field.cqe_prev != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm forw %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}								\
	എങ്കിൽ ((elm)->field.cqe_prev == CIRCLEQ_ENDC(head)) {		\
		എങ്കിൽ ((head)->cqh_first != (elm))				\
			QUEUEDEBUG_ABORT("CIRCLEQ elm first %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	} അല്ലെങ്കിൽ {							\
		എങ്കിൽ ((elm)->field.cqe_prev->field.cqe_next != (elm))	\
			QUEUEDEBUG_ABORT("CIRCLEQ elm prev %p %s:%d",	\
			    (elm), __FILE__, __LINE__);			\
	}
#define QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, field)			\
	(elm)->field.cqe_next = (ശൂന്യം *)1L;				\
	(elm)->field.cqe_prev = (ശൂന്യം *)1L;
#അല്ലെങ്കിൽ
#define QUEUEDEBUG_CIRCLEQ_HEAD(head, field)
#define QUEUEDEBUG_CIRCLEQ_ELM(head, elm, field)
#define QUEUEDEBUG_CIRCLEQ_POSTREMOVE(elm, field)
#endif

#define	CIRCLEQ_HEAD(name, type)					\
ഘടന name {								\
	ഘടന type *cqh_first;		/* first element */		\
	ഘടന type *cqh_last;		/* last element */		\
}

#define	CIRCLEQ_HEAD_INITIALIZER(head)					\
	{ CIRCLEQ_END(&head), CIRCLEQ_END(&head) }

#define	CIRCLEQ_ENTRY(type)						\
ഘടന {								\
	ഘടന type *cqe_next;		/* next element */		\
	ഘടന type *cqe_prev;		/* previous element */		\
}

/*
 * Circular queue functions.
 */
#define	CIRCLEQ_INIT(head) ചെയ്യുക {						\
	(head)->cqh_first = CIRCLEQ_END(head);				\
	(head)->cqh_last = CIRCLEQ_END(head);				\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	CIRCLEQ_തിരുകുക_AFTER(head, listelm, elm, field) ചെയ്യുക {		\
	QUEUEDEBUG_CIRCLEQ_HEAD((head), field)				\
	QUEUEDEBUG_CIRCLEQ_ELM((head), (listelm), field)		\
	(elm)->field.cqe_next = (listelm)->field.cqe_next;		\
	(elm)->field.cqe_prev = (listelm);				\
	എങ്കിൽ ((listelm)->field.cqe_next == CIRCLEQ_ENDC(head))		\
		(head)->cqh_last = (elm);				\
	അല്ലെങ്കിൽ								\
		(listelm)->field.cqe_next->field.cqe_prev = (elm);	\
	(listelm)->field.cqe_next = (elm);				\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	CIRCLEQ_തിരുകുക_BEFORE(head, listelm, elm, field) ചെയ്യുക {		\
	QUEUEDEBUG_CIRCLEQ_HEAD((head), field)				\
	QUEUEDEBUG_CIRCLEQ_ELM((head), (listelm), field)		\
	(elm)->field.cqe_next = (listelm);				\
	(elm)->field.cqe_prev = (listelm)->field.cqe_prev;		\
	എങ്കിൽ ((listelm)->field.cqe_prev == CIRCLEQ_ENDC(head))		\
		(head)->cqh_first = (elm);				\
	അല്ലെങ്കിൽ								\
		(listelm)->field.cqe_prev->field.cqe_next = (elm);	\
	(listelm)->field.cqe_prev = (elm);				\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	CIRCLEQ_തിരുകുക_HEAD(head, elm, field) ചെയ്യുക {			\
	QUEUEDEBUG_CIRCLEQ_HEAD((head), field)				\
	(elm)->field.cqe_next = (head)->cqh_first;			\
	(elm)->field.cqe_prev = CIRCLEQ_END(head);			\
	എങ്കിൽ ((head)->cqh_last == CIRCLEQ_ENDC(head))			\
		(head)->cqh_last = (elm);				\
	അല്ലെങ്കിൽ								\
		(head)->cqh_first->field.cqe_prev = (elm);		\
	(head)->cqh_first = (elm);					\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	CIRCLEQ_തിരുകുക_TAIL(head, elm, field) ചെയ്യുക {			\
	QUEUEDEBUG_CIRCLEQ_HEAD((head), field)				\
	(elm)->field.cqe_next = CIRCLEQ_END(head);			\
	(elm)->field.cqe_prev = (head)->cqh_last;			\
	എങ്കിൽ ((head)->cqh_first == CIRCLEQ_ENDC(head))			\
		(head)->cqh_first = (elm);				\
	അല്ലെങ്കിൽ								\
		(head)->cqh_last->field.cqe_next = (elm);		\
	(head)->cqh_last = (elm);					\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	CIRCLEQ_REMOVE(head, elm, field) ചെയ്യുക {				\
	QUEUEDEBUG_CIRCLEQ_HEAD((head), field)				\
	QUEUEDEBUG_CIRCLEQ_ELM((head), (elm), field)			\
	എങ്കിൽ ((elm)->field.cqe_next == CIRCLEQ_ENDC(head))		\
		(head)->cqh_last = (elm)->field.cqe_prev;		\
	അല്ലെങ്കിൽ								\
		(elm)->field.cqe_next->field.cqe_prev =			\
		    (elm)->field.cqe_prev;				\
	എങ്കിൽ ((elm)->field.cqe_prev == CIRCLEQ_ENDC(head))		\
		(head)->cqh_first = (elm)->field.cqe_next;		\
	അല്ലെങ്കിൽ								\
		(elm)->field.cqe_prev->field.cqe_next =			\
		    (elm)->field.cqe_next;				\
	QUEUEDEBUG_CIRCLEQ_POSTREMOVE((elm), field)			\
} ആകുമ്പോൾ (/*CONSTCOND*/0)

#define	CIRCLEQ_FOREACH(var, head, field)				\
	വേണ്ടി ((var) = ((head)->cqh_first);				\
		(var) != CIRCLEQ_ENDC(head);				\
		(var) = ((var)->field.cqe_next))

#define	CIRCLEQ_FOREACH_REVERSE(var, head, field)			\
	വേണ്ടി ((var) = ((head)->cqh_last);				\
		(var) != CIRCLEQ_ENDC(head);				\
		(var) = ((var)->field.cqe_prev))

/*
 * Circular queue access methods.
 */
#define	CIRCLEQ_FIRST(head)		((head)->cqh_first)
#define	CIRCLEQ_LAST(head)		((head)->cqh_last)
/* For comparisons */
#define	CIRCLEQ_ENDC(head)		(__launder_type(head))
/* For assignments */
#define	CIRCLEQ_END(head)		((ശൂന്യം *)(head))
#define	CIRCLEQ_NEXT(elm, field)	((elm)->field.cqe_next)
#define	CIRCLEQ_PREV(elm, field)	((elm)->field.cqe_prev)
#define	CIRCLEQ_EMPTY(head)						\
    (CIRCLEQ_FIRST(head) == CIRCLEQ_ENDC(head))

#define CIRCLEQ_LOOP_NEXT(head, elm, field)				\
	(((elm)->field.cqe_next == CIRCLEQ_ENDC(head))			\
	    ? ((head)->cqh_first)					\
	    : (elm->field.cqe_next))
#define CIRCLEQ_LOOP_PREV(head, elm, field)				\
	(((elm)->field.cqe_prev == CIRCLEQ_ENDC(head))			\
	    ? ((head)->cqh_last)					\
	    : (elm->field.cqe_prev))
#endif /* !_KERNEL */

#endif	/* !_SYS_QUEUE_H_ */
