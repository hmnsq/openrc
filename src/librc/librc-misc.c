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
rc_yesno(अचर वर्ण *मान)
{
	यदि (!मान) {
		त्रुटि_सं = ENOENT;
		फर्काऊ false;
	}

	यदि (strcasecmp(मान, "yes") == 0 ||
	    strcasecmp(मान, "y") == 0 ||
	    strcasecmp(मान, "true") == 0 ||
	    strcasecmp(मान, "1") == 0)
		फर्काऊ true;

	यदि (strcasecmp(मान, "no") != 0 &&
	    strcasecmp(मान, "n") != 0 &&
	    strcasecmp(मान, "false") != 0 &&
	    strcasecmp(मान, "0") != 0)
		त्रुटि_सं = EINVAL;

	फर्काऊ false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(अचर वर्ण *file, वर्ण **buffer, माप_प्रकार *len)
{
	bool ret = false;
	खाता *fp;
	पूर्णाङ्क fd;
	संरचना stat st;
	माप_प्रकार done, बायाँ;

	fp = ख_खोलो(file, "re");
	यदि (!fp)
		फर्काऊ false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	यदि (ख_स्थिति(fd, &st))
		गच्छ finished;

	बायाँ = st.st_size;
	*len = बायाँ + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	जबसम्म (बायाँ) {
		done = ख_पढ़ो(*buffer, परिमाण(*buffer[0]), बायाँ, fp);
		यदि (done == 0 && ख_त्रुटि(fp))
			गच्छ finished;
		बायाँ -= done;
	}
	ret = true;

 finished:
	यदि (!ret) {
		मुक्त(*buffer);
		*len = 0;
	} अन्यथा
		(*buffer)[*len - 1] = '\0';
	ख_बंद(fp);
	फर्काऊ ret;
}

वर्ण *
rc_proc_getent(अचर वर्ण *ent RC_UNUSED)
{
#ifdef __linux__
	खाता *fp;
	वर्ण *proc = रिक्त, *p, *मान = रिक्त, *save;
	माप_प्रकार i, len;

	यदि (!(fp = ख_खोलो("/proc/cmdline", "r")))
		फर्काऊ रिक्त;

	i = 0;
	यदि (xgetline(&proc, &i, fp) == -1)
		गच्छ out;
	save = proc;

	len = म_माप(ent);
	जबसम्म ((p = strsep(&save, " "))) {
		यदि (म_भेदन(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			यदि (*p == '=')
				p++;
			मान = xstrdup(p);
		}
	}

	यदि (!मान)
		त्रुटि_सं = ENOENT;

out:
	ख_बंद(fp);
	मुक्त(proc);

	फर्काऊ मान;
#अन्यथा
	फर्काऊ रिक्त;
#endif
}

RC_STRINGLIST *
config_list(पूर्णाङ्क dirfd, अचर वर्ण *pathname)
{
	खाता *fp;
	वर्ण *buffer = रिक्त;
	माप_प्रकार len = 0;
	वर्ण *p;
	वर्ण *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	यदि (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		फर्काऊ list;

	जबसम्म (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		जबसम्म ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		यदि (token && (म_माप(token) > 1)) {
			/* If not variable assignment then skip */
			यदि (म_अक्षर(token, '=')) {
				/* Strip the newline if present */
				यदि (token[म_माप(token) - 1] == '\n')
					token[म_माप(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	मुक्त(buffer);
	ख_बंद(fp);

	फर्काऊ list;
}

RC_STRINGLIST *
rc_config_list(अचर वर्ण *file)
{
	फर्काऊ config_list(AT_FDCWD, file);
}

स्थिर शून्य rc_config_set_मान(RC_STRINGLIST *config, वर्ण *मान)
{
	RC_STRING *cline;
	वर्ण *entry;
	माप_प्रकार i = 0;
	वर्ण *newline;
	वर्ण *p = मान;
	bool replaced;
	वर्ण *token;

	यदि (!p)
		फर्काऊ;
	यदि (म_भेदन(p, "export ", 7) == 0)
		p += 7;
	यदि (!(token = strsep(&p, "=")))
		फर्काऊ;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	यदि (*p == '$')
		token = मान;
	अन्यथा
		गर {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} जबसम्म (token && *token == '\0');

	/* Drop a newline if that's all we have */
	यदि (token) {
		i = म_माप(token) - 1;
		यदि (token[i] == '\n')
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
		यदि (म_भेदन(entry, cline->मान, i) == 0 && cline->मान[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			मुक्त(cline->मान);
			cline->मान = newline;
			replaced = true;
			रोक;
		}
	}

	यदि (!replaced) {
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
	वर्ण *tmp = रिक्त;
	वर्ण *मान = रिक्त;
	माप_प्रकार varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = म_माप(override->मान);
		मान = rc_proc_getent(override->मान);

		/* No need to continue if there's nothing to override */
		यदि (!मान) {
			मुक्त(मान);
			जारी;
		}

		यदि (मान != रिक्त) {
			xasprintf(&tmp, "%s=%s", override->मान, मान);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			यदि (म_भेदन(override->मान, cline->मान, varlen) == 0
				&& cline->मान[varlen] == '=') {
				rc_stringlist_delete(config, cline->मान);
				रोक;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		मुक्त(tmp);
		मुक्त(मान);
	}

	rc_stringlist_free(overrides);
#endif
	फर्काऊ config;
}

स्थिर शून्य
rc_config_directory(RC_STRINGLIST *config, पूर्णाङ्क targetfd, अचर वर्ण *dir)
{
	सूची *dp;
	संरचना dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *पङ्क्ति;

	यदि (!(dp = do_opendirat(targetfd, dir)))
		फर्काऊ;

	rc_conf_d_files = rc_stringlist_new();
	जबसम्म ((d = सूची_पढ़ो(dp)) != रिक्त) {
		यदि (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		यदि (!fname->मान)
			जारी;

		rc_conf_d_list = config_list(dirfd(dp), fname->मान);
		TAILQ_FOREACH(पङ्क्ति, rc_conf_d_list, entries)
			यदि (पङ्क्ति->मान)
				rc_config_set_मान(config, पङ्क्ति->मान);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	बंद_सूची(dp);

	फर्काऊ;
}

स्थिर RC_STRINGLIST *
config_load(पूर्णाङ्क dirfd, अचर वर्ण *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *पङ्क्ति;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(पङ्क्ति, list, entries) {
		rc_config_set_मान(config, पङ्क्ति->मान);
	}
	rc_stringlist_free(list);

	फर्काऊ config;
}

RC_STRINGLIST *
rc_config_load(अचर वर्ण *file)
{
	फर्काऊ config_load(AT_FDCWD, file);
}

वर्ण *
rc_config_मान(RC_STRINGLIST *list, अचर वर्ण *entry)
{
	RC_STRING *पङ्क्ति;
	वर्ण *p;
	माप_प्रकार len;

	len = म_माप(entry);
	TAILQ_FOREACH(पङ्क्ति, list, entries) {
		p = म_अक्षर(पङ्क्ति->मान, '=');
		यदि (p != रिक्त) {
			यदि (म_भेदन(entry, पङ्क्ति->मान, len) == 0 && पङ्क्ति->मान[len] == '=')
				फर्काऊ ++p;
		}
	}
	फर्काऊ रिक्त;
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
rc_conf_append(पूर्णाङ्क dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

वर्ण *
rc_conf_मान(अचर वर्ण *setting)
{
	अचर पूर्णाङ्क *dirfds;
	माप_प्रकार dir_गन्ती = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	यदि (rc_conf)
		फर्काऊ rc_config_मान(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	निकास_पर(_free_rc_conf);

	लागि (माप_प्रकार i = 0; i < dir_गन्ती; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		माप_प्रकार idx = dir_गन्ती - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	यदि (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		वर्ण *p = s->मान;
		जबसम्म (p && *p && *p != '=') {
			यदि (है_बड़ा((अचिन्हित वर्ण)*p))
				*p = छोटे((अचिन्हित वर्ण)*p);
			p++;
		}
	}

	फर्काऊ rc_config_मान(rc_conf, setting);
}
