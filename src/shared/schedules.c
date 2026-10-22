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

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <sys/types.h>

#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "schedules.h"
#include "helpers.h"

typedef struct scheduleitem {
	enum {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	정수 value;
	struct scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

static TAILQ_HEAD(, scheduleitem) schedule;

void free_schedulelist(void)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	동안 (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		free(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

정수 parse_signal(const char *applet, const char *sig)
{
	typedef struct signalpair
	{
		const char *name;
		정수 signal;
	} SIGNALPAIR;

#define signalpair_item(name) { #name, SIG##name },

	static const SIGNALPAIR signallist[] = {
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

	unsigned 정수 i = 0;
	const char *s;

	만약 (!sig || *sig == '\0')
		반환 -1;

	만약 (sscanf(sig, "%u", &i) == 1) {
		만약 (i < NSIG)
			반환 i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	만약 (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	아니면
		s = NULL;

	위해 (i = 0; i < ARRAY_SIZE(signallist); ++i)
		만약 (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			반환 signallist[i].signal;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

static SCHEDULEITEM *parse_schedule_item(const char *applet, const char *string)
{
	const char *after_hyph;
	정수 sig;
	SCHEDULEITEM *item = xmalloc(sizeof(*item));

	item->value = 0;
	item->gotoitem = NULL;
	만약 (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	아니면 만약 (isdigit((unsigned char)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		만약 (sscanf(string, "%d", &item->value) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} 아니면 만약 ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->value = (정수)sig;
	} 아니면
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	반환 item;
}

void parse_schedule(const char *applet, const char *string, 정수 timeout)
{
	char buffer[20];
	const char *slash;
	정수 count = 0;
	SCHEDULEITEM *repeatat = NULL;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	만약 (string)
		위해 (slash = string; *slash; slash++)
			만약 (*slash == '/')
				count++;

	free_schedulelist();

	만약 (count == 0) {
		item = xmalloc(sizeof(*item));
		item->type = SC_SIGNAL;
		item->value = timeout;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);

		item = xmalloc(sizeof(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		만약 (string) {
			만약 (sscanf(string, "%d", &item->value) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} 아니면
			item->value = 5;

		반환;
	}

	동안 (string != NULL) {
		만약 ((slash = strchr(string, '/')))
			len = slash - string;
		아니면
			len = strlen(string);

		만약 (len >= (ptrdiff_t)sizeof(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : NULL;

		item = parse_schedule_item(applet, buffer);
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		만약 (item->type == SC_FOREVER) {
			만약 (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			continue;
		}
	}

	만약 (repeatat) {
		item = xmalloc(sizeof(*item));
		item->type = SC_GOTO;
		item->value = 0;
		item->gotoitem = repeatat;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
	}

	반환;
}

/* return number of processes killed, -1 on error */
정수 do_stop(const char *applet, const char *exec, const char *const *argv,
    pid_t pid, uid_t uid, 정수 sig, bool group, bool test, bool quiet)
{
	const char *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	정수 nkilled = 0;
	uid_t target;
	만약 (group)
		kind = "GID";

	만약 (pid > 0)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	아니면
		pids = rc_find_pids(exec, argv, uid, 0);

	만약 (!pids)
		반환 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		만약 (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} 아니면 {
			만약 (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				만약 (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			만약 (group)
				target = -pi->pid;
			아니면
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			만약 (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			아니면 만약 (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			만약 (!killed) {
				nkilled = -1;
			} 아니면 {
				만약 (nkilled != -1)
					nkilled++;
			}
		}
		free(pi);
	}

	free(pids);
	반환 nkilled;
}

정수 run_stop_schedule(const char *applet,
		const char *exec, const char *const *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	정수 nkilled = 0;
	정수 tkilled = 0;
	정수 nrunning = 0;
	long nloops, nsecs;
	struct timespec ts;
	const char *const *p;
	bool progressed = false;

	만약 (!(pid > 0 || exec || uid || (argv && *argv)))
		반환 0;

	만약 (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	만약 (pid > 0) {
		만약 (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} 아니면 {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	만약 (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	만약 (argv && *argv) {
		einfovn("Will stop processes of `");
		만약 (rc_yesno(getenv("EINFO_VERBOSE"))) {
			위해 (p = argv; p && *p; p++) {
				만약 (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	동안 (item) {
		switch (item->type) {
		case SC_GOTO:
			item = item->gotoitem;
			continue;

		case SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->value, group,
					test, quiet);
			만약 (nkilled == 0) {
				만약 (tkilled == 0) {
					만약 (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				반환 tkilled;
			}
			아니면 만약 (nkilled == -1)
				반환 0;

			tkilled += nkilled;
			break;
		case SC_FOREVER:
		case SC_TIMEOUT:
			만약 (item->type == SC_TIMEOUT && item->value < 1) {
				item = NULL;
				break;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			위해 (nsecs = 0; item->type == SC_FOREVER || nsecs < item->value; nsecs++) {
				위해 (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					만약 ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						반환 0;


					만약 (nanosleep(&ts, NULL) == -1) {
						만약 (progressed) {
							printf("\n");
							progressed = false;
						}
						만약 (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							반환 0;
						}
					}
				}
				만약 (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			break;
		default:
			만약 (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			반환 0;
		}

		만약 (item)
			item = TAILQ_NEXT(item, entries);
	}

	만약 (test || (tkilled > 0 && nrunning == 0))
		반환 nkilled;

	만약 (progressed)
		printf("\n");
	만약 (!quiet) {
		만약 (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		아니면
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	반환 -nrunning;
}
