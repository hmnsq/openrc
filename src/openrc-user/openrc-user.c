#ضمن <einfo.h>
#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <grp.h>
#ضمن <pwd.h>
#ضمن <stdlib.h>
#ضمن <sys/types.h>
#ضمن <sys/wait.h>
#ضمن <syslog.h>
#ضمن <string.h>
#ضمن <unistd.h>
#ضمن <signal.h>
#ضمن <spawn.h>

#جيڪڏهن_وصف_ٿيل HAVE_PAM
#ضمن <security/pam_appl.h>
#جيڪڏهن_پورو

#وصف_ڪريو USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#ضمن "helpers.h"
#ضمن "rc.h"
#ضمن "rc_exec.h"

ساكن bool valid_shell(ثابت حرف *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	حرف *سٽ = عدم;
	size_t سائيز;

	إذا (!fp)
		أرجع true;

	طالما (xgetline(&سٽ, &سائيز, fp) != -1) {
		إذا (سٽ[0] == '#')
			استمر;
		إذا (strcmp(سٽ, shell) == 0) {
			ret = true;
			اكسر;
		}
	}

	حرر(سٽ);
	أرجع ret;
}

ساكن bool spawn_openrc(uid_t uid, gid_t gid, ثابت حرف *shell, bool شروعات) {
	حرف *argv0;
	ثابت حرف *argv[] = {
		عدم /* replaced below */, "-c",
		شروعات ? USER_SH "start" : USER_SH "stop", عدم
	};
	بنية exec_نتيجو res;
	بنية exec_دليلون دليلون;

	/* shell might be a multicall binary, e.g busybox.
	 * so setting argv[0] to "-" might not work */
	xasprintf(&argv0, "-%s", valid_shell(shell) ? shell : "/bin/sh");
	argv[0] = argv0;
	دليلون = exec_init(argv);
	دليلون.cmd = shell;
	دليلون.uid = uid;
	دليلون.gid = gid;
	دليلون.redirect_stdout = 3;
	res = do_exec(&دليلون);
	حرر(argv0);
	إذا (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		أرجع false;
	}
	أرجع rc_waitpid(res.pid) == 0;
}

صحيح مکيه(صحيح argc, حرف **argv) {
	ثابت بنية passwd *user;
	حرف *لاگرٿم, *shell;
	sigset_t sigmask;
	صحيح sig, ret = 0;
	uid_t uid;
	gid_t gid;
#جيڪڏهن_وصف_ٿيل HAVE_PAM
	بنية pam_conv conv = { عدم, عدم };
	pam_handle_t *pamh = عدم;
	bool pam_session = false;
	صحيح rc;
#جيڪڏهن_پورو

	setenv("EINFO_LOG", "openrc-user", true);

	إذا (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		أرجع -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	إذا (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		أرجع -1;
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

	xasprintf(&لاگرٿم, "openrc-user.%s", user->pw_name);
	setenv("EINFO_LOG", لاگرٿم, true);
	حرر(لاگرٿم);

#جيڪڏهن_وصف_ٿيل HAVE_PAM
	إذا ((rc = pam_شروعات("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	وإلا إذا ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	وإلا
		pam_session = true;

	لكل (حرف **env = pam_getenvlist(pamh); env && *env; env++) {
		إذا (strchr(*env, '='))
			putenv(xstrdup(*env));
		وإلا
			unsetenv(*env);
	}

	إذا (!pam_session && pamh) {
		pam_پڇاڙي(pamh, rc);
		pamh = عدم;
	}
#جيڪڏهن_پورو

	إذا (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		اذهب ٻاهريون;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, عدم);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	إذا (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

ٻاهريون:
#جيڪڏهن_وصف_ٿيل HAVE_PAM
	إذا (pamh) {
		إذا ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_پڇاڙي(pamh, rc);
	}
#جيڪڏهن_پورو

	حرر(shell);
	أرجع ret;
}
