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

#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dlfcn.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pwd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <selinux/selinux.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <selinux/label.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <selinux/get_default_type.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <selinux/context.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "plugin.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "selinux.h"

/* the context files for selinux */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITRC_FILE "initrc_context"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_AUDIT
#ਅਨ੍ਤਰ੍ਭਾਵ <libaudit.h>
#ਜੇ_ਅੰਤ

/* PAM or shadow for authentication */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#ਨਹੀਂਤਾਂ
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#ਜੇ_ਅੰਤ


/* The handle for the fcontext lookups */
ਸਥਿਰ ਬਣਤਰ selabel_handle *hnd = ਨਿਰਾ;

ਪੂਰਨ selinux_util_label(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਪੂਰਨ retval = 0;
	ਪੂਰਨ enforce;
	ਬਣਤਰ stat st;
	ਅੱਖਰ *con;

	enforce = security_getenforce();
	ਜੇ (retval < 0)
		ਵਾਪਸ retval;

	ਜੇ (!hnd)
		ਵਾਪਸ (enforce) ? -1 : 0;

	retval = lstat(ਰਸਤਾ, &st);
	ਜੇ (retval < 0) {
		ਜੇ (errno == ENOENT)
			ਵਾਪਸ 0;
		ਵਾਪਸ (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, ਰਸਤਾ, st.st_mode);
	ਜੇ (retval < 0) {
		ਜੇ (errno == ENOENT)
			ਵਾਪਸ 0;
		ਵਾਪਸ (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(ਰਸਤਾ, con);
	freecon(con);
	ਜੇ (retval < 0) {
		ਜੇ (errno == ENOENT)
			ਵਾਪਸ 0;
		ਜੇ (errno == ENOTSUP)
			ਵਾਪਸ 0;
		ਵਾਪਸ (enforce) ? -1 : 0;
	}

	ਵਾਪਸ 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
ਪੂਰਨ selinux_util_open(ਖਾਲੀ)
{
	ਪੂਰਨ retval = 0;

	retval = is_selinux_enabled();
	ਜੇ (retval <= 0)
		ਵਾਪਸ retval;

	hnd = selabel_open(SELABEL_CTX_FILE, ਨਿਰਾ, 0);
	ਜੇ (!hnd)
		ਵਾਪਸ -2;

	ਵਾਪਸ 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
ਪੂਰਨ selinux_util_close(ਖਾਲੀ)
{
	ਪੂਰਨ retval = 0;

	retval = is_selinux_enabled();
	ਜੇ (retval <= 0)
		ਵਾਪਸ retval;

	ਜੇ (hnd) {
		selabel_close(hnd);
		hnd = ਨਿਰਾ;
	}

	ਵਾਪਸ 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
ਸਥਿਰ ਪੂਰਨ check_password(ਅੱਖਰ *username)
{
	ਪੂਰਨ ret = 1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
	pam_handle_t *pamh;
	ਪੂਰਨ pam_err = 0;
	ਅਚਲ ਬਣਤਰ pam_conv pconv = {
		misc_conv,
		ਨਿਰਾ
	};

	pam_err = pam_ਸ਼ੁਰੂ(PAM_SERVICE_NAME, username, &pconv, &pamh);
	ਜੇ (pam_err != PAM_SUCCESS) {
		ret = -1;
		ਜਾਓ outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	ਜੇ (pam_err != PAM_SUCCESS) {
		ret = -1;
		ਜਾਓ outpam;
	}

	ret = 0;
outpam:
	pam_ਸਿਰਾ(pamh, pam_err);
	pamh = ਨਿਰਾ;

#ਨਹੀਂਤਾਂ /* authenticating via /etc/shadow instead */
	ਬਣਤਰ spwd *spw;
	ਅੱਖਰ *password;
	ਅੱਖਰ *attempt;

	spw = getspnam(username);
	ਜੇ (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		ਜਾਓ outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	ਜੇ (!attempt) {
		ret = -1;
		ਜਾਓ outshadow;
	}

	ਜੇ (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		ਜਾਓ outshadow;
	}

	/* salt must be at least two characters long */
	ਜੇ (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		ਜਾਓ outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	ਜੇ (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	ਨਹੀਂਤਾਂ
		ret = -1;
outshadow:
#ਜੇ_ਅੰਤ
	ਵਾਪਸ ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
ਸਥਿਰ ਪੂਰਨ check_auth()
{
	ਬਣਤਰ passwd *pw;
	uid_t uid;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_AUDIT
	uid = audit_getloginuid();
	ਜੇ (uid == (uid_t) -1)
		uid = getuid();
#ਨਹੀਂਤਾਂ
	uid = getuid();
#ਜੇ_ਅੰਤ

	pw = getpwuid(uid);
	ਜੇ (!pw) {
		eerror("cannot find your entry in the passwd file.");
		ਵਾਪਸ (-1);
	}

	ਲਿਖੋ("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	ਜੇ (check_password(pw->pw_name) == 0) {
		ਵਾਪਸ 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	ਵਾਪਸ 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
ਸਥਿਰ ਪੂਰਨ read_context_file(ਅਚਲ ਅੱਖਰ *filename, ਅੱਖਰ **context)
{
	ਪੂਰਨ ret = -1;
	FILE *fp;
	ਅੱਖਰ *filepath = ਨਿਰਾ;
	ਅੱਖਰ *ਸਤਰ = ਨਿਰਾ;
	ਅੱਖਰ *p;
	ਅੱਖਰ *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_ਰਸਤਾ(), filename);

	fp = fopen(filepath, "r");
	ਜੇ (fp == ਨਿਰਾ) {
		eerror("Failed to open context file: %s", filename);
		ਮੁਕਤ(filepath);
		ਵਾਪਸ -1;
	}

	ਜਦੋਂ (xgetline(&ਸਤਰ, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = ਸਤਰ;
		ਜਦੋਂ (ਖਾਲੀ_ਥਾਂ_ਹੈ(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		ਜੇ (*p == '\0')
			ਜਾਰੀ;

		/* cut off spaces after the string */
		p2 = p;
		ਜਦੋਂ (!ਖਾਲੀ_ਥਾਂ_ਹੈ(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		ਰੋਕੋ;
	}

	ਮੁਕਤ(ਸਤਰ);
	ਮੁਕਤ(filepath);
	fclose(fp);
	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ read_ਲੜੀਵਾਰ_init_context(ਅੱਖਰ **context)
{
	ਪੂਰਨ ret = -1;
	RC_STRINGLIST *list;
	ਅੱਖਰ *ਮੁੱਲ = ਨਿਰਾ;

	list = rc_config_list(selinux_openrc_contexts_ਰਸਤਾ());
	ਜੇ (list == ਨਿਰਾ)
		ਵਾਪਸ ret;

	ਮੁੱਲ = rc_config_ਮੁੱਲ(list, "run_init");
	ਜੇ (ਮੁੱਲ != ਨਿਰਾ && ਲੜੀ_ਲੰਬਾਈ(ਮੁੱਲ) > 0) {
		*context = xstrdup(ਮੁੱਲ);
		ret = 0;
	}

	rc_stringlist_free(list);
	ਵਾਪਸ ret;
}

ਖਾਲੀ selinux_ਤਿਆਰੀ(ਅੱਖਰ **argv)
{
	ਅੱਖਰ *new_context = ਨਿਰਾ;
	ਅੱਖਰ *curr_context = ਨਿਰਾ;
	context_t curr_con;
	ਅੱਖਰ *curr_t = ਨਿਰਾ;
	ਅੱਖਰ *ਲੜੀਵਾਰ_init_t = ਨਿਰਾ;

	/* Return, if selinux is disabled. */
	ਜੇ (is_selinux_enabled() < 1) {
		ਵਾਪਸ;
	}

	ਜੇ (read_ਲੜੀਵਾਰ_init_context(&ਲੜੀਵਾਰ_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		ਲੜੀਵਾਰ_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", ਲੜੀਵਾਰ_init_t);
	}

	/* Get our current context. */
	ਜੇ (getcon(&curr_context) < 0) {
		ਜੇ (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			ਜਾਓ ਬਾਹਰਲਾ;
		} ਨਹੀਂਤਾਂ {
			perror("getcon");
			ਬਾਹਰ_ਨਿਕਲੋ(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	ਜੇ (!curr_con) {
		ਮੁਕਤ(curr_context);
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	ਜੇ (!curr_t) {
		context_free(curr_con);
		ਮੁਕਤ(curr_context);
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	ਮੁਕਤ(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	ਜੇ (ਲੜੀ_ਤੁਲਨਾ(ਲੜੀਵਾਰ_init_t, curr_t, ਲੜੀ_ਲੰਬਾਈ(ਲੜੀਵਾਰ_init_t)) != 0) {
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਮੁਕਤ(curr_t);
	ਮੁਕਤ(ਲੜੀਵਾਰ_init_t);

	ਜੇ (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	ਜੇ (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	ਜੇ (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	ਮੁਕਤ(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	ਜੇ (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		ਜੇ (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			ਬਾਹਰ_ਨਿਕਲੋ(-1);
		}
	} ਨਹੀਂਤਾਂ ਜੇ (execvp(argv[1], argv + 1)) {
		perror("execvp");
		ਬਾਹਰ_ਨਿਕਲੋ(-1);
	}

ਬਾਹਰਲਾ:
	ਮੁਕਤ(ਲੜੀਵਾਰ_init_t);
	ਮੁਕਤ(curr_t);
}
