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

#define _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), sizeof(_d));
#define _ASSIGN_CAP(_v) do {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, sizeof(ebuffer) - (p - ebuffer)) + 1;    \
	} 当 (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
static const char *_eprefix = NULL;

/* Buffers and structures to hold the final colours */
static char ebuffer[100];
struct ecolor {
	ECOLOR color;
	整数 def;
	const char *name;
};
static char nullstr = '\0';

static const struct ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_NORMAL,  0,       NULL      },
};
static const char *ecolors_str[ARRAY_SIZE(ecolors)];

static char *flush = NULL;
static char *up = NULL;
static char *goto_column = NULL;

static const char *term = NULL;
static bool term_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
static const char *const color_terms[] = {
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
strlcat(char *dst, const char *src, size_t size)
{
	char *d = dst;
	const char *s = src;
	size_t src_n = size;
	size_t dst_n;

	当 (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	如果 (src_n == 0)
		返回 dst_n + strlen(src);

	当 (*s != '\0') {
		如果 (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	返回 dst_n + (s - src);
}
#endif

static bool
yesno(const char *value)
{
	如果 (!value) {
		errno = ENOENT;
		返回 false;
	}

	如果 (strcasecmp(value, "yes") == 0 ||
	    strcasecmp(value, "y") == 0 ||
	    strcasecmp(value, "true") == 0 ||
	    strcasecmp(value, "on") == 0 ||
	    strcasecmp(value, "1") == 0)
		返回 true;

	如果 (strcasecmp(value, "no") != 0 &&
	    strcasecmp(value, "n") != 0 &&
	    strcasecmp(value, "false") != 0 &&
	    strcasecmp(value, "off") != 0 &&
	    strcasecmp(value, "0") != 0)
		errno = EINVAL;

	返回 false;
}

static bool
noyes(const char *value)
{
	整数 serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(value);
	如果 (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	返回 retval;
}

static bool
is_quiet(void)
{
	返回 yesno(getenv("EINFO_QUIET"));
}

static bool
is_really_quiet(void)
{
	返回 yesno(getenv("EERROR_QUIET"));
}

static bool
is_verbose(void)
{
	返回 yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
static char *
tgoto(const char *cap, 整数 col, 整数 line)
{
	static char buf[20];
	char *p, *e, c, dbuf[6];
	整数 oncol = 0, which = line, i;

	p = buf;
	e = p + sizeof(buf);
	当 ((c = *cap++)) {
		如果 (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			如果 (p >= e) {
				errno = E2BIG;
				返回 NULL;
			}
			continue;
		}
		switch (c) {
		case '3':
		case '2':
		case 'd':
			i = 0;
			do
				dbuf[i++] = which % 10 | '0';
			当 ((which /= 10));
			如果 (c != 'd') {
				c -= '0';
				如果 (i > c) {
					errno = EINVAL;
					返回 NULL;
				}
				当 (i < c)
					dbuf[i++] = '0';
			}
			如果 (p + i >= e) {
				errno = E2BIG;
				返回 NULL;
			}
			do
				*p++ = dbuf[--i];
			当 (i);
			break;
		case 'r':
			oncol = 0;
			break;
		case 'i':
			col++;
			line++;
			which++;
			continue;
		default:
			errno = EINVAL;
			返回 NULL;
		}

		oncol = 1 - oncol;
		which = oncol ? col : line;
	}
	*p = '\0';
	返回 buf;
}

static bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	static 整数 in_colour = -1;
	char *e, *ee, *end, *d, *p;
	整数 c;
	const char *_af = NULL, *_ce = NULL, *_ch = NULL;
	const char *_md = NULL, *_me = NULL, *_up = NULL;
	const char *bold;
	char tmp[100];
	unsigned 整数 i = 0;

	如果 (f && !isatty(fileno(f)))
		返回 false;

	如果 (noyes(getenv("EINFO_COLOR")))
		返回 false;

	如果 (in_colour == 0)
		返回 false;
	如果 (in_colour == 1)
		返回 true;

	term_is_cons25 = false;
	如果 (!term) {
		term = getenv("TERM");
		如果 (!term)
			返回 false;
	}
	如果 (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	如果 (strstr(term, "color"))
		in_colour = 1;

	当 (color_terms[i] && in_colour != 1) {
		如果 (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	如果 (in_colour != 1) {
		in_colour = 0;
		返回 false;
	}

	如果 (!_af)
		_af = AF;
	如果 (!_ce)
		_ce = CE;
	如果 (!_ch)
		_ch = CH;
	如果 (!_md)
		_md = MD;
	如果 (!_me)
		_me = ME;
	如果 (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	为 (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		如果 (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			如果 ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				如果 (ee)
					ee += strlen(ecolors[i].name);

				如果 (ee && *ee == '=') {
					d = strdup(ee + 1);
					如果 (d) {
						end = strchr(d, ':');
						如果 (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						如果 (end && *++end == '0')
							bold = _me;
						free(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), sizeof(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), sizeof(tmp));
		} 否则
			_GET_CAP(tmp, _me);

		如果 (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		否则
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, sizeof(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	返回 true;
}

static 整数
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	struct winsize ws;
	char *env = getenv("COLUMNS");
	char *p;
	整数 i;

	如果 (env) {
		i = strtoimax(env, &p, 10);
		如果 (!*p)
			返回 i;
	}

	如果 (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		返回 ws.ws_col;

	返回 DEFAULT_COLS;
}

void
eprefix(const char *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

static void EINFO_PRINTF(2, 0)
elogv(整数 level, const char *EINFO_RESTRICT fmt, va_list ap)
{
	char *e = getenv("EINFO_LOG");
	va_list apc;

	如果 (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

void
elog(整数 level, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

static 整数
_eindent(FILE * EINFO_RESTRICT stream)
{
	char *env = getenv("EINFO_INDENT");
	整数 amount = 0;

	如果 (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		如果 (errno != 0 || amount < 0)
			amount = 0;
		否则 如果 (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	返回 fprintf(stream, "%*s", amount, "");
}

static const char *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned 整数 i;

	如果 (!colour_terminal(f))
		返回 "";

	为 (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		如果 (ecolors[i].color == color)
			返回 ecolors_str[i];
	返回 "";
}

const char *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	如果 (!isatty(fileno(f))) {
		f = stderr;
		如果 (!isatty(fileno(f))) {
			f = stdin;
			如果 (!isatty(fileno(f)))
				f = NULL;
		}
	}

	返回 _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

static 整数 EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, const char *EINFO_RESTRICT fmt, va_list va)
{
	整数 retval = 0;
	char *last = getenv("EINFO_LASTCMD");
	va_list ap;

	如果 (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	如果 (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	如果 (colour_terminal(f))
		fprintf(f, "%s", flush);
	返回 retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

整数
einfon(const char *EINFO_RESTRICT fmt, ...)
{
	整数 retval;
	va_list ap;

	如果 (!fmt || is_quiet())
		返回 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	返回 retval;
}

整数
ewarnn(const char *EINFO_RESTRICT fmt, ...)
{
	整数 retval;
	va_list ap;

	如果 (!fmt || is_quiet())
		返回 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	返回 retval;
}

整数
eerrorn(const char *EINFO_RESTRICT fmt, ...)
{
	整数 retval;
	va_list ap;

	如果 (!fmt || is_really_quiet())
		返回 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	返回 retval;
}

整数
einfo(const char *EINFO_RESTRICT fmt, ...)
{
	整数 retval;
	va_list ap;

	如果 (!fmt || is_quiet())
		返回 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	返回 retval;
}

整数
ewarn(const char *EINFO_RESTRICT fmt, ...)
{
	整数 retval;
	va_list ap;

	如果 (!fmt || is_quiet())
		返回 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	返回 retval;
}

void
ewarnx(const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	如果 (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

整数
eerror(const char *EINFO_RESTRICT fmt, ...)
{
	整数 retval;
	va_list ap;

	如果 (!fmt || is_really_quiet())
		返回 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	返回 retval;
}

void
eerrorx(const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	如果 (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

整数
ebegin(const char *EINFO_RESTRICT fmt, ...)
{
	整数 retval;
	va_list ap;

	如果 (!fmt || is_quiet())
		返回 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	如果 (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	返回 retval;
}

static void
_eend(FILE * EINFO_RESTRICT fp, 整数 col, ECOLOR color, const char *msg)
{
	整数 i;
	整数 cols;

	如果 (!msg)
		返回;

	cols = get_term_columns(fp) - (strlen(msg) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	如果 (!term) {
		term = getenv("TERM");
		如果 (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		否则
			term_is_cons25 = false;
	}
	如果 (term_is_cons25)
		cols--;

	如果 (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), msg,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} 否则 {
		如果 (col > 0)
			为 (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", msg);
	}
}

static 整数 EINFO_PRINTF(3, 0)
_do_eend(const char *cmd, 整数 retval,
    const char *EINFO_RESTRICT fmt, va_list ap)
{
	整数 col = 0;
	FILE *fp = stdout;
	va_list apc;

	如果 (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		如果 (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		否则
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	返回 retval;
}

整数
eend(整数 retval, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	如果 (is_quiet())
		返回 retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	返回 retval;
}

整数
ewend(整数 retval, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	如果 (is_quiet())
		返回 retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	返回 retval;
}

void
ebracket(整数 col, ECOLOR color, const char *msg)
{
	_eend(stdout, col, color, msg);
}

void
eindent(void)
{
	char *env = getenv("EINFO_INDENT");
	整数 amount = 0;
	char *num;

	如果 (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		如果 (errno != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
	xasprintf(&num, "%08d", amount);
	setenv("EINFO_INDENT", num, 1);
	free(num);
}

void eoutdent(void)
{
	char *env = getenv("EINFO_INDENT");
	整数 amount = 0;
	char *num = NULL;
	整数 serrno = errno;

	如果 (!env)
		返回;
	errno = 0;
	amount = strtoimax(env, NULL, 0);
	如果 (errno != 0)
		amount = 0;
	否则
		amount -= INDENT_WIDTH;
	如果 (amount <= 0)
		unsetenv("EINFO_INDENT");
	否则 {
		xasprintf(&num, "%08d", amount);
		setenv("EINFO_INDENT", num, 1);
		free(num);
	}
	errno = serrno;
}

整数
einfovn(const char *EINFO_RESTRICT fmt, ...)
{
	整数 retval;
	va_list ap;

	如果 (!fmt || !is_verbose())
		返回 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	返回 retval;
}

整数
ewarnvn(const char *EINFO_RESTRICT fmt, ...)
{
	整数 retval;
	va_list ap;

	如果 (!fmt || !is_verbose())
		返回 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	返回 retval;
}

整数
einfov(const char *EINFO_RESTRICT fmt, ...)
{
	整数 retval;
	va_list ap;

	如果 (!fmt || !is_verbose())
		返回 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	返回 retval;
}

整数
ewarnv(const char *EINFO_RESTRICT fmt, ...)
{
	整数 retval;
	va_list ap;

	如果 (!fmt || !is_verbose())
		返回 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	返回 retval;
}

整数
ebeginv(const char *EINFO_RESTRICT fmt, ...)
{
	整数 retval;
	va_list ap;

	如果 (!fmt || !is_verbose())
		返回 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	如果 (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	返回 retval;
}

整数
eendv(整数 retval, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	如果 (!is_verbose())
		返回 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	返回 retval;
}

整数
ewendv(整数 retval, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	如果 (!is_verbose())
		返回 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	返回 retval;
}

void
eindentv(void)
{
	如果 (is_verbose())
		eindent();
}

void
eoutdentv(void)
{
	如果 (is_verbose())
		eoutdent();
}
