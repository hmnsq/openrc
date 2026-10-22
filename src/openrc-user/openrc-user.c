#அந்தர்பாவ <einfo.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <grp.h>
#அந்தர்பாவ <pwd.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <syslog.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <spawn.h>

#ifdef HAVE_PAM
#அந்தர்பாவ <security/pam_appl.h>
#endif

#define USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#அந்தர்பாவ "helpers.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "rc_exec.h"

நிலையான bool valid_shell(மாறிலி எழுத்து *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	எழுத்து *வரி = வெற்று;
	size_t size;

	எனில் (!fp)
		திரும்பு true;

	போது (xgetline(&வரி, &size, fp) != -1) {
		எனில் (வரி[0] == '#')
			தொடர்;
		எனில் (strcmp(வரி, shell) == 0) {
			ret = true;
			நிறுத்து;
		}
	}

	விடுவி(வரி);
	திரும்பு ret;
}

நிலையான bool spawn_openrc(uid_t uid, gid_t gid, மாறிலி எழுத்து *shell, bool start) {
	எழுத்து *argv0;
	மாறிலி எழுத்து *argv[] = {
		வெற்று /* replaced below */, "-c",
		start ? USER_SH "start" : USER_SH "stop", வெற்று
	};
	கட்டமைப்பு exec_result res;
	கட்டமைப்பு exec_args args;

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
	விடுவி(argv0);
	எனில் (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		திரும்பு false;
	}
	திரும்பு rc_waitpid(res.pid) == 0;
}

முழு முதன்மை(முழு argc, எழுத்து **argv) {
	மாறிலி கட்டமைப்பு passwd *user;
	எழுத்து *log, *shell;
	sigset_t sigmask;
	முழு sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ifdef HAVE_PAM
	கட்டமைப்பு pam_conv conv = { வெற்று, வெற்று };
	pam_handle_t *pamh = வெற்று;
	bool pam_session = false;
	முழு rc;
#endif

	setenv("EINFO_LOG", "openrc-user", true);

	எனில் (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		திரும்பு -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	எனில் (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		திரும்பு -1;
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
	விடுவி(log);

#ifdef HAVE_PAM
	எனில் ((rc = pam_start("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	இல்லையெனில் எனில் ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	இல்லையெனில்
		pam_session = true;

	க்கு (எழுத்து **env = pam_getenvlist(pamh); env && *env; env++) {
		எனில் (strchr(*env, '='))
			putenv(xstrdup(*env));
		இல்லையெனில்
			unsetenv(*env);
	}

	எனில் (!pam_session && pamh) {
		pam_end(pamh, rc);
		pamh = வெற்று;
	}
#endif

	எனில் (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		செல் out;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, வெற்று);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	எனில் (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

out:
#ifdef HAVE_PAM
	எனில் (pamh) {
		எனில் ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_end(pamh, rc);
	}
#endif

	விடுவி(shell);
	திரும்பு ret;
}
