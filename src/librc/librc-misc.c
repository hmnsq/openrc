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
rc_yesno(ثابت حرف *قدر)
{
	إذا (!قدر) {
		errno = ENOENT;
		أرجع false;
	}

	إذا (strcasecmp(قدر, "yes") == 0 ||
	    strcasecmp(قدر, "y") == 0 ||
	    strcasecmp(قدر, "true") == 0 ||
	    strcasecmp(قدر, "1") == 0)
		أرجع true;

	إذا (strcasecmp(قدر, "no") != 0 &&
	    strcasecmp(قدر, "n") != 0 &&
	    strcasecmp(قدر, "false") != 0 &&
	    strcasecmp(قدر, "0") != 0)
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
	size_t done, بایاں;

	fp = fopen(file, "re");
	إذا (!fp)
		أرجع false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	إذا (fstat(fd, &st))
		اذهب finished;

	بایاں = st.st_size;
	*len = بایاں + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	طالما (بایاں) {
		done = fread(*buffer, حجم(*buffer[0]), بایاں, fp);
		إذا (done == 0 && ferror(fp))
			اذهب finished;
		بایاں -= done;
	}
	ret = true;

 finished:
	إذا (!ret) {
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
	حرف *proc = عدم, *p, *قدر = عدم, *save;
	size_t i, len;

	إذا (!(fp = fopen("/proc/cmdline", "r")))
		أرجع عدم;

	i = 0;
	إذا (xgetline(&proc, &i, fp) == -1)
		اذهب out;
	save = proc;

	len = strlen(ent);
	طالما ((p = strsep(&save, " "))) {
		إذا (strncmp(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			إذا (*p == '=')
				p++;
			قدر = xstrdup(p);
		}
	}

	إذا (!قدر)
		errno = ENOENT;

out:
	fclose(fp);
	حرر(proc);

	أرجع قدر;
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

	إذا (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		أرجع list;

	طالما (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		طالما ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		إذا (token && (strlen(token) > 1)) {
			/* If not variable assignment then skip */
			إذا (strchr(token, '=')) {
				/* Strip the newline if present */
				إذا (token[strlen(token) - 1] == '\n')
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

ساكن فراغ rc_config_set_قدر(RC_STRINGLIST *config, حرف *قدر)
{
	RC_STRING *cline;
	حرف *entry;
	size_t i = 0;
	حرف *newline;
	حرف *p = قدر;
	bool replaced;
	حرف *token;

	إذا (!p)
		أرجع;
	إذا (strncmp(p, "export ", 7) == 0)
		p += 7;
	إذا (!(token = strsep(&p, "=")))
		أرجع;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	إذا (*p == '$')
		token = قدر;
	وإلا
		افعل {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} طالما (token && *token == '\0');

	/* Drop a newline if that's all we have */
	إذا (token) {
		i = strlen(token) - 1;
		إذا (token[i] == '\n')
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
		إذا (strncmp(entry, cline->قدر, i) == 0 && cline->قدر[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			حرر(cline->قدر);
			cline->قدر = newline;
			replaced = true;
			اكسر;
		}
	}

	إذا (!replaced) {
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
	حرف *قدر = عدم;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = strlen(override->قدر);
		قدر = rc_proc_getent(override->قدر);

		/* No need to continue if there's nothing to override */
		إذا (!قدر) {
			حرر(قدر);
			استمر;
		}

		إذا (قدر != عدم) {
			xasprintf(&tmp, "%s=%s", override->قدر, قدر);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			إذا (strncmp(override->قدر, cline->قدر, varlen) == 0
				&& cline->قدر[varlen] == '=') {
				rc_stringlist_delete(config, cline->قدر);
				اكسر;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		حرر(tmp);
		حرر(قدر);
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
	RC_STRING *سطر;

	إذا (!(dp = do_opendirat(targetfd, dir)))
		أرجع;

	rc_conf_d_files = rc_stringlist_new();
	طالما ((d = readdir(dp)) != عدم) {
		إذا (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		إذا (!fname->قدر)
			استمر;

		rc_conf_d_list = config_list(dirfd(dp), fname->قدر);
		TAILQ_FOREACH(سطر, rc_conf_d_list, entries)
			إذا (سطر->قدر)
				rc_config_set_قدر(config, سطر->قدر);
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
	RC_STRING *سطر;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(سطر, list, entries) {
		rc_config_set_قدر(config, سطر->قدر);
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
rc_config_قدر(RC_STRINGLIST *list, ثابت حرف *entry)
{
	RC_STRING *سطر;
	حرف *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(سطر, list, entries) {
		p = strchr(سطر->قدر, '=');
		إذا (p != عدم) {
			إذا (strncmp(entry, سطر->قدر, len) == 0 && سطر->قدر[len] == '=')
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
rc_conf_قدر(ثابت حرف *setting)
{
	ثابت صحيح *dirfds;
	size_t dir_گنتی = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	إذا (rc_conf)
		أرجع rc_config_قدر(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	لكل (size_t i = 0; i < dir_گنتی; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_گنتی - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	إذا (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		حرف *p = s->قدر;
		طالما (p && *p && *p != '=') {
			إذا (isupper((unsigned حرف)*p))
				*p = tolower((unsigned حرف)*p);
			p++;
		}
	}

	أرجع rc_config_قدر(rc_conf, setting);
}
