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

#شامل <stddef.h>
#شامل <errno.h>
#شامل <dlfcn.h>
#شامل <ctype.h>
#شامل <limits.h>
#شامل <pwd.h>
#شامل <unistd.h>

#شامل <selinux/selinux.h>
#شامل <selinux/label.h>
#شامل <selinux/get_default_type.h>
#شامل <selinux/context.h>

#شامل <sys/stat.h>
#شامل <sys/types.h>

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "misc.h"
#شامل "plugin.h"
#شامل "selinux.h"

/* the context files for selinux */
#define INITRC_FILE "initrc_context"

#ifdef HAVE_AUDIT
#شامل <libaudit.h>
#endif

/* PAM or shadow for authentication */
#ifdef HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#وگرنه
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#endif


/* The handle for the fcontext lookups */
ایستا ساختار selabel_handle *hnd = پوچ;

صحیح selinux_util_label(ثابت نویسه *path)
{
	صحیح retval = 0;
	صحیح enforce;
	ساختار stat st;
	نویسه *con;

	enforce = security_getenforce();
	اگر (retval < 0)
		بازگشت retval;

	اگر (!hnd)
		بازگشت (enforce) ? -1 : 0;

	retval = lstat(path, &st);
	اگر (retval < 0) {
		اگر (errno == ENOENT)
			بازگشت 0;
		بازگشت (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, path, st.st_mode);
	اگر (retval < 0) {
		اگر (errno == ENOENT)
			بازگشت 0;
		بازگشت (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(path, con);
	freecon(con);
	اگر (retval < 0) {
		اگر (errno == ENOENT)
			بازگشت 0;
		اگر (errno == ENOTSUP)
			بازگشت 0;
		بازگشت (enforce) ? -1 : 0;
	}

	بازگشت 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
صحیح selinux_util_open(تهی)
{
	صحیح retval = 0;

	retval = is_selinux_enabled();
	اگر (retval <= 0)
		بازگشت retval;

	hnd = selabel_open(SELABEL_CTX_FILE, پوچ, 0);
	اگر (!hnd)
		بازگشت -2;

	بازگشت 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
صحیح selinux_util_close(تهی)
{
	صحیح retval = 0;

	retval = is_selinux_enabled();
	اگر (retval <= 0)
		بازگشت retval;

	اگر (hnd) {
		selabel_close(hnd);
		hnd = پوچ;
	}

	بازگشت 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
ایستا صحیح check_password(نویسه *username)
{
	صحیح ret = 1;
#ifdef HAVE_PAM
	pam_handle_t *pamh;
	صحیح pam_err = 0;
	ثابت ساختار pam_conv pconv = {
		misc_conv,
		پوچ
	};

	pam_err = pam_start(PAM_SERVICE_NAME, username, &pconv, &pamh);
	اگر (pam_err != PAM_SUCCESS) {
		ret = -1;
		برو outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	اگر (pam_err != PAM_SUCCESS) {
		ret = -1;
		برو outpam;
	}

	ret = 0;
outpam:
	pam_end(pamh, pam_err);
	pamh = پوچ;

#وگرنه /* authenticating via /etc/shadow instead */
	ساختار spwd *spw;
	نویسه *password;
	نویسه *attempt;

	spw = getspnam(username);
	اگر (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		برو outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	اگر (!attempt) {
		ret = -1;
		برو outshadow;
	}

	اگر (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		برو outshadow;
	}

	/* salt must be at least two characters long */
	اگر (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		برو outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	اگر (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	وگرنه
		ret = -1;
outshadow:
#endif
	بازگشت ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
ایستا صحیح check_auth()
{
	ساختار passwd *pw;
	uid_t uid;

#ifdef HAVE_AUDIT
	uid = audit_getloginuid();
	اگر (uid == (uid_t) -1)
		uid = getuid();
#وگرنه
	uid = getuid();
#endif

	pw = getpwuid(uid);
	اگر (!pw) {
		eerror("cannot find your entry in the passwd file.");
		بازگشت (-1);
	}

	printf("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	اگر (check_password(pw->pw_name) == 0) {
		بازگشت 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	بازگشت 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
ایستا صحیح read_context_file(ثابت نویسه *filename, نویسه **context)
{
	صحیح ret = -1;
	FILE *fp;
	نویسه *filepath = پوچ;
	نویسه *سطر = پوچ;
	نویسه *p;
	نویسه *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_path(), filename);

	fp = fopen(filepath, "r");
	اگر (fp == پوچ) {
		eerror("Failed to open context file: %s", filename);
		آزاد(filepath);
		بازگشت -1;
	}

	وقتی (xgetline(&سطر, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = سطر;
		وقتی (isspace(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		اگر (*p == '\0')
			ادامه;

		/* cut off spaces after the string */
		p2 = p;
		وقتی (!isspace(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		بشکن;
	}

	آزاد(سطر);
	آزاد(filepath);
	fclose(fp);
	بازگشت ret;
}

ایستا صحیح read_run_init_context(نویسه **context)
{
	صحیح ret = -1;
	RC_STRINGLIST *list;
	نویسه *مقدار = پوچ;

	list = rc_config_list(selinux_openrc_contexts_path());
	اگر (list == پوچ)
		بازگشت ret;

	مقدار = rc_config_مقدار(list, "run_init");
	اگر (مقدار != پوچ && strlen(مقدار) > 0) {
		*context = xstrdup(مقدار);
		ret = 0;
	}

	rc_stringlist_free(list);
	بازگشت ret;
}

تهی selinux_setup(نویسه **argv)
{
	نویسه *new_context = پوچ;
	نویسه *curr_context = پوچ;
	context_t curr_con;
	نویسه *curr_t = پوچ;
	نویسه *run_init_t = پوچ;

	/* Return, if selinux is disabled. */
	اگر (is_selinux_enabled() < 1) {
		بازگشت;
	}

	اگر (read_run_init_context(&run_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		run_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", run_init_t);
	}

	/* Get our current context. */
	اگر (getcon(&curr_context) < 0) {
		اگر (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			برو out;
		} وگرنه {
			perror("getcon");
			exit(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	اگر (!curr_con) {
		آزاد(curr_context);
		برو out;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	اگر (!curr_t) {
		context_free(curr_con);
		آزاد(curr_context);
		برو out;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	آزاد(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	اگر (strncmp(run_init_t, curr_t, strlen(run_init_t)) != 0) {
		برو out;
	}

	آزاد(curr_t);
	آزاد(run_init_t);

	اگر (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	اگر (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	اگر (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	آزاد(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	اگر (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		اگر (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			exit(-1);
		}
	} وگرنه اگر (execvp(argv[1], argv + 1)) {
		perror("execvp");
		exit(-1);
	}

out:
	آزاد(run_init_t);
	آزاد(curr_t);
}
