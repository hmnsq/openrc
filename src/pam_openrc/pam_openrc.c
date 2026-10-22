#include <pwd.h>
#include <grp.h>
#include <security/pam_modules.h>
#include <security/pam_appl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <syslog.h>
#include <unistd.h>

#include "librc.h"
#include "einfo.h"
#include "helpers.h"

static bool
import_env(pam_handle_t *pamh, const char *user)
{
	char *path, *string = NULL;
	bool ret = false;
	size_t size;
	FILE *fp;

	xasprintf(&path, "user/%s/environ", user);

	もし (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), path, O_RDONLY)))
		goto out;

	ながら (getdelim(&string, &size, '\0', fp) != -1)
		pam_putenv(pamh, string);

	free(string);
	fclose(fp);

	ret = true;
out:
	free(path);

	戻る ret;
}

static 整数
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	char *do_autostart = rc_conf_value("rc_autostart_user");
	char *svc_name, *pam_lock, *logins, *script = NULL;
	const char *username = NULL, *session = NULL;
	RC_SERVICE service_status;
	整数 count = 0, fd, status;
	整数 ret = PAM_SUCCESS;
	struct passwd *user;
	pid_t pid = -1;

	errno = 0;
	もし (!rc_yesno(do_autostart) && errno == 0)
		戻る PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	もし (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	もし (pam_get_item(pamh, PAM_SERVICE, (const void **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		戻る PAM_SESSION_ERR;
	}

	もし (session && strcmp(session, "openrc-user") == 0)
		戻る PAM_SUCCESS;

	もし (pam_get_item(pamh, PAM_USER, (const void **)&username) != PAM_SUCCESS)
		戻る PAM_SESSION_ERR;

	もし (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		戻る PAM_SESSION_ERR;
	}

	もし (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		戻る PAM_SESSION_ERR;
	}

	もし (user->pw_uid == 0)
		戻る PAM_SUCCESS;

	xasprintf(&pam_lock, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_lock, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_status = rc_service_state(svc_name);
	もし (service_status & RC_SERVICE_STARTED && !(service_status & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		goto out;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	もし ((fd = svc_lock(pam_lock, false)) == -1) {
		ret = PAM_SESSION_ERR;
		goto out;
	}

	もし (!(script = rc_service_resolve(svc_name))) {
		もし (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			goto unlock;
		}

		もし (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			goto unlock;
		}
	}

	logins = rc_service_value_get(svc_name, "logins");
	もし (logins)
		sscanf(logins, "%d", &count);
	free(logins);

	もし (opening) {
		もし (count == 0) {
			pid = service_start(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		count++;
	} 否则 {
		count--;
		もし (count == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", count);

	もし (pid > 0) {
		waitpid(pid, &status, 0);
		もし (status != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", count);
	rc_service_value_set(svc_name, "logins", logins);
	free(logins);

unlock:
	svc_unlock(pam_lock, fd);
out:
	もし (opening)
		import_env(pamh, user->pw_name);

	free(pam_lock);
	free(svc_name);
	free(script);
	unsetenv("EINFO_LOG");
	戻る ret;
}

PAM_EXTERN 整数 pam_sm_open_session(pam_handle_t *pamh, 整数 flags, 整数 argc, const char **argv) {
	(void) argc;
	(void) argv;

	戻る exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN 整数 pam_sm_close_session(pam_handle_t *pamh, 整数 flags, 整数 argc, const char **argv) {
	(void) argc;
	(void) argv;
	(void) flags;

	戻る exec_openrc(pamh, false, flags & PAM_SILENT);
}
