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
#تعریف_کړه POLL_INTERVAL   20000000
#تعریف_کړه WAIT_PIDFILE   500000000
#تعریف_کړه ONE_دوهم    1000000000
#تعریف_کړه ONE_MS           1000000

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
	صحيح ارزښت;
	بنية scheduleitem *gotoitem;
	TAILQ_ثبت(scheduleitem) entries;
} SCHEDULEITEM;

ساكن TAILQ_سر(, scheduleitem) مهالوېش;

فراغ free_schedulelist(فراغ)
{
	SCHEDULEITEM *s1 = TAILQ_لومړی(&مهالوېش);
	SCHEDULEITEM *s2;

	طالما (s1) {
		s2 = TAILQ_بل(s1, entries);
		حرر(s1);
		s1 = s2;
	}
	TAILQ_INIT(&مهالوېش);
}

صحيح parse_signal(ثابت حرف *applet, ثابت حرف *sig)
{
	تعريفنوع بنية signalpair
	{
		ثابت حرف *name;
		صحيح إشارة;
	} SIGNALPAIR;

#تعریف_کړه signalpair_توکی(name) { #name, SIG##name },

	ساكن ثابت SIGNALPAIR signallist[] = {
		signalpair_توکی(HUP)
		signalpair_توکی(INT)
		signalpair_توکی(QUIT)
		signalpair_توکی(ILL)
		signalpair_توکی(TRAP)
		signalpair_توکی(ABRT)
		signalpair_توکی(BUS)
		signalpair_توکی(FPE)
		signalpair_توکی(KILL)
		signalpair_توکی(USR1)
		signalpair_توکی(SEGV)
		signalpair_توکی(USR2)
		signalpair_توکی(PIPE)
		signalpair_توکی(ALRM)
		signalpair_توکی(غړی_2)
		signalpair_توکی(CHLD)
		signalpair_توکی(CONT)
		signalpair_توکی(STOP)
		signalpair_توکی(TSTP)
		signalpair_توکی(TTIN)
		signalpair_توکی(TTOU)
		signalpair_توکی(URG)
		signalpair_توکی(XCPU)
		signalpair_توکی(XFSZ)
		signalpair_توکی(VTALRM)
		signalpair_توکی(PROF)
#که_تعریف_شوی SIGWINCH
		signalpair_توکی(WINCH)
#د_که_پای
#که_تعریف_شوی SIGIO
		signalpair_توکی(IO)
#د_که_پای
#که_تعریف_شوی SIGPWR
		signalpair_توکی(PWR)
#د_که_پای
		signalpair_توکی(SYS)
		{ "NULL",	0 },
	};

	بینښه صحيح i = 0;
	ثابت حرف *s;

	إذا (!sig || *sig == '\0')
		أرجع -1;

	إذا (له_مزي_ولوله(sig, "%u", &i) == 1) {
		إذا (i < NSIG)
			أرجع i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	إذا (مزي_پرتله(sig, "SIG", 3) == 0)
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

ساكن SCHEDULEITEM *parse_مهالوېش_توکی(ثابت حرف *applet, ثابت حرف *string)
{
	ثابت حرف *after_hyph;
	صحيح sig;
	SCHEDULEITEM *توکی = xmalloc(حجم(*توکی));

	توکی->ارزښت = 0;
	توکی->gotoitem = عدم;
	إذا (strcmp(string,"forever") == 0)
		توکی->type = SC_FOREVER;
	وإلا إذا (که_عدد((بینښه حرف)string[0])) {
		توکی->type = SC_TIMEOUT;
		errno = 0;
		إذا (له_مزي_ولوله(string, "%d", &توکی->ارزښت) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} وإلا إذا ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		توکی->type = SC_SIGNAL;
		توکی->ارزښت = (صحيح)sig;
	} وإلا
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	أرجع توکی;
}

فراغ parse_مهالوېش(ثابت حرف *applet, ثابت حرف *string, صحيح timeout)
{
	حرف بفر[20];
	ثابت حرف *slash;
	صحيح شمېر = 0;
	SCHEDULEITEM *repeatat = عدم;
	size_t len;
	SCHEDULEITEM *توکی;

	TAILQ_INIT(&مهالوېش);
	إذا (string)
		لكل (slash = string; *slash; slash++)
			إذا (*slash == '/')
				شمېر++;

	free_schedulelist();

	إذا (شمېر == 0) {
		توکی = xmalloc(حجم(*توکی));
		توکی->type = SC_SIGNAL;
		توکی->ارزښت = timeout;
		توکی->gotoitem = عدم;
		TAILQ_ورننباسه_لکۍ(&مهالوېش, توکی, entries);

		توکی = xmalloc(حجم(*توکی));
		توکی->type = SC_TIMEOUT;
		توکی->gotoitem = عدم;
		TAILQ_ورننباسه_لکۍ(&مهالوېش, توکی, entries);
		إذا (string) {
			إذا (له_مزي_ولوله(string, "%d", &توکی->ارزښت) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} وإلا
			توکی->ارزښت = 5;

		أرجع;
	}

	طالما (string != عدم) {
		إذا ((slash = strchr(string, '/')))
			len = slash - string;
		وإلا
			len = مزي_اوږدوالی(string);

		إذا (len >= (ptrdiff_t)حجم(بفر))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		حافظه_نقل(بفر, string, len);
		بفر[len] = 0;
		string = slash ? slash + 1 : عدم;

		توکی = parse_مهالوېش_توکی(applet, بفر);
		TAILQ_ورننباسه_لکۍ(&مهالوېش, توکی, entries);
		إذا (توکی->type == SC_FOREVER) {
			إذا (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = توکی;
			استمر;
		}
	}

	إذا (repeatat) {
		توکی = xmalloc(حجم(*توکی));
		توکی->type = SC_GOTO;
		توکی->ارزښت = 0;
		توکی->gotoitem = repeatat;
		TAILQ_ورننباسه_لکۍ(&مهالوېش, توکی, entries);
	}

	أرجع;
}

/* return number of processes killed, -1 on error */
صحيح do_stop(ثابت حرف *applet, ثابت حرف *exec, ثابت حرف *ثابت *argv,
    pid_t pid, uid_t uid, صحيح sig, bool group, bool test, bool quiet)
{
	ثابت حرف *ډول = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	صحيح nkilled = 0;
	uid_t موخه;
	إذا (group)
		ډول = "GID";

	إذا (pid > 0)
		pids = rc_پیدا_کړه_pids(عدم, عدم, 0, pid);
	وإلا
		pids = rc_پیدا_کړه_pids(exec, argv, uid, 0);

	إذا (!pids)
		أرجع 0;

	LIST_FOREACH_خوندي(pi, pids, entries, np) {
		إذا (test) {
			einfo("Would send signal %d to %s %d", sig, ډول, pi->pid);
			nkilled++;
		} وإلا {
			إذا (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, ډول, pi->pid);
				إذا (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, ډول, pi->pid);
			}
			errno = 0;
			إذا (group)
				موخه = -pi->pid;
			وإلا
				موخه = pi->pid;
			killed = (kill(موخه, sig) == 0 ||
			    errno == ESRCH ? true : false);
			إذا (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, ډول, pi->pid, strerror(errno));
			وإلا إذا (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, ډول, pi->pid, strerror(errno));
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

صحيح پرلپسې_stop_مهالوېش(ثابت حرف *applet,
		ثابت حرف *exec, ثابت حرف *ثابت *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool پرمختګ, bool quiet)
{
	SCHEDULEITEM *توکی = TAILQ_لومړی(&مهالوېش);
	صحيح nkilled = 0;
	صحيح tkilled = 0;
	صحيح nrunning = 0;
	اوږد nloops, nsecs;
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
					چاپ_کړه(" ");
				چاپ_کړه("%s", *p);
			}
			چاپ_کړه("'\n");
		}
	}

	طالما (توکی) {
		بدل (توکی->type) {
		حالة SC_GOTO:
			توکی = توکی->gotoitem;
			استمر;

		حالة SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, توکی->ارزښت, group,
					test, quiet);
			إذا (nkilled == 0) {
				إذا (tkilled == 0) {
					إذا (progressed)
						چاپ_کړه("\n");
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
			إذا (توکی->type == SC_TIMEOUT && توکی->ارزښت < 1) {
				توکی = عدم;
				اكسر;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			لكل (nsecs = 0; توکی->type == SC_FOREVER || nsecs < توکی->ارزښت; nsecs++) {
				لكل (nloops = 0;
				     nloops < ONE_دوهم / POLL_INTERVAL;
				     nloops++)
				{
					إذا ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						أرجع 0;


					إذا (nanosleep(&ts, عدم) == -1) {
						إذا (progressed) {
							چاپ_کړه("\n");
							progressed = false;
						}
						إذا (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							أرجع 0;
						}
					}
				}
				إذا (پرمختګ) {
					چاپ_کړه(".");
					fflush(stdout);
					progressed = true;
				}
			}
			اكسر;
		افتراضي:
			إذا (progressed) {
				چاپ_کړه("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, توکی->type);
			أرجع 0;
		}

		إذا (توکی)
			توکی = TAILQ_بل(توکی, entries);
	}

	إذا (test || (tkilled > 0 && nrunning == 0))
		أرجع nkilled;

	إذا (progressed)
		چاپ_کړه("\n");
	إذا (!quiet) {
		إذا (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		وإلا
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	أرجع -nrunning;
}
