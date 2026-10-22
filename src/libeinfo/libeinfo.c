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

#অন্তর্ভাব <sys/ioctl.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <inttypes.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>
#অন্তর্ভাব <syslog.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "helpers.h"

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

#define _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), আকার(_d));
#define _ASSIGN_CAP(_v) করো {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, আকার(ebuffer) - (p - ebuffer)) + 1;    \
	} যখন (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
স্থির ধ্রুবক অক্ষর *_eprefix = ফাঁকা;

/* Buffers and structures to hold the final colours */
স্থির অক্ষর ebuffer[100];
কাঠামো ecolor {
	ECOLOR color;
	পূর্ণসংখ্যা def;
	ধ্রুবক অক্ষর *name;
};
স্থির অক্ষর nullstr = '\0';

স্থির ধ্রুবক কাঠামো ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_NORMAL,  0,       ফাঁকা      },
};
স্থির ধ্রুবক অক্ষর *ecolors_str[ARRAY_আকার(ecolors)];

স্থির অক্ষর *flush = ফাঁকা;
স্থির অক্ষর *up = ফাঁকা;
স্থির অক্ষর *goto_column = ফাঁকা;

স্থির ধ্রুবক অক্ষর *term = ফাঁকা;
স্থির bool term_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
স্থির ধ্রুবক অক্ষর *ধ্রুবক color_terms[] = {
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
	ফাঁকা
};

#ifndef HAVE_STRLCPY
স্থির size_t
strlcat(অক্ষর *dst, ধ্রুবক অক্ষর *src, size_t আকার_2)
{
	অক্ষর *d = dst;
	ধ্রুবক অক্ষর *s = src;
	size_t src_n = আকার_2;
	size_t dst_n;

	যখন (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = আকার_2 - dst_n;

	যদি (src_n == 0)
		ফেরত dst_n + strlen(src);

	যখন (*s != '\0') {
		যদি (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	ফেরত dst_n + (s - src);
}
#endif

স্থির bool
yesno(ধ্রুবক অক্ষর *মান)
{
	যদি (!মান) {
		errno = ENOENT;
		ফেরত false;
	}

	যদি (strcasecmp(মান, "yes") == 0 ||
	    strcasecmp(মান, "y") == 0 ||
	    strcasecmp(মান, "true") == 0 ||
	    strcasecmp(মান, "on") == 0 ||
	    strcasecmp(মান, "1") == 0)
		ফেরত true;

	যদি (strcasecmp(মান, "no") != 0 &&
	    strcasecmp(মান, "n") != 0 &&
	    strcasecmp(মান, "false") != 0 &&
	    strcasecmp(মান, "off") != 0 &&
	    strcasecmp(মান, "0") != 0)
		errno = EINVAL;

	ফেরত false;
}

স্থির bool
noyes(ধ্রুবক অক্ষর *মান)
{
	পূর্ণসংখ্যা serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(মান);
	যদি (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	ফেরত retval;
}

স্থির bool
is_quiet(শূন্য)
{
	ফেরত yesno(getenv("EINFO_QUIET"));
}

স্থির bool
is_really_quiet(শূন্য)
{
	ফেরত yesno(getenv("EERROR_QUIET"));
}

স্থির bool
is_verbose(শূন্য)
{
	ফেরত yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
স্থির অক্ষর *
tgoto(ধ্রুবক অক্ষর *cap, পূর্ণসংখ্যা col, পূর্ণসংখ্যা সারি)
{
	স্থির অক্ষর buf[20];
	অক্ষর *p, *e, c, dbuf[6];
	পূর্ণসংখ্যা oncol = 0, which = সারি, i;

	p = buf;
	e = p + আকার(buf);
	যখন ((c = *cap++)) {
		যদি (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			যদি (p >= e) {
				errno = E2BIG;
				ফেরত ফাঁকা;
			}
			চালু;
		}
		নির্বাচন (c) {
		ক্ষেত্র '3':
		ক্ষেত্র '2':
		ক্ষেত্র 'd':
			i = 0;
			করো
				dbuf[i++] = which % 10 | '0';
			যখন ((which /= 10));
			যদি (c != 'd') {
				c -= '0';
				যদি (i > c) {
					errno = EINVAL;
					ফেরত ফাঁকা;
				}
				যখন (i < c)
					dbuf[i++] = '0';
			}
			যদি (p + i >= e) {
				errno = E2BIG;
				ফেরত ফাঁকা;
			}
			করো
				*p++ = dbuf[--i];
			যখন (i);
			বিরতি;
		ক্ষেত্র 'r':
			oncol = 0;
			বিরতি;
		ক্ষেত্র 'i':
			col++;
			সারি++;
			which++;
			চালু;
		পূর্বনির্ধারিত:
			errno = EINVAL;
			ফেরত ফাঁকা;
		}

		oncol = 1 - oncol;
		which = oncol ? col : সারি;
	}
	*p = '\0';
	ফেরত buf;
}

স্থির bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	স্থির পূর্ণসংখ্যা in_colour = -1;
	অক্ষর *e, *ee, *end, *d, *p;
	পূর্ণসংখ্যা c;
	ধ্রুবক অক্ষর *_af = ফাঁকা, *_ce = ফাঁকা, *_ch = ফাঁকা;
	ধ্রুবক অক্ষর *_md = ফাঁকা, *_me = ফাঁকা, *_up = ফাঁকা;
	ধ্রুবক অক্ষর *bold;
	অক্ষর tmp[100];
	unsigned পূর্ণসংখ্যা i = 0;

	যদি (f && !isatty(fileno(f)))
		ফেরত false;

	যদি (noyes(getenv("EINFO_COLOR")))
		ফেরত false;

	যদি (in_colour == 0)
		ফেরত false;
	যদি (in_colour == 1)
		ফেরত true;

	term_is_cons25 = false;
	যদি (!term) {
		term = getenv("TERM");
		যদি (!term)
			ফেরত false;
	}
	যদি (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	যদি (strstr(term, "color"))
		in_colour = 1;

	যখন (color_terms[i] && in_colour != 1) {
		যদি (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	যদি (in_colour != 1) {
		in_colour = 0;
		ফেরত false;
	}

	যদি (!_af)
		_af = AF;
	যদি (!_ce)
		_ce = CE;
	যদি (!_ch)
		_ch = CH;
	যদি (!_md)
		_md = MD;
	যদি (!_me)
		_me = ME;
	যদি (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	জন্য (i = 0; i < ARRAY_আকার(ecolors); ++i) {
		tmp[0] = '\0';
		যদি (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			যদি ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				যদি (ee)
					ee += strlen(ecolors[i].name);

				যদি (ee && *ee == '=') {
					d = strdup(ee + 1);
					যদি (d) {
						end = strchr(d, ':');
						যদি (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						যদি (end && *++end == '0')
							bold = _me;
						মুক্ত(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), আকার(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), আকার(tmp));
		} অন্যথা
			_GET_CAP(tmp, _me);

		যদি (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		অন্যথা
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, আকার(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	ফেরত true;
}

স্থির পূর্ণসংখ্যা
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	কাঠামো winsize ws;
	অক্ষর *env = getenv("COLUMNS");
	অক্ষর *p;
	পূর্ণসংখ্যা i;

	যদি (env) {
		i = strtoimax(env, &p, 10);
		যদি (!*p)
			ফেরত i;
	}

	যদি (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		ফেরত ws.ws_col;

	ফেরত DEFAULT_COLS;
}

শূন্য
eprefix(ধ্রুবক অক্ষর *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

স্থির শূন্য EINFO_PRINTF(2, 0)
elogv(পূর্ণসংখ্যা level, ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, va_list ap)
{
	অক্ষর *e = getenv("EINFO_LOG");
	va_list apc;

	যদি (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

শূন্য
elog(পূর্ণসংখ্যা level, ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

স্থির পূর্ণসংখ্যা
_eindent(FILE * EINFO_RESTRICT stream)
{
	অক্ষর *env = getenv("EINFO_INDENT");
	পূর্ণসংখ্যা amount = 0;

	যদি (env) {
		errno = 0;
		amount = strtoimax(env, ফাঁকা, 0);
		যদি (errno != 0 || amount < 0)
			amount = 0;
		অন্যথা যদি (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	ফেরত fprintf(stream, "%*s", amount, "");
}

স্থির ধ্রুবক অক্ষর *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned পূর্ণসংখ্যা i;

	যদি (!colour_terminal(f))
		ফেরত "";

	জন্য (i = 0; i < ARRAY_আকার(ecolors); ++i)
		যদি (ecolors[i].color == color)
			ফেরত ecolors_str[i];
	ফেরত "";
}

ধ্রুবক অক্ষর *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	যদি (!isatty(fileno(f))) {
		f = stderr;
		যদি (!isatty(fileno(f))) {
			f = stdin;
			যদি (!isatty(fileno(f)))
				f = ফাঁকা;
		}
	}

	ফেরত _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

স্থির পূর্ণসংখ্যা EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, va_list va)
{
	পূর্ণসংখ্যা retval = 0;
	অক্ষর *last = getenv("EINFO_LASTCMD");
	va_list ap;

	যদি (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	যদি (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	যদি (colour_terminal(f))
		fprintf(f, "%s", flush);
	ফেরত retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

পূর্ণসংখ্যা
einfon(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	পূর্ণসংখ্যা retval;
	va_list ap;

	যদি (!fmt || is_quiet())
		ফেরত 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	ফেরত retval;
}

পূর্ণসংখ্যা
ewarnn(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	পূর্ণসংখ্যা retval;
	va_list ap;

	যদি (!fmt || is_quiet())
		ফেরত 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	ফেরত retval;
}

পূর্ণসংখ্যা
eerrorn(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	পূর্ণসংখ্যা retval;
	va_list ap;

	যদি (!fmt || is_really_quiet())
		ফেরত 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	ফেরত retval;
}

পূর্ণসংখ্যা
einfo(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	পূর্ণসংখ্যা retval;
	va_list ap;

	যদি (!fmt || is_quiet())
		ফেরত 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	ফেরত retval;
}

পূর্ণসংখ্যা
ewarn(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	পূর্ণসংখ্যা retval;
	va_list ap;

	যদি (!fmt || is_quiet())
		ফেরত 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	ফেরত retval;
}

শূন্য
ewarnx(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	যদি (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

পূর্ণসংখ্যা
eerror(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	পূর্ণসংখ্যা retval;
	va_list ap;

	যদি (!fmt || is_really_quiet())
		ফেরত 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	ফেরত retval;
}

শূন্য
eerrorx(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	যদি (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

পূর্ণসংখ্যা
ebegin(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	পূর্ণসংখ্যা retval;
	va_list ap;

	যদি (!fmt || is_quiet())
		ফেরত 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	যদি (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	ফেরত retval;
}

স্থির শূন্য
_eend(FILE * EINFO_RESTRICT fp, পূর্ণসংখ্যা col, ECOLOR color, ধ্রুবক অক্ষর *বার্তা)
{
	পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা cols;

	যদি (!বার্তা)
		ফেরত;

	cols = get_term_columns(fp) - (strlen(বার্তা) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	যদি (!term) {
		term = getenv("TERM");
		যদি (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		অন্যথা
			term_is_cons25 = false;
	}
	যদি (term_is_cons25)
		cols--;

	যদি (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), বার্তা,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} অন্যথা {
		যদি (col > 0)
			জন্য (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", বার্তা);
	}
}

স্থির পূর্ণসংখ্যা EINFO_PRINTF(3, 0)
_do_eend(ধ্রুবক অক্ষর *cmd, পূর্ণসংখ্যা retval,
    ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, va_list ap)
{
	পূর্ণসংখ্যা col = 0;
	FILE *fp = stdout;
	va_list apc;

	যদি (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		যদি (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		অন্যথা
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	ফেরত retval;
}

পূর্ণসংখ্যা
eend(পূর্ণসংখ্যা retval, ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	যদি (is_quiet())
		ফেরত retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	ফেরত retval;
}

পূর্ণসংখ্যা
ewend(পূর্ণসংখ্যা retval, ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	যদি (is_quiet())
		ফেরত retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	ফেরত retval;
}

শূন্য
ebracket(পূর্ণসংখ্যা col, ECOLOR color, ধ্রুবক অক্ষর *বার্তা)
{
	_eend(stdout, col, color, বার্তা);
}

শূন্য
eindent(শূন্য)
{
	অক্ষর *env = getenv("EINFO_INDENT");
	পূর্ণসংখ্যা amount = 0;
	অক্ষর *অঙ্ক;

	যদি (env) {
		errno = 0;
		amount = strtoimax(env, ফাঁকা, 0);
		যদি (errno != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
	xasprintf(&অঙ্ক, "%08d", amount);
	setenv("EINFO_INDENT", অঙ্ক, 1);
	মুক্ত(অঙ্ক);
}

শূন্য eoutdent(শূন্য)
{
	অক্ষর *env = getenv("EINFO_INDENT");
	পূর্ণসংখ্যা amount = 0;
	অক্ষর *অঙ্ক = ফাঁকা;
	পূর্ণসংখ্যা serrno = errno;

	যদি (!env)
		ফেরত;
	errno = 0;
	amount = strtoimax(env, ফাঁকা, 0);
	যদি (errno != 0)
		amount = 0;
	অন্যথা
		amount -= INDENT_WIDTH;
	যদি (amount <= 0)
		unsetenv("EINFO_INDENT");
	অন্যথা {
		xasprintf(&অঙ্ক, "%08d", amount);
		setenv("EINFO_INDENT", অঙ্ক, 1);
		মুক্ত(অঙ্ক);
	}
	errno = serrno;
}

পূর্ণসংখ্যা
einfovn(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	পূর্ণসংখ্যা retval;
	va_list ap;

	যদি (!fmt || !is_verbose())
		ফেরত 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	ফেরত retval;
}

পূর্ণসংখ্যা
ewarnvn(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	পূর্ণসংখ্যা retval;
	va_list ap;

	যদি (!fmt || !is_verbose())
		ফেরত 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	ফেরত retval;
}

পূর্ণসংখ্যা
einfov(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	পূর্ণসংখ্যা retval;
	va_list ap;

	যদি (!fmt || !is_verbose())
		ফেরত 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	ফেরত retval;
}

পূর্ণসংখ্যা
ewarnv(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	পূর্ণসংখ্যা retval;
	va_list ap;

	যদি (!fmt || !is_verbose())
		ফেরত 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	ফেরত retval;
}

পূর্ণসংখ্যা
ebeginv(ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	পূর্ণসংখ্যা retval;
	va_list ap;

	যদি (!fmt || !is_verbose())
		ফেরত 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	যদি (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	ফেরত retval;
}

পূর্ণসংখ্যা
eendv(পূর্ণসংখ্যা retval, ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	যদি (!is_verbose())
		ফেরত 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	ফেরত retval;
}

পূর্ণসংখ্যা
ewendv(পূর্ণসংখ্যা retval, ধ্রুবক অক্ষর *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	যদি (!is_verbose())
		ফেরত 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	ফেরত retval;
}

শূন্য
eindentv(শূন্য)
{
	যদি (is_verbose())
		eindent();
}

শূন্য
eoutdentv(শূন্য)
{
	যদি (is_verbose())
		eoutdent();
}
