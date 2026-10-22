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
#নোচেদ্যদি defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#নোচেদ্যদি defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#নোচেদ্যদি defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#endif

#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <getopt.h>
#অন্তর্ভাৱ <regex.h>
#অন্তর্ভাৱ <stddef.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdint.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <unistd.h>

#অন্তর্ভাৱ "einfo.h"
#অন্তর্ভাৱ "queue.h"
#অন্তর্ভাৱ "rc.h"
#অন্তর্ভাৱ "rc_exec.h"
#অন্তর্ভাৱ "timeutils.h"
#অন্তর্ভাৱ "_usage.h"
#অন্তর্ভাৱ "helpers.h"

ধ্রুৱ ৱর্ণ *applet = রিক্ত;
ধ্রুৱ ৱর্ণ *procmounts = "/proc/mounts";
ধ্রুৱ ৱর্ণ *extraopts = "[mount1] [mount2] ...";
ধ্রুৱ ৱর্ণ getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
ধ্রুৱ সংরচনা option longopts[] = {
	{ "fstype-regex",        1, রিক্ত, 'f'},
	{ "skip-fstype-regex",   1, রিক্ত, 'F'},
	{ "node-regex",          1, রিক্ত, 'n'},
	{ "skip-node-regex",     1, রিক্ত, 'N'},
	{ "options-regex",       1, রিক্ত, 'o'},
	{ "skip-options-regex",  1, রিক্ত, 'O'},
	{ "point-regex",         1, রিক্ত, 'p'},
	{ "skip-point-regex",    1, রিক্ত, 'P'},
	{ "options",             0, রিক্ত, 'i'},
	{ "fstype",              0, রিক্ত, 's'},
	{ "node",                0, রিক্ত, 't'},
	{ "netdev",              0, রিক্ত, 'e'},
	{ "nonetdev",            0, রিক্ত, 'E'},
	longopts_COMMON
};
ধ্রুৱ ৱর্ণ * ধ্রুৱ longopts_help[] = {
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
ধ্রুৱ ৱর্ণ *usagestring = রিক্ত;

#define UMOUNT_ARGS_MAX 16
#define RUN_MAX 32
#define TRY_MAX 3
#define TRY_DELAY_MS 1000

সংরচনা run_queue {
	ধ্রুৱ ৱর্ণ *mntpath;
	int64_t last_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	পূর্ণাঙ্ক try_গুনতি;
	পূর্ণাঙ্ক fuser_stdoutfd;
};

প্রকারনির্দেশ গণনা {
	mount_from,
	mount_to,
	mount_fstype,
	mount_options
} mount_type;

প্রকারনির্দেশ গণনা {
	net_ignore,
	net_yes,
	net_no
} net_opts;

সংরচনা args;
প্রকারনির্দেশ পূর্ণাঙ্ক process_func_t(RC_STRINGLIST *, সংরচনা args *,
    ৱর্ণ *, ৱর্ণ *, ৱর্ণ *, ৱর্ণ *, পূর্ণাঙ্ক);

সংরচনা args {
	process_func_t *process;
	regex_t *গ্ৰন্থি_regex;
	regex_t *skip_গ্ৰন্থি_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	ধ্রুৱ ৱর্ণ *check_mntpath;
};

স্থির পূর্ণাঙ্ক
process_mount(RC_STRINGLIST *list, সংরচনা args *args,
    ৱর্ণ *from, ৱর্ণ *to, ৱর্ণ *fstype, ৱর্ণ *options,
    পূর্ণাঙ্ক netdev)
{
	ৱর্ণ *p;
	RC_STRING *s;

	errno = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	যদি (strcmp(fstype, "rootfs") == 0)
		ঘূৰাই -1;
#endif

	যদি (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		যদি (netdev != 0)
			ঘূৰাই 1;
	} অন্যথা যদি (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		যদি (netdev != 1)
			ঘূৰাই 1;
	} অন্যথা {
		যদি (args->গ্ৰন্থি_regex &&
		    regexec(args->গ্ৰন্থি_regex, from, 0, রিক্ত, 0) != 0)
			ঘূৰাই 1;
		যদি (args->skip_গ্ৰন্থি_regex &&
		    regexec(args->skip_গ্ৰন্থি_regex, from, 0, রিক্ত, 0) == 0)
			ঘূৰাই 1;

		যদি (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, রিক্ত, 0) != 0)
			ঘূৰাই -1;
		যদি (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, রিক্ত, 0) == 0)
			ঘূৰাই -1;

		যদি (args->options_regex &&
		    regexec(args->options_regex, options, 0, রিক্ত, 0) != 0)
			ঘূৰাই -1;
		যদি (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, রিক্ত, 0) == 0)
			ঘূৰাই -1;
	}

	যদি (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    যদি (strcmp(s->মান, to) == 0)
			    ভঙ্গ;
		যদি (!s)
			ঘূৰাই -1;
	}

	ৱিকল্প (args->mount_type) {
	পক্ষ mount_from:
		p = from;
		ভঙ্গ;
	পক্ষ mount_to:
		p = to;
		ভঙ্গ;
	পক্ষ mount_fstype:
		p = fstype;
		ভঙ্গ;
	পক্ষ mount_options:
		p = options;
		ভঙ্গ;
	স্ৱভাৱ:
		p = রিক্ত;
		errno = EINVAL;
		ভঙ্গ;
	}

	যদি (p) {
		errno = 0;
		rc_stringlist_add(list, p);
		ঘূৰাই 0;
	}

	ঘূৰাই -1;
}

#যদি defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
স্থির সংরচনা opt {
	পূর্ণাঙ্ক o_opt;
	ধ্রুৱ ৱর্ণ *o_name;
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
	{ 0, রিক্ত }
};

স্থির RC_STRINGLIST *
find_mounts(সংরচনা args *args, size_t *অঙ্ক_mounts)
{
	সংরচনা statfs *mnts;
	পূর্ণাঙ্ক nmnts;
	পূর্ণাঙ্ক i;
	RC_STRINGLIST *list;
	ৱর্ণ *options = রিক্ত;
	uint64_t flags;
	সংরচনা opt *o;
	পূর্ণাঙ্ক netdev;
	ৱর্ণ *tmp;

	যদি ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	বাবে (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		বাবে (o = optnames; flags && o->o_opt; o++) {
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
		options = রিক্ত;
	}

	ঘূৰাই list;
}

#নোচেদ্যদি defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
স্থির সংরচনা mntent *
getmntfile(ধ্রুৱ ৱর্ণ *file)
{
	সংরচনা mntent *ent = রিক্ত;
	FILE *fp;

	যদি (!(fp = setmntent("/etc/fstab", "r")))
		ঘূৰাই রিক্ত;
	যেতিয়ালৈ ((ent = getmntent(fp)))
		যদি (strcmp(file, ent->mnt_dir) == 0)
			ভঙ্গ;
	endmntent(fp);

	ঘূৰাই ent;
}

স্থির RC_STRINGLIST *
find_mounts(সংরচনা args *args, size_t *অঙ্ক_mounts)
{
	FILE *fp;
	ৱর্ণ *buffer;
	size_t size;
	ৱর্ণ *p;
	ৱর্ণ *from;
	ৱর্ণ *to;
	ৱর্ণ *fst;
	ৱর্ণ *opts;
	সংরচনা mntent *ent;
	পূর্ণাঙ্ক netdev;
	RC_STRINGLIST *list;

	যদি ((fp = fopen(procmounts, "r")) == রিক্ত)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	buffer = রিক্ত;
	যেতিয়ালৈ (xgetline(&buffer, &size, fp) != -1) {
		netdev = -1;
		p = buffer;
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

		মুক্ত(buffer);
		buffer = রিক্ত;
	}
	মুক্ত(buffer);
	fclose(fp);

	ঘূৰাই list;
}

#অন্যথা
#  error "Operating system not supported!"
#endif

স্থির পূর্ণাঙ্ক is_prefix(ধ্রুৱ ৱর্ণ *needle, ধ্রুৱ ৱর্ণ *hay)
{
	size_t nlen = strlen(needle);
	যদি (strncmp(needle, hay, nlen) == 0 && hay[nlen] == '/')
		ঘূৰাই true;
	ঘূৰাই false;
}

স্থির ৱর্ণ *unescape_octal(ৱর্ণ *beg)
{
	পূর্ণাঙ্ক n, i;
	ৱর্ণ *w = beg, *r = beg;
	যেতিয়ালৈ (*r) {
		যদি (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} অন্যথা {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			বাবে (i = n = 0; i < 3; ++i) {
				যদি (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} অন্যথা {
					ভঙ্গ;
				}
			}
			যদি (n)
				*w++ = n;
		}
	}
	*w = '\0';
	ঘূৰাই beg;
}

স্থির পূর্ণাঙ্ক check_is_mounted(RC_STRINGLIST *list RC_UNUSED, সংরচনা args *args,
    ৱর্ণ *from RC_UNUSED, ৱর্ণ *to, ৱর্ণ *fstype RC_UNUSED,
    ৱর্ণ *options RC_UNUSED, পূর্ণাঙ্ক netdev RC_UNUSED)
{
	ঘূৰাই strcmp(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

স্থির পূর্ণাঙ্ক is_mounted(ধ্রুৱ ৱর্ণ *mntpath)
{
	size_t অঙ্ক_mounts = 0;
	সংরচনা args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &অঙ্ক_mounts);
	rc_stringlist_free(l);
	ঘূৰাই অঙ্ক_mounts > 0;
}

স্থির pid_t run_umount(ধ্রুৱ ৱর্ণ *mntpath,
	ধ্রুৱ ৱর্ণ **umount_args, পূর্ণাঙ্ক umount_args_অঙ্ক)
{
	সংরচনা exec_args args;
	সংরচনা exec_result res;
	ধ্রুৱ ৱর্ণ *argv[UMOUNT_ARGS_MAX + 3];
	পূর্ণাঙ্ক k, i = 0;

	argv[i++] = "umount";
	বাবে (k = 0; k < umount_args_অঙ্ক; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = রিক্ত;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	যদি (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	ঘূৰাই res.pid;
}

স্থির শূন্য fuser_run(সংরচনা run_queue *rp, ধ্রুৱ ৱর্ণ *fuser_opt)
{
	স্থির পূর্ণাঙ্ক fuser_exec_failed = 0;
	ধ্রুৱ ৱর্ণ *argv[] = { "fuser", fuser_opt, rp->mntpath, রিক্ত };
	সংরচনা exec_result res;
	সংরচনা exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	যদি (fuser_exec_failed)
		ঘূৰাই;

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

স্থির পূর্ণাঙ্ক fuser_decide(সংরচনা run_queue *rp,
	ধ্রুৱ ৱর্ণ *fuser_opt, ধ্রুৱ ৱর্ণ *fuser_kill_prefix)
{
	স্থির bool has_rc_pid = false;
	স্থির ৱর্ণ *rc_pid;
	ৱর্ণ buf[1<<12];
	ৱর্ণ selfpid[64];
	পূর্ণাঙ্ক read_maybe_truncated;
	ssize_t n;

	যদি (rp->fuser_stdoutfd < 0)
		ঘূৰাই 0;

	buf[0] = ' ';
	n = পঠ(rp->fuser_stdoutfd, buf + 1, পরিমাণ buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == পরিমাণ buf - 3);
	যদি (n < 0 || read_maybe_truncated)
		ঘূৰাই 0;
	যেতিয়ালৈ (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, পরিমাণ selfpid, " %lld ", (long long)getpid());

	যদি (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	যদি (strstr(buf, selfpid) || (rc_pid && strstr(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		ঘূৰাই -1;
	} অন্যথা যদি (strcmp(buf, "  ") == 0) {
		যদি (rp->try_গুনতি >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			ঘূৰাই -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_গুনতি = TRY_MAX;
		ঘূৰাই 0;
	} অন্যথা {
		ৱর্ণ sig[32];
		ধ্রুৱ ৱর্ণ *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, রিক্ত
		};
		সংরচনা exec_result res;
		সংরচনা exec_args args = exec_init(argv);
		args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, পরিমাণ sig, "%s%s", fuser_kill_prefix,
			rp->try_গুনতি == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&args);
		যদি (res.pid > 0)
			rc_waitpid(res.pid);
		ঘূৰাই 0;
	}
}

স্থির regex_t *
get_regex(ধ্রুৱ ৱর্ণ *string)
{
	regex_t *reg = xmalloc(পরিমাণ (*reg));
	পূর্ণাঙ্ক result;
	ৱর্ণ buffer[256];

	যদি ((result = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(result, reg, buffer, পরিমাণ(buffer));
		eerrorx("%s: invalid regex `%s'", applet, buffer);
	}

	ঘূৰাই reg;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
{
	সংরচনা args args;
	regex_t *point_regex = রিক্ত;
	regex_t *skip_point_regex = রিক্ত;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	ৱর্ণ *real_path = রিক্ত;
	পূর্ণাঙ্ক opt;
	পূর্ণাঙ্ক result;
	ৱর্ণ *this_path, *argv0;
	ধ্রুৱ ৱর্ণ *tmps;
	ধ্রুৱ ৱর্ণ *fuser_opt, *fuser_kill_prefix;
	size_t অঙ্ক_mounts = 0;
	size_t unmount_index;
	পূর্ণাঙ্ক doing_unmount = 0;
	pid_t pid;
	পূর্ণাঙ্ক status, flags;
	int64_t tmp, next_retry, now;
	int64_t rc_fuser_timeout = -1;
	ধ্রুৱ ৱর্ণ *umount_args[UMOUNT_ARGS_MAX];
	পূর্ণাঙ্ক umount_args_অঙ্ক = 0;
	ধ্রুৱ ৱর্ণ **mounts = রিক্ত;
	সংরচনা run_queue running[RUN_MAX] = {0};
	সংরচনা run_queue *rp;
	size_t অঙ্ক_running = 0, অঙ্ক_waiting = 0;
	গণনা { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	যদি (_var) মুক্ত(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	যদি (_var) { regfree(_var); মুক্ত(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&args, 0, পরিমাণ(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	যদি (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		যেতিয়ালৈ (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			যদি (strcmp(tmps, "--") == 0)
				ভঙ্গ;
			যদি (umount_args_অঙ্ক >= (পূর্ণাঙ্ক)ARRAY_SIZE(umount_args))
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

	যেতিয়ালৈ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (পূর্ণাঙ্ক *) 0)) != -1)
	{
		ৱিকল্প (opt) {
		পক্ষ 'e':
			args.netdev = net_yes;
			ভঙ্গ;
		পক্ষ 'E':
			args.netdev = net_no;
			ভঙ্গ;
		পক্ষ 'f':
			DO_REG(args.fstype_regex);
			ভঙ্গ;
		পক্ষ 'F':
			DO_REG(args.skip_fstype_regex);
			ভঙ্গ;
		পক্ষ 'n':
			DO_REG(args.গ্ৰন্থি_regex);
			ভঙ্গ;
		পক্ষ 'N':
			DO_REG(args.skip_গ্ৰন্থি_regex);
			ভঙ্গ;
		পক্ষ 'o':
			DO_REG(args.options_regex);
			ভঙ্গ;
		পক্ষ 'O':
			DO_REG(args.skip_options_regex);
			ভঙ্গ;
		পক্ষ 'p':
			DO_REG(point_regex);
			ভঙ্গ;
		পক্ষ 'P':
			DO_REG(skip_point_regex);
			ভঙ্গ;
		পক্ষ 'i':
			args.mount_type = mount_options;
			ভঙ্গ;
		পক্ষ 's':
			args.mount_type = mount_fstype;
			ভঙ্গ;
		পক্ষ 't':
			args.mount_type = mount_from;
			ভঙ্গ;

		case_RC_COMMON_GETOPT
		}
	}

	যেতিয়ালৈ (optind < argc) {
		যদি (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, রিক্ত);
		যদি (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		মুক্ত(real_path);
		real_path = রিক্ত;
	}
	nodes = find_mounts(&args, &অঙ্ক_mounts);
	rc_stringlist_free(args.mounts);

	যদি (doing_unmount)
		mounts = xmalloc(অঙ্ক_mounts * পরিমাণ(*mounts));
	অঙ্ক_mounts = 0;
	result = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		যদি (point_regex &&
		    regexec(point_regex, s->মান, 0, রিক্ত, 0) != 0)
			অনুৱর্ত;
		যদি (skip_point_regex &&
		    regexec(skip_point_regex, s->মান, 0, রিক্ত, 0) == 0)
			অনুৱর্ত;
		যদি (doing_unmount)
			mounts[অঙ্ক_mounts++] = unescape_octal(s->মান);
		অন্যথা যদি (!rc_yesno(getenv("EINFO_QUIET")))
			printf("%s\n", s->মান);
		result = EXIT_SUCCESS;
	}
	যদি (!doing_unmount)
		গচ্ছ exit;

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
	যেতিয়ালৈ (state != STATE_END) ৱিকল্প (state) {
	পক্ষ STATE_RUN:
		বাবে (unmount_index = 0; unmount_index < অঙ্ক_mounts; ++unmount_index) {
			ধ্রুৱ ৱর্ণ *candidate = mounts[unmount_index];
			পূর্ণাঙ্ক safe_to_unmount = 1;
			বাবে (size_t k = 0; safe_to_unmount && k < অঙ্ক_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			বাবে (size_t k = 0; safe_to_unmount && k < অঙ্ক_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			যদি (!safe_to_unmount)
				অনুৱর্ত;
			যদি (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_index--] = mounts[--অঙ্ক_mounts];
			} অন্যথা {
				ভঙ্গ;
			}
		}
		যদি (অঙ্ক_running == RUN_MAX || unmount_index >= অঙ্ক_mounts) {
			state = STATE_REAP;
			ভঙ্গ;
		}
		rp = running + অঙ্ক_running++;
		rp->mntpath = mounts[unmount_index];
		rp->last_exec_time = tm_now();
		rp->try_গুনতি = 0;
		rp->pid = run_umount(rp->mntpath, umount_args, umount_args_অঙ্ক);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_index] = mounts[--অঙ্ক_mounts];
		ভঙ্গ;
	পক্ষ STATE_REAP:
		flags = (অঙ্ক_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = রিক্ত;
		বাবে (size_t i = 0; i < অঙ্ক_running; ++i) {
			rp = running + i;
			যদি (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			যদি (rp->pid == pid && pid > 0)
				ভঙ্গ;
			rp = রিক্ত;
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
				result = EXIT_FAILURE;
			} অন্যথা { /* put into waiting queue */
				rp->pid = -1;
				rp->try_গুনতি += 1;
				অঙ্ক_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} অন্যথা {
			state = STATE_RETRY;
		}
		ভঙ্গ;
	পক্ষ STATE_RETRY:
		rp = রিক্ত;
		next_retry = INT64_MAX;
		বাবে (size_t i = 0; i < অঙ্ক_running; ++i) {
			যদি (running[i].pid > 0)
				অনুৱর্ত;
			যদি (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			অন্যথা
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			যদি (tmp < next_retry) {
				rp = running + i;
				next_retry = tmp;
			}
		}
		যদি (!rp) {
			state = (অঙ্ক_mounts > 0) ? STATE_RUN : STATE_END;
			ভঙ্গ;
		}
		now = tm_now();
		যদি ((rp->pid > 0 || rp->fuser_pid > 0) && next_retry > now) {
			int64_t sleep_for = next_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			যদি (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		যদি ((rp->pid == -1 && rp->fuser_pid == -1) || next_retry <= now) {
			যদি (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, রিক্ত, 0);
				rp->fuser_pid = -1;
			}
			যদি (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--অঙ্ক_running];
				result = EXIT_FAILURE;
			} অন্যথা { /* retry */
				rp->last_exec_time = tm_now();
				rp->pid = run_umount(rp->mntpath,
					umount_args, umount_args_অঙ্ক);
			}
			অঙ্ক_waiting -= 1;
			state = STATE_REAP;
		}
		ভঙ্গ;
	স্ৱভাৱ: ভঙ্গ;
	}

exit:
	মুক্ত(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(args.fstype_regex);
	REG_FREE(args.skip_fstype_regex);
	REG_FREE(args.গ্ৰন্থি_regex);
	REG_FREE(args.skip_গ্ৰন্থি_regex);
	REG_FREE(args.options_regex);
	REG_FREE(args.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	ঘূৰাই result;
}
