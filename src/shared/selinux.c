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
#تعریف_کرو INITRC_FILE "initrc_context"

#اگر_تعریف_شدہ HAVE_AUDIT
#شامل <libaudit.h>
#اگر_ختم

/* PAM or shadow for authentication */
#اگر_تعریف_شدہ HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#ورنہ
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#اگر_ختم


/* The handle for the fcontext lookups */
ساکن ساخت selabel_handle *hnd = تہی;

صحیح selinux_util_label(ثابت حرف *راستہ)
{
	صحیح retval = 0;
	صحیح enforce;
	ساخت stat st;
	حرف *con;

	enforce = security_getenforce();
	اگر (retval < 0)
		واپس retval;

	اگر (!hnd)
		واپس (enforce) ? -1 : 0;

	retval = lstat(راستہ, &st);
	اگر (retval < 0) {
		اگر (errno == ENOENT)
			واپس 0;
		واپس (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, راستہ, st.st_mode);
	اگر (retval < 0) {
		اگر (errno == ENOENT)
			واپس 0;
		واپس (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(راستہ, con);
	freecon(con);
	اگر (retval < 0) {
		اگر (errno == ENOENT)
			واپس 0;
		اگر (errno == ENOTSUP)
			واپس 0;
		واپس (enforce) ? -1 : 0;
	}

	واپس 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
صحیح selinux_util_open(خالی)
{
	صحیح retval = 0;

	retval = is_selinux_enabled();
	اگر (retval <= 0)
		واپس retval;

	hnd = selabel_open(SELABEL_CTX_FILE, تہی, 0);
	اگر (!hnd)
		واپس -2;

	واپس 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
صحیح selinux_util_close(خالی)
{
	صحیح retval = 0;

	retval = is_selinux_enabled();
	اگر (retval <= 0)
		واپس retval;

	اگر (hnd) {
		selabel_close(hnd);
		hnd = تہی;
	}

	واپس 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
ساکن صحیح check_password(حرف *username)
{
	صحیح ret = 1;
#اگر_تعریف_شدہ HAVE_PAM
	pam_handle_t *pamh;
	صحیح pam_err = 0;
	ثابت ساخت pam_conv pconv = {
		misc_conv,
		تہی
	};

	pam_err = pam_آغاز(PAM_SERVICE_NAME, username, &pconv, &pamh);
	اگر (pam_err != PAM_SUCCESS) {
		ret = -1;
		جاؤ outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	اگر (pam_err != PAM_SUCCESS) {
		ret = -1;
		جاؤ outpam;
	}

	ret = 0;
outpam:
	pam_انتہا(pamh, pam_err);
	pamh = تہی;

#ورنہ /* authenticating via /etc/shadow instead */
	ساخت spwd *spw;
	حرف *password;
	حرف *attempt;

	spw = getspnam(username);
	اگر (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		جاؤ outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	اگر (!attempt) {
		ret = -1;
		جاؤ outshadow;
	}

	اگر (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		جاؤ outshadow;
	}

	/* salt must be at least two characters long */
	اگر (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		جاؤ outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	اگر (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	ورنہ
		ret = -1;
outshadow:
#اگر_ختم
	واپس ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
ساکن صحیح check_auth()
{
	ساخت passwd *pw;
	uid_t uid;

#اگر_تعریف_شدہ HAVE_AUDIT
	uid = audit_getloginuid();
	اگر (uid == (uid_t) -1)
		uid = getuid();
#ورنہ
	uid = getuid();
#اگر_ختم

	pw = getpwuid(uid);
	اگر (!pw) {
		eerror("cannot find your entry in the passwd file.");
		واپس (-1);
	}

	لکھو("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	اگر (check_password(pw->pw_name) == 0) {
		واپس 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	واپس 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
ساکن صحیح read_context_file(ثابت حرف *filename, حرف **context)
{
	صحیح ret = -1;
	FILE *fp;
	حرف *filepath = تہی;
	حرف *سطر = تہی;
	حرف *p;
	حرف *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_راستہ(), filename);

	fp = fopen(filepath, "r");
	اگر (fp == تہی) {
		eerror("Failed to open context file: %s", filename);
		آزاد(filepath);
		واپس -1;
	}

	جب (xgetline(&سطر, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = سطر;
		جب (خلا_ہے(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		اگر (*p == '\0')
			جاری;

		/* cut off spaces after the string */
		p2 = p;
		جب (!خلا_ہے(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		توڑو;
	}

	آزاد(سطر);
	آزاد(filepath);
	fclose(fp);
	واپس ret;
}

ساکن صحیح read_تسلسل_init_context(حرف **context)
{
	صحیح ret = -1;
	RC_STRINGLIST *list;
	حرف *قدر = تہی;

	list = rc_config_list(selinux_openrc_contexts_راستہ());
	اگر (list == تہی)
		واپس ret;

	قدر = rc_config_قدر(list, "run_init");
	اگر (قدر != تہی && لڑی_لمبائی(قدر) > 0) {
		*context = xstrdup(قدر);
		ret = 0;
	}

	rc_stringlist_free(list);
	واپس ret;
}

خالی selinux_تیاری(حرف **argv)
{
	حرف *new_context = تہی;
	حرف *curr_context = تہی;
	context_t curr_con;
	حرف *curr_t = تہی;
	حرف *تسلسل_init_t = تہی;

	/* Return, if selinux is disabled. */
	اگر (is_selinux_enabled() < 1) {
		واپس;
	}

	اگر (read_تسلسل_init_context(&تسلسل_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		تسلسل_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", تسلسل_init_t);
	}

	/* Get our current context. */
	اگر (getcon(&curr_context) < 0) {
		اگر (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			جاؤ برآمد;
		} ورنہ {
			perror("getcon");
			باہر_نکلو(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	اگر (!curr_con) {
		آزاد(curr_context);
		جاؤ برآمد;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	اگر (!curr_t) {
		context_free(curr_con);
		آزاد(curr_context);
		جاؤ برآمد;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	آزاد(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	اگر (لڑی_موازنہ(تسلسل_init_t, curr_t, لڑی_لمبائی(تسلسل_init_t)) != 0) {
		جاؤ برآمد;
	}

	آزاد(curr_t);
	آزاد(تسلسل_init_t);

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
			باہر_نکلو(-1);
		}
	} ورنہ اگر (execvp(argv[1], argv + 1)) {
		perror("execvp");
		باہر_نکلو(-1);
	}

برآمد:
	آزاد(تسلسل_init_t);
	آزاد(curr_t);
}
