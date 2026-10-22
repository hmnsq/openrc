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
#define POLL_INTERVAL   20000000
#define WAIT_PIDFILE   500000000
#define ONE_SECOND    1000000000
#define ONE_MS           1000000

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
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

ساكن TAILQ_HEAD(, scheduleitem) schedule;

فراغ free_schedulelist(فراغ)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	طالما (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		حرر(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

صحيح parse_signal(ثابت حرف *applet, ثابت حرف *sig)
{
	تعريفنوع بنية signalpair
	{
		ثابت حرف *name;
		صحيح إشارة;
	} SIGNALPAIR;

#define signalpair_item(name) { #name, SIG##name },

	ساكن ثابت SIGNALPAIR signallist[] = {
		signalpair_item(HUP)
		signalpair_item(INT)
		signalpair_item(QUIT)
		signalpair_item(ILL)
		signalpair_item(TRAP)
		signalpair_item(ABRT)
		signalpair_item(BUS)
		signalpair_item(FPE)
		signalpair_item(KILL)
		signalpair_item(USR1)
		signalpair_item(SEGV)
		signalpair_item(USR2)
		signalpair_item(PIPE)
		signalpair_item(ALRM)
		signalpair_item(TERM)
		signalpair_item(CHLD)
		signalpair_item(CONT)
		signalpair_item(STOP)
		signalpair_item(TSTP)
		signalpair_item(TTIN)
		signalpair_item(TTOU)
		signalpair_item(URG)
		signalpair_item(XCPU)
		signalpair_item(XFSZ)
		signalpair_item(VTALRM)
		signalpair_item(PROF)
#ifdef SIGWINCH
		signalpair_item(WINCH)
#endif
#ifdef SIGIO
		signalpair_item(IO)
#endif
#ifdef SIGPWR
		signalpair_item(PWR)
#endif
		signalpair_item(SYS)
		{ "NULL",	0 },
	};

	unsigned صحيح i = 0;
	ثابت حرف *s;

	إذا (!sig || *sig == '\0')
		أرجع -1;

	إذا (sscanf(sig, "%u", &i) == 1) {
		إذا (i < NSIG)
			أرجع i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	إذا (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	وإلا
		s = عدم;

	لكل (i = 0; i < ARRAY_SIZE(signallist); ++i)
		إذا (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			أرجع signallist[i].إشارة;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

ساكن SCHEDULEITEM *parse_schedule_item(ثابت حرف *applet, ثابت حرف *string)
{
	ثابت حرف *after_hyph;
	صحيح sig;
	SCHEDULEITEM *item = xmalloc(حجم(*item));

	item->قدر = 0;
	item->gotoitem = عدم;
	إذا (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	وإلا إذا (isdigit((unsigned حرف)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		إذا (sscanf(string, "%d", &item->قدر) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} وإلا إذا ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->قدر = (صحيح)sig;
	} وإلا
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	أرجع item;
}

فراغ parse_schedule(ثابت حرف *applet, ثابت حرف *string, صحيح timeout)
{
	حرف buffer[20];
	ثابت حرف *slash;
	صحيح گنتی = 0;
	SCHEDULEITEM *repeatat = عدم;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	إذا (string)
		لكل (slash = string; *slash; slash++)
			إذا (*slash == '/')
				گنتی++;

	free_schedulelist();

	إذا (گنتی == 0) {
		item = xmalloc(حجم(*item));
		item->type = SC_SIGNAL;
		item->قدر = timeout;
		item->gotoitem = عدم;
		TAILQ_داخل_کرو_TAIL(&schedule, item, entries);

		item = xmalloc(حجم(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = عدم;
		TAILQ_داخل_کرو_TAIL(&schedule, item, entries);
		إذا (string) {
			إذا (sscanf(string, "%d", &item->قدر) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} وإلا
			item->قدر = 5;

		أرجع;
	}

	طالما (string != عدم) {
		إذا ((slash = strchr(string, '/')))
			len = slash - string;
		وإلا
			len = strlen(string);

		إذا (len >= (ptrdiff_t)حجم(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : عدم;

		item = parse_schedule_item(applet, buffer);
		TAILQ_داخل_کرو_TAIL(&schedule, item, entries);
		إذا (item->type == SC_FOREVER) {
			إذا (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			استمر;
		}
	}

	إذا (repeatat) {
		item = xmalloc(حجم(*item));
		item->type = SC_GOTO;
		item->قدر = 0;
		item->gotoitem = repeatat;
		TAILQ_داخل_کرو_TAIL(&schedule, item, entries);
	}

	أرجع;
}

/* return number of processes killed, -1 on error */
صحيح do_stop(ثابت حرف *applet, ثابت حرف *exec, ثابت حرف *ثابت *argv,
    pid_t pid, uid_t uid, صحيح sig, bool group, bool test, bool quiet)
{
	ثابت حرف *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	صحيح nkilled = 0;
	uid_t target;
	إذا (group)
		kind = "GID";

	إذا (pid > 0)
		pids = rc_find_pids(عدم, عدم, 0, pid);
	وإلا
		pids = rc_find_pids(exec, argv, uid, 0);

	إذا (!pids)
		أرجع 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		إذا (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} وإلا {
			إذا (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				إذا (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			إذا (group)
				target = -pi->pid;
			وإلا
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			إذا (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			وإلا إذا (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
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

صحيح run_stop_schedule(ثابت حرف *applet,
		ثابت حرف *exec, ثابت حرف *ثابت *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	صحيح nkilled = 0;
	صحيح tkilled = 0;
	صحيح nrunning = 0;
	long nloops, nsecs;
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
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	طالما (item) {
		بدل (item->type) {
		حالة SC_GOTO:
			item = item->gotoitem;
			استمر;

		حالة SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->قدر, group,
					test, quiet);
			إذا (nkilled == 0) {
				إذا (tkilled == 0) {
					إذا (progressed)
						printf("\n");
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
			إذا (item->type == SC_TIMEOUT && item->قدر < 1) {
				item = عدم;
				اكسر;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			لكل (nsecs = 0; item->type == SC_FOREVER || nsecs < item->قدر; nsecs++) {
				لكل (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					إذا ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						أرجع 0;


					إذا (nanosleep(&ts, عدم) == -1) {
						إذا (progressed) {
							printf("\n");
							progressed = false;
						}
						إذا (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							أرجع 0;
						}
					}
				}
				إذا (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			اكسر;
		افتراضي:
			إذا (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			أرجع 0;
		}

		إذا (item)
			item = TAILQ_NEXT(item, entries);
	}

	إذا (test || (tkilled > 0 && nrunning == 0))
		أرجع nkilled;

	إذا (progressed)
		printf("\n");
	إذا (!quiet) {
		إذا (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		وإلا
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	أرجع -nrunning;
}
