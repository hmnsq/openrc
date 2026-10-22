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
rc_yesno(ध्रुव वर्ण *मानम्)
{
	यदि (!मानम्) {
		errno = ENOENT;
		प्रत्यागम false;
	}

	यदि (strcasecmp(मानम्, "yes") == 0 ||
	    strcasecmp(मानम्, "y") == 0 ||
	    strcasecmp(मानम्, "true") == 0 ||
	    strcasecmp(मानम्, "1") == 0)
		प्रत्यागम true;

	यदि (strcasecmp(मानम्, "no") != 0 &&
	    strcasecmp(मानम्, "n") != 0 &&
	    strcasecmp(मानम्, "false") != 0 &&
	    strcasecmp(मानम्, "0") != 0)
		errno = EINVAL;

	प्रत्यागम false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(ध्रुव वर्ण *file, वर्ण **बफर, size_t *len)
{
	bool ret = false;
	FILE *fp;
	पूर्णाङ्क fd;
	संरचना stat st;
	size_t समाप्तम्, वामः;

	fp = fopen(file, "re");
	यदि (!fp)
		प्रत्यागम false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	यदि (fstat(fd, &st))
		गच्छ परिसमाप्तम्;

	वामः = st.st_size;
	*len = वामः + 1; /* NUL terminator */
	*बफर = xrealloc(*बफर, *len);
	यावत् (वामः) {
		समाप्तम् = fread(*बफर, परिमाण(*बफर[0]), वामः, fp);
		यदि (समाप्तम् == 0 && ferror(fp))
			गच्छ परिसमाप्तम्;
		वामः -= समाप्तम्;
	}
	ret = true;

 परिसमाप्तम्:
	यदि (!ret) {
		मुक्त(*बफर);
		*len = 0;
	} अन्यथा
		(*बफर)[*len - 1] = '\0';
	fclose(fp);
	प्रत्यागम ret;
}

वर्ण *
rc_proc_getent(ध्रुव वर्ण *ent RC_अप्रयुक्तम्)
{
#यदि_परिभाषितम् __linux__
	FILE *fp;
	वर्ण *proc = रिक्त, *p, *मानम् = रिक्त, *save;
	size_t i, len;

	यदि (!(fp = fopen("/proc/cmdline", "r")))
		प्रत्यागम रिक्त;

	i = 0;
	यदि (xgetline(&proc, &i, fp) == -1)
		गच्छ बहिर्गमनम्;
	save = proc;

	len = वाक्यदैर्घ्यम्(ent);
	यावत् ((p = strsep(&save, " "))) {
		यदि (वाक्ये_तुलय(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			यदि (*p == '=')
				p++;
			मानम् = xstrdup(p);
		}
	}

	यदि (!मानम्)
		errno = ENOENT;

बहिर्गमनम्:
	fclose(fp);
	मुक्त(proc);

	प्रत्यागम मानम्;
#अन्यथा
	प्रत्यागम रिक्त;
#यदि_अन्तः
}

RC_STRINGLIST *
config_list(पूर्णाङ्क dirfd, ध्रुव वर्ण *pathname)
{
	FILE *fp;
	वर्ण *बफर = रिक्त;
	size_t len = 0;
	वर्ण *p;
	वर्ण *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	यदि (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		प्रत्यागम list;

	यावत् (xgetline(&बफर, &len, fp) != -1) {
		p = बफर;
		/* Strip leading spaces/tabs */
		यावत् ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		यदि (token && (वाक्यदैर्घ्यम्(token) > 1)) {
			/* If not variable assignment then skip */
			यदि (strchr(token, '=')) {
				/* Strip the newline if present */
				यदि (token[वाक्यदैर्घ्यम्(token) - 1] == '\n')
					token[वाक्यदैर्घ्यम्(token) - 1] = 0;

				rc_stringlist_सङ्कलय(list, token);
			}
		}
	}
	मुक्त(बफर);
	fclose(fp);

	प्रत्यागम list;
}

RC_STRINGLIST *
rc_config_list(ध्रुव वर्ण *file)
{
	प्रत्यागम config_list(AT_FDCWD, file);
}

स्थिर शून्य rc_config_समुच्चयः_मानम्(RC_STRINGLIST *config, वर्ण *मानम्)
{
	RC_STRING *cline;
	वर्ण *प्रविष्टिः;
	size_t i = 0;
	वर्ण *newline;
	वर्ण *p = मानम्;
	bool replaced;
	वर्ण *token;

	यदि (!p)
		प्रत्यागम;
	यदि (वाक्ये_तुलय(p, "export ", 7) == 0)
		p += 7;
	यदि (!(token = strsep(&p, "=")))
		प्रत्यागम;

	प्रविष्टिः = xstrdup(token);
	/* Preserve shell coloring */
	यदि (*p == '$')
		token = मानम्;
	अन्यथा
		कुरु {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} यावत् (token && *token == '\0');

	/* Drop a newline if that's all we have */
	यदि (token) {
		i = वाक्यदैर्घ्यम्(token) - 1;
		यदि (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", प्रविष्टिः, token);
	} अन्यथा {
		xasprintf(&newline, "%s=", प्रविष्टिः);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = वाक्यदैर्घ्यम्(प्रविष्टिः);
		यदि (वाक्ये_तुलय(प्रविष्टिः, cline->मानम्, i) == 0 && cline->मानम्[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			मुक्त(cline->मानम्);
			cline->मानम् = newline;
			replaced = true;
			भङ्ग;
		}
	}

	यदि (!replaced) {
		rc_stringlist_सङ्कलय(config, newline);
		मुक्त(newline);
	}
	मुक्त(प्रविष्टिः);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
स्थिर RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#यदि_परिभाषितम् __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	वर्ण *tmp = रिक्त;
	वर्ण *मानम् = रिक्त;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_सङ्कलय(overrides, "rc_interactive");
	rc_stringlist_सङ्कलय(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = वाक्यदैर्घ्यम्(override->मानम्);
		मानम् = rc_proc_getent(override->मानम्);

		/* No need to continue if there's nothing to override */
		यदि (!मानम्) {
			मुक्त(मानम्);
			अनुवर्त;
		}

		यदि (मानम् != रिक्त) {
			xasprintf(&tmp, "%s=%s", override->मानम्, मानम्);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			यदि (वाक्ये_तुलय(override->मानम्, cline->मानम्, varlen) == 0
				&& cline->मानम्[varlen] == '=') {
				rc_stringlist_delete(config, cline->मानम्);
				भङ्ग;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_सङ्कलय(config, tmp);

		मुक्त(tmp);
		मुक्त(मानम्);
	}

	rc_stringlist_free(overrides);
#यदि_अन्तः
	प्रत्यागम config;
}

स्थिर शून्य
rc_config_directory(RC_STRINGLIST *config, पूर्णाङ्क targetfd, ध्रुव वर्ण *dir)
{
	DIR *dp;
	संरचना dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *पङ्क्तिः;

	यदि (!(dp = do_opendirat(targetfd, dir)))
		प्रत्यागम;

	rc_conf_d_files = rc_stringlist_new();
	यावत् ((d = readdir(dp)) != रिक्त) {
		यदि (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_क्रमय(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		यदि (!fname->मानम्)
			अनुवर्त;

		rc_conf_d_list = config_list(dirfd(dp), fname->मानम्);
		TAILQ_FOREACH(पङ्क्तिः, rc_conf_d_list, entries)
			यदि (पङ्क्तिः->मानम्)
				rc_config_समुच्चयः_मानम्(config, पङ्क्तिः->मानम्);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	प्रत्यागम;
}

स्थिर RC_STRINGLIST *
config_आरोपय(पूर्णाङ्क dirfd, ध्रुव वर्ण *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *पङ्क्तिः;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(पङ्क्तिः, list, entries) {
		rc_config_समुच्चयः_मानम्(config, पङ्क्तिः->मानम्);
	}
	rc_stringlist_free(list);

	प्रत्यागम config;
}

RC_STRINGLIST *
rc_config_आरोपय(ध्रुव वर्ण *file)
{
	प्रत्यागम config_आरोपय(AT_FDCWD, file);
}

वर्ण *
rc_config_मानम्(RC_STRINGLIST *list, ध्रुव वर्ण *प्रविष्टिः)
{
	RC_STRING *पङ्क्तिः;
	वर्ण *p;
	size_t len;

	len = वाक्यदैर्घ्यम्(प्रविष्टिः);
	TAILQ_FOREACH(पङ्क्तिः, list, entries) {
		p = strchr(पङ्क्तिः->मानम्, '=');
		यदि (p != रिक्त) {
			यदि (वाक्ये_तुलय(प्रविष्टिः, पङ्क्तिः->मानम्, len) == 0 && पङ्क्तिः->मानम्[len] == '=')
				प्रत्यागम ++p;
		}
	}
	प्रत्यागम रिक्त;
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
	RC_STRINGLIST *conf = config_आरोपय(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

वर्ण *
rc_conf_मानम्(ध्रुव वर्ण *setting)
{
	ध्रुव पूर्णाङ्क *dirfds;
	size_t dir_गणनम् = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	यदि (rc_conf)
		प्रत्यागम rc_config_मानम्(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	प्रति (size_t i = 0; i < dir_गणनम्; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_गणनम् - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	यदि (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_आरोपय(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		वर्ण *p = s->मानम्;
		यावत् (p && *p && *p != '=') {
			यदि (isupper((अचिह्नित वर्ण)*p))
				*p = tolower((अचिह्नित वर्ण)*p);
			p++;
		}
	}

	प्रत्यागम rc_config_मानम्(rc_conf, setting);
}
