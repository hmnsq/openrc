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

#ಯದಿ defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#ನೋಚೇದ್ಯದಿ defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#ನೋಚೇದ್ಯದಿ defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#ನೋಚೇದ್ಯದಿ defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#endif

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

ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *procmounts = "/proc/mounts";
ಧ್ರುವ ವರ್ಣ *extraopts = "[mount1] [mount2] ...";
ಧ್ರುವ ವರ್ಣ getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{ "fstype-regex",        1, ರಿಕ್ತ, 'f'},
	{ "skip-fstype-regex",   1, ರಿಕ್ತ, 'F'},
	{ "node-regex",          1, ರಿಕ್ತ, 'n'},
	{ "skip-node-regex",     1, ರಿಕ್ತ, 'N'},
	{ "options-regex",       1, ರಿಕ್ತ, 'o'},
	{ "skip-options-regex",  1, ರಿಕ್ತ, 'O'},
	{ "point-regex",         1, ರಿಕ್ತ, 'p'},
	{ "skip-point-regex",    1, ರಿಕ್ತ, 'P'},
	{ "options",             0, ರಿಕ್ತ, 'i'},
	{ "fstype",              0, ರಿಕ್ತ, 's'},
	{ "node",                0, ರಿಕ್ತ, 't'},
	{ "netdev",              0, ರಿಕ್ತ, 'e'},
	{ "nonetdev",            0, ರಿಕ್ತ, 'E'},
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[] = {
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
ಧ್ರುವ ವರ್ಣ *usagestring = ರಿಕ್ತ;

#define UMOUNT_ARGS_MAX 16
#define RUN_MAX 32
#define TRY_MAX 3
#define TRY_DELAY_MS 1000

ಸಂರಚನಾ run_queue {
	ಧ್ರುವ ವರ್ಣ *mntpath;
	int64_t last_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	ಪೂರ್ಣಾಙ್ಕ try_ಎಣಿಕೆ;
	ಪೂರ್ಣಾಙ್ಕ fuser_stdoutfd;
};

ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ {
	mount_from,
	mount_to,
	mount_fstype,
	mount_options
} mount_type;

ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ {
	net_ignore,
	net_yes,
	net_no
} net_opts;

ಸಂರಚನಾ args;
ಪ್ರಕಾರನಿರ್ದೇಶ ಪೂರ್ಣಾಙ್ಕ process_func_t(RC_STRINGLIST *, ಸಂರಚನಾ args *,
    ವರ್ಣ *, ವರ್ಣ *, ವರ್ಣ *, ವರ್ಣ *, ಪೂರ್ಣಾಙ್ಕ);

ಸಂರಚನಾ args {
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
	ಧ್ರುವ ವರ್ಣ *check_mntpath;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
process_mount(RC_STRINGLIST *list, ಸಂರಚನಾ args *args,
    ವರ್ಣ *from, ವರ್ಣ *to, ವರ್ಣ *fstype, ವರ್ಣ *options,
    ಪೂರ್ಣಾಙ್ಕ netdev)
{
	ವರ್ಣ *p;
	RC_STRING *s;

	errno = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	ಯದಿ (strcmp(fstype, "rootfs") == 0)
		ಪ್ರತ್ಯಾಗಮ -1;
#endif

	ಯದಿ (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		ಯದಿ (netdev != 0)
			ಪ್ರತ್ಯಾಗಮ 1;
	} ಅನ್ಯಥಾ ಯದಿ (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		ಯದಿ (netdev != 1)
			ಪ್ರತ್ಯಾಗಮ 1;
	} ಅನ್ಯಥಾ {
		ಯದಿ (args->ಗಂಟು_regex &&
		    regexec(args->ಗಂಟು_regex, from, 0, ರಿಕ್ತ, 0) != 0)
			ಪ್ರತ್ಯಾಗಮ 1;
		ಯದಿ (args->skip_ಗಂಟು_regex &&
		    regexec(args->skip_ಗಂಟು_regex, from, 0, ರಿಕ್ತ, 0) == 0)
			ಪ್ರತ್ಯಾಗಮ 1;

		ಯದಿ (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, ರಿಕ್ತ, 0) != 0)
			ಪ್ರತ್ಯಾಗಮ -1;
		ಯದಿ (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, ರಿಕ್ತ, 0) == 0)
			ಪ್ರತ್ಯಾಗಮ -1;

		ಯದಿ (args->options_regex &&
		    regexec(args->options_regex, options, 0, ರಿಕ್ತ, 0) != 0)
			ಪ್ರತ್ಯಾಗಮ -1;
		ಯದಿ (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, ರಿಕ್ತ, 0) == 0)
			ಪ್ರತ್ಯಾಗಮ -1;
	}

	ಯದಿ (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    ಯದಿ (strcmp(s->ಮೌಲ್ಯ, to) == 0)
			    ಭಙ್ಗ;
		ಯದಿ (!s)
			ಪ್ರತ್ಯಾಗಮ -1;
	}

	ವಿಕಲ್ಪ (args->mount_type) {
	ಪಕ್ಷ mount_from:
		p = from;
		ಭಙ್ಗ;
	ಪಕ್ಷ mount_to:
		p = to;
		ಭಙ್ಗ;
	ಪಕ್ಷ mount_fstype:
		p = fstype;
		ಭಙ್ಗ;
	ಪಕ್ಷ mount_options:
		p = options;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		p = ರಿಕ್ತ;
		errno = EINVAL;
		ಭಙ್ಗ;
	}

	ಯದಿ (p) {
		errno = 0;
		rc_stringlist_add(list, p);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ -1;
}

#ಯದಿ defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
ಸ್ಥಿರ ಸಂರಚನಾ opt {
	ಪೂರ್ಣಾಙ್ಕ o_opt;
	ಧ್ರುವ ವರ್ಣ *o_name;
} optnames[] = {
	{ MNT_ASYNC,        "asynchronous" },
	{ MNT_EXPORTED,     "NFS exported" },
	{ MNT_LOCAL,        "local" },
	{ MNT_NOATIME,      "noatime" },
	{ MNT_NOEXEC,       "noexec" },
	{ MNT_NOSUID,       "nosuid" },
#ifdef MNT_NOSYMFOLLOW
	{ MNT_NOSYMFOLLOW,  "nosymfollow" },
#endif
	{ MNT_QUOTA,        "with quotas" },
	{ MNT_RDONLY,       "read-only" },
	{ MNT_SYNCHRONOUS,  "synchronous" },
	{ MNT_UNION,        "union" },
#ifdef MNT_NOCLUSTERR
	{ MNT_NOCLUSTERR,   "noclusterr" },
#endif
#ifdef MNT_NOCLUSTERW
	{ MNT_NOCLUSTERW,   "noclusterw" },
#endif
#ifdef MNT_SUIDDIR
	{ MNT_SUIDDIR,      "suiddir" },
#endif
	{ MNT_SOFTDEP,      "soft-updates" },
#ifdef MNT_MULTILABEL
	{ MNT_MULTILABEL,   "multilabel" },
#endif
#ifdef MNT_ACLS
	{ MNT_ACLS,         "acls" },
#endif
#ifdef MNT_GJOURNAL
	{ MNT_GJOURNAL,     "gjournal" },
#endif
	{ 0, ರಿಕ್ತ }
};

ಸ್ಥಿರ RC_STRINGLIST *
find_mounts(ಸಂರಚನಾ args *args, size_t *ಅಂಕೆ_mounts)
{
	ಸಂರಚನಾ statfs *mnts;
	ಪೂರ್ಣಾಙ್ಕ nmnts;
	ಪೂರ್ಣಾಙ್ಕ i;
	RC_STRINGLIST *list;
	ವರ್ಣ *options = ರಿಕ್ತ;
	uint64_t flags;
	ಸಂರಚನಾ opt *o;
	ಪೂರ್ಣಾಙ್ಕ netdev;
	ವರ್ಣ *tmp;

	ಯದಿ ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	ಪ್ರತಿ (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		ಪ್ರತಿ (o = optnames; flags && o->o_opt; o++) {
			ಯದಿ (flags & o->o_opt) {
				ಯದಿ (o->o_opt == MNT_LOCAL)
					netdev = 1;
				ಯದಿ (!options)
					options = xstrdup(o->o_name);
				ಅನ್ಯಥಾ {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					ಮುಕ್ತ(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*ಅಂಕೆ_mounts += 0 == args->process(list, args,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		ಮುಕ್ತ(options);
		options = ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ list;
}

#ನೋಚೇದ್ಯದಿ defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
ಸ್ಥಿರ ಸಂರಚನಾ mntent *
getmntfile(ಧ್ರುವ ವರ್ಣ *file)
{
	ಸಂರಚನಾ mntent *ent = ರಿಕ್ತ;
	FILE *fp;

	ಯದಿ (!(fp = setmntent("/etc/fstab", "r")))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಯಾವತ್ ((ent = getmntent(fp)))
		ಯದಿ (strcmp(file, ent->mnt_dir) == 0)
			ಭಙ್ಗ;
	endmntent(fp);

	ಪ್ರತ್ಯಾಗಮ ent;
}

ಸ್ಥಿರ RC_STRINGLIST *
find_mounts(ಸಂರಚನಾ args *args, size_t *ಅಂಕೆ_mounts)
{
	FILE *fp;
	ವರ್ಣ *buffer;
	size_t size;
	ವರ್ಣ *p;
	ವರ್ಣ *from;
	ವರ್ಣ *to;
	ವರ್ಣ *fst;
	ವರ್ಣ *opts;
	ಸಂರಚನಾ mntent *ent;
	ಪೂರ್ಣಾಙ್ಕ netdev;
	RC_STRINGLIST *list;

	ಯದಿ ((fp = fopen(procmounts, "r")) == ರಿಕ್ತ)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	buffer = ರಿಕ್ತ;
	ಯಾವತ್ (xgetline(&buffer, &size, fp) != -1) {
		netdev = -1;
		p = buffer;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		ಯದಿ ((ent = getmntfile(to))) {
			ಯದಿ (strstr(ent->mnt_opts, "_netdev"))
				netdev = 0;
			ಅನ್ಯಥಾ
				netdev = 1;
		}

		*ಅಂಕೆ_mounts += 0 == args->process(list, args,
			from, to, fst, opts, netdev);

		ಮುಕ್ತ(buffer);
		buffer = ರಿಕ್ತ;
	}
	ಮುಕ್ತ(buffer);
	fclose(fp);

	ಪ್ರತ್ಯಾಗಮ list;
}

#ಅನ್ಯಥಾ
#  error "Operating system not supported!"
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ is_prefix(ಧ್ರುವ ವರ್ಣ *needle, ಧ್ರುವ ವರ್ಣ *hay)
{
	size_t nlen = strlen(needle);
	ಯದಿ (strncmp(needle, hay, nlen) == 0 && hay[nlen] == '/')
		ಪ್ರತ್ಯಾಗಮ true;
	ಪ್ರತ್ಯಾಗಮ false;
}

ಸ್ಥಿರ ವರ್ಣ *unescape_octal(ವರ್ಣ *beg)
{
	ಪೂರ್ಣಾಙ್ಕ n, i;
	ವರ್ಣ *w = beg, *r = beg;
	ಯಾವತ್ (*r) {
		ಯದಿ (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} ಅನ್ಯಥಾ {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			ಪ್ರತಿ (i = n = 0; i < 3; ++i) {
				ಯದಿ (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} ಅನ್ಯಥಾ {
					ಭಙ್ಗ;
				}
			}
			ಯದಿ (n)
				*w++ = n;
		}
	}
	*w = '\0';
	ಪ್ರತ್ಯಾಗಮ beg;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ check_is_mounted(RC_STRINGLIST *list RC_UNUSED, ಸಂರಚನಾ args *args,
    ವರ್ಣ *from RC_UNUSED, ವರ್ಣ *to, ವರ್ಣ *fstype RC_UNUSED,
    ವರ್ಣ *options RC_UNUSED, ಪೂರ್ಣಾಙ್ಕ netdev RC_UNUSED)
{
	ಪ್ರತ್ಯಾಗಮ strcmp(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ is_mounted(ಧ್ರುವ ವರ್ಣ *mntpath)
{
	size_t ಅಂಕೆ_mounts = 0;
	ಸಂರಚನಾ args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &ಅಂಕೆ_mounts);
	rc_stringlist_free(l);
	ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ_mounts > 0;
}

ಸ್ಥಿರ pid_t run_umount(ಧ್ರುವ ವರ್ಣ *mntpath,
	ಧ್ರುವ ವರ್ಣ **umount_args, ಪೂರ್ಣಾಙ್ಕ umount_args_ಅಂಕೆ)
{
	ಸಂರಚನಾ exec_args args;
	ಸಂರಚನಾ exec_result res;
	ಧ್ರುವ ವರ್ಣ *argv[UMOUNT_ARGS_MAX + 3];
	ಪೂರ್ಣಾಙ್ಕ k, i = 0;

	argv[i++] = "umount";
	ಪ್ರತಿ (k = 0; k < umount_args_ಅಂಕೆ; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = ರಿಕ್ತ;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	ಯದಿ (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	ಪ್ರತ್ಯಾಗಮ res.pid;
}

ಸ್ಥಿರ ಶೂನ್ಯ fuser_run(ಸಂರಚನಾ run_queue *rp, ಧ್ರುವ ವರ್ಣ *fuser_opt)
{
	ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fuser_exec_failed = 0;
	ಧ್ರುವ ವರ್ಣ *argv[] = { "fuser", fuser_opt, rp->mntpath, ರಿಕ್ತ };
	ಸಂರಚನಾ exec_result res;
	ಸಂರಚನಾ exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	ಯದಿ (fuser_exec_failed)
		ಪ್ರತ್ಯಾಗಮ;

	args = exec_init(argv);
	args.redirect_stdout = EXEC_MKPIPE;
	args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	ಯದಿ (res.pid < 0) {
		fuser_exec_failed = 1;
	} ಅನ್ಯಥಾ {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fuser_decide(ಸಂರಚನಾ run_queue *rp,
	ಧ್ರುವ ವರ್ಣ *fuser_opt, ಧ್ರುವ ವರ್ಣ *fuser_kill_prefix)
{
	ಸ್ಥಿರ bool has_rc_pid = false;
	ಸ್ಥಿರ ವರ್ಣ *rc_pid;
	ವರ್ಣ buf[1<<12];
	ವರ್ಣ selfpid[64];
	ಪೂರ್ಣಾಙ್ಕ read_maybe_truncated;
	ssize_t n;

	ಯದಿ (rp->fuser_stdoutfd < 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	buf[0] = ' ';
	n = ಪಠ(rp->fuser_stdoutfd, buf + 1, ಪರಿಮಾಣ buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == ಪರಿಮಾಣ buf - 3);
	ಯದಿ (n < 0 || read_maybe_truncated)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯಾವತ್ (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, ಪರಿಮಾಣ selfpid, " %lld ", (long long)getpid());

	ಯದಿ (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	ಯದಿ (strstr(buf, selfpid) || (rc_pid && strstr(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		ಪ್ರತ್ಯಾಗಮ -1;
	} ಅನ್ಯಥಾ ಯದಿ (strcmp(buf, "  ") == 0) {
		ಯದಿ (rp->try_ಎಣಿಕೆ >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			ಪ್ರತ್ಯಾಗಮ -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_ಎಣಿಕೆ = TRY_MAX;
		ಪ್ರತ್ಯಾಗಮ 0;
	} ಅನ್ಯಥಾ {
		ವರ್ಣ sig[32];
		ಧ್ರುವ ವರ್ಣ *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, ರಿಕ್ತ
		};
		ಸಂರಚನಾ exec_result res;
		ಸಂರಚನಾ exec_args args = exec_init(argv);
		args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, ಪರಿಮಾಣ sig, "%s%s", fuser_kill_prefix,
			rp->try_ಎಣಿಕೆ == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&args);
		ಯದಿ (res.pid > 0)
			rc_waitpid(res.pid);
		ಪ್ರತ್ಯಾಗಮ 0;
	}
}

ಸ್ಥಿರ regex_t *
get_regex(ಧ್ರುವ ವರ್ಣ *string)
{
	regex_t *reg = xmalloc(ಪರಿಮಾಣ (*reg));
	ಪೂರ್ಣಾಙ್ಕ result;
	ವರ್ಣ buffer[256];

	ಯದಿ ((result = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(result, reg, buffer, ಪರಿಮಾಣ(buffer));
		eerrorx("%s: invalid regex `%s'", applet, buffer);
	}

	ಪ್ರತ್ಯಾಗಮ reg;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಸಂರಚನಾ args args;
	regex_t *point_regex = ರಿಕ್ತ;
	regex_t *skip_point_regex = ರಿಕ್ತ;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	ವರ್ಣ *real_path = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ opt;
	ಪೂರ್ಣಾಙ್ಕ result;
	ವರ್ಣ *this_path, *argv0;
	ಧ್ರುವ ವರ್ಣ *tmps;
	ಧ್ರುವ ವರ್ಣ *fuser_opt, *fuser_kill_prefix;
	size_t ಅಂಕೆ_mounts = 0;
	size_t unmount_index;
	ಪೂರ್ಣಾಙ್ಕ doing_unmount = 0;
	pid_t pid;
	ಪೂರ್ಣಾಙ್ಕ status, flags;
	int64_t tmp, next_retry, now;
	int64_t rc_fuser_timeout = -1;
	ಧ್ರುವ ವರ್ಣ *umount_args[UMOUNT_ARGS_MAX];
	ಪೂರ್ಣಾಙ್ಕ umount_args_ಅಂಕೆ = 0;
	ಧ್ರುವ ವರ್ಣ **mounts = ರಿಕ್ತ;
	ಸಂರಚನಾ run_queue running[RUN_MAX] = {0};
	ಸಂರಚನಾ run_queue *rp;
	size_t ಅಂಕೆ_running = 0, ಅಂಕೆ_waiting = 0;
	ಗಣನಾ { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	ಯದಿ (_var) ಮುಕ್ತ(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	ಯದಿ (_var) { regfree(_var); ಮುಕ್ತ(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&args, 0, ಪರಿಮಾಣ(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	ಯದಿ (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		ಯಾವತ್ (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			ಯದಿ (strcmp(tmps, "--") == 0)
				ಭಙ್ಗ;
			ಯದಿ (umount_args_ಅಂಕೆ >= (ಪೂರ್ಣಾಙ್ಕ)ARRAY_SIZE(umount_args))
				eerrorx("%s: Too many umount arguments", applet);
			umount_args[umount_args_ಅಂಕೆ++] = tmps;
		}

		tmps = rc_conf_ಮೌಲ್ಯ("rc_fuser_timeout");
		ಯದಿ (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		ಯದಿ (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		ಯದಿ (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} ಅನ್ಯಥಾ {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಙ್ಕ *) 0)) != -1)
	{
		ವಿಕಲ್ಪ (opt) {
		ಪಕ್ಷ 'e':
			args.netdev = net_yes;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'E':
			args.netdev = net_no;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'f':
			DO_REG(args.fstype_regex);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'F':
			DO_REG(args.skip_fstype_regex);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'n':
			DO_REG(args.ಗಂಟು_regex);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'N':
			DO_REG(args.skip_ಗಂಟು_regex);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'o':
			DO_REG(args.options_regex);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'O':
			DO_REG(args.skip_options_regex);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'p':
			DO_REG(point_regex);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'P':
			DO_REG(skip_point_regex);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'i':
			args.mount_type = mount_options;
			ಭಙ್ಗ;
		ಪಕ್ಷ 's':
			args.mount_type = mount_fstype;
			ಭಙ್ಗ;
		ಪಕ್ಷ 't':
			args.mount_type = mount_from;
			ಭಙ್ಗ;

		case_RC_COMMON_GETOPT
		}
	}

	ಯಾವತ್ (optind < argc) {
		ಯದಿ (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, ರಿಕ್ತ);
		ಯದಿ (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		ಮುಕ್ತ(real_path);
		real_path = ರಿಕ್ತ;
	}
	nodes = find_mounts(&args, &ಅಂಕೆ_mounts);
	rc_stringlist_free(args.mounts);

	ಯದಿ (doing_unmount)
		mounts = xmalloc(ಅಂಕೆ_mounts * ಪರಿಮಾಣ(*mounts));
	ಅಂಕೆ_mounts = 0;
	result = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		ಯದಿ (point_regex &&
		    regexec(point_regex, s->ಮೌಲ್ಯ, 0, ರಿಕ್ತ, 0) != 0)
			ಅನುವರ್ತ;
		ಯದಿ (skip_point_regex &&
		    regexec(skip_point_regex, s->ಮೌಲ್ಯ, 0, ರಿಕ್ತ, 0) == 0)
			ಅನುವರ್ತ;
		ಯದಿ (doing_unmount)
			mounts[ಅಂಕೆ_mounts++] = unescape_octal(s->ಮೌಲ್ಯ);
		ಅನ್ಯಥಾ ಯದಿ (!rc_yesno(getenv("EINFO_QUIET")))
			printf("%s\n", s->ಮೌಲ್ಯ);
		result = EXIT_SUCCESS;
	}
	ಯದಿ (!doing_unmount)
		ಗಚ್ಛ exit;

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
	result = EXIT_SUCCESS;
	state = STATE_RUN;
	ಯಾವತ್ (state != STATE_END) ವಿಕಲ್ಪ (state) {
	ಪಕ್ಷ STATE_RUN:
		ಪ್ರತಿ (unmount_index = 0; unmount_index < ಅಂಕೆ_mounts; ++unmount_index) {
			ಧ್ರುವ ವರ್ಣ *candidate = mounts[unmount_index];
			ಪೂರ್ಣಾಙ್ಕ safe_to_unmount = 1;
			ಪ್ರತಿ (size_t k = 0; safe_to_unmount && k < ಅಂಕೆ_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			ಪ್ರತಿ (size_t k = 0; safe_to_unmount && k < ಅಂಕೆ_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			ಯದಿ (!safe_to_unmount)
				ಅನುವರ್ತ;
			ಯದಿ (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_index--] = mounts[--ಅಂಕೆ_mounts];
			} ಅನ್ಯಥಾ {
				ಭಙ್ಗ;
			}
		}
		ಯದಿ (ಅಂಕೆ_running == RUN_MAX || unmount_index >= ಅಂಕೆ_mounts) {
			state = STATE_REAP;
			ಭಙ್ಗ;
		}
		rp = running + ಅಂಕೆ_running++;
		rp->mntpath = mounts[unmount_index];
		rp->last_exec_time = tm_now();
		rp->try_ಎಣಿಕೆ = 0;
		rp->pid = run_umount(rp->mntpath, umount_args, umount_args_ಅಂಕೆ);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_index] = mounts[--ಅಂಕೆ_mounts];
		ಭಙ್ಗ;
	ಪಕ್ಷ STATE_REAP:
		flags = (ಅಂಕೆ_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = ರಿಕ್ತ;
		ಪ್ರತಿ (size_t i = 0; i < ಅಂಕೆ_running; ++i) {
			rp = running + i;
			ಯದಿ (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			ಯದಿ (rp->pid == pid && pid > 0)
				ಭಙ್ಗ;
			rp = ರಿಕ್ತ;
		}
		ಯದಿ (rp) {
			ಯದಿ ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--ಅಂಕೆ_running];
				state = STATE_RUN;
			} ಅನ್ಯಥಾ ಯದಿ (rp->try_ಎಣಿಕೆ >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--ಅಂಕೆ_running];
				result = EXIT_FAILURE;
			} ಅನ್ಯಥಾ { /* put into waiting queue */
				rp->pid = -1;
				rp->try_ಎಣಿಕೆ += 1;
				ಅಂಕೆ_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} ಅನ್ಯಥಾ {
			state = STATE_RETRY;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ STATE_RETRY:
		rp = ರಿಕ್ತ;
		next_retry = INT64_MAX;
		ಪ್ರತಿ (size_t i = 0; i < ಅಂಕೆ_running; ++i) {
			ಯದಿ (running[i].pid > 0)
				ಅನುವರ್ತ;
			ಯದಿ (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			ಅನ್ಯಥಾ
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			ಯದಿ (tmp < next_retry) {
				rp = running + i;
				next_retry = tmp;
			}
		}
		ಯದಿ (!rp) {
			state = (ಅಂಕೆ_mounts > 0) ? STATE_RUN : STATE_END;
			ಭಙ್ಗ;
		}
		now = tm_now();
		ಯದಿ ((rp->pid > 0 || rp->fuser_pid > 0) && next_retry > now) {
			int64_t sleep_for = next_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			ಯದಿ (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		ಯದಿ ((rp->pid == -1 && rp->fuser_pid == -1) || next_retry <= now) {
			ಯದಿ (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, ರಿಕ್ತ, 0);
				rp->fuser_pid = -1;
			}
			ಯದಿ (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--ಅಂಕೆ_running];
				result = EXIT_FAILURE;
			} ಅನ್ಯಥಾ { /* retry */
				rp->last_exec_time = tm_now();
				rp->pid = run_umount(rp->mntpath,
					umount_args, umount_args_ಅಂಕೆ);
			}
			ಅಂಕೆ_waiting -= 1;
			state = STATE_REAP;
		}
		ಭಙ್ಗ;
	ಸ್ವಭಾವ: ಭಙ್ಗ;
	}

exit:
	ಮುಕ್ತ(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(args.fstype_regex);
	REG_FREE(args.skip_fstype_regex);
	REG_FREE(args.ಗಂಟು_regex);
	REG_FREE(args.skip_ಗಂಟು_regex);
	REG_FREE(args.options_regex);
	REG_FREE(args.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	ಪ್ರತ್ಯಾಗಮ result;
}
