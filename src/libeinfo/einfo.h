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

#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <stdbool.h>

/* Although OpenRC requires C99, linking to us should not. */
#ifdef restrict
# define EINFO_RESTRICT restrict
#অন্যথা
# ifdef __restrict
#  define EINFO_RESTRICT __restrict
# অন্যথা
#  define EINFO_RESTRICT
# endif
#endif

/* __BEGIN_DECLS */
#ifdef __cplusplus
বাহ্য "C" {
#endif

/*! @brief Color types to use */
প্রকারনির্দেশ গণনা
{
	ECOLOR_NORMAL	= 1,
	ECOLOR_GOOD	= 2,
	ECOLOR_WARN	= 3,
	ECOLOR_BAD	= 4,
	ECOLOR_HILITE	= 5,
	ECOLOR_BRACKET	= 6
} ECOLOR;

/*! @brief Returns the ASCII code for the color */
ধ্রুব বর্ণ *ecolor(ECOLOR);

/*! @brief Writes to syslog. */
শূন্য elog(পূর্ণাঙ্ক, ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);

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
পূর্ণাঙ্ক einfon(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
পূর্ণাঙ্ক ewarnn(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
পূর্ণাঙ্ক eerrorn(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
পূর্ণাঙ্ক einfo(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
পূর্ণাঙ্ক ewarn(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
শূন্য ewarnx(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);
পূর্ণাঙ্ক eerror(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
শূন্য eerrorx(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);

পূর্ণাঙ্ক einfovn(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
পূর্ণাঙ্ক ewarnvn(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
পূর্ণাঙ্ক ebeginvn(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
পূর্ণাঙ্ক eendvn(পূর্ণাঙ্ক, ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
পূর্ণাঙ্ক ewendvn(পূর্ণাঙ্ক, ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
পূর্ণাঙ্ক einfov(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
পূর্ণাঙ্ক ewarnv(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
/*@}*/

/*! @ingroup ebegin
 * @brief Display informational messages that may take some time.
 *
 * Similar to einfo, but we add ... to the end of the message */
/*@{*/
পূর্ণাঙ্ক ebeginv(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
পূর্ণাঙ্ক ebegin(ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
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
পূর্ণাঙ্ক eend(পূর্ণাঙ্ক, ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
পূর্ণাঙ্ক ewend(পূর্ণাঙ্ক, ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
শূন্য ebracket(পূর্ণাঙ্ক, ECOLOR, ধ্রুব বর্ণ * EINFO_RESTRICT);

পূর্ণাঙ্ক eendv(পূর্ণাঙ্ক, ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
পূর্ণাঙ্ক ewendv(পূর্ণাঙ্ক, ধ্রুব বর্ণ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
/*@}*/

/*! @ingroup eindent
 * @brief Indents the einfo lines.
 *
 * For each indent you should outdent when done */
/*@{*/
শূন্য eindent(শূন্য);
শূন্য eoutdent(শূন্য);
শূন্য eindentv(শূন্য);
শূন্য eoutdentv(শূন্য);

/*! @brief Prefix each einfo line with something */
শূন্য eprefix(ধ্রুব বর্ণ * EINFO_RESTRICT);

/* __END_DECLS */
#ifdef __cplusplus
}
#endif

#endif
