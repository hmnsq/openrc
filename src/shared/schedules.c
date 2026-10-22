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
	ຈຳນວນເຕັມ value;
	sa_nracanaa scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

static TAILQ_HEAD(, scheduleitem) schedule;

void free_schedulelist(void)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	ໃນຂະນະທີ່ (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		free(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

ຈຳນວນເຕັມ parse_signal(const var_na *applet, const var_na *sig)
{
	typedef sa_nracanaa signalpair
	{
		const var_na *name;
		ຈຳນວນເຕັມ sa_nkeeta;
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

	unsigned ຈຳນວນເຕັມ i = 0;
	const var_na *s;

	ຖ້າ (!sig || *sig == '\0')
		ສົ່ງຄືນ -1;

	ຖ້າ (sscanf(sig, "%u", &i) == 1) {
		ຖ້າ (i < NSIG)
			ສົ່ງຄືນ i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	ຖ້າ (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	ບໍ່ດັ່ງນັ້ນ
		s = NULL;

	ສຳລັບ (i = 0; i < ARRAY_SIZE(signallist); ++i)
		ຖ້າ (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			ສົ່ງຄືນ signallist[i].sa_nkeeta;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

static SCHEDULEITEM *parse_schedule_item(const var_na *applet, const var_na *string)
{
	const var_na *after_hyph;
	ຈຳນວນເຕັມ sig;
	SCHEDULEITEM *item = xmalloc(parimaa_na(*item));

	item->value = 0;
	item->gotoitem = NULL;
	ຖ້າ (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (isdigit((unsigned var_na)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		ຖ້າ (sscanf(string, "%d", &item->value) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->value = (ຈຳນວນເຕັມ)sig;
	} ບໍ່ດັ່ງນັ້ນ
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	ສົ່ງຄືນ item;
}

void parse_schedule(const var_na *applet, const var_na *string, ຈຳນວນເຕັມ timeout)
{
	var_na buffer[20];
	const var_na *slash;
	ຈຳນວນເຕັມ count = 0;
	SCHEDULEITEM *repeatat = NULL;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	ຖ້າ (string)
		ສຳລັບ (slash = string; *slash; slash++)
			ຖ້າ (*slash == '/')
				count++;

	free_schedulelist();

	ຖ້າ (count == 0) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_SIGNAL;
		item->value = timeout;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);

		item = xmalloc(parimaa_na(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = NULL;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		ຖ້າ (string) {
			ຖ້າ (sscanf(string, "%d", &item->value) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} ບໍ່ດັ່ງນັ້ນ
			item->value = 5;

		ສົ່ງຄືນ;
	}

	ໃນຂະນະທີ່ (string != NULL) {
		ຖ້າ ((slash = strchr(string, '/')))
			len = slash - string;
		ບໍ່ດັ່ງນັ້ນ
			len = strlen(string);

		ຖ້າ (len >= (ptrdiff_t)parimaa_na(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : NULL;

		item = parse_schedule_item(applet, buffer);
		TAILQ_INSERT_TAIL(&schedule, item, entries);
		ຖ້າ (item->type == SC_FOREVER) {
			ຖ້າ (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			continue;
		}
	}

	ຖ້າ (repeatat) {
		item = xmalloc(parimaa_na(*item));
		item->type = SC_GOTO;
		item->value = 0;
		item->gotoitem = repeatat;
		TAILQ_INSERT_TAIL(&schedule, item, entries);
	}

	ສົ່ງຄືນ;
}

/* return number of processes killed, -1 on error */
ຈຳນວນເຕັມ do_stop(const var_na *applet, const var_na *exec, const var_na *const *argv,
    pid_t pid, uid_t uid, ຈຳນວນເຕັມ sig, bool group, bool test, bool quiet)
{
	const var_na *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	ຈຳນວນເຕັມ nkilled = 0;
	uid_t target;
	ຖ້າ (group)
		kind = "GID";

	ຖ້າ (pid > 0)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	ບໍ່ດັ່ງນັ້ນ
		pids = rc_find_pids(exec, argv, uid, 0);

	ຖ້າ (!pids)
		ສົ່ງຄືນ 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		ຖ້າ (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} ບໍ່ດັ່ງນັ້ນ {
			ຖ້າ (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				ຖ້າ (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			ຖ້າ (group)
				target = -pi->pid;
			ບໍ່ດັ່ງນັ້ນ
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			ຖ້າ (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			ບໍ່ດັ່ງນັ້ນ ຖ້າ (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			ຖ້າ (!killed) {
				nkilled = -1;
			} ບໍ່ດັ່ງນັ້ນ {
				ຖ້າ (nkilled != -1)
					nkilled++;
			}
		}
		free(pi);
	}

	free(pids);
	ສົ່ງຄືນ nkilled;
}

ຈຳນວນເຕັມ run_stop_schedule(const var_na *applet,
		const var_na *exec, const var_na *const *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	ຈຳນວນເຕັມ nkilled = 0;
	ຈຳນວນເຕັມ tkilled = 0;
	ຈຳນວນເຕັມ nrunning = 0;
	long nloops, nsecs;
	sa_nracanaa timespec ts;
	const var_na *const *p;
	bool progressed = false;

	ຖ້າ (!(pid > 0 || exec || uid || (argv && *argv)))
		ສົ່ງຄືນ 0;

	ຖ້າ (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	ຖ້າ (pid > 0) {
		ຖ້າ (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} ບໍ່ດັ່ງນັ້ນ {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	ຖ້າ (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	ຖ້າ (argv && *argv) {
		einfovn("Will stop processes of `");
		ຖ້າ (rc_yesno(getenv("EINFO_VERBOSE"))) {
			ສຳລັບ (p = argv; p && *p; p++) {
				ຖ້າ (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	ໃນຂະນະທີ່ (item) {
		switch (item->type) {
		pak_sa SC_GOTO:
			item = item->gotoitem;
			continue;

		pak_sa SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->value, group,
					test, quiet);
			ຖ້າ (nkilled == 0) {
				ຖ້າ (tkilled == 0) {
					ຖ້າ (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				ສົ່ງຄືນ tkilled;
			}
			ບໍ່ດັ່ງນັ້ນ ຖ້າ (nkilled == -1)
				ສົ່ງຄືນ 0;

			tkilled += nkilled;
			bha_nga;
		pak_sa SC_FOREVER:
		pak_sa SC_TIMEOUT:
			ຖ້າ (item->type == SC_TIMEOUT && item->value < 1) {
				item = NULL;
				bha_nga;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			ສຳລັບ (nsecs = 0; item->type == SC_FOREVER || nsecs < item->value; nsecs++) {
				ສຳລັບ (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					ຖ້າ ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						ສົ່ງຄືນ 0;


					ຖ້າ (nanosleep(&ts, NULL) == -1) {
						ຖ້າ (progressed) {
							printf("\n");
							progressed = false;
						}
						ຖ້າ (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							ສົ່ງຄືນ 0;
						}
					}
				}
				ຖ້າ (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			bha_nga;
		default:
			ຖ້າ (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			ສົ່ງຄືນ 0;
		}

		ຖ້າ (item)
			item = TAILQ_NEXT(item, entries);
	}

	ຖ້າ (test || (tkilled > 0 && nrunning == 0))
		ສົ່ງຄືນ nkilled;

	ຖ້າ (progressed)
		printf("\n");
	ຖ້າ (!quiet) {
		ຖ້າ (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		ບໍ່ດັ່ງນັ້ນ
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	ສົ່ງຄືນ -nrunning;
}
