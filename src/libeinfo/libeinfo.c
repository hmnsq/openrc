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

#ضمن <sys/ioctl.h>
#ضمن <errno.h>
#ضمن <inttypes.h>
#ضمن <stdarg.h>
#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <syslog.h>
#ضمن <unistd.h>

#ضمن "einfo.h"
#ضمن "helpers.h"

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

#define _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), حجم(_d));
#define _ASSIGN_CAP(_v) 𐡏𐡁𐡃 {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, حجم(ebuffer) - (p - ebuffer)) + 1;    \
	} طالما (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
ساكن ثابت حرف *_eprefix = عدم;

/* Buffers and structures to hold the final colours */
ساكن حرف ebuffer[100];
بنية ecolor {
	ECOLOR color;
	صحيح def;
	ثابت حرف *name;
};
ساكن حرف nullstr = '\0';

ساكن ثابت بنية ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_NORMAL,  0,       عدم      },
};
ساكن ثابت حرف *ecolors_str[ARRAY_SIZE(ecolors)];

ساكن حرف *flush = عدم;
ساكن حرف *up = عدم;
ساكن حرف *goto_column = عدم;

ساكن ثابت حرف *term = عدم;
ساكن bool term_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
ساكن ثابت حرف *ثابت color_terms[] = {
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
	عدم
};

#ifndef HAVE_STRLCPY
ساكن size_t
strlcat(حرف *dst, ثابت حرف *src, size_t size)
{
	حرف *d = dst;
	ثابت حرف *s = src;
	size_t src_n = size;
	size_t dst_n;

	طالما (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	𐡄𐡍 (src_n == 0)
		أرجع dst_n + strlen(src);

	طالما (*s != '\0') {
		𐡄𐡍 (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	أرجع dst_n + (s - src);
}
#endif

ساكن bool
yesno(ثابت حرف *value)
{
	𐡄𐡍 (!value) {
		errno = ENOENT;
		أرجع false;
	}

	𐡄𐡍 (strcasecmp(value, "yes") == 0 ||
	    strcasecmp(value, "y") == 0 ||
	    strcasecmp(value, "true") == 0 ||
	    strcasecmp(value, "on") == 0 ||
	    strcasecmp(value, "1") == 0)
		أرجع true;

	𐡄𐡍 (strcasecmp(value, "no") != 0 &&
	    strcasecmp(value, "n") != 0 &&
	    strcasecmp(value, "false") != 0 &&
	    strcasecmp(value, "off") != 0 &&
	    strcasecmp(value, "0") != 0)
		errno = EINVAL;

	أرجع false;
}

ساكن bool
noyes(ثابت حرف *value)
{
	صحيح serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(value);
	𐡄𐡍 (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	أرجع retval;
}

ساكن bool
is_quiet(فراغ)
{
	أرجع yesno(getenv("EINFO_QUIET"));
}

ساكن bool
is_really_quiet(فراغ)
{
	أرجع yesno(getenv("EERROR_QUIET"));
}

ساكن bool
is_verbose(فراغ)
{
	أرجع yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
ساكن حرف *
tgoto(ثابت حرف *cap, صحيح col, صحيح line)
{
	ساكن حرف buf[20];
	حرف *p, *e, c, dbuf[6];
	صحيح oncol = 0, which = line, i;

	p = buf;
	e = p + حجم(buf);
	طالما ((c = *cap++)) {
		𐡄𐡍 (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			𐡄𐡍 (p >= e) {
				errno = E2BIG;
				أرجع عدم;
			}
			استمر;
		}
		بدل (c) {
		حالة '3':
		حالة '2':
		حالة 'd':
			i = 0;
			𐡏𐡁𐡃
				dbuf[i++] = which % 10 | '0';
			طالما ((which /= 10));
			𐡄𐡍 (c != 'd') {
				c -= '0';
				𐡄𐡍 (i > c) {
					errno = EINVAL;
					أرجع عدم;
				}
				طالما (i < c)
					dbuf[i++] = '0';
			}
			𐡄𐡍 (p + i >= e) {
				errno = E2BIG;
				أرجع عدم;
			}
			𐡏𐡁𐡃
				*p++ = dbuf[--i];
			طالما (i);
			اكسر;
		حالة 'r':
			oncol = 0;
			اكسر;
		حالة 'i':
			col++;
			line++;
			which++;
			استمر;
		افتراضي:
			errno = EINVAL;
			أرجع عدم;
		}

		oncol = 1 - oncol;
		which = oncol ? col : line;
	}
	*p = '\0';
	أرجع buf;
}

ساكن bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	ساكن صحيح in_colour = -1;
	حرف *e, *ee, *end, *d, *p;
	صحيح c;
	ثابت حرف *_af = عدم, *_ce = عدم, *_ch = عدم;
	ثابت حرف *_md = عدم, *_me = عدم, *_up = عدم;
	ثابت حرف *bold;
	حرف tmp[100];
	unsigned صحيح i = 0;

	𐡄𐡍 (f && !isatty(fileno(f)))
		أرجع false;

	𐡄𐡍 (noyes(getenv("EINFO_COLOR")))
		أرجع false;

	𐡄𐡍 (in_colour == 0)
		أرجع false;
	𐡄𐡍 (in_colour == 1)
		أرجع true;

	term_is_cons25 = false;
	𐡄𐡍 (!term) {
		term = getenv("TERM");
		𐡄𐡍 (!term)
			أرجع false;
	}
	𐡄𐡍 (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	𐡄𐡍 (strstr(term, "color"))
		in_colour = 1;

	طالما (color_terms[i] && in_colour != 1) {
		𐡄𐡍 (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	𐡄𐡍 (in_colour != 1) {
		in_colour = 0;
		أرجع false;
	}

	𐡄𐡍 (!_af)
		_af = AF;
	𐡄𐡍 (!_ce)
		_ce = CE;
	𐡄𐡍 (!_ch)
		_ch = CH;
	𐡄𐡍 (!_md)
		_md = MD;
	𐡄𐡍 (!_me)
		_me = ME;
	𐡄𐡍 (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	لكل (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		𐡄𐡍 (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			𐡄𐡍 ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				𐡄𐡍 (ee)
					ee += strlen(ecolors[i].name);

				𐡄𐡍 (ee && *ee == '=') {
					d = strdup(ee + 1);
					𐡄𐡍 (d) {
						end = strchr(d, ':');
						𐡄𐡍 (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						𐡄𐡍 (end && *++end == '0')
							bold = _me;
						حرر(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), حجم(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), حجم(tmp));
		} وإلا
			_GET_CAP(tmp, _me);

		𐡄𐡍 (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		وإلا
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, حجم(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	أرجع true;
}

ساكن صحيح
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	بنية winsize ws;
	حرف *env = getenv("COLUMNS");
	حرف *p;
	صحيح i;

	𐡄𐡍 (env) {
		i = strtoimax(env, &p, 10);
		𐡄𐡍 (!*p)
			أرجع i;
	}

	𐡄𐡍 (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		أرجع ws.ws_col;

	أرجع DEFAULT_COLS;
}

فراغ
eprefix(ثابت حرف *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

ساكن فراغ EINFO_PRINTF(2, 0)
elogv(صحيح level, ثابت حرف *EINFO_RESTRICT fmt, va_list ap)
{
	حرف *e = getenv("EINFO_LOG");
	va_list apc;

	𐡄𐡍 (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

فراغ
elog(صحيح level, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

ساكن صحيح
_eindent(FILE * EINFO_RESTRICT stream)
{
	حرف *env = getenv("EINFO_INDENT");
	صحيح amount = 0;

	𐡄𐡍 (env) {
		errno = 0;
		amount = strtoimax(env, عدم, 0);
		𐡄𐡍 (errno != 0 || amount < 0)
			amount = 0;
		وإلا 𐡄𐡍 (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	أرجع fprintf(stream, "%*s", amount, "");
}

ساكن ثابت حرف *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned صحيح i;

	𐡄𐡍 (!colour_terminal(f))
		أرجع "";

	لكل (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		𐡄𐡍 (ecolors[i].color == color)
			أرجع ecolors_str[i];
	أرجع "";
}

ثابت حرف *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	𐡄𐡍 (!isatty(fileno(f))) {
		f = stderr;
		𐡄𐡍 (!isatty(fileno(f))) {
			f = stdin;
			𐡄𐡍 (!isatty(fileno(f)))
				f = عدم;
		}
	}

	أرجع _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

ساكن صحيح EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, ثابت حرف *EINFO_RESTRICT fmt, va_list va)
{
	صحيح retval = 0;
	حرف *last = getenv("EINFO_LASTCMD");
	va_list ap;

	𐡄𐡍 (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	𐡄𐡍 (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	𐡄𐡍 (colour_terminal(f))
		fprintf(f, "%s", flush);
	أرجع retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

صحيح
einfon(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	𐡄𐡍 (!fmt || is_quiet())
		أرجع 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	أرجع retval;
}

صحيح
ewarnn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	𐡄𐡍 (!fmt || is_quiet())
		أرجع 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	أرجع retval;
}

صحيح
eerrorn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	𐡄𐡍 (!fmt || is_really_quiet())
		أرجع 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	أرجع retval;
}

صحيح
einfo(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	𐡄𐡍 (!fmt || is_quiet())
		أرجع 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	أرجع retval;
}

صحيح
ewarn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	𐡄𐡍 (!fmt || is_quiet())
		أرجع 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	أرجع retval;
}

فراغ
ewarnx(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	𐡄𐡍 (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

صحيح
eerror(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	𐡄𐡍 (!fmt || is_really_quiet())
		أرجع 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	أرجع retval;
}

فراغ
eerrorx(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	𐡄𐡍 (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

صحيح
ebegin(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	𐡄𐡍 (!fmt || is_quiet())
		أرجع 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	𐡄𐡍 (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	أرجع retval;
}

ساكن فراغ
_eend(FILE * EINFO_RESTRICT fp, صحيح col, ECOLOR color, ثابت حرف *msg)
{
	صحيح i;
	صحيح cols;

	𐡄𐡍 (!msg)
		أرجع;

	cols = get_term_columns(fp) - (strlen(msg) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	𐡄𐡍 (!term) {
		term = getenv("TERM");
		𐡄𐡍 (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		وإلا
			term_is_cons25 = false;
	}
	𐡄𐡍 (term_is_cons25)
		cols--;

	𐡄𐡍 (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), msg,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} وإلا {
		𐡄𐡍 (col > 0)
			لكل (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", msg);
	}
}

ساكن صحيح EINFO_PRINTF(3, 0)
_do_eend(ثابت حرف *cmd, صحيح retval,
    ثابت حرف *EINFO_RESTRICT fmt, va_list ap)
{
	صحيح col = 0;
	FILE *fp = stdout;
	va_list apc;

	𐡄𐡍 (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		𐡄𐡍 (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		وإلا
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	أرجع retval;
}

صحيح
eend(صحيح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	𐡄𐡍 (is_quiet())
		أرجع retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	أرجع retval;
}

صحيح
ewend(صحيح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	𐡄𐡍 (is_quiet())
		أرجع retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	أرجع retval;
}

فراغ
ebracket(صحيح col, ECOLOR color, ثابت حرف *msg)
{
	_eend(stdout, col, color, msg);
}

فراغ
eindent(فراغ)
{
	حرف *env = getenv("EINFO_INDENT");
	صحيح amount = 0;
	حرف *num;

	𐡄𐡍 (env) {
		errno = 0;
		amount = strtoimax(env, عدم, 0);
		𐡄𐡍 (errno != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
	xasprintf(&num, "%08d", amount);
	setenv("EINFO_INDENT", num, 1);
	حرر(num);
}

فراغ eoutdent(فراغ)
{
	حرف *env = getenv("EINFO_INDENT");
	صحيح amount = 0;
	حرف *num = عدم;
	صحيح serrno = errno;

	𐡄𐡍 (!env)
		أرجع;
	errno = 0;
	amount = strtoimax(env, عدم, 0);
	𐡄𐡍 (errno != 0)
		amount = 0;
	وإلا
		amount -= INDENT_WIDTH;
	𐡄𐡍 (amount <= 0)
		unsetenv("EINFO_INDENT");
	وإلا {
		xasprintf(&num, "%08d", amount);
		setenv("EINFO_INDENT", num, 1);
		حرر(num);
	}
	errno = serrno;
}

صحيح
einfovn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	𐡄𐡍 (!fmt || !is_verbose())
		أرجع 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	أرجع retval;
}

صحيح
ewarnvn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	𐡄𐡍 (!fmt || !is_verbose())
		أرجع 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	أرجع retval;
}

صحيح
einfov(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	𐡄𐡍 (!fmt || !is_verbose())
		أرجع 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	أرجع retval;
}

صحيح
ewarnv(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	𐡄𐡍 (!fmt || !is_verbose())
		أرجع 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	أرجع retval;
}

صحيح
ebeginv(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	𐡄𐡍 (!fmt || !is_verbose())
		أرجع 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	𐡄𐡍 (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	أرجع retval;
}

صحيح
eendv(صحيح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	𐡄𐡍 (!is_verbose())
		أرجع 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	أرجع retval;
}

صحيح
ewendv(صحيح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	𐡄𐡍 (!is_verbose())
		أرجع 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	أرجع retval;
}

فراغ
eindentv(فراغ)
{
	𐡄𐡍 (is_verbose())
		eindent();
}

فراغ
eoutdentv(فراغ)
{
	𐡄𐡍 (is_verbose())
		eoutdent();
}
