#ਅਨ੍ਤਰ੍ਭਾਵ <einfo.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <grp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pwd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <syslog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spawn.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
#ਅਨ੍ਤਰ੍ਭਾਵ <security/pam_appl.h>
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc_exec.h"

ਸਥਿਰ bool valid_shell(ਅਚਲ ਅੱਖਰ *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	ਅੱਖਰ *ਸਤਰ = ਨਿਰਾ;
	size_t ਮਾਪ;

	ਜੇ (!fp)
		ਵਾਪਸ true;

	ਜਦੋਂ (xgetline(&ਸਤਰ, &ਮਾਪ, fp) != -1) {
		ਜੇ (ਸਤਰ[0] == '#')
			ਜਾਰੀ;
		ਜੇ (strcmp(ਸਤਰ, shell) == 0) {
			ret = true;
			ਰੋਕੋ;
		}
	}

	ਮੁਕਤ(ਸਤਰ);
	ਵਾਪਸ ret;
}

ਸਥਿਰ bool spawn_openrc(uid_t uid, gid_t gid, ਅਚਲ ਅੱਖਰ *shell, bool ਸ਼ੁਰੂ) {
	ਅੱਖਰ *argv0;
	ਅਚਲ ਅੱਖਰ *argv[] = {
		ਨਿਰਾ /* replaced below */, "-c",
		ਸ਼ੁਰੂ ? USER_SH "start" : USER_SH "stop", ਨਿਰਾ
	};
	ਬਣਤਰ exec_ਨਤੀਜਾ res;
	ਬਣਤਰ exec_ਦਲੀਲਾਂ ਦਲੀਲਾਂ;

	/* shell might be a multicall binary, e.g busybox.
	 * so setting argv[0] to "-" might not work */
	xasprintf(&argv0, "-%s", valid_shell(shell) ? shell : "/bin/sh");
	argv[0] = argv0;
	ਦਲੀਲਾਂ = exec_init(argv);
	ਦਲੀਲਾਂ.cmd = shell;
	ਦਲੀਲਾਂ.uid = uid;
	ਦਲੀਲਾਂ.gid = gid;
	ਦਲੀਲਾਂ.redirect_stdout = 3;
	res = do_exec(&ਦਲੀਲਾਂ);
	ਮੁਕਤ(argv0);
	ਜੇ (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		ਵਾਪਸ false;
	}
	ਵਾਪਸ rc_waitpid(res.pid) == 0;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv) {
	ਅਚਲ ਬਣਤਰ passwd *user;
	ਅੱਖਰ *ਲਘੂਗਣਕ, *shell;
	sigset_t sigmask;
	ਪੂਰਨ sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
	ਬਣਤਰ pam_conv conv = { ਨਿਰਾ, ਨਿਰਾ };
	pam_handle_t *pamh = ਨਿਰਾ;
	bool pam_session = false;
	ਪੂਰਨ rc;
#ਜੇ_ਅੰਤ

	setenv("EINFO_LOG", "openrc-user", true);

	ਜੇ (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		ਵਾਪਸ -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	ਜੇ (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		ਵਾਪਸ -1;
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

	xasprintf(&ਲਘੂਗਣਕ, "openrc-user.%s", user->pw_name);
	setenv("EINFO_LOG", ਲਘੂਗਣਕ, true);
	ਮੁਕਤ(ਲਘੂਗਣਕ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
	ਜੇ ((rc = pam_ਸ਼ੁਰੂ("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	ਨਹੀਂਤਾਂ ਜੇ ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	ਨਹੀਂਤਾਂ
		pam_session = true;

	ਲਈ (ਅੱਖਰ **env = pam_getenvlist(pamh); env && *env; env++) {
		ਜੇ (strchr(*env, '='))
			putenv(xstrdup(*env));
		ਨਹੀਂਤਾਂ
			unsetenv(*env);
	}

	ਜੇ (!pam_session && pamh) {
		pam_ਸਿਰਾ(pamh, rc);
		pamh = ਨਿਰਾ;
	}
#ਜੇ_ਅੰਤ

	ਜੇ (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, ਨਿਰਾ);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	ਜੇ (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

ਬਾਹਰਲਾ:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
	ਜੇ (pamh) {
		ਜੇ ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_ਸਿਰਾ(pamh, rc);
	}
#ਜੇ_ਅੰਤ

	ਮੁਕਤ(shell);
	ਵਾਪਸ ret;
}
