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
	सचे (!मानम्) {
		errno = ENOENT;
		पटिनिवत्त false;
	}

	सचे (strcasecmp(मानम्, "yes") == 0 ||
	    strcasecmp(मानम्, "y") == 0 ||
	    strcasecmp(मानम्, "true") == 0 ||
	    strcasecmp(मानम्, "1") == 0)
		पटिनिवत्त true;

	सचे (strcasecmp(मानम्, "no") != 0 &&
	    strcasecmp(मानम्, "n") != 0 &&
	    strcasecmp(मानम्, "false") != 0 &&
	    strcasecmp(मानम्, "0") != 0)
		errno = EINVAL;

	पटिनिवत्त false;
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
	size_t done, वामः;

	fp = fopen(file, "re");
	सचे (!fp)
		पटिनिवत्त false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	सचे (fstat(fd, &st))
		गच्छ finished;

	वामः = st.st_size;
	*len = वामः + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	याव (वामः) {
		done = fread(*buffer, परिमाण(*buffer[0]), वामः, fp);
		सचे (done == 0 && ferror(fp))
			गच्छ finished;
		वामः -= done;
	}
	ret = true;

 finished:
	सचे (!ret) {
		मुक्त(*buffer);
		*len = 0;
	} अञ्ञथा
		(*buffer)[*len - 1] = '\0';
	fclose(fp);
	पटिनिवत्त ret;
}

वर्ण *
rc_proc_getent(ध्रुव वर्ण *ent RC_UNUSED)
{
#ifdef __linux__
	FILE *fp;
	वर्ण *proc = रिक्त, *p, *मानम् = रिक्त, *save;
	size_t i, len;

	सचे (!(fp = fopen("/proc/cmdline", "r")))
		पटिनिवत्त रिक्त;

	i = 0;
	सचे (xgetline(&proc, &i, fp) == -1)
		गच्छ out;
	save = proc;

	len = strlen(ent);
	याव ((p = strsep(&save, " "))) {
		सचे (strncmp(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			सचे (*p == '=')
				p++;
			मानम् = xstrdup(p);
		}
	}

	सचे (!मानम्)
		errno = ENOENT;

out:
	fclose(fp);
	मुक्त(proc);

	पटिनिवत्त मानम्;
#अञ्ञथा
	पटिनिवत्त रिक्त;
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

	सचे (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		पटिनिवत्त list;

	याव (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		याव ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		सचे (token && (strlen(token) > 1)) {
			/* If not variable assignment then skip */
			सचे (strchr(token, '=')) {
				/* Strip the newline if present */
				सचे (token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	मुक्त(buffer);
	fclose(fp);

	पटिनिवत्त list;
}

RC_STRINGLIST *
rc_config_list(ध्रुव वर्ण *file)
{
	पटिनिवत्त config_list(AT_FDCWD, file);
}

स्थिर शून्य rc_config_set_मानम्(RC_STRINGLIST *config, वर्ण *मानम्)
{
	RC_STRING *cline;
	वर्ण *entry;
	size_t i = 0;
	वर्ण *newline;
	वर्ण *p = मानम्;
	bool replaced;
	वर्ण *token;

	सचे (!p)
		पटिनिवत्त;
	सचे (strncmp(p, "export ", 7) == 0)
		p += 7;
	सचे (!(token = strsep(&p, "=")))
		पटिनिवत्त;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	सचे (*p == '$')
		token = मानम्;
	अञ्ञथा
		करोहि {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} याव (token && *token == '\0');

	/* Drop a newline if that's all we have */
	सचे (token) {
		i = strlen(token) - 1;
		सचे (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", entry, token);
	} अञ्ञथा {
		xasprintf(&newline, "%s=", entry);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = strlen(entry);
		सचे (strncmp(entry, cline->मानम्, i) == 0 && cline->मानम्[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			मुक्त(cline->मानम्);
			cline->मानम् = newline;
			replaced = true;
			भङ्ग;
		}
	}

	सचे (!replaced) {
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
	वर्ण *मानम् = रिक्त;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = strlen(override->मानम्);
		मानम् = rc_proc_getent(override->मानम्);

		/* No need to continue if there's nothing to override */
		सचे (!मानम्) {
			मुक्त(मानम्);
			अनुवर्त;
		}

		सचे (मानम् != रिक्त) {
			xasprintf(&tmp, "%s=%s", override->मानम्, मानम्);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			सचे (strncmp(override->मानम्, cline->मानम्, varlen) == 0
				&& cline->मानम्[varlen] == '=') {
				rc_stringlist_delete(config, cline->मानम्);
				भङ्ग;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		मुक्त(tmp);
		मुक्त(मानम्);
	}

	rc_stringlist_free(overrides);
#endif
	पटिनिवत्त config;
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

	सचे (!(dp = do_opendirat(targetfd, dir)))
		पटिनिवत्त;

	rc_conf_d_files = rc_stringlist_new();
	याव ((d = readdir(dp)) != रिक्त) {
		सचे (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		सचे (!fname->मानम्)
			अनुवर्त;

		rc_conf_d_list = config_list(dirfd(dp), fname->मानम्);
		TAILQ_FOREACH(पङ्क्तिः, rc_conf_d_list, entries)
			सचे (पङ्क्तिः->मानम्)
				rc_config_set_मानम्(config, पङ्क्तिः->मानम्);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	पटिनिवत्त;
}

स्थिर RC_STRINGLIST *
config_load(पूर्णाङ्क dirfd, ध्रुव वर्ण *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *पङ्क्तिः;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(पङ्क्तिः, list, entries) {
		rc_config_set_मानम्(config, पङ्क्तिः->मानम्);
	}
	rc_stringlist_free(list);

	पटिनिवत्त config;
}

RC_STRINGLIST *
rc_config_load(ध्रुव वर्ण *file)
{
	पटिनिवत्त config_load(AT_FDCWD, file);
}

वर्ण *
rc_config_मानम्(RC_STRINGLIST *list, ध्रुव वर्ण *entry)
{
	RC_STRING *पङ्क्तिः;
	वर्ण *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(पङ्क्तिः, list, entries) {
		p = strchr(पङ्क्तिः->मानम्, '=');
		सचे (p != रिक्त) {
			सचे (strncmp(entry, पङ्क्तिः->मानम्, len) == 0 && पङ्क्तिः->मानम्[len] == '=')
				पटिनिवत्त ++p;
		}
	}
	पटिनिवत्त रिक्त;
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
rc_conf_मानम्(ध्रुव वर्ण *setting)
{
	ध्रुव पूर्णाङ्क *dirfds;
	size_t dir_गणनम् = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	सचे (rc_conf)
		पटिनिवत्त rc_config_मानम्(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	अत्थाय (size_t i = 0; i < dir_गणनम्; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_गणनम् - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	सचे (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		वर्ण *p = s->मानम्;
		याव (p && *p && *p != '=') {
			सचे (isupper((unsigned वर्ण)*p))
				*p = tolower((unsigned वर्ण)*p);
			p++;
		}
	}

	पटिनिवत्त rc_config_मानम्(rc_conf, setting);
}
