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

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/ioctl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <syslog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

/* In case we cannot work out how many columns from ioctl, supply a default */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_ਖਾਨੇ_ਗਿਣਤੀ		 80

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OK			"ok"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NOT_OK			"!!"

/* Number of spaces for an indent */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INDENT_ਚੌੜਾਈ		2

/* How wide can the indent go? */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INDENT_MAX		40

/* Default colours */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GOOD                    2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WARN                    3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BAD                     1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HILITE                  6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BRACKET                 4

/* ANSI escape codes which mimic termcap */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AF "\033[3%dm"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CE "\033[K"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CH "\033[%dC"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MD "\033[1m"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ME "\033[m"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਉੱਪਰ "\033[A"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), ਆਕਾਰ(_d));
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੌਂਪੋ_CAP(_v) ਕਰੋ {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, ਆਕਾਰ(ebuffer) - (p - ebuffer)) + 1;    \
	} ਜਦੋਂ (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *_eprefix = ਨਿਰਾ;

/* Buffers and structures to hold the final colours */
ਸਥਿਰ ਅੱਖਰ ebuffer[100];
ਬਣਤਰ ecolor {
	ECOLOR color;
	ਪੂਰਨ def;
	ਅਚਲ ਅੱਖਰ *name;
};
ਸਥਿਰ ਅੱਖਰ nullstr = '\0';

ਸਥਿਰ ਅਚਲ ਬਣਤਰ ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_ਸਾਧਾਰਨ,  0,       ਨਿਰਾ      },
};
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ecolors_str[ARRAY_ਮਾਪ(ecolors)];

ਸਥਿਰ ਅੱਖਰ *flush = ਨਿਰਾ;
ਸਥਿਰ ਅੱਖਰ *ਉੱਪਰ_2 = ਨਿਰਾ;
ਸਥਿਰ ਅੱਖਰ *goto_ਕਾਲਮ = ਨਿਰਾ;

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਪਦ = ਨਿਰਾ;
ਸਥਿਰ bool ਪਦ_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ color_terms[] = {
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
	ਨਿਰਾ
};

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_STRLCPY
ਸਥਿਰ size_t
strlcat(ਅੱਖਰ *dst, ਅਚਲ ਅੱਖਰ *src, size_t ਮਾਪ)
{
	ਅੱਖਰ *d = dst;
	ਅਚਲ ਅੱਖਰ *s = src;
	size_t src_n = ਮਾਪ;
	size_t dst_n;

	ਜਦੋਂ (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = ਮਾਪ - dst_n;

	ਜੇ (src_n == 0)
		ਵਾਪਸ dst_n + ਲੜੀ_ਲੰਬਾਈ(src);

	ਜਦੋਂ (*s != '\0') {
		ਜੇ (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	ਵਾਪਸ dst_n + (s - src);
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ bool
yesno(ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	ਜੇ (!ਮੁੱਲ) {
		errno = ENOENT;
		ਵਾਪਸ false;
	}

	ਜੇ (strcasecmp(ਮੁੱਲ, "yes") == 0 ||
	    strcasecmp(ਮੁੱਲ, "y") == 0 ||
	    strcasecmp(ਮੁੱਲ, "true") == 0 ||
	    strcasecmp(ਮੁੱਲ, "on") == 0 ||
	    strcasecmp(ਮੁੱਲ, "1") == 0)
		ਵਾਪਸ true;

	ਜੇ (strcasecmp(ਮੁੱਲ, "no") != 0 &&
	    strcasecmp(ਮੁੱਲ, "n") != 0 &&
	    strcasecmp(ਮੁੱਲ, "false") != 0 &&
	    strcasecmp(ਮੁੱਲ, "off") != 0 &&
	    strcasecmp(ਮੁੱਲ, "0") != 0)
		errno = EINVAL;

	ਵਾਪਸ false;
}

ਸਥਿਰ bool
noyes(ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	ਪੂਰਨ serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(ਮੁੱਲ);
	ਜੇ (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	ਵਾਪਸ retval;
}

ਸਥਿਰ bool
is_quiet(ਖਾਲੀ)
{
	ਵਾਪਸ yesno(getenv("EINFO_QUIET"));
}

ਸਥਿਰ bool
is_really_quiet(ਖਾਲੀ)
{
	ਵਾਪਸ yesno(getenv("EERROR_QUIET"));
}

ਸਥਿਰ bool
is_verbose(ਖਾਲੀ)
{
	ਵਾਪਸ yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
ਸਥਿਰ ਅੱਖਰ *
tgoto(ਅਚਲ ਅੱਖਰ *cap, ਪੂਰਨ ਖਾਨਾ, ਪੂਰਨ ਸਤਰ)
{
	ਸਥਿਰ ਅੱਖਰ buf[20];
	ਅੱਖਰ *p, *e, c, dbuf[6];
	ਪੂਰਨ oncol = 0, which = ਸਤਰ, i;

	p = buf;
	e = p + ਆਕਾਰ(buf);
	ਜਦੋਂ ((c = *cap++)) {
		ਜੇ (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			ਜੇ (p >= e) {
				errno = E2BIG;
				ਵਾਪਸ ਨਿਰਾ;
			}
			ਜਾਰੀ;
		}
		ਚੋਣ (c) {
		ਹਾਲਤ '3':
		ਹਾਲਤ '2':
		ਹਾਲਤ 'd':
			i = 0;
			ਕਰੋ
				dbuf[i++] = which % 10 | '0';
			ਜਦੋਂ ((which /= 10));
			ਜੇ (c != 'd') {
				c -= '0';
				ਜੇ (i > c) {
					errno = EINVAL;
					ਵਾਪਸ ਨਿਰਾ;
				}
				ਜਦੋਂ (i < c)
					dbuf[i++] = '0';
			}
			ਜੇ (p + i >= e) {
				errno = E2BIG;
				ਵਾਪਸ ਨਿਰਾ;
			}
			ਕਰੋ
				*p++ = dbuf[--i];
			ਜਦੋਂ (i);
			ਰੋਕੋ;
		ਹਾਲਤ 'r':
			oncol = 0;
			ਰੋਕੋ;
		ਹਾਲਤ 'i':
			ਖਾਨਾ++;
			ਸਤਰ++;
			which++;
			ਜਾਰੀ;
		ਮੂਲ:
			errno = EINVAL;
			ਵਾਪਸ ਨਿਰਾ;
		}

		oncol = 1 - oncol;
		which = oncol ? ਖਾਨਾ : ਸਤਰ;
	}
	*p = '\0';
	ਵਾਪਸ buf;
}

ਸਥਿਰ bool
ਰੰਗ_terminal(FILE * EINFO_RESTRICT f)
{
	ਸਥਿਰ ਪੂਰਨ ਵਿੱਚ_ਰੰਗ = -1;
	ਅੱਖਰ *e, *ee, *ਸਿਰਾ, *d, *p;
	ਪੂਰਨ c;
	ਅਚਲ ਅੱਖਰ *_af = ਨਿਰਾ, *_ce = ਨਿਰਾ, *_ch = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *_md = ਨਿਰਾ, *_me = ਨਿਰਾ, *ਉੱਪਰ_3 = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *bold;
	ਅੱਖਰ tmp[100];
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i = 0;

	ਜੇ (f && !isatty(fileno(f)))
		ਵਾਪਸ false;

	ਜੇ (noyes(getenv("EINFO_COLOR")))
		ਵਾਪਸ false;

	ਜੇ (ਵਿੱਚ_ਰੰਗ == 0)
		ਵਾਪਸ false;
	ਜੇ (ਵਿੱਚ_ਰੰਗ == 1)
		ਵਾਪਸ true;

	ਪਦ_is_cons25 = false;
	ਜੇ (!ਪਦ) {
		ਪਦ = getenv("TERM");
		ਜੇ (!ਪਦ)
			ਵਾਪਸ false;
	}
	ਜੇ (strcmp(ਪਦ, "cons25") == 0)
		ਪਦ_is_cons25 = true;

	ਜੇ (ਲੜੀ_ਖੋਜ(ਪਦ, "color"))
		ਵਿੱਚ_ਰੰਗ = 1;

	ਜਦੋਂ (color_terms[i] && ਵਿੱਚ_ਰੰਗ != 1) {
		ਜੇ (strcmp(color_terms[i], ਪਦ) == 0) {
			ਵਿੱਚ_ਰੰਗ = 1;
		}
		i++;
	}

	ਜੇ (ਵਿੱਚ_ਰੰਗ != 1) {
		ਵਿੱਚ_ਰੰਗ = 0;
		ਵਾਪਸ false;
	}

	ਜੇ (!_af)
		_af = AF;
	ਜੇ (!_ce)
		_ce = CE;
	ਜੇ (!_ch)
		_ch = CH;
	ਜੇ (!_md)
		_md = MD;
	ਜੇ (!_me)
		_me = ME;
	ਜੇ (!ਉੱਪਰ_3)
		ਉੱਪਰ_3 = ਉੱਪਰ;

	/* Now setup our colours */
	p = ebuffer;
	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(ecolors); ++i) {
		tmp[0] = '\0';
		ਜੇ (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			ਜੇ ((e = getenv("EINFO_COLOR"))) {
				ee = ਲੜੀ_ਖੋਜ(e, ecolors[i].name);
				ਜੇ (ee)
					ee += ਲੜੀ_ਲੰਬਾਈ(ecolors[i].name);

				ਜੇ (ee && *ee == '=') {
					d = strdup(ee + 1);
					ਜੇ (d) {
						ਸਿਰਾ = strchr(d, ':');
						ਜੇ (ਸਿਰਾ)
							*ਸਿਰਾ = '\0';
						c = atoi(d);
						ਸਿਰਾ = strchr(d, ';');
						ਜੇ (ਸਿਰਾ && *++ਸਿਰਾ == '0')
							bold = _me;
						ਮੁਕਤ(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), ਆਕਾਰ(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), ਆਕਾਰ(tmp));
		} ਨਹੀਂਤਾਂ
			_GET_CAP(tmp, _me);

		ਜੇ (tmp[0])
			ਸੌਂਪੋ_CAP(ecolors_str[i]);
		ਨਹੀਂਤਾਂ
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	ਸੌਂਪੋ_CAP(flush);
	_GET_CAP(tmp, ਉੱਪਰ_3);
	ਸੌਂਪੋ_CAP(ਉੱਪਰ_2);
	strlcpy(tmp, _ch, ਆਕਾਰ(tmp));
	ਸੌਂਪੋ_CAP(goto_ਕਾਲਮ);

	ਵਿੱਚ_ਰੰਗ = 1;
	ਵਾਪਸ true;
}

ਸਥਿਰ ਪੂਰਨ
get_ਪਦ_columns(FILE * EINFO_RESTRICT ਧਾਰਾ)
{
	ਬਣਤਰ winsize ws;
	ਅੱਖਰ *env = getenv("COLUMNS");
	ਅੱਖਰ *p;
	ਪੂਰਨ i;

	ਜੇ (env) {
		i = strtoimax(env, &p, 10);
		ਜੇ (!*p)
			ਵਾਪਸ i;
	}

	ਜੇ (ioctl(fileno(ਧਾਰਾ), TIOCGWINSZ, &ws) == 0)
		ਵਾਪਸ ws.ws_col;

	ਵਾਪਸ DEFAULT_ਖਾਨੇ_ਗਿਣਤੀ;
}

ਖਾਲੀ
eprefix(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT ਅਗੇਤਰ)
{
	_eprefix = ਅਗੇਤਰ;
}

ਸਥਿਰ ਖਾਲੀ EINFO_PRINTF(2, 0)
elogv(ਪੂਰਨ level, ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, va_list ap)
{
	ਅੱਖਰ *e = getenv("EINFO_LOG");
	va_list apc;

	ਜੇ (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

ਖਾਲੀ
elog(ਪੂਰਨ level, ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

ਸਥਿਰ ਪੂਰਨ
_eindent(FILE * EINFO_RESTRICT ਧਾਰਾ)
{
	ਅੱਖਰ *env = getenv("EINFO_INDENT");
	ਪੂਰਨ amount = 0;

	ਜੇ (env) {
		errno = 0;
		amount = strtoimax(env, ਨਿਰਾ, 0);
		ਜੇ (errno != 0 || amount < 0)
			amount = 0;
		ਨਹੀਂਤਾਂ ਜੇ (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	ਵਾਪਸ fprintf(ਧਾਰਾ, "%*s", amount, "");
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	ਜੇ (!ਰੰਗ_terminal(f))
		ਵਾਪਸ "";

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(ecolors); ++i)
		ਜੇ (ecolors[i].color == color)
			ਵਾਪਸ ecolors_str[i];
	ਵਾਪਸ "";
}

ਅਚਲ ਅੱਖਰ *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	ਜੇ (!isatty(fileno(f))) {
		f = stderr;
		ਜੇ (!isatty(fileno(f))) {
			f = stdin;
			ਜੇ (!isatty(fileno(f)))
				f = ਨਿਰਾ;
		}
	}

	ਵਾਪਸ _ecolor(f, color);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

ਸਥਿਰ ਪੂਰਨ EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, va_list va)
{
	ਪੂਰਨ retval = 0;
	ਅੱਖਰ *ਆਖਰੀ = getenv("EINFO_LASTCMD");
	va_list ap;

	ਜੇ (ਆਖਰੀ &&
	    *ਆਖਰੀ &&
	    !ਰੰਗ_terminal(f) &&
	    strcmp(ਆਖਰੀ, "ewarn") != 0 &&
	    ਆਖਰੀ[ਲੜੀ_ਲੰਬਾਈ(ਆਖਰੀ) - 1] == 'n')
		fprintf(f, "\n");
	ਜੇ (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_ਸਾਧਾਰਨ));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_ਸਾਧਾਰਨ));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	ਜੇ (ਰੰਗ_terminal(f))
		fprintf(f, "%s", flush);
	ਵਾਪਸ retval;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

ਪੂਰਨ
einfon(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	ਪੂਰਨ retval;
	va_list ap;

	ਜੇ (!fmt || is_quiet())
		ਵਾਪਸ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	ਵਾਪਸ retval;
}

ਪੂਰਨ
ewarnn(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	ਪੂਰਨ retval;
	va_list ap;

	ਜੇ (!fmt || is_quiet())
		ਵਾਪਸ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	ਵਾਪਸ retval;
}

ਪੂਰਨ
eerrorn(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	ਪੂਰਨ retval;
	va_list ap;

	ਜੇ (!fmt || is_really_quiet())
		ਵਾਪਸ 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	ਵਾਪਸ retval;
}

ਪੂਰਨ
einfo(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	ਪੂਰਨ retval;
	va_list ap;

	ਜੇ (!fmt || is_quiet())
		ਵਾਪਸ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += ਲਿਖੋ("\n");
	va_end(ap);
	LASTCMD("einfo");
	ਵਾਪਸ retval;
}

ਪੂਰਨ
ewarn(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	ਪੂਰਨ retval;
	va_list ap;

	ਜੇ (!fmt || is_quiet())
		ਵਾਪਸ 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	ਵਾਪਸ retval;
}

ਖਾਲੀ
ewarnx(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ਜੇ (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
}

ਪੂਰਨ
eerror(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	ਪੂਰਨ retval;
	va_list ap;

	ਜੇ (!fmt || is_really_quiet())
		ਵਾਪਸ 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	ਵਾਪਸ retval;
}

ਖਾਲੀ
eerrorx(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ਜੇ (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
}

ਪੂਰਨ
ebegin(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	ਪੂਰਨ retval;
	va_list ap;

	ਜੇ (!fmt || is_quiet())
		ਵਾਪਸ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += ਲਿਖੋ(" ...");
	ਜੇ (ਰੰਗ_terminal(stdout))
		retval += ਲਿਖੋ("\n");
	LASTCMD("ebegin");
	ਵਾਪਸ retval;
}

ਸਥਿਰ ਖਾਲੀ
_eend(FILE * EINFO_RESTRICT fp, ਪੂਰਨ ਖਾਨਾ, ECOLOR color, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_2)
{
	ਪੂਰਨ i;
	ਪੂਰਨ ਖਾਨੇ_ਗਿਣਤੀ;

	ਜੇ (!ਸੁਨੇਹਾ_2)
		ਵਾਪਸ;

	ਖਾਨੇ_ਗਿਣਤੀ = get_ਪਦ_columns(fp) - (ਲੜੀ_ਲੰਬਾਈ(ਸੁਨੇਹਾ_2) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	ਜੇ (!ਪਦ) {
		ਪਦ = getenv("TERM");
		ਜੇ (ਪਦ && strcmp(ਪਦ, "cons25") == 0)
			ਪਦ_is_cons25 = true;
		ਨਹੀਂਤਾਂ
			ਪਦ_is_cons25 = false;
	}
	ਜੇ (ਪਦ_is_cons25)
		ਖਾਨੇ_ਗਿਣਤੀ--;

	ਜੇ (ਖਾਨੇ_ਗਿਣਤੀ > 0 && ਰੰਗ_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", ਉੱਪਰ_2, tgoto(goto_ਕਾਲਮ, 0, ਖਾਨੇ_ਗਿਣਤੀ),
		    ecolor(ECOLOR_BRACKET), ecolor(color), ਸੁਨੇਹਾ_2,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_ਸਾਧਾਰਨ));
	} ਨਹੀਂਤਾਂ {
		ਜੇ (ਖਾਨਾ > 0)
			ਲਈ (i = 0; i < ਖਾਨੇ_ਗਿਣਤੀ - ਖਾਨਾ; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", ਸੁਨੇਹਾ_2);
	}
}

ਸਥਿਰ ਪੂਰਨ EINFO_PRINTF(3, 0)
_do_eend(ਅਚਲ ਅੱਖਰ *cmd, ਪੂਰਨ retval,
    ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, va_list ap)
{
	ਪੂਰਨ ਖਾਨਾ = 0;
	FILE *fp = stdout;
	va_list apc;

	ਜੇ (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		ਜੇ (strcmp(cmd, "ewend") == 0)
			ਖਾਨਾ = _ewarnvn(fmt, apc);
		ਨਹੀਂਤਾਂ
			ਖਾਨਾ = _eerrorvn(fmt, apc);
		ਖਾਨਾ += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, ਖਾਨਾ,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	ਵਾਪਸ retval;
}

ਪੂਰਨ
eend(ਪੂਰਨ retval, ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ਜੇ (is_quiet())
		ਵਾਪਸ retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	ਵਾਪਸ retval;
}

ਪੂਰਨ
ewend(ਪੂਰਨ retval, ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ਜੇ (is_quiet())
		ਵਾਪਸ retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	ਵਾਪਸ retval;
}

ਖਾਲੀ
ebracket(ਪੂਰਨ ਖਾਨਾ, ECOLOR color, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_2)
{
	_eend(stdout, ਖਾਨਾ, color, ਸੁਨੇਹਾ_2);
}

ਖਾਲੀ
eindent(ਖਾਲੀ)
{
	ਅੱਖਰ *env = getenv("EINFO_INDENT");
	ਪੂਰਨ amount = 0;
	ਅੱਖਰ *ਅੰਕ;

	ਜੇ (env) {
		errno = 0;
		amount = strtoimax(env, ਨਿਰਾ, 0);
		ਜੇ (errno != 0)
			amount = 0;
	}
	amount += INDENT_ਚੌੜਾਈ;
	xasprintf(&ਅੰਕ, "%08d", amount);
	setenv("EINFO_INDENT", ਅੰਕ, 1);
	ਮੁਕਤ(ਅੰਕ);
}

ਖਾਲੀ eoutdent(ਖਾਲੀ)
{
	ਅੱਖਰ *env = getenv("EINFO_INDENT");
	ਪੂਰਨ amount = 0;
	ਅੱਖਰ *ਅੰਕ = ਨਿਰਾ;
	ਪੂਰਨ serrno = errno;

	ਜੇ (!env)
		ਵਾਪਸ;
	errno = 0;
	amount = strtoimax(env, ਨਿਰਾ, 0);
	ਜੇ (errno != 0)
		amount = 0;
	ਨਹੀਂਤਾਂ
		amount -= INDENT_ਚੌੜਾਈ;
	ਜੇ (amount <= 0)
		unsetenv("EINFO_INDENT");
	ਨਹੀਂਤਾਂ {
		xasprintf(&ਅੰਕ, "%08d", amount);
		setenv("EINFO_INDENT", ਅੰਕ, 1);
		ਮੁਕਤ(ਅੰਕ);
	}
	errno = serrno;
}

ਪੂਰਨ
einfovn(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	ਪੂਰਨ retval;
	va_list ap;

	ਜੇ (!fmt || !is_verbose())
		ਵਾਪਸ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	ਵਾਪਸ retval;
}

ਪੂਰਨ
ewarnvn(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	ਪੂਰਨ retval;
	va_list ap;

	ਜੇ (!fmt || !is_verbose())
		ਵਾਪਸ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	ਵਾਪਸ retval;
}

ਪੂਰਨ
einfov(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	ਪੂਰਨ retval;
	va_list ap;

	ਜੇ (!fmt || !is_verbose())
		ਵਾਪਸ 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += ਲਿਖੋ("\n");
	va_end(ap);
	LASTCMD("einfov");
	ਵਾਪਸ retval;
}

ਪੂਰਨ
ewarnv(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	ਪੂਰਨ retval;
	va_list ap;

	ਜੇ (!fmt || !is_verbose())
		ਵਾਪਸ 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += ਲਿਖੋ("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	ਵਾਪਸ retval;
}

ਪੂਰਨ
ebeginv(ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	ਪੂਰਨ retval;
	va_list ap;

	ਜੇ (!fmt || !is_verbose())
		ਵਾਪਸ 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += ਲਿਖੋ(" ...");
	ਜੇ (ਰੰਗ_terminal(stdout))
		retval += ਲਿਖੋ("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	ਵਾਪਸ retval;
}

ਪੂਰਨ
eendv(ਪੂਰਨ retval, ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ਜੇ (!is_verbose())
		ਵਾਪਸ 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	ਵਾਪਸ retval;
}

ਪੂਰਨ
ewendv(ਪੂਰਨ retval, ਅਚਲ ਅੱਖਰ *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ਜੇ (!is_verbose())
		ਵਾਪਸ 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	ਵਾਪਸ retval;
}

ਖਾਲੀ
eindentv(ਖਾਲੀ)
{
	ਜੇ (is_verbose())
		eindent();
}

ਖਾਲੀ
eoutdentv(ਖਾਲੀ)
{
	ਜੇ (is_verbose())
		eoutdent();
}
