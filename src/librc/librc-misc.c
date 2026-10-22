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
rc_yesno(ثابت حرف *قدر)
{
	اگر (!قدر) {
		errno = ENOENT;
		واپس false;
	}

	اگر (strcasecmp(قدر, "yes") == 0 ||
	    strcasecmp(قدر, "y") == 0 ||
	    strcasecmp(قدر, "true") == 0 ||
	    strcasecmp(قدر, "1") == 0)
		واپس true;

	اگر (strcasecmp(قدر, "no") != 0 &&
	    strcasecmp(قدر, "n") != 0 &&
	    strcasecmp(قدر, "false") != 0 &&
	    strcasecmp(قدر, "0") != 0)
		errno = EINVAL;

	واپس false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(ثابت حرف *file, حرف **بفر, size_t *len)
{
	bool ret = false;
	FILE *fp;
	صحیح fd;
	ساخت stat st;
	size_t مکمل, بایاں;

	fp = fopen(file, "re");
	اگر (!fp)
		واپس false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	اگر (fstat(fd, &st))
		جاؤ ختم_شدہ;

	بایاں = st.st_size;
	*len = بایاں + 1; /* NUL terminator */
	*بفر = xrealloc(*بفر, *len);
	جب (بایاں) {
		مکمل = fread(*بفر, حجم(*بفر[0]), بایاں, fp);
		اگر (مکمل == 0 && ferror(fp))
			جاؤ ختم_شدہ;
		بایاں -= مکمل;
	}
	ret = true;

 ختم_شدہ:
	اگر (!ret) {
		آزاد(*بفر);
		*len = 0;
	} ورنہ
		(*بفر)[*len - 1] = '\0';
	fclose(fp);
	واپس ret;
}

حرف *
rc_proc_getent(ثابت حرف *ent RC_غیر_مستعمل)
{
#اگر_تعریف_شدہ __linux__
	FILE *fp;
	حرف *proc = تہی, *p, *قدر = تہی, *save;
	size_t i, len;

	اگر (!(fp = fopen("/proc/cmdline", "r")))
		واپس تہی;

	i = 0;
	اگر (xgetline(&proc, &i, fp) == -1)
		جاؤ برآمد;
	save = proc;

	len = لڑی_لمبائی(ent);
	جب ((p = strsep(&save, " "))) {
		اگر (لڑی_موازنہ(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			اگر (*p == '=')
				p++;
			قدر = xstrdup(p);
		}
	}

	اگر (!قدر)
		errno = ENOENT;

برآمد:
	fclose(fp);
	آزاد(proc);

	واپس قدر;
#ورنہ
	واپس تہی;
#اگر_ختم
}

RC_STRINGLIST *
config_list(صحیح dirfd, ثابت حرف *pathname)
{
	FILE *fp;
	حرف *بفر = تہی;
	size_t len = 0;
	حرف *p;
	حرف *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	اگر (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		واپس list;

	جب (xgetline(&بفر, &len, fp) != -1) {
		p = بفر;
		/* Strip leading spaces/tabs */
		جب ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		اگر (token && (لڑی_لمبائی(token) > 1)) {
			/* If not variable assignment then skip */
			اگر (strchr(token, '=')) {
				/* Strip the newline if present */
				اگر (token[لڑی_لمبائی(token) - 1] == '\n')
					token[لڑی_لمبائی(token) - 1] = 0;

				rc_stringlist_جوڑو(list, token);
			}
		}
	}
	آزاد(بفر);
	fclose(fp);

	واپس list;
}

RC_STRINGLIST *
rc_config_list(ثابت حرف *file)
{
	واپس config_list(AT_FDCWD, file);
}

ساکن خالی rc_config_سیٹ_قدر(RC_STRINGLIST *config, حرف *قدر)
{
	RC_STRING *cline;
	حرف *اندراج_2;
	size_t i = 0;
	حرف *newline;
	حرف *p = قدر;
	bool replaced;
	حرف *token;

	اگر (!p)
		واپس;
	اگر (لڑی_موازنہ(p, "export ", 7) == 0)
		p += 7;
	اگر (!(token = strsep(&p, "=")))
		واپس;

	اندراج_2 = xstrdup(token);
	/* Preserve shell coloring */
	اگر (*p == '$')
		token = قدر;
	ورنہ
		کرو {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} جب (token && *token == '\0');

	/* Drop a newline if that's all we have */
	اگر (token) {
		i = لڑی_لمبائی(token) - 1;
		اگر (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", اندراج_2, token);
	} ورنہ {
		xasprintf(&newline, "%s=", اندراج_2);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = لڑی_لمبائی(اندراج_2);
		اگر (لڑی_موازنہ(اندراج_2, cline->قدر, i) == 0 && cline->قدر[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			آزاد(cline->قدر);
			cline->قدر = newline;
			replaced = true;
			توڑو;
		}
	}

	اگر (!replaced) {
		rc_stringlist_جوڑو(config, newline);
		آزاد(newline);
	}
	آزاد(اندراج_2);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
ساکن RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#اگر_تعریف_شدہ __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	حرف *tmp = تہی;
	حرف *قدر = تہی;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_جوڑو(overrides, "rc_interactive");
	rc_stringlist_جوڑو(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = لڑی_لمبائی(override->قدر);
		قدر = rc_proc_getent(override->قدر);

		/* No need to continue if there's nothing to override */
		اگر (!قدر) {
			آزاد(قدر);
			جاری;
		}

		اگر (قدر != تہی) {
			xasprintf(&tmp, "%s=%s", override->قدر, قدر);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_محفوظ(cline, config, entries, config_np) {
			اگر (لڑی_موازنہ(override->قدر, cline->قدر, varlen) == 0
				&& cline->قدر[varlen] == '=') {
				rc_stringlist_delete(config, cline->قدر);
				توڑو;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_جوڑو(config, tmp);

		آزاد(tmp);
		آزاد(قدر);
	}

	rc_stringlist_free(overrides);
#اگر_ختم
	واپس config;
}

ساکن خالی
rc_config_directory(RC_STRINGLIST *config, صحیح targetfd, ثابت حرف *dir)
{
	DIR *dp;
	ساخت dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *سطر;

	اگر (!(dp = do_opendirat(targetfd, dir)))
		واپس;

	rc_conf_d_files = rc_stringlist_new();
	جب ((d = readdir(dp)) != تہی) {
		اگر (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_ترتیب_دو(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		اگر (!fname->قدر)
			جاری;

		rc_conf_d_list = config_list(dirfd(dp), fname->قدر);
		TAILQ_FOREACH(سطر, rc_conf_d_list, entries)
			اگر (سطر->قدر)
				rc_config_سیٹ_قدر(config, سطر->قدر);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	واپس;
}

ساکن RC_STRINGLIST *
config_بھرو(صحیح dirfd, ثابت حرف *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *سطر;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(سطر, list, entries) {
		rc_config_سیٹ_قدر(config, سطر->قدر);
	}
	rc_stringlist_free(list);

	واپس config;
}

RC_STRINGLIST *
rc_config_بھرو(ثابت حرف *file)
{
	واپس config_بھرو(AT_FDCWD, file);
}

حرف *
rc_config_قدر(RC_STRINGLIST *list, ثابت حرف *اندراج_2)
{
	RC_STRING *سطر;
	حرف *p;
	size_t len;

	len = لڑی_لمبائی(اندراج_2);
	TAILQ_FOREACH(سطر, list, entries) {
		p = strchr(سطر->قدر, '=');
		اگر (p != تہی) {
			اگر (لڑی_موازنہ(اندراج_2, سطر->قدر, len) == 0 && سطر->قدر[len] == '=')
				واپس ++p;
		}
	}
	واپس تہی;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
ساکن RC_STRINGLIST *rc_conf = تہی;

ساکن خالی
_free_rc_conf(خالی)
{
	rc_stringlist_free(rc_conf);
}

ساکن خالی
rc_conf_append(صحیح dirfd)
{
	RC_STRINGLIST *conf = config_بھرو(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

حرف *
rc_conf_قدر(ثابت حرف *setting)
{
	ثابت صحیح *dirfds;
	size_t dir_گنتی = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	اگر (rc_conf)
		واپس rc_config_قدر(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	برائے (size_t i = 0; i < dir_گنتی; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_گنتی - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	اگر (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_بھرو(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		حرف *p = s->قدر;
		جب (p && *p && *p != '=') {
			اگر (isupper((غیرعلامتی حرف)*p))
				*p = tolower((غیرعلامتی حرف)*p);
			p++;
		}
	}

	واپس rc_config_قدر(rc_conf, setting);
}
