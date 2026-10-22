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
#وصف_ڪريو DEFAULT_کانن_ڳڻپ		 80

#وصف_ڪريو OK			"ok"
#وصف_ڪريو NOT_OK			"!!"

/* Number of spaces for an indent */
#وصف_ڪريو INDENT_ويڪر		2

/* How wide can the indent go? */
#وصف_ڪريو INDENT_MAX		40

/* Default colours */
#وصف_ڪريو GOOD                    2
#وصف_ڪريو WARN                    3
#وصف_ڪريو BAD                     1
#وصف_ڪريو HILITE                  6
#وصف_ڪريو BRACKET                 4

/* ANSI escape codes which mimic termcap */
#وصف_ڪريو AF "\033[3%dm"
#وصف_ڪريو CE "\033[K"
#وصف_ڪريو CH "\033[%dC"
#وصف_ڪريو MD "\033[1m"
#وصف_ڪريو ME "\033[m"
#وصف_ڪريو مٿي "\033[A"

#وصف_ڪريو _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), حجم(_d));
#وصف_ڪريو سونپيو_CAP(_v) افعل {						      \
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
	{ ECOLOR_عام,  0,       عدم      },
};
ساكن ثابت حرف *ecolors_str[ARRAY_سائيز(ecolors)];

ساكن حرف *flush = عدم;
ساكن حرف *مٿي_2 = عدم;
ساكن حرف *goto_ڪالم = عدم;

ساكن ثابت حرف *رڪن = عدم;
ساكن bool رڪن_is_cons25 = false;

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

#جيڪڏهن_وصف_نه_ٿيل HAVE_STRLCPY
ساكن size_t
strlcat(حرف *dst, ثابت حرف *src, size_t سائيز)
{
	حرف *d = dst;
	ثابت حرف *s = src;
	size_t src_n = سائيز;
	size_t dst_n;

	طالما (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = سائيز - dst_n;

	إذا (src_n == 0)
		أرجع dst_n + لڙي_ڊيگهه(src);

	طالما (*s != '\0') {
		إذا (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	أرجع dst_n + (s - src);
}
#جيڪڏهن_پورو

ساكن bool
yesno(ثابت حرف *قدر)
{
	إذا (!قدر) {
		errno = ENOENT;
		أرجع false;
	}

	إذا (strcasecmp(قدر, "yes") == 0 ||
	    strcasecmp(قدر, "y") == 0 ||
	    strcasecmp(قدر, "true") == 0 ||
	    strcasecmp(قدر, "on") == 0 ||
	    strcasecmp(قدر, "1") == 0)
		أرجع true;

	إذا (strcasecmp(قدر, "no") != 0 &&
	    strcasecmp(قدر, "n") != 0 &&
	    strcasecmp(قدر, "false") != 0 &&
	    strcasecmp(قدر, "off") != 0 &&
	    strcasecmp(قدر, "0") != 0)
		errno = EINVAL;

	أرجع false;
}

ساكن bool
noyes(ثابت حرف *قدر)
{
	صحيح serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(قدر);
	إذا (errno == 0) {
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
tgoto(ثابت حرف *cap, صحيح کانو, صحيح سٽ)
{
	ساكن حرف buf[20];
	حرف *p, *e, c, dbuf[6];
	صحيح oncol = 0, which = سٽ, i;

	p = buf;
	e = p + حجم(buf);
	طالما ((c = *cap++)) {
		إذا (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			إذا (p >= e) {
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
			افعل
				dbuf[i++] = which % 10 | '0';
			طالما ((which /= 10));
			إذا (c != 'd') {
				c -= '0';
				إذا (i > c) {
					errno = EINVAL;
					أرجع عدم;
				}
				طالما (i < c)
					dbuf[i++] = '0';
			}
			إذا (p + i >= e) {
				errno = E2BIG;
				أرجع عدم;
			}
			افعل
				*p++ = dbuf[--i];
			طالما (i);
			اكسر;
		حالة 'r':
			oncol = 0;
			اكسر;
		حالة 'i':
			کانو++;
			سٽ++;
			which++;
			استمر;
		افتراضي:
			errno = EINVAL;
			أرجع عدم;
		}

		oncol = 1 - oncol;
		which = oncol ? کانو : سٽ;
	}
	*p = '\0';
	أرجع buf;
}

ساكن bool
رنگ_terminal(FILE * EINFO_RESTRICT f)
{
	ساكن صحيح اندريون_رنگ = -1;
	حرف *e, *ee, *پڇاڙي, *d, *p;
	صحيح c;
	ثابت حرف *_af = عدم, *_ce = عدم, *_ch = عدم;
	ثابت حرف *_md = عدم, *_me = عدم, *مٿي_3 = عدم;
	ثابت حرف *bold;
	حرف tmp[100];
	بنانشان صحيح i = 0;

	إذا (f && !isatty(fileno(f)))
		أرجع false;

	إذا (noyes(getenv("EINFO_COLOR")))
		أرجع false;

	إذا (اندريون_رنگ == 0)
		أرجع false;
	إذا (اندريون_رنگ == 1)
		أرجع true;

	رڪن_is_cons25 = false;
	إذا (!رڪن) {
		رڪن = getenv("TERM");
		إذا (!رڪن)
			أرجع false;
	}
	إذا (strcmp(رڪن, "cons25") == 0)
		رڪن_is_cons25 = true;

	إذا (لڙي_ڳولا(رڪن, "color"))
		اندريون_رنگ = 1;

	طالما (color_terms[i] && اندريون_رنگ != 1) {
		إذا (strcmp(color_terms[i], رڪن) == 0) {
			اندريون_رنگ = 1;
		}
		i++;
	}

	إذا (اندريون_رنگ != 1) {
		اندريون_رنگ = 0;
		أرجع false;
	}

	إذا (!_af)
		_af = AF;
	إذا (!_ce)
		_ce = CE;
	إذا (!_ch)
		_ch = CH;
	إذا (!_md)
		_md = MD;
	إذا (!_me)
		_me = ME;
	إذا (!مٿي_3)
		مٿي_3 = مٿي;

	/* Now setup our colours */
	p = ebuffer;
	لكل (i = 0; i < ARRAY_سائيز(ecolors); ++i) {
		tmp[0] = '\0';
		إذا (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			إذا ((e = getenv("EINFO_COLOR"))) {
				ee = لڙي_ڳولا(e, ecolors[i].name);
				إذا (ee)
					ee += لڙي_ڊيگهه(ecolors[i].name);

				إذا (ee && *ee == '=') {
					d = strdup(ee + 1);
					إذا (d) {
						پڇاڙي = strchr(d, ':');
						إذا (پڇاڙي)
							*پڇاڙي = '\0';
						c = atoi(d);
						پڇاڙي = strchr(d, ';');
						إذا (پڇاڙي && *++پڇاڙي == '0')
							bold = _me;
						حرر(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), حجم(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), حجم(tmp));
		} وإلا
			_GET_CAP(tmp, _me);

		إذا (tmp[0])
			سونپيو_CAP(ecolors_str[i]);
		وإلا
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	سونپيو_CAP(flush);
	_GET_CAP(tmp, مٿي_3);
	سونپيو_CAP(مٿي_2);
	strlcpy(tmp, _ch, حجم(tmp));
	سونپيو_CAP(goto_ڪالم);

	اندريون_رنگ = 1;
	أرجع true;
}

ساكن صحيح
get_رڪن_columns(FILE * EINFO_RESTRICT وهڪرو)
{
	بنية winsize ws;
	حرف *env = getenv("COLUMNS");
	حرف *p;
	صحيح i;

	إذا (env) {
		i = strtoimax(env, &p, 10);
		إذا (!*p)
			أرجع i;
	}

	إذا (ioctl(fileno(وهڪرو), TIOCGWINSZ, &ws) == 0)
		أرجع ws.ws_col;

	أرجع DEFAULT_کانن_ڳڻپ;
}

فراغ
eprefix(ثابت حرف *EINFO_RESTRICT اڳياڙي)
{
	_eprefix = اڳياڙي;
}

ساكن فراغ EINFO_PRINTF(2, 0)
elogv(صحيح level, ثابت حرف *EINFO_RESTRICT fmt, va_list ap)
{
	حرف *e = getenv("EINFO_LOG");
	va_list apc;

	إذا (fmt && e) {
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
_eindent(FILE * EINFO_RESTRICT وهڪرو)
{
	حرف *env = getenv("EINFO_INDENT");
	صحيح amount = 0;

	إذا (env) {
		errno = 0;
		amount = strtoimax(env, عدم, 0);
		إذا (errno != 0 || amount < 0)
			amount = 0;
		وإلا إذا (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	أرجع fprintf(وهڪرو, "%*s", amount, "");
}

ساكن ثابت حرف *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	بنانشان صحيح i;

	إذا (!رنگ_terminal(f))
		أرجع "";

	لكل (i = 0; i < ARRAY_سائيز(ecolors); ++i)
		إذا (ecolors[i].color == color)
			أرجع ecolors_str[i];
	أرجع "";
}

ثابت حرف *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	إذا (!isatty(fileno(f))) {
		f = stderr;
		إذا (!isatty(fileno(f))) {
			f = stdin;
			إذا (!isatty(fileno(f)))
				f = عدم;
		}
	}

	أرجع _ecolor(f, color);
}

#وصف_ڪريو LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

ساكن صحيح EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, ثابت حرف *EINFO_RESTRICT fmt, va_list va)
{
	صحيح retval = 0;
	حرف *آخري = getenv("EINFO_LASTCMD");
	va_list ap;

	إذا (آخري &&
	    *آخري &&
	    !رنگ_terminal(f) &&
	    strcmp(آخري, "ewarn") != 0 &&
	    آخري[لڙي_ڊيگهه(آخري) - 1] == 'n')
		fprintf(f, "\n");
	إذا (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_عام));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_عام));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	إذا (رنگ_terminal(f))
		fprintf(f, "%s", flush);
	أرجع retval;
}

#وصف_ڪريو _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#وصف_ڪريو _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#وصف_ڪريو _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

صحيح
einfon(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	إذا (!fmt || is_quiet())
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

	إذا (!fmt || is_quiet())
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

	إذا (!fmt || is_really_quiet())
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

	إذا (!fmt || is_quiet())
		أرجع 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += لکو("\n");
	va_end(ap);
	LASTCMD("einfo");
	أرجع retval;
}

صحيح
ewarn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	إذا (!fmt || is_quiet())
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

	إذا (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	ٻاهر_نڪرو(EXIT_FAILURE);
}

صحيح
eerror(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	إذا (!fmt || is_really_quiet())
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

	إذا (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	ٻاهر_نڪرو(EXIT_FAILURE);
}

صحيح
ebegin(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	إذا (!fmt || is_quiet())
		أرجع 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += لکو(" ...");
	إذا (رنگ_terminal(stdout))
		retval += لکو("\n");
	LASTCMD("ebegin");
	أرجع retval;
}

ساكن فراغ
_eend(FILE * EINFO_RESTRICT fp, صحيح کانو, ECOLOR color, ثابت حرف *نياپو)
{
	صحيح i;
	صحيح کانن_ڳڻپ;

	إذا (!نياپو)
		أرجع;

	کانن_ڳڻپ = get_رڪن_columns(fp) - (لڙي_ڊيگهه(نياپو) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	إذا (!رڪن) {
		رڪن = getenv("TERM");
		إذا (رڪن && strcmp(رڪن, "cons25") == 0)
			رڪن_is_cons25 = true;
		وإلا
			رڪن_is_cons25 = false;
	}
	إذا (رڪن_is_cons25)
		کانن_ڳڻپ--;

	إذا (کانن_ڳڻپ > 0 && رنگ_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", مٿي_2, tgoto(goto_ڪالم, 0, کانن_ڳڻپ),
		    ecolor(ECOLOR_BRACKET), ecolor(color), نياپو,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_عام));
	} وإلا {
		إذا (کانو > 0)
			لكل (i = 0; i < کانن_ڳڻپ - کانو; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", نياپو);
	}
}

ساكن صحيح EINFO_PRINTF(3, 0)
_do_eend(ثابت حرف *cmd, صحيح retval,
    ثابت حرف *EINFO_RESTRICT fmt, va_list ap)
{
	صحيح کانو = 0;
	FILE *fp = stdout;
	va_list apc;

	إذا (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		إذا (strcmp(cmd, "ewend") == 0)
			کانو = _ewarnvn(fmt, apc);
		وإلا
			کانو = _eerrorvn(fmt, apc);
		کانو += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, کانو,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	أرجع retval;
}

صحيح
eend(صحيح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	إذا (is_quiet())
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

	إذا (is_quiet())
		أرجع retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	أرجع retval;
}

فراغ
ebracket(صحيح کانو, ECOLOR color, ثابت حرف *نياپو)
{
	_eend(stdout, کانو, color, نياپو);
}

فراغ
eindent(فراغ)
{
	حرف *env = getenv("EINFO_INDENT");
	صحيح amount = 0;
	حرف *انگ;

	إذا (env) {
		errno = 0;
		amount = strtoimax(env, عدم, 0);
		إذا (errno != 0)
			amount = 0;
	}
	amount += INDENT_ويڪر;
	xasprintf(&انگ, "%08d", amount);
	setenv("EINFO_INDENT", انگ, 1);
	حرر(انگ);
}

فراغ eoutdent(فراغ)
{
	حرف *env = getenv("EINFO_INDENT");
	صحيح amount = 0;
	حرف *انگ = عدم;
	صحيح serrno = errno;

	إذا (!env)
		أرجع;
	errno = 0;
	amount = strtoimax(env, عدم, 0);
	إذا (errno != 0)
		amount = 0;
	وإلا
		amount -= INDENT_ويڪر;
	إذا (amount <= 0)
		unsetenv("EINFO_INDENT");
	وإلا {
		xasprintf(&انگ, "%08d", amount);
		setenv("EINFO_INDENT", انگ, 1);
		حرر(انگ);
	}
	errno = serrno;
}

صحيح
einfovn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	إذا (!fmt || !is_verbose())
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

	إذا (!fmt || !is_verbose())
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

	إذا (!fmt || !is_verbose())
		أرجع 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += لکو("\n");
	va_end(ap);
	LASTCMD("einfov");
	أرجع retval;
}

صحيح
ewarnv(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	إذا (!fmt || !is_verbose())
		أرجع 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += لکو("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	أرجع retval;
}

صحيح
ebeginv(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحيح retval;
	va_list ap;

	إذا (!fmt || !is_verbose())
		أرجع 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += لکو(" ...");
	إذا (رنگ_terminal(stdout))
		retval += لکو("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	أرجع retval;
}

صحيح
eendv(صحيح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	إذا (!is_verbose())
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

	إذا (!is_verbose())
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
	إذا (is_verbose())
		eindent();
}

فراغ
eoutdentv(فراغ)
{
	إذا (is_verbose())
		eoutdent();
}
