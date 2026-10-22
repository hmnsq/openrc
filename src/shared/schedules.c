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
#परिभाषय POLL_INTERVAL   20000000
#परिभाषय WAIT_PIDFILE   500000000
#परिभाषय ONE_SECOND    1000000000
#परिभाषय ONE_MS           1000000

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

प्रकारनिर्देश संरचना scheduleitem {
	गणना {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	पूर्णाङ्क मानम्;
	संरचना scheduleitem *gotoitem;
	TAILQ_प्रविष्टिः(scheduleitem) entries;
} SCHEDULEITEM;

स्थिर TAILQ_शीर्ष(, scheduleitem) कालसारणी;

शून्य free_schedulelist(शून्य)
{
	SCHEDULEITEM *s1 = TAILQ_FIRST(&कालसारणी);
	SCHEDULEITEM *s2;

	यावत् (s1) {
		s2 = TAILQ_अग्र(s1, entries);
		मुक्त(s1);
		s1 = s2;
	}
	TAILQ_INIT(&कालसारणी);
}

पूर्णाङ्क parse_signal(ध्रुव वर्ण *applet, ध्रुव वर्ण *sig)
{
	प्रकारनिर्देश संरचना signalpair
	{
		ध्रुव वर्ण *name;
		पूर्णाङ्क सङ्केत;
	} SIGNALPAIR;

#परिभाषय signalpair_item(name) { #name, SIG##name },

	स्थिर ध्रुव SIGNALPAIR signallist[] = {
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
		signalpair_item(पदांशः_2)
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
#यदि_परिभाषितम् SIGWINCH
		signalpair_item(WINCH)
#यदि_अन्तः
#यदि_परिभाषितम् SIGIO
		signalpair_item(IO)
#यदि_अन्तः
#यदि_परिभाषितम् SIGPWR
		signalpair_item(PWR)
#यदि_अन्तः
		signalpair_item(SYS)
		{ "NULL",	0 },
	};

	अचिह्नित पूर्णाङ्क i = 0;
	ध्रुव वर्ण *s;

	यदि (!sig || *sig == '\0')
		प्रत्यागम -1;

	यदि (वाक्यात्_पठ(sig, "%u", &i) == 1) {
		यदि (i < NSIG)
			प्रत्यागम i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	यदि (वाक्ये_तुलय(sig, "SIG", 3) == 0)
		s = sig + 3;
	अन्यथा
		s = रिक्त;

	प्रति (i = 0; i < ARRAY_आकार(signallist); ++i)
		यदि (वाक्यं_तुलय(sig, signallist[i].name) == 0 ||
		    (s && वाक्यं_तुलय(s, signallist[i].name) == 0))
			प्रत्यागम signallist[i].सङ्केत;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

स्थिर SCHEDULEITEM *parse_कालसारणी_item(ध्रुव वर्ण *applet, ध्रुव वर्ण *string)
{
	ध्रुव वर्ण *after_hyph;
	पूर्णाङ्क sig;
	SCHEDULEITEM *item = xmalloc(परिमाण(*item));

	item->मानम् = 0;
	item->gotoitem = रिक्त;
	यदि (वाक्यं_तुलय(string,"forever") == 0)
		item->type = SC_FOREVER;
	अन्यथा यदि (अङ्कः_वा((अचिह्नित वर्ण)string[0])) {
		item->type = SC_TIMEOUT;
		errno = 0;
		यदि (वाक्यात्_पठ(string, "%d", &item->मानम्) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} अन्यथा यदि ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		item->type = SC_SIGNAL;
		item->मानम् = (पूर्णाङ्क)sig;
	} अन्यथा
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	प्रत्यागम item;
}

शून्य parse_कालसारणी(ध्रुव वर्ण *applet, ध्रुव वर्ण *string, पूर्णाङ्क timeout)
{
	वर्ण बफर[20];
	ध्रुव वर्ण *slash;
	पूर्णाङ्क गणनम् = 0;
	SCHEDULEITEM *repeatat = रिक्त;
	size_t len;
	SCHEDULEITEM *item;

	TAILQ_INIT(&कालसारणी);
	यदि (string)
		प्रति (slash = string; *slash; slash++)
			यदि (*slash == '/')
				गणनम्++;

	free_schedulelist();

	यदि (गणनम् == 0) {
		item = xmalloc(परिमाण(*item));
		item->type = SC_SIGNAL;
		item->मानम् = timeout;
		item->gotoitem = रिक्त;
		TAILQ_निवेशय_पुच्छ(&कालसारणी, item, entries);

		item = xmalloc(परिमाण(*item));
		item->type = SC_TIMEOUT;
		item->gotoitem = रिक्त;
		TAILQ_निवेशय_पुच्छ(&कालसारणी, item, entries);
		यदि (string) {
			यदि (वाक्यात्_पठ(string, "%d", &item->मानम्) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} अन्यथा
			item->मानम् = 5;

		प्रत्यागम;
	}

	यावत् (string != रिक्त) {
		यदि ((slash = strchr(string, '/')))
			len = slash - string;
		अन्यथा
			len = वाक्यदैर्घ्यम्(string);

		यदि (len >= (ptrdiff_t)परिमाण(बफर))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		स्मृतिं_प्रतिलिख(बफर, string, len);
		बफर[len] = 0;
		string = slash ? slash + 1 : रिक्त;

		item = parse_कालसारणी_item(applet, बफर);
		TAILQ_निवेशय_पुच्छ(&कालसारणी, item, entries);
		यदि (item->type == SC_FOREVER) {
			यदि (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = item;
			अनुवर्त;
		}
	}

	यदि (repeatat) {
		item = xmalloc(परिमाण(*item));
		item->type = SC_GOTO;
		item->मानम् = 0;
		item->gotoitem = repeatat;
		TAILQ_निवेशय_पुच्छ(&कालसारणी, item, entries);
	}

	प्रत्यागम;
}

/* return number of processes killed, -1 on error */
पूर्णाङ्क do_stop(ध्रुव वर्ण *applet, ध्रुव वर्ण *exec, ध्रुव वर्ण *ध्रुव *argv,
    pid_t pid, uid_t uid, पूर्णाङ्क sig, bool group, bool test, bool quiet)
{
	ध्रुव वर्ण *विधा = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	पूर्णाङ्क nkilled = 0;
	uid_t target;
	यदि (group)
		विधा = "GID";

	यदि (pid > 0)
		pids = rc_find_pids(रिक्त, रिक्त, 0, pid);
	अन्यथा
		pids = rc_find_pids(exec, argv, uid, 0);

	यदि (!pids)
		प्रत्यागम 0;

	LIST_FOREACH_SAFE(pi, pids, entries, np) {
		यदि (test) {
			einfo("Would send signal %d to %s %d", sig, विधा, pi->pid);
			nkilled++;
		} अन्यथा {
			यदि (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, विधा, pi->pid);
				यदि (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, विधा, pi->pid);
			}
			errno = 0;
			यदि (group)
				target = -pi->pid;
			अन्यथा
				target = pi->pid;
			killed = (kill(target, sig) == 0 ||
			    errno == ESRCH ? true : false);
			यदि (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, विधा, pi->pid, strerror(errno));
			अन्यथा यदि (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, विधा, pi->pid, strerror(errno));
			यदि (!killed) {
				nkilled = -1;
			} अन्यथा {
				यदि (nkilled != -1)
					nkilled++;
			}
		}
		मुक्त(pi);
	}

	मुक्त(pids);
	प्रत्यागम nkilled;
}

पूर्णाङ्क सततम्_stop_कालसारणी(ध्रुव वर्ण *applet,
		ध्रुव वर्ण *exec, ध्रुव वर्ण *ध्रुव *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool प्रगतिः, bool quiet)
{
	SCHEDULEITEM *item = TAILQ_FIRST(&कालसारणी);
	पूर्णाङ्क nkilled = 0;
	पूर्णाङ्क tkilled = 0;
	पूर्णाङ्क nrunning = 0;
	दीर्घ nloops, nsecs;
	संरचना timespec ts;
	ध्रुव वर्ण *ध्रुव *p;
	bool progressed = false;

	यदि (!(pid > 0 || exec || uid || (argv && *argv)))
		प्रत्यागम 0;

	यदि (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	यदि (pid > 0) {
		यदि (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} अन्यथा {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	यदि (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	यदि (argv && *argv) {
		einfovn("Will stop processes of `");
		यदि (rc_yesno(getenv("EINFO_VERBOSE"))) {
			प्रति (p = argv; p && *p; p++) {
				यदि (p != argv)
					प्रारूप_लिख(" ");
				प्रारूप_लिख("%s", *p);
			}
			प्रारूप_लिख("'\n");
		}
	}

	यावत् (item) {
		विकल्प (item->type) {
		पक्ष SC_GOTO:
			item = item->gotoitem;
			अनुवर्त;

		पक्ष SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, item->मानम्, group,
					test, quiet);
			यदि (nkilled == 0) {
				यदि (tkilled == 0) {
					यदि (progressed)
						प्रारूप_लिख("\n");
					eerror("%s: no matching processes found", applet);
				}
				प्रत्यागम tkilled;
			}
			अन्यथा यदि (nkilled == -1)
				प्रत्यागम 0;

			tkilled += nkilled;
			भङ्ग;
		पक्ष SC_FOREVER:
		पक्ष SC_TIMEOUT:
			यदि (item->type == SC_TIMEOUT && item->मानम् < 1) {
				item = रिक्त;
				भङ्ग;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			प्रति (nsecs = 0; item->type == SC_FOREVER || nsecs < item->मानम्; nsecs++) {
				प्रति (nloops = 0;
				     nloops < ONE_SECOND / POLL_INTERVAL;
				     nloops++)
				{
					यदि ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						प्रत्यागम 0;


					यदि (nanosleep(&ts, रिक्त) == -1) {
						यदि (progressed) {
							प्रारूप_लिख("\n");
							progressed = false;
						}
						यदि (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							प्रत्यागम 0;
						}
					}
				}
				यदि (प्रगतिः) {
					प्रारूप_लिख(".");
					fflush(stdout);
					progressed = true;
				}
			}
			भङ्ग;
		स्वभाव:
			यदि (progressed) {
				प्रारूप_लिख("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, item->type);
			प्रत्यागम 0;
		}

		यदि (item)
			item = TAILQ_अग्र(item, entries);
	}

	यदि (test || (tkilled > 0 && nrunning == 0))
		प्रत्यागम nkilled;

	यदि (progressed)
		प्रारूप_लिख("\n");
	यदि (!quiet) {
		यदि (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		अन्यथा
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	प्रत्यागम -nrunning;
}
