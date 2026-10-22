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
	صحيح value;
	بنية scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

ساكن TAILQ_HEAD(, scheduleitem) schedule;

فراغ free_schedulelist(فراغ)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	ܟܡܐ (s1) {
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

	ܐܢ (!sig || *sig == '\0')
		ܦܢܐ -1;

	ܐܢ (sscanf(sig, "%u", &i) == 1) {
		ܐܢ (i < NSIG)
			ܦܢܐ i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	ܐܢ (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	ܐܚܪܢܐ
		s = عدم;

	لكل (i = 0; i < ARRAY_SIZE(signallist); ++i)
		ܐܢ (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			ܦܢܐ signallist[i].إشارة;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

ساكن SCHEDULEITEM *parse_schedule_item(ثابت حرف *applet, ثابت حرف *string)
{
	ثابت حرف *after_hyph;
	صحيح sig;
	SCHEDULEITEM *item = xmalloc(حجم(*item));

	item->value = 0;
	item->gotoitem = عدم;
	ܐܢ (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	ܐܚܪܢܐ ܐܢ (isdigit((unsigned حرف)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		ܐܢ (sscanf(string, "%d", &item->value) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} ܐܚܪܢܐ ܐܢ ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->value = (صحيح)sig;
	} ܐܚܪܢܐ
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	ܦܢܐ item;
}

فراغ parse_schedule(ثابت حرف *applet, ثابت حرف *string, صحيح timeout)
{
	حرف buffer[20];
	ثابت حرف *slash;
	صحيح count = 0;
	SCHEDULEITEM *repeatat = عدم;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	ܐܢ (string)
		لكل (slash = string; *slash; slash++)
			ܐܢ (*slash == '/')
				count++;

	free_schedulelist();

	ܐܢ (count == 0) {
		item = xmalloc(حجم(*item));
		item->type = SC_SIGNAL;
		item->value = timeout;
		item->gotoitem = عدم;
		TAILQ_INSERT_TAIL(&schedule, item, entries);

		item = xmalloc(حجم(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = عدم;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		ܐܢ (string) {
			ܐܢ (sscanf(string, "%d", &item->value) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} ܐܚܪܢܐ
			item->value = 5;

		ܦܢܐ;
	}

	ܟܡܐ (string != عدم) {
		ܐܢ ((slash = strchr(string, '/')))
			len = slash - string;
		ܐܚܪܢܐ
			len = strlen(string);

		ܐܢ (len >= (ptrdiff_t)حجم(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : عدم;

		item = parse_schedule_item(applet, buffer);
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		ܐܢ (item->type == SC_FOREVER) {
			ܐܢ (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			استمر;
		}
	}

	ܐܢ (repeatat) {
		item = xmalloc(حجم(*item));
		item->type = SC_GOTO;
		item->value = 0;
		item->gotoitem = repeatat;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
	}

	ܦܢܐ;
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
	ܐܢ (group)
		kind = "GID";

	ܐܢ (pid > 0)
		pids = rc_find_pids(عدم, عدم, 0, pid);
	ܐܚܪܢܐ
		pids = rc_find_pids(exec, argv, uid, 0);

	ܐܢ (!pids)
		ܦܢܐ 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		ܐܢ (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} ܐܚܪܢܐ {
			ܐܢ (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				ܐܢ (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			ܐܢ (group)
				target = -pi->pid;
			ܐܚܪܢܐ
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			ܐܢ (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			ܐܚܪܢܐ ܐܢ (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			ܐܢ (!killed) {
				nkilled = -1;
			} ܐܚܪܢܐ {
				ܐܢ (nkilled != -1)
					nkilled++;
			}
		}
		حرر(pi);
	}

	حرر(pids);
	ܦܢܐ nkilled;
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

	ܐܢ (!(pid > 0 || exec || uid || (argv && *argv)))
		ܦܢܐ 0;

	ܐܢ (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	ܐܢ (pid > 0) {
		ܐܢ (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} ܐܚܪܢܐ {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	ܐܢ (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	ܐܢ (argv && *argv) {
		einfovn("Will stop processes of `");
		ܐܢ (rc_yesno(getenv("EINFO_VERBOSE"))) {
			لكل (p = argv; p && *p; p++) {
				ܐܢ (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	ܟܡܐ (item) {
		بدل (item->type) {
		حالة SC_GOTO:
			item = item->gotoitem;
			استمر;

		حالة SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->value, group,
					test, quiet);
			ܐܢ (nkilled == 0) {
				ܐܢ (tkilled == 0) {
					ܐܢ (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				ܦܢܐ tkilled;
			}
			ܐܚܪܢܐ ܐܢ (nkilled == -1)
				ܦܢܐ 0;

			tkilled += nkilled;
			اكسر;
		حالة SC_FOREVER:
		حالة SC_TIMEOUT:
			ܐܢ (item->type == SC_TIMEOUT && item->value < 1) {
				item = عدم;
				اكسر;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			لكل (nsecs = 0; item->type == SC_FOREVER || nsecs < item->value; nsecs++) {
				لكل (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					ܐܢ ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						ܦܢܐ 0;


					ܐܢ (nanosleep(&ts, عدم) == -1) {
						ܐܢ (progressed) {
							printf("\n");
							progressed = false;
						}
						ܐܢ (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							ܦܢܐ 0;
						}
					}
				}
				ܐܢ (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			اكسر;
		افتراضي:
			ܐܢ (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			ܦܢܐ 0;
		}

		ܐܢ (item)
			item = TAILQ_NEXT(item, entries);
	}

	ܐܢ (test || (tkilled > 0 && nrunning == 0))
		ܦܢܐ nkilled;

	ܐܢ (progressed)
		printf("\n");
	ܐܢ (!quiet) {
		ܐܢ (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		ܐܚܪܢܐ
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	ܦܢܐ -nrunning;
}
