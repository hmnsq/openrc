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

#شامل <sys/ioctl.h>
#شامل <errno.h>
#شامل <inttypes.h>
#شامل <stdarg.h>
#شامل <stdbool.h>
#شامل <stddef.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <syslog.h>
#شامل <unistd.h>

#شامل "einfo.h"
#شامل "helpers.h"

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

#define _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), اندازه(_d));
#define _ASSIGN_CAP(_v) انجامبده {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, اندازه(ebuffer) - (p - ebuffer)) + 1;    \
	} وقتی (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
ایستا ثابت نویسه *_eprefix = پوچ;

/* Buffers and structures to hold the final colours */
ایستا نویسه ebuffer[100];
ساختار ecolor {
	ECOLOR color;
	صحیح def;
	ثابت نویسه *name;
};
ایستا نویسه nullstr = '\0';

ایستا ثابت ساختار ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_NORMAL,  0,       پوچ      },
};
ایستا ثابت نویسه *ecolors_str[ARRAY_SIZE(ecolors)];

ایستا نویسه *flush = پوچ;
ایستا نویسه *up = پوچ;
ایستا نویسه *goto_column = پوچ;

ایستا ثابت نویسه *term = پوچ;
ایستا bool term_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
ایستا ثابت نویسه *ثابت color_terms[] = {
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
	پوچ
};

#ifndef HAVE_STRLCPY
ایستا size_t
strlcat(نویسه *dst, ثابت نویسه *src, size_t size)
{
	نویسه *d = dst;
	ثابت نویسه *s = src;
	size_t src_n = size;
	size_t dst_n;

	وقتی (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	اگر (src_n == 0)
		بازگشت dst_n + strlen(src);

	وقتی (*s != '\0') {
		اگر (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	بازگشت dst_n + (s - src);
}
#endif

ایستا bool
yesno(ثابت نویسه *مقدار)
{
	اگر (!مقدار) {
		errno = ENOENT;
		بازگشت false;
	}

	اگر (strcasecmp(مقدار, "yes") == 0 ||
	    strcasecmp(مقدار, "y") == 0 ||
	    strcasecmp(مقدار, "true") == 0 ||
	    strcasecmp(مقدار, "on") == 0 ||
	    strcasecmp(مقدار, "1") == 0)
		بازگشت true;

	اگر (strcasecmp(مقدار, "no") != 0 &&
	    strcasecmp(مقدار, "n") != 0 &&
	    strcasecmp(مقدار, "false") != 0 &&
	    strcasecmp(مقدار, "off") != 0 &&
	    strcasecmp(مقدار, "0") != 0)
		errno = EINVAL;

	بازگشت false;
}

ایستا bool
noyes(ثابت نویسه *مقدار)
{
	صحیح serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(مقدار);
	اگر (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	بازگشت retval;
}

ایستا bool
is_quiet(تهی)
{
	بازگشت yesno(getenv("EINFO_QUIET"));
}

ایستا bool
is_really_quiet(تهی)
{
	بازگشت yesno(getenv("EERROR_QUIET"));
}

ایستا bool
is_verbose(تهی)
{
	بازگشت yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
ایستا نویسه *
tgoto(ثابت نویسه *cap, صحیح col, صحیح سطر)
{
	ایستا نویسه buf[20];
	نویسه *p, *e, c, dbuf[6];
	صحیح oncol = 0, which = سطر, i;

	p = buf;
	e = p + اندازه(buf);
	وقتی ((c = *cap++)) {
		اگر (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			اگر (p >= e) {
				errno = E2BIG;
				بازگشت پوچ;
			}
			ادامه;
		}
		انتخاب (c) {
		حالت '3':
		حالت '2':
		حالت 'd':
			i = 0;
			انجامبده
				dbuf[i++] = which % 10 | '0';
			وقتی ((which /= 10));
			اگر (c != 'd') {
				c -= '0';
				اگر (i > c) {
					errno = EINVAL;
					بازگشت پوچ;
				}
				وقتی (i < c)
					dbuf[i++] = '0';
			}
			اگر (p + i >= e) {
				errno = E2BIG;
				بازگشت پوچ;
			}
			انجامبده
				*p++ = dbuf[--i];
			وقتی (i);
			بشکن;
		حالت 'r':
			oncol = 0;
			بشکن;
		حالت 'i':
			col++;
			سطر++;
			which++;
			ادامه;
		پیشفرض:
			errno = EINVAL;
			بازگشت پوچ;
		}

		oncol = 1 - oncol;
		which = oncol ? col : سطر;
	}
	*p = '\0';
	بازگشت buf;
}

ایستا bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	ایستا صحیح in_colour = -1;
	نویسه *e, *ee, *end, *d, *p;
	صحیح c;
	ثابت نویسه *_af = پوچ, *_ce = پوچ, *_ch = پوچ;
	ثابت نویسه *_md = پوچ, *_me = پوچ, *_up = پوچ;
	ثابت نویسه *bold;
	نویسه tmp[100];
	unsigned صحیح i = 0;

	اگر (f && !isatty(fileno(f)))
		بازگشت false;

	اگر (noyes(getenv("EINFO_COLOR")))
		بازگشت false;

	اگر (in_colour == 0)
		بازگشت false;
	اگر (in_colour == 1)
		بازگشت true;

	term_is_cons25 = false;
	اگر (!term) {
		term = getenv("TERM");
		اگر (!term)
			بازگشت false;
	}
	اگر (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	اگر (strstr(term, "color"))
		in_colour = 1;

	وقتی (color_terms[i] && in_colour != 1) {
		اگر (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	اگر (in_colour != 1) {
		in_colour = 0;
		بازگشت false;
	}

	اگر (!_af)
		_af = AF;
	اگر (!_ce)
		_ce = CE;
	اگر (!_ch)
		_ch = CH;
	اگر (!_md)
		_md = MD;
	اگر (!_me)
		_me = ME;
	اگر (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	برای (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		اگر (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			اگر ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				اگر (ee)
					ee += strlen(ecolors[i].name);

				اگر (ee && *ee == '=') {
					d = strdup(ee + 1);
					اگر (d) {
						end = strchr(d, ':');
						اگر (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						اگر (end && *++end == '0')
							bold = _me;
						آزاد(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), اندازه(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), اندازه(tmp));
		} وگرنه
			_GET_CAP(tmp, _me);

		اگر (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		وگرنه
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, اندازه(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	بازگشت true;
}

ایستا صحیح
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	ساختار winsize ws;
	نویسه *env = getenv("COLUMNS");
	نویسه *p;
	صحیح i;

	اگر (env) {
		i = strtoimax(env, &p, 10);
		اگر (!*p)
			بازگشت i;
	}

	اگر (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		بازگشت ws.ws_col;

	بازگشت DEFAULT_COLS;
}

تهی
eprefix(ثابت نویسه *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

ایستا تهی EINFO_PRINTF(2, 0)
elogv(صحیح level, ثابت نویسه *EINFO_RESTRICT fmt, va_list ap)
{
	نویسه *e = getenv("EINFO_LOG");
	va_list apc;

	اگر (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

تهی
elog(صحیح level, ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

ایستا صحیح
_eindent(FILE * EINFO_RESTRICT stream)
{
	نویسه *env = getenv("EINFO_INDENT");
	صحیح amount = 0;

	اگر (env) {
		errno = 0;
		amount = strtoimax(env, پوچ, 0);
		اگر (errno != 0 || amount < 0)
			amount = 0;
		وگرنه اگر (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	بازگشت fprintf(stream, "%*s", amount, "");
}

ایستا ثابت نویسه *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned صحیح i;

	اگر (!colour_terminal(f))
		بازگشت "";

	برای (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		اگر (ecolors[i].color == color)
			بازگشت ecolors_str[i];
	بازگشت "";
}

ثابت نویسه *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	اگر (!isatty(fileno(f))) {
		f = stderr;
		اگر (!isatty(fileno(f))) {
			f = stdin;
			اگر (!isatty(fileno(f)))
				f = پوچ;
		}
	}

	بازگشت _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

ایستا صحیح EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, ثابت نویسه *EINFO_RESTRICT fmt, va_list va)
{
	صحیح retval = 0;
	نویسه *last = getenv("EINFO_LASTCMD");
	va_list ap;

	اگر (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	اگر (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	اگر (colour_terminal(f))
		fprintf(f, "%s", flush);
	بازگشت retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

صحیح
einfon(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_quiet())
		بازگشت 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	بازگشت retval;
}

صحیح
ewarnn(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_quiet())
		بازگشت 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	بازگشت retval;
}

صحیح
eerrorn(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_really_quiet())
		بازگشت 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	بازگشت retval;
}

صحیح
einfo(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_quiet())
		بازگشت 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	بازگشت retval;
}

صحیح
ewarn(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_quiet())
		بازگشت 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	بازگشت retval;
}

تهی
ewarnx(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	اگر (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

صحیح
eerror(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_really_quiet())
		بازگشت 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	بازگشت retval;
}

تهی
eerrorx(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	اگر (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

صحیح
ebegin(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_quiet())
		بازگشت 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	اگر (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	بازگشت retval;
}

ایستا تهی
_eend(FILE * EINFO_RESTRICT fp, صحیح col, ECOLOR color, ثابت نویسه *پیام)
{
	صحیح i;
	صحیح cols;

	اگر (!پیام)
		بازگشت;

	cols = get_term_columns(fp) - (strlen(پیام) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	اگر (!term) {
		term = getenv("TERM");
		اگر (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		وگرنه
			term_is_cons25 = false;
	}
	اگر (term_is_cons25)
		cols--;

	اگر (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), پیام,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} وگرنه {
		اگر (col > 0)
			برای (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", پیام);
	}
}

ایستا صحیح EINFO_PRINTF(3, 0)
_do_eend(ثابت نویسه *cmd, صحیح retval,
    ثابت نویسه *EINFO_RESTRICT fmt, va_list ap)
{
	صحیح col = 0;
	FILE *fp = stdout;
	va_list apc;

	اگر (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		اگر (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		وگرنه
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	بازگشت retval;
}

صحیح
eend(صحیح retval, ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	اگر (is_quiet())
		بازگشت retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	بازگشت retval;
}

صحیح
ewend(صحیح retval, ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	اگر (is_quiet())
		بازگشت retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	بازگشت retval;
}

تهی
ebracket(صحیح col, ECOLOR color, ثابت نویسه *پیام)
{
	_eend(stdout, col, color, پیام);
}

تهی
eindent(تهی)
{
	نویسه *env = getenv("EINFO_INDENT");
	صحیح amount = 0;
	نویسه *رقم;

	اگر (env) {
		errno = 0;
		amount = strtoimax(env, پوچ, 0);
		اگر (errno != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
	xasprintf(&رقم, "%08d", amount);
	setenv("EINFO_INDENT", رقم, 1);
	آزاد(رقم);
}

تهی eoutdent(تهی)
{
	نویسه *env = getenv("EINFO_INDENT");
	صحیح amount = 0;
	نویسه *رقم = پوچ;
	صحیح serrno = errno;

	اگر (!env)
		بازگشت;
	errno = 0;
	amount = strtoimax(env, پوچ, 0);
	اگر (errno != 0)
		amount = 0;
	وگرنه
		amount -= INDENT_WIDTH;
	اگر (amount <= 0)
		unsetenv("EINFO_INDENT");
	وگرنه {
		xasprintf(&رقم, "%08d", amount);
		setenv("EINFO_INDENT", رقم, 1);
		آزاد(رقم);
	}
	errno = serrno;
}

صحیح
einfovn(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || !is_verbose())
		بازگشت 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	بازگشت retval;
}

صحیح
ewarnvn(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || !is_verbose())
		بازگشت 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	بازگشت retval;
}

صحیح
einfov(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || !is_verbose())
		بازگشت 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	بازگشت retval;
}

صحیح
ewarnv(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || !is_verbose())
		بازگشت 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	بازگشت retval;
}

صحیح
ebeginv(ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || !is_verbose())
		بازگشت 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	اگر (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	بازگشت retval;
}

صحیح
eendv(صحیح retval, ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	اگر (!is_verbose())
		بازگشت 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	بازگشت retval;
}

صحیح
ewendv(صحیح retval, ثابت نویسه *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	اگر (!is_verbose())
		بازگشت 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	بازگشت retval;
}

تهی
eindentv(تهی)
{
	اگر (is_verbose())
		eindent();
}

تهی
eoutdentv(تهی)
{
	اگر (is_verbose())
		eoutdent();
}
