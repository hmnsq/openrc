/*
 * selinux.c
 * SELinux helpers to get and set contexts.
 */

/*
 * Copyright (c) 2014-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <dlfcn.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ <selinux/selinux.h>
#ಅನ್ತರ್ಭಾವ <selinux/label.h>
#ಅನ್ತರ್ಭಾವ <selinux/get_default_type.h>
#ಅನ್ತರ್ಭಾವ <selinux/context.h>

#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "plugin.h"
#ಅನ್ತರ್ಭಾವ "selinux.h"

/* the context files for selinux */
#define INITRC_FILE "initrc_context"

#ifdef HAVE_AUDIT
#ಅನ್ತರ್ಭಾವ <libaudit.h>
#endif

/* PAM or shadow for authentication */
#ifdef HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#ಅನ್ಯಥಾ
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#endif


/* The handle for the fcontext lookups */
ಸ್ಥಿರ ಸಂರಚನಾ selabel_handle *hnd = ರಿಕ್ತ;

ಪೂರ್ಣಾಙ್ಕ selinux_util_label(ಧ್ರುವ ವರ್ಣ *path)
{
	ಪೂರ್ಣಾಙ್ಕ retval = 0;
	ಪೂರ್ಣಾಙ್ಕ enforce;
	ಸಂರಚನಾ stat st;
	ವರ್ಣ *con;

	enforce = security_getenforce();
	ಯದಿ (retval < 0)
		ಪ್ರತ್ಯಾಗಮ retval;

	ಯದಿ (!hnd)
		ಪ್ರತ್ಯಾಗಮ (enforce) ? -1 : 0;

	retval = lstat(path, &st);
	ಯದಿ (retval < 0) {
		ಯದಿ (errno == ENOENT)
			ಪ್ರತ್ಯಾಗಮ 0;
		ಪ್ರತ್ಯಾಗಮ (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, path, st.st_mode);
	ಯದಿ (retval < 0) {
		ಯದಿ (errno == ENOENT)
			ಪ್ರತ್ಯಾಗಮ 0;
		ಪ್ರತ್ಯಾಗಮ (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(path, con);
	freecon(con);
	ಯದಿ (retval < 0) {
		ಯದಿ (errno == ENOENT)
			ಪ್ರತ್ಯಾಗಮ 0;
		ಯದಿ (errno == ENOTSUP)
			ಪ್ರತ್ಯಾಗಮ 0;
		ಪ್ರತ್ಯಾಗಮ (enforce) ? -1 : 0;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
ಪೂರ್ಣಾಙ್ಕ selinux_util_open(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ retval = 0;

	retval = is_selinux_enabled();
	ಯದಿ (retval <= 0)
		ಪ್ರತ್ಯಾಗಮ retval;

	hnd = selabel_open(SELABEL_CTX_FILE, ರಿಕ್ತ, 0);
	ಯದಿ (!hnd)
		ಪ್ರತ್ಯಾಗಮ -2;

	ಪ್ರತ್ಯಾಗಮ 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
ಪೂರ್ಣಾಙ್ಕ selinux_util_close(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ retval = 0;

	retval = is_selinux_enabled();
	ಯದಿ (retval <= 0)
		ಪ್ರತ್ಯಾಗಮ retval;

	ಯದಿ (hnd) {
		selabel_close(hnd);
		hnd = ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ check_password(ವರ್ಣ *username)
{
	ಪೂರ್ಣಾಙ್ಕ ret = 1;
#ifdef HAVE_PAM
	pam_handle_t *pamh;
	ಪೂರ್ಣಾಙ್ಕ pam_err = 0;
	ಧ್ರುವ ಸಂರಚನಾ pam_conv pconv = {
		misc_conv,
		ರಿಕ್ತ
	};

	pam_err = pam_start(PAM_SERVICE_NAME, username, &pconv, &pamh);
	ಯದಿ (pam_err != PAM_SUCCESS) {
		ret = -1;
		ಗಚ್ಛ outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	ಯದಿ (pam_err != PAM_SUCCESS) {
		ret = -1;
		ಗಚ್ಛ outpam;
	}

	ret = 0;
outpam:
	pam_end(pamh, pam_err);
	pamh = ರಿಕ್ತ;

#ಅನ್ಯಥಾ /* authenticating via /etc/shadow instead */
	ಸಂರಚನಾ spwd *spw;
	ವರ್ಣ *password;
	ವರ್ಣ *attempt;

	spw = getspnam(username);
	ಯದಿ (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		ಗಚ್ಛ outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	ಯದಿ (!attempt) {
		ret = -1;
		ಗಚ್ಛ outshadow;
	}

	ಯದಿ (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		ಗಚ್ಛ outshadow;
	}

	/* salt must be at least two characters long */
	ಯದಿ (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		ಗಚ್ಛ outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	ಯದಿ (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	ಅನ್ಯಥಾ
		ret = -1;
outshadow:
#endif
	ಪ್ರತ್ಯಾಗಮ ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ check_auth()
{
	ಸಂರಚನಾ passwd *pw;
	uid_t uid;

#ifdef HAVE_AUDIT
	uid = audit_getloginuid();
	ಯದಿ (uid == (uid_t) -1)
		uid = getuid();
#ಅನ್ಯಥಾ
	uid = getuid();
#endif

	pw = getpwuid(uid);
	ಯದಿ (!pw) {
		eerror("cannot find your entry in the passwd file.");
		ಪ್ರತ್ಯಾಗಮ (-1);
	}

	printf("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	ಯದಿ (check_password(pw->pw_name) == 0) {
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	ಪ್ರತ್ಯಾಗಮ 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ read_context_file(ಧ್ರುವ ವರ್ಣ *filename, ವರ್ಣ **context)
{
	ಪೂರ್ಣಾಙ್ಕ ret = -1;
	FILE *fp;
	ವರ್ಣ *filepath = ರಿಕ್ತ;
	ವರ್ಣ *ಸಾಲು = ರಿಕ್ತ;
	ವರ್ಣ *p;
	ವರ್ಣ *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_path(), filename);

	fp = fopen(filepath, "r");
	ಯದಿ (fp == ರಿಕ್ತ) {
		eerror("Failed to open context file: %s", filename);
		ಮುಕ್ತ(filepath);
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	ಯಾವತ್ (xgetline(&ಸಾಲು, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = ಸಾಲು;
		ಯಾವತ್ (isspace(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		ಯದಿ (*p == '\0')
			ಅನುವರ್ತ;

		/* cut off spaces after the string */
		p2 = p;
		ಯಾವತ್ (!isspace(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		ಭಙ್ಗ;
	}

	ಮುಕ್ತ(ಸಾಲು);
	ಮುಕ್ತ(filepath);
	fclose(fp);
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ read_run_init_context(ವರ್ಣ **context)
{
	ಪೂರ್ಣಾಙ್ಕ ret = -1;
	RC_STRINGLIST *list;
	ವರ್ಣ *ಮೌಲ್ಯ = ರಿಕ್ತ;

	list = rc_config_list(selinux_openrc_contexts_path());
	ಯದಿ (list == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ret;

	ಮೌಲ್ಯ = rc_config_ಮೌಲ್ಯ(list, "run_init");
	ಯದಿ (ಮೌಲ್ಯ != ರಿಕ್ತ && strlen(ಮೌಲ್ಯ) > 0) {
		*context = xstrdup(ಮೌಲ್ಯ);
		ret = 0;
	}

	rc_stringlist_free(list);
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ selinux_setup(ವರ್ಣ **argv)
{
	ವರ್ಣ *new_context = ರಿಕ್ತ;
	ವರ್ಣ *curr_context = ರಿಕ್ತ;
	context_t curr_con;
	ವರ್ಣ *curr_t = ರಿಕ್ತ;
	ವರ್ಣ *run_init_t = ರಿಕ್ತ;

	/* Return, if selinux is disabled. */
	ಯದಿ (is_selinux_enabled() < 1) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (read_run_init_context(&run_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		run_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", run_init_t);
	}

	/* Get our current context. */
	ಯದಿ (getcon(&curr_context) < 0) {
		ಯದಿ (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			ಗಚ್ಛ out;
		} ಅನ್ಯಥಾ {
			perror("getcon");
			exit(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	ಯದಿ (!curr_con) {
		ಮುಕ್ತ(curr_context);
		ಗಚ್ಛ out;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	ಯದಿ (!curr_t) {
		context_free(curr_con);
		ಮುಕ್ತ(curr_context);
		ಗಚ್ಛ out;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	ಮುಕ್ತ(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	ಯದಿ (strncmp(run_init_t, curr_t, strlen(run_init_t)) != 0) {
		ಗಚ್ಛ out;
	}

	ಮುಕ್ತ(curr_t);
	ಮುಕ್ತ(run_init_t);

	ಯದಿ (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	ಯದಿ (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	ಯದಿ (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	ಮುಕ್ತ(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	ಯದಿ (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		ಯದಿ (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			exit(-1);
		}
	} ಅನ್ಯಥಾ ಯದಿ (execvp(argv[1], argv + 1)) {
		perror("execvp");
		exit(-1);
	}

out:
	ಮುಕ್ತ(run_init_t);
	ಮುಕ್ತ(curr_t);
}
