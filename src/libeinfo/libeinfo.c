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

#அந்தர்பாவ <sys/ioctl.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <inttypes.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <syslog.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "helpers.h"

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

#define _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), அளவு(_d));
#define _ASSIGN_CAP(_v) செய் {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, அளவு(ebuffer) - (p - ebuffer)) + 1;    \
	} போது (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
நிலையான மாறிலி எழுத்து *_eprefix = வெற்று;

/* Buffers and structures to hold the final colours */
நிலையான எழுத்து ebuffer[100];
கட்டமைப்பு ecolor {
	ECOLOR color;
	முழு def;
	மாறிலி எழுத்து *name;
};
நிலையான எழுத்து nullstr = '\0';

நிலையான மாறிலி கட்டமைப்பு ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_NORMAL,  0,       வெற்று      },
};
நிலையான மாறிலி எழுத்து *ecolors_str[ARRAY_SIZE(ecolors)];

நிலையான எழுத்து *flush = வெற்று;
நிலையான எழுத்து *up = வெற்று;
நிலையான எழுத்து *goto_column = வெற்று;

நிலையான மாறிலி எழுத்து *term = வெற்று;
நிலையான bool term_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
நிலையான மாறிலி எழுத்து *மாறிலி color_terms[] = {
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
	வெற்று
};

#ifndef HAVE_STRLCPY
நிலையான size_t
strlcat(எழுத்து *dst, மாறிலி எழுத்து *src, size_t size)
{
	எழுத்து *d = dst;
	மாறிலி எழுத்து *s = src;
	size_t src_n = size;
	size_t dst_n;

	போது (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	எனில் (src_n == 0)
		திரும்பு dst_n + strlen(src);

	போது (*s != '\0') {
		எனில் (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	திரும்பு dst_n + (s - src);
}
#endif

நிலையான bool
yesno(மாறிலி எழுத்து *மதிப்பு)
{
	எனில் (!மதிப்பு) {
		errno = ENOENT;
		திரும்பு false;
	}

	எனில் (strcasecmp(மதிப்பு, "yes") == 0 ||
	    strcasecmp(மதிப்பு, "y") == 0 ||
	    strcasecmp(மதிப்பு, "true") == 0 ||
	    strcasecmp(மதிப்பு, "on") == 0 ||
	    strcasecmp(மதிப்பு, "1") == 0)
		திரும்பு true;

	எனில் (strcasecmp(மதிப்பு, "no") != 0 &&
	    strcasecmp(மதிப்பு, "n") != 0 &&
	    strcasecmp(மதிப்பு, "false") != 0 &&
	    strcasecmp(மதிப்பு, "off") != 0 &&
	    strcasecmp(மதிப்பு, "0") != 0)
		errno = EINVAL;

	திரும்பு false;
}

நிலையான bool
noyes(மாறிலி எழுத்து *மதிப்பு)
{
	முழு serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(மதிப்பு);
	எனில் (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	திரும்பு retval;
}

நிலையான bool
is_quiet(வெறுமை)
{
	திரும்பு yesno(getenv("EINFO_QUIET"));
}

நிலையான bool
is_really_quiet(வெறுமை)
{
	திரும்பு yesno(getenv("EERROR_QUIET"));
}

நிலையான bool
is_verbose(வெறுமை)
{
	திரும்பு yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
நிலையான எழுத்து *
tgoto(மாறிலி எழுத்து *cap, முழு col, முழு வரி)
{
	நிலையான எழுத்து buf[20];
	எழுத்து *p, *e, c, dbuf[6];
	முழு oncol = 0, which = வரி, i;

	p = buf;
	e = p + அளவு(buf);
	போது ((c = *cap++)) {
		எனில் (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			எனில் (p >= e) {
				errno = E2BIG;
				திரும்பு வெற்று;
			}
			தொடர்;
		}
		தேர்வு (c) {
		நிலை '3':
		நிலை '2':
		நிலை 'd':
			i = 0;
			செய்
				dbuf[i++] = which % 10 | '0';
			போது ((which /= 10));
			எனில் (c != 'd') {
				c -= '0';
				எனில் (i > c) {
					errno = EINVAL;
					திரும்பு வெற்று;
				}
				போது (i < c)
					dbuf[i++] = '0';
			}
			எனில் (p + i >= e) {
				errno = E2BIG;
				திரும்பு வெற்று;
			}
			செய்
				*p++ = dbuf[--i];
			போது (i);
			நிறுத்து;
		நிலை 'r':
			oncol = 0;
			நிறுத்து;
		நிலை 'i':
			col++;
			வரி++;
			which++;
			தொடர்;
		இயல்பு:
			errno = EINVAL;
			திரும்பு வெற்று;
		}

		oncol = 1 - oncol;
		which = oncol ? col : வரி;
	}
	*p = '\0';
	திரும்பு buf;
}

நிலையான bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	நிலையான முழு in_colour = -1;
	எழுத்து *e, *ee, *end, *d, *p;
	முழு c;
	மாறிலி எழுத்து *_af = வெற்று, *_ce = வெற்று, *_ch = வெற்று;
	மாறிலி எழுத்து *_md = வெற்று, *_me = வெற்று, *_up = வெற்று;
	மாறிலி எழுத்து *bold;
	எழுத்து tmp[100];
	unsigned முழு i = 0;

	எனில் (f && !isatty(fileno(f)))
		திரும்பு false;

	எனில் (noyes(getenv("EINFO_COLOR")))
		திரும்பு false;

	எனில் (in_colour == 0)
		திரும்பு false;
	எனில் (in_colour == 1)
		திரும்பு true;

	term_is_cons25 = false;
	எனில் (!term) {
		term = getenv("TERM");
		எனில் (!term)
			திரும்பு false;
	}
	எனில் (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	எனில் (strstr(term, "color"))
		in_colour = 1;

	போது (color_terms[i] && in_colour != 1) {
		எனில் (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	எனில் (in_colour != 1) {
		in_colour = 0;
		திரும்பு false;
	}

	எனில் (!_af)
		_af = AF;
	எனில் (!_ce)
		_ce = CE;
	எனில் (!_ch)
		_ch = CH;
	எனில் (!_md)
		_md = MD;
	எனில் (!_me)
		_me = ME;
	எனில் (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	க்கு (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		எனில் (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			எனில் ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				எனில் (ee)
					ee += strlen(ecolors[i].name);

				எனில் (ee && *ee == '=') {
					d = strdup(ee + 1);
					எனில் (d) {
						end = strchr(d, ':');
						எனில் (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						எனில் (end && *++end == '0')
							bold = _me;
						விடுவி(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), அளவு(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), அளவு(tmp));
		} இல்லையெனில்
			_GET_CAP(tmp, _me);

		எனில் (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		இல்லையெனில்
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, அளவு(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	திரும்பு true;
}

நிலையான முழு
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	கட்டமைப்பு winsize ws;
	எழுத்து *env = getenv("COLUMNS");
	எழுத்து *p;
	முழு i;

	எனில் (env) {
		i = strtoimax(env, &p, 10);
		எனில் (!*p)
			திரும்பு i;
	}

	எனில் (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		திரும்பு ws.ws_col;

	திரும்பு DEFAULT_COLS;
}

வெறுமை
eprefix(மாறிலி எழுத்து *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

நிலையான வெறுமை EINFO_PRINTF(2, 0)
elogv(முழு level, மாறிலி எழுத்து *EINFO_RESTRICT fmt, va_list ap)
{
	எழுத்து *e = getenv("EINFO_LOG");
	va_list apc;

	எனில் (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

வெறுமை
elog(முழு level, மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

நிலையான முழு
_eindent(FILE * EINFO_RESTRICT stream)
{
	எழுத்து *env = getenv("EINFO_INDENT");
	முழு amount = 0;

	எனில் (env) {
		errno = 0;
		amount = strtoimax(env, வெற்று, 0);
		எனில் (errno != 0 || amount < 0)
			amount = 0;
		இல்லையெனில் எனில் (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	திரும்பு fprintf(stream, "%*s", amount, "");
}

நிலையான மாறிலி எழுத்து *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned முழு i;

	எனில் (!colour_terminal(f))
		திரும்பு "";

	க்கு (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		எனில் (ecolors[i].color == color)
			திரும்பு ecolors_str[i];
	திரும்பு "";
}

மாறிலி எழுத்து *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	எனில் (!isatty(fileno(f))) {
		f = stderr;
		எனில் (!isatty(fileno(f))) {
			f = stdin;
			எனில் (!isatty(fileno(f)))
				f = வெற்று;
		}
	}

	திரும்பு _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

நிலையான முழு EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, மாறிலி எழுத்து *EINFO_RESTRICT fmt, va_list va)
{
	முழு retval = 0;
	எழுத்து *last = getenv("EINFO_LASTCMD");
	va_list ap;

	எனில் (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	எனில் (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	எனில் (colour_terminal(f))
		fprintf(f, "%s", flush);
	திரும்பு retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

முழு
einfon(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	முழு retval;
	va_list ap;

	எனில் (!fmt || is_quiet())
		திரும்பு 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	திரும்பு retval;
}

முழு
ewarnn(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	முழு retval;
	va_list ap;

	எனில் (!fmt || is_quiet())
		திரும்பு 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	திரும்பு retval;
}

முழு
eerrorn(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	முழு retval;
	va_list ap;

	எனில் (!fmt || is_really_quiet())
		திரும்பு 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	திரும்பு retval;
}

முழு
einfo(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	முழு retval;
	va_list ap;

	எனில் (!fmt || is_quiet())
		திரும்பு 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	திரும்பு retval;
}

முழு
ewarn(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	முழு retval;
	va_list ap;

	எனில் (!fmt || is_quiet())
		திரும்பு 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	திரும்பு retval;
}

வெறுமை
ewarnx(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	எனில் (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

முழு
eerror(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	முழு retval;
	va_list ap;

	எனில் (!fmt || is_really_quiet())
		திரும்பு 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	திரும்பு retval;
}

வெறுமை
eerrorx(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	எனில் (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

முழு
ebegin(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	முழு retval;
	va_list ap;

	எனில் (!fmt || is_quiet())
		திரும்பு 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	எனில் (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	திரும்பு retval;
}

நிலையான வெறுமை
_eend(FILE * EINFO_RESTRICT fp, முழு col, ECOLOR color, மாறிலி எழுத்து *செய்தி)
{
	முழு i;
	முழு cols;

	எனில் (!செய்தி)
		திரும்பு;

	cols = get_term_columns(fp) - (strlen(செய்தி) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	எனில் (!term) {
		term = getenv("TERM");
		எனில் (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		இல்லையெனில்
			term_is_cons25 = false;
	}
	எனில் (term_is_cons25)
		cols--;

	எனில் (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), செய்தி,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} இல்லையெனில் {
		எனில் (col > 0)
			க்கு (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", செய்தி);
	}
}

நிலையான முழு EINFO_PRINTF(3, 0)
_do_eend(மாறிலி எழுத்து *cmd, முழு retval,
    மாறிலி எழுத்து *EINFO_RESTRICT fmt, va_list ap)
{
	முழு col = 0;
	FILE *fp = stdout;
	va_list apc;

	எனில் (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		எனில் (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		இல்லையெனில்
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	திரும்பு retval;
}

முழு
eend(முழு retval, மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	எனில் (is_quiet())
		திரும்பு retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	திரும்பு retval;
}

முழு
ewend(முழு retval, மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	எனில் (is_quiet())
		திரும்பு retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	திரும்பு retval;
}

வெறுமை
ebracket(முழு col, ECOLOR color, மாறிலி எழுத்து *செய்தி)
{
	_eend(stdout, col, color, செய்தி);
}

வெறுமை
eindent(வெறுமை)
{
	எழுத்து *env = getenv("EINFO_INDENT");
	முழு amount = 0;
	எழுத்து *இலக்கம்;

	எனில் (env) {
		errno = 0;
		amount = strtoimax(env, வெற்று, 0);
		எனில் (errno != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
	xasprintf(&இலக்கம், "%08d", amount);
	setenv("EINFO_INDENT", இலக்கம், 1);
	விடுவி(இலக்கம்);
}

வெறுமை eoutdent(வெறுமை)
{
	எழுத்து *env = getenv("EINFO_INDENT");
	முழு amount = 0;
	எழுத்து *இலக்கம் = வெற்று;
	முழு serrno = errno;

	எனில் (!env)
		திரும்பு;
	errno = 0;
	amount = strtoimax(env, வெற்று, 0);
	எனில் (errno != 0)
		amount = 0;
	இல்லையெனில்
		amount -= INDENT_WIDTH;
	எனில் (amount <= 0)
		unsetenv("EINFO_INDENT");
	இல்லையெனில் {
		xasprintf(&இலக்கம், "%08d", amount);
		setenv("EINFO_INDENT", இலக்கம், 1);
		விடுவி(இலக்கம்);
	}
	errno = serrno;
}

முழு
einfovn(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	முழு retval;
	va_list ap;

	எனில் (!fmt || !is_verbose())
		திரும்பு 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	திரும்பு retval;
}

முழு
ewarnvn(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	முழு retval;
	va_list ap;

	எனில் (!fmt || !is_verbose())
		திரும்பு 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	திரும்பு retval;
}

முழு
einfov(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	முழு retval;
	va_list ap;

	எனில் (!fmt || !is_verbose())
		திரும்பு 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	திரும்பு retval;
}

முழு
ewarnv(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	முழு retval;
	va_list ap;

	எனில் (!fmt || !is_verbose())
		திரும்பு 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	திரும்பு retval;
}

முழு
ebeginv(மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	முழு retval;
	va_list ap;

	எனில் (!fmt || !is_verbose())
		திரும்பு 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	எனில் (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	திரும்பு retval;
}

முழு
eendv(முழு retval, மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	எனில் (!is_verbose())
		திரும்பு 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	திரும்பு retval;
}

முழு
ewendv(முழு retval, மாறிலி எழுத்து *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	எனில் (!is_verbose())
		திரும்பு 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	திரும்பு retval;
}

வெறுமை
eindentv(வெறுமை)
{
	எனில் (is_verbose())
		eindent();
}

வெறுமை
eoutdentv(வெறுமை)
{
	எனில் (is_verbose())
		eoutdent();
}
