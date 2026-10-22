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

#அந்தர்பாவ <fnmatch.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <dirent.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <sys/stat.h>

#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "librc.h"
#அந்தர்பாவ "helpers.h"
#அந்தர்பாவ "misc.h"

bool
rc_yesno(மாறிலி எழுத்து *மதிப்பு)
{
	எனில் (!மதிப்பு) {
		errno = ENOENT;
		திரும்பு false;
	}

	எனில் (strcasecmp(மதிப்பு, "yes") == 0 ||
	    strcasecmp(மதிப்பு, "y") == 0 ||
	    strcasecmp(மதிப்பு, "true") == 0 ||
	    strcasecmp(மதிப்பு, "1") == 0)
		திரும்பு true;

	எனில் (strcasecmp(மதிப்பு, "no") != 0 &&
	    strcasecmp(மதிப்பு, "n") != 0 &&
	    strcasecmp(மதிப்பு, "false") != 0 &&
	    strcasecmp(மதிப்பு, "0") != 0)
		errno = EINVAL;

	திரும்பு false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(மாறிலி எழுத்து *file, எழுத்து **buffer, size_t *len)
{
	bool ret = false;
	FILE *fp;
	முழு fd;
	கட்டமைப்பு stat st;
	size_t done, இடது;

	fp = fopen(file, "re");
	எனில் (!fp)
		திரும்பு false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	எனில் (fstat(fd, &st))
		செல் finished;

	இடது = st.st_size;
	*len = இடது + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	போது (இடது) {
		done = fread(*buffer, அளவு(*buffer[0]), இடது, fp);
		எனில் (done == 0 && ferror(fp))
			செல் finished;
		இடது -= done;
	}
	ret = true;

 finished:
	எனில் (!ret) {
		விடுவி(*buffer);
		*len = 0;
	} இல்லையெனில்
		(*buffer)[*len - 1] = '\0';
	fclose(fp);
	திரும்பு ret;
}

எழுத்து *
rc_proc_getent(மாறிலி எழுத்து *ent RC_UNUSED)
{
#ifdef __linux__
	FILE *fp;
	எழுத்து *proc = வெற்று, *p, *மதிப்பு = வெற்று, *save;
	size_t i, len;

	எனில் (!(fp = fopen("/proc/cmdline", "r")))
		திரும்பு வெற்று;

	i = 0;
	எனில் (xgetline(&proc, &i, fp) == -1)
		செல் out;
	save = proc;

	len = strlen(ent);
	போது ((p = strsep(&save, " "))) {
		எனில் (strncmp(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			எனில் (*p == '=')
				p++;
			மதிப்பு = xstrdup(p);
		}
	}

	எனில் (!மதிப்பு)
		errno = ENOENT;

out:
	fclose(fp);
	விடுவி(proc);

	திரும்பு மதிப்பு;
#இல்லையெனில்
	திரும்பு வெற்று;
#endif
}

RC_STRINGLIST *
config_list(முழு dirfd, மாறிலி எழுத்து *pathname)
{
	FILE *fp;
	எழுத்து *buffer = வெற்று;
	size_t len = 0;
	எழுத்து *p;
	எழுத்து *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	எனில் (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		திரும்பு list;

	போது (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		போது ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		எனில் (token && (strlen(token) > 1)) {
			/* If not variable assignment then skip */
			எனில் (strchr(token, '=')) {
				/* Strip the newline if present */
				எனில் (token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	விடுவி(buffer);
	fclose(fp);

	திரும்பு list;
}

RC_STRINGLIST *
rc_config_list(மாறிலி எழுத்து *file)
{
	திரும்பு config_list(AT_FDCWD, file);
}

நிலையான வெறுமை rc_config_set_மதிப்பு(RC_STRINGLIST *config, எழுத்து *மதிப்பு)
{
	RC_STRING *cline;
	எழுத்து *entry;
	size_t i = 0;
	எழுத்து *newline;
	எழுத்து *p = மதிப்பு;
	bool replaced;
	எழுத்து *token;

	எனில் (!p)
		திரும்பு;
	எனில் (strncmp(p, "export ", 7) == 0)
		p += 7;
	எனில் (!(token = strsep(&p, "=")))
		திரும்பு;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	எனில் (*p == '$')
		token = மதிப்பு;
	இல்லையெனில்
		செய் {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} போது (token && *token == '\0');

	/* Drop a newline if that's all we have */
	எனில் (token) {
		i = strlen(token) - 1;
		எனில் (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", entry, token);
	} இல்லையெனில் {
		xasprintf(&newline, "%s=", entry);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = strlen(entry);
		எனில் (strncmp(entry, cline->மதிப்பு, i) == 0 && cline->மதிப்பு[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			விடுவி(cline->மதிப்பு);
			cline->மதிப்பு = newline;
			replaced = true;
			நிறுத்து;
		}
	}

	எனில் (!replaced) {
		rc_stringlist_add(config, newline);
		விடுவி(newline);
	}
	விடுவி(entry);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
நிலையான RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#ifdef __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	எழுத்து *tmp = வெற்று;
	எழுத்து *மதிப்பு = வெற்று;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = strlen(override->மதிப்பு);
		மதிப்பு = rc_proc_getent(override->மதிப்பு);

		/* No need to continue if there's nothing to override */
		எனில் (!மதிப்பு) {
			விடுவி(மதிப்பு);
			தொடர்;
		}

		எனில் (மதிப்பு != வெற்று) {
			xasprintf(&tmp, "%s=%s", override->மதிப்பு, மதிப்பு);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			எனில் (strncmp(override->மதிப்பு, cline->மதிப்பு, varlen) == 0
				&& cline->மதிப்பு[varlen] == '=') {
				rc_stringlist_delete(config, cline->மதிப்பு);
				நிறுத்து;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		விடுவி(tmp);
		விடுவி(மதிப்பு);
	}

	rc_stringlist_free(overrides);
#endif
	திரும்பு config;
}

நிலையான வெறுமை
rc_config_directory(RC_STRINGLIST *config, முழு targetfd, மாறிலி எழுத்து *dir)
{
	DIR *dp;
	கட்டமைப்பு dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *வரி;

	எனில் (!(dp = do_opendirat(targetfd, dir)))
		திரும்பு;

	rc_conf_d_files = rc_stringlist_new();
	போது ((d = readdir(dp)) != வெற்று) {
		எனில் (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		எனில் (!fname->மதிப்பு)
			தொடர்;

		rc_conf_d_list = config_list(dirfd(dp), fname->மதிப்பு);
		TAILQ_FOREACH(வரி, rc_conf_d_list, entries)
			எனில் (வரி->மதிப்பு)
				rc_config_set_மதிப்பு(config, வரி->மதிப்பு);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	திரும்பு;
}

நிலையான RC_STRINGLIST *
config_load(முழு dirfd, மாறிலி எழுத்து *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *வரி;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(வரி, list, entries) {
		rc_config_set_மதிப்பு(config, வரி->மதிப்பு);
	}
	rc_stringlist_free(list);

	திரும்பு config;
}

RC_STRINGLIST *
rc_config_load(மாறிலி எழுத்து *file)
{
	திரும்பு config_load(AT_FDCWD, file);
}

எழுத்து *
rc_config_மதிப்பு(RC_STRINGLIST *list, மாறிலி எழுத்து *entry)
{
	RC_STRING *வரி;
	எழுத்து *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(வரி, list, entries) {
		p = strchr(வரி->மதிப்பு, '=');
		எனில் (p != வெற்று) {
			எனில் (strncmp(entry, வரி->மதிப்பு, len) == 0 && வரி->மதிப்பு[len] == '=')
				திரும்பு ++p;
		}
	}
	திரும்பு வெற்று;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
நிலையான RC_STRINGLIST *rc_conf = வெற்று;

நிலையான வெறுமை
_free_rc_conf(வெறுமை)
{
	rc_stringlist_free(rc_conf);
}

நிலையான வெறுமை
rc_conf_append(முழு dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

எழுத்து *
rc_conf_மதிப்பு(மாறிலி எழுத்து *setting)
{
	மாறிலி முழு *dirfds;
	size_t dir_மொத்தம் = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	எனில் (rc_conf)
		திரும்பு rc_config_மதிப்பு(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	க்கு (size_t i = 0; i < dir_மொத்தம்; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_மொத்தம் - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	எனில் (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		எழுத்து *p = s->மதிப்பு;
		போது (p && *p && *p != '=') {
			எனில் (isupper((unsigned எழுத்து)*p))
				*p = tolower((unsigned எழுத்து)*p);
			p++;
		}
	}

	திரும்பு rc_config_மதிப்பு(rc_conf, setting);
}
