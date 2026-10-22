#समावेश <einfo.h>
#समावेश <errno.h>
#समावेश <fcntl.h>
#समावेश <grp.h>
#समावेश <pwd.h>
#समावेश <stdlib.h>
#समावेश <sys/types.h>
#समावेश <sys/wait.h>
#समावेश <syslog.h>
#समावेश <string.h>
#समावेश <unistd.h>
#समावेश <signal.h>
#समावेश <spawn.h>

#अगर_घोषित HAVE_PAM
#समावेश <security/pam_appl.h>
#पूर्ण_अगर

#घोषणा USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#समावेश "helpers.h"
#समावेश "rc.h"
#समावेश "rc_exec.h"

अटल bool valid_shell(स्थिर अक्षर *shell) {
	खाता *fp = ख_खोलो(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	अक्षर *पंक्ति = शून्य;
	माप_प्रकार आकार;

	अगर (!fp)
		वापस true;

	जबतक (xgetline(&पंक्ति, &आकार, fp) != -1) {
		अगर (पंक्ति[0] == '#')
			जारी;
		अगर (म_भेद(पंक्ति, shell) == 0) {
			ret = true;
			अवरोध;
		}
	}

	मुक्त(पंक्ति);
	वापस ret;
}

अटल bool spawn_openrc(uid_t uid, gid_t gid, स्थिर अक्षर *shell, bool start) {
	अक्षर *argv0;
	स्थिर अक्षर *argv[] = {
		शून्य /* replaced below */, "-c",
		start ? USER_SH "start" : USER_SH "stop", शून्य
	};
	काष्ठा exec_परिणाम res;
	काष्ठा exec_args args;

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
	मुक्त(argv0);
	अगर (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", म_त्रुटि(त्रुटि_सं));
		वापस false;
	}
	वापस rc_waitpid(res.pid) == 0;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv) {
	स्थिर काष्ठा passwd *user;
	अक्षर *log, *shell;
	sigset_t sigmask;
	पूर्णांक sig, ret = 0;
	uid_t uid;
	gid_t gid;
#अगर_घोषित HAVE_PAM
	काष्ठा pam_conv conv = { शून्य, शून्य };
	pam_handle_t *pamh = शून्य;
	bool pam_session = false;
	पूर्णांक rc;
#पूर्ण_अगर

	setenv("EINFO_LOG", "openrc-user", true);

	अगर (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		वापस -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	अगर (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		वापस -1;
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
	मुक्त(log);

#अगर_घोषित HAVE_PAM
	अगर ((rc = pam_start("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	अन्यथा अगर ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	अन्यथा
		pam_session = true;

	क्रम (अक्षर **env = pam_getenvlist(pamh); env && *env; env++) {
		अगर (म_अक्षर(*env, '='))
			putenv(xstrdup(*env));
		अन्यथा
			unsetenv(*env);
	}

	अगर (!pam_session && pamh) {
		pam_end(pamh, rc);
		pamh = शून्य;
	}
#पूर्ण_अगर

	अगर (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		जाओ out;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, शून्य);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, संक_इति);

	sigwait(&sigmask, &sig);

	अगर (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

out:
#अगर_घोषित HAVE_PAM
	अगर (pamh) {
		अगर ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_end(pamh, rc);
	}
#पूर्ण_अगर

	मुक्त(shell);
	वापस ret;
}
