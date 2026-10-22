#അന്തര്ഭാവ <einfo.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <grp.h>
#അന്തര്ഭാവ <pwd.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <syslog.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <spawn.h>

#ifdef HAVE_PAM
#അന്തര്ഭാവ <security/pam_appl.h>
#endif

#define USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#അന്തര്ഭാവ "helpers.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "rc_exec.h"

സ്ഥിതം bool valid_shell(സ്ഥിരം അക്ഷരം *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	അക്ഷരം *വരി = ശൂന്യ;
	size_t size;

	എങ്കിൽ (!fp)
		തിരികെ true;

	ആകുമ്പോൾ (xgetline(&വരി, &size, fp) != -1) {
		എങ്കിൽ (വരി[0] == '#')
			തുടരുക;
		എങ്കിൽ (strcmp(വരി, shell) == 0) {
			ret = true;
			നിർത്തുക;
		}
	}

	സ്വതന്ത്രം(വരി);
	തിരികെ ret;
}

സ്ഥിതം bool spawn_openrc(uid_t uid, gid_t gid, സ്ഥിരം അക്ഷരം *shell, bool start) {
	അക്ഷരം *argv0;
	സ്ഥിരം അക്ഷരം *argv[] = {
		ശൂന്യ /* replaced below */, "-c",
		start ? USER_SH "start" : USER_SH "stop", ശൂന്യ
	};
	ഘടന exec_result res;
	ഘടന exec_args args;

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
	സ്വതന്ത്രം(argv0);
	എങ്കിൽ (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		തിരികെ false;
	}
	തിരികെ rc_waitpid(res.pid) == 0;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) {
	സ്ഥിരം ഘടന passwd *user;
	അക്ഷരം *log, *shell;
	sigset_t sigmask;
	പൂർണ്ണസംഖ്യ sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ifdef HAVE_PAM
	ഘടന pam_conv conv = { ശൂന്യ, ശൂന്യ };
	pam_handle_t *pamh = ശൂന്യ;
	bool pam_session = false;
	പൂർണ്ണസംഖ്യ rc;
#endif

	setenv("EINFO_LOG", "openrc-user", true);

	എങ്കിൽ (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		തിരികെ -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	എങ്കിൽ (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		തിരികെ -1;
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
	സ്വതന്ത്രം(log);

#ifdef HAVE_PAM
	എങ്കിൽ ((rc = pam_start("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	അല്ലെങ്കിൽ എങ്കിൽ ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	അല്ലെങ്കിൽ
		pam_session = true;

	വേണ്ടി (അക്ഷരം **env = pam_getenvlist(pamh); env && *env; env++) {
		എങ്കിൽ (strchr(*env, '='))
			putenv(xstrdup(*env));
		അല്ലെങ്കിൽ
			unsetenv(*env);
	}

	എങ്കിൽ (!pam_session && pamh) {
		pam_end(pamh, rc);
		pamh = ശൂന്യ;
	}
#endif

	എങ്കിൽ (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		പോകുക out;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, ശൂന്യ);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	എങ്കിൽ (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

out:
#ifdef HAVE_PAM
	എങ്കിൽ (pamh) {
		എങ്കിൽ ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_end(pamh, rc);
	}
#endif

	സ്വതന്ത്രം(shell);
	തിരികെ ret;
}
