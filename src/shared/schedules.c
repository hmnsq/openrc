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

#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <syslog.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <sys/types.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "schedules.h"
#অন্তর্ভাব "helpers.h"

প্রকারলেখ কাঠামো scheduleitem {
	গণনা {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	পূর্ণসংখ্যা মান;
	কাঠামো scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

স্থির TAILQ_শীর্ষ(, scheduleitem) schedule;

শূন্য free_schedulelist(শূন্য)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	যখন (s1) {
		s2 = TAILQ_পরবর্তী(s1, entries);
		মুক্ত(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

পূর্ণসংখ্যা parse_signal(ধ্রুবক অক্ষর *applet, ধ্রুবক অক্ষর *sig)
{
	প্রকারলেখ কাঠামো signalpair
	{
		ধ্রুবক অক্ষর *name;
		পূর্ণসংখ্যা সংকেত;
	} SIGNALPAIR;

#define signalpair_item(name) { #name, SIG##name },

	স্থির ধ্রুবক SIGNALPAIR signallist[] = {
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

	unsigned পূর্ণসংখ্যা i = 0;
	ধ্রুবক অক্ষর *s;

	যদি (!sig || *sig == '\0')
		ফেরত -1;

	যদি (sscanf(sig, "%u", &i) == 1) {
		যদি (i < NSIG)
			ফেরত i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	যদি (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	অন্যথা
		s = ফাঁকা;

	জন্য (i = 0; i < ARRAY_আকার(signallist); ++i)
		যদি (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			ফেরত signallist[i].সংকেত;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

স্থির SCHEDULEITEM *parse_schedule_item(ধ্রুবক অক্ষর *applet, ধ্রুবক অক্ষর *string)
{
	ধ্রুবক অক্ষর *after_hyph;
	পূর্ণসংখ্যা sig;
	SCHEDULEITEM *item = xmalloc(আকার(*item));

	item->মান = 0;
	item->gotoitem = ফাঁকা;
	যদি (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	অন্যথা যদি (isdigit((unsigned অক্ষর)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		যদি (sscanf(string, "%d", &item->মান) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} অন্যথা যদি ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->মান = (পূর্ণসংখ্যা)sig;
	} অন্যথা
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	ফেরত item;
}

শূন্য parse_schedule(ধ্রুবক অক্ষর *applet, ধ্রুবক অক্ষর *string, পূর্ণসংখ্যা timeout)
{
	অক্ষর বাফার[20];
	ধ্রুবক অক্ষর *slash;
	পূর্ণসংখ্যা গুনতি = 0;
	SCHEDULEITEM *repeatat = ফাঁকা;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	যদি (string)
		জন্য (slash = string; *slash; slash++)
			যদি (*slash == '/')
				গুনতি++;

	free_schedulelist();

	যদি (গুনতি == 0) {
		item = xmalloc(আকার(*item));
		item->type = SC_SIGNAL;
		item->মান = timeout;
		item->gotoitem = ফাঁকা;
		TAILQ_ঢোকাও_লেজ(&schedule, item, entries);

		item = xmalloc(আকার(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = ফাঁকা;
		TAILQ_ঢোকাও_লেজ(&schedule, item, entries);
		যদি (string) {
			যদি (sscanf(string, "%d", &item->মান) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} অন্যথা
			item->মান = 5;

		ফেরত;
	}

	যখন (string != ফাঁকা) {
		যদি ((slash = strchr(string, '/')))
			len = slash - string;
		অন্যথা
			len = strlen(string);

		যদি (len >= (ptrdiff_t)আকার(বাফার))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(বাফার, string, len);
		বাফার[len] = 0;
		string = slash ? slash + 1 : ফাঁকা;

		item = parse_schedule_item(applet, বাফার);
		TAILQ_ঢোকাও_লেজ(&schedule, item, entries);
		যদি (item->type == SC_FOREVER) {
			যদি (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			চালু;
		}
	}

	যদি (repeatat) {
		item = xmalloc(আকার(*item));
		item->type = SC_GOTO;
		item->মান = 0;
		item->gotoitem = repeatat;
		TAILQ_ঢোকাও_লেজ(&schedule, item, entries);
	}

	ফেরত;
}

/* return number of processes killed, -1 on error */
পূর্ণসংখ্যা do_stop(ধ্রুবক অক্ষর *applet, ধ্রুবক অক্ষর *exec, ধ্রুবক অক্ষর *ধ্রুবক *argv,
    pid_t pid, uid_t uid, পূর্ণসংখ্যা sig, bool group, bool test, bool quiet)
{
	ধ্রুবক অক্ষর *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	পূর্ণসংখ্যা nkilled = 0;
	uid_t target;
	যদি (group)
		kind = "GID";

	যদি (pid > 0)
		pids = rc_find_pids(ফাঁকা, ফাঁকা, 0, pid);
	অন্যথা
		pids = rc_find_pids(exec, argv, uid, 0);

	যদি (!pids)
		ফেরত 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		যদি (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} অন্যথা {
			যদি (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				যদি (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			যদি (group)
				target = -pi->pid;
			অন্যথা
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			যদি (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			অন্যথা যদি (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			যদি (!killed) {
				nkilled = -1;
			} অন্যথা {
				যদি (nkilled != -1)
					nkilled++;
			}
		}
		মুক্ত(pi);
	}

	মুক্ত(pids);
	ফেরত nkilled;
}

পূর্ণসংখ্যা run_stop_schedule(ধ্রুবক অক্ষর *applet,
		ধ্রুবক অক্ষর *exec, ধ্রুবক অক্ষর *ধ্রুবক *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	পূর্ণসংখ্যা nkilled = 0;
	পূর্ণসংখ্যা tkilled = 0;
	পূর্ণসংখ্যা nrunning = 0;
	long nloops, nsecs;
	কাঠামো timespec ts;
	ধ্রুবক অক্ষর *ধ্রুবক *p;
	bool progressed = false;

	যদি (!(pid > 0 || exec || uid || (argv && *argv)))
		ফেরত 0;

	যদি (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	যদি (pid > 0) {
		যদি (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} অন্যথা {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	যদি (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	যদি (argv && *argv) {
		einfovn("Will stop processes of `");
		যদি (rc_yesno(getenv("EINFO_VERBOSE"))) {
			জন্য (p = argv; p && *p; p++) {
				যদি (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	যখন (item) {
		নির্বাচন (item->type) {
		ক্ষেত্র SC_GOTO:
			item = item->gotoitem;
			চালু;

		ক্ষেত্র SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->মান, group,
					test, quiet);
			যদি (nkilled == 0) {
				যদি (tkilled == 0) {
					যদি (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				ফেরত tkilled;
			}
			অন্যথা যদি (nkilled == -1)
				ফেরত 0;

			tkilled += nkilled;
			বিরতি;
		ক্ষেত্র SC_FOREVER:
		ক্ষেত্র SC_TIMEOUT:
			যদি (item->type == SC_TIMEOUT && item->মান < 1) {
				item = ফাঁকা;
				বিরতি;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			জন্য (nsecs = 0; item->type == SC_FOREVER || nsecs < item->মান; nsecs++) {
				জন্য (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					যদি ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						ফেরত 0;


					যদি (nanosleep(&ts, ফাঁকা) == -1) {
						যদি (progressed) {
							printf("\n");
							progressed = false;
						}
						যদি (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							ফেরত 0;
						}
					}
				}
				যদি (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			বিরতি;
		পূর্বনির্ধারিত:
			যদি (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			ফেরত 0;
		}

		যদি (item)
			item = TAILQ_পরবর্তী(item, entries);
	}

	যদি (test || (tkilled > 0 && nrunning == 0))
		ফেরত nkilled;

	যদি (progressed)
		printf("\n");
	যদি (!quiet) {
		যদি (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		অন্যথা
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	ফেরত -nrunning;
}
