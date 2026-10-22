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
#وصف_ڪريو POLL_INTERVAL   20000000
#وصف_ڪريو WAIT_PIDFILE   500000000
#وصف_ڪريو ONE_ٻيو    1000000000
#وصف_ڪريو ONE_MS           1000000

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
	صحيح قدر;
	بنية scheduleitem *gotoitem;
	TAILQ_داخلا(scheduleitem) entries;
} SCHEDULEITEM;

ساكن TAILQ_سر(, scheduleitem) وقت_جدول;

فراغ free_schedulelist(فراغ)
{
	SCHEDULEITEM *s1 = TAILQ_پهريون(&وقت_جدول);
	SCHEDULEITEM *s2;

	طالما (s1) {
		s2 = TAILQ_ايندڙ(s1, entries);
		حرر(s1);
		s1 = s2;
	}
	TAILQ_INIT(&وقت_جدول);
}

صحيح parse_signal(ثابت حرف *applet, ثابت حرف *sig)
{
	تعريفنوع بنية signalpair
	{
		ثابت حرف *name;
		صحيح إشارة;
	} SIGNALPAIR;

#وصف_ڪريو signalpair_شي(name) { #name, SIG##name },

	ساكن ثابت SIGNALPAIR signallist[] = {
		signalpair_شي(HUP)
		signalpair_شي(INT)
		signalpair_شي(QUIT)
		signalpair_شي(ILL)
		signalpair_شي(TRAP)
		signalpair_شي(ABRT)
		signalpair_شي(BUS)
		signalpair_شي(FPE)
		signalpair_شي(KILL)
		signalpair_شي(USR1)
		signalpair_شي(SEGV)
		signalpair_شي(USR2)
		signalpair_شي(PIPE)
		signalpair_شي(ALRM)
		signalpair_شي(رڪن_2)
		signalpair_شي(CHLD)
		signalpair_شي(CONT)
		signalpair_شي(STOP)
		signalpair_شي(TSTP)
		signalpair_شي(TTIN)
		signalpair_شي(TTOU)
		signalpair_شي(URG)
		signalpair_شي(XCPU)
		signalpair_شي(XFSZ)
		signalpair_شي(VTALRM)
		signalpair_شي(PROF)
#جيڪڏهن_وصف_ٿيل SIGWINCH
		signalpair_شي(WINCH)
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل SIGIO
		signalpair_شي(IO)
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل SIGPWR
		signalpair_شي(PWR)
#جيڪڏهن_پورو
		signalpair_شي(SYS)
		{ "NULL",	0 },
	};

	بنانشان صحيح i = 0;
	ثابت حرف *s;

	إذا (!sig || *sig == '\0')
		أرجع -1;

	إذا (لڙي_مان_پڙهو(sig, "%u", &i) == 1) {
		إذا (i < NSIG)
			أرجع i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	إذا (لڙي_ڀيٽ(sig, "SIG", 3) == 0)
		s = sig + 3;
	وإلا
		s = عدم;

	لكل (i = 0; i < ARRAY_سائيز(signallist); ++i)
		إذا (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			أرجع signallist[i].إشارة;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

ساكن SCHEDULEITEM *parse_وقت_جدول_شي(ثابت حرف *applet, ثابت حرف *string)
{
	ثابت حرف *after_hyph;
	صحيح sig;
	SCHEDULEITEM *شي = xmalloc(حجم(*شي));

	شي->قدر = 0;
	شي->gotoitem = عدم;
	إذا (strcmp(string,"forever") == 0)
		شي->type = SC_FOREVER;
	وإلا إذا (انگ_آهي((بنانشان حرف)string[0])) {
		شي->type = SC_TIMEOUT;
		errno = 0;
		إذا (لڙي_مان_پڙهو(string, "%d", &شي->قدر) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} وإلا إذا ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		شي->type = SC_SIGNAL;
		شي->قدر = (صحيح)sig;
	} وإلا
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	أرجع شي;
}

فراغ parse_وقت_جدول(ثابت حرف *applet, ثابت حرف *string, صحيح timeout)
{
	حرف بفر[20];
	ثابت حرف *slash;
	صحيح ڳڻپ = 0;
	SCHEDULEITEM *repeatat = عدم;
	size_t len;
	SCHEDULEITEM *شي;

	TAILQ_INIT(&وقت_جدول);
	إذا (string)
		لكل (slash = string; *slash; slash++)
			إذا (*slash == '/')
				ڳڻپ++;

	free_schedulelist();

	إذا (ڳڻپ == 0) {
		شي = xmalloc(حجم(*شي));
		شي->type = SC_SIGNAL;
		شي->قدر = timeout;
		شي->gotoitem = عدم;
		TAILQ_وجهو_پڇ(&وقت_جدول, شي, entries);

		شي = xmalloc(حجم(*شي));
		شي->type = SC_TIMEOUT;
		شي->gotoitem = عدم;
		TAILQ_وجهو_پڇ(&وقت_جدول, شي, entries);
		إذا (string) {
			إذا (لڙي_مان_پڙهو(string, "%d", &شي->قدر) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} وإلا
			شي->قدر = 5;

		أرجع;
	}

	طالما (string != عدم) {
		إذا ((slash = strchr(string, '/')))
			len = slash - string;
		وإلا
			len = لڙي_ڊيگهه(string);

		إذا (len >= (ptrdiff_t)حجم(بفر))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		يادگيري_نقل(بفر, string, len);
		بفر[len] = 0;
		string = slash ? slash + 1 : عدم;

		شي = parse_وقت_جدول_شي(applet, بفر);
		TAILQ_وجهو_پڇ(&وقت_جدول, شي, entries);
		إذا (شي->type == SC_FOREVER) {
			إذا (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = شي;
			استمر;
		}
	}

	إذا (repeatat) {
		شي = xmalloc(حجم(*شي));
		شي->type = SC_GOTO;
		شي->قدر = 0;
		شي->gotoitem = repeatat;
		TAILQ_وجهو_پڇ(&وقت_جدول, شي, entries);
	}

	أرجع;
}

/* return number of processes killed, -1 on error */
صحيح do_stop(ثابت حرف *applet, ثابت حرف *exec, ثابت حرف *ثابت *argv,
    pid_t pid, uid_t uid, صحيح sig, bool group, bool test, bool quiet)
{
	ثابت حرف *قسم = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	صحيح nkilled = 0;
	uid_t هدف;
	إذا (group)
		قسم = "GID";

	إذا (pid > 0)
		pids = rc_ڳوليو_pids(عدم, عدم, 0, pid);
	وإلا
		pids = rc_ڳوليو_pids(exec, argv, uid, 0);

	إذا (!pids)
		أرجع 0;

	LIST_FOREACH_محفوظ(pi, pids, entries, np) {
		إذا (test) {
			einfo("Would send signal %d to %s %d", sig, قسم, pi->pid);
			nkilled++;
		} وإلا {
			إذا (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, قسم, pi->pid);
				إذا (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, قسم, pi->pid);
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
				applet, sig, قسم, pi->pid, strerror(errno));
			وإلا إذا (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, قسم, pi->pid, strerror(errno));
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

صحيح لاڳيتو_stop_وقت_جدول(ثابت حرف *applet,
		ثابت حرف *exec, ثابت حرف *ثابت *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool ترقي, bool quiet)
{
	SCHEDULEITEM *شي = TAILQ_پهريون(&وقت_جدول);
	صحيح nkilled = 0;
	صحيح tkilled = 0;
	صحيح nrunning = 0;
	ڊگهو nloops, nsecs;
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
					لکو(" ");
				لکو("%s", *p);
			}
			لکو("'\n");
		}
	}

	طالما (شي) {
		بدل (شي->type) {
		حالة SC_GOTO:
			شي = شي->gotoitem;
			استمر;

		حالة SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, شي->قدر, group,
					test, quiet);
			إذا (nkilled == 0) {
				إذا (tkilled == 0) {
					إذا (progressed)
						لکو("\n");
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
			إذا (شي->type == SC_TIMEOUT && شي->قدر < 1) {
				شي = عدم;
				اكسر;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			لكل (nsecs = 0; شي->type == SC_FOREVER || nsecs < شي->قدر; nsecs++) {
				لكل (nloops = 0;
				     nloops < ONE_ٻيو / POLL_INTERVAL;
				     nloops++)
				{
					إذا ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						أرجع 0;


					إذا (nanosleep(&ts, عدم) == -1) {
						إذا (progressed) {
							لکو("\n");
							progressed = false;
						}
						إذا (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							أرجع 0;
						}
					}
				}
				إذا (ترقي) {
					لکو(".");
					fflush(stdout);
					progressed = true;
				}
			}
			اكسر;
		افتراضي:
			إذا (progressed) {
				لکو("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, شي->type);
			أرجع 0;
		}

		إذا (شي)
			شي = TAILQ_ايندڙ(شي, entries);
	}

	إذا (test || (tkilled > 0 && nrunning == 0))
		أرجع nkilled;

	إذا (progressed)
		لکو("\n");
	إذا (!quiet) {
		إذا (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		وإلا
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	أرجع -nrunning;
}
