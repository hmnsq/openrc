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

#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <dlfcn.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <pwd.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ <selinux/selinux.h>
#அந்தர்பாவ <selinux/label.h>
#அந்தர்பாவ <selinux/get_default_type.h>
#அந்தர்பாவ <selinux/context.h>

#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/types.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "plugin.h"
#அந்தர்பாவ "selinux.h"

/* the context files for selinux */
#define INITRC_FILE "initrc_context"

#ifdef HAVE_AUDIT
#அந்தர்பாவ <libaudit.h>
#endif

/* PAM or shadow for authentication */
#ifdef HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#இல்லையெனில்
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#endif


/* The handle for the fcontext lookups */
நிலையான கட்டமைப்பு selabel_handle *hnd = வெற்று;

முழு selinux_util_label(மாறிலி எழுத்து *path)
{
	முழு retval = 0;
	முழு enforce;
	கட்டமைப்பு stat st;
	எழுத்து *con;

	enforce = security_getenforce();
	எனில் (retval < 0)
		திரும்பு retval;

	எனில் (!hnd)
		திரும்பு (enforce) ? -1 : 0;

	retval = lstat(path, &st);
	எனில் (retval < 0) {
		எனில் (errno == ENOENT)
			திரும்பு 0;
		திரும்பு (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, path, st.st_mode);
	எனில் (retval < 0) {
		எனில் (errno == ENOENT)
			திரும்பு 0;
		திரும்பு (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(path, con);
	freecon(con);
	எனில் (retval < 0) {
		எனில் (errno == ENOENT)
			திரும்பு 0;
		எனில் (errno == ENOTSUP)
			திரும்பு 0;
		திரும்பு (enforce) ? -1 : 0;
	}

	திரும்பு 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
முழு selinux_util_open(வெறுமை)
{
	முழு retval = 0;

	retval = is_selinux_enabled();
	எனில் (retval <= 0)
		திரும்பு retval;

	hnd = selabel_open(SELABEL_CTX_FILE, வெற்று, 0);
	எனில் (!hnd)
		திரும்பு -2;

	திரும்பு 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
முழு selinux_util_close(வெறுமை)
{
	முழு retval = 0;

	retval = is_selinux_enabled();
	எனில் (retval <= 0)
		திரும்பு retval;

	எனில் (hnd) {
		selabel_close(hnd);
		hnd = வெற்று;
	}

	திரும்பு 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
நிலையான முழு check_password(எழுத்து *username)
{
	முழு ret = 1;
#ifdef HAVE_PAM
	pam_handle_t *pamh;
	முழு pam_err = 0;
	மாறிலி கட்டமைப்பு pam_conv pconv = {
		misc_conv,
		வெற்று
	};

	pam_err = pam_start(PAM_SERVICE_NAME, username, &pconv, &pamh);
	எனில் (pam_err != PAM_SUCCESS) {
		ret = -1;
		செல் outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	எனில் (pam_err != PAM_SUCCESS) {
		ret = -1;
		செல் outpam;
	}

	ret = 0;
outpam:
	pam_end(pamh, pam_err);
	pamh = வெற்று;

#இல்லையெனில் /* authenticating via /etc/shadow instead */
	கட்டமைப்பு spwd *spw;
	எழுத்து *password;
	எழுத்து *attempt;

	spw = getspnam(username);
	எனில் (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		செல் outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	எனில் (!attempt) {
		ret = -1;
		செல் outshadow;
	}

	எனில் (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		செல் outshadow;
	}

	/* salt must be at least two characters long */
	எனில் (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		செல் outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	எனில் (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	இல்லையெனில்
		ret = -1;
outshadow:
#endif
	திரும்பு ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
நிலையான முழு check_auth()
{
	கட்டமைப்பு passwd *pw;
	uid_t uid;

#ifdef HAVE_AUDIT
	uid = audit_getloginuid();
	எனில் (uid == (uid_t) -1)
		uid = getuid();
#இல்லையெனில்
	uid = getuid();
#endif

	pw = getpwuid(uid);
	எனில் (!pw) {
		eerror("cannot find your entry in the passwd file.");
		திரும்பு (-1);
	}

	printf("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	எனில் (check_password(pw->pw_name) == 0) {
		திரும்பு 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	திரும்பு 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
நிலையான முழு read_context_file(மாறிலி எழுத்து *filename, எழுத்து **context)
{
	முழு ret = -1;
	FILE *fp;
	எழுத்து *filepath = வெற்று;
	எழுத்து *வரி = வெற்று;
	எழுத்து *p;
	எழுத்து *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_path(), filename);

	fp = fopen(filepath, "r");
	எனில் (fp == வெற்று) {
		eerror("Failed to open context file: %s", filename);
		விடுவி(filepath);
		திரும்பு -1;
	}

	போது (xgetline(&வரி, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = வரி;
		போது (isspace(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		எனில் (*p == '\0')
			தொடர்;

		/* cut off spaces after the string */
		p2 = p;
		போது (!isspace(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		நிறுத்து;
	}

	விடுவி(வரி);
	விடுவி(filepath);
	fclose(fp);
	திரும்பு ret;
}

நிலையான முழு read_run_init_context(எழுத்து **context)
{
	முழு ret = -1;
	RC_STRINGLIST *list;
	எழுத்து *மதிப்பு = வெற்று;

	list = rc_config_list(selinux_openrc_contexts_path());
	எனில் (list == வெற்று)
		திரும்பு ret;

	மதிப்பு = rc_config_மதிப்பு(list, "run_init");
	எனில் (மதிப்பு != வெற்று && strlen(மதிப்பு) > 0) {
		*context = xstrdup(மதிப்பு);
		ret = 0;
	}

	rc_stringlist_free(list);
	திரும்பு ret;
}

வெறுமை selinux_setup(எழுத்து **argv)
{
	எழுத்து *new_context = வெற்று;
	எழுத்து *curr_context = வெற்று;
	context_t curr_con;
	எழுத்து *curr_t = வெற்று;
	எழுத்து *run_init_t = வெற்று;

	/* Return, if selinux is disabled. */
	எனில் (is_selinux_enabled() < 1) {
		திரும்பு;
	}

	எனில் (read_run_init_context(&run_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		run_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", run_init_t);
	}

	/* Get our current context. */
	எனில் (getcon(&curr_context) < 0) {
		எனில் (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			செல் out;
		} இல்லையெனில் {
			perror("getcon");
			exit(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	எனில் (!curr_con) {
		விடுவி(curr_context);
		செல் out;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	எனில் (!curr_t) {
		context_free(curr_con);
		விடுவி(curr_context);
		செல் out;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	விடுவி(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	எனில் (strncmp(run_init_t, curr_t, strlen(run_init_t)) != 0) {
		செல் out;
	}

	விடுவி(curr_t);
	விடுவி(run_init_t);

	எனில் (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	எனில் (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	எனில் (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	விடுவி(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	எனில் (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		எனில் (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			exit(-1);
		}
	} இல்லையெனில் எனில் (execvp(argv[1], argv + 1)) {
		perror("execvp");
		exit(-1);
	}

out:
	விடுவி(run_init_t);
	விடுவி(curr_t);
}
