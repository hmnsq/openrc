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

#अगर_अघोषित __EINFO_H__
#घोषणा __EINFO_H__

#घोषणा EINFO_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))
#घोषणा EINFO_XPRINTF(a, b) __attribute__((__noreturn__, __format__(__printf__, a, b)))

#समावेश <sys/types.h>
#समावेश <stdbool.h>

/* Although OpenRC requires C99, linking to us should not. */
#अगर_घोषित restrict
# define EINFO_RESTRICT restrict
#अन्यथा
# ifdef __restrict
#  define EINFO_RESTRICT __restrict
# अन्यथा
#  define EINFO_RESTRICT
# endif
#पूर्ण_अगर

/* __BEGIN_DECLS */
#अगर_घोषित __cplusplus
बाह्य "C" {
#पूर्ण_अगर

/*! @brief Color types to use */
प्रकार क्रमागत
{
	ECOLOR_NORMAL	= 1,
	ECOLOR_GOOD	= 2,
	ECOLOR_WARN	= 3,
	ECOLOR_BAD	= 4,
	ECOLOR_HILITE	= 5,
	ECOLOR_BRACKET	= 6
} ECOLOR;

/*! @brief Returns the ASCII code for the color */
स्थिर अक्षर *ecolor(ECOLOR);

/*! @brief Writes to syslog. */
व्योम elog(पूर्णांक, स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);

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
पूर्णांक einfon(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
पूर्णांक ewarnn(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
पूर्णांक eerrorn(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
पूर्णांक einfo(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
पूर्णांक ewarn(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
व्योम ewarnx(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);
पूर्णांक eerror(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
व्योम eerrorx(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_XPRINTF(1, 2);

पूर्णांक einfovn(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
पूर्णांक ewarnvn(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
पूर्णांक ebeginvn(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
पूर्णांक eendvn(पूर्णांक, स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
पूर्णांक ewendvn(पूर्णांक, स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
पूर्णांक einfov(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
पूर्णांक ewarnv(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
/*@}*/

/*! @ingroup ebegin
 * @brief Display informational messages that may take some time.
 *
 * Similar to einfo, but we add ... to the end of the message */
/*@{*/
पूर्णांक ebeginv(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
पूर्णांक ebegin(स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(1, 2);
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
पूर्णांक eend(पूर्णांक, स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
पूर्णांक ewend(पूर्णांक, स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
व्योम ebracket(पूर्णांक, ECOLOR, स्थिर अक्षर * EINFO_RESTRICT);

पूर्णांक eendv(पूर्णांक, स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
पूर्णांक ewendv(पूर्णांक, स्थिर अक्षर * EINFO_RESTRICT, ...) EINFO_PRINTF(2, 3);
/*@}*/

/*! @ingroup eindent
 * @brief Indents the einfo lines.
 *
 * For each indent you should outdent when done */
/*@{*/
व्योम eindent(व्योम);
व्योम eoutdent(व्योम);
व्योम eindentv(व्योम);
व्योम eoutdentv(व्योम);

/*! @brief Prefix each einfo line with something */
व्योम eprefix(स्थिर अक्षर * EINFO_RESTRICT);

/* __END_DECLS */
#अगर_घोषित __cplusplus
}
#पूर्ण_अगर

#पूर्ण_अगर
