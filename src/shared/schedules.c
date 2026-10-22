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
#ವ್ಯಾಖ್ಯಾನಿಸು POLL_INTERVAL   20000000
#ವ್ಯಾಖ್ಯಾನಿಸು WAIT_PIDFILE   500000000
#ವ್ಯಾಖ್ಯಾನಿಸು ONE_ಎರಡನೇ    1000000000
#ವ್ಯಾಖ್ಯಾನಿಸು ONE_MS           1000000

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

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ scheduleitem {
	ಎಣಿಕೆ {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ;
	ರಚನೆ scheduleitem *gotoitem;
	TAILQ_ನಮೂದು(scheduleitem) entries;
} SCHEDULEITEM;

ಸ್ಥಿರ TAILQ_ತಲೆ(, scheduleitem) ವೇಳಾಪಟ್ಟಿ;

ಶೂನ್ಯ free_schedulelist(ಶೂನ್ಯ)
{
	SCHEDULEITEM *s1 = TAILQ_ಮೊದಲ(&ವೇಳಾಪಟ್ಟಿ);
	SCHEDULEITEM *s2;

	ಆಗ (s1) {
		s2 = TAILQ_ಮುಂದಿನ(s1, entries);
		ಬಿಡುಗಡೆ(s1);
		s1 = s2;
	}
	TAILQ_INIT(&ವೇಳಾಪಟ್ಟಿ);
}

ಪೂರ್ಣಾಂಕ parse_signal(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sig)
{
	ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ signalpair
	{
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
		ಪೂರ್ಣಾಂಕ ಸಂಕೇತ;
	} SIGNALPAIR;

#ವ್ಯಾಖ್ಯಾನಿಸು signalpair_ಅಂಶ(name) { #name, SIG##name },

	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ SIGNALPAIR signallist[] = {
		signalpair_ಅಂಶ(HUP)
		signalpair_ಅಂಶ(INT)
		signalpair_ಅಂಶ(QUIT)
		signalpair_ಅಂಶ(ILL)
		signalpair_ಅಂಶ(TRAP)
		signalpair_ಅಂಶ(ABRT)
		signalpair_ಅಂಶ(BUS)
		signalpair_ಅಂಶ(FPE)
		signalpair_ಅಂಶ(KILL)
		signalpair_ಅಂಶ(USR1)
		signalpair_ಅಂಶ(SEGV)
		signalpair_ಅಂಶ(USR2)
		signalpair_ಅಂಶ(PIPE)
		signalpair_ಅಂಶ(ALRM)
		signalpair_ಅಂಶ(ಪದ_2)
		signalpair_ಅಂಶ(CHLD)
		signalpair_ಅಂಶ(CONT)
		signalpair_ಅಂಶ(STOP)
		signalpair_ಅಂಶ(TSTP)
		signalpair_ಅಂಶ(TTIN)
		signalpair_ಅಂಶ(TTOU)
		signalpair_ಅಂಶ(URG)
		signalpair_ಅಂಶ(XCPU)
		signalpair_ಅಂಶ(XFSZ)
		signalpair_ಅಂಶ(VTALRM)
		signalpair_ಅಂಶ(PROF)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGWINCH
		signalpair_ಅಂಶ(WINCH)
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGIO
		signalpair_ಅಂಶ(IO)
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGPWR
		signalpair_ಅಂಶ(PWR)
#ಆದರೆ_ಅಂತ್ಯ
		signalpair_ಅಂಶ(SYS)
		{ "NULL",	0 },
	};

	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;

	ಆದರೆ (!sig || *sig == '\0')
		ಹಿಂತಿರುಗಿ -1;

	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(sig, "%u", &i) == 1) {
		ಆದರೆ (i < NSIG)
			ಹಿಂತಿರುಗಿ i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(sig, "SIG", 3) == 0)
		s = sig + 3;
	ಇಲ್ಲದಿದ್ದರೆ
		s = ಖಾಲಿ;

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(signallist); ++i)
		ಆದರೆ (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			ಹಿಂತಿರುಗಿ signallist[i].ಸಂಕೇತ;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

ಸ್ಥಿರ SCHEDULEITEM *parse_ವೇಳಾಪಟ್ಟಿ_ಅಂಶ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *after_hyph;
	ಪೂರ್ಣಾಂಕ sig;
	SCHEDULEITEM *ಅಂಶ = xmalloc(ಗಾತ್ರ(*ಅಂಶ));

	ಅಂಶ->ಮೌಲ್ಯ = 0;
	ಅಂಶ->gotoitem = ಖಾಲಿ;
	ಆದರೆ (strcmp(string,"forever") == 0)
		ಅಂಶ->type = SC_FOREVER;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಂಕಿಯೇ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)string[0])) {
		ಅಂಶ->type = SC_TIMEOUT;
		errno = 0;
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(string, "%d", &ಅಂಶ->ಮೌಲ್ಯ) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		ಅಂಶ->type = SC_SIGNAL;
		ಅಂಶ->ಮೌಲ್ಯ = (ಪೂರ್ಣಾಂಕ)sig;
	} ಇಲ್ಲದಿದ್ದರೆ
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	ಹಿಂತಿರುಗಿ ಅಂಶ;
}

ಶೂನ್ಯ parse_ವೇಳಾಪಟ್ಟಿ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string, ಪೂರ್ಣಾಂಕ timeout)
{
	ಅಕ್ಷರ ಬಫರ್[20];
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *slash;
	ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;
	SCHEDULEITEM *repeatat = ಖಾಲಿ;
	size_t len;
	SCHEDULEITEM *ಅಂಶ;

	TAILQ_INIT(&ವೇಳಾಪಟ್ಟಿ);
	ಆದರೆ (string)
		ಗಾಗಿ (slash = string; *slash; slash++)
			ಆದರೆ (*slash == '/')
				ಎಣಿಕೆ_2++;

	free_schedulelist();

	ಆದರೆ (ಎಣಿಕೆ_2 == 0) {
		ಅಂಶ = xmalloc(ಗಾತ್ರ(*ಅಂಶ));
		ಅಂಶ->type = SC_SIGNAL;
		ಅಂಶ->ಮೌಲ್ಯ = timeout;
		ಅಂಶ->gotoitem = ಖಾಲಿ;
		TAILQ_ಸೇರಿಸು_ಬಾಲ(&ವೇಳಾಪಟ್ಟಿ, ಅಂಶ, entries);

		ಅಂಶ = xmalloc(ಗಾತ್ರ(*ಅಂಶ));
		ಅಂಶ->type = SC_TIMEOUT;
		ಅಂಶ->gotoitem = ಖಾಲಿ;
		TAILQ_ಸೇರಿಸು_ಬಾಲ(&ವೇಳಾಪಟ್ಟಿ, ಅಂಶ, entries);
		ಆದರೆ (string) {
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(string, "%d", &ಅಂಶ->ಮೌಲ್ಯ) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} ಇಲ್ಲದಿದ್ದರೆ
			ಅಂಶ->ಮೌಲ್ಯ = 5;

		ಹಿಂತಿರುಗಿ;
	}

	ಆಗ (string != ಖಾಲಿ) {
		ಆದರೆ ((slash = strchr(string, '/')))
			len = slash - string;
		ಇಲ್ಲದಿದ್ದರೆ
			len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(string);

		ಆದರೆ (len >= (ptrdiff_t)ಗಾತ್ರ(ಬಫರ್))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		ಸ್ಮರಣೆ_ನಕಲು(ಬಫರ್, string, len);
		ಬಫರ್[len] = 0;
		string = slash ? slash + 1 : ಖಾಲಿ;

		ಅಂಶ = parse_ವೇಳಾಪಟ್ಟಿ_ಅಂಶ(applet, ಬಫರ್);
		TAILQ_ಸೇರಿಸು_ಬಾಲ(&ವೇಳಾಪಟ್ಟಿ, ಅಂಶ, entries);
		ಆದರೆ (ಅಂಶ->type == SC_FOREVER) {
			ಆದರೆ (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = ಅಂಶ;
			ಮುಂದುವರಿ;
		}
	}

	ಆದರೆ (repeatat) {
		ಅಂಶ = xmalloc(ಗಾತ್ರ(*ಅಂಶ));
		ಅಂಶ->type = SC_GOTO;
		ಅಂಶ->ಮೌಲ್ಯ = 0;
		ಅಂಶ->gotoitem = repeatat;
		TAILQ_ಸೇರಿಸು_ಬಾಲ(&ವೇಳಾಪಟ್ಟಿ, ಅಂಶ, entries);
	}

	ಹಿಂತಿರುಗಿ;
}

/* return number of processes killed, -1 on error */
ಪೂರ್ಣಾಂಕ do_stop(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *exec, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv,
    pid_t pid, uid_t uid, ಪೂರ್ಣಾಂಕ sig, bool group, bool test, bool quiet)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಗೆ = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	ಪೂರ್ಣಾಂಕ nkilled = 0;
	uid_t ಗುರಿ;
	ಆದರೆ (group)
		ಬಗೆ = "GID";

	ಆದರೆ (pid > 0)
		pids = rc_ಕಂಡುಹಿಡಿ_pids(ಖಾಲಿ, ಖಾಲಿ, 0, pid);
	ಇಲ್ಲದಿದ್ದರೆ
		pids = rc_ಕಂಡುಹಿಡಿ_pids(exec, argv, uid, 0);

	ಆದರೆ (!pids)
		ಹಿಂತಿರುಗಿ 0;

	LIST_FOREACH_ಸುರಕ್ಷಿತ(pi, pids, entries, np) {
		ಆದರೆ (test) {
			einfo("Would send signal %d to %s %d", sig, ಬಗೆ, pi->pid);
			nkilled++;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, ಬಗೆ, pi->pid);
				ಆದರೆ (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, ಬಗೆ, pi->pid);
			}
			errno = 0;
			ಆದರೆ (group)
				ಗುರಿ = -pi->pid;
			ಇಲ್ಲದಿದ್ದರೆ
				ಗುರಿ = pi->pid;
			killed = (kill(ಗುರಿ, sig) == 0 ||
			    errno == ESRCH ? true : false);
			ಆದರೆ (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, ಬಗೆ, pi->pid, strerror(errno));
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, ಬಗೆ, pi->pid, strerror(errno));
			ಆದರೆ (!killed) {
				nkilled = -1;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (nkilled != -1)
					nkilled++;
			}
		}
		ಬಿಡುಗಡೆ(pi);
	}

	ಬಿಡುಗಡೆ(pids);
	ಹಿಂತಿರುಗಿ nkilled;
}

ಪೂರ್ಣಾಂಕ ಸತತ_stop_ವೇಳಾಪಟ್ಟಿ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *exec, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool ಪ್ರಗತಿ, bool quiet)
{
	SCHEDULEITEM *ಅಂಶ = TAILQ_ಮೊದಲ(&ವೇಳಾಪಟ್ಟಿ);
	ಪೂರ್ಣಾಂಕ nkilled = 0;
	ಪೂರ್ಣಾಂಕ tkilled = 0;
	ಪೂರ್ಣಾಂಕ nrunning = 0;
	ದೀರ್ಘ nloops, nsecs;
	ರಚನೆ timespec ts;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *p;
	bool progressed = false;

	ಆದರೆ (!(pid > 0 || exec || uid || (argv && *argv)))
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	ಆದರೆ (pid > 0) {
		ಆದರೆ (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} ಇಲ್ಲದಿದ್ದರೆ {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	ಆದರೆ (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	ಆದರೆ (argv && *argv) {
		einfovn("Will stop processes of `");
		ಆದರೆ (rc_yesno(getenv("EINFO_VERBOSE"))) {
			ಗಾಗಿ (p = argv; p && *p; p++) {
				ಆದರೆ (p != argv)
					ಬರೆ(" ");
				ಬರೆ("%s", *p);
			}
			ಬರೆ("'\n");
		}
	}

	ಆಗ (ಅಂಶ) {
		ಆಯ್ಕೆ (ಅಂಶ->type) {
		ಸ್ಥಿತಿ SC_GOTO:
			ಅಂಶ = ಅಂಶ->gotoitem;
			ಮುಂದುವರಿ;

		ಸ್ಥಿತಿ SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, ಅಂಶ->ಮೌಲ್ಯ, group,
					test, quiet);
			ಆದರೆ (nkilled == 0) {
				ಆದರೆ (tkilled == 0) {
					ಆದರೆ (progressed)
						ಬರೆ("\n");
					eerror("%s: no matching processes found", applet);
				}
				ಹಿಂತಿರುಗಿ tkilled;
			}
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (nkilled == -1)
				ಹಿಂತಿರುಗಿ 0;

			tkilled += nkilled;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ SC_FOREVER:
		ಸ್ಥಿತಿ SC_TIMEOUT:
			ಆದರೆ (ಅಂಶ->type == SC_TIMEOUT && ಅಂಶ->ಮೌಲ್ಯ < 1) {
				ಅಂಶ = ಖಾಲಿ;
				ನಿಲ್ಲಿಸು;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			ಗಾಗಿ (nsecs = 0; ಅಂಶ->type == SC_FOREVER || nsecs < ಅಂಶ->ಮೌಲ್ಯ; nsecs++) {
				ಗಾಗಿ (nloops = 0;
				     nloops < ONE_ಎರಡನೇ / POLL_INTERVAL;
				     nloops++)
				{
					ಆದರೆ ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						ಹಿಂತಿರುಗಿ 0;


					ಆದರೆ (nanosleep(&ts, ಖಾಲಿ) == -1) {
						ಆದರೆ (progressed) {
							ಬರೆ("\n");
							progressed = false;
						}
						ಆದರೆ (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							ಹಿಂತಿರುಗಿ 0;
						}
					}
				}
				ಆದರೆ (ಪ್ರಗತಿ) {
					ಬರೆ(".");
					fflush(stdout);
					progressed = true;
				}
			}
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಆದರೆ (progressed) {
				ಬರೆ("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, ಅಂಶ->type);
			ಹಿಂತಿರುಗಿ 0;
		}

		ಆದರೆ (ಅಂಶ)
			ಅಂಶ = TAILQ_ಮುಂದಿನ(ಅಂಶ, entries);
	}

	ಆದರೆ (test || (tkilled > 0 && nrunning == 0))
		ಹಿಂತಿರುಗಿ nkilled;

	ಆದರೆ (progressed)
		ಬರೆ("\n");
	ಆದರೆ (!quiet) {
		ಆದರೆ (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		ಇಲ್ಲದಿದ್ದರೆ
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	ಹಿಂತಿರುಗಿ -nrunning;
}
