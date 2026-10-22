#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <grp.h>
#ಅನ್ತರ್ಭಾವ <security/pam_modules.h>
#ಅನ್ತರ್ಭಾವ <security/pam_appl.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/file.h>
#ಅನ್ತರ್ಭಾವ <syslog.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "librc.h"
#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಸ್ಥಿರ bool
import_env(pam_handle_t *pamh, ಧ್ರುವ ವರ್ಣ *user)
{
	ವರ್ಣ *path, *string = ರಿಕ್ತ;
	bool ret = false;
	size_t size;
	FILE *fp;

	xasprintf(&path, "user/%s/environ", user);

	ಯದಿ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), path, O_RDONLY)))
		ಗಚ್ಛ out;

	ಯಾವತ್ (getdelim(&string, &size, '\0', fp) != -1)
		pam_putenv(pamh, string);

	ಮುಕ್ತ(string);
	fclose(fp);

	ret = true;
out:
	ಮುಕ್ತ(path);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	ವರ್ಣ *do_autostart = rc_conf_ಮೌಲ್ಯ("rc_autostart_user");
	ವರ್ಣ *svc_name, *pam_lock, *logins, *script = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *username = ರಿಕ್ತ, *session = ರಿಕ್ತ;
	RC_SERVICE service_status;
	ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0, fd, status;
	ಪೂರ್ಣಾಙ್ಕ ret = PAM_SUCCESS;
	ಸಂರಚನಾ passwd *user;
	pid_t pid = -1;

	errno = 0;
	ಯದಿ (!rc_yesno(do_autostart) && errno == 0)
		ಪ್ರತ್ಯಾಗಮ PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	ಯದಿ (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	ಯದಿ (pam_get_item(pamh, PAM_SERVICE, (ಧ್ರುವ ಶೂನ್ಯ **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		ಪ್ರತ್ಯಾಗಮ PAM_SESSION_ERR;
	}

	ಯದಿ (session && strcmp(session, "openrc-user") == 0)
		ಪ್ರತ್ಯಾಗಮ PAM_SUCCESS;

	ಯದಿ (pam_get_item(pamh, PAM_USER, (ಧ್ರುವ ಶೂನ್ಯ **)&username) != PAM_SUCCESS)
		ಪ್ರತ್ಯಾಗಮ PAM_SESSION_ERR;

	ಯದಿ (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		ಪ್ರತ್ಯಾಗಮ PAM_SESSION_ERR;
	}

	ಯದಿ (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		ಪ್ರತ್ಯಾಗಮ PAM_SESSION_ERR;
	}

	ಯದಿ (user->pw_uid == 0)
		ಪ್ರತ್ಯಾಗಮ PAM_SUCCESS;

	xasprintf(&pam_lock, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_lock, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_status = rc_service_state(svc_name);
	ಯದಿ (service_status & RC_SERVICE_STARTED && !(service_status & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		ಗಚ್ಛ out;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	ಯದಿ ((fd = svc_lock(pam_lock, false)) == -1) {
		ret = PAM_SESSION_ERR;
		ಗಚ್ಛ out;
	}

	ಯದಿ (!(script = rc_service_resolve(svc_name))) {
		ಯದಿ (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			ಗಚ್ಛ unlock;
		}

		ಯದಿ (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			ಗಚ್ಛ unlock;
		}
	}

	logins = rc_service_ಮೌಲ್ಯ_get(svc_name, "logins");
	ಯದಿ (logins)
		sscanf(logins, "%d", &ಎಣಿಕೆ);
	ಮುಕ್ತ(logins);

	ಯದಿ (opening) {
		ಯದಿ (ಎಣಿಕೆ == 0) {
			pid = service_start(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		ಎಣಿಕೆ++;
	} ಅನ್ಯಥಾ {
		ಎಣಿಕೆ--;
		ಯದಿ (ಎಣಿಕೆ == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", ಎಣಿಕೆ);

	ಯದಿ (pid > 0) {
		waitpid(pid, &status, 0);
		ಯದಿ (status != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", ಎಣಿಕೆ);
	rc_service_ಮೌಲ್ಯ_set(svc_name, "logins", logins);
	ಮುಕ್ತ(logins);

unlock:
	svc_unlock(pam_lock, fd);
out:
	ಯದಿ (opening)
		import_env(pamh, user->pw_name);

	ಮುಕ್ತ(pam_lock);
	ಮುಕ್ತ(svc_name);
	ಮುಕ್ತ(script);
	unsetenv("EINFO_LOG");
	ಪ್ರತ್ಯಾಗಮ ret;
}

PAM_EXTERN ಪೂರ್ಣಾಙ್ಕ pam_sm_open_session(pam_handle_t *pamh, ಪೂರ್ಣಾಙ್ಕ flags, ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ **argv) {
	(ಶೂನ್ಯ) argc;
	(ಶೂನ್ಯ) argv;

	ಪ್ರತ್ಯಾಗಮ exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN ಪೂರ್ಣಾಙ್ಕ pam_sm_close_session(pam_handle_t *pamh, ಪೂರ್ಣಾಙ್ಕ flags, ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ **argv) {
	(ಶೂನ್ಯ) argc;
	(ಶೂನ್ಯ) argv;
	(ಶೂನ್ಯ) flags;

	ಪ್ರತ್ಯಾಗಮ exec_openrc(pamh, false, flags & PAM_SILENT);
}
