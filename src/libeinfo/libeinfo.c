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

#અન્તર્ભાવ <sys/ioctl.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <syslog.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "helpers.h"

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

#define _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), કદ(_d));
#define _ASSIGN_CAP(_v) કરો {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, કદ(ebuffer) - (p - ebuffer)) + 1;    \
	} જ્યારે (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
સ્થિર અચલ અક્ષર *_eprefix = ખાલી;

/* Buffers and structures to hold the final colours */
સ્થિર અક્ષર ebuffer[100];
સંરચના ecolor {
	ECOLOR color;
	પૂર્ણાંક def;
	અચલ અક્ષર *name;
};
સ્થિર અક્ષર nullstr = '\0';

સ્થિર અચલ સંરચના ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_NORMAL,  0,       ખાલી      },
};
સ્થિર અચલ અક્ષર *ecolors_str[ARRAY_SIZE(ecolors)];

સ્થિર અક્ષર *flush = ખાલી;
સ્થિર અક્ષર *up = ખાલી;
સ્થિર અક્ષર *goto_column = ખાલી;

સ્થિર અચલ અક્ષર *term = ખાલી;
સ્થિર bool term_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
સ્થિર અચલ અક્ષર *અચલ color_terms[] = {
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
	ખાલી
};

#ifndef HAVE_STRLCPY
સ્થિર size_t
strlcat(અક્ષર *dst, અચલ અક્ષર *src, size_t size)
{
	અક્ષર *d = dst;
	અચલ અક્ષર *s = src;
	size_t src_n = size;
	size_t dst_n;

	જ્યારે (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	જો (src_n == 0)
		પાછા dst_n + strlen(src);

	જ્યારે (*s != '\0') {
		જો (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	પાછા dst_n + (s - src);
}
#endif

સ્થિર bool
yesno(અચલ અક્ષર *કિંમત)
{
	જો (!કિંમત) {
		errno = ENOENT;
		પાછા false;
	}

	જો (strcasecmp(કિંમત, "yes") == 0 ||
	    strcasecmp(કિંમત, "y") == 0 ||
	    strcasecmp(કિંમત, "true") == 0 ||
	    strcasecmp(કિંમત, "on") == 0 ||
	    strcasecmp(કિંમત, "1") == 0)
		પાછા true;

	જો (strcasecmp(કિંમત, "no") != 0 &&
	    strcasecmp(કિંમત, "n") != 0 &&
	    strcasecmp(કિંમત, "false") != 0 &&
	    strcasecmp(કિંમત, "off") != 0 &&
	    strcasecmp(કિંમત, "0") != 0)
		errno = EINVAL;

	પાછા false;
}

સ્થિર bool
noyes(અચલ અક્ષર *કિંમત)
{
	પૂર્ણાંક serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(કિંમત);
	જો (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	પાછા retval;
}

સ્થિર bool
is_quiet(શૂન્ય)
{
	પાછા yesno(getenv("EINFO_QUIET"));
}

સ્થિર bool
is_really_quiet(શૂન્ય)
{
	પાછા yesno(getenv("EERROR_QUIET"));
}

સ્થિર bool
is_verbose(શૂન્ય)
{
	પાછા yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
સ્થિર અક્ષર *
tgoto(અચલ અક્ષર *cap, પૂર્ણાંક col, પૂર્ણાંક લીટી)
{
	સ્થિર અક્ષર buf[20];
	અક્ષર *p, *e, c, dbuf[6];
	પૂર્ણાંક oncol = 0, which = લીટી, i;

	p = buf;
	e = p + કદ(buf);
	જ્યારે ((c = *cap++)) {
		જો (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			જો (p >= e) {
				errno = E2BIG;
				પાછા ખાલી;
			}
			ચાલુ;
		}
		પસંદગી (c) {
		સ્થિતિ '3':
		સ્થિતિ '2':
		સ્થિતિ 'd':
			i = 0;
			કરો
				dbuf[i++] = which % 10 | '0';
			જ્યારે ((which /= 10));
			જો (c != 'd') {
				c -= '0';
				જો (i > c) {
					errno = EINVAL;
					પાછા ખાલી;
				}
				જ્યારે (i < c)
					dbuf[i++] = '0';
			}
			જો (p + i >= e) {
				errno = E2BIG;
				પાછા ખાલી;
			}
			કરો
				*p++ = dbuf[--i];
			જ્યારે (i);
			તોડો;
		સ્થિતિ 'r':
			oncol = 0;
			તોડો;
		સ્થિતિ 'i':
			col++;
			લીટી++;
			which++;
			ચાલુ;
		મૂળભૂત:
			errno = EINVAL;
			પાછા ખાલી;
		}

		oncol = 1 - oncol;
		which = oncol ? col : લીટી;
	}
	*p = '\0';
	પાછા buf;
}

સ્થિર bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	સ્થિર પૂર્ણાંક in_colour = -1;
	અક્ષર *e, *ee, *end, *d, *p;
	પૂર્ણાંક c;
	અચલ અક્ષર *_af = ખાલી, *_ce = ખાલી, *_ch = ખાલી;
	અચલ અક્ષર *_md = ખાલી, *_me = ખાલી, *_up = ખાલી;
	અચલ અક્ષર *bold;
	અક્ષર tmp[100];
	unsigned પૂર્ણાંક i = 0;

	જો (f && !isatty(fileno(f)))
		પાછા false;

	જો (noyes(getenv("EINFO_COLOR")))
		પાછા false;

	જો (in_colour == 0)
		પાછા false;
	જો (in_colour == 1)
		પાછા true;

	term_is_cons25 = false;
	જો (!term) {
		term = getenv("TERM");
		જો (!term)
			પાછા false;
	}
	જો (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	જો (strstr(term, "color"))
		in_colour = 1;

	જ્યારે (color_terms[i] && in_colour != 1) {
		જો (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	જો (in_colour != 1) {
		in_colour = 0;
		પાછા false;
	}

	જો (!_af)
		_af = AF;
	જો (!_ce)
		_ce = CE;
	જો (!_ch)
		_ch = CH;
	જો (!_md)
		_md = MD;
	જો (!_me)
		_me = ME;
	જો (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	માટે (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		જો (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			જો ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				જો (ee)
					ee += strlen(ecolors[i].name);

				જો (ee && *ee == '=') {
					d = strdup(ee + 1);
					જો (d) {
						end = strchr(d, ':');
						જો (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						જો (end && *++end == '0')
							bold = _me;
						મુક્ત(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), કદ(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), કદ(tmp));
		} નહીંતર
			_GET_CAP(tmp, _me);

		જો (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		નહીંતર
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, કદ(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	પાછા true;
}

સ્થિર પૂર્ણાંક
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	સંરચના winsize ws;
	અક્ષર *env = getenv("COLUMNS");
	અક્ષર *p;
	પૂર્ણાંક i;

	જો (env) {
		i = strtoimax(env, &p, 10);
		જો (!*p)
			પાછા i;
	}

	જો (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		પાછા ws.ws_col;

	પાછા DEFAULT_COLS;
}

શૂન્ય
eprefix(અચલ અક્ષર *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

સ્થિર શૂન્ય EINFO_PRINTF(2, 0)
elogv(પૂર્ણાંક level, અચલ અક્ષર *EINFO_RESTRICT fmt, va_list ap)
{
	અક્ષર *e = getenv("EINFO_LOG");
	va_list apc;

	જો (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

શૂન્ય
elog(પૂર્ણાંક level, અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

સ્થિર પૂર્ણાંક
_eindent(FILE * EINFO_RESTRICT stream)
{
	અક્ષર *env = getenv("EINFO_INDENT");
	પૂર્ણાંક amount = 0;

	જો (env) {
		errno = 0;
		amount = strtoimax(env, ખાલી, 0);
		જો (errno != 0 || amount < 0)
			amount = 0;
		નહીંતર જો (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	પાછા fprintf(stream, "%*s", amount, "");
}

સ્થિર અચલ અક્ષર *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned પૂર્ણાંક i;

	જો (!colour_terminal(f))
		પાછા "";

	માટે (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		જો (ecolors[i].color == color)
			પાછા ecolors_str[i];
	પાછા "";
}

અચલ અક્ષર *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	જો (!isatty(fileno(f))) {
		f = stderr;
		જો (!isatty(fileno(f))) {
			f = stdin;
			જો (!isatty(fileno(f)))
				f = ખાલી;
		}
	}

	પાછા _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

સ્થિર પૂર્ણાંક EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, અચલ અક્ષર *EINFO_RESTRICT fmt, va_list va)
{
	પૂર્ણાંક retval = 0;
	અક્ષર *last = getenv("EINFO_LASTCMD");
	va_list ap;

	જો (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	જો (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	જો (colour_terminal(f))
		fprintf(f, "%s", flush);
	પાછા retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

પૂર્ણાંક
einfon(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	પૂર્ણાંક retval;
	va_list ap;

	જો (!fmt || is_quiet())
		પાછા 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	પાછા retval;
}

પૂર્ણાંક
ewarnn(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	પૂર્ણાંક retval;
	va_list ap;

	જો (!fmt || is_quiet())
		પાછા 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	પાછા retval;
}

પૂર્ણાંક
eerrorn(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	પૂર્ણાંક retval;
	va_list ap;

	જો (!fmt || is_really_quiet())
		પાછા 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	પાછા retval;
}

પૂર્ણાંક
einfo(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	પૂર્ણાંક retval;
	va_list ap;

	જો (!fmt || is_quiet())
		પાછા 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	પાછા retval;
}

પૂર્ણાંક
ewarn(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	પૂર્ણાંક retval;
	va_list ap;

	જો (!fmt || is_quiet())
		પાછા 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	પાછા retval;
}

શૂન્ય
ewarnx(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	જો (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

પૂર્ણાંક
eerror(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	પૂર્ણાંક retval;
	va_list ap;

	જો (!fmt || is_really_quiet())
		પાછા 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	પાછા retval;
}

શૂન્ય
eerrorx(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	જો (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

પૂર્ણાંક
ebegin(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	પૂર્ણાંક retval;
	va_list ap;

	જો (!fmt || is_quiet())
		પાછા 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	જો (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	પાછા retval;
}

સ્થિર શૂન્ય
_eend(FILE * EINFO_RESTRICT fp, પૂર્ણાંક col, ECOLOR color, અચલ અક્ષર *સંદેશ)
{
	પૂર્ણાંક i;
	પૂર્ણાંક cols;

	જો (!સંદેશ)
		પાછા;

	cols = get_term_columns(fp) - (strlen(સંદેશ) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	જો (!term) {
		term = getenv("TERM");
		જો (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		નહીંતર
			term_is_cons25 = false;
	}
	જો (term_is_cons25)
		cols--;

	જો (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), સંદેશ,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} નહીંતર {
		જો (col > 0)
			માટે (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", સંદેશ);
	}
}

સ્થિર પૂર્ણાંક EINFO_PRINTF(3, 0)
_do_eend(અચલ અક્ષર *cmd, પૂર્ણાંક retval,
    અચલ અક્ષર *EINFO_RESTRICT fmt, va_list ap)
{
	પૂર્ણાંક col = 0;
	FILE *fp = stdout;
	va_list apc;

	જો (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		જો (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		નહીંતર
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	પાછા retval;
}

પૂર્ણાંક
eend(પૂર્ણાંક retval, અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	જો (is_quiet())
		પાછા retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	પાછા retval;
}

પૂર્ણાંક
ewend(પૂર્ણાંક retval, અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	જો (is_quiet())
		પાછા retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	પાછા retval;
}

શૂન્ય
ebracket(પૂર્ણાંક col, ECOLOR color, અચલ અક્ષર *સંદેશ)
{
	_eend(stdout, col, color, સંદેશ);
}

શૂન્ય
eindent(શૂન્ય)
{
	અક્ષર *env = getenv("EINFO_INDENT");
	પૂર્ણાંક amount = 0;
	અક્ષર *અંક;

	જો (env) {
		errno = 0;
		amount = strtoimax(env, ખાલી, 0);
		જો (errno != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
	xasprintf(&અંક, "%08d", amount);
	setenv("EINFO_INDENT", અંક, 1);
	મુક્ત(અંક);
}

શૂન્ય eoutdent(શૂન્ય)
{
	અક્ષર *env = getenv("EINFO_INDENT");
	પૂર્ણાંક amount = 0;
	અક્ષર *અંક = ખાલી;
	પૂર્ણાંક serrno = errno;

	જો (!env)
		પાછા;
	errno = 0;
	amount = strtoimax(env, ખાલી, 0);
	જો (errno != 0)
		amount = 0;
	નહીંતર
		amount -= INDENT_WIDTH;
	જો (amount <= 0)
		unsetenv("EINFO_INDENT");
	નહીંતર {
		xasprintf(&અંક, "%08d", amount);
		setenv("EINFO_INDENT", અંક, 1);
		મુક્ત(અંક);
	}
	errno = serrno;
}

પૂર્ણાંક
einfovn(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	પૂર્ણાંક retval;
	va_list ap;

	જો (!fmt || !is_verbose())
		પાછા 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	પાછા retval;
}

પૂર્ણાંક
ewarnvn(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	પૂર્ણાંક retval;
	va_list ap;

	જો (!fmt || !is_verbose())
		પાછા 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	પાછા retval;
}

પૂર્ણાંક
einfov(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	પૂર્ણાંક retval;
	va_list ap;

	જો (!fmt || !is_verbose())
		પાછા 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	પાછા retval;
}

પૂર્ણાંક
ewarnv(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	પૂર્ણાંક retval;
	va_list ap;

	જો (!fmt || !is_verbose())
		પાછા 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	પાછા retval;
}

પૂર્ણાંક
ebeginv(અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	પૂર્ણાંક retval;
	va_list ap;

	જો (!fmt || !is_verbose())
		પાછા 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	જો (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	પાછા retval;
}

પૂર્ણાંક
eendv(પૂર્ણાંક retval, અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	જો (!is_verbose())
		પાછા 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	પાછા retval;
}

પૂર્ણાંક
ewendv(પૂર્ણાંક retval, અચલ અક્ષર *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	જો (!is_verbose())
		પાછા 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	પાછા retval;
}

શૂન્ય
eindentv(શૂન્ય)
{
	જો (is_verbose())
		eindent();
}

શૂન્ય
eoutdentv(શૂન્ય)
{
	જો (is_verbose())
		eoutdent();
}
