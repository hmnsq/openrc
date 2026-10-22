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
	ціле value;
	struct scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

static TAILQ_HEAD(, scheduleitem) schedule;

void free_schedulelist(void)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	поки (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		free(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

ціле parse_signal(const char *applet, const char *sig)
{
	typedef struct signalpair
	{
		const char *name;
		ціле signal;
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

	unsigned ціле i = 0;
	const char *s;

	якщо (!sig || *sig == '\0')
		повернути -1;

	якщо (sscanf(sig, "%u", &i) == 1) {
		якщо (i < NSIG)
			повернути i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	якщо (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	інакше
		s = NULL;

	для (i = 0; i < ARRAY_SIZE(signallist); ++i)
		якщо (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			повернути signallist[i].signal;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

static SCHEDULEITEM *parse_schedule_item(const char *applet, const char *string)
{
	const char *after_hyph;
	ціле sig;
	SCHEDULEITEM *item = xmalloc(sizeof(*item));

	item->value = 0;
	item->gotoitem = NULL;
	якщо (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	інакше якщо (isdigit((unsigned char)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		якщо (sscanf(string, "%d", &item->value) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} інакше якщо ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->value = (ціле)sig;
	} інакше
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	повернути item;
}

void parse_schedule(const char *applet, const char *string, ціле timeout)
{
	char buffer[20];
	const char *slash;
	ціле count = 0;
	SCHEDULEITEM *repeatat = NULL;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	якщо (string)
		для (slash = string; *slash; slash++)
			якщо (*slash == '/')
				count++;

	free_schedulelist();

	якщо (count == 0) {
		item = xmalloc(sizeof(*item));
		item->type = SC_SIGNAL;
		item->value = timeout;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);

		item = xmalloc(sizeof(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		якщо (string) {
			якщо (sscanf(string, "%d", &item->value) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} інакше
			item->value = 5;

		повернути;
	}

	поки (string != NULL) {
		якщо ((slash = strchr(string, '/')))
			len = slash - string;
		інакше
			len = strlen(string);

		якщо (len >= (ptrdiff_t)sizeof(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : NULL;

		item = parse_schedule_item(applet, buffer);
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		якщо (item->type == SC_FOREVER) {
			якщо (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			continue;
		}
	}

	якщо (repeatat) {
		item = xmalloc(sizeof(*item));
		item->type = SC_GOTO;
		item->value = 0;
		item->gotoitem = repeatat;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
	}

	повернути;
}

/* return number of processes killed, -1 on error */
ціле do_stop(const char *applet, const char *exec, const char *const *argv,
    pid_t pid, uid_t uid, ціле sig, bool group, bool test, bool quiet)
{
	const char *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	ціле nkilled = 0;
	uid_t target;
	якщо (group)
		kind = "GID";

	якщо (pid > 0)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	інакше
		pids = rc_find_pids(exec, argv, uid, 0);

	якщо (!pids)
		повернути 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		якщо (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} інакше {
			якщо (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				якщо (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			якщо (group)
				target = -pi->pid;
			інакше
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			якщо (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			інакше якщо (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			якщо (!killed) {
				nkilled = -1;
			} інакше {
				якщо (nkilled != -1)
					nkilled++;
			}
		}
		free(pi);
	}

	free(pids);
	повернути nkilled;
}

ціле run_stop_schedule(const char *applet,
		const char *exec, const char *const *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	ціле nkilled = 0;
	ціле tkilled = 0;
	ціле nrunning = 0;
	long nloops, nsecs;
	struct timespec ts;
	const char *const *p;
	bool progressed = false;

	якщо (!(pid > 0 || exec || uid || (argv && *argv)))
		повернути 0;

	якщо (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	якщо (pid > 0) {
		якщо (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} інакше {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	якщо (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	якщо (argv && *argv) {
		einfovn("Will stop processes of `");
		якщо (rc_yesno(getenv("EINFO_VERBOSE"))) {
			для (p = argv; p && *p; p++) {
				якщо (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	поки (item) {
		switch (item->type) {
		case SC_GOTO:
			item = item->gotoitem;
			continue;

		case SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->value, group,
					test, quiet);
			якщо (nkilled == 0) {
				якщо (tkilled == 0) {
					якщо (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				повернути tkilled;
			}
			інакше якщо (nkilled == -1)
				повернути 0;

			tkilled += nkilled;
			break;
		case SC_FOREVER:
		case SC_TIMEOUT:
			якщо (item->type == SC_TIMEOUT && item->value < 1) {
				item = NULL;
				break;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			для (nsecs = 0; item->type == SC_FOREVER || nsecs < item->value; nsecs++) {
				для (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					якщо ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						повернути 0;


					якщо (nanosleep(&ts, NULL) == -1) {
						якщо (progressed) {
							printf("\n");
							progressed = false;
						}
						якщо (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							повернути 0;
						}
					}
				}
				якщо (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			break;
		default:
			якщо (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			повернути 0;
		}

		якщо (item)
			item = TAILQ_NEXT(item, entries);
	}

	якщо (test || (tkilled > 0 && nrunning == 0))
		повернути nkilled;

	якщо (progressed)
		printf("\n");
	якщо (!quiet) {
		якщо (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		інакше
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	повернути -nrunning;
}
