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

#నిర్వచితం_కాకపోతే __EINFO_H__
#నిర్వచించు __EINFO_H__

#నిర్వచించు EINFO_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))
#నిర్వచించు EINFO_XPRINTF(a, b) __attribute__((__noreturn__, __format__(__printf__, a, b)))

#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <stdbool.h>

/* Although OpenRC requires C99, linking to us should not. */
#నిర్వచితమైతే పరిమిత
# define EINFO_RESTRICT పరిమిత
#లేకపోతే
# ifdef __restrict
#  define EINFO_RESTRICT __restrict
# లేకపోతే
#  define EINFO_RESTRICT
# endif
#అయితే_ముగింపు

/* __BEGIN_DECLS */
#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#అయితే_ముగింపు

/*! @brief Color types to use */
రకనిర్వచనం గణన
{
	ECOLOR_సాధారణ	= 1,
	ECOLOR_GOOD	= 2,
	ECOLOR_WARN	= 3,
	ECOLOR_BAD	= 4,
	ECOLOR_HILITE	= 5,
	ECOLOR_BRACKET	= 6
} ECOLOR;

/*! @brief Returns the ASCII code for the color */
స్థిరరాశి అక్షరం *ecolor(ECOLOR);

/*! @brief Writes to syslog. */
శూన్య elog(పూర్ణాంకం, స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);

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
పూర్ణాంకం einfon(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
పూర్ణాంకం ewarnn(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
పూర్ణాంకం eerrorn(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
పూర్ణాంకం einfo(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
పూర్ణాంకం ewarn(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
శూన్య ewarnx(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);
పూర్ణాంకం eerror(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
శూన్య eerrorx(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);

పూర్ణాంకం einfovn(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
పూర్ణాంకం ewarnvn(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
పూర్ణాంకం ebeginvn(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
పూర్ణాంకం eendvn(పూర్ణాంకం, స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
పూర్ణాంకం ewendvn(పూర్ణాంకం, స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
పూర్ణాంకం einfov(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
పూర్ణాంకం ewarnv(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
/*@}*/

/*! @ingroup ebegin
 * @brief Display informational messages that may take some time.
 *
 * Similar to einfo, but we add ... to the end of the message */
/*@{*/
పూర్ణాంకం ebeginv(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
పూర్ణాంకం ebegin(స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
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
పూర్ణాంకం eend(పూర్ణాంకం, స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
పూర్ణాంకం ewend(పూర్ణాంకం, స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
శూన్య ebracket(పూర్ణాంకం, ECOLOR, స్థిరరాశి అక్షరం * EINFO_RESTRICT);

పూర్ణాంకం eendv(పూర్ణాంకం, స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
పూర్ణాంకం ewendv(పూర్ణాంకం, స్థిరరాశి అక్షరం * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
/*@}*/

/*! @ingroup eindent
 * @brief Indents the einfo lines.
 *
 * For each indent you should outdent when done */
/*@{*/
శూన్య eindent(శూన్య);
శూన్య eoutdent(శూన్య);
శూన్య eindentv(శూన్య);
శూన్య eoutdentv(శూన్య);

/*! @brief Prefix each einfo line with something */
శూన్య eprefix(స్థిరరాశి అక్షరం * EINFO_RESTRICT);

/* __END_DECLS */
#నిర్వచితమైతే __cplusplus
}
#అయితే_ముగింపు

#అయితే_ముగింపు
