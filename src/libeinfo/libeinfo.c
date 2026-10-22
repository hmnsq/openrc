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
	} trong_khi (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
static const char *_eprefix = NULL;

/* Buffers and structures to hold the final colours */
static char ebuffer[100];
struct ecolor {
	ECOLOR color;
	số_nguyên def;
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

	trong_khi (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	nếu (src_n == 0)
		trả_về dst_n + strlen(src);

	trong_khi (*s != '\0') {
		nếu (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	trả_về dst_n + (s - src);
}
#endif

static bool
yesno(const char *value)
{
	nếu (!value) {
		errno = ENOENT;
		trả_về false;
	}

	nếu (strcasecmp(value, "yes") == 0 ||
	    strcasecmp(value, "y") == 0 ||
	    strcasecmp(value, "true") == 0 ||
	    strcasecmp(value, "on") == 0 ||
	    strcasecmp(value, "1") == 0)
		trả_về true;

	nếu (strcasecmp(value, "no") != 0 &&
	    strcasecmp(value, "n") != 0 &&
	    strcasecmp(value, "false") != 0 &&
	    strcasecmp(value, "off") != 0 &&
	    strcasecmp(value, "0") != 0)
		errno = EINVAL;

	trả_về false;
}

static bool
noyes(const char *value)
{
	số_nguyên serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(value);
	nếu (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	trả_về retval;
}

static bool
is_quiet(void)
{
	trả_về yesno(getenv("EINFO_QUIET"));
}

static bool
is_really_quiet(void)
{
	trả_về yesno(getenv("EERROR_QUIET"));
}

static bool
is_verbose(void)
{
	trả_về yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
static char *
tgoto(const char *cap, số_nguyên col, số_nguyên line)
{
	static char buf[20];
	char *p, *e, c, dbuf[6];
	số_nguyên oncol = 0, which = line, i;

	p = buf;
	e = p + sizeof(buf);
	trong_khi ((c = *cap++)) {
		nếu (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			nếu (p >= e) {
				errno = E2BIG;
				trả_về NULL;
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
			trong_khi ((which /= 10));
			nếu (c != 'd') {
				c -= '0';
				nếu (i > c) {
					errno = EINVAL;
					trả_về NULL;
				}
				trong_khi (i < c)
					dbuf[i++] = '0';
			}
			nếu (p + i >= e) {
				errno = E2BIG;
				trả_về NULL;
			}
			do
				*p++ = dbuf[--i];
			trong_khi (i);
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
			trả_về NULL;
		}

		oncol = 1 - oncol;
		which = oncol ? col : line;
	}
	*p = '\0';
	trả_về buf;
}

static bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	static số_nguyên in_colour = -1;
	char *e, *ee, *end, *d, *p;
	số_nguyên c;
	const char *_af = NULL, *_ce = NULL, *_ch = NULL;
	const char *_md = NULL, *_me = NULL, *_up = NULL;
	const char *bold;
	char tmp[100];
	unsigned số_nguyên i = 0;

	nếu (f && !isatty(fileno(f)))
		trả_về false;

	nếu (noyes(getenv("EINFO_COLOR")))
		trả_về false;

	nếu (in_colour == 0)
		trả_về false;
	nếu (in_colour == 1)
		trả_về true;

	term_is_cons25 = false;
	nếu (!term) {
		term = getenv("TERM");
		nếu (!term)
			trả_về false;
	}
	nếu (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	nếu (strstr(term, "color"))
		in_colour = 1;

	trong_khi (color_terms[i] && in_colour != 1) {
		nếu (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	nếu (in_colour != 1) {
		in_colour = 0;
		trả_về false;
	}

	nếu (!_af)
		_af = AF;
	nếu (!_ce)
		_ce = CE;
	nếu (!_ch)
		_ch = CH;
	nếu (!_md)
		_md = MD;
	nếu (!_me)
		_me = ME;
	nếu (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	cho (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		nếu (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			nếu ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				nếu (ee)
					ee += strlen(ecolors[i].name);

				nếu (ee && *ee == '=') {
					d = strdup(ee + 1);
					nếu (d) {
						end = strchr(d, ':');
						nếu (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						nếu (end && *++end == '0')
							bold = _me;
						free(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), sizeof(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), sizeof(tmp));
		} không_thì
			_GET_CAP(tmp, _me);

		nếu (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		không_thì
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, sizeof(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	trả_về true;
}

static số_nguyên
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	struct winsize ws;
	char *env = getenv("COLUMNS");
	char *p;
	số_nguyên i;

	nếu (env) {
		i = strtoimax(env, &p, 10);
		nếu (!*p)
			trả_về i;
	}

	nếu (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		trả_về ws.ws_col;

	trả_về DEFAULT_COLS;
}

void
eprefix(const char *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

static void EINFO_PRINTF(2, 0)
elogv(số_nguyên level, const char *EINFO_RESTRICT fmt, va_list ap)
{
	char *e = getenv("EINFO_LOG");
	va_list apc;

	nếu (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

void
elog(số_nguyên level, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

static số_nguyên
_eindent(FILE * EINFO_RESTRICT stream)
{
	char *env = getenv("EINFO_INDENT");
	số_nguyên amount = 0;

	nếu (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		nếu (errno != 0 || amount < 0)
			amount = 0;
		không_thì nếu (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	trả_về fprintf(stream, "%*s", amount, "");
}

static const char *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned số_nguyên i;

	nếu (!colour_terminal(f))
		trả_về "";

	cho (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		nếu (ecolors[i].color == color)
			trả_về ecolors_str[i];
	trả_về "";
}

const char *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	nếu (!isatty(fileno(f))) {
		f = stderr;
		nếu (!isatty(fileno(f))) {
			f = stdin;
			nếu (!isatty(fileno(f)))
				f = NULL;
		}
	}

	trả_về _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

static số_nguyên EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, const char *EINFO_RESTRICT fmt, va_list va)
{
	số_nguyên retval = 0;
	char *last = getenv("EINFO_LASTCMD");
	va_list ap;

	nếu (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	nếu (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	nếu (colour_terminal(f))
		fprintf(f, "%s", flush);
	trả_về retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

số_nguyên
einfon(const char *EINFO_RESTRICT fmt, ...)
{
	số_nguyên retval;
	va_list ap;

	nếu (!fmt || is_quiet())
		trả_về 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	trả_về retval;
}

số_nguyên
ewarnn(const char *EINFO_RESTRICT fmt, ...)
{
	số_nguyên retval;
	va_list ap;

	nếu (!fmt || is_quiet())
		trả_về 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	trả_về retval;
}

số_nguyên
eerrorn(const char *EINFO_RESTRICT fmt, ...)
{
	số_nguyên retval;
	va_list ap;

	nếu (!fmt || is_really_quiet())
		trả_về 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	trả_về retval;
}

số_nguyên
einfo(const char *EINFO_RESTRICT fmt, ...)
{
	số_nguyên retval;
	va_list ap;

	nếu (!fmt || is_quiet())
		trả_về 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	trả_về retval;
}

số_nguyên
ewarn(const char *EINFO_RESTRICT fmt, ...)
{
	số_nguyên retval;
	va_list ap;

	nếu (!fmt || is_quiet())
		trả_về 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	trả_về retval;
}

void
ewarnx(const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	nếu (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

số_nguyên
eerror(const char *EINFO_RESTRICT fmt, ...)
{
	số_nguyên retval;
	va_list ap;

	nếu (!fmt || is_really_quiet())
		trả_về 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	trả_về retval;
}

void
eerrorx(const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	nếu (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

số_nguyên
ebegin(const char *EINFO_RESTRICT fmt, ...)
{
	số_nguyên retval;
	va_list ap;

	nếu (!fmt || is_quiet())
		trả_về 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	nếu (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	trả_về retval;
}

static void
_eend(FILE * EINFO_RESTRICT fp, số_nguyên col, ECOLOR color, const char *msg)
{
	số_nguyên i;
	số_nguyên cols;

	nếu (!msg)
		trả_về;

	cols = get_term_columns(fp) - (strlen(msg) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	nếu (!term) {
		term = getenv("TERM");
		nếu (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		không_thì
			term_is_cons25 = false;
	}
	nếu (term_is_cons25)
		cols--;

	nếu (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), msg,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} không_thì {
		nếu (col > 0)
			cho (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", msg);
	}
}

static số_nguyên EINFO_PRINTF(3, 0)
_do_eend(const char *cmd, số_nguyên retval,
    const char *EINFO_RESTRICT fmt, va_list ap)
{
	số_nguyên col = 0;
	FILE *fp = stdout;
	va_list apc;

	nếu (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		nếu (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		không_thì
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	trả_về retval;
}

số_nguyên
eend(số_nguyên retval, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	nếu (is_quiet())
		trả_về retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	trả_về retval;
}

số_nguyên
ewend(số_nguyên retval, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	nếu (is_quiet())
		trả_về retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	trả_về retval;
}

void
ebracket(số_nguyên col, ECOLOR color, const char *msg)
{
	_eend(stdout, col, color, msg);
}

void
eindent(void)
{
	char *env = getenv("EINFO_INDENT");
	số_nguyên amount = 0;
	char *num;

	nếu (env) {
		errno = 0;
		amount = strtoimax(env, NULL, 0);
		nếu (errno != 0)
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
	số_nguyên amount = 0;
	char *num = NULL;
	số_nguyên serrno = errno;

	nếu (!env)
		trả_về;
	errno = 0;
	amount = strtoimax(env, NULL, 0);
	nếu (errno != 0)
		amount = 0;
	không_thì
		amount -= INDENT_WIDTH;
	nếu (amount <= 0)
		unsetenv("EINFO_INDENT");
	không_thì {
		xasprintf(&num, "%08d", amount);
		setenv("EINFO_INDENT", num, 1);
		free(num);
	}
	errno = serrno;
}

số_nguyên
einfovn(const char *EINFO_RESTRICT fmt, ...)
{
	số_nguyên retval;
	va_list ap;

	nếu (!fmt || !is_verbose())
		trả_về 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	trả_về retval;
}

số_nguyên
ewarnvn(const char *EINFO_RESTRICT fmt, ...)
{
	số_nguyên retval;
	va_list ap;

	nếu (!fmt || !is_verbose())
		trả_về 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	trả_về retval;
}

số_nguyên
einfov(const char *EINFO_RESTRICT fmt, ...)
{
	số_nguyên retval;
	va_list ap;

	nếu (!fmt || !is_verbose())
		trả_về 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	trả_về retval;
}

số_nguyên
ewarnv(const char *EINFO_RESTRICT fmt, ...)
{
	số_nguyên retval;
	va_list ap;

	nếu (!fmt || !is_verbose())
		trả_về 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	trả_về retval;
}

số_nguyên
ebeginv(const char *EINFO_RESTRICT fmt, ...)
{
	số_nguyên retval;
	va_list ap;

	nếu (!fmt || !is_verbose())
		trả_về 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	nếu (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	trả_về retval;
}

số_nguyên
eendv(số_nguyên retval, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	nếu (!is_verbose())
		trả_về 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	trả_về retval;
}

số_nguyên
ewendv(số_nguyên retval, const char *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	nếu (!is_verbose())
		trả_về 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	trả_về retval;
}

void
eindentv(void)
{
	nếu (is_verbose())
		eindent();
}

void
eoutdentv(void)
{
	nếu (is_verbose())
		eoutdent();
}
