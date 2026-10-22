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
#تعریف_کړه DEFAULT_ستون_شمېر		 80

#تعریف_کړه OK			"ok"
#تعریف_کړه NOT_OK			"!!"

/* Number of spaces for an indent */
#تعریف_کړه INDENT_پلنوالی		2

/* How wide can the indent go? */
#تعریف_کړه INDENT_MAX		40

/* Default colours */
#تعریف_کړه GOOD                    2
#تعریف_کړه WARN                    3
#تعریف_کړه BAD                     1
#تعریف_کړه HILITE                  6
#تعریف_کړه BRACKET                 4

/* ANSI escape codes which mimic termcap */
#تعریف_کړه AF "\033[3%dm"
#تعریف_کړه CE "\033[K"
#تعریف_کړه CH "\033[%dC"
#تعریف_کړه MD "\033[1m"
#تعریف_کړه ME "\033[m"
#تعریف_کړه پورته "\033[A"

#تعریف_کړه _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), حجم(_d));
#تعریف_کړه ټاکل_CAP(_v) افعل {						      \
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
	{ ECOLOR_نورمال,  0,       عدم      },
};
ساكن ثابت حرف *ecolors_str[ARRAY_اندازه(ecolors)];

ساكن حرف *flush = عدم;
ساكن حرف *پورته_2 = عدم;
ساكن حرف *goto_ستنه = عدم;

ساكن ثابت حرف *غړی = عدم;
ساكن bool غړی_is_cons25 = false;

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

#که_نه_تعریف_شوی HAVE_STRLCPY
ساكن size_t
strlcat(حرف *dst, ثابت حرف *src, size_t اندازه)
{
	حرف *d = dst;
	ثابت حرف *s = src;
	size_t src_n = اندازه;
	size_t dst_n;

	طالما (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = اندازه - dst_n;

	إذا (src_n == 0)
		أرجع dst_n + مزي_اوږدوالی(src);

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
#د_که_پای

ساكن bool
yesno(ثابت حرف *ارزښت)
{
	إذا (!ارزښت) {
		errno = ENOENT;
		أرجع false;
	}

	إذا (strcasecmp(ارزښت, "yes") == 0 ||
	    strcasecmp(ارزښت, "y") == 0 ||
	    strcasecmp(ارزښت, "true") == 0 ||
	    strcasecmp(ارزښت, "on") == 0 ||
	    strcasecmp(ارزښت, "1") == 0)
		أرجع true;

	إذا (strcasecmp(ارزښت, "no") != 0 &&
	    strcasecmp(ارزښت, "n") != 0 &&
	    strcasecmp(ارزښت, "false") != 0 &&
	    strcasecmp(ارزښت, "off") != 0 &&
	    strcasecmp(ارزښت, "0") != 0)
		errno = EINVAL;

	أرجع false;
}

ساكن bool
noyes(ثابت حرف *ارزښت)
{
	صحيح serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(ارزښت);
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
tgoto(ثابت حرف *cap, صحيح ستون, صحيح کرښه)
{
	ساكن حرف buf[20];
	حرف *p, *e, c, dbuf[6];
	صحيح oncol = 0, which = کرښه, i;

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
			ستون++;
			کرښه++;
			which++;
			استمر;
		افتراضي:
			errno = EINVAL;
			أرجع عدم;
		}

		oncol = 1 - oncol;
		which = oncol ? ستون : کرښه;
	}
	*p = '\0';
	أرجع buf;
}

ساكن bool
رنګ_terminal(FILE * EINFO_RESTRICT f)
{
	ساكن صحيح دننه_رنګ = -1;
	حرف *e, *ee, *پای_ټکی, *d, *p;
	صحيح c;
	ثابت حرف *_af = عدم, *_ce = عدم, *_ch = عدم;
	ثابت حرف *_md = عدم, *_me = عدم, *پورته_3 = عدم;
	ثابت حرف *bold;
	حرف tmp[100];
	بینښه صحيح i = 0;

	إذا (f && !isatty(fileno(f)))
		أرجع false;

	إذا (noyes(getenv("EINFO_COLOR")))
		أرجع false;

	إذا (دننه_رنګ == 0)
		أرجع false;
	إذا (دننه_رنګ == 1)
		أرجع true;

	غړی_is_cons25 = false;
	إذا (!غړی) {
		غړی = getenv("TERM");
		إذا (!غړی)
			أرجع false;
	}
	إذا (strcmp(غړی, "cons25") == 0)
		غړی_is_cons25 = true;

	إذا (مزي_پلټنه(غړی, "color"))
		دننه_رنګ = 1;

	طالما (color_terms[i] && دننه_رنګ != 1) {
		إذا (strcmp(color_terms[i], غړی) == 0) {
			دننه_رنګ = 1;
		}
		i++;
	}

	إذا (دننه_رنګ != 1) {
		دننه_رنګ = 0;
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
	إذا (!پورته_3)
		پورته_3 = پورته;

	/* Now setup our colours */
	p = ebuffer;
	لكل (i = 0; i < ARRAY_اندازه(ecolors); ++i) {
		tmp[0] = '\0';
		إذا (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			إذا ((e = getenv("EINFO_COLOR"))) {
				ee = مزي_پلټنه(e, ecolors[i].name);
				إذا (ee)
					ee += مزي_اوږدوالی(ecolors[i].name);

				إذا (ee && *ee == '=') {
					d = strdup(ee + 1);
					إذا (d) {
						پای_ټکی = strchr(d, ':');
						إذا (پای_ټکی)
							*پای_ټکی = '\0';
						c = atoi(d);
						پای_ټکی = strchr(d, ';');
						إذا (پای_ټکی && *++پای_ټکی == '0')
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
			ټاکل_CAP(ecolors_str[i]);
		وإلا
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	ټاکل_CAP(flush);
	_GET_CAP(tmp, پورته_3);
	ټاکل_CAP(پورته_2);
	strlcpy(tmp, _ch, حجم(tmp));
	ټاکل_CAP(goto_ستنه);

	دننه_رنګ = 1;
	أرجع true;
}

ساكن صحيح
get_غړی_columns(FILE * EINFO_RESTRICT بهیر)
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

	إذا (ioctl(fileno(بهیر), TIOCGWINSZ, &ws) == 0)
		أرجع ws.ws_col;

	أرجع DEFAULT_ستون_شمېر;
}

فراغ
eprefix(ثابت حرف *EINFO_RESTRICT مختاړی)
{
	_eprefix = مختاړی;
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
_eindent(FILE * EINFO_RESTRICT بهیر)
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

	أرجع fprintf(بهیر, "%*s", amount, "");
}

ساكن ثابت حرف *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	بینښه صحيح i;

	إذا (!رنګ_terminal(f))
		أرجع "";

	لكل (i = 0; i < ARRAY_اندازه(ecolors); ++i)
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

#تعریف_کړه LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

ساكن صحيح EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, ثابت حرف *EINFO_RESTRICT fmt, va_list va)
{
	صحيح retval = 0;
	حرف *وروستی = getenv("EINFO_LASTCMD");
	va_list ap;

	إذا (وروستی &&
	    *وروستی &&
	    !رنګ_terminal(f) &&
	    strcmp(وروستی, "ewarn") != 0 &&
	    وروستی[مزي_اوږدوالی(وروستی) - 1] == 'n')
		fprintf(f, "\n");
	إذا (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_نورمال));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_نورمال));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	إذا (رنګ_terminal(f))
		fprintf(f, "%s", flush);
	أرجع retval;
}

#تعریف_کړه _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#تعریف_کړه _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#تعریف_کړه _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

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
	retval += چاپ_کړه("\n");
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
	وتل(EXIT_FAILURE);
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
	وتل(EXIT_FAILURE);
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
	retval += چاپ_کړه(" ...");
	إذا (رنګ_terminal(stdout))
		retval += چاپ_کړه("\n");
	LASTCMD("ebegin");
	أرجع retval;
}

ساكن فراغ
_eend(FILE * EINFO_RESTRICT fp, صحيح ستون, ECOLOR color, ثابت حرف *پیغام_2)
{
	صحيح i;
	صحيح ستون_شمېر;

	إذا (!پیغام_2)
		أرجع;

	ستون_شمېر = get_غړی_columns(fp) - (مزي_اوږدوالی(پیغام_2) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	إذا (!غړی) {
		غړی = getenv("TERM");
		إذا (غړی && strcmp(غړی, "cons25") == 0)
			غړی_is_cons25 = true;
		وإلا
			غړی_is_cons25 = false;
	}
	إذا (غړی_is_cons25)
		ستون_شمېر--;

	إذا (ستون_شمېر > 0 && رنګ_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", پورته_2, tgoto(goto_ستنه, 0, ستون_شمېر),
		    ecolor(ECOLOR_BRACKET), ecolor(color), پیغام_2,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_نورمال));
	} وإلا {
		إذا (ستون > 0)
			لكل (i = 0; i < ستون_شمېر - ستون; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", پیغام_2);
	}
}

ساكن صحيح EINFO_PRINTF(3, 0)
_do_eend(ثابت حرف *cmd, صحيح retval,
    ثابت حرف *EINFO_RESTRICT fmt, va_list ap)
{
	صحيح ستون = 0;
	FILE *fp = stdout;
	va_list apc;

	إذا (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		إذا (strcmp(cmd, "ewend") == 0)
			ستون = _ewarnvn(fmt, apc);
		وإلا
			ستون = _eerrorvn(fmt, apc);
		ستون += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, ستون,
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
ebracket(صحيح ستون, ECOLOR color, ثابت حرف *پیغام_2)
{
	_eend(stdout, ستون, color, پیغام_2);
}

فراغ
eindent(فراغ)
{
	حرف *env = getenv("EINFO_INDENT");
	صحيح amount = 0;
	حرف *عدد;

	إذا (env) {
		errno = 0;
		amount = strtoimax(env, عدم, 0);
		إذا (errno != 0)
			amount = 0;
	}
	amount += INDENT_پلنوالی;
	xasprintf(&عدد, "%08d", amount);
	setenv("EINFO_INDENT", عدد, 1);
	حرر(عدد);
}

فراغ eoutdent(فراغ)
{
	حرف *env = getenv("EINFO_INDENT");
	صحيح amount = 0;
	حرف *عدد = عدم;
	صحيح serrno = errno;

	إذا (!env)
		أرجع;
	errno = 0;
	amount = strtoimax(env, عدم, 0);
	إذا (errno != 0)
		amount = 0;
	وإلا
		amount -= INDENT_پلنوالی;
	إذا (amount <= 0)
		unsetenv("EINFO_INDENT");
	وإلا {
		xasprintf(&عدد, "%08d", amount);
		setenv("EINFO_INDENT", عدد, 1);
		حرر(عدد);
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
	retval += چاپ_کړه("\n");
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
	retval += چاپ_کړه("\n");
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
	retval += چاپ_کړه(" ...");
	إذا (رنګ_terminal(stdout))
		retval += چاپ_کړه("\n");
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
