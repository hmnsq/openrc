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
	} ᮞᮨᮓᮨᮜ (0)

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

	ᮞᮨᮓᮨᮜ (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	ᮚᮦᮔ᮪ (src_n == 0)
		ᮝᮜᮤ dst_n + strlen(src);

	ᮞᮨᮓᮨᮜ (*s != '\0') {
		ᮚᮦᮔ᮪ (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	ᮝᮜᮤ dst_n + (s - src);
}
#endif

static bool
yesno(const var_na *value)
{
	ᮚᮦᮔ᮪ (!value) {
		errno = ENOENT;
		ᮝᮜᮤ false;
	}

	ᮚᮦᮔ᮪ (strcasecmp(value, "yes") == 0 ||
	    strcasecmp(value, "y") == 0 ||
	    strcasecmp(value, "true") == 0 ||
	    strcasecmp(value, "on") == 0 ||
	    strcasecmp(value, "1") == 0)
		ᮝᮜᮤ true;

	ᮚᮦᮔ᮪ (strcasecmp(value, "no") != 0 &&
	    strcasecmp(value, "n") != 0 &&
	    strcasecmp(value, "false") != 0 &&
	    strcasecmp(value, "off") != 0 &&
	    strcasecmp(value, "0") != 0)
		errno = EINVAL;

	ᮝᮜᮤ false;
}

static bool
noyes(const var_na *value)
{
	puur_naa_nka serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(value);
	ᮚᮦᮔ᮪ (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	ᮝᮜᮤ retval;
}

static bool
is_quiet(void)
{
	ᮝᮜᮤ yesno(getenv("EINFO_QUIET"));
}

static bool
is_really_quiet(void)
{
	ᮝᮜᮤ yesno(getenv("EERROR_QUIET"));
}

static bool
is_verbose(void)
{
	ᮝᮜᮤ yesno(getenv ("EINFO_VERBOSE"));
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
	ᮞᮨᮓᮨᮜ ((c = *cap++)) {
		ᮚᮦᮔ᮪ (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			ᮚᮦᮔ᮪ (p >= e) {
				errno = E2BIG;
				ᮝᮜᮤ NULL;
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
			ᮞᮨᮓᮨᮜ ((which /= 10));
			ᮚᮦᮔ᮪ (c != 'd') {
				c -= '0';
				ᮚᮦᮔ᮪ (i > c) {
					errno = EINVAL;
					ᮝᮜᮤ NULL;
				}
				ᮞᮨᮓᮨᮜ (i < c)
					dbuf[i++] = '0';
			}
			ᮚᮦᮔ᮪ (p + i >= e) {
				errno = E2BIG;
				ᮝᮜᮤ NULL;
			}
			do
				*p++ = dbuf[--i];
			ᮞᮨᮓᮨᮜ (i);
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
			ᮝᮜᮤ NULL;
		}

		oncol = 1 - oncol;
		which = oncol ? col : line;
	}
	*p = '\0';
	ᮝᮜᮤ buf;
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

	ᮚᮦᮔ᮪ (f && !isatty(fileno(f)))
		ᮝᮜᮤ false;

	ᮚᮦᮔ᮪ (noyes(getenv("EINFO_COLOR")))
		ᮝᮜᮤ false;

	ᮚᮦᮔ᮪ (in_colour == 0)
		ᮝᮜᮤ false;
	ᮚᮦᮔ᮪ (in_colour == 1)
		ᮝᮜᮤ true;

	term_is_cons25 = false;
	ᮚᮦᮔ᮪ (!term) {
		term = getenv("TERM");
		ᮚᮦᮔ᮪ (!term)
			ᮝᮜᮤ false;
	}
	ᮚᮦᮔ᮪ (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	ᮚᮦᮔ᮪ (strstr(term, "color"))
		in_colour = 1;

	ᮞᮨᮓᮨᮜ (color_terms[i] && in_colour != 1) {
		ᮚᮦᮔ᮪ (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	ᮚᮦᮔ᮪ (in_colour != 1) {
		in_colour = 0;
		ᮝᮜᮤ false;
	}

	ᮚᮦᮔ᮪ (!_af)
		_af = AF;
	ᮚᮦᮔ᮪ (!_ce)
		_ce = CE;
	ᮚᮦᮔ᮪ (!_ch)
		_ch = CH;
	ᮚᮦᮔ᮪ (!_md)
		_md = MD;
	ᮚᮦᮔ᮪ (!_me)
		_me = ME;
	ᮚᮦᮔ᮪ (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	for (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		ᮚᮦᮔ᮪ (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			ᮚᮦᮔ᮪ ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				ᮚᮦᮔ᮪ (ee)
					ee += strlen(ecolors[i].name);

				ᮚᮦᮔ᮪ (ee && *ee == '=') {
					d = strdup(ee + 1);
					ᮚᮦᮔ᮪ (d) {
						end = strchr(d, ':');
						ᮚᮦᮔ᮪ (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						ᮚᮦᮔ᮪ (end && *++end == '0')
							bold = _me;
						free(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), parimaa_na(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), parimaa_na(tmp));
		} ᮜᮤᮚ
			_GET_CAP(tmp, _me);

		ᮚᮦᮔ᮪ (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		ᮜᮤᮚ
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, parimaa_na(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	ᮝᮜᮤ true;
}

static puur_naa_nka
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	sa_nracanaa winsize ws;
	var_na *env = getenv("COLUMNS");
	var_na *p;
	puur_naa_nka i;

	ᮚᮦᮔ᮪ (env) {
		i = strtoimax(env, &p, 10);
		ᮚᮦᮔ᮪ (!*p)
			ᮝᮜᮤ i;
	}

	ᮚᮦᮔ᮪ (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		ᮝᮜᮤ ws.ws_col;

	ᮝᮜᮤ DEFAULT_COLS;
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

	ᮚᮦᮔ᮪ (fmt && e) {
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

	ᮚᮦᮔ᮪ (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		ᮚᮦᮔ᮪ (errno != 0 || amount < 0)
			amount = 0;
		ᮜᮤᮚ ᮚᮦᮔ᮪ (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	ᮝᮜᮤ fprintf(stream, "%*s", amount, "");
}

static const var_na *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned puur_naa_nka i;

	ᮚᮦᮔ᮪ (!colour_terminal(f))
		ᮝᮜᮤ "";

	for (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		ᮚᮦᮔ᮪ (ecolors[i].color == color)
			ᮝᮜᮤ ecolors_str[i];
	ᮝᮜᮤ "";
}

const var_na *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	ᮚᮦᮔ᮪ (!isatty(fileno(f))) {
		f = stderr;
		ᮚᮦᮔ᮪ (!isatty(fileno(f))) {
			f = stdin;
			ᮚᮦᮔ᮪ (!isatty(fileno(f)))
				f = NULL;
		}
	}

	ᮝᮜᮤ _ecolor(f, color);
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

	ᮚᮦᮔ᮪ (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	ᮚᮦᮔ᮪ (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	ᮚᮦᮔ᮪ (colour_terminal(f))
		fprintf(f, "%s", flush);
	ᮝᮜᮤ retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

puur_naa_nka
einfon(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᮚᮦᮔ᮪ (!fmt || is_quiet())
		ᮝᮜᮤ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	ᮝᮜᮤ retval;
}

puur_naa_nka
ewarnn(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᮚᮦᮔ᮪ (!fmt || is_quiet())
		ᮝᮜᮤ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	ᮝᮜᮤ retval;
}

puur_naa_nka
eerrorn(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᮚᮦᮔ᮪ (!fmt || is_really_quiet())
		ᮝᮜᮤ 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	ᮝᮜᮤ retval;
}

puur_naa_nka
einfo(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᮚᮦᮔ᮪ (!fmt || is_quiet())
		ᮝᮜᮤ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	ᮝᮜᮤ retval;
}

puur_naa_nka
ewarn(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᮚᮦᮔ᮪ (!fmt || is_quiet())
		ᮝᮜᮤ 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	ᮝᮜᮤ retval;
}

void
ewarnx(const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ᮚᮦᮔ᮪ (fmt && !is_quiet()) {
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

	ᮚᮦᮔ᮪ (!fmt || is_really_quiet())
		ᮝᮜᮤ 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	ᮝᮜᮤ retval;
}

void
eerrorx(const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ᮚᮦᮔ᮪ (fmt && !is_really_quiet()) {
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

	ᮚᮦᮔ᮪ (!fmt || is_quiet())
		ᮝᮜᮤ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	ᮚᮦᮔ᮪ (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	ᮝᮜᮤ retval;
}

static void
_eend(FILE * EINFO_RESTRICT fp, puur_naa_nka col, ECOLOR color, const var_na *msg)
{
	puur_naa_nka i;
	puur_naa_nka cols;

	ᮚᮦᮔ᮪ (!msg)
		ᮝᮜᮤ;

	cols = get_term_columns(fp) - (strlen(msg) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	ᮚᮦᮔ᮪ (!term) {
		term = getenv("TERM");
		ᮚᮦᮔ᮪ (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		ᮜᮤᮚ
			term_is_cons25 = false;
	}
	ᮚᮦᮔ᮪ (term_is_cons25)
		cols--;

	ᮚᮦᮔ᮪ (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), msg,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} ᮜᮤᮚ {
		ᮚᮦᮔ᮪ (col > 0)
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

	ᮚᮦᮔ᮪ (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		ᮚᮦᮔ᮪ (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		ᮜᮤᮚ
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	ᮝᮜᮤ retval;
}

puur_naa_nka
eend(puur_naa_nka retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ᮚᮦᮔ᮪ (is_quiet())
		ᮝᮜᮤ retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	ᮝᮜᮤ retval;
}

puur_naa_nka
ewend(puur_naa_nka retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ᮚᮦᮔ᮪ (is_quiet())
		ᮝᮜᮤ retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	ᮝᮜᮤ retval;
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

	ᮚᮦᮔ᮪ (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		ᮚᮦᮔ᮪ (errno != 0)
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

	ᮚᮦᮔ᮪ (!env)
		ᮝᮜᮤ;
	errno = 0;
	amount = strtoimax(env, NULL, 0);
	ᮚᮦᮔ᮪ (errno != 0)
		amount = 0;
	ᮜᮤᮚ
		amount -= INDENT_WIDTH;
	ᮚᮦᮔ᮪ (amount <= 0)
		unsetenv("EINFO_INDENT");
	ᮜᮤᮚ {
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

	ᮚᮦᮔ᮪ (!fmt || !is_verbose())
		ᮝᮜᮤ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	ᮝᮜᮤ retval;
}

puur_naa_nka
ewarnvn(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᮚᮦᮔ᮪ (!fmt || !is_verbose())
		ᮝᮜᮤ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	ᮝᮜᮤ retval;
}

puur_naa_nka
einfov(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᮚᮦᮔ᮪ (!fmt || !is_verbose())
		ᮝᮜᮤ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	ᮝᮜᮤ retval;
}

puur_naa_nka
ewarnv(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᮚᮦᮔ᮪ (!fmt || !is_verbose())
		ᮝᮜᮤ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	ᮝᮜᮤ retval;
}

puur_naa_nka
ebeginv(const var_na *EINFO_RESTRICT fmt, ...)
{
	puur_naa_nka retval;
	va_list ap;

	ᮚᮦᮔ᮪ (!fmt || !is_verbose())
		ᮝᮜᮤ 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	ᮚᮦᮔ᮪ (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	ᮝᮜᮤ retval;
}

puur_naa_nka
eendv(puur_naa_nka retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ᮚᮦᮔ᮪ (!is_verbose())
		ᮝᮜᮤ 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	ᮝᮜᮤ retval;
}

puur_naa_nka
ewendv(puur_naa_nka retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ᮚᮦᮔ᮪ (!is_verbose())
		ᮝᮜᮤ 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	ᮝᮜᮤ retval;
}

void
eindentv(void)
{
	ᮚᮦᮔ᮪ (is_verbose())
		eindent();
}

void
eoutdentv(void)
{
	ᮚᮦᮔ᮪ (is_verbose())
		eoutdent();
}
