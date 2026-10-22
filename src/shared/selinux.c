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

#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <dlfcn.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <pwd.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ <selinux/selinux.h>
#അന്തര്ഭാവ <selinux/label.h>
#അന്തര്ഭാവ <selinux/get_default_type.h>
#അന്തര്ഭാവ <selinux/context.h>

#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/types.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "misc.h"
#അന്തര്ഭാവ "plugin.h"
#അന്തര്ഭാവ "selinux.h"

/* the context files for selinux */
#define INITRC_FILE "initrc_context"

#ifdef HAVE_AUDIT
#അന്തര്ഭാവ <libaudit.h>
#endif

/* PAM or shadow for authentication */
#ifdef HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#അല്ലെങ്കിൽ
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#endif


/* The handle for the fcontext lookups */
സ്ഥിതം ഘടന selabel_handle *hnd = ശൂന്യ;

പൂർണ്ണസംഖ്യ selinux_util_label(സ്ഥിരം അക്ഷരം *path)
{
	പൂർണ്ണസംഖ്യ retval = 0;
	പൂർണ്ണസംഖ്യ enforce;
	ഘടന stat st;
	അക്ഷരം *con;

	enforce = security_getenforce();
	എങ്കിൽ (retval < 0)
		തിരികെ retval;

	എങ്കിൽ (!hnd)
		തിരികെ (enforce) ? -1 : 0;

	retval = lstat(path, &st);
	എങ്കിൽ (retval < 0) {
		എങ്കിൽ (errno == ENOENT)
			തിരികെ 0;
		തിരികെ (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, path, st.st_mode);
	എങ്കിൽ (retval < 0) {
		എങ്കിൽ (errno == ENOENT)
			തിരികെ 0;
		തിരികെ (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(path, con);
	freecon(con);
	എങ്കിൽ (retval < 0) {
		എങ്കിൽ (errno == ENOENT)
			തിരികെ 0;
		എങ്കിൽ (errno == ENOTSUP)
			തിരികെ 0;
		തിരികെ (enforce) ? -1 : 0;
	}

	തിരികെ 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
പൂർണ്ണസംഖ്യ selinux_util_open(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ retval = 0;

	retval = is_selinux_enabled();
	എങ്കിൽ (retval <= 0)
		തിരികെ retval;

	hnd = selabel_open(SELABEL_CTX_FILE, ശൂന്യ, 0);
	എങ്കിൽ (!hnd)
		തിരികെ -2;

	തിരികെ 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
പൂർണ്ണസംഖ്യ selinux_util_close(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ retval = 0;

	retval = is_selinux_enabled();
	എങ്കിൽ (retval <= 0)
		തിരികെ retval;

	എങ്കിൽ (hnd) {
		selabel_close(hnd);
		hnd = ശൂന്യ;
	}

	തിരികെ 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ check_password(അക്ഷരം *username)
{
	പൂർണ്ണസംഖ്യ ret = 1;
#ifdef HAVE_PAM
	pam_handle_t *pamh;
	പൂർണ്ണസംഖ്യ pam_err = 0;
	സ്ഥിരം ഘടന pam_conv pconv = {
		misc_conv,
		ശൂന്യ
	};

	pam_err = pam_start(PAM_SERVICE_NAME, username, &pconv, &pamh);
	എങ്കിൽ (pam_err != PAM_SUCCESS) {
		ret = -1;
		പോകുക outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	എങ്കിൽ (pam_err != PAM_SUCCESS) {
		ret = -1;
		പോകുക outpam;
	}

	ret = 0;
outpam:
	pam_end(pamh, pam_err);
	pamh = ശൂന്യ;

#അല്ലെങ്കിൽ /* authenticating via /etc/shadow instead */
	ഘടന spwd *spw;
	അക്ഷരം *password;
	അക്ഷരം *attempt;

	spw = getspnam(username);
	എങ്കിൽ (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		പോകുക outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	എങ്കിൽ (!attempt) {
		ret = -1;
		പോകുക outshadow;
	}

	എങ്കിൽ (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		പോകുക outshadow;
	}

	/* salt must be at least two characters long */
	എങ്കിൽ (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		പോകുക outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	എങ്കിൽ (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	അല്ലെങ്കിൽ
		ret = -1;
outshadow:
#endif
	തിരികെ ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
സ്ഥിതം പൂർണ്ണസംഖ്യ check_auth()
{
	ഘടന passwd *pw;
	uid_t uid;

#ifdef HAVE_AUDIT
	uid = audit_getloginuid();
	എങ്കിൽ (uid == (uid_t) -1)
		uid = getuid();
#അല്ലെങ്കിൽ
	uid = getuid();
#endif

	pw = getpwuid(uid);
	എങ്കിൽ (!pw) {
		eerror("cannot find your entry in the passwd file.");
		തിരികെ (-1);
	}

	printf("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	എങ്കിൽ (check_password(pw->pw_name) == 0) {
		തിരികെ 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	തിരികെ 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ read_context_file(സ്ഥിരം അക്ഷരം *filename, അക്ഷരം **context)
{
	പൂർണ്ണസംഖ്യ ret = -1;
	FILE *fp;
	അക്ഷരം *filepath = ശൂന്യ;
	അക്ഷരം *വരി = ശൂന്യ;
	അക്ഷരം *p;
	അക്ഷരം *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_path(), filename);

	fp = fopen(filepath, "r");
	എങ്കിൽ (fp == ശൂന്യ) {
		eerror("Failed to open context file: %s", filename);
		സ്വതന്ത്രം(filepath);
		തിരികെ -1;
	}

	ആകുമ്പോൾ (xgetline(&വരി, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = വരി;
		ആകുമ്പോൾ (isspace(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		എങ്കിൽ (*p == '\0')
			തുടരുക;

		/* cut off spaces after the string */
		p2 = p;
		ആകുമ്പോൾ (!isspace(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		നിർത്തുക;
	}

	സ്വതന്ത്രം(വരി);
	സ്വതന്ത്രം(filepath);
	fclose(fp);
	തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ read_run_init_context(അക്ഷരം **context)
{
	പൂർണ്ണസംഖ്യ ret = -1;
	RC_STRINGLIST *list;
	അക്ഷരം *മൂല്യം = ശൂന്യ;

	list = rc_config_list(selinux_openrc_contexts_path());
	എങ്കിൽ (list == ശൂന്യ)
		തിരികെ ret;

	മൂല്യം = rc_config_മൂല്യം(list, "run_init");
	എങ്കിൽ (മൂല്യം != ശൂന്യ && strlen(മൂല്യം) > 0) {
		*context = xstrdup(മൂല്യം);
		ret = 0;
	}

	rc_stringlist_free(list);
	തിരികെ ret;
}

ശൂന്യം selinux_setup(അക്ഷരം **argv)
{
	അക്ഷരം *new_context = ശൂന്യ;
	അക്ഷരം *curr_context = ശൂന്യ;
	context_t curr_con;
	അക്ഷരം *curr_t = ശൂന്യ;
	അക്ഷരം *run_init_t = ശൂന്യ;

	/* Return, if selinux is disabled. */
	എങ്കിൽ (is_selinux_enabled() < 1) {
		തിരികെ;
	}

	എങ്കിൽ (read_run_init_context(&run_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		run_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", run_init_t);
	}

	/* Get our current context. */
	എങ്കിൽ (getcon(&curr_context) < 0) {
		എങ്കിൽ (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			പോകുക out;
		} അല്ലെങ്കിൽ {
			perror("getcon");
			exit(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	എങ്കിൽ (!curr_con) {
		സ്വതന്ത്രം(curr_context);
		പോകുക out;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	എങ്കിൽ (!curr_t) {
		context_free(curr_con);
		സ്വതന്ത്രം(curr_context);
		പോകുക out;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	സ്വതന്ത്രം(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	എങ്കിൽ (strncmp(run_init_t, curr_t, strlen(run_init_t)) != 0) {
		പോകുക out;
	}

	സ്വതന്ത്രം(curr_t);
	സ്വതന്ത്രം(run_init_t);

	എങ്കിൽ (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	എങ്കിൽ (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	എങ്കിൽ (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	സ്വതന്ത്രം(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	എങ്കിൽ (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		എങ്കിൽ (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			exit(-1);
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (execvp(argv[1], argv + 1)) {
		perror("execvp");
		exit(-1);
	}

out:
	സ്വതന്ത്രം(run_init_t);
	സ്വതന്ത്രം(curr_t);
}
