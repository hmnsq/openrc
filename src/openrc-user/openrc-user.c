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

#ifdef HAVE_PAM
#ಅನ್ತರ್ಭಾವ <security/pam_appl.h>
#endif

#define USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#ಅನ್ತರ್ಭಾವ "helpers.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "rc_exec.h"

ಸ್ಥಿರ bool valid_shell(ಧ್ರುವ ವರ್ಣ *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	ವರ್ಣ *ಸಾಲು = ರಿಕ್ತ;
	size_t size;

	ಯದಿ (!fp)
		ಪ್ರತ್ಯಾಗಮ true;

	ಯಾವತ್ (xgetline(&ಸಾಲು, &size, fp) != -1) {
		ಯದಿ (ಸಾಲು[0] == '#')
			ಅನುವರ್ತ;
		ಯದಿ (strcmp(ಸಾಲು, shell) == 0) {
			ret = true;
			ಭಙ್ಗ;
		}
	}

	ಮುಕ್ತ(ಸಾಲು);
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ bool spawn_openrc(uid_t uid, gid_t gid, ಧ್ರುವ ವರ್ಣ *shell, bool start) {
	ವರ್ಣ *argv0;
	ಧ್ರುವ ವರ್ಣ *argv[] = {
		ರಿಕ್ತ /* replaced below */, "-c",
		start ? USER_SH "start" : USER_SH "stop", ರಿಕ್ತ
	};
	ಸಂರಚನಾ exec_result res;
	ಸಂರಚನಾ exec_args args;

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
	ಮುಕ್ತ(argv0);
	ಯದಿ (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		ಪ್ರತ್ಯಾಗಮ false;
	}
	ಪ್ರತ್ಯಾಗಮ rc_waitpid(res.pid) == 0;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) {
	ಧ್ರುವ ಸಂರಚನಾ passwd *user;
	ವರ್ಣ *log, *shell;
	sigset_t sigmask;
	ಪೂರ್ಣಾಙ್ಕ sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ifdef HAVE_PAM
	ಸಂರಚನಾ pam_conv conv = { ರಿಕ್ತ, ರಿಕ್ತ };
	pam_handle_t *pamh = ರಿಕ್ತ;
	bool pam_session = false;
	ಪೂರ್ಣಾಙ್ಕ rc;
#endif

	setenv("EINFO_LOG", "openrc-user", true);

	ಯದಿ (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	ಯದಿ (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		ಪ್ರತ್ಯಾಗಮ -1;
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
	ಮುಕ್ತ(log);

#ifdef HAVE_PAM
	ಯದಿ ((rc = pam_start("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	ಅನ್ಯಥಾ ಯದಿ ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	ಅನ್ಯಥಾ
		pam_session = true;

	ಪ್ರತಿ (ವರ್ಣ **env = pam_getenvlist(pamh); env && *env; env++) {
		ಯದಿ (strchr(*env, '='))
			putenv(xstrdup(*env));
		ಅನ್ಯಥಾ
			unsetenv(*env);
	}

	ಯದಿ (!pam_session && pamh) {
		pam_end(pamh, rc);
		pamh = ರಿಕ್ತ;
	}
#endif

	ಯದಿ (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		ಗಚ್ಛ out;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, ರಿಕ್ತ);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	ಯದಿ (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

out:
#ifdef HAVE_PAM
	ಯದಿ (pamh) {
		ಯದಿ ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_end(pamh, rc);
	}
#endif

	ಮುಕ್ತ(shell);
	ಪ್ರತ್ಯಾಗಮ ret;
}
