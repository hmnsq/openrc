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

#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <dlfcn.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <pwd.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ <selinux/selinux.h>
#ଅନ୍ତର୍ଭାଵ <selinux/label.h>
#ଅନ୍ତର୍ଭାଵ <selinux/get_default_type.h>
#ଅନ୍ତର୍ଭାଵ <selinux/context.h>

#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "plugin.h"
#ଅନ୍ତର୍ଭାଵ "selinux.h"

/* the context files for selinux */
#define INITRC_FILE "initrc_context"

#ifdef HAVE_AUDIT
#ଅନ୍ତର୍ଭାଵ <libaudit.h>
#endif

/* PAM or shadow for authentication */
#ifdef HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#ଅନ୍ୟଥା
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#endif


/* The handle for the fcontext lookups */
ସ୍ଥିର ସଂରଚନା selabel_handle *hnd = ଖାଲି;

ପୂର୍ଣ୍ଣସଂଖ୍ୟା selinux_util_label(ଅଚଳ ଅକ୍ଷର *path)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା enforce;
	ସଂରଚନା stat st;
	ଅକ୍ଷର *con;

	enforce = security_getenforce();
	ଯଦି (retval < 0)
		ଫେରନ୍ତୁ retval;

	ଯଦି (!hnd)
		ଫେରନ୍ତୁ (enforce) ? -1 : 0;

	retval = lstat(path, &st);
	ଯଦି (retval < 0) {
		ଯଦି (errno == ENOENT)
			ଫେରନ୍ତୁ 0;
		ଫେରନ୍ତୁ (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, path, st.st_mode);
	ଯଦି (retval < 0) {
		ଯଦି (errno == ENOENT)
			ଫେରନ୍ତୁ 0;
		ଫେରନ୍ତୁ (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(path, con);
	freecon(con);
	ଯଦି (retval < 0) {
		ଯଦି (errno == ENOENT)
			ଫେରନ୍ତୁ 0;
		ଯଦି (errno == ENOTSUP)
			ଫେରନ୍ତୁ 0;
		ଫେରନ୍ତୁ (enforce) ? -1 : 0;
	}

	ଫେରନ୍ତୁ 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା selinux_util_open(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval = 0;

	retval = is_selinux_enabled();
	ଯଦି (retval <= 0)
		ଫେରନ୍ତୁ retval;

	hnd = selabel_open(SELABEL_CTX_FILE, ଖାଲି, 0);
	ଯଦି (!hnd)
		ଫେରନ୍ତୁ -2;

	ଫେରନ୍ତୁ 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା selinux_util_close(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval = 0;

	retval = is_selinux_enabled();
	ଯଦି (retval <= 0)
		ଫେରନ୍ତୁ retval;

	ଯଦି (hnd) {
		selabel_close(hnd);
		hnd = ଖାଲି;
	}

	ଫେରନ୍ତୁ 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା check_password(ଅକ୍ଷର *username)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 1;
#ifdef HAVE_PAM
	pam_handle_t *pamh;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pam_err = 0;
	ଅଚଳ ସଂରଚନା pam_conv pconv = {
		misc_conv,
		ଖାଲି
	};

	pam_err = pam_start(PAM_SERVICE_NAME, username, &pconv, &pamh);
	ଯଦି (pam_err != PAM_SUCCESS) {
		ret = -1;
		ଯାଅ outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	ଯଦି (pam_err != PAM_SUCCESS) {
		ret = -1;
		ଯାଅ outpam;
	}

	ret = 0;
outpam:
	pam_end(pamh, pam_err);
	pamh = ଖାଲି;

#ଅନ୍ୟଥା /* authenticating via /etc/shadow instead */
	ସଂରଚନା spwd *spw;
	ଅକ୍ଷର *password;
	ଅକ୍ଷର *attempt;

	spw = getspnam(username);
	ଯଦି (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		ଯାଅ outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	ଯଦି (!attempt) {
		ret = -1;
		ଯାଅ outshadow;
	}

	ଯଦି (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		ଯାଅ outshadow;
	}

	/* salt must be at least two characters long */
	ଯଦି (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		ଯାଅ outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	ଯଦି (password && strcmp(password, spw->sp_pwdp) == 0)
		ret = 0;
	ଅନ୍ୟଥା
		ret = -1;
outshadow:
#endif
	ଫେରନ୍ତୁ ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା check_auth()
{
	ସଂରଚନା passwd *pw;
	uid_t uid;

#ifdef HAVE_AUDIT
	uid = audit_getloginuid();
	ଯଦି (uid == (uid_t) -1)
		uid = getuid();
#ଅନ୍ୟଥା
	uid = getuid();
#endif

	pw = getpwuid(uid);
	ଯଦି (!pw) {
		eerror("cannot find your entry in the passwd file.");
		ଫେରନ୍ତୁ (-1);
	}

	printf("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	ଯଦି (check_password(pw->pw_name) == 0) {
		ଫେରନ୍ତୁ 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	ଫେରନ୍ତୁ 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା read_context_file(ଅଚଳ ଅକ୍ଷର *filename, ଅକ୍ଷର **context)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = -1;
	FILE *fp;
	ଅକ୍ଷର *filepath = ଖାଲି;
	ଅକ୍ଷର *ଧାଡ଼ି = ଖାଲି;
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_path(), filename);

	fp = fopen(filepath, "r");
	ଯଦି (fp == ଖାଲି) {
		eerror("Failed to open context file: %s", filename);
		ମୁକ୍ତ(filepath);
		ଫେରନ୍ତୁ -1;
	}

	ଯେତେବେଳେ (xgetline(&ଧାଡ଼ି, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = ଧାଡ଼ି;
		ଯେତେବେଳେ (isspace(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		ଯଦି (*p == '\0')
			ଜାରି;

		/* cut off spaces after the string */
		p2 = p;
		ଯେତେବେଳେ (!isspace(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		ଭାଙ୍ଗ;
	}

	ମୁକ୍ତ(ଧାଡ଼ି);
	ମୁକ୍ତ(filepath);
	fclose(fp);
	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା read_run_init_context(ଅକ୍ଷର **context)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = -1;
	RC_STRINGLIST *list;
	ଅକ୍ଷର *ମୂଲ୍ୟ = ଖାଲି;

	list = rc_config_list(selinux_openrc_contexts_path());
	ଯଦି (list == ଖାଲି)
		ଫେରନ୍ତୁ ret;

	ମୂଲ୍ୟ = rc_config_ମୂଲ୍ୟ(list, "run_init");
	ଯଦି (ମୂଲ୍ୟ != ଖାଲି && strlen(ମୂଲ୍ୟ) > 0) {
		*context = xstrdup(ମୂଲ୍ୟ);
		ret = 0;
	}

	rc_stringlist_free(list);
	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ selinux_setup(ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର *new_context = ଖାଲି;
	ଅକ୍ଷର *curr_context = ଖାଲି;
	context_t curr_con;
	ଅକ୍ଷର *curr_t = ଖାଲି;
	ଅକ୍ଷର *run_init_t = ଖାଲି;

	/* Return, if selinux is disabled. */
	ଯଦି (is_selinux_enabled() < 1) {
		ଫେରନ୍ତୁ;
	}

	ଯଦି (read_run_init_context(&run_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		run_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", run_init_t);
	}

	/* Get our current context. */
	ଯଦି (getcon(&curr_context) < 0) {
		ଯଦି (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			ଯାଅ out;
		} ଅନ୍ୟଥା {
			perror("getcon");
			exit(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	ଯଦି (!curr_con) {
		ମୁକ୍ତ(curr_context);
		ଯାଅ out;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	ଯଦି (!curr_t) {
		context_free(curr_con);
		ମୁକ୍ତ(curr_context);
		ଯାଅ out;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	ମୁକ୍ତ(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	ଯଦି (strncmp(run_init_t, curr_t, strlen(run_init_t)) != 0) {
		ଯାଅ out;
	}

	ମୁକ୍ତ(curr_t);
	ମୁକ୍ତ(run_init_t);

	ଯଦି (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	ଯଦି (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	ଯଦି (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	ମୁକ୍ତ(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	ଯଦି (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		ଯଦି (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			exit(-1);
		}
	} ଅନ୍ୟଥା ଯଦି (execvp(argv[1], argv + 1)) {
		perror("execvp");
		exit(-1);
	}

out:
	ମୁକ୍ତ(run_init_t);
	ମୁକ୍ତ(curr_t);
}
