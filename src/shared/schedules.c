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

#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <signal.h>
#অন্তর্ভাৱ <stddef.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <syslog.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <sys/types.h>

#অন্তর্ভাৱ "einfo.h"
#অন্তর্ভাৱ "queue.h"
#অন্তর্ভাৱ "rc.h"
#অন্তর্ভাৱ "schedules.h"
#অন্তর্ভাৱ "helpers.h"

প্রকারনির্দেশ সংরচনা scheduleitem {
	গণনা {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	পূর্ণাঙ্ক মান;
	সংরচনা scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

স্থির TAILQ_HEAD(, scheduleitem) schedule;

শূন্য free_schedulelist(শূন্য)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	যেতিয়ালৈ (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		মুক্ত(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

পূর্ণাঙ্ক parse_signal(ধ্রুৱ ৱর্ণ *applet, ধ্রুৱ ৱর্ণ *sig)
{
	প্রকারনির্দেশ সংরচনা signalpair
	{
		ধ্রুৱ ৱর্ণ *name;
		পূর্ণাঙ্ক সঙ্কেত;
	} SIGNALPAIR;

#define signalpair_item(name) { #name, SIG##name },

	স্থির ধ্রুৱ SIGNALPAIR signallist[] = {
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

	unsigned পূর্ণাঙ্ক i = 0;
	ধ্রুৱ ৱর্ণ *s;

	যদি (!sig || *sig == '\0')
		ঘূৰাই -1;

	যদি (sscanf(sig, "%u", &i) == 1) {
		যদি (i < NSIG)
			ঘূৰাই i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	যদি (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	অন্যথা
		s = রিক্ত;

	বাবে (i = 0; i < ARRAY_SIZE(signallist); ++i)
		যদি (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			ঘূৰাই signallist[i].সঙ্কেত;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

স্থির SCHEDULEITEM *parse_schedule_item(ধ্রুৱ ৱর্ণ *applet, ধ্রুৱ ৱর্ণ *string)
{
	ধ্রুৱ ৱর্ণ *after_hyph;
	পূর্ণাঙ্ক sig;
	SCHEDULEITEM *item = xmalloc(পরিমাণ(*item));

	item->মান = 0;
	item->gotoitem = রিক্ত;
	যদি (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	অন্যথা যদি (isdigit((unsigned ৱর্ণ)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		যদি (sscanf(string, "%d", &item->মান) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} অন্যথা যদি ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->মান = (পূর্ণাঙ্ক)sig;
	} অন্যথা
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	ঘূৰাই item;
}

শূন্য parse_schedule(ধ্রুৱ ৱর্ণ *applet, ধ্রুৱ ৱর্ণ *string, পূর্ণাঙ্ক timeout)
{
	ৱর্ণ buffer[20];
	ধ্রুৱ ৱর্ণ *slash;
	পূর্ণাঙ্ক গুনতি = 0;
	SCHEDULEITEM *repeatat = রিক্ত;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	যদি (string)
		বাবে (slash = string; *slash; slash++)
			যদি (*slash == '/')
				গুনতি++;

	free_schedulelist();

	যদি (গুনতি == 0) {
		item = xmalloc(পরিমাণ(*item));
		item->type = SC_SIGNAL;
		item->মান = timeout;
		item->gotoitem = রিক্ত;
		TAILQ_ঢোকাও_TAIL(&schedule, item, entries);

		item = xmalloc(পরিমাণ(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = রিক্ত;
		TAILQ_ঢোকাও_TAIL(&schedule, item, entries);
		যদি (string) {
			যদি (sscanf(string, "%d", &item->মান) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} অন্যথা
			item->মান = 5;

		ঘূৰাই;
	}

	যেতিয়ালৈ (string != রিক্ত) {
		যদি ((slash = strchr(string, '/')))
			len = slash - string;
		অন্যথা
			len = strlen(string);

		যদি (len >= (ptrdiff_t)পরিমাণ(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : রিক্ত;

		item = parse_schedule_item(applet, buffer);
		TAILQ_ঢোকাও_TAIL(&schedule, item, entries);
		যদি (item->type == SC_FOREVER) {
			যদি (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			অনুৱর্ত;
		}
	}

	যদি (repeatat) {
		item = xmalloc(পরিমাণ(*item));
		item->type = SC_GOTO;
		item->মান = 0;
		item->gotoitem = repeatat;
		TAILQ_ঢোকাও_TAIL(&schedule, item, entries);
	}

	ঘূৰাই;
}

/* return number of processes killed, -1 on error */
পূর্ণাঙ্ক do_stop(ধ্রুৱ ৱর্ণ *applet, ধ্রুৱ ৱর্ণ *exec, ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv,
    pid_t pid, uid_t uid, পূর্ণাঙ্ক sig, bool group, bool test, bool quiet)
{
	ধ্রুৱ ৱর্ণ *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	পূর্ণাঙ্ক nkilled = 0;
	uid_t target;
	যদি (group)
		kind = "GID";

	যদি (pid > 0)
		pids = rc_find_pids(রিক্ত, রিক্ত, 0, pid);
	অন্যথা
		pids = rc_find_pids(exec, argv, uid, 0);

	যদি (!pids)
		ঘূৰাই 0;

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
	ঘূৰাই nkilled;
}

পূর্ণাঙ্ক run_stop_schedule(ধ্রুৱ ৱর্ণ *applet,
		ধ্রুৱ ৱর্ণ *exec, ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	পূর্ণাঙ্ক nkilled = 0;
	পূর্ণাঙ্ক tkilled = 0;
	পূর্ণাঙ্ক nrunning = 0;
	long nloops, nsecs;
	সংরচনা timespec ts;
	ধ্রুৱ ৱর্ণ *ধ্রুৱ *p;
	bool progressed = false;

	যদি (!(pid > 0 || exec || uid || (argv && *argv)))
		ঘূৰাই 0;

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
			বাবে (p = argv; p && *p; p++) {
				যদি (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	যেতিয়ালৈ (item) {
		ৱিকল্প (item->type) {
		পক্ষ SC_GOTO:
			item = item->gotoitem;
			অনুৱর্ত;

		পক্ষ SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->মান, group,
					test, quiet);
			যদি (nkilled == 0) {
				যদি (tkilled == 0) {
					যদি (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				ঘূৰাই tkilled;
			}
			অন্যথা যদি (nkilled == -1)
				ঘূৰাই 0;

			tkilled += nkilled;
			ভঙ্গ;
		পক্ষ SC_FOREVER:
		পক্ষ SC_TIMEOUT:
			যদি (item->type == SC_TIMEOUT && item->মান < 1) {
				item = রিক্ত;
				ভঙ্গ;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			বাবে (nsecs = 0; item->type == SC_FOREVER || nsecs < item->মান; nsecs++) {
				বাবে (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					যদি ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						ঘূৰাই 0;


					যদি (nanosleep(&ts, রিক্ত) == -1) {
						যদি (progressed) {
							printf("\n");
							progressed = false;
						}
						যদি (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							ঘূৰাই 0;
						}
					}
				}
				যদি (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			ভঙ্গ;
		স্ৱভাৱ:
			যদি (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			ঘূৰাই 0;
		}

		যদি (item)
			item = TAILQ_NEXT(item, entries);
	}

	যদি (test || (tkilled > 0 && nrunning == 0))
		ঘূৰাই nkilled;

	যদি (progressed)
		printf("\n");
	যদি (!quiet) {
		যদি (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		অন্যথা
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	ঘূৰাই -nrunning;
}
