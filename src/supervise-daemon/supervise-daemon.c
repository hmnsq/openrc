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

#समावेश <errno.h>
#समावेश <inttypes.h>
#समावेश <fcntl.h>
#समावेश <getopt.h>
#समावेश <limits.h>
#समावेश <grp.h>
#समावेश <pwd.h>
#समावेश <signal.h>
#समावेश <stddef.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <stdbool.h>
#समावेश <strings.h>
#अगर_घोषित __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#पूर्ण_अगर
#समावेश <syslog.h>
#समावेश <sys/ioctl.h>
#समावेश <sys/resource.h>
#समावेश <sys/stat.h>
#समावेश <sys/wait.h>
#समावेश <time.h>
#समावेश <unistd.h>

#अगर_घोषित HAVE_PAM
#समावेश <security/pam_appl.h>

/* We are not supporting authentication conversations */
अटल काष्ठा pam_conv conv = { शून्य, शून्य};
#पूर्ण_अगर

#अगर_घोषित __linux__
#समावेश <sys/capability.h>
#पूर्ण_अगर

#समावेश "einfo.h"
#समावेश "queue.h"
#समावेश "rc.h"
#समावेश "misc.h"
#समावेश "rc_exec.h"
#समावेश "plugin.h"
#समावेश "schedules.h"
#समावेश "timeutils.h"
#समावेश "_usage.h"
#समावेश "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
क्रमागत {
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

स्थिर अक्षर *applet = शून्य;
स्थिर अक्षर *extraopts = शून्य;
स्थिर अक्षर getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "healthcheck-timer",        1, शून्य, 'a'},
	{ "healthcheck-delay",        1, शून्य, 'A'},
	{ "capabilities", 1, शून्य, LONGOPT_CAPABILITIES},
	{ "secbits",      1, शून्य, LONGOPT_SECBITS},
	{ "no-new-privs", 0, शून्य, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, शून्य, 'D'},
	{ "respawn-delay-step",   1, शून्य, LONGOPT_RESPAWN_DELAY_STEP},
	{ "respawn-delay-cap",    1, शून्य, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, शून्य, 'd'},
	{ "env",          1, शून्य, 'e'},
	{ "group",        1, शून्य, 'g'},
	{ "stop-group",   0, शून्य, 'G'},
	{ "ionice",       1, शून्य, 'I'},
	{ "stop",         0, शून्य, 'K'},
	{ "umask",        1, शून्य, 'k'},
	{ "respawn-max",    1, शून्य, 'm'},
	{ "nicelevel",    1, शून्य, 'N'},
	{ "oom-score-adj",1, शून्य, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, शून्य, 'p'},
	{ "respawn-period",        1, शून्य, 'P'},
	{ "retry",       1, शून्य, 'R'},
	{ "chroot",       1, शून्य, 'r'},
	{ "signal",       1, शून्य, 's'},
	{ "start",        0, शून्य, 'S'},
	{ "user",         1, शून्य, 'u'},
	{ "stdin",        1, शून्य, '0'},
	{ "stdout",       1, शून्य, '1'},
	{ "stderr",       1, शून्य, '2'},
	{ "stdout-logger",1, शून्य, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, शून्य, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, शून्य, '3'},
	{ "notify",       1, शून्य, LONGOPT_NOTIFY},
	longopts_COMMON
};
स्थिर अक्षर * स्थिर longopts_help[] = {
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
स्थिर अक्षर *usagestring = शून्य;

अटल पूर्णांक healthcheckdelay = 0;
अटल पूर्णांक healthchecktimer = 0;
अटल अस्थिर संक_पूर्ण_प्रकार do_healthcheck = 0;
अटल अस्थिर संक_पूर्ण_प्रकार exiting = 0;
अटल पूर्णांक nicelevel = पूर्णांक_न्यून;
अटल पूर्णांक ionicec = -1;
अटल पूर्णांक ioniced = 0;
अटल पूर्णांक oom_score_adj = पूर्णांक_न्यून;
अटल अक्षर *changeuser, *ch_मूल, *ch_dir;
अटल uid_t uid = 0;
अटल gid_t gid = 0;
अटल bool stopgroup = false;
अटल पूर्णांक devnull_fd = -1;
अटल पूर्णांक stdin_fd;
अटल पूर्णांक stdout_fd;
अटल पूर्णांक stderr_fd;
अटल काष्ठा notify notify;
अटल अक्षर *redirect_stdin = शून्य;
अटल अक्षर *redirect_stderr = शून्य;
अटल अक्षर *redirect_stdout = शून्य;
अटल अक्षर *stderr_process = शून्य;
अटल अक्षर *stdout_process = शून्य;
#अगर_घोषित TIOCNOTTY
अटल पूर्णांक tty_fd = -1;
#पूर्ण_अगर
अटल pid_t child_pid;
अटल पूर्णांक respawn_गिनती = 0;
अटल int64_t respawn_delay; /* default set inside main() */
अटल int64_t respawn_delay_step = TM_MS(128);
अटल int64_t respawn_delay_cap = TM_SEC(30);
अटल int64_t respawn_period; /* default set inside main() */
अटल पूर्णांक respawn_max = 10;
अटल अक्षर *fifopath = शून्य;
अटल पूर्णांक fifo_fd = 0;
अटल अक्षर *pidfile = शून्य;
अटल अक्षर *svcname = शून्य;
अटल bool verbose = false;
#अगर_घोषित __linux__
अटल cap_iab_t cap_iab = शून्य;
अटल अचिन्हित secbits = 0;
#पूर्ण_अगर
#अगर_घोषित PR_SET_NO_NEW_PRIVS
अटल bool no_new_privs = false;
#पूर्ण_अगर

बाह्य अक्षर **environ;

#अगर !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#पूर्ण_अगर
#अगर !defined(__DragonFly__)
अटल अंतरभूत पूर्णांक ioprio_set(पूर्णांक which RC_UNUSED, पूर्णांक who RC_UNUSED,
			     पूर्णांक ioprio RC_UNUSED)
{
#अगर_घोषित SYS_ioprio_set
	वापस syscall(SYS_ioprio_set, which, who, ioprio);
#अन्यथा
	वापस 0;
#पूर्ण_अगर
}
#पूर्ण_अगर

अटल व्योम cleanup(व्योम)
{
	मुक्त(changeuser);
}

RC_NORETURN अटल व्योम re_exec_supervisor(व्योम)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(अक्षर *) शून्य);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			म_त्रुटि(त्रुटि_सं));
	निकास(निकास_त्रुटि);
}

अटल व्योम handle_signal(पूर्णांक sig)
{
	पूर्णांक serrno = त्रुटि_सं;
	pid_t pid;

	चयन (sig) {
	हाल SIGALRM:
		do_healthcheck = 1;
		अवरोध;
	हाल SIGCHLD:
		अगर (exiting)
			जबतक (waitpid((pid_t)(-1), शून्य, WNOHANG) > 0) {}
		अन्यथा {
			जबतक ((pid = waitpid((pid_t)(-1), शून्य, WNOHANG|WNOWAIT)) > 0) {
				अगर (pid == child_pid)
					अवरोध;
				pid = waitpid(pid, शून्य, WNOHANG);
			}
		}
		अवरोध;
	हाल संक_इति:
		exiting = 1;
		अवरोध;
	शेष:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	त्रुटि_सं = serrno;
}

अटल अक्षर * expand_home(स्थिर अक्षर *home, स्थिर अक्षर *path)
{
	अक्षर *opath, *ppath, *p, *nh;
	काष्ठा passwd *pw;

	अगर (!path || *path != '~')
		वापस xstrdup(path);

	opath = ppath = xstrdup(path);
	अगर (ppath[1] != '/' && ppath[1] != '\0') {
		p = म_अक्षर(ppath + 1, '/');
		अगर (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		अगर (pw) {
			home = pw->pw_dir;
			ppath = p;
			अगर (ppath)
				*ppath = '/';
		} अन्यथा
			home = शून्य;
	} अन्यथा
		ppath++;

	अगर (!home) {
		मुक्त(opath);
		वापस xstrdup(path);
	}
	अगर (!ppath) {
		मुक्त(opath);
		वापस xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	मुक्त(opath);
	वापस nh;
}

अटल अक्षर *make_cmdline(अक्षर **argv)
{
	अक्षर **c;
	अक्षर *cmdline;
	माप_प्रकार len;
	खाता *mem = xopen_memstream(&cmdline, &len);

	क्रम (c = argv; c && *c; c++) {
		ख_माला_दो(*c, mem);
		अगर (c[1])
			ख_अक्षर_दो(' ', mem);
	}
	xclose_memstream(mem);
	वापस cmdline;
}

अटल pid_t exec_command(स्थिर अक्षर *cmd)
{
	अक्षर *file;
	pid_t pid = -1;
	sigset_t full;
	sigset_t old;
	काष्ठा sigaction sa;

	file = rc_service_resolve(svcname);
	अगर (access(file, F_OK) != 0) {
		मुक्त(file);
		वापस 0;
	}

	/* We need to block signals until we have forked */
	स_रखो(&sa, 0, माप (sa));
	sa.sa_handler = संक_पूर्व;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);

	pid = विभाजन();
	अगर (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, शून्य);
		sigaction(SIGHUP, &sa, शून्य);
		sigaction(संक_विघ्न, &sa, शून्य);
		sigaction(SIGQUIT, &sa, शून्य);
		sigaction(संक_इति, &sa, शून्य);
		sigaction(SIGUSR1, &sa, शून्य);
		sigaction(SIGWINCH, &sa, शून्य);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, शून्य);

		/* Safe to run now */
		execl(file, file, cmd, (अक्षर *) शून्य);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, म_त्रुटि(त्रुटि_सं));
		_निकास(निकास_त्रुटि);
	}

	अगर (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",म_त्रुटि (त्रुटि_सं));

	sigprocmask(SIG_SETMASK, &old, शून्य);
	मुक्त(file);
	वापस pid;
}

RC_NORETURN अटल व्योम child_process(अक्षर *exec, अक्षर **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	पूर्णांक i;
	अक्षर *p;
	अक्षर *token;
	माप_प्रकार len;
	अक्षर *newpath;
	अक्षर *np;
	अक्षर *cmdline = शून्य;
	समय_प्रकार start_time;
	अक्षर start_गिनती_string[20];
	अक्षर start_time_string[20];
	खाता *fp;
	gid_t group_buf[32], *group_list = group_buf;
	पूर्णांक group_गिनती = ARRAY_आकार(group_buf);

#अगर_घोषित HAVE_PAM
	pam_handle_t *pamh = शून्य;
	पूर्णांक pamr;
	स्थिर अक्षर *स्थिर *pamenv = शून्य;
#पूर्ण_अगर

	setsid();

	अगर (svcname) {
		start_time = समय(शून्य);
		from_time_t(start_time_string, start_time);
		rc_service_मूल्य_set(svcname, "start_time", start_time_string);
		प्र_लिखो(start_गिनती_string, "%i", respawn_गिनती);
		rc_service_मूल्य_set(svcname, "start_count", start_गिनती_string);
		प्र_लिखो(start_गिनती_string, "%d", getpid());
		rc_service_मूल्य_set(svcname, "child_pid", start_गिनती_string);
	}

	अगर (nicelevel != पूर्णांक_न्यून) {
		अगर (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					म_त्रुटि(त्रुटि_सं));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = ख_खोलो("/proc/self/autogroup", "r+");
		अगर (fp) {
			ख_लिखो(fp, "%d\n", nicelevel);
			ख_बंद(fp);
		} अन्यथा अगर (त्रुटि_सं != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, म_त्रुटि(त्रुटि_सं));
	}

	अगर (ionicec != -1 && ioprio_set(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				म_त्रुटि(त्रुटि_सं));

	अगर (oom_score_adj != पूर्णांक_न्यून) {
		fp = ख_खोलो("/proc/self/oom_score_adj", "w");
		अगर (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, म_त्रुटि(त्रुटि_सं));
		ख_लिखो(fp, "%d\n", oom_score_adj);
		ख_बंद(fp);
	}

#अगर_घोषित HAVE_PAM
	अगर (changeuser != शून्य) {
		pamr = pam_start("supervise-daemon",
		    changeuser, &conv, &pamh);

		अगर (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		अगर (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		अगर (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#पूर्ण_अगर

	अगर (changeuser && getgrouplist(changeuser, gid, group_list, &group_गिनती) < 0) {
		group_list = xmalloc(group_गिनती * माप(*group_list));
		अगर (getgrouplist(changeuser, gid, group_list, &group_गिनती) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	अगर (ch_मूल && chroot(ch_मूल) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_मूल, म_त्रुटि(त्रुटि_सं));

	अगर (ch_dir && स_बदलो(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, म_त्रुटि(त्रुटि_सं));

	अगर (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	अगर (changeuser && setgroups(group_गिनती, group_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	अगर (group_list != group_buf)
		मुक्त(group_list);
#अगर_घोषित __linux__
	अगर (uid && cap_setuid(uid))
#अन्यथा
	अगर (uid && setuid(uid))
#पूर्ण_अगर
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#अगर_घोषित __linux__
	अगर (cap_iab != शून्य) {
		i = cap_iab_set_proc(cap_iab);

		अगर (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", म_त्रुटि(त्रुटि_सं));

		अगर (i != 0)
			eerrorx("Could not set iab: %s", म_त्रुटि(त्रुटि_सं));
	}

	अगर (secbits != 0) {
		अगर (cap_set_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, म_त्रुटि(त्रुटि_सं));
	}
#पूर्ण_अगर

#अगर_घोषित PR_SET_NO_NEW_PRIVS
	अगर (no_new_privs) {
		अगर (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
			eerrorx("Could not set No New Privs flag: %s", म_त्रुटि(त्रुटि_सं));
	}
#पूर्ण_अगर

	/* remove the controlling tty */
#अगर_घोषित TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	बंद(tty_fd);
#पूर्ण_अगर

	/* Clean the environment of any RC_ variables */
	env_list = rc_stringlist_new();
	i = 0;
	जबतक (environ[i])
		rc_stringlist_add(env_list, environ[i++]);

#अगर_घोषित HAVE_PAM
	अगर (changeuser != शून्य) {
		pamenv = (स्थिर अक्षर *स्थिर *)pam_getenvlist(pamh);
		अगर (pamenv) {
			जबतक (*pamenv) {
				/* Don't add strings unless they set a var */
				अगर (म_अक्षर(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				अन्यथा
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#पूर्ण_अगर

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		अगर (म_भेदन(env->मूल्य, "RC_", 3) == 0
				|| म_भेदन(env->मूल्य, "SSD_", 4) == 0
				|| म_भेदन(env->मूल्य, "EINFO_", 6) == 0) {
			*म_अक्षर(env->मूल्य, '=') = '\0';
			unsetenv(env->मूल्य);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	अगर ((token = दो_पर्या("PATH"))) {
		len = म_माप(token);
		newpath = np = xmalloc(len + 1);
		जबतक (token && *token) {
			p = म_अक्षर(token, ':');
			अगर (p) {
				*p++ = '\0';
				जबतक (*p == ':')
					p++;
			}
			अगर (म_भेद(token, RC_LIBEXECDIR "/bin") != 0 &&
			    म_भेद(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = म_माप(token);
				अगर (np != newpath)
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
	अगर (redirect_stdin) {
		अगर ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, म_त्रुटि(त्रुटि_सं));
	}
	अगर (redirect_stdout) {
		अगर ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, म_त्रुटि(त्रुटि_सं));
	} अन्यथा अगर (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		अगर (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, म_त्रुटि(त्रुटि_सं));
	}
	अगर (redirect_stderr) {
		अगर ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, म_त्रुटि(त्रुटि_सं));
	} अन्यथा अगर (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		अगर (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, म_त्रुटि(त्रुटि_सं));
	}

	dup2(stdin_fd, STDIN_FILENO);
	अगर (redirect_stdout || stdout_process || rc_yesno(दो_पर्या("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	अगर (redirect_stderr || stderr_process || rc_yesno(दो_पर्या("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	अगर (notify.type == NOTIFY_FD) {
		पूर्णांक error;

		अगर (notify.fd == notify.pipe[1]) {
			पूर्णांक flags = error = fcntl(notify.fd, F_GETFD, 0);
			अगर (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} अन्यथा {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		अगर (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, म_त्रुटि(त्रुटि_सं));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	मुक्त(cmdline);
	execvp(exec, argv);

#अगर_घोषित HAVE_PAM
	अगर (changeuser != शून्य && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#पूर्ण_अगर
	eerrorx("%s: failed to exec `%s': %s", applet, exec,म_त्रुटि(त्रुटि_सं));
}

RC_NORETURN अटल व्योम supervisor(अक्षर *exec, अक्षर **argv)
{
	खाता *fp;
	अक्षर buf[2048];
	अक्षर cmd[2048];
	पूर्णांक गिनती;
	पूर्णांक failing;
	पूर्णांक health_status;
	पूर्णांक healthcheck_respawn;
	पूर्णांक i;
	पूर्णांक nkilled;
	पूर्णांक sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_signals;
	sigset_t signals;
	काष्ठा sigaction sa;
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
	स_रखो(&sa, 0, माप(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, शून्य);
	sigaction(SIGCHLD, &sa, शून्य);
	sigaction(संक_इति, &sa, शून्य);

	fp = ख_खोलो(pidfile, "w");
	अगर (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, म_त्रुटि(त्रुटि_सं));
	ख_लिखो(fp, "%d\n", getpid());
	ख_बंद(fp);

	अगर (svcname)
		rc_service_daemon_set(svcname, exec, (स्थिर अक्षर * स्थिर *) argv,
				pidfile, true);

	/* remove the controlling tty */
#अगर_घोषित TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	बंद(tty_fd);
#पूर्ण_अगर

	/*
	 * Supervisor main loop
	 */
	अगर (healthcheckdelay)
		alarm(healthcheckdelay);
	अन्यथा अगर (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	जबतक (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = खोलो(fifopath, O_RDONLY);
		अगर (fifo_fd > 0) {
			स_रखो(buf, 0, माप(buf));
			गिनती = पढ़ो(fifo_fd, buf, माप(buf) - 1);
			बंद(fifo_fd);
			अगर (गिनती != -1)
				buf[गिनती] = 0;
			अगर (गिनती == 0)
				जारी;
			अगर (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			अगर (strncasecmp(buf, "sig", 3) == 0) {
				अगर ((माला_पूछो(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							child_pid);
					अगर (समाप्त(child_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, child_pid);
				}
			}
			जारी;
		}
		अगर (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			अगर (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_status = rc_waitpid(health_pid);
			अगर (WIFEXITED(health_status) && WEXITSTATUS(health_status) == 0)
				alarm(healthchecktimer);
			अन्यथा {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
				nkilled = run_stop_schedule(applet, शून्य, शून्य, child_pid, 0,
						stopgroup, false, false, true);
				अगर (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							child_pid, म_त्रुटि(त्रुटि_सं));
				अन्यथा
					healthcheck_respawn = 1;
			}
		}
		अगर (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
			nkilled = run_stop_schedule(applet, शून्य, शून्य, child_pid, 0,
					stopgroup, false, false, true);
			अगर (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			जारी;
		}
		wait_pid = waitpid(child_pid, &i, WNOHANG);
		अगर (wait_pid == child_pid) {
			अगर (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, child_pid, WEXITSTATUS(i));
			अन्यथा अगर (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, child_pid, WTERMSIG(i));
		}
		अगर (wait_pid == child_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			अगर (first_spawn == 0)
				first_spawn = respawn_now;
			अगर ((respawn_period > 0)
					&& (respawn_now - first_spawn > respawn_period)) {
				respawn_गिनती = 0;
				first_spawn = 0;
			} अन्यथा
				respawn_गिनती++;
			अगर (respawn_max > 0 && respawn_गिनती > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				जारी;
			}
			sleep_for = respawn_delay + (respawn_delay_step * respawn_गिनती);
			अगर (respawn_delay_step > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			अगर (exiting)
				जारी;
			child_pid = विभाजन();
			अगर (child_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, म_त्रुटि(त्रुटि_सं));
				निकास(निकास_त्रुटि);
			}
			अगर (child_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_signals, शून्य);
				स_रखो(&sa, 0, माप(sa));
				sa.sa_handler = संक_पूर्व;
				sigaction(SIGALRM, &sa, शून्य);
				sigaction(SIGCHLD, &sa, शून्य);
				sigaction(संक_इति, &sa, शून्य);
				child_process(exec, argv);
			}
			अगर (healthcheckdelay)
				alarm(healthcheckdelay);
			अन्यथा अगर (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	अगर (svcname) {
		rc_service_daemon_set(svcname, exec, (स्थिर अक्षर *स्थिर *)argv,
				pidfile, false);
		rc_service_मूल्य_set(svcname, "child_pid", शून्य);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		अगर (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	अगर (pidfile)
		unlink(pidfile);
	अगर (fifopath)
		unlink(fifopath);
	निकास(निकास_सफल);
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक opt;
	अक्षर **c;
	पूर्णांक x;
	bool start = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	अक्षर *exec = शून्य;
	अक्षर *retry = शून्य;
	पूर्णांक sig = संक_इति;
	अक्षर *home = शून्य;
	पूर्णांक tid = 0;
	pid_t pid;
	अक्षर *tmp;
	अक्षर *p;
	अक्षर *token;
	पूर्णांक i;
	पूर्णांक n;
	अक्षर *exec_file = शून्य;
	अक्षर *varbuf = शून्य;
	अक्षर numbuf[64];
	काष्ठा timespec ts;
	काष्ठा passwd *pw;
	काष्ठा group *gr;
	खाता *fp;
	mode_t numask = 022;
	पूर्णांक child_argc = 0;
	अक्षर **child_argv = शून्य;
	अक्षर *str = शून्य;
	अक्षर *cmdline = शून्य;
	स्थिर अक्षर *respawn_delay_str  = "0";
	स्थिर अक्षर *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	निकास_पर(cleanup);
	svcname = दो_पर्या("RC_SVCNAME");
	अगर (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	अगर (argc <= 1 || म_भेद(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	अगर ((tmp = दो_पर्या("SSD_NICELEVEL")))
		अगर (माला_पूछो(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	अगर ((tmp = दो_पर्या("SSD_IONICELEVEL"))) {
		n = माला_पूछो(tmp, "%d:%d", &ionicec, &ioniced);
		अगर (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		अगर (ionicec == 0)
			ioniced = 0;
		अन्यथा अगर (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	अगर ((tmp = दो_पर्या("SSD_OOM_SCORE_ADJ")))
		अगर (माला_पूछो(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = दो_पर्या("USER");
	home = दो_पर्या("HOME");
	अगर (home == शून्य || p == शून्य) {
		pw = getpwuid(getuid());
		अगर (pw != शून्य) {
			अगर (p == शून्य)
				setenv("USER", pw->pw_name, 1);
			अगर (home == शून्य) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	अगर (svcname) {
		argc--;
		argv++;
	}
	जबतक ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (पूर्णांक *) 0)) != -1)
		चयन (opt) {
		हाल 'a':  /* --healthcheck-timer <time> */
			अगर (माला_पूछो(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			अवरोध;

		हाल 'A':  /* --healthcheck-delay <time> */
			अगर (माला_पूछो(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			अवरोध;

		हाल LONGOPT_CAPABILITIES:
#अगर_घोषित __linux__
			cap_iab = cap_iab_from_text(optarg);
			अगर (cap_iab == शून्य)
				eerrorx("Could not parse iab: %s", म_त्रुटि(त्रुटि_सं));
#अन्यथा
			eerrorx("Capabilities support not enabled");
#पूर्ण_अगर
			अवरोध;

		हाल LONGOPT_SECBITS:
#अगर_घोषित __linux__
			अगर (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = शून्य;
			secbits = म_से_अदीर्घ(optarg, &tmp, 0);
			अगर (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#अन्यथा
			eerrorx("Capabilities support not enabled");
#पूर्ण_अगर
			अवरोध;

		हाल LONGOPT_NO_NEW_PRIVS:
#अगर_घोषित PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#अन्यथा
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#पूर्ण_अगर
			अवरोध;

		हाल 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			अवरोध;

		हाल 'I': /* --ionice */
			अगर (माला_पूछो(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			अगर (ionicec == 0)
				ioniced = 0;
			अन्यथा अगर (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			अवरोध;

		हाल 'K':  /* --stop */
			stop = true;
			अवरोध;

		हाल 'N':  /* --nice */
			अगर (माला_पूछो(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			अवरोध;

		हाल LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			अगर (माला_पूछो(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			अवरोध;

		हाल 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			अवरोध;

		हाल 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			अवरोध;
		हाल 'S':  /* --start */
			start = true;
			अवरोध;

		हाल 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			अवरोध;

		हाल 'e': /* --env */
			putenv(optarg);
			अवरोध;

		हाल 'g':  /* --group <group>|<gid> */
			अगर (माला_पूछो(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			अन्यथा
				gr = getgrgid((gid_t)tid);
			अगर (gr == शून्य)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			अवरोध;

		हाल 'G': /* -G */
			stopgroup = true;
			अवरोध;

		हाल 'k':
			अगर (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			अवरोध;

		हाल 'm':  /* --respawn-max count */
			n = माला_पूछो(optarg, "%d", &respawn_max);
			अगर (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			अवरोध;

		हाल 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			अवरोध;

		हाल 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			अवरोध;
		हाल 'r':  /* --chroot /new/root */
			ch_मूल = optarg;
			अवरोध;

		हाल 'u':  /* --user <username>|<uid> */
		{
			अक्षर dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			अगर (माला_पूछो(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			अन्यथा
				pw = getpwuid((uid_t)tid);

			अगर (pw == शून्य)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			अगर (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			अगर (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			अगर (gid == 0)
				gid = pw->pw_gid;

			अगर (p) {
				tmp = strsep (&p, ":");
				अगर (माला_पूछो(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				अन्यथा
					gr = getgrgid((gid_t) tid);

				अगर (gr == शून्य)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		अवरोध;

		हाल '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			अवरोध;

		हाल '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			अवरोध;

		हाल '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			अवरोध;
		हाल '3':  /* --reexec */
			reexec = true;
			अवरोध;

		हाल LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			अवरोध;

		हाल LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			अवरोध;

		हाल LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			अवरोध;

		हाल LONGOPT_RESPAWN_DELAY_STEP:
			respawn_delay_step = parse_duration(optarg);
			अगर (respawn_delay_step < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			अवरोध;

		हाल LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			अगर (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			अवरोध;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(दो_पर्या ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	अगर (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	अगर (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	अगर (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	अगर (ch_मूल && *ch_मूल == '~')
		ch_मूल = expand_home(home, ch_मूल);

	umask(numask);
	अगर (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? दो_पर्या("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	अगर (mkfifo(fifopath, 0600) == -1 && त्रुटि_सं != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, म_त्रुटि(त्रुटि_सं));

	अगर (reexec) {
		str = rc_service_मूल्य_get(svcname, "argc");
		माला_पूछो(str, "%d", &child_argc);
		child_argv = xmalloc((child_argc + 1) * माप(अक्षर *));
		स_रखो(child_argv, 0, (child_argc + 1) * माप(अक्षर *));
		क्रम (x = 0; x < child_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_मूल्य_get(svcname, varbuf);
			child_argv[x] = str;
			मुक्त(varbuf);
			varbuf = शून्य;
		}
		मुक्त(str);
		str = rc_service_मूल्य_get(svcname, "child_pid");
		माला_पूछो(str, "%d", &child_pid);
		मुक्त(str);
		exec = rc_service_मूल्य_get(svcname, "exec");
		pidfile = rc_service_मूल्य_get(svcname, "pidfile");
		retry = rc_service_मूल्य_get(svcname, "retry");
		अगर (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_मूल्य_set(svcname, "retry", retry);
		} अन्यथा
			parse_schedule(applet, शून्य, sig);

		str = rc_service_मूल्य_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_मूल्य_get(svcname, "respawn_delay_step");
		माला_पूछो(str, "%"SCNd64, &respawn_delay_step);
		str = rc_service_मूल्य_get(svcname, "respawn_delay_cap");
		माला_पूछो(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_मूल्य_get(svcname, "respawn_max");
		माला_पूछो(str, "%d", &respawn_max);
		supervisor(exec, child_argv);
	} अन्यथा अगर (start) {
		अगर (exec) {
			अगर (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			अगर (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				अगर (ch_मूल)
					xasprintf(&exec_file, "%s/%s", ch_मूल, exec);
				अन्यथा
					xasprintf(&exec_file, "%s", exec);
			} अन्यथा {
				/* Something in $PATH */
				p = tmp = xstrdup(दो_पर्या("PATH"));
				exec_file = शून्य;
				जबतक ((token = strsep(&p, ":"))) {
					अगर (ch_मूल)
						xasprintf(&exec_file, "%s/%s/%s", ch_मूल, token, exec);
					अन्यथा
						xasprintf(&exec_file, "%s/%s", token, exec);
					अगर (exec_file && access(exec_file, F_OK) == 0)
						अवरोध;
					मुक्त(exec_file);
					exec_file = शून्य;
				}
				मुक्त(tmp);
			}
			अगर (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				मुक्त(exec_file);
				निकास(निकास_त्रुटि);
			}
		} अन्यथा
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		अगर (pid != -1)
			अगर (do_stop(applet, exec, (स्थिर अक्षर * स्थिर *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		अगर (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		अगर (respawn_delay_step > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		अगर (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_मूल्य_set(svcname, "retry", retry);
		} अन्यथा
			parse_schedule(applet, शून्य, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		मुक्त(cmdline);
		cmdline = शून्य;

		/* Remove existing pidfile */
		अगर (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = ख_खोलो(pidfile, "w");
		अगर (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, म_त्रुटि(त्रुटि_सं));
		ख_बंद(fp);

		rc_service_मूल्य_set(svcname, "pidfile", pidfile);
		rc_service_मूल्य_set(svcname, "respawn_delay", respawn_delay_str);
		rc_service_मूल्य_set(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, माप(numbuf), "%"PRId64, respawn_delay_step);
		rc_service_मूल्य_set(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, माप(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_मूल्य_set(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, माप(numbuf), "%i", respawn_max);
		rc_service_मूल्य_set(svcname, "respawn_max", numbuf);
		child_pid = विभाजन();
		अगर (child_pid == -1)
			eerrorx("%s: fork: %s", applet, म_त्रुटि(त्रुटि_सं));
		अगर (child_pid != 0)
			निकास(notify_wait(applet, notify) ? निकास_सफल : निकास_त्रुटि);

#अगर_घोषित TIOCNOTTY
		tty_fd = खोलो("/dev/tty", O_RDWR);
#पूर्ण_अगर
		devnull_fd = खोलो("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		अगर (notify.type == NOTIFY_FD)
			बंद(notify.pipe[0]);

		child_pid = विभाजन();
		अगर (child_pid == -1)
			eerrorx("%s: fork: %s", applet, म_त्रुटि(त्रुटि_सं));
		अन्यथा अगर (child_pid != 0) {
			अगर (notify.type == NOTIFY_FD)
				बंद(notify.pipe[1]);
			c = argv;
			x = 0;
			जबतक (c && *c) {
				varbuf = शून्य;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_मूल्य_set(svcname, varbuf, *c);
				मुक्त(varbuf);
				varbuf = शून्य;
				x++;
				c++;
			}
			snprintf(numbuf, माप(numbuf), "%d", x);
			rc_service_मूल्य_set(svcname, "argc", numbuf);
			rc_service_मूल्य_set(svcname, "exec", exec);
			supervisor(exec, argv);
		} अन्यथा
			child_process(exec, argv);
	} अन्यथा अगर (stop) {
		pid = get_pid(applet, pidfile);
		अगर (pid != -1) {
			i = समाप्त(pid, संक_इति);
			अगर (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			अन्यथा {
				/* wait for the supervisor to go down */
				जबतक (समाप्त(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, शून्य);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		अगर (pidfile)
			unlink(pidfile);
		अगर (svcname) {
			rc_service_daemon_set(svcname, exec,
			    (स्थिर अक्षर *स्थिर *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		निकास(निकास_सफल);
	} अन्यथा अगर (sendsig) {
		fifo_fd = खोलो(fifopath, O_WRONLY |O_NONBLOCK);
		अगर (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, म_त्रुटि(त्रुटि_सं));
		xasprintf(&str, "sig %d", sig);
		x = ग_लिखो(fifo_fd, str, म_माप(str));
		अगर (x == -1) {
			मुक्त(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					म_त्रुटि(त्रुटि_सं));
		}
		मुक्त(tmp);
		निकास(निकास_सफल);
	}
}
