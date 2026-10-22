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
	} နေစဉ် (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
static const var_na *_eprefix = NULL;

/* Buffers and structures to hold the final colours */
static var_na ebuffer[100];
sa_nracanaa ecolor {
	ECOLOR color;
	ကိန်းပြည့် def;
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

	နေစဉ် (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	အကယ်၍ (src_n == 0)
		ပြန်ပေး dst_n + strlen(src);

	နေစဉ် (*s != '\0') {
		အကယ်၍ (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	ပြန်ပေး dst_n + (s - src);
}
#endif

static bool
yesno(const var_na *value)
{
	အကယ်၍ (!value) {
		errno = ENOENT;
		ပြန်ပေး false;
	}

	အကယ်၍ (strcasecmp(value, "yes") == 0 ||
	    strcasecmp(value, "y") == 0 ||
	    strcasecmp(value, "true") == 0 ||
	    strcasecmp(value, "on") == 0 ||
	    strcasecmp(value, "1") == 0)
		ပြန်ပေး true;

	အကယ်၍ (strcasecmp(value, "no") != 0 &&
	    strcasecmp(value, "n") != 0 &&
	    strcasecmp(value, "false") != 0 &&
	    strcasecmp(value, "off") != 0 &&
	    strcasecmp(value, "0") != 0)
		errno = EINVAL;

	ပြန်ပေး false;
}

static bool
noyes(const var_na *value)
{
	ကိန်းပြည့် serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(value);
	အကယ်၍ (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	ပြန်ပေး retval;
}

static bool
is_quiet(void)
{
	ပြန်ပေး yesno(getenv("EINFO_QUIET"));
}

static bool
is_really_quiet(void)
{
	ပြန်ပေး yesno(getenv("EERROR_QUIET"));
}

static bool
is_verbose(void)
{
	ပြန်ပေး yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
static var_na *
tgoto(const var_na *cap, ကိန်းပြည့် col, ကိန်းပြည့် line)
{
	static var_na buf[20];
	var_na *p, *e, c, dbuf[6];
	ကိန်းပြည့် oncol = 0, which = line, i;

	p = buf;
	e = p + parimaa_na(buf);
	နေစဉ် ((c = *cap++)) {
		အကယ်၍ (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			အကယ်၍ (p >= e) {
				errno = E2BIG;
				ပြန်ပေး NULL;
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
			နေစဉ် ((which /= 10));
			အကယ်၍ (c != 'd') {
				c -= '0';
				အကယ်၍ (i > c) {
					errno = EINVAL;
					ပြန်ပေး NULL;
				}
				နေစဉ် (i < c)
					dbuf[i++] = '0';
			}
			အကယ်၍ (p + i >= e) {
				errno = E2BIG;
				ပြန်ပေး NULL;
			}
			do
				*p++ = dbuf[--i];
			နေစဉ် (i);
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
			ပြန်ပေး NULL;
		}

		oncol = 1 - oncol;
		which = oncol ? col : line;
	}
	*p = '\0';
	ပြန်ပေး buf;
}

static bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	static ကိန်းပြည့် in_colour = -1;
	var_na *e, *ee, *end, *d, *p;
	ကိန်းပြည့် c;
	const var_na *_af = NULL, *_ce = NULL, *_ch = NULL;
	const var_na *_md = NULL, *_me = NULL, *_up = NULL;
	const var_na *bold;
	var_na tmp[100];
	unsigned ကိန်းပြည့် i = 0;

	အကယ်၍ (f && !isatty(fileno(f)))
		ပြန်ပေး false;

	အကယ်၍ (noyes(getenv("EINFO_COLOR")))
		ပြန်ပေး false;

	အကယ်၍ (in_colour == 0)
		ပြန်ပေး false;
	အကယ်၍ (in_colour == 1)
		ပြန်ပေး true;

	term_is_cons25 = false;
	အကယ်၍ (!term) {
		term = getenv("TERM");
		အကယ်၍ (!term)
			ပြန်ပေး false;
	}
	အကယ်၍ (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	အကယ်၍ (strstr(term, "color"))
		in_colour = 1;

	နေစဉ် (color_terms[i] && in_colour != 1) {
		အကယ်၍ (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	အကယ်၍ (in_colour != 1) {
		in_colour = 0;
		ပြန်ပေး false;
	}

	အကယ်၍ (!_af)
		_af = AF;
	အကယ်၍ (!_ce)
		_ce = CE;
	အကယ်၍ (!_ch)
		_ch = CH;
	အကယ်၍ (!_md)
		_md = MD;
	အကယ်၍ (!_me)
		_me = ME;
	အကယ်၍ (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	အတွက် (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		အကယ်၍ (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			အကယ်၍ ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				အကယ်၍ (ee)
					ee += strlen(ecolors[i].name);

				အကယ်၍ (ee && *ee == '=') {
					d = strdup(ee + 1);
					အကယ်၍ (d) {
						end = strchr(d, ':');
						အကယ်၍ (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						အကယ်၍ (end && *++end == '0')
							bold = _me;
						free(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), parimaa_na(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), parimaa_na(tmp));
		} မဟုတ်ပါက
			_GET_CAP(tmp, _me);

		အကယ်၍ (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		မဟုတ်ပါက
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, parimaa_na(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	ပြန်ပေး true;
}

static ကိန်းပြည့်
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	sa_nracanaa winsize ws;
	var_na *env = getenv("COLUMNS");
	var_na *p;
	ကိန်းပြည့် i;

	အကယ်၍ (env) {
		i = strtoimax(env, &p, 10);
		အကယ်၍ (!*p)
			ပြန်ပေး i;
	}

	အကယ်၍ (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		ပြန်ပေး ws.ws_col;

	ပြန်ပေး DEFAULT_COLS;
}

void
eprefix(const var_na *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

static void EINFO_PRINTF(2, 0)
elogv(ကိန်းပြည့် level, const var_na *EINFO_RESTRICT fmt, va_list ap)
{
	var_na *e = getenv("EINFO_LOG");
	va_list apc;

	အကယ်၍ (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

void
elog(ကိန်းပြည့် level, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

static ကိန်းပြည့်
_eindent(FILE * EINFO_RESTRICT stream)
{
	var_na *env = getenv("EINFO_INDENT");
	ကိန်းပြည့် amount = 0;

	အကယ်၍ (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		အကယ်၍ (errno != 0 || amount < 0)
			amount = 0;
		မဟုတ်ပါက အကယ်၍ (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	ပြန်ပေး fprintf(stream, "%*s", amount, "");
}

static const var_na *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned ကိန်းပြည့် i;

	အကယ်၍ (!colour_terminal(f))
		ပြန်ပေး "";

	အတွက် (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		အကယ်၍ (ecolors[i].color == color)
			ပြန်ပေး ecolors_str[i];
	ပြန်ပေး "";
}

const var_na *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	အကယ်၍ (!isatty(fileno(f))) {
		f = stderr;
		အကယ်၍ (!isatty(fileno(f))) {
			f = stdin;
			အကယ်၍ (!isatty(fileno(f)))
				f = NULL;
		}
	}

	ပြန်ပေး _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

static ကိန်းပြည့် EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, const var_na *EINFO_RESTRICT fmt, va_list va)
{
	ကိန်းပြည့် retval = 0;
	var_na *last = getenv("EINFO_LASTCMD");
	va_list ap;

	အကယ်၍ (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	အကယ်၍ (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	အကယ်၍ (colour_terminal(f))
		fprintf(f, "%s", flush);
	ပြန်ပေး retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

ကိန်းပြည့်
einfon(const var_na *EINFO_RESTRICT fmt, ...)
{
	ကိန်းပြည့် retval;
	va_list ap;

	အကယ်၍ (!fmt || is_quiet())
		ပြန်ပေး 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	ပြန်ပေး retval;
}

ကိန်းပြည့်
ewarnn(const var_na *EINFO_RESTRICT fmt, ...)
{
	ကိန်းပြည့် retval;
	va_list ap;

	အကယ်၍ (!fmt || is_quiet())
		ပြန်ပေး 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	ပြန်ပေး retval;
}

ကိန်းပြည့်
eerrorn(const var_na *EINFO_RESTRICT fmt, ...)
{
	ကိန်းပြည့် retval;
	va_list ap;

	အကယ်၍ (!fmt || is_really_quiet())
		ပြန်ပေး 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	ပြန်ပေး retval;
}

ကိန်းပြည့်
einfo(const var_na *EINFO_RESTRICT fmt, ...)
{
	ကိန်းပြည့် retval;
	va_list ap;

	အကယ်၍ (!fmt || is_quiet())
		ပြန်ပေး 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	ပြန်ပေး retval;
}

ကိန်းပြည့်
ewarn(const var_na *EINFO_RESTRICT fmt, ...)
{
	ကိန်းပြည့် retval;
	va_list ap;

	အကယ်၍ (!fmt || is_quiet())
		ပြန်ပေး 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	ပြန်ပေး retval;
}

void
ewarnx(const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	အကယ်၍ (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

ကိန်းပြည့်
eerror(const var_na *EINFO_RESTRICT fmt, ...)
{
	ကိန်းပြည့် retval;
	va_list ap;

	အကယ်၍ (!fmt || is_really_quiet())
		ပြန်ပေး 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	ပြန်ပေး retval;
}

void
eerrorx(const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	အကယ်၍ (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

ကိန်းပြည့်
ebegin(const var_na *EINFO_RESTRICT fmt, ...)
{
	ကိန်းပြည့် retval;
	va_list ap;

	အကယ်၍ (!fmt || is_quiet())
		ပြန်ပေး 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	အကယ်၍ (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	ပြန်ပေး retval;
}

static void
_eend(FILE * EINFO_RESTRICT fp, ကိန်းပြည့် col, ECOLOR color, const var_na *msg)
{
	ကိန်းပြည့် i;
	ကိန်းပြည့် cols;

	အကယ်၍ (!msg)
		ပြန်ပေး;

	cols = get_term_columns(fp) - (strlen(msg) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	အကယ်၍ (!term) {
		term = getenv("TERM");
		အကယ်၍ (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		မဟုတ်ပါက
			term_is_cons25 = false;
	}
	အကယ်၍ (term_is_cons25)
		cols--;

	အကယ်၍ (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), msg,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} မဟုတ်ပါက {
		အကယ်၍ (col > 0)
			အတွက် (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", msg);
	}
}

static ကိန်းပြည့် EINFO_PRINTF(3, 0)
_do_eend(const var_na *cmd, ကိန်းပြည့် retval,
    const var_na *EINFO_RESTRICT fmt, va_list ap)
{
	ကိန်းပြည့် col = 0;
	FILE *fp = stdout;
	va_list apc;

	အကယ်၍ (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		အကယ်၍ (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		မဟုတ်ပါက
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	ပြန်ပေး retval;
}

ကိန်းပြည့်
eend(ကိန်းပြည့် retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	အကယ်၍ (is_quiet())
		ပြန်ပေး retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	ပြန်ပေး retval;
}

ကိန်းပြည့်
ewend(ကိန်းပြည့် retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	အကယ်၍ (is_quiet())
		ပြန်ပေး retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	ပြန်ပေး retval;
}

void
ebracket(ကိန်းပြည့် col, ECOLOR color, const var_na *msg)
{
	_eend(stdout, col, color, msg);
}

void
eindent(void)
{
	var_na *env = getenv("EINFO_INDENT");
	ကိန်းပြည့် amount = 0;
	var_na *num;

	အကယ်၍ (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		အကယ်၍ (errno != 0)
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
	ကိန်းပြည့် amount = 0;
	var_na *num = NULL;
	ကိန်းပြည့် serrno = errno;

	အကယ်၍ (!env)
		ပြန်ပေး;
	errno = 0;
	amount = strtoimax(env, NULL, 0);
	အကယ်၍ (errno != 0)
		amount = 0;
	မဟုတ်ပါက
		amount -= INDENT_WIDTH;
	အကယ်၍ (amount <= 0)
		unsetenv("EINFO_INDENT");
	မဟုတ်ပါက {
		xasprintf(&num, "%08d", amount);
		setenv("EINFO_INDENT", num, 1);
		free(num);
	}
	errno = serrno;
}

ကိန်းပြည့်
einfovn(const var_na *EINFO_RESTRICT fmt, ...)
{
	ကိန်းပြည့် retval;
	va_list ap;

	အကယ်၍ (!fmt || !is_verbose())
		ပြန်ပေး 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	ပြန်ပေး retval;
}

ကိန်းပြည့်
ewarnvn(const var_na *EINFO_RESTRICT fmt, ...)
{
	ကိန်းပြည့် retval;
	va_list ap;

	အကယ်၍ (!fmt || !is_verbose())
		ပြန်ပေး 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	ပြန်ပေး retval;
}

ကိန်းပြည့်
einfov(const var_na *EINFO_RESTRICT fmt, ...)
{
	ကိန်းပြည့် retval;
	va_list ap;

	အကယ်၍ (!fmt || !is_verbose())
		ပြန်ပေး 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	ပြန်ပေး retval;
}

ကိန်းပြည့်
ewarnv(const var_na *EINFO_RESTRICT fmt, ...)
{
	ကိန်းပြည့် retval;
	va_list ap;

	အကယ်၍ (!fmt || !is_verbose())
		ပြန်ပေး 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	ပြန်ပေး retval;
}

ကိန်းပြည့်
ebeginv(const var_na *EINFO_RESTRICT fmt, ...)
{
	ကိန်းပြည့် retval;
	va_list ap;

	အကယ်၍ (!fmt || !is_verbose())
		ပြန်ပေး 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	အကယ်၍ (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	ပြန်ပေး retval;
}

ကိန်းပြည့်
eendv(ကိန်းပြည့် retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	အကယ်၍ (!is_verbose())
		ပြန်ပေး 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	ပြန်ပေး retval;
}

ကိန်းပြည့်
ewendv(ကိန်းပြည့် retval, const var_na *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	အကယ်၍ (!is_verbose())
		ပြန်ပေး 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	ပြန်ပေး retval;
}

void
eindentv(void)
{
	အကယ်၍ (is_verbose())
		eindent();
}

void
eoutdentv(void)
{
	အကယ်၍ (is_verbose())
		eoutdent();
}
