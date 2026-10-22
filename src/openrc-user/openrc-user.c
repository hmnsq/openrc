#অন্তর্ভাব <einfo.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <grp.h>
#অন্তর্ভাব <pwd.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/wait.h>
#অন্তর্ভাব <syslog.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <spawn.h>

#ifdef HAVE_PAM
#অন্তর্ভাব <security/pam_appl.h>
#endif

#define USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#অন্তর্ভাব "helpers.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "rc_exec.h"

স্থির bool valid_shell(ধ্রুবক অক্ষর *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	অক্ষর *সারি = ফাঁকা;
	size_t আকার_2;

	যদি (!fp)
		ফেরত true;

	যখন (xgetline(&সারি, &আকার_2, fp) != -1) {
		যদি (সারি[0] == '#')
			চালু;
		যদি (strcmp(সারি, shell) == 0) {
			ret = true;
			বিরতি;
		}
	}

	মুক্ত(সারি);
	ফেরত ret;
}

স্থির bool spawn_openrc(uid_t uid, gid_t gid, ধ্রুবক অক্ষর *shell, bool start) {
	অক্ষর *argv0;
	ধ্রুবক অক্ষর *argv[] = {
		ফাঁকা /* replaced below */, "-c",
		start ? USER_SH "start" : USER_SH "stop", ফাঁকা
	};
	কাঠামো exec_ফলাফল res;
	কাঠামো exec_args args;

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
	মুক্ত(argv0);
	যদি (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		ফেরত false;
	}
	ফেরত rc_waitpid(res.pid) == 0;
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv) {
	ধ্রুবক কাঠামো passwd *user;
	অক্ষর *log, *shell;
	sigset_t sigmask;
	পূর্ণসংখ্যা sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ifdef HAVE_PAM
	কাঠামো pam_conv conv = { ফাঁকা, ফাঁকা };
	pam_handle_t *pamh = ফাঁকা;
	bool pam_session = false;
	পূর্ণসংখ্যা rc;
#endif

	setenv("EINFO_LOG", "openrc-user", true);

	যদি (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		ফেরত -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	যদি (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		ফেরত -1;
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
	মুক্ত(log);

#ifdef HAVE_PAM
	যদি ((rc = pam_start("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	অন্যথা যদি ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	অন্যথা
		pam_session = true;

	জন্য (অক্ষর **env = pam_getenvlist(pamh); env && *env; env++) {
		যদি (strchr(*env, '='))
			putenv(xstrdup(*env));
		অন্যথা
			unsetenv(*env);
	}

	যদি (!pam_session && pamh) {
		pam_end(pamh, rc);
		pamh = ফাঁকা;
	}
#endif

	যদি (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		যাও out;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, ফাঁকা);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	যদি (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

out:
#ifdef HAVE_PAM
	যদি (pamh) {
		যদি ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_end(pamh, rc);
	}
#endif

	মুক্ত(shell);
	ফেরত ret;
}
