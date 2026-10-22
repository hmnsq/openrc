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
#define _ASSIGN_CAP(_v) ܥܒܕ {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, حجم(ebuffer) - (p - ebuffer)) + 1;    \
	} ܟܡܐ (0)

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

	ܟܡܐ (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	ܐܢ (src_n == 0)
		ܦܢܐ dst_n + strlen(src);

	ܟܡܐ (*s != '\0') {
		ܐܢ (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	ܦܢܐ dst_n + (s - src);
}
#endif

ساكن bool
yesno(ثابت حرف *value)
{
	ܐܢ (!value) {
		errno = ENOENT;
		ܦܢܐ false;
	}

	ܐܢ (strcasecmp(value, "yes") == 0 ||
	    strcasecmp(value, "y") == 0 ||
	    strcasecmp(value, "true") == 0 ||
	    strcasecmp(value, "on") == 0 ||
	    strcasecmp(value, "1") == 0)
		ܦܢܐ true;

	ܐܢ (strcasecmp(value, "no") != 0 &&
	    strcasecmp(value, "n") != 0 &&
	    strcasecmp(value, "false") != 0 &&
	    strcasecmp(value, "off") != 0 &&
	    strcasecmp(value, "0") != 0)
		errno = EINVAL;

	ܦܢܐ false;
}

ساكن bool
noyes(ثابت حرف *value)
{
	صحيح serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(value);
	ܐܢ (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	ܦܢܐ retval;
}

ساكن bool
is_quiet(فراغ)
{
	ܦܢܐ yesno(getenv("EINFO_QUIET"));
}

ساكن bool
is_really_quiet(فراغ)
{
	ܦܢܐ yesno(getenv("EERROR_QUIET"));
}

ساكن bool
is_verbose(فراغ)
{
	ܦܢܐ yesno(getenv ("EINFO_VERBOSE"));
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
	ܟܡܐ ((c = *cap++)) {
		ܐܢ (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			ܐܢ (p >= e) {
				errno = E2BIG;
				ܦܢܐ عدم;
			}
			استمر;
		}
		بدل (c) {
		حالة '3':
		حالة '2':
		حالة 'd':
			i = 0;
			ܥܒܕ
				dbuf[i++] = which % 10 | '0';
			ܟܡܐ ((which /= 10));
			ܐܢ (c != 'd') {
				c -= '0';
				ܐܢ (i > c) {
					errno = EINVAL;
					ܦܢܐ عدم;
				}
				ܟܡܐ (i < c)
					dbuf[i++] = '0';
			}
			ܐܢ (p + i >= e) {
				errno = E2BIG;
				ܦܢܐ عدم;
			}
			ܥܒܕ
				*p++ = dbuf[--i];
			ܟܡܐ (i);
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
			ܦܢܐ عدم;
		}

		oncol = 1 - oncol;
		which = oncol ? col : line;
	}
	*p = '\0';
	ܦܢܐ buf;
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

	ܐܢ (f && !isatty(fileno(f)))
		ܦܢܐ false;

	ܐܢ (noyes(getenv("EINFO_COLOR")))
		ܦܢܐ false;

	ܐܢ (in_colour == 0)
		ܦܢܐ false;
	ܐܢ (in_colour == 1)
		ܦܢܐ true;

	term_is_cons25 = false;
	ܐܢ (!term) {
		term = getenv("TERM");
		ܐܢ (!term)
			ܦܢܐ false;
	}
	ܐܢ (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	ܐܢ (strstr(term, "color"))
		in_colour = 1;

	ܟܡܐ (color_terms[i] && in_colour != 1) {
		ܐܢ (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	ܐܢ (in_colour != 1) {
		in_colour = 0;
		ܦܢܐ false;
	}

	ܐܢ (!_af)
		_af = AF;
	ܐܢ (!_ce)
		_ce = CE;
	ܐܢ (!_ch)
		_ch = CH;
	ܐܢ (!_md)
		_md = MD;
	ܐܢ (!_me)
		_me = ME;
	ܐܢ (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	لكل (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		ܐܢ (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			ܐܢ ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				ܐܢ (ee)
					ee += strlen(ecolors[i].name);

				ܐܢ (ee && *ee == '=') {
					d = strdup(ee + 1);
					ܐܢ (d) {
						end = strchr(d, ':');
						ܐܢ (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						ܐܢ (end && *++end == '0')
							bold = _me;
						حرر(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), حجم(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), حجم(tmp));
		} ܐܚܪܢܐ
			_GET_CAP(tmp, _me);

		ܐܢ (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		ܐܚܪܢܐ
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, حجم(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	ܦܢܐ true;
}

ساكن صحيح
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	بنية winsize ws;
	حرف *env = getenv("COLUMNS");
	حرف *p;
	صحيح i;

	ܐܢ (env) {
		i = strtoimax(env, &p, 10);
		ܐܢ (!*p)
			ܦܢܐ i;
	}

	ܐܢ (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		ܦܢܐ ws.ws_col;

	ܦܢܐ DEFAULT_COLS;
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

	ܐܢ (fmt && e) {
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

	ܐܢ (env) {
		errno = 0;
		amount = strtoimax(env, عدم, 0);
		ܐܢ (errno != 0 || amount < 0)
			amount = 0;
		ܐܚܪܢܐ ܐܢ (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	ܦܢܐ fprintf(stream, "%*s", amount, "");
}

ساكن ثابت حرف *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned صحيح i;

	ܐܢ (!colour_terminal(f))
		ܦܢܐ "";

	لكل (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		ܐܢ (ecolors[i].color == color)
			ܦܢܐ ecolors_str[i];
	ܦܢܐ "";
}

ثابت حرف *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	ܐܢ (!isatty(fileno(f))) {
		f = stderr;
		ܐܢ (!isatty(fileno(f))) {
			f = stdin;
			ܐܢ (!isatty(fileno(f)))
				f = عدم;
		}
	}

	ܦܢܐ _ecolor(f, color);
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

	ܐܢ (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	ܐܢ (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	ܐܢ (colour_terminal(f))
		fprintf(f, "%s", flush);
	ܦܢܐ retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

صحيح
einfon(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	ܐܢ (!fmt || is_quiet())
		ܦܢܐ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	ܦܢܐ retval;
}

صحيح
ewarnn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	ܐܢ (!fmt || is_quiet())
		ܦܢܐ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	ܦܢܐ retval;
}

صحيح
eerrorn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	ܐܢ (!fmt || is_really_quiet())
		ܦܢܐ 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	ܦܢܐ retval;
}

صحيح
einfo(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	ܐܢ (!fmt || is_quiet())
		ܦܢܐ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	ܦܢܐ retval;
}

صحيح
ewarn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	ܐܢ (!fmt || is_quiet())
		ܦܢܐ 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	ܦܢܐ retval;
}

فراغ
ewarnx(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ܐܢ (fmt && !is_quiet()) {
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

	ܐܢ (!fmt || is_really_quiet())
		ܦܢܐ 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	ܦܢܐ retval;
}

فراغ
eerrorx(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ܐܢ (fmt && !is_really_quiet()) {
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

	ܐܢ (!fmt || is_quiet())
		ܦܢܐ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	ܐܢ (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	ܦܢܐ retval;
}

ساكن فراغ
_eend(FILE * EINFO_RESTRICT fp, صحيح col, ECOLOR color, ثابت حرف *msg)
{
	صحيح i;
	صحيح cols;

	ܐܢ (!msg)
		ܦܢܐ;

	cols = get_term_columns(fp) - (strlen(msg) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	ܐܢ (!term) {
		term = getenv("TERM");
		ܐܢ (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		ܐܚܪܢܐ
			term_is_cons25 = false;
	}
	ܐܢ (term_is_cons25)
		cols--;

	ܐܢ (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), msg,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} ܐܚܪܢܐ {
		ܐܢ (col > 0)
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

	ܐܢ (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		ܐܢ (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		ܐܚܪܢܐ
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	ܦܢܐ retval;
}

صحيح
eend(صحيح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ܐܢ (is_quiet())
		ܦܢܐ retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	ܦܢܐ retval;
}

صحيح
ewend(صحيح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ܐܢ (is_quiet())
		ܦܢܐ retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	ܦܢܐ retval;
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

	ܐܢ (env) {
		errno = 0;
		amount = strtoimax(env, عدم, 0);
		ܐܢ (errno != 0)
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

	ܐܢ (!env)
		ܦܢܐ;
	errno = 0;
	amount = strtoimax(env, عدم, 0);
	ܐܢ (errno != 0)
		amount = 0;
	ܐܚܪܢܐ
		amount -= INDENT_WIDTH;
	ܐܢ (amount <= 0)
		unsetenv("EINFO_INDENT");
	ܐܚܪܢܐ {
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

	ܐܢ (!fmt || !is_verbose())
		ܦܢܐ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	ܦܢܐ retval;
}

صحيح
ewarnvn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	ܐܢ (!fmt || !is_verbose())
		ܦܢܐ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	ܦܢܐ retval;
}

صحيح
einfov(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	ܐܢ (!fmt || !is_verbose())
		ܦܢܐ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	ܦܢܐ retval;
}

صحيح
ewarnv(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	ܐܢ (!fmt || !is_verbose())
		ܦܢܐ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	ܦܢܐ retval;
}

صحيح
ebeginv(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	ܐܢ (!fmt || !is_verbose())
		ܦܢܐ 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	ܐܢ (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	ܦܢܐ retval;
}

صحيح
eendv(صحيح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ܐܢ (!is_verbose())
		ܦܢܐ 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	ܦܢܐ retval;
}

صحيح
ewendv(صحيح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ܐܢ (!is_verbose())
		ܦܢܐ 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	ܦܢܐ retval;
}

فراغ
eindentv(فراغ)
{
	ܐܢ (is_verbose())
		eindent();
}

فراغ
eoutdentv(فراغ)
{
	ܐܢ (is_verbose())
		eoutdent();
}
