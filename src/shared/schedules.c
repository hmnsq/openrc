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

#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <syslog.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "schedules.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ scheduleitem {
	ಗಣನಾ {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	ಪೂರ್ಣಾಙ್ಕ ಮೌಲ್ಯ;
	ಸಂರಚನಾ scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

ಸ್ಥಿರ TAILQ_HEAD(, scheduleitem) schedule;

ಶೂನ್ಯ free_schedulelist(ಶೂನ್ಯ)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	ಯಾವತ್ (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		ಮುಕ್ತ(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

ಪೂರ್ಣಾಙ್ಕ parse_signal(ಧ್ರುವ ವರ್ಣ *applet, ಧ್ರುವ ವರ್ಣ *sig)
{
	ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ signalpair
	{
		ಧ್ರುವ ವರ್ಣ *name;
		ಪೂರ್ಣಾಙ್ಕ ಸಙ್ಕೇತ;
	} SIGNALPAIR;

#define signalpair_item(name) { #name, SIG##name },

	ಸ್ಥಿರ ಧ್ರುವ SIGNALPAIR signallist[] = {
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

	unsigned ಪೂರ್ಣಾಙ್ಕ i = 0;
	ಧ್ರುವ ವರ್ಣ *s;

	ಯದಿ (!sig || *sig == '\0')
		ಪ್ರತ್ಯಾಗಮ -1;

	ಯದಿ (sscanf(sig, "%u", &i) == 1) {
		ಯದಿ (i < NSIG)
			ಪ್ರತ್ಯಾಗಮ i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	ಯದಿ (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	ಅನ್ಯಥಾ
		s = ರಿಕ್ತ;

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(signallist); ++i)
		ಯದಿ (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			ಪ್ರತ್ಯಾಗಮ signallist[i].ಸಙ್ಕೇತ;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

ಸ್ಥಿರ SCHEDULEITEM *parse_schedule_item(ಧ್ರುವ ವರ್ಣ *applet, ಧ್ರುವ ವರ್ಣ *string)
{
	ಧ್ರುವ ವರ್ಣ *after_hyph;
	ಪೂರ್ಣಾಙ್ಕ sig;
	SCHEDULEITEM *item = xmalloc(ಪರಿಮಾಣ(*item));

	item->ಮೌಲ್ಯ = 0;
	item->gotoitem = ರಿಕ್ತ;
	ಯದಿ (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	ಅನ್ಯಥಾ ಯದಿ (isdigit((unsigned ವರ್ಣ)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		ಯದಿ (sscanf(string, "%d", &item->ಮೌಲ್ಯ) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} ಅನ್ಯಥಾ ಯದಿ ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->ಮೌಲ್ಯ = (ಪೂರ್ಣಾಙ್ಕ)sig;
	} ಅನ್ಯಥಾ
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	ಪ್ರತ್ಯಾಗಮ item;
}

ಶೂನ್ಯ parse_schedule(ಧ್ರುವ ವರ್ಣ *applet, ಧ್ರುವ ವರ್ಣ *string, ಪೂರ್ಣಾಙ್ಕ timeout)
{
	ವರ್ಣ buffer[20];
	ಧ್ರುವ ವರ್ಣ *slash;
	ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0;
	SCHEDULEITEM *repeatat = ರಿಕ್ತ;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	ಯದಿ (string)
		ಪ್ರತಿ (slash = string; *slash; slash++)
			ಯದಿ (*slash == '/')
				ಎಣಿಕೆ++;

	free_schedulelist();

	ಯದಿ (ಎಣಿಕೆ == 0) {
		item = xmalloc(ಪರಿಮಾಣ(*item));
		item->type = SC_SIGNAL;
		item->ಮೌಲ್ಯ = timeout;
		item->gotoitem = ರಿಕ್ತ;
		TAILQ_ಸೇರಿಸು_TAIL(&schedule, item, entries);

		item = xmalloc(ಪರಿಮಾಣ(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = ರಿಕ್ತ;
		TAILQ_ಸೇರಿಸು_TAIL(&schedule, item, entries);
		ಯದಿ (string) {
			ಯದಿ (sscanf(string, "%d", &item->ಮೌಲ್ಯ) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} ಅನ್ಯಥಾ
			item->ಮೌಲ್ಯ = 5;

		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯಾವತ್ (string != ರಿಕ್ತ) {
		ಯದಿ ((slash = strchr(string, '/')))
			len = slash - string;
		ಅನ್ಯಥಾ
			len = strlen(string);

		ಯದಿ (len >= (ptrdiff_t)ಪರಿಮಾಣ(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : ರಿಕ್ತ;

		item = parse_schedule_item(applet, buffer);
		TAILQ_ಸೇರಿಸು_TAIL(&schedule, item, entries);
		ಯದಿ (item->type == SC_FOREVER) {
			ಯದಿ (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			ಅನುವರ್ತ;
		}
	}

	ಯದಿ (repeatat) {
		item = xmalloc(ಪರಿಮಾಣ(*item));
		item->type = SC_GOTO;
		item->ಮೌಲ್ಯ = 0;
		item->gotoitem = repeatat;
		TAILQ_ಸೇರಿಸು_TAIL(&schedule, item, entries);
	}

	ಪ್ರತ್ಯಾಗಮ;
}

/* return number of processes killed, -1 on error */
ಪೂರ್ಣಾಙ್ಕ do_stop(ಧ್ರುವ ವರ್ಣ *applet, ಧ್ರುವ ವರ್ಣ *exec, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv,
    pid_t pid, uid_t uid, ಪೂರ್ಣಾಙ್ಕ sig, bool group, bool test, bool quiet)
{
	ಧ್ರುವ ವರ್ಣ *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	ಪೂರ್ಣಾಙ್ಕ nkilled = 0;
	uid_t target;
	ಯದಿ (group)
		kind = "GID";

	ಯದಿ (pid > 0)
		pids = rc_find_pids(ರಿಕ್ತ, ರಿಕ್ತ, 0, pid);
	ಅನ್ಯಥಾ
		pids = rc_find_pids(exec, argv, uid, 0);

	ಯದಿ (!pids)
		ಪ್ರತ್ಯಾಗಮ 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		ಯದಿ (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} ಅನ್ಯಥಾ {
			ಯದಿ (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				ಯದಿ (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			ಯದಿ (group)
				target = -pi->pid;
			ಅನ್ಯಥಾ
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			ಯದಿ (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			ಅನ್ಯಥಾ ಯದಿ (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			ಯದಿ (!killed) {
				nkilled = -1;
			} ಅನ್ಯಥಾ {
				ಯದಿ (nkilled != -1)
					nkilled++;
			}
		}
		ಮುಕ್ತ(pi);
	}

	ಮುಕ್ತ(pids);
	ಪ್ರತ್ಯಾಗಮ nkilled;
}

ಪೂರ್ಣಾಙ್ಕ run_stop_schedule(ಧ್ರುವ ವರ್ಣ *applet,
		ಧ್ರುವ ವರ್ಣ *exec, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	ಪೂರ್ಣಾಙ್ಕ nkilled = 0;
	ಪೂರ್ಣಾಙ್ಕ tkilled = 0;
	ಪೂರ್ಣಾಙ್ಕ nrunning = 0;
	long nloops, nsecs;
	ಸಂರಚನಾ timespec ts;
	ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *p;
	bool progressed = false;

	ಯದಿ (!(pid > 0 || exec || uid || (argv && *argv)))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	ಯದಿ (pid > 0) {
		ಯದಿ (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} ಅನ್ಯಥಾ {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	ಯದಿ (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	ಯದಿ (argv && *argv) {
		einfovn("Will stop processes of `");
		ಯದಿ (rc_yesno(getenv("EINFO_VERBOSE"))) {
			ಪ್ರತಿ (p = argv; p && *p; p++) {
				ಯದಿ (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	ಯಾವತ್ (item) {
		ವಿಕಲ್ಪ (item->type) {
		ಪಕ್ಷ SC_GOTO:
			item = item->gotoitem;
			ಅನುವರ್ತ;

		ಪಕ್ಷ SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->ಮೌಲ್ಯ, group,
					test, quiet);
			ಯದಿ (nkilled == 0) {
				ಯದಿ (tkilled == 0) {
					ಯದಿ (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				ಪ್ರತ್ಯಾಗಮ tkilled;
			}
			ಅನ್ಯಥಾ ಯದಿ (nkilled == -1)
				ಪ್ರತ್ಯಾಗಮ 0;

			tkilled += nkilled;
			ಭಙ್ಗ;
		ಪಕ್ಷ SC_FOREVER:
		ಪಕ್ಷ SC_TIMEOUT:
			ಯದಿ (item->type == SC_TIMEOUT && item->ಮೌಲ್ಯ < 1) {
				item = ರಿಕ್ತ;
				ಭಙ್ಗ;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			ಪ್ರತಿ (nsecs = 0; item->type == SC_FOREVER || nsecs < item->ಮೌಲ್ಯ; nsecs++) {
				ಪ್ರತಿ (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					ಯದಿ ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						ಪ್ರತ್ಯಾಗಮ 0;


					ಯದಿ (nanosleep(&ts, ರಿಕ್ತ) == -1) {
						ಯದಿ (progressed) {
							printf("\n");
							progressed = false;
						}
						ಯದಿ (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							ಪ್ರತ್ಯಾಗಮ 0;
						}
					}
				}
				ಯದಿ (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಯದಿ (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			ಪ್ರತ್ಯಾಗಮ 0;
		}

		ಯದಿ (item)
			item = TAILQ_NEXT(item, entries);
	}

	ಯದಿ (test || (tkilled > 0 && nrunning == 0))
		ಪ್ರತ್ಯಾಗಮ nkilled;

	ಯದಿ (progressed)
		printf("\n");
	ಯದಿ (!quiet) {
		ಯದಿ (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		ಅನ್ಯಥಾ
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	ಪ್ರತ್ಯಾಗಮ -nrunning;
}
