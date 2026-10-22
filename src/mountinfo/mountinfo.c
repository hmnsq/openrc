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

#यदि defined(__DragonFly__) || defined(__FreeBSD__)
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
	पूर्णाङ्क try_गिनती;
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
	regex_t *गाँठ_regex;
	regex_t *skip_गाँठ_regex;
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

	त्रुटि_सं = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	यदि (म_भेद(fstype, "rootfs") == 0)
		प्रत्यागम -1;
#endif

	यदि (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		यदि (netdev != 0)
			प्रत्यागम 1;
	} अन्यथा यदि (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		यदि (netdev != 1)
			प्रत्यागम 1;
	} अन्यथा {
		यदि (args->गाँठ_regex &&
		    regexec(args->गाँठ_regex, from, 0, रिक्त, 0) != 0)
			प्रत्यागम 1;
		यदि (args->skip_गाँठ_regex &&
		    regexec(args->skip_गाँठ_regex, from, 0, रिक्त, 0) == 0)
			प्रत्यागम 1;

		यदि (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, रिक्त, 0) != 0)
			प्रत्यागम -1;
		यदि (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, रिक्त, 0) == 0)
			प्रत्यागम -1;

		यदि (args->options_regex &&
		    regexec(args->options_regex, options, 0, रिक्त, 0) != 0)
			प्रत्यागम -1;
		यदि (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, रिक्त, 0) == 0)
			प्रत्यागम -1;
	}

	यदि (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    यदि (म_भेद(s->मूल्य, to) == 0)
			    भङ्ग;
		यदि (!s)
			प्रत्यागम -1;
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
		त्रुटि_सं = EINVAL;
		भङ्ग;
	}

	यदि (p) {
		त्रुटि_सं = 0;
		rc_stringlist_add(list, p);
		प्रत्यागम 0;
	}

	प्रत्यागम -1;
}

#यदि defined(BSD) && !defined(__GNU__)

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
find_mounts(संरचना args *args, माप_प्रकार *अंक_mounts)
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

	यदि ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", म_त्रुटि (त्रुटि_सं));

	list = rc_stringlist_new();
	प्रति (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		प्रति (o = optnames; flags && o->o_opt; o++) {
			यदि (flags & o->o_opt) {
				यदि (o->o_opt == MNT_LOCAL)
					netdev = 1;
				यदि (!options)
					options = xstrdup(o->o_name);
				अन्यथा {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					मुक्त(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*अंक_mounts += 0 == args->process(list, args,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		मुक्त(options);
		options = रिक्त;
	}

	प्रत्यागम list;
}

#नोचेद्यदि defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
स्थिर संरचना mntent *
getmntfile(ध्रुव वर्ण *file)
{
	संरचना mntent *ent = रिक्त;
	खाता *fp;

	यदि (!(fp = setmntent("/etc/fstab", "r")))
		प्रत्यागम रिक्त;
	यावत् ((ent = getmntent(fp)))
		यदि (म_भेद(file, ent->mnt_dir) == 0)
			भङ्ग;
	endmntent(fp);

	प्रत्यागम ent;
}

स्थिर RC_STRINGLIST *
find_mounts(संरचना args *args, माप_प्रकार *अंक_mounts)
{
	खाता *fp;
	वर्ण *buffer;
	माप_प्रकार size;
	वर्ण *p;
	वर्ण *from;
	वर्ण *to;
	वर्ण *fst;
	वर्ण *opts;
	संरचना mntent *ent;
	पूर्णाङ्क netdev;
	RC_STRINGLIST *list;

	यदि ((fp = ख_खोलो(procmounts, "r")) == रिक्त)
		eerrorx("getmntinfo: %s", म_त्रुटि(त्रुटि_सं));

	list = rc_stringlist_new();

	buffer = रिक्त;
	यावत् (xgetline(&buffer, &size, fp) != -1) {
		netdev = -1;
		p = buffer;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		यदि ((ent = getmntfile(to))) {
			यदि (म_माला(ent->mnt_opts, "_netdev"))
				netdev = 0;
			अन्यथा
				netdev = 1;
		}

		*अंक_mounts += 0 == args->process(list, args,
			from, to, fst, opts, netdev);

		मुक्त(buffer);
		buffer = रिक्त;
	}
	मुक्त(buffer);
	ख_बंद(fp);

	प्रत्यागम list;
}

#अन्यथा
#  error "Operating system not supported!"
#endif

स्थिर पूर्णाङ्क is_prefix(ध्रुव वर्ण *needle, ध्रुव वर्ण *hay)
{
	माप_प्रकार nlen = म_माप(needle);
	यदि (म_भेदन(needle, hay, nlen) == 0 && hay[nlen] == '/')
		प्रत्यागम true;
	प्रत्यागम false;
}

स्थिर वर्ण *unescape_octal(वर्ण *beg)
{
	पूर्णाङ्क n, i;
	वर्ण *w = beg, *r = beg;
	यावत् (*r) {
		यदि (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} अन्यथा {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			प्रति (i = n = 0; i < 3; ++i) {
				यदि (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} अन्यथा {
					भङ्ग;
				}
			}
			यदि (n)
				*w++ = n;
		}
	}
	*w = '\0';
	प्रत्यागम beg;
}

स्थिर पूर्णाङ्क check_is_mounted(RC_STRINGLIST *list RC_UNUSED, संरचना args *args,
    वर्ण *from RC_UNUSED, वर्ण *to, वर्ण *fstype RC_UNUSED,
    वर्ण *options RC_UNUSED, पूर्णाङ्क netdev RC_UNUSED)
{
	प्रत्यागम म_भेद(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

स्थिर पूर्णाङ्क is_mounted(ध्रुव वर्ण *mntpath)
{
	माप_प्रकार अंक_mounts = 0;
	संरचना args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &अंक_mounts);
	rc_stringlist_free(l);
	प्रत्यागम अंक_mounts > 0;
}

स्थिर pid_t run_umount(ध्रुव वर्ण *mntpath,
	ध्रुव वर्ण **umount_args, पूर्णाङ्क umount_args_अंक)
{
	संरचना exec_args args;
	संरचना exec_result res;
	ध्रुव वर्ण *argv[UMOUNT_ARGS_MAX + 3];
	पूर्णाङ्क k, i = 0;

	argv[i++] = "umount";
	प्रति (k = 0; k < umount_args_अंक; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = रिक्त;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	यदि (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, म_त्रुटि(त्रुटि_सं));
	प्रत्यागम res.pid;
}

स्थिर शून्य fuser_run(संरचना run_queue *rp, ध्रुव वर्ण *fuser_opt)
{
	स्थिर पूर्णाङ्क fuser_exec_failed = 0;
	ध्रुव वर्ण *argv[] = { "fuser", fuser_opt, rp->mntpath, रिक्त };
	संरचना exec_result res;
	संरचना exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	यदि (fuser_exec_failed)
		प्रत्यागम;

	args = exec_init(argv);
	args.redirect_stdout = EXEC_MKPIPE;
	args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	यदि (res.pid < 0) {
		fuser_exec_failed = 1;
	} अन्यथा {
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

	यदि (rp->fuser_stdoutfd < 0)
		प्रत्यागम 0;

	buf[0] = ' ';
	n = पठ(rp->fuser_stdoutfd, buf + 1, परिमाण buf - 3);
	बंद(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == परिमाण buf - 3);
	यदि (n < 0 || read_maybe_truncated)
		प्रत्यागम 0;
	यावत् (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, परिमाण selfpid, " %lld ", (दीर्घ दीर्घ)getpid());

	यदि (!has_rc_pid) {
		rc_pid = दो_पर्या("RC_PID");
		has_rc_pid = true;
	}

	यदि (म_माला(buf, selfpid) || (rc_pid && म_माला(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		प्रत्यागम -1;
	} अन्यथा यदि (म_भेद(buf, "  ") == 0) {
		यदि (rp->try_गिनती >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			प्रत्यागम -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_गिनती = TRY_MAX;
		प्रत्यागम 0;
	} अन्यथा {
		वर्ण sig[32];
		ध्रुव वर्ण *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, रिक्त
		};
		संरचना exec_result res;
		संरचना exec_args args = exec_init(argv);
		args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, परिमाण sig, "%s%s", fuser_kill_prefix,
			rp->try_गिनती == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&args);
		यदि (res.pid > 0)
			rc_waitpid(res.pid);
		प्रत्यागम 0;
	}
}

स्थिर regex_t *
get_regex(ध्रुव वर्ण *string)
{
	regex_t *reg = xmalloc(परिमाण (*reg));
	पूर्णाङ्क result;
	वर्ण buffer[256];

	यदि ((result = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(result, reg, buffer, परिमाण(buffer));
		eerrorx("%s: invalid regex `%s'", applet, buffer);
	}

	प्रत्यागम reg;
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
	माप_प्रकार अंक_mounts = 0;
	माप_प्रकार unmount_index;
	पूर्णाङ्क doing_unmount = 0;
	pid_t pid;
	पूर्णाङ्क status, flags;
	int64_t tmp, next_retry, now;
	int64_t rc_fuser_timeout = -1;
	ध्रुव वर्ण *umount_args[UMOUNT_ARGS_MAX];
	पूर्णाङ्क umount_args_अंक = 0;
	ध्रुव वर्ण **mounts = रिक्त;
	संरचना run_queue running[RUN_MAX] = {0};
	संरचना run_queue *rp;
	माप_प्रकार अंक_running = 0, अंक_waiting = 0;
	गणना { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	यदि (_var) मुक्त(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	यदि (_var) { regfree(_var); मुक्त(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	स_रखो (&args, 0, परिमाण(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	यदि (म_भेद(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		यावत् (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			यदि (म_भेद(tmps, "--") == 0)
				भङ्ग;
			यदि (umount_args_अंक >= (पूर्णाङ्क)ARRAY_SIZE(umount_args))
				eerrorx("%s: Too many umount arguments", applet);
			umount_args[umount_args_अंक++] = tmps;
		}

		tmps = rc_conf_मूल्य("rc_fuser_timeout");
		यदि (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		यदि (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = दो_पर्या("RC_UNAME");
		यदि (!tmps || म_भेद(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} अन्यथा {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	यावत् ((opt = getopt_long(argc, argv, getoptstring,
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
			DO_REG(args.गाँठ_regex);
			भङ्ग;
		पक्ष 'N':
			DO_REG(args.skip_गाँठ_regex);
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

	यावत् (optind < argc) {
		यदि (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, रिक्त);
		यदि (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		मुक्त(real_path);
		real_path = रिक्त;
	}
	nodes = find_mounts(&args, &अंक_mounts);
	rc_stringlist_free(args.mounts);

	यदि (doing_unmount)
		mounts = xmalloc(अंक_mounts * परिमाण(*mounts));
	अंक_mounts = 0;
	result = निकास_त्रुटि;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		यदि (point_regex &&
		    regexec(point_regex, s->मूल्य, 0, रिक्त, 0) != 0)
			अनुवर्त;
		यदि (skip_point_regex &&
		    regexec(skip_point_regex, s->मूल्य, 0, रिक्त, 0) == 0)
			अनुवर्त;
		यदि (doing_unmount)
			mounts[अंक_mounts++] = unescape_octal(s->मूल्य);
		अन्यथा यदि (!rc_yesno(दो_पर्या("EINFO_QUIET")))
			म_लिखो("%s\n", s->मूल्य);
		result = निकास_सफल;
	}
	यदि (!doing_unmount)
		गच्छ निकास;

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
	result = निकास_सफल;
	state = STATE_RUN;
	यावत् (state != STATE_END) विकल्प (state) {
	पक्ष STATE_RUN:
		प्रति (unmount_index = 0; unmount_index < अंक_mounts; ++unmount_index) {
			ध्रुव वर्ण *candidate = mounts[unmount_index];
			पूर्णाङ्क safe_to_unmount = 1;
			प्रति (माप_प्रकार k = 0; safe_to_unmount && k < अंक_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			प्रति (माप_प्रकार k = 0; safe_to_unmount && k < अंक_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			यदि (!safe_to_unmount)
				अनुवर्त;
			यदि (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_index--] = mounts[--अंक_mounts];
			} अन्यथा {
				भङ्ग;
			}
		}
		यदि (अंक_running == RUN_MAX || unmount_index >= अंक_mounts) {
			state = STATE_REAP;
			भङ्ग;
		}
		rp = running + अंक_running++;
		rp->mntpath = mounts[unmount_index];
		rp->last_exec_time = tm_now();
		rp->try_गिनती = 0;
		rp->pid = run_umount(rp->mntpath, umount_args, umount_args_अंक);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_index] = mounts[--अंक_mounts];
		भङ्ग;
	पक्ष STATE_REAP:
		flags = (अंक_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = रिक्त;
		प्रति (माप_प्रकार i = 0; i < अंक_running; ++i) {
			rp = running + i;
			यदि (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			यदि (rp->pid == pid && pid > 0)
				भङ्ग;
			rp = रिक्त;
		}
		यदि (rp) {
			यदि ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--अंक_running];
				state = STATE_RUN;
			} अन्यथा यदि (rp->try_गिनती >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--अंक_running];
				result = निकास_त्रुटि;
			} अन्यथा { /* put into waiting queue */
				rp->pid = -1;
				rp->try_गिनती += 1;
				अंक_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} अन्यथा {
			state = STATE_RETRY;
		}
		भङ्ग;
	पक्ष STATE_RETRY:
		rp = रिक्त;
		next_retry = INT64_MAX;
		प्रति (माप_प्रकार i = 0; i < अंक_running; ++i) {
			यदि (running[i].pid > 0)
				अनुवर्त;
			यदि (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			अन्यथा
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			यदि (tmp < next_retry) {
				rp = running + i;
				next_retry = tmp;
			}
		}
		यदि (!rp) {
			state = (अंक_mounts > 0) ? STATE_RUN : STATE_END;
			भङ्ग;
		}
		now = tm_now();
		यदि ((rp->pid > 0 || rp->fuser_pid > 0) && next_retry > now) {
			int64_t sleep_for = next_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			यदि (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		यदि ((rp->pid == -1 && rp->fuser_pid == -1) || next_retry <= now) {
			यदि (rp->fuser_pid > 0) {
				समाप्त(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, रिक्त, 0);
				rp->fuser_pid = -1;
			}
			यदि (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--अंक_running];
				result = निकास_त्रुटि;
			} अन्यथा { /* retry */
				rp->last_exec_time = tm_now();
				rp->pid = run_umount(rp->mntpath,
					umount_args, umount_args_अंक);
			}
			अंक_waiting -= 1;
			state = STATE_REAP;
		}
		भङ्ग;
	स्वभाव: भङ्ग;
	}

निकास:
	मुक्त(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(args.fstype_regex);
	REG_FREE(args.skip_fstype_regex);
	REG_FREE(args.गाँठ_regex);
	REG_FREE(args.skip_गाँठ_regex);
	REG_FREE(args.options_regex);
	REG_FREE(args.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	प्रत्यागम result;
}
