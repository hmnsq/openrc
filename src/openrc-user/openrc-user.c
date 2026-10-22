#شامل <einfo.h>
#شامل <errno.h>
#شامل <fcntl.h>
#شامل <grp.h>
#شامل <pwd.h>
#شامل <stdlib.h>
#شامل <sys/types.h>
#شامل <sys/wait.h>
#شامل <syslog.h>
#شامل <string.h>
#شامل <unistd.h>
#شامل <signal.h>
#شامل <spawn.h>

#اگر_تعریف_شدہ HAVE_PAM
#شامل <security/pam_appl.h>
#اگر_ختم

#تعریف_کرو USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#شامل "helpers.h"
#شامل "rc.h"
#شامل "rc_exec.h"

ساکن bool valid_shell(ثابت حرف *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	حرف *سطر = تہی;
	size_t سائز;

	اگر (!fp)
		واپس true;

	جب (xgetline(&سطر, &سائز, fp) != -1) {
		اگر (سطر[0] == '#')
			جاری;
		اگر (strcmp(سطر, shell) == 0) {
			ret = true;
			توڑو;
		}
	}

	آزاد(سطر);
	واپس ret;
}

ساکن bool spawn_openrc(uid_t uid, gid_t gid, ثابت حرف *shell, bool آغاز) {
	حرف *argv0;
	ثابت حرف *argv[] = {
		تہی /* replaced below */, "-c",
		آغاز ? USER_SH "start" : USER_SH "stop", تہی
	};
	ساخت exec_نتیجہ res;
	ساخت exec_دلائل دلائل;

	/* shell might be a multicall binary, e.g busybox.
	 * so setting argv[0] to "-" might not work */
	xasprintf(&argv0, "-%s", valid_shell(shell) ? shell : "/bin/sh");
	argv[0] = argv0;
	دلائل = exec_init(argv);
	دلائل.cmd = shell;
	دلائل.uid = uid;
	دلائل.gid = gid;
	دلائل.redirect_stdout = 3;
	res = do_exec(&دلائل);
	آزاد(argv0);
	اگر (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		واپس false;
	}
	واپس rc_waitpid(res.pid) == 0;
}

صحیح مین(صحیح argc, حرف **argv) {
	ثابت ساخت passwd *user;
	حرف *لوگارتھم, *shell;
	sigset_t sigmask;
	صحیح sig, ret = 0;
	uid_t uid;
	gid_t gid;
#اگر_تعریف_شدہ HAVE_PAM
	ساخت pam_conv conv = { تہی, تہی };
	pam_handle_t *pamh = تہی;
	bool pam_session = false;
	صحیح rc;
#اگر_ختم

	setenv("EINFO_LOG", "openrc-user", true);

	اگر (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		واپس -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	اگر (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		واپس -1;
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

	xasprintf(&لوگارتھم, "openrc-user.%s", user->pw_name);
	setenv("EINFO_LOG", لوگارتھم, true);
	آزاد(لوگارتھم);

#اگر_تعریف_شدہ HAVE_PAM
	اگر ((rc = pam_آغاز("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	ورنہ اگر ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	ورنہ
		pam_session = true;

	برائے (حرف **env = pam_getenvlist(pamh); env && *env; env++) {
		اگر (strchr(*env, '='))
			putenv(xstrdup(*env));
		ورنہ
			unsetenv(*env);
	}

	اگر (!pam_session && pamh) {
		pam_انتہا(pamh, rc);
		pamh = تہی;
	}
#اگر_ختم

	اگر (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		جاؤ برآمد;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, تہی);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	اگر (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

برآمد:
#اگر_تعریف_شدہ HAVE_PAM
	اگر (pamh) {
		اگر ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_انتہا(pamh, rc);
	}
#اگر_ختم

	آزاد(shell);
	واپس ret;
}
