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

#ଅନ୍ତର୍ଭାଵ <sys/ioctl.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <syslog.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

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

#define _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), ଆକାର(_d));
#define _ASSIGN_CAP(_v) କର {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, ଆକାର(ebuffer) - (p - ebuffer)) + 1;    \
	} ଯେତେବେଳେ (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *_eprefix = ଖାଲି;

/* Buffers and structures to hold the final colours */
ସ୍ଥିର ଅକ୍ଷର ebuffer[100];
ସଂରଚନା ecolor {
	ECOLOR color;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା def;
	ଅଚଳ ଅକ୍ଷର *name;
};
ସ୍ଥିର ଅକ୍ଷର nullstr = '\0';

ସ୍ଥିର ଅଚଳ ସଂରଚନା ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_NORMAL,  0,       ଖାଲି      },
};
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ecolors_str[ARRAY_SIZE(ecolors)];

ସ୍ଥିର ଅକ୍ଷର *flush = ଖାଲି;
ସ୍ଥିର ଅକ୍ଷର *up = ଖାଲି;
ସ୍ଥିର ଅକ୍ଷର *goto_column = ଖାଲି;

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *term = ଖାଲି;
ସ୍ଥିର bool term_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ color_terms[] = {
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
	ଖାଲି
};

#ifndef HAVE_STRLCPY
ସ୍ଥିର size_t
strlcat(ଅକ୍ଷର *dst, ଅଚଳ ଅକ୍ଷର *src, size_t size)
{
	ଅକ୍ଷର *d = dst;
	ଅଚଳ ଅକ୍ଷର *s = src;
	size_t src_n = size;
	size_t dst_n;

	ଯେତେବେଳେ (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	ଯଦି (src_n == 0)
		ଫେରନ୍ତୁ dst_n + strlen(src);

	ଯେତେବେଳେ (*s != '\0') {
		ଯଦି (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	ଫେରନ୍ତୁ dst_n + (s - src);
}
#endif

ସ୍ଥିର bool
yesno(ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	ଯଦି (!ମୂଲ୍ୟ) {
		errno = ENOENT;
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (strcasecmp(ମୂଲ୍ୟ, "yes") == 0 ||
	    strcasecmp(ମୂଲ୍ୟ, "y") == 0 ||
	    strcasecmp(ମୂଲ୍ୟ, "true") == 0 ||
	    strcasecmp(ମୂଲ୍ୟ, "on") == 0 ||
	    strcasecmp(ମୂଲ୍ୟ, "1") == 0)
		ଫେରନ୍ତୁ true;

	ଯଦି (strcasecmp(ମୂଲ୍ୟ, "no") != 0 &&
	    strcasecmp(ମୂଲ୍ୟ, "n") != 0 &&
	    strcasecmp(ମୂଲ୍ୟ, "false") != 0 &&
	    strcasecmp(ମୂଲ୍ୟ, "off") != 0 &&
	    strcasecmp(ମୂଲ୍ୟ, "0") != 0)
		errno = EINVAL;

	ଫେରନ୍ତୁ false;
}

ସ୍ଥିର bool
noyes(ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(ମୂଲ୍ୟ);
	ଯଦି (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	ଫେରନ୍ତୁ retval;
}

ସ୍ଥିର bool
is_quiet(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ yesno(getenv("EINFO_QUIET"));
}

ସ୍ଥିର bool
is_really_quiet(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ yesno(getenv("EERROR_QUIET"));
}

ସ୍ଥିର bool
is_verbose(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
ସ୍ଥିର ଅକ୍ଷର *
tgoto(ଅଚଳ ଅକ୍ଷର *cap, ପୂର୍ଣ୍ଣସଂଖ୍ୟା col, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି)
{
	ସ୍ଥିର ଅକ୍ଷର buf[20];
	ଅକ୍ଷର *p, *e, c, dbuf[6];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା oncol = 0, which = ଧାଡ଼ି, i;

	p = buf;
	e = p + ଆକାର(buf);
	ଯେତେବେଳେ ((c = *cap++)) {
		ଯଦି (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			ଯଦି (p >= e) {
				errno = E2BIG;
				ଫେରନ୍ତୁ ଖାଲି;
			}
			ଜାରି;
		}
		ବାଛ (c) {
		ସ୍ଥିତି '3':
		ସ୍ଥିତି '2':
		ସ୍ଥିତି 'd':
			i = 0;
			କର
				dbuf[i++] = which % 10 | '0';
			ଯେତେବେଳେ ((which /= 10));
			ଯଦି (c != 'd') {
				c -= '0';
				ଯଦି (i > c) {
					errno = EINVAL;
					ଫେରନ୍ତୁ ଖାଲି;
				}
				ଯେତେବେଳେ (i < c)
					dbuf[i++] = '0';
			}
			ଯଦି (p + i >= e) {
				errno = E2BIG;
				ଫେରନ୍ତୁ ଖାଲି;
			}
			କର
				*p++ = dbuf[--i];
			ଯେତେବେଳେ (i);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'r':
			oncol = 0;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'i':
			col++;
			ଧାଡ଼ି++;
			which++;
			ଜାରି;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			errno = EINVAL;
			ଫେରନ୍ତୁ ଖାଲି;
		}

		oncol = 1 - oncol;
		which = oncol ? col : ଧାଡ଼ି;
	}
	*p = '\0';
	ଫେରନ୍ତୁ buf;
}

ସ୍ଥିର bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା in_colour = -1;
	ଅକ୍ଷର *e, *ee, *end, *d, *p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
	ଅଚଳ ଅକ୍ଷର *_af = ଖାଲି, *_ce = ଖାଲି, *_ch = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *_md = ଖାଲି, *_me = ଖାଲି, *_up = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *bold;
	ଅକ୍ଷର tmp[100];
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;

	ଯଦି (f && !isatty(fileno(f)))
		ଫେରନ୍ତୁ false;

	ଯଦି (noyes(getenv("EINFO_COLOR")))
		ଫେରନ୍ତୁ false;

	ଯଦି (in_colour == 0)
		ଫେରନ୍ତୁ false;
	ଯଦି (in_colour == 1)
		ଫେରନ୍ତୁ true;

	term_is_cons25 = false;
	ଯଦି (!term) {
		term = getenv("TERM");
		ଯଦି (!term)
			ଫେରନ୍ତୁ false;
	}
	ଯଦି (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	ଯଦି (strstr(term, "color"))
		in_colour = 1;

	ଯେତେବେଳେ (color_terms[i] && in_colour != 1) {
		ଯଦି (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	ଯଦି (in_colour != 1) {
		in_colour = 0;
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (!_af)
		_af = AF;
	ଯଦି (!_ce)
		_ce = CE;
	ଯଦି (!_ch)
		_ch = CH;
	ଯଦି (!_md)
		_md = MD;
	ଯଦି (!_me)
		_me = ME;
	ଯଦି (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	ପାଇଁ (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		ଯଦି (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			ଯଦି ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				ଯଦି (ee)
					ee += strlen(ecolors[i].name);

				ଯଦି (ee && *ee == '=') {
					d = strdup(ee + 1);
					ଯଦି (d) {
						end = strchr(d, ':');
						ଯଦି (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						ଯଦି (end && *++end == '0')
							bold = _me;
						ମୁକ୍ତ(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), ଆକାର(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), ଆକାର(tmp));
		} ଅନ୍ୟଥା
			_GET_CAP(tmp, _me);

		ଯଦି (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		ଅନ୍ୟଥା
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, ଆକାର(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	ସଂରଚନା winsize ws;
	ଅକ୍ଷର *env = getenv("COLUMNS");
	ଅକ୍ଷର *p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (env) {
		i = strtoimax(env, &p, 10);
		ଯଦି (!*p)
			ଫେରନ୍ତୁ i;
	}

	ଯଦି (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		ଫେରନ୍ତୁ ws.ws_col;

	ଫେରନ୍ତୁ DEFAULT_COLS;
}

ଶୂନ୍ୟ
eprefix(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

ସ୍ଥିର ଶୂନ୍ୟ EINFO_PRINTF(2, 0)
elogv(ପୂର୍ଣ୍ଣସଂଖ୍ୟା level, ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, va_list ap)
{
	ଅକ୍ଷର *e = getenv("EINFO_LOG");
	va_list apc;

	ଯଦି (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

ଶୂନ୍ୟ
elog(ପୂର୍ଣ୍ଣସଂଖ୍ୟା level, ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
_eindent(FILE * EINFO_RESTRICT stream)
{
	ଅକ୍ଷର *env = getenv("EINFO_INDENT");
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା amount = 0;

	ଯଦି (env) {
		errno = 0;
		amount = strtoimax(env, ଖାଲି, 0);
		ଯଦି (errno != 0 || amount < 0)
			amount = 0;
		ଅନ୍ୟଥା ଯଦି (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	ଫେରନ୍ତୁ fprintf(stream, "%*s", amount, "");
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (!colour_terminal(f))
		ଫେରନ୍ତୁ "";

	ପାଇଁ (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		ଯଦି (ecolors[i].color == color)
			ଫେରନ୍ତୁ ecolors_str[i];
	ଫେରନ୍ତୁ "";
}

ଅଚଳ ଅକ୍ଷର *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	ଯଦି (!isatty(fileno(f))) {
		f = stderr;
		ଯଦି (!isatty(fileno(f))) {
			f = stdin;
			ଯଦି (!isatty(fileno(f)))
				f = ଖାଲି;
		}
	}

	ଫେରନ୍ତୁ _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, va_list va)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval = 0;
	ଅକ୍ଷର *last = getenv("EINFO_LASTCMD");
	va_list ap;

	ଯଦି (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	ଯଦି (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	ଯଦି (colour_terminal(f))
		fprintf(f, "%s", flush);
	ଫେରନ୍ତୁ retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
einfon(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
	va_list ap;

	ଯଦି (!fmt || is_quiet())
		ଫେରନ୍ତୁ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	ଫେରନ୍ତୁ retval;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ewarnn(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
	va_list ap;

	ଯଦି (!fmt || is_quiet())
		ଫେରନ୍ତୁ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	ଫେରନ୍ତୁ retval;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
eerrorn(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
	va_list ap;

	ଯଦି (!fmt || is_really_quiet())
		ଫେରନ୍ତୁ 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	ଫେରନ୍ତୁ retval;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
einfo(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
	va_list ap;

	ଯଦି (!fmt || is_quiet())
		ଫେରନ୍ତୁ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	ଫେରନ୍ତୁ retval;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ewarn(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
	va_list ap;

	ଯଦି (!fmt || is_quiet())
		ଫେରନ୍ତୁ 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	ଫେରନ୍ତୁ retval;
}

ଶୂନ୍ୟ
ewarnx(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ଯଦି (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
eerror(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
	va_list ap;

	ଯଦି (!fmt || is_really_quiet())
		ଫେରନ୍ତୁ 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	ଫେରନ୍ତୁ retval;
}

ଶୂନ୍ୟ
eerrorx(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ଯଦି (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ebegin(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
	va_list ap;

	ଯଦି (!fmt || is_quiet())
		ଫେରନ୍ତୁ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	ଯଦି (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	ଫେରନ୍ତୁ retval;
}

ସ୍ଥିର ଶୂନ୍ୟ
_eend(FILE * EINFO_RESTRICT fp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା col, ECOLOR color, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cols;

	ଯଦି (!ସନ୍ଦେଶ)
		ଫେରନ୍ତୁ;

	cols = get_term_columns(fp) - (strlen(ସନ୍ଦେଶ) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	ଯଦି (!term) {
		term = getenv("TERM");
		ଯଦି (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		ଅନ୍ୟଥା
			term_is_cons25 = false;
	}
	ଯଦି (term_is_cons25)
		cols--;

	ଯଦି (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), ସନ୍ଦେଶ,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} ଅନ୍ୟଥା {
		ଯଦି (col > 0)
			ପାଇଁ (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", ସନ୍ଦେଶ);
	}
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା EINFO_PRINTF(3, 0)
_do_eend(ଅଚଳ ଅକ୍ଷର *cmd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval,
    ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, va_list ap)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା col = 0;
	FILE *fp = stdout;
	va_list apc;

	ଯଦି (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		ଯଦି (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		ଅନ୍ୟଥା
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	ଫେରନ୍ତୁ retval;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
eend(ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval, ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ଯଦି (is_quiet())
		ଫେରନ୍ତୁ retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	ଫେରନ୍ତୁ retval;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ewend(ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval, ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ଯଦି (is_quiet())
		ଫେରନ୍ତୁ retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	ଫେରନ୍ତୁ retval;
}

ଶୂନ୍ୟ
ebracket(ପୂର୍ଣ୍ଣସଂଖ୍ୟା col, ECOLOR color, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ)
{
	_eend(stdout, col, color, ସନ୍ଦେଶ);
}

ଶୂନ୍ୟ
eindent(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *env = getenv("EINFO_INDENT");
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା amount = 0;
	ଅକ୍ଷର *ଅଙ୍କ;

	ଯଦି (env) {
		errno = 0;
		amount = strtoimax(env, ଖାଲି, 0);
		ଯଦି (errno != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
	xasprintf(&ଅଙ୍କ, "%08d", amount);
	setenv("EINFO_INDENT", ଅଙ୍କ, 1);
	ମୁକ୍ତ(ଅଙ୍କ);
}

ଶୂନ୍ୟ eoutdent(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *env = getenv("EINFO_INDENT");
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା amount = 0;
	ଅକ୍ଷର *ଅଙ୍କ = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା serrno = errno;

	ଯଦି (!env)
		ଫେରନ୍ତୁ;
	errno = 0;
	amount = strtoimax(env, ଖାଲି, 0);
	ଯଦି (errno != 0)
		amount = 0;
	ଅନ୍ୟଥା
		amount -= INDENT_WIDTH;
	ଯଦି (amount <= 0)
		unsetenv("EINFO_INDENT");
	ଅନ୍ୟଥା {
		xasprintf(&ଅଙ୍କ, "%08d", amount);
		setenv("EINFO_INDENT", ଅଙ୍କ, 1);
		ମୁକ୍ତ(ଅଙ୍କ);
	}
	errno = serrno;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
einfovn(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
	va_list ap;

	ଯଦି (!fmt || !is_verbose())
		ଫେରନ୍ତୁ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	ଫେରନ୍ତୁ retval;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ewarnvn(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
	va_list ap;

	ଯଦି (!fmt || !is_verbose())
		ଫେରନ୍ତୁ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	ଫେରନ୍ତୁ retval;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
einfov(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
	va_list ap;

	ଯଦି (!fmt || !is_verbose())
		ଫେରନ୍ତୁ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	ଫେରନ୍ତୁ retval;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ewarnv(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
	va_list ap;

	ଯଦି (!fmt || !is_verbose())
		ଫେରନ୍ତୁ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	ଫେରନ୍ତୁ retval;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ebeginv(ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
	va_list ap;

	ଯଦି (!fmt || !is_verbose())
		ଫେରନ୍ତୁ 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	ଯଦି (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	ଫେରନ୍ତୁ retval;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
eendv(ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval, ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ଯଦି (!is_verbose())
		ଫେରନ୍ତୁ 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	ଫେରନ୍ତୁ retval;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ewendv(ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval, ଅଚଳ ଅକ୍ଷର *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ଯଦି (!is_verbose())
		ଫେରନ୍ତୁ 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	ଫେରନ୍ତୁ retval;
}

ଶୂନ୍ୟ
eindentv(ଶୂନ୍ୟ)
{
	ଯଦି (is_verbose())
		eindent();
}

ଶୂନ୍ୟ
eoutdentv(ଶୂନ୍ୟ)
{
	ଯଦି (is_verbose())
		eoutdent();
}
