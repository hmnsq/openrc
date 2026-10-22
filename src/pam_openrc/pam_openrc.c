#شامل <pwd.h>
#شامل <grp.h>
#شامل <security/pam_modules.h>
#شامل <security/pam_appl.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <string.h>
#شامل <sys/file.h>
#شامل <syslog.h>
#شامل <unistd.h>

#شامل "librc.h"
#شامل "einfo.h"
#شامل "helpers.h"

ایستا bool
import_env(pam_handle_t *pamh, ثابت نویسه *user)
{
	نویسه *path, *string = پوچ;
	bool ret = false;
	size_t size;
	FILE *fp;

	xasprintf(&path, "user/%s/environ", user);

	اگر (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), path, O_RDONLY)))
		برو out;

	وقتی (getdelim(&string, &size, '\0', fp) != -1)
		pam_putenv(pamh, string);

	آزاد(string);
	fclose(fp);

	ret = true;
out:
	آزاد(path);

	بازگشت ret;
}

ایستا صحیح
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	نویسه *do_autostart = rc_conf_مقدار("rc_autostart_user");
	نویسه *svc_name, *pam_lock, *logins, *script = پوچ;
	ثابت نویسه *username = پوچ, *session = پوچ;
	RC_SERVICE service_status;
	صحیح شمار = 0, fd, status;
	صحیح ret = PAM_SUCCESS;
	ساختار passwd *user;
	pid_t pid = -1;

	errno = 0;
	اگر (!rc_yesno(do_autostart) && errno == 0)
		بازگشت PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	اگر (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	اگر (pam_get_item(pamh, PAM_SERVICE, (ثابت تهی **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		بازگشت PAM_SESSION_ERR;
	}

	اگر (session && strcmp(session, "openrc-user") == 0)
		بازگشت PAM_SUCCESS;

	اگر (pam_get_item(pamh, PAM_USER, (ثابت تهی **)&username) != PAM_SUCCESS)
		بازگشت PAM_SESSION_ERR;

	اگر (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		بازگشت PAM_SESSION_ERR;
	}

	اگر (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		بازگشت PAM_SESSION_ERR;
	}

	اگر (user->pw_uid == 0)
		بازگشت PAM_SUCCESS;

	xasprintf(&pam_lock, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_lock, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_status = rc_service_state(svc_name);
	اگر (service_status & RC_SERVICE_STARTED && !(service_status & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		برو out;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	اگر ((fd = svc_lock(pam_lock, false)) == -1) {
		ret = PAM_SESSION_ERR;
		برو out;
	}

	اگر (!(script = rc_service_resolve(svc_name))) {
		اگر (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			برو unlock;
		}

		اگر (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			برو unlock;
		}
	}

	logins = rc_service_مقدار_get(svc_name, "logins");
	اگر (logins)
		sscanf(logins, "%d", &شمار);
	آزاد(logins);

	اگر (opening) {
		اگر (شمار == 0) {
			pid = service_start(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		شمار++;
	} وگرنه {
		شمار--;
		اگر (شمار == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", شمار);

	اگر (pid > 0) {
		waitpid(pid, &status, 0);
		اگر (status != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", شمار);
	rc_service_مقدار_set(svc_name, "logins", logins);
	آزاد(logins);

unlock:
	svc_unlock(pam_lock, fd);
out:
	اگر (opening)
		import_env(pamh, user->pw_name);

	آزاد(pam_lock);
	آزاد(svc_name);
	آزاد(script);
	unsetenv("EINFO_LOG");
	بازگشت ret;
}

PAM_EXTERN صحیح pam_sm_open_session(pam_handle_t *pamh, صحیح flags, صحیح argc, ثابت نویسه **argv) {
	(تهی) argc;
	(تهی) argv;

	بازگشت exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN صحیح pam_sm_close_session(pam_handle_t *pamh, صحیح flags, صحیح argc, ثابت نویسه **argv) {
	(تهی) argc;
	(تهی) argv;
	(تهی) flags;

	بازگشت exec_openrc(pamh, false, flags & PAM_SILENT);
}
