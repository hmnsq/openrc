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
#నిర్వచించు POLL_INTERVAL   20000000
#నిర్వచించు WAIT_PIDFILE   500000000
#నిర్వచించు ONE_రెండవ    1000000000
#నిర్వచించు ONE_MS           1000000

#అన్తర్భావ <ctype.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <syslog.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <sys/types.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "schedules.h"
#అన్తర్భావ "helpers.h"

రకనిర్వచనం నిర్మాణం scheduleitem {
	గణన {
		SC_TIMEOUT,
		SC_SIGNAL,
		SC_GOTO,
		SC_FOREVER,
	} type;
	పూర్ణాంకం విలువ;
	నిర్మాణం scheduleitem *gotoitem;
	TAILQ_నమోదు(scheduleitem) entries;
} SCHEDULEITEM;

స్థిర TAILQ_తల(, scheduleitem) కాలపట్టిక;

శూన్య free_schedulelist(శూన్య)
{
	SCHEDULEITEM *s1 = TAILQ_మొదటి(&కాలపట్టిక);
	SCHEDULEITEM *s2;

	అయితే (s1) {
		s2 = TAILQ_తదుపరి(s1, entries);
		విడుదల(s1);
		s1 = s2;
	}
	TAILQ_INIT(&కాలపట్టిక);
}

పూర్ణాంకం parse_signal(స్థిరరాశి అక్షరం *applet, స్థిరరాశి అక్షరం *sig)
{
	రకనిర్వచనం నిర్మాణం signalpair
	{
		స్థిరరాశి అక్షరం *name;
		పూర్ణాంకం సంకేతం;
	} SIGNALPAIR;

#నిర్వచించు signalpair_అంశం(name) { #name, SIG##name },

	స్థిర స్థిరరాశి SIGNALPAIR signallist[] = {
		signalpair_అంశం(HUP)
		signalpair_అంశం(INT)
		signalpair_అంశం(QUIT)
		signalpair_అంశం(ILL)
		signalpair_అంశం(TRAP)
		signalpair_అంశం(ABRT)
		signalpair_అంశం(BUS)
		signalpair_అంశం(FPE)
		signalpair_అంశం(KILL)
		signalpair_అంశం(USR1)
		signalpair_అంశం(SEGV)
		signalpair_అంశం(USR2)
		signalpair_అంశం(PIPE)
		signalpair_అంశం(ALRM)
		signalpair_అంశం(పదం_2)
		signalpair_అంశం(CHLD)
		signalpair_అంశం(CONT)
		signalpair_అంశం(STOP)
		signalpair_అంశం(TSTP)
		signalpair_అంశం(TTIN)
		signalpair_అంశం(TTOU)
		signalpair_అంశం(URG)
		signalpair_అంశం(XCPU)
		signalpair_అంశం(XFSZ)
		signalpair_అంశం(VTALRM)
		signalpair_అంశం(PROF)
#నిర్వచితమైతే SIGWINCH
		signalpair_అంశం(WINCH)
#అయితే_ముగింపు
#నిర్వచితమైతే SIGIO
		signalpair_అంశం(IO)
#అయితే_ముగింపు
#నిర్వచితమైతే SIGPWR
		signalpair_అంశం(PWR)
#అయితే_ముగింపు
		signalpair_అంశం(SYS)
		{ "NULL",	0 },
	};

	గుర్తులేని పూర్ణాంకం i = 0;
	స్థిరరాశి అక్షరం *s;

	ఒకవేళ (!sig || *sig == '\0')
		తిరిగి -1;

	ఒకవేళ (అక్షరశ్రేణి_చదువు(sig, "%u", &i) == 1) {
		ఒకవేళ (i < NSIG)
			తిరిగి i;
		eerrorx("%s: `%s' is not a valid signal", applet, sig);
	}

	ఒకవేళ (అక్షరశ్రేణి_పోలిక(sig, "SIG", 3) == 0)
		s = sig + 3;
	లేకపోతే
		s = ఖాళీ;

	కొరకు (i = 0; i < ARRAY_కొలత(signallist); ++i)
		ఒకవేళ (strcmp(sig, signallist[i].name) == 0 ||
		    (s && strcmp(s, signallist[i].name) == 0))
			తిరిగి signallist[i].సంకేతం;

	eerrorx("%s: `%s' is not a valid signal", applet, sig);
	/* NOTREACHED */
}

స్థిర SCHEDULEITEM *parse_కాలపట్టిక_అంశం(స్థిరరాశి అక్షరం *applet, స్థిరరాశి అక్షరం *string)
{
	స్థిరరాశి అక్షరం *after_hyph;
	పూర్ణాంకం sig;
	SCHEDULEITEM *అంశం = xmalloc(పరిమాణం(*అంశం));

	అంశం->విలువ = 0;
	అంశం->gotoitem = ఖాళీ;
	ఒకవేళ (strcmp(string,"forever") == 0)
		అంశం->type = SC_FOREVER;
	లేకపోతే ఒకవేళ (అంకెనా((గుర్తులేని అక్షరం)string[0])) {
		అంశం->type = SC_TIMEOUT;
		errno = 0;
		ఒకవేళ (అక్షరశ్రేణి_చదువు(string, "%d", &అంశం->విలువ) != 1)
			eerrorx("%s: invalid timeout value in schedule '%s'",
			    applet, string);
	} లేకపోతే ఒకవేళ ((after_hyph = string + (string[0] == '-')) &&
	    ((sig = parse_signal(applet, after_hyph)) != -1))
	{
		అంశం->type = SC_SIGNAL;
		అంశం->విలువ = (పూర్ణాంకం)sig;
	} లేకపోతే
		eerrorx("%s: invalid schedule item `%s'", applet, string);

	తిరిగి అంశం;
}

శూన్య parse_కాలపట్టిక(స్థిరరాశి అక్షరం *applet, స్థిరరాశి అక్షరం *string, పూర్ణాంకం timeout)
{
	అక్షరం బఫరు[20];
	స్థిరరాశి అక్షరం *slash;
	పూర్ణాంకం లెక్క = 0;
	SCHEDULEITEM *repeatat = ఖాళీ;
	size_t len;
	SCHEDULEITEM *అంశం;

	TAILQ_INIT(&కాలపట్టిక);
	ఒకవేళ (string)
		కొరకు (slash = string; *slash; slash++)
			ఒకవేళ (*slash == '/')
				లెక్క++;

	free_schedulelist();

	ఒకవేళ (లెక్క == 0) {
		అంశం = xmalloc(పరిమాణం(*అంశం));
		అంశం->type = SC_SIGNAL;
		అంశం->విలువ = timeout;
		అంశం->gotoitem = ఖాళీ;
		TAILQ_చొప్పించు_తోక(&కాలపట్టిక, అంశం, entries);

		అంశం = xmalloc(పరిమాణం(*అంశం));
		అంశం->type = SC_TIMEOUT;
		అంశం->gotoitem = ఖాళీ;
		TAILQ_చొప్పించు_తోక(&కాలపట్టిక, అంశం, entries);
		ఒకవేళ (string) {
			ఒకవేళ (అక్షరశ్రేణి_చదువు(string, "%d", &అంశం->విలువ) != 1)
				eerrorx("%s: invalid timeout value in schedule '%s'",
					applet, string);
		} లేకపోతే
			అంశం->విలువ = 5;

		తిరిగి;
	}

	అయితే (string != ఖాళీ) {
		ఒకవేళ ((slash = strchr(string, '/')))
			len = slash - string;
		లేకపోతే
			len = అక్షరశ్రేణి_పొడవు(string);

		ఒకవేళ (len >= (ptrdiff_t)పరిమాణం(బఫరు))
			eerrorx("%s: invalid schedule item, far too long",
			    applet);

		జ్ఞాపకం_నకలు(బఫరు, string, len);
		బఫరు[len] = 0;
		string = slash ? slash + 1 : ఖాళీ;

		అంశం = parse_కాలపట్టిక_అంశం(applet, బఫరు);
		TAILQ_చొప్పించు_తోక(&కాలపట్టిక, అంశం, entries);
		ఒకవేళ (అంశం->type == SC_FOREVER) {
			ఒకవేళ (repeatat)
				eerrorx("%s: invalid schedule, `forever' "
				    "appears more than once", applet);

			repeatat = అంశం;
			కొనసాగు;
		}
	}

	ఒకవేళ (repeatat) {
		అంశం = xmalloc(పరిమాణం(*అంశం));
		అంశం->type = SC_GOTO;
		అంశం->విలువ = 0;
		అంశం->gotoitem = repeatat;
		TAILQ_చొప్పించు_తోక(&కాలపట్టిక, అంశం, entries);
	}

	తిరిగి;
}

/* return number of processes killed, -1 on error */
పూర్ణాంకం do_stop(స్థిరరాశి అక్షరం *applet, స్థిరరాశి అక్షరం *exec, స్థిరరాశి అక్షరం *స్థిరరాశి *argv,
    pid_t pid, uid_t uid, పూర్ణాంకం sig, bool group, bool test, bool quiet)
{
	స్థిరరాశి అక్షరం *రకం = "PID";
	RC_PIDLIST *pids;
	RC_PID *pi;
	RC_PID *np;
	bool killed;
	పూర్ణాంకం nkilled = 0;
	uid_t లక్ష్యం;
	ఒకవేళ (group)
		రకం = "GID";

	ఒకవేళ (pid > 0)
		pids = rc_కనుగొను_pids(ఖాళీ, ఖాళీ, 0, pid);
	లేకపోతే
		pids = rc_కనుగొను_pids(exec, argv, uid, 0);

	ఒకవేళ (!pids)
		తిరిగి 0;

	LIST_FOREACH_సురక్షిత(pi, pids, entries, np) {
		ఒకవేళ (test) {
			einfo("Would send signal %d to %s %d", sig, రకం, pi->pid);
			nkilled++;
		} లేకపోతే {
			ఒకవేళ (sig) {
				syslog(LOG_DEBUG, "Sending signal %d to %s %d", sig, రకం, pi->pid);
				ఒకవేళ (!quiet)
					ebeginv("Sending signal %d to %s %d", sig, రకం, pi->pid);
			}
			errno = 0;
			ఒకవేళ (group)
				లక్ష్యం = -pi->pid;
			లేకపోతే
				లక్ష్యం = pi->pid;
			killed = (kill(లక్ష్యం, sig) == 0 ||
			    errno == ESRCH ? true : false);
			ఒకవేళ (!quiet)
				eendv(killed ? 0 : 1,
				"%s: failed to send signal %d to %s %d: %s",
				applet, sig, రకం, pi->pid, strerror(errno));
			లేకపోతే ఒకవేళ (!killed)
				syslog(LOG_ERR, "Failed to send signal %d to %s %d: %s",
						sig, రకం, pi->pid, strerror(errno));
			ఒకవేళ (!killed) {
				nkilled = -1;
			} లేకపోతే {
				ఒకవేళ (nkilled != -1)
					nkilled++;
			}
		}
		విడుదల(pi);
	}

	విడుదల(pids);
	తిరిగి nkilled;
}

పూర్ణాంకం నిరంతరం_stop_కాలపట్టిక(స్థిరరాశి అక్షరం *applet,
		స్థిరరాశి అక్షరం *exec, స్థిరరాశి అక్షరం *స్థిరరాశి *argv,
		pid_t pid, uid_t uid, bool group,
    bool test, bool పురోగతి, bool quiet)
{
	SCHEDULEITEM *అంశం = TAILQ_మొదటి(&కాలపట్టిక);
	పూర్ణాంకం nkilled = 0;
	పూర్ణాంకం tkilled = 0;
	పూర్ణాంకం nrunning = 0;
	దీర్ఘ nloops, nsecs;
	నిర్మాణం timespec ts;
	స్థిరరాశి అక్షరం *స్థిరరాశి *p;
	bool progressed = false;

	ఒకవేళ (!(pid > 0 || exec || uid || (argv && *argv)))
		తిరిగి 0;

	ఒకవేళ (exec) {
		einfov("Will stop %s", exec);
		syslog(LOG_DEBUG, "Will stop %s", exec);
	}
	ఒకవేళ (pid > 0) {
		ఒకవేళ (!group) {
			einfov("Will stop PID %d", pid);
			syslog(LOG_DEBUG, "Will stop PID %d", pid);
		} లేకపోతే {
			einfov("Will stop GID %d", pid);
			syslog(LOG_DEBUG, "Will stop GID %d", pid);
		}
	}
	ఒకవేళ (uid) {
		einfov("Will stop processes owned by UID %d", uid);
		syslog(LOG_DEBUG, "Will stop processes owned by UID %d", uid);
	}
	ఒకవేళ (argv && *argv) {
		einfovn("Will stop processes of `");
		ఒకవేళ (rc_yesno(getenv("EINFO_VERBOSE"))) {
			కొరకు (p = argv; p && *p; p++) {
				ఒకవేళ (p != argv)
					రాయి(" ");
				రాయి("%s", *p);
			}
			రాయి("'\n");
		}
	}

	అయితే (అంశం) {
		ఎంపిక (అంశం->type) {
		స్థితి SC_GOTO:
			అంశం = అంశం->gotoitem;
			కొనసాగు;

		స్థితి SC_SIGNAL:
			nrunning = 0;
			nkilled = do_stop(applet, exec, argv, pid, uid, అంశం->విలువ, group,
					test, quiet);
			ఒకవేళ (nkilled == 0) {
				ఒకవేళ (tkilled == 0) {
					ఒకవేళ (progressed)
						రాయి("\n");
					eerror("%s: no matching processes found", applet);
				}
				తిరిగి tkilled;
			}
			లేకపోతే ఒకవేళ (nkilled == -1)
				తిరిగి 0;

			tkilled += nkilled;
			ఆపు;
		స్థితి SC_FOREVER:
		స్థితి SC_TIMEOUT:
			ఒకవేళ (అంశం->type == SC_TIMEOUT && అంశం->విలువ < 1) {
				అంశం = ఖాళీ;
				ఆపు;
			}

			ts.tv_sec = 0;
			ts.tv_nsec = POLL_INTERVAL;

			కొరకు (nsecs = 0; అంశం->type == SC_FOREVER || nsecs < అంశం->విలువ; nsecs++) {
				కొరకు (nloops = 0;
				     nloops < ONE_రెండవ / POLL_INTERVAL;
				     nloops++)
				{
					ఒకవేళ ((nrunning = do_stop(applet, exec, argv,
						    pid, uid, 0, group, test, quiet)) == 0)
						తిరిగి 0;


					ఒకవేళ (nanosleep(&ts, ఖాళీ) == -1) {
						ఒకవేళ (progressed) {
							రాయి("\n");
							progressed = false;
						}
						ఒకవేళ (errno != EINTR) {
							eerror("%s: nanosleep: %s",
							    applet, strerror(errno));
							తిరిగి 0;
						}
					}
				}
				ఒకవేళ (పురోగతి) {
					రాయి(".");
					fflush(stdout);
					progressed = true;
				}
			}
			ఆపు;
		అప్రమేయ:
			ఒకవేళ (progressed) {
				రాయి("\n");
				progressed = false;
			}
			eerror("%s: invalid schedule item `%d'",
			    applet, అంశం->type);
			తిరిగి 0;
		}

		ఒకవేళ (అంశం)
			అంశం = TAILQ_తదుపరి(అంశం, entries);
	}

	ఒకవేళ (test || (tkilled > 0 && nrunning == 0))
		తిరిగి nkilled;

	ఒకవేళ (progressed)
		రాయి("\n");
	ఒకవేళ (!quiet) {
		ఒకవేళ (nrunning == 1)
			eerror("%s: %d process refused to stop", applet, nrunning);
		లేకపోతే
			eerror("%s: %d process(es) refused to stop", applet, nrunning);
	}

	తిరిగి -nrunning;
}
