#ଅନ୍ତର୍ଭାଵ <pwd.h>
#ଅନ୍ତର୍ଭାଵ <grp.h>
#ଅନ୍ତର୍ଭାଵ <security/pam_modules.h>
#ଅନ୍ତର୍ଭାଵ <security/pam_appl.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/file.h>
#ଅନ୍ତର୍ଭାଵ <syslog.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "librc.h"
#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ସ୍ଥିର bool
import_env(pam_handle_t *pamh, ଅଚଳ ଅକ୍ଷର *user)
{
	ଅକ୍ଷର *path, *string = ଖାଲି;
	bool ret = false;
	size_t size;
	FILE *fp;

	xasprintf(&path, "user/%s/environ", user);

	ଯଦି (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), path, O_RDONLY)))
		ଯାଅ out;

	ଯେତେବେଳେ (getdelim(&string, &size, '\0', fp) != -1)
		pam_putenv(pamh, string);

	ମୁକ୍ତ(string);
	fclose(fp);

	ret = true;
out:
	ମୁକ୍ତ(path);

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	ଅକ୍ଷର *do_autostart = rc_conf_ମୂଲ୍ୟ("rc_autostart_user");
	ଅକ୍ଷର *svc_name, *pam_lock, *logins, *script = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *username = ଖାଲି, *session = ଖାଲି;
	RC_SERVICE service_status;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0, fd, status;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = PAM_SUCCESS;
	ସଂରଚନା passwd *user;
	pid_t pid = -1;

	errno = 0;
	ଯଦି (!rc_yesno(do_autostart) && errno == 0)
		ଫେରନ୍ତୁ PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	ଯଦି (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	ଯଦି (pam_get_item(pamh, PAM_SERVICE, (ଅଚଳ ଶୂନ୍ୟ **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		ଫେରନ୍ତୁ PAM_SESSION_ERR;
	}

	ଯଦି (session && strcmp(session, "openrc-user") == 0)
		ଫେରନ୍ତୁ PAM_SUCCESS;

	ଯଦି (pam_get_item(pamh, PAM_USER, (ଅଚଳ ଶୂନ୍ୟ **)&username) != PAM_SUCCESS)
		ଫେରନ୍ତୁ PAM_SESSION_ERR;

	ଯଦି (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		ଫେରନ୍ତୁ PAM_SESSION_ERR;
	}

	ଯଦି (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		ଫେରନ୍ତୁ PAM_SESSION_ERR;
	}

	ଯଦି (user->pw_uid == 0)
		ଫେରନ୍ତୁ PAM_SUCCESS;

	xasprintf(&pam_lock, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_lock, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_status = rc_service_state(svc_name);
	ଯଦି (service_status & RC_SERVICE_STARTED && !(service_status & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		ଯାଅ out;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	ଯଦି ((fd = svc_lock(pam_lock, false)) == -1) {
		ret = PAM_SESSION_ERR;
		ଯାଅ out;
	}

	ଯଦି (!(script = rc_service_resolve(svc_name))) {
		ଯଦି (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			ଯାଅ unlock;
		}

		ଯଦି (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			ଯାଅ unlock;
		}
	}

	logins = rc_service_ମୂଲ୍ୟ_get(svc_name, "logins");
	ଯଦି (logins)
		sscanf(logins, "%d", &ଗଣନା_2);
	ମୁକ୍ତ(logins);

	ଯଦି (opening) {
		ଯଦି (ଗଣନା_2 == 0) {
			pid = service_start(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		ଗଣନା_2++;
	} ଅନ୍ୟଥା {
		ଗଣନା_2--;
		ଯଦି (ଗଣନା_2 == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", ଗଣନା_2);

	ଯଦି (pid > 0) {
		waitpid(pid, &status, 0);
		ଯଦି (status != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", ଗଣନା_2);
	rc_service_ମୂଲ୍ୟ_set(svc_name, "logins", logins);
	ମୁକ୍ତ(logins);

unlock:
	svc_unlock(pam_lock, fd);
out:
	ଯଦି (opening)
		import_env(pamh, user->pw_name);

	ମୁକ୍ତ(pam_lock);
	ମୁକ୍ତ(svc_name);
	ମୁକ୍ତ(script);
	unsetenv("EINFO_LOG");
	ଫେରନ୍ତୁ ret;
}

PAM_EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା pam_sm_open_session(pam_handle_t *pamh, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର **argv) {
	(ଶୂନ୍ୟ) argc;
	(ଶୂନ୍ୟ) argv;

	ଫେରନ୍ତୁ exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା pam_sm_close_session(pam_handle_t *pamh, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର **argv) {
	(ଶୂନ୍ୟ) argc;
	(ଶୂନ୍ୟ) argv;
	(ଶୂନ୍ୟ) flags;

	ଫେରନ୍ତୁ exec_openrc(pamh, false, flags & PAM_SILENT);
}
