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

#எனில் defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#அல்லதெனில் defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#அல்லதெனில் defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#அல்லதெனில் defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#endif

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <regex.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "rc_exec.h"
#அந்தர்பாவ "timeutils.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "helpers.h"

மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *procmounts = "/proc/mounts";
மாறிலி எழுத்து *extraopts = "[mount1] [mount2] ...";
மாறிலி எழுத்து getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "fstype-regex",        1, வெற்று, 'f'},
	{ "skip-fstype-regex",   1, வெற்று, 'F'},
	{ "node-regex",          1, வெற்று, 'n'},
	{ "skip-node-regex",     1, வெற்று, 'N'},
	{ "options-regex",       1, வெற்று, 'o'},
	{ "skip-options-regex",  1, வெற்று, 'O'},
	{ "point-regex",         1, வெற்று, 'p'},
	{ "skip-point-regex",    1, வெற்று, 'P'},
	{ "options",             0, வெற்று, 'i'},
	{ "fstype",              0, வெற்று, 's'},
	{ "node",                0, வெற்று, 't'},
	{ "netdev",              0, வெற்று, 'e'},
	{ "nonetdev",            0, வெற்று, 'E'},
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
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
மாறிலி எழுத்து *usagestring = வெற்று;

#define UMOUNT_ARGS_MAX 16
#define RUN_MAX 32
#define TRY_MAX 3
#define TRY_DELAY_MS 1000

கட்டமைப்பு run_queue {
	மாறிலி எழுத்து *mntpath;
	int64_t last_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	முழு try_மொத்தம்;
	முழு fuser_stdoutfd;
};

வகைவரை எண்ணிக்கை {
	mount_from,
	mount_to,
	mount_fstype,
	mount_options
} mount_type;

வகைவரை எண்ணிக்கை {
	net_ignore,
	net_yes,
	net_no
} net_opts;

கட்டமைப்பு args;
வகைவரை முழு process_func_t(RC_STRINGLIST *, கட்டமைப்பு args *,
    எழுத்து *, எழுத்து *, எழுத்து *, எழுத்து *, முழு);

கட்டமைப்பு args {
	process_func_t *process;
	regex_t *முனை_regex;
	regex_t *skip_முனை_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	மாறிலி எழுத்து *check_mntpath;
};

நிலையான முழு
process_mount(RC_STRINGLIST *list, கட்டமைப்பு args *args,
    எழுத்து *from, எழுத்து *to, எழுத்து *fstype, எழுத்து *options,
    முழு netdev)
{
	எழுத்து *p;
	RC_STRING *s;

	errno = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	எனில் (strcmp(fstype, "rootfs") == 0)
		திரும்பு -1;
#endif

	எனில் (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		எனில் (netdev != 0)
			திரும்பு 1;
	} இல்லையெனில் எனில் (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		எனில் (netdev != 1)
			திரும்பு 1;
	} இல்லையெனில் {
		எனில் (args->முனை_regex &&
		    regexec(args->முனை_regex, from, 0, வெற்று, 0) != 0)
			திரும்பு 1;
		எனில் (args->skip_முனை_regex &&
		    regexec(args->skip_முனை_regex, from, 0, வெற்று, 0) == 0)
			திரும்பு 1;

		எனில் (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, வெற்று, 0) != 0)
			திரும்பு -1;
		எனில் (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, வெற்று, 0) == 0)
			திரும்பு -1;

		எனில் (args->options_regex &&
		    regexec(args->options_regex, options, 0, வெற்று, 0) != 0)
			திரும்பு -1;
		எனில் (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, வெற்று, 0) == 0)
			திரும்பு -1;
	}

	எனில் (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    எனில் (strcmp(s->மதிப்பு, to) == 0)
			    நிறுத்து;
		எனில் (!s)
			திரும்பு -1;
	}

	தேர்வு (args->mount_type) {
	நிலை mount_from:
		p = from;
		நிறுத்து;
	நிலை mount_to:
		p = to;
		நிறுத்து;
	நிலை mount_fstype:
		p = fstype;
		நிறுத்து;
	நிலை mount_options:
		p = options;
		நிறுத்து;
	இயல்பு:
		p = வெற்று;
		errno = EINVAL;
		நிறுத்து;
	}

	எனில் (p) {
		errno = 0;
		rc_stringlist_add(list, p);
		திரும்பு 0;
	}

	திரும்பு -1;
}

#எனில் defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
நிலையான கட்டமைப்பு opt {
	முழு o_opt;
	மாறிலி எழுத்து *o_name;
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
	{ 0, வெற்று }
};

நிலையான RC_STRINGLIST *
find_mounts(கட்டமைப்பு args *args, size_t *இலக்கம்_mounts)
{
	கட்டமைப்பு statfs *mnts;
	முழு nmnts;
	முழு i;
	RC_STRINGLIST *list;
	எழுத்து *options = வெற்று;
	uint64_t flags;
	கட்டமைப்பு opt *o;
	முழு netdev;
	எழுத்து *tmp;

	எனில் ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	க்கு (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		க்கு (o = optnames; flags && o->o_opt; o++) {
			எனில் (flags & o->o_opt) {
				எனில் (o->o_opt == MNT_LOCAL)
					netdev = 1;
				எனில் (!options)
					options = xstrdup(o->o_name);
				இல்லையெனில் {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					விடுவி(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*இலக்கம்_mounts += 0 == args->process(list, args,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		விடுவி(options);
		options = வெற்று;
	}

	திரும்பு list;
}

#அல்லதெனில் defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
நிலையான கட்டமைப்பு mntent *
getmntfile(மாறிலி எழுத்து *file)
{
	கட்டமைப்பு mntent *ent = வெற்று;
	FILE *fp;

	எனில் (!(fp = setmntent("/etc/fstab", "r")))
		திரும்பு வெற்று;
	போது ((ent = getmntent(fp)))
		எனில் (strcmp(file, ent->mnt_dir) == 0)
			நிறுத்து;
	endmntent(fp);

	திரும்பு ent;
}

நிலையான RC_STRINGLIST *
find_mounts(கட்டமைப்பு args *args, size_t *இலக்கம்_mounts)
{
	FILE *fp;
	எழுத்து *buffer;
	size_t size;
	எழுத்து *p;
	எழுத்து *from;
	எழுத்து *to;
	எழுத்து *fst;
	எழுத்து *opts;
	கட்டமைப்பு mntent *ent;
	முழு netdev;
	RC_STRINGLIST *list;

	எனில் ((fp = fopen(procmounts, "r")) == வெற்று)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	buffer = வெற்று;
	போது (xgetline(&buffer, &size, fp) != -1) {
		netdev = -1;
		p = buffer;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		எனில் ((ent = getmntfile(to))) {
			எனில் (strstr(ent->mnt_opts, "_netdev"))
				netdev = 0;
			இல்லையெனில்
				netdev = 1;
		}

		*இலக்கம்_mounts += 0 == args->process(list, args,
			from, to, fst, opts, netdev);

		விடுவி(buffer);
		buffer = வெற்று;
	}
	விடுவி(buffer);
	fclose(fp);

	திரும்பு list;
}

#இல்லையெனில்
#  error "Operating system not supported!"
#endif

நிலையான முழு is_prefix(மாறிலி எழுத்து *needle, மாறிலி எழுத்து *hay)
{
	size_t nlen = strlen(needle);
	எனில் (strncmp(needle, hay, nlen) == 0 && hay[nlen] == '/')
		திரும்பு true;
	திரும்பு false;
}

நிலையான எழுத்து *unescape_octal(எழுத்து *beg)
{
	முழு n, i;
	எழுத்து *w = beg, *r = beg;
	போது (*r) {
		எனில் (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} இல்லையெனில் {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			க்கு (i = n = 0; i < 3; ++i) {
				எனில் (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} இல்லையெனில் {
					நிறுத்து;
				}
			}
			எனில் (n)
				*w++ = n;
		}
	}
	*w = '\0';
	திரும்பு beg;
}

நிலையான முழு check_is_mounted(RC_STRINGLIST *list RC_UNUSED, கட்டமைப்பு args *args,
    எழுத்து *from RC_UNUSED, எழுத்து *to, எழுத்து *fstype RC_UNUSED,
    எழுத்து *options RC_UNUSED, முழு netdev RC_UNUSED)
{
	திரும்பு strcmp(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

நிலையான முழு is_mounted(மாறிலி எழுத்து *mntpath)
{
	size_t இலக்கம்_mounts = 0;
	கட்டமைப்பு args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &இலக்கம்_mounts);
	rc_stringlist_free(l);
	திரும்பு இலக்கம்_mounts > 0;
}

நிலையான pid_t run_umount(மாறிலி எழுத்து *mntpath,
	மாறிலி எழுத்து **umount_args, முழு umount_args_இலக்கம்)
{
	கட்டமைப்பு exec_args args;
	கட்டமைப்பு exec_result res;
	மாறிலி எழுத்து *argv[UMOUNT_ARGS_MAX + 3];
	முழு k, i = 0;

	argv[i++] = "umount";
	க்கு (k = 0; k < umount_args_இலக்கம்; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = வெற்று;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	எனில் (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	திரும்பு res.pid;
}

நிலையான வெறுமை fuser_run(கட்டமைப்பு run_queue *rp, மாறிலி எழுத்து *fuser_opt)
{
	நிலையான முழு fuser_exec_failed = 0;
	மாறிலி எழுத்து *argv[] = { "fuser", fuser_opt, rp->mntpath, வெற்று };
	கட்டமைப்பு exec_result res;
	கட்டமைப்பு exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	எனில் (fuser_exec_failed)
		திரும்பு;

	args = exec_init(argv);
	args.redirect_stdout = EXEC_MKPIPE;
	args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	எனில் (res.pid < 0) {
		fuser_exec_failed = 1;
	} இல்லையெனில் {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

நிலையான முழு fuser_decide(கட்டமைப்பு run_queue *rp,
	மாறிலி எழுத்து *fuser_opt, மாறிலி எழுத்து *fuser_kill_prefix)
{
	நிலையான bool has_rc_pid = false;
	நிலையான எழுத்து *rc_pid;
	எழுத்து buf[1<<12];
	எழுத்து selfpid[64];
	முழு read_maybe_truncated;
	ssize_t n;

	எனில் (rp->fuser_stdoutfd < 0)
		திரும்பு 0;

	buf[0] = ' ';
	n = பட(rp->fuser_stdoutfd, buf + 1, அளவு buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == அளவு buf - 3);
	எனில் (n < 0 || read_maybe_truncated)
		திரும்பு 0;
	போது (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, அளவு selfpid, " %lld ", (long long)getpid());

	எனில் (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	எனில் (strstr(buf, selfpid) || (rc_pid && strstr(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		திரும்பு -1;
	} இல்லையெனில் எனில் (strcmp(buf, "  ") == 0) {
		எனில் (rp->try_மொத்தம் >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			திரும்பு -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_மொத்தம் = TRY_MAX;
		திரும்பு 0;
	} இல்லையெனில் {
		எழுத்து sig[32];
		மாறிலி எழுத்து *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, வெற்று
		};
		கட்டமைப்பு exec_result res;
		கட்டமைப்பு exec_args args = exec_init(argv);
		args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, அளவு sig, "%s%s", fuser_kill_prefix,
			rp->try_மொத்தம் == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&args);
		எனில் (res.pid > 0)
			rc_waitpid(res.pid);
		திரும்பு 0;
	}
}

நிலையான regex_t *
get_regex(மாறிலி எழுத்து *string)
{
	regex_t *reg = xmalloc(அளவு (*reg));
	முழு result;
	எழுத்து buffer[256];

	எனில் ((result = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(result, reg, buffer, அளவு(buffer));
		eerrorx("%s: invalid regex `%s'", applet, buffer);
	}

	திரும்பு reg;
}

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	கட்டமைப்பு args args;
	regex_t *point_regex = வெற்று;
	regex_t *skip_point_regex = வெற்று;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	எழுத்து *real_path = வெற்று;
	முழு opt;
	முழு result;
	எழுத்து *this_path, *argv0;
	மாறிலி எழுத்து *tmps;
	மாறிலி எழுத்து *fuser_opt, *fuser_kill_prefix;
	size_t இலக்கம்_mounts = 0;
	size_t unmount_index;
	முழு doing_unmount = 0;
	pid_t pid;
	முழு status, flags;
	int64_t tmp, next_retry, now;
	int64_t rc_fuser_timeout = -1;
	மாறிலி எழுத்து *umount_args[UMOUNT_ARGS_MAX];
	முழு umount_args_இலக்கம் = 0;
	மாறிலி எழுத்து **mounts = வெற்று;
	கட்டமைப்பு run_queue running[RUN_MAX] = {0};
	கட்டமைப்பு run_queue *rp;
	size_t இலக்கம்_running = 0, இலக்கம்_waiting = 0;
	எண்ணிக்கை { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	எனில் (_var) விடுவி(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	எனில் (_var) { regfree(_var); விடுவி(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&args, 0, அளவு(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	எனில் (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		போது (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			எனில் (strcmp(tmps, "--") == 0)
				நிறுத்து;
			எனில் (umount_args_இலக்கம் >= (முழு)ARRAY_SIZE(umount_args))
				eerrorx("%s: Too many umount arguments", applet);
			umount_args[umount_args_இலக்கம்++] = tmps;
		}

		tmps = rc_conf_மதிப்பு("rc_fuser_timeout");
		எனில் (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		எனில் (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		எனில் (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} இல்லையெனில் {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	போது ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (முழு *) 0)) != -1)
	{
		தேர்வு (opt) {
		நிலை 'e':
			args.netdev = net_yes;
			நிறுத்து;
		நிலை 'E':
			args.netdev = net_no;
			நிறுத்து;
		நிலை 'f':
			DO_REG(args.fstype_regex);
			நிறுத்து;
		நிலை 'F':
			DO_REG(args.skip_fstype_regex);
			நிறுத்து;
		நிலை 'n':
			DO_REG(args.முனை_regex);
			நிறுத்து;
		நிலை 'N':
			DO_REG(args.skip_முனை_regex);
			நிறுத்து;
		நிலை 'o':
			DO_REG(args.options_regex);
			நிறுத்து;
		நிலை 'O':
			DO_REG(args.skip_options_regex);
			நிறுத்து;
		நிலை 'p':
			DO_REG(point_regex);
			நிறுத்து;
		நிலை 'P':
			DO_REG(skip_point_regex);
			நிறுத்து;
		நிலை 'i':
			args.mount_type = mount_options;
			நிறுத்து;
		நிலை 's':
			args.mount_type = mount_fstype;
			நிறுத்து;
		நிலை 't':
			args.mount_type = mount_from;
			நிறுத்து;

		case_RC_COMMON_GETOPT
		}
	}

	போது (optind < argc) {
		எனில் (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, வெற்று);
		எனில் (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		விடுவி(real_path);
		real_path = வெற்று;
	}
	nodes = find_mounts(&args, &இலக்கம்_mounts);
	rc_stringlist_free(args.mounts);

	எனில் (doing_unmount)
		mounts = xmalloc(இலக்கம்_mounts * அளவு(*mounts));
	இலக்கம்_mounts = 0;
	result = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		எனில் (point_regex &&
		    regexec(point_regex, s->மதிப்பு, 0, வெற்று, 0) != 0)
			தொடர்;
		எனில் (skip_point_regex &&
		    regexec(skip_point_regex, s->மதிப்பு, 0, வெற்று, 0) == 0)
			தொடர்;
		எனில் (doing_unmount)
			mounts[இலக்கம்_mounts++] = unescape_octal(s->மதிப்பு);
		இல்லையெனில் எனில் (!rc_yesno(getenv("EINFO_QUIET")))
			printf("%s\n", s->மதிப்பு);
		result = EXIT_SUCCESS;
	}
	எனில் (!doing_unmount)
		செல் exit;

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
	போது (state != STATE_END) தேர்வு (state) {
	நிலை STATE_RUN:
		க்கு (unmount_index = 0; unmount_index < இலக்கம்_mounts; ++unmount_index) {
			மாறிலி எழுத்து *candidate = mounts[unmount_index];
			முழு safe_to_unmount = 1;
			க்கு (size_t k = 0; safe_to_unmount && k < இலக்கம்_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			க்கு (size_t k = 0; safe_to_unmount && k < இலக்கம்_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			எனில் (!safe_to_unmount)
				தொடர்;
			எனில் (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_index--] = mounts[--இலக்கம்_mounts];
			} இல்லையெனில் {
				நிறுத்து;
			}
		}
		எனில் (இலக்கம்_running == RUN_MAX || unmount_index >= இலக்கம்_mounts) {
			state = STATE_REAP;
			நிறுத்து;
		}
		rp = running + இலக்கம்_running++;
		rp->mntpath = mounts[unmount_index];
		rp->last_exec_time = tm_now();
		rp->try_மொத்தம் = 0;
		rp->pid = run_umount(rp->mntpath, umount_args, umount_args_இலக்கம்);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_index] = mounts[--இலக்கம்_mounts];
		நிறுத்து;
	நிலை STATE_REAP:
		flags = (இலக்கம்_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = வெற்று;
		க்கு (size_t i = 0; i < இலக்கம்_running; ++i) {
			rp = running + i;
			எனில் (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			எனில் (rp->pid == pid && pid > 0)
				நிறுத்து;
			rp = வெற்று;
		}
		எனில் (rp) {
			எனில் ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--இலக்கம்_running];
				state = STATE_RUN;
			} இல்லையெனில் எனில் (rp->try_மொத்தம் >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--இலக்கம்_running];
				result = EXIT_FAILURE;
			} இல்லையெனில் { /* put into waiting queue */
				rp->pid = -1;
				rp->try_மொத்தம் += 1;
				இலக்கம்_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} இல்லையெனில் {
			state = STATE_RETRY;
		}
		நிறுத்து;
	நிலை STATE_RETRY:
		rp = வெற்று;
		next_retry = INT64_MAX;
		க்கு (size_t i = 0; i < இலக்கம்_running; ++i) {
			எனில் (running[i].pid > 0)
				தொடர்;
			எனில் (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			இல்லையெனில்
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			எனில் (tmp < next_retry) {
				rp = running + i;
				next_retry = tmp;
			}
		}
		எனில் (!rp) {
			state = (இலக்கம்_mounts > 0) ? STATE_RUN : STATE_END;
			நிறுத்து;
		}
		now = tm_now();
		எனில் ((rp->pid > 0 || rp->fuser_pid > 0) && next_retry > now) {
			int64_t sleep_for = next_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			எனில் (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		எனில் ((rp->pid == -1 && rp->fuser_pid == -1) || next_retry <= now) {
			எனில் (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, வெற்று, 0);
				rp->fuser_pid = -1;
			}
			எனில் (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--இலக்கம்_running];
				result = EXIT_FAILURE;
			} இல்லையெனில் { /* retry */
				rp->last_exec_time = tm_now();
				rp->pid = run_umount(rp->mntpath,
					umount_args, umount_args_இலக்கம்);
			}
			இலக்கம்_waiting -= 1;
			state = STATE_REAP;
		}
		நிறுத்து;
	இயல்பு: நிறுத்து;
	}

exit:
	விடுவி(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(args.fstype_regex);
	REG_FREE(args.skip_fstype_regex);
	REG_FREE(args.முனை_regex);
	REG_FREE(args.skip_முனை_regex);
	REG_FREE(args.options_regex);
	REG_FREE(args.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	திரும்பு result;
}
