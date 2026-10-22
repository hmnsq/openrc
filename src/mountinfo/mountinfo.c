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

#ଯଦି defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#ନଚେତଯଦି defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#ନଚେତଯଦି defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#ନଚେତଯଦି defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#endif

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <regex.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "rc_exec.h"
#ଅନ୍ତର୍ଭାଵ "timeutils.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *procmounts = "/proc/mounts";
ଅଚଳ ଅକ୍ଷର *extraopts = "[mount1] [mount2] ...";
ଅଚଳ ଅକ୍ଷର getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "fstype-regex",        1, ଖାଲି, 'f'},
	{ "skip-fstype-regex",   1, ଖାଲି, 'F'},
	{ "node-regex",          1, ଖାଲି, 'n'},
	{ "skip-node-regex",     1, ଖାଲି, 'N'},
	{ "options-regex",       1, ଖାଲି, 'o'},
	{ "skip-options-regex",  1, ଖାଲି, 'O'},
	{ "point-regex",         1, ଖାଲି, 'p'},
	{ "skip-point-regex",    1, ଖାଲି, 'P'},
	{ "options",             0, ଖାଲି, 'i'},
	{ "fstype",              0, ଖାଲି, 's'},
	{ "node",                0, ଖାଲି, 't'},
	{ "netdev",              0, ଖାଲି, 'e'},
	{ "nonetdev",            0, ଖାଲି, 'E'},
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[] = {
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
ଅଚଳ ଅକ୍ଷର *usagestring = ଖାଲି;

#define UMOUNT_ARGS_MAX 16
#define RUN_MAX 32
#define TRY_MAX 3
#define TRY_DELAY_MS 1000

ସଂରଚନା run_queue {
	ଅଚଳ ଅକ୍ଷର *mntpath;
	int64_t last_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା try_ଗଣନା;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fuser_stdoutfd;
};

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
	mount_from,
	mount_to,
	mount_fstype,
	mount_options
} mount_type;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
	net_ignore,
	net_yes,
	net_no
} net_opts;

ସଂରଚନା args;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା process_func_t(RC_STRINGLIST *, ସଂରଚନା args *,
    ଅକ୍ଷର *, ଅକ୍ଷର *, ଅକ୍ଷର *, ଅକ୍ଷର *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

ସଂରଚନା args {
	process_func_t *process;
	regex_t *ଗଣ୍ଠି_regex;
	regex_t *skip_ଗଣ୍ଠି_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	ଅଚଳ ଅକ୍ଷର *check_mntpath;
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
process_mount(RC_STRINGLIST *list, ସଂରଚନା args *args,
    ଅକ୍ଷର *from, ଅକ୍ଷର *to, ଅକ୍ଷର *fstype, ଅକ୍ଷର *options,
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା netdev)
{
	ଅକ୍ଷର *p;
	RC_STRING *s;

	errno = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	ଯଦି (strcmp(fstype, "rootfs") == 0)
		ଫେରନ୍ତୁ -1;
#endif

	ଯଦି (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		ଯଦି (netdev != 0)
			ଫେରନ୍ତୁ 1;
	} ଅନ୍ୟଥା ଯଦି (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		ଯଦି (netdev != 1)
			ଫେରନ୍ତୁ 1;
	} ଅନ୍ୟଥା {
		ଯଦି (args->ଗଣ୍ଠି_regex &&
		    regexec(args->ଗଣ୍ଠି_regex, from, 0, ଖାଲି, 0) != 0)
			ଫେରନ୍ତୁ 1;
		ଯଦି (args->skip_ଗଣ୍ଠି_regex &&
		    regexec(args->skip_ଗଣ୍ଠି_regex, from, 0, ଖାଲି, 0) == 0)
			ଫେରନ୍ତୁ 1;

		ଯଦି (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, ଖାଲି, 0) != 0)
			ଫେରନ୍ତୁ -1;
		ଯଦି (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, ଖାଲି, 0) == 0)
			ଫେରନ୍ତୁ -1;

		ଯଦି (args->options_regex &&
		    regexec(args->options_regex, options, 0, ଖାଲି, 0) != 0)
			ଫେରନ୍ତୁ -1;
		ଯଦି (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, ଖାଲି, 0) == 0)
			ଫେରନ୍ତୁ -1;
	}

	ଯଦି (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    ଯଦି (strcmp(s->ମୂଲ୍ୟ, to) == 0)
			    ଭାଙ୍ଗ;
		ଯଦି (!s)
			ଫେରନ୍ତୁ -1;
	}

	ବାଛ (args->mount_type) {
	ସ୍ଥିତି mount_from:
		p = from;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି mount_to:
		p = to;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି mount_fstype:
		p = fstype;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି mount_options:
		p = options;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		p = ଖାଲି;
		errno = EINVAL;
		ଭାଙ୍ଗ;
	}

	ଯଦି (p) {
		errno = 0;
		rc_stringlist_add(list, p);
		ଫେରନ୍ତୁ 0;
	}

	ଫେରନ୍ତୁ -1;
}

#ଯଦି defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
ସ୍ଥିର ସଂରଚନା opt {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା o_opt;
	ଅଚଳ ଅକ୍ଷର *o_name;
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
	{ 0, ଖାଲି }
};

ସ୍ଥିର RC_STRINGLIST *
find_mounts(ସଂରଚନା args *args, size_t *ଅଙ୍କ_mounts)
{
	ସଂରଚନା statfs *mnts;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nmnts;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	RC_STRINGLIST *list;
	ଅକ୍ଷର *options = ଖାଲି;
	uint64_t flags;
	ସଂରଚନା opt *o;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା netdev;
	ଅକ୍ଷର *tmp;

	ଯଦି ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	ପାଇଁ (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		ପାଇଁ (o = optnames; flags && o->o_opt; o++) {
			ଯଦି (flags & o->o_opt) {
				ଯଦି (o->o_opt == MNT_LOCAL)
					netdev = 1;
				ଯଦି (!options)
					options = xstrdup(o->o_name);
				ଅନ୍ୟଥା {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					ମୁକ୍ତ(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*ଅଙ୍କ_mounts += 0 == args->process(list, args,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		ମୁକ୍ତ(options);
		options = ଖାଲି;
	}

	ଫେରନ୍ତୁ list;
}

#ନଚେତଯଦି defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
ସ୍ଥିର ସଂରଚନା mntent *
getmntfile(ଅଚଳ ଅକ୍ଷର *file)
{
	ସଂରଚନା mntent *ent = ଖାଲି;
	FILE *fp;

	ଯଦି (!(fp = setmntent("/etc/fstab", "r")))
		ଫେରନ୍ତୁ ଖାଲି;
	ଯେତେବେଳେ ((ent = getmntent(fp)))
		ଯଦି (strcmp(file, ent->mnt_dir) == 0)
			ଭାଙ୍ଗ;
	endmntent(fp);

	ଫେରନ୍ତୁ ent;
}

ସ୍ଥିର RC_STRINGLIST *
find_mounts(ସଂରଚନା args *args, size_t *ଅଙ୍କ_mounts)
{
	FILE *fp;
	ଅକ୍ଷର *buffer;
	size_t size;
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *from;
	ଅକ୍ଷର *to;
	ଅକ୍ଷର *fst;
	ଅକ୍ଷର *opts;
	ସଂରଚନା mntent *ent;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା netdev;
	RC_STRINGLIST *list;

	ଯଦି ((fp = fopen(procmounts, "r")) == ଖାଲି)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	buffer = ଖାଲି;
	ଯେତେବେଳେ (xgetline(&buffer, &size, fp) != -1) {
		netdev = -1;
		p = buffer;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		ଯଦି ((ent = getmntfile(to))) {
			ଯଦି (strstr(ent->mnt_opts, "_netdev"))
				netdev = 0;
			ଅନ୍ୟଥା
				netdev = 1;
		}

		*ଅଙ୍କ_mounts += 0 == args->process(list, args,
			from, to, fst, opts, netdev);

		ମୁକ୍ତ(buffer);
		buffer = ଖାଲି;
	}
	ମୁକ୍ତ(buffer);
	fclose(fp);

	ଫେରନ୍ତୁ list;
}

#ଅନ୍ୟଥା
#  error "Operating system not supported!"
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_prefix(ଅଚଳ ଅକ୍ଷର *needle, ଅଚଳ ଅକ୍ଷର *hay)
{
	size_t nlen = strlen(needle);
	ଯଦି (strncmp(needle, hay, nlen) == 0 && hay[nlen] == '/')
		ଫେରନ୍ତୁ true;
	ଫେରନ୍ତୁ false;
}

ସ୍ଥିର ଅକ୍ଷର *unescape_octal(ଅକ୍ଷର *beg)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, i;
	ଅକ୍ଷର *w = beg, *r = beg;
	ଯେତେବେଳେ (*r) {
		ଯଦି (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} ଅନ୍ୟଥା {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			ପାଇଁ (i = n = 0; i < 3; ++i) {
				ଯଦି (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} ଅନ୍ୟଥା {
					ଭାଙ୍ଗ;
				}
			}
			ଯଦି (n)
				*w++ = n;
		}
	}
	*w = '\0';
	ଫେରନ୍ତୁ beg;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା check_is_mounted(RC_STRINGLIST *list RC_UNUSED, ସଂରଚନା args *args,
    ଅକ୍ଷର *from RC_UNUSED, ଅକ୍ଷର *to, ଅକ୍ଷର *fstype RC_UNUSED,
    ଅକ୍ଷର *options RC_UNUSED, ପୂର୍ଣ୍ଣସଂଖ୍ୟା netdev RC_UNUSED)
{
	ଫେରନ୍ତୁ strcmp(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_mounted(ଅଚଳ ଅକ୍ଷର *mntpath)
{
	size_t ଅଙ୍କ_mounts = 0;
	ସଂରଚନା args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &ଅଙ୍କ_mounts);
	rc_stringlist_free(l);
	ଫେରନ୍ତୁ ଅଙ୍କ_mounts > 0;
}

ସ୍ଥିର pid_t run_umount(ଅଚଳ ଅକ୍ଷର *mntpath,
	ଅଚଳ ଅକ୍ଷର **umount_args, ପୂର୍ଣ୍ଣସଂଖ୍ୟା umount_args_ଅଙ୍କ)
{
	ସଂରଚନା exec_args args;
	ସଂରଚନା exec_result res;
	ଅଚଳ ଅକ୍ଷର *argv[UMOUNT_ARGS_MAX + 3];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା k, i = 0;

	argv[i++] = "umount";
	ପାଇଁ (k = 0; k < umount_args_ଅଙ୍କ; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = ଖାଲି;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	ଯଦି (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	ଫେରନ୍ତୁ res.pid;
}

ସ୍ଥିର ଶୂନ୍ୟ fuser_run(ସଂରଚନା run_queue *rp, ଅଚଳ ଅକ୍ଷର *fuser_opt)
{
	ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fuser_exec_failed = 0;
	ଅଚଳ ଅକ୍ଷର *argv[] = { "fuser", fuser_opt, rp->mntpath, ଖାଲି };
	ସଂରଚନା exec_result res;
	ସଂରଚନା exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	ଯଦି (fuser_exec_failed)
		ଫେରନ୍ତୁ;

	args = exec_init(argv);
	args.redirect_stdout = EXEC_MKPIPE;
	args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	ଯଦି (res.pid < 0) {
		fuser_exec_failed = 1;
	} ଅନ୍ୟଥା {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fuser_decide(ସଂରଚନା run_queue *rp,
	ଅଚଳ ଅକ୍ଷର *fuser_opt, ଅଚଳ ଅକ୍ଷର *fuser_kill_prefix)
{
	ସ୍ଥିର bool has_rc_pid = false;
	ସ୍ଥିର ଅକ୍ଷର *rc_pid;
	ଅକ୍ଷର buf[1<<12];
	ଅକ୍ଷର selfpid[64];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା read_maybe_truncated;
	ssize_t n;

	ଯଦି (rp->fuser_stdoutfd < 0)
		ଫେରନ୍ତୁ 0;

	buf[0] = ' ';
	n = ପଠ(rp->fuser_stdoutfd, buf + 1, ଆକାର buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == ଆକାର buf - 3);
	ଯଦି (n < 0 || read_maybe_truncated)
		ଫେରନ୍ତୁ 0;
	ଯେତେବେଳେ (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, ଆକାର selfpid, " %lld ", (long long)getpid());

	ଯଦି (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	ଯଦି (strstr(buf, selfpid) || (rc_pid && strstr(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		ଫେରନ୍ତୁ -1;
	} ଅନ୍ୟଥା ଯଦି (strcmp(buf, "  ") == 0) {
		ଯଦି (rp->try_ଗଣନା >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			ଫେରନ୍ତୁ -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_ଗଣନା = TRY_MAX;
		ଫେରନ୍ତୁ 0;
	} ଅନ୍ୟଥା {
		ଅକ୍ଷର sig[32];
		ଅଚଳ ଅକ୍ଷର *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, ଖାଲି
		};
		ସଂରଚନା exec_result res;
		ସଂରଚନା exec_args args = exec_init(argv);
		args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, ଆକାର sig, "%s%s", fuser_kill_prefix,
			rp->try_ଗଣନା == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&args);
		ଯଦି (res.pid > 0)
			rc_waitpid(res.pid);
		ଫେରନ୍ତୁ 0;
	}
}

ସ୍ଥିର regex_t *
get_regex(ଅଚଳ ଅକ୍ଷର *string)
{
	regex_t *reg = xmalloc(ଆକାର (*reg));
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;
	ଅକ୍ଷର buffer[256];

	ଯଦି ((result = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(result, reg, buffer, ଆକାର(buffer));
		eerrorx("%s: invalid regex `%s'", applet, buffer);
	}

	ଫେରନ୍ତୁ reg;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ସଂରଚନା args args;
	regex_t *point_regex = ଖାଲି;
	regex_t *skip_point_regex = ଖାଲି;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	ଅକ୍ଷର *real_path = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;
	ଅକ୍ଷର *this_path, *argv0;
	ଅଚଳ ଅକ୍ଷର *tmps;
	ଅଚଳ ଅକ୍ଷର *fuser_opt, *fuser_kill_prefix;
	size_t ଅଙ୍କ_mounts = 0;
	size_t unmount_index;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା doing_unmount = 0;
	pid_t pid;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status, flags;
	int64_t tmp, next_retry, now;
	int64_t rc_fuser_timeout = -1;
	ଅଚଳ ଅକ୍ଷର *umount_args[UMOUNT_ARGS_MAX];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା umount_args_ଅଙ୍କ = 0;
	ଅଚଳ ଅକ୍ଷର **mounts = ଖାଲି;
	ସଂରଚନା run_queue running[RUN_MAX] = {0};
	ସଂରଚନା run_queue *rp;
	size_t ଅଙ୍କ_running = 0, ଅଙ୍କ_waiting = 0;
	ଗଣନା { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	ଯଦି (_var) ମୁକ୍ତ(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	ଯଦି (_var) { regfree(_var); ମୁକ୍ତ(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&args, 0, ଆକାର(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	ଯଦି (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		ଯେତେବେଳେ (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			ଯଦି (strcmp(tmps, "--") == 0)
				ଭାଙ୍ଗ;
			ଯଦି (umount_args_ଅଙ୍କ >= (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ARRAY_SIZE(umount_args))
				eerrorx("%s: Too many umount arguments", applet);
			umount_args[umount_args_ଅଙ୍କ++] = tmps;
		}

		tmps = rc_conf_ମୂଲ୍ୟ("rc_fuser_timeout");
		ଯଦି (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		ଯଦି (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		ଯଦି (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} ଅନ୍ୟଥା {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) 0)) != -1)
	{
		ବାଛ (opt) {
		ସ୍ଥିତି 'e':
			args.netdev = net_yes;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'E':
			args.netdev = net_no;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'f':
			DO_REG(args.fstype_regex);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'F':
			DO_REG(args.skip_fstype_regex);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'n':
			DO_REG(args.ଗଣ୍ଠି_regex);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'N':
			DO_REG(args.skip_ଗଣ୍ଠି_regex);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'o':
			DO_REG(args.options_regex);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'O':
			DO_REG(args.skip_options_regex);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'p':
			DO_REG(point_regex);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'P':
			DO_REG(skip_point_regex);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'i':
			args.mount_type = mount_options;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 's':
			args.mount_type = mount_fstype;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 't':
			args.mount_type = mount_from;
			ଭାଙ୍ଗ;

		case_RC_COMMON_GETOPT
		}
	}

	ଯେତେବେଳେ (optind < argc) {
		ଯଦି (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, ଖାଲି);
		ଯଦି (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		ମୁକ୍ତ(real_path);
		real_path = ଖାଲି;
	}
	nodes = find_mounts(&args, &ଅଙ୍କ_mounts);
	rc_stringlist_free(args.mounts);

	ଯଦି (doing_unmount)
		mounts = xmalloc(ଅଙ୍କ_mounts * ଆକାର(*mounts));
	ଅଙ୍କ_mounts = 0;
	result = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		ଯଦି (point_regex &&
		    regexec(point_regex, s->ମୂଲ୍ୟ, 0, ଖାଲି, 0) != 0)
			ଜାରି;
		ଯଦି (skip_point_regex &&
		    regexec(skip_point_regex, s->ମୂଲ୍ୟ, 0, ଖାଲି, 0) == 0)
			ଜାରି;
		ଯଦି (doing_unmount)
			mounts[ଅଙ୍କ_mounts++] = unescape_octal(s->ମୂଲ୍ୟ);
		ଅନ୍ୟଥା ଯଦି (!rc_yesno(getenv("EINFO_QUIET")))
			printf("%s\n", s->ମୂଲ୍ୟ);
		result = EXIT_SUCCESS;
	}
	ଯଦି (!doing_unmount)
		ଯାଅ exit;

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
	ଯେତେବେଳେ (state != STATE_END) ବାଛ (state) {
	ସ୍ଥିତି STATE_RUN:
		ପାଇଁ (unmount_index = 0; unmount_index < ଅଙ୍କ_mounts; ++unmount_index) {
			ଅଚଳ ଅକ୍ଷର *candidate = mounts[unmount_index];
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା safe_to_unmount = 1;
			ପାଇଁ (size_t k = 0; safe_to_unmount && k < ଅଙ୍କ_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			ପାଇଁ (size_t k = 0; safe_to_unmount && k < ଅଙ୍କ_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			ଯଦି (!safe_to_unmount)
				ଜାରି;
			ଯଦି (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_index--] = mounts[--ଅଙ୍କ_mounts];
			} ଅନ୍ୟଥା {
				ଭାଙ୍ଗ;
			}
		}
		ଯଦି (ଅଙ୍କ_running == RUN_MAX || unmount_index >= ଅଙ୍କ_mounts) {
			state = STATE_REAP;
			ଭାଙ୍ଗ;
		}
		rp = running + ଅଙ୍କ_running++;
		rp->mntpath = mounts[unmount_index];
		rp->last_exec_time = tm_now();
		rp->try_ଗଣନା = 0;
		rp->pid = run_umount(rp->mntpath, umount_args, umount_args_ଅଙ୍କ);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_index] = mounts[--ଅଙ୍କ_mounts];
		ଭାଙ୍ଗ;
	ସ୍ଥିତି STATE_REAP:
		flags = (ଅଙ୍କ_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = ଖାଲି;
		ପାଇଁ (size_t i = 0; i < ଅଙ୍କ_running; ++i) {
			rp = running + i;
			ଯଦି (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			ଯଦି (rp->pid == pid && pid > 0)
				ଭାଙ୍ଗ;
			rp = ଖାଲି;
		}
		ଯଦି (rp) {
			ଯଦି ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--ଅଙ୍କ_running];
				state = STATE_RUN;
			} ଅନ୍ୟଥା ଯଦି (rp->try_ଗଣନା >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--ଅଙ୍କ_running];
				result = EXIT_FAILURE;
			} ଅନ୍ୟଥା { /* put into waiting queue */
				rp->pid = -1;
				rp->try_ଗଣନା += 1;
				ଅଙ୍କ_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} ଅନ୍ୟଥା {
			state = STATE_RETRY;
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି STATE_RETRY:
		rp = ଖାଲି;
		next_retry = INT64_MAX;
		ପାଇଁ (size_t i = 0; i < ଅଙ୍କ_running; ++i) {
			ଯଦି (running[i].pid > 0)
				ଜାରି;
			ଯଦି (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			ଅନ୍ୟଥା
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			ଯଦି (tmp < next_retry) {
				rp = running + i;
				next_retry = tmp;
			}
		}
		ଯଦି (!rp) {
			state = (ଅଙ୍କ_mounts > 0) ? STATE_RUN : STATE_END;
			ଭାଙ୍ଗ;
		}
		now = tm_now();
		ଯଦି ((rp->pid > 0 || rp->fuser_pid > 0) && next_retry > now) {
			int64_t sleep_for = next_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			ଯଦି (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		ଯଦି ((rp->pid == -1 && rp->fuser_pid == -1) || next_retry <= now) {
			ଯଦି (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, ଖାଲି, 0);
				rp->fuser_pid = -1;
			}
			ଯଦି (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--ଅଙ୍କ_running];
				result = EXIT_FAILURE;
			} ଅନ୍ୟଥା { /* retry */
				rp->last_exec_time = tm_now();
				rp->pid = run_umount(rp->mntpath,
					umount_args, umount_args_ଅଙ୍କ);
			}
			ଅଙ୍କ_waiting -= 1;
			state = STATE_REAP;
		}
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଭାଙ୍ଗ;
	}

exit:
	ମୁକ୍ତ(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(args.fstype_regex);
	REG_FREE(args.skip_fstype_regex);
	REG_FREE(args.ଗଣ୍ଠି_regex);
	REG_FREE(args.skip_ଗଣ୍ଠି_regex);
	REG_FREE(args.options_regex);
	REG_FREE(args.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	ଫେରନ୍ତୁ result;
}
