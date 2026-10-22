#అన్తర్భావ <pwd.h>
#అన్తర్భావ <grp.h>
#అన్తర్భావ <security/pam_modules.h>
#అన్తర్భావ <security/pam_appl.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/file.h>
#అన్తర్భావ <syslog.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "librc.h"
#అన్తర్భావ "einfo.h"
#అన్తర్భావ "helpers.h"

స్థిర bool
import_env(pam_handle_t *pamh, స్థిరరాశి అక్షరం *user)
{
	అక్షరం *మార్గం, *string = ఖాళీ;
	bool ret = false;
	size_t కొలత;
	FILE *fp;

	xasprintf(&మార్గం, "user/%s/environ", user);

	ఒకవేళ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), మార్గం, O_RDONLY)))
		వెళ్ళు బహిర్గతం;

	అయితే (getdelim(&string, &కొలత, '\0', fp) != -1)
		pam_putenv(pamh, string);

	విడుదల(string);
	fclose(fp);

	ret = true;
బహిర్గతం:
	విడుదల(మార్గం);

	తిరిగి ret;
}

స్థిర పూర్ణాంకం
exec_openrc(pam_handle_t *pamh, bool opening, bool quiet)
{
	అక్షరం *do_autostart = rc_conf_విలువ("rc_autostart_user");
	అక్షరం *svc_name, *pam_తాళం, *logins, *script = ఖాళీ;
	స్థిరరాశి అక్షరం *username = ఖాళీ, *session = ఖాళీ;
	RC_SERVICE service_స్థితి;
	పూర్ణాంకం లెక్క = 0, fd, స్థితి_2;
	పూర్ణాంకం ret = PAM_SUCCESS;
	నిర్మాణం passwd *user;
	pid_t pid = -1;

	errno = 0;
	ఒకవేళ (!rc_yesno(do_autostart) && errno == 0)
		తిరిగి PAM_SUCCESS;

	setenv("EINFO_LOG", "pam_openrc", true);

	ఒకవేళ (quiet) {
		setenv("EINFO_QUIET", "yes", true);
		setenv("EERROR_QUIET", "yes", true);
	}

	ఒకవేళ (pam_get_అంశం(pamh, PAM_SERVICE, (స్థిరరాశి శూన్య **)&session) != PAM_SUCCESS) {
		elog(LOG_ERR, "Failed to get PAM_SERVICE");
		తిరిగి PAM_SESSION_ERR;
	}

	ఒకవేళ (session && strcmp(session, "openrc-user") == 0)
		తిరిగి PAM_SUCCESS;

	ఒకవేళ (pam_get_అంశం(pamh, PAM_USER, (స్థిరరాశి శూన్య **)&username) != PAM_SUCCESS)
		తిరిగి PAM_SESSION_ERR;

	ఒకవేళ (!username) {
		elog(LOG_ERR, "PAM_USER unset.");
		తిరిగి PAM_SESSION_ERR;
	}

	ఒకవేళ (!(user = getpwnam(username))) {
		elog(LOG_ERR, "User '%s' not found.", username);
		తిరిగి PAM_SESSION_ERR;
	}

	ఒకవేళ (user->pw_uid == 0)
		తిరిగి PAM_SUCCESS;

	xasprintf(&pam_తాళం, "pam_openrc[%s]", user->pw_name);
	setenv("EINFO_LOG", pam_తాళం, true);

	xasprintf(&svc_name, "user.%s", user->pw_name);

	service_స్థితి = rc_service_state(svc_name);
	ఒకవేళ (service_స్థితి & RC_SERVICE_STARTED && !(service_స్థితి & RC_SERVICE_HOTPLUGGED)) {
		elog(LOG_INFO, "%s started and not hotplugged, skipping session.", svc_name);
		వెళ్ళు బహిర్గతం;
	}

	elog(LOG_INFO, opening ? "starting session" : "stopping session");

	ఒకవేళ ((fd = svc_తాళం(pam_తాళం, false)) == -1) {
		ret = PAM_SESSION_ERR;
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (!(script = rc_service_resolve(svc_name))) {
		ఒకవేళ (!(script = rc_service_resolve("user"))) {
			elog(LOG_ERR, "Failed to resolve %s.", svc_name);
			ret = PAM_SESSION_ERR;
			వెళ్ళు unlock;
		}

		ఒకవేళ (symlinkat(script, rc_dirfd(RC_DIR_INITD), svc_name) != 0) {
			elog(LOG_ERR, "symlink: %s", strerror(errno));
			ret = PAM_SESSION_ERR;
			వెళ్ళు unlock;
		}
	}

	logins = rc_service_విలువ_get(svc_name, "logins");
	ఒకవేళ (logins)
		అక్షరశ్రేణి_చదువు(logins, "%d", &లెక్క);
	విడుదల(logins);

	ఒకవేళ (opening) {
		ఒకవేళ (లెక్క == 0) {
			pid = service_ప్రారంభం(svc_name);
			rc_service_mark(svc_name, RC_SERVICE_HOTPLUGGED);
		}
		లెక్క++;
	} లేకపోతే {
		లెక్క--;
		ఒకవేళ (లెక్క == 0)
			pid = service_stop(svc_name);
	}

	elog(LOG_INFO, "%d sessions", లెక్క);

	ఒకవేళ (pid > 0) {
		waitpid(pid, &స్థితి_2, 0);
		ఒకవేళ (స్థితి_2 != 0)
			ret = PAM_SESSION_ERR;
	}

	xasprintf(&logins, "%d", లెక్క);
	rc_service_విలువ_సమితి(svc_name, "logins", logins);
	విడుదల(logins);

unlock:
	svc_unlock(pam_తాళం, fd);
బహిర్గతం:
	ఒకవేళ (opening)
		import_env(pamh, user->pw_name);

	విడుదల(pam_తాళం);
	విడుదల(svc_name);
	విడుదల(script);
	unsetenv("EINFO_LOG");
	తిరిగి ret;
}

PAM_EXTERN పూర్ణాంకం pam_sm_open_session(pam_handle_t *pamh, పూర్ణాంకం flags, పూర్ణాంకం argc, స్థిరరాశి అక్షరం **argv) {
	(శూన్య) argc;
	(శూన్య) argv;

	తిరిగి exec_openrc(pamh, true, flags & PAM_SILENT);
}

PAM_EXTERN పూర్ణాంకం pam_sm_close_session(pam_handle_t *pamh, పూర్ణాంకం flags, పూర్ణాంకం argc, స్థిరరాశి అక్షరం **argv) {
	(శూన్య) argc;
	(శూన్య) argv;
	(శూన్య) flags;

	తిరిగి exec_openrc(pamh, false, flags & PAM_SILENT);
}
