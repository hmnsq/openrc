#ଅନ୍ତର୍ଭାଵ <einfo.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <grp.h>
#ଅନ୍ତର୍ଭାଵ <pwd.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <syslog.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <spawn.h>

#ifdef HAVE_PAM
#ଅନ୍ତର୍ଭାଵ <security/pam_appl.h>
#endif

#define USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#ଅନ୍ତର୍ଭାଵ "helpers.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "rc_exec.h"

ସ୍ଥିର bool valid_shell(ଅଚଳ ଅକ୍ଷର *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	ଅକ୍ଷର *ଧାଡ଼ି = ଖାଲି;
	size_t size;

	ଯଦି (!fp)
		ଫେରନ୍ତୁ true;

	ଯେତେବେଳେ (xgetline(&ଧାଡ଼ି, &size, fp) != -1) {
		ଯଦି (ଧାଡ଼ି[0] == '#')
			ଜାରି;
		ଯଦି (strcmp(ଧାଡ଼ି, shell) == 0) {
			ret = true;
			ଭାଙ୍ଗ;
		}
	}

	ମୁକ୍ତ(ଧାଡ଼ି);
	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର bool spawn_openrc(uid_t uid, gid_t gid, ଅଚଳ ଅକ୍ଷର *shell, bool start) {
	ଅକ୍ଷର *argv0;
	ଅଚଳ ଅକ୍ଷର *argv[] = {
		ଖାଲି /* replaced below */, "-c",
		start ? USER_SH "start" : USER_SH "stop", ଖାଲି
	};
	ସଂରଚନା exec_result res;
	ସଂରଚନା exec_args args;

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
	ମୁକ୍ତ(argv0);
	ଯଦି (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		ଫେରନ୍ତୁ false;
	}
	ଫେରନ୍ତୁ rc_waitpid(res.pid) == 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) {
	ଅଚଳ ସଂରଚନା passwd *user;
	ଅକ୍ଷର *log, *shell;
	sigset_t sigmask;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ifdef HAVE_PAM
	ସଂରଚନା pam_conv conv = { ଖାଲି, ଖାଲି };
	pam_handle_t *pamh = ଖାଲି;
	bool pam_session = false;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
#endif

	setenv("EINFO_LOG", "openrc-user", true);

	ଯଦି (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		ଫେରନ୍ତୁ -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	ଯଦି (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		ଫେରନ୍ତୁ -1;
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
	ମୁକ୍ତ(log);

#ifdef HAVE_PAM
	ଯଦି ((rc = pam_start("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	ଅନ୍ୟଥା ଯଦି ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	ଅନ୍ୟଥା
		pam_session = true;

	ପାଇଁ (ଅକ୍ଷର **env = pam_getenvlist(pamh); env && *env; env++) {
		ଯଦି (strchr(*env, '='))
			putenv(xstrdup(*env));
		ଅନ୍ୟଥା
			unsetenv(*env);
	}

	ଯଦି (!pam_session && pamh) {
		pam_end(pamh, rc);
		pamh = ଖାଲି;
	}
#endif

	ଯଦି (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		ଯାଅ out;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, ଖାଲି);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	ଯଦି (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

out:
#ifdef HAVE_PAM
	ଯଦି (pamh) {
		ଯଦି ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_end(pamh, rc);
	}
#endif

	ମୁକ୍ତ(shell);
	ଫେରନ୍ତୁ ret;
}
