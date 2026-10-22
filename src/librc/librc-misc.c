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
rc_yesno(ধ্রুব বর্ণ *মান)
{
	যদি (!মান) {
		errno = ENOENT;
		প্রত্যাগম false;
	}

	যদি (strcasecmp(মান, "yes") == 0 ||
	    strcasecmp(মান, "y") == 0 ||
	    strcasecmp(মান, "true") == 0 ||
	    strcasecmp(মান, "1") == 0)
		প্রত্যাগম true;

	যদি (strcasecmp(মান, "no") != 0 &&
	    strcasecmp(মান, "n") != 0 &&
	    strcasecmp(মান, "false") != 0 &&
	    strcasecmp(মান, "0") != 0)
		errno = EINVAL;

	প্রত্যাগম false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(ধ্রুব বর্ণ *file, বর্ণ **buffer, size_t *len)
{
	bool ret = false;
	FILE *fp;
	পূর্ণাঙ্ক fd;
	সংরচনা stat st;
	size_t done, বাম;

	fp = fopen(file, "re");
	যদি (!fp)
		প্রত্যাগম false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	যদি (fstat(fd, &st))
		গচ্ছ finished;

	বাম = st.st_size;
	*len = বাম + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	যাবত্ (বাম) {
		done = fread(*buffer, পরিমাণ(*buffer[0]), বাম, fp);
		যদি (done == 0 && ferror(fp))
			গচ্ছ finished;
		বাম -= done;
	}
	ret = true;

 finished:
	যদি (!ret) {
		মুক্ত(*buffer);
		*len = 0;
	} অন্যথা
		(*buffer)[*len - 1] = '\0';
	fclose(fp);
	প্রত্যাগম ret;
}

বর্ণ *
rc_proc_getent(ধ্রুব বর্ণ *ent RC_UNUSED)
{
#ifdef __linux__
	FILE *fp;
	বর্ণ *proc = রিক্ত, *p, *মান = রিক্ত, *save;
	size_t i, len;

	যদি (!(fp = fopen("/proc/cmdline", "r")))
		প্রত্যাগম রিক্ত;

	i = 0;
	যদি (xgetline(&proc, &i, fp) == -1)
		গচ্ছ out;
	save = proc;

	len = strlen(ent);
	যাবত্ ((p = strsep(&save, " "))) {
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

	প্রত্যাগম মান;
#অন্যথা
	প্রত্যাগম রিক্ত;
#endif
}

RC_STRINGLIST *
config_list(পূর্ণাঙ্ক dirfd, ধ্রুব বর্ণ *pathname)
{
	FILE *fp;
	বর্ণ *buffer = রিক্ত;
	size_t len = 0;
	বর্ণ *p;
	বর্ণ *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	যদি (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		প্রত্যাগম list;

	যাবত্ (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		যাবত্ ((*p == ' ') || (*p == '\t'))
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
	মুক্ত(buffer);
	fclose(fp);

	প্রত্যাগম list;
}

RC_STRINGLIST *
rc_config_list(ধ্রুব বর্ণ *file)
{
	প্রত্যাগম config_list(AT_FDCWD, file);
}

স্থির শূন্য rc_config_set_মান(RC_STRINGLIST *config, বর্ণ *মান)
{
	RC_STRING *cline;
	বর্ণ *entry;
	size_t i = 0;
	বর্ণ *newline;
	বর্ণ *p = মান;
	bool replaced;
	বর্ণ *token;

	যদি (!p)
		প্রত্যাগম;
	যদি (strncmp(p, "export ", 7) == 0)
		p += 7;
	যদি (!(token = strsep(&p, "=")))
		প্রত্যাগম;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	যদি (*p == '$')
		token = মান;
	অন্যথা
		কুরু {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} যাবত্ (token && *token == '\0');

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
			ভঙ্গ;
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
	বর্ণ *tmp = রিক্ত;
	বর্ণ *মান = রিক্ত;
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
			অনুবর্ত;
		}

		যদি (মান != রিক্ত) {
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
				ভঙ্গ;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		মুক্ত(tmp);
		মুক্ত(মান);
	}

	rc_stringlist_free(overrides);
#endif
	প্রত্যাগম config;
}

স্থির শূন্য
rc_config_directory(RC_STRINGLIST *config, পূর্ণাঙ্ক targetfd, ধ্রুব বর্ণ *dir)
{
	DIR *dp;
	সংরচনা dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *সারি;

	যদি (!(dp = do_opendirat(targetfd, dir)))
		প্রত্যাগম;

	rc_conf_d_files = rc_stringlist_new();
	যাবত্ ((d = readdir(dp)) != রিক্ত) {
		যদি (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		যদি (!fname->মান)
			অনুবর্ত;

		rc_conf_d_list = config_list(dirfd(dp), fname->মান);
		TAILQ_FOREACH(সারি, rc_conf_d_list, entries)
			যদি (সারি->মান)
				rc_config_set_মান(config, সারি->মান);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	প্রত্যাগম;
}

স্থির RC_STRINGLIST *
config_load(পূর্ণাঙ্ক dirfd, ধ্রুব বর্ণ *pathname)
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

	প্রত্যাগম config;
}

RC_STRINGLIST *
rc_config_load(ধ্রুব বর্ণ *file)
{
	প্রত্যাগম config_load(AT_FDCWD, file);
}

বর্ণ *
rc_config_মান(RC_STRINGLIST *list, ধ্রুব বর্ণ *entry)
{
	RC_STRING *সারি;
	বর্ণ *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(সারি, list, entries) {
		p = strchr(সারি->মান, '=');
		যদি (p != রিক্ত) {
			যদি (strncmp(entry, সারি->মান, len) == 0 && সারি->মান[len] == '=')
				প্রত্যাগম ++p;
		}
	}
	প্রত্যাগম রিক্ত;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
স্থির RC_STRINGLIST *rc_conf = রিক্ত;

স্থির শূন্য
_free_rc_conf(শূন্য)
{
	rc_stringlist_free(rc_conf);
}

স্থির শূন্য
rc_conf_append(পূর্ণাঙ্ক dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

বর্ণ *
rc_conf_মান(ধ্রুব বর্ণ *setting)
{
	ধ্রুব পূর্ণাঙ্ক *dirfds;
	size_t dir_গুনতি = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	যদি (rc_conf)
		প্রত্যাগম rc_config_মান(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	প্রতি (size_t i = 0; i < dir_গুনতি; i++) {
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
		বর্ণ *p = s->মান;
		যাবত্ (p && *p && *p != '=') {
			যদি (isupper((unsigned বর্ণ)*p))
				*p = tolower((unsigned বর্ণ)*p);
			p++;
		}
	}

	প্রত্যাগম rc_config_মান(rc_conf, setting);
}
