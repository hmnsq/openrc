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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __EINFO_H__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __EINFO_H__

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EINFO_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EINFO_XPRINTF(a, b) __attribute__((__noreturn__, __format__(__printf__, a, b)))

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>

/* Although OpenRC requires C99, linking to us should not. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ਸੀਮਿਤ
# define EINFO_RESTRICT ਸੀਮਿਤ
#ਨਹੀਂਤਾਂ
# ifdef __restrict
#  define EINFO_RESTRICT __restrict
# ਨਹੀਂਤਾਂ
#  define EINFO_RESTRICT
# endif
#ਜੇ_ਅੰਤ

/* __BEGIN_DECLS */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ

/*! @brief Color types to use */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ
{
	ECOLOR_ਸਾਧਾਰਨ	= 1,
	ECOLOR_GOOD	= 2,
	ECOLOR_WARN	= 3,
	ECOLOR_BAD	= 4,
	ECOLOR_HILITE	= 5,
	ECOLOR_BRACKET	= 6
} ECOLOR;

/*! @brief Returns the ASCII code for the color */
ਅਚਲ ਅੱਖਰ *ecolor(ECOLOR);

/*! @brief Writes to syslog. */
ਖਾਲੀ elog(ਪੂਰਨ, ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);

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
ਪੂਰਨ einfon(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ਪੂਰਨ ewarnn(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ਪੂਰਨ eerrorn(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ਪੂਰਨ einfo(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ਪੂਰਨ ewarn(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ਖਾਲੀ ewarnx(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);
ਪੂਰਨ eerror(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ਖਾਲੀ eerrorx(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);

ਪੂਰਨ einfovn(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ਪੂਰਨ ewarnvn(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ਪੂਰਨ ebeginvn(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ਪੂਰਨ eendvn(ਪੂਰਨ, ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ਪੂਰਨ ewendvn(ਪੂਰਨ, ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ਪੂਰਨ einfov(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ਪੂਰਨ ewarnv(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
/*@}*/

/*! @ingroup ebegin
 * @brief Display informational messages that may take some time.
 *
 * Similar to einfo, but we add ... to the end of the message */
/*@{*/
ਪੂਰਨ ebeginv(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ਪੂਰਨ ebegin(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
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
ਪੂਰਨ eend(ਪੂਰਨ, ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ਪੂਰਨ ewend(ਪੂਰਨ, ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ਖਾਲੀ ebracket(ਪੂਰਨ, ECOLOR, ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT);

ਪੂਰਨ eendv(ਪੂਰਨ, ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ਪੂਰਨ ewendv(ਪੂਰਨ, ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
/*@}*/

/*! @ingroup eindent
 * @brief Indents the einfo lines.
 *
 * For each indent you should outdent when done */
/*@{*/
ਖਾਲੀ eindent(ਖਾਲੀ);
ਖਾਲੀ eoutdent(ਖਾਲੀ);
ਖਾਲੀ eindentv(ਖਾਲੀ);
ਖਾਲੀ eoutdentv(ਖਾਲੀ);

/*! @brief Prefix each einfo line with something */
ਖਾਲੀ eprefix(ਅਚਲ ਅੱਖਰ * EINFO_RESTRICT);

/* __END_DECLS */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ
