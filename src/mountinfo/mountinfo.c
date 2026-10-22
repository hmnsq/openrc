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
#د_که_پای

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

#تعریف_کړه UMOUNT_دلیلونه_MAX 16
#تعریف_کړه پرلپسې_MAX 32
#تعریف_کړه TRY_MAX 3
#تعریف_کړه TRY_DELAY_MS 1000

بنية پرلپسې_کتار {
	ثابت حرف *mntpath;
	int64_t وروستی_exec_وخت;
	int64_t fuser_exec_وخت;
	pid_t pid;
	pid_t fuser_pid;
	صحيح try_شمېر;
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

بنية دلیلونه;
تعريفنوع صحيح process_func_t(RC_STRINGLIST *, بنية دلیلونه *,
    حرف *, حرف *, حرف *, حرف *, صحيح);

بنية دلیلونه {
	process_func_t *process;
	regex_t *غوټه_regex;
	regex_t *skip_غوټه_regex;
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
process_mount(RC_STRINGLIST *list, بنية دلیلونه *دلیلونه,
    حرف *from, حرف *منزل, حرف *fstype, حرف *options,
    صحيح netdev)
{
	حرف *p;
	RC_STRING *s;

	errno = ENOENT;

#که_تعریف_شوی __linux__
	/* Skip the really silly rootfs */
	إذا (strcmp(fstype, "rootfs") == 0)
		أرجع -1;
#د_که_پای

	إذا (دلیلونه->netdev == net_yes &&
	    (netdev != -1 || TAILQ_لومړی(دلیلونه->mounts)))
	{
		إذا (netdev != 0)
			أرجع 1;
	} وإلا إذا (دلیلونه->netdev == net_no &&
	    (netdev != -1 || TAILQ_لومړی(دلیلونه->mounts)))
	{
		إذا (netdev != 1)
			أرجع 1;
	} وإلا {
		إذا (دلیلونه->غوټه_regex &&
		    regexec(دلیلونه->غوټه_regex, from, 0, عدم, 0) != 0)
			أرجع 1;
		إذا (دلیلونه->skip_غوټه_regex &&
		    regexec(دلیلونه->skip_غوټه_regex, from, 0, عدم, 0) == 0)
			أرجع 1;

		إذا (دلیلونه->fstype_regex &&
		    regexec(دلیلونه->fstype_regex, fstype, 0, عدم, 0) != 0)
			أرجع -1;
		إذا (دلیلونه->skip_fstype_regex &&
		    regexec(دلیلونه->skip_fstype_regex, fstype, 0, عدم, 0) == 0)
			أرجع -1;

		إذا (دلیلونه->options_regex &&
		    regexec(دلیلونه->options_regex, options, 0, عدم, 0) != 0)
			أرجع -1;
		إذا (دلیلونه->skip_options_regex &&
		    regexec(دلیلونه->skip_options_regex, options, 0, عدم, 0) == 0)
			أرجع -1;
	}

	إذا (TAILQ_لومړی(دلیلونه->mounts)) {
		TAILQ_FOREACH(s, دلیلونه->mounts, entries)
		    إذا (strcmp(s->ارزښت, منزل) == 0)
			    اكسر;
		إذا (!s)
			أرجع -1;
	}

	بدل (دلیلونه->mount_type) {
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
		rc_stringlist_جمع_کړه(list, p);
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
#که_تعریف_شوی MNT_NOSYMFOLLOW
	{ MNT_NOSYMFOLLOW,  "nosymfollow" },
#د_که_پای
	{ MNT_QUOTA,        "with quotas" },
	{ MNT_RDONLY,       "read-only" },
	{ MNT_SYNCHRONOUS,  "synchronous" },
	{ MNT_UNION,        "union" },
#که_تعریف_شوی MNT_NOCLUSTERR
	{ MNT_NOCLUSTERR,   "noclusterr" },
#د_که_پای
#که_تعریف_شوی MNT_NOCLUSTERW
	{ MNT_NOCLUSTERW,   "noclusterw" },
#د_که_پای
#که_تعریف_شوی MNT_SUIDDIR
	{ MNT_SUIDDIR,      "suiddir" },
#د_که_پای
	{ MNT_SOFTDEP,      "soft-updates" },
#که_تعریف_شوی MNT_MULTILABEL
	{ MNT_MULTILABEL,   "multilabel" },
#د_که_پای
#که_تعریف_شوی MNT_ACLS
	{ MNT_ACLS,         "acls" },
#د_که_پای
#که_تعریف_شوی MNT_GJOURNAL
	{ MNT_GJOURNAL,     "gjournal" },
#د_که_پای
	{ 0, عدم }
};

ساكن RC_STRINGLIST *
پیدا_کړه_mounts(بنية دلیلونه *دلیلونه, size_t *عدد_mounts)
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

		*عدد_mounts += 0 == دلیلونه->process(list, دلیلونه,
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
پیدا_کړه_mounts(بنية دلیلونه *دلیلونه, size_t *عدد_mounts)
{
	FILE *fp;
	حرف *بفر;
	size_t اندازه;
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
	طالما (xgetline(&بفر, &اندازه, fp) != -1) {
		netdev = -1;
		p = بفر;
		from = strsep(&p, " ");
		منزل = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		إذا ((ent = getmntfile(منزل))) {
			إذا (مزي_پلټنه(ent->mnt_opts, "_netdev"))
				netdev = 0;
			وإلا
				netdev = 1;
		}

		*عدد_mounts += 0 == دلیلونه->process(list, دلیلونه,
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
#د_که_پای

ساكن صحيح is_مختاړی(ثابت حرف *needle, ثابت حرف *hay)
{
	size_t nlen = مزي_اوږدوالی(needle);
	إذا (مزي_پرتله(needle, hay, nlen) == 0 && hay[nlen] == '/')
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

ساكن صحيح check_is_mounted(RC_STRINGLIST *list RC_ناکارول_شوی, بنية دلیلونه *دلیلونه,
    حرف *from RC_ناکارول_شوی, حرف *منزل, حرف *fstype RC_ناکارول_شوی,
    حرف *options RC_ناکارول_شوی, صحيح netdev RC_ناکارول_شوی)
{
	أرجع strcmp(دلیلونه->check_mntpath, unescape_octal(منزل)) == 0 ? 0 : -1;
}

ساكن صحيح is_mounted(ثابت حرف *mntpath)
{
	size_t عدد_mounts = 0;
	بنية دلیلونه دلیلونه = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = پیدا_کړه_mounts(&دلیلونه, &عدد_mounts);
	rc_stringlist_free(l);
	أرجع عدد_mounts > 0;
}

ساكن pid_t پرلپسې_umount(ثابت حرف *mntpath,
	ثابت حرف **umount_دلیلونه, صحيح umount_دلیلونه_عدد)
{
	بنية exec_دلیلونه دلیلونه;
	بنية exec_پایله res;
	ثابت حرف *argv[UMOUNT_دلیلونه_MAX + 3];
	صحيح k, i = 0;

	argv[i++] = "umount";
	لكل (k = 0; k < umount_دلیلونه_عدد; ++k)
		argv[i++] = umount_دلیلونه[k];
	argv[i++] = mntpath;
	argv[i++] = عدم;

	دلیلونه = exec_init(argv);
	دلیلونه.redirect_stdout = دلیلونه.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&دلیلونه);
	إذا (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	أرجع res.pid;
}

ساكن فراغ fuser_پرلپسې(بنية پرلپسې_کتار *rp, ثابت حرف *fuser_opt)
{
	ساكن صحيح fuser_exec_failed = 0;
	ثابت حرف *argv[] = { "fuser", fuser_opt, rp->mntpath, عدم };
	بنية exec_پایله res;
	بنية exec_دلیلونه دلیلونه;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	إذا (fuser_exec_failed)
		أرجع;

	دلیلونه = exec_init(argv);
	دلیلونه.redirect_stdout = EXEC_MKPIPE;
	دلیلونه.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&دلیلونه);
	إذا (res.pid < 0) {
		fuser_exec_failed = 1;
	} وإلا {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_وخت = tm_now();
	}
}

ساكن صحيح fuser_decide(بنية پرلپسې_کتار *rp,
	ثابت حرف *fuser_opt, ثابت حرف *fuser_kill_مختاړی)
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
	snprintf(selfpid, حجم selfpid, " %lld ", (اوږد اوږد)getpid());

	إذا (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	إذا (مزي_پلټنه(buf, selfpid) || (rc_pid && مزي_پلټنه(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		أرجع -1;
	} وإلا إذا (strcmp(buf, "  ") == 0) {
		إذا (rp->try_شمېر >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			أرجع -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_شمېر = TRY_MAX;
		أرجع 0;
	} وإلا {
		حرف sig[32];
		ثابت حرف *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, عدم
		};
		بنية exec_پایله res;
		بنية exec_دلیلونه دلیلونه = exec_init(argv);
		دلیلونه.redirect_stdout = دلیلونه.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, حجم sig, "%s%s", fuser_kill_مختاړی,
			rp->try_شمېر == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&دلیلونه);
		إذا (res.pid > 0)
			rc_waitpid(res.pid);
		أرجع 0;
	}
}

ساكن regex_t *
get_regex(ثابت حرف *string)
{
	regex_t *reg = xmalloc(حجم (*reg));
	صحيح پایله;
	حرف بفر[256];

	إذا ((پایله = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(پایله, reg, بفر, حجم(بفر));
		eerrorx("%s: invalid regex `%s'", applet, بفر);
	}

	أرجع reg;
}

صحيح اصلي(صحيح argc, حرف **argv)
{
	بنية دلیلونه دلیلونه;
	regex_t *ټکی_regex = عدم;
	regex_t *skip_ټکی_regex = عدم;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	حرف *real_لار = عدم;
	صحيح opt;
	صحيح پایله;
	حرف *this_لار, *argv0;
	ثابت حرف *tmps;
	ثابت حرف *fuser_opt, *fuser_kill_مختاړی;
	size_t عدد_mounts = 0;
	size_t unmount_شاخص;
	صحيح doing_unmount = 0;
	pid_t pid;
	صحيح حالت, flags;
	int64_t tmp, بل_retry, now;
	int64_t rc_fuser_timeout = -1;
	ثابت حرف *umount_دلیلونه[UMOUNT_دلیلونه_MAX];
	صحيح umount_دلیلونه_عدد = 0;
	ثابت حرف **mounts = عدم;
	بنية پرلپسې_کتار running[پرلپسې_MAX] = {0};
	بنية پرلپسې_کتار *rp;
	size_t عدد_running = 0, عدد_انتظار = 0;
	تعداد { STATE_پرلپسې, STATE_REAP, STATE_RETRY, STATE_پای_ټکی } state;

#تعریف_کړه DO_REG(_var)							      \
	إذا (_var) حرر(_var);						      \
	_var = get_regex(optarg);
#تعریف_کړه REG_FREE(_var)							      \
	إذا (_var) { regfree(_var); حرر(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&دلیلونه, 0, حجم(دلیلونه));
	دلیلونه.mount_type = mount_منزل;
	دلیلونه.netdev = net_ignore;
	دلیلونه.mounts = rc_stringlist_new();
	دلیلونه.process = process_mount;

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
			إذا (umount_دلیلونه_عدد >= (صحيح)ARRAY_اندازه(umount_دلیلونه))
				eerrorx("%s: Too many umount arguments", applet);
			umount_دلیلونه[umount_دلیلونه_عدد++] = tmps;
		}

		tmps = rc_conf_ارزښت("rc_fuser_timeout");
		إذا (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		إذا (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		إذا (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_مختاړی = "-";
		} وإلا {
			fuser_opt = "-cm";
			fuser_kill_مختاړی = "-s";
		}
	}

	طالما ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
		حالة 'e':
			دلیلونه.netdev = net_yes;
			اكسر;
		حالة 'E':
			دلیلونه.netdev = net_no;
			اكسر;
		حالة 'f':
			DO_REG(دلیلونه.fstype_regex);
			اكسر;
		حالة 'F':
			DO_REG(دلیلونه.skip_fstype_regex);
			اكسر;
		حالة 'n':
			DO_REG(دلیلونه.غوټه_regex);
			اكسر;
		حالة 'N':
			DO_REG(دلیلونه.skip_غوټه_regex);
			اكسر;
		حالة 'o':
			DO_REG(دلیلونه.options_regex);
			اكسر;
		حالة 'O':
			DO_REG(دلیلونه.skip_options_regex);
			اكسر;
		حالة 'p':
			DO_REG(ټکی_regex);
			اكسر;
		حالة 'P':
			DO_REG(skip_ټکی_regex);
			اكسر;
		حالة 'i':
			دلیلونه.mount_type = mount_options;
			اكسر;
		حالة 's':
			دلیلونه.mount_type = mount_fstype;
			اكسر;
		حالة 't':
			دلیلونه.mount_type = mount_from;
			اكسر;

		case_RC_COMMON_GETOPT
		}
	}

	طالما (optind < argc) {
		إذا (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_لار = argv[optind++];
		real_لار = realpath(this_لار, عدم);
		إذا (real_لار)
			this_لار = real_لار;
		rc_stringlist_جمع_کړه(دلیلونه.mounts, this_لار);
		حرر(real_لار);
		real_لار = عدم;
	}
	nodes = پیدا_کړه_mounts(&دلیلونه, &عدد_mounts);
	rc_stringlist_free(دلیلونه.mounts);

	إذا (doing_unmount)
		mounts = xmalloc(عدد_mounts * حجم(*mounts));
	عدد_mounts = 0;
	پایله = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_سرچپه(s, nodes, rc_stringlist, entries) {
		إذا (ټکی_regex &&
		    regexec(ټکی_regex, s->ارزښت, 0, عدم, 0) != 0)
			استمر;
		إذا (skip_ټکی_regex &&
		    regexec(skip_ټکی_regex, s->ارزښت, 0, عدم, 0) == 0)
			استمر;
		إذا (doing_unmount)
			mounts[عدد_mounts++] = unescape_octal(s->ارزښت);
		وإلا إذا (!rc_yesno(getenv("EINFO_QUIET")))
			چاپ_کړه("%s\n", s->ارزښت);
		پایله = EXIT_SUCCESS;
	}
	إذا (!doing_unmount)
		اذهب وتل;

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
	پایله = EXIT_SUCCESS;
	state = STATE_پرلپسې;
	طالما (state != STATE_پای_ټکی) بدل (state) {
	حالة STATE_پرلپسې:
		لكل (unmount_شاخص = 0; unmount_شاخص < عدد_mounts; ++unmount_شاخص) {
			ثابت حرف *candidate = mounts[unmount_شاخص];
			صحيح خوندي_منزل_unmount = 1;
			لكل (size_t k = 0; خوندي_منزل_unmount && k < عدد_running; ++k)
				خوندي_منزل_unmount = !is_مختاړی(candidate, running[k].mntpath);
			لكل (size_t k = 0; خوندي_منزل_unmount && k < عدد_mounts; ++k)
				خوندي_منزل_unmount = !is_مختاړی(candidate, mounts[k]);
			إذا (!خوندي_منزل_unmount)
				استمر;
			إذا (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_شاخص--] = mounts[--عدد_mounts];
			} وإلا {
				اكسر;
			}
		}
		إذا (عدد_running == پرلپسې_MAX || unmount_شاخص >= عدد_mounts) {
			state = STATE_REAP;
			اكسر;
		}
		rp = running + عدد_running++;
		rp->mntpath = mounts[unmount_شاخص];
		rp->وروستی_exec_وخت = tm_now();
		rp->try_شمېر = 0;
		rp->pid = پرلپسې_umount(rp->mntpath, umount_دلیلونه, umount_دلیلونه_عدد);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_وخت = -1;
		mounts[unmount_شاخص] = mounts[--عدد_mounts];
		اكسر;
	حالة STATE_REAP:
		flags = (عدد_انتظار > 0) ? WNOHANG : 0;
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
				state = STATE_پرلپسې;
			} وإلا إذا (rp->try_شمېر >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--عدد_running];
				پایله = EXIT_FAILURE;
			} وإلا { /* put into waiting queue */
				rp->pid = -1;
				rp->try_شمېر += 1;
				عدد_انتظار += 1;
				fuser_پرلپسې(rp, fuser_opt);
			}
		} وإلا {
			state = STATE_RETRY;
		}
		اكسر;
	حالة STATE_RETRY:
		rp = عدم;
		بل_retry = INT64_MAX;
		لكل (size_t i = 0; i < عدد_running; ++i) {
			إذا (running[i].pid > 0)
				استمر;
			إذا (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_وخت + rc_fuser_timeout;
			وإلا
				tmp = running[i].وروستی_exec_وخت + TRY_DELAY_MS;
			إذا (tmp < بل_retry) {
				rp = running + i;
				بل_retry = tmp;
			}
		}
		إذا (!rp) {
			state = (عدد_mounts > 0) ? STATE_پرلپسې : STATE_پای_ټکی;
			اكسر;
		}
		now = tm_now();
		إذا ((rp->pid > 0 || rp->fuser_pid > 0) && بل_retry > now) {
			int64_t sleep_for = بل_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			إذا (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		إذا ((rp->pid == -1 && rp->fuser_pid == -1) || بل_retry <= now) {
			إذا (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, عدم, 0);
				rp->fuser_pid = -1;
			}
			إذا (fuser_decide(rp, fuser_opt, fuser_kill_مختاړی) < 0) { /* abort */
				*rp = running[--عدد_running];
				پایله = EXIT_FAILURE;
			} وإلا { /* retry */
				rp->وروستی_exec_وخت = tm_now();
				rp->pid = پرلپسې_umount(rp->mntpath,
					umount_دلیلونه, umount_دلیلونه_عدد);
			}
			عدد_انتظار -= 1;
			state = STATE_REAP;
		}
		اكسر;
	افتراضي: اكسر;
	}

وتل:
	حرر(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(دلیلونه.fstype_regex);
	REG_FREE(دلیلونه.skip_fstype_regex);
	REG_FREE(دلیلونه.غوټه_regex);
	REG_FREE(دلیلونه.skip_غوټه_regex);
	REG_FREE(دلیلونه.options_regex);
	REG_FREE(دلیلونه.skip_options_regex);
	REG_FREE(ټکی_regex);
	REG_FREE(skip_ټکی_regex);

	أرجع پایله;
}
