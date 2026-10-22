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
	} док (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
static const char *_eprefix = NULL;

/* Buffers and structures to hold the final colours */
static char ebuffer[100];
struct ecolor {
	ECOLOR color;
	цео def;
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

	док (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	ако (src_n == 0)
		врати dst_n + strlen(src);

	док (*s != '\0') {
		ако (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	врати dst_n + (s - src);
}
#endif

static bool
yesno(const char *value)
{
	ако (!value) {
		errno = ENOENT;
		врати false;
	}

	ако (strcasecmp(value, "yes") == 0 ||
	    strcasecmp(value, "y") == 0 ||
	    strcasecmp(value, "true") == 0 ||
	    strcasecmp(value, "on") == 0 ||
	    strcasecmp(value, "1") == 0)
		врати true;

	ако (strcasecmp(value, "no") != 0 &&
	    strcasecmp(value, "n") != 0 &&
	    strcasecmp(value, "false") != 0 &&
	    strcasecmp(value, "off") != 0 &&
	    strcasecmp(value, "0") != 0)
		errno = EINVAL;

	врати false;
}

static bool
noyes(const char *value)
{
	цео serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(value);
	ако (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	врати retval;
}

static bool
is_quiet(void)
{
	врати yesno(getenv("EINFO_QUIET"));
}

static bool
is_really_quiet(void)
{
	врати yesno(getenv("EERROR_QUIET"));
}

static bool
is_verbose(void)
{
	врати yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
static char *
tgoto(const char *cap, цео col, цео line)
{
	static char buf[20];
	char *p, *e, c, dbuf[6];
	цео oncol = 0, which = line, i;

	p = buf;
	e = p + sizeof(buf);
	док ((c = *cap++)) {
		ако (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			ако (p >= e) {
				errno = E2BIG;
				врати NULL;
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
			док ((which /= 10));
			ако (c != 'd') {
				c -= '0';
				ако (i > c) {
					errno = EINVAL;
					врати NULL;
				}
				док (i < c)
					dbuf[i++] = '0';
			}
			ако (p + i >= e) {
				errno = E2BIG;
				врати NULL;
			}
			do
				*p++ = dbuf[--i];
			док (i);
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
			врати NULL;
		}

		oncol = 1 - oncol;
		which = oncol ? col : line;
	}
	*p = '\0';
	врати buf;
}

static bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	static цео in_colour = -1;
	char *e, *ee, *end, *d, *p;
	цео c;
	const char *_af = NULL, *_ce = NULL, *_ch = NULL;
	const char *_md = NULL, *_me = NULL, *_up = NULL;
	const char *bold;
	char tmp[100];
	unsigned цео i = 0;

	ако (f && !isatty(fileno(f)))
		врати false;

	ако (noyes(getenv("EINFO_COLOR")))
		врати false;

	ако (in_colour == 0)
		врати false;
	ако (in_colour == 1)
		врати true;

	term_is_cons25 = false;
	ако (!term) {
		term = getenv("TERM");
		ако (!term)
			врати false;
	}
	ако (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	ако (strstr(term, "color"))
		in_colour = 1;

	док (color_terms[i] && in_colour != 1) {
		ако (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	ако (in_colour != 1) {
		in_colour = 0;
		врати false;
	}

	ако (!_af)
		_af = AF;
	ако (!_ce)
		_ce = CE;
	ако (!_ch)
		_ch = CH;
	ако (!_md)
		_md = MD;
	ако (!_me)
		_me = ME;
	ако (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	за (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		ако (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			ако ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				ако (ee)
					ee += strlen(ecolors[i].name);

				ако (ee && *ee == '=') {
					d = strdup(ee + 1);
					ако (d) {
						end = strchr(d, ':');
						ако (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						ако (end && *++end == '0')
							bold = _me;
						free(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), sizeof(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), sizeof(tmp));
		} инако
			_GET_CAP(tmp, _me);

		ако (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		инако
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, sizeof(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	врати true;
}

static цео
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	struct winsize ws;
	char *env = getenv("COLUMNS");
	char *p;
	цео i;

	ако (env) {
		i = strtoimax(env, &p, 10);
		ако (!*p)
			врати i;
	}

	ако (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		врати ws.ws_col;

	врати DEFAULT_COLS;
}

void
eprefix(const char *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

static void EINFO_PRINTF(2, 0)
elogv(цео level, const char *EINFO_RESTRICT fmt, va_list ap)
{
	char *e = getenv("EINFO_LOG");
	va_list apc;

	ако (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

void
elog(цео level, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

static цео
_eindent(FILE * EINFO_RESTRICT stream)
{
	char *env = getenv("EINFO_INDENT");
	цео amount = 0;

	ако (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		ако (errno != 0 || amount < 0)
			amount = 0;
		инако ако (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	врати fprintf(stream, "%*s", amount, "");
}

static const char *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned цео i;

	ако (!colour_terminal(f))
		врати "";

	за (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		ако (ecolors[i].color == color)
			врати ecolors_str[i];
	врати "";
}

const char *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	ако (!isatty(fileno(f))) {
		f = stderr;
		ако (!isatty(fileno(f))) {
			f = stdin;
			ако (!isatty(fileno(f)))
				f = NULL;
		}
	}

	врати _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

static цео EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, const char *EINFO_RESTRICT fmt, va_list va)
{
	цео retval = 0;
	char *last = getenv("EINFO_LASTCMD");
	va_list ap;

	ако (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	ако (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	ако (colour_terminal(f))
		fprintf(f, "%s", flush);
	врати retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

цео
einfon(const char *EINFO_RESTRICT fmt, ...)
{
	цео retval;
	va_list ap;

	ако (!fmt || is_quiet())
		врати 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	врати retval;
}

цео
ewarnn(const char *EINFO_RESTRICT fmt, ...)
{
	цео retval;
	va_list ap;

	ако (!fmt || is_quiet())
		врати 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	врати retval;
}

цео
eerrorn(const char *EINFO_RESTRICT fmt, ...)
{
	цео retval;
	va_list ap;

	ако (!fmt || is_really_quiet())
		врати 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	врати retval;
}

цео
einfo(const char *EINFO_RESTRICT fmt, ...)
{
	цео retval;
	va_list ap;

	ако (!fmt || is_quiet())
		врати 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	врати retval;
}

цео
ewarn(const char *EINFO_RESTRICT fmt, ...)
{
	цео retval;
	va_list ap;

	ако (!fmt || is_quiet())
		врати 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	врати retval;
}

void
ewarnx(const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ако (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

цео
eerror(const char *EINFO_RESTRICT fmt, ...)
{
	цео retval;
	va_list ap;

	ако (!fmt || is_really_quiet())
		врати 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	врати retval;
}

void
eerrorx(const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ако (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

цео
ebegin(const char *EINFO_RESTRICT fmt, ...)
{
	цео retval;
	va_list ap;

	ако (!fmt || is_quiet())
		врати 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	ако (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	врати retval;
}

static void
_eend(FILE * EINFO_RESTRICT fp, цео col, ECOLOR color, const char *msg)
{
	цео i;
	цео cols;

	ако (!msg)
		врати;

	cols = get_term_columns(fp) - (strlen(msg) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	ако (!term) {
		term = getenv("TERM");
		ако (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		инако
			term_is_cons25 = false;
	}
	ако (term_is_cons25)
		cols--;

	ако (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), msg,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} инако {
		ако (col > 0)
			за (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", msg);
	}
}

static цео EINFO_PRINTF(3, 0)
_do_eend(const char *cmd, цео retval,
    const char *EINFO_RESTRICT fmt, va_list ap)
{
	цео col = 0;
	FILE *fp = stdout;
	va_list apc;

	ако (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		ако (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		инако
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	врати retval;
}

цео
eend(цео retval, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ако (is_quiet())
		врати retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	врати retval;
}

цео
ewend(цео retval, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ако (is_quiet())
		врати retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	врати retval;
}

void
ebracket(цео col, ECOLOR color, const char *msg)
{
	_eend(stdout, col, color, msg);
}

void
eindent(void)
{
	char *env = getenv("EINFO_INDENT");
	цео amount = 0;
	char *num;

	ако (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		ако (errno != 0)
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
	цео amount = 0;
	char *num = NULL;
	цео serrno = errno;

	ако (!env)
		врати;
	errno = 0;
	amount = strtoimax(env, NULL, 0);
	ако (errno != 0)
		amount = 0;
	инако
		amount -= INDENT_WIDTH;
	ако (amount <= 0)
		unsetenv("EINFO_INDENT");
	инако {
		xasprintf(&num, "%08d", amount);
		setenv("EINFO_INDENT", num, 1);
		free(num);
	}
	errno = serrno;
}

цео
einfovn(const char *EINFO_RESTRICT fmt, ...)
{
	цео retval;
	va_list ap;

	ако (!fmt || !is_verbose())
		врати 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	врати retval;
}

цео
ewarnvn(const char *EINFO_RESTRICT fmt, ...)
{
	цео retval;
	va_list ap;

	ако (!fmt || !is_verbose())
		врати 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	врати retval;
}

цео
einfov(const char *EINFO_RESTRICT fmt, ...)
{
	цео retval;
	va_list ap;

	ако (!fmt || !is_verbose())
		врати 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	врати retval;
}

цео
ewarnv(const char *EINFO_RESTRICT fmt, ...)
{
	цео retval;
	va_list ap;

	ако (!fmt || !is_verbose())
		врати 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	врати retval;
}

цео
ebeginv(const char *EINFO_RESTRICT fmt, ...)
{
	цео retval;
	va_list ap;

	ако (!fmt || !is_verbose())
		врати 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	ако (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	врати retval;
}

цео
eendv(цео retval, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ако (!is_verbose())
		врати 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	врати retval;
}

цео
ewendv(цео retval, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ако (!is_verbose())
		врати 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	врати retval;
}

void
eindentv(void)
{
	ако (is_verbose())
		eindent();
}

void
eoutdentv(void)
{
	ако (is_verbose())
		eoutdent();
}
