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

#অন্তর্ভাৱ <sys/ioctl.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <inttypes.h>
#অন্তর্ভাৱ <stdarg.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stddef.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <strings.h>
#অন্তর্ভাৱ <syslog.h>
#অন্তর্ভাৱ <unistd.h>

#অন্তর্ভাৱ "einfo.h"
#অন্তর্ভাৱ "helpers.h"

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

#define _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), পরিমাণ(_d));
#define _ASSIGN_CAP(_v) কৰা {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, পরিমাণ(ebuffer) - (p - ebuffer)) + 1;    \
	} যেতিয়ালৈ (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
স্থির ধ্রুৱ ৱর্ণ *_eprefix = রিক্ত;

/* Buffers and structures to hold the final colours */
স্থির ৱর্ণ ebuffer[100];
সংরচনা ecolor {
	ECOLOR color;
	পূর্ণাঙ্ক def;
	ধ্রুৱ ৱর্ণ *name;
};
স্থির ৱর্ণ nullstr = '\0';

স্থির ধ্রুৱ সংরচনা ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_NORMAL,  0,       রিক্ত      },
};
স্থির ধ্রুৱ ৱর্ণ *ecolors_str[ARRAY_SIZE(ecolors)];

স্থির ৱর্ণ *flush = রিক্ত;
স্থির ৱর্ণ *up = রিক্ত;
স্থির ৱর্ণ *goto_column = রিক্ত;

স্থির ধ্রুৱ ৱর্ণ *term = রিক্ত;
স্থির bool term_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
স্থির ধ্রুৱ ৱর্ণ *ধ্রুৱ color_terms[] = {
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
	রিক্ত
};

#ifndef HAVE_STRLCPY
স্থির size_t
strlcat(ৱর্ণ *dst, ধ্রুৱ ৱর্ণ *src, size_t size)
{
	ৱর্ণ *d = dst;
	ধ্রুৱ ৱর্ণ *s = src;
	size_t src_n = size;
	size_t dst_n;

	যেতিয়ালৈ (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	যদি (src_n == 0)
		ঘূৰাই dst_n + strlen(src);

	যেতিয়ালৈ (*s != '\0') {
		যদি (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	ঘূৰাই dst_n + (s - src);
}
#endif

স্থির bool
yesno(ধ্রুৱ ৱর্ণ *মান)
{
	যদি (!মান) {
		errno = ENOENT;
		ঘূৰাই false;
	}

	যদি (strcasecmp(মান, "yes") == 0 ||
	    strcasecmp(মান, "y") == 0 ||
	    strcasecmp(মান, "true") == 0 ||
	    strcasecmp(মান, "on") == 0 ||
	    strcasecmp(মান, "1") == 0)
		ঘূৰাই true;

	যদি (strcasecmp(মান, "no") != 0 &&
	    strcasecmp(মান, "n") != 0 &&
	    strcasecmp(মান, "false") != 0 &&
	    strcasecmp(মান, "off") != 0 &&
	    strcasecmp(মান, "0") != 0)
		errno = EINVAL;

	ঘূৰাই false;
}

স্থির bool
noyes(ধ্রুৱ ৱর্ণ *মান)
{
	পূর্ণাঙ্ক serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(মান);
	যদি (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	ঘূৰাই retval;
}

স্থির bool
is_quiet(শূন্য)
{
	ঘূৰাই yesno(getenv("EINFO_QUIET"));
}

স্থির bool
is_really_quiet(শূন্য)
{
	ঘূৰাই yesno(getenv("EERROR_QUIET"));
}

স্থির bool
is_verbose(শূন্য)
{
	ঘূৰাই yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
স্থির ৱর্ণ *
tgoto(ধ্রুৱ ৱর্ণ *cap, পূর্ণাঙ্ক col, পূর্ণাঙ্ক সাৰি)
{
	স্থির ৱর্ণ buf[20];
	ৱর্ণ *p, *e, c, dbuf[6];
	পূর্ণাঙ্ক oncol = 0, which = সাৰি, i;

	p = buf;
	e = p + পরিমাণ(buf);
	যেতিয়ালৈ ((c = *cap++)) {
		যদি (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			যদি (p >= e) {
				errno = E2BIG;
				ঘূৰাই রিক্ত;
			}
			অনুৱর্ত;
		}
		ৱিকল্প (c) {
		পক্ষ '3':
		পক্ষ '2':
		পক্ষ 'd':
			i = 0;
			কৰা
				dbuf[i++] = which % 10 | '0';
			যেতিয়ালৈ ((which /= 10));
			যদি (c != 'd') {
				c -= '0';
				যদি (i > c) {
					errno = EINVAL;
					ঘূৰাই রিক্ত;
				}
				যেতিয়ালৈ (i < c)
					dbuf[i++] = '0';
			}
			যদি (p + i >= e) {
				errno = E2BIG;
				ঘূৰাই রিক্ত;
			}
			কৰা
				*p++ = dbuf[--i];
			যেতিয়ালৈ (i);
			ভঙ্গ;
		পক্ষ 'r':
			oncol = 0;
			ভঙ্গ;
		পক্ষ 'i':
			col++;
			সাৰি++;
			which++;
			অনুৱর্ত;
		স্ৱভাৱ:
			errno = EINVAL;
			ঘূৰাই রিক্ত;
		}

		oncol = 1 - oncol;
		which = oncol ? col : সাৰি;
	}
	*p = '\0';
	ঘূৰাই buf;
}

স্থির bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	স্থির পূর্ণাঙ্ক in_colour = -1;
	ৱর্ণ *e, *ee, *end, *d, *p;
	পূর্ণাঙ্ক c;
	ধ্রুৱ ৱর্ণ *_af = রিক্ত, *_ce = রিক্ত, *_ch = রিক্ত;
	ধ্রুৱ ৱর্ণ *_md = রিক্ত, *_me = রিক্ত, *_up = রিক্ত;
	ধ্রুৱ ৱর্ণ *bold;
	ৱর্ণ tmp[100];
	unsigned পূর্ণাঙ্ক i = 0;

	যদি (f && !isatty(fileno(f)))
		ঘূৰাই false;

	যদি (noyes(getenv("EINFO_COLOR")))
		ঘূৰাই false;

	যদি (in_colour == 0)
		ঘূৰাই false;
	যদি (in_colour == 1)
		ঘূৰাই true;

	term_is_cons25 = false;
	যদি (!term) {
		term = getenv("TERM");
		যদি (!term)
			ঘূৰাই false;
	}
	যদি (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	যদি (strstr(term, "color"))
		in_colour = 1;

	যেতিয়ালৈ (color_terms[i] && in_colour != 1) {
		যদি (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	যদি (in_colour != 1) {
		in_colour = 0;
		ঘূৰাই false;
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
	বাবে (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
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
			strlcpy(tmp, tgoto(bold, 0, 0), পরিমাণ(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), পরিমাণ(tmp));
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
	strlcpy(tmp, _ch, পরিমাণ(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	ঘূৰাই true;
}

স্থির পূর্ণাঙ্ক
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	সংরচনা winsize ws;
	ৱর্ণ *env = getenv("COLUMNS");
	ৱর্ণ *p;
	পূর্ণাঙ্ক i;

	যদি (env) {
		i = strtoimax(env, &p, 10);
		যদি (!*p)
			ঘূৰাই i;
	}

	যদি (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		ঘূৰাই ws.ws_col;

	ঘূৰাই DEFAULT_COLS;
}

শূন্য
eprefix(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

স্থির শূন্য EINFO_PRINTF(2, 0)
elogv(পূর্ণাঙ্ক level, ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, va_list ap)
{
	ৱর্ণ *e = getenv("EINFO_LOG");
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
elog(পূর্ণাঙ্ক level, ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

স্থির পূর্ণাঙ্ক
_eindent(FILE * EINFO_RESTRICT stream)
{
	ৱর্ণ *env = getenv("EINFO_INDENT");
	পূর্ণাঙ্ক amount = 0;

	যদি (env) {
		errno = 0;
		amount = strtoimax(env, রিক্ত, 0);
		যদি (errno != 0 || amount < 0)
			amount = 0;
		অন্যথা যদি (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	ঘূৰাই fprintf(stream, "%*s", amount, "");
}

স্থির ধ্রুৱ ৱর্ণ *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned পূর্ণাঙ্ক i;

	যদি (!colour_terminal(f))
		ঘূৰাই "";

	বাবে (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		যদি (ecolors[i].color == color)
			ঘূৰাই ecolors_str[i];
	ঘূৰাই "";
}

ধ্রুৱ ৱর্ণ *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	যদি (!isatty(fileno(f))) {
		f = stderr;
		যদি (!isatty(fileno(f))) {
			f = stdin;
			যদি (!isatty(fileno(f)))
				f = রিক্ত;
		}
	}

	ঘূৰাই _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

স্থির পূর্ণাঙ্ক EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, va_list va)
{
	পূর্ণাঙ্ক retval = 0;
	ৱর্ণ *last = getenv("EINFO_LASTCMD");
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
	ঘূৰাই retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

পূর্ণাঙ্ক
einfon(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	পূর্ণাঙ্ক retval;
	va_list ap;

	যদি (!fmt || is_quiet())
		ঘূৰাই 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	ঘূৰাই retval;
}

পূর্ণাঙ্ক
ewarnn(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	পূর্ণাঙ্ক retval;
	va_list ap;

	যদি (!fmt || is_quiet())
		ঘূৰাই 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	ঘূৰাই retval;
}

পূর্ণাঙ্ক
eerrorn(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	পূর্ণাঙ্ক retval;
	va_list ap;

	যদি (!fmt || is_really_quiet())
		ঘূৰাই 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	ঘূৰাই retval;
}

পূর্ণাঙ্ক
einfo(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	পূর্ণাঙ্ক retval;
	va_list ap;

	যদি (!fmt || is_quiet())
		ঘূৰাই 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	ঘূৰাই retval;
}

পূর্ণাঙ্ক
ewarn(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	পূর্ণাঙ্ক retval;
	va_list ap;

	যদি (!fmt || is_quiet())
		ঘূৰাই 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	ঘূৰাই retval;
}

শূন্য
ewarnx(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
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

পূর্ণাঙ্ক
eerror(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	পূর্ণাঙ্ক retval;
	va_list ap;

	যদি (!fmt || is_really_quiet())
		ঘূৰাই 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	ঘূৰাই retval;
}

শূন্য
eerrorx(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
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

পূর্ণাঙ্ক
ebegin(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	পূর্ণাঙ্ক retval;
	va_list ap;

	যদি (!fmt || is_quiet())
		ঘূৰাই 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	যদি (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	ঘূৰাই retval;
}

স্থির শূন্য
_eend(FILE * EINFO_RESTRICT fp, পূর্ণাঙ্ক col, ECOLOR color, ধ্রুৱ ৱর্ণ *বাৰ্তা)
{
	পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক cols;

	যদি (!বাৰ্তা)
		ঘূৰাই;

	cols = get_term_columns(fp) - (strlen(বাৰ্তা) + 5);

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
		    ecolor(ECOLOR_BRACKET), ecolor(color), বাৰ্তা,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} অন্যথা {
		যদি (col > 0)
			বাবে (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", বাৰ্তা);
	}
}

স্থির পূর্ণাঙ্ক EINFO_PRINTF(3, 0)
_do_eend(ধ্রুৱ ৱর্ণ *cmd, পূর্ণাঙ্ক retval,
    ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, va_list ap)
{
	পূর্ণাঙ্ক col = 0;
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
	ঘূৰাই retval;
}

পূর্ণাঙ্ক
eend(পূর্ণাঙ্ক retval, ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	যদি (is_quiet())
		ঘূৰাই retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	ঘূৰাই retval;
}

পূর্ণাঙ্ক
ewend(পূর্ণাঙ্ক retval, ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	যদি (is_quiet())
		ঘূৰাই retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	ঘূৰাই retval;
}

শূন্য
ebracket(পূর্ণাঙ্ক col, ECOLOR color, ধ্রুৱ ৱর্ণ *বাৰ্তা)
{
	_eend(stdout, col, color, বাৰ্তা);
}

শূন্য
eindent(শূন্য)
{
	ৱর্ণ *env = getenv("EINFO_INDENT");
	পূর্ণাঙ্ক amount = 0;
	ৱর্ণ *অঙ্ক;

	যদি (env) {
		errno = 0;
		amount = strtoimax(env, রিক্ত, 0);
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
	ৱর্ণ *env = getenv("EINFO_INDENT");
	পূর্ণাঙ্ক amount = 0;
	ৱর্ণ *অঙ্ক = রিক্ত;
	পূর্ণাঙ্ক serrno = errno;

	যদি (!env)
		ঘূৰাই;
	errno = 0;
	amount = strtoimax(env, রিক্ত, 0);
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

পূর্ণাঙ্ক
einfovn(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	পূর্ণাঙ্ক retval;
	va_list ap;

	যদি (!fmt || !is_verbose())
		ঘূৰাই 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	ঘূৰাই retval;
}

পূর্ণাঙ্ক
ewarnvn(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	পূর্ণাঙ্ক retval;
	va_list ap;

	যদি (!fmt || !is_verbose())
		ঘূৰাই 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	ঘূৰাই retval;
}

পূর্ণাঙ্ক
einfov(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	পূর্ণাঙ্ক retval;
	va_list ap;

	যদি (!fmt || !is_verbose())
		ঘূৰাই 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	ঘূৰাই retval;
}

পূর্ণাঙ্ক
ewarnv(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	পূর্ণাঙ্ক retval;
	va_list ap;

	যদি (!fmt || !is_verbose())
		ঘূৰাই 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	ঘূৰাই retval;
}

পূর্ণাঙ্ক
ebeginv(ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	পূর্ণাঙ্ক retval;
	va_list ap;

	যদি (!fmt || !is_verbose())
		ঘূৰাই 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	যদি (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	ঘূৰাই retval;
}

পূর্ণাঙ্ক
eendv(পূর্ণাঙ্ক retval, ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	যদি (!is_verbose())
		ঘূৰাই 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	ঘূৰাই retval;
}

পূর্ণাঙ্ক
ewendv(পূর্ণাঙ্ক retval, ধ্রুৱ ৱর্ণ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	যদি (!is_verbose())
		ঘূৰাই 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	ঘূৰাই retval;
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
