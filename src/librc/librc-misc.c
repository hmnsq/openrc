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

#അന്തര്ഭാവ <fnmatch.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <dirent.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <sys/stat.h>

#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "librc.h"
#അന്തര്ഭാവ "helpers.h"
#അന്തര്ഭാവ "misc.h"

bool
rc_yesno(സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	എങ്കിൽ (!മൂല്യം) {
		errno = ENOENT;
		തിരികെ false;
	}

	എങ്കിൽ (strcasecmp(മൂല്യം, "yes") == 0 ||
	    strcasecmp(മൂല്യം, "y") == 0 ||
	    strcasecmp(മൂല്യം, "true") == 0 ||
	    strcasecmp(മൂല്യം, "1") == 0)
		തിരികെ true;

	എങ്കിൽ (strcasecmp(മൂല്യം, "no") != 0 &&
	    strcasecmp(മൂല്യം, "n") != 0 &&
	    strcasecmp(മൂല്യം, "false") != 0 &&
	    strcasecmp(മൂല്യം, "0") != 0)
		errno = EINVAL;

	തിരികെ false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(സ്ഥിരം അക്ഷരം *file, അക്ഷരം **buffer, size_t *len)
{
	bool ret = false;
	FILE *fp;
	പൂർണ്ണസംഖ്യ fd;
	ഘടന stat st;
	size_t done, ഇടത്;

	fp = fopen(file, "re");
	എങ്കിൽ (!fp)
		തിരികെ false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	എങ്കിൽ (fstat(fd, &st))
		പോകുക finished;

	ഇടത് = st.st_size;
	*len = ഇടത് + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	ആകുമ്പോൾ (ഇടത്) {
		done = fread(*buffer, വലുപ്പം(*buffer[0]), ഇടത്, fp);
		എങ്കിൽ (done == 0 && ferror(fp))
			പോകുക finished;
		ഇടത് -= done;
	}
	ret = true;

 finished:
	എങ്കിൽ (!ret) {
		സ്വതന്ത്രം(*buffer);
		*len = 0;
	} അല്ലെങ്കിൽ
		(*buffer)[*len - 1] = '\0';
	fclose(fp);
	തിരികെ ret;
}

അക്ഷരം *
rc_proc_getent(സ്ഥിരം അക്ഷരം *ent RC_UNUSED)
{
#ifdef __linux__
	FILE *fp;
	അക്ഷരം *proc = ശൂന്യ, *p, *മൂല്യം = ശൂന്യ, *save;
	size_t i, len;

	എങ്കിൽ (!(fp = fopen("/proc/cmdline", "r")))
		തിരികെ ശൂന്യ;

	i = 0;
	എങ്കിൽ (xgetline(&proc, &i, fp) == -1)
		പോകുക out;
	save = proc;

	len = strlen(ent);
	ആകുമ്പോൾ ((p = strsep(&save, " "))) {
		എങ്കിൽ (strncmp(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			എങ്കിൽ (*p == '=')
				p++;
			മൂല്യം = xstrdup(p);
		}
	}

	എങ്കിൽ (!മൂല്യം)
		errno = ENOENT;

out:
	fclose(fp);
	സ്വതന്ത്രം(proc);

	തിരികെ മൂല്യം;
#അല്ലെങ്കിൽ
	തിരികെ ശൂന്യ;
#endif
}

RC_STRINGLIST *
config_list(പൂർണ്ണസംഖ്യ dirfd, സ്ഥിരം അക്ഷരം *pathname)
{
	FILE *fp;
	അക്ഷരം *buffer = ശൂന്യ;
	size_t len = 0;
	അക്ഷരം *p;
	അക്ഷരം *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	എങ്കിൽ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		തിരികെ list;

	ആകുമ്പോൾ (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		ആകുമ്പോൾ ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		എങ്കിൽ (token && (strlen(token) > 1)) {
			/* If not variable assignment then skip */
			എങ്കിൽ (strchr(token, '=')) {
				/* Strip the newline if present */
				എങ്കിൽ (token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	സ്വതന്ത്രം(buffer);
	fclose(fp);

	തിരികെ list;
}

RC_STRINGLIST *
rc_config_list(സ്ഥിരം അക്ഷരം *file)
{
	തിരികെ config_list(AT_FDCWD, file);
}

സ്ഥിതം ശൂന്യം rc_config_set_മൂല്യം(RC_STRINGLIST *config, അക്ഷരം *മൂല്യം)
{
	RC_STRING *cline;
	അക്ഷരം *entry;
	size_t i = 0;
	അക്ഷരം *newline;
	അക്ഷരം *p = മൂല്യം;
	bool replaced;
	അക്ഷരം *token;

	എങ്കിൽ (!p)
		തിരികെ;
	എങ്കിൽ (strncmp(p, "export ", 7) == 0)
		p += 7;
	എങ്കിൽ (!(token = strsep(&p, "=")))
		തിരികെ;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	എങ്കിൽ (*p == '$')
		token = മൂല്യം;
	അല്ലെങ്കിൽ
		ചെയ്യുക {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} ആകുമ്പോൾ (token && *token == '\0');

	/* Drop a newline if that's all we have */
	എങ്കിൽ (token) {
		i = strlen(token) - 1;
		എങ്കിൽ (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", entry, token);
	} അല്ലെങ്കിൽ {
		xasprintf(&newline, "%s=", entry);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = strlen(entry);
		എങ്കിൽ (strncmp(entry, cline->മൂല്യം, i) == 0 && cline->മൂല്യം[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			സ്വതന്ത്രം(cline->മൂല്യം);
			cline->മൂല്യം = newline;
			replaced = true;
			നിർത്തുക;
		}
	}

	എങ്കിൽ (!replaced) {
		rc_stringlist_add(config, newline);
		സ്വതന്ത്രം(newline);
	}
	സ്വതന്ത്രം(entry);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
സ്ഥിതം RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#ifdef __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	അക്ഷരം *tmp = ശൂന്യ;
	അക്ഷരം *മൂല്യം = ശൂന്യ;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = strlen(override->മൂല്യം);
		മൂല്യം = rc_proc_getent(override->മൂല്യം);

		/* No need to continue if there's nothing to override */
		എങ്കിൽ (!മൂല്യം) {
			സ്വതന്ത്രം(മൂല്യം);
			തുടരുക;
		}

		എങ്കിൽ (മൂല്യം != ശൂന്യ) {
			xasprintf(&tmp, "%s=%s", override->മൂല്യം, മൂല്യം);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			എങ്കിൽ (strncmp(override->മൂല്യം, cline->മൂല്യം, varlen) == 0
				&& cline->മൂല്യം[varlen] == '=') {
				rc_stringlist_delete(config, cline->മൂല്യം);
				നിർത്തുക;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		സ്വതന്ത്രം(tmp);
		സ്വതന്ത്രം(മൂല്യം);
	}

	rc_stringlist_free(overrides);
#endif
	തിരികെ config;
}

സ്ഥിതം ശൂന്യം
rc_config_directory(RC_STRINGLIST *config, പൂർണ്ണസംഖ്യ targetfd, സ്ഥിരം അക്ഷരം *dir)
{
	DIR *dp;
	ഘടന dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *വരി;

	എങ്കിൽ (!(dp = do_opendirat(targetfd, dir)))
		തിരികെ;

	rc_conf_d_files = rc_stringlist_new();
	ആകുമ്പോൾ ((d = readdir(dp)) != ശൂന്യ) {
		എങ്കിൽ (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		എങ്കിൽ (!fname->മൂല്യം)
			തുടരുക;

		rc_conf_d_list = config_list(dirfd(dp), fname->മൂല്യം);
		TAILQ_FOREACH(വരി, rc_conf_d_list, entries)
			എങ്കിൽ (വരി->മൂല്യം)
				rc_config_set_മൂല്യം(config, വരി->മൂല്യം);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	തിരികെ;
}

സ്ഥിതം RC_STRINGLIST *
config_load(പൂർണ്ണസംഖ്യ dirfd, സ്ഥിരം അക്ഷരം *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *വരി;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(വരി, list, entries) {
		rc_config_set_മൂല്യം(config, വരി->മൂല്യം);
	}
	rc_stringlist_free(list);

	തിരികെ config;
}

RC_STRINGLIST *
rc_config_load(സ്ഥിരം അക്ഷരം *file)
{
	തിരികെ config_load(AT_FDCWD, file);
}

അക്ഷരം *
rc_config_മൂല്യം(RC_STRINGLIST *list, സ്ഥിരം അക്ഷരം *entry)
{
	RC_STRING *വരി;
	അക്ഷരം *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(വരി, list, entries) {
		p = strchr(വരി->മൂല്യം, '=');
		എങ്കിൽ (p != ശൂന്യ) {
			എങ്കിൽ (strncmp(entry, വരി->മൂല്യം, len) == 0 && വരി->മൂല്യം[len] == '=')
				തിരികെ ++p;
		}
	}
	തിരികെ ശൂന്യ;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
സ്ഥിതം RC_STRINGLIST *rc_conf = ശൂന്യ;

സ്ഥിതം ശൂന്യം
_free_rc_conf(ശൂന്യം)
{
	rc_stringlist_free(rc_conf);
}

സ്ഥിതം ശൂന്യം
rc_conf_append(പൂർണ്ണസംഖ്യ dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

അക്ഷരം *
rc_conf_മൂല്യം(സ്ഥിരം അക്ഷരം *setting)
{
	സ്ഥിരം പൂർണ്ണസംഖ്യ *dirfds;
	size_t dir_എണ്ണം = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	എങ്കിൽ (rc_conf)
		തിരികെ rc_config_മൂല്യം(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	വേണ്ടി (size_t i = 0; i < dir_എണ്ണം; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_എണ്ണം - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	എങ്കിൽ (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		അക്ഷരം *p = s->മൂല്യം;
		ആകുമ്പോൾ (p && *p && *p != '=') {
			എങ്കിൽ (isupper((unsigned അക്ഷരം)*p))
				*p = tolower((unsigned അക്ഷരം)*p);
			p++;
		}
	}

	തിരികെ rc_config_മൂല്യം(rc_conf, setting);
}
