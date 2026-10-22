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

#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>

/* Although OpenRC requires C99, linking to us should not. */
#ifdef restrict
# define EINFO_RESTRICT restrict
#ଅନ୍ୟଥା
# ifdef __restrict
#  define EINFO_RESTRICT __restrict
# ଅନ୍ୟଥା
#  define EINFO_RESTRICT
# endif
#endif

/* __BEGIN_DECLS */
#ifdef __cplusplus
ବାହ୍ୟ "C" {
#endif

/*! @brief Color types to use */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା
{
	ECOLOR_NORMAL	= 1,
	ECOLOR_GOOD	= 2,
	ECOLOR_WARN	= 3,
	ECOLOR_BAD	= 4,
	ECOLOR_HILITE	= 5,
	ECOLOR_BRACKET	= 6
} ECOLOR;

/*! @brief Returns the ASCII code for the color */
ଅଚଳ ଅକ୍ଷର *ecolor(ECOLOR);

/*! @brief Writes to syslog. */
ଶୂନ୍ୟ elog(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);

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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା einfon(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ewarnn(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା eerrorn(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା einfo(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ewarn(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ଶୂନ୍ୟ ewarnx(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା eerror(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ଶୂନ୍ୟ eerrorx(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);

ପୂର୍ଣ୍ଣସଂଖ୍ୟା einfovn(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ewarnvn(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ebeginvn(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା eendvn(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ewendvn(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା einfov(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ewarnv(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
/*@}*/

/*! @ingroup ebegin
 * @brief Display informational messages that may take some time.
 *
 * Similar to einfo, but we add ... to the end of the message */
/*@{*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ebeginv(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ebegin(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା eend(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ewend(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ଶୂନ୍ୟ ebracket(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ECOLOR, ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT);

ପୂର୍ଣ୍ଣସଂଖ୍ୟା eendv(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ewendv(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
/*@}*/

/*! @ingroup eindent
 * @brief Indents the einfo lines.
 *
 * For each indent you should outdent when done */
/*@{*/
ଶୂନ୍ୟ eindent(ଶୂନ୍ୟ);
ଶୂନ୍ୟ eoutdent(ଶୂନ୍ୟ);
ଶୂନ୍ୟ eindentv(ଶୂନ୍ୟ);
ଶୂନ୍ୟ eoutdentv(ଶୂନ୍ୟ);

/*! @brief Prefix each einfo line with something */
ଶୂନ୍ୟ eprefix(ଅଚଳ ଅକ୍ଷର * EINFO_RESTRICT);

/* __END_DECLS */
#ifdef __cplusplus
}
#endif

#endif
