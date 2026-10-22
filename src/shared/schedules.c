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

typedef sa_nracanaa scheduleitem {
	ga_nanaa {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	จำนวนเต็ม value;
	sa_nracanaa scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

static TAILQ_HEAD(, scheduleitem) schedule;

void free_schedulelist(void)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	ขณะ (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		free(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

จำนวนเต็ม parse_signal(const var_na *applet, const var_na *sig)
{
	typedef sa_nracanaa signalpair
	{
		const var_na *name;
		จำนวนเต็ม sa_nkeeta;
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

	unsigned จำนวนเต็ม i = 0;
	const var_na *s;

	ถ้า (!sig || *sig == '\0')
		คืน -1;

	ถ้า (sscanf(sig, "%u", &i) == 1) {
		ถ้า (i < NSIG)
			คืน i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	ถ้า (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	มิฉะนั้น
		s = NULL;

	สำหรับ (i = 0; i < ARRAY_SIZE(signallist); ++i)
		ถ้า (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			คืน signallist[i].sa_nkeeta;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

static SCHEDULEITEM *parse_schedule_item(const var_na *applet, const var_na *string)
{
	const var_na *after_hyph;
	จำนวนเต็ม sig;
	SCHEDULEITEM *item = xmalloc(parimaa_na(*item));

	item->value = 0;
	item->gotoitem = NULL;
	ถ้า (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	มิฉะนั้น ถ้า (isdigit((unsigned var_na)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		ถ้า (sscanf(string, "%d", &item->value) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} มิฉะนั้น ถ้า ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->value = (จำนวนเต็ม)sig;
	} มิฉะนั้น
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	คืน item;
}

void parse_schedule(const var_na *applet, const var_na *string, จำนวนเต็ม timeout)
{
	var_na buffer[20];
	const var_na *slash;
	จำนวนเต็ม count = 0;
	SCHEDULEITEM *repeatat = NULL;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	ถ้า (string)
		สำหรับ (slash = string; *slash; slash++)
			ถ้า (*slash == '/')
				count++;

	free_schedulelist();

	ถ้า (count == 0) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_SIGNAL;
		item->value = timeout;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);

		item = xmalloc(parimaa_na(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		ถ้า (string) {
			ถ้า (sscanf(string, "%d", &item->value) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} มิฉะนั้น
			item->value = 5;

		คืน;
	}

	ขณะ (string != NULL) {
		ถ้า ((slash = strchr(string, '/')))
			len = slash - string;
		มิฉะนั้น
			len = strlen(string);

		ถ้า (len >= (ptrdiff_t)parimaa_na(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : NULL;

		item = parse_schedule_item(applet, buffer);
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		ถ้า (item->type == SC_FOREVER) {
			ถ้า (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			continue;
		}
	}

	ถ้า (repeatat) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_GOTO;
		item->value = 0;
		item->gotoitem = repeatat;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
	}

	คืน;
}

/* return number of processes killed, -1 on error */
จำนวนเต็ม do_stop(const var_na *applet, const var_na *exec, const var_na *const *argv,
    pid_t pid, uid_t uid, จำนวนเต็ม sig, bool group, bool test, bool quiet)
{
	const var_na *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	จำนวนเต็ม nkilled = 0;
	uid_t target;
	ถ้า (group)
		kind = "GID";

	ถ้า (pid > 0)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	มิฉะนั้น
		pids = rc_find_pids(exec, argv, uid, 0);

	ถ้า (!pids)
		คืน 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		ถ้า (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} มิฉะนั้น {
			ถ้า (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				ถ้า (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			ถ้า (group)
				target = -pi->pid;
			มิฉะนั้น
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			ถ้า (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			มิฉะนั้น ถ้า (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			ถ้า (!killed) {
				nkilled = -1;
			} มิฉะนั้น {
				ถ้า (nkilled != -1)
					nkilled++;
			}
		}
		free(pi);
	}

	free(pids);
	คืน nkilled;
}

จำนวนเต็ม run_stop_schedule(const var_na *applet,
		const var_na *exec, const var_na *const *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	จำนวนเต็ม nkilled = 0;
	จำนวนเต็ม tkilled = 0;
	จำนวนเต็ม nrunning = 0;
	long nloops, nsecs;
	sa_nracanaa timespec ts;
	const var_na *const *p;
	bool progressed = false;

	ถ้า (!(pid > 0 || exec || uid || (argv && *argv)))
		คืน 0;

	ถ้า (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	ถ้า (pid > 0) {
		ถ้า (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} มิฉะนั้น {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	ถ้า (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	ถ้า (argv && *argv) {
		einfovn("Will stop processes of `");
		ถ้า (rc_yesno(getenv("EINFO_VERBOSE"))) {
			สำหรับ (p = argv; p && *p; p++) {
				ถ้า (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	ขณะ (item) {
		switch (item->type) {
		pak_sa SC_GOTO:
			item = item->gotoitem;
			continue;

		pak_sa SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->value, group,
					test, quiet);
			ถ้า (nkilled == 0) {
				ถ้า (tkilled == 0) {
					ถ้า (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				คืน tkilled;
			}
			มิฉะนั้น ถ้า (nkilled == -1)
				คืน 0;

			tkilled += nkilled;
			bha_nga;
		pak_sa SC_FOREVER:
		pak_sa SC_TIMEOUT:
			ถ้า (item->type == SC_TIMEOUT && item->value < 1) {
				item = NULL;
				bha_nga;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			สำหรับ (nsecs = 0; item->type == SC_FOREVER || nsecs < item->value; nsecs++) {
				สำหรับ (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					ถ้า ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						คืน 0;


					ถ้า (nanosleep(&ts, NULL) == -1) {
						ถ้า (progressed) {
							printf("\n");
							progressed = false;
						}
						ถ้า (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							คืน 0;
						}
					}
				}
				ถ้า (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			bha_nga;
		default:
			ถ้า (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			คืน 0;
		}

		ถ้า (item)
			item = TAILQ_NEXT(item, entries);
	}

	ถ้า (test || (tkilled > 0 && nrunning == 0))
		คืน nkilled;

	ถ้า (progressed)
		printf("\n");
	ถ้า (!quiet) {
		ถ้า (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		มิฉะนั้น
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	คืน -nrunning;
}
