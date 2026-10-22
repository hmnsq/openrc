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

#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <syslog.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <sys/types.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "schedules.h"
#અન્તર્ભાવ "helpers.h"

પ્રકારવ્યાખ્યા સંરચના scheduleitem {
	ગણના {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	પૂર્ણાંક કિંમત;
	સંરચના scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

સ્થિર TAILQ_HEAD(, scheduleitem) schedule;

શૂન્ય free_schedulelist(શૂન્ય)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	જ્યારે (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		મુક્ત(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

પૂર્ણાંક parse_signal(અચલ અક્ષર *applet, અચલ અક્ષર *sig)
{
	પ્રકારવ્યાખ્યા સંરચના signalpair
	{
		અચલ અક્ષર *name;
		પૂર્ણાંક સંકેત;
	} SIGNALPAIR;

#define signalpair_item(name) { #name, SIG##name },

	સ્થિર અચલ SIGNALPAIR signallist[] = {
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

	unsigned પૂર્ણાંક i = 0;
	અચલ અક્ષર *s;

	જો (!sig || *sig == '\0')
		પાછા -1;

	જો (sscanf(sig, "%u", &i) == 1) {
		જો (i < NSIG)
			પાછા i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	જો (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	નહીંતર
		s = ખાલી;

	માટે (i = 0; i < ARRAY_SIZE(signallist); ++i)
		જો (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			પાછા signallist[i].સંકેત;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

સ્થિર SCHEDULEITEM *parse_schedule_item(અચલ અક્ષર *applet, અચલ અક્ષર *string)
{
	અચલ અક્ષર *after_hyph;
	પૂર્ણાંક sig;
	SCHEDULEITEM *item = xmalloc(કદ(*item));

	item->કિંમત = 0;
	item->gotoitem = ખાલી;
	જો (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	નહીંતર જો (isdigit((unsigned અક્ષર)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		જો (sscanf(string, "%d", &item->કિંમત) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} નહીંતર જો ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->કિંમત = (પૂર્ણાંક)sig;
	} નહીંતર
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	પાછા item;
}

શૂન્ય parse_schedule(અચલ અક્ષર *applet, અચલ અક્ષર *string, પૂર્ણાંક timeout)
{
	અક્ષર buffer[20];
	અચલ અક્ષર *slash;
	પૂર્ણાંક ગણતરી = 0;
	SCHEDULEITEM *repeatat = ખાલી;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	જો (string)
		માટે (slash = string; *slash; slash++)
			જો (*slash == '/')
				ગણતરી++;

	free_schedulelist();

	જો (ગણતરી == 0) {
		item = xmalloc(કદ(*item));
		item->type = SC_SIGNAL;
		item->કિંમત = timeout;
		item->gotoitem = ખાલી;
		TAILQ_ઉમેરો_TAIL(&schedule, item, entries);

		item = xmalloc(કદ(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = ખાલી;
		TAILQ_ઉમેરો_TAIL(&schedule, item, entries);
		જો (string) {
			જો (sscanf(string, "%d", &item->કિંમત) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} નહીંતર
			item->કિંમત = 5;

		પાછા;
	}

	જ્યારે (string != ખાલી) {
		જો ((slash = strchr(string, '/')))
			len = slash - string;
		નહીંતર
			len = strlen(string);

		જો (len >= (ptrdiff_t)કદ(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : ખાલી;

		item = parse_schedule_item(applet, buffer);
		TAILQ_ઉમેરો_TAIL(&schedule, item, entries);
		જો (item->type == SC_FOREVER) {
			જો (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			ચાલુ;
		}
	}

	જો (repeatat) {
		item = xmalloc(કદ(*item));
		item->type = SC_GOTO;
		item->કિંમત = 0;
		item->gotoitem = repeatat;
		TAILQ_ઉમેરો_TAIL(&schedule, item, entries);
	}

	પાછા;
}

/* return number of processes killed, -1 on error */
પૂર્ણાંક do_stop(અચલ અક્ષર *applet, અચલ અક્ષર *exec, અચલ અક્ષર *અચલ *argv,
    pid_t pid, uid_t uid, પૂર્ણાંક sig, bool group, bool test, bool quiet)
{
	અચલ અક્ષર *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	પૂર્ણાંક nkilled = 0;
	uid_t target;
	જો (group)
		kind = "GID";

	જો (pid > 0)
		pids = rc_find_pids(ખાલી, ખાલી, 0, pid);
	નહીંતર
		pids = rc_find_pids(exec, argv, uid, 0);

	જો (!pids)
		પાછા 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		જો (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} નહીંતર {
			જો (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				જો (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			જો (group)
				target = -pi->pid;
			નહીંતર
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			જો (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			નહીંતર જો (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			જો (!killed) {
				nkilled = -1;
			} નહીંતર {
				જો (nkilled != -1)
					nkilled++;
			}
		}
		મુક્ત(pi);
	}

	મુક્ત(pids);
	પાછા nkilled;
}

પૂર્ણાંક run_stop_schedule(અચલ અક્ષર *applet,
		અચલ અક્ષર *exec, અચલ અક્ષર *અચલ *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	પૂર્ણાંક nkilled = 0;
	પૂર્ણાંક tkilled = 0;
	પૂર્ણાંક nrunning = 0;
	long nloops, nsecs;
	સંરચના timespec ts;
	અચલ અક્ષર *અચલ *p;
	bool progressed = false;

	જો (!(pid > 0 || exec || uid || (argv && *argv)))
		પાછા 0;

	જો (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	જો (pid > 0) {
		જો (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} નહીંતર {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	જો (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	જો (argv && *argv) {
		einfovn("Will stop processes of `");
		જો (rc_yesno(getenv("EINFO_VERBOSE"))) {
			માટે (p = argv; p && *p; p++) {
				જો (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	જ્યારે (item) {
		પસંદગી (item->type) {
		સ્થિતિ SC_GOTO:
			item = item->gotoitem;
			ચાલુ;

		સ્થિતિ SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->કિંમત, group,
					test, quiet);
			જો (nkilled == 0) {
				જો (tkilled == 0) {
					જો (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				પાછા tkilled;
			}
			નહીંતર જો (nkilled == -1)
				પાછા 0;

			tkilled += nkilled;
			તોડો;
		સ્થિતિ SC_FOREVER:
		સ્થિતિ SC_TIMEOUT:
			જો (item->type == SC_TIMEOUT && item->કિંમત < 1) {
				item = ખાલી;
				તોડો;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			માટે (nsecs = 0; item->type == SC_FOREVER || nsecs < item->કિંમત; nsecs++) {
				માટે (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					જો ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						પાછા 0;


					જો (nanosleep(&ts, ખાલી) == -1) {
						જો (progressed) {
							printf("\n");
							progressed = false;
						}
						જો (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							પાછા 0;
						}
					}
				}
				જો (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			તોડો;
		મૂળભૂત:
			જો (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			પાછા 0;
		}

		જો (item)
			item = TAILQ_NEXT(item, entries);
	}

	જો (test || (tkilled > 0 && nrunning == 0))
		પાછા nkilled;

	જો (progressed)
		printf("\n");
	જો (!quiet) {
		જો (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		નહીંતર
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	પાછા -nrunning;
}
