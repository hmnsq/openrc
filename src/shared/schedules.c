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
	ཧྲིལ value;
	sa_nracanaa scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

static TAILQ_HEAD(, scheduleitem) schedule;

void free_schedulelist(void)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	སྐབས (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		free(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

ཧྲིལ parse_signal(const var_na *applet, const var_na *sig)
{
	typedef sa_nracanaa signalpair
	{
		const var_na *name;
		ཧྲིལ sa_nkeeta;
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

	unsigned ཧྲིལ i = 0;
	const var_na *s;

	གལ་སྲིད (!sig || *sig == '\0')
		ལོག -1;

	གལ་སྲིད (sscanf(sig, "%u", &i) == 1) {
		གལ་སྲིད (i < NSIG)
			ལོག i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	གལ་སྲིད (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	དེ་མིན
		s = NULL;

	དོན་ལུ (i = 0; i < ARRAY_SIZE(signallist); ++i)
		གལ་སྲིད (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			ལོག signallist[i].sa_nkeeta;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

static SCHEDULEITEM *parse_schedule_item(const var_na *applet, const var_na *string)
{
	const var_na *after_hyph;
	ཧྲིལ sig;
	SCHEDULEITEM *item = xmalloc(parimaa_na(*item));

	item->value = 0;
	item->gotoitem = NULL;
	གལ་སྲིད (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	དེ་མིན གལ་སྲིད (isdigit((unsigned var_na)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		གལ་སྲིད (sscanf(string, "%d", &item->value) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} དེ་མིན གལ་སྲིད ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->value = (ཧྲིལ)sig;
	} དེ་མིན
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	ལོག item;
}

void parse_schedule(const var_na *applet, const var_na *string, ཧྲིལ timeout)
{
	var_na buffer[20];
	const var_na *slash;
	ཧྲིལ count = 0;
	SCHEDULEITEM *repeatat = NULL;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	གལ་སྲིད (string)
		དོན་ལུ (slash = string; *slash; slash++)
			གལ་སྲིད (*slash == '/')
				count++;

	free_schedulelist();

	གལ་སྲིད (count == 0) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_SIGNAL;
		item->value = timeout;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);

		item = xmalloc(parimaa_na(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		གལ་སྲིད (string) {
			གལ་སྲིད (sscanf(string, "%d", &item->value) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} དེ་མིན
			item->value = 5;

		ལོག;
	}

	སྐབས (string != NULL) {
		གལ་སྲིད ((slash = strchr(string, '/')))
			len = slash - string;
		དེ་མིན
			len = strlen(string);

		གལ་སྲིད (len >= (ptrdiff_t)parimaa_na(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : NULL;

		item = parse_schedule_item(applet, buffer);
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		གལ་སྲིད (item->type == SC_FOREVER) {
			གལ་སྲིད (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			continue;
		}
	}

	གལ་སྲིད (repeatat) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_GOTO;
		item->value = 0;
		item->gotoitem = repeatat;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
	}

	ལོག;
}

/* return number of processes killed, -1 on error */
ཧྲིལ do_stop(const var_na *applet, const var_na *exec, const var_na *const *argv,
    pid_t pid, uid_t uid, ཧྲིལ sig, bool group, bool test, bool quiet)
{
	const var_na *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	ཧྲིལ nkilled = 0;
	uid_t target;
	གལ་སྲིད (group)
		kind = "GID";

	གལ་སྲིད (pid > 0)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	དེ་མིན
		pids = rc_find_pids(exec, argv, uid, 0);

	གལ་སྲིད (!pids)
		ལོག 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		གལ་སྲིད (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} དེ་མིན {
			གལ་སྲིད (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				གལ་སྲིད (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			གལ་སྲིད (group)
				target = -pi->pid;
			དེ་མིན
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			གལ་སྲིད (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			དེ་མིན གལ་སྲིད (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			གལ་སྲིད (!killed) {
				nkilled = -1;
			} དེ་མིན {
				གལ་སྲིད (nkilled != -1)
					nkilled++;
			}
		}
		free(pi);
	}

	free(pids);
	ལོག nkilled;
}

ཧྲིལ run_stop_schedule(const var_na *applet,
		const var_na *exec, const var_na *const *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	ཧྲིལ nkilled = 0;
	ཧྲིལ tkilled = 0;
	ཧྲིལ nrunning = 0;
	long nloops, nsecs;
	sa_nracanaa timespec ts;
	const var_na *const *p;
	bool progressed = false;

	གལ་སྲིད (!(pid > 0 || exec || uid || (argv && *argv)))
		ལོག 0;

	གལ་སྲིད (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	གལ་སྲིད (pid > 0) {
		གལ་སྲིད (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} དེ་མིན {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	གལ་སྲིད (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	གལ་སྲིད (argv && *argv) {
		einfovn("Will stop processes of `");
		གལ་སྲིད (rc_yesno(getenv("EINFO_VERBOSE"))) {
			དོན་ལུ (p = argv; p && *p; p++) {
				གལ་སྲིད (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	སྐབས (item) {
		switch (item->type) {
		pak_sa SC_GOTO:
			item = item->gotoitem;
			continue;

		pak_sa SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->value, group,
					test, quiet);
			གལ་སྲིད (nkilled == 0) {
				གལ་སྲིད (tkilled == 0) {
					གལ་སྲིད (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				ལོག tkilled;
			}
			དེ་མིན གལ་སྲིད (nkilled == -1)
				ལོག 0;

			tkilled += nkilled;
			bha_nga;
		pak_sa SC_FOREVER:
		pak_sa SC_TIMEOUT:
			གལ་སྲིད (item->type == SC_TIMEOUT && item->value < 1) {
				item = NULL;
				bha_nga;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			དོན་ལུ (nsecs = 0; item->type == SC_FOREVER || nsecs < item->value; nsecs++) {
				དོན་ལུ (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					གལ་སྲིད ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						ལོག 0;


					གལ་སྲིད (nanosleep(&ts, NULL) == -1) {
						གལ་སྲིད (progressed) {
							printf("\n");
							progressed = false;
						}
						གལ་སྲིད (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							ལོག 0;
						}
					}
				}
				གལ་སྲིད (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			bha_nga;
		default:
			གལ་སྲིད (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			ལོག 0;
		}

		གལ་སྲིད (item)
			item = TAILQ_NEXT(item, entries);
	}

	གལ་སྲིད (test || (tkilled > 0 && nrunning == 0))
		ལོག nkilled;

	གལ་སྲིད (progressed)
		printf("\n");
	གལ་སྲིད (!quiet) {
		གལ་སྲིད (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		དེ་མིན
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	ལོག -nrunning;
}
