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

#ਜੇ defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#ਜਾਂਜੇ defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#ਜਾਂਜੇ defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#ਜਾਂਜੇ defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <regex.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc_exec.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "timeutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ *procmounts = "/proc/mounts";
ਅਚਲ ਅੱਖਰ *extraopts = "[mount1] [mount2] ...";
ਅਚਲ ਅੱਖਰ getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "fstype-regex",        1, ਨਿਰਾ, 'f'},
	{ "skip-fstype-regex",   1, ਨਿਰਾ, 'F'},
	{ "node-regex",          1, ਨਿਰਾ, 'n'},
	{ "skip-node-regex",     1, ਨਿਰਾ, 'N'},
	{ "options-regex",       1, ਨਿਰਾ, 'o'},
	{ "skip-options-regex",  1, ਨਿਰਾ, 'O'},
	{ "point-regex",         1, ਨਿਰਾ, 'p'},
	{ "skip-point-regex",    1, ਨਿਰਾ, 'P'},
	{ "options",             0, ਨਿਰਾ, 'i'},
	{ "fstype",              0, ਨਿਰਾ, 's'},
	{ "node",                0, ਨਿਰਾ, 't'},
	{ "netdev",              0, ਨਿਰਾ, 'e'},
	{ "nonetdev",            0, ਨਿਰਾ, 'E'},
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
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
ਅਚਲ ਅੱਖਰ *usagestring = ਨਿਰਾ;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UMOUNT_ਦਲੀਲਾਂ_MAX 16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਲੜੀਵਾਰ_MAX 32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRY_MAX 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRY_DELAY_MS 1000

ਬਣਤਰ ਲੜੀਵਾਰ_ਕਤਾਰ {
	ਅਚਲ ਅੱਖਰ *mntpath;
	int64_t ਆਖਰੀ_exec_ਸਮਾਂ;
	int64_t fuser_exec_ਸਮਾਂ;
	pid_t pid;
	pid_t fuser_pid;
	ਪੂਰਨ try_ਗਿਣਤੀ;
	ਪੂਰਨ fuser_stdoutfd;
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	mount_from,
	mount_ਮੰਜ਼ਿਲ,
	mount_fstype,
	mount_options
} mount_type;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	net_ignore,
	net_yes,
	net_no
} net_opts;

ਬਣਤਰ ਦਲੀਲਾਂ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ process_func_t(RC_STRINGLIST *, ਬਣਤਰ ਦਲੀਲਾਂ *,
    ਅੱਖਰ *, ਅੱਖਰ *, ਅੱਖਰ *, ਅੱਖਰ *, ਪੂਰਨ);

ਬਣਤਰ ਦਲੀਲਾਂ {
	process_func_t *process;
	regex_t *ਗੰਢ_regex;
	regex_t *skip_ਗੰਢ_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	ਅਚਲ ਅੱਖਰ *check_mntpath;
};

ਸਥਿਰ ਪੂਰਨ
process_mount(RC_STRINGLIST *list, ਬਣਤਰ ਦਲੀਲਾਂ *ਦਲੀਲਾਂ,
    ਅੱਖਰ *from, ਅੱਖਰ *ਮੰਜ਼ਿਲ, ਅੱਖਰ *fstype, ਅੱਖਰ *options,
    ਪੂਰਨ netdev)
{
	ਅੱਖਰ *p;
	RC_STRING *s;

	errno = ENOENT;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	/* Skip the really silly rootfs */
	ਜੇ (strcmp(fstype, "rootfs") == 0)
		ਵਾਪਸ -1;
#ਜੇ_ਅੰਤ

	ਜੇ (ਦਲੀਲਾਂ->netdev == net_yes &&
	    (netdev != -1 || TAILQ_ਪਹਿਲਾ(ਦਲੀਲਾਂ->mounts)))
	{
		ਜੇ (netdev != 0)
			ਵਾਪਸ 1;
	} ਨਹੀਂਤਾਂ ਜੇ (ਦਲੀਲਾਂ->netdev == net_no &&
	    (netdev != -1 || TAILQ_ਪਹਿਲਾ(ਦਲੀਲਾਂ->mounts)))
	{
		ਜੇ (netdev != 1)
			ਵਾਪਸ 1;
	} ਨਹੀਂਤਾਂ {
		ਜੇ (ਦਲੀਲਾਂ->ਗੰਢ_regex &&
		    regexec(ਦਲੀਲਾਂ->ਗੰਢ_regex, from, 0, ਨਿਰਾ, 0) != 0)
			ਵਾਪਸ 1;
		ਜੇ (ਦਲੀਲਾਂ->skip_ਗੰਢ_regex &&
		    regexec(ਦਲੀਲਾਂ->skip_ਗੰਢ_regex, from, 0, ਨਿਰਾ, 0) == 0)
			ਵਾਪਸ 1;

		ਜੇ (ਦਲੀਲਾਂ->fstype_regex &&
		    regexec(ਦਲੀਲਾਂ->fstype_regex, fstype, 0, ਨਿਰਾ, 0) != 0)
			ਵਾਪਸ -1;
		ਜੇ (ਦਲੀਲਾਂ->skip_fstype_regex &&
		    regexec(ਦਲੀਲਾਂ->skip_fstype_regex, fstype, 0, ਨਿਰਾ, 0) == 0)
			ਵਾਪਸ -1;

		ਜੇ (ਦਲੀਲਾਂ->options_regex &&
		    regexec(ਦਲੀਲਾਂ->options_regex, options, 0, ਨਿਰਾ, 0) != 0)
			ਵਾਪਸ -1;
		ਜੇ (ਦਲੀਲਾਂ->skip_options_regex &&
		    regexec(ਦਲੀਲਾਂ->skip_options_regex, options, 0, ਨਿਰਾ, 0) == 0)
			ਵਾਪਸ -1;
	}

	ਜੇ (TAILQ_ਪਹਿਲਾ(ਦਲੀਲਾਂ->mounts)) {
		TAILQ_FOREACH(s, ਦਲੀਲਾਂ->mounts, entries)
		    ਜੇ (strcmp(s->ਮੁੱਲ, ਮੰਜ਼ਿਲ) == 0)
			    ਰੋਕੋ;
		ਜੇ (!s)
			ਵਾਪਸ -1;
	}

	ਚੋਣ (ਦਲੀਲਾਂ->mount_type) {
	ਹਾਲਤ mount_from:
		p = from;
		ਰੋਕੋ;
	ਹਾਲਤ mount_ਮੰਜ਼ਿਲ:
		p = ਮੰਜ਼ਿਲ;
		ਰੋਕੋ;
	ਹਾਲਤ mount_fstype:
		p = fstype;
		ਰੋਕੋ;
	ਹਾਲਤ mount_options:
		p = options;
		ਰੋਕੋ;
	ਮੂਲ:
		p = ਨਿਰਾ;
		errno = EINVAL;
		ਰੋਕੋ;
	}

	ਜੇ (p) {
		errno = 0;
		rc_stringlist_ਜੋੜੋ(list, p);
		ਵਾਪਸ 0;
	}

	ਵਾਪਸ -1;
}

#ਜੇ defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
ਸਥਿਰ ਬਣਤਰ opt {
	ਪੂਰਨ o_opt;
	ਅਚਲ ਅੱਖਰ *o_name;
} optnames[] = {
	{ MNT_ASYNC,        "asynchronous" },
	{ MNT_EXPORTED,     "NFS exported" },
	{ MNT_LOCAL,        "local" },
	{ MNT_NOATIME,      "noatime" },
	{ MNT_NOEXEC,       "noexec" },
	{ MNT_NOSUID,       "nosuid" },
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MNT_NOSYMFOLLOW
	{ MNT_NOSYMFOLLOW,  "nosymfollow" },
#ਜੇ_ਅੰਤ
	{ MNT_QUOTA,        "with quotas" },
	{ MNT_RDONLY,       "read-only" },
	{ MNT_SYNCHRONOUS,  "synchronous" },
	{ MNT_UNION,        "union" },
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MNT_NOCLUSTERR
	{ MNT_NOCLUSTERR,   "noclusterr" },
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MNT_NOCLUSTERW
	{ MNT_NOCLUSTERW,   "noclusterw" },
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MNT_SUIDDIR
	{ MNT_SUIDDIR,      "suiddir" },
#ਜੇ_ਅੰਤ
	{ MNT_SOFTDEP,      "soft-updates" },
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MNT_MULTILABEL
	{ MNT_MULTILABEL,   "multilabel" },
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MNT_ACLS
	{ MNT_ACLS,         "acls" },
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MNT_GJOURNAL
	{ MNT_GJOURNAL,     "gjournal" },
#ਜੇ_ਅੰਤ
	{ 0, ਨਿਰਾ }
};

ਸਥਿਰ RC_STRINGLIST *
ਲੱਭੋ_mounts(ਬਣਤਰ ਦਲੀਲਾਂ *ਦਲੀਲਾਂ, size_t *ਅੰਕ_mounts)
{
	ਬਣਤਰ statfs *mnts;
	ਪੂਰਨ nmnts;
	ਪੂਰਨ i;
	RC_STRINGLIST *list;
	ਅੱਖਰ *options = ਨਿਰਾ;
	uint64_t flags;
	ਬਣਤਰ opt *o;
	ਪੂਰਨ netdev;
	ਅੱਖਰ *tmp;

	ਜੇ ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	ਲਈ (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		ਲਈ (o = optnames; flags && o->o_opt; o++) {
			ਜੇ (flags & o->o_opt) {
				ਜੇ (o->o_opt == MNT_LOCAL)
					netdev = 1;
				ਜੇ (!options)
					options = xstrdup(o->o_name);
				ਨਹੀਂਤਾਂ {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					ਮੁਕਤ(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*ਅੰਕ_mounts += 0 == ਦਲੀਲਾਂ->process(list, ਦਲੀਲਾਂ,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		ਮੁਕਤ(options);
		options = ਨਿਰਾ;
	}

	ਵਾਪਸ list;
}

#ਜਾਂਜੇ defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
ਸਥਿਰ ਬਣਤਰ mntent *
getmntfile(ਅਚਲ ਅੱਖਰ *file)
{
	ਬਣਤਰ mntent *ent = ਨਿਰਾ;
	FILE *fp;

	ਜੇ (!(fp = setmntent("/etc/fstab", "r")))
		ਵਾਪਸ ਨਿਰਾ;
	ਜਦੋਂ ((ent = getmntent(fp)))
		ਜੇ (strcmp(file, ent->mnt_dir) == 0)
			ਰੋਕੋ;
	endmntent(fp);

	ਵਾਪਸ ent;
}

ਸਥਿਰ RC_STRINGLIST *
ਲੱਭੋ_mounts(ਬਣਤਰ ਦਲੀਲਾਂ *ਦਲੀਲਾਂ, size_t *ਅੰਕ_mounts)
{
	FILE *fp;
	ਅੱਖਰ *ਬਫਰ;
	size_t ਮਾਪ;
	ਅੱਖਰ *p;
	ਅੱਖਰ *from;
	ਅੱਖਰ *ਮੰਜ਼ਿਲ;
	ਅੱਖਰ *fst;
	ਅੱਖਰ *opts;
	ਬਣਤਰ mntent *ent;
	ਪੂਰਨ netdev;
	RC_STRINGLIST *list;

	ਜੇ ((fp = fopen(procmounts, "r")) == ਨਿਰਾ)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	ਬਫਰ = ਨਿਰਾ;
	ਜਦੋਂ (xgetline(&ਬਫਰ, &ਮਾਪ, fp) != -1) {
		netdev = -1;
		p = ਬਫਰ;
		from = strsep(&p, " ");
		ਮੰਜ਼ਿਲ = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		ਜੇ ((ent = getmntfile(ਮੰਜ਼ਿਲ))) {
			ਜੇ (ਲੜੀ_ਖੋਜ(ent->mnt_opts, "_netdev"))
				netdev = 0;
			ਨਹੀਂਤਾਂ
				netdev = 1;
		}

		*ਅੰਕ_mounts += 0 == ਦਲੀਲਾਂ->process(list, ਦਲੀਲਾਂ,
			from, ਮੰਜ਼ਿਲ, fst, opts, netdev);

		ਮੁਕਤ(ਬਫਰ);
		ਬਫਰ = ਨਿਰਾ;
	}
	ਮੁਕਤ(ਬਫਰ);
	fclose(fp);

	ਵਾਪਸ list;
}

#ਨਹੀਂਤਾਂ
#  error "Operating system not supported!"
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ is_ਅਗੇਤਰ(ਅਚਲ ਅੱਖਰ *needle, ਅਚਲ ਅੱਖਰ *hay)
{
	size_t nlen = ਲੜੀ_ਲੰਬਾਈ(needle);
	ਜੇ (ਲੜੀ_ਤੁਲਨਾ(needle, hay, nlen) == 0 && hay[nlen] == '/')
		ਵਾਪਸ true;
	ਵਾਪਸ false;
}

ਸਥਿਰ ਅੱਖਰ *unescape_octal(ਅੱਖਰ *beg)
{
	ਪੂਰਨ n, i;
	ਅੱਖਰ *w = beg, *r = beg;
	ਜਦੋਂ (*r) {
		ਜੇ (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} ਨਹੀਂਤਾਂ {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			ਲਈ (i = n = 0; i < 3; ++i) {
				ਜੇ (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} ਨਹੀਂਤਾਂ {
					ਰੋਕੋ;
				}
			}
			ਜੇ (n)
				*w++ = n;
		}
	}
	*w = '\0';
	ਵਾਪਸ beg;
}

ਸਥਿਰ ਪੂਰਨ check_is_mounted(RC_STRINGLIST *list RC_ਅਣਵਰਤਿਆ, ਬਣਤਰ ਦਲੀਲਾਂ *ਦਲੀਲਾਂ,
    ਅੱਖਰ *from RC_ਅਣਵਰਤਿਆ, ਅੱਖਰ *ਮੰਜ਼ਿਲ, ਅੱਖਰ *fstype RC_ਅਣਵਰਤਿਆ,
    ਅੱਖਰ *options RC_ਅਣਵਰਤਿਆ, ਪੂਰਨ netdev RC_ਅਣਵਰਤਿਆ)
{
	ਵਾਪਸ strcmp(ਦਲੀਲਾਂ->check_mntpath, unescape_octal(ਮੰਜ਼ਿਲ)) == 0 ? 0 : -1;
}

ਸਥਿਰ ਪੂਰਨ is_mounted(ਅਚਲ ਅੱਖਰ *mntpath)
{
	size_t ਅੰਕ_mounts = 0;
	ਬਣਤਰ ਦਲੀਲਾਂ ਦਲੀਲਾਂ = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = ਲੱਭੋ_mounts(&ਦਲੀਲਾਂ, &ਅੰਕ_mounts);
	rc_stringlist_free(l);
	ਵਾਪਸ ਅੰਕ_mounts > 0;
}

ਸਥਿਰ pid_t ਲੜੀਵਾਰ_umount(ਅਚਲ ਅੱਖਰ *mntpath,
	ਅਚਲ ਅੱਖਰ **umount_ਦਲੀਲਾਂ, ਪੂਰਨ umount_ਦਲੀਲਾਂ_ਅੰਕ)
{
	ਬਣਤਰ exec_ਦਲੀਲਾਂ ਦਲੀਲਾਂ;
	ਬਣਤਰ exec_ਨਤੀਜਾ res;
	ਅਚਲ ਅੱਖਰ *argv[UMOUNT_ਦਲੀਲਾਂ_MAX + 3];
	ਪੂਰਨ k, i = 0;

	argv[i++] = "umount";
	ਲਈ (k = 0; k < umount_ਦਲੀਲਾਂ_ਅੰਕ; ++k)
		argv[i++] = umount_ਦਲੀਲਾਂ[k];
	argv[i++] = mntpath;
	argv[i++] = ਨਿਰਾ;

	ਦਲੀਲਾਂ = exec_init(argv);
	ਦਲੀਲਾਂ.redirect_stdout = ਦਲੀਲਾਂ.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&ਦਲੀਲਾਂ);
	ਜੇ (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	ਵਾਪਸ res.pid;
}

ਸਥਿਰ ਖਾਲੀ fuser_ਲੜੀਵਾਰ(ਬਣਤਰ ਲੜੀਵਾਰ_ਕਤਾਰ *rp, ਅਚਲ ਅੱਖਰ *fuser_opt)
{
	ਸਥਿਰ ਪੂਰਨ fuser_exec_failed = 0;
	ਅਚਲ ਅੱਖਰ *argv[] = { "fuser", fuser_opt, rp->mntpath, ਨਿਰਾ };
	ਬਣਤਰ exec_ਨਤੀਜਾ res;
	ਬਣਤਰ exec_ਦਲੀਲਾਂ ਦਲੀਲਾਂ;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	ਜੇ (fuser_exec_failed)
		ਵਾਪਸ;

	ਦਲੀਲਾਂ = exec_init(argv);
	ਦਲੀਲਾਂ.redirect_stdout = EXEC_MKPIPE;
	ਦਲੀਲਾਂ.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&ਦਲੀਲਾਂ);
	ਜੇ (res.pid < 0) {
		fuser_exec_failed = 1;
	} ਨਹੀਂਤਾਂ {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_ਸਮਾਂ = tm_now();
	}
}

ਸਥਿਰ ਪੂਰਨ fuser_decide(ਬਣਤਰ ਲੜੀਵਾਰ_ਕਤਾਰ *rp,
	ਅਚਲ ਅੱਖਰ *fuser_opt, ਅਚਲ ਅੱਖਰ *fuser_kill_ਅਗੇਤਰ)
{
	ਸਥਿਰ bool has_rc_pid = false;
	ਸਥਿਰ ਅੱਖਰ *rc_pid;
	ਅੱਖਰ buf[1<<12];
	ਅੱਖਰ selfpid[64];
	ਪੂਰਨ read_maybe_truncated;
	ssize_t n;

	ਜੇ (rp->fuser_stdoutfd < 0)
		ਵਾਪਸ 0;

	buf[0] = ' ';
	n = ਪਠ(rp->fuser_stdoutfd, buf + 1, ਆਕਾਰ buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == ਆਕਾਰ buf - 3);
	ਜੇ (n < 0 || read_maybe_truncated)
		ਵਾਪਸ 0;
	ਜਦੋਂ (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, ਆਕਾਰ selfpid, " %lld ", (ਲੰਮਾ ਲੰਮਾ)getpid());

	ਜੇ (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	ਜੇ (ਲੜੀ_ਖੋਜ(buf, selfpid) || (rc_pid && ਲੜੀ_ਖੋਜ(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		ਵਾਪਸ -1;
	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(buf, "  ") == 0) {
		ਜੇ (rp->try_ਗਿਣਤੀ >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			ਵਾਪਸ -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_ਗਿਣਤੀ = TRY_MAX;
		ਵਾਪਸ 0;
	} ਨਹੀਂਤਾਂ {
		ਅੱਖਰ sig[32];
		ਅਚਲ ਅੱਖਰ *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, ਨਿਰਾ
		};
		ਬਣਤਰ exec_ਨਤੀਜਾ res;
		ਬਣਤਰ exec_ਦਲੀਲਾਂ ਦਲੀਲਾਂ = exec_init(argv);
		ਦਲੀਲਾਂ.redirect_stdout = ਦਲੀਲਾਂ.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, ਆਕਾਰ sig, "%s%s", fuser_kill_ਅਗੇਤਰ,
			rp->try_ਗਿਣਤੀ == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&ਦਲੀਲਾਂ);
		ਜੇ (res.pid > 0)
			rc_waitpid(res.pid);
		ਵਾਪਸ 0;
	}
}

ਸਥਿਰ regex_t *
get_regex(ਅਚਲ ਅੱਖਰ *string)
{
	regex_t *reg = xmalloc(ਆਕਾਰ (*reg));
	ਪੂਰਨ ਨਤੀਜਾ;
	ਅੱਖਰ ਬਫਰ[256];

	ਜੇ ((ਨਤੀਜਾ = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(ਨਤੀਜਾ, reg, ਬਫਰ, ਆਕਾਰ(ਬਫਰ));
		eerrorx("%s: invalid regex `%s'", applet, ਬਫਰ);
	}

	ਵਾਪਸ reg;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਬਣਤਰ ਦਲੀਲਾਂ ਦਲੀਲਾਂ;
	regex_t *ਬਿੰਦੂ_regex = ਨਿਰਾ;
	regex_t *skip_ਬਿੰਦੂ_regex = ਨਿਰਾ;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	ਅੱਖਰ *real_ਰਸਤਾ = ਨਿਰਾ;
	ਪੂਰਨ opt;
	ਪੂਰਨ ਨਤੀਜਾ;
	ਅੱਖਰ *this_ਰਸਤਾ, *argv0;
	ਅਚਲ ਅੱਖਰ *tmps;
	ਅਚਲ ਅੱਖਰ *fuser_opt, *fuser_kill_ਅਗੇਤਰ;
	size_t ਅੰਕ_mounts = 0;
	size_t unmount_ਸੂਚਕਾਂਕ;
	ਪੂਰਨ doing_unmount = 0;
	pid_t pid;
	ਪੂਰਨ ਸਥਿਤੀ, flags;
	int64_t tmp, ਅਗਲਾ_retry, now;
	int64_t rc_fuser_timeout = -1;
	ਅਚਲ ਅੱਖਰ *umount_ਦਲੀਲਾਂ[UMOUNT_ਦਲੀਲਾਂ_MAX];
	ਪੂਰਨ umount_ਦਲੀਲਾਂ_ਅੰਕ = 0;
	ਅਚਲ ਅੱਖਰ **mounts = ਨਿਰਾ;
	ਬਣਤਰ ਲੜੀਵਾਰ_ਕਤਾਰ running[ਲੜੀਵਾਰ_MAX] = {0};
	ਬਣਤਰ ਲੜੀਵਾਰ_ਕਤਾਰ *rp;
	size_t ਅੰਕ_running = 0, ਅੰਕ_ਉਡੀਕ = 0;
	ਗਿਣਤੀ { STATE_ਲੜੀਵਾਰ, STATE_REAP, STATE_RETRY, STATE_ਸਿਰਾ } state;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DO_REG(_var)							      \
	ਜੇ (_var) ਮੁਕਤ(_var);						      \
	_var = get_regex(optarg);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REG_FREE(_var)							      \
	ਜੇ (_var) { regfree(_var); ਮੁਕਤ(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&ਦਲੀਲਾਂ, 0, ਆਕਾਰ(ਦਲੀਲਾਂ));
	ਦਲੀਲਾਂ.mount_type = mount_ਮੰਜ਼ਿਲ;
	ਦਲੀਲਾਂ.netdev = net_ignore;
	ਦਲੀਲਾਂ.mounts = rc_stringlist_new();
	ਦਲੀਲਾਂ.process = process_mount;

	ਜੇ (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		ਜਦੋਂ (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			ਜੇ (strcmp(tmps, "--") == 0)
				ਰੋਕੋ;
			ਜੇ (umount_ਦਲੀਲਾਂ_ਅੰਕ >= (ਪੂਰਨ)ARRAY_ਮਾਪ(umount_ਦਲੀਲਾਂ))
				eerrorx("%s: Too many umount arguments", applet);
			umount_ਦਲੀਲਾਂ[umount_ਦਲੀਲਾਂ_ਅੰਕ++] = tmps;
		}

		tmps = rc_conf_ਮੁੱਲ("rc_fuser_timeout");
		ਜੇ (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		ਜੇ (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		ਜੇ (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_ਅਗੇਤਰ = "-";
		} ਨਹੀਂਤਾਂ {
			fuser_opt = "-cm";
			fuser_kill_ਅਗੇਤਰ = "-s";
		}
	}

	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ਪੂਰਨ *) 0)) != -1)
	{
		ਚੋਣ (opt) {
		ਹਾਲਤ 'e':
			ਦਲੀਲਾਂ.netdev = net_yes;
			ਰੋਕੋ;
		ਹਾਲਤ 'E':
			ਦਲੀਲਾਂ.netdev = net_no;
			ਰੋਕੋ;
		ਹਾਲਤ 'f':
			DO_REG(ਦਲੀਲਾਂ.fstype_regex);
			ਰੋਕੋ;
		ਹਾਲਤ 'F':
			DO_REG(ਦਲੀਲਾਂ.skip_fstype_regex);
			ਰੋਕੋ;
		ਹਾਲਤ 'n':
			DO_REG(ਦਲੀਲਾਂ.ਗੰਢ_regex);
			ਰੋਕੋ;
		ਹਾਲਤ 'N':
			DO_REG(ਦਲੀਲਾਂ.skip_ਗੰਢ_regex);
			ਰੋਕੋ;
		ਹਾਲਤ 'o':
			DO_REG(ਦਲੀਲਾਂ.options_regex);
			ਰੋਕੋ;
		ਹਾਲਤ 'O':
			DO_REG(ਦਲੀਲਾਂ.skip_options_regex);
			ਰੋਕੋ;
		ਹਾਲਤ 'p':
			DO_REG(ਬਿੰਦੂ_regex);
			ਰੋਕੋ;
		ਹਾਲਤ 'P':
			DO_REG(skip_ਬਿੰਦੂ_regex);
			ਰੋਕੋ;
		ਹਾਲਤ 'i':
			ਦਲੀਲਾਂ.mount_type = mount_options;
			ਰੋਕੋ;
		ਹਾਲਤ 's':
			ਦਲੀਲਾਂ.mount_type = mount_fstype;
			ਰੋਕੋ;
		ਹਾਲਤ 't':
			ਦਲੀਲਾਂ.mount_type = mount_from;
			ਰੋਕੋ;

		case_RC_COMMON_GETOPT
		}
	}

	ਜਦੋਂ (optind < argc) {
		ਜੇ (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_ਰਸਤਾ = argv[optind++];
		real_ਰਸਤਾ = realpath(this_ਰਸਤਾ, ਨਿਰਾ);
		ਜੇ (real_ਰਸਤਾ)
			this_ਰਸਤਾ = real_ਰਸਤਾ;
		rc_stringlist_ਜੋੜੋ(ਦਲੀਲਾਂ.mounts, this_ਰਸਤਾ);
		ਮੁਕਤ(real_ਰਸਤਾ);
		real_ਰਸਤਾ = ਨਿਰਾ;
	}
	nodes = ਲੱਭੋ_mounts(&ਦਲੀਲਾਂ, &ਅੰਕ_mounts);
	rc_stringlist_free(ਦਲੀਲਾਂ.mounts);

	ਜੇ (doing_unmount)
		mounts = xmalloc(ਅੰਕ_mounts * ਆਕਾਰ(*mounts));
	ਅੰਕ_mounts = 0;
	ਨਤੀਜਾ = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_ਉਲਟ(s, nodes, rc_stringlist, entries) {
		ਜੇ (ਬਿੰਦੂ_regex &&
		    regexec(ਬਿੰਦੂ_regex, s->ਮੁੱਲ, 0, ਨਿਰਾ, 0) != 0)
			ਜਾਰੀ;
		ਜੇ (skip_ਬਿੰਦੂ_regex &&
		    regexec(skip_ਬਿੰਦੂ_regex, s->ਮੁੱਲ, 0, ਨਿਰਾ, 0) == 0)
			ਜਾਰੀ;
		ਜੇ (doing_unmount)
			mounts[ਅੰਕ_mounts++] = unescape_octal(s->ਮੁੱਲ);
		ਨਹੀਂਤਾਂ ਜੇ (!rc_yesno(getenv("EINFO_QUIET")))
			ਲਿਖੋ("%s\n", s->ਮੁੱਲ);
		ਨਤੀਜਾ = EXIT_SUCCESS;
	}
	ਜੇ (!doing_unmount)
		ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;

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
	ਨਤੀਜਾ = EXIT_SUCCESS;
	state = STATE_ਲੜੀਵਾਰ;
	ਜਦੋਂ (state != STATE_ਸਿਰਾ) ਚੋਣ (state) {
	ਹਾਲਤ STATE_ਲੜੀਵਾਰ:
		ਲਈ (unmount_ਸੂਚਕਾਂਕ = 0; unmount_ਸੂਚਕਾਂਕ < ਅੰਕ_mounts; ++unmount_ਸੂਚਕਾਂਕ) {
			ਅਚਲ ਅੱਖਰ *candidate = mounts[unmount_ਸੂਚਕਾਂਕ];
			ਪੂਰਨ ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_unmount = 1;
			ਲਈ (size_t k = 0; ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_unmount && k < ਅੰਕ_running; ++k)
				ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_unmount = !is_ਅਗੇਤਰ(candidate, running[k].mntpath);
			ਲਈ (size_t k = 0; ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_unmount && k < ਅੰਕ_mounts; ++k)
				ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_unmount = !is_ਅਗੇਤਰ(candidate, mounts[k]);
			ਜੇ (!ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_unmount)
				ਜਾਰੀ;
			ਜੇ (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_ਸੂਚਕਾਂਕ--] = mounts[--ਅੰਕ_mounts];
			} ਨਹੀਂਤਾਂ {
				ਰੋਕੋ;
			}
		}
		ਜੇ (ਅੰਕ_running == ਲੜੀਵਾਰ_MAX || unmount_ਸੂਚਕਾਂਕ >= ਅੰਕ_mounts) {
			state = STATE_REAP;
			ਰੋਕੋ;
		}
		rp = running + ਅੰਕ_running++;
		rp->mntpath = mounts[unmount_ਸੂਚਕਾਂਕ];
		rp->ਆਖਰੀ_exec_ਸਮਾਂ = tm_now();
		rp->try_ਗਿਣਤੀ = 0;
		rp->pid = ਲੜੀਵਾਰ_umount(rp->mntpath, umount_ਦਲੀਲਾਂ, umount_ਦਲੀਲਾਂ_ਅੰਕ);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_ਸਮਾਂ = -1;
		mounts[unmount_ਸੂਚਕਾਂਕ] = mounts[--ਅੰਕ_mounts];
		ਰੋਕੋ;
	ਹਾਲਤ STATE_REAP:
		flags = (ਅੰਕ_ਉਡੀਕ > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &ਸਥਿਤੀ, flags);
		rp = ਨਿਰਾ;
		ਲਈ (size_t i = 0; i < ਅੰਕ_running; ++i) {
			rp = running + i;
			ਜੇ (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			ਜੇ (rp->pid == pid && pid > 0)
				ਰੋਕੋ;
			rp = ਨਿਰਾ;
		}
		ਜੇ (rp) {
			ਜੇ ((WIFEXITED(ਸਥਿਤੀ) && WEXITSTATUS(ਸਥਿਤੀ) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--ਅੰਕ_running];
				state = STATE_ਲੜੀਵਾਰ;
			} ਨਹੀਂਤਾਂ ਜੇ (rp->try_ਗਿਣਤੀ >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--ਅੰਕ_running];
				ਨਤੀਜਾ = EXIT_FAILURE;
			} ਨਹੀਂਤਾਂ { /* put into waiting queue */
				rp->pid = -1;
				rp->try_ਗਿਣਤੀ += 1;
				ਅੰਕ_ਉਡੀਕ += 1;
				fuser_ਲੜੀਵਾਰ(rp, fuser_opt);
			}
		} ਨਹੀਂਤਾਂ {
			state = STATE_RETRY;
		}
		ਰੋਕੋ;
	ਹਾਲਤ STATE_RETRY:
		rp = ਨਿਰਾ;
		ਅਗਲਾ_retry = INT64_MAX;
		ਲਈ (size_t i = 0; i < ਅੰਕ_running; ++i) {
			ਜੇ (running[i].pid > 0)
				ਜਾਰੀ;
			ਜੇ (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_ਸਮਾਂ + rc_fuser_timeout;
			ਨਹੀਂਤਾਂ
				tmp = running[i].ਆਖਰੀ_exec_ਸਮਾਂ + TRY_DELAY_MS;
			ਜੇ (tmp < ਅਗਲਾ_retry) {
				rp = running + i;
				ਅਗਲਾ_retry = tmp;
			}
		}
		ਜੇ (!rp) {
			state = (ਅੰਕ_mounts > 0) ? STATE_ਲੜੀਵਾਰ : STATE_ਸਿਰਾ;
			ਰੋਕੋ;
		}
		now = tm_now();
		ਜੇ ((rp->pid > 0 || rp->fuser_pid > 0) && ਅਗਲਾ_retry > now) {
			int64_t sleep_for = ਅਗਲਾ_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			ਜੇ (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		ਜੇ ((rp->pid == -1 && rp->fuser_pid == -1) || ਅਗਲਾ_retry <= now) {
			ਜੇ (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, ਨਿਰਾ, 0);
				rp->fuser_pid = -1;
			}
			ਜੇ (fuser_decide(rp, fuser_opt, fuser_kill_ਅਗੇਤਰ) < 0) { /* abort */
				*rp = running[--ਅੰਕ_running];
				ਨਤੀਜਾ = EXIT_FAILURE;
			} ਨਹੀਂਤਾਂ { /* retry */
				rp->ਆਖਰੀ_exec_ਸਮਾਂ = tm_now();
				rp->pid = ਲੜੀਵਾਰ_umount(rp->mntpath,
					umount_ਦਲੀਲਾਂ, umount_ਦਲੀਲਾਂ_ਅੰਕ);
			}
			ਅੰਕ_ਉਡੀਕ -= 1;
			state = STATE_REAP;
		}
		ਰੋਕੋ;
	ਮੂਲ: ਰੋਕੋ;
	}

ਬਾਹਰ_ਨਿਕਲੋ:
	ਮੁਕਤ(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(ਦਲੀਲਾਂ.fstype_regex);
	REG_FREE(ਦਲੀਲਾਂ.skip_fstype_regex);
	REG_FREE(ਦਲੀਲਾਂ.ਗੰਢ_regex);
	REG_FREE(ਦਲੀਲਾਂ.skip_ਗੰਢ_regex);
	REG_FREE(ਦਲੀਲਾਂ.options_regex);
	REG_FREE(ਦਲੀਲਾਂ.skip_options_regex);
	REG_FREE(ਬਿੰਦੂ_regex);
	REG_FREE(skip_ਬਿੰਦੂ_regex);

	ਵਾਪਸ ਨਤੀਜਾ;
}
