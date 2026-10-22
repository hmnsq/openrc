#ਅਨ੍ਤਰ੍ਭਾਵ <pwd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <grp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <security/pam_modules.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <security/pam_appl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/file.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <syslog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "librc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਸਥਿਰ bool
import_env(pam_handle_t *pamh, ਅਚਲ ਅੱਖਰ *user)
{
	ਅੱਖਰ *ਰਸਤਾ, *string = ਨਿਰਾ;
	bool ret = false;
	size_t ਮਾਪ;
	FILE *fp;

	xasprintf(&ਰਸਤਾ, "user/%s/environ", user);

	ਜੇ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), ਰਸਤਾ, O_RDONLY)))
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜਦੋਂ (getdelim(&string, &ਮਾਪ, '\0', fp) != -1)
		pam_putenv(pamh, string);

	ਮੁਕਤ(string);
	fclose(fp);

	ret = true;
ਬਾਹਰਲਾ:
	ਮੁਕਤ(ਰਸਤਾ);

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	ਅੱਖਰ *do_autostart = rc_conf_ਮੁੱਲ("rc_autostart_user");
	ਅੱਖਰ *svc_name, *pam_ਤਾਲਾ, *logins, *script = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *username = ਨਿਰਾ, *session = ਨਿਰਾ;
	RC_SERVICE service_ਸਥਿਤੀ;
	ਪੂਰਨ ਗਿਣਤੀ_2 = 0, fd, ਸਥਿਤੀ;
	ਪੂਰਨ ret = PAM_SUCCESS;
	ਬਣਤਰ passwd *user;
	pid_t pid = -1;

	errno = 0;
	ਜੇ (!rc_yesno(do_autostart) && errno == 0)
		ਵਾਪਸ PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	ਜੇ (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	ਜੇ (pam_get_ਵਸਤੂ(pamh, PAM_SERVICE, (ਅਚਲ ਖਾਲੀ **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		ਵਾਪਸ PAM_SESSION_ERR;
	}

	ਜੇ (session && strcmp(session, "openrc-user") == 0)
		ਵਾਪਸ PAM_SUCCESS;

	ਜੇ (pam_get_ਵਸਤੂ(pamh, PAM_USER, (ਅਚਲ ਖਾਲੀ **)&username) != PAM_SUCCESS)
		ਵਾਪਸ PAM_SESSION_ERR;

	ਜੇ (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		ਵਾਪਸ PAM_SESSION_ERR;
	}

	ਜੇ (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		ਵਾਪਸ PAM_SESSION_ERR;
	}

	ਜੇ (user->pw_uid == 0)
		ਵਾਪਸ PAM_SUCCESS;

	xasprintf(&pam_ਤਾਲਾ, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_ਤਾਲਾ, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_ਸਥਿਤੀ = rc_service_state(svc_name);
	ਜੇ (service_ਸਥਿਤੀ & RC_SERVICE_STARTED && !(service_ਸਥਿਤੀ & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	ਜੇ ((fd = svc_ਤਾਲਾ(pam_ਤਾਲਾ, false)) == -1) {
		ret = PAM_SESSION_ERR;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (!(script = rc_service_resolve(svc_name))) {
		ਜੇ (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			ਜਾਓ unlock;
		}

		ਜੇ (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			ਜਾਓ unlock;
		}
	}

	logins = rc_service_ਮੁੱਲ_get(svc_name, "logins");
	ਜੇ (logins)
		ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(logins, "%d", &ਗਿਣਤੀ_2);
	ਮੁਕਤ(logins);

	ਜੇ (opening) {
		ਜੇ (ਗਿਣਤੀ_2 == 0) {
			pid = service_ਸ਼ੁਰੂ(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		ਗਿਣਤੀ_2++;
	} ਨਹੀਂਤਾਂ {
		ਗਿਣਤੀ_2--;
		ਜੇ (ਗਿਣਤੀ_2 == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", ਗਿਣਤੀ_2);

	ਜੇ (pid > 0) {
		waitpid(pid, &ਸਥਿਤੀ, 0);
		ਜੇ (ਸਥਿਤੀ != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", ਗਿਣਤੀ_2);
	rc_service_ਮੁੱਲ_ਸੈੱਟ(svc_name, "logins", logins);
	ਮੁਕਤ(logins);

unlock:
	svc_unlock(pam_ਤਾਲਾ, fd);
ਬਾਹਰਲਾ:
	ਜੇ (opening)
		import_env(pamh, user->pw_name);

	ਮੁਕਤ(pam_ਤਾਲਾ);
	ਮੁਕਤ(svc_name);
	ਮੁਕਤ(script);
	unsetenv("EINFO_LOG");
	ਵਾਪਸ ret;
}

PAM_EXTERN ਪੂਰਨ pam_sm_open_session(pam_handle_t *pamh, ਪੂਰਨ flags, ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ **argv) {
	(ਖਾਲੀ) argc;
	(ਖਾਲੀ) argv;

	ਵਾਪਸ exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN ਪੂਰਨ pam_sm_close_session(pam_handle_t *pamh, ਪੂਰਨ flags, ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ **argv) {
	(ਖਾਲੀ) argc;
	(ਖਾਲੀ) argv;
	(ਖਾਲੀ) flags;

	ਵਾਪਸ exec_openrc(pamh, false, flags & PAM_SILENT);
}
