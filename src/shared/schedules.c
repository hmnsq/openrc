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
#تعریف_کن POLL_INTERVAL   20000000
#تعریف_کن WAIT_PIDFILE   500000000
#تعریف_کن ONE_دومی    1000000000
#تعریف_کن ONE_MS           1000000

#ضمن <ctype.h>
#ضمن <errno.h>
#ضمن <signal.h>
#ضمن <stddef.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <syslog.h>
#ضمن <time.h>
#ضمن <sys/types.h>

#ضمن "einfo.h"
#ضمن "queue.h"
#ضمن "rc.h"
#ضمن "schedules.h"
#ضمن "helpers.h"

تعريفنوع بنية scheduleitem {
	تعداد {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	صحيح مقدار;
	بنية scheduleitem *gotoitem;
	TAILQ_مدخل(scheduleitem) entries;
} SCHEDULEITEM;

ساكن TAILQ_سر(, scheduleitem) زمانبندی;

فراغ free_schedulelist(فراغ)
{
	SCHEDULEITEM *s1 = TAILQ_اولی(&زمانبندی);
	SCHEDULEITEM *s2;

	طالما (s1) {
		s2 = TAILQ_بعدی(s1, entries);
		حرر(s1);
		s1 = s2;
	}
	TAILQ_INIT(&زمانبندی);
}

صحيح parse_signal(ثابت حرف *applet, ثابت حرف *sig)
{
	تعريفنوع بنية signalpair
	{
		ثابت حرف *name;
		صحيح إشارة;
	} SIGNALPAIR;

#تعریف_کن signalpair_قلم(name) { #name, SIG##name },

	ساكن ثابت SIGNALPAIR signallist[] = {
		signalpair_قلم(HUP)
		signalpair_قلم(INT)
		signalpair_قلم(QUIT)
		signalpair_قلم(ILL)
		signalpair_قلم(TRAP)
		signalpair_قلم(ABRT)
		signalpair_قلم(BUS)
		signalpair_قلم(FPE)
		signalpair_قلم(KILL)
		signalpair_قلم(USR1)
		signalpair_قلم(SEGV)
		signalpair_قلم(USR2)
		signalpair_قلم(PIPE)
		signalpair_قلم(ALRM)
		signalpair_قلم(جمله_2)
		signalpair_قلم(CHLD)
		signalpair_قلم(CONT)
		signalpair_قلم(STOP)
		signalpair_قلم(TSTP)
		signalpair_قلم(TTIN)
		signalpair_قلم(TTOU)
		signalpair_قلم(URG)
		signalpair_قلم(XCPU)
		signalpair_قلم(XFSZ)
		signalpair_قلم(VTALRM)
		signalpair_قلم(PROF)
#اگر_تعریف_شده SIGWINCH
		signalpair_قلم(WINCH)
#پایان_اگر
#اگر_تعریف_شده SIGIO
		signalpair_قلم(IO)
#پایان_اگر
#اگر_تعریف_شده SIGPWR
		signalpair_قلم(PWR)
#پایان_اگر
		signalpair_قلم(SYS)
		{ "NULL",	0 },
	};

	بیعلامت صحيح i = 0;
	ثابت حرف *s;

	إذا (!sig || *sig == '\0')
		أرجع -1;

	إذا (از_رشته_بخوان(sig, "%u", &i) == 1) {
		إذا (i < NSIG)
			أرجع i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	إذا (مقایسه_رشته(sig, "SIG", 3) == 0)
		s = sig + 3;
	وإلا
		s = عدم;

	لكل (i = 0; i < ARRAY_اندازه(signallist); ++i)
		إذا (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			أرجع signallist[i].إشارة;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

ساكن SCHEDULEITEM *parse_زمانبندی_قلم(ثابت حرف *applet, ثابت حرف *string)
{
	ثابت حرف *after_hyph;
	صحيح sig;
	SCHEDULEITEM *قلم = xmalloc(حجم(*قلم));

	قلم->مقدار = 0;
	قلم->gotoitem = عدم;
	إذا (strcmp(string,"forever") == 0)
		قلم->type = SC_FOREVER;
	وإلا إذا (آیا_رقم((بیعلامت حرف)string[0])) {
		قلم->type = SC_TIMEOUT;
		errno = 0;
		إذا (از_رشته_بخوان(string, "%d", &قلم->مقدار) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} وإلا إذا ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		قلم->type = SC_SIGNAL;
		قلم->مقدار = (صحيح)sig;
	} وإلا
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	أرجع قلم;
}

فراغ parse_زمانبندی(ثابت حرف *applet, ثابت حرف *string, صحيح timeout)
{
	حرف میانگیر[20];
	ثابت حرف *slash;
	صحيح شمار = 0;
	SCHEDULEITEM *repeatat = عدم;
	size_t len;
	SCHEDULEITEM *قلم;

	TAILQ_INIT(&زمانبندی);
	إذا (string)
		لكل (slash = string; *slash; slash++)
			إذا (*slash == '/')
				شمار++;

	free_schedulelist();

	إذا (شمار == 0) {
		قلم = xmalloc(حجم(*قلم));
		قلم->type = SC_SIGNAL;
		قلم->مقدار = timeout;
		قلم->gotoitem = عدم;
		TAILQ_درج_دم(&زمانبندی, قلم, entries);

		قلم = xmalloc(حجم(*قلم));
		قلم->type = SC_TIMEOUT;
		قلم->gotoitem = عدم;
		TAILQ_درج_دم(&زمانبندی, قلم, entries);
		إذا (string) {
			إذا (از_رشته_بخوان(string, "%d", &قلم->مقدار) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} وإلا
			قلم->مقدار = 5;

		أرجع;
	}

	طالما (string != عدم) {
		إذا ((slash = strchr(string, '/')))
			len = slash - string;
		وإلا
			len = طول_رشته(string);

		إذا (len >= (ptrdiff_t)حجم(میانگیر))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		کپی_حافظه(میانگیر, string, len);
		میانگیر[len] = 0;
		string = slash ? slash + 1 : عدم;

		قلم = parse_زمانبندی_قلم(applet, میانگیر);
		TAILQ_درج_دم(&زمانبندی, قلم, entries);
		إذا (قلم->type == SC_FOREVER) {
			إذا (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = قلم;
			استمر;
		}
	}

	إذا (repeatat) {
		قلم = xmalloc(حجم(*قلم));
		قلم->type = SC_GOTO;
		قلم->مقدار = 0;
		قلم->gotoitem = repeatat;
		TAILQ_درج_دم(&زمانبندی, قلم, entries);
	}

	أرجع;
}

/* return number of processes killed, -1 on error */
صحيح do_stop(ثابت حرف *applet, ثابت حرف *exec, ثابت حرف *ثابت *argv,
    pid_t pid, uid_t uid, صحيح sig, bool group, bool test, bool quiet)
{
	ثابت حرف *نوع = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	صحيح nkilled = 0;
	uid_t هدف;
	إذا (group)
		نوع = "GID";

	إذا (pid > 0)
		pids = rc_پیدا_کن_pids(عدم, عدم, 0, pid);
	وإلا
		pids = rc_پیدا_کن_pids(exec, argv, uid, 0);

	إذا (!pids)
		أرجع 0;

	LIST_FOREACH_امن(pi, pids, entries, np) {
		إذا (test) {
			einfo("Would send signal %d to %s %d", sig, نوع, pi->pid);
			nkilled++;
		} وإلا {
			إذا (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, نوع, pi->pid);
				إذا (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, نوع, pi->pid);
			}
			errno = 0;
			إذا (group)
				هدف = -pi->pid;
			وإلا
				هدف = pi->pid;
			killed = (kill(هدف, sig) == 0 ||
			    errno == ESRCH ? true : false);
			إذا (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, نوع, pi->pid, strerror(errno));
			وإلا إذا (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, نوع, pi->pid, strerror(errno));
			إذا (!killed) {
				nkilled = -1;
			} وإلا {
				إذا (nkilled != -1)
					nkilled++;
			}
		}
		حرر(pi);
	}

	حرر(pids);
	أرجع nkilled;
}

صحيح پیاپی_stop_زمانبندی(ثابت حرف *applet,
		ثابت حرف *exec, ثابت حرف *ثابت *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool پیشرفت, bool quiet)
{
	SCHEDULEITEM *قلم = TAILQ_اولی(&زمانبندی);
	صحيح nkilled = 0;
	صحيح tkilled = 0;
	صحيح nrunning = 0;
	دراز nloops, nsecs;
	بنية timespec ts;
	ثابت حرف *ثابت *p;
	bool progressed = false;

	إذا (!(pid > 0 || exec || uid || (argv && *argv)))
		أرجع 0;

	إذا (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	إذا (pid > 0) {
		إذا (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} وإلا {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	إذا (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	إذا (argv && *argv) {
		einfovn("Will stop processes of `");
		إذا (rc_yesno(getenv("EINFO_VERBOSE"))) {
			لكل (p = argv; p && *p; p++) {
				إذا (p != argv)
					چاپ_کن(" ");
				چاپ_کن("%s", *p);
			}
			چاپ_کن("'\n");
		}
	}

	طالما (قلم) {
		بدل (قلم->type) {
		حالة SC_GOTO:
			قلم = قلم->gotoitem;
			استمر;

		حالة SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, قلم->مقدار, group,
					test, quiet);
			إذا (nkilled == 0) {
				إذا (tkilled == 0) {
					إذا (progressed)
						چاپ_کن("\n");
					eerror("%s: no matching processes found", applet);
				}
				أرجع tkilled;
			}
			وإلا إذا (nkilled == -1)
				أرجع 0;

			tkilled += nkilled;
			اكسر;
		حالة SC_FOREVER:
		حالة SC_TIMEOUT:
			إذا (قلم->type == SC_TIMEOUT && قلم->مقدار < 1) {
				قلم = عدم;
				اكسر;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			لكل (nsecs = 0; قلم->type == SC_FOREVER || nsecs < قلم->مقدار; nsecs++) {
				لكل (nloops = 0;
				     nloops < ONE_دومی / POLL_INTERVAL;
				     nloops++)
				{
					إذا ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						أرجع 0;


					إذا (nanosleep(&ts, عدم) == -1) {
						إذا (progressed) {
							چاپ_کن("\n");
							progressed = false;
						}
						إذا (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							أرجع 0;
						}
					}
				}
				إذا (پیشرفت) {
					چاپ_کن(".");
					fflush(stdout);
					progressed = true;
				}
			}
			اكسر;
		افتراضي:
			إذا (progressed) {
				چاپ_کن("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, قلم->type);
			أرجع 0;
		}

		إذا (قلم)
			قلم = TAILQ_بعدی(قلم, entries);
	}

	إذا (test || (tkilled > 0 && nrunning == 0))
		أرجع nkilled;

	إذا (progressed)
		چاپ_کن("\n");
	إذا (!quiet) {
		إذا (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		وإلا
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	أرجع -nrunning;
}
