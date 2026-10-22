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
	puur_naa_nka value;
	sa_nracanaa scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

static TAILQ_HEAD(, scheduleitem) schedule;

void free_schedulelist(void)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	ᨀᨛ (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		free(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

puur_naa_nka parse_signal(const var_na *applet, const var_na *sig)
{
	typedef sa_nracanaa signalpair
	{
		const var_na *name;
		puur_naa_nka sa_nkeeta;
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

	unsigned puur_naa_nka i = 0;
	const var_na *s;

	ᨀᨘ (!sig || *sig == '\0')
		ᨀᨘᨒ -1;

	ᨀᨘ (sscanf(sig, "%u", &i) == 1) {
		ᨀᨘ (i < NSIG)
			ᨀᨘᨒ i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	ᨀᨘ (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	ᨀᨘᨁ
		s = NULL;

	for (i = 0; i < ARRAY_SIZE(signallist); ++i)
		ᨀᨘ (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			ᨀᨘᨒ signallist[i].sa_nkeeta;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

static SCHEDULEITEM *parse_schedule_item(const var_na *applet, const var_na *string)
{
	const var_na *after_hyph;
	puur_naa_nka sig;
	SCHEDULEITEM *item = xmalloc(parimaa_na(*item));

	item->value = 0;
	item->gotoitem = NULL;
	ᨀᨘ (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	ᨀᨘᨁ ᨀᨘ (isdigit((unsigned var_na)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		ᨀᨘ (sscanf(string, "%d", &item->value) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} ᨀᨘᨁ ᨀᨘ ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->value = (puur_naa_nka)sig;
	} ᨀᨘᨁ
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	ᨀᨘᨒ item;
}

void parse_schedule(const var_na *applet, const var_na *string, puur_naa_nka timeout)
{
	var_na buffer[20];
	const var_na *slash;
	puur_naa_nka count = 0;
	SCHEDULEITEM *repeatat = NULL;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	ᨀᨘ (string)
		for (slash = string; *slash; slash++)
			ᨀᨘ (*slash == '/')
				count++;

	free_schedulelist();

	ᨀᨘ (count == 0) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_SIGNAL;
		item->value = timeout;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);

		item = xmalloc(parimaa_na(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		ᨀᨘ (string) {
			ᨀᨘ (sscanf(string, "%d", &item->value) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} ᨀᨘᨁ
			item->value = 5;

		ᨀᨘᨒ;
	}

	ᨀᨛ (string != NULL) {
		ᨀᨘ ((slash = strchr(string, '/')))
			len = slash - string;
		ᨀᨘᨁ
			len = strlen(string);

		ᨀᨘ (len >= (ptrdiff_t)parimaa_na(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : NULL;

		item = parse_schedule_item(applet, buffer);
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		ᨀᨘ (item->type == SC_FOREVER) {
			ᨀᨘ (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			continue;
		}
	}

	ᨀᨘ (repeatat) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_GOTO;
		item->value = 0;
		item->gotoitem = repeatat;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
	}

	ᨀᨘᨒ;
}

/* return number of processes killed, -1 on error */
puur_naa_nka do_stop(const var_na *applet, const var_na *exec, const var_na *const *argv,
    pid_t pid, uid_t uid, puur_naa_nka sig, bool group, bool test, bool quiet)
{
	const var_na *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	puur_naa_nka nkilled = 0;
	uid_t target;
	ᨀᨘ (group)
		kind = "GID";

	ᨀᨘ (pid > 0)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	ᨀᨘᨁ
		pids = rc_find_pids(exec, argv, uid, 0);

	ᨀᨘ (!pids)
		ᨀᨘᨒ 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		ᨀᨘ (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} ᨀᨘᨁ {
			ᨀᨘ (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				ᨀᨘ (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			ᨀᨘ (group)
				target = -pi->pid;
			ᨀᨘᨁ
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			ᨀᨘ (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			ᨀᨘᨁ ᨀᨘ (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			ᨀᨘ (!killed) {
				nkilled = -1;
			} ᨀᨘᨁ {
				ᨀᨘ (nkilled != -1)
					nkilled++;
			}
		}
		free(pi);
	}

	free(pids);
	ᨀᨘᨒ nkilled;
}

puur_naa_nka run_stop_schedule(const var_na *applet,
		const var_na *exec, const var_na *const *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	puur_naa_nka nkilled = 0;
	puur_naa_nka tkilled = 0;
	puur_naa_nka nrunning = 0;
	long nloops, nsecs;
	sa_nracanaa timespec ts;
	const var_na *const *p;
	bool progressed = false;

	ᨀᨘ (!(pid > 0 || exec || uid || (argv && *argv)))
		ᨀᨘᨒ 0;

	ᨀᨘ (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	ᨀᨘ (pid > 0) {
		ᨀᨘ (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} ᨀᨘᨁ {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	ᨀᨘ (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	ᨀᨘ (argv && *argv) {
		einfovn("Will stop processes of `");
		ᨀᨘ (rc_yesno(getenv("EINFO_VERBOSE"))) {
			for (p = argv; p && *p; p++) {
				ᨀᨘ (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	ᨀᨛ (item) {
		switch (item->type) {
		pak_sa SC_GOTO:
			item = item->gotoitem;
			continue;

		pak_sa SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->value, group,
					test, quiet);
			ᨀᨘ (nkilled == 0) {
				ᨀᨘ (tkilled == 0) {
					ᨀᨘ (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				ᨀᨘᨒ tkilled;
			}
			ᨀᨘᨁ ᨀᨘ (nkilled == -1)
				ᨀᨘᨒ 0;

			tkilled += nkilled;
			bha_nga;
		pak_sa SC_FOREVER:
		pak_sa SC_TIMEOUT:
			ᨀᨘ (item->type == SC_TIMEOUT && item->value < 1) {
				item = NULL;
				bha_nga;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			for (nsecs = 0; item->type == SC_FOREVER || nsecs < item->value; nsecs++) {
				for (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					ᨀᨘ ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						ᨀᨘᨒ 0;


					ᨀᨘ (nanosleep(&ts, NULL) == -1) {
						ᨀᨘ (progressed) {
							printf("\n");
							progressed = false;
						}
						ᨀᨘ (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							ᨀᨘᨒ 0;
						}
					}
				}
				ᨀᨘ (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			bha_nga;
		default:
			ᨀᨘ (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			ᨀᨘᨒ 0;
		}

		ᨀᨘ (item)
			item = TAILQ_NEXT(item, entries);
	}

	ᨀᨘ (test || (tkilled > 0 && nrunning == 0))
		ᨀᨘᨒ nkilled;

	ᨀᨘ (progressed)
		printf("\n");
	ᨀᨘ (!quiet) {
		ᨀᨘ (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		ᨀᨘᨁ
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	ᨀᨘᨒ -nrunning;
}
