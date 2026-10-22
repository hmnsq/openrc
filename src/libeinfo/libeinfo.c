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

#define _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), आकार(_d));
#define _ASSIGN_CAP(_v) करा {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, आकार(ebuffer) - (p - ebuffer)) + 1;    \
	} पर्यंत (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
स्थिर स्थिरांक अक्षर *_eprefix = रिक्त;

/* Buffers and structures to hold the final colours */
स्थिर अक्षर ebuffer[100];
संरचना ecolor {
	ECOLOR color;
	पूर्णांक def;
	स्थिरांक अक्षर *name;
};
स्थिर अक्षर nullstr = '\0';

स्थिर स्थिरांक संरचना ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_NORMAL,  0,       रिक्त      },
};
स्थिर स्थिरांक अक्षर *ecolors_str[ARRAY_SIZE(ecolors)];

स्थिर अक्षर *flush = रिक्त;
स्थिर अक्षर *up = रिक्त;
स्थिर अक्षर *goto_column = रिक्त;

स्थिर स्थिरांक अक्षर *term = रिक्त;
स्थिर bool term_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
स्थिर स्थिरांक अक्षर *स्थिरांक color_terms[] = {
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
strlcat(अक्षर *dst, स्थिरांक अक्षर *src, माप_प्रकार size)
{
	अक्षर *d = dst;
	स्थिरांक अक्षर *s = src;
	माप_प्रकार src_n = size;
	माप_प्रकार dst_n;

	पर्यंत (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	जर (src_n == 0)
		परत dst_n + म_माप(src);

	पर्यंत (*s != '\0') {
		जर (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	परत dst_n + (s - src);
}
#endif

स्थिर bool
yesno(स्थिरांक अक्षर *मूल्य)
{
	जर (!मूल्य) {
		त्रुटि_सं = ENOENT;
		परत false;
	}

	जर (strcasecmp(मूल्य, "yes") == 0 ||
	    strcasecmp(मूल्य, "y") == 0 ||
	    strcasecmp(मूल्य, "true") == 0 ||
	    strcasecmp(मूल्य, "on") == 0 ||
	    strcasecmp(मूल्य, "1") == 0)
		परत true;

	जर (strcasecmp(मूल्य, "no") != 0 &&
	    strcasecmp(मूल्य, "n") != 0 &&
	    strcasecmp(मूल्य, "false") != 0 &&
	    strcasecmp(मूल्य, "off") != 0 &&
	    strcasecmp(मूल्य, "0") != 0)
		त्रुटि_सं = EINVAL;

	परत false;
}

स्थिर bool
noyes(स्थिरांक अक्षर *मूल्य)
{
	पूर्णांक serrno = त्रुटि_सं;
	bool retval;

	त्रुटि_सं = 0;
	retval = yesno(मूल्य);
	जर (त्रुटि_सं == 0) {
		retval = !retval;
		त्रुटि_सं = serrno;
	}

	परत retval;
}

स्थिर bool
is_quiet(शून्य)
{
	परत yesno(दो_पर्या("EINFO_QUIET"));
}

स्थिर bool
is_really_quiet(शून्य)
{
	परत yesno(दो_पर्या("EERROR_QUIET"));
}

स्थिर bool
is_verbose(शून्य)
{
	परत yesno(दो_पर्या ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
स्थिर अक्षर *
tgoto(स्थिरांक अक्षर *cap, पूर्णांक col, पूर्णांक ओळ)
{
	स्थिर अक्षर buf[20];
	अक्षर *p, *e, c, dbuf[6];
	पूर्णांक oncol = 0, which = ओळ, i;

	p = buf;
	e = p + आकार(buf);
	पर्यंत ((c = *cap++)) {
		जर (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			जर (p >= e) {
				त्रुटि_सं = E2BIG;
				परत रिक्त;
			}
			सुरूठेवा;
		}
		निवड (c) {
		स्थिती '3':
		स्थिती '2':
		स्थिती 'd':
			i = 0;
			करा
				dbuf[i++] = which % 10 | '0';
			पर्यंत ((which /= 10));
			जर (c != 'd') {
				c -= '0';
				जर (i > c) {
					त्रुटि_सं = EINVAL;
					परत रिक्त;
				}
				पर्यंत (i < c)
					dbuf[i++] = '0';
			}
			जर (p + i >= e) {
				त्रुटि_सं = E2BIG;
				परत रिक्त;
			}
			करा
				*p++ = dbuf[--i];
			पर्यंत (i);
			थांबा;
		स्थिती 'r':
			oncol = 0;
			थांबा;
		स्थिती 'i':
			col++;
			ओळ++;
			which++;
			सुरूठेवा;
		पूर्वनिर्धारित:
			त्रुटि_सं = EINVAL;
			परत रिक्त;
		}

		oncol = 1 - oncol;
		which = oncol ? col : ओळ;
	}
	*p = '\0';
	परत buf;
}

स्थिर bool
colour_terminal(खाता * EINFO_RESTRICT f)
{
	स्थिर पूर्णांक in_colour = -1;
	अक्षर *e, *ee, *end, *d, *p;
	पूर्णांक c;
	स्थिरांक अक्षर *_af = रिक्त, *_ce = रिक्त, *_ch = रिक्त;
	स्थिरांक अक्षर *_md = रिक्त, *_me = रिक्त, *_up = रिक्त;
	स्थिरांक अक्षर *bold;
	अक्षर tmp[100];
	अचिन्हित पूर्णांक i = 0;

	जर (f && !isatty(fileno(f)))
		परत false;

	जर (noyes(दो_पर्या("EINFO_COLOR")))
		परत false;

	जर (in_colour == 0)
		परत false;
	जर (in_colour == 1)
		परत true;

	term_is_cons25 = false;
	जर (!term) {
		term = दो_पर्या("TERM");
		जर (!term)
			परत false;
	}
	जर (म_भेद(term, "cons25") == 0)
		term_is_cons25 = true;

	जर (म_माला(term, "color"))
		in_colour = 1;

	पर्यंत (color_terms[i] && in_colour != 1) {
		जर (म_भेद(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	जर (in_colour != 1) {
		in_colour = 0;
		परत false;
	}

	जर (!_af)
		_af = AF;
	जर (!_ce)
		_ce = CE;
	जर (!_ch)
		_ch = CH;
	जर (!_md)
		_md = MD;
	जर (!_me)
		_me = ME;
	जर (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	साठी (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		जर (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			जर ((e = दो_पर्या("EINFO_COLOR"))) {
				ee = म_माला(e, ecolors[i].name);
				जर (ee)
					ee += म_माप(ecolors[i].name);

				जर (ee && *ee == '=') {
					d = strdup(ee + 1);
					जर (d) {
						end = म_अक्षर(d, ':');
						जर (end)
							*end = '\0';
						c = म_से_प(d);
						end = म_अक्षर(d, ';');
						जर (end && *++end == '0')
							bold = _me;
						मुक्त(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), आकार(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), आकार(tmp));
		} अन्यथा
			_GET_CAP(tmp, _me);

		जर (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		अन्यथा
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, आकार(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	परत true;
}

स्थिर पूर्णांक
get_term_columns(खाता * EINFO_RESTRICT stream)
{
	संरचना winsize ws;
	अक्षर *env = दो_पर्या("COLUMNS");
	अक्षर *p;
	पूर्णांक i;

	जर (env) {
		i = strtoimax(env, &p, 10);
		जर (!*p)
			परत i;
	}

	जर (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		परत ws.ws_col;

	परत DEFAULT_COLS;
}

शून्य
eprefix(स्थिरांक अक्षर *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

स्थिर शून्य EINFO_PRINTF(2, 0)
elogv(पूर्णांक level, स्थिरांक अक्षर *EINFO_RESTRICT fmt, बहु_सूची ap)
{
	अक्षर *e = दो_पर्या("EINFO_LOG");
	बहु_सूची apc;

	जर (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		बहु_पूर्ण(apc);
		closelog();
	}
}

शून्य
elog(पूर्णांक level, स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	बहु_शुरू(ap, fmt);
	elogv(level, fmt, ap);
	बहु_पूर्ण(ap);
}

स्थिर पूर्णांक
_eindent(खाता * EINFO_RESTRICT stream)
{
	अक्षर *env = दो_पर्या("EINFO_INDENT");
	पूर्णांक amount = 0;

	जर (env) {
		त्रुटि_सं = 0;
		amount = strtoimax(env, रिक्त, 0);
		जर (त्रुटि_सं != 0 || amount < 0)
			amount = 0;
		अन्यथा जर (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	परत ख_लिखो(stream, "%*s", amount, "");
}

स्थिर स्थिरांक अक्षर *
_ecolor(खाता * EINFO_RESTRICT f, ECOLOR color)
{
	अचिन्हित पूर्णांक i;

	जर (!colour_terminal(f))
		परत "";

	साठी (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		जर (ecolors[i].color == color)
			परत ecolors_str[i];
	परत "";
}

स्थिरांक अक्षर *
ecolor(ECOLOR color)
{
	खाता *f = मानक_निकास;

	/* Try and guess a valid tty */
	जर (!isatty(fileno(f))) {
		f = मानक_त्रुटि;
		जर (!isatty(fileno(f))) {
			f = मानक_निवेश;
			जर (!isatty(fileno(f)))
				f = रिक्त;
		}
	}

	परत _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

स्थिर पूर्णांक EINFO_PRINTF(3, 0)
	_einfo(खाता *f, ECOLOR color, स्थिरांक अक्षर *EINFO_RESTRICT fmt, बहु_सूची va)
{
	पूर्णांक retval = 0;
	अक्षर *last = दो_पर्या("EINFO_LASTCMD");
	बहु_सूची ap;

	जर (last &&
	    *last &&
	    !colour_terminal(f) &&
	    म_भेद(last, "ewarn") != 0 &&
	    last[म_माप(last) - 1] == 'n')
		ख_लिखो(f, "\n");
	जर (_eprefix)
		ख_लिखो(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	ख_लिखो(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += भख_लिखो(f, fmt, ap) + 3;
	बहु_पूर्ण(ap); \
	जर (colour_terminal(f))
		ख_लिखो(f, "%s", flush);
	परत retval;
}

#define _einfovn(fmt, ap) _einfo(मानक_निकास, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(मानक_त्रुटि, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(मानक_त्रुटि, ECOLOR_BAD, fmt, ap)

पूर्णांक
einfon(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	जर (!fmt || is_quiet())
		परत 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("einfon");
	परत retval;
}

पूर्णांक
ewarnn(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	जर (!fmt || is_quiet())
		परत 0;
	बहु_शुरू(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("ewarnn");
	परत retval;
}

पूर्णांक
eerrorn(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	जर (!fmt || is_really_quiet())
		परत 0;
	बहु_शुरू(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("errorn");
	परत retval;
}

पूर्णांक
einfo(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	जर (!fmt || is_quiet())
		परत 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += म_लिखो("\n");
	बहु_पूर्ण(ap);
	LASTCMD("einfo");
	परत retval;
}

पूर्णांक
ewarn(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	जर (!fmt || is_quiet())
		परत 0;
	बहु_शुरू(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += ख_लिखो(मानक_त्रुटि, "\n");
	बहु_पूर्ण(ap);
	LASTCMD("ewarn");
	परत retval;
}

शून्य
ewarnx(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	जर (fmt && !is_quiet()) {
		बहु_शुरू(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		बहु_पूर्ण(ap);
		ख_लिखो(मानक_त्रुटि, "\n");
	}
	निकास(निकास_त्रुटि);
}

पूर्णांक
eerror(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	जर (!fmt || is_really_quiet())
		परत 0;
	बहु_शुरू(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	बहु_पूर्ण(ap);
	retval += ख_लिखो(मानक_त्रुटि, "\n");
	LASTCMD("eerror");
	परत retval;
}

शून्य
eerrorx(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	जर (fmt && !is_really_quiet()) {
		बहु_शुरू(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		बहु_पूर्ण(ap);
		ख_लिखो(मानक_त्रुटि, "\n");
	}
	निकास(निकास_त्रुटि);
}

पूर्णांक
ebegin(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	जर (!fmt || is_quiet())
		परत 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	बहु_पूर्ण(ap);
	retval += म_लिखो(" ...");
	जर (colour_terminal(मानक_निकास))
		retval += म_लिखो("\n");
	LASTCMD("ebegin");
	परत retval;
}

स्थिर शून्य
_eend(खाता * EINFO_RESTRICT fp, पूर्णांक col, ECOLOR color, स्थिरांक अक्षर *संदेश)
{
	पूर्णांक i;
	पूर्णांक cols;

	जर (!संदेश)
		परत;

	cols = get_term_columns(fp) - (म_माप(संदेश) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	जर (!term) {
		term = दो_पर्या("TERM");
		जर (term && म_भेद(term, "cons25") == 0)
			term_is_cons25 = true;
		अन्यथा
			term_is_cons25 = false;
	}
	जर (term_is_cons25)
		cols--;

	जर (cols > 0 && colour_terminal(fp)) {
		ख_लिखो(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), संदेश,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} अन्यथा {
		जर (col > 0)
			साठी (i = 0; i < cols - col; i++)
				ख_लिखो(fp, " ");
		ख_लिखो(fp, " [ %s ]\n", संदेश);
	}
}

स्थिर पूर्णांक EINFO_PRINTF(3, 0)
_do_eend(स्थिरांक अक्षर *cmd, पूर्णांक retval,
    स्थिरांक अक्षर *EINFO_RESTRICT fmt, बहु_सूची ap)
{
	पूर्णांक col = 0;
	खाता *fp = मानक_निकास;
	बहु_सूची apc;

	जर (fmt && *fmt != '\0' && retval != 0) {
		fp = मानक_त्रुटि;
		va_copy(apc, ap);
		जर (म_भेद(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		अन्यथा
			col = _eerrorvn(fmt, apc);
		col += ख_लिखो(fp, "\n");
		बहु_पूर्ण(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	परत retval;
}

पूर्णांक
eend(पूर्णांक retval, स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	जर (is_quiet())
		परत retval;
	बहु_शुरू(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("eend");
	परत retval;
}

पूर्णांक
ewend(पूर्णांक retval, स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	जर (is_quiet())
		परत retval;
	बहु_शुरू(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("ewend");
	परत retval;
}

शून्य
ebracket(पूर्णांक col, ECOLOR color, स्थिरांक अक्षर *संदेश)
{
	_eend(मानक_निकास, col, color, संदेश);
}

शून्य
eindent(शून्य)
{
	अक्षर *env = दो_पर्या("EINFO_INDENT");
	पूर्णांक amount = 0;
	अक्षर *अंक;

	जर (env) {
		त्रुटि_सं = 0;
		amount = strtoimax(env, रिक्त, 0);
		जर (त्रुटि_सं != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
	xasprintf(&अंक, "%08d", amount);
	setenv("EINFO_INDENT", अंक, 1);
	मुक्त(अंक);
}

शून्य eoutdent(शून्य)
{
	अक्षर *env = दो_पर्या("EINFO_INDENT");
	पूर्णांक amount = 0;
	अक्षर *अंक = रिक्त;
	पूर्णांक serrno = त्रुटि_सं;

	जर (!env)
		परत;
	त्रुटि_सं = 0;
	amount = strtoimax(env, रिक्त, 0);
	जर (त्रुटि_सं != 0)
		amount = 0;
	अन्यथा
		amount -= INDENT_WIDTH;
	जर (amount <= 0)
		unsetenv("EINFO_INDENT");
	अन्यथा {
		xasprintf(&अंक, "%08d", amount);
		setenv("EINFO_INDENT", अंक, 1);
		मुक्त(अंक);
	}
	त्रुटि_सं = serrno;
}

पूर्णांक
einfovn(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	जर (!fmt || !is_verbose())
		परत 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("einfovn");
	परत retval;
}

पूर्णांक
ewarnvn(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	जर (!fmt || !is_verbose())
		परत 0;
	बहु_शुरू(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("ewarnvn");
	परत retval;
}

पूर्णांक
einfov(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	जर (!fmt || !is_verbose())
		परत 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += म_लिखो("\n");
	बहु_पूर्ण(ap);
	LASTCMD("einfov");
	परत retval;
}

पूर्णांक
ewarnv(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	जर (!fmt || !is_verbose())
		परत 0;
	बहु_शुरू(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += म_लिखो("\n");
	बहु_पूर्ण(ap);
	LASTCMD("ewarnv");
	परत retval;
}

पूर्णांक
ebeginv(स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	जर (!fmt || !is_verbose())
		परत 0;

	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += म_लिखो(" ...");
	जर (colour_terminal(मानक_निकास))
		retval += म_लिखो("\n");
	बहु_पूर्ण(ap);
	LASTCMD("ebeginv");
	परत retval;
}

पूर्णांक
eendv(पूर्णांक retval, स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	जर (!is_verbose())
		परत 0;
	बहु_शुरू(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("eendv");
	परत retval;
}

पूर्णांक
ewendv(पूर्णांक retval, स्थिरांक अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	जर (!is_verbose())
		परत 0;
	बहु_शुरू(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("ewendv");
	परत retval;
}

शून्य
eindentv(शून्य)
{
	जर (is_verbose())
		eindent();
}

शून्य
eoutdentv(शून्य)
{
	जर (is_verbose())
		eoutdent();
}
