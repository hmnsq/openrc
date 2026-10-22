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
rc_yesno(const var_na *value)
{
	ᨀᨘ (!value) {
		errno = ENOENT;
		ᨀᨘᨒ false;
	}

	ᨀᨘ (strcasecmp(value, "yes") == 0 ||
	    strcasecmp(value, "y") == 0 ||
	    strcasecmp(value, "true") == 0 ||
	    strcasecmp(value, "1") == 0)
		ᨀᨘᨒ true;

	ᨀᨘ (strcasecmp(value, "no") != 0 &&
	    strcasecmp(value, "n") != 0 &&
	    strcasecmp(value, "false") != 0 &&
	    strcasecmp(value, "0") != 0)
		errno = EINVAL;

	ᨀᨘᨒ false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(const var_na *file, var_na **buffer, size_t *len)
{
	bool ret = false;
	FILE *fp;
	puur_naa_nka fd;
	sa_nracanaa stat st;
	size_t done, left;

	fp = fopen(file, "re");
	ᨀᨘ (!fp)
		ᨀᨘᨒ false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	ᨀᨘ (fstat(fd, &st))
		goto finished;

	left = st.st_size;
	*len = left + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	ᨀᨛ (left) {
		done = fread(*buffer, parimaa_na(*buffer[0]), left, fp);
		ᨀᨘ (done == 0 && ferror(fp))
			goto finished;
		left -= done;
	}
	ret = true;

 finished:
	ᨀᨘ (!ret) {
		free(*buffer);
		*len = 0;
	} ᨀᨘᨁ
		(*buffer)[*len - 1] = '\0';
	fclose(fp);
	ᨀᨘᨒ ret;
}

var_na *
rc_proc_getent(const var_na *ent RC_UNUSED)
{
#ifdef __linux__
	FILE *fp;
	var_na *proc = NULL, *p, *value = NULL, *save;
	size_t i, len;

	ᨀᨘ (!(fp = fopen("/proc/cmdline", "r")))
		ᨀᨘᨒ NULL;

	i = 0;
	ᨀᨘ (xgetline(&proc, &i, fp) == -1)
		goto out;
	save = proc;

	len = strlen(ent);
	ᨀᨛ ((p = strsep(&save, " "))) {
		ᨀᨘ (strncmp(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			ᨀᨘ (*p == '=')
				p++;
			value = xstrdup(p);
		}
	}

	ᨀᨘ (!value)
		errno = ENOENT;

out:
	fclose(fp);
	free(proc);

	ᨀᨘᨒ value;
#ᨀᨘᨁ
	ᨀᨘᨒ NULL;
#endif
}

RC_STRINGLIST *
config_list(puur_naa_nka dirfd, const var_na *pathname)
{
	FILE *fp;
	var_na *buffer = NULL;
	size_t len = 0;
	var_na *p;
	var_na *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	ᨀᨘ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		ᨀᨘᨒ list;

	ᨀᨛ (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		ᨀᨛ ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		ᨀᨘ (token && (strlen(token) > 1)) {
			/* If not variable assignment then skip */
			ᨀᨘ (strchr(token, '=')) {
				/* Strip the newline if present */
				ᨀᨘ (token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	free(buffer);
	fclose(fp);

	ᨀᨘᨒ list;
}

RC_STRINGLIST *
rc_config_list(const var_na *file)
{
	ᨀᨘᨒ config_list(AT_FDCWD, file);
}

static void rc_config_set_value(RC_STRINGLIST *config, var_na *value)
{
	RC_STRING *cline;
	var_na *entry;
	size_t i = 0;
	var_na *newline;
	var_na *p = value;
	bool replaced;
	var_na *token;

	ᨀᨘ (!p)
		ᨀᨘᨒ;
	ᨀᨘ (strncmp(p, "export ", 7) == 0)
		p += 7;
	ᨀᨘ (!(token = strsep(&p, "=")))
		ᨀᨘᨒ;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	ᨀᨘ (*p == '$')
		token = value;
	ᨀᨘᨁ
		do {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} ᨀᨛ (token && *token == '\0');

	/* Drop a newline if that's all we have */
	ᨀᨘ (token) {
		i = strlen(token) - 1;
		ᨀᨘ (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", entry, token);
	} ᨀᨘᨁ {
		xasprintf(&newline, "%s=", entry);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = strlen(entry);
		ᨀᨘ (strncmp(entry, cline->value, i) == 0 && cline->value[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			free(cline->value);
			cline->value = newline;
			replaced = true;
			bha_nga;
		}
	}

	ᨀᨘ (!replaced) {
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
	var_na *tmp = NULL;
	var_na *value = NULL;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = strlen(override->value);
		value = rc_proc_getent(override->value);

		/* No need to continue if there's nothing to override */
		ᨀᨘ (!value) {
			free(value);
			continue;
		}

		ᨀᨘ (value != NULL) {
			xasprintf(&tmp, "%s=%s", override->value, value);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			ᨀᨘ (strncmp(override->value, cline->value, varlen) == 0
				&& cline->value[varlen] == '=') {
				rc_stringlist_delete(config, cline->value);
				bha_nga;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		free(tmp);
		free(value);
	}

	rc_stringlist_free(overrides);
#endif
	ᨀᨘᨒ config;
}

static void
rc_config_directory(RC_STRINGLIST *config, puur_naa_nka targetfd, const var_na *dir)
{
	DIR *dp;
	sa_nracanaa dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *line;

	ᨀᨘ (!(dp = do_opendirat(targetfd, dir)))
		ᨀᨘᨒ;

	rc_conf_d_files = rc_stringlist_new();
	ᨀᨛ ((d = readdir(dp)) != NULL) {
		ᨀᨘ (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		ᨀᨘ (!fname->value)
			continue;

		rc_conf_d_list = config_list(dirfd(dp), fname->value);
		TAILQ_FOREACH(line, rc_conf_d_list, entries)
			ᨀᨘ (line->value)
				rc_config_set_value(config, line->value);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	ᨀᨘᨒ;
}

static RC_STRINGLIST *
config_load(puur_naa_nka dirfd, const var_na *pathname)
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

	ᨀᨘᨒ config;
}

RC_STRINGLIST *
rc_config_load(const var_na *file)
{
	ᨀᨘᨒ config_load(AT_FDCWD, file);
}

var_na *
rc_config_value(RC_STRINGLIST *list, const var_na *entry)
{
	RC_STRING *line;
	var_na *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(line, list, entries) {
		p = strchr(line->value, '=');
		ᨀᨘ (p != NULL) {
			ᨀᨘ (strncmp(entry, line->value, len) == 0 && line->value[len] == '=')
				ᨀᨘᨒ ++p;
		}
	}
	ᨀᨘᨒ NULL;
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
rc_conf_append(puur_naa_nka dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

var_na *
rc_conf_value(const var_na *setting)
{
	const puur_naa_nka *dirfds;
	size_t dir_count = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	ᨀᨘ (rc_conf)
		ᨀᨘᨒ rc_config_value(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	for (size_t i = 0; i < dir_count; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_count - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	ᨀᨘ (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		var_na *p = s->value;
		ᨀᨛ (p && *p && *p != '=') {
			ᨀᨘ (isupper((unsigned var_na)*p))
				*p = tolower((unsigned var_na)*p);
			p++;
		}
	}

	ᨀᨘᨒ rc_config_value(rc_conf, setting);
}
