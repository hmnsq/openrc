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

#અન્તર્ભાવ <fnmatch.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <dirent.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <sys/stat.h>

#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "librc.h"
#અન્તર્ભાવ "helpers.h"
#અન્તર્ભાવ "misc.h"

bool
rc_yesno(અચલ અક્ષર *કિંમત)
{
	જો (!કિંમત) {
		errno = ENOENT;
		પાછા false;
	}

	જો (strcasecmp(કિંમત, "yes") == 0 ||
	    strcasecmp(કિંમત, "y") == 0 ||
	    strcasecmp(કિંમત, "true") == 0 ||
	    strcasecmp(કિંમત, "1") == 0)
		પાછા true;

	જો (strcasecmp(કિંમત, "no") != 0 &&
	    strcasecmp(કિંમત, "n") != 0 &&
	    strcasecmp(કિંમત, "false") != 0 &&
	    strcasecmp(કિંમત, "0") != 0)
		errno = EINVAL;

	પાછા false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(અચલ અક્ષર *file, અક્ષર **buffer, size_t *len)
{
	bool ret = false;
	FILE *fp;
	પૂર્ણાંક fd;
	સંરચના stat st;
	size_t done, ડાબું;

	fp = fopen(file, "re");
	જો (!fp)
		પાછા false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	જો (fstat(fd, &st))
		જાઓ finished;

	ડાબું = st.st_size;
	*len = ડાબું + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	જ્યારે (ડાબું) {
		done = fread(*buffer, કદ(*buffer[0]), ડાબું, fp);
		જો (done == 0 && ferror(fp))
			જાઓ finished;
		ડાબું -= done;
	}
	ret = true;

 finished:
	જો (!ret) {
		મુક્ત(*buffer);
		*len = 0;
	} નહીંતર
		(*buffer)[*len - 1] = '\0';
	fclose(fp);
	પાછા ret;
}

અક્ષર *
rc_proc_getent(અચલ અક્ષર *ent RC_UNUSED)
{
#ifdef __linux__
	FILE *fp;
	અક્ષર *proc = ખાલી, *p, *કિંમત = ખાલી, *save;
	size_t i, len;

	જો (!(fp = fopen("/proc/cmdline", "r")))
		પાછા ખાલી;

	i = 0;
	જો (xgetline(&proc, &i, fp) == -1)
		જાઓ out;
	save = proc;

	len = strlen(ent);
	જ્યારે ((p = strsep(&save, " "))) {
		જો (strncmp(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			જો (*p == '=')
				p++;
			કિંમત = xstrdup(p);
		}
	}

	જો (!કિંમત)
		errno = ENOENT;

out:
	fclose(fp);
	મુક્ત(proc);

	પાછા કિંમત;
#નહીંતર
	પાછા ખાલી;
#endif
}

RC_STRINGLIST *
config_list(પૂર્ણાંક dirfd, અચલ અક્ષર *pathname)
{
	FILE *fp;
	અક્ષર *buffer = ખાલી;
	size_t len = 0;
	અક્ષર *p;
	અક્ષર *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	જો (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		પાછા list;

	જ્યારે (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		જ્યારે ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		જો (token && (strlen(token) > 1)) {
			/* If not variable assignment then skip */
			જો (strchr(token, '=')) {
				/* Strip the newline if present */
				જો (token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	મુક્ત(buffer);
	fclose(fp);

	પાછા list;
}

RC_STRINGLIST *
rc_config_list(અચલ અક્ષર *file)
{
	પાછા config_list(AT_FDCWD, file);
}

સ્થિર શૂન્ય rc_config_set_કિંમત(RC_STRINGLIST *config, અક્ષર *કિંમત)
{
	RC_STRING *cline;
	અક્ષર *entry;
	size_t i = 0;
	અક્ષર *newline;
	અક્ષર *p = કિંમત;
	bool replaced;
	અક્ષર *token;

	જો (!p)
		પાછા;
	જો (strncmp(p, "export ", 7) == 0)
		p += 7;
	જો (!(token = strsep(&p, "=")))
		પાછા;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	જો (*p == '$')
		token = કિંમત;
	નહીંતર
		કરો {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} જ્યારે (token && *token == '\0');

	/* Drop a newline if that's all we have */
	જો (token) {
		i = strlen(token) - 1;
		જો (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", entry, token);
	} નહીંતર {
		xasprintf(&newline, "%s=", entry);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = strlen(entry);
		જો (strncmp(entry, cline->કિંમત, i) == 0 && cline->કિંમત[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			મુક્ત(cline->કિંમત);
			cline->કિંમત = newline;
			replaced = true;
			તોડો;
		}
	}

	જો (!replaced) {
		rc_stringlist_add(config, newline);
		મુક્ત(newline);
	}
	મુક્ત(entry);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
સ્થિર RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#ifdef __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	અક્ષર *tmp = ખાલી;
	અક્ષર *કિંમત = ખાલી;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = strlen(override->કિંમત);
		કિંમત = rc_proc_getent(override->કિંમત);

		/* No need to continue if there's nothing to override */
		જો (!કિંમત) {
			મુક્ત(કિંમત);
			ચાલુ;
		}

		જો (કિંમત != ખાલી) {
			xasprintf(&tmp, "%s=%s", override->કિંમત, કિંમત);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			જો (strncmp(override->કિંમત, cline->કિંમત, varlen) == 0
				&& cline->કિંમત[varlen] == '=') {
				rc_stringlist_delete(config, cline->કિંમત);
				તોડો;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		મુક્ત(tmp);
		મુક્ત(કિંમત);
	}

	rc_stringlist_free(overrides);
#endif
	પાછા config;
}

સ્થિર શૂન્ય
rc_config_directory(RC_STRINGLIST *config, પૂર્ણાંક targetfd, અચલ અક્ષર *dir)
{
	DIR *dp;
	સંરચના dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *લીટી;

	જો (!(dp = do_opendirat(targetfd, dir)))
		પાછા;

	rc_conf_d_files = rc_stringlist_new();
	જ્યારે ((d = readdir(dp)) != ખાલી) {
		જો (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		જો (!fname->કિંમત)
			ચાલુ;

		rc_conf_d_list = config_list(dirfd(dp), fname->કિંમત);
		TAILQ_FOREACH(લીટી, rc_conf_d_list, entries)
			જો (લીટી->કિંમત)
				rc_config_set_કિંમત(config, લીટી->કિંમત);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	પાછા;
}

સ્થિર RC_STRINGLIST *
config_load(પૂર્ણાંક dirfd, અચલ અક્ષર *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *લીટી;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(લીટી, list, entries) {
		rc_config_set_કિંમત(config, લીટી->કિંમત);
	}
	rc_stringlist_free(list);

	પાછા config;
}

RC_STRINGLIST *
rc_config_load(અચલ અક્ષર *file)
{
	પાછા config_load(AT_FDCWD, file);
}

અક્ષર *
rc_config_કિંમત(RC_STRINGLIST *list, અચલ અક્ષર *entry)
{
	RC_STRING *લીટી;
	અક્ષર *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(લીટી, list, entries) {
		p = strchr(લીટી->કિંમત, '=');
		જો (p != ખાલી) {
			જો (strncmp(entry, લીટી->કિંમત, len) == 0 && લીટી->કિંમત[len] == '=')
				પાછા ++p;
		}
	}
	પાછા ખાલી;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
સ્થિર RC_STRINGLIST *rc_conf = ખાલી;

સ્થિર શૂન્ય
_free_rc_conf(શૂન્ય)
{
	rc_stringlist_free(rc_conf);
}

સ્થિર શૂન્ય
rc_conf_append(પૂર્ણાંક dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

અક્ષર *
rc_conf_કિંમત(અચલ અક્ષર *setting)
{
	અચલ પૂર્ણાંક *dirfds;
	size_t dir_ગણતરી = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	જો (rc_conf)
		પાછા rc_config_કિંમત(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	માટે (size_t i = 0; i < dir_ગણતરી; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_ગણતરી - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	જો (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		અક્ષર *p = s->કિંમત;
		જ્યારે (p && *p && *p != '=') {
			જો (isupper((unsigned અક્ષર)*p))
				*p = tolower((unsigned અક્ષર)*p);
			p++;
		}
	}

	પાછા rc_config_કિંમત(rc_conf, setting);
}
