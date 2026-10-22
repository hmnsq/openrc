#అన్తర్భావ <einfo.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <grp.h>
#అన్తర్భావ <pwd.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <syslog.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <spawn.h>

#నిర్వచితమైతే HAVE_PAM
#అన్తర్భావ <security/pam_appl.h>
#అయితే_ముగింపు

#నిర్వచించు USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#అన్తర్భావ "helpers.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "rc_exec.h"

స్థిర bool valid_shell(స్థిరరాశి అక్షరం *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	అక్షరం *పంక్తి = ఖాళీ;
	size_t కొలత;

	ఒకవేళ (!fp)
		తిరిగి true;

	అయితే (xgetline(&పంక్తి, &కొలత, fp) != -1) {
		ఒకవేళ (పంక్తి[0] == '#')
			కొనసాగు;
		ఒకవేళ (strcmp(పంక్తి, shell) == 0) {
			ret = true;
			ఆపు;
		}
	}

	విడుదల(పంక్తి);
	తిరిగి ret;
}

స్థిర bool spawn_openrc(uid_t uid, gid_t gid, స్థిరరాశి అక్షరం *shell, bool ప్రారంభం) {
	అక్షరం *argv0;
	స్థిరరాశి అక్షరం *argv[] = {
		ఖాళీ /* replaced below */, "-c",
		ప్రారంభం ? USER_SH "start" : USER_SH "stop", ఖాళీ
	};
	నిర్మాణం exec_ఫలితం res;
	నిర్మాణం exec_వాదనలు వాదనలు;

	/* shell might be a multicall binary, e.g busybox.
	 * so setting argv[0] to "-" might not work */
	xasprintf(&argv0, "-%s", valid_shell(shell) ? shell : "/bin/sh");
	argv[0] = argv0;
	వాదనలు = exec_init(argv);
	వాదనలు.cmd = shell;
	వాదనలు.uid = uid;
	వాదనలు.gid = gid;
	వాదనలు.redirect_stdout = 3;
	res = do_exec(&వాదనలు);
	విడుదల(argv0);
	ఒకవేళ (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		తిరిగి false;
	}
	తిరిగి rc_waitpid(res.pid) == 0;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv) {
	స్థిరరాశి నిర్మాణం passwd *user;
	అక్షరం *సంవర్గమానం, *shell;
	sigset_t sigmask;
	పూర్ణాంకం sig, ret = 0;
	uid_t uid;
	gid_t gid;
#నిర్వచితమైతే HAVE_PAM
	నిర్మాణం pam_conv conv = { ఖాళీ, ఖాళీ };
	pam_handle_t *pamh = ఖాళీ;
	bool pam_session = false;
	పూర్ణాంకం rc;
#అయితే_ముగింపు

	setenv("EINFO_LOG", "openrc-user", true);

	ఒకవేళ (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		తిరిగి -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	ఒకవేళ (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		తిరిగి -1;
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

	xasprintf(&సంవర్గమానం, "openrc-user.%s", user->pw_name);
	setenv("EINFO_LOG", సంవర్గమానం, true);
	విడుదల(సంవర్గమానం);

#నిర్వచితమైతే HAVE_PAM
	ఒకవేళ ((rc = pam_ప్రారంభం("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	లేకపోతే ఒకవేళ ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	లేకపోతే
		pam_session = true;

	కొరకు (అక్షరం **env = pam_getenvlist(pamh); env && *env; env++) {
		ఒకవేళ (strchr(*env, '='))
			putenv(xstrdup(*env));
		లేకపోతే
			unsetenv(*env);
	}

	ఒకవేళ (!pam_session && pamh) {
		pam_చివర(pamh, rc);
		pamh = ఖాళీ;
	}
#అయితే_ముగింపు

	ఒకవేళ (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		వెళ్ళు బహిర్గతం;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, ఖాళీ);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	ఒకవేళ (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

బహిర్గతం:
#నిర్వచితమైతే HAVE_PAM
	ఒకవేళ (pamh) {
		ఒకవేళ ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_చివర(pamh, rc);
	}
#అయితే_ముగింపు

	విడుదల(shell);
	తిరిగి ret;
}
