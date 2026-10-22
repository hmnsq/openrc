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

#अन्तर्भाव <ctype.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <syslog.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <sys/types.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "schedules.h"
#अन्तर्भाव "helpers.h"

प्रकारनिर्देश संरचना scheduleitem {
	गणना {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	पूर्णाङ्क मानम्;
	संरचना scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

स्थिर TAILQ_HEAD(, scheduleitem) schedule;

शून्य free_schedulelist(शून्य)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	जाव (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		मुक्त(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

पूर्णाङ्क parse_signal(ध्रुव वर्ण *applet, ध्रुव वर्ण *sig)
{
	प्रकारनिर्देश संरचना signalpair
	{
		ध्रुव वर्ण *name;
		पूर्णाङ्क सङ्केत;
	} SIGNALPAIR;

#define signalpair_item(name) { #name, SIG##name },

	स्थिर ध्रुव SIGNALPAIR signallist[] = {
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

	unsigned पूर्णाङ्क i = 0;
	ध्रुव वर्ण *s;

	जइ (!sig || *sig == '\0')
		प्रत्यागम -1;

	जइ (sscanf(sig, "%u", &i) == 1) {
		जइ (i < NSIG)
			प्रत्यागम i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	जइ (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	अण्णहा
		s = रिक्त;

	प्रति (i = 0; i < ARRAY_SIZE(signallist); ++i)
		जइ (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			प्रत्यागम signallist[i].सङ्केत;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

स्थिर SCHEDULEITEM *parse_schedule_item(ध्रुव वर्ण *applet, ध्रुव वर्ण *string)
{
	ध्रुव वर्ण *after_hyph;
	पूर्णाङ्क sig;
	SCHEDULEITEM *item = xmalloc(परिमाण(*item));

	item->मानम् = 0;
	item->gotoitem = रिक्त;
	जइ (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	अण्णहा जइ (isdigit((unsigned वर्ण)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		जइ (sscanf(string, "%d", &item->मानम्) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} अण्णहा जइ ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->मानम् = (पूर्णाङ्क)sig;
	} अण्णहा
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	प्रत्यागम item;
}

शून्य parse_schedule(ध्रुव वर्ण *applet, ध्रुव वर्ण *string, पूर्णाङ्क timeout)
{
	वर्ण buffer[20];
	ध्रुव वर्ण *slash;
	पूर्णाङ्क गणनम् = 0;
	SCHEDULEITEM *repeatat = रिक्त;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	जइ (string)
		प्रति (slash = string; *slash; slash++)
			जइ (*slash == '/')
				गणनम्++;

	free_schedulelist();

	जइ (गणनम् == 0) {
		item = xmalloc(परिमाण(*item));
		item->type = SC_SIGNAL;
		item->मानम् = timeout;
		item->gotoitem = रिक्त;
		TAILQ_निवेशय_TAIL(&schedule, item, entries);

		item = xmalloc(परिमाण(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = रिक्त;
		TAILQ_निवेशय_TAIL(&schedule, item, entries);
		जइ (string) {
			जइ (sscanf(string, "%d", &item->मानम्) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} अण्णहा
			item->मानम् = 5;

		प्रत्यागम;
	}

	जाव (string != रिक्त) {
		जइ ((slash = strchr(string, '/')))
			len = slash - string;
		अण्णहा
			len = strlen(string);

		जइ (len >= (ptrdiff_t)परिमाण(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : रिक्त;

		item = parse_schedule_item(applet, buffer);
		TAILQ_निवेशय_TAIL(&schedule, item, entries);
		जइ (item->type == SC_FOREVER) {
			जइ (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			अनुवर्त;
		}
	}

	जइ (repeatat) {
		item = xmalloc(परिमाण(*item));
		item->type = SC_GOTO;
		item->मानम् = 0;
		item->gotoitem = repeatat;
		TAILQ_निवेशय_TAIL(&schedule, item, entries);
	}

	प्रत्यागम;
}

/* return number of processes killed, -1 on error */
पूर्णाङ्क do_stop(ध्रुव वर्ण *applet, ध्रुव वर्ण *exec, ध्रुव वर्ण *ध्रुव *argv,
    pid_t pid, uid_t uid, पूर्णाङ्क sig, bool group, bool test, bool quiet)
{
	ध्रुव वर्ण *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	पूर्णाङ्क nkilled = 0;
	uid_t target;
	जइ (group)
		kind = "GID";

	जइ (pid > 0)
		pids = rc_find_pids(रिक्त, रिक्त, 0, pid);
	अण्णहा
		pids = rc_find_pids(exec, argv, uid, 0);

	जइ (!pids)
		प्रत्यागम 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		जइ (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} अण्णहा {
			जइ (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				जइ (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			जइ (group)
				target = -pi->pid;
			अण्णहा
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			जइ (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			अण्णहा जइ (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			जइ (!killed) {
				nkilled = -1;
			} अण्णहा {
				जइ (nkilled != -1)
					nkilled++;
			}
		}
		मुक्त(pi);
	}

	मुक्त(pids);
	प्रत्यागम nkilled;
}

पूर्णाङ्क run_stop_schedule(ध्रुव वर्ण *applet,
		ध्रुव वर्ण *exec, ध्रुव वर्ण *ध्रुव *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	पूर्णाङ्क nkilled = 0;
	पूर्णाङ्क tkilled = 0;
	पूर्णाङ्क nrunning = 0;
	long nloops, nsecs;
	संरचना timespec ts;
	ध्रुव वर्ण *ध्रुव *p;
	bool progressed = false;

	जइ (!(pid > 0 || exec || uid || (argv && *argv)))
		प्रत्यागम 0;

	जइ (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	जइ (pid > 0) {
		जइ (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} अण्णहा {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	जइ (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	जइ (argv && *argv) {
		einfovn("Will stop processes of `");
		जइ (rc_yesno(getenv("EINFO_VERBOSE"))) {
			प्रति (p = argv; p && *p; p++) {
				जइ (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	जाव (item) {
		विकल्प (item->type) {
		पक्ष SC_GOTO:
			item = item->gotoitem;
			अनुवर्त;

		पक्ष SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->मानम्, group,
					test, quiet);
			जइ (nkilled == 0) {
				जइ (tkilled == 0) {
					जइ (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				प्रत्यागम tkilled;
			}
			अण्णहा जइ (nkilled == -1)
				प्रत्यागम 0;

			tkilled += nkilled;
			भङ्ग;
		पक्ष SC_FOREVER:
		पक्ष SC_TIMEOUT:
			जइ (item->type == SC_TIMEOUT && item->मानम् < 1) {
				item = रिक्त;
				भङ्ग;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			प्रति (nsecs = 0; item->type == SC_FOREVER || nsecs < item->मानम्; nsecs++) {
				प्रति (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					जइ ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						प्रत्यागम 0;


					जइ (nanosleep(&ts, रिक्त) == -1) {
						जइ (progressed) {
							printf("\n");
							progressed = false;
						}
						जइ (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							प्रत्यागम 0;
						}
					}
				}
				जइ (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			भङ्ग;
		स्वभाव:
			जइ (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			प्रत्यागम 0;
		}

		जइ (item)
			item = TAILQ_NEXT(item, entries);
	}

	जइ (test || (tkilled > 0 && nrunning == 0))
		प्रत्यागम nkilled;

	जइ (progressed)
		printf("\n");
	जइ (!quiet) {
		जइ (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		अण्णहा
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	प्रत्यागम -nrunning;
}
