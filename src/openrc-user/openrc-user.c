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

#ifdef HAVE_PAM
#شامل <security/pam_appl.h>
#endif

#define USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#شامل "helpers.h"
#شامل "rc.h"
#شامل "rc_exec.h"

ایستا bool valid_shell(ثابت نویسه *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	نویسه *سطر = پوچ;
	size_t size;

	اگر (!fp)
		بازگشت true;

	وقتی (xgetline(&سطر, &size, fp) != -1) {
		اگر (سطر[0] == '#')
			ادامه;
		اگر (strcmp(سطر, shell) == 0) {
			ret = true;
			بشکن;
		}
	}

	آزاد(سطر);
	بازگشت ret;
}

ایستا bool spawn_openrc(uid_t uid, gid_t gid, ثابت نویسه *shell, bool start) {
	نویسه *argv0;
	ثابت نویسه *argv[] = {
		پوچ /* replaced below */, "-c",
		start ? USER_SH "start" : USER_SH "stop", پوچ
	};
	ساختار exec_result res;
	ساختار exec_args args;

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
	آزاد(argv0);
	اگر (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		بازگشت false;
	}
	بازگشت rc_waitpid(res.pid) == 0;
}

صحیح اصلی(صحیح argc, نویسه **argv) {
	ثابت ساختار passwd *user;
	نویسه *log, *shell;
	sigset_t sigmask;
	صحیح sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ifdef HAVE_PAM
	ساختار pam_conv conv = { پوچ, پوچ };
	pam_handle_t *pamh = پوچ;
	bool pam_session = false;
	صحیح rc;
#endif

	setenv("EINFO_LOG", "openrc-user", true);

	اگر (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		بازگشت -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	اگر (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		بازگشت -1;
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
	آزاد(log);

#ifdef HAVE_PAM
	اگر ((rc = pam_start("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	وگرنه اگر ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	وگرنه
		pam_session = true;

	برای (نویسه **env = pam_getenvlist(pamh); env && *env; env++) {
		اگر (strchr(*env, '='))
			putenv(xstrdup(*env));
		وگرنه
			unsetenv(*env);
	}

	اگر (!pam_session && pamh) {
		pam_end(pamh, rc);
		pamh = پوچ;
	}
#endif

	اگر (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		برو out;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, پوچ);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	اگر (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

out:
#ifdef HAVE_PAM
	اگر (pamh) {
		اگر ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_end(pamh, rc);
	}
#endif

	آزاد(shell);
	بازگشت ret;
}
