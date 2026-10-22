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

#اگر defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#وگرنهاگر defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#وگرنهاگر defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#وگرنهاگر defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#endif

#شامل <errno.h>
#شامل <getopt.h>
#شامل <regex.h>
#شامل <stddef.h>
#شامل <stdio.h>
#شامل <stdint.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <unistd.h>

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "rc_exec.h"
#شامل "timeutils.h"
#شامل "_usage.h"
#شامل "helpers.h"

ثابت نویسه *applet = پوچ;
ثابت نویسه *procmounts = "/proc/mounts";
ثابت نویسه *extraopts = "[mount1] [mount2] ...";
ثابت نویسه getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{ "fstype-regex",        1, پوچ, 'f'},
	{ "skip-fstype-regex",   1, پوچ, 'F'},
	{ "node-regex",          1, پوچ, 'n'},
	{ "skip-node-regex",     1, پوچ, 'N'},
	{ "options-regex",       1, پوچ, 'o'},
	{ "skip-options-regex",  1, پوچ, 'O'},
	{ "point-regex",         1, پوچ, 'p'},
	{ "skip-point-regex",    1, پوچ, 'P'},
	{ "options",             0, پوچ, 'i'},
	{ "fstype",              0, پوچ, 's'},
	{ "node",                0, پوچ, 't'},
	{ "netdev",              0, پوچ, 'e'},
	{ "nonetdev",            0, پوچ, 'E'},
	longopts_COMMON
};
ثابت نویسه * ثابت longopts_help[] = {
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
ثابت نویسه *usagestring = پوچ;

#define UMOUNT_ARGS_MAX 16
#define RUN_MAX 32
#define TRY_MAX 3
#define TRY_DELAY_MS 1000

ساختار run_queue {
	ثابت نویسه *mntpath;
	int64_t last_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	صحیح try_شمار;
	صحیح fuser_stdoutfd;
};

تعریفنوع شمارش {
	mount_from,
	mount_to,
	mount_fstype,
	mount_options
} mount_type;

تعریفنوع شمارش {
	net_ignore,
	net_yes,
	net_no
} net_opts;

ساختار args;
تعریفنوع صحیح process_func_t(RC_STRINGLIST *, ساختار args *,
    نویسه *, نویسه *, نویسه *, نویسه *, صحیح);

ساختار args {
	process_func_t *process;
	regex_t *گره_regex;
	regex_t *skip_گره_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	ثابت نویسه *check_mntpath;
};

ایستا صحیح
process_mount(RC_STRINGLIST *list, ساختار args *args,
    نویسه *from, نویسه *to, نویسه *fstype, نویسه *options,
    صحیح netdev)
{
	نویسه *p;
	RC_STRING *s;

	errno = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	اگر (strcmp(fstype, "rootfs") == 0)
		بازگشت -1;
#endif

	اگر (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		اگر (netdev != 0)
			بازگشت 1;
	} وگرنه اگر (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		اگر (netdev != 1)
			بازگشت 1;
	} وگرنه {
		اگر (args->گره_regex &&
		    regexec(args->گره_regex, from, 0, پوچ, 0) != 0)
			بازگشت 1;
		اگر (args->skip_گره_regex &&
		    regexec(args->skip_گره_regex, from, 0, پوچ, 0) == 0)
			بازگشت 1;

		اگر (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, پوچ, 0) != 0)
			بازگشت -1;
		اگر (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, پوچ, 0) == 0)
			بازگشت -1;

		اگر (args->options_regex &&
		    regexec(args->options_regex, options, 0, پوچ, 0) != 0)
			بازگشت -1;
		اگر (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, پوچ, 0) == 0)
			بازگشت -1;
	}

	اگر (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    اگر (strcmp(s->مقدار, to) == 0)
			    بشکن;
		اگر (!s)
			بازگشت -1;
	}

	انتخاب (args->mount_type) {
	حالت mount_from:
		p = from;
		بشکن;
	حالت mount_to:
		p = to;
		بشکن;
	حالت mount_fstype:
		p = fstype;
		بشکن;
	حالت mount_options:
		p = options;
		بشکن;
	پیشفرض:
		p = پوچ;
		errno = EINVAL;
		بشکن;
	}

	اگر (p) {
		errno = 0;
		rc_stringlist_add(list, p);
		بازگشت 0;
	}

	بازگشت -1;
}

#اگر defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
ایستا ساختار opt {
	صحیح o_opt;
	ثابت نویسه *o_name;
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
	{ 0, پوچ }
};

ایستا RC_STRINGLIST *
find_mounts(ساختار args *args, size_t *رقم_mounts)
{
	ساختار statfs *mnts;
	صحیح nmnts;
	صحیح i;
	RC_STRINGLIST *list;
	نویسه *options = پوچ;
	uint64_t flags;
	ساختار opt *o;
	صحیح netdev;
	نویسه *tmp;

	اگر ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	برای (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		برای (o = optnames; flags && o->o_opt; o++) {
			اگر (flags & o->o_opt) {
				اگر (o->o_opt == MNT_LOCAL)
					netdev = 1;
				اگر (!options)
					options = xstrdup(o->o_name);
				وگرنه {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					آزاد(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*رقم_mounts += 0 == args->process(list, args,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		آزاد(options);
		options = پوچ;
	}

	بازگشت list;
}

#وگرنهاگر defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
ایستا ساختار mntent *
getmntfile(ثابت نویسه *file)
{
	ساختار mntent *ent = پوچ;
	FILE *fp;

	اگر (!(fp = setmntent("/etc/fstab", "r")))
		بازگشت پوچ;
	وقتی ((ent = getmntent(fp)))
		اگر (strcmp(file, ent->mnt_dir) == 0)
			بشکن;
	endmntent(fp);

	بازگشت ent;
}

ایستا RC_STRINGLIST *
find_mounts(ساختار args *args, size_t *رقم_mounts)
{
	FILE *fp;
	نویسه *buffer;
	size_t size;
	نویسه *p;
	نویسه *from;
	نویسه *to;
	نویسه *fst;
	نویسه *opts;
	ساختار mntent *ent;
	صحیح netdev;
	RC_STRINGLIST *list;

	اگر ((fp = fopen(procmounts, "r")) == پوچ)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	buffer = پوچ;
	وقتی (xgetline(&buffer, &size, fp) != -1) {
		netdev = -1;
		p = buffer;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		اگر ((ent = getmntfile(to))) {
			اگر (strstr(ent->mnt_opts, "_netdev"))
				netdev = 0;
			وگرنه
				netdev = 1;
		}

		*رقم_mounts += 0 == args->process(list, args,
			from, to, fst, opts, netdev);

		آزاد(buffer);
		buffer = پوچ;
	}
	آزاد(buffer);
	fclose(fp);

	بازگشت list;
}

#وگرنه
#  error "Operating system not supported!"
#endif

ایستا صحیح is_prefix(ثابت نویسه *needle, ثابت نویسه *hay)
{
	size_t nlen = strlen(needle);
	اگر (strncmp(needle, hay, nlen) == 0 && hay[nlen] == '/')
		بازگشت true;
	بازگشت false;
}

ایستا نویسه *unescape_octal(نویسه *beg)
{
	صحیح n, i;
	نویسه *w = beg, *r = beg;
	وقتی (*r) {
		اگر (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} وگرنه {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			برای (i = n = 0; i < 3; ++i) {
				اگر (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} وگرنه {
					بشکن;
				}
			}
			اگر (n)
				*w++ = n;
		}
	}
	*w = '\0';
	بازگشت beg;
}

ایستا صحیح check_is_mounted(RC_STRINGLIST *list RC_UNUSED, ساختار args *args,
    نویسه *from RC_UNUSED, نویسه *to, نویسه *fstype RC_UNUSED,
    نویسه *options RC_UNUSED, صحیح netdev RC_UNUSED)
{
	بازگشت strcmp(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

ایستا صحیح is_mounted(ثابت نویسه *mntpath)
{
	size_t رقم_mounts = 0;
	ساختار args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &رقم_mounts);
	rc_stringlist_free(l);
	بازگشت رقم_mounts > 0;
}

ایستا pid_t run_umount(ثابت نویسه *mntpath,
	ثابت نویسه **umount_args, صحیح umount_args_رقم)
{
	ساختار exec_args args;
	ساختار exec_result res;
	ثابت نویسه *argv[UMOUNT_ARGS_MAX + 3];
	صحیح k, i = 0;

	argv[i++] = "umount";
	برای (k = 0; k < umount_args_رقم; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = پوچ;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	اگر (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	بازگشت res.pid;
}

ایستا تهی fuser_run(ساختار run_queue *rp, ثابت نویسه *fuser_opt)
{
	ایستا صحیح fuser_exec_failed = 0;
	ثابت نویسه *argv[] = { "fuser", fuser_opt, rp->mntpath, پوچ };
	ساختار exec_result res;
	ساختار exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	اگر (fuser_exec_failed)
		بازگشت;

	args = exec_init(argv);
	args.redirect_stdout = EXEC_MKPIPE;
	args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	اگر (res.pid < 0) {
		fuser_exec_failed = 1;
	} وگرنه {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

ایستا صحیح fuser_decide(ساختار run_queue *rp,
	ثابت نویسه *fuser_opt, ثابت نویسه *fuser_kill_prefix)
{
	ایستا bool has_rc_pid = false;
	ایستا نویسه *rc_pid;
	نویسه buf[1<<12];
	نویسه selfpid[64];
	صحیح read_maybe_truncated;
	ssize_t n;

	اگر (rp->fuser_stdoutfd < 0)
		بازگشت 0;

	buf[0] = ' ';
	n = اقرأ(rp->fuser_stdoutfd, buf + 1, اندازه buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == اندازه buf - 3);
	اگر (n < 0 || read_maybe_truncated)
		بازگشت 0;
	وقتی (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, اندازه selfpid, " %lld ", (long long)getpid());

	اگر (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	اگر (strstr(buf, selfpid) || (rc_pid && strstr(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		بازگشت -1;
	} وگرنه اگر (strcmp(buf, "  ") == 0) {
		اگر (rp->try_شمار >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			بازگشت -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_شمار = TRY_MAX;
		بازگشت 0;
	} وگرنه {
		نویسه sig[32];
		ثابت نویسه *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, پوچ
		};
		ساختار exec_result res;
		ساختار exec_args args = exec_init(argv);
		args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, اندازه sig, "%s%s", fuser_kill_prefix,
			rp->try_شمار == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&args);
		اگر (res.pid > 0)
			rc_waitpid(res.pid);
		بازگشت 0;
	}
}

ایستا regex_t *
get_regex(ثابت نویسه *string)
{
	regex_t *reg = xmalloc(اندازه (*reg));
	صحیح result;
	نویسه buffer[256];

	اگر ((result = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(result, reg, buffer, اندازه(buffer));
		eerrorx("%s: invalid regex `%s'", applet, buffer);
	}

	بازگشت reg;
}

صحیح اصلی(صحیح argc, نویسه **argv)
{
	ساختار args args;
	regex_t *point_regex = پوچ;
	regex_t *skip_point_regex = پوچ;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	نویسه *real_path = پوچ;
	صحیح opt;
	صحیح result;
	نویسه *this_path, *argv0;
	ثابت نویسه *tmps;
	ثابت نویسه *fuser_opt, *fuser_kill_prefix;
	size_t رقم_mounts = 0;
	size_t unmount_index;
	صحیح doing_unmount = 0;
	pid_t pid;
	صحیح status, flags;
	int64_t tmp, next_retry, now;
	int64_t rc_fuser_timeout = -1;
	ثابت نویسه *umount_args[UMOUNT_ARGS_MAX];
	صحیح umount_args_رقم = 0;
	ثابت نویسه **mounts = پوچ;
	ساختار run_queue running[RUN_MAX] = {0};
	ساختار run_queue *rp;
	size_t رقم_running = 0, رقم_waiting = 0;
	شمارش { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	اگر (_var) آزاد(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	اگر (_var) { regfree(_var); آزاد(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&args, 0, اندازه(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	اگر (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		وقتی (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			اگر (strcmp(tmps, "--") == 0)
				بشکن;
			اگر (umount_args_رقم >= (صحیح)ARRAY_SIZE(umount_args))
				eerrorx("%s: Too many umount arguments", applet);
			umount_args[umount_args_رقم++] = tmps;
		}

		tmps = rc_conf_مقدار("rc_fuser_timeout");
		اگر (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		اگر (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		اگر (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} وگرنه {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	وقتی ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
		حالت 'e':
			args.netdev = net_yes;
			بشکن;
		حالت 'E':
			args.netdev = net_no;
			بشکن;
		حالت 'f':
			DO_REG(args.fstype_regex);
			بشکن;
		حالت 'F':
			DO_REG(args.skip_fstype_regex);
			بشکن;
		حالت 'n':
			DO_REG(args.گره_regex);
			بشکن;
		حالت 'N':
			DO_REG(args.skip_گره_regex);
			بشکن;
		حالت 'o':
			DO_REG(args.options_regex);
			بشکن;
		حالت 'O':
			DO_REG(args.skip_options_regex);
			بشکن;
		حالت 'p':
			DO_REG(point_regex);
			بشکن;
		حالت 'P':
			DO_REG(skip_point_regex);
			بشکن;
		حالت 'i':
			args.mount_type = mount_options;
			بشکن;
		حالت 's':
			args.mount_type = mount_fstype;
			بشکن;
		حالت 't':
			args.mount_type = mount_from;
			بشکن;

		case_RC_COMMON_GETOPT
		}
	}

	وقتی (optind < argc) {
		اگر (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, پوچ);
		اگر (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		آزاد(real_path);
		real_path = پوچ;
	}
	nodes = find_mounts(&args, &رقم_mounts);
	rc_stringlist_free(args.mounts);

	اگر (doing_unmount)
		mounts = xmalloc(رقم_mounts * اندازه(*mounts));
	رقم_mounts = 0;
	result = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		اگر (point_regex &&
		    regexec(point_regex, s->مقدار, 0, پوچ, 0) != 0)
			ادامه;
		اگر (skip_point_regex &&
		    regexec(skip_point_regex, s->مقدار, 0, پوچ, 0) == 0)
			ادامه;
		اگر (doing_unmount)
			mounts[رقم_mounts++] = unescape_octal(s->مقدار);
		وگرنه اگر (!rc_yesno(getenv("EINFO_QUIET")))
			printf("%s\n", s->مقدار);
		result = EXIT_SUCCESS;
	}
	اگر (!doing_unmount)
		برو exit;

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
	وقتی (state != STATE_END) انتخاب (state) {
	حالت STATE_RUN:
		برای (unmount_index = 0; unmount_index < رقم_mounts; ++unmount_index) {
			ثابت نویسه *candidate = mounts[unmount_index];
			صحیح safe_to_unmount = 1;
			برای (size_t k = 0; safe_to_unmount && k < رقم_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			برای (size_t k = 0; safe_to_unmount && k < رقم_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			اگر (!safe_to_unmount)
				ادامه;
			اگر (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_index--] = mounts[--رقم_mounts];
			} وگرنه {
				بشکن;
			}
		}
		اگر (رقم_running == RUN_MAX || unmount_index >= رقم_mounts) {
			state = STATE_REAP;
			بشکن;
		}
		rp = running + رقم_running++;
		rp->mntpath = mounts[unmount_index];
		rp->last_exec_time = tm_now();
		rp->try_شمار = 0;
		rp->pid = run_umount(rp->mntpath, umount_args, umount_args_رقم);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_index] = mounts[--رقم_mounts];
		بشکن;
	حالت STATE_REAP:
		flags = (رقم_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = پوچ;
		برای (size_t i = 0; i < رقم_running; ++i) {
			rp = running + i;
			اگر (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			اگر (rp->pid == pid && pid > 0)
				بشکن;
			rp = پوچ;
		}
		اگر (rp) {
			اگر ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--رقم_running];
				state = STATE_RUN;
			} وگرنه اگر (rp->try_شمار >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--رقم_running];
				result = EXIT_FAILURE;
			} وگرنه { /* put into waiting queue */
				rp->pid = -1;
				rp->try_شمار += 1;
				رقم_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} وگرنه {
			state = STATE_RETRY;
		}
		بشکن;
	حالت STATE_RETRY:
		rp = پوچ;
		next_retry = INT64_MAX;
		برای (size_t i = 0; i < رقم_running; ++i) {
			اگر (running[i].pid > 0)
				ادامه;
			اگر (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			وگرنه
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			اگر (tmp < next_retry) {
				rp = running + i;
				next_retry = tmp;
			}
		}
		اگر (!rp) {
			state = (رقم_mounts > 0) ? STATE_RUN : STATE_END;
			بشکن;
		}
		now = tm_now();
		اگر ((rp->pid > 0 || rp->fuser_pid > 0) && next_retry > now) {
			int64_t sleep_for = next_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			اگر (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		اگر ((rp->pid == -1 && rp->fuser_pid == -1) || next_retry <= now) {
			اگر (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, پوچ, 0);
				rp->fuser_pid = -1;
			}
			اگر (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--رقم_running];
				result = EXIT_FAILURE;
			} وگرنه { /* retry */
				rp->last_exec_time = tm_now();
				rp->pid = run_umount(rp->mntpath,
					umount_args, umount_args_رقم);
			}
			رقم_waiting -= 1;
			state = STATE_REAP;
		}
		بشکن;
	پیشفرض: بشکن;
	}

exit:
	آزاد(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(args.fstype_regex);
	REG_FREE(args.skip_fstype_regex);
	REG_FREE(args.گره_regex);
	REG_FREE(args.skip_گره_regex);
	REG_FREE(args.options_regex);
	REG_FREE(args.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	بازگشت result;
}
