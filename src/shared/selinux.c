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
#ವ್ಯಾಖ್ಯಾನಿಸು INITRC_FILE "initrc_context"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_AUDIT
#ಅನ್ತರ್ಭಾವ <libaudit.h>
#ಆದರೆ_ಅಂತ್ಯ

/* PAM or shadow for authentication */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#ಇಲ್ಲದಿದ್ದರೆ
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#ಆದರೆ_ಅಂತ್ಯ


/* The handle for the fcontext lookups */
ಸ್ಥಿರ ರಚನೆ selabel_handle *hnd = ಖಾಲಿ;

ಪೂರ್ಣಾಂಕ selinux_util_label(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಪೂರ್ಣಾಂಕ retval = 0;
	ಪೂರ್ಣಾಂಕ enforce;
	ರಚನೆ stat st;
	ಅಕ್ಷರ *con;

	enforce = security_getenforce();
	ಆದರೆ (retval < 0)
		ಹಿಂತಿರುಗಿ retval;

	ಆದರೆ (!hnd)
		ಹಿಂತಿರುಗಿ (enforce) ? -1 : 0;

	retval = lstat(ಮಾರ್ಗ, &st);
	ಆದರೆ (retval < 0) {
		ಆದರೆ (errno == ENOENT)
			ಹಿಂತಿರುಗಿ 0;
		ಹಿಂತಿರುಗಿ (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, ಮಾರ್ಗ, st.st_mode);
	ಆದರೆ (retval < 0) {
		ಆದರೆ (errno == ENOENT)
			ಹಿಂತಿರುಗಿ 0;
		ಹಿಂತಿರುಗಿ (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(ಮಾರ್ಗ, con);
	freecon(con);
	ಆದರೆ (retval < 0) {
		ಆದರೆ (errno == ENOENT)
			ಹಿಂತಿರುಗಿ 0;
		ಆದರೆ (errno == ENOTSUP)
			ಹಿಂತಿರುಗಿ 0;
		ಹಿಂತಿರುಗಿ (enforce) ? -1 : 0;
	}

	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
ಪೂರ್ಣಾಂಕ selinux_util_open(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ retval = 0;

	retval = is_selinux_enabled();
	ಆದರೆ (retval <= 0)
		ಹಿಂತಿರುಗಿ retval;

	hnd = selabel_open(SELABEL_CTX_FILE, ಖಾಲಿ, 0);
	ಆದರೆ (!hnd)
		ಹಿಂತಿರುಗಿ -2;

	ಹಿಂತಿರುಗಿ 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
ಪೂರ್ಣಾಂಕ selinux_util_close(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ retval = 0;

	retval = is_selinux_enabled();
	ಆದರೆ (retval <= 0)
		ಹಿಂತಿರುಗಿ retval;

	ಆದರೆ (hnd) {
		selabel_close(hnd);
		hnd = ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_password(ಅಕ್ಷರ *username)
{
	ಪೂರ್ಣಾಂಕ ret = 1;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
	pam_handle_t *pamh;
	ಪೂರ್ಣಾಂಕ pam_err = 0;
	ಸ್ಥಿರಾಂಕ ರಚನೆ pam_conv pconv = {
		misc_conv,
		ಖಾಲಿ
	};

	pam_err = pam_ಆರಂಭ(PAM_SERVICE_NAME, username, &pconv, &pamh);
	ಆದರೆ (pam_err != PAM_SUCCESS) {
		ret = -1;
		ಹೋಗು outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	ಆದರೆ (pam_err != PAM_SUCCESS) {
		ret = -1;
		ಹೋಗು outpam;
	}

	ret = 0;
outpam:
	pam_ಕೊನೆ(pamh, pam_err);
	pamh = ಖಾಲಿ;

#ಇಲ್ಲದಿದ್ದರೆ /* authenticating via /etc/shadow instead */
	ರಚನೆ spwd *spw;
	ಅಕ್ಷರ *password;
	ಅಕ್ಷರ *attempt;

	spw = getspnam(username);
	ಆದರೆ (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		ಹೋಗು outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	ಆದರೆ (!attempt) {
		ret = -1;
		ಹೋಗು outshadow;
	}

	ಆದರೆ (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		ಹೋಗು outshadow;
	}

	/* salt must be at least two characters long */
	ಆದರೆ (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		ಹೋಗು outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	ಆದರೆ (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	ಇಲ್ಲದಿದ್ದರೆ
		ret = -1;
outshadow:
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_auth()
{
	ರಚನೆ passwd *pw;
	uid_t uid;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_AUDIT
	uid = audit_getloginuid();
	ಆದರೆ (uid == (uid_t) -1)
		uid = getuid();
#ಇಲ್ಲದಿದ್ದರೆ
	uid = getuid();
#ಆದರೆ_ಅಂತ್ಯ

	pw = getpwuid(uid);
	ಆದರೆ (!pw) {
		eerror("cannot find your entry in the passwd file.");
		ಹಿಂತಿರುಗಿ (-1);
	}

	ಬರೆ("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	ಆದರೆ (check_password(pw->pw_name) == 0) {
		ಹಿಂತಿರುಗಿ 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	ಹಿಂತಿರುಗಿ 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ read_context_file(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename, ಅಕ್ಷರ **context)
{
	ಪೂರ್ಣಾಂಕ ret = -1;
	FILE *fp;
	ಅಕ್ಷರ *filepath = ಖಾಲಿ;
	ಅಕ್ಷರ *ಸಾಲು = ಖಾಲಿ;
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_ಮಾರ್ಗ(), filename);

	fp = fopen(filepath, "r");
	ಆದರೆ (fp == ಖಾಲಿ) {
		eerror("Failed to open context file: %s", filename);
		ಬಿಡುಗಡೆ(filepath);
		ಹಿಂತಿರುಗಿ -1;
	}

	ಆಗ (xgetline(&ಸಾಲು, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = ಸಾಲು;
		ಆಗ (ಖಾಲಿಯೇ(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		ಆದರೆ (*p == '\0')
			ಮುಂದುವರಿ;

		/* cut off spaces after the string */
		p2 = p;
		ಆಗ (!ಖಾಲಿಯೇ(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		ನಿಲ್ಲಿಸು;
	}

	ಬಿಡುಗಡೆ(ಸಾಲು);
	ಬಿಡುಗಡೆ(filepath);
	fclose(fp);
	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ read_ಸತತ_init_context(ಅಕ್ಷರ **context)
{
	ಪೂರ್ಣಾಂಕ ret = -1;
	RC_STRINGLIST *list;
	ಅಕ್ಷರ *ಮೌಲ್ಯ = ಖಾಲಿ;

	list = rc_config_list(selinux_openrc_contexts_ಮಾರ್ಗ());
	ಆದರೆ (list == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ret;

	ಮೌಲ್ಯ = rc_config_ಮೌಲ್ಯ(list, "run_init");
	ಆದರೆ (ಮೌಲ್ಯ != ಖಾಲಿ && ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮೌಲ್ಯ) > 0) {
		*context = xstrdup(ಮೌಲ್ಯ);
		ret = 0;
	}

	rc_stringlist_free(list);
	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ selinux_ಸಿದ್ಧತೆ(ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ *new_context = ಖಾಲಿ;
	ಅಕ್ಷರ *curr_context = ಖಾಲಿ;
	context_t curr_con;
	ಅಕ್ಷರ *curr_t = ಖಾಲಿ;
	ಅಕ್ಷರ *ಸತತ_init_t = ಖಾಲಿ;

	/* Return, if selinux is disabled. */
	ಆದರೆ (is_selinux_enabled() < 1) {
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (read_ಸತತ_init_context(&ಸತತ_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		ಸತತ_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", ಸತತ_init_t);
	}

	/* Get our current context. */
	ಆದರೆ (getcon(&curr_context) < 0) {
		ಆದರೆ (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			ಹೋಗು ಹೊರಹೋಗು;
		} ಇಲ್ಲದಿದ್ದರೆ {
			perror("getcon");
			ನಿರ್ಗಮಿಸು(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	ಆದರೆ (!curr_con) {
		ಬಿಡುಗಡೆ(curr_context);
		ಹೋಗು ಹೊರಹೋಗು;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	ಆದರೆ (!curr_t) {
		context_free(curr_con);
		ಬಿಡುಗಡೆ(curr_context);
		ಹೋಗು ಹೊರಹೋಗು;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	ಬಿಡುಗಡೆ(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ಸತತ_init_t, curr_t, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಸತತ_init_t)) != 0) {
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಬಿಡುಗಡೆ(curr_t);
	ಬಿಡುಗಡೆ(ಸತತ_init_t);

	ಆದರೆ (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	ಆದರೆ (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	ಆದರೆ (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	ಬಿಡುಗಡೆ(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	ಆದರೆ (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		ಆದರೆ (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			ನಿರ್ಗಮಿಸು(-1);
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (execvp(argv[1], argv + 1)) {
		perror("execvp");
		ನಿರ್ಗಮಿಸು(-1);
	}

ಹೊರಹೋಗು:
	ಬಿಡುಗಡೆ(ಸತತ_init_t);
	ಬಿಡುಗಡೆ(curr_t);
}
