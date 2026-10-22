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

#include <sys/types.h>
#include <stdbool.h>

/* Although OpenRC requires C99, linking to us should not. */
#ifdef restrict
# define EINFO_RESTRICT restrict
#မဟုတ်ပါက
# ifdef __restrict
#  define EINFO_RESTRICT __restrict
# မဟုတ်ပါက
#  define EINFO_RESTRICT
# endif
#endif

/* __BEGIN_DECLS */
#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Color types to use */
typedef ga_nanaa
{
	ECOLOR_NORMAL	= 1,
	ECOLOR_GOOD	= 2,
	ECOLOR_WARN	= 3,
	ECOLOR_BAD	= 4,
	ECOLOR_HILITE	= 5,
	ECOLOR_BRACKET	= 6
} ECOLOR;

/*! @brief Returns the ASCII code for the color */
const var_na *ecolor(ECOLOR);

/*! @brief Writes to syslog. */
void elog(ကိန်းပြည့်, const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);

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
ကိန်းပြည့် einfon(const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ကိန်းပြည့် ewarnn(const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ကိန်းပြည့် eerrorn(const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ကိန်းပြည့် einfo(const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ကိန်းပြည့် ewarn(const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
void ewarnx(const var_na * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);
ကိန်းပြည့် eerror(const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
void eerrorx(const var_na * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);

ကိန်းပြည့် einfovn(const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ကိန်းပြည့် ewarnvn(const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ကိန်းပြည့် ebeginvn(const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ကိန်းပြည့် eendvn(ကိန်းပြည့်, const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ကိန်းပြည့် ewendvn(ကိန်းပြည့်, const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ကိန်းပြည့် einfov(const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ကိန်းပြည့် ewarnv(const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
/*@}*/

/*! @ingroup ebegin
 * @brief Display informational messages that may take some time.
 *
 * Similar to einfo, but we add ... to the end of the message */
/*@{*/
ကိန်းပြည့် ebeginv(const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ကိန်းပြည့် ebegin(const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
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
ကိန်းပြည့် eend(ကိန်းပြည့်, const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ကိန်းပြည့် ewend(ကိန်းပြည့်, const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
void ebracket(ကိန်းပြည့်, ECOLOR, const var_na * EINFO_RESTRICT);

ကိန်းပြည့် eendv(ကိန်းပြည့်, const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ကိန်းပြည့် ewendv(ကိန်းပြည့်, const var_na * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
/*@}*/

/*! @ingroup eindent
 * @brief Indents the einfo lines.
 *
 * For each indent you should outdent when done */
/*@{*/
void eindent(void);
void eoutdent(void);
void eindentv(void);
void eoutdentv(void);

/*! @brief Prefix each einfo line with something */
void eprefix(const var_na * EINFO_RESTRICT);

/* __END_DECLS */
#ifdef __cplusplus
}
#endif

#endif
