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

#ಅನ್ತರ್ಭಾವ <fnmatch.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <dirent.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>

#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "librc.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"
#ಅನ್ತರ್ಭಾವ "misc.h"

bool
rc_yesno(ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	ಯದಿ (!ಮೌಲ್ಯ) {
		errno = ENOENT;
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (strcasecmp(ಮೌಲ್ಯ, "yes") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "y") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "true") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "1") == 0)
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (strcasecmp(ಮೌಲ್ಯ, "no") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "n") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "false") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "0") != 0)
		errno = EINVAL;

	ಪ್ರತ್ಯಾಗಮ false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(ಧ್ರುವ ವರ್ಣ *file, ವರ್ಣ **buffer, size_t *len)
{
	bool ret = false;
	FILE *fp;
	ಪೂರ್ಣಾಙ್ಕ fd;
	ಸಂರಚನಾ stat st;
	size_t done, ಎಡ;

	fp = fopen(file, "re");
	ಯದಿ (!fp)
		ಪ್ರತ್ಯಾಗಮ false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	ಯದಿ (fstat(fd, &st))
		ಗಚ್ಛ finished;

	ಎಡ = st.st_size;
	*len = ಎಡ + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	ಯಾವತ್ (ಎಡ) {
		done = fread(*buffer, ಪರಿಮಾಣ(*buffer[0]), ಎಡ, fp);
		ಯದಿ (done == 0 && ferror(fp))
			ಗಚ್ಛ finished;
		ಎಡ -= done;
	}
	ret = true;

 finished:
	ಯದಿ (!ret) {
		ಮುಕ್ತ(*buffer);
		*len = 0;
	} ಅನ್ಯಥಾ
		(*buffer)[*len - 1] = '\0';
	fclose(fp);
	ಪ್ರತ್ಯಾಗಮ ret;
}

ವರ್ಣ *
rc_proc_getent(ಧ್ರುವ ವರ್ಣ *ent RC_UNUSED)
{
#ifdef __linux__
	FILE *fp;
	ವರ್ಣ *proc = ರಿಕ್ತ, *p, *ಮೌಲ್ಯ = ರಿಕ್ತ, *save;
	size_t i, len;

	ಯದಿ (!(fp = fopen("/proc/cmdline", "r")))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	i = 0;
	ಯದಿ (xgetline(&proc, &i, fp) == -1)
		ಗಚ್ಛ out;
	save = proc;

	len = strlen(ent);
	ಯಾವತ್ ((p = strsep(&save, " "))) {
		ಯದಿ (strncmp(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			ಯದಿ (*p == '=')
				p++;
			ಮೌಲ್ಯ = xstrdup(p);
		}
	}

	ಯದಿ (!ಮೌಲ್ಯ)
		errno = ENOENT;

out:
	fclose(fp);
	ಮುಕ್ತ(proc);

	ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
#endif
}

RC_STRINGLIST *
config_list(ಪೂರ್ಣಾಙ್ಕ dirfd, ಧ್ರುವ ವರ್ಣ *pathname)
{
	FILE *fp;
	ವರ್ಣ *buffer = ರಿಕ್ತ;
	size_t len = 0;
	ವರ್ಣ *p;
	ವರ್ಣ *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	ಯದಿ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		ಪ್ರತ್ಯಾಗಮ list;

	ಯಾವತ್ (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		ಯಾವತ್ ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		ಯದಿ (token && (strlen(token) > 1)) {
			/* If not variable assignment then skip */
			ಯದಿ (strchr(token, '=')) {
				/* Strip the newline if present */
				ಯದಿ (token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	ಮುಕ್ತ(buffer);
	fclose(fp);

	ಪ್ರತ್ಯಾಗಮ list;
}

RC_STRINGLIST *
rc_config_list(ಧ್ರುವ ವರ್ಣ *file)
{
	ಪ್ರತ್ಯಾಗಮ config_list(AT_FDCWD, file);
}

ಸ್ಥಿರ ಶೂನ್ಯ rc_config_set_ಮೌಲ್ಯ(RC_STRINGLIST *config, ವರ್ಣ *ಮೌಲ್ಯ)
{
	RC_STRING *cline;
	ವರ್ಣ *entry;
	size_t i = 0;
	ವರ್ಣ *newline;
	ವರ್ಣ *p = ಮೌಲ್ಯ;
	bool replaced;
	ವರ್ಣ *token;

	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ;
	ಯದಿ (strncmp(p, "export ", 7) == 0)
		p += 7;
	ಯದಿ (!(token = strsep(&p, "=")))
		ಪ್ರತ್ಯಾಗಮ;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	ಯದಿ (*p == '$')
		token = ಮೌಲ್ಯ;
	ಅನ್ಯಥಾ
		ಕುರು {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} ಯಾವತ್ (token && *token == '\0');

	/* Drop a newline if that's all we have */
	ಯದಿ (token) {
		i = strlen(token) - 1;
		ಯದಿ (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", entry, token);
	} ಅನ್ಯಥಾ {
		xasprintf(&newline, "%s=", entry);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = strlen(entry);
		ಯದಿ (strncmp(entry, cline->ಮೌಲ್ಯ, i) == 0 && cline->ಮೌಲ್ಯ[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			ಮುಕ್ತ(cline->ಮೌಲ್ಯ);
			cline->ಮೌಲ್ಯ = newline;
			replaced = true;
			ಭಙ್ಗ;
		}
	}

	ಯದಿ (!replaced) {
		rc_stringlist_add(config, newline);
		ಮುಕ್ತ(newline);
	}
	ಮುಕ್ತ(entry);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
ಸ್ಥಿರ RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#ifdef __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	ವರ್ಣ *tmp = ರಿಕ್ತ;
	ವರ್ಣ *ಮೌಲ್ಯ = ರಿಕ್ತ;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = strlen(override->ಮೌಲ್ಯ);
		ಮೌಲ್ಯ = rc_proc_getent(override->ಮೌಲ್ಯ);

		/* No need to continue if there's nothing to override */
		ಯದಿ (!ಮೌಲ್ಯ) {
			ಮುಕ್ತ(ಮೌಲ್ಯ);
			ಅನುವರ್ತ;
		}

		ಯದಿ (ಮೌಲ್ಯ != ರಿಕ್ತ) {
			xasprintf(&tmp, "%s=%s", override->ಮೌಲ್ಯ, ಮೌಲ್ಯ);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			ಯದಿ (strncmp(override->ಮೌಲ್ಯ, cline->ಮೌಲ್ಯ, varlen) == 0
				&& cline->ಮೌಲ್ಯ[varlen] == '=') {
				rc_stringlist_delete(config, cline->ಮೌಲ್ಯ);
				ಭಙ್ಗ;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		ಮುಕ್ತ(tmp);
		ಮುಕ್ತ(ಮೌಲ್ಯ);
	}

	rc_stringlist_free(overrides);
#endif
	ಪ್ರತ್ಯಾಗಮ config;
}

ಸ್ಥಿರ ಶೂನ್ಯ
rc_config_directory(RC_STRINGLIST *config, ಪೂರ್ಣಾಙ್ಕ targetfd, ಧ್ರುವ ವರ್ಣ *dir)
{
	DIR *dp;
	ಸಂರಚನಾ dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *ಸಾಲು;

	ಯದಿ (!(dp = do_opendirat(targetfd, dir)))
		ಪ್ರತ್ಯಾಗಮ;

	rc_conf_d_files = rc_stringlist_new();
	ಯಾವತ್ ((d = readdir(dp)) != ರಿಕ್ತ) {
		ಯದಿ (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		ಯದಿ (!fname->ಮೌಲ್ಯ)
			ಅನುವರ್ತ;

		rc_conf_d_list = config_list(dirfd(dp), fname->ಮೌಲ್ಯ);
		TAILQ_FOREACH(ಸಾಲು, rc_conf_d_list, entries)
			ಯದಿ (ಸಾಲು->ಮೌಲ್ಯ)
				rc_config_set_ಮೌಲ್ಯ(config, ಸಾಲು->ಮೌಲ್ಯ);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	ಪ್ರತ್ಯಾಗಮ;
}

ಸ್ಥಿರ RC_STRINGLIST *
config_load(ಪೂರ್ಣಾಙ್ಕ dirfd, ಧ್ರುವ ವರ್ಣ *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *ಸಾಲು;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(ಸಾಲು, list, entries) {
		rc_config_set_ಮೌಲ್ಯ(config, ಸಾಲು->ಮೌಲ್ಯ);
	}
	rc_stringlist_free(list);

	ಪ್ರತ್ಯಾಗಮ config;
}

RC_STRINGLIST *
rc_config_load(ಧ್ರುವ ವರ್ಣ *file)
{
	ಪ್ರತ್ಯಾಗಮ config_load(AT_FDCWD, file);
}

ವರ್ಣ *
rc_config_ಮೌಲ್ಯ(RC_STRINGLIST *list, ಧ್ರುವ ವರ್ಣ *entry)
{
	RC_STRING *ಸಾಲು;
	ವರ್ಣ *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(ಸಾಲು, list, entries) {
		p = strchr(ಸಾಲು->ಮೌಲ್ಯ, '=');
		ಯದಿ (p != ರಿಕ್ತ) {
			ಯದಿ (strncmp(entry, ಸಾಲು->ಮೌಲ್ಯ, len) == 0 && ಸಾಲು->ಮೌಲ್ಯ[len] == '=')
				ಪ್ರತ್ಯಾಗಮ ++p;
		}
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
ಸ್ಥಿರ RC_STRINGLIST *rc_conf = ರಿಕ್ತ;

ಸ್ಥಿರ ಶೂನ್ಯ
_free_rc_conf(ಶೂನ್ಯ)
{
	rc_stringlist_free(rc_conf);
}

ಸ್ಥಿರ ಶೂನ್ಯ
rc_conf_append(ಪೂರ್ಣಾಙ್ಕ dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

ವರ್ಣ *
rc_conf_ಮೌಲ್ಯ(ಧ್ರುವ ವರ್ಣ *setting)
{
	ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ *dirfds;
	size_t dir_ಎಣಿಕೆ = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	ಯದಿ (rc_conf)
		ಪ್ರತ್ಯಾಗಮ rc_config_ಮೌಲ್ಯ(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	ಪ್ರತಿ (size_t i = 0; i < dir_ಎಣಿಕೆ; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_ಎಣಿಕೆ - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	ಯದಿ (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		ವರ್ಣ *p = s->ಮೌಲ್ಯ;
		ಯಾವತ್ (p && *p && *p != '=') {
			ಯದಿ (isupper((unsigned ವರ್ಣ)*p))
				*p = tolower((unsigned ವರ್ಣ)*p);
			p++;
		}
	}

	ಪ್ರತ್ಯಾಗಮ rc_config_ಮೌಲ್ಯ(rc_conf, setting);
}
