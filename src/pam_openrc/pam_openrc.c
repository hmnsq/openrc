#ضمن <pwd.h>
#ضمن <grp.h>
#ضمن <security/pam_modules.h>
#ضمن <security/pam_appl.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <sys/file.h>
#ضمن <syslog.h>
#ضمن <unistd.h>

#ضمن "librc.h"
#ضمن "einfo.h"
#ضمن "helpers.h"

ساكن bool
import_env(pam_handle_t *pamh, ثابت حرف *user)
{
	حرف *لار, *string = عدم;
	bool ret = false;
	size_t اندازه;
	FILE *fp;

	xasprintf(&لار, "user/%s/environ", user);

	إذا (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), لار, O_RDONLY)))
		اذهب بهر;

	طالما (getdelim(&string, &اندازه, '\0', fp) != -1)
		pam_putenv(pamh, string);

	حرر(string);
	fclose(fp);

	ret = true;
بهر:
	حرر(لار);

	أرجع ret;
}

ساكن صحيح
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	حرف *do_autostart = rc_conf_ارزښت("rc_autostart_user");
	حرف *svc_name, *pam_قلف, *logins, *script = عدم;
	ثابت حرف *username = عدم, *session = عدم;
	RC_SERVICE service_حالت;
	صحيح شمېر = 0, fd, حالت;
	صحيح ret = PAM_SUCCESS;
	بنية passwd *user;
	pid_t pid = -1;

	errno = 0;
	إذا (!rc_yesno(do_autostart) && errno == 0)
		أرجع PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	إذا (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	إذا (pam_get_توکی(pamh, PAM_SERVICE, (ثابت فراغ **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		أرجع PAM_SESSION_ERR;
	}

	إذا (session && strcmp(session, "openrc-user") == 0)
		أرجع PAM_SUCCESS;

	إذا (pam_get_توکی(pamh, PAM_USER, (ثابت فراغ **)&username) != PAM_SUCCESS)
		أرجع PAM_SESSION_ERR;

	إذا (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		أرجع PAM_SESSION_ERR;
	}

	إذا (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		أرجع PAM_SESSION_ERR;
	}

	إذا (user->pw_uid == 0)
		أرجع PAM_SUCCESS;

	xasprintf(&pam_قلف, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_قلف, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_حالت = rc_service_state(svc_name);
	إذا (service_حالت & RC_SERVICE_STARTED && !(service_حالت & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		اذهب بهر;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	إذا ((fd = svc_قلف(pam_قلف, false)) == -1) {
		ret = PAM_SESSION_ERR;
		اذهب بهر;
	}

	إذا (!(script = rc_service_resolve(svc_name))) {
		إذا (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			اذهب unlock;
		}

		إذا (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			اذهب unlock;
		}
	}

	logins = rc_service_ارزښت_get(svc_name, "logins");
	إذا (logins)
		له_مزي_ولوله(logins, "%d", &شمېر);
	حرر(logins);

	إذا (opening) {
		إذا (شمېر == 0) {
			pid = service_پیل(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		شمېر++;
	} وإلا {
		شمېر--;
		إذا (شمېر == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", شمېر);

	إذا (pid > 0) {
		waitpid(pid, &حالت, 0);
		إذا (حالت != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", شمېر);
	rc_service_ارزښت_سېټ(svc_name, "logins", logins);
	حرر(logins);

unlock:
	svc_unlock(pam_قلف, fd);
بهر:
	إذا (opening)
		import_env(pamh, user->pw_name);

	حرر(pam_قلف);
	حرر(svc_name);
	حرر(script);
	unsetenv("EINFO_LOG");
	أرجع ret;
}

PAM_EXTERN صحيح pam_sm_open_session(pam_handle_t *pamh, صحيح flags, صحيح argc, ثابت حرف **argv) {
	(فراغ) argc;
	(فراغ) argv;

	أرجع exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN صحيح pam_sm_close_session(pam_handle_t *pamh, صحيح flags, صحيح argc, ثابت حرف **argv) {
	(فراغ) argc;
	(فراغ) argv;
	(فراغ) flags;

	أرجع exec_openrc(pamh, false, flags & PAM_SILENT);
}
