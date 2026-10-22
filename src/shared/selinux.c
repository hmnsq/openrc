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

#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <dlfcn.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <pwd.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব <selinux/selinux.h>
#অন্তর্ভাব <selinux/label.h>
#অন্তর্ভাব <selinux/get_default_type.h>
#অন্তর্ভাব <selinux/context.h>

#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/types.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "misc.h"
#অন্তর্ভাব "plugin.h"
#অন্তর্ভাব "selinux.h"

/* the context files for selinux */
#define INITRC_FILE "initrc_context"

#ifdef HAVE_AUDIT
#অন্তর্ভাব <libaudit.h>
#endif

/* PAM or shadow for authentication */
#ifdef HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#অন্যথা
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#endif


/* The handle for the fcontext lookups */
স্থির সংরচনা selabel_handle *hnd = রিক্ত;

পূর্ণাঙ্ক selinux_util_label(ধ্রুব বর্ণ *path)
{
	পূর্ণাঙ্ক retval = 0;
	পূর্ণাঙ্ক enforce;
	সংরচনা stat st;
	বর্ণ *con;

	enforce = security_getenforce();
	যদি (retval < 0)
		প্রত্যাগম retval;

	যদি (!hnd)
		প্রত্যাগম (enforce) ? -1 : 0;

	retval = lstat(path, &st);
	যদি (retval < 0) {
		যদি (errno == ENOENT)
			প্রত্যাগম 0;
		প্রত্যাগম (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, path, st.st_mode);
	যদি (retval < 0) {
		যদি (errno == ENOENT)
			প্রত্যাগম 0;
		প্রত্যাগম (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(path, con);
	freecon(con);
	যদি (retval < 0) {
		যদি (errno == ENOENT)
			প্রত্যাগম 0;
		যদি (errno == ENOTSUP)
			প্রত্যাগম 0;
		প্রত্যাগম (enforce) ? -1 : 0;
	}

	প্রত্যাগম 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
পূর্ণাঙ্ক selinux_util_open(শূন্য)
{
	পূর্ণাঙ্ক retval = 0;

	retval = is_selinux_enabled();
	যদি (retval <= 0)
		প্রত্যাগম retval;

	hnd = selabel_open(SELABEL_CTX_FILE, রিক্ত, 0);
	যদি (!hnd)
		প্রত্যাগম -2;

	প্রত্যাগম 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
পূর্ণাঙ্ক selinux_util_close(শূন্য)
{
	পূর্ণাঙ্ক retval = 0;

	retval = is_selinux_enabled();
	যদি (retval <= 0)
		প্রত্যাগম retval;

	যদি (hnd) {
		selabel_close(hnd);
		hnd = রিক্ত;
	}

	প্রত্যাগম 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
স্থির পূর্ণাঙ্ক check_password(বর্ণ *username)
{
	পূর্ণাঙ্ক ret = 1;
#ifdef HAVE_PAM
	pam_handle_t *pamh;
	পূর্ণাঙ্ক pam_err = 0;
	ধ্রুব সংরচনা pam_conv pconv = {
		misc_conv,
		রিক্ত
	};

	pam_err = pam_start(PAM_SERVICE_NAME, username, &pconv, &pamh);
	যদি (pam_err != PAM_SUCCESS) {
		ret = -1;
		গচ্ছ outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	যদি (pam_err != PAM_SUCCESS) {
		ret = -1;
		গচ্ছ outpam;
	}

	ret = 0;
outpam:
	pam_end(pamh, pam_err);
	pamh = রিক্ত;

#অন্যথা /* authenticating via /etc/shadow instead */
	সংরচনা spwd *spw;
	বর্ণ *password;
	বর্ণ *attempt;

	spw = getspnam(username);
	যদি (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		গচ্ছ outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	যদি (!attempt) {
		ret = -1;
		গচ্ছ outshadow;
	}

	যদি (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		গচ্ছ outshadow;
	}

	/* salt must be at least two characters long */
	যদি (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		গচ্ছ outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	যদি (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	অন্যথা
		ret = -1;
outshadow:
#endif
	প্রত্যাগম ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
স্থির পূর্ণাঙ্ক check_auth()
{
	সংরচনা passwd *pw;
	uid_t uid;

#ifdef HAVE_AUDIT
	uid = audit_getloginuid();
	যদি (uid == (uid_t) -1)
		uid = getuid();
#অন্যথা
	uid = getuid();
#endif

	pw = getpwuid(uid);
	যদি (!pw) {
		eerror("cannot find your entry in the passwd file.");
		প্রত্যাগম (-1);
	}

	printf("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	যদি (check_password(pw->pw_name) == 0) {
		প্রত্যাগম 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	প্রত্যাগম 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
স্থির পূর্ণাঙ্ক read_context_file(ধ্রুব বর্ণ *filename, বর্ণ **context)
{
	পূর্ণাঙ্ক ret = -1;
	FILE *fp;
	বর্ণ *filepath = রিক্ত;
	বর্ণ *সারি = রিক্ত;
	বর্ণ *p;
	বর্ণ *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_path(), filename);

	fp = fopen(filepath, "r");
	যদি (fp == রিক্ত) {
		eerror("Failed to open context file: %s", filename);
		মুক্ত(filepath);
		প্রত্যাগম -1;
	}

	যাবত্ (xgetline(&সারি, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = সারি;
		যাবত্ (isspace(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		যদি (*p == '\0')
			অনুবর্ত;

		/* cut off spaces after the string */
		p2 = p;
		যাবত্ (!isspace(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		ভঙ্গ;
	}

	মুক্ত(সারি);
	মুক্ত(filepath);
	fclose(fp);
	প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক read_run_init_context(বর্ণ **context)
{
	পূর্ণাঙ্ক ret = -1;
	RC_STRINGLIST *list;
	বর্ণ *মান = রিক্ত;

	list = rc_config_list(selinux_openrc_contexts_path());
	যদি (list == রিক্ত)
		প্রত্যাগম ret;

	মান = rc_config_মান(list, "run_init");
	যদি (মান != রিক্ত && strlen(মান) > 0) {
		*context = xstrdup(মান);
		ret = 0;
	}

	rc_stringlist_free(list);
	প্রত্যাগম ret;
}

শূন্য selinux_setup(বর্ণ **argv)
{
	বর্ণ *new_context = রিক্ত;
	বর্ণ *curr_context = রিক্ত;
	context_t curr_con;
	বর্ণ *curr_t = রিক্ত;
	বর্ণ *run_init_t = রিক্ত;

	/* Return, if selinux is disabled. */
	যদি (is_selinux_enabled() < 1) {
		প্রত্যাগম;
	}

	যদি (read_run_init_context(&run_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		run_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", run_init_t);
	}

	/* Get our current context. */
	যদি (getcon(&curr_context) < 0) {
		যদি (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			গচ্ছ out;
		} অন্যথা {
			perror("getcon");
			exit(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	যদি (!curr_con) {
		মুক্ত(curr_context);
		গচ্ছ out;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	যদি (!curr_t) {
		context_free(curr_con);
		মুক্ত(curr_context);
		গচ্ছ out;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	মুক্ত(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	যদি (strncmp(run_init_t, curr_t, strlen(run_init_t)) != 0) {
		গচ্ছ out;
	}

	মুক্ত(curr_t);
	মুক্ত(run_init_t);

	যদি (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	যদি (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	যদি (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	মুক্ত(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	যদি (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		যদি (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			exit(-1);
		}
	} অন্যথা যদি (execvp(argv[1], argv + 1)) {
		perror("execvp");
		exit(-1);
	}

out:
	মুক্ত(run_init_t);
	মুক্ত(curr_t);
}
