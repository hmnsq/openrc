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
rc_yesno(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ಆದರೆ (!ಮೌಲ್ಯ) {
		errno = ENOENT;
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (strcasecmp(ಮೌಲ್ಯ, "yes") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "y") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "true") == 0 ||
	    strcasecmp(ಮೌಲ್ಯ, "1") == 0)
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (strcasecmp(ಮೌಲ್ಯ, "no") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "n") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "false") != 0 &&
	    strcasecmp(ಮೌಲ್ಯ, "0") != 0)
		errno = EINVAL;

	ಹಿಂತಿರುಗಿ false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file, ಅಕ್ಷರ **ಬಫರ್, size_t *len)
{
	bool ret = false;
	FILE *fp;
	ಪೂರ್ಣಾಂಕ fd;
	ರಚನೆ stat st;
	size_t ಮುಗಿದ, ಎಡ;

	fp = fopen(file, "re");
	ಆದರೆ (!fp)
		ಹಿಂತಿರುಗಿ false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	ಆದರೆ (fstat(fd, &st))
		ಹೋಗು ಪೂರ್ಣಗೊಂಡ;

	ಎಡ = st.st_size;
	*len = ಎಡ + 1; /* NUL terminator */
	*ಬಫರ್ = xrealloc(*ಬಫರ್, *len);
	ಆಗ (ಎಡ) {
		ಮುಗಿದ = fread(*ಬಫರ್, ಗಾತ್ರ(*ಬಫರ್[0]), ಎಡ, fp);
		ಆದರೆ (ಮುಗಿದ == 0 && ferror(fp))
			ಹೋಗು ಪೂರ್ಣಗೊಂಡ;
		ಎಡ -= ಮುಗಿದ;
	}
	ret = true;

 ಪೂರ್ಣಗೊಂಡ:
	ಆದರೆ (!ret) {
		ಬಿಡುಗಡೆ(*ಬಫರ್);
		*len = 0;
	} ಇಲ್ಲದಿದ್ದರೆ
		(*ಬಫರ್)[*len - 1] = '\0';
	fclose(fp);
	ಹಿಂತಿರುಗಿ ret;
}

ಅಕ್ಷರ *
rc_proc_getent(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ent RC_ಬಳಸದ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	FILE *fp;
	ಅಕ್ಷರ *proc = ಖಾಲಿ, *p, *ಮೌಲ್ಯ = ಖಾಲಿ, *save;
	size_t i, len;

	ಆದರೆ (!(fp = fopen("/proc/cmdline", "r")))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	i = 0;
	ಆದರೆ (xgetline(&proc, &i, fp) == -1)
		ಹೋಗು ಹೊರಹೋಗು;
	save = proc;

	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ent);
	ಆಗ ((p = strsep(&save, " "))) {
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			ಆದರೆ (*p == '=')
				p++;
			ಮೌಲ್ಯ = xstrdup(p);
		}
	}

	ಆದರೆ (!ಮೌಲ್ಯ)
		errno = ENOENT;

ಹೊರಹೋಗು:
	fclose(fp);
	ಬಿಡುಗಡೆ(proc);

	ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
}

RC_STRINGLIST *
config_list(ಪೂರ್ಣಾಂಕ dirfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pathname)
{
	FILE *fp;
	ಅಕ್ಷರ *ಬಫರ್ = ಖಾಲಿ;
	size_t len = 0;
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	ಆದರೆ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		ಹಿಂತಿರುಗಿ list;

	ಆಗ (xgetline(&ಬಫರ್, &len, fp) != -1) {
		p = ಬಫರ್;
		/* Strip leading spaces/tabs */
		ಆಗ ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		ಆದರೆ (token && (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(token) > 1)) {
			/* If not variable assignment then skip */
			ಆದರೆ (strchr(token, '=')) {
				/* Strip the newline if present */
				ಆದರೆ (token[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(token) - 1] == '\n')
					token[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(token) - 1] = 0;

				rc_stringlist_ಕೂಡಿಸು(list, token);
			}
		}
	}
	ಬಿಡುಗಡೆ(ಬಫರ್);
	fclose(fp);

	ಹಿಂತಿರುಗಿ list;
}

RC_STRINGLIST *
rc_config_list(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file)
{
	ಹಿಂತಿರುಗಿ config_list(AT_FDCWD, file);
}

ಸ್ಥಿರ ಶೂನ್ಯ rc_config_ಗಣ_ಮೌಲ್ಯ(RC_STRINGLIST *config, ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	RC_STRING *cline;
	ಅಕ್ಷರ *ನಮೂದು;
	size_t i = 0;
	ಅಕ್ಷರ *newline;
	ಅಕ್ಷರ *p = ಮೌಲ್ಯ;
	bool replaced;
	ಅಕ್ಷರ *token;

	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ;
	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(p, "export ", 7) == 0)
		p += 7;
	ಆದರೆ (!(token = strsep(&p, "=")))
		ಹಿಂತಿರುಗಿ;

	ನಮೂದು = xstrdup(token);
	/* Preserve shell coloring */
	ಆದರೆ (*p == '$')
		token = ಮೌಲ್ಯ;
	ಇಲ್ಲದಿದ್ದರೆ
		ಮಾಡು {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} ಆಗ (token && *token == '\0');

	/* Drop a newline if that's all we have */
	ಆದರೆ (token) {
		i = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(token) - 1;
		ಆದರೆ (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", ನಮೂದು, token);
	} ಇಲ್ಲದಿದ್ದರೆ {
		xasprintf(&newline, "%s=", ನಮೂದು);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ನಮೂದು);
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ನಮೂದು, cline->ಮೌಲ್ಯ, i) == 0 && cline->ಮೌಲ್ಯ[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			ಬಿಡುಗಡೆ(cline->ಮೌಲ್ಯ);
			cline->ಮೌಲ್ಯ = newline;
			replaced = true;
			ನಿಲ್ಲಿಸು;
		}
	}

	ಆದರೆ (!replaced) {
		rc_stringlist_ಕೂಡಿಸು(config, newline);
		ಬಿಡುಗಡೆ(newline);
	}
	ಬಿಡುಗಡೆ(ನಮೂದು);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
ಸ್ಥಿರ RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	ಅಕ್ಷರ *tmp = ಖಾಲಿ;
	ಅಕ್ಷರ *ಮೌಲ್ಯ = ಖಾಲಿ;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_ಕೂಡಿಸು(overrides, "rc_interactive");
	rc_stringlist_ಕೂಡಿಸು(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(override->ಮೌಲ್ಯ);
		ಮೌಲ್ಯ = rc_proc_getent(override->ಮೌಲ್ಯ);

		/* No need to continue if there's nothing to override */
		ಆದರೆ (!ಮೌಲ್ಯ) {
			ಬಿಡುಗಡೆ(ಮೌಲ್ಯ);
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (ಮೌಲ್ಯ != ಖಾಲಿ) {
			xasprintf(&tmp, "%s=%s", override->ಮೌಲ್ಯ, ಮೌಲ್ಯ);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_ಸುರಕ್ಷಿತ(cline, config, entries, config_np) {
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(override->ಮೌಲ್ಯ, cline->ಮೌಲ್ಯ, varlen) == 0
				&& cline->ಮೌಲ್ಯ[varlen] == '=') {
				rc_stringlist_delete(config, cline->ಮೌಲ್ಯ);
				ನಿಲ್ಲಿಸು;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_ಕೂಡಿಸು(config, tmp);

		ಬಿಡುಗಡೆ(tmp);
		ಬಿಡುಗಡೆ(ಮೌಲ್ಯ);
	}

	rc_stringlist_free(overrides);
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ config;
}

ಸ್ಥಿರ ಶೂನ್ಯ
rc_config_directory(RC_STRINGLIST *config, ಪೂರ್ಣಾಂಕ targetfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dir)
{
	DIR *dp;
	ರಚನೆ dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *ಸಾಲು;

	ಆದರೆ (!(dp = do_opendirat(targetfd, dir)))
		ಹಿಂತಿರುಗಿ;

	rc_conf_d_files = rc_stringlist_new();
	ಆಗ ((d = readdir(dp)) != ಖಾಲಿ) {
		ಆದರೆ (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_ವಿಂಗಡಿಸು(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		ಆದರೆ (!fname->ಮೌಲ್ಯ)
			ಮುಂದುವರಿ;

		rc_conf_d_list = config_list(dirfd(dp), fname->ಮೌಲ್ಯ);
		TAILQ_FOREACH(ಸಾಲು, rc_conf_d_list, entries)
			ಆದರೆ (ಸಾಲು->ಮೌಲ್ಯ)
				rc_config_ಗಣ_ಮೌಲ್ಯ(config, ಸಾಲು->ಮೌಲ್ಯ);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	ಹಿಂತಿರುಗಿ;
}

ಸ್ಥಿರ RC_STRINGLIST *
config_ತುಂಬು(ಪೂರ್ಣಾಂಕ dirfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *ಸಾಲು;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(ಸಾಲು, list, entries) {
		rc_config_ಗಣ_ಮೌಲ್ಯ(config, ಸಾಲು->ಮೌಲ್ಯ);
	}
	rc_stringlist_free(list);

	ಹಿಂತಿರುಗಿ config;
}

RC_STRINGLIST *
rc_config_ತುಂಬು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file)
{
	ಹಿಂತಿರುಗಿ config_ತುಂಬು(AT_FDCWD, file);
}

ಅಕ್ಷರ *
rc_config_ಮೌಲ್ಯ(RC_STRINGLIST *list, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ನಮೂದು)
{
	RC_STRING *ಸಾಲು;
	ಅಕ್ಷರ *p;
	size_t len;

	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ನಮೂದು);
	TAILQ_FOREACH(ಸಾಲು, list, entries) {
		p = strchr(ಸಾಲು->ಮೌಲ್ಯ, '=');
		ಆದರೆ (p != ಖಾಲಿ) {
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ನಮೂದು, ಸಾಲು->ಮೌಲ್ಯ, len) == 0 && ಸಾಲು->ಮೌಲ್ಯ[len] == '=')
				ಹಿಂತಿರುಗಿ ++p;
		}
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
ಸ್ಥಿರ RC_STRINGLIST *rc_conf = ಖಾಲಿ;

ಸ್ಥಿರ ಶೂನ್ಯ
_free_rc_conf(ಶೂನ್ಯ)
{
	rc_stringlist_free(rc_conf);
}

ಸ್ಥಿರ ಶೂನ್ಯ
rc_conf_append(ಪೂರ್ಣಾಂಕ dirfd)
{
	RC_STRINGLIST *conf = config_ತುಂಬು(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

ಅಕ್ಷರ *
rc_conf_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *setting)
{
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ *dirfds;
	size_t dir_ಎಣಿಕೆ = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	ಆದರೆ (rc_conf)
		ಹಿಂತಿರುಗಿ rc_config_ಮೌಲ್ಯ(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	ಗಾಗಿ (size_t i = 0; i < dir_ಎಣಿಕೆ; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_ಎಣಿಕೆ - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	ಆದರೆ (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_ತುಂಬು(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		ಅಕ್ಷರ *p = s->ಮೌಲ್ಯ;
		ಆಗ (p && *p && *p != '=') {
			ಆದರೆ (isupper((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*p))
				*p = tolower((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*p);
			p++;
		}
	}

	ಹಿಂತಿರುಗಿ rc_config_ಮೌಲ್ಯ(rc_conf, setting);
}
