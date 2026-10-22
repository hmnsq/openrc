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
#تعریف_کرو DEFAULT_ستون_تعداد		 80

#تعریف_کرو OK			"ok"
#تعریف_کرو NOT_OK			"!!"

/* Number of spaces for an indent */
#تعریف_کرو INDENT_چوڑائی		2

/* How wide can the indent go? */
#تعریف_کرو INDENT_MAX		40

/* Default colours */
#تعریف_کرو GOOD                    2
#تعریف_کرو WARN                    3
#تعریف_کرو BAD                     1
#تعریف_کرو HILITE                  6
#تعریف_کرو BRACKET                 4

/* ANSI escape codes which mimic termcap */
#تعریف_کرو AF "\033[3%dm"
#تعریف_کرو CE "\033[K"
#تعریف_کرو CH "\033[%dC"
#تعریف_کرو MD "\033[1m"
#تعریف_کرو ME "\033[m"
#تعریف_کرو اوپر "\033[A"

#تعریف_کرو _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), حجم(_d));
#تعریف_کرو تفویض_CAP(_v) کرو {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, حجم(ebuffer) - (p - ebuffer)) + 1;    \
	} جب (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
ساکن ثابت حرف *_eprefix = تہی;

/* Buffers and structures to hold the final colours */
ساکن حرف ebuffer[100];
ساخت ecolor {
	ECOLOR color;
	صحیح def;
	ثابت حرف *name;
};
ساکن حرف nullstr = '\0';

ساکن ثابت ساخت ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_معمول,  0,       تہی      },
};
ساکن ثابت حرف *ecolors_str[ARRAY_سائز(ecolors)];

ساکن حرف *flush = تہی;
ساکن حرف *اوپر_2 = تہی;
ساکن حرف *goto_عمود = تہی;

ساکن ثابت حرف *رکن = تہی;
ساکن bool رکن_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
ساکن ثابت حرف *ثابت color_terms[] = {
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
	تہی
};

#اگر_غیر_تعریف_شدہ HAVE_STRLCPY
ساکن size_t
strlcat(حرف *dst, ثابت حرف *src, size_t سائز)
{
	حرف *d = dst;
	ثابت حرف *s = src;
	size_t src_n = سائز;
	size_t dst_n;

	جب (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = سائز - dst_n;

	اگر (src_n == 0)
		واپس dst_n + لڑی_لمبائی(src);

	جب (*s != '\0') {
		اگر (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	واپس dst_n + (s - src);
}
#اگر_ختم

ساکن bool
yesno(ثابت حرف *قدر)
{
	اگر (!قدر) {
		errno = ENOENT;
		واپس false;
	}

	اگر (strcasecmp(قدر, "yes") == 0 ||
	    strcasecmp(قدر, "y") == 0 ||
	    strcasecmp(قدر, "true") == 0 ||
	    strcasecmp(قدر, "on") == 0 ||
	    strcasecmp(قدر, "1") == 0)
		واپس true;

	اگر (strcasecmp(قدر, "no") != 0 &&
	    strcasecmp(قدر, "n") != 0 &&
	    strcasecmp(قدر, "false") != 0 &&
	    strcasecmp(قدر, "off") != 0 &&
	    strcasecmp(قدر, "0") != 0)
		errno = EINVAL;

	واپس false;
}

ساکن bool
noyes(ثابت حرف *قدر)
{
	صحیح serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(قدر);
	اگر (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	واپس retval;
}

ساکن bool
is_quiet(خالی)
{
	واپس yesno(getenv("EINFO_QUIET"));
}

ساکن bool
is_really_quiet(خالی)
{
	واپس yesno(getenv("EERROR_QUIET"));
}

ساکن bool
is_verbose(خالی)
{
	واپس yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
ساکن حرف *
tgoto(ثابت حرف *cap, صحیح ستون, صحیح سطر)
{
	ساکن حرف buf[20];
	حرف *p, *e, c, dbuf[6];
	صحیح oncol = 0, which = سطر, i;

	p = buf;
	e = p + حجم(buf);
	جب ((c = *cap++)) {
		اگر (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			اگر (p >= e) {
				errno = E2BIG;
				واپس تہی;
			}
			جاری;
		}
		انتخاب (c) {
		حالت '3':
		حالت '2':
		حالت 'd':
			i = 0;
			کرو
				dbuf[i++] = which % 10 | '0';
			جب ((which /= 10));
			اگر (c != 'd') {
				c -= '0';
				اگر (i > c) {
					errno = EINVAL;
					واپس تہی;
				}
				جب (i < c)
					dbuf[i++] = '0';
			}
			اگر (p + i >= e) {
				errno = E2BIG;
				واپس تہی;
			}
			کرو
				*p++ = dbuf[--i];
			جب (i);
			توڑو;
		حالت 'r':
			oncol = 0;
			توڑو;
		حالت 'i':
			ستون++;
			سطر++;
			which++;
			جاری;
		طےشدہ:
			errno = EINVAL;
			واپس تہی;
		}

		oncol = 1 - oncol;
		which = oncol ? ستون : سطر;
	}
	*p = '\0';
	واپس buf;
}

ساکن bool
رنگ_terminal(FILE * EINFO_RESTRICT f)
{
	ساکن صحیح اندر_رنگ = -1;
	حرف *e, *ee, *انتہا, *d, *p;
	صحیح c;
	ثابت حرف *_af = تہی, *_ce = تہی, *_ch = تہی;
	ثابت حرف *_md = تہی, *_me = تہی, *اوپر_3 = تہی;
	ثابت حرف *bold;
	حرف tmp[100];
	غیرعلامتی صحیح i = 0;

	اگر (f && !isatty(fileno(f)))
		واپس false;

	اگر (noyes(getenv("EINFO_COLOR")))
		واپس false;

	اگر (اندر_رنگ == 0)
		واپس false;
	اگر (اندر_رنگ == 1)
		واپس true;

	رکن_is_cons25 = false;
	اگر (!رکن) {
		رکن = getenv("TERM");
		اگر (!رکن)
			واپس false;
	}
	اگر (strcmp(رکن, "cons25") == 0)
		رکن_is_cons25 = true;

	اگر (لڑی_تلاش(رکن, "color"))
		اندر_رنگ = 1;

	جب (color_terms[i] && اندر_رنگ != 1) {
		اگر (strcmp(color_terms[i], رکن) == 0) {
			اندر_رنگ = 1;
		}
		i++;
	}

	اگر (اندر_رنگ != 1) {
		اندر_رنگ = 0;
		واپس false;
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
	اگر (!اوپر_3)
		اوپر_3 = اوپر;

	/* Now setup our colours */
	p = ebuffer;
	برائے (i = 0; i < ARRAY_سائز(ecolors); ++i) {
		tmp[0] = '\0';
		اگر (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			اگر ((e = getenv("EINFO_COLOR"))) {
				ee = لڑی_تلاش(e, ecolors[i].name);
				اگر (ee)
					ee += لڑی_لمبائی(ecolors[i].name);

				اگر (ee && *ee == '=') {
					d = strdup(ee + 1);
					اگر (d) {
						انتہا = strchr(d, ':');
						اگر (انتہا)
							*انتہا = '\0';
						c = atoi(d);
						انتہا = strchr(d, ';');
						اگر (انتہا && *++انتہا == '0')
							bold = _me;
						آزاد(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), حجم(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), حجم(tmp));
		} ورنہ
			_GET_CAP(tmp, _me);

		اگر (tmp[0])
			تفویض_CAP(ecolors_str[i]);
		ورنہ
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	تفویض_CAP(flush);
	_GET_CAP(tmp, اوپر_3);
	تفویض_CAP(اوپر_2);
	strlcpy(tmp, _ch, حجم(tmp));
	تفویض_CAP(goto_عمود);

	اندر_رنگ = 1;
	واپس true;
}

ساکن صحیح
get_رکن_columns(FILE * EINFO_RESTRICT دھارا)
{
	ساخت winsize ws;
	حرف *env = getenv("COLUMNS");
	حرف *p;
	صحیح i;

	اگر (env) {
		i = strtoimax(env, &p, 10);
		اگر (!*p)
			واپس i;
	}

	اگر (ioctl(fileno(دھارا), TIOCGWINSZ, &ws) == 0)
		واپس ws.ws_col;

	واپس DEFAULT_ستون_تعداد;
}

خالی
eprefix(ثابت حرف *EINFO_RESTRICT سابقہ)
{
	_eprefix = سابقہ;
}

ساکن خالی EINFO_PRINTF(2, 0)
elogv(صحیح level, ثابت حرف *EINFO_RESTRICT fmt, va_list ap)
{
	حرف *e = getenv("EINFO_LOG");
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

خالی
elog(صحیح level, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

ساکن صحیح
_eindent(FILE * EINFO_RESTRICT دھارا)
{
	حرف *env = getenv("EINFO_INDENT");
	صحیح amount = 0;

	اگر (env) {
		errno = 0;
		amount = strtoimax(env, تہی, 0);
		اگر (errno != 0 || amount < 0)
			amount = 0;
		ورنہ اگر (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	واپس fprintf(دھارا, "%*s", amount, "");
}

ساکن ثابت حرف *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	غیرعلامتی صحیح i;

	اگر (!رنگ_terminal(f))
		واپس "";

	برائے (i = 0; i < ARRAY_سائز(ecolors); ++i)
		اگر (ecolors[i].color == color)
			واپس ecolors_str[i];
	واپس "";
}

ثابت حرف *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	اگر (!isatty(fileno(f))) {
		f = stderr;
		اگر (!isatty(fileno(f))) {
			f = stdin;
			اگر (!isatty(fileno(f)))
				f = تہی;
		}
	}

	واپس _ecolor(f, color);
}

#تعریف_کرو LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

ساکن صحیح EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, ثابت حرف *EINFO_RESTRICT fmt, va_list va)
{
	صحیح retval = 0;
	حرف *آخری = getenv("EINFO_LASTCMD");
	va_list ap;

	اگر (آخری &&
	    *آخری &&
	    !رنگ_terminal(f) &&
	    strcmp(آخری, "ewarn") != 0 &&
	    آخری[لڑی_لمبائی(آخری) - 1] == 'n')
		fprintf(f, "\n");
	اگر (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_معمول));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_معمول));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	اگر (رنگ_terminal(f))
		fprintf(f, "%s", flush);
	واپس retval;
}

#تعریف_کرو _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#تعریف_کرو _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#تعریف_کرو _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

صحیح
einfon(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_quiet())
		واپس 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	واپس retval;
}

صحیح
ewarnn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_quiet())
		واپس 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	واپس retval;
}

صحیح
eerrorn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_really_quiet())
		واپس 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	واپس retval;
}

صحیح
einfo(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_quiet())
		واپس 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += لکھو("\n");
	va_end(ap);
	LASTCMD("einfo");
	واپس retval;
}

صحیح
ewarn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_quiet())
		واپس 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	واپس retval;
}

خالی
ewarnx(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	اگر (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	باہر_نکلو(EXIT_FAILURE);
}

صحیح
eerror(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_really_quiet())
		واپس 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	واپس retval;
}

خالی
eerrorx(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	اگر (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	باہر_نکلو(EXIT_FAILURE);
}

صحیح
ebegin(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || is_quiet())
		واپس 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += لکھو(" ...");
	اگر (رنگ_terminal(stdout))
		retval += لکھو("\n");
	LASTCMD("ebegin");
	واپس retval;
}

ساکن خالی
_eend(FILE * EINFO_RESTRICT fp, صحیح ستون, ECOLOR color, ثابت حرف *پیغام_2)
{
	صحیح i;
	صحیح ستون_تعداد;

	اگر (!پیغام_2)
		واپس;

	ستون_تعداد = get_رکن_columns(fp) - (لڑی_لمبائی(پیغام_2) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	اگر (!رکن) {
		رکن = getenv("TERM");
		اگر (رکن && strcmp(رکن, "cons25") == 0)
			رکن_is_cons25 = true;
		ورنہ
			رکن_is_cons25 = false;
	}
	اگر (رکن_is_cons25)
		ستون_تعداد--;

	اگر (ستون_تعداد > 0 && رنگ_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", اوپر_2, tgoto(goto_عمود, 0, ستون_تعداد),
		    ecolor(ECOLOR_BRACKET), ecolor(color), پیغام_2,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_معمول));
	} ورنہ {
		اگر (ستون > 0)
			برائے (i = 0; i < ستون_تعداد - ستون; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", پیغام_2);
	}
}

ساکن صحیح EINFO_PRINTF(3, 0)
_do_eend(ثابت حرف *cmd, صحیح retval,
    ثابت حرف *EINFO_RESTRICT fmt, va_list ap)
{
	صحیح ستون = 0;
	FILE *fp = stdout;
	va_list apc;

	اگر (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		اگر (strcmp(cmd, "ewend") == 0)
			ستون = _ewarnvn(fmt, apc);
		ورنہ
			ستون = _eerrorvn(fmt, apc);
		ستون += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, ستون,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	واپس retval;
}

صحیح
eend(صحیح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	اگر (is_quiet())
		واپس retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	واپس retval;
}

صحیح
ewend(صحیح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	اگر (is_quiet())
		واپس retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	واپس retval;
}

خالی
ebracket(صحیح ستون, ECOLOR color, ثابت حرف *پیغام_2)
{
	_eend(stdout, ستون, color, پیغام_2);
}

خالی
eindent(خالی)
{
	حرف *env = getenv("EINFO_INDENT");
	صحیح amount = 0;
	حرف *عدد;

	اگر (env) {
		errno = 0;
		amount = strtoimax(env, تہی, 0);
		اگر (errno != 0)
			amount = 0;
	}
	amount += INDENT_چوڑائی;
	xasprintf(&عدد, "%08d", amount);
	setenv("EINFO_INDENT", عدد, 1);
	آزاد(عدد);
}

خالی eoutdent(خالی)
{
	حرف *env = getenv("EINFO_INDENT");
	صحیح amount = 0;
	حرف *عدد = تہی;
	صحیح serrno = errno;

	اگر (!env)
		واپس;
	errno = 0;
	amount = strtoimax(env, تہی, 0);
	اگر (errno != 0)
		amount = 0;
	ورنہ
		amount -= INDENT_چوڑائی;
	اگر (amount <= 0)
		unsetenv("EINFO_INDENT");
	ورنہ {
		xasprintf(&عدد, "%08d", amount);
		setenv("EINFO_INDENT", عدد, 1);
		آزاد(عدد);
	}
	errno = serrno;
}

صحیح
einfovn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || !is_verbose())
		واپس 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	واپس retval;
}

صحیح
ewarnvn(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || !is_verbose())
		واپس 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	واپس retval;
}

صحیح
einfov(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || !is_verbose())
		واپس 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += لکھو("\n");
	va_end(ap);
	LASTCMD("einfov");
	واپس retval;
}

صحیح
ewarnv(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || !is_verbose())
		واپس 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += لکھو("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	واپس retval;
}

صحیح
ebeginv(ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	صحیح retval;
	va_list ap;

	اگر (!fmt || !is_verbose())
		واپس 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += لکھو(" ...");
	اگر (رنگ_terminal(stdout))
		retval += لکھو("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	واپس retval;
}

صحیح
eendv(صحیح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	اگر (!is_verbose())
		واپس 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	واپس retval;
}

صحیح
ewendv(صحیح retval, ثابت حرف *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	اگر (!is_verbose())
		واپس 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	واپس retval;
}

خالی
eindentv(خالی)
{
	اگر (is_verbose())
		eindent();
}

خالی
eoutdentv(خالی)
{
	اگر (is_verbose())
		eoutdent();
}
