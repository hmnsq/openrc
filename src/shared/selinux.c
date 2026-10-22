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

#ضمن <stddef.h>
#ضمن <errno.h>
#ضمن <dlfcn.h>
#ضمن <ctype.h>
#ضمن <limits.h>
#ضمن <pwd.h>
#ضمن <unistd.h>

#ضمن <selinux/selinux.h>
#ضمن <selinux/label.h>
#ضمن <selinux/get_default_type.h>
#ضمن <selinux/context.h>

#ضمن <sys/stat.h>
#ضمن <sys/types.h>

#ضمن "einfo.h"
#ضمن "queue.h"
#ضمن "rc.h"
#ضمن "misc.h"
#ضمن "plugin.h"
#ضمن "selinux.h"

/* the context files for selinux */
#define INITRC_FILE "initrc_context"

#ifdef HAVE_AUDIT
#ضمن <libaudit.h>
#endif

/* PAM or shadow for authentication */
#ifdef HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#ܐܚܪܢܐ
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#endif


/* The handle for the fcontext lookups */
ساكن بنية selabel_handle *hnd = عدم;

صحيح selinux_util_label(ثابت حرف *path)
{
	صحيح retval = 0;
	صحيح enforce;
	بنية stat st;
	حرف *con;

	enforce = security_getenforce();
	ܐܢ (retval < 0)
		ܦܢܐ retval;

	ܐܢ (!hnd)
		ܦܢܐ (enforce) ? -1 : 0;

	retval = lstat(path, &st);
	ܐܢ (retval < 0) {
		ܐܢ (errno == ENOENT)
			ܦܢܐ 0;
		ܦܢܐ (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, path, st.st_mode);
	ܐܢ (retval < 0) {
		ܐܢ (errno == ENOENT)
			ܦܢܐ 0;
		ܦܢܐ (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(path, con);
	freecon(con);
	ܐܢ (retval < 0) {
		ܐܢ (errno == ENOENT)
			ܦܢܐ 0;
		ܐܢ (errno == ENOTSUP)
			ܦܢܐ 0;
		ܦܢܐ (enforce) ? -1 : 0;
	}

	ܦܢܐ 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
صحيح selinux_util_open(فراغ)
{
	صحيح retval = 0;

	retval = is_selinux_enabled();
	ܐܢ (retval <= 0)
		ܦܢܐ retval;

	hnd = selabel_open(SELABEL_CTX_FILE, عدم, 0);
	ܐܢ (!hnd)
		ܦܢܐ -2;

	ܦܢܐ 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
صحيح selinux_util_close(فراغ)
{
	صحيح retval = 0;

	retval = is_selinux_enabled();
	ܐܢ (retval <= 0)
		ܦܢܐ retval;

	ܐܢ (hnd) {
		selabel_close(hnd);
		hnd = عدم;
	}

	ܦܢܐ 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
ساكن صحيح check_password(حرف *username)
{
	صحيح ret = 1;
#ifdef HAVE_PAM
	pam_handle_t *pamh;
	صحيح pam_err = 0;
	ثابت بنية pam_conv pconv = {
		misc_conv,
		عدم
	};

	pam_err = pam_start(PAM_SERVICE_NAME, username, &pconv, &pamh);
	ܐܢ (pam_err != PAM_SUCCESS) {
		ret = -1;
		اذهب outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	ܐܢ (pam_err != PAM_SUCCESS) {
		ret = -1;
		اذهب outpam;
	}

	ret = 0;
outpam:
	pam_end(pamh, pam_err);
	pamh = عدم;

#ܐܚܪܢܐ /* authenticating via /etc/shadow instead */
	بنية spwd *spw;
	حرف *password;
	حرف *attempt;

	spw = getspnam(username);
	ܐܢ (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		اذهب outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	ܐܢ (!attempt) {
		ret = -1;
		اذهب outshadow;
	}

	ܐܢ (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		اذهب outshadow;
	}

	/* salt must be at least two characters long */
	ܐܢ (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		اذهب outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	ܐܢ (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	ܐܚܪܢܐ
		ret = -1;
outshadow:
#endif
	ܦܢܐ ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
ساكن صحيح check_auth()
{
	بنية passwd *pw;
	uid_t uid;

#ifdef HAVE_AUDIT
	uid = audit_getloginuid();
	ܐܢ (uid == (uid_t) -1)
		uid = getuid();
#ܐܚܪܢܐ
	uid = getuid();
#endif

	pw = getpwuid(uid);
	ܐܢ (!pw) {
		eerror("cannot find your entry in the passwd file.");
		ܦܢܐ (-1);
	}

	printf("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	ܐܢ (check_password(pw->pw_name) == 0) {
		ܦܢܐ 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	ܦܢܐ 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
ساكن صحيح read_context_file(ثابت حرف *filename, حرف **context)
{
	صحيح ret = -1;
	FILE *fp;
	حرف *filepath = عدم;
	حرف *line = عدم;
	حرف *p;
	حرف *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_path(), filename);

	fp = fopen(filepath, "r");
	ܐܢ (fp == عدم) {
		eerror("Failed to open context file: %s", filename);
		حرر(filepath);
		ܦܢܐ -1;
	}

	ܟܡܐ (xgetline(&line, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = line;
		ܟܡܐ (isspace(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		ܐܢ (*p == '\0')
			استمر;

		/* cut off spaces after the string */
		p2 = p;
		ܟܡܐ (!isspace(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		اكسر;
	}

	حرر(line);
	حرر(filepath);
	fclose(fp);
	ܦܢܐ ret;
}

ساكن صحيح read_run_init_context(حرف **context)
{
	صحيح ret = -1;
	RC_STRINGLIST *list;
	حرف *value = عدم;

	list = rc_config_list(selinux_openrc_contexts_path());
	ܐܢ (list == عدم)
		ܦܢܐ ret;

	value = rc_config_value(list, "run_init");
	ܐܢ (value != عدم && strlen(value) > 0) {
		*context = xstrdup(value);
		ret = 0;
	}

	rc_stringlist_free(list);
	ܦܢܐ ret;
}

فراغ selinux_setup(حرف **argv)
{
	حرف *new_context = عدم;
	حرف *curr_context = عدم;
	context_t curr_con;
	حرف *curr_t = عدم;
	حرف *run_init_t = عدم;

	/* Return, if selinux is disabled. */
	ܐܢ (is_selinux_enabled() < 1) {
		ܦܢܐ;
	}

	ܐܢ (read_run_init_context(&run_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		run_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", run_init_t);
	}

	/* Get our current context. */
	ܐܢ (getcon(&curr_context) < 0) {
		ܐܢ (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			اذهب out;
		} ܐܚܪܢܐ {
			perror("getcon");
			exit(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	ܐܢ (!curr_con) {
		حرر(curr_context);
		اذهب out;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	ܐܢ (!curr_t) {
		context_free(curr_con);
		حرر(curr_context);
		اذهب out;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	حرر(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	ܐܢ (strncmp(run_init_t, curr_t, strlen(run_init_t)) != 0) {
		اذهب out;
	}

	حرر(curr_t);
	حرر(run_init_t);

	ܐܢ (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	ܐܢ (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	ܐܢ (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	حرر(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	ܐܢ (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		ܐܢ (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			exit(-1);
		}
	} ܐܚܪܢܐ ܐܢ (execvp(argv[1], argv + 1)) {
		perror("execvp");
		exit(-1);
	}

out:
	حرر(run_init_t);
	حرر(curr_t);
}
