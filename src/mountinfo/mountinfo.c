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

#ຖ້າ defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#elif defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#elif defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#elif defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#endif

#include <errno.h>
#include <getopt.h>
#include <regex.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "rc_exec.h"
#include "timeutils.h"
#include "_usage.h"
#include "helpers.h"

const var_na *applet = NULL;
const var_na *procmounts = "/proc/mounts";
const var_na *extraopts = "[mount1] [mount2] ...";
const var_na getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
const sa_nracanaa option longopts[] = {
	{ "fstype-regex",        1, NULL, 'f'},
	{ "skip-fstype-regex",   1, NULL, 'F'},
	{ "node-regex",          1, NULL, 'n'},
	{ "skip-node-regex",     1, NULL, 'N'},
	{ "options-regex",       1, NULL, 'o'},
	{ "skip-options-regex",  1, NULL, 'O'},
	{ "point-regex",         1, NULL, 'p'},
	{ "skip-point-regex",    1, NULL, 'P'},
	{ "options",             0, NULL, 'i'},
	{ "fstype",              0, NULL, 's'},
	{ "node",                0, NULL, 't'},
	{ "netdev",              0, NULL, 'e'},
	{ "nonetdev",            0, NULL, 'E'},
	longopts_COMMON
};
const var_na * const longopts_help[] = {
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
const var_na *usagestring = NULL;

#define UMOUNT_ARGS_MAX 16
#define RUN_MAX 32
#define TRY_MAX 3
#define TRY_DELAY_MS 1000

sa_nracanaa run_queue {
	const var_na *mntpath;
	int64_t last_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	ຈຳນວນເຕັມ try_count;
	ຈຳນວນເຕັມ fuser_stdoutfd;
};

typedef ga_nanaa {
	mount_from,
	mount_to,
	mount_fstype,
	mount_options
} mount_type;

typedef ga_nanaa {
	net_ignore,
	net_yes,
	net_no
} net_opts;

sa_nracanaa args;
typedef ຈຳນວນເຕັມ process_func_t(RC_STRINGLIST *, sa_nracanaa args *,
    var_na *, var_na *, var_na *, var_na *, ຈຳນວນເຕັມ);

sa_nracanaa args {
	process_func_t *process;
	regex_t *node_regex;
	regex_t *skip_node_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	const var_na *check_mntpath;
};

static ຈຳນວນເຕັມ
process_mount(RC_STRINGLIST *list, sa_nracanaa args *args,
    var_na *from, var_na *to, var_na *fstype, var_na *options,
    ຈຳນວນເຕັມ netdev)
{
	var_na *p;
	RC_STRING *s;

	errno = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	ຖ້າ (strcmp(fstype, "rootfs") == 0)
		ສົ່ງຄືນ -1;
#endif

	ຖ້າ (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		ຖ້າ (netdev != 0)
			ສົ່ງຄືນ 1;
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		ຖ້າ (netdev != 1)
			ສົ່ງຄືນ 1;
	} ບໍ່ດັ່ງນັ້ນ {
		ຖ້າ (args->node_regex &&
		    regexec(args->node_regex, from, 0, NULL, 0) != 0)
			ສົ່ງຄືນ 1;
		ຖ້າ (args->skip_node_regex &&
		    regexec(args->skip_node_regex, from, 0, NULL, 0) == 0)
			ສົ່ງຄືນ 1;

		ຖ້າ (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, NULL, 0) != 0)
			ສົ່ງຄືນ -1;
		ຖ້າ (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, NULL, 0) == 0)
			ສົ່ງຄືນ -1;

		ຖ້າ (args->options_regex &&
		    regexec(args->options_regex, options, 0, NULL, 0) != 0)
			ສົ່ງຄືນ -1;
		ຖ້າ (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, NULL, 0) == 0)
			ສົ່ງຄືນ -1;
	}

	ຖ້າ (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    ຖ້າ (strcmp(s->value, to) == 0)
			    bha_nga;
		ຖ້າ (!s)
			ສົ່ງຄືນ -1;
	}

	switch (args->mount_type) {
	pak_sa mount_from:
		p = from;
		bha_nga;
	pak_sa mount_to:
		p = to;
		bha_nga;
	pak_sa mount_fstype:
		p = fstype;
		bha_nga;
	pak_sa mount_options:
		p = options;
		bha_nga;
	default:
		p = NULL;
		errno = EINVAL;
		bha_nga;
	}

	ຖ້າ (p) {
		errno = 0;
		rc_stringlist_add(list, p);
		ສົ່ງຄືນ 0;
	}

	ສົ່ງຄືນ -1;
}

#ຖ້າ defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
static sa_nracanaa opt {
	ຈຳນວນເຕັມ o_opt;
	const var_na *o_name;
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
	{ 0, NULL }
};

static RC_STRINGLIST *
find_mounts(sa_nracanaa args *args, size_t *num_mounts)
{
	sa_nracanaa statfs *mnts;
	ຈຳນວນເຕັມ nmnts;
	ຈຳນວນເຕັມ i;
	RC_STRINGLIST *list;
	var_na *options = NULL;
	uint64_t flags;
	sa_nracanaa opt *o;
	ຈຳນວນເຕັມ netdev;
	var_na *tmp;

	ຖ້າ ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	ສຳລັບ (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		ສຳລັບ (o = optnames; flags && o->o_opt; o++) {
			ຖ້າ (flags & o->o_opt) {
				ຖ້າ (o->o_opt == MNT_LOCAL)
					netdev = 1;
				ຖ້າ (!options)
					options = xstrdup(o->o_name);
				ບໍ່ດັ່ງນັ້ນ {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					free(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*num_mounts += 0 == args->process(list, args,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		free(options);
		options = NULL;
	}

	ສົ່ງຄືນ list;
}

#elif defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
static sa_nracanaa mntent *
getmntfile(const var_na *file)
{
	sa_nracanaa mntent *ent = NULL;
	FILE *fp;

	ຖ້າ (!(fp = setmntent("/etc/fstab", "r")))
		ສົ່ງຄືນ NULL;
	ໃນຂະນະທີ່ ((ent = getmntent(fp)))
		ຖ້າ (strcmp(file, ent->mnt_dir) == 0)
			bha_nga;
	endmntent(fp);

	ສົ່ງຄືນ ent;
}

static RC_STRINGLIST *
find_mounts(sa_nracanaa args *args, size_t *num_mounts)
{
	FILE *fp;
	var_na *buffer;
	size_t size;
	var_na *p;
	var_na *from;
	var_na *to;
	var_na *fst;
	var_na *opts;
	sa_nracanaa mntent *ent;
	ຈຳນວນເຕັມ netdev;
	RC_STRINGLIST *list;

	ຖ້າ ((fp = fopen(procmounts, "r")) == NULL)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	buffer = NULL;
	ໃນຂະນະທີ່ (xgetline(&buffer, &size, fp) != -1) {
		netdev = -1;
		p = buffer;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		ຖ້າ ((ent = getmntfile(to))) {
			ຖ້າ (strstr(ent->mnt_opts, "_netdev"))
				netdev = 0;
			ບໍ່ດັ່ງນັ້ນ
				netdev = 1;
		}

		*num_mounts += 0 == args->process(list, args,
			from, to, fst, opts, netdev);

		free(buffer);
		buffer = NULL;
	}
	free(buffer);
	fclose(fp);

	ສົ່ງຄືນ list;
}

#ບໍ່ດັ່ງນັ້ນ
#  error "Operating system not supported!"
#endif

static ຈຳນວນເຕັມ is_prefix(const var_na *needle, const var_na *hay)
{
	size_t nlen = strlen(needle);
	ຖ້າ (strncmp(needle, hay, nlen) == 0 && hay[nlen] == '/')
		ສົ່ງຄືນ true;
	ສົ່ງຄືນ false;
}

static var_na *unescape_octal(var_na *beg)
{
	ຈຳນວນເຕັມ n, i;
	var_na *w = beg, *r = beg;
	ໃນຂະນະທີ່ (*r) {
		ຖ້າ (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} ບໍ່ດັ່ງນັ້ນ {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			ສຳລັບ (i = n = 0; i < 3; ++i) {
				ຖ້າ (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} ບໍ່ດັ່ງນັ້ນ {
					bha_nga;
				}
			}
			ຖ້າ (n)
				*w++ = n;
		}
	}
	*w = '\0';
	ສົ່ງຄືນ beg;
}

static ຈຳນວນເຕັມ check_is_mounted(RC_STRINGLIST *list RC_UNUSED, sa_nracanaa args *args,
    var_na *from RC_UNUSED, var_na *to, var_na *fstype RC_UNUSED,
    var_na *options RC_UNUSED, ຈຳນວນເຕັມ netdev RC_UNUSED)
{
	ສົ່ງຄືນ strcmp(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

static ຈຳນວນເຕັມ is_mounted(const var_na *mntpath)
{
	size_t num_mounts = 0;
	sa_nracanaa args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &num_mounts);
	rc_stringlist_free(l);
	ສົ່ງຄືນ num_mounts > 0;
}

static pid_t run_umount(const var_na *mntpath,
	const var_na **umount_args, ຈຳນວນເຕັມ umount_args_num)
{
	sa_nracanaa exec_args args;
	sa_nracanaa exec_result res;
	const var_na *argv[UMOUNT_ARGS_MAX + 3];
	ຈຳນວນເຕັມ k, i = 0;

	argv[i++] = "umount";
	ສຳລັບ (k = 0; k < umount_args_num; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = NULL;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	ຖ້າ (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	ສົ່ງຄືນ res.pid;
}

static void fuser_run(sa_nracanaa run_queue *rp, const var_na *fuser_opt)
{
	static ຈຳນວນເຕັມ fuser_exec_failed = 0;
	const var_na *argv[] = { "fuser", fuser_opt, rp->mntpath, NULL };
	sa_nracanaa exec_result res;
	sa_nracanaa exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	ຖ້າ (fuser_exec_failed)
		ສົ່ງຄືນ;

	args = exec_init(argv);
	args.redirect_stdout = EXEC_MKPIPE;
	args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	ຖ້າ (res.pid < 0) {
		fuser_exec_failed = 1;
	} ບໍ່ດັ່ງນັ້ນ {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

static ຈຳນວນເຕັມ fuser_decide(sa_nracanaa run_queue *rp,
	const var_na *fuser_opt, const var_na *fuser_kill_prefix)
{
	static bool has_rc_pid = false;
	static var_na *rc_pid;
	var_na buf[1<<12];
	var_na selfpid[64];
	ຈຳນວນເຕັມ read_maybe_truncated;
	ssize_t n;

	ຖ້າ (rp->fuser_stdoutfd < 0)
		ສົ່ງຄືນ 0;

	buf[0] = ' ';
	n = read(rp->fuser_stdoutfd, buf + 1, parimaa_na buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == parimaa_na buf - 3);
	ຖ້າ (n < 0 || read_maybe_truncated)
		ສົ່ງຄືນ 0;
	ໃນຂະນະທີ່ (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, parimaa_na selfpid, " %lld ", (long long)getpid());

	ຖ້າ (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	ຖ້າ (strstr(buf, selfpid) || (rc_pid && strstr(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		ສົ່ງຄືນ -1;
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(buf, "  ") == 0) {
		ຖ້າ (rp->try_count >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			ສົ່ງຄືນ -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_count = TRY_MAX;
		ສົ່ງຄືນ 0;
	} ບໍ່ດັ່ງນັ້ນ {
		var_na sig[32];
		const var_na *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, NULL
		};
		sa_nracanaa exec_result res;
		sa_nracanaa exec_args args = exec_init(argv);
		args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, parimaa_na sig, "%s%s", fuser_kill_prefix,
			rp->try_count == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&args);
		ຖ້າ (res.pid > 0)
			rc_waitpid(res.pid);
		ສົ່ງຄືນ 0;
	}
}

static regex_t *
get_regex(const var_na *string)
{
	regex_t *reg = xmalloc(parimaa_na (*reg));
	ຈຳນວນເຕັມ result;
	var_na buffer[256];

	ຖ້າ ((result = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(result, reg, buffer, parimaa_na(buffer));
		eerrorx("%s: invalid regex `%s'", applet, buffer);
	}

	ສົ່ງຄືນ reg;
}

ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na **argv)
{
	sa_nracanaa args args;
	regex_t *point_regex = NULL;
	regex_t *skip_point_regex = NULL;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	var_na *real_path = NULL;
	ຈຳນວນເຕັມ opt;
	ຈຳນວນເຕັມ result;
	var_na *this_path, *argv0;
	const var_na *tmps;
	const var_na *fuser_opt, *fuser_kill_prefix;
	size_t num_mounts = 0;
	size_t unmount_index;
	ຈຳນວນເຕັມ doing_unmount = 0;
	pid_t pid;
	ຈຳນວນເຕັມ status, flags;
	int64_t tmp, next_retry, now;
	int64_t rc_fuser_timeout = -1;
	const var_na *umount_args[UMOUNT_ARGS_MAX];
	ຈຳນວນເຕັມ umount_args_num = 0;
	const var_na **mounts = NULL;
	sa_nracanaa run_queue running[RUN_MAX] = {0};
	sa_nracanaa run_queue *rp;
	size_t num_running = 0, num_waiting = 0;
	ga_nanaa { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	ຖ້າ (_var) free(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	ຖ້າ (_var) { regfree(_var); free(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&args, 0, parimaa_na(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	ຖ້າ (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		ໃນຂະນະທີ່ (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			ຖ້າ (strcmp(tmps, "--") == 0)
				bha_nga;
			ຖ້າ (umount_args_num >= (ຈຳນວນເຕັມ)ARRAY_SIZE(umount_args))
				eerrorx("%s: Too many umount arguments", applet);
			umount_args[umount_args_num++] = tmps;
		}

		tmps = rc_conf_value("rc_fuser_timeout");
		ຖ້າ (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		ຖ້າ (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		ຖ້າ (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} ບໍ່ດັ່ງນັ້ນ {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	ໃນຂະນະທີ່ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ຈຳນວນເຕັມ *) 0)) != -1)
	{
		switch (opt) {
		pak_sa 'e':
			args.netdev = net_yes;
			bha_nga;
		pak_sa 'E':
			args.netdev = net_no;
			bha_nga;
		pak_sa 'f':
			DO_REG(args.fstype_regex);
			bha_nga;
		pak_sa 'F':
			DO_REG(args.skip_fstype_regex);
			bha_nga;
		pak_sa 'n':
			DO_REG(args.node_regex);
			bha_nga;
		pak_sa 'N':
			DO_REG(args.skip_node_regex);
			bha_nga;
		pak_sa 'o':
			DO_REG(args.options_regex);
			bha_nga;
		pak_sa 'O':
			DO_REG(args.skip_options_regex);
			bha_nga;
		pak_sa 'p':
			DO_REG(point_regex);
			bha_nga;
		pak_sa 'P':
			DO_REG(skip_point_regex);
			bha_nga;
		pak_sa 'i':
			args.mount_type = mount_options;
			bha_nga;
		pak_sa 's':
			args.mount_type = mount_fstype;
			bha_nga;
		pak_sa 't':
			args.mount_type = mount_from;
			bha_nga;

		case_RC_COMMON_GETOPT
		}
	}

	ໃນຂະນະທີ່ (optind < argc) {
		ຖ້າ (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, NULL);
		ຖ້າ (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		free(real_path);
		real_path = NULL;
	}
	nodes = find_mounts(&args, &num_mounts);
	rc_stringlist_free(args.mounts);

	ຖ້າ (doing_unmount)
		mounts = xmalloc(num_mounts * parimaa_na(*mounts));
	num_mounts = 0;
	result = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		ຖ້າ (point_regex &&
		    regexec(point_regex, s->value, 0, NULL, 0) != 0)
			continue;
		ຖ້າ (skip_point_regex &&
		    regexec(skip_point_regex, s->value, 0, NULL, 0) == 0)
			continue;
		ຖ້າ (doing_unmount)
			mounts[num_mounts++] = unescape_octal(s->value);
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (!rc_yesno(getenv("EINFO_QUIET")))
			printf("%s\n", s->value);
		result = EXIT_SUCCESS;
	}
	ຖ້າ (!doing_unmount)
		goto exit;

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
	ໃນຂະນະທີ່ (state != STATE_END) switch (state) {
	pak_sa STATE_RUN:
		ສຳລັບ (unmount_index = 0; unmount_index < num_mounts; ++unmount_index) {
			const var_na *candidate = mounts[unmount_index];
			ຈຳນວນເຕັມ safe_to_unmount = 1;
			ສຳລັບ (size_t k = 0; safe_to_unmount && k < num_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			ສຳລັບ (size_t k = 0; safe_to_unmount && k < num_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			ຖ້າ (!safe_to_unmount)
				continue;
			ຖ້າ (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_index--] = mounts[--num_mounts];
			} ບໍ່ດັ່ງນັ້ນ {
				bha_nga;
			}
		}
		ຖ້າ (num_running == RUN_MAX || unmount_index >= num_mounts) {
			state = STATE_REAP;
			bha_nga;
		}
		rp = running + num_running++;
		rp->mntpath = mounts[unmount_index];
		rp->last_exec_time = tm_now();
		rp->try_count = 0;
		rp->pid = run_umount(rp->mntpath, umount_args, umount_args_num);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_index] = mounts[--num_mounts];
		bha_nga;
	pak_sa STATE_REAP:
		flags = (num_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = NULL;
		ສຳລັບ (size_t i = 0; i < num_running; ++i) {
			rp = running + i;
			ຖ້າ (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			ຖ້າ (rp->pid == pid && pid > 0)
				bha_nga;
			rp = NULL;
		}
		ຖ້າ (rp) {
			ຖ້າ ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--num_running];
				state = STATE_RUN;
			} ບໍ່ດັ່ງນັ້ນ ຖ້າ (rp->try_count >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--num_running];
				result = EXIT_FAILURE;
			} ບໍ່ດັ່ງນັ້ນ { /* put into waiting queue */
				rp->pid = -1;
				rp->try_count += 1;
				num_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} ບໍ່ດັ່ງນັ້ນ {
			state = STATE_RETRY;
		}
		bha_nga;
	pak_sa STATE_RETRY:
		rp = NULL;
		next_retry = INT64_MAX;
		ສຳລັບ (size_t i = 0; i < num_running; ++i) {
			ຖ້າ (running[i].pid > 0)
				continue;
			ຖ້າ (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			ບໍ່ດັ່ງນັ້ນ
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			ຖ້າ (tmp < next_retry) {
				rp = running + i;
				next_retry = tmp;
			}
		}
		ຖ້າ (!rp) {
			state = (num_mounts > 0) ? STATE_RUN : STATE_END;
			bha_nga;
		}
		now = tm_now();
		ຖ້າ ((rp->pid > 0 || rp->fuser_pid > 0) && next_retry > now) {
			int64_t sleep_for = next_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			ຖ້າ (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		ຖ້າ ((rp->pid == -1 && rp->fuser_pid == -1) || next_retry <= now) {
			ຖ້າ (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, NULL, 0);
				rp->fuser_pid = -1;
			}
			ຖ້າ (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--num_running];
				result = EXIT_FAILURE;
			} ບໍ່ດັ່ງນັ້ນ { /* retry */
				rp->last_exec_time = tm_now();
				rp->pid = run_umount(rp->mntpath,
					umount_args, umount_args_num);
			}
			num_waiting -= 1;
			state = STATE_REAP;
		}
		bha_nga;
	default: bha_nga;
	}

exit:
	free(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(args.fstype_regex);
	REG_FREE(args.skip_fstype_regex);
	REG_FREE(args.node_regex);
	REG_FREE(args.skip_node_regex);
	REG_FREE(args.options_regex);
	REG_FREE(args.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	ສົ່ງຄືນ result;
}
