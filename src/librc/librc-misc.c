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

#ਅਨ੍ਤਰ੍ਭਾਵ <fnmatch.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dirent.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "librc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"

bool
rc_yesno(ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	ਜੇ (!ਮੁੱਲ) {
		errno = ENOENT;
		ਵਾਪਸ false;
	}

	ਜੇ (strcasecmp(ਮੁੱਲ, "yes") == 0 ||
	    strcasecmp(ਮੁੱਲ, "y") == 0 ||
	    strcasecmp(ਮੁੱਲ, "true") == 0 ||
	    strcasecmp(ਮੁੱਲ, "1") == 0)
		ਵਾਪਸ true;

	ਜੇ (strcasecmp(ਮੁੱਲ, "no") != 0 &&
	    strcasecmp(ਮੁੱਲ, "n") != 0 &&
	    strcasecmp(ਮੁੱਲ, "false") != 0 &&
	    strcasecmp(ਮੁੱਲ, "0") != 0)
		errno = EINVAL;

	ਵਾਪਸ false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(ਅਚਲ ਅੱਖਰ *file, ਅੱਖਰ **ਬਫਰ, size_t *len)
{
	bool ret = false;
	FILE *fp;
	ਪੂਰਨ fd;
	ਬਣਤਰ stat st;
	size_t ਪੂਰਾ, ਖੱਬਾ;

	fp = fopen(file, "re");
	ਜੇ (!fp)
		ਵਾਪਸ false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	ਜੇ (fstat(fd, &st))
		ਜਾਓ ਮੁਕੰਮਲ;

	ਖੱਬਾ = st.st_size;
	*len = ਖੱਬਾ + 1; /* NUL terminator */
	*ਬਫਰ = xrealloc(*ਬਫਰ, *len);
	ਜਦੋਂ (ਖੱਬਾ) {
		ਪੂਰਾ = fread(*ਬਫਰ, ਆਕਾਰ(*ਬਫਰ[0]), ਖੱਬਾ, fp);
		ਜੇ (ਪੂਰਾ == 0 && ferror(fp))
			ਜਾਓ ਮੁਕੰਮਲ;
		ਖੱਬਾ -= ਪੂਰਾ;
	}
	ret = true;

 ਮੁਕੰਮਲ:
	ਜੇ (!ret) {
		ਮੁਕਤ(*ਬਫਰ);
		*len = 0;
	} ਨਹੀਂਤਾਂ
		(*ਬਫਰ)[*len - 1] = '\0';
	fclose(fp);
	ਵਾਪਸ ret;
}

ਅੱਖਰ *
rc_proc_getent(ਅਚਲ ਅੱਖਰ *ent RC_ਅਣਵਰਤਿਆ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	FILE *fp;
	ਅੱਖਰ *proc = ਨਿਰਾ, *p, *ਮੁੱਲ = ਨਿਰਾ, *save;
	size_t i, len;

	ਜੇ (!(fp = fopen("/proc/cmdline", "r")))
		ਵਾਪਸ ਨਿਰਾ;

	i = 0;
	ਜੇ (xgetline(&proc, &i, fp) == -1)
		ਜਾਓ ਬਾਹਰਲਾ;
	save = proc;

	len = ਲੜੀ_ਲੰਬਾਈ(ent);
	ਜਦੋਂ ((p = strsep(&save, " "))) {
		ਜੇ (ਲੜੀ_ਤੁਲਨਾ(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			ਜੇ (*p == '=')
				p++;
			ਮੁੱਲ = xstrdup(p);
		}
	}

	ਜੇ (!ਮੁੱਲ)
		errno = ENOENT;

ਬਾਹਰਲਾ:
	fclose(fp);
	ਮੁਕਤ(proc);

	ਵਾਪਸ ਮੁੱਲ;
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ ਨਿਰਾ;
#ਜੇ_ਅੰਤ
}

RC_STRINGLIST *
config_list(ਪੂਰਨ dirfd, ਅਚਲ ਅੱਖਰ *pathname)
{
	FILE *fp;
	ਅੱਖਰ *ਬਫਰ = ਨਿਰਾ;
	size_t len = 0;
	ਅੱਖਰ *p;
	ਅੱਖਰ *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	ਜੇ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		ਵਾਪਸ list;

	ਜਦੋਂ (xgetline(&ਬਫਰ, &len, fp) != -1) {
		p = ਬਫਰ;
		/* Strip leading spaces/tabs */
		ਜਦੋਂ ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		ਜੇ (token && (ਲੜੀ_ਲੰਬਾਈ(token) > 1)) {
			/* If not variable assignment then skip */
			ਜੇ (strchr(token, '=')) {
				/* Strip the newline if present */
				ਜੇ (token[ਲੜੀ_ਲੰਬਾਈ(token) - 1] == '\n')
					token[ਲੜੀ_ਲੰਬਾਈ(token) - 1] = 0;

				rc_stringlist_ਜੋੜੋ(list, token);
			}
		}
	}
	ਮੁਕਤ(ਬਫਰ);
	fclose(fp);

	ਵਾਪਸ list;
}

RC_STRINGLIST *
rc_config_list(ਅਚਲ ਅੱਖਰ *file)
{
	ਵਾਪਸ config_list(AT_FDCWD, file);
}

ਸਥਿਰ ਖਾਲੀ rc_config_ਸੈੱਟ_ਮੁੱਲ(RC_STRINGLIST *config, ਅੱਖਰ *ਮੁੱਲ)
{
	RC_STRING *cline;
	ਅੱਖਰ *ਇੰਦਰਾਜ;
	size_t i = 0;
	ਅੱਖਰ *newline;
	ਅੱਖਰ *p = ਮੁੱਲ;
	bool replaced;
	ਅੱਖਰ *token;

	ਜੇ (!p)
		ਵਾਪਸ;
	ਜੇ (ਲੜੀ_ਤੁਲਨਾ(p, "export ", 7) == 0)
		p += 7;
	ਜੇ (!(token = strsep(&p, "=")))
		ਵਾਪਸ;

	ਇੰਦਰਾਜ = xstrdup(token);
	/* Preserve shell coloring */
	ਜੇ (*p == '$')
		token = ਮੁੱਲ;
	ਨਹੀਂਤਾਂ
		ਕਰੋ {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} ਜਦੋਂ (token && *token == '\0');

	/* Drop a newline if that's all we have */
	ਜੇ (token) {
		i = ਲੜੀ_ਲੰਬਾਈ(token) - 1;
		ਜੇ (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", ਇੰਦਰਾਜ, token);
	} ਨਹੀਂਤਾਂ {
		xasprintf(&newline, "%s=", ਇੰਦਰਾਜ);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = ਲੜੀ_ਲੰਬਾਈ(ਇੰਦਰਾਜ);
		ਜੇ (ਲੜੀ_ਤੁਲਨਾ(ਇੰਦਰਾਜ, cline->ਮੁੱਲ, i) == 0 && cline->ਮੁੱਲ[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			ਮੁਕਤ(cline->ਮੁੱਲ);
			cline->ਮੁੱਲ = newline;
			replaced = true;
			ਰੋਕੋ;
		}
	}

	ਜੇ (!replaced) {
		rc_stringlist_ਜੋੜੋ(config, newline);
		ਮੁਕਤ(newline);
	}
	ਮੁਕਤ(ਇੰਦਰਾਜ);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
ਸਥਿਰ RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	ਅੱਖਰ *tmp = ਨਿਰਾ;
	ਅੱਖਰ *ਮੁੱਲ = ਨਿਰਾ;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_ਜੋੜੋ(overrides, "rc_interactive");
	rc_stringlist_ਜੋੜੋ(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = ਲੜੀ_ਲੰਬਾਈ(override->ਮੁੱਲ);
		ਮੁੱਲ = rc_proc_getent(override->ਮੁੱਲ);

		/* No need to continue if there's nothing to override */
		ਜੇ (!ਮੁੱਲ) {
			ਮੁਕਤ(ਮੁੱਲ);
			ਜਾਰੀ;
		}

		ਜੇ (ਮੁੱਲ != ਨਿਰਾ) {
			xasprintf(&tmp, "%s=%s", override->ਮੁੱਲ, ਮੁੱਲ);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(cline, config, entries, config_np) {
			ਜੇ (ਲੜੀ_ਤੁਲਨਾ(override->ਮੁੱਲ, cline->ਮੁੱਲ, varlen) == 0
				&& cline->ਮੁੱਲ[varlen] == '=') {
				rc_stringlist_delete(config, cline->ਮੁੱਲ);
				ਰੋਕੋ;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_ਜੋੜੋ(config, tmp);

		ਮੁਕਤ(tmp);
		ਮੁਕਤ(ਮੁੱਲ);
	}

	rc_stringlist_free(overrides);
#ਜੇ_ਅੰਤ
	ਵਾਪਸ config;
}

ਸਥਿਰ ਖਾਲੀ
rc_config_directory(RC_STRINGLIST *config, ਪੂਰਨ targetfd, ਅਚਲ ਅੱਖਰ *dir)
{
	DIR *dp;
	ਬਣਤਰ dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *ਸਤਰ;

	ਜੇ (!(dp = do_opendirat(targetfd, dir)))
		ਵਾਪਸ;

	rc_conf_d_files = rc_stringlist_new();
	ਜਦੋਂ ((d = readdir(dp)) != ਨਿਰਾ) {
		ਜੇ (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_ਛਾਂਟੋ(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		ਜੇ (!fname->ਮੁੱਲ)
			ਜਾਰੀ;

		rc_conf_d_list = config_list(dirfd(dp), fname->ਮੁੱਲ);
		TAILQ_FOREACH(ਸਤਰ, rc_conf_d_list, entries)
			ਜੇ (ਸਤਰ->ਮੁੱਲ)
				rc_config_ਸੈੱਟ_ਮੁੱਲ(config, ਸਤਰ->ਮੁੱਲ);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	ਵਾਪਸ;
}

ਸਥਿਰ RC_STRINGLIST *
config_ਲੱਦੋ(ਪੂਰਨ dirfd, ਅਚਲ ਅੱਖਰ *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *ਸਤਰ;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(ਸਤਰ, list, entries) {
		rc_config_ਸੈੱਟ_ਮੁੱਲ(config, ਸਤਰ->ਮੁੱਲ);
	}
	rc_stringlist_free(list);

	ਵਾਪਸ config;
}

RC_STRINGLIST *
rc_config_ਲੱਦੋ(ਅਚਲ ਅੱਖਰ *file)
{
	ਵਾਪਸ config_ਲੱਦੋ(AT_FDCWD, file);
}

ਅੱਖਰ *
rc_config_ਮੁੱਲ(RC_STRINGLIST *list, ਅਚਲ ਅੱਖਰ *ਇੰਦਰਾਜ)
{
	RC_STRING *ਸਤਰ;
	ਅੱਖਰ *p;
	size_t len;

	len = ਲੜੀ_ਲੰਬਾਈ(ਇੰਦਰਾਜ);
	TAILQ_FOREACH(ਸਤਰ, list, entries) {
		p = strchr(ਸਤਰ->ਮੁੱਲ, '=');
		ਜੇ (p != ਨਿਰਾ) {
			ਜੇ (ਲੜੀ_ਤੁਲਨਾ(ਇੰਦਰਾਜ, ਸਤਰ->ਮੁੱਲ, len) == 0 && ਸਤਰ->ਮੁੱਲ[len] == '=')
				ਵਾਪਸ ++p;
		}
	}
	ਵਾਪਸ ਨਿਰਾ;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
ਸਥਿਰ RC_STRINGLIST *rc_conf = ਨਿਰਾ;

ਸਥਿਰ ਖਾਲੀ
_free_rc_conf(ਖਾਲੀ)
{
	rc_stringlist_free(rc_conf);
}

ਸਥਿਰ ਖਾਲੀ
rc_conf_append(ਪੂਰਨ dirfd)
{
	RC_STRINGLIST *conf = config_ਲੱਦੋ(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

ਅੱਖਰ *
rc_conf_ਮੁੱਲ(ਅਚਲ ਅੱਖਰ *setting)
{
	ਅਚਲ ਪੂਰਨ *dirfds;
	size_t dir_ਗਿਣਤੀ = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	ਜੇ (rc_conf)
		ਵਾਪਸ rc_config_ਮੁੱਲ(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	ਲਈ (size_t i = 0; i < dir_ਗਿਣਤੀ; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_ਗਿਣਤੀ - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	ਜੇ (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_ਲੱਦੋ(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		ਅੱਖਰ *p = s->ਮੁੱਲ;
		ਜਦੋਂ (p && *p && *p != '=') {
			ਜੇ (isupper((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*p))
				*p = tolower((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*p);
			p++;
		}
	}

	ਵਾਪਸ rc_config_ਮੁੱਲ(rc_conf, setting);
}
