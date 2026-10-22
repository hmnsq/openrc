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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __EINFO_H__
#ವ್ಯಾಖ್ಯಾನಿಸು __EINFO_H__

#ವ್ಯಾಖ್ಯಾನಿಸು EINFO_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))
#ವ್ಯಾಖ್ಯಾನಿಸು EINFO_XPRINTF(a, b) __attribute__((__noreturn__, __format__(__printf__, a, b)))

#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>

/* Although OpenRC requires C99, linking to us should not. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ಸೀಮಿತ
# define EINFO_RESTRICT ಸೀಮಿತ
#ಇಲ್ಲದಿದ್ದರೆ
# ifdef __restrict
#  define EINFO_RESTRICT __restrict
# ಇಲ್ಲದಿದ್ದರೆ
#  define EINFO_RESTRICT
# endif
#ಆದರೆ_ಅಂತ್ಯ

/* __BEGIN_DECLS */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
ಬಾಹ್ಯ "C" {
#ಆದರೆ_ಅಂತ್ಯ

/*! @brief Color types to use */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ
{
	ECOLOR_ಸಾಮಾನ್ಯ	= 1,
	ECOLOR_GOOD	= 2,
	ECOLOR_WARN	= 3,
	ECOLOR_BAD	= 4,
	ECOLOR_HILITE	= 5,
	ECOLOR_BRACKET	= 6
} ECOLOR;

/*! @brief Returns the ASCII code for the color */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ecolor(ECOLOR);

/*! @brief Writes to syslog. */
ಶೂನ್ಯ elog(ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);

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
ಪೂರ್ಣಾಂಕ einfon(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ಪೂರ್ಣಾಂಕ ewarnn(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ಪೂರ್ಣಾಂಕ eerrorn(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ಪೂರ್ಣಾಂಕ einfo(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ಪೂರ್ಣಾಂಕ ewarn(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ಶೂನ್ಯ ewarnx(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);
ಪೂರ್ಣಾಂಕ eerror(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ಶೂನ್ಯ eerrorx(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);

ಪೂರ್ಣಾಂಕ einfovn(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ಪೂರ್ಣಾಂಕ ewarnvn(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ಪೂರ್ಣಾಂಕ ebeginvn(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ಪೂರ್ಣಾಂಕ eendvn(ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ಪೂರ್ಣಾಂಕ ewendvn(ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ಪೂರ್ಣಾಂಕ einfov(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ಪೂರ್ಣಾಂಕ ewarnv(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
/*@}*/

/*! @ingroup ebegin
 * @brief Display informational messages that may take some time.
 *
 * Similar to einfo, but we add ... to the end of the message */
/*@{*/
ಪೂರ್ಣಾಂಕ ebeginv(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ಪೂರ್ಣಾಂಕ ebegin(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
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
ಪೂರ್ಣಾಂಕ eend(ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ಪೂರ್ಣಾಂಕ ewend(ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ಶೂನ್ಯ ebracket(ಪೂರ್ಣಾಂಕ, ECOLOR, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT);

ಪೂರ್ಣಾಂಕ eendv(ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ಪೂರ್ಣಾಂಕ ewendv(ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
/*@}*/

/*! @ingroup eindent
 * @brief Indents the einfo lines.
 *
 * For each indent you should outdent when done */
/*@{*/
ಶೂನ್ಯ eindent(ಶೂನ್ಯ);
ಶೂನ್ಯ eoutdent(ಶೂನ್ಯ);
ಶೂನ್ಯ eindentv(ಶೂನ್ಯ);
ಶೂನ್ಯ eoutdentv(ಶೂನ್ಯ);

/*! @brief Prefix each einfo line with something */
ಶೂನ್ಯ eprefix(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * EINFO_RESTRICT);

/* __END_DECLS */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ
