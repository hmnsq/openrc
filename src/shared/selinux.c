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

पूर्णाङ्क selinux_util_label(अचर वर्ण *path)
{
	पूर्णाङ्क retval = 0;
	पूर्णाङ्क enforce;
	संरचना stat st;
	वर्ण *con;

	enforce = security_getenforce();
	यदि (retval < 0)
		फर्काऊ retval;

	यदि (!hnd)
		फर्काऊ (enforce) ? -1 : 0;

	retval = lstat(path, &st);
	यदि (retval < 0) {
		यदि (त्रुटि_सं == ENOENT)
			फर्काऊ 0;
		फर्काऊ (enforce) ? -1 : 0;
	}

	/* lookup the context */
	retval = selabel_lookup_raw(hnd, &con, path, st.st_mode);
	यदि (retval < 0) {
		यदि (त्रुटि_सं == ENOENT)
			फर्काऊ 0;
		फर्काऊ (enforce) ? -1 : 0;
	}

	/* apply the context */
	retval = lsetfilecon(path, con);
	freecon(con);
	यदि (retval < 0) {
		यदि (त्रुटि_सं == ENOENT)
			फर्काऊ 0;
		यदि (त्रुटि_सं == ENOTSUP)
			फर्काऊ 0;
		फर्काऊ (enforce) ? -1 : 0;
	}

	फर्काऊ 0;
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
		फर्काऊ retval;

	hnd = selabel_open(SELABEL_CTX_FILE, रिक्त, 0);
	यदि (!hnd)
		फर्काऊ -2;

	फर्काऊ 1;
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
		फर्काऊ retval;

	यदि (hnd) {
		selabel_close(hnd);
		hnd = रिक्त;
	}

	फर्काऊ 0;
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
#ifdef HAVE_PAM
	pam_handle_t *pamh;
	पूर्णाङ्क pam_err = 0;
	अचर संरचना pam_conv pconv = {
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
	pam_end(pamh, pam_err);
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

	यदि (password && म_भेद(password, spw->sp_pwdp) == 0)
		ret = 0;
	अन्यथा
		ret = -1;
outshadow:
#endif
	फर्काऊ ret;
}

/* Authenticates the user, returns 0 on success, 1 on fail */
स्थिर पूर्णाङ्क check_auth()
{
	संरचना passwd *pw;
	uid_t uid;

#ifdef HAVE_AUDIT
	uid = audit_getloginuid();
	यदि (uid == (uid_t) -1)
		uid = getuid();
#अन्यथा
	uid = getuid();
#endif

	pw = getpwuid(uid);
	यदि (!pw) {
		eerror("cannot find your entry in the passwd file.");
		फर्काऊ (-1);
	}

	म_लिखो("Authenticating %s.\n", pw->pw_name);

	/* do the actual check */
	यदि (check_password(pw->pw_name) == 0) {
		फर्काऊ 0;
	}

	eerrorx("Authentication failed for %s", pw->pw_name);
	फर्काऊ 1;
}

/*
 * Read the context from the given context file. context must be free'd by the user.
 */
स्थिर पूर्णाङ्क read_context_file(अचर वर्ण *filename, वर्ण **context)
{
	पूर्णाङ्क ret = -1;
	खाता *fp;
	वर्ण *filepath = रिक्त;
	वर्ण *पङ्क्ति = रिक्त;
	वर्ण *p;
	वर्ण *p2;
	माप_प्रकार len = 0;

	xasprintf(&filepath, "%s/%s", selinux_contexts_path(), filename);

	fp = ख_खोलो(filepath, "r");
	यदि (fp == रिक्त) {
		eerror("Failed to open context file: %s", filename);
		मुक्त(filepath);
		फर्काऊ -1;
	}

	जबसम्म (xgetline(&पङ्क्ति, &len, fp) != -1) {
		/* cut off spaces before the string */
		p = पङ्क्ति;
		जबसम्म (है_खाली(*p) && *p != '\0')
			p++;

		/* empty string, skip */
		यदि (*p == '\0')
			जारी;

		/* cut off spaces after the string */
		p2 = p;
		जबसम्म (!है_खाली(*p2) && *p2 != '\0')
			p2++;
		*p2 = '\0';

		*context = xstrdup(p);
		ret = 0;
		रोक;
	}

	मुक्त(पङ्क्ति);
	मुक्त(filepath);
	ख_बंद(fp);
	फर्काऊ ret;
}

स्थिर पूर्णाङ्क read_run_init_context(वर्ण **context)
{
	पूर्णाङ्क ret = -1;
	RC_STRINGLIST *list;
	वर्ण *मान = रिक्त;

	list = rc_config_list(selinux_openrc_contexts_path());
	यदि (list == रिक्त)
		फर्काऊ ret;

	मान = rc_config_मान(list, "run_init");
	यदि (मान != रिक्त && म_माप(मान) > 0) {
		*context = xstrdup(मान);
		ret = 0;
	}

	rc_stringlist_free(list);
	फर्काऊ ret;
}

शून्य selinux_setup(वर्ण **argv)
{
	वर्ण *new_context = रिक्त;
	वर्ण *curr_context = रिक्त;
	context_t curr_con;
	वर्ण *curr_t = रिक्त;
	वर्ण *run_init_t = रिक्त;

	/* Return, if selinux is disabled. */
	यदि (is_selinux_enabled() < 1) {
		फर्काऊ;
	}

	यदि (read_run_init_context(&run_init_t) != 0) {
		/* assume a reasonable default, rather than bailing out */
		run_init_t = xstrdup("run_init_t");
		ewarn("Assuming SELinux run_init type is %s", run_init_t);
	}

	/* Get our current context. */
	यदि (getcon(&curr_context) < 0) {
		यदि (त्रुटि_सं == ENOENT) {
			/* should only hit this if proc is not mounted.  this
			 * happens on Gentoo right after init starts, when
			 * the init script processing starts.
			 */
			गच्छ out;
		} अन्यथा {
			लिखो_त्रुटि("getcon");
			निकास(1);
		}
	}

	/* extract the type from the context */
	curr_con = context_new(curr_context);
	यदि (!curr_con) {
		मुक्त(curr_context);
		गच्छ out;
	}

	curr_t = xstrdup(context_type_get(curr_con));
	यदि (!curr_t) {
		context_free(curr_con);
		मुक्त(curr_context);
		गच्छ out;
	}

	/* dont need them anymore so free() now */
	context_free(curr_con);
	मुक्त(curr_context);

	/* if we are not in the run_init domain, we should not do anything */
	यदि (म_भेदन(run_init_t, curr_t, म_माप(run_init_t)) != 0) {
		गच्छ out;
	}

	मुक्त(curr_t);
	मुक्त(run_init_t);

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
			लिखो_त्रुटि("execvp");
			निकास(-1);
		}
	} अन्यथा यदि (execvp(argv[1], argv + 1)) {
		लिखो_त्रुटि("execvp");
		निकास(-1);
	}

out:
	मुक्त(run_init_t);
	मुक्त(curr_t);
}
