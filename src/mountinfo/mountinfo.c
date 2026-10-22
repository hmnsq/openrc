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

#যদি defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#নতুবা_যদি defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#নতুবা_যদি defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#নতুবা_যদি defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#endif

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <regex.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "rc_exec.h"
#অন্তর্ভাব "timeutils.h"
#অন্তর্ভাব "_usage.h"
#অন্তর্ভাব "helpers.h"

ধ্রুবক অক্ষর *applet = ফাঁকা;
ধ্রুবক অক্ষর *procmounts = "/proc/mounts";
ধ্রুবক অক্ষর *extraopts = "[mount1] [mount2] ...";
ধ্রুবক অক্ষর getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
ধ্রুবক কাঠামো option longopts[] = {
	{ "fstype-regex",        1, ফাঁকা, 'f'},
	{ "skip-fstype-regex",   1, ফাঁকা, 'F'},
	{ "node-regex",          1, ফাঁকা, 'n'},
	{ "skip-node-regex",     1, ফাঁকা, 'N'},
	{ "options-regex",       1, ফাঁকা, 'o'},
	{ "skip-options-regex",  1, ফাঁকা, 'O'},
	{ "point-regex",         1, ফাঁকা, 'p'},
	{ "skip-point-regex",    1, ফাঁকা, 'P'},
	{ "options",             0, ফাঁকা, 'i'},
	{ "fstype",              0, ফাঁকা, 's'},
	{ "node",                0, ফাঁকা, 't'},
	{ "netdev",              0, ফাঁকা, 'e'},
	{ "nonetdev",            0, ফাঁকা, 'E'},
	longopts_COMMON
};
ধ্রুবক অক্ষর * ধ্রুবক longopts_help[] = {
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
ধ্রুবক অক্ষর *usagestring = ফাঁকা;

#define UMOUNT_ARGS_MAX 16
#define RUN_MAX 32
#define TRY_MAX 3
#define TRY_DELAY_MS 1000

কাঠামো run_queue {
	ধ্রুবক অক্ষর *mntpath;
	int64_t last_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	পূর্ণসংখ্যা try_গুনতি;
	পূর্ণসংখ্যা fuser_stdoutfd;
};

প্রকারলেখ গণনা {
	mount_from,
	mount_to,
	mount_fstype,
	mount_options
} mount_type;

প্রকারলেখ গণনা {
	net_ignore,
	net_yes,
	net_no
} net_opts;

কাঠামো args;
প্রকারলেখ পূর্ণসংখ্যা process_func_t(RC_STRINGLIST *, কাঠামো args *,
    অক্ষর *, অক্ষর *, অক্ষর *, অক্ষর *, পূর্ণসংখ্যা);

কাঠামো args {
	process_func_t *process;
	regex_t *গ্রন্থি_regex;
	regex_t *skip_গ্রন্থি_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	ধ্রুবক অক্ষর *check_mntpath;
};

স্থির পূর্ণসংখ্যা
process_mount(RC_STRINGLIST *list, কাঠামো args *args,
    অক্ষর *from, অক্ষর *to, অক্ষর *fstype, অক্ষর *options,
    পূর্ণসংখ্যা netdev)
{
	অক্ষর *p;
	RC_STRING *s;

	errno = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	যদি (strcmp(fstype, "rootfs") == 0)
		ফেরত -1;
#endif

	যদি (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		যদি (netdev != 0)
			ফেরত 1;
	} অন্যথা যদি (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		যদি (netdev != 1)
			ফেরত 1;
	} অন্যথা {
		যদি (args->গ্রন্থি_regex &&
		    regexec(args->গ্রন্থি_regex, from, 0, ফাঁকা, 0) != 0)
			ফেরত 1;
		যদি (args->skip_গ্রন্থি_regex &&
		    regexec(args->skip_গ্রন্থি_regex, from, 0, ফাঁকা, 0) == 0)
			ফেরত 1;

		যদি (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, ফাঁকা, 0) != 0)
			ফেরত -1;
		যদি (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, ফাঁকা, 0) == 0)
			ফেরত -1;

		যদি (args->options_regex &&
		    regexec(args->options_regex, options, 0, ফাঁকা, 0) != 0)
			ফেরত -1;
		যদি (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, ফাঁকা, 0) == 0)
			ফেরত -1;
	}

	যদি (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    যদি (strcmp(s->মান, to) == 0)
			    বিরতি;
		যদি (!s)
			ফেরত -1;
	}

	নির্বাচন (args->mount_type) {
	ক্ষেত্র mount_from:
		p = from;
		বিরতি;
	ক্ষেত্র mount_to:
		p = to;
		বিরতি;
	ক্ষেত্র mount_fstype:
		p = fstype;
		বিরতি;
	ক্ষেত্র mount_options:
		p = options;
		বিরতি;
	পূর্বনির্ধারিত:
		p = ফাঁকা;
		errno = EINVAL;
		বিরতি;
	}

	যদি (p) {
		errno = 0;
		rc_stringlist_add(list, p);
		ফেরত 0;
	}

	ফেরত -1;
}

#যদি defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
স্থির কাঠামো opt {
	পূর্ণসংখ্যা o_opt;
	ধ্রুবক অক্ষর *o_name;
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
	{ 0, ফাঁকা }
};

স্থির RC_STRINGLIST *
find_mounts(কাঠামো args *args, size_t *অঙ্ক_mounts)
{
	কাঠামো statfs *mnts;
	পূর্ণসংখ্যা nmnts;
	পূর্ণসংখ্যা i;
	RC_STRINGLIST *list;
	অক্ষর *options = ফাঁকা;
	uint64_t flags;
	কাঠামো opt *o;
	পূর্ণসংখ্যা netdev;
	অক্ষর *tmp;

	যদি ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	জন্য (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		জন্য (o = optnames; flags && o->o_opt; o++) {
			যদি (flags & o->o_opt) {
				যদি (o->o_opt == MNT_LOCAL)
					netdev = 1;
				যদি (!options)
					options = xstrdup(o->o_name);
				অন্যথা {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					মুক্ত(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*অঙ্ক_mounts += 0 == args->process(list, args,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		মুক্ত(options);
		options = ফাঁকা;
	}

	ফেরত list;
}

#নতুবা_যদি defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
স্থির কাঠামো mntent *
getmntfile(ধ্রুবক অক্ষর *file)
{
	কাঠামো mntent *ent = ফাঁকা;
	FILE *fp;

	যদি (!(fp = setmntent("/etc/fstab", "r")))
		ফেরত ফাঁকা;
	যখন ((ent = getmntent(fp)))
		যদি (strcmp(file, ent->mnt_dir) == 0)
			বিরতি;
	endmntent(fp);

	ফেরত ent;
}

স্থির RC_STRINGLIST *
find_mounts(কাঠামো args *args, size_t *অঙ্ক_mounts)
{
	FILE *fp;
	অক্ষর *বাফার;
	size_t আকার_2;
	অক্ষর *p;
	অক্ষর *from;
	অক্ষর *to;
	অক্ষর *fst;
	অক্ষর *opts;
	কাঠামো mntent *ent;
	পূর্ণসংখ্যা netdev;
	RC_STRINGLIST *list;

	যদি ((fp = fopen(procmounts, "r")) == ফাঁকা)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	বাফার = ফাঁকা;
	যখন (xgetline(&বাফার, &আকার_2, fp) != -1) {
		netdev = -1;
		p = বাফার;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		যদি ((ent = getmntfile(to))) {
			যদি (strstr(ent->mnt_opts, "_netdev"))
				netdev = 0;
			অন্যথা
				netdev = 1;
		}

		*অঙ্ক_mounts += 0 == args->process(list, args,
			from, to, fst, opts, netdev);

		মুক্ত(বাফার);
		বাফার = ফাঁকা;
	}
	মুক্ত(বাফার);
	fclose(fp);

	ফেরত list;
}

#অন্যথা
#  error "Operating system not supported!"
#endif

স্থির পূর্ণসংখ্যা is_prefix(ধ্রুবক অক্ষর *needle, ধ্রুবক অক্ষর *hay)
{
	size_t nlen = strlen(needle);
	যদি (strncmp(needle, hay, nlen) == 0 && hay[nlen] == '/')
		ফেরত true;
	ফেরত false;
}

স্থির অক্ষর *unescape_octal(অক্ষর *beg)
{
	পূর্ণসংখ্যা n, i;
	অক্ষর *w = beg, *r = beg;
	যখন (*r) {
		যদি (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} অন্যথা {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			জন্য (i = n = 0; i < 3; ++i) {
				যদি (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} অন্যথা {
					বিরতি;
				}
			}
			যদি (n)
				*w++ = n;
		}
	}
	*w = '\0';
	ফেরত beg;
}

স্থির পূর্ণসংখ্যা check_is_mounted(RC_STRINGLIST *list RC_UNUSED, কাঠামো args *args,
    অক্ষর *from RC_UNUSED, অক্ষর *to, অক্ষর *fstype RC_UNUSED,
    অক্ষর *options RC_UNUSED, পূর্ণসংখ্যা netdev RC_UNUSED)
{
	ফেরত strcmp(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

স্থির পূর্ণসংখ্যা is_mounted(ধ্রুবক অক্ষর *mntpath)
{
	size_t অঙ্ক_mounts = 0;
	কাঠামো args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &অঙ্ক_mounts);
	rc_stringlist_free(l);
	ফেরত অঙ্ক_mounts > 0;
}

স্থির pid_t run_umount(ধ্রুবক অক্ষর *mntpath,
	ধ্রুবক অক্ষর **umount_args, পূর্ণসংখ্যা umount_args_অঙ্ক)
{
	কাঠামো exec_args args;
	কাঠামো exec_ফলাফল res;
	ধ্রুবক অক্ষর *argv[UMOUNT_ARGS_MAX + 3];
	পূর্ণসংখ্যা k, i = 0;

	argv[i++] = "umount";
	জন্য (k = 0; k < umount_args_অঙ্ক; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = ফাঁকা;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	যদি (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	ফেরত res.pid;
}

স্থির শূন্য fuser_run(কাঠামো run_queue *rp, ধ্রুবক অক্ষর *fuser_opt)
{
	স্থির পূর্ণসংখ্যা fuser_exec_failed = 0;
	ধ্রুবক অক্ষর *argv[] = { "fuser", fuser_opt, rp->mntpath, ফাঁকা };
	কাঠামো exec_ফলাফল res;
	কাঠামো exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	যদি (fuser_exec_failed)
		ফেরত;

	args = exec_init(argv);
	args.redirect_stdout = EXEC_MKPIPE;
	args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	যদি (res.pid < 0) {
		fuser_exec_failed = 1;
	} অন্যথা {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

স্থির পূর্ণসংখ্যা fuser_decide(কাঠামো run_queue *rp,
	ধ্রুবক অক্ষর *fuser_opt, ধ্রুবক অক্ষর *fuser_kill_prefix)
{
	স্থির bool has_rc_pid = false;
	স্থির অক্ষর *rc_pid;
	অক্ষর buf[1<<12];
	অক্ষর selfpid[64];
	পূর্ণসংখ্যা read_maybe_truncated;
	ssize_t n;

	যদি (rp->fuser_stdoutfd < 0)
		ফেরত 0;

	buf[0] = ' ';
	n = পঠ(rp->fuser_stdoutfd, buf + 1, আকার buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == আকার buf - 3);
	যদি (n < 0 || read_maybe_truncated)
		ফেরত 0;
	যখন (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, আকার selfpid, " %lld ", (long long)getpid());

	যদি (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	যদি (strstr(buf, selfpid) || (rc_pid && strstr(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		ফেরত -1;
	} অন্যথা যদি (strcmp(buf, "  ") == 0) {
		যদি (rp->try_গুনতি >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			ফেরত -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_গুনতি = TRY_MAX;
		ফেরত 0;
	} অন্যথা {
		অক্ষর sig[32];
		ধ্রুবক অক্ষর *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, ফাঁকা
		};
		কাঠামো exec_ফলাফল res;
		কাঠামো exec_args args = exec_init(argv);
		args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, আকার sig, "%s%s", fuser_kill_prefix,
			rp->try_গুনতি == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&args);
		যদি (res.pid > 0)
			rc_waitpid(res.pid);
		ফেরত 0;
	}
}

স্থির regex_t *
get_regex(ধ্রুবক অক্ষর *string)
{
	regex_t *reg = xmalloc(আকার (*reg));
	পূর্ণসংখ্যা ফলাফল;
	অক্ষর বাফার[256];

	যদি ((ফলাফল = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(ফলাফল, reg, বাফার, আকার(বাফার));
		eerrorx("%s: invalid regex `%s'", applet, বাফার);
	}

	ফেরত reg;
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	কাঠামো args args;
	regex_t *point_regex = ফাঁকা;
	regex_t *skip_point_regex = ফাঁকা;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	অক্ষর *real_path = ফাঁকা;
	পূর্ণসংখ্যা opt;
	পূর্ণসংখ্যা ফলাফল;
	অক্ষর *this_path, *argv0;
	ধ্রুবক অক্ষর *tmps;
	ধ্রুবক অক্ষর *fuser_opt, *fuser_kill_prefix;
	size_t অঙ্ক_mounts = 0;
	size_t unmount_সূচক;
	পূর্ণসংখ্যা doing_unmount = 0;
	pid_t pid;
	পূর্ণসংখ্যা status, flags;
	int64_t tmp, পরবর্তী_retry, now;
	int64_t rc_fuser_timeout = -1;
	ধ্রুবক অক্ষর *umount_args[UMOUNT_ARGS_MAX];
	পূর্ণসংখ্যা umount_args_অঙ্ক = 0;
	ধ্রুবক অক্ষর **mounts = ফাঁকা;
	কাঠামো run_queue running[RUN_MAX] = {0};
	কাঠামো run_queue *rp;
	size_t অঙ্ক_running = 0, অঙ্ক_waiting = 0;
	গণনা { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	যদি (_var) মুক্ত(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	যদি (_var) { regfree(_var); মুক্ত(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&args, 0, আকার(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	যদি (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		যখন (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			যদি (strcmp(tmps, "--") == 0)
				বিরতি;
			যদি (umount_args_অঙ্ক >= (পূর্ণসংখ্যা)ARRAY_আকার(umount_args))
				eerrorx("%s: Too many umount arguments", applet);
			umount_args[umount_args_অঙ্ক++] = tmps;
		}

		tmps = rc_conf_মান("rc_fuser_timeout");
		যদি (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		যদি (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		যদি (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} অন্যথা {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	যখন ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (পূর্ণসংখ্যা *) 0)) != -1)
	{
		নির্বাচন (opt) {
		ক্ষেত্র 'e':
			args.netdev = net_yes;
			বিরতি;
		ক্ষেত্র 'E':
			args.netdev = net_no;
			বিরতি;
		ক্ষেত্র 'f':
			DO_REG(args.fstype_regex);
			বিরতি;
		ক্ষেত্র 'F':
			DO_REG(args.skip_fstype_regex);
			বিরতি;
		ক্ষেত্র 'n':
			DO_REG(args.গ্রন্থি_regex);
			বিরতি;
		ক্ষেত্র 'N':
			DO_REG(args.skip_গ্রন্থি_regex);
			বিরতি;
		ক্ষেত্র 'o':
			DO_REG(args.options_regex);
			বিরতি;
		ক্ষেত্র 'O':
			DO_REG(args.skip_options_regex);
			বিরতি;
		ক্ষেত্র 'p':
			DO_REG(point_regex);
			বিরতি;
		ক্ষেত্র 'P':
			DO_REG(skip_point_regex);
			বিরতি;
		ক্ষেত্র 'i':
			args.mount_type = mount_options;
			বিরতি;
		ক্ষেত্র 's':
			args.mount_type = mount_fstype;
			বিরতি;
		ক্ষেত্র 't':
			args.mount_type = mount_from;
			বিরতি;

		case_RC_COMMON_GETOPT
		}
	}

	যখন (optind < argc) {
		যদি (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, ফাঁকা);
		যদি (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		মুক্ত(real_path);
		real_path = ফাঁকা;
	}
	nodes = find_mounts(&args, &অঙ্ক_mounts);
	rc_stringlist_free(args.mounts);

	যদি (doing_unmount)
		mounts = xmalloc(অঙ্ক_mounts * আকার(*mounts));
	অঙ্ক_mounts = 0;
	ফলাফল = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		যদি (point_regex &&
		    regexec(point_regex, s->মান, 0, ফাঁকা, 0) != 0)
			চালু;
		যদি (skip_point_regex &&
		    regexec(skip_point_regex, s->মান, 0, ফাঁকা, 0) == 0)
			চালু;
		যদি (doing_unmount)
			mounts[অঙ্ক_mounts++] = unescape_octal(s->মান);
		অন্যথা যদি (!rc_yesno(getenv("EINFO_QUIET")))
			printf("%s\n", s->মান);
		ফলাফল = EXIT_SUCCESS;
	}
	যদি (!doing_unmount)
		যাও exit;

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
	ফলাফল = EXIT_SUCCESS;
	state = STATE_RUN;
	যখন (state != STATE_END) নির্বাচন (state) {
	ক্ষেত্র STATE_RUN:
		জন্য (unmount_সূচক = 0; unmount_সূচক < অঙ্ক_mounts; ++unmount_সূচক) {
			ধ্রুবক অক্ষর *candidate = mounts[unmount_সূচক];
			পূর্ণসংখ্যা safe_to_unmount = 1;
			জন্য (size_t k = 0; safe_to_unmount && k < অঙ্ক_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			জন্য (size_t k = 0; safe_to_unmount && k < অঙ্ক_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			যদি (!safe_to_unmount)
				চালু;
			যদি (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_সূচক--] = mounts[--অঙ্ক_mounts];
			} অন্যথা {
				বিরতি;
			}
		}
		যদি (অঙ্ক_running == RUN_MAX || unmount_সূচক >= অঙ্ক_mounts) {
			state = STATE_REAP;
			বিরতি;
		}
		rp = running + অঙ্ক_running++;
		rp->mntpath = mounts[unmount_সূচক];
		rp->last_exec_time = tm_now();
		rp->try_গুনতি = 0;
		rp->pid = run_umount(rp->mntpath, umount_args, umount_args_অঙ্ক);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_সূচক] = mounts[--অঙ্ক_mounts];
		বিরতি;
	ক্ষেত্র STATE_REAP:
		flags = (অঙ্ক_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = ফাঁকা;
		জন্য (size_t i = 0; i < অঙ্ক_running; ++i) {
			rp = running + i;
			যদি (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			যদি (rp->pid == pid && pid > 0)
				বিরতি;
			rp = ফাঁকা;
		}
		যদি (rp) {
			যদি ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--অঙ্ক_running];
				state = STATE_RUN;
			} অন্যথা যদি (rp->try_গুনতি >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--অঙ্ক_running];
				ফলাফল = EXIT_FAILURE;
			} অন্যথা { /* put into waiting queue */
				rp->pid = -1;
				rp->try_গুনতি += 1;
				অঙ্ক_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} অন্যথা {
			state = STATE_RETRY;
		}
		বিরতি;
	ক্ষেত্র STATE_RETRY:
		rp = ফাঁকা;
		পরবর্তী_retry = INT64_MAX;
		জন্য (size_t i = 0; i < অঙ্ক_running; ++i) {
			যদি (running[i].pid > 0)
				চালু;
			যদি (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			অন্যথা
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			যদি (tmp < পরবর্তী_retry) {
				rp = running + i;
				পরবর্তী_retry = tmp;
			}
		}
		যদি (!rp) {
			state = (অঙ্ক_mounts > 0) ? STATE_RUN : STATE_END;
			বিরতি;
		}
		now = tm_now();
		যদি ((rp->pid > 0 || rp->fuser_pid > 0) && পরবর্তী_retry > now) {
			int64_t sleep_for = পরবর্তী_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			যদি (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		যদি ((rp->pid == -1 && rp->fuser_pid == -1) || পরবর্তী_retry <= now) {
			যদি (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, ফাঁকা, 0);
				rp->fuser_pid = -1;
			}
			যদি (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--অঙ্ক_running];
				ফলাফল = EXIT_FAILURE;
			} অন্যথা { /* retry */
				rp->last_exec_time = tm_now();
				rp->pid = run_umount(rp->mntpath,
					umount_args, umount_args_অঙ্ক);
			}
			অঙ্ক_waiting -= 1;
			state = STATE_REAP;
		}
		বিরতি;
	পূর্বনির্ধারিত: বিরতি;
	}

exit:
	মুক্ত(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(args.fstype_regex);
	REG_FREE(args.skip_fstype_regex);
	REG_FREE(args.গ্রন্থি_regex);
	REG_FREE(args.skip_গ্রন্থি_regex);
	REG_FREE(args.options_regex);
	REG_FREE(args.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	ফেরত ফলাফল;
}
