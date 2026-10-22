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
	} ᨀᨛ (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
static const var_na *_eprefix = NULL;

/* Buffers and structures to hold the final colours */
static var_na ebuffer[100];
sa_nracanaa ecolor {
	ECOLOR color;
	puur_naa_nka def;
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

	ᨀᨛ (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	ᨀᨘ (src_n == 0)
		ᨀᨘᨒ dst_n + strlen(src);

	ᨀᨛ (*s != '\0') {
		ᨀᨘ (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	ᨀᨘᨒ dst_n + (s - src);
}
#endif

static bool
yesno(const var_na *value)
{
	ᨀᨘ (!value) {
		errno = ENOENT;
		ᨀᨘᨒ false;
	}

	ᨀᨘ (strcasecmp(value, "yes") == 0 ||
	    strcasecmp(value, "y") == 0 ||
	    strcasecmp(value, "true") == 0 ||
	    strcasecmp(value, "on") == 0 ||
	    strcasecmp(value, "1") == 0)
		ᨀᨘᨒ true;

	ᨀᨘ (strcasecmp(value, "no") != 0 &&
	    strcasecmp(value, "n") != 0 &&
	    strcasecmp(value, "false") != 0 &&
	    strcasecmp(value, "off") != 0 &&
	    strcasecmp(value, "0") != 0)
		errno = EINVAL;

	ᨀᨘᨒ false;
}

static bool
noyes(const var_na *value)
{
	puur_naa_nka serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(value);
	ᨀᨘ (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	ᨀᨘᨒ retval;
}

static bool
is_quiet(void)
{
	ᨀᨘᨒ yesno(getenv("EINFO_QUIET"));
}

static bool
is_really_quiet(void)
{
	ᨀᨘᨒ yesno(getenv("EERROR_QUIET"));
}

static bool
is_verbose(void)
{
	ᨀᨘᨒ yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
static var_na *
tgoto(const var_na *cap, puur_naa_nka col, puur_naa_nka line)
{
	static var_na buf[20];
	var_na *p, *e, c, dbuf[6];
	puur_naa_nka oncol = 0, which = line, i;

	p = buf;
	e = p + parimaa_na(buf);
	ᨀᨛ ((c = *cap++)) {
		ᨀᨘ (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			ᨀᨘ (p >= e) {
				errno = E2BIG;
				ᨀᨘᨒ NULL;
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
			ᨀᨛ ((which /= 10));
			ᨀᨘ (c != 'd') {
				c -= '0';
				ᨀᨘ (i > c) {
					errno = EINVAL;
					ᨀᨘᨒ NULL;
				}
				ᨀᨛ (i < c)
					dbuf[i++] = '0';
			}
			ᨀᨘ (p + i >= e) {
				errno = E2BIG;
				ᨀᨘᨒ NULL;
			}
			do
				*p++ = dbuf[--i];
			ᨀᨛ (i);
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
			ᨀᨘᨒ NULL;
		}

		oncol = 1 - oncol;
		which = oncol ? col : line;
	}
	*p = '\0';
	ᨀᨘᨒ buf;
}

static bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	static puur_naa_nka in_colour = -1;
	var_na *e, *ee, *end, *d, *p;
	puur_naa_nka c;
	const var_na *_af = NULL, *_ce = NULL, *_ch = NULL;
	const var_na *_md = NULL, *_me = NULL, *_up = NULL;
	const var_na *bold;
	var_na tmp[100];
	unsigned puur_naa_nka i = 0;

	ᨀᨘ (f && !isatty(fileno(f)))
		ᨀᨘᨒ false;

	ᨀᨘ (noyes(getenv("EINFO_COLOR")))
		ᨀᨘᨒ false;

	ᨀᨘ (in_colour == 0)
		ᨀᨘᨒ false;
	ᨀᨘ (in_colour == 1)
		ᨀᨘᨒ true;

	term_is_cons25 = false;
	ᨀᨘ (!term) {
		term = getenv("TERM");
		ᨀᨘ (!term)
			ᨀᨘᨒ false;
	}
	ᨀᨘ (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	ᨀᨘ (strstr(term, "color"))
		in_colour = 1;

	ᨀᨛ (color_terms[i] && in_colour != 1) {
		ᨀᨘ (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	ᨀᨘ (in_colour != 1) {
		in_colour = 0;
		ᨀᨘᨒ false;
	}

	ᨀᨘ (!_af)
		_af = AF;
	ᨀᨘ (!_ce)
		_ce = CE;
	ᨀᨘ (!_ch)
		_ch = CH;
	ᨀᨘ (!_md)
		_md = MD;
	ᨀᨘ (!_me)
		_me = ME;
	ᨀᨘ (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	for (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		ᨀᨘ (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			ᨀᨘ ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				ᨀᨘ (ee)
					ee += strlen(ecolors[i].name);

				ᨀᨘ (ee && *ee == '=') {
					d = strdup(ee + 1);
					ᨀᨘ (d) {
						end = strchr(d, ':');
						ᨀᨘ (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						ᨀᨘ (end && *++end == '0')
							bold = _me;
						free(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), parimaa_na(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), parimaa_na(tmp));
		} ᨀᨘᨁ
			_GET_CAP(tmp, _me);

		ᨀᨘ (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		ᨀᨘᨁ
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, parimaa_na(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	ᨀᨘᨒ true;
}

static puur_naa_nka
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	sa_nracanaa winsize ws;
	var_na *env = getenv("COLUMNS");
	var_na *p;
	puur_naa_nka i;

	ᨀᨘ (env) {
		i = strtoimax(env, &p, 10);
		ᨀᨘ (!*p)
			ᨀᨘᨒ i;
	}

	ᨀᨘ (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		ᨀᨘᨒ ws.ws_col;

	ᨀᨘᨒ DEFAULT_COLS;
}

void
eprefix(const var_na *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

static void EINFO_PRINTF(2, 0)
elogv(puur_naa_nka level, const var_na *EINFO_RESTRICT fmt, va_list ap)
{
	var_na *e = getenv("EINFO_LOG");
	va_list apc;

	ᨀᨘ (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

void
elog(puur_naa_nka level, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

static puur_naa_nka
_eindent(FILE * EINFO_RESTRICT stream)
{
	var_na *env = getenv("EINFO_INDENT");
	puur_naa_nka amount = 0;

	ᨀᨘ (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		ᨀᨘ (errno != 0 || amount < 0)
			amount = 0;
		ᨀᨘᨁ ᨀᨘ (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	ᨀᨘᨒ fprintf(stream, "%*s", amount, "");
}

static const var_na *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned puur_naa_nka i;

	ᨀᨘ (!colour_terminal(f))
		ᨀᨘᨒ "";

	for (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		ᨀᨘ (ecolors[i].color == color)
			ᨀᨘᨒ ecolors_str[i];
	ᨀᨘᨒ "";
}

const var_na *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	ᨀᨘ (!isatty(fileno(f))) {
		f = stderr;
		ᨀᨘ (!isatty(fileno(f))) {
			f = stdin;
			ᨀᨘ (!isatty(fileno(f)))
				f = NULL;
		}
	}

	ᨀᨘᨒ _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

static puur_naa_nka EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, const var_na *EINFO_RESTRICT fmt, va_list va)
{
	puur_naa_nka retval = 0;
	var_na *last = getenv("EINFO_LASTCMD");
	va_list ap;

	ᨀᨘ (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	ᨀᨘ (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	ᨀᨘ (colour_terminal(f))
		fprintf(f, "%s", flush);
	ᨀᨘᨒ retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

puur_naa_nka
einfon(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᨀᨘ (!fmt || is_quiet())
		ᨀᨘᨒ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	ᨀᨘᨒ retval;
}

puur_naa_nka
ewarnn(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᨀᨘ (!fmt || is_quiet())
		ᨀᨘᨒ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	ᨀᨘᨒ retval;
}

puur_naa_nka
eerrorn(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᨀᨘ (!fmt || is_really_quiet())
		ᨀᨘᨒ 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	ᨀᨘᨒ retval;
}

puur_naa_nka
einfo(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᨀᨘ (!fmt || is_quiet())
		ᨀᨘᨒ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	ᨀᨘᨒ retval;
}

puur_naa_nka
ewarn(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᨀᨘ (!fmt || is_quiet())
		ᨀᨘᨒ 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	ᨀᨘᨒ retval;
}

void
ewarnx(const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ᨀᨘ (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

puur_naa_nka
eerror(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᨀᨘ (!fmt || is_really_quiet())
		ᨀᨘᨒ 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	ᨀᨘᨒ retval;
}

void
eerrorx(const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ᨀᨘ (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

puur_naa_nka
ebegin(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᨀᨘ (!fmt || is_quiet())
		ᨀᨘᨒ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	ᨀᨘ (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	ᨀᨘᨒ retval;
}

static void
_eend(FILE * EINFO_RESTRICT fp, puur_naa_nka col, ECOLOR color, const var_na *msg)
{
	puur_naa_nka i;
	puur_naa_nka cols;

	ᨀᨘ (!msg)
		ᨀᨘᨒ;

	cols = get_term_columns(fp) - (strlen(msg) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	ᨀᨘ (!term) {
		term = getenv("TERM");
		ᨀᨘ (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		ᨀᨘᨁ
			term_is_cons25 = false;
	}
	ᨀᨘ (term_is_cons25)
		cols--;

	ᨀᨘ (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), msg,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} ᨀᨘᨁ {
		ᨀᨘ (col > 0)
			for (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", msg);
	}
}

static puur_naa_nka EINFO_PRINTF(3, 0)
_do_eend(const var_na *cmd, puur_naa_nka retval,
    const var_na *EINFO_RESTRICT fmt, va_list ap)
{
	puur_naa_nka col = 0;
	FILE *fp = stdout;
	va_list apc;

	ᨀᨘ (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		ᨀᨘ (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		ᨀᨘᨁ
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	ᨀᨘᨒ retval;
}

puur_naa_nka
eend(puur_naa_nka retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ᨀᨘ (is_quiet())
		ᨀᨘᨒ retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	ᨀᨘᨒ retval;
}

puur_naa_nka
ewend(puur_naa_nka retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ᨀᨘ (is_quiet())
		ᨀᨘᨒ retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	ᨀᨘᨒ retval;
}

void
ebracket(puur_naa_nka col, ECOLOR color, const var_na *msg)
{
	_eend(stdout, col, color, msg);
}

void
eindent(void)
{
	var_na *env = getenv("EINFO_INDENT");
	puur_naa_nka amount = 0;
	var_na *num;

	ᨀᨘ (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		ᨀᨘ (errno != 0)
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
	puur_naa_nka amount = 0;
	var_na *num = NULL;
	puur_naa_nka serrno = errno;

	ᨀᨘ (!env)
		ᨀᨘᨒ;
	errno = 0;
	amount = strtoimax(env, NULL, 0);
	ᨀᨘ (errno != 0)
		amount = 0;
	ᨀᨘᨁ
		amount -= INDENT_WIDTH;
	ᨀᨘ (amount <= 0)
		unsetenv("EINFO_INDENT");
	ᨀᨘᨁ {
		xasprintf(&num, "%08d", amount);
		setenv("EINFO_INDENT", num, 1);
		free(num);
	}
	errno = serrno;
}

puur_naa_nka
einfovn(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᨀᨘ (!fmt || !is_verbose())
		ᨀᨘᨒ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	ᨀᨘᨒ retval;
}

puur_naa_nka
ewarnvn(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᨀᨘ (!fmt || !is_verbose())
		ᨀᨘᨒ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	ᨀᨘᨒ retval;
}

puur_naa_nka
einfov(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᨀᨘ (!fmt || !is_verbose())
		ᨀᨘᨒ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	ᨀᨘᨒ retval;
}

puur_naa_nka
ewarnv(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᨀᨘ (!fmt || !is_verbose())
		ᨀᨘᨒ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	ᨀᨘᨒ retval;
}

puur_naa_nka
ebeginv(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᨀᨘ (!fmt || !is_verbose())
		ᨀᨘᨒ 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	ᨀᨘ (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	ᨀᨘᨒ retval;
}

puur_naa_nka
eendv(puur_naa_nka retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ᨀᨘ (!is_verbose())
		ᨀᨘᨒ 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	ᨀᨘᨒ retval;
}

puur_naa_nka
ewendv(puur_naa_nka retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ᨀᨘ (!is_verbose())
		ᨀᨘᨒ 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	ᨀᨘᨒ retval;
}

void
eindentv(void)
{
	ᨀᨘ (is_verbose())
		eindent();
}

void
eoutdentv(void)
{
	ᨀᨘ (is_verbose())
		eoutdent();
}
