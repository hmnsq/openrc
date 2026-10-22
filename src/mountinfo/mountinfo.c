/*
 * mountinfo.c
 * Obtains information about mounted filesystems.
 */

/*
 * Copyright 2007-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ಆದರೆ defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#ಅಥವಾಆದರೆ defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#ಅಥವಾಆದರೆ defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#ಅಥವಾಆದರೆ defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <regex.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "rc_exec.h"
#ಅನ್ತರ್ಭಾವ "timeutils.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *procmounts = "/proc/mounts";
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts = "[mount1] [mount2] ...";
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{ "fstype-regex",        1, ಖಾಲಿ, 'f'},
	{ "skip-fstype-regex",   1, ಖಾಲಿ, 'F'},
	{ "node-regex",          1, ಖಾಲಿ, 'n'},
	{ "skip-node-regex",     1, ಖಾಲಿ, 'N'},
	{ "options-regex",       1, ಖಾಲಿ, 'o'},
	{ "skip-options-regex",  1, ಖಾಲಿ, 'O'},
	{ "point-regex",         1, ಖಾಲಿ, 'p'},
	{ "skip-point-regex",    1, ಖಾಲಿ, 'P'},
	{ "options",             0, ಖಾಲಿ, 'i'},
	{ "fstype",              0, ಖಾಲಿ, 's'},
	{ "node",                0, ಖಾಲಿ, 't'},
	{ "netdev",              0, ಖಾಲಿ, 'e'},
	{ "nonetdev",            0, ಖಾಲಿ, 'E'},
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[] = {
	"fstype regex to find",
	"fstype regex to skip",
	"node regex to find",
	"node regex to skip",
	"options regex to find",
	"options regex to skip",
	"point regex to find",
	"point regex to skip",
	"print options",
	"print fstype",
	"print node",
	"is it a network device",
	"is it not a network device",
	longopts_help_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ಖಾಲಿ;

#ವ್ಯಾಖ್ಯಾನಿಸು UMOUNT_ವಾದಗಳು_MAX 16
#ವ್ಯಾಖ್ಯಾನಿಸು ಸತತ_MAX 32
#ವ್ಯಾಖ್ಯಾನಿಸು TRY_MAX 3
#ವ್ಯಾಖ್ಯಾನಿಸು TRY_DELAY_MS 1000

ರಚನೆ ಸತತ_ಸರತಿ {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mntpath;
	int64_t ಕೊನೆಯ_exec_ಸಮಯ;
	int64_t fuser_exec_ಸಮಯ;
	pid_t pid;
	pid_t fuser_pid;
	ಪೂರ್ಣಾಂಕ try_ಎಣಿಕೆ;
	ಪೂರ್ಣಾಂಕ fuser_stdoutfd;
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	mount_from,
	mount_ಗಮ್ಯ,
	mount_fstype,
	mount_options
} mount_type;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	net_ignore,
	net_yes,
	net_no
} net_opts;

ರಚನೆ ವಾದಗಳು;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ process_func_t(RC_STRINGLIST *, ರಚನೆ ವಾದಗಳು *,
    ಅಕ್ಷರ *, ಅಕ್ಷರ *, ಅಕ್ಷರ *, ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ);

ರಚನೆ ವಾದಗಳು {
	process_func_t *process;
	regex_t *ಗಂಟು_regex;
	regex_t *skip_ಗಂಟು_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *check_mntpath;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
process_mount(RC_STRINGLIST *list, ರಚನೆ ವಾದಗಳು *ವಾದಗಳು,
    ಅಕ್ಷರ *from, ಅಕ್ಷರ *ಗಮ್ಯ, ಅಕ್ಷರ *fstype, ಅಕ್ಷರ *options,
    ಪೂರ್ಣಾಂಕ netdev)
{
	ಅಕ್ಷರ *p;
	RC_STRING *s;

	errno = ENOENT;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	/* Skip the really silly rootfs */
	ಆದರೆ (strcmp(fstype, "rootfs") == 0)
		ಹಿಂತಿರುಗಿ -1;
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (ವಾದಗಳು->netdev == net_yes &&
	    (netdev != -1 || TAILQ_ಮೊದಲ(ವಾದಗಳು->mounts)))
	{
		ಆದರೆ (netdev != 0)
			ಹಿಂತಿರುಗಿ 1;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ವಾದಗಳು->netdev == net_no &&
	    (netdev != -1 || TAILQ_ಮೊದಲ(ವಾದಗಳು->mounts)))
	{
		ಆದರೆ (netdev != 1)
			ಹಿಂತಿರುಗಿ 1;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (ವಾದಗಳು->ಗಂಟು_regex &&
		    regexec(ವಾದಗಳು->ಗಂಟು_regex, from, 0, ಖಾಲಿ, 0) != 0)
			ಹಿಂತಿರುಗಿ 1;
		ಆದರೆ (ವಾದಗಳು->skip_ಗಂಟು_regex &&
		    regexec(ವಾದಗಳು->skip_ಗಂಟು_regex, from, 0, ಖಾಲಿ, 0) == 0)
			ಹಿಂತಿರುಗಿ 1;

		ಆದರೆ (ವಾದಗಳು->fstype_regex &&
		    regexec(ವಾದಗಳು->fstype_regex, fstype, 0, ಖಾಲಿ, 0) != 0)
			ಹಿಂತಿರುಗಿ -1;
		ಆದರೆ (ವಾದಗಳು->skip_fstype_regex &&
		    regexec(ವಾದಗಳು->skip_fstype_regex, fstype, 0, ಖಾಲಿ, 0) == 0)
			ಹಿಂತಿರುಗಿ -1;

		ಆದರೆ (ವಾದಗಳು->options_regex &&
		    regexec(ವಾದಗಳು->options_regex, options, 0, ಖಾಲಿ, 0) != 0)
			ಹಿಂತಿರುಗಿ -1;
		ಆದರೆ (ವಾದಗಳು->skip_options_regex &&
		    regexec(ವಾದಗಳು->skip_options_regex, options, 0, ಖಾಲಿ, 0) == 0)
			ಹಿಂತಿರುಗಿ -1;
	}

	ಆದರೆ (TAILQ_ಮೊದಲ(ವಾದಗಳು->mounts)) {
		TAILQ_FOREACH(s, ವಾದಗಳು->mounts, entries)
		    ಆದರೆ (strcmp(s->ಮೌಲ್ಯ, ಗಮ್ಯ) == 0)
			    ನಿಲ್ಲಿಸು;
		ಆದರೆ (!s)
			ಹಿಂತಿರುಗಿ -1;
	}

	ಆಯ್ಕೆ (ವಾದಗಳು->mount_type) {
	ಸ್ಥಿತಿ mount_from:
		p = from;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ mount_ಗಮ್ಯ:
		p = ಗಮ್ಯ;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ mount_fstype:
		p = fstype;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ mount_options:
		p = options;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		p = ಖಾಲಿ;
		errno = EINVAL;
		ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (p) {
		errno = 0;
		rc_stringlist_ಕೂಡಿಸು(list, p);
		ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ -1;
}

#ಆದರೆ defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
ಸ್ಥಿರ ರಚನೆ opt {
	ಪೂರ್ಣಾಂಕ o_opt;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *o_name;
} optnames[] = {
	{ MNT_ASYNC,        "asynchronous" },
	{ MNT_EXPORTED,     "NFS exported" },
	{ MNT_LOCAL,        "local" },
	{ MNT_NOATIME,      "noatime" },
	{ MNT_NOEXEC,       "noexec" },
	{ MNT_NOSUID,       "nosuid" },
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MNT_NOSYMFOLLOW
	{ MNT_NOSYMFOLLOW,  "nosymfollow" },
#ಆದರೆ_ಅಂತ್ಯ
	{ MNT_QUOTA,        "with quotas" },
	{ MNT_RDONLY,       "read-only" },
	{ MNT_SYNCHRONOUS,  "synchronous" },
	{ MNT_UNION,        "union" },
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MNT_NOCLUSTERR
	{ MNT_NOCLUSTERR,   "noclusterr" },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MNT_NOCLUSTERW
	{ MNT_NOCLUSTERW,   "noclusterw" },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MNT_SUIDDIR
	{ MNT_SUIDDIR,      "suiddir" },
#ಆದರೆ_ಅಂತ್ಯ
	{ MNT_SOFTDEP,      "soft-updates" },
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MNT_MULTILABEL
	{ MNT_MULTILABEL,   "multilabel" },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MNT_ACLS
	{ MNT_ACLS,         "acls" },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MNT_GJOURNAL
	{ MNT_GJOURNAL,     "gjournal" },
#ಆದರೆ_ಅಂತ್ಯ
	{ 0, ಖಾಲಿ }
};

ಸ್ಥಿರ RC_STRINGLIST *
ಕಂಡುಹಿಡಿ_mounts(ರಚನೆ ವಾದಗಳು *ವಾದಗಳು, size_t *ಅಂಕೆ_mounts)
{
	ರಚನೆ statfs *mnts;
	ಪೂರ್ಣಾಂಕ nmnts;
	ಪೂರ್ಣಾಂಕ i;
	RC_STRINGLIST *list;
	ಅಕ್ಷರ *options = ಖಾಲಿ;
	uint64_t flags;
	ರಚನೆ opt *o;
	ಪೂರ್ಣಾಂಕ netdev;
	ಅಕ್ಷರ *tmp;

	ಆದರೆ ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	ಗಾಗಿ (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		ಗಾಗಿ (o = optnames; flags && o->o_opt; o++) {
			ಆದರೆ (flags & o->o_opt) {
				ಆದರೆ (o->o_opt == MNT_LOCAL)
					netdev = 1;
				ಆದರೆ (!options)
					options = xstrdup(o->o_name);
				ಇಲ್ಲದಿದ್ದರೆ {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					ಬಿಡುಗಡೆ(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*ಅಂಕೆ_mounts += 0 == ವಾದಗಳು->process(list, ವಾದಗಳು,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		ಬಿಡುಗಡೆ(options);
		options = ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ list;
}

#ಅಥವಾಆದರೆ defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
ಸ್ಥಿರ ರಚನೆ mntent *
getmntfile(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file)
{
	ರಚನೆ mntent *ent = ಖಾಲಿ;
	FILE *fp;

	ಆದರೆ (!(fp = setmntent("/etc/fstab", "r")))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಆಗ ((ent = getmntent(fp)))
		ಆದರೆ (strcmp(file, ent->mnt_dir) == 0)
			ನಿಲ್ಲಿಸು;
	endmntent(fp);

	ಹಿಂತಿರುಗಿ ent;
}

ಸ್ಥಿರ RC_STRINGLIST *
ಕಂಡುಹಿಡಿ_mounts(ರಚನೆ ವಾದಗಳು *ವಾದಗಳು, size_t *ಅಂಕೆ_mounts)
{
	FILE *fp;
	ಅಕ್ಷರ *ಬಫರ್;
	size_t ಗಾತ್ರ_2;
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *from;
	ಅಕ್ಷರ *ಗಮ್ಯ;
	ಅಕ್ಷರ *fst;
	ಅಕ್ಷರ *opts;
	ರಚನೆ mntent *ent;
	ಪೂರ್ಣಾಂಕ netdev;
	RC_STRINGLIST *list;

	ಆದರೆ ((fp = fopen(procmounts, "r")) == ಖಾಲಿ)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	ಬಫರ್ = ಖಾಲಿ;
	ಆಗ (xgetline(&ಬಫರ್, &ಗಾತ್ರ_2, fp) != -1) {
		netdev = -1;
		p = ಬಫರ್;
		from = strsep(&p, " ");
		ಗಮ್ಯ = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		ಆದರೆ ((ent = getmntfile(ಗಮ್ಯ))) {
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(ent->mnt_opts, "_netdev"))
				netdev = 0;
			ಇಲ್ಲದಿದ್ದರೆ
				netdev = 1;
		}

		*ಅಂಕೆ_mounts += 0 == ವಾದಗಳು->process(list, ವಾದಗಳು,
			from, ಗಮ್ಯ, fst, opts, netdev);

		ಬಿಡುಗಡೆ(ಬಫರ್);
		ಬಫರ್ = ಖಾಲಿ;
	}
	ಬಿಡುಗಡೆ(ಬಫರ್);
	fclose(fp);

	ಹಿಂತಿರುಗಿ list;
}

#ಇಲ್ಲದಿದ್ದರೆ
#  error "Operating system not supported!"
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ is_ಪೂರ್ವಪ್ರತ್ಯಯ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *needle, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hay)
{
	size_t nlen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(needle);
	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(needle, hay, nlen) == 0 && hay[nlen] == '/')
		ಹಿಂತಿರುಗಿ true;
	ಹಿಂತಿರುಗಿ false;
}

ಸ್ಥಿರ ಅಕ್ಷರ *unescape_octal(ಅಕ್ಷರ *beg)
{
	ಪೂರ್ಣಾಂಕ n, i;
	ಅಕ್ಷರ *w = beg, *r = beg;
	ಆಗ (*r) {
		ಆದರೆ (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			ಗಾಗಿ (i = n = 0; i < 3; ++i) {
				ಆದರೆ (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} ಇಲ್ಲದಿದ್ದರೆ {
					ನಿಲ್ಲಿಸು;
				}
			}
			ಆದರೆ (n)
				*w++ = n;
		}
	}
	*w = '\0';
	ಹಿಂತಿರುಗಿ beg;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_is_mounted(RC_STRINGLIST *list RC_ಬಳಸದ, ರಚನೆ ವಾದಗಳು *ವಾದಗಳು,
    ಅಕ್ಷರ *from RC_ಬಳಸದ, ಅಕ್ಷರ *ಗಮ್ಯ, ಅಕ್ಷರ *fstype RC_ಬಳಸದ,
    ಅಕ್ಷರ *options RC_ಬಳಸದ, ಪೂರ್ಣಾಂಕ netdev RC_ಬಳಸದ)
{
	ಹಿಂತಿರುಗಿ strcmp(ವಾದಗಳು->check_mntpath, unescape_octal(ಗಮ್ಯ)) == 0 ? 0 : -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ is_mounted(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mntpath)
{
	size_t ಅಂಕೆ_mounts = 0;
	ರಚನೆ ವಾದಗಳು ವಾದಗಳು = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = ಕಂಡುಹಿಡಿ_mounts(&ವಾದಗಳು, &ಅಂಕೆ_mounts);
	rc_stringlist_free(l);
	ಹಿಂತಿರುಗಿ ಅಂಕೆ_mounts > 0;
}

ಸ್ಥಿರ pid_t ಸತತ_umount(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mntpath,
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **umount_ವಾದಗಳು, ಪೂರ್ಣಾಂಕ umount_ವಾದಗಳು_ಅಂಕೆ)
{
	ರಚನೆ exec_ವಾದಗಳು ವಾದಗಳು;
	ರಚನೆ exec_ಫಲಿತಾಂಶ res;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argv[UMOUNT_ವಾದಗಳು_MAX + 3];
	ಪೂರ್ಣಾಂಕ k, i = 0;

	argv[i++] = "umount";
	ಗಾಗಿ (k = 0; k < umount_ವಾದಗಳು_ಅಂಕೆ; ++k)
		argv[i++] = umount_ವಾದಗಳು[k];
	argv[i++] = mntpath;
	argv[i++] = ಖಾಲಿ;

	ವಾದಗಳು = exec_init(argv);
	ವಾದಗಳು.redirect_stdout = ವಾದಗಳು.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&ವಾದಗಳು);
	ಆದರೆ (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	ಹಿಂತಿರುಗಿ res.pid;
}

ಸ್ಥಿರ ಶೂನ್ಯ fuser_ಸತತ(ರಚನೆ ಸತತ_ಸರತಿ *rp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fuser_opt)
{
	ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fuser_exec_failed = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argv[] = { "fuser", fuser_opt, rp->mntpath, ಖಾಲಿ };
	ರಚನೆ exec_ಫಲಿತಾಂಶ res;
	ರಚನೆ exec_ವಾದಗಳು ವಾದಗಳು;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	ಆದರೆ (fuser_exec_failed)
		ಹಿಂತಿರುಗಿ;

	ವಾದಗಳು = exec_init(argv);
	ವಾದಗಳು.redirect_stdout = EXEC_MKPIPE;
	ವಾದಗಳು.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&ವಾದಗಳು);
	ಆದರೆ (res.pid < 0) {
		fuser_exec_failed = 1;
	} ಇಲ್ಲದಿದ್ದರೆ {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_ಸಮಯ = tm_now();
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fuser_decide(ರಚನೆ ಸತತ_ಸರತಿ *rp,
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fuser_opt, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fuser_kill_ಪೂರ್ವಪ್ರತ್ಯಯ)
{
	ಸ್ಥಿರ bool has_rc_pid = false;
	ಸ್ಥಿರ ಅಕ್ಷರ *rc_pid;
	ಅಕ್ಷರ buf[1<<12];
	ಅಕ್ಷರ selfpid[64];
	ಪೂರ್ಣಾಂಕ read_maybe_truncated;
	ssize_t n;

	ಆದರೆ (rp->fuser_stdoutfd < 0)
		ಹಿಂತಿರುಗಿ 0;

	buf[0] = ' ';
	n = ಪಠ(rp->fuser_stdoutfd, buf + 1, ಗಾತ್ರ buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == ಗಾತ್ರ buf - 3);
	ಆದರೆ (n < 0 || read_maybe_truncated)
		ಹಿಂತಿರುಗಿ 0;
	ಆಗ (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, ಗಾತ್ರ selfpid, " %lld ", (ದೀರ್ಘ ದೀರ್ಘ)getpid());

	ಆದರೆ (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(buf, selfpid) || (rc_pid && ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		ಹಿಂತಿರುಗಿ -1;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(buf, "  ") == 0) {
		ಆದರೆ (rp->try_ಎಣಿಕೆ >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			ಹಿಂತಿರುಗಿ -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_ಎಣಿಕೆ = TRY_MAX;
		ಹಿಂತಿರುಗಿ 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಅಕ್ಷರ sig[32];
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, ಖಾಲಿ
		};
		ರಚನೆ exec_ಫಲಿತಾಂಶ res;
		ರಚನೆ exec_ವಾದಗಳು ವಾದಗಳು = exec_init(argv);
		ವಾದಗಳು.redirect_stdout = ವಾದಗಳು.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, ಗಾತ್ರ sig, "%s%s", fuser_kill_ಪೂರ್ವಪ್ರತ್ಯಯ,
			rp->try_ಎಣಿಕೆ == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&ವಾದಗಳು);
		ಆದರೆ (res.pid > 0)
			rc_waitpid(res.pid);
		ಹಿಂತಿರುಗಿ 0;
	}
}

ಸ್ಥಿರ regex_t *
get_regex(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string)
{
	regex_t *reg = xmalloc(ಗಾತ್ರ (*reg));
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;
	ಅಕ್ಷರ ಬಫರ್[256];

	ಆದರೆ ((ಫಲಿತಾಂಶ = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(ಫಲಿತಾಂಶ, reg, ಬಫರ್, ಗಾತ್ರ(ಬಫರ್));
		eerrorx("%s: invalid regex `%s'", applet, ಬಫರ್);
	}

	ಹಿಂತಿರುಗಿ reg;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ರಚನೆ ವಾದಗಳು ವಾದಗಳು;
	regex_t *ಬಿಂದು_regex = ಖಾಲಿ;
	regex_t *skip_ಬಿಂದು_regex = ಖಾಲಿ;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	ಅಕ್ಷರ *real_ಮಾರ್ಗ = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ opt;
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;
	ಅಕ್ಷರ *this_ಮಾರ್ಗ, *argv0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tmps;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fuser_opt, *fuser_kill_ಪೂರ್ವಪ್ರತ್ಯಯ;
	size_t ಅಂಕೆ_mounts = 0;
	size_t unmount_ಸೂಚ್ಯಂಕ;
	ಪೂರ್ಣಾಂಕ doing_unmount = 0;
	pid_t pid;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, flags;
	int64_t tmp, ಮುಂದಿನ_retry, now;
	int64_t rc_fuser_timeout = -1;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *umount_ವಾದಗಳು[UMOUNT_ವಾದಗಳು_MAX];
	ಪೂರ್ಣಾಂಕ umount_ವಾದಗಳು_ಅಂಕೆ = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **mounts = ಖಾಲಿ;
	ರಚನೆ ಸತತ_ಸರತಿ running[ಸತತ_MAX] = {0};
	ರಚನೆ ಸತತ_ಸರತಿ *rp;
	size_t ಅಂಕೆ_running = 0, ಅಂಕೆ_ಕಾಯುವಿಕೆ = 0;
	ಎಣಿಕೆ { STATE_ಸತತ, STATE_REAP, STATE_RETRY, STATE_ಕೊನೆ } state;

#ವ್ಯಾಖ್ಯಾನಿಸು DO_REG(_var)							      \
	ಆದರೆ (_var) ಬಿಡುಗಡೆ(_var);						      \
	_var = get_regex(optarg);
#ವ್ಯಾಖ್ಯಾನಿಸು REG_FREE(_var)							      \
	ಆದರೆ (_var) { regfree(_var); ಬಿಡುಗಡೆ(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&ವಾದಗಳು, 0, ಗಾತ್ರ(ವಾದಗಳು));
	ವಾದಗಳು.mount_type = mount_ಗಮ್ಯ;
	ವಾದಗಳು.netdev = net_ignore;
	ವಾದಗಳು.mounts = rc_stringlist_new();
	ವಾದಗಳು.process = process_mount;

	ಆದರೆ (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		ಆಗ (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			ಆದರೆ (strcmp(tmps, "--") == 0)
				ನಿಲ್ಲಿಸು;
			ಆದರೆ (umount_ವಾದಗಳು_ಅಂಕೆ >= (ಪೂರ್ಣಾಂಕ)ARRAY_ಗಾತ್ರ(umount_ವಾದಗಳು))
				eerrorx("%s: Too many umount arguments", applet);
			umount_ವಾದಗಳು[umount_ವಾದಗಳು_ಅಂಕೆ++] = tmps;
		}

		tmps = rc_conf_ಮೌಲ್ಯ("rc_fuser_timeout");
		ಆದರೆ (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		ಆದರೆ (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		ಆದರೆ (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_ಪೂರ್ವಪ್ರತ್ಯಯ = "-";
		} ಇಲ್ಲದಿದ್ದರೆ {
			fuser_opt = "-cm";
			fuser_kill_ಪೂರ್ವಪ್ರತ್ಯಯ = "-s";
		}
	}

	ಆಗ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಂಕ *) 0)) != -1)
	{
		ಆಯ್ಕೆ (opt) {
		ಸ್ಥಿತಿ 'e':
			ವಾದಗಳು.netdev = net_yes;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'E':
			ವಾದಗಳು.netdev = net_no;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'f':
			DO_REG(ವಾದಗಳು.fstype_regex);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'F':
			DO_REG(ವಾದಗಳು.skip_fstype_regex);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'n':
			DO_REG(ವಾದಗಳು.ಗಂಟು_regex);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'N':
			DO_REG(ವಾದಗಳು.skip_ಗಂಟು_regex);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'o':
			DO_REG(ವಾದಗಳು.options_regex);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'O':
			DO_REG(ವಾದಗಳು.skip_options_regex);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'p':
			DO_REG(ಬಿಂದು_regex);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'P':
			DO_REG(skip_ಬಿಂದು_regex);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'i':
			ವಾದಗಳು.mount_type = mount_options;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 's':
			ವಾದಗಳು.mount_type = mount_fstype;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 't':
			ವಾದಗಳು.mount_type = mount_from;
			ನಿಲ್ಲಿಸು;

		case_RC_COMMON_GETOPT
		}
	}

	ಆಗ (optind < argc) {
		ಆದರೆ (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_ಮಾರ್ಗ = argv[optind++];
		real_ಮಾರ್ಗ = realpath(this_ಮಾರ್ಗ, ಖಾಲಿ);
		ಆದರೆ (real_ಮಾರ್ಗ)
			this_ಮಾರ್ಗ = real_ಮಾರ್ಗ;
		rc_stringlist_ಕೂಡಿಸು(ವಾದಗಳು.mounts, this_ಮಾರ್ಗ);
		ಬಿಡುಗಡೆ(real_ಮಾರ್ಗ);
		real_ಮಾರ್ಗ = ಖಾಲಿ;
	}
	nodes = ಕಂಡುಹಿಡಿ_mounts(&ವಾದಗಳು, &ಅಂಕೆ_mounts);
	rc_stringlist_free(ವಾದಗಳು.mounts);

	ಆದರೆ (doing_unmount)
		mounts = xmalloc(ಅಂಕೆ_mounts * ಗಾತ್ರ(*mounts));
	ಅಂಕೆ_mounts = 0;
	ಫಲಿತಾಂಶ = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_ಹಿಮ್ಮುಖ(s, nodes, rc_stringlist, entries) {
		ಆದರೆ (ಬಿಂದು_regex &&
		    regexec(ಬಿಂದು_regex, s->ಮೌಲ್ಯ, 0, ಖಾಲಿ, 0) != 0)
			ಮುಂದುವರಿ;
		ಆದರೆ (skip_ಬಿಂದು_regex &&
		    regexec(skip_ಬಿಂದು_regex, s->ಮೌಲ್ಯ, 0, ಖಾಲಿ, 0) == 0)
			ಮುಂದುವರಿ;
		ಆದರೆ (doing_unmount)
			mounts[ಅಂಕೆ_mounts++] = unescape_octal(s->ಮೌಲ್ಯ);
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!rc_yesno(getenv("EINFO_QUIET")))
			ಬರೆ("%s\n", s->ಮೌಲ್ಯ);
		ಫಲಿತಾಂಶ = EXIT_SUCCESS;
	}
	ಆದರೆ (!doing_unmount)
		ಹೋಗು ನಿರ್ಗಮಿಸು;

	/* STATE_RUN:
	 * can unmount => stays in STATE_RUN
	 * cannot unmount (for any of the reasons below) => STATE_REAP
	 *   (a) nothing left to unmount
	 *   (b) running queue is full
	 *   (c) conflicts with running queue
	 *
	 * STATE_REAP:
	 * successful reap => STATE_RUN
	 * couldn't reap with WNOHANG and there are retries pending => STATE_RETRY
	 * nothing left to reap => STATE_RETRY
	 *
	 * STATE_RETRY:
	 * successfully launched a retry => STATE_REAP
	 * need to wait before retring -> sleep
	 *    sleep successful => STATE_RETRY
	 *    sleep interrupted via SIGCHILD (EINTR) => STATE_REAP
	 * nothing left to retry, reap
	 *    and nothing to run either => STATE_END
	 *    otherwise => STATE_RUN
	 */
	ಫಲಿತಾಂಶ = EXIT_SUCCESS;
	state = STATE_ಸತತ;
	ಆಗ (state != STATE_ಕೊನೆ) ಆಯ್ಕೆ (state) {
	ಸ್ಥಿತಿ STATE_ಸತತ:
		ಗಾಗಿ (unmount_ಸೂಚ್ಯಂಕ = 0; unmount_ಸೂಚ್ಯಂಕ < ಅಂಕೆ_mounts; ++unmount_ಸೂಚ್ಯಂಕ) {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *candidate = mounts[unmount_ಸೂಚ್ಯಂಕ];
			ಪೂರ್ಣಾಂಕ ಸುರಕ್ಷಿತ_ಗಮ್ಯ_unmount = 1;
			ಗಾಗಿ (size_t k = 0; ಸುರಕ್ಷಿತ_ಗಮ್ಯ_unmount && k < ಅಂಕೆ_running; ++k)
				ಸುರಕ್ಷಿತ_ಗಮ್ಯ_unmount = !is_ಪೂರ್ವಪ್ರತ್ಯಯ(candidate, running[k].mntpath);
			ಗಾಗಿ (size_t k = 0; ಸುರಕ್ಷಿತ_ಗಮ್ಯ_unmount && k < ಅಂಕೆ_mounts; ++k)
				ಸುರಕ್ಷಿತ_ಗಮ್ಯ_unmount = !is_ಪೂರ್ವಪ್ರತ್ಯಯ(candidate, mounts[k]);
			ಆದರೆ (!ಸುರಕ್ಷಿತ_ಗಮ್ಯ_unmount)
				ಮುಂದುವರಿ;
			ಆದರೆ (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_ಸೂಚ್ಯಂಕ--] = mounts[--ಅಂಕೆ_mounts];
			} ಇಲ್ಲದಿದ್ದರೆ {
				ನಿಲ್ಲಿಸು;
			}
		}
		ಆದರೆ (ಅಂಕೆ_running == ಸತತ_MAX || unmount_ಸೂಚ್ಯಂಕ >= ಅಂಕೆ_mounts) {
			state = STATE_REAP;
			ನಿಲ್ಲಿಸು;
		}
		rp = running + ಅಂಕೆ_running++;
		rp->mntpath = mounts[unmount_ಸೂಚ್ಯಂಕ];
		rp->ಕೊನೆಯ_exec_ಸಮಯ = tm_now();
		rp->try_ಎಣಿಕೆ = 0;
		rp->pid = ಸತತ_umount(rp->mntpath, umount_ವಾದಗಳು, umount_ವಾದಗಳು_ಅಂಕೆ);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_ಸಮಯ = -1;
		mounts[unmount_ಸೂಚ್ಯಂಕ] = mounts[--ಅಂಕೆ_mounts];
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ STATE_REAP:
		flags = (ಅಂಕೆ_ಕಾಯುವಿಕೆ > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &ಸ್ಥಿತಿ_2, flags);
		rp = ಖಾಲಿ;
		ಗಾಗಿ (size_t i = 0; i < ಅಂಕೆ_running; ++i) {
			rp = running + i;
			ಆದರೆ (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			ಆದರೆ (rp->pid == pid && pid > 0)
				ನಿಲ್ಲಿಸು;
			rp = ಖಾಲಿ;
		}
		ಆದರೆ (rp) {
			ಆದರೆ ((WIFEXITED(ಸ್ಥಿತಿ_2) && WEXITSTATUS(ಸ್ಥಿತಿ_2) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--ಅಂಕೆ_running];
				state = STATE_ಸತತ;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rp->try_ಎಣಿಕೆ >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--ಅಂಕೆ_running];
				ಫಲಿತಾಂಶ = EXIT_FAILURE;
			} ಇಲ್ಲದಿದ್ದರೆ { /* put into waiting queue */
				rp->pid = -1;
				rp->try_ಎಣಿಕೆ += 1;
				ಅಂಕೆ_ಕಾಯುವಿಕೆ += 1;
				fuser_ಸತತ(rp, fuser_opt);
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			state = STATE_RETRY;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ STATE_RETRY:
		rp = ಖಾಲಿ;
		ಮುಂದಿನ_retry = INT64_MAX;
		ಗಾಗಿ (size_t i = 0; i < ಅಂಕೆ_running; ++i) {
			ಆದರೆ (running[i].pid > 0)
				ಮುಂದುವರಿ;
			ಆದರೆ (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_ಸಮಯ + rc_fuser_timeout;
			ಇಲ್ಲದಿದ್ದರೆ
				tmp = running[i].ಕೊನೆಯ_exec_ಸಮಯ + TRY_DELAY_MS;
			ಆದರೆ (tmp < ಮುಂದಿನ_retry) {
				rp = running + i;
				ಮುಂದಿನ_retry = tmp;
			}
		}
		ಆದರೆ (!rp) {
			state = (ಅಂಕೆ_mounts > 0) ? STATE_ಸತತ : STATE_ಕೊನೆ;
			ನಿಲ್ಲಿಸು;
		}
		now = tm_now();
		ಆದರೆ ((rp->pid > 0 || rp->fuser_pid > 0) && ಮುಂದಿನ_retry > now) {
			int64_t sleep_for = ಮುಂದಿನ_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			ಆದರೆ (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		ಆದರೆ ((rp->pid == -1 && rp->fuser_pid == -1) || ಮುಂದಿನ_retry <= now) {
			ಆದರೆ (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, ಖಾಲಿ, 0);
				rp->fuser_pid = -1;
			}
			ಆದರೆ (fuser_decide(rp, fuser_opt, fuser_kill_ಪೂರ್ವಪ್ರತ್ಯಯ) < 0) { /* abort */
				*rp = running[--ಅಂಕೆ_running];
				ಫಲಿತಾಂಶ = EXIT_FAILURE;
			} ಇಲ್ಲದಿದ್ದರೆ { /* retry */
				rp->ಕೊನೆಯ_exec_ಸಮಯ = tm_now();
				rp->pid = ಸತತ_umount(rp->mntpath,
					umount_ವಾದಗಳು, umount_ವಾದಗಳು_ಅಂಕೆ);
			}
			ಅಂಕೆ_ಕಾಯುವಿಕೆ -= 1;
			state = STATE_REAP;
		}
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ: ನಿಲ್ಲಿಸು;
	}

ನಿರ್ಗಮಿಸು:
	ಬಿಡುಗಡೆ(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(ವಾದಗಳು.fstype_regex);
	REG_FREE(ವಾದಗಳು.skip_fstype_regex);
	REG_FREE(ವಾದಗಳು.ಗಂಟು_regex);
	REG_FREE(ವಾದಗಳು.skip_ಗಂಟು_regex);
	REG_FREE(ವಾದಗಳು.options_regex);
	REG_FREE(ವಾದಗಳು.skip_options_regex);
	REG_FREE(ಬಿಂದು_regex);
	REG_FREE(skip_ಬಿಂದು_regex);

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}
