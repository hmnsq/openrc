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
import_env(pam_handle_t *pamh, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *user)
{
	ಅಕ್ಷರ *ಮಾರ್ಗ, *string = ಖಾಲಿ;
	bool ret = false;
	size_t ಗಾತ್ರ_2;
	FILE *fp;

	xasprintf(&ಮಾರ್ಗ, "user/%s/environ", user);

	ಆದರೆ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), ಮಾರ್ಗ, O_RDONLY)))
		ಹೋಗು ಹೊರಹೋಗು;

	ಆಗ (getdelim(&string, &ಗಾತ್ರ_2, '\0', fp) != -1)
		pam_putenv(pamh, string);

	ಬಿಡುಗಡೆ(string);
	fclose(fp);

	ret = true;
ಹೊರಹೋಗು:
	ಬಿಡುಗಡೆ(ಮಾರ್ಗ);

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	ಅಕ್ಷರ *do_autostart = rc_conf_ಮೌಲ್ಯ("rc_autostart_user");
	ಅಕ್ಷರ *svc_name, *pam_ಬೀಗ, *logins, *script = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *username = ಖಾಲಿ, *session = ಖಾಲಿ;
	RC_SERVICE service_ಸ್ಥಿತಿ;
	ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0, fd, ಸ್ಥಿತಿ_2;
	ಪೂರ್ಣಾಂಕ ret = PAM_SUCCESS;
	ರಚನೆ passwd *user;
	pid_t pid = -1;

	errno = 0;
	ಆದರೆ (!rc_yesno(do_autostart) && errno == 0)
		ಹಿಂತಿರುಗಿ PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	ಆದರೆ (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	ಆದರೆ (pam_get_ಅಂಶ(pamh, PAM_SERVICE, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		ಹಿಂತಿರುಗಿ PAM_SESSION_ERR;
	}

	ಆದರೆ (session && strcmp(session, "openrc-user") == 0)
		ಹಿಂತಿರುಗಿ PAM_SUCCESS;

	ಆದರೆ (pam_get_ಅಂಶ(pamh, PAM_USER, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **)&username) != PAM_SUCCESS)
		ಹಿಂತಿರುಗಿ PAM_SESSION_ERR;

	ಆದರೆ (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		ಹಿಂತಿರುಗಿ PAM_SESSION_ERR;
	}

	ಆದರೆ (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		ಹಿಂತಿರುಗಿ PAM_SESSION_ERR;
	}

	ಆದರೆ (user->pw_uid == 0)
		ಹಿಂತಿರುಗಿ PAM_SUCCESS;

	xasprintf(&pam_ಬೀಗ, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_ಬೀಗ, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_ಸ್ಥಿತಿ = rc_service_state(svc_name);
	ಆದರೆ (service_ಸ್ಥಿತಿ & RC_SERVICE_STARTED && !(service_ಸ್ಥಿತಿ & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		ಹೋಗು ಹೊರಹೋಗು;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	ಆದರೆ ((fd = svc_ಬೀಗ(pam_ಬೀಗ, false)) == -1) {
		ret = PAM_SESSION_ERR;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (!(script = rc_service_resolve(svc_name))) {
		ಆದರೆ (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			ಹೋಗು unlock;
		}

		ಆದರೆ (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			ಹೋಗು unlock;
		}
	}

	logins = rc_service_ಮೌಲ್ಯ_get(svc_name, "logins");
	ಆದರೆ (logins)
		ಅಕ್ಷರಸರಣಿ_ಓದು(logins, "%d", &ಎಣಿಕೆ_2);
	ಬಿಡುಗಡೆ(logins);

	ಆದರೆ (opening) {
		ಆದರೆ (ಎಣಿಕೆ_2 == 0) {
			pid = service_ಆರಂಭ(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		ಎಣಿಕೆ_2++;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಎಣಿಕೆ_2--;
		ಆದರೆ (ಎಣಿಕೆ_2 == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", ಎಣಿಕೆ_2);

	ಆದರೆ (pid > 0) {
		waitpid(pid, &ಸ್ಥಿತಿ_2, 0);
		ಆದರೆ (ಸ್ಥಿತಿ_2 != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", ಎಣಿಕೆ_2);
	rc_service_ಮೌಲ್ಯ_ಗಣ(svc_name, "logins", logins);
	ಬಿಡುಗಡೆ(logins);

unlock:
	svc_unlock(pam_ಬೀಗ, fd);
ಹೊರಹೋಗು:
	ಆದರೆ (opening)
		import_env(pamh, user->pw_name);

	ಬಿಡುಗಡೆ(pam_ಬೀಗ);
	ಬಿಡುಗಡೆ(svc_name);
	ಬಿಡುಗಡೆ(script);
	unsetenv("EINFO_LOG");
	ಹಿಂತಿರುಗಿ ret;
}

PAM_EXTERN ಪೂರ್ಣಾಂಕ pam_sm_open_session(pam_handle_t *pamh, ಪೂರ್ಣಾಂಕ flags, ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv) {
	(ಶೂನ್ಯ) argc;
	(ಶೂನ್ಯ) argv;

	ಹಿಂತಿರುಗಿ exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN ಪೂರ್ಣಾಂಕ pam_sm_close_session(pam_handle_t *pamh, ಪೂರ್ಣಾಂಕ flags, ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv) {
	(ಶೂನ್ಯ) argc;
	(ಶೂನ್ಯ) argv;
	(ಶೂನ್ಯ) flags;

	ಹಿಂತಿರುಗಿ exec_openrc(pamh, false, flags & PAM_SILENT);
}
