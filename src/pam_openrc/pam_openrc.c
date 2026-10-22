#شامل <pwd.h>
#شامل <grp.h>
#شامل <security/pam_modules.h>
#شامل <security/pam_appl.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <string.h>
#شامل <sys/file.h>
#شامل <syslog.h>
#شامل <unistd.h>

#شامل "librc.h"
#شامل "einfo.h"
#شامل "helpers.h"

ساکن bool
import_env(pam_handle_t *pamh, ثابت حرف *user)
{
	حرف *راستہ, *string = تہی;
	bool ret = false;
	size_t سائز;
	FILE *fp;

	xasprintf(&راستہ, "user/%s/environ", user);

	اگر (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), راستہ, O_RDONLY)))
		جاؤ برآمد;

	جب (getdelim(&string, &سائز, '\0', fp) != -1)
		pam_putenv(pamh, string);

	آزاد(string);
	fclose(fp);

	ret = true;
برآمد:
	آزاد(راستہ);

	واپس ret;
}

ساکن صحیح
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	حرف *do_autostart = rc_conf_قدر("rc_autostart_user");
	حرف *svc_name, *pam_تالا, *logins, *script = تہی;
	ثابت حرف *username = تہی, *session = تہی;
	RC_SERVICE service_حالت;
	صحیح گنتی = 0, fd, حالت_2;
	صحیح ret = PAM_SUCCESS;
	ساخت passwd *user;
	pid_t pid = -1;

	errno = 0;
	اگر (!rc_yesno(do_autostart) && errno == 0)
		واپس PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	اگر (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	اگر (pam_get_شے(pamh, PAM_SERVICE, (ثابت خالی **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		واپس PAM_SESSION_ERR;
	}

	اگر (session && strcmp(session, "openrc-user") == 0)
		واپس PAM_SUCCESS;

	اگر (pam_get_شے(pamh, PAM_USER, (ثابت خالی **)&username) != PAM_SUCCESS)
		واپس PAM_SESSION_ERR;

	اگر (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		واپس PAM_SESSION_ERR;
	}

	اگر (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		واپس PAM_SESSION_ERR;
	}

	اگر (user->pw_uid == 0)
		واپس PAM_SUCCESS;

	xasprintf(&pam_تالا, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_تالا, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_حالت = rc_service_state(svc_name);
	اگر (service_حالت & RC_SERVICE_STARTED && !(service_حالت & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		جاؤ برآمد;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	اگر ((fd = svc_تالا(pam_تالا, false)) == -1) {
		ret = PAM_SESSION_ERR;
		جاؤ برآمد;
	}

	اگر (!(script = rc_service_resolve(svc_name))) {
		اگر (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			جاؤ unlock;
		}

		اگر (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			جاؤ unlock;
		}
	}

	logins = rc_service_قدر_get(svc_name, "logins");
	اگر (logins)
		لڑی_سے_پڑھو(logins, "%d", &گنتی);
	آزاد(logins);

	اگر (opening) {
		اگر (گنتی == 0) {
			pid = service_آغاز(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		گنتی++;
	} ورنہ {
		گنتی--;
		اگر (گنتی == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", گنتی);

	اگر (pid > 0) {
		waitpid(pid, &حالت_2, 0);
		اگر (حالت_2 != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", گنتی);
	rc_service_قدر_سیٹ(svc_name, "logins", logins);
	آزاد(logins);

unlock:
	svc_unlock(pam_تالا, fd);
برآمد:
	اگر (opening)
		import_env(pamh, user->pw_name);

	آزاد(pam_تالا);
	آزاد(svc_name);
	آزاد(script);
	unsetenv("EINFO_LOG");
	واپس ret;
}

PAM_EXTERN صحیح pam_sm_open_session(pam_handle_t *pamh, صحیح flags, صحیح argc, ثابت حرف **argv) {
	(خالی) argc;
	(خالی) argv;

	واپس exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN صحیح pam_sm_close_session(pam_handle_t *pamh, صحیح flags, صحیح argc, ثابت حرف **argv) {
	(خالی) argc;
	(خالی) argv;
	(خالی) flags;

	واپس exec_openrc(pamh, false, flags & PAM_SILENT);
}
