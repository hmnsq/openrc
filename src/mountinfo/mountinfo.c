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

#ᱡᱩᱫᱤ defined(__DragonFly__) || defined(__FreeBSD__)
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
	puur_naa_nka try_count;
	puur_naa_nka fuser_stdoutfd;
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
typedef puur_naa_nka process_func_t(RC_STRINGLIST *, sa_nracanaa args *,
    var_na *, var_na *, var_na *, var_na *, puur_naa_nka);

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

static puur_naa_nka
process_mount(RC_STRINGLIST *list, sa_nracanaa args *args,
    var_na *from, var_na *to, var_na *fstype, var_na *options,
    puur_naa_nka netdev)
{
	var_na *p;
	RC_STRING *s;

	errno = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	ᱡᱩᱫᱤ (strcmp(fstype, "rootfs") == 0)
		ᱨᱩᱣᱟᱹᱲ -1;
#endif

	ᱡᱩᱫᱤ (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		ᱡᱩᱫᱤ (netdev != 0)
			ᱨᱩᱣᱟᱹᱲ 1;
	} ᱵᱟᱝ ᱡᱩᱫᱤ (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		ᱡᱩᱫᱤ (netdev != 1)
			ᱨᱩᱣᱟᱹᱲ 1;
	} ᱵᱟᱝ {
		ᱡᱩᱫᱤ (args->node_regex &&
		    regexec(args->node_regex, from, 0, NULL, 0) != 0)
			ᱨᱩᱣᱟᱹᱲ 1;
		ᱡᱩᱫᱤ (args->skip_node_regex &&
		    regexec(args->skip_node_regex, from, 0, NULL, 0) == 0)
			ᱨᱩᱣᱟᱹᱲ 1;

		ᱡᱩᱫᱤ (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, NULL, 0) != 0)
			ᱨᱩᱣᱟᱹᱲ -1;
		ᱡᱩᱫᱤ (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, NULL, 0) == 0)
			ᱨᱩᱣᱟᱹᱲ -1;

		ᱡᱩᱫᱤ (args->options_regex &&
		    regexec(args->options_regex, options, 0, NULL, 0) != 0)
			ᱨᱩᱣᱟᱹᱲ -1;
		ᱡᱩᱫᱤ (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, NULL, 0) == 0)
			ᱨᱩᱣᱟᱹᱲ -1;
	}

	ᱡᱩᱫᱤ (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    ᱡᱩᱫᱤ (strcmp(s->value, to) == 0)
			    bha_nga;
		ᱡᱩᱫᱤ (!s)
			ᱨᱩᱣᱟᱹᱲ -1;
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

	ᱡᱩᱫᱤ (p) {
		errno = 0;
		rc_stringlist_add(list, p);
		ᱨᱩᱣᱟᱹᱲ 0;
	}

	ᱨᱩᱣᱟᱹᱲ -1;
}

#ᱡᱩᱫᱤ defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
static sa_nracanaa opt {
	puur_naa_nka o_opt;
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
	puur_naa_nka nmnts;
	puur_naa_nka i;
	RC_STRINGLIST *list;
	var_na *options = NULL;
	uint64_t flags;
	sa_nracanaa opt *o;
	puur_naa_nka netdev;
	var_na *tmp;

	ᱡᱩᱫᱤ ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	for (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		for (o = optnames; flags && o->o_opt; o++) {
			ᱡᱩᱫᱤ (flags & o->o_opt) {
				ᱡᱩᱫᱤ (o->o_opt == MNT_LOCAL)
					netdev = 1;
				ᱡᱩᱫᱤ (!options)
					options = xstrdup(o->o_name);
				ᱵᱟᱝ {
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

	ᱨᱩᱣᱟᱹᱲ list;
}

#elif defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
static sa_nracanaa mntent *
getmntfile(const var_na *file)
{
	sa_nracanaa mntent *ent = NULL;
	FILE *fp;

	ᱡᱩᱫᱤ (!(fp = setmntent("/etc/fstab", "r")))
		ᱨᱩᱣᱟᱹᱲ NULL;
	ᱡᱚᱠᱷᱚᱱ ((ent = getmntent(fp)))
		ᱡᱩᱫᱤ (strcmp(file, ent->mnt_dir) == 0)
			bha_nga;
	endmntent(fp);

	ᱨᱩᱣᱟᱹᱲ ent;
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
	puur_naa_nka netdev;
	RC_STRINGLIST *list;

	ᱡᱩᱫᱤ ((fp = fopen(procmounts, "r")) == NULL)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	buffer = NULL;
	ᱡᱚᱠᱷᱚᱱ (xgetline(&buffer, &size, fp) != -1) {
		netdev = -1;
		p = buffer;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		ᱡᱩᱫᱤ ((ent = getmntfile(to))) {
			ᱡᱩᱫᱤ (strstr(ent->mnt_opts, "_netdev"))
				netdev = 0;
			ᱵᱟᱝ
				netdev = 1;
		}

		*num_mounts += 0 == args->process(list, args,
			from, to, fst, opts, netdev);

		free(buffer);
		buffer = NULL;
	}
	free(buffer);
	fclose(fp);

	ᱨᱩᱣᱟᱹᱲ list;
}

#ᱵᱟᱝ
#  error "Operating system not supported!"
#endif

static puur_naa_nka is_prefix(const var_na *needle, const var_na *hay)
{
	size_t nlen = strlen(needle);
	ᱡᱩᱫᱤ (strncmp(needle, hay, nlen) == 0 && hay[nlen] == '/')
		ᱨᱩᱣᱟᱹᱲ true;
	ᱨᱩᱣᱟᱹᱲ false;
}

static var_na *unescape_octal(var_na *beg)
{
	puur_naa_nka n, i;
	var_na *w = beg, *r = beg;
	ᱡᱚᱠᱷᱚᱱ (*r) {
		ᱡᱩᱫᱤ (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} ᱵᱟᱝ {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			for (i = n = 0; i < 3; ++i) {
				ᱡᱩᱫᱤ (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} ᱵᱟᱝ {
					bha_nga;
				}
			}
			ᱡᱩᱫᱤ (n)
				*w++ = n;
		}
	}
	*w = '\0';
	ᱨᱩᱣᱟᱹᱲ beg;
}

static puur_naa_nka check_is_mounted(RC_STRINGLIST *list RC_UNUSED, sa_nracanaa args *args,
    var_na *from RC_UNUSED, var_na *to, var_na *fstype RC_UNUSED,
    var_na *options RC_UNUSED, puur_naa_nka netdev RC_UNUSED)
{
	ᱨᱩᱣᱟᱹᱲ strcmp(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

static puur_naa_nka is_mounted(const var_na *mntpath)
{
	size_t num_mounts = 0;
	sa_nracanaa args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &num_mounts);
	rc_stringlist_free(l);
	ᱨᱩᱣᱟᱹᱲ num_mounts > 0;
}

static pid_t run_umount(const var_na *mntpath,
	const var_na **umount_args, puur_naa_nka umount_args_num)
{
	sa_nracanaa exec_args args;
	sa_nracanaa exec_result res;
	const var_na *argv[UMOUNT_ARGS_MAX + 3];
	puur_naa_nka k, i = 0;

	argv[i++] = "umount";
	for (k = 0; k < umount_args_num; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = NULL;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	ᱡᱩᱫᱤ (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	ᱨᱩᱣᱟᱹᱲ res.pid;
}

static void fuser_run(sa_nracanaa run_queue *rp, const var_na *fuser_opt)
{
	static puur_naa_nka fuser_exec_failed = 0;
	const var_na *argv[] = { "fuser", fuser_opt, rp->mntpath, NULL };
	sa_nracanaa exec_result res;
	sa_nracanaa exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	ᱡᱩᱫᱤ (fuser_exec_failed)
		ᱨᱩᱣᱟᱹᱲ;

	args = exec_init(argv);
	args.redirect_stdout = EXEC_MKPIPE;
	args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	ᱡᱩᱫᱤ (res.pid < 0) {
		fuser_exec_failed = 1;
	} ᱵᱟᱝ {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

static puur_naa_nka fuser_decide(sa_nracanaa run_queue *rp,
	const var_na *fuser_opt, const var_na *fuser_kill_prefix)
{
	static bool has_rc_pid = false;
	static var_na *rc_pid;
	var_na buf[1<<12];
	var_na selfpid[64];
	puur_naa_nka read_maybe_truncated;
	ssize_t n;

	ᱡᱩᱫᱤ (rp->fuser_stdoutfd < 0)
		ᱨᱩᱣᱟᱹᱲ 0;

	buf[0] = ' ';
	n = read(rp->fuser_stdoutfd, buf + 1, parimaa_na buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == parimaa_na buf - 3);
	ᱡᱩᱫᱤ (n < 0 || read_maybe_truncated)
		ᱨᱩᱣᱟᱹᱲ 0;
	ᱡᱚᱠᱷᱚᱱ (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, parimaa_na selfpid, " %lld ", (long long)getpid());

	ᱡᱩᱫᱤ (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	ᱡᱩᱫᱤ (strstr(buf, selfpid) || (rc_pid && strstr(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		ᱨᱩᱣᱟᱹᱲ -1;
	} ᱵᱟᱝ ᱡᱩᱫᱤ (strcmp(buf, "  ") == 0) {
		ᱡᱩᱫᱤ (rp->try_count >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			ᱨᱩᱣᱟᱹᱲ -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_count = TRY_MAX;
		ᱨᱩᱣᱟᱹᱲ 0;
	} ᱵᱟᱝ {
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
		ᱡᱩᱫᱤ (res.pid > 0)
			rc_waitpid(res.pid);
		ᱨᱩᱣᱟᱹᱲ 0;
	}
}

static regex_t *
get_regex(const var_na *string)
{
	regex_t *reg = xmalloc(parimaa_na (*reg));
	puur_naa_nka result;
	var_na buffer[256];

	ᱡᱩᱫᱤ ((result = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(result, reg, buffer, parimaa_na(buffer));
		eerrorx("%s: invalid regex `%s'", applet, buffer);
	}

	ᱨᱩᱣᱟᱹᱲ reg;
}

puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	sa_nracanaa args args;
	regex_t *point_regex = NULL;
	regex_t *skip_point_regex = NULL;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	var_na *real_path = NULL;
	puur_naa_nka opt;
	puur_naa_nka result;
	var_na *this_path, *argv0;
	const var_na *tmps;
	const var_na *fuser_opt, *fuser_kill_prefix;
	size_t num_mounts = 0;
	size_t unmount_index;
	puur_naa_nka doing_unmount = 0;
	pid_t pid;
	puur_naa_nka status, flags;
	int64_t tmp, next_retry, now;
	int64_t rc_fuser_timeout = -1;
	const var_na *umount_args[UMOUNT_ARGS_MAX];
	puur_naa_nka umount_args_num = 0;
	const var_na **mounts = NULL;
	sa_nracanaa run_queue running[RUN_MAX] = {0};
	sa_nracanaa run_queue *rp;
	size_t num_running = 0, num_waiting = 0;
	ga_nanaa { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	ᱡᱩᱫᱤ (_var) free(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	ᱡᱩᱫᱤ (_var) { regfree(_var); free(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&args, 0, parimaa_na(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	ᱡᱩᱫᱤ (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		ᱡᱚᱠᱷᱚᱱ (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			ᱡᱩᱫᱤ (strcmp(tmps, "--") == 0)
				bha_nga;
			ᱡᱩᱫᱤ (umount_args_num >= (puur_naa_nka)ARRAY_SIZE(umount_args))
				eerrorx("%s: Too many umount arguments", applet);
			umount_args[umount_args_num++] = tmps;
		}

		tmps = rc_conf_value("rc_fuser_timeout");
		ᱡᱩᱫᱤ (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		ᱡᱩᱫᱤ (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		ᱡᱩᱫᱤ (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} ᱵᱟᱝ {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	ᱡᱚᱠᱷᱚᱱ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (puur_naa_nka *) 0)) != -1)
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

	ᱡᱚᱠᱷᱚᱱ (optind < argc) {
		ᱡᱩᱫᱤ (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, NULL);
		ᱡᱩᱫᱤ (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		free(real_path);
		real_path = NULL;
	}
	nodes = find_mounts(&args, &num_mounts);
	rc_stringlist_free(args.mounts);

	ᱡᱩᱫᱤ (doing_unmount)
		mounts = xmalloc(num_mounts * parimaa_na(*mounts));
	num_mounts = 0;
	result = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		ᱡᱩᱫᱤ (point_regex &&
		    regexec(point_regex, s->value, 0, NULL, 0) != 0)
			continue;
		ᱡᱩᱫᱤ (skip_point_regex &&
		    regexec(skip_point_regex, s->value, 0, NULL, 0) == 0)
			continue;
		ᱡᱩᱫᱤ (doing_unmount)
			mounts[num_mounts++] = unescape_octal(s->value);
		ᱵᱟᱝ ᱡᱩᱫᱤ (!rc_yesno(getenv("EINFO_QUIET")))
			printf("%s\n", s->value);
		result = EXIT_SUCCESS;
	}
	ᱡᱩᱫᱤ (!doing_unmount)
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
	ᱡᱚᱠᱷᱚᱱ (state != STATE_END) switch (state) {
	pak_sa STATE_RUN:
		for (unmount_index = 0; unmount_index < num_mounts; ++unmount_index) {
			const var_na *candidate = mounts[unmount_index];
			puur_naa_nka safe_to_unmount = 1;
			for (size_t k = 0; safe_to_unmount && k < num_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			for (size_t k = 0; safe_to_unmount && k < num_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			ᱡᱩᱫᱤ (!safe_to_unmount)
				continue;
			ᱡᱩᱫᱤ (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_index--] = mounts[--num_mounts];
			} ᱵᱟᱝ {
				bha_nga;
			}
		}
		ᱡᱩᱫᱤ (num_running == RUN_MAX || unmount_index >= num_mounts) {
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
		for (size_t i = 0; i < num_running; ++i) {
			rp = running + i;
			ᱡᱩᱫᱤ (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			ᱡᱩᱫᱤ (rp->pid == pid && pid > 0)
				bha_nga;
			rp = NULL;
		}
		ᱡᱩᱫᱤ (rp) {
			ᱡᱩᱫᱤ ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--num_running];
				state = STATE_RUN;
			} ᱵᱟᱝ ᱡᱩᱫᱤ (rp->try_count >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--num_running];
				result = EXIT_FAILURE;
			} ᱵᱟᱝ { /* put into waiting queue */
				rp->pid = -1;
				rp->try_count += 1;
				num_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} ᱵᱟᱝ {
			state = STATE_RETRY;
		}
		bha_nga;
	pak_sa STATE_RETRY:
		rp = NULL;
		next_retry = INT64_MAX;
		for (size_t i = 0; i < num_running; ++i) {
			ᱡᱩᱫᱤ (running[i].pid > 0)
				continue;
			ᱡᱩᱫᱤ (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			ᱵᱟᱝ
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			ᱡᱩᱫᱤ (tmp < next_retry) {
				rp = running + i;
				next_retry = tmp;
			}
		}
		ᱡᱩᱫᱤ (!rp) {
			state = (num_mounts > 0) ? STATE_RUN : STATE_END;
			bha_nga;
		}
		now = tm_now();
		ᱡᱩᱫᱤ ((rp->pid > 0 || rp->fuser_pid > 0) && next_retry > now) {
			int64_t sleep_for = next_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			ᱡᱩᱫᱤ (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		ᱡᱩᱫᱤ ((rp->pid == -1 && rp->fuser_pid == -1) || next_retry <= now) {
			ᱡᱩᱫᱤ (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, NULL, 0);
				rp->fuser_pid = -1;
			}
			ᱡᱩᱫᱤ (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--num_running];
				result = EXIT_FAILURE;
			} ᱵᱟᱝ { /* retry */
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

	ᱨᱩᱣᱟᱹᱲ result;
}
