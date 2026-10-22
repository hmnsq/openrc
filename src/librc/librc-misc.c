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
rc_yesno(ثابت حرف *ارزښت)
{
	إذا (!ارزښت) {
		errno = ENOENT;
		أرجع false;
	}

	إذا (strcasecmp(ارزښت, "yes") == 0 ||
	    strcasecmp(ارزښت, "y") == 0 ||
	    strcasecmp(ارزښت, "true") == 0 ||
	    strcasecmp(ارزښت, "1") == 0)
		أرجع true;

	إذا (strcasecmp(ارزښت, "no") != 0 &&
	    strcasecmp(ارزښت, "n") != 0 &&
	    strcasecmp(ارزښت, "false") != 0 &&
	    strcasecmp(ارزښت, "0") != 0)
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
rc_getfile(ثابت حرف *file, حرف **بفر, size_t *len)
{
	bool ret = false;
	FILE *fp;
	صحيح fd;
	بنية stat st;
	size_t بشپړ, کیڼ;

	fp = fopen(file, "re");
	إذا (!fp)
		أرجع false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	إذا (fstat(fd, &st))
		اذهب پای_ته_رسېدلی;

	کیڼ = st.st_size;
	*len = کیڼ + 1; /* NUL terminator */
	*بفر = xrealloc(*بفر, *len);
	طالما (کیڼ) {
		بشپړ = fread(*بفر, حجم(*بفر[0]), کیڼ, fp);
		إذا (بشپړ == 0 && ferror(fp))
			اذهب پای_ته_رسېدلی;
		کیڼ -= بشپړ;
	}
	ret = true;

 پای_ته_رسېدلی:
	إذا (!ret) {
		حرر(*بفر);
		*len = 0;
	} وإلا
		(*بفر)[*len - 1] = '\0';
	fclose(fp);
	أرجع ret;
}

حرف *
rc_proc_getent(ثابت حرف *ent RC_ناکارول_شوی)
{
#که_تعریف_شوی __linux__
	FILE *fp;
	حرف *proc = عدم, *p, *ارزښت = عدم, *save;
	size_t i, len;

	إذا (!(fp = fopen("/proc/cmdline", "r")))
		أرجع عدم;

	i = 0;
	إذا (xgetline(&proc, &i, fp) == -1)
		اذهب بهر;
	save = proc;

	len = مزي_اوږدوالی(ent);
	طالما ((p = strsep(&save, " "))) {
		إذا (مزي_پرتله(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			إذا (*p == '=')
				p++;
			ارزښت = xstrdup(p);
		}
	}

	إذا (!ارزښت)
		errno = ENOENT;

بهر:
	fclose(fp);
	حرر(proc);

	أرجع ارزښت;
#وإلا
	أرجع عدم;
#د_که_پای
}

RC_STRINGLIST *
config_list(صحيح dirfd, ثابت حرف *pathname)
{
	FILE *fp;
	حرف *بفر = عدم;
	size_t len = 0;
	حرف *p;
	حرف *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	إذا (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		أرجع list;

	طالما (xgetline(&بفر, &len, fp) != -1) {
		p = بفر;
		/* Strip leading spaces/tabs */
		طالما ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		إذا (token && (مزي_اوږدوالی(token) > 1)) {
			/* If not variable assignment then skip */
			إذا (strchr(token, '=')) {
				/* Strip the newline if present */
				إذا (token[مزي_اوږدوالی(token) - 1] == '\n')
					token[مزي_اوږدوالی(token) - 1] = 0;

				rc_stringlist_جمع_کړه(list, token);
			}
		}
	}
	حرر(بفر);
	fclose(fp);

	أرجع list;
}

RC_STRINGLIST *
rc_config_list(ثابت حرف *file)
{
	أرجع config_list(AT_FDCWD, file);
}

ساكن فراغ rc_config_سېټ_ارزښت(RC_STRINGLIST *config, حرف *ارزښت)
{
	RC_STRING *cline;
	حرف *ثبت;
	size_t i = 0;
	حرف *newline;
	حرف *p = ارزښت;
	bool replaced;
	حرف *token;

	إذا (!p)
		أرجع;
	إذا (مزي_پرتله(p, "export ", 7) == 0)
		p += 7;
	إذا (!(token = strsep(&p, "=")))
		أرجع;

	ثبت = xstrdup(token);
	/* Preserve shell coloring */
	إذا (*p == '$')
		token = ارزښت;
	وإلا
		افعل {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} طالما (token && *token == '\0');

	/* Drop a newline if that's all we have */
	إذا (token) {
		i = مزي_اوږدوالی(token) - 1;
		إذا (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", ثبت, token);
	} وإلا {
		xasprintf(&newline, "%s=", ثبت);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = مزي_اوږدوالی(ثبت);
		إذا (مزي_پرتله(ثبت, cline->ارزښت, i) == 0 && cline->ارزښت[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			حرر(cline->ارزښت);
			cline->ارزښت = newline;
			replaced = true;
			اكسر;
		}
	}

	إذا (!replaced) {
		rc_stringlist_جمع_کړه(config, newline);
		حرر(newline);
	}
	حرر(ثبت);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
ساكن RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#که_تعریف_شوی __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	حرف *tmp = عدم;
	حرف *ارزښت = عدم;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_جمع_کړه(overrides, "rc_interactive");
	rc_stringlist_جمع_کړه(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = مزي_اوږدوالی(override->ارزښت);
		ارزښت = rc_proc_getent(override->ارزښت);

		/* No need to continue if there's nothing to override */
		إذا (!ارزښت) {
			حرر(ارزښت);
			استمر;
		}

		إذا (ارزښت != عدم) {
			xasprintf(&tmp, "%s=%s", override->ارزښت, ارزښت);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_خوندي(cline, config, entries, config_np) {
			إذا (مزي_پرتله(override->ارزښت, cline->ارزښت, varlen) == 0
				&& cline->ارزښت[varlen] == '=') {
				rc_stringlist_delete(config, cline->ارزښت);
				اكسر;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_جمع_کړه(config, tmp);

		حرر(tmp);
		حرر(ارزښت);
	}

	rc_stringlist_free(overrides);
#د_که_پای
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
	RC_STRING *کرښه;

	إذا (!(dp = do_opendirat(targetfd, dir)))
		أرجع;

	rc_conf_d_files = rc_stringlist_new();
	طالما ((d = readdir(dp)) != عدم) {
		إذا (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_ترتیبول(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		إذا (!fname->ارزښت)
			استمر;

		rc_conf_d_list = config_list(dirfd(dp), fname->ارزښت);
		TAILQ_FOREACH(کرښه, rc_conf_d_list, entries)
			إذا (کرښه->ارزښت)
				rc_config_سېټ_ارزښت(config, کرښه->ارزښت);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	أرجع;
}

ساكن RC_STRINGLIST *
config_بارول(صحيح dirfd, ثابت حرف *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *کرښه;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(کرښه, list, entries) {
		rc_config_سېټ_ارزښت(config, کرښه->ارزښت);
	}
	rc_stringlist_free(list);

	أرجع config;
}

RC_STRINGLIST *
rc_config_بارول(ثابت حرف *file)
{
	أرجع config_بارول(AT_FDCWD, file);
}

حرف *
rc_config_ارزښت(RC_STRINGLIST *list, ثابت حرف *ثبت)
{
	RC_STRING *کرښه;
	حرف *p;
	size_t len;

	len = مزي_اوږدوالی(ثبت);
	TAILQ_FOREACH(کرښه, list, entries) {
		p = strchr(کرښه->ارزښت, '=');
		إذا (p != عدم) {
			إذا (مزي_پرتله(ثبت, کرښه->ارزښت, len) == 0 && کرښه->ارزښت[len] == '=')
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
	RC_STRINGLIST *conf = config_بارول(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

حرف *
rc_conf_ارزښت(ثابت حرف *setting)
{
	ثابت صحيح *dirfds;
	size_t dir_شمېر = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	إذا (rc_conf)
		أرجع rc_config_ارزښت(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	لكل (size_t i = 0; i < dir_شمېر; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_شمېر - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	إذا (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_بارول(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		حرف *p = s->ارزښت;
		طالما (p && *p && *p != '=') {
			إذا (isupper((بینښه حرف)*p))
				*p = tolower((بینښه حرف)*p);
			p++;
		}
	}

	أرجع rc_config_ارزښت(rc_conf, setting);
}
