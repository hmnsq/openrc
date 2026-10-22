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

#अन्तर्भाव <sys/ioctl.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <syslog.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "helpers.h"

/* In case we cannot work out how many columns from ioctl, supply a default */
#परिभाषय DEFAULT_स्तम्भसङ्ख्या		 80

#परिभाषय OK			"ok"
#परिभाषय NOT_OK			"!!"

/* Number of spaces for an indent */
#परिभाषय INDENT_विस्तारः		2

/* How wide can the indent go? */
#परिभाषय INDENT_MAX		40

/* Default colours */
#परिभाषय GOOD                    2
#परिभाषय WARN                    3
#परिभाषय BAD                     1
#परिभाषय HILITE                  6
#परिभाषय BRACKET                 4

/* ANSI escape codes which mimic termcap */
#परिभाषय AF "\033[3%dm"
#परिभाषय CE "\033[K"
#परिभाषय CH "\033[%dC"
#परिभाषय MD "\033[1m"
#परिभाषय ME "\033[m"
#परिभाषय ऊर्ध्वम् "\033[A"

#परिभाषय _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), परिमाण(_d));
#परिभाषय नियोजय_CAP(_v) कुरु {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, परिमाण(ebuffer) - (p - ebuffer)) + 1;    \
	} यावत् (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
स्थिर ध्रुव वर्ण *_eprefix = रिक्त;

/* Buffers and structures to hold the final colours */
स्थिर वर्ण ebuffer[100];
संरचना ecolor {
	ECOLOR color;
	पूर्णाङ्क def;
	ध्रुव वर्ण *name;
};
स्थिर वर्ण nullstr = '\0';

स्थिर ध्रुव संरचना ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_प्रसामान्यम्,  0,       रिक्त      },
};
स्थिर ध्रुव वर्ण *ecolors_str[ARRAY_आकार(ecolors)];

स्थिर वर्ण *flush = रिक्त;
स्थिर वर्ण *ऊर्ध्वम्_2 = रिक्त;
स्थिर वर्ण *goto_column = रिक्त;

स्थिर ध्रुव वर्ण *पदांशः = रिक्त;
स्थिर bool पदांशः_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
स्थिर ध्रुव वर्ण *ध्रुव color_terms[] = {
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
	रिक्त
};

#यदि_अपरिभाषितम् HAVE_STRLCPY
स्थिर size_t
strlcat(वर्ण *dst, ध्रुव वर्ण *src, size_t आकार)
{
	वर्ण *d = dst;
	ध्रुव वर्ण *s = src;
	size_t src_n = आकार;
	size_t dst_n;

	यावत् (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = आकार - dst_n;

	यदि (src_n == 0)
		प्रत्यागम dst_n + वाक्यदैर्घ्यम्(src);

	यावत् (*s != '\0') {
		यदि (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	प्रत्यागम dst_n + (s - src);
}
#यदि_अन्तः

स्थिर bool
yesno(ध्रुव वर्ण *मानम्)
{
	यदि (!मानम्) {
		errno = ENOENT;
		प्रत्यागम false;
	}

	यदि (strcasecmp(मानम्, "yes") == 0 ||
	    strcasecmp(मानम्, "y") == 0 ||
	    strcasecmp(मानम्, "true") == 0 ||
	    strcasecmp(मानम्, "on") == 0 ||
	    strcasecmp(मानम्, "1") == 0)
		प्रत्यागम true;

	यदि (strcasecmp(मानम्, "no") != 0 &&
	    strcasecmp(मानम्, "n") != 0 &&
	    strcasecmp(मानम्, "false") != 0 &&
	    strcasecmp(मानम्, "off") != 0 &&
	    strcasecmp(मानम्, "0") != 0)
		errno = EINVAL;

	प्रत्यागम false;
}

स्थिर bool
noyes(ध्रुव वर्ण *मानम्)
{
	पूर्णाङ्क serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(मानम्);
	यदि (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	प्रत्यागम retval;
}

स्थिर bool
is_quiet(शून्य)
{
	प्रत्यागम yesno(getenv("EINFO_QUIET"));
}

स्थिर bool
is_really_quiet(शून्य)
{
	प्रत्यागम yesno(getenv("EERROR_QUIET"));
}

स्थिर bool
is_verbose(शून्य)
{
	प्रत्यागम yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
स्थिर वर्ण *
tgoto(ध्रुव वर्ण *cap, पूर्णाङ्क col, पूर्णाङ्क पङ्क्तिः)
{
	स्थिर वर्ण buf[20];
	वर्ण *p, *e, c, dbuf[6];
	पूर्णाङ्क oncol = 0, which = पङ्क्तिः, i;

	p = buf;
	e = p + परिमाण(buf);
	यावत् ((c = *cap++)) {
		यदि (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			यदि (p >= e) {
				errno = E2BIG;
				प्रत्यागम रिक्त;
			}
			अनुवर्त;
		}
		विकल्प (c) {
		पक्ष '3':
		पक्ष '2':
		पक्ष 'd':
			i = 0;
			कुरु
				dbuf[i++] = which % 10 | '0';
			यावत् ((which /= 10));
			यदि (c != 'd') {
				c -= '0';
				यदि (i > c) {
					errno = EINVAL;
					प्रत्यागम रिक्त;
				}
				यावत् (i < c)
					dbuf[i++] = '0';
			}
			यदि (p + i >= e) {
				errno = E2BIG;
				प्रत्यागम रिक्त;
			}
			कुरु
				*p++ = dbuf[--i];
			यावत् (i);
			भङ्ग;
		पक्ष 'r':
			oncol = 0;
			भङ्ग;
		पक्ष 'i':
			col++;
			पङ्क्तिः++;
			which++;
			अनुवर्त;
		स्वभाव:
			errno = EINVAL;
			प्रत्यागम रिक्त;
		}

		oncol = 1 - oncol;
		which = oncol ? col : पङ्क्तिः;
	}
	*p = '\0';
	प्रत्यागम buf;
}

स्थिर bool
रङ्गः_terminal(FILE * EINFO_RESTRICT f)
{
	स्थिर पूर्णाङ्क अन्तर्_रङ्गः = -1;
	वर्ण *e, *ee, *अन्तः, *d, *p;
	पूर्णाङ्क c;
	ध्रुव वर्ण *_af = रिक्त, *_ce = रिक्त, *_ch = रिक्त;
	ध्रुव वर्ण *_md = रिक्त, *_me = रिक्त, *ऊर्ध्वम्_3 = रिक्त;
	ध्रुव वर्ण *bold;
	वर्ण tmp[100];
	अचिह्नित पूर्णाङ्क i = 0;

	यदि (f && !isatty(fileno(f)))
		प्रत्यागम false;

	यदि (noyes(getenv("EINFO_COLOR")))
		प्रत्यागम false;

	यदि (अन्तर्_रङ्गः == 0)
		प्रत्यागम false;
	यदि (अन्तर्_रङ्गः == 1)
		प्रत्यागम true;

	पदांशः_is_cons25 = false;
	यदि (!पदांशः) {
		पदांशः = getenv("TERM");
		यदि (!पदांशः)
			प्रत्यागम false;
	}
	यदि (वाक्यं_तुलय(पदांशः, "cons25") == 0)
		पदांशः_is_cons25 = true;

	यदि (वाक्ये_अन्विष(पदांशः, "color"))
		अन्तर्_रङ्गः = 1;

	यावत् (color_terms[i] && अन्तर्_रङ्गः != 1) {
		यदि (वाक्यं_तुलय(color_terms[i], पदांशः) == 0) {
			अन्तर्_रङ्गः = 1;
		}
		i++;
	}

	यदि (अन्तर्_रङ्गः != 1) {
		अन्तर्_रङ्गः = 0;
		प्रत्यागम false;
	}

	यदि (!_af)
		_af = AF;
	यदि (!_ce)
		_ce = CE;
	यदि (!_ch)
		_ch = CH;
	यदि (!_md)
		_md = MD;
	यदि (!_me)
		_me = ME;
	यदि (!ऊर्ध्वम्_3)
		ऊर्ध्वम्_3 = ऊर्ध्वम्;

	/* Now setup our colours */
	p = ebuffer;
	प्रति (i = 0; i < ARRAY_आकार(ecolors); ++i) {
		tmp[0] = '\0';
		यदि (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			यदि ((e = getenv("EINFO_COLOR"))) {
				ee = वाक्ये_अन्विष(e, ecolors[i].name);
				यदि (ee)
					ee += वाक्यदैर्घ्यम्(ecolors[i].name);

				यदि (ee && *ee == '=') {
					d = strdup(ee + 1);
					यदि (d) {
						अन्तः = strchr(d, ':');
						यदि (अन्तः)
							*अन्तः = '\0';
						c = atoi(d);
						अन्तः = strchr(d, ';');
						यदि (अन्तः && *++अन्तः == '0')
							bold = _me;
						मुक्त(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), परिमाण(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), परिमाण(tmp));
		} अन्यथा
			_GET_CAP(tmp, _me);

		यदि (tmp[0])
			नियोजय_CAP(ecolors_str[i]);
		अन्यथा
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	नियोजय_CAP(flush);
	_GET_CAP(tmp, ऊर्ध्वम्_3);
	नियोजय_CAP(ऊर्ध्वम्_2);
	strlcpy(tmp, _ch, परिमाण(tmp));
	नियोजय_CAP(goto_column);

	अन्तर्_रङ्गः = 1;
	प्रत्यागम true;
}

स्थिर पूर्णाङ्क
get_पदांशः_columns(FILE * EINFO_RESTRICT प्रवाहः)
{
	संरचना winsize ws;
	वर्ण *env = getenv("COLUMNS");
	वर्ण *p;
	पूर्णाङ्क i;

	यदि (env) {
		i = strtoimax(env, &p, 10);
		यदि (!*p)
			प्रत्यागम i;
	}

	यदि (ioctl(fileno(प्रवाहः), TIOCGWINSZ, &ws) == 0)
		प्रत्यागम ws.ws_col;

	प्रत्यागम DEFAULT_स्तम्भसङ्ख्या;
}

शून्य
eprefix(ध्रुव वर्ण *EINFO_RESTRICT उपसर्गः)
{
	_eprefix = उपसर्गः;
}

स्थिर शून्य EINFO_PRINTF(2, 0)
elogv(पूर्णाङ्क level, ध्रुव वर्ण *EINFO_RESTRICT fmt, va_list ap)
{
	वर्ण *e = getenv("EINFO_LOG");
	va_list apc;

	यदि (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

शून्य
elog(पूर्णाङ्क level, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

स्थिर पूर्णाङ्क
_eindent(FILE * EINFO_RESTRICT प्रवाहः)
{
	वर्ण *env = getenv("EINFO_INDENT");
	पूर्णाङ्क amount = 0;

	यदि (env) {
		errno = 0;
		amount = strtoimax(env, रिक्त, 0);
		यदि (errno != 0 || amount < 0)
			amount = 0;
		अन्यथा यदि (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	प्रत्यागम fprintf(प्रवाहः, "%*s", amount, "");
}

स्थिर ध्रुव वर्ण *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	अचिह्नित पूर्णाङ्क i;

	यदि (!रङ्गः_terminal(f))
		प्रत्यागम "";

	प्रति (i = 0; i < ARRAY_आकार(ecolors); ++i)
		यदि (ecolors[i].color == color)
			प्रत्यागम ecolors_str[i];
	प्रत्यागम "";
}

ध्रुव वर्ण *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	यदि (!isatty(fileno(f))) {
		f = stderr;
		यदि (!isatty(fileno(f))) {
			f = stdin;
			यदि (!isatty(fileno(f)))
				f = रिक्त;
		}
	}

	प्रत्यागम _ecolor(f, color);
}

#परिभाषय LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

स्थिर पूर्णाङ्क EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, ध्रुव वर्ण *EINFO_RESTRICT fmt, va_list va)
{
	पूर्णाङ्क retval = 0;
	वर्ण *चरमः = getenv("EINFO_LASTCMD");
	va_list ap;

	यदि (चरमः &&
	    *चरमः &&
	    !रङ्गः_terminal(f) &&
	    वाक्यं_तुलय(चरमः, "ewarn") != 0 &&
	    चरमः[वाक्यदैर्घ्यम्(चरमः) - 1] == 'n')
		fprintf(f, "\n");
	यदि (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_प्रसामान्यम्));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_प्रसामान्यम्));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	यदि (रङ्गः_terminal(f))
		fprintf(f, "%s", flush);
	प्रत्यागम retval;
}

#परिभाषय _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#परिभाषय _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#परिभाषय _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

पूर्णाङ्क
einfon(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	यदि (!fmt || is_quiet())
		प्रत्यागम 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	प्रत्यागम retval;
}

पूर्णाङ्क
ewarnn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	यदि (!fmt || is_quiet())
		प्रत्यागम 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	प्रत्यागम retval;
}

पूर्णाङ्क
eerrorn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	यदि (!fmt || is_really_quiet())
		प्रत्यागम 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	प्रत्यागम retval;
}

पूर्णाङ्क
einfo(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	यदि (!fmt || is_quiet())
		प्रत्यागम 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += प्रारूप_लिख("\n");
	va_end(ap);
	LASTCMD("einfo");
	प्रत्यागम retval;
}

पूर्णाङ्क
ewarn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	यदि (!fmt || is_quiet())
		प्रत्यागम 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	प्रत्यागम retval;
}

शून्य
ewarnx(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	यदि (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	निर्गच्छ(EXIT_FAILURE);
}

पूर्णाङ्क
eerror(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	यदि (!fmt || is_really_quiet())
		प्रत्यागम 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	प्रत्यागम retval;
}

शून्य
eerrorx(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	यदि (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	निर्गच्छ(EXIT_FAILURE);
}

पूर्णाङ्क
ebegin(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	यदि (!fmt || is_quiet())
		प्रत्यागम 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += प्रारूप_लिख(" ...");
	यदि (रङ्गः_terminal(stdout))
		retval += प्रारूप_लिख("\n");
	LASTCMD("ebegin");
	प्रत्यागम retval;
}

स्थिर शून्य
_eend(FILE * EINFO_RESTRICT fp, पूर्णाङ्क col, ECOLOR color, ध्रुव वर्ण *सन्देशः_2)
{
	पूर्णाङ्क i;
	पूर्णाङ्क स्तम्भसङ्ख्या;

	यदि (!सन्देशः_2)
		प्रत्यागम;

	स्तम्भसङ्ख्या = get_पदांशः_columns(fp) - (वाक्यदैर्घ्यम्(सन्देशः_2) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	यदि (!पदांशः) {
		पदांशः = getenv("TERM");
		यदि (पदांशः && वाक्यं_तुलय(पदांशः, "cons25") == 0)
			पदांशः_is_cons25 = true;
		अन्यथा
			पदांशः_is_cons25 = false;
	}
	यदि (पदांशः_is_cons25)
		स्तम्भसङ्ख्या--;

	यदि (स्तम्भसङ्ख्या > 0 && रङ्गः_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", ऊर्ध्वम्_2, tgoto(goto_column, 0, स्तम्भसङ्ख्या),
		    ecolor(ECOLOR_BRACKET), ecolor(color), सन्देशः_2,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_प्रसामान्यम्));
	} अन्यथा {
		यदि (col > 0)
			प्रति (i = 0; i < स्तम्भसङ्ख्या - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", सन्देशः_2);
	}
}

स्थिर पूर्णाङ्क EINFO_PRINTF(3, 0)
_do_eend(ध्रुव वर्ण *cmd, पूर्णाङ्क retval,
    ध्रुव वर्ण *EINFO_RESTRICT fmt, va_list ap)
{
	पूर्णाङ्क col = 0;
	FILE *fp = stdout;
	va_list apc;

	यदि (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		यदि (वाक्यं_तुलय(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		अन्यथा
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	प्रत्यागम retval;
}

पूर्णाङ्क
eend(पूर्णाङ्क retval, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	यदि (is_quiet())
		प्रत्यागम retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	प्रत्यागम retval;
}

पूर्णाङ्क
ewend(पूर्णाङ्क retval, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	यदि (is_quiet())
		प्रत्यागम retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	प्रत्यागम retval;
}

शून्य
ebracket(पूर्णाङ्क col, ECOLOR color, ध्रुव वर्ण *सन्देशः_2)
{
	_eend(stdout, col, color, सन्देशः_2);
}

शून्य
eindent(शून्य)
{
	वर्ण *env = getenv("EINFO_INDENT");
	पूर्णाङ्क amount = 0;
	वर्ण *अङ्कः;

	यदि (env) {
		errno = 0;
		amount = strtoimax(env, रिक्त, 0);
		यदि (errno != 0)
			amount = 0;
	}
	amount += INDENT_विस्तारः;
	xasprintf(&अङ्कः, "%08d", amount);
	setenv("EINFO_INDENT", अङ्कः, 1);
	मुक्त(अङ्कः);
}

शून्य eoutdent(शून्य)
{
	वर्ण *env = getenv("EINFO_INDENT");
	पूर्णाङ्क amount = 0;
	वर्ण *अङ्कः = रिक्त;
	पूर्णाङ्क serrno = errno;

	यदि (!env)
		प्रत्यागम;
	errno = 0;
	amount = strtoimax(env, रिक्त, 0);
	यदि (errno != 0)
		amount = 0;
	अन्यथा
		amount -= INDENT_विस्तारः;
	यदि (amount <= 0)
		unsetenv("EINFO_INDENT");
	अन्यथा {
		xasprintf(&अङ्कः, "%08d", amount);
		setenv("EINFO_INDENT", अङ्कः, 1);
		मुक्त(अङ्कः);
	}
	errno = serrno;
}

पूर्णाङ्क
einfovn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	यदि (!fmt || !is_verbose())
		प्रत्यागम 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	प्रत्यागम retval;
}

पूर्णाङ्क
ewarnvn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	यदि (!fmt || !is_verbose())
		प्रत्यागम 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	प्रत्यागम retval;
}

पूर्णाङ्क
einfov(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	यदि (!fmt || !is_verbose())
		प्रत्यागम 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += प्रारूप_लिख("\n");
	va_end(ap);
	LASTCMD("einfov");
	प्रत्यागम retval;
}

पूर्णाङ्क
ewarnv(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	यदि (!fmt || !is_verbose())
		प्रत्यागम 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += प्रारूप_लिख("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	प्रत्यागम retval;
}

पूर्णाङ्क
ebeginv(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	यदि (!fmt || !is_verbose())
		प्रत्यागम 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += प्रारूप_लिख(" ...");
	यदि (रङ्गः_terminal(stdout))
		retval += प्रारूप_लिख("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	प्रत्यागम retval;
}

पूर्णाङ्क
eendv(पूर्णाङ्क retval, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	यदि (!is_verbose())
		प्रत्यागम 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	प्रत्यागम retval;
}

पूर्णाङ्क
ewendv(पूर्णाङ्क retval, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	यदि (!is_verbose())
		प्रत्यागम 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	प्रत्यागम retval;
}

शून्य
eindentv(शून्य)
{
	यदि (is_verbose())
		eindent();
}

शून्य
eoutdentv(शून्य)
{
	यदि (is_verbose())
		eoutdent();
}
