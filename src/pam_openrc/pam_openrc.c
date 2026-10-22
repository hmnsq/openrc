#അന്തര്ഭാവ <pwd.h>
#അന്തര്ഭാവ <grp.h>
#അന്തര്ഭാവ <security/pam_modules.h>
#അന്തര്ഭാവ <security/pam_appl.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/file.h>
#അന്തര്ഭാവ <syslog.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "librc.h"
#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "helpers.h"

സ്ഥിതം bool
import_env(pam_handle_t *pamh, സ്ഥിരം അക്ഷരം *user)
{
	അക്ഷരം *path, *string = ശൂന്യ;
	bool ret = false;
	size_t size;
	FILE *fp;

	xasprintf(&path, "user/%s/environ", user);

	എങ്കിൽ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), path, O_RDONLY)))
		പോകുക out;

	ആകുമ്പോൾ (getdelim(&string, &size, '\0', fp) != -1)
		pam_putenv(pamh, string);

	സ്വതന്ത്രം(string);
	fclose(fp);

	ret = true;
out:
	സ്വതന്ത്രം(path);

	തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	അക്ഷരം *do_autostart = rc_conf_മൂല്യം("rc_autostart_user");
	അക്ഷരം *svc_name, *pam_lock, *logins, *script = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *username = ശൂന്യ, *session = ശൂന്യ;
	RC_SERVICE service_status;
	പൂർണ്ണസംഖ്യ എണ്ണം = 0, fd, status;
	പൂർണ്ണസംഖ്യ ret = PAM_SUCCESS;
	ഘടന passwd *user;
	pid_t pid = -1;

	errno = 0;
	എങ്കിൽ (!rc_yesno(do_autostart) && errno == 0)
		തിരികെ PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	എങ്കിൽ (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	എങ്കിൽ (pam_get_item(pamh, PAM_SERVICE, (സ്ഥിരം ശൂന്യം **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		തിരികെ PAM_SESSION_ERR;
	}

	എങ്കിൽ (session && strcmp(session, "openrc-user") == 0)
		തിരികെ PAM_SUCCESS;

	എങ്കിൽ (pam_get_item(pamh, PAM_USER, (സ്ഥിരം ശൂന്യം **)&username) != PAM_SUCCESS)
		തിരികെ PAM_SESSION_ERR;

	എങ്കിൽ (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		തിരികെ PAM_SESSION_ERR;
	}

	എങ്കിൽ (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		തിരികെ PAM_SESSION_ERR;
	}

	എങ്കിൽ (user->pw_uid == 0)
		തിരികെ PAM_SUCCESS;

	xasprintf(&pam_lock, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_lock, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_status = rc_service_state(svc_name);
	എങ്കിൽ (service_status & RC_SERVICE_STARTED && !(service_status & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		പോകുക out;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	എങ്കിൽ ((fd = svc_lock(pam_lock, false)) == -1) {
		ret = PAM_SESSION_ERR;
		പോകുക out;
	}

	എങ്കിൽ (!(script = rc_service_resolve(svc_name))) {
		എങ്കിൽ (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			പോകുക unlock;
		}

		എങ്കിൽ (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			പോകുക unlock;
		}
	}

	logins = rc_service_മൂല്യം_get(svc_name, "logins");
	എങ്കിൽ (logins)
		sscanf(logins, "%d", &എണ്ണം);
	സ്വതന്ത്രം(logins);

	എങ്കിൽ (opening) {
		എങ്കിൽ (എണ്ണം == 0) {
			pid = service_start(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		എണ്ണം++;
	} അല്ലെങ്കിൽ {
		എണ്ണം--;
		എങ്കിൽ (എണ്ണം == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", എണ്ണം);

	എങ്കിൽ (pid > 0) {
		waitpid(pid, &status, 0);
		എങ്കിൽ (status != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", എണ്ണം);
	rc_service_മൂല്യം_set(svc_name, "logins", logins);
	സ്വതന്ത്രം(logins);

unlock:
	svc_unlock(pam_lock, fd);
out:
	എങ്കിൽ (opening)
		import_env(pamh, user->pw_name);

	സ്വതന്ത്രം(pam_lock);
	സ്വതന്ത്രം(svc_name);
	സ്വതന്ത്രം(script);
	unsetenv("EINFO_LOG");
	തിരികെ ret;
}

PAM_EXTERN പൂർണ്ണസംഖ്യ pam_sm_open_session(pam_handle_t *pamh, പൂർണ്ണസംഖ്യ flags, പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം **argv) {
	(ശൂന്യം) argc;
	(ശൂന്യം) argv;

	തിരികെ exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN പൂർണ്ണസംഖ്യ pam_sm_close_session(pam_handle_t *pamh, പൂർണ്ണസംഖ്യ flags, പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം **argv) {
	(ശൂന്യം) argc;
	(ശൂന്യം) argv;
	(ശൂന്യം) flags;

	തിരികെ exec_openrc(pamh, false, flags & PAM_SILENT);
}
