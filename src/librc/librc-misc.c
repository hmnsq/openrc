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

#شامل <fnmatch.h>
#شامل <ctype.h>
#شامل <dirent.h>
#شامل <errno.h>
#شامل <limits.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <sys/stat.h>

#شامل "queue.h"
#شامل "librc.h"
#شامل "helpers.h"
#شامل "misc.h"

bool
rc_yesno(ثابت نویسه *مقدار)
{
	اگر (!مقدار) {
		errno = ENOENT;
		بازگشت false;
	}

	اگر (strcasecmp(مقدار, "yes") == 0 ||
	    strcasecmp(مقدار, "y") == 0 ||
	    strcasecmp(مقدار, "true") == 0 ||
	    strcasecmp(مقدار, "1") == 0)
		بازگشت true;

	اگر (strcasecmp(مقدار, "no") != 0 &&
	    strcasecmp(مقدار, "n") != 0 &&
	    strcasecmp(مقدار, "false") != 0 &&
	    strcasecmp(مقدار, "0") != 0)
		errno = EINVAL;

	بازگشت false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(ثابت نویسه *file, نویسه **buffer, size_t *len)
{
	bool ret = false;
	FILE *fp;
	صحیح fd;
	ساختار stat st;
	size_t done, چپ;

	fp = fopen(file, "re");
	اگر (!fp)
		بازگشت false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	اگر (fstat(fd, &st))
		برو finished;

	چپ = st.st_size;
	*len = چپ + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	وقتی (چپ) {
		done = fread(*buffer, اندازه(*buffer[0]), چپ, fp);
		اگر (done == 0 && ferror(fp))
			برو finished;
		چپ -= done;
	}
	ret = true;

 finished:
	اگر (!ret) {
		آزاد(*buffer);
		*len = 0;
	} وگرنه
		(*buffer)[*len - 1] = '\0';
	fclose(fp);
	بازگشت ret;
}

نویسه *
rc_proc_getent(ثابت نویسه *ent RC_UNUSED)
{
#ifdef __linux__
	FILE *fp;
	نویسه *proc = پوچ, *p, *مقدار = پوچ, *save;
	size_t i, len;

	اگر (!(fp = fopen("/proc/cmdline", "r")))
		بازگشت پوچ;

	i = 0;
	اگر (xgetline(&proc, &i, fp) == -1)
		برو out;
	save = proc;

	len = strlen(ent);
	وقتی ((p = strsep(&save, " "))) {
		اگر (strncmp(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			اگر (*p == '=')
				p++;
			مقدار = xstrdup(p);
		}
	}

	اگر (!مقدار)
		errno = ENOENT;

out:
	fclose(fp);
	آزاد(proc);

	بازگشت مقدار;
#وگرنه
	بازگشت پوچ;
#endif
}

RC_STRINGLIST *
config_list(صحیح dirfd, ثابت نویسه *pathname)
{
	FILE *fp;
	نویسه *buffer = پوچ;
	size_t len = 0;
	نویسه *p;
	نویسه *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	اگر (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		بازگشت list;

	وقتی (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		وقتی ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		اگر (token && (strlen(token) > 1)) {
			/* If not variable assignment then skip */
			اگر (strchr(token, '=')) {
				/* Strip the newline if present */
				اگر (token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	آزاد(buffer);
	fclose(fp);

	بازگشت list;
}

RC_STRINGLIST *
rc_config_list(ثابت نویسه *file)
{
	بازگشت config_list(AT_FDCWD, file);
}

ایستا تهی rc_config_set_مقدار(RC_STRINGLIST *config, نویسه *مقدار)
{
	RC_STRING *cline;
	نویسه *entry;
	size_t i = 0;
	نویسه *newline;
	نویسه *p = مقدار;
	bool replaced;
	نویسه *token;

	اگر (!p)
		بازگشت;
	اگر (strncmp(p, "export ", 7) == 0)
		p += 7;
	اگر (!(token = strsep(&p, "=")))
		بازگشت;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	اگر (*p == '$')
		token = مقدار;
	وگرنه
		انجامبده {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} وقتی (token && *token == '\0');

	/* Drop a newline if that's all we have */
	اگر (token) {
		i = strlen(token) - 1;
		اگر (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", entry, token);
	} وگرنه {
		xasprintf(&newline, "%s=", entry);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = strlen(entry);
		اگر (strncmp(entry, cline->مقدار, i) == 0 && cline->مقدار[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			آزاد(cline->مقدار);
			cline->مقدار = newline;
			replaced = true;
			بشکن;
		}
	}

	اگر (!replaced) {
		rc_stringlist_add(config, newline);
		آزاد(newline);
	}
	آزاد(entry);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
ایستا RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#ifdef __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	نویسه *tmp = پوچ;
	نویسه *مقدار = پوچ;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = strlen(override->مقدار);
		مقدار = rc_proc_getent(override->مقدار);

		/* No need to continue if there's nothing to override */
		اگر (!مقدار) {
			آزاد(مقدار);
			ادامه;
		}

		اگر (مقدار != پوچ) {
			xasprintf(&tmp, "%s=%s", override->مقدار, مقدار);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			اگر (strncmp(override->مقدار, cline->مقدار, varlen) == 0
				&& cline->مقدار[varlen] == '=') {
				rc_stringlist_delete(config, cline->مقدار);
				بشکن;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		آزاد(tmp);
		آزاد(مقدار);
	}

	rc_stringlist_free(overrides);
#endif
	بازگشت config;
}

ایستا تهی
rc_config_directory(RC_STRINGLIST *config, صحیح targetfd, ثابت نویسه *dir)
{
	DIR *dp;
	ساختار dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *سطر;

	اگر (!(dp = do_opendirat(targetfd, dir)))
		بازگشت;

	rc_conf_d_files = rc_stringlist_new();
	وقتی ((d = readdir(dp)) != پوچ) {
		اگر (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		اگر (!fname->مقدار)
			ادامه;

		rc_conf_d_list = config_list(dirfd(dp), fname->مقدار);
		TAILQ_FOREACH(سطر, rc_conf_d_list, entries)
			اگر (سطر->مقدار)
				rc_config_set_مقدار(config, سطر->مقدار);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	بازگشت;
}

ایستا RC_STRINGLIST *
config_load(صحیح dirfd, ثابت نویسه *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *سطر;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(سطر, list, entries) {
		rc_config_set_مقدار(config, سطر->مقدار);
	}
	rc_stringlist_free(list);

	بازگشت config;
}

RC_STRINGLIST *
rc_config_load(ثابت نویسه *file)
{
	بازگشت config_load(AT_FDCWD, file);
}

نویسه *
rc_config_مقدار(RC_STRINGLIST *list, ثابت نویسه *entry)
{
	RC_STRING *سطر;
	نویسه *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(سطر, list, entries) {
		p = strchr(سطر->مقدار, '=');
		اگر (p != پوچ) {
			اگر (strncmp(entry, سطر->مقدار, len) == 0 && سطر->مقدار[len] == '=')
				بازگشت ++p;
		}
	}
	بازگشت پوچ;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
ایستا RC_STRINGLIST *rc_conf = پوچ;

ایستا تهی
_free_rc_conf(تهی)
{
	rc_stringlist_free(rc_conf);
}

ایستا تهی
rc_conf_append(صحیح dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

نویسه *
rc_conf_مقدار(ثابت نویسه *setting)
{
	ثابت صحیح *dirfds;
	size_t dir_شمار = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	اگر (rc_conf)
		بازگشت rc_config_مقدار(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	برای (size_t i = 0; i < dir_شمار; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_شمار - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	اگر (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		نویسه *p = s->مقدار;
		وقتی (p && *p && *p != '=') {
			اگر (isupper((unsigned نویسه)*p))
				*p = tolower((unsigned نویسه)*p);
			p++;
		}
	}

	بازگشت rc_config_مقدار(rc_conf, setting);
}
