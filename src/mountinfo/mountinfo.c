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
#جے_انت

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

#تعریف_کرو UMOUNT_دلیلاں_MAX 16
#تعریف_کرو لگاتار_MAX 32
#تعریف_کرو TRY_MAX 3
#تعریف_کرو TRY_DELAY_MS 1000

بنية لگاتار_قطار {
	ثابت حرف *mntpath;
	int64_t آخری_exec_ویلا;
	int64_t fuser_exec_ویلا;
	pid_t pid;
	pid_t fuser_pid;
	صحيح try_گنتی;
	صحيح fuser_stdoutfd;
};

تعريفنوع تعداد {
	mount_from,
	mount_منزل,
	mount_fstype,
	mount_options
} mount_type;

تعريفنوع تعداد {
	net_ignore,
	net_yes,
	net_no
} net_opts;

بنية دلیلاں;
تعريفنوع صحيح process_func_t(RC_STRINGLIST *, بنية دلیلاں *,
    حرف *, حرف *, حرف *, حرف *, صحيح);

بنية دلیلاں {
	process_func_t *process;
	regex_t *گرہ_regex;
	regex_t *skip_گرہ_regex;
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
process_mount(RC_STRINGLIST *list, بنية دلیلاں *دلیلاں,
    حرف *from, حرف *منزل, حرف *fstype, حرف *options,
    صحيح netdev)
{
	حرف *p;
	RC_STRING *s;

	errno = ENOENT;

#جے_تعریف_شدہ __linux__
	/* Skip the really silly rootfs */
	إذا (strcmp(fstype, "rootfs") == 0)
		أرجع -1;
#جے_انت

	إذا (دلیلاں->netdev == net_yes &&
	    (netdev != -1 || TAILQ_پہلا(دلیلاں->mounts)))
	{
		إذا (netdev != 0)
			أرجع 1;
	} وإلا إذا (دلیلاں->netdev == net_no &&
	    (netdev != -1 || TAILQ_پہلا(دلیلاں->mounts)))
	{
		إذا (netdev != 1)
			أرجع 1;
	} وإلا {
		إذا (دلیلاں->گرہ_regex &&
		    regexec(دلیلاں->گرہ_regex, from, 0, عدم, 0) != 0)
			أرجع 1;
		إذا (دلیلاں->skip_گرہ_regex &&
		    regexec(دلیلاں->skip_گرہ_regex, from, 0, عدم, 0) == 0)
			أرجع 1;

		إذا (دلیلاں->fstype_regex &&
		    regexec(دلیلاں->fstype_regex, fstype, 0, عدم, 0) != 0)
			أرجع -1;
		إذا (دلیلاں->skip_fstype_regex &&
		    regexec(دلیلاں->skip_fstype_regex, fstype, 0, عدم, 0) == 0)
			أرجع -1;

		إذا (دلیلاں->options_regex &&
		    regexec(دلیلاں->options_regex, options, 0, عدم, 0) != 0)
			أرجع -1;
		إذا (دلیلاں->skip_options_regex &&
		    regexec(دلیلاں->skip_options_regex, options, 0, عدم, 0) == 0)
			أرجع -1;
	}

	إذا (TAILQ_پہلا(دلیلاں->mounts)) {
		TAILQ_FOREACH(s, دلیلاں->mounts, entries)
		    إذا (strcmp(s->قدر, منزل) == 0)
			    اكسر;
		إذا (!s)
			أرجع -1;
	}

	بدل (دلیلاں->mount_type) {
	حالة mount_from:
		p = from;
		اكسر;
	حالة mount_منزل:
		p = منزل;
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
		rc_stringlist_جوڑو(list, p);
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
#جے_تعریف_شدہ MNT_NOSYMFOLLOW
	{ MNT_NOSYMFOLLOW,  "nosymfollow" },
#جے_انت
	{ MNT_QUOTA,        "with quotas" },
	{ MNT_RDONLY,       "read-only" },
	{ MNT_SYNCHRONOUS,  "synchronous" },
	{ MNT_UNION,        "union" },
#جے_تعریف_شدہ MNT_NOCLUSTERR
	{ MNT_NOCLUSTERR,   "noclusterr" },
#جے_انت
#جے_تعریف_شدہ MNT_NOCLUSTERW
	{ MNT_NOCLUSTERW,   "noclusterw" },
#جے_انت
#جے_تعریف_شدہ MNT_SUIDDIR
	{ MNT_SUIDDIR,      "suiddir" },
#جے_انت
	{ MNT_SOFTDEP,      "soft-updates" },
#جے_تعریف_شدہ MNT_MULTILABEL
	{ MNT_MULTILABEL,   "multilabel" },
#جے_انت
#جے_تعریف_شدہ MNT_ACLS
	{ MNT_ACLS,         "acls" },
#جے_انت
#جے_تعریف_شدہ MNT_GJOURNAL
	{ MNT_GJOURNAL,     "gjournal" },
#جے_انت
	{ 0, عدم }
};

ساكن RC_STRINGLIST *
لبھو_mounts(بنية دلیلاں *دلیلاں, size_t *عدد_mounts)
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

		*عدد_mounts += 0 == دلیلاں->process(list, دلیلاں,
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
لبھو_mounts(بنية دلیلاں *دلیلاں, size_t *عدد_mounts)
{
	FILE *fp;
	حرف *بفر;
	size_t ناپ;
	حرف *p;
	حرف *from;
	حرف *منزل;
	حرف *fst;
	حرف *opts;
	بنية mntent *ent;
	صحيح netdev;
	RC_STRINGLIST *list;

	إذا ((fp = fopen(procmounts, "r")) == عدم)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	بفر = عدم;
	طالما (xgetline(&بفر, &ناپ, fp) != -1) {
		netdev = -1;
		p = بفر;
		from = strsep(&p, " ");
		منزل = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		إذا ((ent = getmntfile(منزل))) {
			إذا (لڑی_کھوج(ent->mnt_opts, "_netdev"))
				netdev = 0;
			وإلا
				netdev = 1;
		}

		*عدد_mounts += 0 == دلیلاں->process(list, دلیلاں,
			from, منزل, fst, opts, netdev);

		حرر(بفر);
		بفر = عدم;
	}
	حرر(بفر);
	fclose(fp);

	أرجع list;
}

#وإلا
#  error "Operating system not supported!"
#جے_انت

ساكن صحيح is_اگیتر(ثابت حرف *needle, ثابت حرف *hay)
{
	size_t nlen = لڑی_لمبائی(needle);
	إذا (لڑی_مقابلہ(needle, hay, nlen) == 0 && hay[nlen] == '/')
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

ساكن صحيح check_is_mounted(RC_STRINGLIST *list RC_بن_ورتیا, بنية دلیلاں *دلیلاں,
    حرف *from RC_بن_ورتیا, حرف *منزل, حرف *fstype RC_بن_ورتیا,
    حرف *options RC_بن_ورتیا, صحيح netdev RC_بن_ورتیا)
{
	أرجع strcmp(دلیلاں->check_mntpath, unescape_octal(منزل)) == 0 ? 0 : -1;
}

ساكن صحيح is_mounted(ثابت حرف *mntpath)
{
	size_t عدد_mounts = 0;
	بنية دلیلاں دلیلاں = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = لبھو_mounts(&دلیلاں, &عدد_mounts);
	rc_stringlist_free(l);
	أرجع عدد_mounts > 0;
}

ساكن pid_t لگاتار_umount(ثابت حرف *mntpath,
	ثابت حرف **umount_دلیلاں, صحيح umount_دلیلاں_عدد)
{
	بنية exec_دلیلاں دلیلاں;
	بنية exec_نتیجہ res;
	ثابت حرف *argv[UMOUNT_دلیلاں_MAX + 3];
	صحيح k, i = 0;

	argv[i++] = "umount";
	لكل (k = 0; k < umount_دلیلاں_عدد; ++k)
		argv[i++] = umount_دلیلاں[k];
	argv[i++] = mntpath;
	argv[i++] = عدم;

	دلیلاں = exec_init(argv);
	دلیلاں.redirect_stdout = دلیلاں.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&دلیلاں);
	إذا (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	أرجع res.pid;
}

ساكن فراغ fuser_لگاتار(بنية لگاتار_قطار *rp, ثابت حرف *fuser_opt)
{
	ساكن صحيح fuser_exec_failed = 0;
	ثابت حرف *argv[] = { "fuser", fuser_opt, rp->mntpath, عدم };
	بنية exec_نتیجہ res;
	بنية exec_دلیلاں دلیلاں;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	إذا (fuser_exec_failed)
		أرجع;

	دلیلاں = exec_init(argv);
	دلیلاں.redirect_stdout = EXEC_MKPIPE;
	دلیلاں.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&دلیلاں);
	إذا (res.pid < 0) {
		fuser_exec_failed = 1;
	} وإلا {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_ویلا = tm_now();
	}
}

ساكن صحيح fuser_decide(بنية لگاتار_قطار *rp,
	ثابت حرف *fuser_opt, ثابت حرف *fuser_kill_اگیتر)
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
	snprintf(selfpid, حجم selfpid, " %lld ", (لما لما)getpid());

	إذا (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	إذا (لڑی_کھوج(buf, selfpid) || (rc_pid && لڑی_کھوج(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		أرجع -1;
	} وإلا إذا (strcmp(buf, "  ") == 0) {
		إذا (rp->try_گنتی >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			أرجع -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_گنتی = TRY_MAX;
		أرجع 0;
	} وإلا {
		حرف sig[32];
		ثابت حرف *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, عدم
		};
		بنية exec_نتیجہ res;
		بنية exec_دلیلاں دلیلاں = exec_init(argv);
		دلیلاں.redirect_stdout = دلیلاں.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, حجم sig, "%s%s", fuser_kill_اگیتر,
			rp->try_گنتی == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&دلیلاں);
		إذا (res.pid > 0)
			rc_waitpid(res.pid);
		أرجع 0;
	}
}

ساكن regex_t *
get_regex(ثابت حرف *string)
{
	regex_t *reg = xmalloc(حجم (*reg));
	صحيح نتیجہ;
	حرف بفر[256];

	إذا ((نتیجہ = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(نتیجہ, reg, بفر, حجم(بفر));
		eerrorx("%s: invalid regex `%s'", applet, بفر);
	}

	أرجع reg;
}

صحيح مکھ(صحيح argc, حرف **argv)
{
	بنية دلیلاں دلیلاں;
	regex_t *نقطہ_regex = عدم;
	regex_t *skip_نقطہ_regex = عدم;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	حرف *real_رستہ = عدم;
	صحيح opt;
	صحيح نتیجہ;
	حرف *this_رستہ, *argv0;
	ثابت حرف *tmps;
	ثابت حرف *fuser_opt, *fuser_kill_اگیتر;
	size_t عدد_mounts = 0;
	size_t unmount_اشاریہ;
	صحيح doing_unmount = 0;
	pid_t pid;
	صحيح حالت, flags;
	int64_t tmp, اگلا_retry, now;
	int64_t rc_fuser_timeout = -1;
	ثابت حرف *umount_دلیلاں[UMOUNT_دلیلاں_MAX];
	صحيح umount_دلیلاں_عدد = 0;
	ثابت حرف **mounts = عدم;
	بنية لگاتار_قطار running[لگاتار_MAX] = {0};
	بنية لگاتار_قطار *rp;
	size_t عدد_running = 0, عدد_اڈیک = 0;
	تعداد { STATE_لگاتار, STATE_REAP, STATE_RETRY, STATE_سرا } state;

#تعریف_کرو DO_REG(_var)							      \
	إذا (_var) حرر(_var);						      \
	_var = get_regex(optarg);
#تعریف_کرو REG_FREE(_var)							      \
	إذا (_var) { regfree(_var); حرر(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&دلیلاں, 0, حجم(دلیلاں));
	دلیلاں.mount_type = mount_منزل;
	دلیلاں.netdev = net_ignore;
	دلیلاں.mounts = rc_stringlist_new();
	دلیلاں.process = process_mount;

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
			إذا (umount_دلیلاں_عدد >= (صحيح)ARRAY_ناپ(umount_دلیلاں))
				eerrorx("%s: Too many umount arguments", applet);
			umount_دلیلاں[umount_دلیلاں_عدد++] = tmps;
		}

		tmps = rc_conf_قدر("rc_fuser_timeout");
		إذا (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		إذا (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		إذا (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_اگیتر = "-";
		} وإلا {
			fuser_opt = "-cm";
			fuser_kill_اگیتر = "-s";
		}
	}

	طالما ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
		حالة 'e':
			دلیلاں.netdev = net_yes;
			اكسر;
		حالة 'E':
			دلیلاں.netdev = net_no;
			اكسر;
		حالة 'f':
			DO_REG(دلیلاں.fstype_regex);
			اكسر;
		حالة 'F':
			DO_REG(دلیلاں.skip_fstype_regex);
			اكسر;
		حالة 'n':
			DO_REG(دلیلاں.گرہ_regex);
			اكسر;
		حالة 'N':
			DO_REG(دلیلاں.skip_گرہ_regex);
			اكسر;
		حالة 'o':
			DO_REG(دلیلاں.options_regex);
			اكسر;
		حالة 'O':
			DO_REG(دلیلاں.skip_options_regex);
			اكسر;
		حالة 'p':
			DO_REG(نقطہ_regex);
			اكسر;
		حالة 'P':
			DO_REG(skip_نقطہ_regex);
			اكسر;
		حالة 'i':
			دلیلاں.mount_type = mount_options;
			اكسر;
		حالة 's':
			دلیلاں.mount_type = mount_fstype;
			اكسر;
		حالة 't':
			دلیلاں.mount_type = mount_from;
			اكسر;

		case_RC_COMMON_GETOPT
		}
	}

	طالما (optind < argc) {
		إذا (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_رستہ = argv[optind++];
		real_رستہ = realpath(this_رستہ, عدم);
		إذا (real_رستہ)
			this_رستہ = real_رستہ;
		rc_stringlist_جوڑو(دلیلاں.mounts, this_رستہ);
		حرر(real_رستہ);
		real_رستہ = عدم;
	}
	nodes = لبھو_mounts(&دلیلاں, &عدد_mounts);
	rc_stringlist_free(دلیلاں.mounts);

	إذا (doing_unmount)
		mounts = xmalloc(عدد_mounts * حجم(*mounts));
	عدد_mounts = 0;
	نتیجہ = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_الٹ(s, nodes, rc_stringlist, entries) {
		إذا (نقطہ_regex &&
		    regexec(نقطہ_regex, s->قدر, 0, عدم, 0) != 0)
			استمر;
		إذا (skip_نقطہ_regex &&
		    regexec(skip_نقطہ_regex, s->قدر, 0, عدم, 0) == 0)
			استمر;
		إذا (doing_unmount)
			mounts[عدد_mounts++] = unescape_octal(s->قدر);
		وإلا إذا (!rc_yesno(getenv("EINFO_QUIET")))
			لکھو("%s\n", s->قدر);
		نتیجہ = EXIT_SUCCESS;
	}
	إذا (!doing_unmount)
		اذهب باہر_نکلو;

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
	نتیجہ = EXIT_SUCCESS;
	state = STATE_لگاتار;
	طالما (state != STATE_سرا) بدل (state) {
	حالة STATE_لگاتار:
		لكل (unmount_اشاریہ = 0; unmount_اشاریہ < عدد_mounts; ++unmount_اشاریہ) {
			ثابت حرف *candidate = mounts[unmount_اشاریہ];
			صحيح محفوظ_منزل_unmount = 1;
			لكل (size_t k = 0; محفوظ_منزل_unmount && k < عدد_running; ++k)
				محفوظ_منزل_unmount = !is_اگیتر(candidate, running[k].mntpath);
			لكل (size_t k = 0; محفوظ_منزل_unmount && k < عدد_mounts; ++k)
				محفوظ_منزل_unmount = !is_اگیتر(candidate, mounts[k]);
			إذا (!محفوظ_منزل_unmount)
				استمر;
			إذا (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_اشاریہ--] = mounts[--عدد_mounts];
			} وإلا {
				اكسر;
			}
		}
		إذا (عدد_running == لگاتار_MAX || unmount_اشاریہ >= عدد_mounts) {
			state = STATE_REAP;
			اكسر;
		}
		rp = running + عدد_running++;
		rp->mntpath = mounts[unmount_اشاریہ];
		rp->آخری_exec_ویلا = tm_now();
		rp->try_گنتی = 0;
		rp->pid = لگاتار_umount(rp->mntpath, umount_دلیلاں, umount_دلیلاں_عدد);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_ویلا = -1;
		mounts[unmount_اشاریہ] = mounts[--عدد_mounts];
		اكسر;
	حالة STATE_REAP:
		flags = (عدد_اڈیک > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &حالت, flags);
		rp = عدم;
		لكل (size_t i = 0; i < عدد_running; ++i) {
			rp = running + i;
			إذا (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			إذا (rp->pid == pid && pid > 0)
				اكسر;
			rp = عدم;
		}
		إذا (rp) {
			إذا ((WIFEXITED(حالت) && WEXITSTATUS(حالت) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--عدد_running];
				state = STATE_لگاتار;
			} وإلا إذا (rp->try_گنتی >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--عدد_running];
				نتیجہ = EXIT_FAILURE;
			} وإلا { /* put into waiting queue */
				rp->pid = -1;
				rp->try_گنتی += 1;
				عدد_اڈیک += 1;
				fuser_لگاتار(rp, fuser_opt);
			}
		} وإلا {
			state = STATE_RETRY;
		}
		اكسر;
	حالة STATE_RETRY:
		rp = عدم;
		اگلا_retry = INT64_MAX;
		لكل (size_t i = 0; i < عدد_running; ++i) {
			إذا (running[i].pid > 0)
				استمر;
			إذا (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_ویلا + rc_fuser_timeout;
			وإلا
				tmp = running[i].آخری_exec_ویلا + TRY_DELAY_MS;
			إذا (tmp < اگلا_retry) {
				rp = running + i;
				اگلا_retry = tmp;
			}
		}
		إذا (!rp) {
			state = (عدد_mounts > 0) ? STATE_لگاتار : STATE_سرا;
			اكسر;
		}
		now = tm_now();
		إذا ((rp->pid > 0 || rp->fuser_pid > 0) && اگلا_retry > now) {
			int64_t sleep_for = اگلا_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			إذا (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		إذا ((rp->pid == -1 && rp->fuser_pid == -1) || اگلا_retry <= now) {
			إذا (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, عدم, 0);
				rp->fuser_pid = -1;
			}
			إذا (fuser_decide(rp, fuser_opt, fuser_kill_اگیتر) < 0) { /* abort */
				*rp = running[--عدد_running];
				نتیجہ = EXIT_FAILURE;
			} وإلا { /* retry */
				rp->آخری_exec_ویلا = tm_now();
				rp->pid = لگاتار_umount(rp->mntpath,
					umount_دلیلاں, umount_دلیلاں_عدد);
			}
			عدد_اڈیک -= 1;
			state = STATE_REAP;
		}
		اكسر;
	افتراضي: اكسر;
	}

باہر_نکلو:
	حرر(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(دلیلاں.fstype_regex);
	REG_FREE(دلیلاں.skip_fstype_regex);
	REG_FREE(دلیلاں.گرہ_regex);
	REG_FREE(دلیلاں.skip_گرہ_regex);
	REG_FREE(دلیلاں.options_regex);
	REG_FREE(دلیلاں.skip_options_regex);
	REG_FREE(نقطہ_regex);
	REG_FREE(skip_نقطہ_regex);

	أرجع نتیجہ;
}
