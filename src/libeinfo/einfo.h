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

#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <stdbool.h>

/* Although OpenRC requires C99, linking to us should not. */
#ifdef restrict
# define EINFO_RESTRICT restrict
#അല്ലെങ്കിൽ
# ifdef __restrict
#  define EINFO_RESTRICT __restrict
# അല്ലെങ്കിൽ
#  define EINFO_RESTRICT
# endif
#endif

/* __BEGIN_DECLS */
#ifdef __cplusplus
ബാഹ്യ "C" {
#endif

/*! @brief Color types to use */
തരനിർവചനം എണ്ണൽ
{
	ECOLOR_NORMAL	= 1,
	ECOLOR_GOOD	= 2,
	ECOLOR_WARN	= 3,
	ECOLOR_BAD	= 4,
	ECOLOR_HILITE	= 5,
	ECOLOR_BRACKET	= 6
} ECOLOR;

/*! @brief Returns the ASCII code for the color */
സ്ഥിരം അക്ഷരം *ecolor(ECOLOR);

/*! @brief Writes to syslog. */
ശൂന്യം elog(പൂർണ്ണസംഖ്യ, സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);

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
പൂർണ്ണസംഖ്യ einfon(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
പൂർണ്ണസംഖ്യ ewarnn(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
പൂർണ്ണസംഖ്യ eerrorn(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
പൂർണ്ണസംഖ്യ einfo(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
പൂർണ്ണസംഖ്യ ewarn(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ശൂന്യം ewarnx(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);
പൂർണ്ണസംഖ്യ eerror(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ശൂന്യം eerrorx(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);

പൂർണ്ണസംഖ്യ einfovn(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
പൂർണ്ണസംഖ്യ ewarnvn(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
പൂർണ്ണസംഖ്യ ebeginvn(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
പൂർണ്ണസംഖ്യ eendvn(പൂർണ്ണസംഖ്യ, സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
പൂർണ്ണസംഖ്യ ewendvn(പൂർണ്ണസംഖ്യ, സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
പൂർണ്ണസംഖ്യ einfov(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
പൂർണ്ണസംഖ്യ ewarnv(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
/*@}*/

/*! @ingroup ebegin
 * @brief Display informational messages that may take some time.
 *
 * Similar to einfo, but we add ... to the end of the message */
/*@{*/
പൂർണ്ണസംഖ്യ ebeginv(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
പൂർണ്ണസംഖ്യ ebegin(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
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
പൂർണ്ണസംഖ്യ eend(പൂർണ്ണസംഖ്യ, സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
പൂർണ്ണസംഖ്യ ewend(പൂർണ്ണസംഖ്യ, സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ശൂന്യം ebracket(പൂർണ്ണസംഖ്യ, ECOLOR, സ്ഥിരം അക്ഷരം * EINFO_RESTRICT);

പൂർണ്ണസംഖ്യ eendv(പൂർണ്ണസംഖ്യ, സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
പൂർണ്ണസംഖ്യ ewendv(പൂർണ്ണസംഖ്യ, സ്ഥിരം അക്ഷരം * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
/*@}*/

/*! @ingroup eindent
 * @brief Indents the einfo lines.
 *
 * For each indent you should outdent when done */
/*@{*/
ശൂന്യം eindent(ശൂന്യം);
ശൂന്യം eoutdent(ശൂന്യം);
ശൂന്യം eindentv(ശൂന്യം);
ശൂന്യം eoutdentv(ശൂന്യം);

/*! @brief Prefix each einfo line with something */
ശൂന്യം eprefix(സ്ഥിരം അക്ഷരം * EINFO_RESTRICT);

/* __END_DECLS */
#ifdef __cplusplus
}
#endif

#endif
