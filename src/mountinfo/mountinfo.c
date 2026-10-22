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
#پایان_اگر

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

#تعریف_کن UMOUNT_آرگومانها_MAX 16
#تعریف_کن پیاپی_MAX 32
#تعریف_کن TRY_MAX 3
#تعریف_کن TRY_DELAY_MS 1000

بنية پیاپی_صف {
	ثابت حرف *mntpath;
	int64_t آخرین_exec_زمان;
	int64_t fuser_exec_زمان;
	pid_t pid;
	pid_t fuser_pid;
	صحيح try_شمار;
	صحيح fuser_stdoutfd;
};

تعريفنوع تعداد {
	mount_from,
	mount_مقصد,
	mount_fstype,
	mount_options
} mount_type;

تعريفنوع تعداد {
	net_ignore,
	net_yes,
	net_no
} net_opts;

بنية آرگومانها;
تعريفنوع صحيح process_func_t(RC_STRINGLIST *, بنية آرگومانها *,
    حرف *, حرف *, حرف *, حرف *, صحيح);

بنية آرگومانها {
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
	ثابت حرف *check_mntpath;
};

ساكن صحيح
process_mount(RC_STRINGLIST *list, بنية آرگومانها *آرگومانها,
    حرف *from, حرف *مقصد, حرف *fstype, حرف *options,
    صحيح netdev)
{
	حرف *p;
	RC_STRING *s;

	errno = ENOENT;

#اگر_تعریف_شده __linux__
	/* Skip the really silly rootfs */
	إذا (strcmp(fstype, "rootfs") == 0)
		أرجع -1;
#پایان_اگر

	إذا (آرگومانها->netdev == net_yes &&
	    (netdev != -1 || TAILQ_اولی(آرگومانها->mounts)))
	{
		إذا (netdev != 0)
			أرجع 1;
	} وإلا إذا (آرگومانها->netdev == net_no &&
	    (netdev != -1 || TAILQ_اولی(آرگومانها->mounts)))
	{
		إذا (netdev != 1)
			أرجع 1;
	} وإلا {
		إذا (آرگومانها->گره_regex &&
		    regexec(آرگومانها->گره_regex, from, 0, عدم, 0) != 0)
			أرجع 1;
		إذا (آرگومانها->skip_گره_regex &&
		    regexec(آرگومانها->skip_گره_regex, from, 0, عدم, 0) == 0)
			أرجع 1;

		إذا (آرگومانها->fstype_regex &&
		    regexec(آرگومانها->fstype_regex, fstype, 0, عدم, 0) != 0)
			أرجع -1;
		إذا (آرگومانها->skip_fstype_regex &&
		    regexec(آرگومانها->skip_fstype_regex, fstype, 0, عدم, 0) == 0)
			أرجع -1;

		إذا (آرگومانها->options_regex &&
		    regexec(آرگومانها->options_regex, options, 0, عدم, 0) != 0)
			أرجع -1;
		إذا (آرگومانها->skip_options_regex &&
		    regexec(آرگومانها->skip_options_regex, options, 0, عدم, 0) == 0)
			أرجع -1;
	}

	إذا (TAILQ_اولی(آرگومانها->mounts)) {
		TAILQ_FOREACH(s, آرگومانها->mounts, entries)
		    إذا (strcmp(s->مقدار, مقصد) == 0)
			    اكسر;
		إذا (!s)
			أرجع -1;
	}

	بدل (آرگومانها->mount_type) {
	حالة mount_from:
		p = from;
		اكسر;
	حالة mount_مقصد:
		p = مقصد;
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
		rc_stringlist_جمع_کن(list, p);
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
#اگر_تعریف_شده MNT_NOSYMFOLLOW
	{ MNT_NOSYMFOLLOW,  "nosymfollow" },
#پایان_اگر
	{ MNT_QUOTA,        "with quotas" },
	{ MNT_RDONLY,       "read-only" },
	{ MNT_SYNCHRONOUS,  "synchronous" },
	{ MNT_UNION,        "union" },
#اگر_تعریف_شده MNT_NOCLUSTERR
	{ MNT_NOCLUSTERR,   "noclusterr" },
#پایان_اگر
#اگر_تعریف_شده MNT_NOCLUSTERW
	{ MNT_NOCLUSTERW,   "noclusterw" },
#پایان_اگر
#اگر_تعریف_شده MNT_SUIDDIR
	{ MNT_SUIDDIR,      "suiddir" },
#پایان_اگر
	{ MNT_SOFTDEP,      "soft-updates" },
#اگر_تعریف_شده MNT_MULTILABEL
	{ MNT_MULTILABEL,   "multilabel" },
#پایان_اگر
#اگر_تعریف_شده MNT_ACLS
	{ MNT_ACLS,         "acls" },
#پایان_اگر
#اگر_تعریف_شده MNT_GJOURNAL
	{ MNT_GJOURNAL,     "gjournal" },
#پایان_اگر
	{ 0, عدم }
};

ساكن RC_STRINGLIST *
پیدا_کن_mounts(بنية آرگومانها *آرگومانها, size_t *رقم_mounts)
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

		*رقم_mounts += 0 == آرگومانها->process(list, آرگومانها,
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
پیدا_کن_mounts(بنية آرگومانها *آرگومانها, size_t *رقم_mounts)
{
	FILE *fp;
	حرف *میانگیر;
	size_t اندازه;
	حرف *p;
	حرف *from;
	حرف *مقصد;
	حرف *fst;
	حرف *opts;
	بنية mntent *ent;
	صحيح netdev;
	RC_STRINGLIST *list;

	إذا ((fp = fopen(procmounts, "r")) == عدم)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	میانگیر = عدم;
	طالما (xgetline(&میانگیر, &اندازه, fp) != -1) {
		netdev = -1;
		p = میانگیر;
		from = strsep(&p, " ");
		مقصد = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		إذا ((ent = getmntfile(مقصد))) {
			إذا (جستجوی_رشته(ent->mnt_opts, "_netdev"))
				netdev = 0;
			وإلا
				netdev = 1;
		}

		*رقم_mounts += 0 == آرگومانها->process(list, آرگومانها,
			from, مقصد, fst, opts, netdev);

		حرر(میانگیر);
		میانگیر = عدم;
	}
	حرر(میانگیر);
	fclose(fp);

	أرجع list;
}

#وإلا
#  error "Operating system not supported!"
#پایان_اگر

ساكن صحيح is_پیشوند(ثابت حرف *needle, ثابت حرف *hay)
{
	size_t nlen = طول_رشته(needle);
	إذا (مقایسه_رشته(needle, hay, nlen) == 0 && hay[nlen] == '/')
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

ساكن صحيح check_is_mounted(RC_STRINGLIST *list RC_استفادهنشده, بنية آرگومانها *آرگومانها,
    حرف *from RC_استفادهنشده, حرف *مقصد, حرف *fstype RC_استفادهنشده,
    حرف *options RC_استفادهنشده, صحيح netdev RC_استفادهنشده)
{
	أرجع strcmp(آرگومانها->check_mntpath, unescape_octal(مقصد)) == 0 ? 0 : -1;
}

ساكن صحيح is_mounted(ثابت حرف *mntpath)
{
	size_t رقم_mounts = 0;
	بنية آرگومانها آرگومانها = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = پیدا_کن_mounts(&آرگومانها, &رقم_mounts);
	rc_stringlist_free(l);
	أرجع رقم_mounts > 0;
}

ساكن pid_t پیاپی_umount(ثابت حرف *mntpath,
	ثابت حرف **umount_آرگومانها, صحيح umount_آرگومانها_رقم)
{
	بنية exec_آرگومانها آرگومانها;
	بنية exec_نتیجه res;
	ثابت حرف *argv[UMOUNT_آرگومانها_MAX + 3];
	صحيح k, i = 0;

	argv[i++] = "umount";
	لكل (k = 0; k < umount_آرگومانها_رقم; ++k)
		argv[i++] = umount_آرگومانها[k];
	argv[i++] = mntpath;
	argv[i++] = عدم;

	آرگومانها = exec_init(argv);
	آرگومانها.redirect_stdout = آرگومانها.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&آرگومانها);
	إذا (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	أرجع res.pid;
}

ساكن فراغ fuser_پیاپی(بنية پیاپی_صف *rp, ثابت حرف *fuser_opt)
{
	ساكن صحيح fuser_exec_failed = 0;
	ثابت حرف *argv[] = { "fuser", fuser_opt, rp->mntpath, عدم };
	بنية exec_نتیجه res;
	بنية exec_آرگومانها آرگومانها;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	إذا (fuser_exec_failed)
		أرجع;

	آرگومانها = exec_init(argv);
	آرگومانها.redirect_stdout = EXEC_MKPIPE;
	آرگومانها.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&آرگومانها);
	إذا (res.pid < 0) {
		fuser_exec_failed = 1;
	} وإلا {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_زمان = tm_now();
	}
}

ساكن صحيح fuser_decide(بنية پیاپی_صف *rp,
	ثابت حرف *fuser_opt, ثابت حرف *fuser_kill_پیشوند)
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
	snprintf(selfpid, حجم selfpid, " %lld ", (دراز دراز)getpid());

	إذا (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	إذا (جستجوی_رشته(buf, selfpid) || (rc_pid && جستجوی_رشته(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		أرجع -1;
	} وإلا إذا (strcmp(buf, "  ") == 0) {
		إذا (rp->try_شمار >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			أرجع -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_شمار = TRY_MAX;
		أرجع 0;
	} وإلا {
		حرف sig[32];
		ثابت حرف *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, عدم
		};
		بنية exec_نتیجه res;
		بنية exec_آرگومانها آرگومانها = exec_init(argv);
		آرگومانها.redirect_stdout = آرگومانها.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, حجم sig, "%s%s", fuser_kill_پیشوند,
			rp->try_شمار == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&آرگومانها);
		إذا (res.pid > 0)
			rc_waitpid(res.pid);
		أرجع 0;
	}
}

ساكن regex_t *
get_regex(ثابت حرف *string)
{
	regex_t *reg = xmalloc(حجم (*reg));
	صحيح نتیجه;
	حرف میانگیر[256];

	إذا ((نتیجه = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(نتیجه, reg, میانگیر, حجم(میانگیر));
		eerrorx("%s: invalid regex `%s'", applet, میانگیر);
	}

	أرجع reg;
}

صحيح اصلی(صحيح argc, حرف **argv)
{
	بنية آرگومانها آرگومانها;
	regex_t *نقطه_regex = عدم;
	regex_t *skip_نقطه_regex = عدم;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	حرف *real_مسیر = عدم;
	صحيح opt;
	صحيح نتیجه;
	حرف *this_مسیر, *argv0;
	ثابت حرف *tmps;
	ثابت حرف *fuser_opt, *fuser_kill_پیشوند;
	size_t رقم_mounts = 0;
	size_t unmount_شاخص;
	صحيح doing_unmount = 0;
	pid_t pid;
	صحيح وضعیت, flags;
	int64_t tmp, بعدی_retry, now;
	int64_t rc_fuser_timeout = -1;
	ثابت حرف *umount_آرگومانها[UMOUNT_آرگومانها_MAX];
	صحيح umount_آرگومانها_رقم = 0;
	ثابت حرف **mounts = عدم;
	بنية پیاپی_صف running[پیاپی_MAX] = {0};
	بنية پیاپی_صف *rp;
	size_t رقم_running = 0, رقم_انتظار = 0;
	تعداد { STATE_پیاپی, STATE_REAP, STATE_RETRY, STATE_انتها } state;

#تعریف_کن DO_REG(_var)							      \
	إذا (_var) حرر(_var);						      \
	_var = get_regex(optarg);
#تعریف_کن REG_FREE(_var)							      \
	إذا (_var) { regfree(_var); حرر(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&آرگومانها, 0, حجم(آرگومانها));
	آرگومانها.mount_type = mount_مقصد;
	آرگومانها.netdev = net_ignore;
	آرگومانها.mounts = rc_stringlist_new();
	آرگومانها.process = process_mount;

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
			إذا (umount_آرگومانها_رقم >= (صحيح)ARRAY_اندازه(umount_آرگومانها))
				eerrorx("%s: Too many umount arguments", applet);
			umount_آرگومانها[umount_آرگومانها_رقم++] = tmps;
		}

		tmps = rc_conf_مقدار("rc_fuser_timeout");
		إذا (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		إذا (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		إذا (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_پیشوند = "-";
		} وإلا {
			fuser_opt = "-cm";
			fuser_kill_پیشوند = "-s";
		}
	}

	طالما ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
		حالة 'e':
			آرگومانها.netdev = net_yes;
			اكسر;
		حالة 'E':
			آرگومانها.netdev = net_no;
			اكسر;
		حالة 'f':
			DO_REG(آرگومانها.fstype_regex);
			اكسر;
		حالة 'F':
			DO_REG(آرگومانها.skip_fstype_regex);
			اكسر;
		حالة 'n':
			DO_REG(آرگومانها.گره_regex);
			اكسر;
		حالة 'N':
			DO_REG(آرگومانها.skip_گره_regex);
			اكسر;
		حالة 'o':
			DO_REG(آرگومانها.options_regex);
			اكسر;
		حالة 'O':
			DO_REG(آرگومانها.skip_options_regex);
			اكسر;
		حالة 'p':
			DO_REG(نقطه_regex);
			اكسر;
		حالة 'P':
			DO_REG(skip_نقطه_regex);
			اكسر;
		حالة 'i':
			آرگومانها.mount_type = mount_options;
			اكسر;
		حالة 's':
			آرگومانها.mount_type = mount_fstype;
			اكسر;
		حالة 't':
			آرگومانها.mount_type = mount_from;
			اكسر;

		case_RC_COMMON_GETOPT
		}
	}

	طالما (optind < argc) {
		إذا (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_مسیر = argv[optind++];
		real_مسیر = realpath(this_مسیر, عدم);
		إذا (real_مسیر)
			this_مسیر = real_مسیر;
		rc_stringlist_جمع_کن(آرگومانها.mounts, this_مسیر);
		حرر(real_مسیر);
		real_مسیر = عدم;
	}
	nodes = پیدا_کن_mounts(&آرگومانها, &رقم_mounts);
	rc_stringlist_free(آرگومانها.mounts);

	إذا (doing_unmount)
		mounts = xmalloc(رقم_mounts * حجم(*mounts));
	رقم_mounts = 0;
	نتیجه = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_معکوس(s, nodes, rc_stringlist, entries) {
		إذا (نقطه_regex &&
		    regexec(نقطه_regex, s->مقدار, 0, عدم, 0) != 0)
			استمر;
		إذا (skip_نقطه_regex &&
		    regexec(skip_نقطه_regex, s->مقدار, 0, عدم, 0) == 0)
			استمر;
		إذا (doing_unmount)
			mounts[رقم_mounts++] = unescape_octal(s->مقدار);
		وإلا إذا (!rc_yesno(getenv("EINFO_QUIET")))
			چاپ_کن("%s\n", s->مقدار);
		نتیجه = EXIT_SUCCESS;
	}
	إذا (!doing_unmount)
		اذهب خروج;

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
	نتیجه = EXIT_SUCCESS;
	state = STATE_پیاپی;
	طالما (state != STATE_انتها) بدل (state) {
	حالة STATE_پیاپی:
		لكل (unmount_شاخص = 0; unmount_شاخص < رقم_mounts; ++unmount_شاخص) {
			ثابت حرف *candidate = mounts[unmount_شاخص];
			صحيح امن_مقصد_unmount = 1;
			لكل (size_t k = 0; امن_مقصد_unmount && k < رقم_running; ++k)
				امن_مقصد_unmount = !is_پیشوند(candidate, running[k].mntpath);
			لكل (size_t k = 0; امن_مقصد_unmount && k < رقم_mounts; ++k)
				امن_مقصد_unmount = !is_پیشوند(candidate, mounts[k]);
			إذا (!امن_مقصد_unmount)
				استمر;
			إذا (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_شاخص--] = mounts[--رقم_mounts];
			} وإلا {
				اكسر;
			}
		}
		إذا (رقم_running == پیاپی_MAX || unmount_شاخص >= رقم_mounts) {
			state = STATE_REAP;
			اكسر;
		}
		rp = running + رقم_running++;
		rp->mntpath = mounts[unmount_شاخص];
		rp->آخرین_exec_زمان = tm_now();
		rp->try_شمار = 0;
		rp->pid = پیاپی_umount(rp->mntpath, umount_آرگومانها, umount_آرگومانها_رقم);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_زمان = -1;
		mounts[unmount_شاخص] = mounts[--رقم_mounts];
		اكسر;
	حالة STATE_REAP:
		flags = (رقم_انتظار > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &وضعیت, flags);
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
			إذا ((WIFEXITED(وضعیت) && WEXITSTATUS(وضعیت) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--رقم_running];
				state = STATE_پیاپی;
			} وإلا إذا (rp->try_شمار >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--رقم_running];
				نتیجه = EXIT_FAILURE;
			} وإلا { /* put into waiting queue */
				rp->pid = -1;
				rp->try_شمار += 1;
				رقم_انتظار += 1;
				fuser_پیاپی(rp, fuser_opt);
			}
		} وإلا {
			state = STATE_RETRY;
		}
		اكسر;
	حالة STATE_RETRY:
		rp = عدم;
		بعدی_retry = INT64_MAX;
		لكل (size_t i = 0; i < رقم_running; ++i) {
			إذا (running[i].pid > 0)
				استمر;
			إذا (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_زمان + rc_fuser_timeout;
			وإلا
				tmp = running[i].آخرین_exec_زمان + TRY_DELAY_MS;
			إذا (tmp < بعدی_retry) {
				rp = running + i;
				بعدی_retry = tmp;
			}
		}
		إذا (!rp) {
			state = (رقم_mounts > 0) ? STATE_پیاپی : STATE_انتها;
			اكسر;
		}
		now = tm_now();
		إذا ((rp->pid > 0 || rp->fuser_pid > 0) && بعدی_retry > now) {
			int64_t sleep_for = بعدی_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			إذا (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		إذا ((rp->pid == -1 && rp->fuser_pid == -1) || بعدی_retry <= now) {
			إذا (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, عدم, 0);
				rp->fuser_pid = -1;
			}
			إذا (fuser_decide(rp, fuser_opt, fuser_kill_پیشوند) < 0) { /* abort */
				*rp = running[--رقم_running];
				نتیجه = EXIT_FAILURE;
			} وإلا { /* retry */
				rp->آخرین_exec_زمان = tm_now();
				rp->pid = پیاپی_umount(rp->mntpath,
					umount_آرگومانها, umount_آرگومانها_رقم);
			}
			رقم_انتظار -= 1;
			state = STATE_REAP;
		}
		اكسر;
	افتراضي: اكسر;
	}

خروج:
	حرر(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(آرگومانها.fstype_regex);
	REG_FREE(آرگومانها.skip_fstype_regex);
	REG_FREE(آرگومانها.گره_regex);
	REG_FREE(آرگومانها.skip_گره_regex);
	REG_FREE(آرگومانها.options_regex);
	REG_FREE(آرگومانها.skip_options_regex);
	REG_FREE(نقطه_regex);
	REG_FREE(skip_نقطه_regex);

	أرجع نتیجه;
}
