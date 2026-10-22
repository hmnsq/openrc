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
#घोषणा POLL_INTERVAL   20000000
#घोषणा WAIT_PIDFILE   500000000
#घोषणा ONE_SECOND    1000000000
#घोषणा ONE_MS           1000000

#समावेश <ctype.h>
#समावेश <errno.h>
#समावेश <signal.h>
#समावेश <stddef.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <syslog.h>
#समावेश <time.h>
#समावेश <sys/types.h>

#समावेश "einfo.h"
#समावेश "queue.h"
#समावेश "rc.h"
#समावेश "schedules.h"
#समावेश "helpers.h"

प्रकार काष्ठा scheduleitem {
	क्रमागत {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	पूर्णांक मूल्य;
	काष्ठा scheduleitem *gotoitem;
	TAILQ_ENTRY(scheduleitem) entries;
} SCHEDULEITEM;

अटल TAILQ_शीर्ष(, scheduleitem) schedule;

व्योम free_schedulelist(व्योम)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&schedule);
	SCHEDULEITEM *s2;

	जबतक (s1) {
		s2 = TAILQ_अग्र(s1, entries);
		मुक्त(s1);
		s1 = s2;
	}
	TAILQ_INIT(&schedule);
}

पूर्णांक parse_signal(स्थिर अक्षर *applet, स्थिर अक्षर *sig)
{
	प्रकार काष्ठा signalpair
	{
		स्थिर अक्षर *name;
		पूर्णांक संकेत;
	} SIGNALPAIR;

#घोषणा signalpair_item(name) { #name, SIG##name },

	अटल स्थिर SIGNALPAIR signallist[] = {
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
#अगर_घोषित SIGWINCH
		signalpair_item(WINCH)
#पूर्ण_अगर
#अगर_घोषित SIGIO
		signalpair_item(IO)
#पूर्ण_अगर
#अगर_घोषित SIGPWR
		signalpair_item(PWR)
#पूर्ण_अगर
		signalpair_item(SYS)
		{ "NULL",	0 },
	};

	अचिन्हित पूर्णांक i = 0;
	स्थिर अक्षर *s;

	अगर (!sig || *sig == '\0')
		वापस -1;

	अगर (माला_पूछो(sig, "%u", &i) == 1) {
		अगर (i < NSIG)
			वापस i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	अगर (म_भेदन(sig, "SIG", 3) == 0)
		s = sig + 3;
	अन्यथा
		s = शून्य;

	क्रम (i = 0; i < ARRAY_आकार(signallist); ++i)
		अगर (म_भेद(sig, signallist[i].name) == 0 ||
		    (s && म_भेद(s, signallist[i].name) == 0))
			वापस signallist[i].संकेत;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

अटल SCHEDULEITEM *parse_schedule_item(स्थिर अक्षर *applet, स्थिर अक्षर *string)
{
	स्थिर अक्षर *after_hyph;
	पूर्णांक sig;
	SCHEDULEITEM *item = xmalloc(माप(*item));

	item->मूल्य = 0;
	item->gotoitem = शून्य;
	अगर (म_भेद(string,"forever") == 0)
		item->type = SC_FOREVER;
	अन्यथा अगर (है_अंक((अचिन्हित अक्षर)string[0])) {
		item->type = SC_TIMEOUT;
		त्रुटि_सं = 0;
		अगर (माला_पूछो(string, "%d", &item->मूल्य) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} अन्यथा अगर ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->मूल्य = (पूर्णांक)sig;
	} अन्यथा
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	वापस item;
}

व्योम parse_schedule(स्थिर अक्षर *applet, स्थिर अक्षर *string, पूर्णांक timeout)
{
	अक्षर बफर[20];
	स्थिर अक्षर *slash;
	पूर्णांक गिनती = 0;
	SCHEDULEITEM *repeatat = शून्य;
	माप_प्रकार len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&schedule);
	अगर (string)
		क्रम (slash = string; *slash; slash++)
			अगर (*slash == '/')
				गिनती++;

	free_schedulelist();

	अगर (गिनती == 0) {
		item = xmalloc(माप(*item));
		item->type = SC_SIGNAL;
		item->मूल्य = timeout;
		item->gotoitem = शून्य;
		TAILQ_जोड़ो_पुच्छ(&schedule, item, entries);

		item = xmalloc(माप(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = शून्य;
		TAILQ_जोड़ो_पुच्छ(&schedule, item, entries);
		अगर (string) {
			अगर (माला_पूछो(string, "%d", &item->मूल्य) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} अन्यथा
			item->मूल्य = 5;

		वापस;
	}

	जबतक (string != शून्य) {
		अगर ((slash = म_अक्षर(string, '/')))
			len = slash - string;
		अन्यथा
			len = म_माप(string);

		अगर (len >= (सूचक_भेद_प्रकार)माप(बफर))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		स_नकल(बफर, string, len);
		बफर[len] = 0;
		string = slash ? slash + 1 : शून्य;

		item = parse_schedule_item(applet, बफर);
		TAILQ_जोड़ो_पुच्छ(&schedule, item, entries);
		अगर (item->type == SC_FOREVER) {
			अगर (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			जारी;
		}
	}

	अगर (repeatat) {
		item = xmalloc(माप(*item));
		item->type = SC_GOTO;
		item->मूल्य = 0;
		item->gotoitem = repeatat;
		TAILQ_जोड़ो_पुच्छ(&schedule, item, entries);
	}

	वापस;
}

/* return number of processes killed, -1 on error */
पूर्णांक do_stop(स्थिर अक्षर *applet, स्थिर अक्षर *exec, स्थिर अक्षर *स्थिर *argv,
    pid_t pid, uid_t uid, पूर्णांक sig, bool group, bool test, bool quiet)
{
	स्थिर अक्षर *kind = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	पूर्णांक nkilled = 0;
	uid_t target;
	अगर (group)
		kind = "GID";

	अगर (pid > 0)
		pids = rc_find_pids(शून्य, शून्य, 0, pid);
	अन्यथा
		pids = rc_find_pids(exec, argv, uid, 0);

	अगर (!pids)
		वापस 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		अगर (test) {
			einfo("Would send signal %d to %s %d", sig, kind, pi->pid);
			nkilled++;
		} अन्यथा {
			अगर (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, kind, pi->pid);
				अगर (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, kind, pi->pid);
			}
			त्रुटि_सं = 0;
			अगर (group)
				target = -pi->pid;
			अन्यथा
				target = pi->pid;
			killed = (समाप्त(target, sig) == 0 ||
			    त्रुटि_सं == ESRCH ? true : false);
			अगर (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, kind, pi->pid, म_त्रुटि(त्रुटि_सं));
			अन्यथा अगर (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, kind, pi->pid, म_त्रुटि(त्रुटि_सं));
			अगर (!killed) {
				nkilled = -1;
			} अन्यथा {
				अगर (nkilled != -1)
					nkilled++;
			}
		}
		मुक्त(pi);
	}

	मुक्त(pids);
	वापस nkilled;
}

पूर्णांक run_stop_schedule(स्थिर अक्षर *applet,
		स्थिर अक्षर *exec, स्थिर अक्षर *स्थिर *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool progress, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&schedule);
	पूर्णांक nkilled = 0;
	पूर्णांक tkilled = 0;
	पूर्णांक nrunning = 0;
	दीर्घ nloops, nsecs;
	काष्ठा timespec ts;
	स्थिर अक्षर *स्थिर *p;
	bool progressed = false;

	अगर (!(pid > 0 || exec || uid || (argv && *argv)))
		वापस 0;

	अगर (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	अगर (pid > 0) {
		अगर (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} अन्यथा {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	अगर (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	अगर (argv && *argv) {
		einfovn("Will stop processes of `");
		अगर (rc_yesno(दो_पर्या("EINFO_VERBOSE"))) {
			क्रम (p = argv; p && *p; p++) {
				अगर (p != argv)
					म_लिखो(" ");
				म_लिखो("%s", *p);
			}
			म_लिखो("'\n");
		}
	}

	जबतक (item) {
		चयन (item->type) {
		हाल SC_GOTO:
			item = item->gotoitem;
			जारी;

		हाल SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->मूल्य, group,
					test, quiet);
			अगर (nkilled == 0) {
				अगर (tkilled == 0) {
					अगर (progressed)
						म_लिखो("\n");
					eerror("%s: no matching processes found", applet);
				}
				वापस tkilled;
			}
			अन्यथा अगर (nkilled == -1)
				वापस 0;

			tkilled += nkilled;
			अवरोध;
		हाल SC_FOREVER:
		हाल SC_TIMEOUT:
			अगर (item->type == SC_TIMEOUT && item->मूल्य < 1) {
				item = शून्य;
				अवरोध;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			क्रम (nsecs = 0; item->type == SC_FOREVER || nsecs < item->मूल्य; nsecs++) {
				क्रम (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					अगर ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						वापस 0;


					अगर (nanosleep(&ts, शून्य) == -1) {
						अगर (progressed) {
							म_लिखो("\n");
							progressed = false;
						}
						अगर (त्रुटि_सं != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, म_त्रुटि(त्रुटि_सं));
							वापस 0;
						}
					}
				}
				अगर (progress) {
					म_लिखो(".");
					ख_साफ(मानक_निकास);
					progressed = true;
				}
			}
			अवरोध;
		शेष:
			अगर (progressed) {
				म_लिखो("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			वापस 0;
		}

		अगर (item)
			item = TAILQ_अग्र(item, entries);
	}

	अगर (test || (tkilled > 0 && nrunning == 0))
		वापस nkilled;

	अगर (progressed)
		म_लिखो("\n");
	अगर (!quiet) {
		अगर (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		अन्यथा
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	वापस -nrunning;
}
