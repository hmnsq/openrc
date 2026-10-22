/*
 * Copyright (c) 2007-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ifndef __EINFO_H__
#define __EINFO_H__

#define EINFO_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))
#define EINFO_XPRINTF(a, b) __attribute__((__noreturn__, __format__(__printf__, a, b)))

#شامل <sys/types.h>
#شامل <stdbool.h>

/* Although OpenRC requires C99, linking to us should not. */
#ifdef restrict
# define EINFO_RESTRICT restrict
#وگرنه
# ifdef __restrict
#  define EINFO_RESTRICT __restrict
# وگرنه
#  define EINFO_RESTRICT
# endif
#endif

/* __BEGIN_DECLS */
#ifdef __cplusplus
بیرونی "C" {
#endif

/*! @brief Color types to use */
تعریفنوع شمارش
{
	ECOLOR_NORMAL	= 1,
	ECOLOR_GOOD	= 2,
	ECOLOR_WARN	= 3,
	ECOLOR_BAD	= 4,
	ECOLOR_HILITE	= 5,
	ECOLOR_BRACKET	= 6
} ECOLOR;

/*! @brief Returns the ASCII code for the color */
ثابت نویسه *ecolor(ECOLOR);

/*! @brief Writes to syslog. */
تهی elog(صحیح, ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);

/*!
 * @brief Display informational messages.
 *
 * The einfo family of functions display messages in a consistent manner
 * across applications. Basically they prefix the message with
 * " * ". If the terminal can handle color then we color the * based on
 * the command used. Otherwise we are identical to the printf function.
 *
 * - einfo  - green
 * - ewarn  - yellow
 * - eerror - red
 *
 * The n suffix denotes that no new line should be printed.
 * The v suffix means only print if EINFO_VERBOSE is yes.
 * The x suffix means function will exit() returning failure.
 */
/*@{*/
صحیح einfon(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
صحیح ewarnn(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
صحیح eerrorn(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
صحیح einfo(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
صحیح ewarn(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
تهی ewarnx(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);
صحیح eerror(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
تهی eerrorx(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);

صحیح einfovn(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
صحیح ewarnvn(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
صحیح ebeginvn(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
صحیح eendvn(صحیح, ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
صحیح ewendvn(صحیح, ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
صحیح einfov(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
صحیح ewarnv(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
/*@}*/

/*! @ingroup ebegin
 * @brief Display informational messages that may take some time.
 *
 * Similar to einfo, but we add ... to the end of the message */
/*@{*/
صحیح ebeginv(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
صحیح ebegin(ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
/*@}*/

/*! @ingroup eend
 * @brief End an ebegin.
 *
 * If you ebegin, you should eend also.
 * eend places [ ok ] or [ !! ] at the end of the terminal line depending on
 * retval (0 or ok, anything else for !!)
 *
 * ebracket allows you to specify the position, color and message */
/*@{*/
صحیح eend(صحیح, ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
صحیح ewend(صحیح, ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
تهی ebracket(صحیح, ECOLOR, ثابت نویسه * EINFO_RESTRICT);

صحیح eendv(صحیح, ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
صحیح ewendv(صحیح, ثابت نویسه * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
/*@}*/

/*! @ingroup eindent
 * @brief Indents the einfo lines.
 *
 * For each indent you should outdent when done */
/*@{*/
تهی eindent(تهی);
تهی eoutdent(تهی);
تهی eindentv(تهی);
تهی eoutdentv(تهی);

/*! @brief Prefix each einfo line with something */
تهی eprefix(ثابت نویسه * EINFO_RESTRICT);

/* __END_DECLS */
#ifdef __cplusplus
}
#endif

#endif
