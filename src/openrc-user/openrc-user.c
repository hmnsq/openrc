#ಅನ್ತರ್ಭಾವ <einfo.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <grp.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <syslog.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <spawn.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
#ಅನ್ತರ್ಭಾವ <security/pam_appl.h>
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#ಅನ್ತರ್ಭಾವ "helpers.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "rc_exec.h"

ಸ್ಥಿರ bool valid_shell(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	ಅಕ್ಷರ *ಸಾಲು = ಖಾಲಿ;
	size_t ಗಾತ್ರ_2;

	ಆದರೆ (!fp)
		ಹಿಂತಿರುಗಿ true;

	ಆಗ (xgetline(&ಸಾಲು, &ಗಾತ್ರ_2, fp) != -1) {
		ಆದರೆ (ಸಾಲು[0] == '#')
			ಮುಂದುವರಿ;
		ಆದರೆ (strcmp(ಸಾಲು, shell) == 0) {
			ret = true;
			ನಿಲ್ಲಿಸು;
		}
	}

	ಬಿಡುಗಡೆ(ಸಾಲು);
	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ bool spawn_openrc(uid_t uid, gid_t gid, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *shell, bool ಆರಂಭ) {
	ಅಕ್ಷರ *argv0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argv[] = {
		ಖಾಲಿ /* replaced below */, "-c",
		ಆರಂಭ ? USER_SH "start" : USER_SH "stop", ಖಾಲಿ
	};
	ರಚನೆ exec_ಫಲಿತಾಂಶ res;
	ರಚನೆ exec_ವಾದಗಳು ವಾದಗಳು;

	/* shell might be a multicall binary, e.g busybox.
	 * so setting argv[0] to "-" might not work */
	xasprintf(&argv0, "-%s", valid_shell(shell) ? shell : "/bin/sh");
	argv[0] = argv0;
	ವಾದಗಳು = exec_init(argv);
	ವಾದಗಳು.cmd = shell;
	ವಾದಗಳು.uid = uid;
	ವಾದಗಳು.gid = gid;
	ವಾದಗಳು.redirect_stdout = 3;
	res = do_exec(&ವಾದಗಳು);
	ಬಿಡುಗಡೆ(argv0);
	ಆದರೆ (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		ಹಿಂತಿರುಗಿ false;
	}
	ಹಿಂತಿರುಗಿ rc_waitpid(res.pid) == 0;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) {
	ಸ್ಥಿರಾಂಕ ರಚನೆ passwd *user;
	ಅಕ್ಷರ *ಲಘುಗಣಕ, *shell;
	sigset_t sigmask;
	ಪೂರ್ಣಾಂಕ sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
	ರಚನೆ pam_conv conv = { ಖಾಲಿ, ಖಾಲಿ };
	pam_handle_t *pamh = ಖಾಲಿ;
	bool pam_session = false;
	ಪೂರ್ಣಾಂಕ rc;
#ಆದರೆ_ಅಂತ್ಯ

	setenv("EINFO_LOG", "openrc-user", true);

	ಆದರೆ (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		ಹಿಂತಿರುಗಿ -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	ಆದರೆ (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		ಹಿಂತಿರುಗಿ -1;
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

	xasprintf(&ಲಘುಗಣಕ, "openrc-user.%s", user->pw_name);
	setenv("EINFO_LOG", ಲಘುಗಣಕ, true);
	ಬಿಡುಗಡೆ(ಲಘುಗಣಕ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
	ಆದರೆ ((rc = pam_ಆರಂಭ("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	ಇಲ್ಲದಿದ್ದರೆ
		pam_session = true;

	ಗಾಗಿ (ಅಕ್ಷರ **env = pam_getenvlist(pamh); env && *env; env++) {
		ಆದರೆ (strchr(*env, '='))
			putenv(xstrdup(*env));
		ಇಲ್ಲದಿದ್ದರೆ
			unsetenv(*env);
	}

	ಆದರೆ (!pam_session && pamh) {
		pam_ಕೊನೆ(pamh, rc);
		pamh = ಖಾಲಿ;
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, ಖಾಲಿ);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	ಆದರೆ (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

ಹೊರಹೋಗು:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
	ಆದರೆ (pamh) {
		ಆದರೆ ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_ಕೊನೆ(pamh, rc);
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಬಿಡುಗಡೆ(shell);
	ಹಿಂತಿರುಗಿ ret;
}
