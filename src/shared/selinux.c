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

#अन्तर्भाव <stddef.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <dlfcn.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <pwd.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव <selinux/selinux.h>
#अन्तर्भाव <selinux/label.h>
#अन्तर्भाव <selinux/get_default_type.h>
#अन्तर्भाव <selinux/context.h>

#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/types.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "plugin.h"
#अन्तर्भाव "selinux.h"

/* the context files for selinux */
#परिभाषय INITRC_FILE "initrc_context"

#यदि_परिभाषितम् HAVE_AUDIT
#अन्तर्भाव <libaudit.h>
#यदि_अन्तः

/* PAM or shadow for authentication */
#यदि_परिभाषितम् HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#अन्यथा
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#यदि_अन्तः


/* The handle for the fcontext lookups */
स्थिर संरचना selabel_handle *hnd = रिक्त;

पूर्णाङ्क selinux_util_label(ध्रुव वर्ण *मार्गः)
{
	पूर्णाङ्क retval = 0;
	पूर्णाङ्क enforce;
	संरचना stat st;
	वर्ण *con;

	enforce = security_getenforce();
	यदि (retval < 0)
		प्रत्यागम retval;

	यदि (!hnd)
		प्रत्यागम (enforce) ? -1 : 0;

	retval = lstat(मार्गः, &st);
	यदि (retval < 0) {
		यदि (errno == ENOENT)
			प्रत्यागम 0;
		प्रत्यागम (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, मार्गः, st.st_mode);
	यदि (retval < 0) {
		यदि (errno == ENOENT)
			प्रत्यागम 0;
		प्रत्यागम (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(मार्गः, con);
	freecon(con);
	यदि (retval < 0) {
		यदि (errno == ENOENT)
			प्रत्यागम 0;
		यदि (errno == ENOTSUP)
			प्रत्यागम 0;
		प्रत्यागम (enforce) ? -1 : 0;
	}

	प्रत्यागम 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
पूर्णाङ्क selinux_util_open(शून्य)
{
	पूर्णाङ्क retval = 0;

	retval = is_selinux_enabled();
	यदि (retval <= 0)
		प्रत्यागम retval;

	hnd = selabel_open(SELABEL_CTX_FILE, रिक्त, 0);
	यदि (!hnd)
		प्रत्यागम -2;

	प्रत्यागम 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
पूर्णाङ्क selinux_util_close(शून्य)
{
	पूर्णाङ्क retval = 0;

	retval = is_selinux_enabled();
	यदि (retval <= 0)
		प्रत्यागम retval;

	यदि (hnd) {
		selabel_close(hnd);
		hnd = रिक्त;
	}

	प्रत्यागम 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
स्थिर पूर्णाङ्क check_password(वर्ण *username)
{
	पूर्णाङ्क ret = 1;
#यदि_परिभाषितम् HAVE_PAM
	pam_handle_t *pamh;
	पूर्णाङ्क pam_err = 0;
	ध्रुव संरचना pam_conv pconv = {
		misc_conv,
		रिक्त
	};

	pam_err = pam_start(PAM_SERVICE_NAME, username, &pconv, &pamh);
	यदि (pam_err != PAM_SUCCESS) {
		ret = -1;
		गच्छ outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	यदि (pam_err != PAM_SUCCESS) {
		ret = -1;
		गच्छ outpam;
	}

	ret = 0;
outpam:
	pam_अन्तः(pamh, pam_err);
	pamh = रिक्त;

#अन्यथा /* authenticating via /etc/shadow instead */
	संरचना spwd *spw;
	वर्ण *password;
	वर्ण *attempt;

	spw = getspnam(username);
	यदि (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		गच्छ outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	यदि (!attempt) {
		ret = -1;
		गच्छ outshadow;
	}

	यदि (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		गच्छ outshadow;
	}

	/* salt must be at least two characters long */
	यदि (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		गच्छ outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	यदि (password && वाक्यं_तुलय(password, spw->sp_pwdp) == 0)
		ret = 0;
	अन्यथा
		ret = -1;
outshadow:
#यदि_अन्तः
	प्रत्यागम ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
स्थिर पूर्णाङ्क check_auth()
{
	संरचना passwd *pw;
	uid_t uid;

#यदि_परिभाषितम् HAVE_AUDIT
	uid = audit_getloginuid();
	यदि (uid == (uid_t) -1)
		uid = getuid();
#अन्यथा
	uid = getuid();
#यदि_अन्तः

	pw = getpwuid(uid);
	यदि (!pw) {
		eerror("cannot find your entry in the passwd file.");
		प्रत्यागम (-1);
	}

	प्रारूप_लिख("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	यदि (check_password(pw->pw_name) == 0) {
		प्रत्यागम 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	प्रत्यागम 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
स्थिर पूर्णाङ्क read_context_file(ध्रुव वर्ण *filename, वर्ण **context)
{
	पूर्णाङ्क ret = -1;
	FILE *fp;
	वर्ण *filepath = रिक्त;
	वर्ण *पङ्क्तिः = रिक्त;
	वर्ण *p;
	वर्ण *p2;
	size_t len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_मार्गः(), filename);

	fp = fopen(filepath, "r");
	यदि (fp == रिक्त) {
		eerror("Failed to open context file: %s", filename);
		मुक्त(filepath);
		प्रत्यागम -1;
	}

	यावत् (xgetline(&पङ्क्तिः, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = पङ्क्तिः;
		यावत् (रिक्तम्_वा(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		यदि (*p == '\0')
			अनुवर्त;

		/* cut off spaces after the string */
		p2 = p;
		यावत् (!रिक्तम्_वा(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		भङ्ग;
	}

	मुक्त(पङ्क्तिः);
	मुक्त(filepath);
	fclose(fp);
	प्रत्यागम ret;
}

स्थिर पूर्णाङ्क read_सततम्_init_context(वर्ण **context)
{
	पूर्णाङ्क ret = -1;
	RC_STRINGLIST *list;
	वर्ण *मानम् = रिक्त;

	list = rc_config_list(selinux_openrc_contexts_मार्गः());
	यदि (list == रिक्त)
		प्रत्यागम ret;

	मानम् = rc_config_मानम्(list, "run_init");
	यदि (मानम् != रिक्त && वाक्यदैर्घ्यम्(मानम्) > 0) {
		*context = xstrdup(मानम्);
		ret = 0;
	}

	rc_stringlist_free(list);
	प्रत्यागम ret;
}

शून्य selinux_सज्जता(वर्ण **argv)
{
	वर्ण *new_context = रिक्त;
	वर्ण *curr_context = रिक्त;
	context_t curr_con;
	वर्ण *curr_t = रिक्त;
	वर्ण *सततम्_init_t = रिक्त;

	/* Return, if selinux is disabled. */
	यदि (is_selinux_enabled() < 1) {
		प्रत्यागम;
	}

	यदि (read_सततम्_init_context(&सततम्_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		सततम्_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", सततम्_init_t);
	}

	/* Get our current context. */
	यदि (getcon(&curr_context) < 0) {
		यदि (errno == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			गच्छ बहिर्गमनम्;
		} अन्यथा {
			perror("getcon");
			निर्गच्छ(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	यदि (!curr_con) {
		मुक्त(curr_context);
		गच्छ बहिर्गमनम्;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	यदि (!curr_t) {
		context_free(curr_con);
		मुक्त(curr_context);
		गच्छ बहिर्गमनम्;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	मुक्त(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	यदि (वाक्ये_तुलय(सततम्_init_t, curr_t, वाक्यदैर्घ्यम्(सततम्_init_t)) != 0) {
		गच्छ बहिर्गमनम्;
	}

	मुक्त(curr_t);
	मुक्त(सततम्_init_t);

	यदि (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	यदि (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	यदि (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	मुक्त(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	यदि (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		यदि (execvp("/usr/sbin/open_init_pty", argv)) {
			perror("execvp");
			निर्गच्छ(-1);
		}
	} अन्यथा यदि (execvp(argv[1], argv + 1)) {
		perror("execvp");
		निर्गच्छ(-1);
	}

बहिर्गमनम्:
	मुक्त(सततम्_init_t);
	मुक्त(curr_t);
}
