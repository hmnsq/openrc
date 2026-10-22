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

#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <dlfcn.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <pwd.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ <selinux/selinux.h>
#અન્તર્ભાવ <selinux/label.h>
#અન્તર્ભાવ <selinux/get_default_type.h>
#અન્તર્ભાવ <selinux/context.h>

#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/types.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "misc.h"
#અન્તર્ભાવ "plugin.h"
#અન્તર્ભાવ "selinux.h"

/* the context files for selinux */
#define INITRC_FILE "initrc_context"

#ifdef HAVE_AUDIT
#અન્તર્ભાવ <libaudit.h>
#endif

/* PAM or shadow for authentication */
#ifdef HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#નહીંતર
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#endif


/* The handle for the fcontext lookups */
સ્થિર સંરચના selabel_handle *hnd = ખાલી;

પૂર્ણાંક selinux_util_label(અચલ અક્ષર *path)
{
	પૂર્ણાંક retval = 0;
	પૂર્ણાંક enforce;
	સંરચના stat st;
	અક્ષર *con;

	enforce = security_getenforce();
	જો (retval < 0)
		પાછા retval;

	જો (!hnd)
		પાછા (enforce) ? -1 : 0;

	retval = lstat(path, &st);
	જો (retval < 0) {
		જો (errno == ENOENT)
			પાછા 0;
		પાછા (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, path, st.st_mode);
	જો (retval < 0) {
		જો (errno == ENOENT)
			પાછા 0;
		પાછા (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(path, con);
	freecon(con);
	જો (retval < 0) {
		જો (errno == ENOENT)
			પાછા 0;
		જો (errno == ENOTSUP)
			પાછા 0;
		પાછા (enforce) ? -1 : 0;
	}

	પાછા 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
પૂર્ણાંક selinux_util_open(શૂન્ય)
{
	પૂર્ણાંક retval = 0;

	retval = is_selinux_enabled();
	જો (retval <= 0)
		પાછા retval;

	hnd = selabel_open(SELABEL_CTX_FILE, ખાલી, 0);
	જો (!hnd)
		પાછા -2;

	પાછા 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
પૂર્ણાંક selinux_util_close(શૂન્ય)
{
	પૂર્ણાંક retval = 0;

	retval = is_selinux_enabled();
	જો (retval <= 0)
		પાછા retval;

	જો (hnd) {
		selabel_close(hnd);
		hnd = ખાલી;
	}

	પાછા 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
સ્થિર પૂર્ણાંક check_password(અક્ષર *username)
{
	પૂર્ણાંક ret = 1;
#ifdef HAVE_PAM
	pam_handle_t *pamh;
	પૂર્ણાંક pam_err = 0;
	અચલ સંરચના pam_conv pconv = {
		misc_conv,
		ખાલી
	};

	pam_err = pam_start(PAM_SERVICE_NAME, username, &pconv, &pamh);
	જો (pam_err != PAM_SUCCESS) {
		ret = -1;
		જાઓ outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	જો (pam_err != PAM_SUCCESS) {
		ret = -1;
		જાઓ outpam;
	}

	ret = 0;
outpam:
	pam_end(pamh, pam_err);
	pamh = ખાલી;

#નહીંતર /* authenticating via /etc/shadow instead */
	સંરચના spwd *spw;
	અક્ષર *password;
	અક્ષર *attempt;

	spw = getspnam(username);
	જો (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		જાઓ outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	જો (!attempt) {
		ret = -1;
		જાઓ outshadow;
	}

	જો (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		જાઓ outshadow;
	}

	/* salt must be at least two characters long */
	જો (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		જાઓ outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	જો (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	નહીંતર
		ret = -1;
outshadow:
#endif
	પાછા ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
સ્થિર પૂર્ણાંક check_auth()
{
	સંરચના passwd *pw;
	uid_t uid;

#ifdef HAVE_AUDIT
	uid = audit_getloginuid();
	જો (uid == (uid_t) -1)
		uid = getuid();
#નહીંતર
	uid = getuid();
#endif

	pw = getpwuid(uid);
	જો (!pw) {
		eerror("cannot find your entry in the passwd file.");
		પાછા (-1);
	}

	printf("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	જો (check_password(pw->pw_name) == 0) {
		પાછા 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	પાછા 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
સ્થિર પૂર્ણાંક read_context_file(અચલ અક્ષર *filename, અક્ષર **context)
{
	પૂર્ણાંક ret = -1;
	FILE *fp;
	અક્ષર *filepath = ખાલી;
	અક્ષર *લીટી = ખાલી;
	અક્ષર *p;
	અક્ષર *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_path(), filename);

	fp = fopen(filepath, "r");
	જો (fp == ખાલી) {
		eerror("Failed to open context file: %s", filename);
		મુક્ત(filepath);
		પાછા -1;
	}

	જ્યારે (xgetline(&લીટી, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = લીટી;
		જ્યારે (isspace(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		જો (*p == '\0')
			ચાલુ;

		/* cut off spaces after the string */
		p2 = p;
		જ્યારે (!isspace(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		તોડો;
	}

	મુક્ત(લીટી);
	મુક્ત(filepath);
	fclose(fp);
	પાછા ret;
}

સ્થિર પૂર્ણાંક read_run_init_context(અક્ષર **context)
{
	પૂર્ણાંક ret = -1;
	RC_STRINGLIST *list;
	અક્ષર *કિંમત = ખાલી;

	list = rc_config_list(selinux_openrc_contexts_path());
	જો (list == ખાલી)
		પાછા ret;

	કિંમત = rc_config_કિંમત(list, "run_init");
	જો (કિંમત != ખાલી && strlen(કિંમત) > 0) {
		*context = xstrdup(કિંમત);
		ret = 0;
	}

	rc_stringlist_free(list);
	પાછા ret;
}

શૂન્ય selinux_setup(અક્ષર **argv)
{
	અક્ષર *new_context = ખાલી;
	અક્ષર *curr_context = ખાલી;
	context_t curr_con;
	અક્ષર *curr_t = ખાલી;
	અક્ષર *run_init_t = ખાલી;

	/* Return, if selinux is disabled. */
	જો (is_selinux_enabled() < 1) {
		પાછા;
	}

	જો (read_run_init_context(&run_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		run_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", run_init_t);
	}

	/* Get our current context. */
	જો (getcon(&curr_context) < 0) {
		જો (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			જાઓ out;
		} નહીંતર {
			perror("getcon");
			exit(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	જો (!curr_con) {
		મુક્ત(curr_context);
		જાઓ out;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	જો (!curr_t) {
		context_free(curr_con);
		મુક્ત(curr_context);
		જાઓ out;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	મુક્ત(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	જો (strncmp(run_init_t, curr_t, strlen(run_init_t)) != 0) {
		જાઓ out;
	}

	મુક્ત(curr_t);
	મુક્ત(run_init_t);

	જો (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	જો (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	જો (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	મુક્ત(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	જો (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		જો (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			exit(-1);
		}
	} નહીંતર જો (execvp(argv[1], argv + 1)) {
		perror("execvp");
		exit(-1);
	}

out:
	મુક્ત(run_init_t);
	મુક્ત(curr_t);
}
