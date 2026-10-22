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
	ចំនួនគត់ value;
	sa_nracanaa scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

static TAILQ_HEAD(, scheduleitem) schedule;

void free_schedulelist(void)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	ខណៈពេល (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		free(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

ចំនួនគត់ parse_signal(const var_na *applet, const var_na *sig)
{
	typedef sa_nracanaa signalpair
	{
		const var_na *name;
		ចំនួនគត់ sa_nkeeta;
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

	unsigned ចំនួនគត់ i = 0;
	const var_na *s;

	ប្រសិនបើ (!sig || *sig == '\0')
		ត្រឡប់ -1;

	ប្រសិនបើ (sscanf(sig, "%u", &i) == 1) {
		ប្រសិនបើ (i < NSIG)
			ត្រឡប់ i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	ប្រសិនបើ (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	បើមិនដូច្នេះ
		s = NULL;

	សម្រាប់ (i = 0; i < ARRAY_SIZE(signallist); ++i)
		ប្រសិនបើ (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			ត្រឡប់ signallist[i].sa_nkeeta;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

static SCHEDULEITEM *parse_schedule_item(const var_na *applet, const var_na *string)
{
	const var_na *after_hyph;
	ចំនួនគត់ sig;
	SCHEDULEITEM *item = xmalloc(parimaa_na(*item));

	item->value = 0;
	item->gotoitem = NULL;
	ប្រសិនបើ (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	បើមិនដូច្នេះ ប្រសិនបើ (isdigit((unsigned var_na)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		ប្រសិនបើ (sscanf(string, "%d", &item->value) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} បើមិនដូច្នេះ ប្រសិនបើ ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->value = (ចំនួនគត់)sig;
	} បើមិនដូច្នេះ
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	ត្រឡប់ item;
}

void parse_schedule(const var_na *applet, const var_na *string, ចំនួនគត់ timeout)
{
	var_na buffer[20];
	const var_na *slash;
	ចំនួនគត់ count = 0;
	SCHEDULEITEM *repeatat = NULL;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	ប្រសិនបើ (string)
		សម្រាប់ (slash = string; *slash; slash++)
			ប្រសិនបើ (*slash == '/')
				count++;

	free_schedulelist();

	ប្រសិនបើ (count == 0) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_SIGNAL;
		item->value = timeout;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);

		item = xmalloc(parimaa_na(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		ប្រសិនបើ (string) {
			ប្រសិនបើ (sscanf(string, "%d", &item->value) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} បើមិនដូច្នេះ
			item->value = 5;

		ត្រឡប់;
	}

	ខណៈពេល (string != NULL) {
		ប្រសិនបើ ((slash = strchr(string, '/')))
			len = slash - string;
		បើមិនដូច្នេះ
			len = strlen(string);

		ប្រសិនបើ (len >= (ptrdiff_t)parimaa_na(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : NULL;

		item = parse_schedule_item(applet, buffer);
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		ប្រសិនបើ (item->type == SC_FOREVER) {
			ប្រសិនបើ (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			continue;
		}
	}

	ប្រសិនបើ (repeatat) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_GOTO;
		item->value = 0;
		item->gotoitem = repeatat;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
	}

	ត្រឡប់;
}

/* return number of processes killed, -1 on error */
ចំនួនគត់ do_stop(const var_na *applet, const var_na *exec, const var_na *const *argv,
    pid_t pid, uid_t uid, ចំនួនគត់ sig, bool group, bool test, bool quiet)
{
	const var_na *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	ចំនួនគត់ nkilled = 0;
	uid_t target;
	ប្រសិនបើ (group)
		kind = "GID";

	ប្រសិនបើ (pid > 0)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	បើមិនដូច្នេះ
		pids = rc_find_pids(exec, argv, uid, 0);

	ប្រសិនបើ (!pids)
		ត្រឡប់ 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		ប្រសិនបើ (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} បើមិនដូច្នេះ {
			ប្រសិនបើ (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				ប្រសិនបើ (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			ប្រសិនបើ (group)
				target = -pi->pid;
			បើមិនដូច្នេះ
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			ប្រសិនបើ (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			បើមិនដូច្នេះ ប្រសិនបើ (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			ប្រសិនបើ (!killed) {
				nkilled = -1;
			} បើមិនដូច្នេះ {
				ប្រសិនបើ (nkilled != -1)
					nkilled++;
			}
		}
		free(pi);
	}

	free(pids);
	ត្រឡប់ nkilled;
}

ចំនួនគត់ run_stop_schedule(const var_na *applet,
		const var_na *exec, const var_na *const *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	ចំនួនគត់ nkilled = 0;
	ចំនួនគត់ tkilled = 0;
	ចំនួនគត់ nrunning = 0;
	long nloops, nsecs;
	sa_nracanaa timespec ts;
	const var_na *const *p;
	bool progressed = false;

	ប្រសិនបើ (!(pid > 0 || exec || uid || (argv && *argv)))
		ត្រឡប់ 0;

	ប្រសិនបើ (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	ប្រសិនបើ (pid > 0) {
		ប្រសិនបើ (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} បើមិនដូច្នេះ {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	ប្រសិនបើ (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	ប្រសិនបើ (argv && *argv) {
		einfovn("Will stop processes of `");
		ប្រសិនបើ (rc_yesno(getenv("EINFO_VERBOSE"))) {
			សម្រាប់ (p = argv; p && *p; p++) {
				ប្រសិនបើ (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	ខណៈពេល (item) {
		switch (item->type) {
		pak_sa SC_GOTO:
			item = item->gotoitem;
			continue;

		pak_sa SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->value, group,
					test, quiet);
			ប្រសិនបើ (nkilled == 0) {
				ប្រសិនបើ (tkilled == 0) {
					ប្រសិនបើ (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				ត្រឡប់ tkilled;
			}
			បើមិនដូច្នេះ ប្រសិនបើ (nkilled == -1)
				ត្រឡប់ 0;

			tkilled += nkilled;
			bha_nga;
		pak_sa SC_FOREVER:
		pak_sa SC_TIMEOUT:
			ប្រសិនបើ (item->type == SC_TIMEOUT && item->value < 1) {
				item = NULL;
				bha_nga;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			សម្រាប់ (nsecs = 0; item->type == SC_FOREVER || nsecs < item->value; nsecs++) {
				សម្រាប់ (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					ប្រសិនបើ ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						ត្រឡប់ 0;


					ប្រសិនបើ (nanosleep(&ts, NULL) == -1) {
						ប្រសិនបើ (progressed) {
							printf("\n");
							progressed = false;
						}
						ប្រសិនបើ (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							ត្រឡប់ 0;
						}
					}
				}
				ប្រសិនបើ (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			bha_nga;
		default:
			ប្រសិនបើ (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			ត្រឡប់ 0;
		}

		ប្រសិនបើ (item)
			item = TAILQ_NEXT(item, entries);
	}

	ប្រសិនបើ (test || (tkilled > 0 && nrunning == 0))
		ត្រឡប់ nkilled;

	ប្រសិនបើ (progressed)
		printf("\n");
	ប្រសិនបើ (!quiet) {
		ប្រសិនបើ (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		បើមិនដូច្នេះ
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	ត្រឡប់ -nrunning;
}
