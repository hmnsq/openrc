#অন্তর্ভাৱ <pwd.h>
#অন্তর্ভাৱ <grp.h>
#অন্তর্ভাৱ <security/pam_modules.h>
#অন্তর্ভাৱ <security/pam_appl.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <sys/file.h>
#অন্তর্ভাৱ <syslog.h>
#অন্তর্ভাৱ <unistd.h>

#অন্তর্ভাৱ "librc.h"
#অন্তর্ভাৱ "einfo.h"
#অন্তর্ভাৱ "helpers.h"

স্থির bool
import_env(pam_handle_t *pamh, ধ্রুৱ ৱর্ণ *user)
{
	ৱর্ণ *path, *string = রিক্ত;
	bool ret = false;
	size_t size;
	FILE *fp;

	xasprintf(&path, "user/%s/environ", user);

	যদি (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), path, O_RDONLY)))
		গচ্ছ out;

	যেতিয়ালৈ (getdelim(&string, &size, '\0', fp) != -1)
		pam_putenv(pamh, string);

	মুক্ত(string);
	fclose(fp);

	ret = true;
out:
	মুক্ত(path);

	ঘূৰাই ret;
}

স্থির পূর্ণাঙ্ক
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	ৱর্ণ *do_autostart = rc_conf_মান("rc_autostart_user");
	ৱর্ণ *svc_name, *pam_lock, *logins, *script = রিক্ত;
	ধ্রুৱ ৱর্ণ *username = রিক্ত, *session = রিক্ত;
	RC_SERVICE service_status;
	পূর্ণাঙ্ক গুনতি = 0, fd, status;
	পূর্ণাঙ্ক ret = PAM_SUCCESS;
	সংরচনা passwd *user;
	pid_t pid = -1;

	errno = 0;
	যদি (!rc_yesno(do_autostart) && errno == 0)
		ঘূৰাই PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	যদি (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	যদি (pam_get_item(pamh, PAM_SERVICE, (ধ্রুৱ শূন্য **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		ঘূৰাই PAM_SESSION_ERR;
	}

	যদি (session && strcmp(session, "openrc-user") == 0)
		ঘূৰাই PAM_SUCCESS;

	যদি (pam_get_item(pamh, PAM_USER, (ধ্রুৱ শূন্য **)&username) != PAM_SUCCESS)
		ঘূৰাই PAM_SESSION_ERR;

	যদি (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		ঘূৰাই PAM_SESSION_ERR;
	}

	যদি (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		ঘূৰাই PAM_SESSION_ERR;
	}

	যদি (user->pw_uid == 0)
		ঘূৰাই PAM_SUCCESS;

	xasprintf(&pam_lock, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_lock, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_status = rc_service_state(svc_name);
	যদি (service_status & RC_SERVICE_STARTED && !(service_status & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		গচ্ছ out;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	যদি ((fd = svc_lock(pam_lock, false)) == -1) {
		ret = PAM_SESSION_ERR;
		গচ্ছ out;
	}

	যদি (!(script = rc_service_resolve(svc_name))) {
		যদি (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			গচ্ছ unlock;
		}

		যদি (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			গচ্ছ unlock;
		}
	}

	logins = rc_service_মান_get(svc_name, "logins");
	যদি (logins)
		sscanf(logins, "%d", &গুনতি);
	মুক্ত(logins);

	যদি (opening) {
		যদি (গুনতি == 0) {
			pid = service_start(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		গুনতি++;
	} অন্যথা {
		গুনতি--;
		যদি (গুনতি == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", গুনতি);

	যদি (pid > 0) {
		waitpid(pid, &status, 0);
		যদি (status != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", গুনতি);
	rc_service_মান_set(svc_name, "logins", logins);
	মুক্ত(logins);

unlock:
	svc_unlock(pam_lock, fd);
out:
	যদি (opening)
		import_env(pamh, user->pw_name);

	মুক্ত(pam_lock);
	মুক্ত(svc_name);
	মুক্ত(script);
	unsetenv("EINFO_LOG");
	ঘূৰাই ret;
}

PAM_EXTERN পূর্ণাঙ্ক pam_sm_open_session(pam_handle_t *pamh, পূর্ণাঙ্ক flags, পূর্ণাঙ্ক argc, ধ্রুৱ ৱর্ণ **argv) {
	(শূন্য) argc;
	(শূন্য) argv;

	ঘূৰাই exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN পূর্ণাঙ্ক pam_sm_close_session(pam_handle_t *pamh, পূর্ণাঙ্ক flags, পূর্ণাঙ্ক argc, ধ্রুৱ ৱর্ণ **argv) {
	(শূন্য) argc;
	(শূন্য) argv;
	(শূন্য) flags;

	ঘূৰাই exec_openrc(pamh, false, flags & PAM_SILENT);
}
