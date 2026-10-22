/*
 * supervise-daemon
 * This is a supervisor for daemons.
 * It will start a daemon and make sure it restarts if it crashes.
 */

/*
 * Copyright (c) 2016-2025 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#अन्तर्भाव <errno.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <grp.h>
#अन्तर्भाव <pwd.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <strings.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#अन्तर्भाव <syslog.h>
#अन्तर्भाव <sys/ioctl.h>
#अन्तर्भाव <sys/resource.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <unistd.h>

#ifdef HAVE_PAM
#अन्तर्भाव <security/pam_appl.h>

/* We are not supporting authentication conversations */
स्थिर संरचना pam_conv conv = { रिक्त, रिक्त};
#endif

#ifdef __linux__
#अन्तर्भाव <sys/capability.h>
#endif

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "rc_exec.h"
#अन्तर्भाव "plugin.h"
#अन्तर्भाव "schedules.h"
#अन्तर्भाव "timeutils.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
गणना {
  /* This has to come first so following values stay in the 0x100+ range. */
  LONGOPT_BASE = 0x100,

  LONGOPT_CAPABILITIES,
  LONGOPT_OOM_SCORE_ADJ,
  LONGOPT_NO_NEW_PRIVS,
  LONGOPT_SECBITS,
  LONGOPT_STDERR_LOGGER,
  LONGOPT_STDOUT_LOGGER,
  LONGOPT_NOTIFY,
  LONGOPT_RESPAWN_DELAY_STEP,
  LONGOPT_RESPAWN_DELAY_CAP,
};

अचर वर्ण *applet = रिक्त;
अचर वर्ण *extraopts = रिक्त;
अचर वर्ण getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
अचर संरचना option longopts[] = {
	{ "healthcheck-timer",        1, रिक्त, 'a'},
	{ "healthcheck-delay",        1, रिक्त, 'A'},
	{ "capabilities", 1, रिक्त, LONGOPT_CAPABILITIES},
	{ "secbits",      1, रिक्त, LONGOPT_SECBITS},
	{ "no-new-privs", 0, रिक्त, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, रिक्त, 'D'},
	{ "respawn-delay-step",   1, रिक्त, LONGOPT_RESPAWN_DELAY_STEP},
	{ "respawn-delay-cap",    1, रिक्त, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, रिक्त, 'd'},
	{ "env",          1, रिक्त, 'e'},
	{ "group",        1, रिक्त, 'g'},
	{ "stop-group",   0, रिक्त, 'G'},
	{ "ionice",       1, रिक्त, 'I'},
	{ "stop",         0, रिक्त, 'K'},
	{ "umask",        1, रिक्त, 'k'},
	{ "respawn-max",    1, रिक्त, 'm'},
	{ "nicelevel",    1, रिक्त, 'N'},
	{ "oom-score-adj",1, रिक्त, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, रिक्त, 'p'},
	{ "respawn-period",        1, रिक्त, 'P'},
	{ "retry",       1, रिक्त, 'R'},
	{ "chroot",       1, रिक्त, 'r'},
	{ "signal",       1, रिक्त, 's'},
	{ "start",        0, रिक्त, 'S'},
	{ "user",         1, रिक्त, 'u'},
	{ "stdin",        1, रिक्त, '0'},
	{ "stdout",       1, रिक्त, '1'},
	{ "stderr",       1, रिक्त, '2'},
	{ "stdout-logger",1, रिक्त, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, रिक्त, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, रिक्त, '3'},
	{ "notify",       1, रिक्त, LONGOPT_NOTIFY},
	longopts_COMMON
};
अचर वर्ण * अचर longopts_help[] = {
	"set an initial health check delay",
	"set a health check timer",
	"Set the inheritable, ambient and bounding capabilities",
	"Set the security-bits for the program",
	"Set the No New Privs flag for the program",
	"Set a respawn delay",
	"Increase the respawn delay by this amount for every retry",
	"Set maximum respawn delay when respawn-delay-step is also active",
	"Change the PWD",
	"Set an environment string",
	"Change the process group",
	"Stop the whole process group",
	"Set an ionice class:data when starting",
	"Stop daemon",
	"Set the umask for the daemon",
	"set maximum number of respawn attempts",
	"Set a nicelevel when starting",
	"Set OOM score adjustment when starting",
	"Match pid found in this file",
	"Set respawn time period",
	"Retry schedule to use when stopping",
	"Chroot to this directory",
	"Send a signal to the daemon",
	"Start daemon",
	"Change the process user",
	"Redirect stdin to file",
	"Redirect stdout to file",
	"Redirect stderr to file",
	"Redirect stdout to process",
	"Redirect stderr to process",
	"reexec (used internally)",
	"Configures experimental notification behaviour",
	longopts_help_COMMON
};
अचर वर्ण *usagestring = रिक्त;

स्थिर पूर्णाङ्क healthcheckdelay = 0;
स्थिर पूर्णाङ्क healthchecktimer = 0;
स्थिर चञ्चल संक_पूर्ण_प्रकार do_healthcheck = 0;
स्थिर चञ्चल संक_पूर्ण_प्रकार exiting = 0;
स्थिर पूर्णाङ्क nicelevel = पूर्णांक_न्यून;
स्थिर पूर्णाङ्क ionicec = -1;
स्थिर पूर्णाङ्क ioniced = 0;
स्थिर पूर्णाङ्क oom_score_adj = पूर्णांक_न्यून;
स्थिर वर्ण *changeuser, *ch_जरा, *ch_dir;
स्थिर uid_t uid = 0;
स्थिर gid_t gid = 0;
स्थिर bool stopgroup = false;
स्थिर पूर्णाङ्क devnull_fd = -1;
स्थिर पूर्णाङ्क stdin_fd;
स्थिर पूर्णाङ्क stdout_fd;
स्थिर पूर्णाङ्क stderr_fd;
स्थिर संरचना notify notify;
स्थिर वर्ण *redirect_stdin = रिक्त;
स्थिर वर्ण *redirect_stderr = रिक्त;
स्थिर वर्ण *redirect_stdout = रिक्त;
स्थिर वर्ण *stderr_process = रिक्त;
स्थिर वर्ण *stdout_process = रिक्त;
#ifdef TIOCNOTTY
स्थिर पूर्णाङ्क tty_fd = -1;
#endif
स्थिर pid_t child_pid;
स्थिर पूर्णाङ्क respawn_गन्ती = 0;
स्थिर int64_t respawn_delay; /* default set inside main() */
स्थिर int64_t respawn_delay_step = TM_MS(128);
स्थिर int64_t respawn_delay_cap = TM_SEC(30);
स्थिर int64_t respawn_period; /* default set inside main() */
स्थिर पूर्णाङ्क respawn_max = 10;
स्थिर वर्ण *fifopath = रिक्त;
स्थिर पूर्णाङ्क fifo_fd = 0;
स्थिर वर्ण *pidfile = रिक्त;
स्थिर वर्ण *svcname = रिक्त;
स्थिर bool verbose = false;
#ifdef __linux__
स्थिर cap_iab_t cap_iab = रिक्त;
स्थिर अचिन्हित secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
स्थिर bool no_new_privs = false;
#endif

बाह्य वर्ण **environ;

#यदि !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#यदि !defined(__DragonFly__)
स्थिर अन्तःस्थ पूर्णाङ्क ioprio_set(पूर्णाङ्क which RC_UNUSED, पूर्णाङ्क who RC_UNUSED,
			     पूर्णाङ्क ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	फर्काऊ syscall(SYS_ioprio_set, which, who, ioprio);
#अन्यथा
	फर्काऊ 0;
#endif
}
#endif

स्थिर शून्य cleanup(शून्य)
{
	मुक्त(changeuser);
}

RC_NORETURN स्थिर शून्य re_exec_supervisor(शून्य)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(वर्ण *) रिक्त);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			म_त्रुटि(त्रुटि_सं));
	निकास(निकास_त्रुटि);
}

स्थिर शून्य handle_signal(पूर्णाङ्क sig)
{
	पूर्णाङ्क serrno = त्रुटि_सं;
	pid_t pid;

	विकल्प (sig) {
	पक्ष SIGALRM:
		do_healthcheck = 1;
		रोक;
	पक्ष SIGCHLD:
		यदि (exiting)
			जबसम्म (waitpid((pid_t)(-1), रिक्त, WNOHANG) > 0) {}
		अन्यथा {
			जबसम्म ((pid = waitpid((pid_t)(-1), रिक्त, WNOHANG|WNOWAIT)) > 0) {
				यदि (pid == child_pid)
					रोक;
				pid = waitpid(pid, रिक्त, WNOHANG);
			}
		}
		रोक;
	पक्ष संक_इति:
		exiting = 1;
		रोक;
	स्वभाव:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	त्रुटि_सं = serrno;
}

स्थिर वर्ण * expand_home(अचर वर्ण *home, अचर वर्ण *path)
{
	वर्ण *opath, *ppath, *p, *nh;
	संरचना passwd *pw;

	यदि (!path || *path != '~')
		फर्काऊ xstrdup(path);

	opath = ppath = xstrdup(path);
	यदि (ppath[1] != '/' && ppath[1] != '\0') {
		p = म_अक्षर(ppath + 1, '/');
		यदि (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		यदि (pw) {
			home = pw->pw_dir;
			ppath = p;
			यदि (ppath)
				*ppath = '/';
		} अन्यथा
			home = रिक्त;
	} अन्यथा
		ppath++;

	यदि (!home) {
		मुक्त(opath);
		फर्काऊ xstrdup(path);
	}
	यदि (!ppath) {
		मुक्त(opath);
		फर्काऊ xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	मुक्त(opath);
	फर्काऊ nh;
}

स्थिर वर्ण *make_cmdline(वर्ण **argv)
{
	वर्ण **c;
	वर्ण *cmdline;
	माप_प्रकार len;
	खाता *mem = xopen_memstream(&cmdline, &len);

	लागि (c = argv; c && *c; c++) {
		ख_माला_दो(*c, mem);
		यदि (c[1])
			ख_अक्षर_दो(' ', mem);
	}
	xclose_memstream(mem);
	फर्काऊ cmdline;
}

स्थिर pid_t exec_command(अचर वर्ण *cmd)
{
	वर्ण *file;
	pid_t pid = -1;
	sigset_t full;
	sigset_t old;
	संरचना sigaction sa;

	file = rc_service_resolve(svcname);
	यदि (access(file, F_OK) != 0) {
		मुक्त(file);
		फर्काऊ 0;
	}

	/* We need to block signals until we have forked */
	स_रखो(&sa, 0, परिमाण (sa));
	sa.sa_handler = संक_पूर्व;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);

	pid = विभाजन();
	यदि (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, रिक्त);
		sigaction(SIGHUP, &sa, रिक्त);
		sigaction(संक_विघ्न, &sa, रिक्त);
		sigaction(SIGQUIT, &sa, रिक्त);
		sigaction(संक_इति, &sa, रिक्त);
		sigaction(SIGUSR1, &sa, रिक्त);
		sigaction(SIGWINCH, &sa, रिक्त);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, रिक्त);

		/* Safe to run now */
		execl(file, file, cmd, (वर्ण *) रिक्त);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, म_त्रुटि(त्रुटि_सं));
		_exit(निकास_त्रुटि);
	}

	यदि (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",म_त्रुटि (त्रुटि_सं));

	sigprocmask(SIG_SETMASK, &old, रिक्त);
	मुक्त(file);
	फर्काऊ pid;
}

RC_NORETURN स्थिर शून्य child_process(वर्ण *exec, वर्ण **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	पूर्णाङ्क i;
	वर्ण *p;
	वर्ण *token;
	माप_प्रकार len;
	वर्ण *newpath;
	वर्ण *np;
	वर्ण *cmdline = रिक्त;
	समय_प्रकार start_time;
	वर्ण start_गन्ती_string[20];
	वर्ण start_time_string[20];
	खाता *fp;
	gid_t group_buf[32], *group_list = group_buf;
	पूर्णाङ्क group_गन्ती = ARRAY_SIZE(group_buf);

#ifdef HAVE_PAM
	pam_handle_t *pamh = रिक्त;
	पूर्णाङ्क pamr;
	अचर वर्ण *अचर *pamenv = रिक्त;
#endif

	setsid();

	यदि (svcname) {
		start_time = समय(रिक्त);
		from_time_t(start_time_string, start_time);
		rc_service_मान_set(svcname, "start_time", start_time_string);
		प्र_लिखो(start_गन्ती_string, "%i", respawn_गन्ती);
		rc_service_मान_set(svcname, "start_count", start_गन्ती_string);
		प्र_लिखो(start_गन्ती_string, "%d", getpid());
		rc_service_मान_set(svcname, "child_pid", start_गन्ती_string);
	}

	यदि (nicelevel != पूर्णांक_न्यून) {
		यदि (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					म_त्रुटि(त्रुटि_सं));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = ख_खोलो("/proc/self/autogroup", "r+");
		यदि (fp) {
			ख_लिखो(fp, "%d\n", nicelevel);
			ख_बंद(fp);
		} अन्यथा यदि (त्रुटि_सं != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, म_त्रुटि(त्रुटि_सं));
	}

	यदि (ionicec != -1 && ioprio_set(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				म_त्रुटि(त्रुटि_सं));

	यदि (oom_score_adj != पूर्णांक_न्यून) {
		fp = ख_खोलो("/proc/self/oom_score_adj", "w");
		यदि (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, म_त्रुटि(त्रुटि_सं));
		ख_लिखो(fp, "%d\n", oom_score_adj);
		ख_बंद(fp);
	}

#ifdef HAVE_PAM
	यदि (changeuser != रिक्त) {
		pamr = pam_start("supervise-daemon",
		    changeuser, &conv, &pamh);

		यदि (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		यदि (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		यदि (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#endif

	यदि (changeuser && getgrouplist(changeuser, gid, group_list, &group_गन्ती) < 0) {
		group_list = xmalloc(group_गन्ती * परिमाण(*group_list));
		यदि (getgrouplist(changeuser, gid, group_list, &group_गन्ती) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	यदि (ch_जरा && chroot(ch_जरा) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_जरा, म_त्रुटि(त्रुटि_सं));

	यदि (ch_dir && स_बदलो(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, म_त्रुटि(त्रुटि_सं));

	यदि (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	यदि (changeuser && setgroups(group_गन्ती, group_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	यदि (group_list != group_buf)
		मुक्त(group_list);
#ifdef __linux__
	यदि (uid && cap_setuid(uid))
#अन्यथा
	यदि (uid && setuid(uid))
#endif
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#ifdef __linux__
	यदि (cap_iab != रिक्त) {
		i = cap_iab_set_proc(cap_iab);

		यदि (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", म_त्रुटि(त्रुटि_सं));

		यदि (i != 0)
			eerrorx("Could not set iab: %s", म_त्रुटि(त्रुटि_सं));
	}

	यदि (secbits != 0) {
		यदि (cap_set_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, म_त्रुटि(त्रुटि_सं));
	}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
	यदि (no_new_privs) {
		यदि (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
			eerrorx("Could not set No New Privs flag: %s", म_त्रुटि(त्रुटि_सं));
	}
#endif

	/* remove the controlling tty */
#ifdef TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	बंद(tty_fd);
#endif

	/* Clean the environment of any RC_ variables */
	env_list = rc_stringlist_new();
	i = 0;
	जबसम्म (environ[i])
		rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
	यदि (changeuser != रिक्त) {
		pamenv = (अचर वर्ण *अचर *)pam_getenvlist(pamh);
		यदि (pamenv) {
			जबसम्म (*pamenv) {
				/* Don't add strings unless they set a var */
				यदि (म_अक्षर(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				अन्यथा
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#endif

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		यदि (म_भेदन(env->मान, "RC_", 3) == 0
				|| म_भेदन(env->मान, "SSD_", 4) == 0
				|| म_भेदन(env->मान, "EINFO_", 6) == 0) {
			*म_अक्षर(env->मान, '=') = '\0';
			unsetenv(env->मान);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	यदि ((token = दो_पर्या("PATH"))) {
		len = म_माप(token);
		newpath = np = xmalloc(len + 1);
		जबसम्म (token && *token) {
			p = म_अक्षर(token, ':');
			यदि (p) {
				*p++ = '\0';
				जबसम्म (*p == ':')
					p++;
			}
			यदि (म_भेद(token, RC_LIBEXECDIR "/bin") != 0 &&
			    म_भेद(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = म_माप(token);
				यदि (np != newpath)
					*np++ = ':';
				स_नकल(np, token, len);
				np += len;
				}
			token = p;
		}
		*np = '\0';
		unsetenv("PATH");
		setenv("PATH", newpath, 1);
	}

	stdin_fd = devnull_fd;
	stdout_fd = devnull_fd;
	stderr_fd = devnull_fd;
	यदि (redirect_stdin) {
		यदि ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, म_त्रुटि(त्रुटि_सं));
	}
	यदि (redirect_stdout) {
		यदि ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, म_त्रुटि(त्रुटि_सं));
	} अन्यथा यदि (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		यदि (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, म_त्रुटि(त्रुटि_सं));
	}
	यदि (redirect_stderr) {
		यदि ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, म_त्रुटि(त्रुटि_सं));
	} अन्यथा यदि (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		यदि (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, म_त्रुटि(त्रुटि_सं));
	}

	dup2(stdin_fd, STDIN_FILENO);
	यदि (redirect_stdout || stdout_process || rc_yesno(दो_पर्या("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	यदि (redirect_stderr || stderr_process || rc_yesno(दो_पर्या("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	यदि (notify.type == NOTIFY_FD) {
		पूर्णाङ्क error;

		यदि (notify.fd == notify.pipe[1]) {
			पूर्णाङ्क flags = error = fcntl(notify.fd, F_GETFD, 0);
			यदि (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} अन्यथा {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		यदि (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, म_त्रुटि(त्रुटि_सं));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	मुक्त(cmdline);
	execvp(exec, argv);

#ifdef HAVE_PAM
	यदि (changeuser != रिक्त && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#endif
	eerrorx("%s: failed to exec `%s': %s", applet, exec,म_त्रुटि(त्रुटि_सं));
}

RC_NORETURN स्थिर शून्य supervisor(वर्ण *exec, वर्ण **argv)
{
	खाता *fp;
	वर्ण buf[2048];
	वर्ण cmd[2048];
	पूर्णाङ्क गन्ती;
	पूर्णाङ्क failing;
	पूर्णाङ्क health_status;
	पूर्णाङ्क healthcheck_respawn;
	पूर्णाङ्क i;
	पूर्णाङ्क nkilled;
	पूर्णाङ्क sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_signals;
	sigset_t signals;
	संरचना sigaction sa;
	int64_t respawn_now = 0;
	int64_t first_spawn = 0;
	int64_t sleep_for;

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGALRM);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, संक_इति);
	sigprocmask(SIG_SETMASK, &signals, &old_signals);

	/* install signal  handler */
	स_रखो(&sa, 0, परिमाण(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, रिक्त);
	sigaction(SIGCHLD, &sa, रिक्त);
	sigaction(संक_इति, &sa, रिक्त);

	fp = ख_खोलो(pidfile, "w");
	यदि (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, म_त्रुटि(त्रुटि_सं));
	ख_लिखो(fp, "%d\n", getpid());
	ख_बंद(fp);

	यदि (svcname)
		rc_service_daemon_set(svcname, exec, (अचर वर्ण * अचर *) argv,
				pidfile, true);

	/* remove the controlling tty */
#ifdef TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	बंद(tty_fd);
#endif

	/*
	 * Supervisor main loop
	 */
	यदि (healthcheckdelay)
		alarm(healthcheckdelay);
	अन्यथा यदि (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	जबसम्म (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = खोलो(fifopath, O_RDONLY);
		यदि (fifo_fd > 0) {
			स_रखो(buf, 0, परिमाण(buf));
			गन्ती = पठ(fifo_fd, buf, परिमाण(buf) - 1);
			बंद(fifo_fd);
			यदि (गन्ती != -1)
				buf[गन्ती] = 0;
			यदि (गन्ती == 0)
				जारी;
			यदि (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			यदि (strncasecmp(buf, "sig", 3) == 0) {
				यदि ((माला_पूछो(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							child_pid);
					यदि (समाप्त(child_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, child_pid);
				}
			}
			जारी;
		}
		यदि (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			यदि (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_status = rc_waitpid(health_pid);
			यदि (WIFEXITED(health_status) && WEXITSTATUS(health_status) == 0)
				alarm(healthchecktimer);
			अन्यथा {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
				nkilled = run_stop_schedule(applet, रिक्त, रिक्त, child_pid, 0,
						stopgroup, false, false, true);
				यदि (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							child_pid, म_त्रुटि(त्रुटि_सं));
				अन्यथा
					healthcheck_respawn = 1;
			}
		}
		यदि (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
			nkilled = run_stop_schedule(applet, रिक्त, रिक्त, child_pid, 0,
					stopgroup, false, false, true);
			यदि (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			जारी;
		}
		wait_pid = waitpid(child_pid, &i, WNOHANG);
		यदि (wait_pid == child_pid) {
			यदि (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, child_pid, WEXITSTATUS(i));
			अन्यथा यदि (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, child_pid, WTERMSIG(i));
		}
		यदि (wait_pid == child_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			यदि (first_spawn == 0)
				first_spawn = respawn_now;
			यदि ((respawn_period > 0)
					&& (respawn_now - first_spawn > respawn_period)) {
				respawn_गन्ती = 0;
				first_spawn = 0;
			} अन्यथा
				respawn_गन्ती++;
			यदि (respawn_max > 0 && respawn_गन्ती > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				जारी;
			}
			sleep_for = respawn_delay + (respawn_delay_step * respawn_गन्ती);
			यदि (respawn_delay_step > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			यदि (exiting)
				जारी;
			child_pid = विभाजन();
			यदि (child_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, म_त्रुटि(त्रुटि_सं));
				निकास(निकास_त्रुटि);
			}
			यदि (child_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_signals, रिक्त);
				स_रखो(&sa, 0, परिमाण(sa));
				sa.sa_handler = संक_पूर्व;
				sigaction(SIGALRM, &sa, रिक्त);
				sigaction(SIGCHLD, &sa, रिक्त);
				sigaction(संक_इति, &sa, रिक्त);
				child_process(exec, argv);
			}
			यदि (healthcheckdelay)
				alarm(healthcheckdelay);
			अन्यथा यदि (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	यदि (svcname) {
		rc_service_daemon_set(svcname, exec, (अचर वर्ण *अचर *)argv,
				pidfile, false);
		rc_service_मान_set(svcname, "child_pid", रिक्त);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		यदि (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	यदि (pidfile)
		unlink(pidfile);
	यदि (fifopath)
		unlink(fifopath);
	निकास(निकास_सफल);
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क opt;
	वर्ण **c;
	पूर्णाङ्क x;
	bool start = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	वर्ण *exec = रिक्त;
	वर्ण *retry = रिक्त;
	पूर्णाङ्क sig = संक_इति;
	वर्ण *home = रिक्त;
	पूर्णाङ्क tid = 0;
	pid_t pid;
	वर्ण *tmp;
	वर्ण *p;
	वर्ण *token;
	पूर्णाङ्क i;
	पूर्णाङ्क n;
	वर्ण *exec_file = रिक्त;
	वर्ण *varbuf = रिक्त;
	वर्ण numbuf[64];
	संरचना timespec ts;
	संरचना passwd *pw;
	संरचना group *gr;
	खाता *fp;
	mode_t numask = 022;
	पूर्णाङ्क child_argc = 0;
	वर्ण **child_argv = रिक्त;
	वर्ण *str = रिक्त;
	वर्ण *cmdline = रिक्त;
	अचर वर्ण *respawn_delay_str  = "0";
	अचर वर्ण *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	निकास_पर(cleanup);
	svcname = दो_पर्या("RC_SVCNAME");
	यदि (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	यदि (argc <= 1 || म_भेद(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	यदि ((tmp = दो_पर्या("SSD_NICELEVEL")))
		यदि (माला_पूछो(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	यदि ((tmp = दो_पर्या("SSD_IONICELEVEL"))) {
		n = माला_पूछो(tmp, "%d:%d", &ionicec, &ioniced);
		यदि (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		यदि (ionicec == 0)
			ioniced = 0;
		अन्यथा यदि (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	यदि ((tmp = दो_पर्या("SSD_OOM_SCORE_ADJ")))
		यदि (माला_पूछो(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = दो_पर्या("USER");
	home = दो_पर्या("HOME");
	यदि (home == रिक्त || p == रिक्त) {
		pw = getpwuid(getuid());
		यदि (pw != रिक्त) {
			यदि (p == रिक्त)
				setenv("USER", pw->pw_name, 1);
			यदि (home == रिक्त) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	यदि (svcname) {
		argc--;
		argv++;
	}
	जबसम्म ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (पूर्णाङ्क *) 0)) != -1)
		विकल्प (opt) {
		पक्ष 'a':  /* --healthcheck-timer <time> */
			यदि (माला_पूछो(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			रोक;

		पक्ष 'A':  /* --healthcheck-delay <time> */
			यदि (माला_पूछो(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			रोक;

		पक्ष LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			यदि (cap_iab == रिक्त)
				eerrorx("Could not parse iab: %s", म_त्रुटि(त्रुटि_सं));
#अन्यथा
			eerrorx("Capabilities support not enabled");
#endif
			रोक;

		पक्ष LONGOPT_SECBITS:
#ifdef __linux__
			यदि (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = रिक्त;
			secbits = म_से_अदीर्घ(optarg, &tmp, 0);
			यदि (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#अन्यथा
			eerrorx("Capabilities support not enabled");
#endif
			रोक;

		पक्ष LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#अन्यथा
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			रोक;

		पक्ष 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			रोक;

		पक्ष 'I': /* --ionice */
			यदि (माला_पूछो(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			यदि (ionicec == 0)
				ioniced = 0;
			अन्यथा यदि (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			रोक;

		पक्ष 'K':  /* --stop */
			stop = true;
			रोक;

		पक्ष 'N':  /* --nice */
			यदि (माला_पूछो(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			रोक;

		पक्ष LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			यदि (माला_पूछो(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			रोक;

		पक्ष 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			रोक;

		पक्ष 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			रोक;
		पक्ष 'S':  /* --start */
			start = true;
			रोक;

		पक्ष 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			रोक;

		पक्ष 'e': /* --env */
			putenv(optarg);
			रोक;

		पक्ष 'g':  /* --group <group>|<gid> */
			यदि (माला_पूछो(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			अन्यथा
				gr = getgrgid((gid_t)tid);
			यदि (gr == रिक्त)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			रोक;

		पक्ष 'G': /* -G */
			stopgroup = true;
			रोक;

		पक्ष 'k':
			यदि (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			रोक;

		पक्ष 'm':  /* --respawn-max count */
			n = माला_पूछो(optarg, "%d", &respawn_max);
			यदि (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			रोक;

		पक्ष 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			रोक;

		पक्ष 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			रोक;
		पक्ष 'r':  /* --chroot /new/root */
			ch_जरा = optarg;
			रोक;

		पक्ष 'u':  /* --user <username>|<uid> */
		{
			वर्ण dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			यदि (माला_पूछो(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			अन्यथा
				pw = getpwuid((uid_t)tid);

			यदि (pw == रिक्त)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			यदि (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			यदि (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			यदि (gid == 0)
				gid = pw->pw_gid;

			यदि (p) {
				tmp = strsep (&p, ":");
				यदि (माला_पूछो(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				अन्यथा
					gr = getgrgid((gid_t) tid);

				यदि (gr == रिक्त)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		रोक;

		पक्ष '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			रोक;

		पक्ष '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			रोक;

		पक्ष '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			रोक;
		पक्ष '3':  /* --reexec */
			reexec = true;
			रोक;

		पक्ष LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			रोक;

		पक्ष LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			रोक;

		पक्ष LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			रोक;

		पक्ष LONGOPT_RESPAWN_DELAY_STEP:
			respawn_delay_step = parse_duration(optarg);
			यदि (respawn_delay_step < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			रोक;

		पक्ष LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			यदि (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			रोक;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(दो_पर्या ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	यदि (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	यदि (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	यदि (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	यदि (ch_जरा && *ch_जरा == '~')
		ch_जरा = expand_home(home, ch_जरा);

	umask(numask);
	यदि (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? दो_पर्या("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	यदि (mkfifo(fifopath, 0600) == -1 && त्रुटि_सं != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, म_त्रुटि(त्रुटि_सं));

	यदि (reexec) {
		str = rc_service_मान_get(svcname, "argc");
		माला_पूछो(str, "%d", &child_argc);
		child_argv = xmalloc((child_argc + 1) * परिमाण(वर्ण *));
		स_रखो(child_argv, 0, (child_argc + 1) * परिमाण(वर्ण *));
		लागि (x = 0; x < child_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_मान_get(svcname, varbuf);
			child_argv[x] = str;
			मुक्त(varbuf);
			varbuf = रिक्त;
		}
		मुक्त(str);
		str = rc_service_मान_get(svcname, "child_pid");
		माला_पूछो(str, "%d", &child_pid);
		मुक्त(str);
		exec = rc_service_मान_get(svcname, "exec");
		pidfile = rc_service_मान_get(svcname, "pidfile");
		retry = rc_service_मान_get(svcname, "retry");
		यदि (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_मान_set(svcname, "retry", retry);
		} अन्यथा
			parse_schedule(applet, रिक्त, sig);

		str = rc_service_मान_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_मान_get(svcname, "respawn_delay_step");
		माला_पूछो(str, "%"SCNd64, &respawn_delay_step);
		str = rc_service_मान_get(svcname, "respawn_delay_cap");
		माला_पूछो(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_मान_get(svcname, "respawn_max");
		माला_पूछो(str, "%d", &respawn_max);
		supervisor(exec, child_argv);
	} अन्यथा यदि (start) {
		यदि (exec) {
			यदि (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			यदि (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				यदि (ch_जरा)
					xasprintf(&exec_file, "%s/%s", ch_जरा, exec);
				अन्यथा
					xasprintf(&exec_file, "%s", exec);
			} अन्यथा {
				/* Something in $PATH */
				p = tmp = xstrdup(दो_पर्या("PATH"));
				exec_file = रिक्त;
				जबसम्म ((token = strsep(&p, ":"))) {
					यदि (ch_जरा)
						xasprintf(&exec_file, "%s/%s/%s", ch_जरा, token, exec);
					अन्यथा
						xasprintf(&exec_file, "%s/%s", token, exec);
					यदि (exec_file && access(exec_file, F_OK) == 0)
						रोक;
					मुक्त(exec_file);
					exec_file = रिक्त;
				}
				मुक्त(tmp);
			}
			यदि (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				मुक्त(exec_file);
				निकास(निकास_त्रुटि);
			}
		} अन्यथा
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		यदि (pid != -1)
			यदि (do_stop(applet, exec, (अचर वर्ण * अचर *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		यदि (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		यदि (respawn_delay_step > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		यदि (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_मान_set(svcname, "retry", retry);
		} अन्यथा
			parse_schedule(applet, रिक्त, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		मुक्त(cmdline);
		cmdline = रिक्त;

		/* Remove existing pidfile */
		यदि (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = ख_खोलो(pidfile, "w");
		यदि (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, म_त्रुटि(त्रुटि_सं));
		ख_बंद(fp);

		rc_service_मान_set(svcname, "pidfile", pidfile);
		rc_service_मान_set(svcname, "respawn_delay", respawn_delay_str);
		rc_service_मान_set(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, परिमाण(numbuf), "%"PRId64, respawn_delay_step);
		rc_service_मान_set(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, परिमाण(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_मान_set(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, परिमाण(numbuf), "%i", respawn_max);
		rc_service_मान_set(svcname, "respawn_max", numbuf);
		child_pid = विभाजन();
		यदि (child_pid == -1)
			eerrorx("%s: fork: %s", applet, म_त्रुटि(त्रुटि_सं));
		यदि (child_pid != 0)
			निकास(notify_wait(applet, notify) ? निकास_सफल : निकास_त्रुटि);

#ifdef TIOCNOTTY
		tty_fd = खोलो("/dev/tty", O_RDWR);
#endif
		devnull_fd = खोलो("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		यदि (notify.type == NOTIFY_FD)
			बंद(notify.pipe[0]);

		child_pid = विभाजन();
		यदि (child_pid == -1)
			eerrorx("%s: fork: %s", applet, म_त्रुटि(त्रुटि_सं));
		अन्यथा यदि (child_pid != 0) {
			यदि (notify.type == NOTIFY_FD)
				बंद(notify.pipe[1]);
			c = argv;
			x = 0;
			जबसम्म (c && *c) {
				varbuf = रिक्त;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_मान_set(svcname, varbuf, *c);
				मुक्त(varbuf);
				varbuf = रिक्त;
				x++;
				c++;
			}
			snprintf(numbuf, परिमाण(numbuf), "%d", x);
			rc_service_मान_set(svcname, "argc", numbuf);
			rc_service_मान_set(svcname, "exec", exec);
			supervisor(exec, argv);
		} अन्यथा
			child_process(exec, argv);
	} अन्यथा यदि (stop) {
		pid = get_pid(applet, pidfile);
		यदि (pid != -1) {
			i = समाप्त(pid, संक_इति);
			यदि (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			अन्यथा {
				/* wait for the supervisor to go down */
				जबसम्म (समाप्त(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, रिक्त);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		यदि (pidfile)
			unlink(pidfile);
		यदि (svcname) {
			rc_service_daemon_set(svcname, exec,
			    (अचर वर्ण *अचर *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		निकास(निकास_सफल);
	} अन्यथा यदि (sendsig) {
		fifo_fd = खोलो(fifopath, O_WRONLY |O_NONBLOCK);
		यदि (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, म_त्रुटि(त्रुटि_सं));
		xasprintf(&str, "sig %d", sig);
		x = लिख(fifo_fd, str, म_माप(str));
		यदि (x == -1) {
			मुक्त(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					म_त्रुटि(त्रुटि_सं));
		}
		मुक्त(tmp);
		निकास(निकास_सफल);
	}
}
