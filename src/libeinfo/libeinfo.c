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
#define _ASSIGN_CAP(_v) करोहि {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, परिमाण(ebuffer) - (p - ebuffer)) + 1;    \
	} याव (0)

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
स्थिर size_t
strlcat(वर्ण *dst, ध्रुव वर्ण *src, size_t size)
{
	वर्ण *d = dst;
	ध्रुव वर्ण *s = src;
	size_t src_n = size;
	size_t dst_n;

	याव (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = size - dst_n;

	सचे (src_n == 0)
		पटिनिवत्त dst_n + strlen(src);

	याव (*s != '\0') {
		सचे (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	पटिनिवत्त dst_n + (s - src);
}
#endif

स्थिर bool
yesno(ध्रुव वर्ण *मानम्)
{
	सचे (!मानम्) {
		errno = ENOENT;
		पटिनिवत्त false;
	}

	सचे (strcasecmp(मानम्, "yes") == 0 ||
	    strcasecmp(मानम्, "y") == 0 ||
	    strcasecmp(मानम्, "true") == 0 ||
	    strcasecmp(मानम्, "on") == 0 ||
	    strcasecmp(मानम्, "1") == 0)
		पटिनिवत्त true;

	सचे (strcasecmp(मानम्, "no") != 0 &&
	    strcasecmp(मानम्, "n") != 0 &&
	    strcasecmp(मानम्, "false") != 0 &&
	    strcasecmp(मानम्, "off") != 0 &&
	    strcasecmp(मानम्, "0") != 0)
		errno = EINVAL;

	पटिनिवत्त false;
}

स्थिर bool
noyes(ध्रुव वर्ण *मानम्)
{
	पूर्णाङ्क serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(मानम्);
	सचे (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	पटिनिवत्त retval;
}

स्थिर bool
is_quiet(शून्य)
{
	पटिनिवत्त yesno(getenv("EINFO_QUIET"));
}

स्थिर bool
is_really_quiet(शून्य)
{
	पटिनिवत्त yesno(getenv("EERROR_QUIET"));
}

स्थिर bool
is_verbose(शून्य)
{
	पटिनिवत्त yesno(getenv ("EINFO_VERBOSE"));
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
	याव ((c = *cap++)) {
		सचे (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			सचे (p >= e) {
				errno = E2BIG;
				पटिनिवत्त रिक्त;
			}
			अनुवर्त;
		}
		विकल्प (c) {
		पक्ष '3':
		पक्ष '2':
		पक्ष 'd':
			i = 0;
			करोहि
				dbuf[i++] = which % 10 | '0';
			याव ((which /= 10));
			सचे (c != 'd') {
				c -= '0';
				सचे (i > c) {
					errno = EINVAL;
					पटिनिवत्त रिक्त;
				}
				याव (i < c)
					dbuf[i++] = '0';
			}
			सचे (p + i >= e) {
				errno = E2BIG;
				पटिनिवत्त रिक्त;
			}
			करोहि
				*p++ = dbuf[--i];
			याव (i);
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
			पटिनिवत्त रिक्त;
		}

		oncol = 1 - oncol;
		which = oncol ? col : पङ्क्तिः;
	}
	*p = '\0';
	पटिनिवत्त buf;
}

स्थिर bool
colour_terminal(FILE * EINFO_RESTRICT f)
{
	स्थिर पूर्णाङ्क in_colour = -1;
	वर्ण *e, *ee, *end, *d, *p;
	पूर्णाङ्क c;
	ध्रुव वर्ण *_af = रिक्त, *_ce = रिक्त, *_ch = रिक्त;
	ध्रुव वर्ण *_md = रिक्त, *_me = रिक्त, *_up = रिक्त;
	ध्रुव वर्ण *bold;
	वर्ण tmp[100];
	unsigned पूर्णाङ्क i = 0;

	सचे (f && !isatty(fileno(f)))
		पटिनिवत्त false;

	सचे (noyes(getenv("EINFO_COLOR")))
		पटिनिवत्त false;

	सचे (in_colour == 0)
		पटिनिवत्त false;
	सचे (in_colour == 1)
		पटिनिवत्त true;

	term_is_cons25 = false;
	सचे (!term) {
		term = getenv("TERM");
		सचे (!term)
			पटिनिवत्त false;
	}
	सचे (strcmp(term, "cons25") == 0)
		term_is_cons25 = true;

	सचे (strstr(term, "color"))
		in_colour = 1;

	याव (color_terms[i] && in_colour != 1) {
		सचे (strcmp(color_terms[i], term) == 0) {
			in_colour = 1;
		}
		i++;
	}

	सचे (in_colour != 1) {
		in_colour = 0;
		पटिनिवत्त false;
	}

	सचे (!_af)
		_af = AF;
	सचे (!_ce)
		_ce = CE;
	सचे (!_ch)
		_ch = CH;
	सचे (!_md)
		_md = MD;
	सचे (!_me)
		_me = ME;
	सचे (!_up)
		_up = UP;

	/* Now setup our colours */
	p = ebuffer;
	अत्थाय (i = 0; i < ARRAY_SIZE(ecolors); ++i) {
		tmp[0] = '\0';
		सचे (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			सचे ((e = getenv("EINFO_COLOR"))) {
				ee = strstr(e, ecolors[i].name);
				सचे (ee)
					ee += strlen(ecolors[i].name);

				सचे (ee && *ee == '=') {
					d = strdup(ee + 1);
					सचे (d) {
						end = strchr(d, ':');
						सचे (end)
							*end = '\0';
						c = atoi(d);
						end = strchr(d, ';');
						सचे (end && *++end == '0')
							bold = _me;
						मुक्त(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), परिमाण(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), परिमाण(tmp));
		} अञ्ञथा
			_GET_CAP(tmp, _me);

		सचे (tmp[0])
			_ASSIGN_CAP(ecolors_str[i]);
		अञ्ञथा
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	_ASSIGN_CAP(flush);
	_GET_CAP(tmp, _up);
	_ASSIGN_CAP(up);
	strlcpy(tmp, _ch, परिमाण(tmp));
	_ASSIGN_CAP(goto_column);

	in_colour = 1;
	पटिनिवत्त true;
}

स्थिर पूर्णाङ्क
get_term_columns(FILE * EINFO_RESTRICT stream)
{
	संरचना winsize ws;
	वर्ण *env = getenv("COLUMNS");
	वर्ण *p;
	पूर्णाङ्क i;

	सचे (env) {
		i = strtoimax(env, &p, 10);
		सचे (!*p)
			पटिनिवत्त i;
	}

	सचे (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
		पटिनिवत्त ws.ws_col;

	पटिनिवत्त DEFAULT_COLS;
}

शून्य
eprefix(ध्रुव वर्ण *EINFO_RESTRICT prefix)
{
	_eprefix = prefix;
}

स्थिर शून्य EINFO_PRINTF(2, 0)
elogv(पूर्णाङ्क level, ध्रुव वर्ण *EINFO_RESTRICT fmt, va_list ap)
{
	वर्ण *e = getenv("EINFO_LOG");
	va_list apc;

	सचे (fmt && e) {
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
_eindent(FILE * EINFO_RESTRICT stream)
{
	वर्ण *env = getenv("EINFO_INDENT");
	पूर्णाङ्क amount = 0;

	सचे (env) {
		errno = 0;
		amount = strtoimax(env, रिक्त, 0);
		सचे (errno != 0 || amount < 0)
			amount = 0;
		अञ्ञथा सचे (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	पटिनिवत्त fprintf(stream, "%*s", amount, "");
}

स्थिर ध्रुव वर्ण *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	unsigned पूर्णाङ्क i;

	सचे (!colour_terminal(f))
		पटिनिवत्त "";

	अत्थाय (i = 0; i < ARRAY_SIZE(ecolors); ++i)
		सचे (ecolors[i].color == color)
			पटिनिवत्त ecolors_str[i];
	पटिनिवत्त "";
}

ध्रुव वर्ण *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	सचे (!isatty(fileno(f))) {
		f = stderr;
		सचे (!isatty(fileno(f))) {
			f = stdin;
			सचे (!isatty(fileno(f)))
				f = रिक्त;
		}
	}

	पटिनिवत्त _ecolor(f, color);
}

#define LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

स्थिर पूर्णाङ्क EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, ध्रुव वर्ण *EINFO_RESTRICT fmt, va_list va)
{
	पूर्णाङ्क retval = 0;
	वर्ण *last = getenv("EINFO_LASTCMD");
	va_list ap;

	सचे (last &&
	    *last &&
	    !colour_terminal(f) &&
	    strcmp(last, "ewarn") != 0 &&
	    last[strlen(last) - 1] == 'n')
		fprintf(f, "\n");
	सचे (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_NORMAL));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_NORMAL));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	सचे (colour_terminal(f))
		fprintf(f, "%s", flush);
	पटिनिवत्त retval;
}

#define _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#define _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#define _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

पूर्णाङ्क
einfon(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	सचे (!fmt || is_quiet())
		पटिनिवत्त 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	पटिनिवत्त retval;
}

पूर्णाङ्क
ewarnn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	सचे (!fmt || is_quiet())
		पटिनिवत्त 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	पटिनिवत्त retval;
}

पूर्णाङ्क
eerrorn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	सचे (!fmt || is_really_quiet())
		पटिनिवत्त 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	पटिनिवत्त retval;
}

पूर्णाङ्क
einfo(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	सचे (!fmt || is_quiet())
		पटिनिवत्त 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfo");
	पटिनिवत्त retval;
}

पूर्णाङ्क
ewarn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	सचे (!fmt || is_quiet())
		पटिनिवत्त 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	पटिनिवत्त retval;
}

शून्य
ewarnx(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	सचे (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

पूर्णाङ्क
eerror(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	सचे (!fmt || is_really_quiet())
		पटिनिवत्त 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	पटिनिवत्त retval;
}

शून्य
eerrorx(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	सचे (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

पूर्णाङ्क
ebegin(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	सचे (!fmt || is_quiet())
		पटिनिवत्त 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += printf(" ...");
	सचे (colour_terminal(stdout))
		retval += printf("\n");
	LASTCMD("ebegin");
	पटिनिवत्त retval;
}

स्थिर शून्य
_eend(FILE * EINFO_RESTRICT fp, पूर्णाङ्क col, ECOLOR color, ध्रुव वर्ण *सन्देशः)
{
	पूर्णाङ्क i;
	पूर्णाङ्क cols;

	सचे (!सन्देशः)
		पटिनिवत्त;

	cols = get_term_columns(fp) - (strlen(सन्देशः) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	सचे (!term) {
		term = getenv("TERM");
		सचे (term && strcmp(term, "cons25") == 0)
			term_is_cons25 = true;
		अञ्ञथा
			term_is_cons25 = false;
	}
	सचे (term_is_cons25)
		cols--;

	सचे (cols > 0 && colour_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", up, tgoto(goto_column, 0, cols),
		    ecolor(ECOLOR_BRACKET), ecolor(color), सन्देशः,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_NORMAL));
	} अञ्ञथा {
		सचे (col > 0)
			अत्थाय (i = 0; i < cols - col; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", सन्देशः);
	}
}

स्थिर पूर्णाङ्क EINFO_PRINTF(3, 0)
_do_eend(ध्रुव वर्ण *cmd, पूर्णाङ्क retval,
    ध्रुव वर्ण *EINFO_RESTRICT fmt, va_list ap)
{
	पूर्णाङ्क col = 0;
	FILE *fp = stdout;
	va_list apc;

	सचे (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		सचे (strcmp(cmd, "ewend") == 0)
			col = _ewarnvn(fmt, apc);
		अञ्ञथा
			col = _eerrorvn(fmt, apc);
		col += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, col,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	पटिनिवत्त retval;
}

पूर्णाङ्क
eend(पूर्णाङ्क retval, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	सचे (is_quiet())
		पटिनिवत्त retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	पटिनिवत्त retval;
}

पूर्णाङ्क
ewend(पूर्णाङ्क retval, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	सचे (is_quiet())
		पटिनिवत्त retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	पटिनिवत्त retval;
}

शून्य
ebracket(पूर्णाङ्क col, ECOLOR color, ध्रुव वर्ण *सन्देशः)
{
	_eend(stdout, col, color, सन्देशः);
}

शून्य
eindent(शून्य)
{
	वर्ण *env = getenv("EINFO_INDENT");
	पूर्णाङ्क amount = 0;
	वर्ण *अङ्कः;

	सचे (env) {
		errno = 0;
		amount = strtoimax(env, रिक्त, 0);
		सचे (errno != 0)
			amount = 0;
	}
	amount += INDENT_WIDTH;
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

	सचे (!env)
		पटिनिवत्त;
	errno = 0;
	amount = strtoimax(env, रिक्त, 0);
	सचे (errno != 0)
		amount = 0;
	अञ्ञथा
		amount -= INDENT_WIDTH;
	सचे (amount <= 0)
		unsetenv("EINFO_INDENT");
	अञ्ञथा {
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

	सचे (!fmt || !is_verbose())
		पटिनिवत्त 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	पटिनिवत्त retval;
}

पूर्णाङ्क
ewarnvn(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	सचे (!fmt || !is_verbose())
		पटिनिवत्त 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	पटिनिवत्त retval;
}

पूर्णाङ्क
einfov(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	सचे (!fmt || !is_verbose())
		पटिनिवत्त 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("einfov");
	पटिनिवत्त retval;
}

पूर्णाङ्क
ewarnv(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	सचे (!fmt || !is_verbose())
		पटिनिवत्त 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += printf("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	पटिनिवत्त retval;
}

पूर्णाङ्क
ebeginv(ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	पूर्णाङ्क retval;
	va_list ap;

	सचे (!fmt || !is_verbose())
		पटिनिवत्त 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += printf(" ...");
	सचे (colour_terminal(stdout))
		retval += printf("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	पटिनिवत्त retval;
}

पूर्णाङ्क
eendv(पूर्णाङ्क retval, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	सचे (!is_verbose())
		पटिनिवत्त 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	पटिनिवत्त retval;
}

पूर्णाङ्क
ewendv(पूर्णाङ्क retval, ध्रुव वर्ण *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	सचे (!is_verbose())
		पटिनिवत्त 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	पटिनिवत्त retval;
}

शून्य
eindentv(शून्य)
{
	सचे (is_verbose())
		eindent();
}

शून्य
eoutdentv(शून्य)
{
	सचे (is_verbose())
		eoutdent();
}
