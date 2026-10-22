#અન્તર્ભાવ <pwd.h>
#અન્તર્ભાવ <grp.h>
#અન્તર્ભાવ <security/pam_modules.h>
#અન્તર્ભાવ <security/pam_appl.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/file.h>
#અન્તર્ભાવ <syslog.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "librc.h"
#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "helpers.h"

સ્થિર bool
import_env(pam_handle_t *pamh, અચલ અક્ષર *user)
{
	અક્ષર *path, *string = ખાલી;
	bool ret = false;
	size_t size;
	FILE *fp;

	xasprintf(&path, "user/%s/environ", user);

	જો (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), path, O_RDONLY)))
		જાઓ out;

	જ્યારે (getdelim(&string, &size, '\0', fp) != -1)
		pam_putenv(pamh, string);

	મુક્ત(string);
	fclose(fp);

	ret = true;
out:
	મુક્ત(path);

	પાછા ret;
}

સ્થિર પૂર્ણાંક
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	અક્ષર *do_autostart = rc_conf_કિંમત("rc_autostart_user");
	અક્ષર *svc_name, *pam_lock, *logins, *script = ખાલી;
	અચલ અક્ષર *username = ખાલી, *session = ખાલી;
	RC_SERVICE service_status;
	પૂર્ણાંક ગણતરી = 0, fd, status;
	પૂર્ણાંક ret = PAM_SUCCESS;
	સંરચના passwd *user;
	pid_t pid = -1;

	errno = 0;
	જો (!rc_yesno(do_autostart) && errno == 0)
		પાછા PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	જો (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	જો (pam_get_item(pamh, PAM_SERVICE, (અચલ શૂન્ય **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		પાછા PAM_SESSION_ERR;
	}

	જો (session && strcmp(session, "openrc-user") == 0)
		પાછા PAM_SUCCESS;

	જો (pam_get_item(pamh, PAM_USER, (અચલ શૂન્ય **)&username) != PAM_SUCCESS)
		પાછા PAM_SESSION_ERR;

	જો (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		પાછા PAM_SESSION_ERR;
	}

	જો (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		પાછા PAM_SESSION_ERR;
	}

	જો (user->pw_uid == 0)
		પાછા PAM_SUCCESS;

	xasprintf(&pam_lock, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_lock, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_status = rc_service_state(svc_name);
	જો (service_status & RC_SERVICE_STARTED && !(service_status & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		જાઓ out;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	જો ((fd = svc_lock(pam_lock, false)) == -1) {
		ret = PAM_SESSION_ERR;
		જાઓ out;
	}

	જો (!(script = rc_service_resolve(svc_name))) {
		જો (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			જાઓ unlock;
		}

		જો (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			જાઓ unlock;
		}
	}

	logins = rc_service_કિંમત_get(svc_name, "logins");
	જો (logins)
		sscanf(logins, "%d", &ગણતરી);
	મુક્ત(logins);

	જો (opening) {
		જો (ગણતરી == 0) {
			pid = service_start(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		ગણતરી++;
	} નહીંતર {
		ગણતરી--;
		જો (ગણતરી == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", ગણતરી);

	જો (pid > 0) {
		waitpid(pid, &status, 0);
		જો (status != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", ગણતરી);
	rc_service_કિંમત_set(svc_name, "logins", logins);
	મુક્ત(logins);

unlock:
	svc_unlock(pam_lock, fd);
out:
	જો (opening)
		import_env(pamh, user->pw_name);

	મુક્ત(pam_lock);
	મુક્ત(svc_name);
	મુક્ત(script);
	unsetenv("EINFO_LOG");
	પાછા ret;
}

PAM_EXTERN પૂર્ણાંક pam_sm_open_session(pam_handle_t *pamh, પૂર્ણાંક flags, પૂર્ણાંક argc, અચલ અક્ષર **argv) {
	(શૂન્ય) argc;
	(શૂન્ય) argv;

	પાછા exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN પૂર્ણાંક pam_sm_close_session(pam_handle_t *pamh, પૂર્ણાંક flags, પૂર્ણાંક argc, અચલ અક્ષર **argv) {
	(શૂન્ય) argc;
	(શૂન્ય) argv;
	(શૂન્ય) flags;

	પાછા exec_openrc(pamh, false, flags & PAM_SILENT);
}
