#अन्तर्भाव <einfo.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <grp.h>
#अन्तर्भाव <pwd.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <syslog.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <spawn.h>

#ifdef HAVE_PAM
#अन्तर्भाव <security/pam_appl.h>
#endif

#define USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#अन्तर्भाव "helpers.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "rc_exec.h"

स्थिर bool valid_shell(ध्रुव वर्ण *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	वर्ण *पङ्क्तिः = रिक्त;
	size_t size;

	जइ (!fp)
		प्रत्यागम true;

	जाव (xgetline(&पङ्क्तिः, &size, fp) != -1) {
		जइ (पङ्क्तिः[0] == '#')
			अनुवर्त;
		जइ (strcmp(पङ्क्तिः, shell) == 0) {
			ret = true;
			भङ्ग;
		}
	}

	मुक्त(पङ्क्तिः);
	प्रत्यागम ret;
}

स्थिर bool spawn_openrc(uid_t uid, gid_t gid, ध्रुव वर्ण *shell, bool start) {
	वर्ण *argv0;
	ध्रुव वर्ण *argv[] = {
		रिक्त /* replaced below */, "-c",
		start ? USER_SH "start" : USER_SH "stop", रिक्त
	};
	संरचना exec_result res;
	संरचना exec_args args;

	/* shell might be a multicall binary, e.g busybox.
	 * so setting argv[0] to "-" might not work */
	xasprintf(&argv0, "-%s", valid_shell(shell) ? shell : "/bin/sh");
	argv[0] = argv0;
	args = exec_init(argv);
	args.cmd = shell;
	args.uid = uid;
	args.gid = gid;
	args.redirect_stdout = 3;
	res = do_exec(&args);
	मुक्त(argv0);
	जइ (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		प्रत्यागम false;
	}
	प्रत्यागम rc_waitpid(res.pid) == 0;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv) {
	ध्रुव संरचना passwd *user;
	वर्ण *log, *shell;
	sigset_t sigmask;
	पूर्णाङ्क sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ifdef HAVE_PAM
	संरचना pam_conv conv = { रिक्त, रिक्त };
	pam_handle_t *pamh = रिक्त;
	bool pam_session = false;
	पूर्णाङ्क rc;
#endif

	setenv("EINFO_LOG", "openrc-user", true);

	जइ (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		प्रत्यागम -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	जइ (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		प्रत्यागम -1;
	}

	/* sssd's pam module wrongly calls getpwnam, meaning
	 * pam_open_session can clobber user. So we need to copy the
	 * parts we need, see https://bugs.gentoo.org/970235 */
	uid = user->pw_uid;
	gid = user->pw_gid;
	shell = xstrdup(user->pw_shell);

	setenv("USER", user->pw_name, true);
	setenv("LOGNAME", user->pw_name, true);
	setenv("HOME", user->pw_dir, true);
	setenv("SHELL", user->pw_shell, true);

	initgroups(user->pw_name, user->pw_gid);

	xasprintf(&log, "openrc-user.%s", user->pw_name);
	setenv("EINFO_LOG", log, true);
	मुक्त(log);

#ifdef HAVE_PAM
	जइ ((rc = pam_start("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	अण्णहा जइ ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	अण्णहा
		pam_session = true;

	प्रति (वर्ण **env = pam_getenvlist(pamh); env && *env; env++) {
		जइ (strchr(*env, '='))
			putenv(xstrdup(*env));
		अण्णहा
			unsetenv(*env);
	}

	जइ (!pam_session && pamh) {
		pam_end(pamh, rc);
		pamh = रिक्त;
	}
#endif

	जइ (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		गच्छ out;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, रिक्त);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	जइ (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

out:
#ifdef HAVE_PAM
	जइ (pamh) {
		जइ ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_end(pamh, rc);
	}
#endif

	मुक्त(shell);
	प्रत्यागम ret;
}
