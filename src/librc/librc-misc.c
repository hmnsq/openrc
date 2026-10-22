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

#ଅନ୍ତର୍ଭାଵ <fnmatch.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <dirent.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>

#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "librc.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"

bool
rc_yesno(ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	ଯଦି (!ମୂଲ୍ୟ) {
		errno = ENOENT;
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (strcasecmp(ମୂଲ୍ୟ, "yes") == 0 ||
	    strcasecmp(ମୂଲ୍ୟ, "y") == 0 ||
	    strcasecmp(ମୂଲ୍ୟ, "true") == 0 ||
	    strcasecmp(ମୂଲ୍ୟ, "1") == 0)
		ଫେରନ୍ତୁ true;

	ଯଦି (strcasecmp(ମୂଲ୍ୟ, "no") != 0 &&
	    strcasecmp(ମୂଲ୍ୟ, "n") != 0 &&
	    strcasecmp(ମୂଲ୍ୟ, "false") != 0 &&
	    strcasecmp(ମୂଲ୍ୟ, "0") != 0)
		errno = EINVAL;

	ଫେରନ୍ତୁ false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(ଅଚଳ ଅକ୍ଷର *file, ଅକ୍ଷର **buffer, size_t *len)
{
	bool ret = false;
	FILE *fp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
	ସଂରଚନା stat st;
	size_t done, ବାମ;

	fp = fopen(file, "re");
	ଯଦି (!fp)
		ଫେରନ୍ତୁ false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	ଯଦି (fstat(fd, &st))
		ଯାଅ finished;

	ବାମ = st.st_size;
	*len = ବାମ + 1; /* NUL terminator */
	*buffer = xrealloc(*buffer, *len);
	ଯେତେବେଳେ (ବାମ) {
		done = fread(*buffer, ଆକାର(*buffer[0]), ବାମ, fp);
		ଯଦି (done == 0 && ferror(fp))
			ଯାଅ finished;
		ବାମ -= done;
	}
	ret = true;

 finished:
	ଯଦି (!ret) {
		ମୁକ୍ତ(*buffer);
		*len = 0;
	} ଅନ୍ୟଥା
		(*buffer)[*len - 1] = '\0';
	fclose(fp);
	ଫେରନ୍ତୁ ret;
}

ଅକ୍ଷର *
rc_proc_getent(ଅଚଳ ଅକ୍ଷର *ent RC_UNUSED)
{
#ifdef __linux__
	FILE *fp;
	ଅକ୍ଷର *proc = ଖାଲି, *p, *ମୂଲ୍ୟ = ଖାଲି, *save;
	size_t i, len;

	ଯଦି (!(fp = fopen("/proc/cmdline", "r")))
		ଫେରନ୍ତୁ ଖାଲି;

	i = 0;
	ଯଦି (xgetline(&proc, &i, fp) == -1)
		ଯାଅ out;
	save = proc;

	len = strlen(ent);
	ଯେତେବେଳେ ((p = strsep(&save, " "))) {
		ଯଦି (strncmp(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			ଯଦି (*p == '=')
				p++;
			ମୂଲ୍ୟ = xstrdup(p);
		}
	}

	ଯଦି (!ମୂଲ୍ୟ)
		errno = ENOENT;

out:
	fclose(fp);
	ମୁକ୍ତ(proc);

	ଫେରନ୍ତୁ ମୂଲ୍ୟ;
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ ଖାଲି;
#endif
}

RC_STRINGLIST *
config_list(ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd, ଅଚଳ ଅକ୍ଷର *pathname)
{
	FILE *fp;
	ଅକ୍ଷର *buffer = ଖାଲି;
	size_t len = 0;
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	ଯଦି (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		ଫେରନ୍ତୁ list;

	ଯେତେବେଳେ (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		/* Strip leading spaces/tabs */
		ଯେତେବେଳେ ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		ଯଦି (token && (strlen(token) > 1)) {
			/* If not variable assignment then skip */
			ଯଦି (strchr(token, '=')) {
				/* Strip the newline if present */
				ଯଦି (token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = 0;

				rc_stringlist_add(list, token);
			}
		}
	}
	ମୁକ୍ତ(buffer);
	fclose(fp);

	ଫେରନ୍ତୁ list;
}

RC_STRINGLIST *
rc_config_list(ଅଚଳ ଅକ୍ଷର *file)
{
	ଫେରନ୍ତୁ config_list(AT_FDCWD, file);
}

ସ୍ଥିର ଶୂନ୍ୟ rc_config_set_ମୂଲ୍ୟ(RC_STRINGLIST *config, ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	RC_STRING *cline;
	ଅକ୍ଷର *entry;
	size_t i = 0;
	ଅକ୍ଷର *newline;
	ଅକ୍ଷର *p = ମୂଲ୍ୟ;
	bool replaced;
	ଅକ୍ଷର *token;

	ଯଦି (!p)
		ଫେରନ୍ତୁ;
	ଯଦି (strncmp(p, "export ", 7) == 0)
		p += 7;
	ଯଦି (!(token = strsep(&p, "=")))
		ଫେରନ୍ତୁ;

	entry = xstrdup(token);
	/* Preserve shell coloring */
	ଯଦି (*p == '$')
		token = ମୂଲ୍ୟ;
	ଅନ୍ୟଥା
		କର {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} ଯେତେବେଳେ (token && *token == '\0');

	/* Drop a newline if that's all we have */
	ଯଦି (token) {
		i = strlen(token) - 1;
		ଯଦି (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", entry, token);
	} ଅନ୍ୟଥା {
		xasprintf(&newline, "%s=", entry);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = strlen(entry);
		ଯଦି (strncmp(entry, cline->ମୂଲ୍ୟ, i) == 0 && cline->ମୂଲ୍ୟ[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			ମୁକ୍ତ(cline->ମୂଲ୍ୟ);
			cline->ମୂଲ୍ୟ = newline;
			replaced = true;
			ଭାଙ୍ଗ;
		}
	}

	ଯଦି (!replaced) {
		rc_stringlist_add(config, newline);
		ମୁକ୍ତ(newline);
	}
	ମୁକ୍ତ(entry);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
ସ୍ଥିର RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#ifdef __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	ଅକ୍ଷର *tmp = ଖାଲି;
	ଅକ୍ଷର *ମୂଲ୍ୟ = ଖାଲି;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_add(overrides, "rc_interactive");
	rc_stringlist_add(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = strlen(override->ମୂଲ୍ୟ);
		ମୂଲ୍ୟ = rc_proc_getent(override->ମୂଲ୍ୟ);

		/* No need to continue if there's nothing to override */
		ଯଦି (!ମୂଲ୍ୟ) {
			ମୁକ୍ତ(ମୂଲ୍ୟ);
			ଜାରି;
		}

		ଯଦି (ମୂଲ୍ୟ != ଖାଲି) {
			xasprintf(&tmp, "%s=%s", override->ମୂଲ୍ୟ, ମୂଲ୍ୟ);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_SAFE(cline, config, entries, config_np) {
			ଯଦି (strncmp(override->ମୂଲ୍ୟ, cline->ମୂଲ୍ୟ, varlen) == 0
				&& cline->ମୂଲ୍ୟ[varlen] == '=') {
				rc_stringlist_delete(config, cline->ମୂଲ୍ୟ);
				ଭାଙ୍ଗ;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_add(config, tmp);

		ମୁକ୍ତ(tmp);
		ମୁକ୍ତ(ମୂଲ୍ୟ);
	}

	rc_stringlist_free(overrides);
#endif
	ଫେରନ୍ତୁ config;
}

ସ୍ଥିର ଶୂନ୍ୟ
rc_config_directory(RC_STRINGLIST *config, ପୂର୍ଣ୍ଣସଂଖ୍ୟା targetfd, ଅଚଳ ଅକ୍ଷର *dir)
{
	DIR *dp;
	ସଂରଚନା dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *ଧାଡ଼ି;

	ଯଦି (!(dp = do_opendirat(targetfd, dir)))
		ଫେରନ୍ତୁ;

	rc_conf_d_files = rc_stringlist_new();
	ଯେତେବେଳେ ((d = readdir(dp)) != ଖାଲି) {
		ଯଦି (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_sort(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		ଯଦି (!fname->ମୂଲ୍ୟ)
			ଜାରି;

		rc_conf_d_list = config_list(dirfd(dp), fname->ମୂଲ୍ୟ);
		TAILQ_FOREACH(ଧାଡ଼ି, rc_conf_d_list, entries)
			ଯଦି (ଧାଡ଼ି->ମୂଲ୍ୟ)
				rc_config_set_ମୂଲ୍ୟ(config, ଧାଡ଼ି->ମୂଲ୍ୟ);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	ଫେରନ୍ତୁ;
}

ସ୍ଥିର RC_STRINGLIST *
config_load(ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd, ଅଚଳ ଅକ୍ଷର *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *ଧାଡ଼ି;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(ଧାଡ଼ି, list, entries) {
		rc_config_set_ମୂଲ୍ୟ(config, ଧାଡ଼ି->ମୂଲ୍ୟ);
	}
	rc_stringlist_free(list);

	ଫେରନ୍ତୁ config;
}

RC_STRINGLIST *
rc_config_load(ଅଚଳ ଅକ୍ଷର *file)
{
	ଫେରନ୍ତୁ config_load(AT_FDCWD, file);
}

ଅକ୍ଷର *
rc_config_ମୂଲ୍ୟ(RC_STRINGLIST *list, ଅଚଳ ଅକ୍ଷର *entry)
{
	RC_STRING *ଧାଡ଼ି;
	ଅକ୍ଷର *p;
	size_t len;

	len = strlen(entry);
	TAILQ_FOREACH(ଧାଡ଼ି, list, entries) {
		p = strchr(ଧାଡ଼ି->ମୂଲ୍ୟ, '=');
		ଯଦି (p != ଖାଲି) {
			ଯଦି (strncmp(entry, ଧାଡ଼ି->ମୂଲ୍ୟ, len) == 0 && ଧାଡ଼ି->ମୂଲ୍ୟ[len] == '=')
				ଫେରନ୍ତୁ ++p;
		}
	}
	ଫେରନ୍ତୁ ଖାଲି;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
ସ୍ଥିର RC_STRINGLIST *rc_conf = ଖାଲି;

ସ୍ଥିର ଶୂନ୍ୟ
_free_rc_conf(ଶୂନ୍ୟ)
{
	rc_stringlist_free(rc_conf);
}

ସ୍ଥିର ଶୂନ୍ୟ
rc_conf_append(ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd)
{
	RC_STRINGLIST *conf = config_load(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

ଅକ୍ଷର *
rc_conf_ମୂଲ୍ୟ(ଅଚଳ ଅକ୍ଷର *setting)
{
	ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା *dirfds;
	size_t dir_ଗଣନା = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	ଯଦି (rc_conf)
		ଫେରନ୍ତୁ rc_config_ମୂଲ୍ୟ(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	ପାଇଁ (size_t i = 0; i < dir_ଗଣନା; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_ଗଣନା - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	ଯଦି (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_load(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		ଅକ୍ଷର *p = s->ମୂଲ୍ୟ;
		ଯେତେବେଳେ (p && *p && *p != '=') {
			ଯଦି (isupper((unsigned ଅକ୍ଷର)*p))
				*p = tolower((unsigned ଅକ୍ଷର)*p);
			p++;
		}
	}

	ଫେରନ୍ତୁ rc_config_ମୂଲ୍ୟ(rc_conf, setting);
}
