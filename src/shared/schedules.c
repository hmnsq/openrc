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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ POLL_INTERVAL   20000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WAIT_PIDFILE   500000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ONE_ਦੂਜਾ    1000000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ONE_MS           1000000

#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <syslog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "schedules.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ scheduleitem {
	ਗਿਣਤੀ {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	ਪੂਰਨ ਮੁੱਲ;
	ਬਣਤਰ scheduleitem *gotoitem;
	TAILQ_ਇੰਦਰਾਜ(scheduleitem) entries;
} SCHEDULEITEM;

ਸਥਿਰ TAILQ_ਸਿਰ(, scheduleitem) ਸਮਾਂ_ਸਾਰਣੀ;

ਖਾਲੀ free_schedulelist(ਖਾਲੀ)
{
	SCHEDULEITEM *s1 = TAILQ_ਪਹਿਲਾ(&ਸਮਾਂ_ਸਾਰਣੀ);
	SCHEDULEITEM *s2;

	ਜਦੋਂ (s1) {
		s2 = TAILQ_ਅਗਲਾ(s1, entries);
		ਮੁਕਤ(s1);
		s1 = s2;
	}
	TAILQ_INIT(&ਸਮਾਂ_ਸਾਰਣੀ);
}

ਪੂਰਨ parse_signal(ਅਚਲ ਅੱਖਰ *applet, ਅਚਲ ਅੱਖਰ *sig)
{
	ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ signalpair
	{
		ਅਚਲ ਅੱਖਰ *name;
		ਪੂਰਨ ਸੰਕੇਤ;
	} SIGNALPAIR;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ signalpair_ਵਸਤੂ(name) { #name, SIG##name },

	ਸਥਿਰ ਅਚਲ SIGNALPAIR signallist[] = {
		signalpair_ਵਸਤੂ(HUP)
		signalpair_ਵਸਤੂ(INT)
		signalpair_ਵਸਤੂ(QUIT)
		signalpair_ਵਸਤੂ(ILL)
		signalpair_ਵਸਤੂ(TRAP)
		signalpair_ਵਸਤੂ(ABRT)
		signalpair_ਵਸਤੂ(BUS)
		signalpair_ਵਸਤੂ(FPE)
		signalpair_ਵਸਤੂ(KILL)
		signalpair_ਵਸਤੂ(USR1)
		signalpair_ਵਸਤੂ(SEGV)
		signalpair_ਵਸਤੂ(USR2)
		signalpair_ਵਸਤੂ(PIPE)
		signalpair_ਵਸਤੂ(ALRM)
		signalpair_ਵਸਤੂ(ਪਦ_2)
		signalpair_ਵਸਤੂ(CHLD)
		signalpair_ਵਸਤੂ(CONT)
		signalpair_ਵਸਤੂ(STOP)
		signalpair_ਵਸਤੂ(TSTP)
		signalpair_ਵਸਤੂ(TTIN)
		signalpair_ਵਸਤੂ(TTOU)
		signalpair_ਵਸਤੂ(URG)
		signalpair_ਵਸਤੂ(XCPU)
		signalpair_ਵਸਤੂ(XFSZ)
		signalpair_ਵਸਤੂ(VTALRM)
		signalpair_ਵਸਤੂ(PROF)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGWINCH
		signalpair_ਵਸਤੂ(WINCH)
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGIO
		signalpair_ਵਸਤੂ(IO)
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGPWR
		signalpair_ਵਸਤੂ(PWR)
#ਜੇ_ਅੰਤ
		signalpair_ਵਸਤੂ(SYS)
		{ "NULL",	0 },
	};

	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i = 0;
	ਅਚਲ ਅੱਖਰ *s;

	ਜੇ (!sig || *sig == '\0')
		ਵਾਪਸ -1;

	ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(sig, "%u", &i) == 1) {
		ਜੇ (i < NSIG)
			ਵਾਪਸ i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	ਜੇ (ਲੜੀ_ਤੁਲਨਾ(sig, "SIG", 3) == 0)
		s = sig + 3;
	ਨਹੀਂਤਾਂ
		s = ਨਿਰਾ;

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(signallist); ++i)
		ਜੇ (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			ਵਾਪਸ signallist[i].ਸੰਕੇਤ;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

ਸਥਿਰ SCHEDULEITEM *parse_ਸਮਾਂ_ਸਾਰਣੀ_ਵਸਤੂ(ਅਚਲ ਅੱਖਰ *applet, ਅਚਲ ਅੱਖਰ *string)
{
	ਅਚਲ ਅੱਖਰ *after_hyph;
	ਪੂਰਨ sig;
	SCHEDULEITEM *ਵਸਤੂ = xmalloc(ਆਕਾਰ(*ਵਸਤੂ));

	ਵਸਤੂ->ਮੁੱਲ = 0;
	ਵਸਤੂ->gotoitem = ਨਿਰਾ;
	ਜੇ (strcmp(string,"forever") == 0)
		ਵਸਤੂ->type = SC_FOREVER;
	ਨਹੀਂਤਾਂ ਜੇ (ਅੰਕ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)string[0])) {
		ਵਸਤੂ->type = SC_TIMEOUT;
		errno = 0;
		ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(string, "%d", &ਵਸਤੂ->ਮੁੱਲ) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} ਨਹੀਂਤਾਂ ਜੇ ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		ਵਸਤੂ->type = SC_SIGNAL;
		ਵਸਤੂ->ਮੁੱਲ = (ਪੂਰਨ)sig;
	} ਨਹੀਂਤਾਂ
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	ਵਾਪਸ ਵਸਤੂ;
}

ਖਾਲੀ parse_ਸਮਾਂ_ਸਾਰਣੀ(ਅਚਲ ਅੱਖਰ *applet, ਅਚਲ ਅੱਖਰ *string, ਪੂਰਨ timeout)
{
	ਅੱਖਰ ਬਫਰ[20];
	ਅਚਲ ਅੱਖਰ *slash;
	ਪੂਰਨ ਗਿਣਤੀ_2 = 0;
	SCHEDULEITEM *repeatat = ਨਿਰਾ;
	size_t len;
	SCHEDULEITEM *ਵਸਤੂ;

	TAILQ_INIT(&ਸਮਾਂ_ਸਾਰਣੀ);
	ਜੇ (string)
		ਲਈ (slash = string; *slash; slash++)
			ਜੇ (*slash == '/')
				ਗਿਣਤੀ_2++;

	free_schedulelist();

	ਜੇ (ਗਿਣਤੀ_2 == 0) {
		ਵਸਤੂ = xmalloc(ਆਕਾਰ(*ਵਸਤੂ));
		ਵਸਤੂ->type = SC_SIGNAL;
		ਵਸਤੂ->ਮੁੱਲ = timeout;
		ਵਸਤੂ->gotoitem = ਨਿਰਾ;
		TAILQ_ਪਾਓ_ਪੂਛ(&ਸਮਾਂ_ਸਾਰਣੀ, ਵਸਤੂ, entries);

		ਵਸਤੂ = xmalloc(ਆਕਾਰ(*ਵਸਤੂ));
		ਵਸਤੂ->type = SC_TIMEOUT;
		ਵਸਤੂ->gotoitem = ਨਿਰਾ;
		TAILQ_ਪਾਓ_ਪੂਛ(&ਸਮਾਂ_ਸਾਰਣੀ, ਵਸਤੂ, entries);
		ਜੇ (string) {
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(string, "%d", &ਵਸਤੂ->ਮੁੱਲ) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} ਨਹੀਂਤਾਂ
			ਵਸਤੂ->ਮੁੱਲ = 5;

		ਵਾਪਸ;
	}

	ਜਦੋਂ (string != ਨਿਰਾ) {
		ਜੇ ((slash = strchr(string, '/')))
			len = slash - string;
		ਨਹੀਂਤਾਂ
			len = ਲੜੀ_ਲੰਬਾਈ(string);

		ਜੇ (len >= (ptrdiff_t)ਆਕਾਰ(ਬਫਰ))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਫਰ, string, len);
		ਬਫਰ[len] = 0;
		string = slash ? slash + 1 : ਨਿਰਾ;

		ਵਸਤੂ = parse_ਸਮਾਂ_ਸਾਰਣੀ_ਵਸਤੂ(applet, ਬਫਰ);
		TAILQ_ਪਾਓ_ਪੂਛ(&ਸਮਾਂ_ਸਾਰਣੀ, ਵਸਤੂ, entries);
		ਜੇ (ਵਸਤੂ->type == SC_FOREVER) {
			ਜੇ (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = ਵਸਤੂ;
			ਜਾਰੀ;
		}
	}

	ਜੇ (repeatat) {
		ਵਸਤੂ = xmalloc(ਆਕਾਰ(*ਵਸਤੂ));
		ਵਸਤੂ->type = SC_GOTO;
		ਵਸਤੂ->ਮੁੱਲ = 0;
		ਵਸਤੂ->gotoitem = repeatat;
		TAILQ_ਪਾਓ_ਪੂਛ(&ਸਮਾਂ_ਸਾਰਣੀ, ਵਸਤੂ, entries);
	}

	ਵਾਪਸ;
}

/* return number of processes killed, -1 on error */
ਪੂਰਨ do_stop(ਅਚਲ ਅੱਖਰ *applet, ਅਚਲ ਅੱਖਰ *exec, ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv,
    pid_t pid, uid_t uid, ਪੂਰਨ sig, bool group, bool test, bool quiet)
{
	ਅਚਲ ਅੱਖਰ *ਕਿਸਮ = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	ਪੂਰਨ nkilled = 0;
	uid_t ਨਿਸ਼ਾਨਾ;
	ਜੇ (group)
		ਕਿਸਮ = "GID";

	ਜੇ (pid > 0)
		pids = rc_ਲੱਭੋ_pids(ਨਿਰਾ, ਨਿਰਾ, 0, pid);
	ਨਹੀਂਤਾਂ
		pids = rc_ਲੱਭੋ_pids(exec, argv, uid, 0);

	ਜੇ (!pids)
		ਵਾਪਸ 0;

	LIST_FOREACH_ਸੁਰੱਖਿਅਤ(pi, pids, entries, np) {
		ਜੇ (test) {
			einfo("Would send signal %d to %s %d", sig, ਕਿਸਮ, pi->pid);
			nkilled++;
		} ਨਹੀਂਤਾਂ {
			ਜੇ (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, ਕਿਸਮ, pi->pid);
				ਜੇ (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, ਕਿਸਮ, pi->pid);
			}
			errno = 0;
			ਜੇ (group)
				ਨਿਸ਼ਾਨਾ = -pi->pid;
			ਨਹੀਂਤਾਂ
				ਨਿਸ਼ਾਨਾ = pi->pid;
			killed = (kill(ਨਿਸ਼ਾਨਾ, sig) == 0 ||
			    errno == ESRCH ? true : false);
			ਜੇ (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, ਕਿਸਮ, pi->pid, strerror(errno));
			ਨਹੀਂਤਾਂ ਜੇ (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, ਕਿਸਮ, pi->pid, strerror(errno));
			ਜੇ (!killed) {
				nkilled = -1;
			} ਨਹੀਂਤਾਂ {
				ਜੇ (nkilled != -1)
					nkilled++;
			}
		}
		ਮੁਕਤ(pi);
	}

	ਮੁਕਤ(pids);
	ਵਾਪਸ nkilled;
}

ਪੂਰਨ ਲੜੀਵਾਰ_stop_ਸਮਾਂ_ਸਾਰਣੀ(ਅਚਲ ਅੱਖਰ *applet,
		ਅਚਲ ਅੱਖਰ *exec, ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool ਤਰੱਕੀ, bool quiet)
{
	SCHEDULEITEM *ਵਸਤੂ = TAILQ_ਪਹਿਲਾ(&ਸਮਾਂ_ਸਾਰਣੀ);
	ਪੂਰਨ nkilled = 0;
	ਪੂਰਨ tkilled = 0;
	ਪੂਰਨ nrunning = 0;
	ਲੰਮਾ nloops, nsecs;
	ਬਣਤਰ timespec ts;
	ਅਚਲ ਅੱਖਰ *ਅਚਲ *p;
	bool progressed = false;

	ਜੇ (!(pid > 0 || exec || uid || (argv && *argv)))
		ਵਾਪਸ 0;

	ਜੇ (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	ਜੇ (pid > 0) {
		ਜੇ (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} ਨਹੀਂਤਾਂ {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	ਜੇ (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	ਜੇ (argv && *argv) {
		einfovn("Will stop processes of `");
		ਜੇ (rc_yesno(getenv("EINFO_VERBOSE"))) {
			ਲਈ (p = argv; p && *p; p++) {
				ਜੇ (p != argv)
					ਲਿਖੋ(" ");
				ਲਿਖੋ("%s", *p);
			}
			ਲਿਖੋ("'\n");
		}
	}

	ਜਦੋਂ (ਵਸਤੂ) {
		ਚੋਣ (ਵਸਤੂ->type) {
		ਹਾਲਤ SC_GOTO:
			ਵਸਤੂ = ਵਸਤੂ->gotoitem;
			ਜਾਰੀ;

		ਹਾਲਤ SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, ਵਸਤੂ->ਮੁੱਲ, group,
					test, quiet);
			ਜੇ (nkilled == 0) {
				ਜੇ (tkilled == 0) {
					ਜੇ (progressed)
						ਲਿਖੋ("\n");
					eerror("%s: no matching processes found", applet);
				}
				ਵਾਪਸ tkilled;
			}
			ਨਹੀਂਤਾਂ ਜੇ (nkilled == -1)
				ਵਾਪਸ 0;

			tkilled += nkilled;
			ਰੋਕੋ;
		ਹਾਲਤ SC_FOREVER:
		ਹਾਲਤ SC_TIMEOUT:
			ਜੇ (ਵਸਤੂ->type == SC_TIMEOUT && ਵਸਤੂ->ਮੁੱਲ < 1) {
				ਵਸਤੂ = ਨਿਰਾ;
				ਰੋਕੋ;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			ਲਈ (nsecs = 0; ਵਸਤੂ->type == SC_FOREVER || nsecs < ਵਸਤੂ->ਮੁੱਲ; nsecs++) {
				ਲਈ (nloops = 0;
				     nloops < ONE_ਦੂਜਾ / POLL_INTERVAL;
				     nloops++)
				{
					ਜੇ ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						ਵਾਪਸ 0;


					ਜੇ (nanosleep(&ts, ਨਿਰਾ) == -1) {
						ਜੇ (progressed) {
							ਲਿਖੋ("\n");
							progressed = false;
						}
						ਜੇ (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							ਵਾਪਸ 0;
						}
					}
				}
				ਜੇ (ਤਰੱਕੀ) {
					ਲਿਖੋ(".");
					fflush(stdout);
					progressed = true;
				}
			}
			ਰੋਕੋ;
		ਮੂਲ:
			ਜੇ (progressed) {
				ਲਿਖੋ("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, ਵਸਤੂ->type);
			ਵਾਪਸ 0;
		}

		ਜੇ (ਵਸਤੂ)
			ਵਸਤੂ = TAILQ_ਅਗਲਾ(ਵਸਤੂ, entries);
	}

	ਜੇ (test || (tkilled > 0 && nrunning == 0))
		ਵਾਪਸ nkilled;

	ਜੇ (progressed)
		ਲਿਖੋ("\n");
	ਜੇ (!quiet) {
		ਜੇ (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		ਨਹੀਂਤਾਂ
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	ਵਾਪਸ -nrunning;
}
