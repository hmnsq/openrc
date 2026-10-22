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

#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <syslog.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <sys/types.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "schedules.h"
#அந்தர்பாவ "helpers.h"

வகைவரை கட்டமைப்பு scheduleitem {
	எண்ணிக்கை {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	முழு மதிப்பு;
	கட்டமைப்பு scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

நிலையான TAILQ_HEAD(, scheduleitem) schedule;

வெறுமை free_schedulelist(வெறுமை)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	போது (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		விடுவி(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

முழு parse_signal(மாறிலி எழுத்து *applet, மாறிலி எழுத்து *sig)
{
	வகைவரை கட்டமைப்பு signalpair
	{
		மாறிலி எழுத்து *name;
		முழு சமிக்ஞை;
	} SIGNALPAIR;

#define signalpair_item(name) { #name, SIG##name },

	நிலையான மாறிலி SIGNALPAIR signallist[] = {
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

	unsigned முழு i = 0;
	மாறிலி எழுத்து *s;

	எனில் (!sig || *sig == '\0')
		திரும்பு -1;

	எனில் (sscanf(sig, "%u", &i) == 1) {
		எனில் (i < NSIG)
			திரும்பு i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	எனில் (strncmp(sig, "SIG", 3) == 0)
		s = sig + 3;
	இல்லையெனில்
		s = வெற்று;

	க்கு (i = 0; i < ARRAY_SIZE(signallist); ++i)
		எனில் (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			திரும்பு signallist[i].சமிக்ஞை;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

நிலையான SCHEDULEITEM *parse_schedule_item(மாறிலி எழுத்து *applet, மாறிலி எழுத்து *string)
{
	மாறிலி எழுத்து *after_hyph;
	முழு sig;
	SCHEDULEITEM *item = xmalloc(அளவு(*item));

	item->மதிப்பு = 0;
	item->gotoitem = வெற்று;
	எனில் (strcmp(string,"forever") == 0)
		item->type = SC_FOREVER;
	இல்லையெனில் எனில் (isdigit((unsigned எழுத்து)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		எனில் (sscanf(string, "%d", &item->மதிப்பு) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} இல்லையெனில் எனில் ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->மதிப்பு = (முழு)sig;
	} இல்லையெனில்
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	திரும்பு item;
}

வெறுமை parse_schedule(மாறிலி எழுத்து *applet, மாறிலி எழுத்து *string, முழு timeout)
{
	எழுத்து buffer[20];
	மாறிலி எழுத்து *slash;
	முழு மொத்தம் = 0;
	SCHEDULEITEM *repeatat = வெற்று;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	எனில் (string)
		க்கு (slash = string; *slash; slash++)
			எனில் (*slash == '/')
				மொத்தம்++;

	free_schedulelist();

	எனில் (மொத்தம் == 0) {
		item = xmalloc(அளவு(*item));
		item->type = SC_SIGNAL;
		item->மதிப்பு = timeout;
		item->gotoitem = வெற்று;
		TAILQ_செருகு_TAIL(&schedule, item, entries);

		item = xmalloc(அளவு(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = வெற்று;
		TAILQ_செருகு_TAIL(&schedule, item, entries);
		எனில் (string) {
			எனில் (sscanf(string, "%d", &item->மதிப்பு) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} இல்லையெனில்
			item->மதிப்பு = 5;

		திரும்பு;
	}

	போது (string != வெற்று) {
		எனில் ((slash = strchr(string, '/')))
			len = slash - string;
		இல்லையெனில்
			len = strlen(string);

		எனில் (len >= (ptrdiff_t)அளவு(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		memcpy(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : வெற்று;

		item = parse_schedule_item(applet, buffer);
		TAILQ_செருகு_TAIL(&schedule, item, entries);
		எனில் (item->type == SC_FOREVER) {
			எனில் (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			தொடர்;
		}
	}

	எனில் (repeatat) {
		item = xmalloc(அளவு(*item));
		item->type = SC_GOTO;
		item->மதிப்பு = 0;
		item->gotoitem = repeatat;
		TAILQ_செருகு_TAIL(&schedule, item, entries);
	}

	திரும்பு;
}

/* return number of processes killed, -1 on error */
முழு do_stop(மாறிலி எழுத்து *applet, மாறிலி எழுத்து *exec, மாறிலி எழுத்து *மாறிலி *argv,
    pid_t pid, uid_t uid, முழு sig, bool group, bool test, bool quiet)
{
	மாறிலி எழுத்து *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	முழு nkilled = 0;
	uid_t target;
	எனில் (group)
		kind = "GID";

	எனில் (pid > 0)
		pids = rc_find_pids(வெற்று, வெற்று, 0, pid);
	இல்லையெனில்
		pids = rc_find_pids(exec, argv, uid, 0);

	எனில் (!pids)
		திரும்பு 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		எனில் (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} இல்லையெனில் {
			எனில் (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				எனில் (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			errno = 0;
			எனில் (group)
				target = -pi->pid;
			இல்லையெனில்
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			எனில் (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, strerror(errno));
			இல்லையெனில் எனில் (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, strerror(errno));
			எனில் (!killed) {
				nkilled = -1;
			} இல்லையெனில் {
				எனில் (nkilled != -1)
					nkilled++;
			}
		}
		விடுவி(pi);
	}

	விடுவி(pids);
	திரும்பு nkilled;
}

முழு run_stop_schedule(மாறிலி எழுத்து *applet,
		மாறிலி எழுத்து *exec, மாறிலி எழுத்து *மாறிலி *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	முழு nkilled = 0;
	முழு tkilled = 0;
	முழு nrunning = 0;
	long nloops, nsecs;
	கட்டமைப்பு timespec ts;
	மாறிலி எழுத்து *மாறிலி *p;
	bool progressed = false;

	எனில் (!(pid > 0 || exec || uid || (argv && *argv)))
		திரும்பு 0;

	எனில் (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	எனில் (pid > 0) {
		எனில் (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} இல்லையெனில் {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	எனில் (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	எனில் (argv && *argv) {
		einfovn("Will stop processes of `");
		எனில் (rc_yesno(getenv("EINFO_VERBOSE"))) {
			க்கு (p = argv; p && *p; p++) {
				எனில் (p != argv)
					printf(" ");
				printf("%s", *p);
			}
			printf("'\n");
		}
	}

	போது (item) {
		தேர்வு (item->type) {
		நிலை SC_GOTO:
			item = item->gotoitem;
			தொடர்;

		நிலை SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->மதிப்பு, group,
					test, quiet);
			எனில் (nkilled == 0) {
				எனில் (tkilled == 0) {
					எனில் (progressed)
						printf("\n");
					eerror("%s: no matching processes found", applet);
				}
				திரும்பு tkilled;
			}
			இல்லையெனில் எனில் (nkilled == -1)
				திரும்பு 0;

			tkilled += nkilled;
			நிறுத்து;
		நிலை SC_FOREVER:
		நிலை SC_TIMEOUT:
			எனில் (item->type == SC_TIMEOUT && item->மதிப்பு < 1) {
				item = வெற்று;
				நிறுத்து;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			க்கு (nsecs = 0; item->type == SC_FOREVER || nsecs < item->மதிப்பு; nsecs++) {
				க்கு (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					எனில் ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						திரும்பு 0;


					எனில் (nanosleep(&ts, வெற்று) == -1) {
						எனில் (progressed) {
							printf("\n");
							progressed = false;
						}
						எனில் (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							திரும்பு 0;
						}
					}
				}
				எனில் (progress) {
					printf(".");
					fflush(stdout);
					progressed = true;
				}
			}
			நிறுத்து;
		இயல்பு:
			எனில் (progressed) {
				printf("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			திரும்பு 0;
		}

		எனில் (item)
			item = TAILQ_NEXT(item, entries);
	}

	எனில் (test || (tkilled > 0 && nrunning == 0))
		திரும்பு nkilled;

	எனில் (progressed)
		printf("\n");
	எனில் (!quiet) {
		எனில் (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		இல்லையெனில்
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	திரும்பு -nrunning;
}
