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

#define _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), परिमाण(_d));
#define _ASSIGN_CAP(_v) कुरु {						      \
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
	{ ECOLOR_NORMAL,  0,       रिक्त      },
};
स्थिर ध्रुव वर्ण *ecolors_str[ARRAY_SIZE(ecolors)];

स्थिर वर्ण *flush = रिक्त;
स्थिर वर्ण *up = रिक्त;
स्थिर वर्ण *goto_column = रिक्त;

स्थिर ध्रुव वर्ण *term = रिक्त;
स्थिर bool term_is_cons25 = false;

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

#ifndef HAVE_STRLCPY
स्थिर माप_प्रकार
strlcat(वर्ण *dst, ध्रुव वर्ण *src, माप_प्रकार size)
{
	वर्ण *d = dst;
	ध्रुव वर्ण *s = src;
	माप_प्रकार src_n = size;
	माप_प्रकार dst_n;

	यावत् (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	यदि (src_n == 0)
		प्रत्यागम dst_n + म_माप(src);

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
#endif

स्थिर bool
yesno(ध्रुव वर्ण *मूल्य)
{
	यदि (!मूल्य) {
		त्रुटि_सं = ENOENT;
		प्रत्यागम false;
	}

	यदि (strcasecmp(मूल्य, "yes") == 0 ||
	    strcasecmp(मूल्य, "y") == 0 ||
	    strcasecmp(मूल्य, "true") == 0 ||
	    strcasecmp(मूल्य, "on") == 0 ||
	    strcasecmp(मूल्य, "1") == 0)
		प्रत्यागम true;

	यदि (strcasecmp(मूल्य, "no") != 0 &&
	    strcasecmp(मूल्य, "n") != 0 &&
	    strcasecmp(मूल्य, "false") != 0 &&
	    strcasecmp(मूल्य, "off") != 0 &&
	    strcasecmp(मूल्य, "0") != 0)
		त्रुटि_सं = EINVAL;

	प्रत्यागम false;
}

स्थिर bool
noyes(ध्रुव वर्ण *मूल्य)
{
	पूर्णाङ्क serrno = त्रुटि_सं;
	bool retval;

	त्रुटि_सं = 0;
	retval = yesno(मूल्य);
	यदि (त्रुटि_सं == 0) {
		retval = !retval;
		त्रुटि_सं = serrno;
	}

	प्रत्यागम retval;
}

स्थिर bool
is_quiet(शून्य)
{
	प्रत्यागम yesno(दो_पर्या("EINFO_QUIET"));
}

स्थिर bool
is_really_quiet(शून्य)
{
	प्रत्यागम yesno(दो_पर्या("EERROR_QUIET"));
}

स्थिर bool
is_verbose(शून्य)
{
	प्रत्यागम yesno(दो_पर्या ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
स्थिर वर्ण *
tgoto(ध्रुव वर्ण *cap, पूर्णाङ्क col, पूर्णाङ्क पंक्ति)
{
	स्थिर वर्ण buf[20];
	वर्ण *p, *e, c, dbuf[6];
	पूर्णाङ्क oncol = 0, which = पंक्ति, i;

	p = buf;
	e = p + परिमाण(buf);
	यावत् ((c = *cap++)) {
		यदि (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			यदि (p >= e) {
				त्रुटि_सं = E2BIG;
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
					त्रुटि_सं = EINVAL;
					प्रत्यागम रिक्त;
				}
				यावत् (i < c)
					dbuf[i++] = '0';
			}
			यदि (p + i >= e) {
				त्रुटि_सं = E2BIG;
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
			पंक्ति++;
			which++;
			अनुवर्त;
		स्वभाव:
			त्रुटि_सं = EINVAL;
			प्रत्यागम रिक्त;
		}

		oncol = 1 - oncol;
		which = oncol ? col : पंक्ति;
	}
	*p = '\0';
	प्रत्यागम buf;
}

स्थिर bool
colour_terminal(खाता * EINFO_RESTRICT f)
{
	स्थिर पूर्णाङ्क in_colour = -1;
	वर्ण *e, *ee, *end, *d, *p;
	पूर्णाङ्क c;
	ध्रुव वर्ण *_af = रिक्त, *_ce = रिक्त, *_ch = रिक्त;
	ध्रुव वर्ण *_md = रिक्त, *_me = रिक्त, *_up = रिक्त;
	ध्रुव वर्ण *bold;
	वर्ण tmp[100];
	अचिन्हित पूर्णाङ्क i = 0;

	यदि (f && !isatty(fileno(f)))
		प्रत्यागम false;

	यदि (noyes(दो_पर्या("EINFO_COLOR")))
		प्रत्यागम false;

	यदि (in_colour == 0)
		प्रत्यागम false;
	यदि (in_colour == 1)
		प्रत्यागम true;

	term_is_cons25 = false;
	यदि (!term) {
		term = दो_पर्या("TERM");
		यदि (!term)
			प्रत्यागम false;
	}
	यदि (म_भेद(term, "cons25") == 0)
		term_is_cons25 = true;

	यदि (म_माला(term, "color"))
		in_colour = 1;

	यावत् (color_terms[i] && in_colour != 1) {
		यदि (म_भेद(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	यदि (in_colour != 1) {
		in_colour = 0;
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
	यदि (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	प्रति (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		यदि (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			यदि ((e = दो_पर्या("EINFO_COLOR"))) {
				ee = म_माला(e, ecolors[i].name);
				यदि (ee)
					ee += म_माप(ecolors[i].name);

				यदि (ee && *ee == '=') {
					d = strdup(ee + 1);
					यदि (d) {
						end = म_अक्षर(d, ':');
						यदि (end)
							*end = '\0';
						c = म_से_प(d);
						end = म_अक्षर(d, ';');
						यदि (end && *++end == '0')
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
			_ASSIGN_CAP(ecolors_str[i]);
		अन्यथा
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, परिमाण(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	प्रत्यागम true;
}

स्थिर पूर्णाङ्क
get_term_columns(खाता * EINFO_RESTRICT stream)
{
	संरचना winsize ws;
	वर्ण *env = दो_पर्या("COLUMNS");
	वर्ण *p;
	पूर्णाङ्क i;

	यदि (env) {
		i = strtoimax(env, &p, 10);
		यदि (!*p)
			प्रत्यागम i;
	}

	यदि (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		प्रत्यागम ws.ws_col;

	प्रत्यागम DEFAULT_COLS;
}

शून्य
eprefix(ध्रुव वर्ण *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

स्थिर शून्य EINFO_PRINTF(2, 0)
elogv(पूर्णाङ्क level, ध्रुव वर्ण *EINFO_RESTRICT fmt, बहु_सूची ap)
{
	वर्ण *e = दो_पर्या("EINFO_LOG");
	बहु_सूची apc;

	यदि (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		बहु_पूर्ण(apc);
		closelog();
	}
}

शून्य
elog(पूर्णाङ्क level, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	बहु_शुरू(ap, fmt);
	elogv(level, fmt, ap);
	बहु_पूर्ण(ap);
}

स्थिर पूर्णाङ्क
_eindent(खाता * EINFO_RESTRICT stream)
{
	वर्ण *env = दो_पर्या("EINFO_INDENT");
	पूर्णाङ्क amount = 0;

	यदि (env) {
		त्रुटि_सं = 0;
		amount = strtoimax(env, रिक्त, 0);
		यदि (त्रुटि_सं != 0 || amount < 0)
			amount = 0;
		अन्यथा यदि (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	प्रत्यागम ख_लिखो(stream, "%*s", amount, "");
}

स्थिर ध्रुव वर्ण *
_ecolor(खाता * EINFO_RESTRICT f, ECOLOR color)
{
	अचिन्हित पूर्णाङ्क i;

	यदि (!colour_terminal(f))
		प्रत्यागम "";

	प्रति (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		यदि (ecolors[i].color == color)
			प्रत्यागम ecolors_str[i];
	प्रत्यागम "";
}

ध्रुव वर्ण *
ecolor(ECOLOR color)
{
	खाता *f = मानक_निकास;

	/* Try and guess a valid tty */
	यदि (!isatty(fileno(f))) {
		f = मानक_त्रुटि;
		यदि (!isatty(fileno(f))) {
			f = मानक_निवेश;
			यदि (!isatty(fileno(f)))
				f = रिक्त;
		}
	}

	प्रत्यागम _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

स्थिर पूर्णाङ्क EINFO_PRINTF(3, 0)
	_einfo(खाता *f, ECOLOR color, ध्रुव वर्ण *EINFO_RESTRICT fmt, बहु_सूची va)
{
	पूर्णाङ्क retval = 0;
	वर्ण *last = दो_पर्या("EINFO_LASTCMD");
	बहु_सूची ap;

	यदि (last &&
	    *last &&
	    !colour_terminal(f) &&
	    म_भेद(last, "ewarn") != 0 &&
	    last[म_माप(last) - 1] == 'n')
		ख_लिखो(f, "\n");
	यदि (_eprefix)
		ख_लिखो(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	ख_लिखो(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += भख_लिखो(f, fmt, ap) + 3;
	बहु_पूर्ण(ap); \
	यदि (colour_terminal(f))
		ख_लिखो(f, "%s", flush);
	प्रत्यागम retval;
}

#define _einfovn(fmt, ap) _einfo(मानक_निकास, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(मानक_त्रुटि, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(मानक_त्रुटि, ECOLOR_BAD, fmt, ap)

पूर्णाङ्क
einfon(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	बहु_सूची ap;

	यदि (!fmt || is_quiet())
		प्रत्यागम 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("einfon");
	प्रत्यागम retval;
}

पूर्णाङ्क
ewarnn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	बहु_सूची ap;

	यदि (!fmt || is_quiet())
		प्रत्यागम 0;
	बहु_शुरू(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("ewarnn");
	प्रत्यागम retval;
}

पूर्णाङ्क
eerrorn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	बहु_सूची ap;

	यदि (!fmt || is_really_quiet())
		प्रत्यागम 0;
	बहु_शुरू(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("errorn");
	प्रत्यागम retval;
}

पूर्णाङ्क
einfo(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	बहु_सूची ap;

	यदि (!fmt || is_quiet())
		प्रत्यागम 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += म_लिखो("\n");
	बहु_पूर्ण(ap);
	LASTCMD("einfo");
	प्रत्यागम retval;
}

पूर्णाङ्क
ewarn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	बहु_सूची ap;

	यदि (!fmt || is_quiet())
		प्रत्यागम 0;
	बहु_शुरू(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += ख_लिखो(मानक_त्रुटि, "\n");
	बहु_पूर्ण(ap);
	LASTCMD("ewarn");
	प्रत्यागम retval;
}

शून्य
ewarnx(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	यदि (fmt && !is_quiet()) {
		बहु_शुरू(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		बहु_पूर्ण(ap);
		ख_लिखो(मानक_त्रुटि, "\n");
	}
	निकास(निकास_त्रुटि);
}

पूर्णाङ्क
eerror(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	बहु_सूची ap;

	यदि (!fmt || is_really_quiet())
		प्रत्यागम 0;
	बहु_शुरू(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	बहु_पूर्ण(ap);
	retval += ख_लिखो(मानक_त्रुटि, "\n");
	LASTCMD("eerror");
	प्रत्यागम retval;
}

शून्य
eerrorx(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	यदि (fmt && !is_really_quiet()) {
		बहु_शुरू(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		बहु_पूर्ण(ap);
		ख_लिखो(मानक_त्रुटि, "\n");
	}
	निकास(निकास_त्रुटि);
}

पूर्णाङ्क
ebegin(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	बहु_सूची ap;

	यदि (!fmt || is_quiet())
		प्रत्यागम 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	बहु_पूर्ण(ap);
	retval += म_लिखो(" ...");
	यदि (colour_terminal(मानक_निकास))
		retval += म_लिखो("\n");
	LASTCMD("ebegin");
	प्रत्यागम retval;
}

स्थिर शून्य
_eend(खाता * EINFO_RESTRICT fp, पूर्णाङ्क col, ECOLOR color, ध्रुव वर्ण *संदेश)
{
	पूर्णाङ्क i;
	पूर्णाङ्क cols;

	यदि (!संदेश)
		प्रत्यागम;

	cols = get_term_columns(fp) - (म_माप(संदेश) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	यदि (!term) {
		term = दो_पर्या("TERM");
		यदि (term && म_भेद(term, "cons25") == 0)
			term_is_cons25 = true;
		अन्यथा
			term_is_cons25 = false;
	}
	यदि (term_is_cons25)
		cols--;

	यदि (cols > 0 && colour_terminal(fp)) {
		ख_लिखो(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), संदेश,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} अन्यथा {
		यदि (col > 0)
			प्रति (i = 0; i < cols - col; i++)
				ख_लिखो(fp, " ");
		ख_लिखो(fp, " [ %s ]\n", संदेश);
	}
}

स्थिर पूर्णाङ्क EINFO_PRINTF(3, 0)
_do_eend(ध्रुव वर्ण *cmd, पूर्णाङ्क retval,
    ध्रुव वर्ण *EINFO_RESTRICT fmt, बहु_सूची ap)
{
	पूर्णाङ्क col = 0;
	खाता *fp = मानक_निकास;
	बहु_सूची apc;

	यदि (fmt && *fmt != '\0' && retval != 0) {
		fp = मानक_त्रुटि;
		va_copy(apc, ap);
		यदि (म_भेद(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		अन्यथा
			col = _eerrorvn(fmt, apc);
		col += ख_लिखो(fp, "\n");
		बहु_पूर्ण(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	प्रत्यागम retval;
}

पूर्णाङ्क
eend(पूर्णाङ्क retval, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	यदि (is_quiet())
		प्रत्यागम retval;
	बहु_शुरू(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("eend");
	प्रत्यागम retval;
}

पूर्णाङ्क
ewend(पूर्णाङ्क retval, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	यदि (is_quiet())
		प्रत्यागम retval;
	बहु_शुरू(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("ewend");
	प्रत्यागम retval;
}

शून्य
ebracket(पूर्णाङ्क col, ECOLOR color, ध्रुव वर्ण *संदेश)
{
	_eend(मानक_निकास, col, color, संदेश);
}

शून्य
eindent(शून्य)
{
	वर्ण *env = दो_पर्या("EINFO_INDENT");
	पूर्णाङ्क amount = 0;
	वर्ण *अंक;

	यदि (env) {
		त्रुटि_सं = 0;
		amount = strtoimax(env, रिक्त, 0);
		यदि (त्रुटि_सं != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
	xasprintf(&अंक, "%08d", amount);
	setenv("EINFO_INDENT", अंक, 1);
	मुक्त(अंक);
}

शून्य eoutdent(शून्य)
{
	वर्ण *env = दो_पर्या("EINFO_INDENT");
	पूर्णाङ्क amount = 0;
	वर्ण *अंक = रिक्त;
	पूर्णाङ्क serrno = त्रुटि_सं;

	यदि (!env)
		प्रत्यागम;
	त्रुटि_सं = 0;
	amount = strtoimax(env, रिक्त, 0);
	यदि (त्रुटि_सं != 0)
		amount = 0;
	अन्यथा
		amount -= INDENT_WIDTH;
	यदि (amount <= 0)
		unsetenv("EINFO_INDENT");
	अन्यथा {
		xasprintf(&अंक, "%08d", amount);
		setenv("EINFO_INDENT", अंक, 1);
		मुक्त(अंक);
	}
	त्रुटि_सं = serrno;
}

पूर्णाङ्क
einfovn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	बहु_सूची ap;

	यदि (!fmt || !is_verbose())
		प्रत्यागम 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("einfovn");
	प्रत्यागम retval;
}

पूर्णाङ्क
ewarnvn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	बहु_सूची ap;

	यदि (!fmt || !is_verbose())
		प्रत्यागम 0;
	बहु_शुरू(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("ewarnvn");
	प्रत्यागम retval;
}

पूर्णाङ्क
einfov(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	बहु_सूची ap;

	यदि (!fmt || !is_verbose())
		प्रत्यागम 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += म_लिखो("\n");
	बहु_पूर्ण(ap);
	LASTCMD("einfov");
	प्रत्यागम retval;
}

पूर्णाङ्क
ewarnv(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	बहु_सूची ap;

	यदि (!fmt || !is_verbose())
		प्रत्यागम 0;
	बहु_शुरू(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += म_लिखो("\n");
	बहु_पूर्ण(ap);
	LASTCMD("ewarnv");
	प्रत्यागम retval;
}

पूर्णाङ्क
ebeginv(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	बहु_सूची ap;

	यदि (!fmt || !is_verbose())
		प्रत्यागम 0;

	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += म_लिखो(" ...");
	यदि (colour_terminal(मानक_निकास))
		retval += म_लिखो("\n");
	बहु_पूर्ण(ap);
	LASTCMD("ebeginv");
	प्रत्यागम retval;
}

पूर्णाङ्क
eendv(पूर्णाङ्क retval, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	यदि (!is_verbose())
		प्रत्यागम 0;
	बहु_शुरू(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("eendv");
	प्रत्यागम retval;
}

पूर्णाङ्क
ewendv(पूर्णाङ्क retval, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	यदि (!is_verbose())
		प्रत्यागम 0;
	बहु_शुरू(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	बहु_पूर्ण(ap);
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
