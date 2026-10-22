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

#إذا defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#وإلاإذا defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#وإلاإذا defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#وإلاإذا defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#endif

#ضمن <errno.h>
#ضمن <getopt.h>
#ضمن <regex.h>
#ضمن <stddef.h>
#ضمن <stdio.h>
#ضمن <stdint.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <unistd.h>

#ضمن "einfo.h"
#ضمن "queue.h"
#ضمن "rc.h"
#ضمن "rc_exec.h"
#ضمن "timeutils.h"
#ضمن "_usage.h"
#ضمن "helpers.h"

ثابت حرف *applet = عدم;
ثابت حرف *procmounts = "/proc/mounts";
ثابت حرف *extraopts = "[mount1] [mount2] ...";
ثابت حرف getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "fstype-regex",        1, عدم, 'f'},
	{ "skip-fstype-regex",   1, عدم, 'F'},
	{ "node-regex",          1, عدم, 'n'},
	{ "skip-node-regex",     1, عدم, 'N'},
	{ "options-regex",       1, عدم, 'o'},
	{ "skip-options-regex",  1, عدم, 'O'},
	{ "point-regex",         1, عدم, 'p'},
	{ "skip-point-regex",    1, عدم, 'P'},
	{ "options",             0, عدم, 'i'},
	{ "fstype",              0, عدم, 's'},
	{ "node",                0, عدم, 't'},
	{ "netdev",              0, عدم, 'e'},
	{ "nonetdev",            0, عدم, 'E'},
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
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
ثابت حرف *usagestring = عدم;

#define UMOUNT_وسائط_MAX 16
#define RUN_MAX 32
#define TRY_MAX 3
#define TRY_DELAY_MS 1000

بنية run_queue {
	ثابت حرف *mntpath;
	int64_t last_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	صحيح try_عدد;
	صحيح fuser_stdoutfd;
};

تعريفنوع تعداد {
	mount_from,
	mount_to,
	mount_fstype,
	mount_options
} mount_type;

تعريفنوع تعداد {
	net_ignore,
	net_yes,
	net_no
} net_opts;

بنية وسائط;
تعريفنوع صحيح process_func_t(RC_STRINGLIST *, بنية وسائط *,
    حرف *, حرف *, حرف *, حرف *, صحيح);

بنية وسائط {
	process_func_t *process;
	regex_t *عقدة_regex;
	regex_t *skip_عقدة_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	ثابت حرف *check_mntpath;
};

ساكن صحيح
process_mount(RC_STRINGLIST *list, بنية وسائط *وسائط,
    حرف *from, حرف *to, حرف *fstype, حرف *options,
    صحيح netdev)
{
	حرف *p;
	RC_STRING *s;

	errno = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	إذا (strcmp(fstype, "rootfs") == 0)
		أرجع -1;
#endif

	إذا (وسائط->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(وسائط->mounts)))
	{
		إذا (netdev != 0)
			أرجع 1;
	} وإلا إذا (وسائط->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(وسائط->mounts)))
	{
		إذا (netdev != 1)
			أرجع 1;
	} وإلا {
		إذا (وسائط->عقدة_regex &&
		    regexec(وسائط->عقدة_regex, from, 0, عدم, 0) != 0)
			أرجع 1;
		إذا (وسائط->skip_عقدة_regex &&
		    regexec(وسائط->skip_عقدة_regex, from, 0, عدم, 0) == 0)
			أرجع 1;

		إذا (وسائط->fstype_regex &&
		    regexec(وسائط->fstype_regex, fstype, 0, عدم, 0) != 0)
			أرجع -1;
		إذا (وسائط->skip_fstype_regex &&
		    regexec(وسائط->skip_fstype_regex, fstype, 0, عدم, 0) == 0)
			أرجع -1;

		إذا (وسائط->options_regex &&
		    regexec(وسائط->options_regex, options, 0, عدم, 0) != 0)
			أرجع -1;
		إذا (وسائط->skip_options_regex &&
		    regexec(وسائط->skip_options_regex, options, 0, عدم, 0) == 0)
			أرجع -1;
	}

	إذا (TAILQ_FIRST(وسائط->mounts)) {
		TAILQ_FOREACH(s, وسائط->mounts, entries)
		    إذا (strcmp(s->قيمة, to) == 0)
			    اكسر;
		إذا (!s)
			أرجع -1;
	}

	بدل (وسائط->mount_type) {
	حالة mount_from:
		p = from;
		اكسر;
	حالة mount_to:
		p = to;
		اكسر;
	حالة mount_fstype:
		p = fstype;
		اكسر;
	حالة mount_options:
		p = options;
		اكسر;
	افتراضي:
		p = عدم;
		errno = EINVAL;
		اكسر;
	}

	إذا (p) {
		errno = 0;
		rc_stringlist_add(list, p);
		أرجع 0;
	}

	أرجع -1;
}

#إذا defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
ساكن بنية opt {
	صحيح o_opt;
	ثابت حرف *o_name;
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
	{ 0, عدم }
};

ساكن RC_STRINGLIST *
find_mounts(بنية وسائط *وسائط, size_t *رقم_mounts)
{
	بنية statfs *mnts;
	صحيح nmnts;
	صحيح i;
	RC_STRINGLIST *list;
	حرف *options = عدم;
	uint64_t flags;
	بنية opt *o;
	صحيح netdev;
	حرف *tmp;

	إذا ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	لكل (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		لكل (o = optnames; flags && o->o_opt; o++) {
			إذا (flags & o->o_opt) {
				إذا (o->o_opt == MNT_LOCAL)
					netdev = 1;
				إذا (!options)
					options = xstrdup(o->o_name);
				وإلا {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					حرر(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*رقم_mounts += 0 == وسائط->process(list, وسائط,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		حرر(options);
		options = عدم;
	}

	أرجع list;
}

#وإلاإذا defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
ساكن بنية mntent *
getmntfile(ثابت حرف *file)
{
	بنية mntent *ent = عدم;
	FILE *fp;

	إذا (!(fp = setmntent("/etc/fstab", "r")))
		أرجع عدم;
	طالما ((ent = getmntent(fp)))
		إذا (strcmp(file, ent->mnt_dir) == 0)
			اكسر;
	endmntent(fp);

	أرجع ent;
}

ساكن RC_STRINGLIST *
find_mounts(بنية وسائط *وسائط, size_t *رقم_mounts)
{
	FILE *fp;
	حرف *buffer;
	size_t size;
	حرف *p;
	حرف *from;
	حرف *to;
	حرف *fst;
	حرف *opts;
	بنية mntent *ent;
	صحيح netdev;
	RC_STRINGLIST *list;

	إذا ((fp = fopen(procmounts, "r")) == عدم)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	buffer = عدم;
	طالما (xgetline(&buffer, &size, fp) != -1) {
		netdev = -1;
		p = buffer;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		إذا ((ent = getmntfile(to))) {
			إذا (strstr(ent->mnt_opts, "_netdev"))
				netdev = 0;
			وإلا
				netdev = 1;
		}

		*رقم_mounts += 0 == وسائط->process(list, وسائط,
			from, to, fst, opts, netdev);

		حرر(buffer);
		buffer = عدم;
	}
	حرر(buffer);
	fclose(fp);

	أرجع list;
}

#وإلا
#  error "Operating system not supported!"
#endif

ساكن صحيح is_prefix(ثابت حرف *needle, ثابت حرف *hay)
{
	size_t nlen = strlen(needle);
	إذا (strncmp(needle, hay, nlen) == 0 && hay[nlen] == '/')
		أرجع true;
	أرجع false;
}

ساكن حرف *unescape_octal(حرف *beg)
{
	صحيح n, i;
	حرف *w = beg, *r = beg;
	طالما (*r) {
		إذا (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} وإلا {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			لكل (i = n = 0; i < 3; ++i) {
				إذا (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} وإلا {
					اكسر;
				}
			}
			إذا (n)
				*w++ = n;
		}
	}
	*w = '\0';
	أرجع beg;
}

ساكن صحيح check_is_mounted(RC_STRINGLIST *list RC_UNUSED, بنية وسائط *وسائط,
    حرف *from RC_UNUSED, حرف *to, حرف *fstype RC_UNUSED,
    حرف *options RC_UNUSED, صحيح netdev RC_UNUSED)
{
	أرجع strcmp(وسائط->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

ساكن صحيح is_mounted(ثابت حرف *mntpath)
{
	size_t رقم_mounts = 0;
	بنية وسائط وسائط = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&وسائط, &رقم_mounts);
	rc_stringlist_free(l);
	أرجع رقم_mounts > 0;
}

ساكن pid_t run_umount(ثابت حرف *mntpath,
	ثابت حرف **umount_وسائط, صحيح umount_وسائط_رقم)
{
	بنية exec_وسائط وسائط;
	بنية exec_result res;
	ثابت حرف *argv[UMOUNT_وسائط_MAX + 3];
	صحيح k, i = 0;

	argv[i++] = "umount";
	لكل (k = 0; k < umount_وسائط_رقم; ++k)
		argv[i++] = umount_وسائط[k];
	argv[i++] = mntpath;
	argv[i++] = عدم;

	وسائط = exec_init(argv);
	وسائط.redirect_stdout = وسائط.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&وسائط);
	إذا (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	أرجع res.pid;
}

ساكن فراغ fuser_run(بنية run_queue *rp, ثابت حرف *fuser_opt)
{
	ساكن صحيح fuser_exec_failed = 0;
	ثابت حرف *argv[] = { "fuser", fuser_opt, rp->mntpath, عدم };
	بنية exec_result res;
	بنية exec_وسائط وسائط;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	إذا (fuser_exec_failed)
		أرجع;

	وسائط = exec_init(argv);
	وسائط.redirect_stdout = EXEC_MKPIPE;
	وسائط.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&وسائط);
	إذا (res.pid < 0) {
		fuser_exec_failed = 1;
	} وإلا {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

ساكن صحيح fuser_decide(بنية run_queue *rp,
	ثابت حرف *fuser_opt, ثابت حرف *fuser_kill_prefix)
{
	ساكن bool has_rc_pid = false;
	ساكن حرف *rc_pid;
	حرف buf[1<<12];
	حرف selfpid[64];
	صحيح read_maybe_truncated;
	ssize_t n;

	إذا (rp->fuser_stdoutfd < 0)
		أرجع 0;

	buf[0] = ' ';
	n = اقرأ(rp->fuser_stdoutfd, buf + 1, حجم buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == حجم buf - 3);
	إذا (n < 0 || read_maybe_truncated)
		أرجع 0;
	طالما (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, حجم selfpid, " %lld ", (long long)getpid());

	إذا (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	إذا (strstr(buf, selfpid) || (rc_pid && strstr(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		أرجع -1;
	} وإلا إذا (strcmp(buf, "  ") == 0) {
		إذا (rp->try_عدد >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			أرجع -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_عدد = TRY_MAX;
		أرجع 0;
	} وإلا {
		حرف sig[32];
		ثابت حرف *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, عدم
		};
		بنية exec_result res;
		بنية exec_وسائط وسائط = exec_init(argv);
		وسائط.redirect_stdout = وسائط.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, حجم sig, "%s%s", fuser_kill_prefix,
			rp->try_عدد == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&وسائط);
		إذا (res.pid > 0)
			rc_waitpid(res.pid);
		أرجع 0;
	}
}

ساكن regex_t *
get_regex(ثابت حرف *string)
{
	regex_t *reg = xmalloc(حجم (*reg));
	صحيح result;
	حرف buffer[256];

	إذا ((result = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(result, reg, buffer, حجم(buffer));
		eerrorx("%s: invalid regex `%s'", applet, buffer);
	}

	أرجع reg;
}

صحيح main(صحيح argc, حرف **argv)
{
	بنية وسائط وسائط;
	regex_t *point_regex = عدم;
	regex_t *skip_point_regex = عدم;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	حرف *real_path = عدم;
	صحيح opt;
	صحيح result;
	حرف *this_path, *argv0;
	ثابت حرف *tmps;
	ثابت حرف *fuser_opt, *fuser_kill_prefix;
	size_t رقم_mounts = 0;
	size_t unmount_index;
	صحيح doing_unmount = 0;
	pid_t pid;
	صحيح status, flags;
	int64_t tmp, next_retry, now;
	int64_t rc_fuser_timeout = -1;
	ثابت حرف *umount_وسائط[UMOUNT_وسائط_MAX];
	صحيح umount_وسائط_رقم = 0;
	ثابت حرف **mounts = عدم;
	بنية run_queue running[RUN_MAX] = {0};
	بنية run_queue *rp;
	size_t رقم_running = 0, رقم_waiting = 0;
	تعداد { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	إذا (_var) حرر(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	إذا (_var) { regfree(_var); حرر(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&وسائط, 0, حجم(وسائط));
	وسائط.mount_type = mount_to;
	وسائط.netdev = net_ignore;
	وسائط.mounts = rc_stringlist_new();
	وسائط.process = process_mount;

	إذا (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		طالما (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			إذا (strcmp(tmps, "--") == 0)
				اكسر;
			إذا (umount_وسائط_رقم >= (صحيح)ARRAY_SIZE(umount_وسائط))
				eerrorx("%s: Too many umount arguments", applet);
			umount_وسائط[umount_وسائط_رقم++] = tmps;
		}

		tmps = rc_conf_قيمة("rc_fuser_timeout");
		إذا (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		إذا (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		إذا (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} وإلا {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	طالما ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
		حالة 'e':
			وسائط.netdev = net_yes;
			اكسر;
		حالة 'E':
			وسائط.netdev = net_no;
			اكسر;
		حالة 'f':
			DO_REG(وسائط.fstype_regex);
			اكسر;
		حالة 'F':
			DO_REG(وسائط.skip_fstype_regex);
			اكسر;
		حالة 'n':
			DO_REG(وسائط.عقدة_regex);
			اكسر;
		حالة 'N':
			DO_REG(وسائط.skip_عقدة_regex);
			اكسر;
		حالة 'o':
			DO_REG(وسائط.options_regex);
			اكسر;
		حالة 'O':
			DO_REG(وسائط.skip_options_regex);
			اكسر;
		حالة 'p':
			DO_REG(point_regex);
			اكسر;
		حالة 'P':
			DO_REG(skip_point_regex);
			اكسر;
		حالة 'i':
			وسائط.mount_type = mount_options;
			اكسر;
		حالة 's':
			وسائط.mount_type = mount_fstype;
			اكسر;
		حالة 't':
			وسائط.mount_type = mount_from;
			اكسر;

		case_RC_COMMON_GETOPT
		}
	}

	طالما (optind < argc) {
		إذا (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, عدم);
		إذا (real_path)
			this_path = real_path;
		rc_stringlist_add(وسائط.mounts, this_path);
		حرر(real_path);
		real_path = عدم;
	}
	nodes = find_mounts(&وسائط, &رقم_mounts);
	rc_stringlist_free(وسائط.mounts);

	إذا (doing_unmount)
		mounts = xmalloc(رقم_mounts * حجم(*mounts));
	رقم_mounts = 0;
	result = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		إذا (point_regex &&
		    regexec(point_regex, s->قيمة, 0, عدم, 0) != 0)
			استمر;
		إذا (skip_point_regex &&
		    regexec(skip_point_regex, s->قيمة, 0, عدم, 0) == 0)
			استمر;
		إذا (doing_unmount)
			mounts[رقم_mounts++] = unescape_octal(s->قيمة);
		وإلا إذا (!rc_yesno(getenv("EINFO_QUIET")))
			printf("%s\n", s->قيمة);
		result = EXIT_SUCCESS;
	}
	إذا (!doing_unmount)
		اذهب exit;

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
	طالما (state != STATE_END) بدل (state) {
	حالة STATE_RUN:
		لكل (unmount_index = 0; unmount_index < رقم_mounts; ++unmount_index) {
			ثابت حرف *candidate = mounts[unmount_index];
			صحيح safe_to_unmount = 1;
			لكل (size_t k = 0; safe_to_unmount && k < رقم_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			لكل (size_t k = 0; safe_to_unmount && k < رقم_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			إذا (!safe_to_unmount)
				استمر;
			إذا (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_index--] = mounts[--رقم_mounts];
			} وإلا {
				اكسر;
			}
		}
		إذا (رقم_running == RUN_MAX || unmount_index >= رقم_mounts) {
			state = STATE_REAP;
			اكسر;
		}
		rp = running + رقم_running++;
		rp->mntpath = mounts[unmount_index];
		rp->last_exec_time = tm_now();
		rp->try_عدد = 0;
		rp->pid = run_umount(rp->mntpath, umount_وسائط, umount_وسائط_رقم);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_index] = mounts[--رقم_mounts];
		اكسر;
	حالة STATE_REAP:
		flags = (رقم_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = عدم;
		لكل (size_t i = 0; i < رقم_running; ++i) {
			rp = running + i;
			إذا (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			إذا (rp->pid == pid && pid > 0)
				اكسر;
			rp = عدم;
		}
		إذا (rp) {
			إذا ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--رقم_running];
				state = STATE_RUN;
			} وإلا إذا (rp->try_عدد >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--رقم_running];
				result = EXIT_FAILURE;
			} وإلا { /* put into waiting queue */
				rp->pid = -1;
				rp->try_عدد += 1;
				رقم_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} وإلا {
			state = STATE_RETRY;
		}
		اكسر;
	حالة STATE_RETRY:
		rp = عدم;
		next_retry = INT64_MAX;
		لكل (size_t i = 0; i < رقم_running; ++i) {
			إذا (running[i].pid > 0)
				استمر;
			إذا (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			وإلا
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			إذا (tmp < next_retry) {
				rp = running + i;
				next_retry = tmp;
			}
		}
		إذا (!rp) {
			state = (رقم_mounts > 0) ? STATE_RUN : STATE_END;
			اكسر;
		}
		now = tm_now();
		إذا ((rp->pid > 0 || rp->fuser_pid > 0) && next_retry > now) {
			int64_t sleep_for = next_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			إذا (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		إذا ((rp->pid == -1 && rp->fuser_pid == -1) || next_retry <= now) {
			إذا (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, عدم, 0);
				rp->fuser_pid = -1;
			}
			إذا (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--رقم_running];
				result = EXIT_FAILURE;
			} وإلا { /* retry */
				rp->last_exec_time = tm_now();
				rp->pid = run_umount(rp->mntpath,
					umount_وسائط, umount_وسائط_رقم);
			}
			رقم_waiting -= 1;
			state = STATE_REAP;
		}
		اكسر;
	افتراضي: اكسر;
	}

exit:
	حرر(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(وسائط.fstype_regex);
	REG_FREE(وسائط.skip_fstype_regex);
	REG_FREE(وسائط.عقدة_regex);
	REG_FREE(وسائط.skip_عقدة_regex);
	REG_FREE(وسائط.options_regex);
	REG_FREE(وسائط.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	أرجع result;
}
