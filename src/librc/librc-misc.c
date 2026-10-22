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

#అన్తర్భావ <fnmatch.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <dirent.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <sys/stat.h>

#అన్తర్భావ "queue.h"
#అన్తర్భావ "librc.h"
#అన్తర్భావ "helpers.h"
#అన్తర్భావ "misc.h"

bool
rc_yesno(స్థిరరాశి అక్షరం *విలువ)
{
	ఒకవేళ (!విలువ) {
		errno = ENOENT;
		తిరిగి false;
	}

	ఒకవేళ (strcasecmp(విలువ, "yes") == 0 ||
	    strcasecmp(విలువ, "y") == 0 ||
	    strcasecmp(విలువ, "true") == 0 ||
	    strcasecmp(విలువ, "1") == 0)
		తిరిగి true;

	ఒకవేళ (strcasecmp(విలువ, "no") != 0 &&
	    strcasecmp(విలువ, "n") != 0 &&
	    strcasecmp(విలువ, "false") != 0 &&
	    strcasecmp(విలువ, "0") != 0)
		errno = EINVAL;

	తిరిగి false;
}


/**
 * Read the entire @file into the buffer and set @len to the
 * size of the buffer when finished. For C strings, this will
 * be strlen(buffer) + 1.
 * Don't forget to free the buffer afterwards!
 */
bool
rc_getfile(స్థిరరాశి అక్షరం *file, అక్షరం **బఫరు, size_t *len)
{
	bool ret = false;
	FILE *fp;
	పూర్ణాంకం fd;
	నిర్మాణం stat st;
	size_t పూర్తి, ఎడమ;

	fp = fopen(file, "re");
	ఒకవేళ (!fp)
		తిరిగి false;

	/* assume fileno() never fails */
	fd = fileno(fp);

	ఒకవేళ (fstat(fd, &st))
		వెళ్ళు ముగిసిన;

	ఎడమ = st.st_size;
	*len = ఎడమ + 1; /* NUL terminator */
	*బఫరు = xrealloc(*బఫరు, *len);
	అయితే (ఎడమ) {
		పూర్తి = fread(*బఫరు, పరిమాణం(*బఫరు[0]), ఎడమ, fp);
		ఒకవేళ (పూర్తి == 0 && ferror(fp))
			వెళ్ళు ముగిసిన;
		ఎడమ -= పూర్తి;
	}
	ret = true;

 ముగిసిన:
	ఒకవేళ (!ret) {
		విడుదల(*బఫరు);
		*len = 0;
	} లేకపోతే
		(*బఫరు)[*len - 1] = '\0';
	fclose(fp);
	తిరిగి ret;
}

అక్షరం *
rc_proc_getent(స్థిరరాశి అక్షరం *ent RC_వాడని)
{
#నిర్వచితమైతే __linux__
	FILE *fp;
	అక్షరం *proc = ఖాళీ, *p, *విలువ = ఖాళీ, *save;
	size_t i, len;

	ఒకవేళ (!(fp = fopen("/proc/cmdline", "r")))
		తిరిగి ఖాళీ;

	i = 0;
	ఒకవేళ (xgetline(&proc, &i, fp) == -1)
		వెళ్ళు బహిర్గతం;
	save = proc;

	len = అక్షరశ్రేణి_పొడవు(ent);
	అయితే ((p = strsep(&save, " "))) {
		ఒకవేళ (అక్షరశ్రేణి_పోలిక(ent, p, len) == 0 && (p[len] == '\0' || p[len] == ' ' || p[len] == '=')) {
			p += len;
			ఒకవేళ (*p == '=')
				p++;
			విలువ = xstrdup(p);
		}
	}

	ఒకవేళ (!విలువ)
		errno = ENOENT;

బహిర్గతం:
	fclose(fp);
	విడుదల(proc);

	తిరిగి విలువ;
#లేకపోతే
	తిరిగి ఖాళీ;
#అయితే_ముగింపు
}

RC_STRINGLIST *
config_list(పూర్ణాంకం dirfd, స్థిరరాశి అక్షరం *pathname)
{
	FILE *fp;
	అక్షరం *బఫరు = ఖాళీ;
	size_t len = 0;
	అక్షరం *p;
	అక్షరం *token;
	RC_STRINGLIST *list = rc_stringlist_new();

	ఒకవేళ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		తిరిగి list;

	అయితే (xgetline(&బఫరు, &len, fp) != -1) {
		p = బఫరు;
		/* Strip leading spaces/tabs */
		అయితే ((*p == ' ') || (*p == '\t'))
			p++;

		/* Get entry - we do not want comments */
		token = strsep(&p, "#");
		ఒకవేళ (token && (అక్షరశ్రేణి_పొడవు(token) > 1)) {
			/* If not variable assignment then skip */
			ఒకవేళ (strchr(token, '=')) {
				/* Strip the newline if present */
				ఒకవేళ (token[అక్షరశ్రేణి_పొడవు(token) - 1] == '\n')
					token[అక్షరశ్రేణి_పొడవు(token) - 1] = 0;

				rc_stringlist_కూడు(list, token);
			}
		}
	}
	విడుదల(బఫరు);
	fclose(fp);

	తిరిగి list;
}

RC_STRINGLIST *
rc_config_list(స్థిరరాశి అక్షరం *file)
{
	తిరిగి config_list(AT_FDCWD, file);
}

స్థిర శూన్య rc_config_సమితి_విలువ(RC_STRINGLIST *config, అక్షరం *విలువ)
{
	RC_STRING *cline;
	అక్షరం *నమోదు_2;
	size_t i = 0;
	అక్షరం *newline;
	అక్షరం *p = విలువ;
	bool replaced;
	అక్షరం *token;

	ఒకవేళ (!p)
		తిరిగి;
	ఒకవేళ (అక్షరశ్రేణి_పోలిక(p, "export ", 7) == 0)
		p += 7;
	ఒకవేళ (!(token = strsep(&p, "=")))
		తిరిగి;

	నమోదు_2 = xstrdup(token);
	/* Preserve shell coloring */
	ఒకవేళ (*p == '$')
		token = విలువ;
	లేకపోతే
		చేయి {
			/* Bash variables are usually quoted */
			token = strsep(&p, "\"\'");
		} అయితే (token && *token == '\0');

	/* Drop a newline if that's all we have */
	ఒకవేళ (token) {
		i = అక్షరశ్రేణి_పొడవు(token) - 1;
		ఒకవేళ (token[i] == '\n')
			token[i] = 0;

		xasprintf(&newline, "%s=%s", నమోదు_2, token);
	} లేకపోతే {
		xasprintf(&newline, "%s=", నమోదు_2);
	}

	replaced = false;
	/* In shells the last item takes precedence, so we need to remove
	   any prior values we may already have */
	TAILQ_FOREACH(cline, config, entries) {
		i = అక్షరశ్రేణి_పొడవు(నమోదు_2);
		ఒకవేళ (అక్షరశ్రేణి_పోలిక(నమోదు_2, cline->విలువ, i) == 0 && cline->విలువ[i] == '=') {
			/* We have a match now - to save time we directly replace it */
			విడుదల(cline->విలువ);
			cline->విలువ = newline;
			replaced = true;
			ఆపు;
		}
	}

	ఒకవేళ (!replaced) {
		rc_stringlist_కూడు(config, newline);
		విడుదల(newline);
	}
	విడుదల(నమోదు_2);
}

/*
 * Override some specific rc.conf options on the kernel command line.
 * I only know how to do this in Linux, so if someone wants to supply
 * a patch for this on *BSD or tell me how to write the code to do this,
 * any suggestions are welcome.
 */
స్థిర RC_STRINGLIST *rc_config_kcl(RC_STRINGLIST *config)
{
#నిర్వచితమైతే __linux__
	RC_STRINGLIST *overrides;
	RC_STRING *cline, *override, *config_np;
	అక్షరం *tmp = ఖాళీ;
	అక్షరం *విలువ = ఖాళీ;
	size_t varlen = 0;

	overrides = rc_stringlist_new();

	/* A list of variables which may be overridden on the kernel command line */
	rc_stringlist_కూడు(overrides, "rc_interactive");
	rc_stringlist_కూడు(overrides, "rc_parallel");

	TAILQ_FOREACH(override, overrides, entries) {
		varlen = అక్షరశ్రేణి_పొడవు(override->విలువ);
		విలువ = rc_proc_getent(override->విలువ);

		/* No need to continue if there's nothing to override */
		ఒకవేళ (!విలువ) {
			విడుదల(విలువ);
			కొనసాగు;
		}

		ఒకవేళ (విలువ != ఖాళీ) {
			xasprintf(&tmp, "%s=%s", override->విలువ, విలువ);
		}

		/*
		 * Whenever necessary remove the old config entry first to prevent
		 * duplicates
		 */
		TAILQ_FOREACH_సురక్షిత(cline, config, entries, config_np) {
			ఒకవేళ (అక్షరశ్రేణి_పోలిక(override->విలువ, cline->విలువ, varlen) == 0
				&& cline->విలువ[varlen] == '=') {
				rc_stringlist_delete(config, cline->విలువ);
				ఆపు;
			}
		}

		/* Add the option (var/value) to the current config */
		rc_stringlist_కూడు(config, tmp);

		విడుదల(tmp);
		విడుదల(విలువ);
	}

	rc_stringlist_free(overrides);
#అయితే_ముగింపు
	తిరిగి config;
}

స్థిర శూన్య
rc_config_directory(RC_STRINGLIST *config, పూర్ణాంకం targetfd, స్థిరరాశి అక్షరం *dir)
{
	DIR *dp;
	నిర్మాణం dirent *d;
	RC_STRINGLIST *rc_conf_d_files;
	RC_STRING *fname;
	RC_STRINGLIST *rc_conf_d_list;
	RC_STRING *పంక్తి;

	ఒకవేళ (!(dp = do_opendirat(targetfd, dir)))
		తిరిగి;

	rc_conf_d_files = rc_stringlist_new();
	అయితే ((d = readdir(dp)) != ఖాళీ) {
		ఒకవేళ (fnmatch("*.conf", d->d_name, FNM_PATHNAME) == 0) {
			rc_stringlist_addu(rc_conf_d_files, d->d_name);
		}
	}

	rc_stringlist_క్రమీకరించు(&rc_conf_d_files);
	TAILQ_FOREACH(fname, rc_conf_d_files, entries) {
		ఒకవేళ (!fname->విలువ)
			కొనసాగు;

		rc_conf_d_list = config_list(dirfd(dp), fname->విలువ);
		TAILQ_FOREACH(పంక్తి, rc_conf_d_list, entries)
			ఒకవేళ (పంక్తి->విలువ)
				rc_config_సమితి_విలువ(config, పంక్తి->విలువ);
		rc_stringlist_free(rc_conf_d_list);
	}

	rc_stringlist_free(rc_conf_d_files);
	closedir(dp);

	తిరిగి;
}

స్థిర RC_STRINGLIST *
config_ఎక్కించు(పూర్ణాంకం dirfd, స్థిరరాశి అక్షరం *pathname)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *config;
	RC_STRING *పంక్తి;

	list = config_list(dirfd, pathname);
	config = rc_stringlist_new();
	TAILQ_FOREACH(పంక్తి, list, entries) {
		rc_config_సమితి_విలువ(config, పంక్తి->విలువ);
	}
	rc_stringlist_free(list);

	తిరిగి config;
}

RC_STRINGLIST *
rc_config_ఎక్కించు(స్థిరరాశి అక్షరం *file)
{
	తిరిగి config_ఎక్కించు(AT_FDCWD, file);
}

అక్షరం *
rc_config_విలువ(RC_STRINGLIST *list, స్థిరరాశి అక్షరం *నమోదు_2)
{
	RC_STRING *పంక్తి;
	అక్షరం *p;
	size_t len;

	len = అక్షరశ్రేణి_పొడవు(నమోదు_2);
	TAILQ_FOREACH(పంక్తి, list, entries) {
		p = strchr(పంక్తి->విలువ, '=');
		ఒకవేళ (p != ఖాళీ) {
			ఒకవేళ (అక్షరశ్రేణి_పోలిక(నమోదు_2, పంక్తి->విలువ, len) == 0 && పంక్తి->విలువ[len] == '=')
				తిరిగి ++p;
		}
	}
	తిరిగి ఖాళీ;
}

/* Global for caching the strings loaded from rc.conf to avoid reparsing for
 * each rc_conf_value call */
స్థిర RC_STRINGLIST *rc_conf = ఖాళీ;

స్థిర శూన్య
_free_rc_conf(శూన్య)
{
	rc_stringlist_free(rc_conf);
}

స్థిర శూన్య
rc_conf_append(పూర్ణాంకం dirfd)
{
	RC_STRINGLIST *conf = config_ఎక్కించు(dirfd, "rc.conf");
	TAILQ_CONCAT(rc_conf, conf, entries);
	rc_stringlist_free(conf);
}

అక్షరం *
rc_conf_విలువ(స్థిరరాశి అక్షరం *setting)
{
	స్థిరరాశి పూర్ణాంకం *dirfds;
	size_t dir_లెక్క = rc_scriptdirfds(&dirfds);
	RC_STRING *s;

	ఒకవేళ (rc_conf)
		తిరిగి rc_config_విలువ(rc_conf, setting);

	rc_conf = rc_stringlist_new();
	atexit(_free_rc_conf);

	కొరకు (size_t i = 0; i < dir_లెక్క; i++) {
		/* load configs in reverse order, so earlier entries override later ones */
		size_t idx = dir_లెక్క - 1 - i;
		rc_conf_append(dirfds[idx]);
		rc_config_directory(rc_conf, dirfds[idx], "rc.conf.d");
	}

	/* Support old configs. */
	ఒకవేళ (!rc_is_user() && access(RC_CONF_OLD, F_OK) == 0) {
		RC_STRINGLIST *old_conf = config_ఎక్కించు(AT_FDCWD, RC_CONF_OLD);
		TAILQ_CONCAT(rc_conf, old_conf, entries);
		rc_stringlist_free(old_conf);
	}

	rc_conf = rc_config_kcl(rc_conf);

	/* Convert old uppercase to lowercase */
	TAILQ_FOREACH(s, rc_conf, entries) {
		అక్షరం *p = s->విలువ;
		అయితే (p && *p && *p != '=') {
			ఒకవేళ (isupper((గుర్తులేని అక్షరం)*p))
				*p = tolower((గుర్తులేని అక్షరం)*p);
			p++;
		}
	}

	తిరిగి rc_config_విలువ(rc_conf, setting);
}
