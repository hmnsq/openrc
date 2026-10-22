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

#include <sys/ioctl.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <unistd.h>

#include "einfo.h"
#include "helpers.h"

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

#define _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), parimaa_na(_d));
#define _ASSIGN_CAP(_v) do {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, parimaa_na(ebuffer) - (p - ebuffer)) + 1;    \
	} ขณะ (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
static const var_na *_eprefix = NULL;

/* Buffers and structures to hold the final colours */
static var_na ebuffer[100];
sa_nracanaa ecolor {
	ECOLOR color;
	จำนวนเต็ม def;
	const var_na *name;
};
static var_na nullstr = '\0';

static const sa_nracanaa ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_NORMAL,  0,       NULL      },
};
static const var_na *ecolors_str[ARRAY_SIZE(ecolors)];

static var_na *flush = NULL;
static var_na *up = NULL;
static var_na *goto_column = NULL;

static const var_na *term = NULL;
static bool term_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
static const var_na *const color_terms[] = {
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
	NULL
};

#ifndef HAVE_STRLCPY
static size_t
strlcat(var_na *dst, const var_na *src, size_t size)
{
	var_na *d = dst;
	const var_na *s = src;
	size_t src_n = size;
	size_t dst_n;

	ขณะ (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	ถ้า (src_n == 0)
		คืน dst_n + strlen(src);

	ขณะ (*s != '\0') {
		ถ้า (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	คืน dst_n + (s - src);
}
#endif

static bool
yesno(const var_na *value)
{
	ถ้า (!value) {
		errno = ENOENT;
		คืน false;
	}

	ถ้า (strcasecmp(value, "yes") == 0 ||
	    strcasecmp(value, "y") == 0 ||
	    strcasecmp(value, "true") == 0 ||
	    strcasecmp(value, "on") == 0 ||
	    strcasecmp(value, "1") == 0)
		คืน true;

	ถ้า (strcasecmp(value, "no") != 0 &&
	    strcasecmp(value, "n") != 0 &&
	    strcasecmp(value, "false") != 0 &&
	    strcasecmp(value, "off") != 0 &&
	    strcasecmp(value, "0") != 0)
		errno = EINVAL;

	คืน false;
}

static bool
noyes(const var_na *value)
{
	จำนวนเต็ม serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(value);
	ถ้า (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	คืน retval;
}

static bool
is_quiet(void)
{
	คืน yesno(getenv("EINFO_QUIET"));
}

static bool
is_really_quiet(void)
{
	คืน yesno(getenv("EERROR_QUIET"));
}

static bool
is_verbose(void)
{
	คืน yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
static var_na *
tgoto(const var_na *cap, จำนวนเต็ม col, จำนวนเต็ม line)
{
	static var_na buf[20];
	var_na *p, *e, c, dbuf[6];
	จำนวนเต็ม oncol = 0, which = line, i;

	p = buf;
	e = p + parimaa_na(buf);
	ขณะ ((c = *cap++)) {
		ถ้า (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			ถ้า (p >= e) {
				errno = E2BIG;
				คืน NULL;
			}
			continue;
		}
		switch (c) {
		pak_sa '3':
		pak_sa '2':
		pak_sa 'd':
			i = 0;
			do
				dbuf[i++] = which % 10 | '0';
			ขณะ ((which /= 10));
			ถ้า (c != 'd') {
				c -= '0';
				ถ้า (i > c) {
					errno = EINVAL;
					คืน NULL;
				}
				ขณะ (i < c)
					dbuf[i++] = '0';
			}
			ถ้า (p + i >= e) {
				errno = E2BIG;
				คืน NULL;
			}
			do
				*p++ = dbuf[--i];
			ขณะ (i);
			bha_nga;
		pak_sa 'r':
			oncol = 0;
			bha_nga;
		pak_sa 'i':
			col++;
			line++;
			which++;
			continue;
		default:
			errno = EINVAL;
			คืน NULL;
		}

		oncol = 1 - oncol;
		which = oncol ? col : line;
	}
	*p = '\0';
	คืน buf;
}

static bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	static จำนวนเต็ม in_colour = -1;
	var_na *e, *ee, *end, *d, *p;
	จำนวนเต็ม c;
	const var_na *_af = NULL, *_ce = NULL, *_ch = NULL;
	const var_na *_md = NULL, *_me = NULL, *_up = NULL;
	const var_na *bold;
	var_na tmp[100];
	unsigned จำนวนเต็ม i = 0;

	ถ้า (f && !isatty(fileno(f)))
		คืน false;

	ถ้า (noyes(getenv("EINFO_COLOR")))
		คืน false;

	ถ้า (in_colour == 0)
		คืน false;
	ถ้า (in_colour == 1)
		คืน true;

	term_is_cons25 = false;
	ถ้า (!term) {
		term = getenv("TERM");
		ถ้า (!term)
			คืน false;
	}
	ถ้า (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	ถ้า (strstr(term, "color"))
		in_colour = 1;

	ขณะ (color_terms[i] && in_colour != 1) {
		ถ้า (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	ถ้า (in_colour != 1) {
		in_colour = 0;
		คืน false;
	}

	ถ้า (!_af)
		_af = AF;
	ถ้า (!_ce)
		_ce = CE;
	ถ้า (!_ch)
		_ch = CH;
	ถ้า (!_md)
		_md = MD;
	ถ้า (!_me)
		_me = ME;
	ถ้า (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	สำหรับ (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		ถ้า (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			ถ้า ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				ถ้า (ee)
					ee += strlen(ecolors[i].name);

				ถ้า (ee && *ee == '=') {
					d = strdup(ee + 1);
					ถ้า (d) {
						end = strchr(d, ':');
						ถ้า (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						ถ้า (end && *++end == '0')
							bold = _me;
						free(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), parimaa_na(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), parimaa_na(tmp));
		} มิฉะนั้น
			_GET_CAP(tmp, _me);

		ถ้า (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		มิฉะนั้น
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, parimaa_na(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	คืน true;
}

static จำนวนเต็ม
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	sa_nracanaa winsize ws;
	var_na *env = getenv("COLUMNS");
	var_na *p;
	จำนวนเต็ม i;

	ถ้า (env) {
		i = strtoimax(env, &p, 10);
		ถ้า (!*p)
			คืน i;
	}

	ถ้า (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		คืน ws.ws_col;

	คืน DEFAULT_COLS;
}

void
eprefix(const var_na *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

static void EINFO_PRINTF(2, 0)
elogv(จำนวนเต็ม level, const var_na *EINFO_RESTRICT fmt, va_list ap)
{
	var_na *e = getenv("EINFO_LOG");
	va_list apc;

	ถ้า (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

void
elog(จำนวนเต็ม level, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

static จำนวนเต็ม
_eindent(FILE * EINFO_RESTRICT stream)
{
	var_na *env = getenv("EINFO_INDENT");
	จำนวนเต็ม amount = 0;

	ถ้า (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		ถ้า (errno != 0 || amount < 0)
			amount = 0;
		มิฉะนั้น ถ้า (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	คืน fprintf(stream, "%*s", amount, "");
}

static const var_na *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned จำนวนเต็ม i;

	ถ้า (!colour_terminal(f))
		คืน "";

	สำหรับ (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		ถ้า (ecolors[i].color == color)
			คืน ecolors_str[i];
	คืน "";
}

const var_na *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	ถ้า (!isatty(fileno(f))) {
		f = stderr;
		ถ้า (!isatty(fileno(f))) {
			f = stdin;
			ถ้า (!isatty(fileno(f)))
				f = NULL;
		}
	}

	คืน _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

static จำนวนเต็ม EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, const var_na *EINFO_RESTRICT fmt, va_list va)
{
	จำนวนเต็ม retval = 0;
	var_na *last = getenv("EINFO_LASTCMD");
	va_list ap;

	ถ้า (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	ถ้า (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	ถ้า (colour_terminal(f))
		fprintf(f, "%s", flush);
	คืน retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

จำนวนเต็ม
einfon(const var_na *EINFO_RESTRICT fmt, ...)
{
	จำนวนเต็ม retval;
	va_list ap;

	ถ้า (!fmt || is_quiet())
		คืน 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	คืน retval;
}

จำนวนเต็ม
ewarnn(const var_na *EINFO_RESTRICT fmt, ...)
{
	จำนวนเต็ม retval;
	va_list ap;

	ถ้า (!fmt || is_quiet())
		คืน 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	คืน retval;
}

จำนวนเต็ม
eerrorn(const var_na *EINFO_RESTRICT fmt, ...)
{
	จำนวนเต็ม retval;
	va_list ap;

	ถ้า (!fmt || is_really_quiet())
		คืน 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	คืน retval;
}

จำนวนเต็ม
einfo(const var_na *EINFO_RESTRICT fmt, ...)
{
	จำนวนเต็ม retval;
	va_list ap;

	ถ้า (!fmt || is_quiet())
		คืน 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	คืน retval;
}

จำนวนเต็ม
ewarn(const var_na *EINFO_RESTRICT fmt, ...)
{
	จำนวนเต็ม retval;
	va_list ap;

	ถ้า (!fmt || is_quiet())
		คืน 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	คืน retval;
}

void
ewarnx(const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ถ้า (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

จำนวนเต็ม
eerror(const var_na *EINFO_RESTRICT fmt, ...)
{
	จำนวนเต็ม retval;
	va_list ap;

	ถ้า (!fmt || is_really_quiet())
		คืน 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	คืน retval;
}

void
eerrorx(const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ถ้า (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

จำนวนเต็ม
ebegin(const var_na *EINFO_RESTRICT fmt, ...)
{
	จำนวนเต็ม retval;
	va_list ap;

	ถ้า (!fmt || is_quiet())
		คืน 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	ถ้า (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	คืน retval;
}

static void
_eend(FILE * EINFO_RESTRICT fp, จำนวนเต็ม col, ECOLOR color, const var_na *msg)
{
	จำนวนเต็ม i;
	จำนวนเต็ม cols;

	ถ้า (!msg)
		คืน;

	cols = get_term_columns(fp) - (strlen(msg) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	ถ้า (!term) {
		term = getenv("TERM");
		ถ้า (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		มิฉะนั้น
			term_is_cons25 = false;
	}
	ถ้า (term_is_cons25)
		cols--;

	ถ้า (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), msg,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} มิฉะนั้น {
		ถ้า (col > 0)
			สำหรับ (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", msg);
	}
}

static จำนวนเต็ม EINFO_PRINTF(3, 0)
_do_eend(const var_na *cmd, จำนวนเต็ม retval,
    const var_na *EINFO_RESTRICT fmt, va_list ap)
{
	จำนวนเต็ม col = 0;
	FILE *fp = stdout;
	va_list apc;

	ถ้า (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		ถ้า (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		มิฉะนั้น
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	คืน retval;
}

จำนวนเต็ม
eend(จำนวนเต็ม retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ถ้า (is_quiet())
		คืน retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	คืน retval;
}

จำนวนเต็ม
ewend(จำนวนเต็ม retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ถ้า (is_quiet())
		คืน retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	คืน retval;
}

void
ebracket(จำนวนเต็ม col, ECOLOR color, const var_na *msg)
{
	_eend(stdout, col, color, msg);
}

void
eindent(void)
{
	var_na *env = getenv("EINFO_INDENT");
	จำนวนเต็ม amount = 0;
	var_na *num;

	ถ้า (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		ถ้า (errno != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
	xasprintf(&num, "%08d", amount);
	setenv("EINFO_INDENT", num, 1);
	free(num);
}

void eoutdent(void)
{
	var_na *env = getenv("EINFO_INDENT");
	จำนวนเต็ม amount = 0;
	var_na *num = NULL;
	จำนวนเต็ม serrno = errno;

	ถ้า (!env)
		คืน;
	errno = 0;
	amount = strtoimax(env, NULL, 0);
	ถ้า (errno != 0)
		amount = 0;
	มิฉะนั้น
		amount -= INDENT_WIDTH;
	ถ้า (amount <= 0)
		unsetenv("EINFO_INDENT");
	มิฉะนั้น {
		xasprintf(&num, "%08d", amount);
		setenv("EINFO_INDENT", num, 1);
		free(num);
	}
	errno = serrno;
}

จำนวนเต็ม
einfovn(const var_na *EINFO_RESTRICT fmt, ...)
{
	จำนวนเต็ม retval;
	va_list ap;

	ถ้า (!fmt || !is_verbose())
		คืน 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	คืน retval;
}

จำนวนเต็ม
ewarnvn(const var_na *EINFO_RESTRICT fmt, ...)
{
	จำนวนเต็ม retval;
	va_list ap;

	ถ้า (!fmt || !is_verbose())
		คืน 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	คืน retval;
}

จำนวนเต็ม
einfov(const var_na *EINFO_RESTRICT fmt, ...)
{
	จำนวนเต็ม retval;
	va_list ap;

	ถ้า (!fmt || !is_verbose())
		คืน 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	คืน retval;
}

จำนวนเต็ม
ewarnv(const var_na *EINFO_RESTRICT fmt, ...)
{
	จำนวนเต็ม retval;
	va_list ap;

	ถ้า (!fmt || !is_verbose())
		คืน 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	คืน retval;
}

จำนวนเต็ม
ebeginv(const var_na *EINFO_RESTRICT fmt, ...)
{
	จำนวนเต็ม retval;
	va_list ap;

	ถ้า (!fmt || !is_verbose())
		คืน 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	ถ้า (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	คืน retval;
}

จำนวนเต็ม
eendv(จำนวนเต็ม retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ถ้า (!is_verbose())
		คืน 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	คืน retval;
}

จำนวนเต็ม
ewendv(จำนวนเต็ม retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ถ้า (!is_verbose())
		คืน 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	คืน retval;
}

void
eindentv(void)
{
	ถ้า (is_verbose())
		eindent();
}

void
eoutdentv(void)
{
	ถ้า (is_verbose())
		eoutdent();
}
