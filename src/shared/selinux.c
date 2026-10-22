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
#иначе
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#endif


/* The handle for the fcontext lookups */
static struct selabel_handle *hnd = NULL;

цяло selinux_util_label(const char *path)
{
	цяло retval = 0;
	цяло enforce;
	struct stat st;
	char *con;

	enforce = security_getenforce();
	ако (retval < 0)
		върни retval;

	ако (!hnd)
		върни (enforce) ? -1 : 0;

	retval = lstat(path, &st);
	ако (retval < 0) {
		ако (errno == ENOENT)
			върни 0;
		върни (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, path, st.st_mode);
	ако (retval < 0) {
		ако (errno == ENOENT)
			върни 0;
		върни (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(path, con);
	freecon(con);
	ако (retval < 0) {
		ако (errno == ENOENT)
			върни 0;
		ако (errno == ENOTSUP)
			върни 0;
		върни (enforce) ? -1 : 0;
	}

	върни 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
цяло selinux_util_open(void)
{
	цяло retval = 0;

	retval = is_selinux_enabled();
	ако (retval <= 0)
		върни retval;

	hnd = selabel_open(SELABEL_CTX_FILE, NULL, 0);
	ако (!hnd)
		върни -2;

	върни 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
цяло selinux_util_close(void)
{
	цяло retval = 0;

	retval = is_selinux_enabled();
	ако (retval <= 0)
		върни retval;

	ако (hnd) {
		selabel_close(hnd);
		hnd = NULL;
	}

	върни 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
static цяло check_password(char *username)
{
	цяло ret = 1;
#ifdef HAVE_PAM
	pam_handle_t *pamh;
	цяло pam_err = 0;
	const struct pam_conv pconv = {
		misc_conv,
		NULL
	};

	pam_err = pam_start(PAM_SERVICE_NAME, username, &pconv, &pamh);
	ако (pam_err != PAM_SUCCESS) {
		ret = -1;
		goto outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	ако (pam_err != PAM_SUCCESS) {
		ret = -1;
		goto outpam;
	}

	ret = 0;
outpam:
	pam_end(pamh, pam_err);
	pamh = NULL;

#иначе /* authenticating via /etc/shadow instead */
	struct spwd *spw;
	char *password;
	char *attempt;

	spw = getspnam(username);
	ако (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		goto outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	ако (!attempt) {
		ret = -1;
		goto outshadow;
	}

	ако (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		goto outshadow;
	}

	/* salt must be at least two characters long */
	ако (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		goto outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	ако (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	иначе
		ret = -1;
outshadow:
#endif
	върни ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
static цяло check_auth()
{
	struct passwd *pw;
	uid_t uid;

#ifdef HAVE_AUDIT
	uid = audit_getloginuid();
	ако (uid == (uid_t) -1)
		uid = getuid();
#иначе
	uid = getuid();
#endif

	pw = getpwuid(uid);
	ако (!pw) {
		eerror("cannot find your entry in the passwd file.");
		върни (-1);
	}

	printf("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	ако (check_password(pw->pw_name) == 0) {
		върни 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	върни 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
static цяло read_context_file(const char *filename, char **context)
{
	цяло ret = -1;
	FILE *fp;
	char *filepath = NULL;
	char *line = NULL;
	char *p;
	char *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_path(), filename);

	fp = fopen(filepath, "r");
	ако (fp == NULL) {
		eerror("Failed to open context file: %s", filename);
		free(filepath);
		върни -1;
	}

	докато (xgetline(&line, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = line;
		докато (isspace(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		ако (*p == '\0')
			continue;

		/* cut off spaces after the string */
		p2 = p;
		докато (!isspace(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		break;
	}

	free(line);
	free(filepath);
	fclose(fp);
	върни ret;
}

static цяло read_run_init_context(char **context)
{
	цяло ret = -1;
	RC_STRINGLIST *list;
	char *value = NULL;

	list = rc_config_list(selinux_openrc_contexts_path());
	ако (list == NULL)
		върни ret;

	value = rc_config_value(list, "run_init");
	ако (value != NULL && strlen(value) > 0) {
		*context = xstrdup(value);
		ret = 0;
	}

	rc_stringlist_free(list);
	върни ret;
}

void selinux_setup(char **argv)
{
	char *new_context = NULL;
	char *curr_context = NULL;
	context_t curr_con;
	char *curr_t = NULL;
	char *run_init_t = NULL;

	/* Return, if selinux is disabled. */
	ако (is_selinux_enabled() < 1) {
		върни;
	}

	ако (read_run_init_context(&run_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		run_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", run_init_t);
	}

	/* Get our current context. */
	ако (getcon(&curr_context) < 0) {
		ако (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			goto out;
		} иначе {
			perror("getcon");
			exit(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	ако (!curr_con) {
		free(curr_context);
		goto out;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	ако (!curr_t) {
		context_free(curr_con);
		free(curr_context);
		goto out;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	free(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	ако (strncmp(run_init_t, curr_t, strlen(run_init_t)) != 0) {
		goto out;
	}

	free(curr_t);
	free(run_init_t);

	ако (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	ако (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	ако (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	free(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	ако (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		ако (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			exit(-1);
		}
	} иначе ако (execvp(argv[1], argv + 1)) {
		perror("execvp");
		exit(-1);
	}

out:
	free(run_init_t);
	free(curr_t);
}
