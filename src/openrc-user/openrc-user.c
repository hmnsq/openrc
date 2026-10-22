#અન્તર્ભાવ <einfo.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <grp.h>
#અન્તર્ભાવ <pwd.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <syslog.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <spawn.h>

#ifdef HAVE_PAM
#અન્તર્ભાવ <security/pam_appl.h>
#endif

#define USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#અન્તર્ભાવ "helpers.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "rc_exec.h"

સ્થિર bool valid_shell(અચલ અક્ષર *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	અક્ષર *લીટી = ખાલી;
	size_t size;

	જો (!fp)
		પાછા true;

	જ્યારે (xgetline(&લીટી, &size, fp) != -1) {
		જો (લીટી[0] == '#')
			ચાલુ;
		જો (strcmp(લીટી, shell) == 0) {
			ret = true;
			તોડો;
		}
	}

	મુક્ત(લીટી);
	પાછા ret;
}

સ્થિર bool spawn_openrc(uid_t uid, gid_t gid, અચલ અક્ષર *shell, bool start) {
	અક્ષર *argv0;
	અચલ અક્ષર *argv[] = {
		ખાલી /* replaced below */, "-c",
		start ? USER_SH "start" : USER_SH "stop", ખાલી
	};
	સંરચના exec_result res;
	સંરચના exec_args args;

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
	મુક્ત(argv0);
	જો (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		પાછા false;
	}
	પાછા rc_waitpid(res.pid) == 0;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv) {
	અચલ સંરચના passwd *user;
	અક્ષર *log, *shell;
	sigset_t sigmask;
	પૂર્ણાંક sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ifdef HAVE_PAM
	સંરચના pam_conv conv = { ખાલી, ખાલી };
	pam_handle_t *pamh = ખાલી;
	bool pam_session = false;
	પૂર્ણાંક rc;
#endif

	setenv("EINFO_LOG", "openrc-user", true);

	જો (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		પાછા -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	જો (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		પાછા -1;
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
	મુક્ત(log);

#ifdef HAVE_PAM
	જો ((rc = pam_start("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	નહીંતર જો ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	નહીંતર
		pam_session = true;

	માટે (અક્ષર **env = pam_getenvlist(pamh); env && *env; env++) {
		જો (strchr(*env, '='))
			putenv(xstrdup(*env));
		નહીંતર
			unsetenv(*env);
	}

	જો (!pam_session && pamh) {
		pam_end(pamh, rc);
		pamh = ખાલી;
	}
#endif

	જો (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		જાઓ out;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, ખાલી);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	જો (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

out:
#ifdef HAVE_PAM
	જો (pamh) {
		જો ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_end(pamh, rc);
	}
#endif

	મુક્ત(shell);
	પાછા ret;
}
