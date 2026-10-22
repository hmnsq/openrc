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
	पूर्णाङ्क मूल्य;
	संरचना scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

स्थिर TAILQ_HEAD(, scheduleitem) schedule;

शून्य free_schedulelist(शून्य)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	यावत् (s1) {
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

	यदि (!sig || *sig == '\0')
		प्रत्यागम -1;

	यदि (sscanf(sig, "%u", &i) == 1) {
		यदि (i < NSIG)
			प्रत्यागम i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	यदि (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	अन्यथा
		s = रिक्त;

	प्रति (i = 0; i < ARRAY_SIZE(signallist); ++i)
		यदि (strcmp(sig, signallist[i].name) == 0 ||
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

	item->मूल्य = 0;
	item->gotoitem = रिक्त;
	यदि (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	अन्यथा यदि (isdigit((unsigned वर्ण)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		यदि (sscanf(string, "%d", &item->मूल्य) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} अन्यथा यदि ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->मूल्य = (पूर्णाङ्क)sig;
	} अन्यथा
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	प्रत्यागम item;
}

शून्य parse_schedule(ध्रुव वर्ण *applet, ध्रुव वर्ण *string, पूर्णाङ्क timeout)
{
	वर्ण buffer[20];
	ध्रुव वर्ण *slash;
	पूर्णाङ्क गिनती = 0;
	SCHEDULEITEM *repeatat = रिक्त;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	यदि (string)
		प्रति (slash = string; *slash; slash++)
			यदि (*slash == '/')
				गिनती++;

	free_schedulelist();

	यदि (गिनती == 0) {
		item = xmalloc(परिमाण(*item));
		item->type = SC_SIGNAL;
		item->मूल्य = timeout;
		item->gotoitem = रिक्त;
		TAILQ_जोड़ो_TAIL(&schedule, item, entries);

		item = xmalloc(परिमाण(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = रिक्त;
		TAILQ_जोड़ो_TAIL(&schedule, item, entries);
		यदि (string) {
			यदि (sscanf(string, "%d", &item->मूल्य) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} अन्यथा
			item->मूल्य = 5;

		प्रत्यागम;
	}

	यावत् (string != रिक्त) {
		यदि ((slash = strchr(string, '/')))
			len = slash - string;
		अन्यथा
			len = strlen(string);

		यदि (len >= (ptrdiff_t)परिमाण(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : रिक्त;

		item = parse_schedule_item(applet, buffer);
		TAILQ_जोड़ो_TAIL(&schedule, item, entries);
		यदि (item->type == SC_FOREVER) {
			यदि (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			अनुवर्त;
		}
	}

	यदि (repeatat) {
		item = xmalloc(परिमाण(*item));
		item->type = SC_GOTO;
		item->मूल्य = 0;
		item->gotoitem = repeatat;
		TAILQ_जोड़ो_TAIL(&schedule, item, entries);
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
	यदि (group)
		kind = "GID";

	यदि (pid > 0)
		pids = rc_find_pids(रिक्त, रिक्त, 0, pid);
	अन्यथा
		pids = rc_find_pids(exec, argv, uid, 0);

	यदि (!pids)
		प्रत्यागम 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		यदि (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} अन्यथा {
			यदि (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				यदि (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			यदि (group)
				target = -pi->pid;
			अन्यथा
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			यदि (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			अन्यथा यदि (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			यदि (!killed) {
				nkilled = -1;
			} अन्यथा {
				यदि (nkilled != -1)
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

	यदि (!(pid > 0 || exec || uid || (argv && *argv)))
		प्रत्यागम 0;

	यदि (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	यदि (pid > 0) {
		यदि (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} अन्यथा {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	यदि (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	यदि (argv && *argv) {
		einfovn("Will stop processes of `");
		यदि (rc_yesno(getenv("EINFO_VERBOSE"))) {
			प्रति (p = argv; p && *p; p++) {
				यदि (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	यावत् (item) {
		विकल्प (item->type) {
		पक्ष SC_GOTO:
			item = item->gotoitem;
			अनुवर्त;

		पक्ष SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->मूल्य, group,
					test, quiet);
			यदि (nkilled == 0) {
				यदि (tkilled == 0) {
					यदि (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				प्रत्यागम tkilled;
			}
			अन्यथा यदि (nkilled == -1)
				प्रत्यागम 0;

			tkilled += nkilled;
			भङ्ग;
		पक्ष SC_FOREVER:
		पक्ष SC_TIMEOUT:
			यदि (item->type == SC_TIMEOUT && item->मूल्य < 1) {
				item = रिक्त;
				भङ्ग;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			प्रति (nsecs = 0; item->type == SC_FOREVER || nsecs < item->मूल्य; nsecs++) {
				प्रति (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					यदि ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						प्रत्यागम 0;


					यदि (nanosleep(&ts, रिक्त) == -1) {
						यदि (progressed) {
							printf("\n");
							progressed = false;
						}
						यदि (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							प्रत्यागम 0;
						}
					}
				}
				यदि (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			भङ्ग;
		स्वभाव:
			यदि (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			प्रत्यागम 0;
		}

		यदि (item)
			item = TAILQ_NEXT(item, entries);
	}

	यदि (test || (tkilled > 0 && nrunning == 0))
		प्रत्यागम nkilled;

	यदि (progressed)
		printf("\n");
	यदि (!quiet) {
		यदि (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		अन्यथा
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	प्रत्यागम -nrunning;
}
