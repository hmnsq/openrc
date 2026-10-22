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
#تعریف_کرو ONE_دوجا    1000000000
#تعریف_کرو ONE_MS           1000000

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
	TAILQ_اندراج(scheduleitem) entries;
} SCHEDULEITEM;

ساكن TAILQ_سر(, scheduleitem) ویلا_سارنی;

فراغ free_schedulelist(فراغ)
{
	SCHEDULEITEM *s1 = TAILQ_پہلا(&ویلا_سارنی);
	SCHEDULEITEM *s2;

	طالما (s1) {
		s2 = TAILQ_اگلا(s1, entries);
		حرر(s1);
		s1 = s2;
	}
	TAILQ_INIT(&ویلا_سارنی);
}

صحيح parse_signal(ثابت حرف *applet, ثابت حرف *sig)
{
	تعريفنوع بنية signalpair
	{
		ثابت حرف *name;
		صحيح إشارة;
	} SIGNALPAIR;

#تعریف_کرو signalpair_شے(name) { #name, SIG##name },

	ساكن ثابت SIGNALPAIR signallist[] = {
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
#جے_تعریف_شدہ SIGWINCH
		signalpair_شے(WINCH)
#جے_انت
#جے_تعریف_شدہ SIGIO
		signalpair_شے(IO)
#جے_انت
#جے_تعریف_شدہ SIGPWR
		signalpair_شے(PWR)
#جے_انت
		signalpair_شے(SYS)
		{ "NULL",	0 },
	};

	بنانشانی صحيح i = 0;
	ثابت حرف *s;

	إذا (!sig || *sig == '\0')
		أرجع -1;

	إذا (لڑی_توں_پڑھو(sig, "%u", &i) == 1) {
		إذا (i < NSIG)
			أرجع i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	إذا (لڑی_مقابلہ(sig, "SIG", 3) == 0)
		s = sig + 3;
	وإلا
		s = عدم;

	لكل (i = 0; i < ARRAY_ناپ(signallist); ++i)
		إذا (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			أرجع signallist[i].إشارة;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

ساكن SCHEDULEITEM *parse_ویلا_سارنی_شے(ثابت حرف *applet, ثابت حرف *string)
{
	ثابت حرف *after_hyph;
	صحيح sig;
	SCHEDULEITEM *شے = xmalloc(حجم(*شے));

	شے->قدر = 0;
	شے->gotoitem = عدم;
	إذا (strcmp(string,"forever") == 0)
		شے->type = SC_FOREVER;
	وإلا إذا (ہندسہ_اے((بنانشانی حرف)string[0])) {
		شے->type = SC_TIMEOUT;
		errno = 0;
		إذا (لڑی_توں_پڑھو(string, "%d", &شے->قدر) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} وإلا إذا ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		شے->type = SC_SIGNAL;
		شے->قدر = (صحيح)sig;
	} وإلا
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	أرجع شے;
}

فراغ parse_ویلا_سارنی(ثابت حرف *applet, ثابت حرف *string, صحيح timeout)
{
	حرف بفر[20];
	ثابت حرف *slash;
	صحيح گنتی = 0;
	SCHEDULEITEM *repeatat = عدم;
	size_t len;
	SCHEDULEITEM *شے;

	TAILQ_INIT(&ویلا_سارنی);
	إذا (string)
		لكل (slash = string; *slash; slash++)
			إذا (*slash == '/')
				گنتی++;

	free_schedulelist();

	إذا (گنتی == 0) {
		شے = xmalloc(حجم(*شے));
		شے->type = SC_SIGNAL;
		شے->قدر = timeout;
		شے->gotoitem = عدم;
		TAILQ_داخل_کرو_پوچھ(&ویلا_سارنی, شے, entries);

		شے = xmalloc(حجم(*شے));
		شے->type = SC_TIMEOUT;
		شے->gotoitem = عدم;
		TAILQ_داخل_کرو_پوچھ(&ویلا_سارنی, شے, entries);
		إذا (string) {
			إذا (لڑی_توں_پڑھو(string, "%d", &شے->قدر) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} وإلا
			شے->قدر = 5;

		أرجع;
	}

	طالما (string != عدم) {
		إذا ((slash = strchr(string, '/')))
			len = slash - string;
		وإلا
			len = لڑی_لمبائی(string);

		إذا (len >= (ptrdiff_t)حجم(بفر))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		حافظہ_نقل(بفر, string, len);
		بفر[len] = 0;
		string = slash ? slash + 1 : عدم;

		شے = parse_ویلا_سارنی_شے(applet, بفر);
		TAILQ_داخل_کرو_پوچھ(&ویلا_سارنی, شے, entries);
		إذا (شے->type == SC_FOREVER) {
			إذا (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = شے;
			استمر;
		}
	}

	إذا (repeatat) {
		شے = xmalloc(حجم(*شے));
		شے->type = SC_GOTO;
		شے->قدر = 0;
		شے->gotoitem = repeatat;
		TAILQ_داخل_کرو_پوچھ(&ویلا_سارنی, شے, entries);
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
	uid_t نشانہ;
	إذا (group)
		قسم = "GID";

	إذا (pid > 0)
		pids = rc_لبھو_pids(عدم, عدم, 0, pid);
	وإلا
		pids = rc_لبھو_pids(exec, argv, uid, 0);

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
				نشانہ = -pi->pid;
			وإلا
				نشانہ = pi->pid;
			killed = (kill(نشانہ, sig) == 0 ||
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

صحيح لگاتار_stop_ویلا_سارنی(ثابت حرف *applet,
		ثابت حرف *exec, ثابت حرف *ثابت *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool ترقی, bool quiet)
{
	SCHEDULEITEM *شے = TAILQ_پہلا(&ویلا_سارنی);
	صحيح nkilled = 0;
	صحيح tkilled = 0;
	صحيح nrunning = 0;
	لما nloops, nsecs;
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
					لکھو(" ");
				لکھو("%s", *p);
			}
			لکھو("'\n");
		}
	}

	طالما (شے) {
		بدل (شے->type) {
		حالة SC_GOTO:
			شے = شے->gotoitem;
			استمر;

		حالة SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, شے->قدر, group,
					test, quiet);
			إذا (nkilled == 0) {
				إذا (tkilled == 0) {
					إذا (progressed)
						لکھو("\n");
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
			إذا (شے->type == SC_TIMEOUT && شے->قدر < 1) {
				شے = عدم;
				اكسر;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			لكل (nsecs = 0; شے->type == SC_FOREVER || nsecs < شے->قدر; nsecs++) {
				لكل (nloops = 0;
				     nloops < ONE_دوجا / POLL_INTERVAL;
				     nloops++)
				{
					إذا ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						أرجع 0;


					إذا (nanosleep(&ts, عدم) == -1) {
						إذا (progressed) {
							لکھو("\n");
							progressed = false;
						}
						إذا (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							أرجع 0;
						}
					}
				}
				إذا (ترقی) {
					لکھو(".");
					fflush(stdout);
					progressed = true;
				}
			}
			اكسر;
		افتراضي:
			إذا (progressed) {
				لکھو("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, شے->type);
			أرجع 0;
		}

		إذا (شے)
			شے = TAILQ_اگلا(شے, entries);
	}

	إذا (test || (tkilled > 0 && nrunning == 0))
		أرجع nkilled;

	إذا (progressed)
		لکھو("\n");
	إذا (!quiet) {
		إذا (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		وإلا
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	أرجع -nrunning;
}
