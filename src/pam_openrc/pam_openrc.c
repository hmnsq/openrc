#ضمن <pwd.h>
#ضمن <grp.h>
#ضمن <security/pam_modules.h>
#ضمن <security/pam_appl.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <sys/file.h>
#ضمن <syslog.h>
#ضمن <unistd.h>

#ضمن "librc.h"
#ضمن "einfo.h"
#ضمن "helpers.h"

ساكن bool
import_env(pam_handle_t *pamh, ثابت حرف *user)
{
	حرف *path, *string = عدم;
	bool ret = false;
	size_t size;
	FILE *fp;

	xasprintf(&path, "user/%s/environ", user);

	אם (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), path, O_RDONLY)))
		اذهب out;

	כלעוד (getdelim(&string, &size, '\0', fp) != -1)
		pam_putenv(pamh, string);

	حرر(string);
	fclose(fp);

	ret = true;
out:
	حرر(path);

	החזר ret;
}

ساكن صحيح
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	حرف *do_autostart = rc_conf_value("rc_autostart_user");
	حرف *svc_name, *pam_lock, *logins, *script = عدم;
	ثابت حرف *username = عدم, *session = عدم;
	RC_SERVICE service_status;
	صحيح count = 0, fd, status;
	صحيح ret = PAM_SUCCESS;
	بنية passwd *user;
	pid_t pid = -1;

	errno = 0;
	אם (!rc_yesno(do_autostart) && errno == 0)
		החזר PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	אם (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	אם (pam_get_item(pamh, PAM_SERVICE, (ثابت فراغ **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		החזר PAM_SESSION_ERR;
	}

	אם (session && strcmp(session, "openrc-user") == 0)
		החזר PAM_SUCCESS;

	אם (pam_get_item(pamh, PAM_USER, (ثابت فراغ **)&username) != PAM_SUCCESS)
		החזר PAM_SESSION_ERR;

	אם (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		החזר PAM_SESSION_ERR;
	}

	אם (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		החזר PAM_SESSION_ERR;
	}

	אם (user->pw_uid == 0)
		החזר PAM_SUCCESS;

	xasprintf(&pam_lock, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_lock, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_status = rc_service_state(svc_name);
	אם (service_status & RC_SERVICE_STARTED && !(service_status & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		اذهب out;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	אם ((fd = svc_lock(pam_lock, false)) == -1) {
		ret = PAM_SESSION_ERR;
		اذهب out;
	}

	אם (!(script = rc_service_resolve(svc_name))) {
		אם (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			اذهب unlock;
		}

		אם (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			اذهب unlock;
		}
	}

	logins = rc_service_value_get(svc_name, "logins");
	אם (logins)
		sscanf(logins, "%d", &count);
	حرر(logins);

	אם (opening) {
		אם (count == 0) {
			pid = service_start(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		count++;
	} אחרת {
		count--;
		אם (count == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", count);

	אם (pid > 0) {
		waitpid(pid, &status, 0);
		אם (status != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", count);
	rc_service_value_set(svc_name, "logins", logins);
	حرر(logins);

unlock:
	svc_unlock(pam_lock, fd);
out:
	אם (opening)
		import_env(pamh, user->pw_name);

	حرر(pam_lock);
	حرر(svc_name);
	حرر(script);
	unsetenv("EINFO_LOG");
	החזר ret;
}

PAM_EXTERN صحيح pam_sm_open_session(pam_handle_t *pamh, صحيح flags, صحيح argc, ثابت حرف **argv) {
	(فراغ) argc;
	(فراغ) argv;

	החזר exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN صحيح pam_sm_close_session(pam_handle_t *pamh, صحيح flags, صحيح argc, ثابت حرف **argv) {
	(فراغ) argc;
	(فراغ) argv;
	(فراغ) flags;

	החזר exec_openrc(pamh, false, flags & PAM_SILENT);
}
