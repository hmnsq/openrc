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
import_env(pam_handle_t *pamh, const var_na *user)
{
	var_na *path, *string = NULL;
	bool ret = false;
	size_t size;
	FILE *fp;

	xasprintf(&path, "user/%s/environ", user);

	ប្រសិនបើ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), path, O_RDONLY)))
		goto out;

	ខណៈពេល (getdelim(&string, &size, '\0', fp) != -1)
		pam_putenv(pamh, string);

	free(string);
	fclose(fp);

	ret = true;
out:
	free(path);

	ត្រឡប់ ret;
}

static ចំនួនគត់
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	var_na *do_autostart = rc_conf_value("rc_autostart_user");
	var_na *svc_name, *pam_lock, *logins, *script = NULL;
	const var_na *username = NULL, *session = NULL;
	RC_SERVICE service_status;
	ចំនួនគត់ count = 0, fd, status;
	ចំនួនគត់ ret = PAM_SUCCESS;
	sa_nracanaa passwd *user;
	pid_t pid = -1;

	errno = 0;
	ប្រសិនបើ (!rc_yesno(do_autostart) && errno == 0)
		ត្រឡប់ PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	ប្រសិនបើ (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	ប្រសិនបើ (pam_get_item(pamh, PAM_SERVICE, (const void **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		ត្រឡប់ PAM_SESSION_ERR;
	}

	ប្រសិនបើ (session && strcmp(session, "openrc-user") == 0)
		ត្រឡប់ PAM_SUCCESS;

	ប្រសិនបើ (pam_get_item(pamh, PAM_USER, (const void **)&username) != PAM_SUCCESS)
		ត្រឡប់ PAM_SESSION_ERR;

	ប្រសិនបើ (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		ត្រឡប់ PAM_SESSION_ERR;
	}

	ប្រសិនបើ (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		ត្រឡប់ PAM_SESSION_ERR;
	}

	ប្រសិនបើ (user->pw_uid == 0)
		ត្រឡប់ PAM_SUCCESS;

	xasprintf(&pam_lock, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_lock, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_status = rc_service_state(svc_name);
	ប្រសិនបើ (service_status & RC_SERVICE_STARTED && !(service_status & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		goto out;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	ប្រសិនបើ ((fd = svc_lock(pam_lock, false)) == -1) {
		ret = PAM_SESSION_ERR;
		goto out;
	}

	ប្រសិនបើ (!(script = rc_service_resolve(svc_name))) {
		ប្រសិនបើ (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			goto unlock;
		}

		ប្រសិនបើ (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			goto unlock;
		}
	}

	logins = rc_service_value_get(svc_name, "logins");
	ប្រសិនបើ (logins)
		sscanf(logins, "%d", &count);
	free(logins);

	ប្រសិនបើ (opening) {
		ប្រសិនបើ (count == 0) {
			pid = service_start(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		count++;
	} បើមិនដូច្នេះ {
		count--;
		ប្រសិនបើ (count == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", count);

	ប្រសិនបើ (pid > 0) {
		waitpid(pid, &status, 0);
		ប្រសិនបើ (status != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", count);
	rc_service_value_set(svc_name, "logins", logins);
	free(logins);

unlock:
	svc_unlock(pam_lock, fd);
out:
	ប្រសិនបើ (opening)
		import_env(pamh, user->pw_name);

	free(pam_lock);
	free(svc_name);
	free(script);
	unsetenv("EINFO_LOG");
	ត្រឡប់ ret;
}

PAM_EXTERN ចំនួនគត់ pam_sm_open_session(pam_handle_t *pamh, ចំនួនគត់ flags, ចំនួនគត់ argc, const var_na **argv) {
	(void) argc;
	(void) argv;

	ត្រឡប់ exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN ចំនួនគត់ pam_sm_close_session(pam_handle_t *pamh, ចំនួនគត់ flags, ចំនួនគត់ argc, const var_na **argv) {
	(void) argc;
	(void) argv;
	(void) flags;

	ត្រឡប់ exec_openrc(pamh, false, flags & PAM_SILENT);
}
