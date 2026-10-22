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

#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <stdbool.h>

/* Although OpenRC requires C99, linking to us should not. */
#ifdef restrict
# define EINFO_RESTRICT restrict
#இல்லையெனில்
# ifdef __restrict
#  define EINFO_RESTRICT __restrict
# இல்லையெனில்
#  define EINFO_RESTRICT
# endif
#endif

/* __BEGIN_DECLS */
#ifdef __cplusplus
வெளி "C" {
#endif

/*! @brief Color types to use */
வகைவரை எண்ணிக்கை
{
	ECOLOR_NORMAL	= 1,
	ECOLOR_GOOD	= 2,
	ECOLOR_WARN	= 3,
	ECOLOR_BAD	= 4,
	ECOLOR_HILITE	= 5,
	ECOLOR_BRACKET	= 6
} ECOLOR;

/*! @brief Returns the ASCII code for the color */
மாறிலி எழுத்து *ecolor(ECOLOR);

/*! @brief Writes to syslog. */
வெறுமை elog(முழு, மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);

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
முழு einfon(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
முழு ewarnn(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
முழு eerrorn(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
முழு einfo(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
முழு ewarn(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
வெறுமை ewarnx(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);
முழு eerror(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
வெறுமை eerrorx(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);

முழு einfovn(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
முழு ewarnvn(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
முழு ebeginvn(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
முழு eendvn(முழு, மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
முழு ewendvn(முழு, மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
முழு einfov(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
முழு ewarnv(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
/*@}*/

/*! @ingroup ebegin
 * @brief Display informational messages that may take some time.
 *
 * Similar to einfo, but we add ... to the end of the message */
/*@{*/
முழு ebeginv(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
முழு ebegin(மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
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
முழு eend(முழு, மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
முழு ewend(முழு, மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
வெறுமை ebracket(முழு, ECOLOR, மாறிலி எழுத்து * EINFO_RESTRICT);

முழு eendv(முழு, மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
முழு ewendv(முழு, மாறிலி எழுத்து * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
/*@}*/

/*! @ingroup eindent
 * @brief Indents the einfo lines.
 *
 * For each indent you should outdent when done */
/*@{*/
வெறுமை eindent(வெறுமை);
வெறுமை eoutdent(வெறுமை);
வெறுமை eindentv(வெறுமை);
வெறுமை eoutdentv(வெறுமை);

/*! @brief Prefix each einfo line with something */
வெறுமை eprefix(மாறிலி எழுத்து * EINFO_RESTRICT);

/* __END_DECLS */
#ifdef __cplusplus
}
#endif

#endif
