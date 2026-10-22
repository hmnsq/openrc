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
	char *लिन्या = NULL;
	size_t size;

	if (!fp)
		return true;

	while (xgetline(&लिन्या, &size, fp) != -1) {
		if (लिन्या[0] == '#')
			continue;
		if (strcmp(लिन्या, shell) == 0) {
			ret = true;
			break;
		}
	}

	free(लिन्या);
	return ret;
}

static bool spawn_openrc(uid_t uid, gid_t gid, const char *shell, bool start) {
	char *argv0;
	const char *argv[] = {
		NULL /* replaced below */, "-c",
		start ? USER_SH "start" : USER_SH "stop", NULL
	};
	struct exec_result res;
	struct exec_मगा_कातविरान मगा_कातविरान;

	/* shell might be a multicall binary, e.g busybox.
	 * so setting argv[0] to "-" might not work */
	xasprintf(&argv0, "-%s", valid_shell(shell) ? shell : "/bin/sh");
	argv[0] = argv0;
	मगा_कातविरान = exec_init(argv);
	मगा_कातविरान.cmd = shell;
	मगा_कातविरान.uid = uid;
	मगा_कातविरान.gid = gid;
	मगा_कातविरान.redirect_stdout = 3;
	res = do_exec(&मगा_कातविरान);
	free(argv0);
	if (res.pid < 0) {
		elog(LOG_ERR, "do_exec: %s", strerror(errno));
		return false;
	}
	return rc_waitpid(res.pid) == 0;
}

int main(int argc, char **argv) {
	const struct passwd *user;
	char *log, *shell;
	sigset_t sigmask;
	int sig, ret = 0;
	uid_t uid;
	gid_t gid;
#ifdef HAVE_PAM
	struct pam_conv conv = { NULL, NULL };
	pam_handle_t *pamh = NULL;
	bool pam_session = false;
	int rc;
#endif

	setenv("EINFO_LOG", "openrc-user", true);

	if (argc != 2) {
		elog(LOG_ERR, "Invalid usage. %s <username>", argv[0]);
		return -1;
	}
	/* We can't rely on the supervisor to drop perms since
	 * pam modules might need root perms. */
	if (!(user = getpwnam(argv[1]))) {
		elog(LOG_ERR, "Invalid username %s.", argv[1]);
		return -1;
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
	if ((rc = pam_start("openrc-user", user->pw_name, &conv, &pamh)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to start pam: %s", pam_strerror(pamh, rc));
	else if ((rc = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
		elog(LOG_ERR, "Failed to open session: %s", pam_strerror(pamh, rc));
	else
		pam_session = true;

	for (char **env = pam_getenvlist(pamh); env && *env; env++) {
		if (strchr(*env, '='))
			putenv(xstrdup(*env));
		else
			unsetenv(*env);
	}

	if (!pam_session && pamh) {
		pam_end(pamh, rc);
		pamh = NULL;
	}
#endif

	if (!spawn_openrc(uid, gid, shell, true)) {
		ret = -1;
		goto out;
	}

	sigfillset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, NULL);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	sigwait(&sigmask, &sig);

	if (!spawn_openrc(uid, gid, shell, false))
		ret = -1;

out:
#ifdef HAVE_PAM
	if (pamh) {
		if ((rc = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS)
			elog(LOG_ERR, "Failed to close session: %s", pam_strerror(pamh, rc));
		pam_end(pamh, rc);
	}
#endif

	free(shell);
	return ret;
}
