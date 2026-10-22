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

#अन्तर्भाव <ctype.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <syslog.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <sys/types.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "schedules.h"
#अन्तर्भाव "helpers.h"

प्रकारव्याख्या संरचना scheduleitem {
	गणना {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	पूर्णांक मूल्य;
	संरचना scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

स्थिर TAILQ_HEAD(, scheduleitem) schedule;

शून्य free_schedulelist(शून्य)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	पर्यंत (s1) {
		s2 = TAILQ_NEXT(s1, entries);
		मुक्त(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

पूर्णांक parse_signal(स्थिरांक अक्षर *applet, स्थिरांक अक्षर *sig)
{
	प्रकारव्याख्या संरचना signalpair
	{
		स्थिरांक अक्षर *name;
		पूर्णांक संकेत;
	} SIGNALPAIR;

#define signalpair_item(name) { #name, SIG##name },

	स्थिर स्थिरांक SIGNALPAIR signallist[] = {
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

	अचिन्हित पूर्णांक i = 0;
	स्थिरांक अक्षर *s;

	जर (!sig || *sig == '\0')
		परत -1;

	जर (माला_पूछो(sig, "%u", &i) == 1) {
		जर (i < NSIG)
			परत i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	जर (म_भेदन(sig, "SIG", 3) == 0)
		s = sig + 3;
	अन्यथा
		s = रिक्त;

	साठी (i = 0; i < ARRAY_SIZE(signallist); ++i)
		जर (म_भेद(sig, signallist[i].name) == 0 ||
		    (s && म_भेद(s, signallist[i].name) == 0))
			परत signallist[i].संकेत;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

स्थिर SCHEDULEITEM *parse_schedule_item(स्थिरांक अक्षर *applet, स्थिरांक अक्षर *string)
{
	स्थिरांक अक्षर *after_hyph;
	पूर्णांक sig;
	SCHEDULEITEM *item = xmalloc(आकार(*item));

	item->मूल्य = 0;
	item->gotoitem = रिक्त;
	जर (म_भेद(string,"forever") == 0)
		item->type = SC_FOREVER;
	अन्यथा जर (है_अंक((अचिन्हित अक्षर)string[0])) {
		item->type = SC_TIMEOUT;
		त्रुटि_सं = 0;
		जर (माला_पूछो(string, "%d", &item->मूल्य) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} अन्यथा जर ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->मूल्य = (पूर्णांक)sig;
	} अन्यथा
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	परत item;
}

शून्य parse_schedule(स्थिरांक अक्षर *applet, स्थिरांक अक्षर *string, पूर्णांक timeout)
{
	अक्षर buffer[20];
	स्थिरांक अक्षर *slash;
	पूर्णांक मोजणी = 0;
	SCHEDULEITEM *repeatat = रिक्त;
	माप_प्रकार len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	जर (string)
		साठी (slash = string; *slash; slash++)
			जर (*slash == '/')
				मोजणी++;

	free_schedulelist();

	जर (मोजणी == 0) {
		item = xmalloc(आकार(*item));
		item->type = SC_SIGNAL;
		item->मूल्य = timeout;
		item->gotoitem = रिक्त;
		TAILQ_घाला_TAIL(&schedule, item, entries);

		item = xmalloc(आकार(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = रिक्त;
		TAILQ_घाला_TAIL(&schedule, item, entries);
		जर (string) {
			जर (माला_पूछो(string, "%d", &item->मूल्य) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} अन्यथा
			item->मूल्य = 5;

		परत;
	}

	पर्यंत (string != रिक्त) {
		जर ((slash = म_अक्षर(string, '/')))
			len = slash - string;
		अन्यथा
			len = म_माप(string);

		जर (len >= (सूचक_भेद_प्रकार)आकार(buffer))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		स_नकल(buffer, string, len);
		buffer[len] = 0;
		string = slash ? slash + 1 : रिक्त;

		item = parse_schedule_item(applet, buffer);
		TAILQ_घाला_TAIL(&schedule, item, entries);
		जर (item->type == SC_FOREVER) {
			जर (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			सुरूठेवा;
		}
	}

	जर (repeatat) {
		item = xmalloc(आकार(*item));
		item->type = SC_GOTO;
		item->मूल्य = 0;
		item->gotoitem = repeatat;
		TAILQ_घाला_TAIL(&schedule, item, entries);
	}

	परत;
}

/* return number of processes killed, -1 on error */
पूर्णांक do_stop(स्थिरांक अक्षर *applet, स्थिरांक अक्षर *exec, स्थिरांक अक्षर *स्थिरांक *argv,
    pid_t pid, uid_t uid, पूर्णांक sig, bool group, bool test, bool quiet)
{
	स्थिरांक अक्षर *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	पूर्णांक nkilled = 0;
	uid_t target;
	जर (group)
		kind = "GID";

	जर (pid > 0)
		pids = rc_find_pids(रिक्त, रिक्त, 0, pid);
	अन्यथा
		pids = rc_find_pids(exec, argv, uid, 0);

	जर (!pids)
		परत 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		जर (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} अन्यथा {
			जर (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				जर (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			त्रुटि_सं = 0;
			जर (group)
				target = -pi->pid;
			अन्यथा
				target = pi->pid;
			killed = (समाप्त(target, sig) == 0 ||
			    त्रुटि_सं == ESRCH ? true : false);
			जर (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, म_त्रुटि(त्रुटि_सं));
			अन्यथा जर (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, म_त्रुटि(त्रुटि_सं));
			जर (!killed) {
				nkilled = -1;
			} अन्यथा {
				जर (nkilled != -1)
					nkilled++;
			}
		}
		मुक्त(pi);
	}

	मुक्त(pids);
	परत nkilled;
}

पूर्णांक run_stop_schedule(स्थिरांक अक्षर *applet,
		स्थिरांक अक्षर *exec, स्थिरांक अक्षर *स्थिरांक *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	पूर्णांक nkilled = 0;
	पूर्णांक tkilled = 0;
	पूर्णांक nrunning = 0;
	दीर्घ nloops, nsecs;
	संरचना timespec ts;
	स्थिरांक अक्षर *स्थिरांक *p;
	bool progressed = false;

	जर (!(pid > 0 || exec || uid || (argv && *argv)))
		परत 0;

	जर (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	जर (pid > 0) {
		जर (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} अन्यथा {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	जर (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	जर (argv && *argv) {
		einfovn("Will stop processes of `");
		जर (rc_yesno(दो_पर्या("EINFO_VERBOSE"))) {
			साठी (p = argv; p && *p; p++) {
				जर (p != argv)
					म_लिखो(" ");
				म_लिखो("%s", *p);
			}
			म_लिखो("'\n");
		}
	}

	पर्यंत (item) {
		निवड (item->type) {
		स्थिती SC_GOTO:
			item = item->gotoitem;
			सुरूठेवा;

		स्थिती SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->मूल्य, group,
					test, quiet);
			जर (nkilled == 0) {
				जर (tkilled == 0) {
					जर (progressed)
						म_लिखो("\n");
					eerror("%s: no matching processes found", applet);
				}
				परत tkilled;
			}
			अन्यथा जर (nkilled == -1)
				परत 0;

			tkilled += nkilled;
			थांबा;
		स्थिती SC_FOREVER:
		स्थिती SC_TIMEOUT:
			जर (item->type == SC_TIMEOUT && item->मूल्य < 1) {
				item = रिक्त;
				थांबा;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			साठी (nsecs = 0; item->type == SC_FOREVER || nsecs < item->मूल्य; nsecs++) {
				साठी (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					जर ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						परत 0;


					जर (nanosleep(&ts, रिक्त) == -1) {
						जर (progressed) {
							म_लिखो("\n");
							progressed = false;
						}
						जर (त्रुटि_सं != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, म_त्रुटि(त्रुटि_सं));
							परत 0;
						}
					}
				}
				जर (progress) {
					म_लिखो(".");
					ख_साफ(मानक_निकास);
					progressed = true;
				}
			}
			थांबा;
		पूर्वनिर्धारित:
			जर (progressed) {
				म_लिखो("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			परत 0;
		}

		जर (item)
			item = TAILQ_NEXT(item, entries);
	}

	जर (test || (tkilled > 0 && nrunning == 0))
		परत nkilled;

	जर (progressed)
		म_लिखो("\n");
	जर (!quiet) {
		जर (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		अन्यथा
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	परत -nrunning;
}
