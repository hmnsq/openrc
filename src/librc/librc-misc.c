/*
 * rc-misc.c
 * rc misc functions
*/

/*
 * Copyright (c) 2007-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#समावेश <fnmatch.h>
#समावेश <ctype.h>
#समावेश <dirent.h>
#समावेश <errno.h>
#समावेश <limits.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <sys/stat.h>

#समावेश "queue.h"
#समावेश "librc.h"
#समावेश "helpers.h"
#समावेश "misc.h"

bool
rc_yesno(स्थिर अक्षर *मूल्य)
{
	अगर (!मूल्य) {
		त्रुटि_सं = ENOENT;
		वापस false;
	}

	अगर (strcasecmp(मूल्य, "yes") == 0 ||
	    strcasecmp(मूल्य, "y") == 0 ||
	    strcasecmp(मूल्य, "true") == 0 ||
	    strcasecmp(मूल्य, "1") == 0)
		वापस true;

	अगर (strcasecmp(मूल्य, "no") != 0 &&
	    strcasecmp(मूल्य, "n") != 0 &&
	    strcasecmp(मूल्य, "false") != 0 &&
	    strcasecmp(मूल्य, "0") != 0)
		त्रुटि_सं = EINVAL;

	वापस false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(स्थिर अक्षर *file, अक्षर **बफर, माप_प्रकार *len)
{
	bool ret = false;
	खाता *fp;
	पूर्णांक fd;
	काष्ठा stat st;
	माप_प्रकार done, बायाँ;

	fp = ख_खोलो(file, "re");
	अगर (!fp)
		वापस false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	अगर (ख_स्थिति(fd, &st))
		जाओ finished;

	बायाँ = st.st_size;
	*len = बायाँ + 1; /* NUL terminator */
	*बफर = xrealloc(*बफर, *len);
	जबतक (बायाँ) {
		done = ख_पढ़ो(*बफर, माप(*बफर[0]), बायाँ, fp);
		अगर (done == 0 && ख_त्रुटि(fp))
			जाओ finished;
		बायाँ -= done;
	}
	ret = true;

 finished:
	अगर (!ret) {
		मुक्त(*बफर);
		*len = 0;
	} अन्यथा
		(*बफर)[*len - 1] = '\0';
	ख_बंद(fp);
	वापस ret;
}

अक्षर *
rc_proc_getent(स्थिर अक्षर *ent RC_UNUSED)
{
#अगर_घोषित __linux__
	खाता *fp;
	अक्षर *proc = शून्य, *p, *मूल्य = शून्य, *save;
	माप_प्रकार i, len;

	अगर (!(fp = ख_खोलो("/proc/cmdline", "r")))
		वापस शून्य;

	i = 0;
	अगर (xgetline(&proc, &i, fp) == -1)
		जाओ out;
	save = proc;

	len = म_माप(ent);
	जबतक ((p = strsep(&save, " "))) {
		अगर (म_भेदन(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			अगर (*p == '=')
				p++;
			मूल्य = xstrdup(p);
		}
	}

	अगर (!मूल्य)
		त्रुटि_सं = ENOENT;

out:
	ख_बंद(fp);
	मुक्त(proc);

	वापस मूल्य;
#अन्यथा
	वापस शून्य;
#पूर्ण_अगर
}

RC_STRINGLIST *
config_list(पूर्णांक dirfd, स्थिर अक्षर *pathname)
{
	खाता *fp;
	अक्षर *बफर = शून्य;
	माप_प्रकार len = 0;
	अक्षर *p;
	अक्षर *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	अगर (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		वापस list;

	जबतक (xgetline(&बफर, &len, fp) != -1) {
		p = बफर;
		/* Strip leading spaces/tabs */
		जबतक ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		अगर (token && (म_माप(token) > 1)) {
			/* If not variable assignment then skip */
			अगर (म_अक्षर(token, '=')) {
				/* Strip the newline if present */
				अगर (token[म_माप(token) - 1] == '\n')
					token[म_माप(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	मुक्त(बफर);
	ख_बंद(fp);

	वापस list;
}

RC_STRINGLIST *
rc_config_list(स्थिर अक्षर *file)
{
	वापस config_list(AT_FDCWD, file);
}

अटल व्योम rc_config_set_मूल्य(RC_STRINGLIST *config, अक्षर *मूल्य)
{
	RC_STRING *cline;
	अक्षर *entry;
	माप_प्रकार i = 0;
	अक्षर *newline;
	अक्षर *p = मूल्य;
	bool replaced;
	अक्षर *token;

	अगर (!p)
		वापस;
	अगर (म_भेदन(p, "export ", 7) == 0)
		p += 7;
	अगर (!(token = strsep(&p, "=")))
		वापस;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	अगर (*p == '$')
		token = मूल्य;
	अन्यथा
		करो {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} जबतक (token && *token == '\0');

	/* Drop a newline if that's all we have */
	अगर (token) {
		i = म_माप(token) - 1;
		अगर (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", entry, token);
	} अन्यथा {
		xasprintf(&newline, "%s=", entry);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = म_माप(entry);
		अगर (म_भेदन(entry, cline->मूल्य, i) == 0 && cline->मूल्य[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			मुक्त(cline->मूल्य);
			cline->मूल्य = newline;
			replaced = true;
			अवरोध;
		}
	}

	अगर (!replaced) {
		rc_stringlist_add(config, newline);
		मुक्त(newline);
	}
	मुक्त(entry);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
अटल RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#अगर_घोषित __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	अक्षर *tmp = शून्य;
	अक्षर *मूल्य = शून्य;
	माप_प्रकार varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = म_माप(override->मूल्य);
		मूल्य = rc_proc_getent(override->मूल्य);

		/* No need to continue if there's nothing to override */
		अगर (!मूल्य) {
			मुक्त(मूल्य);
			जारी;
		}

		अगर (मूल्य != शून्य) {
			xasprintf(&tmp, "%s=%s", override->मूल्य, मूल्य);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			अगर (म_भेदन(override->मूल्य, cline->मूल्य, varlen) == 0
				&& cline->मूल्य[varlen] == '=') {
				rc_stringlist_delete(config, cline->मूल्य);
				अवरोध;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		मुक्त(tmp);
		मुक्त(मूल्य);
	}

	rc_stringlist_free(overrides);
#पूर्ण_अगर
	वापस config;
}

अटल व्योम
rc_config_directory(RC_STRINGLIST *config, पूर्णांक targetfd, स्थिर अक्षर *dir)
{
	सूची *dp;
	काष्ठा dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *पंक्ति;

	अगर (!(dp = do_opendirat(targetfd, dir)))
		वापस;

	rc_conf_d_files = rc_stringlist_new();
	जबतक ((d = सूची_पढ़ो(dp)) != शून्य) {
		अगर (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		अगर (!fname->मूल्य)
			जारी;

		rc_conf_d_list = config_list(dirfd(dp), fname->मूल्य);
		TAILQ_FOREACH(पंक्ति, rc_conf_d_list, entries)
			अगर (पंक्ति->मूल्य)
				rc_config_set_मूल्य(config, पंक्ति->मूल्य);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	बंद_सूची(dp);

	वापस;
}

अटल RC_STRINGLIST *
config_load(पूर्णांक dirfd, स्थिर अक्षर *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *पंक्ति;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(पंक्ति, list, entries) {
		rc_config_set_मूल्य(config, पंक्ति->मूल्य);
	}
	rc_stringlist_free(list);

	वापस config;
}

RC_STRINGLIST *
rc_config_load(स्थिर अक्षर *file)
{
	वापस config_load(AT_FDCWD, file);
}

अक्षर *
rc_config_मूल्य(RC_STRINGLIST *list, स्थिर अक्षर *entry)
{
	RC_STRING *पंक्ति;
	अक्षर *p;
	माप_प्रकार len;

	len = म_माप(entry);
	TAILQ_FOREACH(पंक्ति, list, entries) {
		p = म_अक्षर(पंक्ति->मूल्य, '=');
		अगर (p != शून्य) {
			अगर (म_भेदन(entry, पंक्ति->मूल्य, len) == 0 && पंक्ति->मूल्य[len] == '=')
				वापस ++p;
		}
	}
	वापस शून्य;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
अटल RC_STRINGLIST *rc_conf = शून्य;

अटल व्योम
_free_rc_conf(व्योम)
{
	rc_stringlist_free(rc_conf);
}

अटल व्योम
rc_conf_append(पूर्णांक dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

अक्षर *
rc_conf_मूल्य(स्थिर अक्षर *setting)
{
	स्थिर पूर्णांक *dirfds;
	माप_प्रकार dir_गिनती = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	अगर (rc_conf)
		वापस rc_config_मूल्य(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	निकास_पर(_free_rc_conf);

	क्रम (माप_प्रकार i = 0; i < dir_गिनती; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		माप_प्रकार idx = dir_गिनती - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	अगर (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		अक्षर *p = s->मूल्य;
		जबतक (p && *p && *p != '=') {
			अगर (है_बड़ा((अचिन्हित अक्षर)*p))
				*p = छोटे((अचिन्हित अक्षर)*p);
			p++;
		}
	}

	वापस rc_config_मूल्य(rc_conf, setting);
}
