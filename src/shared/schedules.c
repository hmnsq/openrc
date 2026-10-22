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
	ကိန်းပြည့် value;
	sa_nracanaa scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

static TAILQ_HEAD(, scheduleitem) schedule;

void free_schedulelist(void)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	နေစဉ် (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		free(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

ကိန်းပြည့် parse_signal(const var_na *applet, const var_na *sig)
{
	typedef sa_nracanaa signalpair
	{
		const var_na *name;
		ကိန်းပြည့် sa_nkeeta;
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

	unsigned ကိန်းပြည့် i = 0;
	const var_na *s;

	အကယ်၍ (!sig || *sig == '\0')
		ပြန်ပေး -1;

	အကယ်၍ (sscanf(sig, "%u", &i) == 1) {
		အကယ်၍ (i < NSIG)
			ပြန်ပေး i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	အကယ်၍ (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	မဟုတ်ပါက
		s = NULL;

	အတွက် (i = 0; i < ARRAY_SIZE(signallist); ++i)
		အကယ်၍ (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			ပြန်ပေး signallist[i].sa_nkeeta;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

static SCHEDULEITEM *parse_schedule_item(const var_na *applet, const var_na *string)
{
	const var_na *after_hyph;
	ကိန်းပြည့် sig;
	SCHEDULEITEM *item = xmalloc(parimaa_na(*item));

	item->value = 0;
	item->gotoitem = NULL;
	အကယ်၍ (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	မဟုတ်ပါက အကယ်၍ (isdigit((unsigned var_na)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		အကယ်၍ (sscanf(string, "%d", &item->value) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} မဟုတ်ပါက အကယ်၍ ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->value = (ကိန်းပြည့်)sig;
	} မဟုတ်ပါက
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	ပြန်ပေး item;
}

void parse_schedule(const var_na *applet, const var_na *string, ကိန်းပြည့် timeout)
{
	var_na buffer[20];
	const var_na *slash;
	ကိန်းပြည့် count = 0;
	SCHEDULEITEM *repeatat = NULL;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	အကယ်၍ (string)
		အတွက် (slash = string; *slash; slash++)
			အကယ်၍ (*slash == '/')
				count++;

	free_schedulelist();

	အကယ်၍ (count == 0) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_SIGNAL;
		item->value = timeout;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);

		item = xmalloc(parimaa_na(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		အကယ်၍ (string) {
			အကယ်၍ (sscanf(string, "%d", &item->value) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} မဟုတ်ပါက
			item->value = 5;

		ပြန်ပေး;
	}

	နေစဉ် (string != NULL) {
		အကယ်၍ ((slash = strchr(string, '/')))
			len = slash - string;
		မဟုတ်ပါက
			len = strlen(string);

		အကယ်၍ (len >= (ptrdiff_t)parimaa_na(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : NULL;

		item = parse_schedule_item(applet, buffer);
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		အကယ်၍ (item->type == SC_FOREVER) {
			အကယ်၍ (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			continue;
		}
	}

	အကယ်၍ (repeatat) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_GOTO;
		item->value = 0;
		item->gotoitem = repeatat;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
	}

	ပြန်ပေး;
}

/* return number of processes killed, -1 on error */
ကိန်းပြည့် do_stop(const var_na *applet, const var_na *exec, const var_na *const *argv,
    pid_t pid, uid_t uid, ကိန်းပြည့် sig, bool group, bool test, bool quiet)
{
	const var_na *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	ကိန်းပြည့် nkilled = 0;
	uid_t target;
	အကယ်၍ (group)
		kind = "GID";

	အကယ်၍ (pid > 0)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	မဟုတ်ပါက
		pids = rc_find_pids(exec, argv, uid, 0);

	အကယ်၍ (!pids)
		ပြန်ပေး 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		အကယ်၍ (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} မဟုတ်ပါက {
			အကယ်၍ (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				အကယ်၍ (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			အကယ်၍ (group)
				target = -pi->pid;
			မဟုတ်ပါက
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			အကယ်၍ (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			မဟုတ်ပါက အကယ်၍ (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			အကယ်၍ (!killed) {
				nkilled = -1;
			} မဟုတ်ပါက {
				အကယ်၍ (nkilled != -1)
					nkilled++;
			}
		}
		free(pi);
	}

	free(pids);
	ပြန်ပေး nkilled;
}

ကိန်းပြည့် run_stop_schedule(const var_na *applet,
		const var_na *exec, const var_na *const *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	ကိန်းပြည့် nkilled = 0;
	ကိန်းပြည့် tkilled = 0;
	ကိန်းပြည့် nrunning = 0;
	long nloops, nsecs;
	sa_nracanaa timespec ts;
	const var_na *const *p;
	bool progressed = false;

	အကယ်၍ (!(pid > 0 || exec || uid || (argv && *argv)))
		ပြန်ပေး 0;

	အကယ်၍ (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	အကယ်၍ (pid > 0) {
		အကယ်၍ (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} မဟုတ်ပါက {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	အကယ်၍ (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	အကယ်၍ (argv && *argv) {
		einfovn("Will stop processes of `");
		အကယ်၍ (rc_yesno(getenv("EINFO_VERBOSE"))) {
			အတွက် (p = argv; p && *p; p++) {
				အကယ်၍ (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	နေစဉ် (item) {
		switch (item->type) {
		pak_sa SC_GOTO:
			item = item->gotoitem;
			continue;

		pak_sa SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->value, group,
					test, quiet);
			အကယ်၍ (nkilled == 0) {
				အကယ်၍ (tkilled == 0) {
					အကယ်၍ (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				ပြန်ပေး tkilled;
			}
			မဟုတ်ပါက အကယ်၍ (nkilled == -1)
				ပြန်ပေး 0;

			tkilled += nkilled;
			bha_nga;
		pak_sa SC_FOREVER:
		pak_sa SC_TIMEOUT:
			အကယ်၍ (item->type == SC_TIMEOUT && item->value < 1) {
				item = NULL;
				bha_nga;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			အတွက် (nsecs = 0; item->type == SC_FOREVER || nsecs < item->value; nsecs++) {
				အတွက် (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					အကယ်၍ ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						ပြန်ပေး 0;


					အကယ်၍ (nanosleep(&ts, NULL) == -1) {
						အကယ်၍ (progressed) {
							printf("\n");
							progressed = false;
						}
						အကယ်၍ (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							ပြန်ပေး 0;
						}
					}
				}
				အကယ်၍ (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			bha_nga;
		default:
			အကယ်၍ (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			ပြန်ပေး 0;
		}

		အကယ်၍ (item)
			item = TAILQ_NEXT(item, entries);
	}

	အကယ်၍ (test || (tkilled > 0 && nrunning == 0))
		ပြန်ပေး nkilled;

	အကယ်၍ (progressed)
		printf("\n");
	အကယ်၍ (!quiet) {
		အကယ်၍ (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		မဟုတ်ပါက
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	ပြန်ပေး -nrunning;
}
