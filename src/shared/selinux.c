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
#define INITRC_FILE "initrc_context"

#ifdef HAVE_AUDIT
#अन्तर्भाव <libaudit.h>
#endif

/* PAM or shadow for authentication */
#ifdef HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#अन्यथा
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#endif


/* The handle for the fcontext lookups */
स्थिर संरचना selabel_handle *hnd = रिक्त;

पूर्णांक selinux_util_label(स्थिरांक अक्षर *path)
{
	पूर्णांक retval = 0;
	पूर्णांक enforce;
	संरचना stat st;
	अक्षर *con;

	enforce = security_getenforce();
	जर (retval < 0)
		परत retval;

	जर (!hnd)
		परत (enforce) ? -1 : 0;

	retval = lstat(path, &st);
	जर (retval < 0) {
		जर (त्रुटि_सं == ENOENT)
			परत 0;
		परत (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, path, st.st_mode);
	जर (retval < 0) {
		जर (त्रुटि_सं == ENOENT)
			परत 0;
		परत (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(path, con);
	freecon(con);
	जर (retval < 0) {
		जर (त्रुटि_सं == ENOENT)
			परत 0;
		जर (त्रुटि_सं == ENOTSUP)
			परत 0;
		परत (enforce) ? -1 : 0;
	}

	परत 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
पूर्णांक selinux_util_open(शून्य)
{
	पूर्णांक retval = 0;

	retval = is_selinux_enabled();
	जर (retval <= 0)
		परत retval;

	hnd = selabel_open(SELABEL_CTX_FILE, रिक्त, 0);
	जर (!hnd)
		परत -2;

	परत 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
पूर्णांक selinux_util_close(शून्य)
{
	पूर्णांक retval = 0;

	retval = is_selinux_enabled();
	जर (retval <= 0)
		परत retval;

	जर (hnd) {
		selabel_close(hnd);
		hnd = रिक्त;
	}

	परत 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
स्थिर पूर्णांक check_password(अक्षर *username)
{
	पूर्णांक ret = 1;
#ifdef HAVE_PAM
	pam_handle_t *pamh;
	पूर्णांक pam_err = 0;
	स्थिरांक संरचना pam_conv pconv = {
		misc_conv,
		रिक्त
	};

	pam_err = pam_start(PAM_SERVICE_NAME, username, &pconv, &pamh);
	जर (pam_err != PAM_SUCCESS) {
		ret = -1;
		जा outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	जर (pam_err != PAM_SUCCESS) {
		ret = -1;
		जा outpam;
	}

	ret = 0;
outpam:
	pam_end(pamh, pam_err);
	pamh = रिक्त;

#अन्यथा /* authenticating via /etc/shadow instead */
	संरचना spwd *spw;
	अक्षर *password;
	अक्षर *attempt;

	spw = getspnam(username);
	जर (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		जा outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	जर (!attempt) {
		ret = -1;
		जा outshadow;
	}

	जर (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		जा outshadow;
	}

	/* salt must be at least two characters long */
	जर (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		जा outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	जर (password && म_भेद(password, spw->sp_pwdp) == 0)
		ret = 0;
	अन्यथा
		ret = -1;
outshadow:
#endif
	परत ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
स्थिर पूर्णांक check_auth()
{
	संरचना passwd *pw;
	uid_t uid;

#ifdef HAVE_AUDIT
	uid = audit_getloginuid();
	जर (uid == (uid_t) -1)
		uid = getuid();
#अन्यथा
	uid = getuid();
#endif

	pw = getpwuid(uid);
	जर (!pw) {
		eerror("cannot find your entry in the passwd file.");
		परत (-1);
	}

	म_लिखो("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	जर (check_password(pw->pw_name) == 0) {
		परत 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	परत 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
स्थिर पूर्णांक read_context_file(स्थिरांक अक्षर *filename, अक्षर **context)
{
	पूर्णांक ret = -1;
	खाता *fp;
	अक्षर *filepath = रिक्त;
	अक्षर *ओळ = रिक्त;
	अक्षर *p;
	अक्षर *p2;
	माप_प्रकार len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_path(), filename);

	fp = ख_खोलो(filepath, "r");
	जर (fp == रिक्त) {
		eerror("Failed to open context file: %s", filename);
		मुक्त(filepath);
		परत -1;
	}

	पर्यंत (xgetline(&ओळ, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = ओळ;
		पर्यंत (है_खाली(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		जर (*p == '\0')
			सुरूठेवा;

		/* cut off spaces after the string */
		p2 = p;
		पर्यंत (!है_खाली(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		थांबा;
	}

	मुक्त(ओळ);
	मुक्त(filepath);
	ख_बंद(fp);
	परत ret;
}

स्थिर पूर्णांक read_run_init_context(अक्षर **context)
{
	पूर्णांक ret = -1;
	RC_STRINGLIST *list;
	अक्षर *मूल्य = रिक्त;

	list = rc_config_list(selinux_openrc_contexts_path());
	जर (list == रिक्त)
		परत ret;

	मूल्य = rc_config_मूल्य(list, "run_init");
	जर (मूल्य != रिक्त && म_माप(मूल्य) > 0) {
		*context = xstrdup(मूल्य);
		ret = 0;
	}

	rc_stringlist_free(list);
	परत ret;
}

शून्य selinux_setup(अक्षर **argv)
{
	अक्षर *new_context = रिक्त;
	अक्षर *curr_context = रिक्त;
	context_t curr_con;
	अक्षर *curr_t = रिक्त;
	अक्षर *run_init_t = रिक्त;

	/* Return, if selinux is disabled. */
	जर (is_selinux_enabled() < 1) {
		परत;
	}

	जर (read_run_init_context(&run_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		run_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", run_init_t);
	}

	/* Get our current context. */
	जर (getcon(&curr_context) < 0) {
		जर (त्रुटि_सं == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			जा out;
		} अन्यथा {
			लिखो_त्रुटि("getcon");
			निकास(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	जर (!curr_con) {
		मुक्त(curr_context);
		जा out;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	जर (!curr_t) {
		context_free(curr_con);
		मुक्त(curr_context);
		जा out;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	मुक्त(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	जर (म_भेदन(run_init_t, curr_t, म_माप(run_init_t)) != 0) {
		जा out;
	}

	मुक्त(curr_t);
	मुक्त(run_init_t);

	जर (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	जर (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	जर (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	मुक्त(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	जर (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		जर (execvp("/usr/sbin/open_init_pty", argv)) {
			लिखो_त्रुटि("execvp");
			निकास(-1);
		}
	} अन्यथा जर (execvp(argv[1], argv + 1)) {
		लिखो_त्रुटि("execvp");
		निकास(-1);
	}

out:
	मुक्त(run_init_t);
	मुक्त(curr_t);
}
