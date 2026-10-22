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

#که_تعریف_شوی HAVE_PAM
#ضمن <security/pam_appl.h>
#د_که_پای

#تعریف_کړه USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#ضمن "helpers.h"
#ضمن "rc.h"
#ضمن "rc_exec.h"

ساكن bool valid_shell(ثابت حرف *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	حرف *کرښه = عدم;
	size_t اندازه;

	إذا (!fp)
		أرجع true;

	طالما (xgetline(&کرښه, &اندازه, fp) != -1) {
		إذا (کرښه[0] == '#')
			استمر;
		إذا (strcmp(کرښه, shell) == 0) {
			ret = true;
			اكسر;
		}
	}

	حرر(کرښه);
	أرجع ret;
}

ساكن bool spawn_openrc(uid_t uid, gid_t gid, ثابت حرف *shell, bool پیل) {
	حرف *argv0;
	ثابت حرف *argv[] = {
		عدم /* replaced below */, "-c",
		پیل ? USER_SH "start" : USER_SH "stop", عدم
	};
	بنية exec_پایله res;
	بنية exec_دلیلونه دلیلونه;

	/* shell might be a multicall binary, e.g busybox.
	 * so setting argv[0] to "-" might not work */
	xasprintf(&argv0, "-%s", valid_shell(shell) ? shell : "/bin/sh");
	argv[0] = argv0;
	دلیلونه = exec_init(argv);
	دلیلونه.cmd = shell;
	دلیلونه.uid = uid;
	دلیلونه.gid = gid;
	دلیلونه.redirect_stdout = 3;
	res = do_exec(&دلیلونه);
	حرر(argv0);
	إذا (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		أرجع false;
	}
	أرجع rc_waitpid(res.pid) == 0;
}

صحيح اصلي(صحيح argc, حرف **argv) {
	ثابت بنية passwd *user;
	حرف *لوګاریتم, *shell;
	sigset_t sigmask;
	صحيح sig, ret = 0;
	uid_t uid;
	gid_t gid;
#که_تعریف_شوی HAVE_PAM
	بنية pam_conv conv = { عدم, عدم };
	pam_handle_t *pamh = عدم;
	bool pam_session = false;
	صحيح rc;
#د_که_پای

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

	xasprintf(&لوګاریتم, "openrc-user.%s", user->pw_name);
	setenv("EINFO_LOG", لوګاریتم, true);
	حرر(لوګاریتم);

#که_تعریف_شوی HAVE_PAM
	إذا ((rc = pam_پیل("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
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
		pam_پای_ټکی(pamh, rc);
		pamh = عدم;
	}
#د_که_پای

	إذا (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		اذهب بهر;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, عدم);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	إذا (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

بهر:
#که_تعریف_شوی HAVE_PAM
	إذا (pamh) {
		إذا ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_پای_ټکی(pamh, rc);
	}
#د_که_پای

	حرر(shell);
	أرجع ret;
}
