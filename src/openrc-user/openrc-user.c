#include <einfo.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <spawn.h>

#ifdef HAVE_PAM
#include <security/pam_appl.h>
#endif

#define USER_SH RC_LIBEXECDIR "/sh/openrc-user.sh "

#include "helpers.h"
#include "rc.h"
#include "rc_exec.h"

static bool valid_shell(const var_na *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	var_na *line = NULL;
	size_t size;

	གལ་ཏེ (!fp)
		ཕྱིར true;

	སྐབས (xgetline(&line, &size, fp) != -1) {
		གལ་ཏེ (line[0] == '#')
			continue;
		གལ་ཏེ (strcmp(line, shell) == 0) {
			ret = true;
			bha_nga;
		}
	}

	free(line);
	ཕྱིར ret;
}

static bool spawn_openrc(uid_t uid, gid_t gid, const var_na *shell, bool start) {
	var_na *argv0;
	const var_na *argv[] = {
		NULL /* replaced below */, "-c",
		start ? USER_SH "start" : USER_SH "stop", NULL
	};
	sa_nracanaa exec_result res;
	sa_nracanaa exec_args args;

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
	free(argv0);
	གལ་ཏེ (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		ཕྱིར false;
	}
	ཕྱིར rc_waitpid(res.pid) == 0;
}

ཧྲིལ གཙོ(ཧྲིལ argc, var_na **argv) {
	const sa_nracanaa passwd *user;
	var_na *log, *shell;
	sigset_t sigmask;
	ཧྲིལ sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ifdef HAVE_PAM
	sa_nracanaa pam_conv conv = { NULL, NULL };
	pam_handle_t *pamh = NULL;
	bool pam_session = false;
	ཧྲིལ rc;
#endif

	setenv("EINFO_LOG", "openrc-user", true);

	གལ་ཏེ (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		ཕྱིར -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	གལ་ཏེ (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		ཕྱིར -1;
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
	free(log);

#ifdef HAVE_PAM
	གལ་ཏེ ((rc = pam_start("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	དེ་མིན གལ་ཏེ ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	དེ་མིན
		pam_session = true;

	ཆེད་དུ (var_na **env = pam_getenvlist(pamh); env && *env; env++) {
		གལ་ཏེ (strchr(*env, '='))
			putenv(xstrdup(*env));
		དེ་མིན
			unsetenv(*env);
	}

	གལ་ཏེ (!pam_session && pamh) {
		pam_end(pamh, rc);
		pamh = NULL;
	}
#endif

	གལ་ཏེ (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		goto out;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, NULL);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	གལ་ཏེ (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

out:
#ifdef HAVE_PAM
	གལ་ཏེ (pamh) {
		གལ་ཏེ ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_end(pamh, rc);
	}
#endif

	free(shell);
	ཕྱིར ret;
}
