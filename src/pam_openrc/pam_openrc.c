#அந்தர்பாவ <pwd.h>
#அந்தர்பாவ <grp.h>
#அந்தர்பாவ <security/pam_modules.h>
#அந்தர்பாவ <security/pam_appl.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/file.h>
#அந்தர்பாவ <syslog.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "librc.h"
#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "helpers.h"

நிலையான bool
import_env(pam_handle_t *pamh, மாறிலி எழுத்து *user)
{
	எழுத்து *path, *string = வெற்று;
	bool ret = false;
	size_t size;
	FILE *fp;

	xasprintf(&path, "user/%s/environ", user);

	எனில் (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), path, O_RDONLY)))
		செல் out;

	போது (getdelim(&string, &size, '\0', fp) != -1)
		pam_putenv(pamh, string);

	விடுவி(string);
	fclose(fp);

	ret = true;
out:
	விடுவி(path);

	திரும்பு ret;
}

நிலையான முழு
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	எழுத்து *do_autostart = rc_conf_மதிப்பு("rc_autostart_user");
	எழுத்து *svc_name, *pam_lock, *logins, *script = வெற்று;
	மாறிலி எழுத்து *username = வெற்று, *session = வெற்று;
	RC_SERVICE service_status;
	முழு மொத்தம் = 0, fd, status;
	முழு ret = PAM_SUCCESS;
	கட்டமைப்பு passwd *user;
	pid_t pid = -1;

	errno = 0;
	எனில் (!rc_yesno(do_autostart) && errno == 0)
		திரும்பு PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	எனில் (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	எனில் (pam_get_item(pamh, PAM_SERVICE, (மாறிலி வெறுமை **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		திரும்பு PAM_SESSION_ERR;
	}

	எனில் (session && strcmp(session, "openrc-user") == 0)
		திரும்பு PAM_SUCCESS;

	எனில் (pam_get_item(pamh, PAM_USER, (மாறிலி வெறுமை **)&username) != PAM_SUCCESS)
		திரும்பு PAM_SESSION_ERR;

	எனில் (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		திரும்பு PAM_SESSION_ERR;
	}

	எனில் (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		திரும்பு PAM_SESSION_ERR;
	}

	எனில் (user->pw_uid == 0)
		திரும்பு PAM_SUCCESS;

	xasprintf(&pam_lock, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_lock, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_status = rc_service_state(svc_name);
	எனில் (service_status & RC_SERVICE_STARTED && !(service_status & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		செல் out;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	எனில் ((fd = svc_lock(pam_lock, false)) == -1) {
		ret = PAM_SESSION_ERR;
		செல் out;
	}

	எனில் (!(script = rc_service_resolve(svc_name))) {
		எனில் (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			செல் unlock;
		}

		எனில் (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			செல் unlock;
		}
	}

	logins = rc_service_மதிப்பு_get(svc_name, "logins");
	எனில் (logins)
		sscanf(logins, "%d", &மொத்தம்);
	விடுவி(logins);

	எனில் (opening) {
		எனில் (மொத்தம் == 0) {
			pid = service_start(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		மொத்தம்++;
	} இல்லையெனில் {
		மொத்தம்--;
		எனில் (மொத்தம் == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", மொத்தம்);

	எனில் (pid > 0) {
		waitpid(pid, &status, 0);
		எனில் (status != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", மொத்தம்);
	rc_service_மதிப்பு_set(svc_name, "logins", logins);
	விடுவி(logins);

unlock:
	svc_unlock(pam_lock, fd);
out:
	எனில் (opening)
		import_env(pamh, user->pw_name);

	விடுவி(pam_lock);
	விடுவி(svc_name);
	விடுவி(script);
	unsetenv("EINFO_LOG");
	திரும்பு ret;
}

PAM_EXTERN முழு pam_sm_open_session(pam_handle_t *pamh, முழு flags, முழு argc, மாறிலி எழுத்து **argv) {
	(வெறுமை) argc;
	(வெறுமை) argv;

	திரும்பு exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN முழு pam_sm_close_session(pam_handle_t *pamh, முழு flags, முழு argc, மாறிலி எழுத்து **argv) {
	(வெறுமை) argc;
	(வெறுமை) argv;
	(வெறுமை) flags;

	திரும்பு exec_openrc(pamh, false, flags & PAM_SILENT);
}
