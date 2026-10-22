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

تعریفنوع ساختار scheduleitem {
	شمارش {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	صحیح مقدار;
	ساختار scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

ایستا TAILQ_HEAD(, scheduleitem) schedule;

تهی free_schedulelist(تهی)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	وقتی (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		آزاد(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

صحیح parse_signal(ثابت نویسه *applet, ثابت نویسه *sig)
{
	تعریفنوع ساختار signalpair
	{
		ثابت نویسه *name;
		صحیح سیگنال;
	} SIGNALPAIR;

#define signalpair_item(name) { #name, SIG##name },

	ایستا ثابت SIGNALPAIR signallist[] = {
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

	unsigned صحیح i = 0;
	ثابت نویسه *s;

	اگر (!sig || *sig == '\0')
		بازگشت -1;

	اگر (sscanf(sig, "%u", &i) == 1) {
		اگر (i < NSIG)
			بازگشت i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	اگر (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	وگرنه
		s = پوچ;

	برای (i = 0; i < ARRAY_SIZE(signallist); ++i)
		اگر (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			بازگشت signallist[i].سیگنال;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

ایستا SCHEDULEITEM *parse_schedule_item(ثابت نویسه *applet, ثابت نویسه *string)
{
	ثابت نویسه *after_hyph;
	صحیح sig;
	SCHEDULEITEM *item = xmalloc(اندازه(*item));

	item->مقدار = 0;
	item->gotoitem = پوچ;
	اگر (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	وگرنه اگر (isdigit((unsigned نویسه)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		اگر (sscanf(string, "%d", &item->مقدار) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} وگرنه اگر ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->مقدار = (صحیح)sig;
	} وگرنه
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	بازگشت item;
}

تهی parse_schedule(ثابت نویسه *applet, ثابت نویسه *string, صحیح timeout)
{
	نویسه buffer[20];
	ثابت نویسه *slash;
	صحیح شمار = 0;
	SCHEDULEITEM *repeatat = پوچ;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	اگر (string)
		برای (slash = string; *slash; slash++)
			اگر (*slash == '/')
				شمار++;

	free_schedulelist();

	اگر (شمار == 0) {
		item = xmalloc(اندازه(*item));
		item->type = SC_SIGNAL;
		item->مقدار = timeout;
		item->gotoitem = پوچ;
		TAILQ_درج_TAIL(&schedule, item, entries);

		item = xmalloc(اندازه(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = پوچ;
		TAILQ_درج_TAIL(&schedule, item, entries);
		اگر (string) {
			اگر (sscanf(string, "%d", &item->مقدار) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} وگرنه
			item->مقدار = 5;

		بازگشت;
	}

	وقتی (string != پوچ) {
		اگر ((slash = strchr(string, '/')))
			len = slash - string;
		وگرنه
			len = strlen(string);

		اگر (len >= (ptrdiff_t)اندازه(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : پوچ;

		item = parse_schedule_item(applet, buffer);
		TAILQ_درج_TAIL(&schedule, item, entries);
		اگر (item->type == SC_FOREVER) {
			اگر (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			ادامه;
		}
	}

	اگر (repeatat) {
		item = xmalloc(اندازه(*item));
		item->type = SC_GOTO;
		item->مقدار = 0;
		item->gotoitem = repeatat;
		TAILQ_درج_TAIL(&schedule, item, entries);
	}

	بازگشت;
}

/* return number of processes killed, -1 on error */
صحیح do_stop(ثابت نویسه *applet, ثابت نویسه *exec, ثابت نویسه *ثابت *argv,
    pid_t pid, uid_t uid, صحیح sig, bool group, bool test, bool quiet)
{
	ثابت نویسه *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	صحیح nkilled = 0;
	uid_t target;
	اگر (group)
		kind = "GID";

	اگر (pid > 0)
		pids = rc_find_pids(پوچ, پوچ, 0, pid);
	وگرنه
		pids = rc_find_pids(exec, argv, uid, 0);

	اگر (!pids)
		بازگشت 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		اگر (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} وگرنه {
			اگر (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				اگر (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			اگر (group)
				target = -pi->pid;
			وگرنه
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			اگر (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			وگرنه اگر (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			اگر (!killed) {
				nkilled = -1;
			} وگرنه {
				اگر (nkilled != -1)
					nkilled++;
			}
		}
		آزاد(pi);
	}

	آزاد(pids);
	بازگشت nkilled;
}

صحیح run_stop_schedule(ثابت نویسه *applet,
		ثابت نویسه *exec, ثابت نویسه *ثابت *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	صحیح nkilled = 0;
	صحیح tkilled = 0;
	صحیح nrunning = 0;
	long nloops, nsecs;
	ساختار timespec ts;
	ثابت نویسه *ثابت *p;
	bool progressed = false;

	اگر (!(pid > 0 || exec || uid || (argv && *argv)))
		بازگشت 0;

	اگر (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	اگر (pid > 0) {
		اگر (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} وگرنه {
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
			برای (p = argv; p && *p; p++) {
				اگر (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	وقتی (item) {
		انتخاب (item->type) {
		حالت SC_GOTO:
			item = item->gotoitem;
			ادامه;

		حالت SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->مقدار, group,
					test, quiet);
			اگر (nkilled == 0) {
				اگر (tkilled == 0) {
					اگر (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				بازگشت tkilled;
			}
			وگرنه اگر (nkilled == -1)
				بازگشت 0;

			tkilled += nkilled;
			بشکن;
		حالت SC_FOREVER:
		حالت SC_TIMEOUT:
			اگر (item->type == SC_TIMEOUT && item->مقدار < 1) {
				item = پوچ;
				بشکن;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			برای (nsecs = 0; item->type == SC_FOREVER || nsecs < item->مقدار; nsecs++) {
				برای (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					اگر ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						بازگشت 0;


					اگر (nanosleep(&ts, پوچ) == -1) {
						اگر (progressed) {
							printf("\n");
							progressed = false;
						}
						اگر (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							بازگشت 0;
						}
					}
				}
				اگر (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			بشکن;
		پیشفرض:
			اگر (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			بازگشت 0;
		}

		اگر (item)
			item = TAILQ_NEXT(item, entries);
	}

	اگر (test || (tkilled > 0 && nrunning == 0))
		بازگشت nkilled;

	اگر (progressed)
		printf("\n");
	اگر (!quiet) {
		اگر (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		وگرنه
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	بازگشت -nrunning;
}
