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
#وگرنہ_اگر defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#وگرنہ_اگر defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#وگرنہ_اگر defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#اگر_ختم

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

ثابت حرف *applet = تہی;
ثابت حرف *procmounts = "/proc/mounts";
ثابت حرف *extraopts = "[mount1] [mount2] ...";
ثابت حرف getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{ "fstype-regex",        1, تہی, 'f'},
	{ "skip-fstype-regex",   1, تہی, 'F'},
	{ "node-regex",          1, تہی, 'n'},
	{ "skip-node-regex",     1, تہی, 'N'},
	{ "options-regex",       1, تہی, 'o'},
	{ "skip-options-regex",  1, تہی, 'O'},
	{ "point-regex",         1, تہی, 'p'},
	{ "skip-point-regex",    1, تہی, 'P'},
	{ "options",             0, تہی, 'i'},
	{ "fstype",              0, تہی, 's'},
	{ "node",                0, تہی, 't'},
	{ "netdev",              0, تہی, 'e'},
	{ "nonetdev",            0, تہی, 'E'},
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
ثابت حرف *usagestring = تہی;

#تعریف_کرو UMOUNT_دلائل_MAX 16
#تعریف_کرو تسلسل_MAX 32
#تعریف_کرو TRY_MAX 3
#تعریف_کرو TRY_DELAY_MS 1000

ساخت تسلسل_قطار {
	ثابت حرف *mntpath;
	int64_t آخری_exec_وقت;
	int64_t fuser_exec_وقت;
	pid_t pid;
	pid_t fuser_pid;
	صحیح try_گنتی;
	صحیح fuser_stdoutfd;
};

قسمتعریف شمار {
	mount_from,
	mount_مقصد,
	mount_fstype,
	mount_options
} mount_type;

قسمتعریف شمار {
	net_ignore,
	net_yes,
	net_no
} net_opts;

ساخت دلائل;
قسمتعریف صحیح process_func_t(RC_STRINGLIST *, ساخت دلائل *,
    حرف *, حرف *, حرف *, حرف *, صحیح);

ساخت دلائل {
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

ساکن صحیح
process_mount(RC_STRINGLIST *list, ساخت دلائل *دلائل,
    حرف *from, حرف *مقصد, حرف *fstype, حرف *options,
    صحیح netdev)
{
	حرف *p;
	RC_STRING *s;

	errno = ENOENT;

#اگر_تعریف_شدہ __linux__
	/* Skip the really silly rootfs */
	اگر (strcmp(fstype, "rootfs") == 0)
		واپس -1;
#اگر_ختم

	اگر (دلائل->netdev == net_yes &&
	    (netdev != -1 || TAILQ_پہلا(دلائل->mounts)))
	{
		اگر (netdev != 0)
			واپس 1;
	} ورنہ اگر (دلائل->netdev == net_no &&
	    (netdev != -1 || TAILQ_پہلا(دلائل->mounts)))
	{
		اگر (netdev != 1)
			واپس 1;
	} ورنہ {
		اگر (دلائل->گرہ_regex &&
		    regexec(دلائل->گرہ_regex, from, 0, تہی, 0) != 0)
			واپس 1;
		اگر (دلائل->skip_گرہ_regex &&
		    regexec(دلائل->skip_گرہ_regex, from, 0, تہی, 0) == 0)
			واپس 1;

		اگر (دلائل->fstype_regex &&
		    regexec(دلائل->fstype_regex, fstype, 0, تہی, 0) != 0)
			واپس -1;
		اگر (دلائل->skip_fstype_regex &&
		    regexec(دلائل->skip_fstype_regex, fstype, 0, تہی, 0) == 0)
			واپس -1;

		اگر (دلائل->options_regex &&
		    regexec(دلائل->options_regex, options, 0, تہی, 0) != 0)
			واپس -1;
		اگر (دلائل->skip_options_regex &&
		    regexec(دلائل->skip_options_regex, options, 0, تہی, 0) == 0)
			واپس -1;
	}

	اگر (TAILQ_پہلا(دلائل->mounts)) {
		TAILQ_FOREACH(s, دلائل->mounts, entries)
		    اگر (strcmp(s->قدر, مقصد) == 0)
			    توڑو;
		اگر (!s)
			واپس -1;
	}

	انتخاب (دلائل->mount_type) {
	حالت mount_from:
		p = from;
		توڑو;
	حالت mount_مقصد:
		p = مقصد;
		توڑو;
	حالت mount_fstype:
		p = fstype;
		توڑو;
	حالت mount_options:
		p = options;
		توڑو;
	طےشدہ:
		p = تہی;
		errno = EINVAL;
		توڑو;
	}

	اگر (p) {
		errno = 0;
		rc_stringlist_جوڑو(list, p);
		واپس 0;
	}

	واپس -1;
}

#اگر defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
ساکن ساخت opt {
	صحیح o_opt;
	ثابت حرف *o_name;
} optnames[] = {
	{ MNT_ASYNC,        "asynchronous" },
	{ MNT_EXPORTED,     "NFS exported" },
	{ MNT_LOCAL,        "local" },
	{ MNT_NOATIME,      "noatime" },
	{ MNT_NOEXEC,       "noexec" },
	{ MNT_NOSUID,       "nosuid" },
#اگر_تعریف_شدہ MNT_NOSYMFOLLOW
	{ MNT_NOSYMFOLLOW,  "nosymfollow" },
#اگر_ختم
	{ MNT_QUOTA,        "with quotas" },
	{ MNT_RDONLY,       "read-only" },
	{ MNT_SYNCHRONOUS,  "synchronous" },
	{ MNT_UNION,        "union" },
#اگر_تعریف_شدہ MNT_NOCLUSTERR
	{ MNT_NOCLUSTERR,   "noclusterr" },
#اگر_ختم
#اگر_تعریف_شدہ MNT_NOCLUSTERW
	{ MNT_NOCLUSTERW,   "noclusterw" },
#اگر_ختم
#اگر_تعریف_شدہ MNT_SUIDDIR
	{ MNT_SUIDDIR,      "suiddir" },
#اگر_ختم
	{ MNT_SOFTDEP,      "soft-updates" },
#اگر_تعریف_شدہ MNT_MULTILABEL
	{ MNT_MULTILABEL,   "multilabel" },
#اگر_ختم
#اگر_تعریف_شدہ MNT_ACLS
	{ MNT_ACLS,         "acls" },
#اگر_ختم
#اگر_تعریف_شدہ MNT_GJOURNAL
	{ MNT_GJOURNAL,     "gjournal" },
#اگر_ختم
	{ 0, تہی }
};

ساکن RC_STRINGLIST *
ڈھونڈو_mounts(ساخت دلائل *دلائل, size_t *عدد_mounts)
{
	ساخت statfs *mnts;
	صحیح nmnts;
	صحیح i;
	RC_STRINGLIST *list;
	حرف *options = تہی;
	uint64_t flags;
	ساخت opt *o;
	صحیح netdev;
	حرف *tmp;

	اگر ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	برائے (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		برائے (o = optnames; flags && o->o_opt; o++) {
			اگر (flags & o->o_opt) {
				اگر (o->o_opt == MNT_LOCAL)
					netdev = 1;
				اگر (!options)
					options = xstrdup(o->o_name);
				ورنہ {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					آزاد(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*عدد_mounts += 0 == دلائل->process(list, دلائل,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		آزاد(options);
		options = تہی;
	}

	واپس list;
}

#وگرنہ_اگر defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
ساکن ساخت mntent *
getmntfile(ثابت حرف *file)
{
	ساخت mntent *ent = تہی;
	FILE *fp;

	اگر (!(fp = setmntent("/etc/fstab", "r")))
		واپس تہی;
	جب ((ent = getmntent(fp)))
		اگر (strcmp(file, ent->mnt_dir) == 0)
			توڑو;
	endmntent(fp);

	واپس ent;
}

ساکن RC_STRINGLIST *
ڈھونڈو_mounts(ساخت دلائل *دلائل, size_t *عدد_mounts)
{
	FILE *fp;
	حرف *بفر;
	size_t سائز;
	حرف *p;
	حرف *from;
	حرف *مقصد;
	حرف *fst;
	حرف *opts;
	ساخت mntent *ent;
	صحیح netdev;
	RC_STRINGLIST *list;

	اگر ((fp = fopen(procmounts, "r")) == تہی)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	بفر = تہی;
	جب (xgetline(&بفر, &سائز, fp) != -1) {
		netdev = -1;
		p = بفر;
		from = strsep(&p, " ");
		مقصد = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		اگر ((ent = getmntfile(مقصد))) {
			اگر (لڑی_تلاش(ent->mnt_opts, "_netdev"))
				netdev = 0;
			ورنہ
				netdev = 1;
		}

		*عدد_mounts += 0 == دلائل->process(list, دلائل,
			from, مقصد, fst, opts, netdev);

		آزاد(بفر);
		بفر = تہی;
	}
	آزاد(بفر);
	fclose(fp);

	واپس list;
}

#ورنہ
#  error "Operating system not supported!"
#اگر_ختم

ساکن صحیح is_سابقہ(ثابت حرف *needle, ثابت حرف *hay)
{
	size_t nlen = لڑی_لمبائی(needle);
	اگر (لڑی_موازنہ(needle, hay, nlen) == 0 && hay[nlen] == '/')
		واپس true;
	واپس false;
}

ساکن حرف *unescape_octal(حرف *beg)
{
	صحیح n, i;
	حرف *w = beg, *r = beg;
	جب (*r) {
		اگر (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} ورنہ {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			برائے (i = n = 0; i < 3; ++i) {
				اگر (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} ورنہ {
					توڑو;
				}
			}
			اگر (n)
				*w++ = n;
		}
	}
	*w = '\0';
	واپس beg;
}

ساکن صحیح check_is_mounted(RC_STRINGLIST *list RC_غیر_مستعمل, ساخت دلائل *دلائل,
    حرف *from RC_غیر_مستعمل, حرف *مقصد, حرف *fstype RC_غیر_مستعمل,
    حرف *options RC_غیر_مستعمل, صحیح netdev RC_غیر_مستعمل)
{
	واپس strcmp(دلائل->check_mntpath, unescape_octal(مقصد)) == 0 ? 0 : -1;
}

ساکن صحیح is_mounted(ثابت حرف *mntpath)
{
	size_t عدد_mounts = 0;
	ساخت دلائل دلائل = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = ڈھونڈو_mounts(&دلائل, &عدد_mounts);
	rc_stringlist_free(l);
	واپس عدد_mounts > 0;
}

ساکن pid_t تسلسل_umount(ثابت حرف *mntpath,
	ثابت حرف **umount_دلائل, صحیح umount_دلائل_عدد)
{
	ساخت exec_دلائل دلائل;
	ساخت exec_نتیجہ res;
	ثابت حرف *argv[UMOUNT_دلائل_MAX + 3];
	صحیح k, i = 0;

	argv[i++] = "umount";
	برائے (k = 0; k < umount_دلائل_عدد; ++k)
		argv[i++] = umount_دلائل[k];
	argv[i++] = mntpath;
	argv[i++] = تہی;

	دلائل = exec_init(argv);
	دلائل.redirect_stdout = دلائل.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&دلائل);
	اگر (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	واپس res.pid;
}

ساکن خالی fuser_تسلسل(ساخت تسلسل_قطار *rp, ثابت حرف *fuser_opt)
{
	ساکن صحیح fuser_exec_failed = 0;
	ثابت حرف *argv[] = { "fuser", fuser_opt, rp->mntpath, تہی };
	ساخت exec_نتیجہ res;
	ساخت exec_دلائل دلائل;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	اگر (fuser_exec_failed)
		واپس;

	دلائل = exec_init(argv);
	دلائل.redirect_stdout = EXEC_MKPIPE;
	دلائل.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&دلائل);
	اگر (res.pid < 0) {
		fuser_exec_failed = 1;
	} ورنہ {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_وقت = tm_now();
	}
}

ساکن صحیح fuser_decide(ساخت تسلسل_قطار *rp,
	ثابت حرف *fuser_opt, ثابت حرف *fuser_kill_سابقہ)
{
	ساکن bool has_rc_pid = false;
	ساکن حرف *rc_pid;
	حرف buf[1<<12];
	حرف selfpid[64];
	صحیح read_maybe_truncated;
	ssize_t n;

	اگر (rp->fuser_stdoutfd < 0)
		واپس 0;

	buf[0] = ' ';
	n = اقرأ(rp->fuser_stdoutfd, buf + 1, حجم buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == حجم buf - 3);
	اگر (n < 0 || read_maybe_truncated)
		واپس 0;
	جب (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, حجم selfpid, " %lld ", (طویل طویل)getpid());

	اگر (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	اگر (لڑی_تلاش(buf, selfpid) || (rc_pid && لڑی_تلاش(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		واپس -1;
	} ورنہ اگر (strcmp(buf, "  ") == 0) {
		اگر (rp->try_گنتی >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			واپس -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_گنتی = TRY_MAX;
		واپس 0;
	} ورنہ {
		حرف sig[32];
		ثابت حرف *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, تہی
		};
		ساخت exec_نتیجہ res;
		ساخت exec_دلائل دلائل = exec_init(argv);
		دلائل.redirect_stdout = دلائل.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, حجم sig, "%s%s", fuser_kill_سابقہ,
			rp->try_گنتی == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&دلائل);
		اگر (res.pid > 0)
			rc_waitpid(res.pid);
		واپس 0;
	}
}

ساکن regex_t *
get_regex(ثابت حرف *string)
{
	regex_t *reg = xmalloc(حجم (*reg));
	صحیح نتیجہ;
	حرف بفر[256];

	اگر ((نتیجہ = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(نتیجہ, reg, بفر, حجم(بفر));
		eerrorx("%s: invalid regex `%s'", applet, بفر);
	}

	واپس reg;
}

صحیح مین(صحیح argc, حرف **argv)
{
	ساخت دلائل دلائل;
	regex_t *نقطہ_regex = تہی;
	regex_t *skip_نقطہ_regex = تہی;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	حرف *real_راستہ = تہی;
	صحیح opt;
	صحیح نتیجہ;
	حرف *this_راستہ, *argv0;
	ثابت حرف *tmps;
	ثابت حرف *fuser_opt, *fuser_kill_سابقہ;
	size_t عدد_mounts = 0;
	size_t unmount_اشاریہ;
	صحیح doing_unmount = 0;
	pid_t pid;
	صحیح حالت_2, flags;
	int64_t tmp, اگلا_retry, now;
	int64_t rc_fuser_timeout = -1;
	ثابت حرف *umount_دلائل[UMOUNT_دلائل_MAX];
	صحیح umount_دلائل_عدد = 0;
	ثابت حرف **mounts = تہی;
	ساخت تسلسل_قطار running[تسلسل_MAX] = {0};
	ساخت تسلسل_قطار *rp;
	size_t عدد_running = 0, عدد_انتظار = 0;
	شمار { STATE_تسلسل, STATE_REAP, STATE_RETRY, STATE_انتہا } state;

#تعریف_کرو DO_REG(_var)							      \
	اگر (_var) آزاد(_var);						      \
	_var = get_regex(optarg);
#تعریف_کرو REG_FREE(_var)							      \
	اگر (_var) { regfree(_var); آزاد(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&دلائل, 0, حجم(دلائل));
	دلائل.mount_type = mount_مقصد;
	دلائل.netdev = net_ignore;
	دلائل.mounts = rc_stringlist_new();
	دلائل.process = process_mount;

	اگر (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		جب (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			اگر (strcmp(tmps, "--") == 0)
				توڑو;
			اگر (umount_دلائل_عدد >= (صحیح)ARRAY_سائز(umount_دلائل))
				eerrorx("%s: Too many umount arguments", applet);
			umount_دلائل[umount_دلائل_عدد++] = tmps;
		}

		tmps = rc_conf_قدر("rc_fuser_timeout");
		اگر (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		اگر (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		اگر (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_سابقہ = "-";
		} ورنہ {
			fuser_opt = "-cm";
			fuser_kill_سابقہ = "-s";
		}
	}

	جب ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
		حالت 'e':
			دلائل.netdev = net_yes;
			توڑو;
		حالت 'E':
			دلائل.netdev = net_no;
			توڑو;
		حالت 'f':
			DO_REG(دلائل.fstype_regex);
			توڑو;
		حالت 'F':
			DO_REG(دلائل.skip_fstype_regex);
			توڑو;
		حالت 'n':
			DO_REG(دلائل.گرہ_regex);
			توڑو;
		حالت 'N':
			DO_REG(دلائل.skip_گرہ_regex);
			توڑو;
		حالت 'o':
			DO_REG(دلائل.options_regex);
			توڑو;
		حالت 'O':
			DO_REG(دلائل.skip_options_regex);
			توڑو;
		حالت 'p':
			DO_REG(نقطہ_regex);
			توڑو;
		حالت 'P':
			DO_REG(skip_نقطہ_regex);
			توڑو;
		حالت 'i':
			دلائل.mount_type = mount_options;
			توڑو;
		حالت 's':
			دلائل.mount_type = mount_fstype;
			توڑو;
		حالت 't':
			دلائل.mount_type = mount_from;
			توڑو;

		case_RC_COMMON_GETOPT
		}
	}

	جب (optind < argc) {
		اگر (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_راستہ = argv[optind++];
		real_راستہ = realpath(this_راستہ, تہی);
		اگر (real_راستہ)
			this_راستہ = real_راستہ;
		rc_stringlist_جوڑو(دلائل.mounts, this_راستہ);
		آزاد(real_راستہ);
		real_راستہ = تہی;
	}
	nodes = ڈھونڈو_mounts(&دلائل, &عدد_mounts);
	rc_stringlist_free(دلائل.mounts);

	اگر (doing_unmount)
		mounts = xmalloc(عدد_mounts * حجم(*mounts));
	عدد_mounts = 0;
	نتیجہ = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_الٹ(s, nodes, rc_stringlist, entries) {
		اگر (نقطہ_regex &&
		    regexec(نقطہ_regex, s->قدر, 0, تہی, 0) != 0)
			جاری;
		اگر (skip_نقطہ_regex &&
		    regexec(skip_نقطہ_regex, s->قدر, 0, تہی, 0) == 0)
			جاری;
		اگر (doing_unmount)
			mounts[عدد_mounts++] = unescape_octal(s->قدر);
		ورنہ اگر (!rc_yesno(getenv("EINFO_QUIET")))
			لکھو("%s\n", s->قدر);
		نتیجہ = EXIT_SUCCESS;
	}
	اگر (!doing_unmount)
		جاؤ باہر_نکلو;

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
	state = STATE_تسلسل;
	جب (state != STATE_انتہا) انتخاب (state) {
	حالت STATE_تسلسل:
		برائے (unmount_اشاریہ = 0; unmount_اشاریہ < عدد_mounts; ++unmount_اشاریہ) {
			ثابت حرف *candidate = mounts[unmount_اشاریہ];
			صحیح محفوظ_مقصد_unmount = 1;
			برائے (size_t k = 0; محفوظ_مقصد_unmount && k < عدد_running; ++k)
				محفوظ_مقصد_unmount = !is_سابقہ(candidate, running[k].mntpath);
			برائے (size_t k = 0; محفوظ_مقصد_unmount && k < عدد_mounts; ++k)
				محفوظ_مقصد_unmount = !is_سابقہ(candidate, mounts[k]);
			اگر (!محفوظ_مقصد_unmount)
				جاری;
			اگر (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_اشاریہ--] = mounts[--عدد_mounts];
			} ورنہ {
				توڑو;
			}
		}
		اگر (عدد_running == تسلسل_MAX || unmount_اشاریہ >= عدد_mounts) {
			state = STATE_REAP;
			توڑو;
		}
		rp = running + عدد_running++;
		rp->mntpath = mounts[unmount_اشاریہ];
		rp->آخری_exec_وقت = tm_now();
		rp->try_گنتی = 0;
		rp->pid = تسلسل_umount(rp->mntpath, umount_دلائل, umount_دلائل_عدد);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_وقت = -1;
		mounts[unmount_اشاریہ] = mounts[--عدد_mounts];
		توڑو;
	حالت STATE_REAP:
		flags = (عدد_انتظار > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &حالت_2, flags);
		rp = تہی;
		برائے (size_t i = 0; i < عدد_running; ++i) {
			rp = running + i;
			اگر (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			اگر (rp->pid == pid && pid > 0)
				توڑو;
			rp = تہی;
		}
		اگر (rp) {
			اگر ((WIFEXITED(حالت_2) && WEXITSTATUS(حالت_2) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--عدد_running];
				state = STATE_تسلسل;
			} ورنہ اگر (rp->try_گنتی >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--عدد_running];
				نتیجہ = EXIT_FAILURE;
			} ورنہ { /* put into waiting queue */
				rp->pid = -1;
				rp->try_گنتی += 1;
				عدد_انتظار += 1;
				fuser_تسلسل(rp, fuser_opt);
			}
		} ورنہ {
			state = STATE_RETRY;
		}
		توڑو;
	حالت STATE_RETRY:
		rp = تہی;
		اگلا_retry = INT64_MAX;
		برائے (size_t i = 0; i < عدد_running; ++i) {
			اگر (running[i].pid > 0)
				جاری;
			اگر (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_وقت + rc_fuser_timeout;
			ورنہ
				tmp = running[i].آخری_exec_وقت + TRY_DELAY_MS;
			اگر (tmp < اگلا_retry) {
				rp = running + i;
				اگلا_retry = tmp;
			}
		}
		اگر (!rp) {
			state = (عدد_mounts > 0) ? STATE_تسلسل : STATE_انتہا;
			توڑو;
		}
		now = tm_now();
		اگر ((rp->pid > 0 || rp->fuser_pid > 0) && اگلا_retry > now) {
			int64_t sleep_for = اگلا_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			اگر (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		اگر ((rp->pid == -1 && rp->fuser_pid == -1) || اگلا_retry <= now) {
			اگر (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, تہی, 0);
				rp->fuser_pid = -1;
			}
			اگر (fuser_decide(rp, fuser_opt, fuser_kill_سابقہ) < 0) { /* abort */
				*rp = running[--عدد_running];
				نتیجہ = EXIT_FAILURE;
			} ورنہ { /* retry */
				rp->آخری_exec_وقت = tm_now();
				rp->pid = تسلسل_umount(rp->mntpath,
					umount_دلائل, umount_دلائل_عدد);
			}
			عدد_انتظار -= 1;
			state = STATE_REAP;
		}
		توڑو;
	طےشدہ: توڑو;
	}

باہر_نکلو:
	آزاد(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(دلائل.fstype_regex);
	REG_FREE(دلائل.skip_fstype_regex);
	REG_FREE(دلائل.گرہ_regex);
	REG_FREE(دلائل.skip_گرہ_regex);
	REG_FREE(دلائل.options_regex);
	REG_FREE(دلائل.skip_options_regex);
	REG_FREE(نقطہ_regex);
	REG_FREE(skip_نقطہ_regex);

	واپس نتیجہ;
}
