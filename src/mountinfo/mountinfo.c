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

#सचे defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#नोचेद्यदि defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#नोचेद्यदि defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#नोचेद्यदि defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#endif

#अन्तर्भाव <errno.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <regex.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "rc_exec.h"
#अन्तर्भाव "timeutils.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"

ध्रुव वर्ण *applet = रिक्त;
ध्रुव वर्ण *procmounts = "/proc/mounts";
ध्रुव वर्ण *extraopts = "[mount1] [mount2] ...";
ध्रुव वर्ण getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
ध्रुव संरचना option longopts[] = {
	{ "fstype-regex",        1, रिक्त, 'f'},
	{ "skip-fstype-regex",   1, रिक्त, 'F'},
	{ "node-regex",          1, रिक्त, 'n'},
	{ "skip-node-regex",     1, रिक्त, 'N'},
	{ "options-regex",       1, रिक्त, 'o'},
	{ "skip-options-regex",  1, रिक्त, 'O'},
	{ "point-regex",         1, रिक्त, 'p'},
	{ "skip-point-regex",    1, रिक्त, 'P'},
	{ "options",             0, रिक्त, 'i'},
	{ "fstype",              0, रिक्त, 's'},
	{ "node",                0, रिक्त, 't'},
	{ "netdev",              0, रिक्त, 'e'},
	{ "nonetdev",            0, रिक्त, 'E'},
	longopts_COMMON
};
ध्रुव वर्ण * ध्रुव longopts_help[] = {
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
ध्रुव वर्ण *usagestring = रिक्त;

#define UMOUNT_ARGS_MAX 16
#define RUN_MAX 32
#define TRY_MAX 3
#define TRY_DELAY_MS 1000

संरचना run_queue {
	ध्रुव वर्ण *mntpath;
	int64_t last_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	पूर्णाङ्क try_गणनम्;
	पूर्णाङ्क fuser_stdoutfd;
};

प्रकारनिर्देश गणना {
	mount_from,
	mount_to,
	mount_fstype,
	mount_options
} mount_type;

प्रकारनिर्देश गणना {
	net_ignore,
	net_yes,
	net_no
} net_opts;

संरचना args;
प्रकारनिर्देश पूर्णाङ्क process_func_t(RC_STRINGLIST *, संरचना args *,
    वर्ण *, वर्ण *, वर्ण *, वर्ण *, पूर्णाङ्क);

संरचना args {
	process_func_t *process;
	regex_t *ग्रन्थिः_regex;
	regex_t *skip_ग्रन्थिः_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	ध्रुव वर्ण *check_mntpath;
};

स्थिर पूर्णाङ्क
process_mount(RC_STRINGLIST *list, संरचना args *args,
    वर्ण *from, वर्ण *to, वर्ण *fstype, वर्ण *options,
    पूर्णाङ्क netdev)
{
	वर्ण *p;
	RC_STRING *s;

	errno = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	सचे (strcmp(fstype, "rootfs") == 0)
		पटिनिवत्त -1;
#endif

	सचे (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		सचे (netdev != 0)
			पटिनिवत्त 1;
	} अञ्ञथा सचे (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		सचे (netdev != 1)
			पटिनिवत्त 1;
	} अञ्ञथा {
		सचे (args->ग्रन्थिः_regex &&
		    regexec(args->ग्रन्थिः_regex, from, 0, रिक्त, 0) != 0)
			पटिनिवत्त 1;
		सचे (args->skip_ग्रन्थिः_regex &&
		    regexec(args->skip_ग्रन्थिः_regex, from, 0, रिक्त, 0) == 0)
			पटिनिवत्त 1;

		सचे (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, रिक्त, 0) != 0)
			पटिनिवत्त -1;
		सचे (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, रिक्त, 0) == 0)
			पटिनिवत्त -1;

		सचे (args->options_regex &&
		    regexec(args->options_regex, options, 0, रिक्त, 0) != 0)
			पटिनिवत्त -1;
		सचे (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, रिक्त, 0) == 0)
			पटिनिवत्त -1;
	}

	सचे (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    सचे (strcmp(s->मानम्, to) == 0)
			    भङ्ग;
		सचे (!s)
			पटिनिवत्त -1;
	}

	विकल्प (args->mount_type) {
	पक्ष mount_from:
		p = from;
		भङ्ग;
	पक्ष mount_to:
		p = to;
		भङ्ग;
	पक्ष mount_fstype:
		p = fstype;
		भङ्ग;
	पक्ष mount_options:
		p = options;
		भङ्ग;
	स्वभाव:
		p = रिक्त;
		errno = EINVAL;
		भङ्ग;
	}

	सचे (p) {
		errno = 0;
		rc_stringlist_add(list, p);
		पटिनिवत्त 0;
	}

	पटिनिवत्त -1;
}

#सचे defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
स्थिर संरचना opt {
	पूर्णाङ्क o_opt;
	ध्रुव वर्ण *o_name;
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
	{ 0, रिक्त }
};

स्थिर RC_STRINGLIST *
find_mounts(संरचना args *args, size_t *अङ्कः_mounts)
{
	संरचना statfs *mnts;
	पूर्णाङ्क nmnts;
	पूर्णाङ्क i;
	RC_STRINGLIST *list;
	वर्ण *options = रिक्त;
	uint64_t flags;
	संरचना opt *o;
	पूर्णाङ्क netdev;
	वर्ण *tmp;

	सचे ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	अत्थाय (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		अत्थाय (o = optnames; flags && o->o_opt; o++) {
			सचे (flags & o->o_opt) {
				सचे (o->o_opt == MNT_LOCAL)
					netdev = 1;
				सचे (!options)
					options = xstrdup(o->o_name);
				अञ्ञथा {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					मुक्त(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*अङ्कः_mounts += 0 == args->process(list, args,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		मुक्त(options);
		options = रिक्त;
	}

	पटिनिवत्त list;
}

#नोचेद्यदि defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
स्थिर संरचना mntent *
getmntfile(ध्रुव वर्ण *file)
{
	संरचना mntent *ent = रिक्त;
	FILE *fp;

	सचे (!(fp = setmntent("/etc/fstab", "r")))
		पटिनिवत्त रिक्त;
	याव ((ent = getmntent(fp)))
		सचे (strcmp(file, ent->mnt_dir) == 0)
			भङ्ग;
	endmntent(fp);

	पटिनिवत्त ent;
}

स्थिर RC_STRINGLIST *
find_mounts(संरचना args *args, size_t *अङ्कः_mounts)
{
	FILE *fp;
	वर्ण *buffer;
	size_t size;
	वर्ण *p;
	वर्ण *from;
	वर्ण *to;
	वर्ण *fst;
	वर्ण *opts;
	संरचना mntent *ent;
	पूर्णाङ्क netdev;
	RC_STRINGLIST *list;

	सचे ((fp = fopen(procmounts, "r")) == रिक्त)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	buffer = रिक्त;
	याव (xgetline(&buffer, &size, fp) != -1) {
		netdev = -1;
		p = buffer;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		सचे ((ent = getmntfile(to))) {
			सचे (strstr(ent->mnt_opts, "_netdev"))
				netdev = 0;
			अञ्ञथा
				netdev = 1;
		}

		*अङ्कः_mounts += 0 == args->process(list, args,
			from, to, fst, opts, netdev);

		मुक्त(buffer);
		buffer = रिक्त;
	}
	मुक्त(buffer);
	fclose(fp);

	पटिनिवत्त list;
}

#अञ्ञथा
#  error "Operating system not supported!"
#endif

स्थिर पूर्णाङ्क is_prefix(ध्रुव वर्ण *needle, ध्रुव वर्ण *hay)
{
	size_t nlen = strlen(needle);
	सचे (strncmp(needle, hay, nlen) == 0 && hay[nlen] == '/')
		पटिनिवत्त true;
	पटिनिवत्त false;
}

स्थिर वर्ण *unescape_octal(वर्ण *beg)
{
	पूर्णाङ्क n, i;
	वर्ण *w = beg, *r = beg;
	याव (*r) {
		सचे (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} अञ्ञथा {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			अत्थाय (i = n = 0; i < 3; ++i) {
				सचे (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} अञ्ञथा {
					भङ्ग;
				}
			}
			सचे (n)
				*w++ = n;
		}
	}
	*w = '\0';
	पटिनिवत्त beg;
}

स्थिर पूर्णाङ्क check_is_mounted(RC_STRINGLIST *list RC_UNUSED, संरचना args *args,
    वर्ण *from RC_UNUSED, वर्ण *to, वर्ण *fstype RC_UNUSED,
    वर्ण *options RC_UNUSED, पूर्णाङ्क netdev RC_UNUSED)
{
	पटिनिवत्त strcmp(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

स्थिर पूर्णाङ्क is_mounted(ध्रुव वर्ण *mntpath)
{
	size_t अङ्कः_mounts = 0;
	संरचना args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &अङ्कः_mounts);
	rc_stringlist_free(l);
	पटिनिवत्त अङ्कः_mounts > 0;
}

स्थिर pid_t run_umount(ध्रुव वर्ण *mntpath,
	ध्रुव वर्ण **umount_args, पूर्णाङ्क umount_args_अङ्कः)
{
	संरचना exec_args args;
	संरचना exec_result res;
	ध्रुव वर्ण *argv[UMOUNT_ARGS_MAX + 3];
	पूर्णाङ्क k, i = 0;

	argv[i++] = "umount";
	अत्थाय (k = 0; k < umount_args_अङ्कः; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = रिक्त;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	सचे (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	पटिनिवत्त res.pid;
}

स्थिर शून्य fuser_run(संरचना run_queue *rp, ध्रुव वर्ण *fuser_opt)
{
	स्थिर पूर्णाङ्क fuser_exec_failed = 0;
	ध्रुव वर्ण *argv[] = { "fuser", fuser_opt, rp->mntpath, रिक्त };
	संरचना exec_result res;
	संरचना exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	सचे (fuser_exec_failed)
		पटिनिवत्त;

	args = exec_init(argv);
	args.redirect_stdout = EXEC_MKPIPE;
	args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	सचे (res.pid < 0) {
		fuser_exec_failed = 1;
	} अञ्ञथा {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

स्थिर पूर्णाङ्क fuser_decide(संरचना run_queue *rp,
	ध्रुव वर्ण *fuser_opt, ध्रुव वर्ण *fuser_kill_prefix)
{
	स्थिर bool has_rc_pid = false;
	स्थिर वर्ण *rc_pid;
	वर्ण buf[1<<12];
	वर्ण selfpid[64];
	पूर्णाङ्क read_maybe_truncated;
	ssize_t n;

	सचे (rp->fuser_stdoutfd < 0)
		पटिनिवत्त 0;

	buf[0] = ' ';
	n = पठ(rp->fuser_stdoutfd, buf + 1, परिमाण buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == परिमाण buf - 3);
	सचे (n < 0 || read_maybe_truncated)
		पटिनिवत्त 0;
	याव (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, परिमाण selfpid, " %lld ", (long long)getpid());

	सचे (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	सचे (strstr(buf, selfpid) || (rc_pid && strstr(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		पटिनिवत्त -1;
	} अञ्ञथा सचे (strcmp(buf, "  ") == 0) {
		सचे (rp->try_गणनम् >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			पटिनिवत्त -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_गणनम् = TRY_MAX;
		पटिनिवत्त 0;
	} अञ्ञथा {
		वर्ण sig[32];
		ध्रुव वर्ण *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, रिक्त
		};
		संरचना exec_result res;
		संरचना exec_args args = exec_init(argv);
		args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, परिमाण sig, "%s%s", fuser_kill_prefix,
			rp->try_गणनम् == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&args);
		सचे (res.pid > 0)
			rc_waitpid(res.pid);
		पटिनिवत्त 0;
	}
}

स्थिर regex_t *
get_regex(ध्रुव वर्ण *string)
{
	regex_t *reg = xmalloc(परिमाण (*reg));
	पूर्णाङ्क result;
	वर्ण buffer[256];

	सचे ((result = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(result, reg, buffer, परिमाण(buffer));
		eerrorx("%s: invalid regex `%s'", applet, buffer);
	}

	पटिनिवत्त reg;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	संरचना args args;
	regex_t *point_regex = रिक्त;
	regex_t *skip_point_regex = रिक्त;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	वर्ण *real_path = रिक्त;
	पूर्णाङ्क opt;
	पूर्णाङ्क result;
	वर्ण *this_path, *argv0;
	ध्रुव वर्ण *tmps;
	ध्रुव वर्ण *fuser_opt, *fuser_kill_prefix;
	size_t अङ्कः_mounts = 0;
	size_t unmount_index;
	पूर्णाङ्क doing_unmount = 0;
	pid_t pid;
	पूर्णाङ्क status, flags;
	int64_t tmp, next_retry, now;
	int64_t rc_fuser_timeout = -1;
	ध्रुव वर्ण *umount_args[UMOUNT_ARGS_MAX];
	पूर्णाङ्क umount_args_अङ्कः = 0;
	ध्रुव वर्ण **mounts = रिक्त;
	संरचना run_queue running[RUN_MAX] = {0};
	संरचना run_queue *rp;
	size_t अङ्कः_running = 0, अङ्कः_waiting = 0;
	गणना { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	सचे (_var) मुक्त(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	सचे (_var) { regfree(_var); मुक्त(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&args, 0, परिमाण(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	सचे (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		याव (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			सचे (strcmp(tmps, "--") == 0)
				भङ्ग;
			सचे (umount_args_अङ्कः >= (पूर्णाङ्क)ARRAY_SIZE(umount_args))
				eerrorx("%s: Too many umount arguments", applet);
			umount_args[umount_args_अङ्कः++] = tmps;
		}

		tmps = rc_conf_मानम्("rc_fuser_timeout");
		सचे (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		सचे (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		सचे (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} अञ्ञथा {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	याव ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णाङ्क *) 0)) != -1)
	{
		विकल्प (opt) {
		पक्ष 'e':
			args.netdev = net_yes;
			भङ्ग;
		पक्ष 'E':
			args.netdev = net_no;
			भङ्ग;
		पक्ष 'f':
			DO_REG(args.fstype_regex);
			भङ्ग;
		पक्ष 'F':
			DO_REG(args.skip_fstype_regex);
			भङ्ग;
		पक्ष 'n':
			DO_REG(args.ग्रन्थिः_regex);
			भङ्ग;
		पक्ष 'N':
			DO_REG(args.skip_ग्रन्थिः_regex);
			भङ्ग;
		पक्ष 'o':
			DO_REG(args.options_regex);
			भङ्ग;
		पक्ष 'O':
			DO_REG(args.skip_options_regex);
			भङ्ग;
		पक्ष 'p':
			DO_REG(point_regex);
			भङ्ग;
		पक्ष 'P':
			DO_REG(skip_point_regex);
			भङ्ग;
		पक्ष 'i':
			args.mount_type = mount_options;
			भङ्ग;
		पक्ष 's':
			args.mount_type = mount_fstype;
			भङ्ग;
		पक्ष 't':
			args.mount_type = mount_from;
			भङ्ग;

		case_RC_COMMON_GETOPT
		}
	}

	याव (optind < argc) {
		सचे (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, रिक्त);
		सचे (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		मुक्त(real_path);
		real_path = रिक्त;
	}
	nodes = find_mounts(&args, &अङ्कः_mounts);
	rc_stringlist_free(args.mounts);

	सचे (doing_unmount)
		mounts = xmalloc(अङ्कः_mounts * परिमाण(*mounts));
	अङ्कः_mounts = 0;
	result = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		सचे (point_regex &&
		    regexec(point_regex, s->मानम्, 0, रिक्त, 0) != 0)
			अनुवर्त;
		सचे (skip_point_regex &&
		    regexec(skip_point_regex, s->मानम्, 0, रिक्त, 0) == 0)
			अनुवर्त;
		सचे (doing_unmount)
			mounts[अङ्कः_mounts++] = unescape_octal(s->मानम्);
		अञ्ञथा सचे (!rc_yesno(getenv("EINFO_QUIET")))
			printf("%s\n", s->मानम्);
		result = EXIT_SUCCESS;
	}
	सचे (!doing_unmount)
		गच्छ exit;

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
	याव (state != STATE_END) विकल्प (state) {
	पक्ष STATE_RUN:
		अत्थाय (unmount_index = 0; unmount_index < अङ्कः_mounts; ++unmount_index) {
			ध्रुव वर्ण *candidate = mounts[unmount_index];
			पूर्णाङ्क safe_to_unmount = 1;
			अत्थाय (size_t k = 0; safe_to_unmount && k < अङ्कः_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			अत्थाय (size_t k = 0; safe_to_unmount && k < अङ्कः_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			सचे (!safe_to_unmount)
				अनुवर्त;
			सचे (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_index--] = mounts[--अङ्कः_mounts];
			} अञ्ञथा {
				भङ्ग;
			}
		}
		सचे (अङ्कः_running == RUN_MAX || unmount_index >= अङ्कः_mounts) {
			state = STATE_REAP;
			भङ्ग;
		}
		rp = running + अङ्कः_running++;
		rp->mntpath = mounts[unmount_index];
		rp->last_exec_time = tm_now();
		rp->try_गणनम् = 0;
		rp->pid = run_umount(rp->mntpath, umount_args, umount_args_अङ्कः);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_index] = mounts[--अङ्कः_mounts];
		भङ्ग;
	पक्ष STATE_REAP:
		flags = (अङ्कः_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = रिक्त;
		अत्थाय (size_t i = 0; i < अङ्कः_running; ++i) {
			rp = running + i;
			सचे (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			सचे (rp->pid == pid && pid > 0)
				भङ्ग;
			rp = रिक्त;
		}
		सचे (rp) {
			सचे ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--अङ्कः_running];
				state = STATE_RUN;
			} अञ्ञथा सचे (rp->try_गणनम् >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--अङ्कः_running];
				result = EXIT_FAILURE;
			} अञ्ञथा { /* put into waiting queue */
				rp->pid = -1;
				rp->try_गणनम् += 1;
				अङ्कः_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} अञ्ञथा {
			state = STATE_RETRY;
		}
		भङ्ग;
	पक्ष STATE_RETRY:
		rp = रिक्त;
		next_retry = INT64_MAX;
		अत्थाय (size_t i = 0; i < अङ्कः_running; ++i) {
			सचे (running[i].pid > 0)
				अनुवर्त;
			सचे (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			अञ्ञथा
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			सचे (tmp < next_retry) {
				rp = running + i;
				next_retry = tmp;
			}
		}
		सचे (!rp) {
			state = (अङ्कः_mounts > 0) ? STATE_RUN : STATE_END;
			भङ्ग;
		}
		now = tm_now();
		सचे ((rp->pid > 0 || rp->fuser_pid > 0) && next_retry > now) {
			int64_t sleep_for = next_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			सचे (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		सचे ((rp->pid == -1 && rp->fuser_pid == -1) || next_retry <= now) {
			सचे (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, रिक्त, 0);
				rp->fuser_pid = -1;
			}
			सचे (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--अङ्कः_running];
				result = EXIT_FAILURE;
			} अञ्ञथा { /* retry */
				rp->last_exec_time = tm_now();
				rp->pid = run_umount(rp->mntpath,
					umount_args, umount_args_अङ्कः);
			}
			अङ्कः_waiting -= 1;
			state = STATE_REAP;
		}
		भङ्ग;
	स्वभाव: भङ्ग;
	}

exit:
	मुक्त(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(args.fstype_regex);
	REG_FREE(args.skip_fstype_regex);
	REG_FREE(args.ग्रन्थिः_regex);
	REG_FREE(args.skip_ग्रन्थिः_regex);
	REG_FREE(args.options_regex);
	REG_FREE(args.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	पटिनिवत्त result;
}
