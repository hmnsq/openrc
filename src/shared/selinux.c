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

#include <stddef.h>
#include <errno.h>
#include <dlfcn.h>
#include <ctype.h>
#include <limits.h>
#include <pwd.h>
#include <unistd.h>

#include <selinux/selinux.h>
#include <selinux/label.h>
#include <selinux/get_default_type.h>
#include <selinux/context.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "misc.h"
#include "plugin.h"
#include "selinux.h"

/* the context files for selinux */
#define INITRC_FILE "initrc_context"

#ifdef HAVE_AUDIT
#include <libaudit.h>
#endif

/* PAM or shadow for authentication */
#ifdef HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#ᬮᬶᬬ
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#endif


/* The handle for the fcontext lookups */
static sa_nracanaa selabel_handle *hnd = NULL;

puur_naa_nka selinux_util_label(const var_na *path)
{
	puur_naa_nka retval = 0;
	puur_naa_nka enforce;
	sa_nracanaa stat st;
	var_na *con;

	enforce = security_getenforce();
	ᬬᬾᬦ᭄ (retval < 0)
		ᬯᬮᬶ retval;

	ᬬᬾᬦ᭄ (!hnd)
		ᬯᬮᬶ (enforce) ? -1 : 0;

	retval = lstat(path, &st);
	ᬬᬾᬦ᭄ (retval < 0) {
		ᬬᬾᬦ᭄ (errno == ENOENT)
			ᬯᬮᬶ 0;
		ᬯᬮᬶ (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, path, st.st_mode);
	ᬬᬾᬦ᭄ (retval < 0) {
		ᬬᬾᬦ᭄ (errno == ENOENT)
			ᬯᬮᬶ 0;
		ᬯᬮᬶ (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(path, con);
	freecon(con);
	ᬬᬾᬦ᭄ (retval < 0) {
		ᬬᬾᬦ᭄ (errno == ENOENT)
			ᬯᬮᬶ 0;
		ᬬᬾᬦ᭄ (errno == ENOTSUP)
			ᬯᬮᬶ 0;
		ᬯᬮᬶ (enforce) ? -1 : 0;
	}

	ᬯᬮᬶ 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
puur_naa_nka selinux_util_open(void)
{
	puur_naa_nka retval = 0;

	retval = is_selinux_enabled();
	ᬬᬾᬦ᭄ (retval <= 0)
		ᬯᬮᬶ retval;

	hnd = selabel_open(SELABEL_CTX_FILE, NULL, 0);
	ᬬᬾᬦ᭄ (!hnd)
		ᬯᬮᬶ -2;

	ᬯᬮᬶ 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
puur_naa_nka selinux_util_close(void)
{
	puur_naa_nka retval = 0;

	retval = is_selinux_enabled();
	ᬬᬾᬦ᭄ (retval <= 0)
		ᬯᬮᬶ retval;

	ᬬᬾᬦ᭄ (hnd) {
		selabel_close(hnd);
		hnd = NULL;
	}

	ᬯᬮᬶ 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
static puur_naa_nka check_password(var_na *username)
{
	puur_naa_nka ret = 1;
#ifdef HAVE_PAM
	pam_handle_t *pamh;
	puur_naa_nka pam_err = 0;
	const sa_nracanaa pam_conv pconv = {
		misc_conv,
		NULL
	};

	pam_err = pam_start(PAM_SERVICE_NAME, username, &pconv, &pamh);
	ᬬᬾᬦ᭄ (pam_err != PAM_SUCCESS) {
		ret = -1;
		goto outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	ᬬᬾᬦ᭄ (pam_err != PAM_SUCCESS) {
		ret = -1;
		goto outpam;
	}

	ret = 0;
outpam:
	pam_end(pamh, pam_err);
	pamh = NULL;

#ᬮᬶᬬ /* authenticating via /etc/shadow instead */
	sa_nracanaa spwd *spw;
	var_na *password;
	var_na *attempt;

	spw = getspnam(username);
	ᬬᬾᬦ᭄ (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		goto outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	ᬬᬾᬦ᭄ (!attempt) {
		ret = -1;
		goto outshadow;
	}

	ᬬᬾᬦ᭄ (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		goto outshadow;
	}

	/* salt must be at least two characters long */
	ᬬᬾᬦ᭄ (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		goto outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	ᬬᬾᬦ᭄ (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	ᬮᬶᬬ
		ret = -1;
outshadow:
#endif
	ᬯᬮᬶ ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
static puur_naa_nka check_auth()
{
	sa_nracanaa passwd *pw;
	uid_t uid;

#ifdef HAVE_AUDIT
	uid = audit_getloginuid();
	ᬬᬾᬦ᭄ (uid == (uid_t) -1)
		uid = getuid();
#ᬮᬶᬬ
	uid = getuid();
#endif

	pw = getpwuid(uid);
	ᬬᬾᬦ᭄ (!pw) {
		eerror("cannot find your entry in the passwd file.");
		ᬯᬮᬶ (-1);
	}

	printf("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	ᬬᬾᬦ᭄ (check_password(pw->pw_name) == 0) {
		ᬯᬮᬶ 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	ᬯᬮᬶ 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
static puur_naa_nka read_context_file(const var_na *filename, var_na **context)
{
	puur_naa_nka ret = -1;
	FILE *fp;
	var_na *filepath = NULL;
	var_na *line = NULL;
	var_na *p;
	var_na *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_path(), filename);

	fp = fopen(filepath, "r");
	ᬬᬾᬦ᭄ (fp == NULL) {
		eerror("Failed to open context file: %s", filename);
		free(filepath);
		ᬯᬮᬶ -1;
	}

	ᬲᭂᬤᭂᬮ (xgetline(&line, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = line;
		ᬲᭂᬤᭂᬮ (isspace(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		ᬬᬾᬦ᭄ (*p == '\0')
			continue;

		/* cut off spaces after the string */
		p2 = p;
		ᬲᭂᬤᭂᬮ (!isspace(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		bha_nga;
	}

	free(line);
	free(filepath);
	fclose(fp);
	ᬯᬮᬶ ret;
}

static puur_naa_nka read_run_init_context(var_na **context)
{
	puur_naa_nka ret = -1;
	RC_STRINGLIST *list;
	var_na *value = NULL;

	list = rc_config_list(selinux_openrc_contexts_path());
	ᬬᬾᬦ᭄ (list == NULL)
		ᬯᬮᬶ ret;

	value = rc_config_value(list, "run_init");
	ᬬᬾᬦ᭄ (value != NULL && strlen(value) > 0) {
		*context = xstrdup(value);
		ret = 0;
	}

	rc_stringlist_free(list);
	ᬯᬮᬶ ret;
}

void selinux_setup(var_na **argv)
{
	var_na *new_context = NULL;
	var_na *curr_context = NULL;
	context_t curr_con;
	var_na *curr_t = NULL;
	var_na *run_init_t = NULL;

	/* Return, if selinux is disabled. */
	ᬬᬾᬦ᭄ (is_selinux_enabled() < 1) {
		ᬯᬮᬶ;
	}

	ᬬᬾᬦ᭄ (read_run_init_context(&run_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		run_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", run_init_t);
	}

	/* Get our current context. */
	ᬬᬾᬦ᭄ (getcon(&curr_context) < 0) {
		ᬬᬾᬦ᭄ (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			goto out;
		} ᬮᬶᬬ {
			perror("getcon");
			exit(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	ᬬᬾᬦ᭄ (!curr_con) {
		free(curr_context);
		goto out;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	ᬬᬾᬦ᭄ (!curr_t) {
		context_free(curr_con);
		free(curr_context);
		goto out;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	free(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	ᬬᬾᬦ᭄ (strncmp(run_init_t, curr_t, strlen(run_init_t)) != 0) {
		goto out;
	}

	free(curr_t);
	free(run_init_t);

	ᬬᬾᬦ᭄ (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	ᬬᬾᬦ᭄ (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	ᬬᬾᬦ᭄ (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	free(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	ᬬᬾᬦ᭄ (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		ᬬᬾᬦ᭄ (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			exit(-1);
		}
	} ᬮᬶᬬ ᬬᬾᬦ᭄ (execvp(argv[1], argv + 1)) {
		perror("execvp");
		exit(-1);
	}

out:
	free(run_init_t);
	free(curr_t);
}
