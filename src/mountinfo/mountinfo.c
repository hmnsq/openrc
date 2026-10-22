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

#જો defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#અથવાજો defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#અથવાજો defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#અથવાજો defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#endif

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <regex.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "rc_exec.h"
#અન્તર્ભાવ "timeutils.h"
#અન્તર્ભાવ "_usage.h"
#અન્તર્ભાવ "helpers.h"

અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *procmounts = "/proc/mounts";
અચલ અક્ષર *extraopts = "[mount1] [mount2] ...";
અચલ અક્ષર getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{ "fstype-regex",        1, ખાલી, 'f'},
	{ "skip-fstype-regex",   1, ખાલી, 'F'},
	{ "node-regex",          1, ખાલી, 'n'},
	{ "skip-node-regex",     1, ખાલી, 'N'},
	{ "options-regex",       1, ખાલી, 'o'},
	{ "skip-options-regex",  1, ખાલી, 'O'},
	{ "point-regex",         1, ખાલી, 'p'},
	{ "skip-point-regex",    1, ખાલી, 'P'},
	{ "options",             0, ખાલી, 'i'},
	{ "fstype",              0, ખાલી, 's'},
	{ "node",                0, ખાલી, 't'},
	{ "netdev",              0, ખાલી, 'e'},
	{ "nonetdev",            0, ખાલી, 'E'},
	longopts_COMMON
};
અચલ અક્ષર * અચલ longopts_help[] = {
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
અચલ અક્ષર *usagestring = ખાલી;

#define UMOUNT_ARGS_MAX 16
#define RUN_MAX 32
#define TRY_MAX 3
#define TRY_DELAY_MS 1000

સંરચના run_queue {
	અચલ અક્ષર *mntpath;
	int64_t last_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	પૂર્ણાંક try_ગણતરી;
	પૂર્ણાંક fuser_stdoutfd;
};

પ્રકારવ્યાખ્યા ગણના {
	mount_from,
	mount_to,
	mount_fstype,
	mount_options
} mount_type;

પ્રકારવ્યાખ્યા ગણના {
	net_ignore,
	net_yes,
	net_no
} net_opts;

સંરચના args;
પ્રકારવ્યાખ્યા પૂર્ણાંક process_func_t(RC_STRINGLIST *, સંરચના args *,
    અક્ષર *, અક્ષર *, અક્ષર *, અક્ષર *, પૂર્ણાંક);

સંરચના args {
	process_func_t *process;
	regex_t *ગાંઠ_regex;
	regex_t *skip_ગાંઠ_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	અચલ અક્ષર *check_mntpath;
};

સ્થિર પૂર્ણાંક
process_mount(RC_STRINGLIST *list, સંરચના args *args,
    અક્ષર *from, અક્ષર *to, અક્ષર *fstype, અક્ષર *options,
    પૂર્ણાંક netdev)
{
	અક્ષર *p;
	RC_STRING *s;

	errno = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	જો (strcmp(fstype, "rootfs") == 0)
		પાછા -1;
#endif

	જો (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		જો (netdev != 0)
			પાછા 1;
	} નહીંતર જો (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		જો (netdev != 1)
			પાછા 1;
	} નહીંતર {
		જો (args->ગાંઠ_regex &&
		    regexec(args->ગાંઠ_regex, from, 0, ખાલી, 0) != 0)
			પાછા 1;
		જો (args->skip_ગાંઠ_regex &&
		    regexec(args->skip_ગાંઠ_regex, from, 0, ખાલી, 0) == 0)
			પાછા 1;

		જો (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, ખાલી, 0) != 0)
			પાછા -1;
		જો (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, ખાલી, 0) == 0)
			પાછા -1;

		જો (args->options_regex &&
		    regexec(args->options_regex, options, 0, ખાલી, 0) != 0)
			પાછા -1;
		જો (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, ખાલી, 0) == 0)
			પાછા -1;
	}

	જો (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    જો (strcmp(s->કિંમત, to) == 0)
			    તોડો;
		જો (!s)
			પાછા -1;
	}

	પસંદગી (args->mount_type) {
	સ્થિતિ mount_from:
		p = from;
		તોડો;
	સ્થિતિ mount_to:
		p = to;
		તોડો;
	સ્થિતિ mount_fstype:
		p = fstype;
		તોડો;
	સ્થિતિ mount_options:
		p = options;
		તોડો;
	મૂળભૂત:
		p = ખાલી;
		errno = EINVAL;
		તોડો;
	}

	જો (p) {
		errno = 0;
		rc_stringlist_add(list, p);
		પાછા 0;
	}

	પાછા -1;
}

#જો defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
સ્થિર સંરચના opt {
	પૂર્ણાંક o_opt;
	અચલ અક્ષર *o_name;
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
	{ 0, ખાલી }
};

સ્થિર RC_STRINGLIST *
find_mounts(સંરચના args *args, size_t *અંક_mounts)
{
	સંરચના statfs *mnts;
	પૂર્ણાંક nmnts;
	પૂર્ણાંક i;
	RC_STRINGLIST *list;
	અક્ષર *options = ખાલી;
	uint64_t flags;
	સંરચના opt *o;
	પૂર્ણાંક netdev;
	અક્ષર *tmp;

	જો ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	માટે (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		માટે (o = optnames; flags && o->o_opt; o++) {
			જો (flags & o->o_opt) {
				જો (o->o_opt == MNT_LOCAL)
					netdev = 1;
				જો (!options)
					options = xstrdup(o->o_name);
				નહીંતર {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					મુક્ત(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*અંક_mounts += 0 == args->process(list, args,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		મુક્ત(options);
		options = ખાલી;
	}

	પાછા list;
}

#અથવાજો defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
સ્થિર સંરચના mntent *
getmntfile(અચલ અક્ષર *file)
{
	સંરચના mntent *ent = ખાલી;
	FILE *fp;

	જો (!(fp = setmntent("/etc/fstab", "r")))
		પાછા ખાલી;
	જ્યારે ((ent = getmntent(fp)))
		જો (strcmp(file, ent->mnt_dir) == 0)
			તોડો;
	endmntent(fp);

	પાછા ent;
}

સ્થિર RC_STRINGLIST *
find_mounts(સંરચના args *args, size_t *અંક_mounts)
{
	FILE *fp;
	અક્ષર *buffer;
	size_t size;
	અક્ષર *p;
	અક્ષર *from;
	અક્ષર *to;
	અક્ષર *fst;
	અક્ષર *opts;
	સંરચના mntent *ent;
	પૂર્ણાંક netdev;
	RC_STRINGLIST *list;

	જો ((fp = fopen(procmounts, "r")) == ખાલી)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	buffer = ખાલી;
	જ્યારે (xgetline(&buffer, &size, fp) != -1) {
		netdev = -1;
		p = buffer;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		જો ((ent = getmntfile(to))) {
			જો (strstr(ent->mnt_opts, "_netdev"))
				netdev = 0;
			નહીંતર
				netdev = 1;
		}

		*અંક_mounts += 0 == args->process(list, args,
			from, to, fst, opts, netdev);

		મુક્ત(buffer);
		buffer = ખાલી;
	}
	મુક્ત(buffer);
	fclose(fp);

	પાછા list;
}

#નહીંતર
#  error "Operating system not supported!"
#endif

સ્થિર પૂર્ણાંક is_prefix(અચલ અક્ષર *needle, અચલ અક્ષર *hay)
{
	size_t nlen = strlen(needle);
	જો (strncmp(needle, hay, nlen) == 0 && hay[nlen] == '/')
		પાછા true;
	પાછા false;
}

સ્થિર અક્ષર *unescape_octal(અક્ષર *beg)
{
	પૂર્ણાંક n, i;
	અક્ષર *w = beg, *r = beg;
	જ્યારે (*r) {
		જો (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} નહીંતર {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			માટે (i = n = 0; i < 3; ++i) {
				જો (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} નહીંતર {
					તોડો;
				}
			}
			જો (n)
				*w++ = n;
		}
	}
	*w = '\0';
	પાછા beg;
}

સ્થિર પૂર્ણાંક check_is_mounted(RC_STRINGLIST *list RC_UNUSED, સંરચના args *args,
    અક્ષર *from RC_UNUSED, અક્ષર *to, અક્ષર *fstype RC_UNUSED,
    અક્ષર *options RC_UNUSED, પૂર્ણાંક netdev RC_UNUSED)
{
	પાછા strcmp(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

સ્થિર પૂર્ણાંક is_mounted(અચલ અક્ષર *mntpath)
{
	size_t અંક_mounts = 0;
	સંરચના args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &અંક_mounts);
	rc_stringlist_free(l);
	પાછા અંક_mounts > 0;
}

સ્થિર pid_t run_umount(અચલ અક્ષર *mntpath,
	અચલ અક્ષર **umount_args, પૂર્ણાંક umount_args_અંક)
{
	સંરચના exec_args args;
	સંરચના exec_result res;
	અચલ અક્ષર *argv[UMOUNT_ARGS_MAX + 3];
	પૂર્ણાંક k, i = 0;

	argv[i++] = "umount";
	માટે (k = 0; k < umount_args_અંક; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = ખાલી;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	જો (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	પાછા res.pid;
}

સ્થિર શૂન્ય fuser_run(સંરચના run_queue *rp, અચલ અક્ષર *fuser_opt)
{
	સ્થિર પૂર્ણાંક fuser_exec_failed = 0;
	અચલ અક્ષર *argv[] = { "fuser", fuser_opt, rp->mntpath, ખાલી };
	સંરચના exec_result res;
	સંરચના exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	જો (fuser_exec_failed)
		પાછા;

	args = exec_init(argv);
	args.redirect_stdout = EXEC_MKPIPE;
	args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	જો (res.pid < 0) {
		fuser_exec_failed = 1;
	} નહીંતર {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

સ્થિર પૂર્ણાંક fuser_decide(સંરચના run_queue *rp,
	અચલ અક્ષર *fuser_opt, અચલ અક્ષર *fuser_kill_prefix)
{
	સ્થિર bool has_rc_pid = false;
	સ્થિર અક્ષર *rc_pid;
	અક્ષર buf[1<<12];
	અક્ષર selfpid[64];
	પૂર્ણાંક read_maybe_truncated;
	ssize_t n;

	જો (rp->fuser_stdoutfd < 0)
		પાછા 0;

	buf[0] = ' ';
	n = પઠ(rp->fuser_stdoutfd, buf + 1, કદ buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == કદ buf - 3);
	જો (n < 0 || read_maybe_truncated)
		પાછા 0;
	જ્યારે (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, કદ selfpid, " %lld ", (long long)getpid());

	જો (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	જો (strstr(buf, selfpid) || (rc_pid && strstr(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		પાછા -1;
	} નહીંતર જો (strcmp(buf, "  ") == 0) {
		જો (rp->try_ગણતરી >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			પાછા -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_ગણતરી = TRY_MAX;
		પાછા 0;
	} નહીંતર {
		અક્ષર sig[32];
		અચલ અક્ષર *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, ખાલી
		};
		સંરચના exec_result res;
		સંરચના exec_args args = exec_init(argv);
		args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, કદ sig, "%s%s", fuser_kill_prefix,
			rp->try_ગણતરી == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&args);
		જો (res.pid > 0)
			rc_waitpid(res.pid);
		પાછા 0;
	}
}

સ્થિર regex_t *
get_regex(અચલ અક્ષર *string)
{
	regex_t *reg = xmalloc(કદ (*reg));
	પૂર્ણાંક result;
	અક્ષર buffer[256];

	જો ((result = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(result, reg, buffer, કદ(buffer));
		eerrorx("%s: invalid regex `%s'", applet, buffer);
	}

	પાછા reg;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	સંરચના args args;
	regex_t *point_regex = ખાલી;
	regex_t *skip_point_regex = ખાલી;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	અક્ષર *real_path = ખાલી;
	પૂર્ણાંક opt;
	પૂર્ણાંક result;
	અક્ષર *this_path, *argv0;
	અચલ અક્ષર *tmps;
	અચલ અક્ષર *fuser_opt, *fuser_kill_prefix;
	size_t અંક_mounts = 0;
	size_t unmount_index;
	પૂર્ણાંક doing_unmount = 0;
	pid_t pid;
	પૂર્ણાંક status, flags;
	int64_t tmp, next_retry, now;
	int64_t rc_fuser_timeout = -1;
	અચલ અક્ષર *umount_args[UMOUNT_ARGS_MAX];
	પૂર્ણાંક umount_args_અંક = 0;
	અચલ અક્ષર **mounts = ખાલી;
	સંરચના run_queue running[RUN_MAX] = {0};
	સંરચના run_queue *rp;
	size_t અંક_running = 0, અંક_waiting = 0;
	ગણના { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	જો (_var) મુક્ત(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	જો (_var) { regfree(_var); મુક્ત(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&args, 0, કદ(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	જો (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		જ્યારે (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			જો (strcmp(tmps, "--") == 0)
				તોડો;
			જો (umount_args_અંક >= (પૂર્ણાંક)ARRAY_SIZE(umount_args))
				eerrorx("%s: Too many umount arguments", applet);
			umount_args[umount_args_અંક++] = tmps;
		}

		tmps = rc_conf_કિંમત("rc_fuser_timeout");
		જો (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		જો (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		જો (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} નહીંતર {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	જ્યારે ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (પૂર્ણાંક *) 0)) != -1)
	{
		પસંદગી (opt) {
		સ્થિતિ 'e':
			args.netdev = net_yes;
			તોડો;
		સ્થિતિ 'E':
			args.netdev = net_no;
			તોડો;
		સ્થિતિ 'f':
			DO_REG(args.fstype_regex);
			તોડો;
		સ્થિતિ 'F':
			DO_REG(args.skip_fstype_regex);
			તોડો;
		સ્થિતિ 'n':
			DO_REG(args.ગાંઠ_regex);
			તોડો;
		સ્થિતિ 'N':
			DO_REG(args.skip_ગાંઠ_regex);
			તોડો;
		સ્થિતિ 'o':
			DO_REG(args.options_regex);
			તોડો;
		સ્થિતિ 'O':
			DO_REG(args.skip_options_regex);
			તોડો;
		સ્થિતિ 'p':
			DO_REG(point_regex);
			તોડો;
		સ્થિતિ 'P':
			DO_REG(skip_point_regex);
			તોડો;
		સ્થિતિ 'i':
			args.mount_type = mount_options;
			તોડો;
		સ્થિતિ 's':
			args.mount_type = mount_fstype;
			તોડો;
		સ્થિતિ 't':
			args.mount_type = mount_from;
			તોડો;

		case_RC_COMMON_GETOPT
		}
	}

	જ્યારે (optind < argc) {
		જો (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, ખાલી);
		જો (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		મુક્ત(real_path);
		real_path = ખાલી;
	}
	nodes = find_mounts(&args, &અંક_mounts);
	rc_stringlist_free(args.mounts);

	જો (doing_unmount)
		mounts = xmalloc(અંક_mounts * કદ(*mounts));
	અંક_mounts = 0;
	result = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		જો (point_regex &&
		    regexec(point_regex, s->કિંમત, 0, ખાલી, 0) != 0)
			ચાલુ;
		જો (skip_point_regex &&
		    regexec(skip_point_regex, s->કિંમત, 0, ખાલી, 0) == 0)
			ચાલુ;
		જો (doing_unmount)
			mounts[અંક_mounts++] = unescape_octal(s->કિંમત);
		નહીંતર જો (!rc_yesno(getenv("EINFO_QUIET")))
			printf("%s\n", s->કિંમત);
		result = EXIT_SUCCESS;
	}
	જો (!doing_unmount)
		જાઓ exit;

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
	જ્યારે (state != STATE_END) પસંદગી (state) {
	સ્થિતિ STATE_RUN:
		માટે (unmount_index = 0; unmount_index < અંક_mounts; ++unmount_index) {
			અચલ અક્ષર *candidate = mounts[unmount_index];
			પૂર્ણાંક safe_to_unmount = 1;
			માટે (size_t k = 0; safe_to_unmount && k < અંક_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			માટે (size_t k = 0; safe_to_unmount && k < અંક_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			જો (!safe_to_unmount)
				ચાલુ;
			જો (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_index--] = mounts[--અંક_mounts];
			} નહીંતર {
				તોડો;
			}
		}
		જો (અંક_running == RUN_MAX || unmount_index >= અંક_mounts) {
			state = STATE_REAP;
			તોડો;
		}
		rp = running + અંક_running++;
		rp->mntpath = mounts[unmount_index];
		rp->last_exec_time = tm_now();
		rp->try_ગણતરી = 0;
		rp->pid = run_umount(rp->mntpath, umount_args, umount_args_અંક);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_index] = mounts[--અંક_mounts];
		તોડો;
	સ્થિતિ STATE_REAP:
		flags = (અંક_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = ખાલી;
		માટે (size_t i = 0; i < અંક_running; ++i) {
			rp = running + i;
			જો (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			જો (rp->pid == pid && pid > 0)
				તોડો;
			rp = ખાલી;
		}
		જો (rp) {
			જો ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--અંક_running];
				state = STATE_RUN;
			} નહીંતર જો (rp->try_ગણતરી >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--અંક_running];
				result = EXIT_FAILURE;
			} નહીંતર { /* put into waiting queue */
				rp->pid = -1;
				rp->try_ગણતરી += 1;
				અંક_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} નહીંતર {
			state = STATE_RETRY;
		}
		તોડો;
	સ્થિતિ STATE_RETRY:
		rp = ખાલી;
		next_retry = INT64_MAX;
		માટે (size_t i = 0; i < અંક_running; ++i) {
			જો (running[i].pid > 0)
				ચાલુ;
			જો (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			નહીંતર
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			જો (tmp < next_retry) {
				rp = running + i;
				next_retry = tmp;
			}
		}
		જો (!rp) {
			state = (અંક_mounts > 0) ? STATE_RUN : STATE_END;
			તોડો;
		}
		now = tm_now();
		જો ((rp->pid > 0 || rp->fuser_pid > 0) && next_retry > now) {
			int64_t sleep_for = next_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			જો (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		જો ((rp->pid == -1 && rp->fuser_pid == -1) || next_retry <= now) {
			જો (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, ખાલી, 0);
				rp->fuser_pid = -1;
			}
			જો (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--અંક_running];
				result = EXIT_FAILURE;
			} નહીંતર { /* retry */
				rp->last_exec_time = tm_now();
				rp->pid = run_umount(rp->mntpath,
					umount_args, umount_args_અંક);
			}
			અંક_waiting -= 1;
			state = STATE_REAP;
		}
		તોડો;
	મૂળભૂત: તોડો;
	}

exit:
	મુક્ત(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(args.fstype_regex);
	REG_FREE(args.skip_fstype_regex);
	REG_FREE(args.ગાંઠ_regex);
	REG_FREE(args.skip_ગાંઠ_regex);
	REG_FREE(args.options_regex);
	REG_FREE(args.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	પાછા result;
}
