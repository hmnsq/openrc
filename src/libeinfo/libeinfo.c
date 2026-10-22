/*
  einfo.c
  Informational functions
*/

/*
 * Copyright (c) 2007-2025 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ಅನ್ತರ್ಭಾವ <sys/ioctl.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <syslog.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

/* In case we cannot work out how many columns from ioctl, supply a default */
#ವ್ಯಾಖ್ಯಾನಿಸು DEFAULT_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ		 80

#ವ್ಯಾಖ್ಯಾನಿಸು OK			"ok"
#ವ್ಯಾಖ್ಯಾನಿಸು NOT_OK			"!!"

/* Number of spaces for an indent */
#ವ್ಯಾಖ್ಯಾನಿಸು INDENT_ಅಗಲ		2

/* How wide can the indent go? */
#ವ್ಯಾಖ್ಯಾನಿಸು INDENT_MAX		40

/* Default colours */
#ವ್ಯಾಖ್ಯಾನಿಸು GOOD                    2
#ವ್ಯಾಖ್ಯಾನಿಸು WARN                    3
#ವ್ಯಾಖ್ಯಾನಿಸು BAD                     1
#ವ್ಯಾಖ್ಯಾನಿಸು HILITE                  6
#ವ್ಯಾಖ್ಯಾನಿಸು BRACKET                 4

/* ANSI escape codes which mimic termcap */
#ವ್ಯಾಖ್ಯಾನಿಸು AF "\033[3%dm"
#ವ್ಯಾಖ್ಯಾನಿಸು CE "\033[K"
#ವ್ಯಾಖ್ಯಾನಿಸು CH "\033[%dC"
#ವ್ಯಾಖ್ಯಾನಿಸು MD "\033[1m"
#ವ್ಯಾಖ್ಯಾನಿಸು ME "\033[m"
#ವ್ಯಾಖ್ಯಾನಿಸು ಮೇಲೆ "\033[A"

#ವ್ಯಾಖ್ಯಾನಿಸು _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), ಗಾತ್ರ(_d));
#ವ್ಯಾಖ್ಯಾನಿಸು ನಿಯೋಜಿಸು_CAP(_v) ಮಾಡು {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, ಗಾತ್ರ(ebuffer) - (p - ebuffer)) + 1;    \
	} ಆಗ (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *_eprefix = ಖಾಲಿ;

/* Buffers and structures to hold the final colours */
ಸ್ಥಿರ ಅಕ್ಷರ ebuffer[100];
ರಚನೆ ecolor {
	ECOLOR color;
	ಪೂರ್ಣಾಂಕ def;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
};
ಸ್ಥಿರ ಅಕ್ಷರ nullstr = '\0';

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_ಸಾಮಾನ್ಯ,  0,       ಖಾಲಿ      },
};
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ecolors_str[ARRAY_ಗಾತ್ರ(ecolors)];

ಸ್ಥಿರ ಅಕ್ಷರ *flush = ಖಾಲಿ;
ಸ್ಥಿರ ಅಕ್ಷರ *ಮೇಲೆ_2 = ಖಾಲಿ;
ಸ್ಥಿರ ಅಕ್ಷರ *goto_ಸ್ತಂಭ = ಖಾಲಿ;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪದ = ಖಾಲಿ;
ಸ್ಥಿರ bool ಪದ_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ color_terms[] = {
	"Eterm",
	"alacritty",
	"ansi",
	"con132x25",
	"con132x30",
	"con132x43",
	"con132x60",
	"con80x25",
	"con80x28",
	"con80x30",
	"con80x43",
	"con80x50",
	"con80x60",
	"cons25",
	"console",
	"cygwin",
	"dtterm",
	"foot",
	"gnome",
	"konsole",
	"kterm",
	"linux",
	"linux-c",
	"mlterm",
	"putty",
	"rxvt",
	"rxvt-cygwin",
	"rxvt-cygwin-native",
	"rxvt-unicode",
	"screen",
	"screen-bce",
	"screen-w",
	"screen.linux",
	"vt100",
	"vt220",
	"wsvt25",
	"xterm",
	"xterm-debian",
	"xterm-kitty",
    "xterm-ghostty",
	ಖಾಲಿ
};

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ HAVE_STRLCPY
ಸ್ಥಿರ size_t
strlcat(ಅಕ್ಷರ *dst, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, size_t ಗಾತ್ರ_2)
{
	ಅಕ್ಷರ *d = dst;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = src;
	size_t src_n = ಗಾತ್ರ_2;
	size_t dst_n;

	ಆಗ (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = ಗಾತ್ರ_2 - dst_n;

	ಆದರೆ (src_n == 0)
		ಹಿಂತಿರುಗಿ dst_n + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(src);

	ಆಗ (*s != '\0') {
		ಆದರೆ (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	ಹಿಂತಿರುಗಿ dst_n + (s - src);
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ bool
yesno(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ಆದರೆ (!ಮೌಲ್ಯ) {
		errno = ENOENT;
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (strcasecmp(ಮೌಲ್ಯ, "yes") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "y") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "true") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "on") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "1") == 0)
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (strcasecmp(ಮೌಲ್ಯ, "no") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "n") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "false") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "off") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "0") != 0)
		errno = EINVAL;

	ಹಿಂತಿರುಗಿ false;
}

ಸ್ಥಿರ bool
noyes(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ಪೂರ್ಣಾಂಕ serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(ಮೌಲ್ಯ);
	ಆದರೆ (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	ಹಿಂತಿರುಗಿ retval;
}

ಸ್ಥಿರ bool
is_quiet(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ yesno(getenv("EINFO_QUIET"));
}

ಸ್ಥಿರ bool
is_really_quiet(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ yesno(getenv("EERROR_QUIET"));
}

ಸ್ಥಿರ bool
is_verbose(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
ಸ್ಥಿರ ಅಕ್ಷರ *
tgoto(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cap, ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು, ಪೂರ್ಣಾಂಕ ಸಾಲು)
{
	ಸ್ಥಿರ ಅಕ್ಷರ buf[20];
	ಅಕ್ಷರ *p, *e, c, dbuf[6];
	ಪೂರ್ಣಾಂಕ oncol = 0, which = ಸಾಲು, i;

	p = buf;
	e = p + ಗಾತ್ರ(buf);
	ಆಗ ((c = *cap++)) {
		ಆದರೆ (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			ಆದರೆ (p >= e) {
				errno = E2BIG;
				ಹಿಂತಿರುಗಿ ಖಾಲಿ;
			}
			ಮುಂದುವರಿ;
		}
		ಆಯ್ಕೆ (c) {
		ಸ್ಥಿತಿ '3':
		ಸ್ಥಿತಿ '2':
		ಸ್ಥಿತಿ 'd':
			i = 0;
			ಮಾಡು
				dbuf[i++] = which % 10 | '0';
			ಆಗ ((which /= 10));
			ಆದರೆ (c != 'd') {
				c -= '0';
				ಆದರೆ (i > c) {
					errno = EINVAL;
					ಹಿಂತಿರುಗಿ ಖಾಲಿ;
				}
				ಆಗ (i < c)
					dbuf[i++] = '0';
			}
			ಆದರೆ (p + i >= e) {
				errno = E2BIG;
				ಹಿಂತಿರುಗಿ ಖಾಲಿ;
			}
			ಮಾಡು
				*p++ = dbuf[--i];
			ಆಗ (i);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'r':
			oncol = 0;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'i':
			ಲಂಬಸಾಲು++;
			ಸಾಲು++;
			which++;
			ಮುಂದುವರಿ;
		ಪೂರ್ವನಿಯೋಜಿತ:
			errno = EINVAL;
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		oncol = 1 - oncol;
		which = oncol ? ಲಂಬಸಾಲು : ಸಾಲು;
	}
	*p = '\0';
	ಹಿಂತಿರುಗಿ buf;
}

ಸ್ಥಿರ bool
ಬಣ್ಣ_terminal(FILE * EINFO_RESTRICT f)
{
	ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಒಳ_ಬಣ್ಣ = -1;
	ಅಕ್ಷರ *e, *ee, *ಕೊನೆ, *d, *p;
	ಪೂರ್ಣಾಂಕ c;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *_af = ಖಾಲಿ, *_ce = ಖಾಲಿ, *_ch = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *_md = ಖಾಲಿ, *_me = ಖಾಲಿ, *ಮೇಲೆ_3 = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *bold;
	ಅಕ್ಷರ tmp[100];
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i = 0;

	ಆದರೆ (f && !isatty(fileno(f)))
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (noyes(getenv("EINFO_COLOR")))
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (ಒಳ_ಬಣ್ಣ == 0)
		ಹಿಂತಿರುಗಿ false;
	ಆದರೆ (ಒಳ_ಬಣ್ಣ == 1)
		ಹಿಂತಿರುಗಿ true;

	ಪದ_is_cons25 = false;
	ಆದರೆ (!ಪದ) {
		ಪದ = getenv("TERM");
		ಆದರೆ (!ಪದ)
			ಹಿಂತಿರುಗಿ false;
	}
	ಆದರೆ (strcmp(ಪದ, "cons25") == 0)
		ಪದ_is_cons25 = true;

	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(ಪದ, "color"))
		ಒಳ_ಬಣ್ಣ = 1;

	ಆಗ (color_terms[i] && ಒಳ_ಬಣ್ಣ != 1) {
		ಆದರೆ (strcmp(color_terms[i], ಪದ) == 0) {
			ಒಳ_ಬಣ್ಣ = 1;
		}
		i++;
	}

	ಆದರೆ (ಒಳ_ಬಣ್ಣ != 1) {
		ಒಳ_ಬಣ್ಣ = 0;
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (!_af)
		_af = AF;
	ಆದರೆ (!_ce)
		_ce = CE;
	ಆದರೆ (!_ch)
		_ch = CH;
	ಆದರೆ (!_md)
		_md = MD;
	ಆದರೆ (!_me)
		_me = ME;
	ಆದರೆ (!ಮೇಲೆ_3)
		ಮೇಲೆ_3 = ಮೇಲೆ;

	/* Now setup our colours */
	p = ebuffer;
	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(ecolors); ++i) {
		tmp[0] = '\0';
		ಆದರೆ (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			ಆದರೆ ((e = getenv("EINFO_COLOR"))) {
				ee = ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(e, ecolors[i].name);
				ಆದರೆ (ee)
					ee += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ecolors[i].name);

				ಆದರೆ (ee && *ee == '=') {
					d = strdup(ee + 1);
					ಆದರೆ (d) {
						ಕೊನೆ = strchr(d, ':');
						ಆದರೆ (ಕೊನೆ)
							*ಕೊನೆ = '\0';
						c = atoi(d);
						ಕೊನೆ = strchr(d, ';');
						ಆದರೆ (ಕೊನೆ && *++ಕೊನೆ == '0')
							bold = _me;
						ಬಿಡುಗಡೆ(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), ಗಾತ್ರ(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), ಗಾತ್ರ(tmp));
		} ಇಲ್ಲದಿದ್ದರೆ
			_GET_CAP(tmp, _me);

		ಆದರೆ (tmp[0])
			ನಿಯೋಜಿಸು_CAP(ecolors_str[i]);
		ಇಲ್ಲದಿದ್ದರೆ
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	ನಿಯೋಜಿಸು_CAP(flush);
	_GET_CAP(tmp, ಮೇಲೆ_3);
	ನಿಯೋಜಿಸು_CAP(ಮೇಲೆ_2);
	strlcpy(tmp, _ch, ಗಾತ್ರ(tmp));
	ನಿಯೋಜಿಸು_CAP(goto_ಸ್ತಂಭ);

	ಒಳ_ಬಣ್ಣ = 1;
	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
get_ಪದ_columns(FILE * EINFO_RESTRICT ಹರಿವು)
{
	ರಚನೆ winsize ws;
	ಅಕ್ಷರ *env = getenv("COLUMNS");
	ಅಕ್ಷರ *p;
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (env) {
		i = strtoimax(env, &p, 10);
		ಆದರೆ (!*p)
			ಹಿಂತಿರುಗಿ i;
	}

	ಆದರೆ (ioctl(fileno(ಹರಿವು), TIOCGWINSZ, &ws) == 0)
		ಹಿಂತಿರುಗಿ ws.ws_col;

	ಹಿಂತಿರುಗಿ DEFAULT_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;
}

ಶೂನ್ಯ
eprefix(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT ಪೂರ್ವಪ್ರತ್ಯಯ)
{
	_eprefix = ಪೂರ್ವಪ್ರತ್ಯಯ;
}

ಸ್ಥಿರ ಶೂನ್ಯ EINFO_PRINTF(2, 0)
elogv(ಪೂರ್ಣಾಂಕ level, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, va_list ap)
{
	ಅಕ್ಷರ *e = getenv("EINFO_LOG");
	va_list apc;

	ಆದರೆ (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

ಶೂನ್ಯ
elog(ಪೂರ್ಣಾಂಕ level, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
_eindent(FILE * EINFO_RESTRICT ಹರಿವು)
{
	ಅಕ್ಷರ *env = getenv("EINFO_INDENT");
	ಪೂರ್ಣಾಂಕ amount = 0;

	ಆದರೆ (env) {
		errno = 0;
		amount = strtoimax(env, ಖಾಲಿ, 0);
		ಆದರೆ (errno != 0 || amount < 0)
			amount = 0;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	ಹಿಂತಿರುಗಿ fprintf(ಹರಿವು, "%*s", amount, "");
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (!ಬಣ್ಣ_terminal(f))
		ಹಿಂತಿರುಗಿ "";

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(ecolors); ++i)
		ಆದರೆ (ecolors[i].color == color)
			ಹಿಂತಿರುಗಿ ecolors_str[i];
	ಹಿಂತಿರುಗಿ "";
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	ಆದರೆ (!isatty(fileno(f))) {
		f = stderr;
		ಆದರೆ (!isatty(fileno(f))) {
			f = stdin;
			ಆದರೆ (!isatty(fileno(f)))
				f = ಖಾಲಿ;
		}
	}

	ಹಿಂತಿರುಗಿ _ecolor(f, color);
}

#ವ್ಯಾಖ್ಯಾನಿಸು LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, va_list va)
{
	ಪೂರ್ಣಾಂಕ retval = 0;
	ಅಕ್ಷರ *ಕೊನೆಯ = getenv("EINFO_LASTCMD");
	va_list ap;

	ಆದರೆ (ಕೊನೆಯ &&
	    *ಕೊನೆಯ &&
	    !ಬಣ್ಣ_terminal(f) &&
	    strcmp(ಕೊನೆಯ, "ewarn") != 0 &&
	    ಕೊನೆಯ[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಕೊನೆಯ) - 1] == 'n')
		fprintf(f, "\n");
	ಆದರೆ (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_ಸಾಮಾನ್ಯ));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_ಸಾಮಾನ್ಯ));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	ಆದರೆ (ಬಣ್ಣ_terminal(f))
		fprintf(f, "%s", flush);
	ಹಿಂತಿರುಗಿ retval;
}

#ವ್ಯಾಖ್ಯಾನಿಸು _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#ವ್ಯಾಖ್ಯಾನಿಸು _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#ವ್ಯಾಖ್ಯಾನಿಸು _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

ಪೂರ್ಣಾಂಕ
einfon(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಂಕ retval;
	va_list ap;

	ಆದರೆ (!fmt || is_quiet())
		ಹಿಂತಿರುಗಿ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	ಹಿಂತಿರುಗಿ retval;
}

ಪೂರ್ಣಾಂಕ
ewarnn(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಂಕ retval;
	va_list ap;

	ಆದರೆ (!fmt || is_quiet())
		ಹಿಂತಿರುಗಿ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	ಹಿಂತಿರುಗಿ retval;
}

ಪೂರ್ಣಾಂಕ
eerrorn(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಂಕ retval;
	va_list ap;

	ಆದರೆ (!fmt || is_really_quiet())
		ಹಿಂತಿರುಗಿ 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	ಹಿಂತಿರುಗಿ retval;
}

ಪೂರ್ಣಾಂಕ
einfo(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಂಕ retval;
	va_list ap;

	ಆದರೆ (!fmt || is_quiet())
		ಹಿಂತಿರುಗಿ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += ಬರೆ("\n");
	va_end(ap);
	LASTCMD("einfo");
	ಹಿಂತಿರುಗಿ retval;
}

ಪೂರ್ಣಾಂಕ
ewarn(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಂಕ retval;
	va_list ap;

	ಆದರೆ (!fmt || is_quiet())
		ಹಿಂತಿರುಗಿ 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	ಹಿಂತಿರುಗಿ retval;
}

ಶೂನ್ಯ
ewarnx(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ಆದರೆ (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
}

ಪೂರ್ಣಾಂಕ
eerror(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಂಕ retval;
	va_list ap;

	ಆದರೆ (!fmt || is_really_quiet())
		ಹಿಂತಿರುಗಿ 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	ಹಿಂತಿರುಗಿ retval;
}

ಶೂನ್ಯ
eerrorx(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ಆದರೆ (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
}

ಪೂರ್ಣಾಂಕ
ebegin(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಂಕ retval;
	va_list ap;

	ಆದರೆ (!fmt || is_quiet())
		ಹಿಂತಿರುಗಿ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += ಬರೆ(" ...");
	ಆದರೆ (ಬಣ್ಣ_terminal(stdout))
		retval += ಬರೆ("\n");
	LASTCMD("ebegin");
	ಹಿಂತಿರುಗಿ retval;
}

ಸ್ಥಿರ ಶೂನ್ಯ
_eend(FILE * EINFO_RESTRICT fp, ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು, ECOLOR color, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_2)
{
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;

	ಆದರೆ (!ಸಂದೇಶ_2)
		ಹಿಂತಿರುಗಿ;

	ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = get_ಪದ_columns(fp) - (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಸಂದೇಶ_2) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	ಆದರೆ (!ಪದ) {
		ಪದ = getenv("TERM");
		ಆದರೆ (ಪದ && strcmp(ಪದ, "cons25") == 0)
			ಪದ_is_cons25 = true;
		ಇಲ್ಲದಿದ್ದರೆ
			ಪದ_is_cons25 = false;
	}
	ಆದರೆ (ಪದ_is_cons25)
		ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ--;

	ಆದರೆ (ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ > 0 && ಬಣ್ಣ_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", ಮೇಲೆ_2, tgoto(goto_ಸ್ತಂಭ, 0, ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ),
		    ecolor(ECOLOR_BRACKET), ecolor(color), ಸಂದೇಶ_2,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_ಸಾಮಾನ್ಯ));
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (ಲಂಬಸಾಲು > 0)
			ಗಾಗಿ (i = 0; i < ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ - ಲಂಬಸಾಲು; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", ಸಂದೇಶ_2);
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ EINFO_PRINTF(3, 0)
_do_eend(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd, ಪೂರ್ಣಾಂಕ retval,
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, va_list ap)
{
	ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು = 0;
	FILE *fp = stdout;
	va_list apc;

	ಆದರೆ (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		ಆದರೆ (strcmp(cmd, "ewend") == 0)
			ಲಂಬಸಾಲು = _ewarnvn(fmt, apc);
		ಇಲ್ಲದಿದ್ದರೆ
			ಲಂಬಸಾಲು = _eerrorvn(fmt, apc);
		ಲಂಬಸಾಲು += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, ಲಂಬಸಾಲು,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	ಹಿಂತಿರುಗಿ retval;
}

ಪೂರ್ಣಾಂಕ
eend(ಪೂರ್ಣಾಂಕ retval, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ಆದರೆ (is_quiet())
		ಹಿಂತಿರುಗಿ retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	ಹಿಂತಿರುಗಿ retval;
}

ಪೂರ್ಣಾಂಕ
ewend(ಪೂರ್ಣಾಂಕ retval, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ಆದರೆ (is_quiet())
		ಹಿಂತಿರುಗಿ retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	ಹಿಂತಿರುಗಿ retval;
}

ಶೂನ್ಯ
ebracket(ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು, ECOLOR color, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_2)
{
	_eend(stdout, ಲಂಬಸಾಲು, color, ಸಂದೇಶ_2);
}

ಶೂನ್ಯ
eindent(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *env = getenv("EINFO_INDENT");
	ಪೂರ್ಣಾಂಕ amount = 0;
	ಅಕ್ಷರ *ಅಂಕೆ;

	ಆದರೆ (env) {
		errno = 0;
		amount = strtoimax(env, ಖಾಲಿ, 0);
		ಆದರೆ (errno != 0)
			amount = 0;
	}
	amount += INDENT_ಅಗಲ;
	xasprintf(&ಅಂಕೆ, "%08d", amount);
	setenv("EINFO_INDENT", ಅಂಕೆ, 1);
	ಬಿಡುಗಡೆ(ಅಂಕೆ);
}

ಶೂನ್ಯ eoutdent(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *env = getenv("EINFO_INDENT");
	ಪೂರ್ಣಾಂಕ amount = 0;
	ಅಕ್ಷರ *ಅಂಕೆ = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ serrno = errno;

	ಆದರೆ (!env)
		ಹಿಂತಿರುಗಿ;
	errno = 0;
	amount = strtoimax(env, ಖಾಲಿ, 0);
	ಆದರೆ (errno != 0)
		amount = 0;
	ಇಲ್ಲದಿದ್ದರೆ
		amount -= INDENT_ಅಗಲ;
	ಆದರೆ (amount <= 0)
		unsetenv("EINFO_INDENT");
	ಇಲ್ಲದಿದ್ದರೆ {
		xasprintf(&ಅಂಕೆ, "%08d", amount);
		setenv("EINFO_INDENT", ಅಂಕೆ, 1);
		ಬಿಡುಗಡೆ(ಅಂಕೆ);
	}
	errno = serrno;
}

ಪೂರ್ಣಾಂಕ
einfovn(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಂಕ retval;
	va_list ap;

	ಆದರೆ (!fmt || !is_verbose())
		ಹಿಂತಿರುಗಿ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	ಹಿಂತಿರುಗಿ retval;
}

ಪೂರ್ಣಾಂಕ
ewarnvn(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಂಕ retval;
	va_list ap;

	ಆದರೆ (!fmt || !is_verbose())
		ಹಿಂತಿರುಗಿ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	ಹಿಂತಿರುಗಿ retval;
}

ಪೂರ್ಣಾಂಕ
einfov(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಂಕ retval;
	va_list ap;

	ಆದರೆ (!fmt || !is_verbose())
		ಹಿಂತಿರುಗಿ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += ಬರೆ("\n");
	va_end(ap);
	LASTCMD("einfov");
	ಹಿಂತಿರುಗಿ retval;
}

ಪೂರ್ಣಾಂಕ
ewarnv(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಂಕ retval;
	va_list ap;

	ಆದರೆ (!fmt || !is_verbose())
		ಹಿಂತಿರುಗಿ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += ಬರೆ("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	ಹಿಂತಿರುಗಿ retval;
}

ಪೂರ್ಣಾಂಕ
ebeginv(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಂಕ retval;
	va_list ap;

	ಆದರೆ (!fmt || !is_verbose())
		ಹಿಂತಿರುಗಿ 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += ಬರೆ(" ...");
	ಆದರೆ (ಬಣ್ಣ_terminal(stdout))
		retval += ಬರೆ("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	ಹಿಂತಿರುಗಿ retval;
}

ಪೂರ್ಣಾಂಕ
eendv(ಪೂರ್ಣಾಂಕ retval, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ಆದರೆ (!is_verbose())
		ಹಿಂತಿರುಗಿ 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	ಹಿಂತಿರುಗಿ retval;
}

ಪೂರ್ಣಾಂಕ
ewendv(ಪೂರ್ಣಾಂಕ retval, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ಆದರೆ (!is_verbose())
		ಹಿಂತಿರುಗಿ 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	ಹಿಂತಿರುಗಿ retval;
}

ಶೂನ್ಯ
eindentv(ಶೂನ್ಯ)
{
	ಆದರೆ (is_verbose())
		eindent();
}

ಶೂನ್ಯ
eoutdentv(ಶೂನ್ಯ)
{
	ಆದರೆ (is_verbose())
		eoutdent();
}
