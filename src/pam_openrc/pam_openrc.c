#अन्तर्भाव <pwd.h>
#अन्तर्भाव <grp.h>
#अन्तर्भाव <security/pam_modules.h>
#अन्तर्भाव <security/pam_appl.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/file.h>
#अन्तर्भाव <syslog.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "librc.h"
#अन्तर्भाव "einfo.h"
#अन्तर्भाव "helpers.h"

स्थिर bool
import_env(pam_handle_t *pamh, ध्रुव वर्ण *user)
{
	वर्ण *path, *string = रिक्त;
	bool ret = false;
	size_t size;
	FILE *fp;

	xasprintf(&path, "user/%s/environ", user);

	जइ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), path, O_RDONLY)))
		गच्छ out;

	जाव (getdelim(&string, &size, '\0', fp) != -1)
		pam_putenv(pamh, string);

	मुक्त(string);
	fclose(fp);

	ret = true;
out:
	मुक्त(path);

	प्रत्यागम ret;
}

स्थिर पूर्णाङ्क
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	वर्ण *do_autostart = rc_conf_मानम्("rc_autostart_user");
	वर्ण *svc_name, *pam_lock, *logins, *script = रिक्त;
	ध्रुव वर्ण *username = रिक्त, *session = रिक्त;
	RC_SERVICE service_status;
	पूर्णाङ्क गणनम् = 0, fd, status;
	पूर्णाङ्क ret = PAM_SUCCESS;
	संरचना passwd *user;
	pid_t pid = -1;

	errno = 0;
	जइ (!rc_yesno(do_autostart) && errno == 0)
		प्रत्यागम PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	जइ (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	जइ (pam_get_item(pamh, PAM_SERVICE, (ध्रुव शून्य **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		प्रत्यागम PAM_SESSION_ERR;
	}

	जइ (session && strcmp(session, "openrc-user") == 0)
		प्रत्यागम PAM_SUCCESS;

	जइ (pam_get_item(pamh, PAM_USER, (ध्रुव शून्य **)&username) != PAM_SUCCESS)
		प्रत्यागम PAM_SESSION_ERR;

	जइ (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		प्रत्यागम PAM_SESSION_ERR;
	}

	जइ (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		प्रत्यागम PAM_SESSION_ERR;
	}

	जइ (user->pw_uid == 0)
		प्रत्यागम PAM_SUCCESS;

	xasprintf(&pam_lock, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_lock, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_status = rc_service_state(svc_name);
	जइ (service_status & RC_SERVICE_STARTED && !(service_status & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		गच्छ out;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	जइ ((fd = svc_lock(pam_lock, false)) == -1) {
		ret = PAM_SESSION_ERR;
		गच्छ out;
	}

	जइ (!(script = rc_service_resolve(svc_name))) {
		जइ (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			गच्छ unlock;
		}

		जइ (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			गच्छ unlock;
		}
	}

	logins = rc_service_मानम्_get(svc_name, "logins");
	जइ (logins)
		sscanf(logins, "%d", &गणनम्);
	मुक्त(logins);

	जइ (opening) {
		जइ (गणनम् == 0) {
			pid = service_start(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		गणनम्++;
	} अण्णहा {
		गणनम्--;
		जइ (गणनम् == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", गणनम्);

	जइ (pid > 0) {
		waitpid(pid, &status, 0);
		जइ (status != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", गणनम्);
	rc_service_मानम्_set(svc_name, "logins", logins);
	मुक्त(logins);

unlock:
	svc_unlock(pam_lock, fd);
out:
	जइ (opening)
		import_env(pamh, user->pw_name);

	मुक्त(pam_lock);
	मुक्त(svc_name);
	मुक्त(script);
	unsetenv("EINFO_LOG");
	प्रत्यागम ret;
}

PAM_EXTERN पूर्णाङ्क pam_sm_open_session(pam_handle_t *pamh, पूर्णाङ्क flags, पूर्णाङ्क argc, ध्रुव वर्ण **argv) {
	(शून्य) argc;
	(शून्य) argv;

	प्रत्यागम exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN पूर्णाङ्क pam_sm_close_session(pam_handle_t *pamh, पूर्णाङ्क flags, पूर्णाङ्क argc, ध्रुव वर्ण **argv) {
	(शून्य) argc;
	(शून्य) argv;
	(शून्य) flags;

	प्रत्यागम exec_openrc(pamh, false, flags & PAM_SILENT);
}
