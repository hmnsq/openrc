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

#అన్తర్భావ <stddef.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <dlfcn.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <pwd.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ <selinux/selinux.h>
#అన్తర్భావ <selinux/label.h>
#అన్తర్భావ <selinux/get_default_type.h>
#అన్తర్భావ <selinux/context.h>

#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/types.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "plugin.h"
#అన్తర్భావ "selinux.h"

/* the context files for selinux */
#నిర్వచించు INITRC_FILE "initrc_context"

#నిర్వచితమైతే HAVE_AUDIT
#అన్తర్భావ <libaudit.h>
#అయితే_ముగింపు

/* PAM or shadow for authentication */
#నిర్వచితమైతే HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#లేకపోతే
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#అయితే_ముగింపు


/* The handle for the fcontext lookups */
స్థిర నిర్మాణం selabel_handle *hnd = ఖాళీ;

పూర్ణాంకం selinux_util_label(స్థిరరాశి అక్షరం *మార్గం)
{
	పూర్ణాంకం retval = 0;
	పూర్ణాంకం enforce;
	నిర్మాణం stat st;
	అక్షరం *con;

	enforce = security_getenforce();
	ఒకవేళ (retval < 0)
		తిరిగి retval;

	ఒకవేళ (!hnd)
		తిరిగి (enforce) ? -1 : 0;

	retval = lstat(మార్గం, &st);
	ఒకవేళ (retval < 0) {
		ఒకవేళ (errno == ENOENT)
			తిరిగి 0;
		తిరిగి (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, మార్గం, st.st_mode);
	ఒకవేళ (retval < 0) {
		ఒకవేళ (errno == ENOENT)
			తిరిగి 0;
		తిరిగి (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(మార్గం, con);
	freecon(con);
	ఒకవేళ (retval < 0) {
		ఒకవేళ (errno == ENOENT)
			తిరిగి 0;
		ఒకవేళ (errno == ENOTSUP)
			తిరిగి 0;
		తిరిగి (enforce) ? -1 : 0;
	}

	తిరిగి 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
పూర్ణాంకం selinux_util_open(శూన్య)
{
	పూర్ణాంకం retval = 0;

	retval = is_selinux_enabled();
	ఒకవేళ (retval <= 0)
		తిరిగి retval;

	hnd = selabel_open(SELABEL_CTX_FILE, ఖాళీ, 0);
	ఒకవేళ (!hnd)
		తిరిగి -2;

	తిరిగి 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
పూర్ణాంకం selinux_util_close(శూన్య)
{
	పూర్ణాంకం retval = 0;

	retval = is_selinux_enabled();
	ఒకవేళ (retval <= 0)
		తిరిగి retval;

	ఒకవేళ (hnd) {
		selabel_close(hnd);
		hnd = ఖాళీ;
	}

	తిరిగి 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
స్థిర పూర్ణాంకం check_password(అక్షరం *username)
{
	పూర్ణాంకం ret = 1;
#నిర్వచితమైతే HAVE_PAM
	pam_handle_t *pamh;
	పూర్ణాంకం pam_err = 0;
	స్థిరరాశి నిర్మాణం pam_conv pconv = {
		misc_conv,
		ఖాళీ
	};

	pam_err = pam_ప్రారంభం(PAM_SERVICE_NAME, username, &pconv, &pamh);
	ఒకవేళ (pam_err != PAM_SUCCESS) {
		ret = -1;
		వెళ్ళు outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	ఒకవేళ (pam_err != PAM_SUCCESS) {
		ret = -1;
		వెళ్ళు outpam;
	}

	ret = 0;
outpam:
	pam_చివర(pamh, pam_err);
	pamh = ఖాళీ;

#లేకపోతే /* authenticating via /etc/shadow instead */
	నిర్మాణం spwd *spw;
	అక్షరం *password;
	అక్షరం *attempt;

	spw = getspnam(username);
	ఒకవేళ (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		వెళ్ళు outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	ఒకవేళ (!attempt) {
		ret = -1;
		వెళ్ళు outshadow;
	}

	ఒకవేళ (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		వెళ్ళు outshadow;
	}

	/* salt must be at least two characters long */
	ఒకవేళ (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		వెళ్ళు outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	ఒకవేళ (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	లేకపోతే
		ret = -1;
outshadow:
#అయితే_ముగింపు
	తిరిగి ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
స్థిర పూర్ణాంకం check_auth()
{
	నిర్మాణం passwd *pw;
	uid_t uid;

#నిర్వచితమైతే HAVE_AUDIT
	uid = audit_getloginuid();
	ఒకవేళ (uid == (uid_t) -1)
		uid = getuid();
#లేకపోతే
	uid = getuid();
#అయితే_ముగింపు

	pw = getpwuid(uid);
	ఒకవేళ (!pw) {
		eerror("cannot find your entry in the passwd file.");
		తిరిగి (-1);
	}

	రాయి("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	ఒకవేళ (check_password(pw->pw_name) == 0) {
		తిరిగి 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	తిరిగి 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
స్థిర పూర్ణాంకం read_context_file(స్థిరరాశి అక్షరం *filename, అక్షరం **context)
{
	పూర్ణాంకం ret = -1;
	FILE *fp;
	అక్షరం *filepath = ఖాళీ;
	అక్షరం *పంక్తి = ఖాళీ;
	అక్షరం *p;
	అక్షరం *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_మార్గం(), filename);

	fp = fopen(filepath, "r");
	ఒకవేళ (fp == ఖాళీ) {
		eerror("Failed to open context file: %s", filename);
		విడుదల(filepath);
		తిరిగి -1;
	}

	అయితే (xgetline(&పంక్తి, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = పంక్తి;
		అయితే (ఖాళీనా(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		ఒకవేళ (*p == '\0')
			కొనసాగు;

		/* cut off spaces after the string */
		p2 = p;
		అయితే (!ఖాళీనా(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		ఆపు;
	}

	విడుదల(పంక్తి);
	విడుదల(filepath);
	fclose(fp);
	తిరిగి ret;
}

స్థిర పూర్ణాంకం read_నిరంతరం_init_context(అక్షరం **context)
{
	పూర్ణాంకం ret = -1;
	RC_STRINGLIST *list;
	అక్షరం *విలువ = ఖాళీ;

	list = rc_config_list(selinux_openrc_contexts_మార్గం());
	ఒకవేళ (list == ఖాళీ)
		తిరిగి ret;

	విలువ = rc_config_విలువ(list, "run_init");
	ఒకవేళ (విలువ != ఖాళీ && అక్షరశ్రేణి_పొడవు(విలువ) > 0) {
		*context = xstrdup(విలువ);
		ret = 0;
	}

	rc_stringlist_free(list);
	తిరిగి ret;
}

శూన్య selinux_అమరిక(అక్షరం **argv)
{
	అక్షరం *new_context = ఖాళీ;
	అక్షరం *curr_context = ఖాళీ;
	context_t curr_con;
	అక్షరం *curr_t = ఖాళీ;
	అక్షరం *నిరంతరం_init_t = ఖాళీ;

	/* Return, if selinux is disabled. */
	ఒకవేళ (is_selinux_enabled() < 1) {
		తిరిగి;
	}

	ఒకవేళ (read_నిరంతరం_init_context(&నిరంతరం_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		నిరంతరం_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", నిరంతరం_init_t);
	}

	/* Get our current context. */
	ఒకవేళ (getcon(&curr_context) < 0) {
		ఒకవేళ (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			వెళ్ళు బహిర్గతం;
		} లేకపోతే {
			perror("getcon");
			నిష్క్రమించు(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	ఒకవేళ (!curr_con) {
		విడుదల(curr_context);
		వెళ్ళు బహిర్గతం;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	ఒకవేళ (!curr_t) {
		context_free(curr_con);
		విడుదల(curr_context);
		వెళ్ళు బహిర్గతం;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	విడుదల(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	ఒకవేళ (అక్షరశ్రేణి_పోలిక(నిరంతరం_init_t, curr_t, అక్షరశ్రేణి_పొడవు(నిరంతరం_init_t)) != 0) {
		వెళ్ళు బహిర్గతం;
	}

	విడుదల(curr_t);
	విడుదల(నిరంతరం_init_t);

	ఒకవేళ (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	ఒకవేళ (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	ఒకవేళ (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	విడుదల(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	ఒకవేళ (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		ఒకవేళ (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			నిష్క్రమించు(-1);
		}
	} లేకపోతే ఒకవేళ (execvp(argv[1], argv + 1)) {
		perror("execvp");
		నిష్క్రమించు(-1);
	}

బహిర్గతం:
	విడుదల(నిరంతరం_init_t);
	విడుదల(curr_t);
}
