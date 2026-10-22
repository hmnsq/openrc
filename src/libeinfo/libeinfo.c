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

#समावेश <sys/ioctl.h>
#समावेश <errno.h>
#समावेश <inttypes.h>
#समावेश <stdarg.h>
#समावेश <stdbool.h>
#समावेश <stddef.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <syslog.h>
#समावेश <unistd.h>

#समावेश "einfo.h"
#समावेश "helpers.h"

/* In case we cannot work out how many columns from ioctl, supply a default */
#घोषणा DEFAULT_COLS		 80

#घोषणा OK			"ok"
#घोषणा NOT_OK			"!!"

/* Number of spaces for an indent */
#घोषणा INDENT_WIDTH		2

/* How wide can the indent go? */
#घोषणा INDENT_MAX		40

/* Default colours */
#घोषणा GOOD                    2
#घोषणा WARN                    3
#घोषणा BAD                     1
#घोषणा HILITE                  6
#घोषणा BRACKET                 4

/* ANSI escape codes which mimic termcap */
#घोषणा AF "\033[3%dm"
#घोषणा CE "\033[K"
#घोषणा CH "\033[%dC"
#घोषणा MD "\033[1m"
#घोषणा ME "\033[m"
#घोषणा UP "\033[A"

#घोषणा _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), माप(_d));
#घोषणा _ASSIGN_CAP(_v) करो {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, माप(ebuffer) - (p - ebuffer)) + 1;    \
	} जबतक (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
अटल स्थिर अक्षर *_eprefix = शून्य;

/* Buffers and structures to hold the final colours */
अटल अक्षर ebuffer[100];
काष्ठा ecolor {
	ECOLOR color;
	पूर्णांक def;
	स्थिर अक्षर *name;
};
अटल अक्षर nullstr = '\0';

अटल स्थिर काष्ठा ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_NORMAL,  0,       शून्य      },
};
अटल स्थिर अक्षर *ecolors_str[ARRAY_आकार(ecolors)];

अटल अक्षर *flush = शून्य;
अटल अक्षर *up = शून्य;
अटल अक्षर *goto_column = शून्य;

अटल स्थिर अक्षर *term = शून्य;
अटल bool term_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
अटल स्थिर अक्षर *स्थिर color_terms[] = {
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
	शून्य
};

#अगर_अघोषित HAVE_STRLCPY
अटल माप_प्रकार
strlcat(अक्षर *dst, स्थिर अक्षर *src, माप_प्रकार आकार)
{
	अक्षर *d = dst;
	स्थिर अक्षर *s = src;
	माप_प्रकार src_n = आकार;
	माप_प्रकार dst_n;

	जबतक (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = आकार - dst_n;

	अगर (src_n == 0)
		वापस dst_n + म_माप(src);

	जबतक (*s != '\0') {
		अगर (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	वापस dst_n + (s - src);
}
#पूर्ण_अगर

अटल bool
yesno(स्थिर अक्षर *मूल्य)
{
	अगर (!मूल्य) {
		त्रुटि_सं = ENOENT;
		वापस false;
	}

	अगर (strcasecmp(मूल्य, "yes") == 0 ||
	    strcasecmp(मूल्य, "y") == 0 ||
	    strcasecmp(मूल्य, "true") == 0 ||
	    strcasecmp(मूल्य, "on") == 0 ||
	    strcasecmp(मूल्य, "1") == 0)
		वापस true;

	अगर (strcasecmp(मूल्य, "no") != 0 &&
	    strcasecmp(मूल्य, "n") != 0 &&
	    strcasecmp(मूल्य, "false") != 0 &&
	    strcasecmp(मूल्य, "off") != 0 &&
	    strcasecmp(मूल्य, "0") != 0)
		त्रुटि_सं = EINVAL;

	वापस false;
}

अटल bool
noyes(स्थिर अक्षर *मूल्य)
{
	पूर्णांक serrno = त्रुटि_सं;
	bool retval;

	त्रुटि_सं = 0;
	retval = yesno(मूल्य);
	अगर (त्रुटि_सं == 0) {
		retval = !retval;
		त्रुटि_सं = serrno;
	}

	वापस retval;
}

अटल bool
is_quiet(व्योम)
{
	वापस yesno(दो_पर्या("EINFO_QUIET"));
}

अटल bool
is_really_quiet(व्योम)
{
	वापस yesno(दो_पर्या("EERROR_QUIET"));
}

अटल bool
is_verbose(व्योम)
{
	वापस yesno(दो_पर्या ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
अटल अक्षर *
tgoto(स्थिर अक्षर *cap, पूर्णांक col, पूर्णांक पंक्ति)
{
	अटल अक्षर buf[20];
	अक्षर *p, *e, c, dbuf[6];
	पूर्णांक oncol = 0, which = पंक्ति, i;

	p = buf;
	e = p + माप(buf);
	जबतक ((c = *cap++)) {
		अगर (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			अगर (p >= e) {
				त्रुटि_सं = E2BIG;
				वापस शून्य;
			}
			जारी;
		}
		चयन (c) {
		हाल '3':
		हाल '2':
		हाल 'd':
			i = 0;
			करो
				dbuf[i++] = which % 10 | '0';
			जबतक ((which /= 10));
			अगर (c != 'd') {
				c -= '0';
				अगर (i > c) {
					त्रुटि_सं = EINVAL;
					वापस शून्य;
				}
				जबतक (i < c)
					dbuf[i++] = '0';
			}
			अगर (p + i >= e) {
				त्रुटि_सं = E2BIG;
				वापस शून्य;
			}
			करो
				*p++ = dbuf[--i];
			जबतक (i);
			अवरोध;
		हाल 'r':
			oncol = 0;
			अवरोध;
		हाल 'i':
			col++;
			पंक्ति++;
			which++;
			जारी;
		शेष:
			त्रुटि_सं = EINVAL;
			वापस शून्य;
		}

		oncol = 1 - oncol;
		which = oncol ? col : पंक्ति;
	}
	*p = '\0';
	वापस buf;
}

अटल bool
colour_terminal(खाता * EINFO_RESTRICT f)
{
	अटल पूर्णांक in_colour = -1;
	अक्षर *e, *ee, *end, *d, *p;
	पूर्णांक c;
	स्थिर अक्षर *_af = शून्य, *_ce = शून्य, *_ch = शून्य;
	स्थिर अक्षर *_md = शून्य, *_me = शून्य, *_up = शून्य;
	स्थिर अक्षर *bold;
	अक्षर tmp[100];
	अचिन्हित पूर्णांक i = 0;

	अगर (f && !isatty(fileno(f)))
		वापस false;

	अगर (noyes(दो_पर्या("EINFO_COLOR")))
		वापस false;

	अगर (in_colour == 0)
		वापस false;
	अगर (in_colour == 1)
		वापस true;

	term_is_cons25 = false;
	अगर (!term) {
		term = दो_पर्या("TERM");
		अगर (!term)
			वापस false;
	}
	अगर (म_भेद(term, "cons25") == 0)
		term_is_cons25 = true;

	अगर (म_माला(term, "color"))
		in_colour = 1;

	जबतक (color_terms[i] && in_colour != 1) {
		अगर (म_भेद(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	अगर (in_colour != 1) {
		in_colour = 0;
		वापस false;
	}

	अगर (!_af)
		_af = AF;
	अगर (!_ce)
		_ce = CE;
	अगर (!_ch)
		_ch = CH;
	अगर (!_md)
		_md = MD;
	अगर (!_me)
		_me = ME;
	अगर (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	क्रम (i = 0; i < ARRAY_आकार(ecolors); ++i) {
		tmp[0] = '\0';
		अगर (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			अगर ((e = दो_पर्या("EINFO_COLOR"))) {
				ee = म_माला(e, ecolors[i].name);
				अगर (ee)
					ee += म_माप(ecolors[i].name);

				अगर (ee && *ee == '=') {
					d = strdup(ee + 1);
					अगर (d) {
						end = म_अक्षर(d, ':');
						अगर (end)
							*end = '\0';
						c = म_से_प(d);
						end = म_अक्षर(d, ';');
						अगर (end && *++end == '0')
							bold = _me;
						मुक्त(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), माप(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), माप(tmp));
		} अन्यथा
			_GET_CAP(tmp, _me);

		अगर (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		अन्यथा
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, माप(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	वापस true;
}

अटल पूर्णांक
get_term_columns(खाता * EINFO_RESTRICT stream)
{
	काष्ठा winsize ws;
	अक्षर *env = दो_पर्या("COLUMNS");
	अक्षर *p;
	पूर्णांक i;

	अगर (env) {
		i = strtoimax(env, &p, 10);
		अगर (!*p)
			वापस i;
	}

	अगर (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		वापस ws.ws_col;

	वापस DEFAULT_COLS;
}

व्योम
eprefix(स्थिर अक्षर *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

अटल व्योम EINFO_PRINTF(2, 0)
elogv(पूर्णांक level, स्थिर अक्षर *EINFO_RESTRICT fmt, बहु_सूची ap)
{
	अक्षर *e = दो_पर्या("EINFO_LOG");
	बहु_सूची apc;

	अगर (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		बहु_पूर्ण(apc);
		closelog();
	}
}

व्योम
elog(पूर्णांक level, स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	बहु_शुरू(ap, fmt);
	elogv(level, fmt, ap);
	बहु_पूर्ण(ap);
}

अटल पूर्णांक
_eindent(खाता * EINFO_RESTRICT stream)
{
	अक्षर *env = दो_पर्या("EINFO_INDENT");
	पूर्णांक amount = 0;

	अगर (env) {
		त्रुटि_सं = 0;
		amount = strtoimax(env, शून्य, 0);
		अगर (त्रुटि_सं != 0 || amount < 0)
			amount = 0;
		अन्यथा अगर (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	वापस ख_लिखो(stream, "%*s", amount, "");
}

अटल स्थिर अक्षर *
_ecolor(खाता * EINFO_RESTRICT f, ECOLOR color)
{
	अचिन्हित पूर्णांक i;

	अगर (!colour_terminal(f))
		वापस "";

	क्रम (i = 0; i < ARRAY_आकार(ecolors); ++i)
		अगर (ecolors[i].color == color)
			वापस ecolors_str[i];
	वापस "";
}

स्थिर अक्षर *
ecolor(ECOLOR color)
{
	खाता *f = मानक_निकास;

	/* Try and guess a valid tty */
	अगर (!isatty(fileno(f))) {
		f = मानक_त्रुटि;
		अगर (!isatty(fileno(f))) {
			f = मानक_निवेश;
			अगर (!isatty(fileno(f)))
				f = शून्य;
		}
	}

	वापस _ecolor(f, color);
}

#घोषणा LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

अटल पूर्णांक EINFO_PRINTF(3, 0)
	_einfo(खाता *f, ECOLOR color, स्थिर अक्षर *EINFO_RESTRICT fmt, बहु_सूची va)
{
	पूर्णांक retval = 0;
	अक्षर *last = दो_पर्या("EINFO_LASTCMD");
	बहु_सूची ap;

	अगर (last &&
	    *last &&
	    !colour_terminal(f) &&
	    म_भेद(last, "ewarn") != 0 &&
	    last[म_माप(last) - 1] == 'n')
		ख_लिखो(f, "\n");
	अगर (_eprefix)
		ख_लिखो(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	ख_लिखो(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += भख_लिखो(f, fmt, ap) + 3;
	बहु_पूर्ण(ap); \
	अगर (colour_terminal(f))
		ख_लिखो(f, "%s", flush);
	वापस retval;
}

#घोषणा _einfovn(fmt, ap) _einfo(मानक_निकास, ECOLOR_GOOD, fmt, ap)
#घोषणा _ewarnvn(fmt, ap) _einfo(मानक_त्रुटि, ECOLOR_WARN, fmt, ap)
#घोषणा _eerrorvn(fmt, ap) _einfo(मानक_त्रुटि, ECOLOR_BAD, fmt, ap)

पूर्णांक
einfon(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	अगर (!fmt || is_quiet())
		वापस 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("einfon");
	वापस retval;
}

पूर्णांक
ewarnn(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	अगर (!fmt || is_quiet())
		वापस 0;
	बहु_शुरू(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("ewarnn");
	वापस retval;
}

पूर्णांक
eerrorn(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	अगर (!fmt || is_really_quiet())
		वापस 0;
	बहु_शुरू(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("errorn");
	वापस retval;
}

पूर्णांक
einfo(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	अगर (!fmt || is_quiet())
		वापस 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += म_लिखो("\n");
	बहु_पूर्ण(ap);
	LASTCMD("einfo");
	वापस retval;
}

पूर्णांक
ewarn(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	अगर (!fmt || is_quiet())
		वापस 0;
	बहु_शुरू(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += ख_लिखो(मानक_त्रुटि, "\n");
	बहु_पूर्ण(ap);
	LASTCMD("ewarn");
	वापस retval;
}

व्योम
ewarnx(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	अगर (fmt && !is_quiet()) {
		बहु_शुरू(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		बहु_पूर्ण(ap);
		ख_लिखो(मानक_त्रुटि, "\n");
	}
	निकास(निकास_त्रुटि);
}

पूर्णांक
eerror(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	अगर (!fmt || is_really_quiet())
		वापस 0;
	बहु_शुरू(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	बहु_पूर्ण(ap);
	retval += ख_लिखो(मानक_त्रुटि, "\n");
	LASTCMD("eerror");
	वापस retval;
}

व्योम
eerrorx(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	अगर (fmt && !is_really_quiet()) {
		बहु_शुरू(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		बहु_पूर्ण(ap);
		ख_लिखो(मानक_त्रुटि, "\n");
	}
	निकास(निकास_त्रुटि);
}

पूर्णांक
ebegin(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	अगर (!fmt || is_quiet())
		वापस 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	बहु_पूर्ण(ap);
	retval += म_लिखो(" ...");
	अगर (colour_terminal(मानक_निकास))
		retval += म_लिखो("\n");
	LASTCMD("ebegin");
	वापस retval;
}

अटल व्योम
_eend(खाता * EINFO_RESTRICT fp, पूर्णांक col, ECOLOR color, स्थिर अक्षर *संदेश)
{
	पूर्णांक i;
	पूर्णांक cols;

	अगर (!संदेश)
		वापस;

	cols = get_term_columns(fp) - (म_माप(संदेश) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	अगर (!term) {
		term = दो_पर्या("TERM");
		अगर (term && म_भेद(term, "cons25") == 0)
			term_is_cons25 = true;
		अन्यथा
			term_is_cons25 = false;
	}
	अगर (term_is_cons25)
		cols--;

	अगर (cols > 0 && colour_terminal(fp)) {
		ख_लिखो(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), संदेश,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} अन्यथा {
		अगर (col > 0)
			क्रम (i = 0; i < cols - col; i++)
				ख_लिखो(fp, " ");
		ख_लिखो(fp, " [ %s ]\n", संदेश);
	}
}

अटल पूर्णांक EINFO_PRINTF(3, 0)
_do_eend(स्थिर अक्षर *cmd, पूर्णांक retval,
    स्थिर अक्षर *EINFO_RESTRICT fmt, बहु_सूची ap)
{
	पूर्णांक col = 0;
	खाता *fp = मानक_निकास;
	बहु_सूची apc;

	अगर (fmt && *fmt != '\0' && retval != 0) {
		fp = मानक_त्रुटि;
		va_copy(apc, ap);
		अगर (म_भेद(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		अन्यथा
			col = _eerrorvn(fmt, apc);
		col += ख_लिखो(fp, "\n");
		बहु_पूर्ण(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	वापस retval;
}

पूर्णांक
eend(पूर्णांक retval, स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	अगर (is_quiet())
		वापस retval;
	बहु_शुरू(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("eend");
	वापस retval;
}

पूर्णांक
ewend(पूर्णांक retval, स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	अगर (is_quiet())
		वापस retval;
	बहु_शुरू(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("ewend");
	वापस retval;
}

व्योम
ebracket(पूर्णांक col, ECOLOR color, स्थिर अक्षर *संदेश)
{
	_eend(मानक_निकास, col, color, संदेश);
}

व्योम
eindent(व्योम)
{
	अक्षर *env = दो_पर्या("EINFO_INDENT");
	पूर्णांक amount = 0;
	अक्षर *अंक;

	अगर (env) {
		त्रुटि_सं = 0;
		amount = strtoimax(env, शून्य, 0);
		अगर (त्रुटि_सं != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
	xasprintf(&अंक, "%08d", amount);
	setenv("EINFO_INDENT", अंक, 1);
	मुक्त(अंक);
}

व्योम eoutdent(व्योम)
{
	अक्षर *env = दो_पर्या("EINFO_INDENT");
	पूर्णांक amount = 0;
	अक्षर *अंक = शून्य;
	पूर्णांक serrno = त्रुटि_सं;

	अगर (!env)
		वापस;
	त्रुटि_सं = 0;
	amount = strtoimax(env, शून्य, 0);
	अगर (त्रुटि_सं != 0)
		amount = 0;
	अन्यथा
		amount -= INDENT_WIDTH;
	अगर (amount <= 0)
		unsetenv("EINFO_INDENT");
	अन्यथा {
		xasprintf(&अंक, "%08d", amount);
		setenv("EINFO_INDENT", अंक, 1);
		मुक्त(अंक);
	}
	त्रुटि_सं = serrno;
}

पूर्णांक
einfovn(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	अगर (!fmt || !is_verbose())
		वापस 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("einfovn");
	वापस retval;
}

पूर्णांक
ewarnvn(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	अगर (!fmt || !is_verbose())
		वापस 0;
	बहु_शुरू(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("ewarnvn");
	वापस retval;
}

पूर्णांक
einfov(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	अगर (!fmt || !is_verbose())
		वापस 0;
	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += म_लिखो("\n");
	बहु_पूर्ण(ap);
	LASTCMD("einfov");
	वापस retval;
}

पूर्णांक
ewarnv(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	अगर (!fmt || !is_verbose())
		वापस 0;
	बहु_शुरू(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += म_लिखो("\n");
	बहु_पूर्ण(ap);
	LASTCMD("ewarnv");
	वापस retval;
}

पूर्णांक
ebeginv(स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	पूर्णांक retval;
	बहु_सूची ap;

	अगर (!fmt || !is_verbose())
		वापस 0;

	बहु_शुरू(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += म_लिखो(" ...");
	अगर (colour_terminal(मानक_निकास))
		retval += म_लिखो("\n");
	बहु_पूर्ण(ap);
	LASTCMD("ebeginv");
	वापस retval;
}

पूर्णांक
eendv(पूर्णांक retval, स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	अगर (!is_verbose())
		वापस 0;
	बहु_शुरू(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("eendv");
	वापस retval;
}

पूर्णांक
ewendv(पूर्णांक retval, स्थिर अक्षर *EINFO_RESTRICT fmt, ...)
{
	बहु_सूची ap;

	अगर (!is_verbose())
		वापस 0;
	बहु_शुरू(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	बहु_पूर्ण(ap);
	LASTCMD("ewendv");
	वापस retval;
}

व्योम
eindentv(व्योम)
{
	अगर (is_verbose())
		eindent();
}

व्योम
eoutdentv(व्योम)
{
	अगर (is_verbose())
		eoutdent();
}
