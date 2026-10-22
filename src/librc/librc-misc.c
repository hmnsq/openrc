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
rc_yesno(ध्रुव वर्ण *मूल्य)
{
	यदि (!मूल्य) {
		errno = ENOENT;
		प्रत्यागम false;
	}

	यदि (strcasecmp(मूल्य, "yes") == 0 ||
	    strcasecmp(मूल्य, "y") == 0 ||
	    strcasecmp(मूल्य, "true") == 0 ||
	    strcasecmp(मूल्य, "1") == 0)
		प्रत्यागम true;

	यदि (strcasecmp(मूल्य, "no") != 0 &&
	    strcasecmp(मूल्य, "n") != 0 &&
	    strcasecmp(मूल्य, "false") != 0 &&
	    strcasecmp(मूल्य, "0") != 0)
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
rc_getfile(ध्रुव वर्ण *file, वर्ण **buffer, size_t *len)
{
	bool ret = false;
	FILE *fp;
	पूर्णाङ्क fd;
	संरचना stat st;
	size_t done, डावा;

	fp = fopen(file, "re");
	यदि (!fp)
		प्रत्यागम false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	यदि (fstat(fd, &st))
		गच्छ finished;

	डावा = st.st_size;
	*len = डावा + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	यावत् (डावा) {
		done = fread(*buffer, परिमाण(*buffer[0]), डावा, fp);
		यदि (done == 0 && ferror(fp))
			गच्छ finished;
		डावा -= done;
	}
	ret = true;

 finished:
	यदि (!ret) {
		मुक्त(*buffer);
		*len = 0;
	} अन्यथा
		(*buffer)[*len - 1] = '\0';
	fclose(fp);
	प्रत्यागम ret;
}

वर्ण *
rc_proc_getent(ध्रुव वर्ण *ent RC_UNUSED)
{
#ifdef __linux__
	FILE *fp;
	वर्ण *proc = रिक्त, *p, *मूल्य = रिक्त, *save;
	size_t i, len;

	यदि (!(fp = fopen("/proc/cmdline", "r")))
		प्रत्यागम रिक्त;

	i = 0;
	यदि (xgetline(&proc, &i, fp) == -1)
		गच्छ out;
	save = proc;

	len = strlen(ent);
	यावत् ((p = strsep(&save, " "))) {
		यदि (strncmp(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			यदि (*p == '=')
				p++;
			मूल्य = xstrdup(p);
		}
	}

	यदि (!मूल्य)
		errno = ENOENT;

out:
	fclose(fp);
	मुक्त(proc);

	प्रत्यागम मूल्य;
#अन्यथा
	प्रत्यागम रिक्त;
#endif
}

RC_STRINGLIST *
config_list(पूर्णाङ्क dirfd, ध्रुव वर्ण *pathname)
{
	FILE *fp;
	वर्ण *buffer = रिक्त;
	size_t len = 0;
	वर्ण *p;
	वर्ण *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	यदि (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		प्रत्यागम list;

	यावत् (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		यावत् ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		यदि (token && (strlen(token) > 1)) {
			/* If not variable assignment then skip */
			यदि (strchr(token, '=')) {
				/* Strip the newline if present */
				यदि (token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	मुक्त(buffer);
	fclose(fp);

	प्रत्यागम list;
}

RC_STRINGLIST *
rc_config_list(ध्रुव वर्ण *file)
{
	प्रत्यागम config_list(AT_FDCWD, file);
}

स्थिर शून्य rc_config_set_मूल्य(RC_STRINGLIST *config, वर्ण *मूल्य)
{
	RC_STRING *cline;
	वर्ण *entry;
	size_t i = 0;
	वर्ण *newline;
	वर्ण *p = मूल्य;
	bool replaced;
	वर्ण *token;

	यदि (!p)
		प्रत्यागम;
	यदि (strncmp(p, "export ", 7) == 0)
		p += 7;
	यदि (!(token = strsep(&p, "=")))
		प्रत्यागम;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	यदि (*p == '$')
		token = मूल्य;
	अन्यथा
		कुरु {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} यावत् (token && *token == '\0');

	/* Drop a newline if that's all we have */
	यदि (token) {
		i = strlen(token) - 1;
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
		i = strlen(entry);
		यदि (strncmp(entry, cline->मूल्य, i) == 0 && cline->मूल्य[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			मुक्त(cline->मूल्य);
			cline->मूल्य = newline;
			replaced = true;
			भङ्ग;
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
	वर्ण *मूल्य = रिक्त;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = strlen(override->मूल्य);
		मूल्य = rc_proc_getent(override->मूल्य);

		/* No need to continue if there's nothing to override */
		यदि (!मूल्य) {
			मुक्त(मूल्य);
			अनुवर्त;
		}

		यदि (मूल्य != रिक्त) {
			xasprintf(&tmp, "%s=%s", override->मूल्य, मूल्य);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			यदि (strncmp(override->मूल्य, cline->मूल्य, varlen) == 0
				&& cline->मूल्य[varlen] == '=') {
				rc_stringlist_delete(config, cline->मूल्य);
				भङ्ग;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		मुक्त(tmp);
		मुक्त(मूल्य);
	}

	rc_stringlist_free(overrides);
#endif
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
	RC_STRING *ओळ;

	यदि (!(dp = do_opendirat(targetfd, dir)))
		प्रत्यागम;

	rc_conf_d_files = rc_stringlist_new();
	यावत् ((d = readdir(dp)) != रिक्त) {
		यदि (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		यदि (!fname->मूल्य)
			अनुवर्त;

		rc_conf_d_list = config_list(dirfd(dp), fname->मूल्य);
		TAILQ_FOREACH(ओळ, rc_conf_d_list, entries)
			यदि (ओळ->मूल्य)
				rc_config_set_मूल्य(config, ओळ->मूल्य);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	प्रत्यागम;
}

स्थिर RC_STRINGLIST *
config_load(पूर्णाङ्क dirfd, ध्रुव वर्ण *pathname)
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

	प्रत्यागम config;
}

RC_STRINGLIST *
rc_config_load(ध्रुव वर्ण *file)
{
	प्रत्यागम config_load(AT_FDCWD, file);
}

वर्ण *
rc_config_मूल्य(RC_STRINGLIST *list, ध्रुव वर्ण *entry)
{
	RC_STRING *ओळ;
	वर्ण *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(ओळ, list, entries) {
		p = strchr(ओळ->मूल्य, '=');
		यदि (p != रिक्त) {
			यदि (strncmp(entry, ओळ->मूल्य, len) == 0 && ओळ->मूल्य[len] == '=')
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
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

वर्ण *
rc_conf_मूल्य(ध्रुव वर्ण *setting)
{
	ध्रुव पूर्णाङ्क *dirfds;
	size_t dir_मोजणी = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	यदि (rc_conf)
		प्रत्यागम rc_config_मूल्य(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	प्रति (size_t i = 0; i < dir_मोजणी; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_मोजणी - 1 - i;
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
		वर्ण *p = s->मूल्य;
		यावत् (p && *p && *p != '=') {
			यदि (isupper((unsigned वर्ण)*p))
				*p = tolower((unsigned वर्ण)*p);
			p++;
		}
	}

	प्रत्यागम rc_config_मूल्य(rc_conf, setting);
}
