/*
 * The functions in this file control the stopping of daemons by
 * start-stop-daemon and supervise-daemon.
 */

/*
 * Copyright (c) 2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

/* nano seconds */
#تعریف_کرو POLL_INTERVAL   20000000
#تعریف_کرو WAIT_PIDFILE   500000000
#تعریف_کرو ONE_دوسرا    1000000000
#تعریف_کرو ONE_MS           1000000

#شامل <ctype.h>
#شامل <errno.h>
#شامل <signal.h>
#شامل <stddef.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <syslog.h>
#شامل <time.h>
#شامل <sys/types.h>

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "schedules.h"
#شامل "helpers.h"

قسمتعریف ساخت scheduleitem {
	شمار {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	صحیح قدر;
	ساخت scheduleitem *gotoitem;
	TAILQ_اندراج(scheduleitem) entries;
} SCHEDULEITEM;

ساکن TAILQ_سر(, scheduleitem) جدول_وقت;

خالی free_schedulelist(خالی)
{
	SCHEDULEITEM *s1 = TAILQ_پہلا(&جدول_وقت);
	SCHEDULEITEM *s2;

	جب (s1) {
		s2 = TAILQ_اگلا(s1, entries);
		آزاد(s1);
		s1 = s2;
	}
	TAILQ_INIT(&جدول_وقت);
}

صحیح parse_signal(ثابت حرف *applet, ثابت حرف *sig)
{
	قسمتعریف ساخت signalpair
	{
		ثابت حرف *name;
		صحیح اشارہ;
	} SIGNALPAIR;

#تعریف_کرو signalpair_شے(name) { #name, SIG##name },

	ساکن ثابت SIGNALPAIR signallist[] = {
		signalpair_شے(HUP)
		signalpair_شے(INT)
		signalpair_شے(QUIT)
		signalpair_شے(ILL)
		signalpair_شے(TRAP)
		signalpair_شے(ABRT)
		signalpair_شے(BUS)
		signalpair_شے(FPE)
		signalpair_شے(KILL)
		signalpair_شے(USR1)
		signalpair_شے(SEGV)
		signalpair_شے(USR2)
		signalpair_شے(PIPE)
		signalpair_شے(ALRM)
		signalpair_شے(رکن_2)
		signalpair_شے(CHLD)
		signalpair_شے(CONT)
		signalpair_شے(STOP)
		signalpair_شے(TSTP)
		signalpair_شے(TTIN)
		signalpair_شے(TTOU)
		signalpair_شے(URG)
		signalpair_شے(XCPU)
		signalpair_شے(XFSZ)
		signalpair_شے(VTALRM)
		signalpair_شے(PROF)
#اگر_تعریف_شدہ SIGWINCH
		signalpair_شے(WINCH)
#اگر_ختم
#اگر_تعریف_شدہ SIGIO
		signalpair_شے(IO)
#اگر_ختم
#اگر_تعریف_شدہ SIGPWR
		signalpair_شے(PWR)
#اگر_ختم
		signalpair_شے(SYS)
		{ "NULL",	0 },
	};

	غیرعلامتی صحیح i = 0;
	ثابت حرف *s;

	اگر (!sig || *sig == '\0')
		واپس -1;

	اگر (لڑی_سے_پڑھو(sig, "%u", &i) == 1) {
		اگر (i < NSIG)
			واپس i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	اگر (لڑی_موازنہ(sig, "SIG", 3) == 0)
		s = sig + 3;
	ورنہ
		s = تہی;

	برائے (i = 0; i < ARRAY_سائز(signallist); ++i)
		اگر (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			واپس signallist[i].اشارہ;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

ساکن SCHEDULEITEM *parse_جدول_وقت_شے(ثابت حرف *applet, ثابت حرف *string)
{
	ثابت حرف *after_hyph;
	صحیح sig;
	SCHEDULEITEM *شے = xmalloc(حجم(*شے));

	شے->قدر = 0;
	شے->gotoitem = تہی;
	اگر (strcmp(string,"forever") == 0)
		شے->type = SC_FOREVER;
	ورنہ اگر (ہندسہ_ہے((غیرعلامتی حرف)string[0])) {
		شے->type = SC_TIMEOUT;
		errno = 0;
		اگر (لڑی_سے_پڑھو(string, "%d", &شے->قدر) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} ورنہ اگر ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		شے->type = SC_SIGNAL;
		شے->قدر = (صحیح)sig;
	} ورنہ
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	واپس شے;
}

خالی parse_جدول_وقت(ثابت حرف *applet, ثابت حرف *string, صحیح timeout)
{
	حرف بفر[20];
	ثابت حرف *slash;
	صحیح گنتی = 0;
	SCHEDULEITEM *repeatat = تہی;
	size_t len;
	SCHEDULEITEM *شے;

	TAILQ_INIT(&جدول_وقت);
	اگر (string)
		برائے (slash = string; *slash; slash++)
			اگر (*slash == '/')
				گنتی++;

	free_schedulelist();

	اگر (گنتی == 0) {
		شے = xmalloc(حجم(*شے));
		شے->type = SC_SIGNAL;
		شے->قدر = timeout;
		شے->gotoitem = تہی;
		TAILQ_داخل_کرو_دم(&جدول_وقت, شے, entries);

		شے = xmalloc(حجم(*شے));
		شے->type = SC_TIMEOUT;
		شے->gotoitem = تہی;
		TAILQ_داخل_کرو_دم(&جدول_وقت, شے, entries);
		اگر (string) {
			اگر (لڑی_سے_پڑھو(string, "%d", &شے->قدر) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} ورنہ
			شے->قدر = 5;

		واپس;
	}

	جب (string != تہی) {
		اگر ((slash = strchr(string, '/')))
			len = slash - string;
		ورنہ
			len = لڑی_لمبائی(string);

		اگر (len >= (ptrdiff_t)حجم(بفر))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		حافظہ_نقل(بفر, string, len);
		بفر[len] = 0;
		string = slash ? slash + 1 : تہی;

		شے = parse_جدول_وقت_شے(applet, بفر);
		TAILQ_داخل_کرو_دم(&جدول_وقت, شے, entries);
		اگر (شے->type == SC_FOREVER) {
			اگر (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = شے;
			جاری;
		}
	}

	اگر (repeatat) {
		شے = xmalloc(حجم(*شے));
		شے->type = SC_GOTO;
		شے->قدر = 0;
		شے->gotoitem = repeatat;
		TAILQ_داخل_کرو_دم(&جدول_وقت, شے, entries);
	}

	واپس;
}

/* return number of processes killed, -1 on error */
صحیح do_stop(ثابت حرف *applet, ثابت حرف *exec, ثابت حرف *ثابت *argv,
    pid_t pid, uid_t uid, صحیح sig, bool group, bool test, bool quiet)
{
	ثابت حرف *قسم = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	صحیح nkilled = 0;
	uid_t ہدف;
	اگر (group)
		قسم = "GID";

	اگر (pid > 0)
		pids = rc_ڈھونڈو_pids(تہی, تہی, 0, pid);
	ورنہ
		pids = rc_ڈھونڈو_pids(exec, argv, uid, 0);

	اگر (!pids)
		واپس 0;

	LIST_FOREACH_محفوظ(pi, pids, entries, np) {
		اگر (test) {
			einfo("Would send signal %d to %s %d", sig, قسم, pi->pid);
			nkilled++;
		} ورنہ {
			اگر (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, قسم, pi->pid);
				اگر (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, قسم, pi->pid);
			}
			errno = 0;
			اگر (group)
				ہدف = -pi->pid;
			ورنہ
				ہدف = pi->pid;
			killed = (kill(ہدف, sig) == 0 ||
			    errno == ESRCH ? true : false);
			اگر (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, قسم, pi->pid, strerror(errno));
			ورنہ اگر (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, قسم, pi->pid, strerror(errno));
			اگر (!killed) {
				nkilled = -1;
			} ورنہ {
				اگر (nkilled != -1)
					nkilled++;
			}
		}
		آزاد(pi);
	}

	آزاد(pids);
	واپس nkilled;
}

صحیح تسلسل_stop_جدول_وقت(ثابت حرف *applet,
		ثابت حرف *exec, ثابت حرف *ثابت *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool پیش_رفت, bool quiet)
{
	SCHEDULEITEM *شے = TAILQ_پہلا(&جدول_وقت);
	صحیح nkilled = 0;
	صحیح tkilled = 0;
	صحیح nrunning = 0;
	طویل nloops, nsecs;
	ساخت timespec ts;
	ثابت حرف *ثابت *p;
	bool progressed = false;

	اگر (!(pid > 0 || exec || uid || (argv && *argv)))
		واپس 0;

	اگر (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	اگر (pid > 0) {
		اگر (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} ورنہ {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	اگر (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	اگر (argv && *argv) {
		einfovn("Will stop processes of `");
		اگر (rc_yesno(getenv("EINFO_VERBOSE"))) {
			برائے (p = argv; p && *p; p++) {
				اگر (p != argv)
					لکھو(" ");
				لکھو("%s", *p);
			}
			لکھو("'\n");
		}
	}

	جب (شے) {
		انتخاب (شے->type) {
		حالت SC_GOTO:
			شے = شے->gotoitem;
			جاری;

		حالت SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, شے->قدر, group,
					test, quiet);
			اگر (nkilled == 0) {
				اگر (tkilled == 0) {
					اگر (progressed)
						لکھو("\n");
					eerror("%s: no matching processes found", applet);
				}
				واپس tkilled;
			}
			ورنہ اگر (nkilled == -1)
				واپس 0;

			tkilled += nkilled;
			توڑو;
		حالت SC_FOREVER:
		حالت SC_TIMEOUT:
			اگر (شے->type == SC_TIMEOUT && شے->قدر < 1) {
				شے = تہی;
				توڑو;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			برائے (nsecs = 0; شے->type == SC_FOREVER || nsecs < شے->قدر; nsecs++) {
				برائے (nloops = 0;
				     nloops < ONE_دوسرا / POLL_INTERVAL;
				     nloops++)
				{
					اگر ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						واپس 0;


					اگر (nanosleep(&ts, تہی) == -1) {
						اگر (progressed) {
							لکھو("\n");
							progressed = false;
						}
						اگر (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							واپس 0;
						}
					}
				}
				اگر (پیش_رفت) {
					لکھو(".");
					fflush(stdout);
					progressed = true;
				}
			}
			توڑو;
		طےشدہ:
			اگر (progressed) {
				لکھو("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, شے->type);
			واپس 0;
		}

		اگر (شے)
			شے = TAILQ_اگلا(شے, entries);
	}

	اگر (test || (tkilled > 0 && nrunning == 0))
		واپس nkilled;

	اگر (progressed)
		لکھو("\n");
	اگر (!quiet) {
		اگر (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		ورنہ
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	واپس -nrunning;
}
