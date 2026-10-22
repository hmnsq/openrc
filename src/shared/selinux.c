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

#समावेश <stddef.h>
#समावेश <errno.h>
#समावेश <dlfcn.h>
#समावेश <ctype.h>
#समावेश <limits.h>
#समावेश <pwd.h>
#समावेश <unistd.h>

#समावेश <selinux/selinux.h>
#समावेश <selinux/label.h>
#समावेश <selinux/get_default_type.h>
#समावेश <selinux/context.h>

#समावेश <sys/stat.h>
#समावेश <sys/types.h>

#समावेश "einfo.h"
#समावेश "queue.h"
#समावेश "rc.h"
#समावेश "misc.h"
#समावेश "plugin.h"
#समावेश "selinux.h"

/* the context files for selinux */
#घोषणा INITRC_FILE "initrc_context"

#अगर_घोषित HAVE_AUDIT
#समावेश <libaudit.h>
#पूर्ण_अगर

/* PAM or shadow for authentication */
#अगर_घोषित HAVE_PAM
#    define PAM_SERVICE_NAME "run_init" /* the name of this program for PAM */
#    include <security/pam_appl.h>
#    include <security/pam_misc.h>
#अन्यथा
#    define PASSWORD_PROMPT "Password:"
#    include <crypt.h>
#    include <shadow.h>
#    include <string.h>
#पूर्ण_अगर


/* The handle for the fcontext lookups */
अटल काष्ठा selabel_handle *hnd = शून्य;

पूर्णांक selinux_util_label(स्थिर अक्षर *path)
{
	पूर्णांक retval = 0;
	पूर्णांक enforce;
	काष्ठा stat st;
	अक्षर *con;

	enforce = security_getenforce();
	अगर (retval < 0)
		वापस retval;

	अगर (!hnd)
		वापस (enforce) ? -1 : 0;

	retval = lstat(path, &st);
	अगर (retval < 0) {
		अगर (त्रुटि_सं == ENOENT)
			वापस 0;
		वापस (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, path, st.st_mode);
	अगर (retval < 0) {
		अगर (त्रुटि_सं == ENOENT)
			वापस 0;
		वापस (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(path, con);
	freecon(con);
	अगर (retval < 0) {
		अगर (त्रुटि_सं == ENOENT)
			वापस 0;
		अगर (त्रुटि_सं == ENOTSUP)
			वापस 0;
		वापस (enforce) ? -1 : 0;
	}

	वापस 0;
}

/*
 * Open the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
पूर्णांक selinux_util_open(व्योम)
{
	पूर्णांक retval = 0;

	retval = is_selinux_enabled();
	अगर (retval <= 0)
		वापस retval;

	hnd = selabel_open(SELABEL_CTX_FILE, शून्य, 0);
	अगर (!hnd)
		वापस -2;

	वापस 1;
}

/*
 * Close the label handle
 * returns 1 on success, 0 if no selinux, negative on error
 */
पूर्णांक selinux_util_close(व्योम)
{
	पूर्णांक retval = 0;

	retval = is_selinux_enabled();
	अगर (retval <= 0)
		वापस retval;

	अगर (hnd) {
		selabel_close(hnd);
		hnd = शून्य;
	}

	वापस 0;
}

/*
 * This will check the users password and return 0 on success or -1 on fail
 *
 * We ask for the password to make sure it is intended vs run by malicious software.
 * Actual authorization is covered by the policy itself.
 */
अटल पूर्णांक check_password(अक्षर *username)
{
	पूर्णांक ret = 1;
#अगर_घोषित HAVE_PAM
	pam_handle_t *pamh;
	पूर्णांक pam_err = 0;
	स्थिर काष्ठा pam_conv pconv = {
		misc_conv,
		शून्य
	};

	pam_err = pam_start(PAM_SERVICE_NAME, username, &pconv, &pamh);
	अगर (pam_err != PAM_SUCCESS) {
		ret = -1;
		जाओ outpam;
	}

	pam_err = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	अगर (pam_err != PAM_SUCCESS) {
		ret = -1;
		जाओ outpam;
	}

	ret = 0;
outpam:
	pam_end(pamh, pam_err);
	pamh = शून्य;

#अन्यथा /* authenticating via /etc/shadow instead */
	काष्ठा spwd *spw;
	अक्षर *password;
	अक्षर *attempt;

	spw = getspnam(username);
	अगर (!spw) {
		eerror("Failed to read shadow entry");
		ret = -1;
		जाओ outshadow;
	}

	attempt = getpass(PASSWORD_PROMPT);
	अगर (!attempt) {
		ret = -1;
		जाओ outshadow;
	}

	अगर (*spw->sp_pwdp == '\0' && *attempt == '\0') {
		ret = -1;
		जाओ outshadow;
	}

	/* salt must be at least two characters long */
	अगर (!(spw->sp_pwdp[0] && spw->sp_pwdp[1])) {
		ret = -1;
		जाओ outshadow;
	}

	/* encrypt the password attempt */
	password = crypt(attempt, spw->sp_pwdp);

	अगर (password && म_भेद(password, spw->sp_pwdp) == 0)
		ret = 0;
	अन्यथा
		ret = -1;
outshadow:
#पूर्ण_अगर
	वापस ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
अटल पूर्णांक check_auth()
{
	काष्ठा passwd *pw;
	uid_t uid;

#अगर_घोषित HAVE_AUDIT
	uid = audit_getloginuid();
	अगर (uid == (uid_t) -1)
		uid = getuid();
#अन्यथा
	uid = getuid();
#पूर्ण_अगर

	pw = getpwuid(uid);
	अगर (!pw) {
		eerror("cannot find your entry in the passwd file.");
		वापस (-1);
	}

	म_लिखो("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	अगर (check_password(pw->pw_name) == 0) {
		वापस 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	वापस 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
अटल पूर्णांक read_context_file(स्थिर अक्षर *filename, अक्षर **context)
{
	पूर्णांक ret = -1;
	खाता *fp;
	अक्षर *filepath = शून्य;
	अक्षर *पंक्ति = शून्य;
	अक्षर *p;
	अक्षर *p2;
	माप_प्रकार len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_path(), filename);

	fp = ख_खोलो(filepath, "r");
	अगर (fp == शून्य) {
		eerror("Failed to open context file: %s", filename);
		मुक्त(filepath);
		वापस -1;
	}

	जबतक (xgetline(&पंक्ति, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = पंक्ति;
		जबतक (है_खाली(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		अगर (*p == '\0')
			जारी;

		/* cut off spaces after the string */
		p2 = p;
		जबतक (!है_खाली(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		अवरोध;
	}

	मुक्त(पंक्ति);
	मुक्त(filepath);
	ख_बंद(fp);
	वापस ret;
}

अटल पूर्णांक read_run_init_context(अक्षर **context)
{
	पूर्णांक ret = -1;
	RC_STRINGLIST *list;
	अक्षर *मूल्य = शून्य;

	list = rc_config_list(selinux_openrc_contexts_path());
	अगर (list == शून्य)
		वापस ret;

	मूल्य = rc_config_मूल्य(list, "run_init");
	अगर (मूल्य != शून्य && म_माप(मूल्य) > 0) {
		*context = xstrdup(मूल्य);
		ret = 0;
	}

	rc_stringlist_free(list);
	वापस ret;
}

व्योम selinux_setup(अक्षर **argv)
{
	अक्षर *new_context = शून्य;
	अक्षर *curr_context = शून्य;
	context_t curr_con;
	अक्षर *curr_t = शून्य;
	अक्षर *run_init_t = शून्य;

	/* Return, if selinux is disabled. */
	अगर (is_selinux_enabled() < 1) {
		वापस;
	}

	अगर (read_run_init_context(&run_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		run_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", run_init_t);
	}

	/* Get our current context. */
	अगर (getcon(&curr_context) < 0) {
		अगर (त्रुटि_सं == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			जाओ out;
		} अन्यथा {
			लिखो_त्रुटि("getcon");
			निकास(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	अगर (!curr_con) {
		मुक्त(curr_context);
		जाओ out;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	अगर (!curr_t) {
		context_free(curr_con);
		मुक्त(curr_context);
		जाओ out;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	मुक्त(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	अगर (म_भेदन(run_init_t, curr_t, म_माप(run_init_t)) != 0) {
		जाओ out;
	}

	मुक्त(curr_t);
	मुक्त(run_init_t);

	अगर (check_auth() != 0) {
		eerrorx("Authentication failed.");
	}

	/* Get the context for the script to be run in. */
	अगर (read_context_file(INITRC_FILE, &new_context) != 0) {
		/* assume a reasonable default, rather than bailing out */
		new_context = xstrdup("system_u:system_r:initrc_t");
		ewarn("Assuming SELinux initrc context is %s", new_context);
	}

	/* Set the new context */
	अगर (setexeccon(new_context) < 0) {
		eerrorx("Could not set SELinux exec context to %s.", new_context);
	}

	मुक्त(new_context);

	/*
	 * exec will recycle ptys so try and use open_init_pty if it exists
	 * which will open the pty with initrc_devpts_t, if it doesn't exist,
	 * fall back to plain exec
	 */
	अगर (access("/usr/sbin/open_init_pty", X_OK) == 0) {
		अगर (execvp("/usr/sbin/open_init_pty", argv)) {
			लिखो_त्रुटि("execvp");
			निकास(-1);
		}
	} अन्यथा अगर (execvp(argv[1], argv + 1)) {
		लिखो_त्रुटि("execvp");
		निकास(-1);
	}

out:
	मुक्त(run_init_t);
	मुक्त(curr_t);
}
