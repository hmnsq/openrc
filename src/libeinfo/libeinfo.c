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
#define DEFAULT_COLS		 80

#define OK			"ok"
#define NOT_OK			"!!"

/* Number of spaces for an indent */
#define INDENT_WIDTH		2

/* How wide can the indent go? */
#define INDENT_MAX		40

/* Default colours */
#define GOOD                    2
#define WARN                    3
#define BAD                     1
#define HILITE                  6
#define BRACKET                 4

/* ANSI escape codes which mimic termcap */
#define AF "\033[3%dm"
#define CE "\033[K"
#define CH "\033[%dC"
#define MD "\033[1m"
#define ME "\033[m"
#define UP "\033[A"

#define _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), ಪರಿಮಾಣ(_d));
#define _ASSIGN_CAP(_v) ಕುರು {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, ಪರಿಮಾಣ(ebuffer) - (p - ebuffer)) + 1;    \
	} ಯಾವತ್ (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *_eprefix = ರಿಕ್ತ;

/* Buffers and structures to hold the final colours */
ಸ್ಥಿರ ವರ್ಣ ebuffer[100];
ಸಂರಚನಾ ecolor {
	ECOLOR color;
	ಪೂರ್ಣಾಙ್ಕ def;
	ಧ್ರುವ ವರ್ಣ *name;
};
ಸ್ಥಿರ ವರ್ಣ nullstr = '\0';

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_NORMAL,  0,       ರಿಕ್ತ      },
};
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ecolors_str[ARRAY_SIZE(ecolors)];

ಸ್ಥಿರ ವರ್ಣ *flush = ರಿಕ್ತ;
ಸ್ಥಿರ ವರ್ಣ *up = ರಿಕ್ತ;
ಸ್ಥಿರ ವರ್ಣ *goto_column = ರಿಕ್ತ;

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *term = ರಿಕ್ತ;
ಸ್ಥಿರ bool term_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ color_terms[] = {
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
	ರಿಕ್ತ
};

#ifndef HAVE_STRLCPY
ಸ್ಥಿರ size_t
strlcat(ವರ್ಣ *dst, ಧ್ರುವ ವರ್ಣ *src, size_t size)
{
	ವರ್ಣ *d = dst;
	ಧ್ರುವ ವರ್ಣ *s = src;
	size_t src_n = size;
	size_t dst_n;

	ಯಾವತ್ (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	ಯದಿ (src_n == 0)
		ಪ್ರತ್ಯಾಗಮ dst_n + strlen(src);

	ಯಾವತ್ (*s != '\0') {
		ಯದಿ (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	ಪ್ರತ್ಯಾಗಮ dst_n + (s - src);
}
#endif

ಸ್ಥಿರ bool
yesno(ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	ಯದಿ (!ಮೌಲ್ಯ) {
		errno = ENOENT;
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (strcasecmp(ಮೌಲ್ಯ, "yes") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "y") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "true") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "on") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "1") == 0)
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (strcasecmp(ಮೌಲ್ಯ, "no") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "n") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "false") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "off") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "0") != 0)
		errno = EINVAL;

	ಪ್ರತ್ಯಾಗಮ false;
}

ಸ್ಥಿರ bool
noyes(ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(ಮೌಲ್ಯ);
	ಯದಿ (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	ಪ್ರತ್ಯಾಗಮ retval;
}

ಸ್ಥಿರ bool
is_quiet(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ yesno(getenv("EINFO_QUIET"));
}

ಸ್ಥಿರ bool
is_really_quiet(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ yesno(getenv("EERROR_QUIET"));
}

ಸ್ಥಿರ bool
is_verbose(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
ಸ್ಥಿರ ವರ್ಣ *
tgoto(ಧ್ರುವ ವರ್ಣ *cap, ಪೂರ್ಣಾಙ್ಕ col, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು)
{
	ಸ್ಥಿರ ವರ್ಣ buf[20];
	ವರ್ಣ *p, *e, c, dbuf[6];
	ಪೂರ್ಣಾಙ್ಕ oncol = 0, which = ಸಾಲು, i;

	p = buf;
	e = p + ಪರಿಮಾಣ(buf);
	ಯಾವತ್ ((c = *cap++)) {
		ಯದಿ (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			ಯದಿ (p >= e) {
				errno = E2BIG;
				ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
			}
			ಅನುವರ್ತ;
		}
		ವಿಕಲ್ಪ (c) {
		ಪಕ್ಷ '3':
		ಪಕ್ಷ '2':
		ಪಕ್ಷ 'd':
			i = 0;
			ಕುರು
				dbuf[i++] = which % 10 | '0';
			ಯಾವತ್ ((which /= 10));
			ಯದಿ (c != 'd') {
				c -= '0';
				ಯದಿ (i > c) {
					errno = EINVAL;
					ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
				}
				ಯಾವತ್ (i < c)
					dbuf[i++] = '0';
			}
			ಯದಿ (p + i >= e) {
				errno = E2BIG;
				ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
			}
			ಕುರು
				*p++ = dbuf[--i];
			ಯಾವತ್ (i);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'r':
			oncol = 0;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'i':
			col++;
			ಸಾಲು++;
			which++;
			ಅನುವರ್ತ;
		ಸ್ವಭಾವ:
			errno = EINVAL;
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		oncol = 1 - oncol;
		which = oncol ? col : ಸಾಲು;
	}
	*p = '\0';
	ಪ್ರತ್ಯಾಗಮ buf;
}

ಸ್ಥಿರ bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ in_colour = -1;
	ವರ್ಣ *e, *ee, *end, *d, *p;
	ಪೂರ್ಣಾಙ್ಕ c;
	ಧ್ರುವ ವರ್ಣ *_af = ರಿಕ್ತ, *_ce = ರಿಕ್ತ, *_ch = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *_md = ರಿಕ್ತ, *_me = ರಿಕ್ತ, *_up = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *bold;
	ವರ್ಣ tmp[100];
	unsigned ಪೂರ್ಣಾಙ್ಕ i = 0;

	ಯದಿ (f && !isatty(fileno(f)))
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (noyes(getenv("EINFO_COLOR")))
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (in_colour == 0)
		ಪ್ರತ್ಯಾಗಮ false;
	ಯದಿ (in_colour == 1)
		ಪ್ರತ್ಯಾಗಮ true;

	term_is_cons25 = false;
	ಯದಿ (!term) {
		term = getenv("TERM");
		ಯದಿ (!term)
			ಪ್ರತ್ಯಾಗಮ false;
	}
	ಯದಿ (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	ಯದಿ (strstr(term, "color"))
		in_colour = 1;

	ಯಾವತ್ (color_terms[i] && in_colour != 1) {
		ಯದಿ (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	ಯದಿ (in_colour != 1) {
		in_colour = 0;
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (!_af)
		_af = AF;
	ಯದಿ (!_ce)
		_ce = CE;
	ಯದಿ (!_ch)
		_ch = CH;
	ಯದಿ (!_md)
		_md = MD;
	ಯದಿ (!_me)
		_me = ME;
	ಯದಿ (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		ಯದಿ (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			ಯದಿ ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				ಯದಿ (ee)
					ee += strlen(ecolors[i].name);

				ಯದಿ (ee && *ee == '=') {
					d = strdup(ee + 1);
					ಯದಿ (d) {
						end = strchr(d, ':');
						ಯದಿ (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						ಯದಿ (end && *++end == '0')
							bold = _me;
						ಮುಕ್ತ(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), ಪರಿಮಾಣ(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), ಪರಿಮಾಣ(tmp));
		} ಅನ್ಯಥಾ
			_GET_CAP(tmp, _me);

		ಯದಿ (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		ಅನ್ಯಥಾ
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, ಪರಿಮಾಣ(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	ಸಂರಚನಾ winsize ws;
	ವರ್ಣ *env = getenv("COLUMNS");
	ವರ್ಣ *p;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (env) {
		i = strtoimax(env, &p, 10);
		ಯದಿ (!*p)
			ಪ್ರತ್ಯಾಗಮ i;
	}

	ಯದಿ (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		ಪ್ರತ್ಯಾಗಮ ws.ws_col;

	ಪ್ರತ್ಯಾಗಮ DEFAULT_COLS;
}

ಶೂನ್ಯ
eprefix(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

ಸ್ಥಿರ ಶೂನ್ಯ EINFO_PRINTF(2, 0)
elogv(ಪೂರ್ಣಾಙ್ಕ level, ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, va_list ap)
{
	ವರ್ಣ *e = getenv("EINFO_LOG");
	va_list apc;

	ಯದಿ (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

ಶೂನ್ಯ
elog(ಪೂರ್ಣಾಙ್ಕ level, ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
_eindent(FILE * EINFO_RESTRICT stream)
{
	ವರ್ಣ *env = getenv("EINFO_INDENT");
	ಪೂರ್ಣಾಙ್ಕ amount = 0;

	ಯದಿ (env) {
		errno = 0;
		amount = strtoimax(env, ರಿಕ್ತ, 0);
		ಯದಿ (errno != 0 || amount < 0)
			amount = 0;
		ಅನ್ಯಥಾ ಯದಿ (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	ಪ್ರತ್ಯಾಗಮ fprintf(stream, "%*s", amount, "");
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (!colour_terminal(f))
		ಪ್ರತ್ಯಾಗಮ "";

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		ಯದಿ (ecolors[i].color == color)
			ಪ್ರತ್ಯಾಗಮ ecolors_str[i];
	ಪ್ರತ್ಯಾಗಮ "";
}

ಧ್ರುವ ವರ್ಣ *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	ಯದಿ (!isatty(fileno(f))) {
		f = stderr;
		ಯದಿ (!isatty(fileno(f))) {
			f = stdin;
			ಯದಿ (!isatty(fileno(f)))
				f = ರಿಕ್ತ;
		}
	}

	ಪ್ರತ್ಯಾಗಮ _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, va_list va)
{
	ಪೂರ್ಣಾಙ್ಕ retval = 0;
	ವರ್ಣ *last = getenv("EINFO_LASTCMD");
	va_list ap;

	ಯದಿ (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	ಯದಿ (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	ಯದಿ (colour_terminal(f))
		fprintf(f, "%s", flush);
	ಪ್ರತ್ಯಾಗಮ retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

ಪೂರ್ಣಾಙ್ಕ
einfon(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಙ್ಕ retval;
	va_list ap;

	ಯದಿ (!fmt || is_quiet())
		ಪ್ರತ್ಯಾಗಮ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಪೂರ್ಣಾಙ್ಕ
ewarnn(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಙ್ಕ retval;
	va_list ap;

	ಯದಿ (!fmt || is_quiet())
		ಪ್ರತ್ಯಾಗಮ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಪೂರ್ಣಾಙ್ಕ
eerrorn(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಙ್ಕ retval;
	va_list ap;

	ಯದಿ (!fmt || is_really_quiet())
		ಪ್ರತ್ಯಾಗಮ 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಪೂರ್ಣಾಙ್ಕ
einfo(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಙ್ಕ retval;
	va_list ap;

	ಯದಿ (!fmt || is_quiet())
		ಪ್ರತ್ಯಾಗಮ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಪೂರ್ಣಾಙ್ಕ
ewarn(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಙ್ಕ retval;
	va_list ap;

	ಯದಿ (!fmt || is_quiet())
		ಪ್ರತ್ಯಾಗಮ 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಶೂನ್ಯ
ewarnx(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ಯದಿ (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

ಪೂರ್ಣಾಙ್ಕ
eerror(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಙ್ಕ retval;
	va_list ap;

	ಯದಿ (!fmt || is_really_quiet())
		ಪ್ರತ್ಯಾಗಮ 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಶೂನ್ಯ
eerrorx(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ಯದಿ (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

ಪೂರ್ಣಾಙ್ಕ
ebegin(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಙ್ಕ retval;
	va_list ap;

	ಯದಿ (!fmt || is_quiet())
		ಪ್ರತ್ಯಾಗಮ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	ಯದಿ (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಸ್ಥಿರ ಶೂನ್ಯ
_eend(FILE * EINFO_RESTRICT fp, ಪೂರ್ಣಾಙ್ಕ col, ECOLOR color, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ cols;

	ಯದಿ (!ಸಂದೇಶ)
		ಪ್ರತ್ಯಾಗಮ;

	cols = get_term_columns(fp) - (strlen(ಸಂದೇಶ) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	ಯದಿ (!term) {
		term = getenv("TERM");
		ಯದಿ (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		ಅನ್ಯಥಾ
			term_is_cons25 = false;
	}
	ಯದಿ (term_is_cons25)
		cols--;

	ಯದಿ (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), ಸಂದೇಶ,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} ಅನ್ಯಥಾ {
		ಯದಿ (col > 0)
			ಪ್ರತಿ (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", ಸಂದೇಶ);
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ EINFO_PRINTF(3, 0)
_do_eend(ಧ್ರುವ ವರ್ಣ *cmd, ಪೂರ್ಣಾಙ್ಕ retval,
    ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, va_list ap)
{
	ಪೂರ್ಣಾಙ್ಕ col = 0;
	FILE *fp = stdout;
	va_list apc;

	ಯದಿ (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		ಯದಿ (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		ಅನ್ಯಥಾ
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಪೂರ್ಣಾಙ್ಕ
eend(ಪೂರ್ಣಾಙ್ಕ retval, ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ಯದಿ (is_quiet())
		ಪ್ರತ್ಯಾಗಮ retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಪೂರ್ಣಾಙ್ಕ
ewend(ಪೂರ್ಣಾಙ್ಕ retval, ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ಯದಿ (is_quiet())
		ಪ್ರತ್ಯಾಗಮ retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಶೂನ್ಯ
ebracket(ಪೂರ್ಣಾಙ್ಕ col, ECOLOR color, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ)
{
	_eend(stdout, col, color, ಸಂದೇಶ);
}

ಶೂನ್ಯ
eindent(ಶೂನ್ಯ)
{
	ವರ್ಣ *env = getenv("EINFO_INDENT");
	ಪೂರ್ಣಾಙ್ಕ amount = 0;
	ವರ್ಣ *ಅಂಕೆ;

	ಯದಿ (env) {
		errno = 0;
		amount = strtoimax(env, ರಿಕ್ತ, 0);
		ಯದಿ (errno != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
	xasprintf(&ಅಂಕೆ, "%08d", amount);
	setenv("EINFO_INDENT", ಅಂಕೆ, 1);
	ಮುಕ್ತ(ಅಂಕೆ);
}

ಶೂನ್ಯ eoutdent(ಶೂನ್ಯ)
{
	ವರ್ಣ *env = getenv("EINFO_INDENT");
	ಪೂರ್ಣಾಙ್ಕ amount = 0;
	ವರ್ಣ *ಅಂಕೆ = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ serrno = errno;

	ಯದಿ (!env)
		ಪ್ರತ್ಯಾಗಮ;
	errno = 0;
	amount = strtoimax(env, ರಿಕ್ತ, 0);
	ಯದಿ (errno != 0)
		amount = 0;
	ಅನ್ಯಥಾ
		amount -= INDENT_WIDTH;
	ಯದಿ (amount <= 0)
		unsetenv("EINFO_INDENT");
	ಅನ್ಯಥಾ {
		xasprintf(&ಅಂಕೆ, "%08d", amount);
		setenv("EINFO_INDENT", ಅಂಕೆ, 1);
		ಮುಕ್ತ(ಅಂಕೆ);
	}
	errno = serrno;
}

ಪೂರ್ಣಾಙ್ಕ
einfovn(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಙ್ಕ retval;
	va_list ap;

	ಯದಿ (!fmt || !is_verbose())
		ಪ್ರತ್ಯಾಗಮ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಪೂರ್ಣಾಙ್ಕ
ewarnvn(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಙ್ಕ retval;
	va_list ap;

	ಯದಿ (!fmt || !is_verbose())
		ಪ್ರತ್ಯಾಗಮ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಪೂರ್ಣಾಙ್ಕ
einfov(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಙ್ಕ retval;
	va_list ap;

	ಯದಿ (!fmt || !is_verbose())
		ಪ್ರತ್ಯಾಗಮ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಪೂರ್ಣಾಙ್ಕ
ewarnv(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಙ್ಕ retval;
	va_list ap;

	ಯದಿ (!fmt || !is_verbose())
		ಪ್ರತ್ಯಾಗಮ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಪೂರ್ಣಾಙ್ಕ
ebeginv(ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	ಪೂರ್ಣಾಙ್ಕ retval;
	va_list ap;

	ಯದಿ (!fmt || !is_verbose())
		ಪ್ರತ್ಯಾಗಮ 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	ಯದಿ (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಪೂರ್ಣಾಙ್ಕ
eendv(ಪೂರ್ಣಾಙ್ಕ retval, ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ಯದಿ (!is_verbose())
		ಪ್ರತ್ಯಾಗಮ 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಪೂರ್ಣಾಙ್ಕ
ewendv(ಪೂರ್ಣಾಙ್ಕ retval, ಧ್ರುವ ವರ್ಣ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ಯದಿ (!is_verbose())
		ಪ್ರತ್ಯಾಗಮ 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಶೂನ್ಯ
eindentv(ಶೂನ್ಯ)
{
	ಯದಿ (is_verbose())
		eindent();
}

ಶೂನ್ಯ
eoutdentv(ಶೂನ್ಯ)
{
	ಯದಿ (is_verbose())
		eoutdent();
}
