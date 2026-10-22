#समावेश <pwd.h>
#समावेश <grp.h>
#समावेश <security/pam_modules.h>
#समावेश <security/pam_appl.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <string.h>
#समावेश <sys/file.h>
#समावेश <syslog.h>
#समावेश <unistd.h>

#समावेश "librc.h"
#समावेश "einfo.h"
#समावेश "helpers.h"

अटल bool
import_env(pam_handle_t *pamh, स्थिर अक्षर *user)
{
	अक्षर *path, *string = शून्य;
	bool ret = false;
	माप_प्रकार आकार;
	खाता *fp;

	xasprintf(&path, "user/%s/environ", user);

	अगर (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), path, O_RDONLY)))
		जाओ out;

	जबतक (getdelim(&string, &आकार, '\0', fp) != -1)
		pam_putenv(pamh, string);

	मुक्त(string);
	ख_बंद(fp);

	ret = true;
out:
	मुक्त(path);

	वापस ret;
}

अटल पूर्णांक
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	अक्षर *do_autostart = rc_conf_मूल्य("rc_autostart_user");
	अक्षर *svc_name, *pam_lock, *logins, *script = शून्य;
	स्थिर अक्षर *username = शून्य, *session = शून्य;
	RC_SERVICE service_status;
	पूर्णांक गिनती = 0, fd, status;
	पूर्णांक ret = PAM_SUCCESS;
	काष्ठा passwd *user;
	pid_t pid = -1;

	त्रुटि_सं = 0;
	अगर (!rc_yesno(do_autostart) && त्रुटि_सं == 0)
		वापस PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	अगर (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	अगर (pam_get_item(pamh, PAM_SERVICE, (स्थिर व्योम **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		वापस PAM_SESSION_ERR;
	}

	अगर (session && म_भेद(session, "openrc-user") == 0)
		वापस PAM_SUCCESS;

	अगर (pam_get_item(pamh, PAM_USER, (स्थिर व्योम **)&username) != PAM_SUCCESS)
		वापस PAM_SESSION_ERR;

	अगर (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		वापस PAM_SESSION_ERR;
	}

	अगर (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		वापस PAM_SESSION_ERR;
	}

	अगर (user->pw_uid == 0)
		वापस PAM_SUCCESS;

	xasprintf(&pam_lock, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_lock, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_status = rc_service_state(svc_name);
	अगर (service_status & RC_SERVICE_STARTED && !(service_status & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		जाओ out;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	अगर ((fd = svc_lock(pam_lock, false)) == -1) {
		ret = PAM_SESSION_ERR;
		जाओ out;
	}

	अगर (!(script = rc_service_resolve(svc_name))) {
		अगर (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			जाओ unlock;
		}

		अगर (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", म_त्रुटि(त्रुटि_सं));
			ret = PAM_SESSION_ERR;
			जाओ unlock;
		}
	}

	logins = rc_service_मूल्य_get(svc_name, "logins");
	अगर (logins)
		माला_पूछो(logins, "%d", &गिनती);
	मुक्त(logins);

	अगर (opening) {
		अगर (गिनती == 0) {
			pid = service_start(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		गिनती++;
	} अन्यथा {
		गिनती--;
		अगर (गिनती == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", गिनती);

	अगर (pid > 0) {
		waitpid(pid, &status, 0);
		अगर (status != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", गिनती);
	rc_service_मूल्य_set(svc_name, "logins", logins);
	मुक्त(logins);

unlock:
	svc_unlock(pam_lock, fd);
out:
	अगर (opening)
		import_env(pamh, user->pw_name);

	मुक्त(pam_lock);
	मुक्त(svc_name);
	मुक्त(script);
	unsetenv("EINFO_LOG");
	वापस ret;
}

PAM_EXTERN पूर्णांक pam_sm_open_session(pam_handle_t *pamh, पूर्णांक flags, पूर्णांक argc, स्थिर अक्षर **argv) {
	(व्योम) argc;
	(व्योम) argv;

	वापस exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN पूर्णांक pam_sm_close_session(pam_handle_t *pamh, पूर्णांक flags, पूर्णांक argc, स्थिर अक्षर **argv) {
	(व्योम) argc;
	(व्योम) argv;
	(व्योम) flags;

	वापस exec_openrc(pamh, false, flags & PAM_SILENT);
}
