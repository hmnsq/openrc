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

	ၶၢဝ်း (s1) {
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

	ပေႃး (!sig || *sig == '\0')
		ပွၵ်ႈ -1;

	ပေႃး (sscanf(sig, "%u", &i) == 1) {
		ပေႃး (i < NSIG)
			ပွၵ်ႈ i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	ပေႃး (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	ပေႃးဢမ်ႇ
		s = NULL;

	for (i = 0; i < ARRAY_SIZE(signallist); ++i)
		ပေႃး (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			ပွၵ်ႈ signallist[i].sa_nkeeta;

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
	ပေႃး (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	ပေႃးဢမ်ႇ ပေႃး (isdigit((unsigned var_na)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		ပေႃး (sscanf(string, "%d", &item->value) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} ပေႃးဢမ်ႇ ပေႃး ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->value = (puur_naa_nka)sig;
	} ပေႃးဢမ်ႇ
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	ပွၵ်ႈ item;
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
	ပေႃး (string)
		for (slash = string; *slash; slash++)
			ပေႃး (*slash == '/')
				count++;

	free_schedulelist();

	ပေႃး (count == 0) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_SIGNAL;
		item->value = timeout;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);

		item = xmalloc(parimaa_na(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		ပေႃး (string) {
			ပေႃး (sscanf(string, "%d", &item->value) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} ပေႃးဢမ်ႇ
			item->value = 5;

		ပွၵ်ႈ;
	}

	ၶၢဝ်း (string != NULL) {
		ပေႃး ((slash = strchr(string, '/')))
			len = slash - string;
		ပေႃးဢမ်ႇ
			len = strlen(string);

		ပေႃး (len >= (ptrdiff_t)parimaa_na(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : NULL;

		item = parse_schedule_item(applet, buffer);
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		ပေႃး (item->type == SC_FOREVER) {
			ပေႃး (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			continue;
		}
	}

	ပေႃး (repeatat) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_GOTO;
		item->value = 0;
		item->gotoitem = repeatat;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
	}

	ပွၵ်ႈ;
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
	ပေႃး (group)
		kind = "GID";

	ပေႃး (pid > 0)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	ပေႃးဢမ်ႇ
		pids = rc_find_pids(exec, argv, uid, 0);

	ပေႃး (!pids)
		ပွၵ်ႈ 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		ပေႃး (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} ပေႃးဢမ်ႇ {
			ပေႃး (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				ပေႃး (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			ပေႃး (group)
				target = -pi->pid;
			ပေႃးဢမ်ႇ
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			ပေႃး (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			ပေႃးဢမ်ႇ ပေႃး (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			ပေႃး (!killed) {
				nkilled = -1;
			} ပေႃးဢမ်ႇ {
				ပေႃး (nkilled != -1)
					nkilled++;
			}
		}
		free(pi);
	}

	free(pids);
	ပွၵ်ႈ nkilled;
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

	ပေႃး (!(pid > 0 || exec || uid || (argv && *argv)))
		ပွၵ်ႈ 0;

	ပေႃး (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	ပေႃး (pid > 0) {
		ပေႃး (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} ပေႃးဢမ်ႇ {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	ပေႃး (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	ပေႃး (argv && *argv) {
		einfovn("Will stop processes of `");
		ပေႃး (rc_yesno(getenv("EINFO_VERBOSE"))) {
			for (p = argv; p && *p; p++) {
				ပေႃး (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	ၶၢဝ်း (item) {
		switch (item->type) {
		pak_sa SC_GOTO:
			item = item->gotoitem;
			continue;

		pak_sa SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->value, group,
					test, quiet);
			ပေႃး (nkilled == 0) {
				ပေႃး (tkilled == 0) {
					ပေႃး (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				ပွၵ်ႈ tkilled;
			}
			ပေႃးဢမ်ႇ ပေႃး (nkilled == -1)
				ပွၵ်ႈ 0;

			tkilled += nkilled;
			bha_nga;
		pak_sa SC_FOREVER:
		pak_sa SC_TIMEOUT:
			ပေႃး (item->type == SC_TIMEOUT && item->value < 1) {
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
					ပေႃး ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						ပွၵ်ႈ 0;


					ပေႃး (nanosleep(&ts, NULL) == -1) {
						ပေႃး (progressed) {
							printf("\n");
							progressed = false;
						}
						ပေႃး (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							ပွၵ်ႈ 0;
						}
					}
				}
				ပေႃး (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			bha_nga;
		default:
			ပေႃး (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			ပွၵ်ႈ 0;
		}

		ပေႃး (item)
			item = TAILQ_NEXT(item, entries);
	}

	ပေႃး (test || (tkilled > 0 && nrunning == 0))
		ပွၵ်ႈ nkilled;

	ပေႃး (progressed)
		printf("\n");
	ပေႃး (!quiet) {
		ပေႃး (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		ပေႃးဢမ်ႇ
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	ပွၵ်ႈ -nrunning;
}
