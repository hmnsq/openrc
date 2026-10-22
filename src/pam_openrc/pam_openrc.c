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
import_env(pam_handle_t *pamh, अचर वर्ण *user)
{
	वर्ण *path, *string = रिक्त;
	bool ret = false;
	माप_प्रकार size;
	खाता *fp;

	xasprintf(&path, "user/%s/environ", user);

	यदि (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), path, O_RDONLY)))
		गच्छ out;

	जबसम्म (getdelim(&string, &size, '\0', fp) != -1)
		pam_putenv(pamh, string);

	मुक्त(string);
	ख_बंद(fp);

	ret = true;
out:
	मुक्त(path);

	फर्काऊ ret;
}

स्थिर पूर्णाङ्क
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	वर्ण *do_autostart = rc_conf_मान("rc_autostart_user");
	वर्ण *svc_name, *pam_lock, *logins, *script = रिक्त;
	अचर वर्ण *username = रिक्त, *session = रिक्त;
	RC_SERVICE service_status;
	पूर्णाङ्क गन्ती = 0, fd, status;
	पूर्णाङ्क ret = PAM_SUCCESS;
	संरचना passwd *user;
	pid_t pid = -1;

	त्रुटि_सं = 0;
	यदि (!rc_yesno(do_autostart) && त्रुटि_सं == 0)
		फर्काऊ PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	यदि (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	यदि (pam_get_item(pamh, PAM_SERVICE, (अचर शून्य **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		फर्काऊ PAM_SESSION_ERR;
	}

	यदि (session && म_भेद(session, "openrc-user") == 0)
		फर्काऊ PAM_SUCCESS;

	यदि (pam_get_item(pamh, PAM_USER, (अचर शून्य **)&username) != PAM_SUCCESS)
		फर्काऊ PAM_SESSION_ERR;

	यदि (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		फर्काऊ PAM_SESSION_ERR;
	}

	यदि (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		फर्काऊ PAM_SESSION_ERR;
	}

	यदि (user->pw_uid == 0)
		फर्काऊ PAM_SUCCESS;

	xasprintf(&pam_lock, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_lock, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_status = rc_service_state(svc_name);
	यदि (service_status & RC_SERVICE_STARTED && !(service_status & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		गच्छ out;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	यदि ((fd = svc_lock(pam_lock, false)) == -1) {
		ret = PAM_SESSION_ERR;
		गच्छ out;
	}

	यदि (!(script = rc_service_resolve(svc_name))) {
		यदि (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			गच्छ unlock;
		}

		यदि (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", म_त्रुटि(त्रुटि_सं));
			ret = PAM_SESSION_ERR;
			गच्छ unlock;
		}
	}

	logins = rc_service_मान_get(svc_name, "logins");
	यदि (logins)
		माला_पूछो(logins, "%d", &गन्ती);
	मुक्त(logins);

	यदि (opening) {
		यदि (गन्ती == 0) {
			pid = service_start(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		गन्ती++;
	} अन्यथा {
		गन्ती--;
		यदि (गन्ती == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", गन्ती);

	यदि (pid > 0) {
		waitpid(pid, &status, 0);
		यदि (status != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", गन्ती);
	rc_service_मान_set(svc_name, "logins", logins);
	मुक्त(logins);

unlock:
	svc_unlock(pam_lock, fd);
out:
	यदि (opening)
		import_env(pamh, user->pw_name);

	मुक्त(pam_lock);
	मुक्त(svc_name);
	मुक्त(script);
	unsetenv("EINFO_LOG");
	फर्काऊ ret;
}

PAM_EXTERN पूर्णाङ्क pam_sm_open_session(pam_handle_t *pamh, पूर्णाङ्क flags, पूर्णाङ्क argc, अचर वर्ण **argv) {
	(शून्य) argc;
	(शून्य) argv;

	फर्काऊ exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN पूर्णाङ्क pam_sm_close_session(pam_handle_t *pamh, पूर्णाङ्क flags, पूर्णाङ्क argc, अचर वर्ण **argv) {
	(शून्य) argc;
	(शून्य) argv;
	(शून्य) flags;

	फर्काऊ exec_openrc(pamh, false, flags & PAM_SILENT);
}
