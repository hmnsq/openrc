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

static bool valid_shell(const char *shell) {
	FILE *fp = fopen(RC_SYSCONFDIR "/shells", "r");
	bool ret = false;
	char *line = NULL;
	size_t size;

	якщо (!fp)
		повернути true;

	поки (xgetline(&line, &size, fp) != -1) {
		якщо (line[0] == '#')
			continue;
		якщо (strcmp(line, shell) == 0) {
			ret = true;
			break;
		}
	}

	free(line);
	повернути ret;
}

static bool spawn_openrc(uid_t uid, gid_t gid, const char *shell, bool start) {
	char *argv0;
	const char *argv[] = {
		NULL /* replaced below */, "-c",
		start ? USER_SH "start" : USER_SH "stop", NULL
	};
	struct exec_result res;
	struct exec_args args;

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
	якщо (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		повернути false;
	}
	повернути rc_waitpid(res.pid) == 0;
}

ціле main(ціле argc, char **argv) {
	const struct passwd *user;
	char *log, *shell;
	sigset_t sigmask;
	ціле sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ifdef HAVE_PAM
	struct pam_conv conv = { NULL, NULL };
	pam_handle_t *pamh = NULL;
	bool pam_session = false;
	ціле rc;
#endif

	setenv("EINFO_LOG", "openrc-user", true);

	якщо (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		повернути -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	якщо (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		повернути -1;
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
	якщо ((rc = pam_start("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	інакше якщо ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	інакше
		pam_session = true;

	для (char **env = pam_getenvlist(pamh); env && *env; env++) {
		якщо (strchr(*env, '='))
			putenv(xstrdup(*env));
		інакше
			unsetenv(*env);
	}

	якщо (!pam_session && pamh) {
		pam_end(pamh, rc);
		pamh = NULL;
	}
#endif

	якщо (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		goto out;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, NULL);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	якщо (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

out:
#ifdef HAVE_PAM
	якщо (pamh) {
		якщо ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_end(pamh, rc);
	}
#endif

	free(shell);
	повернути ret;
}
