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

#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <syslog.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <sys/types.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "schedules.h"
#അന്തര്ഭാവ "helpers.h"

തരനിർവചനം ഘടന scheduleitem {
	എണ്ണൽ {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	പൂർണ്ണസംഖ്യ മൂല്യം;
	ഘടന scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

സ്ഥിതം TAILQ_HEAD(, scheduleitem) schedule;

ശൂന്യം free_schedulelist(ശൂന്യം)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	ആകുമ്പോൾ (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		സ്വതന്ത്രം(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

പൂർണ്ണസംഖ്യ parse_signal(സ്ഥിരം അക്ഷരം *applet, സ്ഥിരം അക്ഷരം *sig)
{
	തരനിർവചനം ഘടന signalpair
	{
		സ്ഥിരം അക്ഷരം *name;
		പൂർണ്ണസംഖ്യ സൂചന;
	} SIGNALPAIR;

#define signalpair_item(name) { #name, SIG##name },

	സ്ഥിതം സ്ഥിരം SIGNALPAIR signallist[] = {
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

	unsigned പൂർണ്ണസംഖ്യ i = 0;
	സ്ഥിരം അക്ഷരം *s;

	എങ്കിൽ (!sig || *sig == '\0')
		തിരികെ -1;

	എങ്കിൽ (sscanf(sig, "%u", &i) == 1) {
		എങ്കിൽ (i < NSIG)
			തിരികെ i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	എങ്കിൽ (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	അല്ലെങ്കിൽ
		s = ശൂന്യ;

	വേണ്ടി (i = 0; i < ARRAY_SIZE(signallist); ++i)
		എങ്കിൽ (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			തിരികെ signallist[i].സൂചന;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

സ്ഥിതം SCHEDULEITEM *parse_schedule_item(സ്ഥിരം അക്ഷരം *applet, സ്ഥിരം അക്ഷരം *string)
{
	സ്ഥിരം അക്ഷരം *after_hyph;
	പൂർണ്ണസംഖ്യ sig;
	SCHEDULEITEM *item = xmalloc(വലുപ്പം(*item));

	item->മൂല്യം = 0;
	item->gotoitem = ശൂന്യ;
	എങ്കിൽ (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	അല്ലെങ്കിൽ എങ്കിൽ (isdigit((unsigned അക്ഷരം)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		എങ്കിൽ (sscanf(string, "%d", &item->മൂല്യം) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} അല്ലെങ്കിൽ എങ്കിൽ ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->മൂല്യം = (പൂർണ്ണസംഖ്യ)sig;
	} അല്ലെങ്കിൽ
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	തിരികെ item;
}

ശൂന്യം parse_schedule(സ്ഥിരം അക്ഷരം *applet, സ്ഥിരം അക്ഷരം *string, പൂർണ്ണസംഖ്യ timeout)
{
	അക്ഷരം buffer[20];
	സ്ഥിരം അക്ഷരം *slash;
	പൂർണ്ണസംഖ്യ എണ്ണം = 0;
	SCHEDULEITEM *repeatat = ശൂന്യ;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	എങ്കിൽ (string)
		വേണ്ടി (slash = string; *slash; slash++)
			എങ്കിൽ (*slash == '/')
				എണ്ണം++;

	free_schedulelist();

	എങ്കിൽ (എണ്ണം == 0) {
		item = xmalloc(വലുപ്പം(*item));
		item->type = SC_SIGNAL;
		item->മൂല്യം = timeout;
		item->gotoitem = ശൂന്യ;
		TAILQ_തിരുകുക_TAIL(&schedule, item, entries);

		item = xmalloc(വലുപ്പം(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = ശൂന്യ;
		TAILQ_തിരുകുക_TAIL(&schedule, item, entries);
		എങ്കിൽ (string) {
			എങ്കിൽ (sscanf(string, "%d", &item->മൂല്യം) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} അല്ലെങ്കിൽ
			item->മൂല്യം = 5;

		തിരികെ;
	}

	ആകുമ്പോൾ (string != ശൂന്യ) {
		എങ്കിൽ ((slash = strchr(string, '/')))
			len = slash - string;
		അല്ലെങ്കിൽ
			len = strlen(string);

		എങ്കിൽ (len >= (ptrdiff_t)വലുപ്പം(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : ശൂന്യ;

		item = parse_schedule_item(applet, buffer);
		TAILQ_തിരുകുക_TAIL(&schedule, item, entries);
		എങ്കിൽ (item->type == SC_FOREVER) {
			എങ്കിൽ (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			തുടരുക;
		}
	}

	എങ്കിൽ (repeatat) {
		item = xmalloc(വലുപ്പം(*item));
		item->type = SC_GOTO;
		item->മൂല്യം = 0;
		item->gotoitem = repeatat;
		TAILQ_തിരുകുക_TAIL(&schedule, item, entries);
	}

	തിരികെ;
}

/* return number of processes killed, -1 on error */
പൂർണ്ണസംഖ്യ do_stop(സ്ഥിരം അക്ഷരം *applet, സ്ഥിരം അക്ഷരം *exec, സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv,
    pid_t pid, uid_t uid, പൂർണ്ണസംഖ്യ sig, bool group, bool test, bool quiet)
{
	സ്ഥിരം അക്ഷരം *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	പൂർണ്ണസംഖ്യ nkilled = 0;
	uid_t target;
	എങ്കിൽ (group)
		kind = "GID";

	എങ്കിൽ (pid > 0)
		pids = rc_find_pids(ശൂന്യ, ശൂന്യ, 0, pid);
	അല്ലെങ്കിൽ
		pids = rc_find_pids(exec, argv, uid, 0);

	എങ്കിൽ (!pids)
		തിരികെ 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		എങ്കിൽ (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				എങ്കിൽ (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			എങ്കിൽ (group)
				target = -pi->pid;
			അല്ലെങ്കിൽ
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			എങ്കിൽ (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			അല്ലെങ്കിൽ എങ്കിൽ (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			എങ്കിൽ (!killed) {
				nkilled = -1;
			} അല്ലെങ്കിൽ {
				എങ്കിൽ (nkilled != -1)
					nkilled++;
			}
		}
		സ്വതന്ത്രം(pi);
	}

	സ്വതന്ത്രം(pids);
	തിരികെ nkilled;
}

പൂർണ്ണസംഖ്യ run_stop_schedule(സ്ഥിരം അക്ഷരം *applet,
		സ്ഥിരം അക്ഷരം *exec, സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	പൂർണ്ണസംഖ്യ nkilled = 0;
	പൂർണ്ണസംഖ്യ tkilled = 0;
	പൂർണ്ണസംഖ്യ nrunning = 0;
	long nloops, nsecs;
	ഘടന timespec ts;
	സ്ഥിരം അക്ഷരം *സ്ഥിരം *p;
	bool progressed = false;

	എങ്കിൽ (!(pid > 0 || exec || uid || (argv && *argv)))
		തിരികെ 0;

	എങ്കിൽ (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	എങ്കിൽ (pid > 0) {
		എങ്കിൽ (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} അല്ലെങ്കിൽ {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	എങ്കിൽ (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	എങ്കിൽ (argv && *argv) {
		einfovn("Will stop processes of `");
		എങ്കിൽ (rc_yesno(getenv("EINFO_VERBOSE"))) {
			വേണ്ടി (p = argv; p && *p; p++) {
				എങ്കിൽ (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	ആകുമ്പോൾ (item) {
		തിരഞ്ഞെടുപ്പ് (item->type) {
		അവസ്ഥ SC_GOTO:
			item = item->gotoitem;
			തുടരുക;

		അവസ്ഥ SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->മൂല്യം, group,
					test, quiet);
			എങ്കിൽ (nkilled == 0) {
				എങ്കിൽ (tkilled == 0) {
					എങ്കിൽ (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				തിരികെ tkilled;
			}
			അല്ലെങ്കിൽ എങ്കിൽ (nkilled == -1)
				തിരികെ 0;

			tkilled += nkilled;
			നിർത്തുക;
		അവസ്ഥ SC_FOREVER:
		അവസ്ഥ SC_TIMEOUT:
			എങ്കിൽ (item->type == SC_TIMEOUT && item->മൂല്യം < 1) {
				item = ശൂന്യ;
				നിർത്തുക;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			വേണ്ടി (nsecs = 0; item->type == SC_FOREVER || nsecs < item->മൂല്യം; nsecs++) {
				വേണ്ടി (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					എങ്കിൽ ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						തിരികെ 0;


					എങ്കിൽ (nanosleep(&ts, ശൂന്യ) == -1) {
						എങ്കിൽ (progressed) {
							printf("\n");
							progressed = false;
						}
						എങ്കിൽ (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							തിരികെ 0;
						}
					}
				}
				എങ്കിൽ (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			എങ്കിൽ (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			തിരികെ 0;
		}

		എങ്കിൽ (item)
			item = TAILQ_NEXT(item, entries);
	}

	എങ്കിൽ (test || (tkilled > 0 && nrunning == 0))
		തിരികെ nkilled;

	എങ്കിൽ (progressed)
		printf("\n");
	എങ്കിൽ (!quiet) {
		എങ്കിൽ (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		അല്ലെങ്കിൽ
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	തിരികെ -nrunning;
}
