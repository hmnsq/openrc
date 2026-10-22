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

#ضمن <fnmatch.h>
#ضمن <ctype.h>
#ضمن <dirent.h>
#ضمن <errno.h>
#ضمن <limits.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <sys/stat.h>

#ضمن "queue.h"
#ضمن "librc.h"
#ضمن "helpers.h"
#ضمن "misc.h"

bool
rc_yesno(ثابت حرف *value)
{
	𐡄𐡍 (!value) {
		errno = ENOENT;
		أرجع false;
	}

	𐡄𐡍 (strcasecmp(value, "yes") == 0 ||
	    strcasecmp(value, "y") == 0 ||
	    strcasecmp(value, "true") == 0 ||
	    strcasecmp(value, "1") == 0)
		أرجع true;

	𐡄𐡍 (strcasecmp(value, "no") != 0 &&
	    strcasecmp(value, "n") != 0 &&
	    strcasecmp(value, "false") != 0 &&
	    strcasecmp(value, "0") != 0)
		errno = EINVAL;

	أرجع false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(ثابت حرف *file, حرف **buffer, size_t *len)
{
	bool ret = false;
	FILE *fp;
	صحيح fd;
	بنية stat st;
	size_t done, left;

	fp = fopen(file, "re");
	𐡄𐡍 (!fp)
		أرجع false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	𐡄𐡍 (fstat(fd, &st))
		اذهب finished;

	left = st.st_size;
	*len = left + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	طالما (left) {
		done = fread(*buffer, حجم(*buffer[0]), left, fp);
		𐡄𐡍 (done == 0 && ferror(fp))
			اذهب finished;
		left -= done;
	}
	ret = true;

 finished:
	𐡄𐡍 (!ret) {
		حرر(*buffer);
		*len = 0;
	} وإلا
		(*buffer)[*len - 1] = '\0';
	fclose(fp);
	أرجع ret;
}

حرف *
rc_proc_getent(ثابت حرف *ent RC_UNUSED)
{
#ifdef __linux__
	FILE *fp;
	حرف *proc = عدم, *p, *value = عدم, *save;
	size_t i, len;

	𐡄𐡍 (!(fp = fopen("/proc/cmdline", "r")))
		أرجع عدم;

	i = 0;
	𐡄𐡍 (xgetline(&proc, &i, fp) == -1)
		اذهب out;
	save = proc;

	len = strlen(ent);
	طالما ((p = strsep(&save, " "))) {
		𐡄𐡍 (strncmp(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			𐡄𐡍 (*p == '=')
				p++;
			value = xstrdup(p);
		}
	}

	𐡄𐡍 (!value)
		errno = ENOENT;

out:
	fclose(fp);
	حرر(proc);

	أرجع value;
#وإلا
	أرجع عدم;
#endif
}

RC_STRINGLIST *
config_list(صحيح dirfd, ثابت حرف *pathname)
{
	FILE *fp;
	حرف *buffer = عدم;
	size_t len = 0;
	حرف *p;
	حرف *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	𐡄𐡍 (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		أرجع list;

	طالما (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		طالما ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		𐡄𐡍 (token && (strlen(token) > 1)) {
			/* If not variable assignment then skip */
			𐡄𐡍 (strchr(token, '=')) {
				/* Strip the newline if present */
				𐡄𐡍 (token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	حرر(buffer);
	fclose(fp);

	أرجع list;
}

RC_STRINGLIST *
rc_config_list(ثابت حرف *file)
{
	أرجع config_list(AT_FDCWD, file);
}

ساكن فراغ rc_config_set_value(RC_STRINGLIST *config, حرف *value)
{
	RC_STRING *cline;
	حرف *entry;
	size_t i = 0;
	حرف *newline;
	حرف *p = value;
	bool replaced;
	حرف *token;

	𐡄𐡍 (!p)
		أرجع;
	𐡄𐡍 (strncmp(p, "export ", 7) == 0)
		p += 7;
	𐡄𐡍 (!(token = strsep(&p, "=")))
		أرجع;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	𐡄𐡍 (*p == '$')
		token = value;
	وإلا
		𐡏𐡁𐡃 {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} طالما (token && *token == '\0');

	/* Drop a newline if that's all we have */
	𐡄𐡍 (token) {
		i = strlen(token) - 1;
		𐡄𐡍 (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", entry, token);
	} وإلا {
		xasprintf(&newline, "%s=", entry);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = strlen(entry);
		𐡄𐡍 (strncmp(entry, cline->value, i) == 0 && cline->value[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			حرر(cline->value);
			cline->value = newline;
			replaced = true;
			اكسر;
		}
	}

	𐡄𐡍 (!replaced) {
		rc_stringlist_add(config, newline);
		حرر(newline);
	}
	حرر(entry);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
ساكن RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#ifdef __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	حرف *tmp = عدم;
	حرف *value = عدم;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = strlen(override->value);
		value = rc_proc_getent(override->value);

		/* No need to continue if there's nothing to override */
		𐡄𐡍 (!value) {
			حرر(value);
			استمر;
		}

		𐡄𐡍 (value != عدم) {
			xasprintf(&tmp, "%s=%s", override->value, value);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			𐡄𐡍 (strncmp(override->value, cline->value, varlen) == 0
				&& cline->value[varlen] == '=') {
				rc_stringlist_delete(config, cline->value);
				اكسر;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		حرر(tmp);
		حرر(value);
	}

	rc_stringlist_free(overrides);
#endif
	أرجع config;
}

ساكن فراغ
rc_config_directory(RC_STRINGLIST *config, صحيح targetfd, ثابت حرف *dir)
{
	DIR *dp;
	بنية dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *line;

	𐡄𐡍 (!(dp = do_opendirat(targetfd, dir)))
		أرجع;

	rc_conf_d_files = rc_stringlist_new();
	طالما ((d = readdir(dp)) != عدم) {
		𐡄𐡍 (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		𐡄𐡍 (!fname->value)
			استمر;

		rc_conf_d_list = config_list(dirfd(dp), fname->value);
		TAILQ_FOREACH(line, rc_conf_d_list, entries)
			𐡄𐡍 (line->value)
				rc_config_set_value(config, line->value);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	أرجع;
}

ساكن RC_STRINGLIST *
config_load(صحيح dirfd, ثابت حرف *pathname)
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

	أرجع config;
}

RC_STRINGLIST *
rc_config_load(ثابت حرف *file)
{
	أرجع config_load(AT_FDCWD, file);
}

حرف *
rc_config_value(RC_STRINGLIST *list, ثابت حرف *entry)
{
	RC_STRING *line;
	حرف *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(line, list, entries) {
		p = strchr(line->value, '=');
		𐡄𐡍 (p != عدم) {
			𐡄𐡍 (strncmp(entry, line->value, len) == 0 && line->value[len] == '=')
				أرجع ++p;
		}
	}
	أرجع عدم;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
ساكن RC_STRINGLIST *rc_conf = عدم;

ساكن فراغ
_free_rc_conf(فراغ)
{
	rc_stringlist_free(rc_conf);
}

ساكن فراغ
rc_conf_append(صحيح dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

حرف *
rc_conf_value(ثابت حرف *setting)
{
	ثابت صحيح *dirfds;
	size_t dir_count = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	𐡄𐡍 (rc_conf)
		أرجع rc_config_value(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	لكل (size_t i = 0; i < dir_count; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_count - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	𐡄𐡍 (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		حرف *p = s->value;
		طالما (p && *p && *p != '=') {
			𐡄𐡍 (isupper((unsigned حرف)*p))
				*p = tolower((unsigned حرف)*p);
			p++;
		}
	}

	أرجع rc_config_value(rc_conf, setting);
}
