#অন্তর্ভাৱ <einfo.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <grp.h>
#অন্তর্ভাৱ <pwd.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/wait.h>
#অন্তর্ভাৱ <syslog.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <signal.h>
#অন্তর্ভাৱ <spawn.h>

#ifdef HAVE_PAM
#অন্তর্ভাৱ <security/pam_appl.h>
#endif

#define USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#অন্তর্ভাৱ "helpers.h"
#অন্তর্ভাৱ "rc.h"
#অন্তর্ভাৱ "rc_exec.h"

স্থির bool valid_shell(ধ্রুৱ ৱর্ণ *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	ৱর্ণ *সাৰি = রিক্ত;
	size_t size;

	যদি (!fp)
		ঘূৰাই true;

	যেতিয়ালৈ (xgetline(&সাৰি, &size, fp) != -1) {
		যদি (সাৰি[0] == '#')
			অনুৱর্ত;
		যদি (strcmp(সাৰি, shell) == 0) {
			ret = true;
			ভঙ্গ;
		}
	}

	মুক্ত(সাৰি);
	ঘূৰাই ret;
}

স্থির bool spawn_openrc(uid_t uid, gid_t gid, ধ্রুৱ ৱর্ণ *shell, bool start) {
	ৱর্ণ *argv0;
	ধ্রুৱ ৱর্ণ *argv[] = {
		রিক্ত /* replaced below */, "-c",
		start ? USER_SH "start" : USER_SH "stop", রিক্ত
	};
	সংরচনা exec_result res;
	সংরচনা exec_args args;

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
		ঘূৰাই false;
	}
	ঘূৰাই rc_waitpid(res.pid) == 0;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv) {
	ধ্রুৱ সংরচনা passwd *user;
	ৱর্ণ *log, *shell;
	sigset_t sigmask;
	পূর্ণাঙ্ক sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ifdef HAVE_PAM
	সংরচনা pam_conv conv = { রিক্ত, রিক্ত };
	pam_handle_t *pamh = রিক্ত;
	bool pam_session = false;
	পূর্ণাঙ্ক rc;
#endif

	setenv("EINFO_LOG", "openrc-user", true);

	যদি (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		ঘূৰাই -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	যদি (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		ঘূৰাই -1;
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

	বাবে (ৱর্ণ **env = pam_getenvlist(pamh); env && *env; env++) {
		যদি (strchr(*env, '='))
			putenv(xstrdup(*env));
		অন্যথা
			unsetenv(*env);
	}

	যদি (!pam_session && pamh) {
		pam_end(pamh, rc);
		pamh = রিক্ত;
	}
#endif

	যদি (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		গচ্ছ out;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, রিক্ত);

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
	ঘূৰাই ret;
}
