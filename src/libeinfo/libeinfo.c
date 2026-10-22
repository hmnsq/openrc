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

#അന്തര്ഭാവ <sys/ioctl.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <inttypes.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <syslog.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "helpers.h"

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

#define _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), വലുപ്പം(_d));
#define _ASSIGN_CAP(_v) ചെയ്യുക {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, വലുപ്പം(ebuffer) - (p - ebuffer)) + 1;    \
	} ആകുമ്പോൾ (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
സ്ഥിതം സ്ഥിരം അക്ഷരം *_eprefix = ശൂന്യ;

/* Buffers and structures to hold the final colours */
സ്ഥിതം അക്ഷരം ebuffer[100];
ഘടന ecolor {
	ECOLOR color;
	പൂർണ്ണസംഖ്യ def;
	സ്ഥിരം അക്ഷരം *name;
};
സ്ഥിതം അക്ഷരം nullstr = '\0';

സ്ഥിതം സ്ഥിരം ഘടന ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_NORMAL,  0,       ശൂന്യ      },
};
സ്ഥിതം സ്ഥിരം അക്ഷരം *ecolors_str[ARRAY_SIZE(ecolors)];

സ്ഥിതം അക്ഷരം *flush = ശൂന്യ;
സ്ഥിതം അക്ഷരം *up = ശൂന്യ;
സ്ഥിതം അക്ഷരം *goto_column = ശൂന്യ;

സ്ഥിതം സ്ഥിരം അക്ഷരം *term = ശൂന്യ;
സ്ഥിതം bool term_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം color_terms[] = {
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
	ശൂന്യ
};

#ifndef HAVE_STRLCPY
സ്ഥിതം size_t
strlcat(അക്ഷരം *dst, സ്ഥിരം അക്ഷരം *src, size_t size)
{
	അക്ഷരം *d = dst;
	സ്ഥിരം അക്ഷരം *s = src;
	size_t src_n = size;
	size_t dst_n;

	ആകുമ്പോൾ (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	എങ്കിൽ (src_n == 0)
		തിരികെ dst_n + strlen(src);

	ആകുമ്പോൾ (*s != '\0') {
		എങ്കിൽ (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	തിരികെ dst_n + (s - src);
}
#endif

സ്ഥിതം bool
yesno(സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	എങ്കിൽ (!മൂല്യം) {
		errno = ENOENT;
		തിരികെ false;
	}

	എങ്കിൽ (strcasecmp(മൂല്യം, "yes") == 0 ||
	    strcasecmp(മൂല്യം, "y") == 0 ||
	    strcasecmp(മൂല്യം, "true") == 0 ||
	    strcasecmp(മൂല്യം, "on") == 0 ||
	    strcasecmp(മൂല്യം, "1") == 0)
		തിരികെ true;

	എങ്കിൽ (strcasecmp(മൂല്യം, "no") != 0 &&
	    strcasecmp(മൂല്യം, "n") != 0 &&
	    strcasecmp(മൂല്യം, "false") != 0 &&
	    strcasecmp(മൂല്യം, "off") != 0 &&
	    strcasecmp(മൂല്യം, "0") != 0)
		errno = EINVAL;

	തിരികെ false;
}

സ്ഥിതം bool
noyes(സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	പൂർണ്ണസംഖ്യ serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(മൂല്യം);
	എങ്കിൽ (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	തിരികെ retval;
}

സ്ഥിതം bool
is_quiet(ശൂന്യം)
{
	തിരികെ yesno(getenv("EINFO_QUIET"));
}

സ്ഥിതം bool
is_really_quiet(ശൂന്യം)
{
	തിരികെ yesno(getenv("EERROR_QUIET"));
}

സ്ഥിതം bool
is_verbose(ശൂന്യം)
{
	തിരികെ yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
സ്ഥിതം അക്ഷരം *
tgoto(സ്ഥിരം അക്ഷരം *cap, പൂർണ്ണസംഖ്യ col, പൂർണ്ണസംഖ്യ വരി)
{
	സ്ഥിതം അക്ഷരം buf[20];
	അക്ഷരം *p, *e, c, dbuf[6];
	പൂർണ്ണസംഖ്യ oncol = 0, which = വരി, i;

	p = buf;
	e = p + വലുപ്പം(buf);
	ആകുമ്പോൾ ((c = *cap++)) {
		എങ്കിൽ (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			എങ്കിൽ (p >= e) {
				errno = E2BIG;
				തിരികെ ശൂന്യ;
			}
			തുടരുക;
		}
		തിരഞ്ഞെടുപ്പ് (c) {
		അവസ്ഥ '3':
		അവസ്ഥ '2':
		അവസ്ഥ 'd':
			i = 0;
			ചെയ്യുക
				dbuf[i++] = which % 10 | '0';
			ആകുമ്പോൾ ((which /= 10));
			എങ്കിൽ (c != 'd') {
				c -= '0';
				എങ്കിൽ (i > c) {
					errno = EINVAL;
					തിരികെ ശൂന്യ;
				}
				ആകുമ്പോൾ (i < c)
					dbuf[i++] = '0';
			}
			എങ്കിൽ (p + i >= e) {
				errno = E2BIG;
				തിരികെ ശൂന്യ;
			}
			ചെയ്യുക
				*p++ = dbuf[--i];
			ആകുമ്പോൾ (i);
			നിർത്തുക;
		അവസ്ഥ 'r':
			oncol = 0;
			നിർത്തുക;
		അവസ്ഥ 'i':
			col++;
			വരി++;
			which++;
			തുടരുക;
		സ്ഥിരസ്ഥിതി:
			errno = EINVAL;
			തിരികെ ശൂന്യ;
		}

		oncol = 1 - oncol;
		which = oncol ? col : വരി;
	}
	*p = '\0';
	തിരികെ buf;
}

സ്ഥിതം bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	സ്ഥിതം പൂർണ്ണസംഖ്യ in_colour = -1;
	അക്ഷരം *e, *ee, *end, *d, *p;
	പൂർണ്ണസംഖ്യ c;
	സ്ഥിരം അക്ഷരം *_af = ശൂന്യ, *_ce = ശൂന്യ, *_ch = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *_md = ശൂന്യ, *_me = ശൂന്യ, *_up = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *bold;
	അക്ഷരം tmp[100];
	unsigned പൂർണ്ണസംഖ്യ i = 0;

	എങ്കിൽ (f && !isatty(fileno(f)))
		തിരികെ false;

	എങ്കിൽ (noyes(getenv("EINFO_COLOR")))
		തിരികെ false;

	എങ്കിൽ (in_colour == 0)
		തിരികെ false;
	എങ്കിൽ (in_colour == 1)
		തിരികെ true;

	term_is_cons25 = false;
	എങ്കിൽ (!term) {
		term = getenv("TERM");
		എങ്കിൽ (!term)
			തിരികെ false;
	}
	എങ്കിൽ (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	എങ്കിൽ (strstr(term, "color"))
		in_colour = 1;

	ആകുമ്പോൾ (color_terms[i] && in_colour != 1) {
		എങ്കിൽ (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	എങ്കിൽ (in_colour != 1) {
		in_colour = 0;
		തിരികെ false;
	}

	എങ്കിൽ (!_af)
		_af = AF;
	എങ്കിൽ (!_ce)
		_ce = CE;
	എങ്കിൽ (!_ch)
		_ch = CH;
	എങ്കിൽ (!_md)
		_md = MD;
	എങ്കിൽ (!_me)
		_me = ME;
	എങ്കിൽ (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	വേണ്ടി (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		എങ്കിൽ (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			എങ്കിൽ ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				എങ്കിൽ (ee)
					ee += strlen(ecolors[i].name);

				എങ്കിൽ (ee && *ee == '=') {
					d = strdup(ee + 1);
					എങ്കിൽ (d) {
						end = strchr(d, ':');
						എങ്കിൽ (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						എങ്കിൽ (end && *++end == '0')
							bold = _me;
						സ്വതന്ത്രം(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), വലുപ്പം(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), വലുപ്പം(tmp));
		} അല്ലെങ്കിൽ
			_GET_CAP(tmp, _me);

		എങ്കിൽ (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		അല്ലെങ്കിൽ
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, വലുപ്പം(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	തിരികെ true;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	ഘടന winsize ws;
	അക്ഷരം *env = getenv("COLUMNS");
	അക്ഷരം *p;
	പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (env) {
		i = strtoimax(env, &p, 10);
		എങ്കിൽ (!*p)
			തിരികെ i;
	}

	എങ്കിൽ (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		തിരികെ ws.ws_col;

	തിരികെ DEFAULT_COLS;
}

ശൂന്യം
eprefix(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

സ്ഥിതം ശൂന്യം EINFO_PRINTF(2, 0)
elogv(പൂർണ്ണസംഖ്യ level, സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, va_list ap)
{
	അക്ഷരം *e = getenv("EINFO_LOG");
	va_list apc;

	എങ്കിൽ (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

ശൂന്യം
elog(പൂർണ്ണസംഖ്യ level, സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
_eindent(FILE * EINFO_RESTRICT stream)
{
	അക്ഷരം *env = getenv("EINFO_INDENT");
	പൂർണ്ണസംഖ്യ amount = 0;

	എങ്കിൽ (env) {
		errno = 0;
		amount = strtoimax(env, ശൂന്യ, 0);
		എങ്കിൽ (errno != 0 || amount < 0)
			amount = 0;
		അല്ലെങ്കിൽ എങ്കിൽ (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	തിരികെ fprintf(stream, "%*s", amount, "");
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (!colour_terminal(f))
		തിരികെ "";

	വേണ്ടി (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		എങ്കിൽ (ecolors[i].color == color)
			തിരികെ ecolors_str[i];
	തിരികെ "";
}

സ്ഥിരം അക്ഷരം *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	എങ്കിൽ (!isatty(fileno(f))) {
		f = stderr;
		എങ്കിൽ (!isatty(fileno(f))) {
			f = stdin;
			എങ്കിൽ (!isatty(fileno(f)))
				f = ശൂന്യ;
		}
	}

	തിരികെ _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

സ്ഥിതം പൂർണ്ണസംഖ്യ EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, va_list va)
{
	പൂർണ്ണസംഖ്യ retval = 0;
	അക്ഷരം *last = getenv("EINFO_LASTCMD");
	va_list ap;

	എങ്കിൽ (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	എങ്കിൽ (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	എങ്കിൽ (colour_terminal(f))
		fprintf(f, "%s", flush);
	തിരികെ retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

പൂർണ്ണസംഖ്യ
einfon(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	പൂർണ്ണസംഖ്യ retval;
	va_list ap;

	എങ്കിൽ (!fmt || is_quiet())
		തിരികെ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	തിരികെ retval;
}

പൂർണ്ണസംഖ്യ
ewarnn(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	പൂർണ്ണസംഖ്യ retval;
	va_list ap;

	എങ്കിൽ (!fmt || is_quiet())
		തിരികെ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	തിരികെ retval;
}

പൂർണ്ണസംഖ്യ
eerrorn(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	പൂർണ്ണസംഖ്യ retval;
	va_list ap;

	എങ്കിൽ (!fmt || is_really_quiet())
		തിരികെ 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	തിരികെ retval;
}

പൂർണ്ണസംഖ്യ
einfo(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	പൂർണ്ണസംഖ്യ retval;
	va_list ap;

	എങ്കിൽ (!fmt || is_quiet())
		തിരികെ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	തിരികെ retval;
}

പൂർണ്ണസംഖ്യ
ewarn(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	പൂർണ്ണസംഖ്യ retval;
	va_list ap;

	എങ്കിൽ (!fmt || is_quiet())
		തിരികെ 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	തിരികെ retval;
}

ശൂന്യം
ewarnx(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	എങ്കിൽ (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

പൂർണ്ണസംഖ്യ
eerror(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	പൂർണ്ണസംഖ്യ retval;
	va_list ap;

	എങ്കിൽ (!fmt || is_really_quiet())
		തിരികെ 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	തിരികെ retval;
}

ശൂന്യം
eerrorx(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	എങ്കിൽ (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

പൂർണ്ണസംഖ്യ
ebegin(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	പൂർണ്ണസംഖ്യ retval;
	va_list ap;

	എങ്കിൽ (!fmt || is_quiet())
		തിരികെ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	എങ്കിൽ (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	തിരികെ retval;
}

സ്ഥിതം ശൂന്യം
_eend(FILE * EINFO_RESTRICT fp, പൂർണ്ണസംഖ്യ col, ECOLOR color, സ്ഥിരം അക്ഷരം *സന്ദേശം)
{
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ cols;

	എങ്കിൽ (!സന്ദേശം)
		തിരികെ;

	cols = get_term_columns(fp) - (strlen(സന്ദേശം) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	എങ്കിൽ (!term) {
		term = getenv("TERM");
		എങ്കിൽ (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		അല്ലെങ്കിൽ
			term_is_cons25 = false;
	}
	എങ്കിൽ (term_is_cons25)
		cols--;

	എങ്കിൽ (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), സന്ദേശം,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (col > 0)
			വേണ്ടി (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", സന്ദേശം);
	}
}

സ്ഥിതം പൂർണ്ണസംഖ്യ EINFO_PRINTF(3, 0)
_do_eend(സ്ഥിരം അക്ഷരം *cmd, പൂർണ്ണസംഖ്യ retval,
    സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, va_list ap)
{
	പൂർണ്ണസംഖ്യ col = 0;
	FILE *fp = stdout;
	va_list apc;

	എങ്കിൽ (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		എങ്കിൽ (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		അല്ലെങ്കിൽ
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	തിരികെ retval;
}

പൂർണ്ണസംഖ്യ
eend(പൂർണ്ണസംഖ്യ retval, സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	എങ്കിൽ (is_quiet())
		തിരികെ retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	തിരികെ retval;
}

പൂർണ്ണസംഖ്യ
ewend(പൂർണ്ണസംഖ്യ retval, സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	എങ്കിൽ (is_quiet())
		തിരികെ retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	തിരികെ retval;
}

ശൂന്യം
ebracket(പൂർണ്ണസംഖ്യ col, ECOLOR color, സ്ഥിരം അക്ഷരം *സന്ദേശം)
{
	_eend(stdout, col, color, സന്ദേശം);
}

ശൂന്യം
eindent(ശൂന്യം)
{
	അക്ഷരം *env = getenv("EINFO_INDENT");
	പൂർണ്ണസംഖ്യ amount = 0;
	അക്ഷരം *അക്കം;

	എങ്കിൽ (env) {
		errno = 0;
		amount = strtoimax(env, ശൂന്യ, 0);
		എങ്കിൽ (errno != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
	xasprintf(&അക്കം, "%08d", amount);
	setenv("EINFO_INDENT", അക്കം, 1);
	സ്വതന്ത്രം(അക്കം);
}

ശൂന്യം eoutdent(ശൂന്യം)
{
	അക്ഷരം *env = getenv("EINFO_INDENT");
	പൂർണ്ണസംഖ്യ amount = 0;
	അക്ഷരം *അക്കം = ശൂന്യ;
	പൂർണ്ണസംഖ്യ serrno = errno;

	എങ്കിൽ (!env)
		തിരികെ;
	errno = 0;
	amount = strtoimax(env, ശൂന്യ, 0);
	എങ്കിൽ (errno != 0)
		amount = 0;
	അല്ലെങ്കിൽ
		amount -= INDENT_WIDTH;
	എങ്കിൽ (amount <= 0)
		unsetenv("EINFO_INDENT");
	അല്ലെങ്കിൽ {
		xasprintf(&അക്കം, "%08d", amount);
		setenv("EINFO_INDENT", അക്കം, 1);
		സ്വതന്ത്രം(അക്കം);
	}
	errno = serrno;
}

പൂർണ്ണസംഖ്യ
einfovn(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	പൂർണ്ണസംഖ്യ retval;
	va_list ap;

	എങ്കിൽ (!fmt || !is_verbose())
		തിരികെ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	തിരികെ retval;
}

പൂർണ്ണസംഖ്യ
ewarnvn(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	പൂർണ്ണസംഖ്യ retval;
	va_list ap;

	എങ്കിൽ (!fmt || !is_verbose())
		തിരികെ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	തിരികെ retval;
}

പൂർണ്ണസംഖ്യ
einfov(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	പൂർണ്ണസംഖ്യ retval;
	va_list ap;

	എങ്കിൽ (!fmt || !is_verbose())
		തിരികെ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	തിരികെ retval;
}

പൂർണ്ണസംഖ്യ
ewarnv(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	പൂർണ്ണസംഖ്യ retval;
	va_list ap;

	എങ്കിൽ (!fmt || !is_verbose())
		തിരികെ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	തിരികെ retval;
}

പൂർണ്ണസംഖ്യ
ebeginv(സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	പൂർണ്ണസംഖ്യ retval;
	va_list ap;

	എങ്കിൽ (!fmt || !is_verbose())
		തിരികെ 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	എങ്കിൽ (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	തിരികെ retval;
}

പൂർണ്ണസംഖ്യ
eendv(പൂർണ്ണസംഖ്യ retval, സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	എങ്കിൽ (!is_verbose())
		തിരികെ 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	തിരികെ retval;
}

പൂർണ്ണസംഖ്യ
ewendv(പൂർണ്ണസംഖ്യ retval, സ്ഥിരം അക്ഷരം *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	എങ്കിൽ (!is_verbose())
		തിരികെ 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	തിരികെ retval;
}

ശൂന്യം
eindentv(ശൂന്യം)
{
	എങ്കിൽ (is_verbose())
		eindent();
}

ശൂന്യം
eoutdentv(ശൂന്യം)
{
	എങ്കിൽ (is_verbose())
		eoutdent();
}
