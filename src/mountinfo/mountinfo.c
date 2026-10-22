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

#जर defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#किंवाजर defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#किंवाजर defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#किंवाजर defined(__linux__) || (defined(__FreeBSD_kernel__) && \
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

स्थिरांक अक्षर *applet = रिक्त;
स्थिरांक अक्षर *procmounts = "/proc/mounts";
स्थिरांक अक्षर *extraopts = "[mount1] [mount2] ...";
स्थिरांक अक्षर getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
स्थिरांक संरचना option longopts[] = {
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
स्थिरांक अक्षर * स्थिरांक longopts_help[] = {
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
स्थिरांक अक्षर *usagestring = रिक्त;

#define UMOUNT_ARGS_MAX 16
#define RUN_MAX 32
#define TRY_MAX 3
#define TRY_DELAY_MS 1000

संरचना run_queue {
	स्थिरांक अक्षर *mntpath;
	int64_t last_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	पूर्णांक try_मोजणी;
	पूर्णांक fuser_stdoutfd;
};

प्रकारव्याख्या गणना {
	mount_from,
	mount_to,
	mount_fstype,
	mount_options
} mount_type;

प्रकारव्याख्या गणना {
	net_ignore,
	net_yes,
	net_no
} net_opts;

संरचना args;
प्रकारव्याख्या पूर्णांक process_func_t(RC_STRINGLIST *, संरचना args *,
    अक्षर *, अक्षर *, अक्षर *, अक्षर *, पूर्णांक);

संरचना args {
	process_func_t *process;
	regex_t *गाठ_regex;
	regex_t *skip_गाठ_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	स्थिरांक अक्षर *check_mntpath;
};

स्थिर पूर्णांक
process_mount(RC_STRINGLIST *list, संरचना args *args,
    अक्षर *from, अक्षर *to, अक्षर *fstype, अक्षर *options,
    पूर्णांक netdev)
{
	अक्षर *p;
	RC_STRING *s;

	त्रुटि_सं = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	जर (म_भेद(fstype, "rootfs") == 0)
		परत -1;
#endif

	जर (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		जर (netdev != 0)
			परत 1;
	} अन्यथा जर (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		जर (netdev != 1)
			परत 1;
	} अन्यथा {
		जर (args->गाठ_regex &&
		    regexec(args->गाठ_regex, from, 0, रिक्त, 0) != 0)
			परत 1;
		जर (args->skip_गाठ_regex &&
		    regexec(args->skip_गाठ_regex, from, 0, रिक्त, 0) == 0)
			परत 1;

		जर (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, रिक्त, 0) != 0)
			परत -1;
		जर (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, रिक्त, 0) == 0)
			परत -1;

		जर (args->options_regex &&
		    regexec(args->options_regex, options, 0, रिक्त, 0) != 0)
			परत -1;
		जर (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, रिक्त, 0) == 0)
			परत -1;
	}

	जर (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    जर (म_भेद(s->मूल्य, to) == 0)
			    थांबा;
		जर (!s)
			परत -1;
	}

	निवड (args->mount_type) {
	स्थिती mount_from:
		p = from;
		थांबा;
	स्थिती mount_to:
		p = to;
		थांबा;
	स्थिती mount_fstype:
		p = fstype;
		थांबा;
	स्थिती mount_options:
		p = options;
		थांबा;
	पूर्वनिर्धारित:
		p = रिक्त;
		त्रुटि_सं = EINVAL;
		थांबा;
	}

	जर (p) {
		त्रुटि_सं = 0;
		rc_stringlist_add(list, p);
		परत 0;
	}

	परत -1;
}

#जर defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
स्थिर संरचना opt {
	पूर्णांक o_opt;
	स्थिरांक अक्षर *o_name;
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
	पूर्णांक nmnts;
	पूर्णांक i;
	RC_STRINGLIST *list;
	अक्षर *options = रिक्त;
	uint64_t flags;
	संरचना opt *o;
	पूर्णांक netdev;
	अक्षर *tmp;

	जर ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", म_त्रुटि (त्रुटि_सं));

	list = rc_stringlist_new();
	साठी (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		साठी (o = optnames; flags && o->o_opt; o++) {
			जर (flags & o->o_opt) {
				जर (o->o_opt == MNT_LOCAL)
					netdev = 1;
				जर (!options)
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

	परत list;
}

#किंवाजर defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
स्थिर संरचना mntent *
getmntfile(स्थिरांक अक्षर *file)
{
	संरचना mntent *ent = रिक्त;
	खाता *fp;

	जर (!(fp = setmntent("/etc/fstab", "r")))
		परत रिक्त;
	पर्यंत ((ent = getmntent(fp)))
		जर (म_भेद(file, ent->mnt_dir) == 0)
			थांबा;
	endmntent(fp);

	परत ent;
}

स्थिर RC_STRINGLIST *
find_mounts(संरचना args *args, माप_प्रकार *अंक_mounts)
{
	खाता *fp;
	अक्षर *buffer;
	माप_प्रकार size;
	अक्षर *p;
	अक्षर *from;
	अक्षर *to;
	अक्षर *fst;
	अक्षर *opts;
	संरचना mntent *ent;
	पूर्णांक netdev;
	RC_STRINGLIST *list;

	जर ((fp = ख_खोलो(procmounts, "r")) == रिक्त)
		eerrorx("getmntinfo: %s", म_त्रुटि(त्रुटि_सं));

	list = rc_stringlist_new();

	buffer = रिक्त;
	पर्यंत (xgetline(&buffer, &size, fp) != -1) {
		netdev = -1;
		p = buffer;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		जर ((ent = getmntfile(to))) {
			जर (म_माला(ent->mnt_opts, "_netdev"))
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

	परत list;
}

#अन्यथा
#  error "Operating system not supported!"
#endif

स्थिर पूर्णांक is_prefix(स्थिरांक अक्षर *needle, स्थिरांक अक्षर *hay)
{
	माप_प्रकार nlen = म_माप(needle);
	जर (म_भेदन(needle, hay, nlen) == 0 && hay[nlen] == '/')
		परत true;
	परत false;
}

स्थिर अक्षर *unescape_octal(अक्षर *beg)
{
	पूर्णांक n, i;
	अक्षर *w = beg, *r = beg;
	पर्यंत (*r) {
		जर (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} अन्यथा {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			साठी (i = n = 0; i < 3; ++i) {
				जर (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} अन्यथा {
					थांबा;
				}
			}
			जर (n)
				*w++ = n;
		}
	}
	*w = '\0';
	परत beg;
}

स्थिर पूर्णांक check_is_mounted(RC_STRINGLIST *list RC_UNUSED, संरचना args *args,
    अक्षर *from RC_UNUSED, अक्षर *to, अक्षर *fstype RC_UNUSED,
    अक्षर *options RC_UNUSED, पूर्णांक netdev RC_UNUSED)
{
	परत म_भेद(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

स्थिर पूर्णांक is_mounted(स्थिरांक अक्षर *mntpath)
{
	माप_प्रकार अंक_mounts = 0;
	संरचना args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &अंक_mounts);
	rc_stringlist_free(l);
	परत अंक_mounts > 0;
}

स्थिर pid_t run_umount(स्थिरांक अक्षर *mntpath,
	स्थिरांक अक्षर **umount_args, पूर्णांक umount_args_अंक)
{
	संरचना exec_args args;
	संरचना exec_result res;
	स्थिरांक अक्षर *argv[UMOUNT_ARGS_MAX + 3];
	पूर्णांक k, i = 0;

	argv[i++] = "umount";
	साठी (k = 0; k < umount_args_अंक; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = रिक्त;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	जर (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, म_त्रुटि(त्रुटि_सं));
	परत res.pid;
}

स्थिर शून्य fuser_run(संरचना run_queue *rp, स्थिरांक अक्षर *fuser_opt)
{
	स्थिर पूर्णांक fuser_exec_failed = 0;
	स्थिरांक अक्षर *argv[] = { "fuser", fuser_opt, rp->mntpath, रिक्त };
	संरचना exec_result res;
	संरचना exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	जर (fuser_exec_failed)
		परत;

	args = exec_init(argv);
	args.redirect_stdout = EXEC_MKPIPE;
	args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	जर (res.pid < 0) {
		fuser_exec_failed = 1;
	} अन्यथा {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

स्थिर पूर्णांक fuser_decide(संरचना run_queue *rp,
	स्थिरांक अक्षर *fuser_opt, स्थिरांक अक्षर *fuser_kill_prefix)
{
	स्थिर bool has_rc_pid = false;
	स्थिर अक्षर *rc_pid;
	अक्षर buf[1<<12];
	अक्षर selfpid[64];
	पूर्णांक read_maybe_truncated;
	ssize_t n;

	जर (rp->fuser_stdoutfd < 0)
		परत 0;

	buf[0] = ' ';
	n = पठ(rp->fuser_stdoutfd, buf + 1, आकार buf - 3);
	बंद(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == आकार buf - 3);
	जर (n < 0 || read_maybe_truncated)
		परत 0;
	पर्यंत (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, आकार selfpid, " %lld ", (दीर्घ दीर्घ)getpid());

	जर (!has_rc_pid) {
		rc_pid = दो_पर्या("RC_PID");
		has_rc_pid = true;
	}

	जर (म_माला(buf, selfpid) || (rc_pid && म_माला(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		परत -1;
	} अन्यथा जर (म_भेद(buf, "  ") == 0) {
		जर (rp->try_मोजणी >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			परत -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_मोजणी = TRY_MAX;
		परत 0;
	} अन्यथा {
		अक्षर sig[32];
		स्थिरांक अक्षर *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, रिक्त
		};
		संरचना exec_result res;
		संरचना exec_args args = exec_init(argv);
		args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, आकार sig, "%s%s", fuser_kill_prefix,
			rp->try_मोजणी == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&args);
		जर (res.pid > 0)
			rc_waitpid(res.pid);
		परत 0;
	}
}

स्थिर regex_t *
get_regex(स्थिरांक अक्षर *string)
{
	regex_t *reg = xmalloc(आकार (*reg));
	पूर्णांक result;
	अक्षर buffer[256];

	जर ((result = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(result, reg, buffer, आकार(buffer));
		eerrorx("%s: invalid regex `%s'", applet, buffer);
	}

	परत reg;
}

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	संरचना args args;
	regex_t *point_regex = रिक्त;
	regex_t *skip_point_regex = रिक्त;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	अक्षर *real_path = रिक्त;
	पूर्णांक opt;
	पूर्णांक result;
	अक्षर *this_path, *argv0;
	स्थिरांक अक्षर *tmps;
	स्थिरांक अक्षर *fuser_opt, *fuser_kill_prefix;
	माप_प्रकार अंक_mounts = 0;
	माप_प्रकार unmount_index;
	पूर्णांक doing_unmount = 0;
	pid_t pid;
	पूर्णांक status, flags;
	int64_t tmp, next_retry, now;
	int64_t rc_fuser_timeout = -1;
	स्थिरांक अक्षर *umount_args[UMOUNT_ARGS_MAX];
	पूर्णांक umount_args_अंक = 0;
	स्थिरांक अक्षर **mounts = रिक्त;
	संरचना run_queue running[RUN_MAX] = {0};
	संरचना run_queue *rp;
	माप_प्रकार अंक_running = 0, अंक_waiting = 0;
	गणना { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	जर (_var) मुक्त(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	जर (_var) { regfree(_var); मुक्त(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	स_रखो (&args, 0, आकार(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	जर (म_भेद(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		पर्यंत (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			जर (म_भेद(tmps, "--") == 0)
				थांबा;
			जर (umount_args_अंक >= (पूर्णांक)ARRAY_SIZE(umount_args))
				eerrorx("%s: Too many umount arguments", applet);
			umount_args[umount_args_अंक++] = tmps;
		}

		tmps = rc_conf_मूल्य("rc_fuser_timeout");
		जर (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		जर (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = दो_पर्या("RC_UNAME");
		जर (!tmps || म_भेद(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} अन्यथा {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	पर्यंत ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		निवड (opt) {
		स्थिती 'e':
			args.netdev = net_yes;
			थांबा;
		स्थिती 'E':
			args.netdev = net_no;
			थांबा;
		स्थिती 'f':
			DO_REG(args.fstype_regex);
			थांबा;
		स्थिती 'F':
			DO_REG(args.skip_fstype_regex);
			थांबा;
		स्थिती 'n':
			DO_REG(args.गाठ_regex);
			थांबा;
		स्थिती 'N':
			DO_REG(args.skip_गाठ_regex);
			थांबा;
		स्थिती 'o':
			DO_REG(args.options_regex);
			थांबा;
		स्थिती 'O':
			DO_REG(args.skip_options_regex);
			थांबा;
		स्थिती 'p':
			DO_REG(point_regex);
			थांबा;
		स्थिती 'P':
			DO_REG(skip_point_regex);
			थांबा;
		स्थिती 'i':
			args.mount_type = mount_options;
			थांबा;
		स्थिती 's':
			args.mount_type = mount_fstype;
			थांबा;
		स्थिती 't':
			args.mount_type = mount_from;
			थांबा;

		case_RC_COMMON_GETOPT
		}
	}

	पर्यंत (optind < argc) {
		जर (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, रिक्त);
		जर (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		मुक्त(real_path);
		real_path = रिक्त;
	}
	nodes = find_mounts(&args, &अंक_mounts);
	rc_stringlist_free(args.mounts);

	जर (doing_unmount)
		mounts = xmalloc(अंक_mounts * आकार(*mounts));
	अंक_mounts = 0;
	result = निकास_त्रुटि;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		जर (point_regex &&
		    regexec(point_regex, s->मूल्य, 0, रिक्त, 0) != 0)
			सुरूठेवा;
		जर (skip_point_regex &&
		    regexec(skip_point_regex, s->मूल्य, 0, रिक्त, 0) == 0)
			सुरूठेवा;
		जर (doing_unmount)
			mounts[अंक_mounts++] = unescape_octal(s->मूल्य);
		अन्यथा जर (!rc_yesno(दो_पर्या("EINFO_QUIET")))
			म_लिखो("%s\n", s->मूल्य);
		result = निकास_सफल;
	}
	जर (!doing_unmount)
		जा निकास;

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
	पर्यंत (state != STATE_END) निवड (state) {
	स्थिती STATE_RUN:
		साठी (unmount_index = 0; unmount_index < अंक_mounts; ++unmount_index) {
			स्थिरांक अक्षर *candidate = mounts[unmount_index];
			पूर्णांक safe_to_unmount = 1;
			साठी (माप_प्रकार k = 0; safe_to_unmount && k < अंक_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			साठी (माप_प्रकार k = 0; safe_to_unmount && k < अंक_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			जर (!safe_to_unmount)
				सुरूठेवा;
			जर (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_index--] = mounts[--अंक_mounts];
			} अन्यथा {
				थांबा;
			}
		}
		जर (अंक_running == RUN_MAX || unmount_index >= अंक_mounts) {
			state = STATE_REAP;
			थांबा;
		}
		rp = running + अंक_running++;
		rp->mntpath = mounts[unmount_index];
		rp->last_exec_time = tm_now();
		rp->try_मोजणी = 0;
		rp->pid = run_umount(rp->mntpath, umount_args, umount_args_अंक);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_index] = mounts[--अंक_mounts];
		थांबा;
	स्थिती STATE_REAP:
		flags = (अंक_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = रिक्त;
		साठी (माप_प्रकार i = 0; i < अंक_running; ++i) {
			rp = running + i;
			जर (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			जर (rp->pid == pid && pid > 0)
				थांबा;
			rp = रिक्त;
		}
		जर (rp) {
			जर ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--अंक_running];
				state = STATE_RUN;
			} अन्यथा जर (rp->try_मोजणी >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--अंक_running];
				result = निकास_त्रुटि;
			} अन्यथा { /* put into waiting queue */
				rp->pid = -1;
				rp->try_मोजणी += 1;
				अंक_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} अन्यथा {
			state = STATE_RETRY;
		}
		थांबा;
	स्थिती STATE_RETRY:
		rp = रिक्त;
		next_retry = INT64_MAX;
		साठी (माप_प्रकार i = 0; i < अंक_running; ++i) {
			जर (running[i].pid > 0)
				सुरूठेवा;
			जर (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			अन्यथा
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			जर (tmp < next_retry) {
				rp = running + i;
				next_retry = tmp;
			}
		}
		जर (!rp) {
			state = (अंक_mounts > 0) ? STATE_RUN : STATE_END;
			थांबा;
		}
		now = tm_now();
		जर ((rp->pid > 0 || rp->fuser_pid > 0) && next_retry > now) {
			int64_t sleep_for = next_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			जर (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		जर ((rp->pid == -1 && rp->fuser_pid == -1) || next_retry <= now) {
			जर (rp->fuser_pid > 0) {
				समाप्त(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, रिक्त, 0);
				rp->fuser_pid = -1;
			}
			जर (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
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
		थांबा;
	पूर्वनिर्धारित: थांबा;
	}

निकास:
	मुक्त(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(args.fstype_regex);
	REG_FREE(args.skip_fstype_regex);
	REG_FREE(args.गाठ_regex);
	REG_FREE(args.skip_गाठ_regex);
	REG_FREE(args.options_regex);
	REG_FREE(args.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	परत result;
}
