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

#अन्तर्भाव <fnmatch.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <dirent.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <sys/stat.h>

#अन्तर्भाव "queue.h"
#अन्तर्भाव "librc.h"
#अन्तर्भाव "helpers.h"
#अन्तर्भाव "misc.h"

bool
rc_yesno(स्थिरांक अक्षर *मूल्य)
{
	जर (!मूल्य) {
		त्रुटि_सं = ENOENT;
		परत false;
	}

	जर (strcasecmp(मूल्य, "yes") == 0 ||
	    strcasecmp(मूल्य, "y") == 0 ||
	    strcasecmp(मूल्य, "true") == 0 ||
	    strcasecmp(मूल्य, "1") == 0)
		परत true;

	जर (strcasecmp(मूल्य, "no") != 0 &&
	    strcasecmp(मूल्य, "n") != 0 &&
	    strcasecmp(मूल्य, "false") != 0 &&
	    strcasecmp(मूल्य, "0") != 0)
		त्रुटि_सं = EINVAL;

	परत false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(स्थिरांक अक्षर *file, अक्षर **buffer, माप_प्रकार *len)
{
	bool ret = false;
	खाता *fp;
	पूर्णांक fd;
	संरचना stat st;
	माप_प्रकार done, डावा;

	fp = ख_खोलो(file, "re");
	जर (!fp)
		परत false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	जर (ख_स्थिति(fd, &st))
		जा finished;

	डावा = st.st_size;
	*len = डावा + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	पर्यंत (डावा) {
		done = ख_पढ़ो(*buffer, आकार(*buffer[0]), डावा, fp);
		जर (done == 0 && ख_त्रुटि(fp))
			जा finished;
		डावा -= done;
	}
	ret = true;

 finished:
	जर (!ret) {
		मुक्त(*buffer);
		*len = 0;
	} अन्यथा
		(*buffer)[*len - 1] = '\0';
	ख_बंद(fp);
	परत ret;
}

अक्षर *
rc_proc_getent(स्थिरांक अक्षर *ent RC_UNUSED)
{
#ifdef __linux__
	खाता *fp;
	अक्षर *proc = रिक्त, *p, *मूल्य = रिक्त, *save;
	माप_प्रकार i, len;

	जर (!(fp = ख_खोलो("/proc/cmdline", "r")))
		परत रिक्त;

	i = 0;
	जर (xgetline(&proc, &i, fp) == -1)
		जा out;
	save = proc;

	len = म_माप(ent);
	पर्यंत ((p = strsep(&save, " "))) {
		जर (म_भेदन(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			जर (*p == '=')
				p++;
			मूल्य = xstrdup(p);
		}
	}

	जर (!मूल्य)
		त्रुटि_सं = ENOENT;

out:
	ख_बंद(fp);
	मुक्त(proc);

	परत मूल्य;
#अन्यथा
	परत रिक्त;
#endif
}

RC_STRINGLIST *
config_list(पूर्णांक dirfd, स्थिरांक अक्षर *pathname)
{
	खाता *fp;
	अक्षर *buffer = रिक्त;
	माप_प्रकार len = 0;
	अक्षर *p;
	अक्षर *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	जर (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		परत list;

	पर्यंत (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		पर्यंत ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		जर (token && (म_माप(token) > 1)) {
			/* If not variable assignment then skip */
			जर (म_अक्षर(token, '=')) {
				/* Strip the newline if present */
				जर (token[म_माप(token) - 1] == '\n')
					token[म_माप(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	मुक्त(buffer);
	ख_बंद(fp);

	परत list;
}

RC_STRINGLIST *
rc_config_list(स्थिरांक अक्षर *file)
{
	परत config_list(AT_FDCWD, file);
}

स्थिर शून्य rc_config_set_मूल्य(RC_STRINGLIST *config, अक्षर *मूल्य)
{
	RC_STRING *cline;
	अक्षर *entry;
	माप_प्रकार i = 0;
	अक्षर *newline;
	अक्षर *p = मूल्य;
	bool replaced;
	अक्षर *token;

	जर (!p)
		परत;
	जर (म_भेदन(p, "export ", 7) == 0)
		p += 7;
	जर (!(token = strsep(&p, "=")))
		परत;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	जर (*p == '$')
		token = मूल्य;
	अन्यथा
		करा {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} पर्यंत (token && *token == '\0');

	/* Drop a newline if that's all we have */
	जर (token) {
		i = म_माप(token) - 1;
		जर (token[i] == '\n')
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
		जर (म_भेदन(entry, cline->मूल्य, i) == 0 && cline->मूल्य[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			मुक्त(cline->मूल्य);
			cline->मूल्य = newline;
			replaced = true;
			थांबा;
		}
	}

	जर (!replaced) {
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
स्थिर RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#ifdef __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	अक्षर *tmp = रिक्त;
	अक्षर *मूल्य = रिक्त;
	माप_प्रकार varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = म_माप(override->मूल्य);
		मूल्य = rc_proc_getent(override->मूल्य);

		/* No need to continue if there's nothing to override */
		जर (!मूल्य) {
			मुक्त(मूल्य);
			सुरूठेवा;
		}

		जर (मूल्य != रिक्त) {
			xasprintf(&tmp, "%s=%s", override->मूल्य, मूल्य);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			जर (म_भेदन(override->मूल्य, cline->मूल्य, varlen) == 0
				&& cline->मूल्य[varlen] == '=') {
				rc_stringlist_delete(config, cline->मूल्य);
				थांबा;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		मुक्त(tmp);
		मुक्त(मूल्य);
	}

	rc_stringlist_free(overrides);
#endif
	परत config;
}

स्थिर शून्य
rc_config_directory(RC_STRINGLIST *config, पूर्णांक targetfd, स्थिरांक अक्षर *dir)
{
	सूची *dp;
	संरचना dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *ओळ;

	जर (!(dp = do_opendirat(targetfd, dir)))
		परत;

	rc_conf_d_files = rc_stringlist_new();
	पर्यंत ((d = सूची_पढ़ो(dp)) != रिक्त) {
		जर (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		जर (!fname->मूल्य)
			सुरूठेवा;

		rc_conf_d_list = config_list(dirfd(dp), fname->मूल्य);
		TAILQ_FOREACH(ओळ, rc_conf_d_list, entries)
			जर (ओळ->मूल्य)
				rc_config_set_मूल्य(config, ओळ->मूल्य);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	बंद_सूची(dp);

	परत;
}

स्थिर RC_STRINGLIST *
config_load(पूर्णांक dirfd, स्थिरांक अक्षर *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *ओळ;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(ओळ, list, entries) {
		rc_config_set_मूल्य(config, ओळ->मूल्य);
	}
	rc_stringlist_free(list);

	परत config;
}

RC_STRINGLIST *
rc_config_load(स्थिरांक अक्षर *file)
{
	परत config_load(AT_FDCWD, file);
}

अक्षर *
rc_config_मूल्य(RC_STRINGLIST *list, स्थिरांक अक्षर *entry)
{
	RC_STRING *ओळ;
	अक्षर *p;
	माप_प्रकार len;

	len = म_माप(entry);
	TAILQ_FOREACH(ओळ, list, entries) {
		p = म_अक्षर(ओळ->मूल्य, '=');
		जर (p != रिक्त) {
			जर (म_भेदन(entry, ओळ->मूल्य, len) == 0 && ओळ->मूल्य[len] == '=')
				परत ++p;
		}
	}
	परत रिक्त;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
स्थिर RC_STRINGLIST *rc_conf = रिक्त;

स्थिर शून्य
_free_rc_conf(शून्य)
{
	rc_stringlist_free(rc_conf);
}

स्थिर शून्य
rc_conf_append(पूर्णांक dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

अक्षर *
rc_conf_मूल्य(स्थिरांक अक्षर *setting)
{
	स्थिरांक पूर्णांक *dirfds;
	माप_प्रकार dir_मोजणी = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	जर (rc_conf)
		परत rc_config_मूल्य(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	निकास_पर(_free_rc_conf);

	साठी (माप_प्रकार i = 0; i < dir_मोजणी; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		माप_प्रकार idx = dir_मोजणी - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	जर (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		अक्षर *p = s->मूल्य;
		पर्यंत (p && *p && *p != '=') {
			जर (है_बड़ा((अचिन्हित अक्षर)*p))
				*p = छोटे((अचिन्हित अक्षर)*p);
			p++;
		}
	}

	परत rc_config_मूल्य(rc_conf, setting);
}
