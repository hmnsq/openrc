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

#অন্তর্ভাব <fnmatch.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <dirent.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>
#অন্তর্ভাব <sys/stat.h>

#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "librc.h"
#অন্তর্ভাব "helpers.h"
#অন্তর্ভাব "misc.h"

bool
rc_yesno(ধ্রুবক অক্ষর *মান)
{
	যদি (!মান) {
		errno = ENOENT;
		ফেরত false;
	}

	যদি (strcasecmp(মান, "yes") == 0 ||
	    strcasecmp(মান, "y") == 0 ||
	    strcasecmp(মান, "true") == 0 ||
	    strcasecmp(মান, "1") == 0)
		ফেরত true;

	যদি (strcasecmp(মান, "no") != 0 &&
	    strcasecmp(মান, "n") != 0 &&
	    strcasecmp(মান, "false") != 0 &&
	    strcasecmp(মান, "0") != 0)
		errno = EINVAL;

	ফেরত false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(ধ্রুবক অক্ষর *file, অক্ষর **বাফার, size_t *len)
{
	bool ret = false;
	FILE *fp;
	পূর্ণসংখ্যা fd;
	কাঠামো stat st;
	size_t done, বাম;

	fp = fopen(file, "re");
	যদি (!fp)
		ফেরত false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	যদি (fstat(fd, &st))
		যাও finished;

	বাম = st.st_size;
	*len = বাম + 1; /* NUL terminator */
	*বাফার = xrealloc(*বাফার, *len);
	যখন (বাম) {
		done = fread(*বাফার, আকার(*বাফার[0]), বাম, fp);
		যদি (done == 0 && ferror(fp))
			যাও finished;
		বাম -= done;
	}
	ret = true;

 finished:
	যদি (!ret) {
		মুক্ত(*বাফার);
		*len = 0;
	} অন্যথা
		(*বাফার)[*len - 1] = '\0';
	fclose(fp);
	ফেরত ret;
}

অক্ষর *
rc_proc_getent(ধ্রুবক অক্ষর *ent RC_UNUSED)
{
#ifdef __linux__
	FILE *fp;
	অক্ষর *proc = ফাঁকা, *p, *মান = ফাঁকা, *save;
	size_t i, len;

	যদি (!(fp = fopen("/proc/cmdline", "r")))
		ফেরত ফাঁকা;

	i = 0;
	যদি (xgetline(&proc, &i, fp) == -1)
		যাও out;
	save = proc;

	len = strlen(ent);
	যখন ((p = strsep(&save, " "))) {
		যদি (strncmp(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			যদি (*p == '=')
				p++;
			মান = xstrdup(p);
		}
	}

	যদি (!মান)
		errno = ENOENT;

out:
	fclose(fp);
	মুক্ত(proc);

	ফেরত মান;
#অন্যথা
	ফেরত ফাঁকা;
#endif
}

RC_STRINGLIST *
config_list(পূর্ণসংখ্যা dirfd, ধ্রুবক অক্ষর *pathname)
{
	FILE *fp;
	অক্ষর *বাফার = ফাঁকা;
	size_t len = 0;
	অক্ষর *p;
	অক্ষর *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	যদি (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		ফেরত list;

	যখন (xgetline(&বাফার, &len, fp) != -1) {
		p = বাফার;
		/* Strip leading spaces/tabs */
		যখন ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		যদি (token && (strlen(token) > 1)) {
			/* If not variable assignment then skip */
			যদি (strchr(token, '=')) {
				/* Strip the newline if present */
				যদি (token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	মুক্ত(বাফার);
	fclose(fp);

	ফেরত list;
}

RC_STRINGLIST *
rc_config_list(ধ্রুবক অক্ষর *file)
{
	ফেরত config_list(AT_FDCWD, file);
}

স্থির শূন্য rc_config_set_মান(RC_STRINGLIST *config, অক্ষর *মান)
{
	RC_STRING *cline;
	অক্ষর *entry;
	size_t i = 0;
	অক্ষর *newline;
	অক্ষর *p = মান;
	bool replaced;
	অক্ষর *token;

	যদি (!p)
		ফেরত;
	যদি (strncmp(p, "export ", 7) == 0)
		p += 7;
	যদি (!(token = strsep(&p, "=")))
		ফেরত;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	যদি (*p == '$')
		token = মান;
	অন্যথা
		করো {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} যখন (token && *token == '\0');

	/* Drop a newline if that's all we have */
	যদি (token) {
		i = strlen(token) - 1;
		যদি (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", entry, token);
	} অন্যথা {
		xasprintf(&newline, "%s=", entry);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = strlen(entry);
		যদি (strncmp(entry, cline->মান, i) == 0 && cline->মান[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			মুক্ত(cline->মান);
			cline->মান = newline;
			replaced = true;
			বিরতি;
		}
	}

	যদি (!replaced) {
		rc_stringlist_add(config, newline);
		মুক্ত(newline);
	}
	মুক্ত(entry);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
স্থির RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#ifdef __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	অক্ষর *tmp = ফাঁকা;
	অক্ষর *মান = ফাঁকা;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = strlen(override->মান);
		মান = rc_proc_getent(override->মান);

		/* No need to continue if there's nothing to override */
		যদি (!মান) {
			মুক্ত(মান);
			চালু;
		}

		যদি (মান != ফাঁকা) {
			xasprintf(&tmp, "%s=%s", override->মান, মান);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			যদি (strncmp(override->মান, cline->মান, varlen) == 0
				&& cline->মান[varlen] == '=') {
				rc_stringlist_delete(config, cline->মান);
				বিরতি;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		মুক্ত(tmp);
		মুক্ত(মান);
	}

	rc_stringlist_free(overrides);
#endif
	ফেরত config;
}

স্থির শূন্য
rc_config_directory(RC_STRINGLIST *config, পূর্ণসংখ্যা targetfd, ধ্রুবক অক্ষর *dir)
{
	DIR *dp;
	কাঠামো dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *সারি;

	যদি (!(dp = do_opendirat(targetfd, dir)))
		ফেরত;

	rc_conf_d_files = rc_stringlist_new();
	যখন ((d = readdir(dp)) != ফাঁকা) {
		যদি (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		যদি (!fname->মান)
			চালু;

		rc_conf_d_list = config_list(dirfd(dp), fname->মান);
		TAILQ_FOREACH(সারি, rc_conf_d_list, entries)
			যদি (সারি->মান)
				rc_config_set_মান(config, সারি->মান);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	ফেরত;
}

স্থির RC_STRINGLIST *
config_load(পূর্ণসংখ্যা dirfd, ধ্রুবক অক্ষর *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *সারি;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(সারি, list, entries) {
		rc_config_set_মান(config, সারি->মান);
	}
	rc_stringlist_free(list);

	ফেরত config;
}

RC_STRINGLIST *
rc_config_load(ধ্রুবক অক্ষর *file)
{
	ফেরত config_load(AT_FDCWD, file);
}

অক্ষর *
rc_config_মান(RC_STRINGLIST *list, ধ্রুবক অক্ষর *entry)
{
	RC_STRING *সারি;
	অক্ষর *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(সারি, list, entries) {
		p = strchr(সারি->মান, '=');
		যদি (p != ফাঁকা) {
			যদি (strncmp(entry, সারি->মান, len) == 0 && সারি->মান[len] == '=')
				ফেরত ++p;
		}
	}
	ফেরত ফাঁকা;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
স্থির RC_STRINGLIST *rc_conf = ফাঁকা;

স্থির শূন্য
_free_rc_conf(শূন্য)
{
	rc_stringlist_free(rc_conf);
}

স্থির শূন্য
rc_conf_append(পূর্ণসংখ্যা dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

অক্ষর *
rc_conf_মান(ধ্রুবক অক্ষর *setting)
{
	ধ্রুবক পূর্ণসংখ্যা *dirfds;
	size_t dir_গুনতি = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	যদি (rc_conf)
		ফেরত rc_config_মান(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	জন্য (size_t i = 0; i < dir_গুনতি; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_গুনতি - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	যদি (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		অক্ষর *p = s->মান;
		যখন (p && *p && *p != '=') {
			যদি (isupper((unsigned অক্ষর)*p))
				*p = tolower((unsigned অক্ষর)*p);
			p++;
		}
	}

	ফেরত rc_config_মান(rc_conf, setting);
}
