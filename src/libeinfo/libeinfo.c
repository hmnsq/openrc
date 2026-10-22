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

#అన్తర్భావ <sys/ioctl.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <inttypes.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <syslog.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "helpers.h"

/* In case we cannot work out how many columns from ioctl, supply a default */
#నిర్వచించు DEFAULT_నిలువుల_సంఖ్య		 80

#నిర్వచించు OK			"ok"
#నిర్వచించు NOT_OK			"!!"

/* Number of spaces for an indent */
#నిర్వచించు INDENT_వెడల్పు		2

/* How wide can the indent go? */
#నిర్వచించు INDENT_MAX		40

/* Default colours */
#నిర్వచించు GOOD                    2
#నిర్వచించు WARN                    3
#నిర్వచించు BAD                     1
#నిర్వచించు HILITE                  6
#నిర్వచించు BRACKET                 4

/* ANSI escape codes which mimic termcap */
#నిర్వచించు AF "\033[3%dm"
#నిర్వచించు CE "\033[K"
#నిర్వచించు CH "\033[%dC"
#నిర్వచించు MD "\033[1m"
#నిర్వచించు ME "\033[m"
#నిర్వచించు పైకి "\033[A"

#నిర్వచించు _GET_CAP(_d, _c) strlcpy(_d, tgoto(_c, 0, 0), పరిమాణం(_d));
#నిర్వచించు కేటాయించు_CAP(_v) చేయి {						      \
		_v = p;							      \
		p += strlcpy(p, tmp, పరిమాణం(ebuffer) - (p - ebuffer)) + 1;    \
	} అయితే (0)

/* A pointer to a string to prefix to einfo/ewarn/eerror messages */
స్థిర స్థిరరాశి అక్షరం *_eprefix = ఖాళీ;

/* Buffers and structures to hold the final colours */
స్థిర అక్షరం ebuffer[100];
నిర్మాణం ecolor {
	ECOLOR color;
	పూర్ణాంకం def;
	స్థిరరాశి అక్షరం *name;
};
స్థిర అక్షరం nullstr = '\0';

స్థిర స్థిరరాశి నిర్మాణం ecolor ecolors[] = {
	{ ECOLOR_GOOD,    GOOD,    "good"    },
	{ ECOLOR_WARN,    WARN,    "warn"    },
	{ ECOLOR_BAD,     BAD,     "bad"     },
	{ ECOLOR_HILITE,  HILITE,  "hilite"  },
	{ ECOLOR_BRACKET, BRACKET, "bracket" },
	{ ECOLOR_సాధారణ,  0,       ఖాళీ      },
};
స్థిర స్థిరరాశి అక్షరం *ecolors_str[ARRAY_కొలత(ecolors)];

స్థిర అక్షరం *flush = ఖాళీ;
స్థిర అక్షరం *పైకి_2 = ఖాళీ;
స్థిర అక్షరం *goto_స్తంభం = ఖాళీ;

స్థిర స్థిరరాశి అక్షరం *పదం = ఖాళీ;
స్థిర bool పదం_is_cons25 = false;

/* Hardcoded list of colour capable terms
 * Only terminals without "color" in the name need to be explicitly listed */
స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి color_terms[] = {
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
	ఖాళీ
};

#నిర్వచితం_కాకపోతే HAVE_STRLCPY
స్థిర size_t
strlcat(అక్షరం *dst, స్థిరరాశి అక్షరం *src, size_t కొలత)
{
	అక్షరం *d = dst;
	స్థిరరాశి అక్షరం *s = src;
	size_t src_n = కొలత;
	size_t dst_n;

	అయితే (src_n-- != 0 && *d != '\0')
		d++;
	dst_n = d - dst;
	src_n = కొలత - dst_n;

	ఒకవేళ (src_n == 0)
		తిరిగి dst_n + అక్షరశ్రేణి_పొడవు(src);

	అయితే (*s != '\0') {
		ఒకవేళ (src_n != 1) {
			*d++ = *s;
			src_n--;
		}
		s++;
	}
	*d = '\0';

	తిరిగి dst_n + (s - src);
}
#అయితే_ముగింపు

స్థిర bool
yesno(స్థిరరాశి అక్షరం *విలువ)
{
	ఒకవేళ (!విలువ) {
		errno = ENOENT;
		తిరిగి false;
	}

	ఒకవేళ (strcasecmp(విలువ, "yes") == 0 ||
	    strcasecmp(విలువ, "y") == 0 ||
	    strcasecmp(విలువ, "true") == 0 ||
	    strcasecmp(విలువ, "on") == 0 ||
	    strcasecmp(విలువ, "1") == 0)
		తిరిగి true;

	ఒకవేళ (strcasecmp(విలువ, "no") != 0 &&
	    strcasecmp(విలువ, "n") != 0 &&
	    strcasecmp(విలువ, "false") != 0 &&
	    strcasecmp(విలువ, "off") != 0 &&
	    strcasecmp(విలువ, "0") != 0)
		errno = EINVAL;

	తిరిగి false;
}

స్థిర bool
noyes(స్థిరరాశి అక్షరం *విలువ)
{
	పూర్ణాంకం serrno = errno;
	bool retval;

	errno = 0;
	retval = yesno(విలువ);
	ఒకవేళ (errno == 0) {
		retval = !retval;
		errno = serrno;
	}

	తిరిగి retval;
}

స్థిర bool
is_quiet(శూన్య)
{
	తిరిగి yesno(getenv("EINFO_QUIET"));
}

స్థిర bool
is_really_quiet(శూన్య)
{
	తిరిగి yesno(getenv("EERROR_QUIET"));
}

స్థిర bool
is_verbose(శూన్య)
{
	తిరిగి yesno(getenv ("EINFO_VERBOSE"));
}

/* Fake tgoto call - very crapy, but works for our needs */
స్థిర అక్షరం *
tgoto(స్థిరరాశి అక్షరం *cap, పూర్ణాంకం నిలువు, పూర్ణాంకం పంక్తి)
{
	స్థిర అక్షరం buf[20];
	అక్షరం *p, *e, c, dbuf[6];
	పూర్ణాంకం oncol = 0, which = పంక్తి, i;

	p = buf;
	e = p + పరిమాణం(buf);
	అయితే ((c = *cap++)) {
		ఒకవేళ (c != '%' || ((c = *cap++) == '%')) {
			*p++ = c;
			ఒకవేళ (p >= e) {
				errno = E2BIG;
				తిరిగి ఖాళీ;
			}
			కొనసాగు;
		}
		ఎంపిక (c) {
		స్థితి '3':
		స్థితి '2':
		స్థితి 'd':
			i = 0;
			చేయి
				dbuf[i++] = which % 10 | '0';
			అయితే ((which /= 10));
			ఒకవేళ (c != 'd') {
				c -= '0';
				ఒకవేళ (i > c) {
					errno = EINVAL;
					తిరిగి ఖాళీ;
				}
				అయితే (i < c)
					dbuf[i++] = '0';
			}
			ఒకవేళ (p + i >= e) {
				errno = E2BIG;
				తిరిగి ఖాళీ;
			}
			చేయి
				*p++ = dbuf[--i];
			అయితే (i);
			ఆపు;
		స్థితి 'r':
			oncol = 0;
			ఆపు;
		స్థితి 'i':
			నిలువు++;
			పంక్తి++;
			which++;
			కొనసాగు;
		అప్రమేయ:
			errno = EINVAL;
			తిరిగి ఖాళీ;
		}

		oncol = 1 - oncol;
		which = oncol ? నిలువు : పంక్తి;
	}
	*p = '\0';
	తిరిగి buf;
}

స్థిర bool
రంగు_terminal(FILE * EINFO_RESTRICT f)
{
	స్థిర పూర్ణాంకం లోపల_రంగు = -1;
	అక్షరం *e, *ee, *చివర, *d, *p;
	పూర్ణాంకం c;
	స్థిరరాశి అక్షరం *_af = ఖాళీ, *_ce = ఖాళీ, *_ch = ఖాళీ;
	స్థిరరాశి అక్షరం *_md = ఖాళీ, *_me = ఖాళీ, *పైకి_3 = ఖాళీ;
	స్థిరరాశి అక్షరం *bold;
	అక్షరం tmp[100];
	గుర్తులేని పూర్ణాంకం i = 0;

	ఒకవేళ (f && !isatty(fileno(f)))
		తిరిగి false;

	ఒకవేళ (noyes(getenv("EINFO_COLOR")))
		తిరిగి false;

	ఒకవేళ (లోపల_రంగు == 0)
		తిరిగి false;
	ఒకవేళ (లోపల_రంగు == 1)
		తిరిగి true;

	పదం_is_cons25 = false;
	ఒకవేళ (!పదం) {
		పదం = getenv("TERM");
		ఒకవేళ (!పదం)
			తిరిగి false;
	}
	ఒకవేళ (strcmp(పదం, "cons25") == 0)
		పదం_is_cons25 = true;

	ఒకవేళ (అక్షరశ్రేణి_శోధన(పదం, "color"))
		లోపల_రంగు = 1;

	అయితే (color_terms[i] && లోపల_రంగు != 1) {
		ఒకవేళ (strcmp(color_terms[i], పదం) == 0) {
			లోపల_రంగు = 1;
		}
		i++;
	}

	ఒకవేళ (లోపల_రంగు != 1) {
		లోపల_రంగు = 0;
		తిరిగి false;
	}

	ఒకవేళ (!_af)
		_af = AF;
	ఒకవేళ (!_ce)
		_ce = CE;
	ఒకవేళ (!_ch)
		_ch = CH;
	ఒకవేళ (!_md)
		_md = MD;
	ఒకవేళ (!_me)
		_me = ME;
	ఒకవేళ (!పైకి_3)
		పైకి_3 = పైకి;

	/* Now setup our colours */
	p = ebuffer;
	కొరకు (i = 0; i < ARRAY_కొలత(ecolors); ++i) {
		tmp[0] = '\0';
		ఒకవేళ (ecolors[i].name) {
			bold = _md;
			c = ecolors[i].def;

			/* See if the user wants to override the colour
			 * We use a :col;bold: format like 2;1: for bold green
			 * and 1;0: for a normal red */
			ఒకవేళ ((e = getenv("EINFO_COLOR"))) {
				ee = అక్షరశ్రేణి_శోధన(e, ecolors[i].name);
				ఒకవేళ (ee)
					ee += అక్షరశ్రేణి_పొడవు(ecolors[i].name);

				ఒకవేళ (ee && *ee == '=') {
					d = strdup(ee + 1);
					ఒకవేళ (d) {
						చివర = strchr(d, ':');
						ఒకవేళ (చివర)
							*చివర = '\0';
						c = atoi(d);
						చివర = strchr(d, ';');
						ఒకవేళ (చివర && *++చివర == '0')
							bold = _me;
						విడుదల(d);
					}
				}
			}
			strlcpy(tmp, tgoto(bold, 0, 0), పరిమాణం(tmp));
			strlcat(tmp, tgoto(_af, 0, c & 0x07), పరిమాణం(tmp));
		} లేకపోతే
			_GET_CAP(tmp, _me);

		ఒకవేళ (tmp[0])
			కేటాయించు_CAP(ecolors_str[i]);
		లేకపోతే
			ecolors_str[i] = &nullstr;
	}

	_GET_CAP(tmp, _ce);
	కేటాయించు_CAP(flush);
	_GET_CAP(tmp, పైకి_3);
	కేటాయించు_CAP(పైకి_2);
	strlcpy(tmp, _ch, పరిమాణం(tmp));
	కేటాయించు_CAP(goto_స్తంభం);

	లోపల_రంగు = 1;
	తిరిగి true;
}

స్థిర పూర్ణాంకం
get_పదం_columns(FILE * EINFO_RESTRICT ప్రవాహం)
{
	నిర్మాణం winsize ws;
	అక్షరం *env = getenv("COLUMNS");
	అక్షరం *p;
	పూర్ణాంకం i;

	ఒకవేళ (env) {
		i = strtoimax(env, &p, 10);
		ఒకవేళ (!*p)
			తిరిగి i;
	}

	ఒకవేళ (ioctl(fileno(ప్రవాహం), TIOCGWINSZ, &ws) == 0)
		తిరిగి ws.ws_col;

	తిరిగి DEFAULT_నిలువుల_సంఖ్య;
}

శూన్య
eprefix(స్థిరరాశి అక్షరం *EINFO_RESTRICT ఉపసర్గ)
{
	_eprefix = ఉపసర్గ;
}

స్థిర శూన్య EINFO_PRINTF(2, 0)
elogv(పూర్ణాంకం level, స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, va_list ap)
{
	అక్షరం *e = getenv("EINFO_LOG");
	va_list apc;

	ఒకవేళ (fmt && e) {
		closelog();
		openlog(e, LOG_PID, LOG_DAEMON);
		va_copy(apc, ap);
		vsyslog(level, fmt, apc);
		va_end(apc);
		closelog();
	}
}

శూన్య
elog(పూర్ణాంకం level, స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	elogv(level, fmt, ap);
	va_end(ap);
}

స్థిర పూర్ణాంకం
_eindent(FILE * EINFO_RESTRICT ప్రవాహం)
{
	అక్షరం *env = getenv("EINFO_INDENT");
	పూర్ణాంకం amount = 0;

	ఒకవేళ (env) {
		errno = 0;
		amount = strtoimax(env, ఖాళీ, 0);
		ఒకవేళ (errno != 0 || amount < 0)
			amount = 0;
		లేకపోతే ఒకవేళ (amount > INDENT_MAX)
			amount = INDENT_MAX;
	}

	తిరిగి fprintf(ప్రవాహం, "%*s", amount, "");
}

స్థిర స్థిరరాశి అక్షరం *
_ecolor(FILE * EINFO_RESTRICT f, ECOLOR color)
{
	గుర్తులేని పూర్ణాంకం i;

	ఒకవేళ (!రంగు_terminal(f))
		తిరిగి "";

	కొరకు (i = 0; i < ARRAY_కొలత(ecolors); ++i)
		ఒకవేళ (ecolors[i].color == color)
			తిరిగి ecolors_str[i];
	తిరిగి "";
}

స్థిరరాశి అక్షరం *
ecolor(ECOLOR color)
{
	FILE *f = stdout;

	/* Try and guess a valid tty */
	ఒకవేళ (!isatty(fileno(f))) {
		f = stderr;
		ఒకవేళ (!isatty(fileno(f))) {
			f = stdin;
			ఒకవేళ (!isatty(fileno(f)))
				f = ఖాళీ;
		}
	}

	తిరిగి _ecolor(f, color);
}

#నిర్వచించు LASTCMD(_cmd) {							      \
		unsetenv("EINFO_LASTCMD");				      \
		setenv("EINFO_LASTCMD", _cmd, 1);			      \
	}

స్థిర పూర్ణాంకం EINFO_PRINTF(3, 0)
	_einfo(FILE *f, ECOLOR color, స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, va_list va)
{
	పూర్ణాంకం retval = 0;
	అక్షరం *చివరి = getenv("EINFO_LASTCMD");
	va_list ap;

	ఒకవేళ (చివరి &&
	    *చివరి &&
	    !రంగు_terminal(f) &&
	    strcmp(చివరి, "ewarn") != 0 &&
	    చివరి[అక్షరశ్రేణి_పొడవు(చివరి) - 1] == 'n')
		fprintf(f, "\n");
	ఒకవేళ (_eprefix)
		fprintf(f, "%s%s%s|", _ecolor(f, color), _eprefix, _ecolor(f, ECOLOR_సాధారణ));
	fprintf(f, " %s*%s ", _ecolor(f, color), _ecolor(f, ECOLOR_సాధారణ));
	retval += _eindent(f);
	va_copy(ap, va);
	retval += vfprintf(f, fmt, ap) + 3;
	va_end(ap); \
	ఒకవేళ (రంగు_terminal(f))
		fprintf(f, "%s", flush);
	తిరిగి retval;
}

#నిర్వచించు _einfovn(fmt, ap) _einfo(stdout, ECOLOR_GOOD, fmt, ap)
#నిర్వచించు _ewarnvn(fmt, ap) _einfo(stderr, ECOLOR_WARN, fmt, ap)
#నిర్వచించు _eerrorvn(fmt, ap) _einfo(stderr, ECOLOR_BAD, fmt, ap)

పూర్ణాంకం
einfon(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	పూర్ణాంకం retval;
	va_list ap;

	ఒకవేళ (!fmt || is_quiet())
		తిరిగి 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfon");
	తిరిగి retval;
}

పూర్ణాంకం
ewarnn(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	పూర్ణాంకం retval;
	va_list ap;

	ఒకవేళ (!fmt || is_quiet())
		తిరిగి 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnn");
	తిరిగి retval;
}

పూర్ణాంకం
eerrorn(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	పూర్ణాంకం retval;
	va_list ap;

	ఒకవేళ (!fmt || is_really_quiet())
		తిరిగి 0;
	va_start(ap, fmt);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	LASTCMD("errorn");
	తిరిగి retval;
}

పూర్ణాంకం
einfo(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	పూర్ణాంకం retval;
	va_list ap;

	ఒకవేళ (!fmt || is_quiet())
		తిరిగి 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += రాయి("\n");
	va_end(ap);
	LASTCMD("einfo");
	తిరిగి retval;
}

పూర్ణాంకం
ewarn(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	పూర్ణాంకం retval;
	va_list ap;

	ఒకవేళ (!fmt || is_quiet())
		తిరిగి 0;
	va_start(ap, fmt);
	elogv(LOG_WARNING, fmt, ap);
	retval = _ewarnvn(fmt, ap);
	retval += fprintf(stderr, "\n");
	va_end(ap);
	LASTCMD("ewarn");
	తిరిగి retval;
}

శూన్య
ewarnx(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ఒకవేళ (fmt && !is_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_WARNING, fmt, ap);
		_ewarnvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	నిష్క్రమించు(EXIT_FAILURE);
}

పూర్ణాంకం
eerror(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	పూర్ణాంకం retval;
	va_list ap;

	ఒకవేళ (!fmt || is_really_quiet())
		తిరిగి 0;
	va_start(ap, fmt);
	elogv(LOG_ERR, fmt, ap);
	retval = _eerrorvn(fmt, ap);
	va_end(ap);
	retval += fprintf(stderr, "\n");
	LASTCMD("eerror");
	తిరిగి retval;
}

శూన్య
eerrorx(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ఒకవేళ (fmt && !is_really_quiet()) {
		va_start(ap, fmt);
		elogv(LOG_ERR, fmt, ap);
		_eerrorvn(fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
	నిష్క్రమించు(EXIT_FAILURE);
}

పూర్ణాంకం
ebegin(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	పూర్ణాంకం retval;
	va_list ap;

	ఒకవేళ (!fmt || is_quiet())
		తిరిగి 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	retval += రాయి(" ...");
	ఒకవేళ (రంగు_terminal(stdout))
		retval += రాయి("\n");
	LASTCMD("ebegin");
	తిరిగి retval;
}

స్థిర శూన్య
_eend(FILE * EINFO_RESTRICT fp, పూర్ణాంకం నిలువు, ECOLOR color, స్థిరరాశి అక్షరం *సందేశం_2)
{
	పూర్ణాంకం i;
	పూర్ణాంకం నిలువుల_సంఖ్య;

	ఒకవేళ (!సందేశం_2)
		తిరిగి;

	నిలువుల_సంఖ్య = get_పదం_columns(fp) - (అక్షరశ్రేణి_పొడవు(సందేశం_2) + 5);

	/* cons25 is special - we need to remove one char, otherwise things
	 * do not align properly at all. */
	ఒకవేళ (!పదం) {
		పదం = getenv("TERM");
		ఒకవేళ (పదం && strcmp(పదం, "cons25") == 0)
			పదం_is_cons25 = true;
		లేకపోతే
			పదం_is_cons25 = false;
	}
	ఒకవేళ (పదం_is_cons25)
		నిలువుల_సంఖ్య--;

	ఒకవేళ (నిలువుల_సంఖ్య > 0 && రంగు_terminal(fp)) {
		fprintf(fp, "%s%s %s[%s %s %s]%s\n", పైకి_2, tgoto(goto_స్తంభం, 0, నిలువుల_సంఖ్య),
		    ecolor(ECOLOR_BRACKET), ecolor(color), సందేశం_2,
		    ecolor(ECOLOR_BRACKET), ecolor(ECOLOR_సాధారణ));
	} లేకపోతే {
		ఒకవేళ (నిలువు > 0)
			కొరకు (i = 0; i < నిలువుల_సంఖ్య - నిలువు; i++)
				fprintf(fp, " ");
		fprintf(fp, " [ %s ]\n", సందేశం_2);
	}
}

స్థిర పూర్ణాంకం EINFO_PRINTF(3, 0)
_do_eend(స్థిరరాశి అక్షరం *cmd, పూర్ణాంకం retval,
    స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, va_list ap)
{
	పూర్ణాంకం నిలువు = 0;
	FILE *fp = stdout;
	va_list apc;

	ఒకవేళ (fmt && *fmt != '\0' && retval != 0) {
		fp = stderr;
		va_copy(apc, ap);
		ఒకవేళ (strcmp(cmd, "ewend") == 0)
			నిలువు = _ewarnvn(fmt, apc);
		లేకపోతే
			నిలువు = _eerrorvn(fmt, apc);
		నిలువు += fprintf(fp, "\n");
		va_end(apc);
	}
	_eend(fp, నిలువు,
	    retval == 0 ? ECOLOR_GOOD : ECOLOR_BAD,
	    retval == 0 ? OK : NOT_OK);
	తిరిగి retval;
}

పూర్ణాంకం
eend(పూర్ణాంకం retval, స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ఒకవేళ (is_quiet())
		తిరిగి retval;
	va_start(ap, fmt);
	_do_eend("eend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eend");
	తిరిగి retval;
}

పూర్ణాంకం
ewend(పూర్ణాంకం retval, స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ఒకవేళ (is_quiet())
		తిరిగి retval;
	va_start(ap, fmt);
	_do_eend("ewend", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewend");
	తిరిగి retval;
}

శూన్య
ebracket(పూర్ణాంకం నిలువు, ECOLOR color, స్థిరరాశి అక్షరం *సందేశం_2)
{
	_eend(stdout, నిలువు, color, సందేశం_2);
}

శూన్య
eindent(శూన్య)
{
	అక్షరం *env = getenv("EINFO_INDENT");
	పూర్ణాంకం amount = 0;
	అక్షరం *అంకె;

	ఒకవేళ (env) {
		errno = 0;
		amount = strtoimax(env, ఖాళీ, 0);
		ఒకవేళ (errno != 0)
			amount = 0;
	}
	amount += INDENT_వెడల్పు;
	xasprintf(&అంకె, "%08d", amount);
	setenv("EINFO_INDENT", అంకె, 1);
	విడుదల(అంకె);
}

శూన్య eoutdent(శూన్య)
{
	అక్షరం *env = getenv("EINFO_INDENT");
	పూర్ణాంకం amount = 0;
	అక్షరం *అంకె = ఖాళీ;
	పూర్ణాంకం serrno = errno;

	ఒకవేళ (!env)
		తిరిగి;
	errno = 0;
	amount = strtoimax(env, ఖాళీ, 0);
	ఒకవేళ (errno != 0)
		amount = 0;
	లేకపోతే
		amount -= INDENT_వెడల్పు;
	ఒకవేళ (amount <= 0)
		unsetenv("EINFO_INDENT");
	లేకపోతే {
		xasprintf(&అంకె, "%08d", amount);
		setenv("EINFO_INDENT", అంకె, 1);
		విడుదల(అంకె);
	}
	errno = serrno;
}

పూర్ణాంకం
einfovn(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	పూర్ణాంకం retval;
	va_list ap;

	ఒకవేళ (!fmt || !is_verbose())
		తిరిగి 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	va_end(ap);
	LASTCMD("einfovn");
	తిరిగి retval;
}

పూర్ణాంకం
ewarnvn(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	పూర్ణాంకం retval;
	va_list ap;

	ఒకవేళ (!fmt || !is_verbose())
		తిరిగి 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	va_end(ap);
	LASTCMD("ewarnvn");
	తిరిగి retval;
}

పూర్ణాంకం
einfov(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	పూర్ణాంకం retval;
	va_list ap;

	ఒకవేళ (!fmt || !is_verbose())
		తిరిగి 0;
	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += రాయి("\n");
	va_end(ap);
	LASTCMD("einfov");
	తిరిగి retval;
}

పూర్ణాంకం
ewarnv(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	పూర్ణాంకం retval;
	va_list ap;

	ఒకవేళ (!fmt || !is_verbose())
		తిరిగి 0;
	va_start(ap, fmt);
	retval = _ewarnvn(fmt, ap);
	retval += రాయి("\n");
	va_end(ap);
	LASTCMD("ewarnv");
	తిరిగి retval;
}

పూర్ణాంకం
ebeginv(స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	పూర్ణాంకం retval;
	va_list ap;

	ఒకవేళ (!fmt || !is_verbose())
		తిరిగి 0;

	va_start(ap, fmt);
	retval = _einfovn(fmt, ap);
	retval += రాయి(" ...");
	ఒకవేళ (రంగు_terminal(stdout))
		retval += రాయి("\n");
	va_end(ap);
	LASTCMD("ebeginv");
	తిరిగి retval;
}

పూర్ణాంకం
eendv(పూర్ణాంకం retval, స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ఒకవేళ (!is_verbose())
		తిరిగి 0;
	va_start(ap, fmt);
	_do_eend("eendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("eendv");
	తిరిగి retval;
}

పూర్ణాంకం
ewendv(పూర్ణాంకం retval, స్థిరరాశి అక్షరం *EINFO_RESTRICT fmt, ...)
{
	va_list ap;

	ఒకవేళ (!is_verbose())
		తిరిగి 0;
	va_start(ap, fmt);
	_do_eend("ewendv", retval, fmt, ap);
	va_end(ap);
	LASTCMD("ewendv");
	తిరిగి retval;
}

శూన్య
eindentv(శూన్య)
{
	ఒకవేళ (is_verbose())
		eindent();
}

శూన్య
eoutdentv(శూన్య)
{
	ఒకవేళ (is_verbose())
		eoutdent();
}
