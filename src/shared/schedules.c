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

#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <syslog.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "schedules.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା scheduleitem {
	ଗଣନା {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ;
	ସଂରଚନା scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

ସ୍ଥିର TAILQ_HEAD(, scheduleitem) schedule;

ଶୂନ୍ୟ free_schedulelist(ଶୂନ୍ୟ)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	ଯେତେବେଳେ (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		ମୁକ୍ତ(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_signal(ଅଚଳ ଅକ୍ଷର *applet, ଅଚଳ ଅକ୍ଷର *sig)
{
	ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା signalpair
	{
		ଅଚଳ ଅକ୍ଷର *name;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ସଙ୍କେତ;
	} SIGNALPAIR;

#define signalpair_item(name) { #name, SIG##name },

	ସ୍ଥିର ଅଚଳ SIGNALPAIR signallist[] = {
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

	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
	ଅଚଳ ଅକ୍ଷର *s;

	ଯଦି (!sig || *sig == '\0')
		ଫେରନ୍ତୁ -1;

	ଯଦି (sscanf(sig, "%u", &i) == 1) {
		ଯଦି (i < NSIG)
			ଫେରନ୍ତୁ i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	ଯଦି (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	ଅନ୍ୟଥା
		s = ଖାଲି;

	ପାଇଁ (i = 0; i < ARRAY_SIZE(signallist); ++i)
		ଯଦି (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			ଫେରନ୍ତୁ signallist[i].ସଙ୍କେତ;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

ସ୍ଥିର SCHEDULEITEM *parse_schedule_item(ଅଚଳ ଅକ୍ଷର *applet, ଅଚଳ ଅକ୍ଷର *string)
{
	ଅଚଳ ଅକ୍ଷର *after_hyph;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig;
	SCHEDULEITEM *item = xmalloc(ଆକାର(*item));

	item->ମୂଲ୍ୟ = 0;
	item->gotoitem = ଖାଲି;
	ଯଦି (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	ଅନ୍ୟଥା ଯଦି (isdigit((unsigned ଅକ୍ଷର)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		ଯଦି (sscanf(string, "%d", &item->ମୂଲ୍ୟ) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} ଅନ୍ୟଥା ଯଦି ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->ମୂଲ୍ୟ = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)sig;
	} ଅନ୍ୟଥା
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	ଫେରନ୍ତୁ item;
}

ଶୂନ୍ୟ parse_schedule(ଅଚଳ ଅକ୍ଷର *applet, ଅଚଳ ଅକ୍ଷର *string, ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout)
{
	ଅକ୍ଷର buffer[20];
	ଅଚଳ ଅକ୍ଷର *slash;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;
	SCHEDULEITEM *repeatat = ଖାଲି;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	ଯଦି (string)
		ପାଇଁ (slash = string; *slash; slash++)
			ଯଦି (*slash == '/')
				ଗଣନା_2++;

	free_schedulelist();

	ଯଦି (ଗଣନା_2 == 0) {
		item = xmalloc(ଆକାର(*item));
		item->type = SC_SIGNAL;
		item->ମୂଲ୍ୟ = timeout;
		item->gotoitem = ଖାଲି;
		TAILQ_ଭର୍ତ୍ତି_TAIL(&schedule, item, entries);

		item = xmalloc(ଆକାର(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = ଖାଲି;
		TAILQ_ଭର୍ତ୍ତି_TAIL(&schedule, item, entries);
		ଯଦି (string) {
			ଯଦି (sscanf(string, "%d", &item->ମୂଲ୍ୟ) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} ଅନ୍ୟଥା
			item->ମୂଲ୍ୟ = 5;

		ଫେରନ୍ତୁ;
	}

	ଯେତେବେଳେ (string != ଖାଲି) {
		ଯଦି ((slash = strchr(string, '/')))
			len = slash - string;
		ଅନ୍ୟଥା
			len = strlen(string);

		ଯଦି (len >= (ptrdiff_t)ଆକାର(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : ଖାଲି;

		item = parse_schedule_item(applet, buffer);
		TAILQ_ଭର୍ତ୍ତି_TAIL(&schedule, item, entries);
		ଯଦି (item->type == SC_FOREVER) {
			ଯଦି (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			ଜାରି;
		}
	}

	ଯଦି (repeatat) {
		item = xmalloc(ଆକାର(*item));
		item->type = SC_GOTO;
		item->ମୂଲ୍ୟ = 0;
		item->gotoitem = repeatat;
		TAILQ_ଭର୍ତ୍ତି_TAIL(&schedule, item, entries);
	}

	ଫେରନ୍ତୁ;
}

/* return number of processes killed, -1 on error */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_stop(ଅଚଳ ଅକ୍ଷର *applet, ଅଚଳ ଅକ୍ଷର *exec, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv,
    pid_t pid, uid_t uid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig, bool group, bool test, bool quiet)
{
	ଅଚଳ ଅକ୍ଷର *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nkilled = 0;
	uid_t target;
	ଯଦି (group)
		kind = "GID";

	ଯଦି (pid > 0)
		pids = rc_find_pids(ଖାଲି, ଖାଲି, 0, pid);
	ଅନ୍ୟଥା
		pids = rc_find_pids(exec, argv, uid, 0);

	ଯଦି (!pids)
		ଫେରନ୍ତୁ 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		ଯଦି (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} ଅନ୍ୟଥା {
			ଯଦି (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				ଯଦି (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			ଯଦି (group)
				target = -pi->pid;
			ଅନ୍ୟଥା
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			ଯଦି (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			ଅନ୍ୟଥା ଯଦି (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			ଯଦି (!killed) {
				nkilled = -1;
			} ଅନ୍ୟଥା {
				ଯଦି (nkilled != -1)
					nkilled++;
			}
		}
		ମୁକ୍ତ(pi);
	}

	ମୁକ୍ତ(pids);
	ଫେରନ୍ତୁ nkilled;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_stop_schedule(ଅଚଳ ଅକ୍ଷର *applet,
		ଅଚଳ ଅକ୍ଷର *exec, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nkilled = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା tkilled = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nrunning = 0;
	long nloops, nsecs;
	ସଂରଚନା timespec ts;
	ଅଚଳ ଅକ୍ଷର *ଅଚଳ *p;
	bool progressed = false;

	ଯଦି (!(pid > 0 || exec || uid || (argv && *argv)))
		ଫେରନ୍ତୁ 0;

	ଯଦି (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	ଯଦି (pid > 0) {
		ଯଦି (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} ଅନ୍ୟଥା {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	ଯଦି (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	ଯଦି (argv && *argv) {
		einfovn("Will stop processes of `");
		ଯଦି (rc_yesno(getenv("EINFO_VERBOSE"))) {
			ପାଇଁ (p = argv; p && *p; p++) {
				ଯଦି (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	ଯେତେବେଳେ (item) {
		ବାଛ (item->type) {
		ସ୍ଥିତି SC_GOTO:
			item = item->gotoitem;
			ଜାରି;

		ସ୍ଥିତି SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->ମୂଲ୍ୟ, group,
					test, quiet);
			ଯଦି (nkilled == 0) {
				ଯଦି (tkilled == 0) {
					ଯଦି (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				ଫେରନ୍ତୁ tkilled;
			}
			ଅନ୍ୟଥା ଯଦି (nkilled == -1)
				ଫେରନ୍ତୁ 0;

			tkilled += nkilled;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି SC_FOREVER:
		ସ୍ଥିତି SC_TIMEOUT:
			ଯଦି (item->type == SC_TIMEOUT && item->ମୂଲ୍ୟ < 1) {
				item = ଖାଲି;
				ଭାଙ୍ଗ;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			ପାଇଁ (nsecs = 0; item->type == SC_FOREVER || nsecs < item->ମୂଲ୍ୟ; nsecs++) {
				ପାଇଁ (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					ଯଦି ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						ଫେରନ୍ତୁ 0;


					ଯଦି (nanosleep(&ts, ଖାଲି) == -1) {
						ଯଦି (progressed) {
							printf("\n");
							progressed = false;
						}
						ଯଦି (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							ଫେରନ୍ତୁ 0;
						}
					}
				}
				ଯଦି (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଯଦି (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			ଫେରନ୍ତୁ 0;
		}

		ଯଦି (item)
			item = TAILQ_NEXT(item, entries);
	}

	ଯଦି (test || (tkilled > 0 && nrunning == 0))
		ଫେରନ୍ତୁ nkilled;

	ଯଦି (progressed)
		printf("\n");
	ଯଦି (!quiet) {
		ଯଦି (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		ଅନ୍ୟଥା
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	ଫେରନ୍ତୁ -nrunning;
}
