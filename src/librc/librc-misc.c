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

#include <fnmatch.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#include "queue.h"
#include "librc.h"
#include "helpers.h"
#include "misc.h"

bool
rc_yesno(const char *value)
{
	만약 (!value) {
		errno = ENOENT;
		반환 false;
	}

	만약 (strcasecmp(value, "yes") == 0 ||
	    strcasecmp(value, "y") == 0 ||
	    strcasecmp(value, "true") == 0 ||
	    strcasecmp(value, "1") == 0)
		반환 true;

	만약 (strcasecmp(value, "no") != 0 &&
	    strcasecmp(value, "n") != 0 &&
	    strcasecmp(value, "false") != 0 &&
	    strcasecmp(value, "0") != 0)
		errno = EINVAL;

	반환 false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(const char *file, char **buffer, size_t *len)
{
	bool ret = false;
	FILE *fp;
	정수 fd;
	struct stat st;
	size_t done, left;

	fp = fopen(file, "re");
	만약 (!fp)
		반환 false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	만약 (fstat(fd, &st))
		goto finished;

	left = st.st_size;
	*len = left + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	동안 (left) {
		done = fread(*buffer, sizeof(*buffer[0]), left, fp);
		만약 (done == 0 && ferror(fp))
			goto finished;
		left -= done;
	}
	ret = true;

 finished:
	만약 (!ret) {
		free(*buffer);
		*len = 0;
	} 아니면
		(*buffer)[*len - 1] = '\0';
	fclose(fp);
	반환 ret;
}

char *
rc_proc_getent(const char *ent RC_UNUSED)
{
#ifdef __linux__
	FILE *fp;
	char *proc = NULL, *p, *value = NULL, *save;
	size_t i, len;

	만약 (!(fp = fopen("/proc/cmdline", "r")))
		반환 NULL;

	i = 0;
	만약 (xgetline(&proc, &i, fp) == -1)
		goto out;
	save = proc;

	len = strlen(ent);
	동안 ((p = strsep(&save, " "))) {
		만약 (strncmp(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			만약 (*p == '=')
				p++;
			value = xstrdup(p);
		}
	}

	만약 (!value)
		errno = ENOENT;

out:
	fclose(fp);
	free(proc);

	반환 value;
#아니면
	반환 NULL;
#endif
}

RC_STRINGLIST *
config_list(정수 dirfd, const char *pathname)
{
	FILE *fp;
	char *buffer = NULL;
	size_t len = 0;
	char *p;
	char *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	만약 (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		반환 list;

	동안 (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		동안 ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		만약 (token && (strlen(token) > 1)) {
			/* If not variable assignment then skip */
			만약 (strchr(token, '=')) {
				/* Strip the newline if present */
				만약 (token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	free(buffer);
	fclose(fp);

	반환 list;
}

RC_STRINGLIST *
rc_config_list(const char *file)
{
	반환 config_list(AT_FDCWD, file);
}

static void rc_config_set_value(RC_STRINGLIST *config, char *value)
{
	RC_STRING *cline;
	char *entry;
	size_t i = 0;
	char *newline;
	char *p = value;
	bool replaced;
	char *token;

	만약 (!p)
		반환;
	만약 (strncmp(p, "export ", 7) == 0)
		p += 7;
	만약 (!(token = strsep(&p, "=")))
		반환;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	만약 (*p == '$')
		token = value;
	아니면
		do {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} 동안 (token && *token == '\0');

	/* Drop a newline if that's all we have */
	만약 (token) {
		i = strlen(token) - 1;
		만약 (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", entry, token);
	} 아니면 {
		xasprintf(&newline, "%s=", entry);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = strlen(entry);
		만약 (strncmp(entry, cline->value, i) == 0 && cline->value[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			free(cline->value);
			cline->value = newline;
			replaced = true;
			break;
		}
	}

	만약 (!replaced) {
		rc_stringlist_add(config, newline);
		free(newline);
	}
	free(entry);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
static RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#ifdef __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	char *tmp = NULL;
	char *value = NULL;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = strlen(override->value);
		value = rc_proc_getent(override->value);

		/* No need to continue if there's nothing to override */
		만약 (!value) {
			free(value);
			continue;
		}

		만약 (value != NULL) {
			xasprintf(&tmp, "%s=%s", override->value, value);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			만약 (strncmp(override->value, cline->value, varlen) == 0
				&& cline->value[varlen] == '=') {
				rc_stringlist_delete(config, cline->value);
				break;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		free(tmp);
		free(value);
	}

	rc_stringlist_free(overrides);
#endif
	반환 config;
}

static void
rc_config_directory(RC_STRINGLIST *config, 정수 targetfd, const char *dir)
{
	DIR *dp;
	struct dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *line;

	만약 (!(dp = do_opendirat(targetfd, dir)))
		반환;

	rc_conf_d_files = rc_stringlist_new();
	동안 ((d = readdir(dp)) != NULL) {
		만약 (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		만약 (!fname->value)
			continue;

		rc_conf_d_list = config_list(dirfd(dp), fname->value);
		TAILQ_FOREACH(line, rc_conf_d_list, entries)
			만약 (line->value)
				rc_config_set_value(config, line->value);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	반환;
}

static RC_STRINGLIST *
config_load(정수 dirfd, const char *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *line;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(line, list, entries) {
		rc_config_set_value(config, line->value);
	}
	rc_stringlist_free(list);

	반환 config;
}

RC_STRINGLIST *
rc_config_load(const char *file)
{
	반환 config_load(AT_FDCWD, file);
}

char *
rc_config_value(RC_STRINGLIST *list, const char *entry)
{
	RC_STRING *line;
	char *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(line, list, entries) {
		p = strchr(line->value, '=');
		만약 (p != NULL) {
			만약 (strncmp(entry, line->value, len) == 0 && line->value[len] == '=')
				반환 ++p;
		}
	}
	반환 NULL;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
static RC_STRINGLIST *rc_conf = NULL;

static void
_free_rc_conf(void)
{
	rc_stringlist_free(rc_conf);
}

static void
rc_conf_append(정수 dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

char *
rc_conf_value(const char *setting)
{
	const 정수 *dirfds;
	size_t dir_count = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	만약 (rc_conf)
		반환 rc_config_value(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	위해 (size_t i = 0; i < dir_count; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_count - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	만약 (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		char *p = s->value;
		동안 (p && *p && *p != '=') {
			만약 (isupper((unsigned char)*p))
				*p = tolower((unsigned char)*p);
			p++;
		}
	}

	반환 rc_config_value(rc_conf, setting);
}
