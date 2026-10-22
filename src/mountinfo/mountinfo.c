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

#अगर defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#या_अगर defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#या_अगर defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#या_अगर defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#पूर्ण_अगर

#समावेश <errno.h>
#समावेश <getopt.h>
#समावेश <regex.h>
#समावेश <stddef.h>
#समावेश <stdio.h>
#समावेश <stdint.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <unistd.h>

#समावेश "einfo.h"
#समावेश "queue.h"
#समावेश "rc.h"
#समावेश "rc_exec.h"
#समावेश "timeutils.h"
#समावेश "_usage.h"
#समावेश "helpers.h"

स्थिर अक्षर *applet = शून्य;
स्थिर अक्षर *procmounts = "/proc/mounts";
स्थिर अक्षर *extraopts = "[mount1] [mount2] ...";
स्थिर अक्षर getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "fstype-regex",        1, शून्य, 'f'},
	{ "skip-fstype-regex",   1, शून्य, 'F'},
	{ "node-regex",          1, शून्य, 'n'},
	{ "skip-node-regex",     1, शून्य, 'N'},
	{ "options-regex",       1, शून्य, 'o'},
	{ "skip-options-regex",  1, शून्य, 'O'},
	{ "point-regex",         1, शून्य, 'p'},
	{ "skip-point-regex",    1, शून्य, 'P'},
	{ "options",             0, शून्य, 'i'},
	{ "fstype",              0, शून्य, 's'},
	{ "node",                0, शून्य, 't'},
	{ "netdev",              0, शून्य, 'e'},
	{ "nonetdev",            0, शून्य, 'E'},
	longopts_COMMON
};
स्थिर अक्षर * स्थिर longopts_help[] = {
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
स्थिर अक्षर *usagestring = शून्य;

#घोषणा UMOUNT_ARGS_MAX 16
#घोषणा RUN_MAX 32
#घोषणा TRY_MAX 3
#घोषणा TRY_DELAY_MS 1000

काष्ठा run_queue {
	स्थिर अक्षर *mntpath;
	int64_t last_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	पूर्णांक try_गिनती;
	पूर्णांक fuser_stdoutfd;
};

प्रकार क्रमागत {
	mount_from,
	mount_to,
	mount_fstype,
	mount_options
} mount_type;

प्रकार क्रमागत {
	net_ignore,
	net_yes,
	net_no
} net_opts;

काष्ठा args;
प्रकार पूर्णांक process_func_t(RC_STRINGLIST *, काष्ठा args *,
    अक्षर *, अक्षर *, अक्षर *, अक्षर *, पूर्णांक);

काष्ठा args {
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
	स्थिर अक्षर *check_mntpath;
};

अटल पूर्णांक
process_mount(RC_STRINGLIST *list, काष्ठा args *args,
    अक्षर *from, अक्षर *to, अक्षर *fstype, अक्षर *options,
    पूर्णांक netdev)
{
	अक्षर *p;
	RC_STRING *s;

	त्रुटि_सं = ENOENT;

#अगर_घोषित __linux__
	/* Skip the really silly rootfs */
	अगर (म_भेद(fstype, "rootfs") == 0)
		वापस -1;
#पूर्ण_अगर

	अगर (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		अगर (netdev != 0)
			वापस 1;
	} अन्यथा अगर (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		अगर (netdev != 1)
			वापस 1;
	} अन्यथा {
		अगर (args->गाँठ_regex &&
		    regexec(args->गाँठ_regex, from, 0, शून्य, 0) != 0)
			वापस 1;
		अगर (args->skip_गाँठ_regex &&
		    regexec(args->skip_गाँठ_regex, from, 0, शून्य, 0) == 0)
			वापस 1;

		अगर (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, शून्य, 0) != 0)
			वापस -1;
		अगर (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, शून्य, 0) == 0)
			वापस -1;

		अगर (args->options_regex &&
		    regexec(args->options_regex, options, 0, शून्य, 0) != 0)
			वापस -1;
		अगर (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, शून्य, 0) == 0)
			वापस -1;
	}

	अगर (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    अगर (म_भेद(s->मूल्य, to) == 0)
			    अवरोध;
		अगर (!s)
			वापस -1;
	}

	चयन (args->mount_type) {
	हाल mount_from:
		p = from;
		अवरोध;
	हाल mount_to:
		p = to;
		अवरोध;
	हाल mount_fstype:
		p = fstype;
		अवरोध;
	हाल mount_options:
		p = options;
		अवरोध;
	शेष:
		p = शून्य;
		त्रुटि_सं = EINVAL;
		अवरोध;
	}

	अगर (p) {
		त्रुटि_सं = 0;
		rc_stringlist_add(list, p);
		वापस 0;
	}

	वापस -1;
}

#अगर defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
अटल काष्ठा opt {
	पूर्णांक o_opt;
	स्थिर अक्षर *o_name;
} optnames[] = {
	{ MNT_ASYNC,        "asynchronous" },
	{ MNT_EXPORTED,     "NFS exported" },
	{ MNT_LOCAL,        "local" },
	{ MNT_NOATIME,      "noatime" },
	{ MNT_NOEXEC,       "noexec" },
	{ MNT_NOSUID,       "nosuid" },
#अगर_घोषित MNT_NOSYMFOLLOW
	{ MNT_NOSYMFOLLOW,  "nosymfollow" },
#पूर्ण_अगर
	{ MNT_QUOTA,        "with quotas" },
	{ MNT_RDONLY,       "read-only" },
	{ MNT_SYNCHRONOUS,  "synchronous" },
	{ MNT_UNION,        "union" },
#अगर_घोषित MNT_NOCLUSTERR
	{ MNT_NOCLUSTERR,   "noclusterr" },
#पूर्ण_अगर
#अगर_घोषित MNT_NOCLUSTERW
	{ MNT_NOCLUSTERW,   "noclusterw" },
#पूर्ण_अगर
#अगर_घोषित MNT_SUIDDIR
	{ MNT_SUIDDIR,      "suiddir" },
#पूर्ण_अगर
	{ MNT_SOFTDEP,      "soft-updates" },
#अगर_घोषित MNT_MULTILABEL
	{ MNT_MULTILABEL,   "multilabel" },
#पूर्ण_अगर
#अगर_घोषित MNT_ACLS
	{ MNT_ACLS,         "acls" },
#पूर्ण_अगर
#अगर_घोषित MNT_GJOURNAL
	{ MNT_GJOURNAL,     "gjournal" },
#पूर्ण_अगर
	{ 0, शून्य }
};

अटल RC_STRINGLIST *
find_mounts(काष्ठा args *args, माप_प्रकार *अंक_mounts)
{
	काष्ठा statfs *mnts;
	पूर्णांक nmnts;
	पूर्णांक i;
	RC_STRINGLIST *list;
	अक्षर *options = शून्य;
	uint64_t flags;
	काष्ठा opt *o;
	पूर्णांक netdev;
	अक्षर *tmp;

	अगर ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", म_त्रुटि (त्रुटि_सं));

	list = rc_stringlist_new();
	क्रम (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		क्रम (o = optnames; flags && o->o_opt; o++) {
			अगर (flags & o->o_opt) {
				अगर (o->o_opt == MNT_LOCAL)
					netdev = 1;
				अगर (!options)
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
		options = शून्य;
	}

	वापस list;
}

#या_अगर defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
अटल काष्ठा mntent *
getmntfile(स्थिर अक्षर *file)
{
	काष्ठा mntent *ent = शून्य;
	खाता *fp;

	अगर (!(fp = setmntent("/etc/fstab", "r")))
		वापस शून्य;
	जबतक ((ent = getmntent(fp)))
		अगर (म_भेद(file, ent->mnt_dir) == 0)
			अवरोध;
	endmntent(fp);

	वापस ent;
}

अटल RC_STRINGLIST *
find_mounts(काष्ठा args *args, माप_प्रकार *अंक_mounts)
{
	खाता *fp;
	अक्षर *बफर;
	माप_प्रकार आकार;
	अक्षर *p;
	अक्षर *from;
	अक्षर *to;
	अक्षर *fst;
	अक्षर *opts;
	काष्ठा mntent *ent;
	पूर्णांक netdev;
	RC_STRINGLIST *list;

	अगर ((fp = ख_खोलो(procmounts, "r")) == शून्य)
		eerrorx("getmntinfo: %s", म_त्रुटि(त्रुटि_सं));

	list = rc_stringlist_new();

	बफर = शून्य;
	जबतक (xgetline(&बफर, &आकार, fp) != -1) {
		netdev = -1;
		p = बफर;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		अगर ((ent = getmntfile(to))) {
			अगर (म_माला(ent->mnt_opts, "_netdev"))
				netdev = 0;
			अन्यथा
				netdev = 1;
		}

		*अंक_mounts += 0 == args->process(list, args,
			from, to, fst, opts, netdev);

		मुक्त(बफर);
		बफर = शून्य;
	}
	मुक्त(बफर);
	ख_बंद(fp);

	वापस list;
}

#अन्यथा
#  error "Operating system not supported!"
#पूर्ण_अगर

अटल पूर्णांक is_prefix(स्थिर अक्षर *needle, स्थिर अक्षर *hay)
{
	माप_प्रकार nlen = म_माप(needle);
	अगर (म_भेदन(needle, hay, nlen) == 0 && hay[nlen] == '/')
		वापस true;
	वापस false;
}

अटल अक्षर *unescape_octal(अक्षर *beg)
{
	पूर्णांक n, i;
	अक्षर *w = beg, *r = beg;
	जबतक (*r) {
		अगर (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} अन्यथा {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			क्रम (i = n = 0; i < 3; ++i) {
				अगर (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} अन्यथा {
					अवरोध;
				}
			}
			अगर (n)
				*w++ = n;
		}
	}
	*w = '\0';
	वापस beg;
}

अटल पूर्णांक check_is_mounted(RC_STRINGLIST *list RC_UNUSED, काष्ठा args *args,
    अक्षर *from RC_UNUSED, अक्षर *to, अक्षर *fstype RC_UNUSED,
    अक्षर *options RC_UNUSED, पूर्णांक netdev RC_UNUSED)
{
	वापस म_भेद(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

अटल पूर्णांक is_mounted(स्थिर अक्षर *mntpath)
{
	माप_प्रकार अंक_mounts = 0;
	काष्ठा args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &अंक_mounts);
	rc_stringlist_free(l);
	वापस अंक_mounts > 0;
}

अटल pid_t run_umount(स्थिर अक्षर *mntpath,
	स्थिर अक्षर **umount_args, पूर्णांक umount_args_अंक)
{
	काष्ठा exec_args args;
	काष्ठा exec_परिणाम res;
	स्थिर अक्षर *argv[UMOUNT_ARGS_MAX + 3];
	पूर्णांक k, i = 0;

	argv[i++] = "umount";
	क्रम (k = 0; k < umount_args_अंक; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = शून्य;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	अगर (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, म_त्रुटि(त्रुटि_सं));
	वापस res.pid;
}

अटल व्योम fuser_run(काष्ठा run_queue *rp, स्थिर अक्षर *fuser_opt)
{
	अटल पूर्णांक fuser_exec_failed = 0;
	स्थिर अक्षर *argv[] = { "fuser", fuser_opt, rp->mntpath, शून्य };
	काष्ठा exec_परिणाम res;
	काष्ठा exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	अगर (fuser_exec_failed)
		वापस;

	args = exec_init(argv);
	args.redirect_stdout = EXEC_MKPIPE;
	args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	अगर (res.pid < 0) {
		fuser_exec_failed = 1;
	} अन्यथा {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

अटल पूर्णांक fuser_decide(काष्ठा run_queue *rp,
	स्थिर अक्षर *fuser_opt, स्थिर अक्षर *fuser_kill_prefix)
{
	अटल bool has_rc_pid = false;
	अटल अक्षर *rc_pid;
	अक्षर buf[1<<12];
	अक्षर selfpid[64];
	पूर्णांक read_maybe_truncated;
	ssize_t n;

	अगर (rp->fuser_stdoutfd < 0)
		वापस 0;

	buf[0] = ' ';
	n = पढ़ो(rp->fuser_stdoutfd, buf + 1, माप buf - 3);
	बंद(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == माप buf - 3);
	अगर (n < 0 || read_maybe_truncated)
		वापस 0;
	जबतक (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, माप selfpid, " %lld ", (दीर्घ दीर्घ)getpid());

	अगर (!has_rc_pid) {
		rc_pid = दो_पर्या("RC_PID");
		has_rc_pid = true;
	}

	अगर (म_माला(buf, selfpid) || (rc_pid && म_माला(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		वापस -1;
	} अन्यथा अगर (म_भेद(buf, "  ") == 0) {
		अगर (rp->try_गिनती >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			वापस -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_गिनती = TRY_MAX;
		वापस 0;
	} अन्यथा {
		अक्षर sig[32];
		स्थिर अक्षर *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, शून्य
		};
		काष्ठा exec_परिणाम res;
		काष्ठा exec_args args = exec_init(argv);
		args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, माप sig, "%s%s", fuser_kill_prefix,
			rp->try_गिनती == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&args);
		अगर (res.pid > 0)
			rc_waitpid(res.pid);
		वापस 0;
	}
}

अटल regex_t *
get_regex(स्थिर अक्षर *string)
{
	regex_t *reg = xmalloc(माप (*reg));
	पूर्णांक परिणाम;
	अक्षर बफर[256];

	अगर ((परिणाम = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(परिणाम, reg, बफर, माप(बफर));
		eerrorx("%s: invalid regex `%s'", applet, बफर);
	}

	वापस reg;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	काष्ठा args args;
	regex_t *point_regex = शून्य;
	regex_t *skip_point_regex = शून्य;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	अक्षर *real_path = शून्य;
	पूर्णांक opt;
	पूर्णांक परिणाम;
	अक्षर *this_path, *argv0;
	स्थिर अक्षर *tmps;
	स्थिर अक्षर *fuser_opt, *fuser_kill_prefix;
	माप_प्रकार अंक_mounts = 0;
	माप_प्रकार unmount_सूचकांक;
	पूर्णांक doing_unmount = 0;
	pid_t pid;
	पूर्णांक status, flags;
	int64_t tmp, अग्र_retry, now;
	int64_t rc_fuser_timeout = -1;
	स्थिर अक्षर *umount_args[UMOUNT_ARGS_MAX];
	पूर्णांक umount_args_अंक = 0;
	स्थिर अक्षर **mounts = शून्य;
	काष्ठा run_queue running[RUN_MAX] = {0};
	काष्ठा run_queue *rp;
	माप_प्रकार अंक_running = 0, अंक_waiting = 0;
	क्रमागत { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#घोषणा DO_REG(_var)							      \
	अगर (_var) मुक्त(_var);						      \
	_var = get_regex(optarg);
#घोषणा REG_FREE(_var)							      \
	अगर (_var) { regfree(_var); मुक्त(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	स_रखो (&args, 0, माप(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	अगर (म_भेद(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		जबतक (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			अगर (म_भेद(tmps, "--") == 0)
				अवरोध;
			अगर (umount_args_अंक >= (पूर्णांक)ARRAY_आकार(umount_args))
				eerrorx("%s: Too many umount arguments", applet);
			umount_args[umount_args_अंक++] = tmps;
		}

		tmps = rc_conf_मूल्य("rc_fuser_timeout");
		अगर (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		अगर (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = दो_पर्या("RC_UNAME");
		अगर (!tmps || म_भेद(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} अन्यथा {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	जबतक ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		चयन (opt) {
		हाल 'e':
			args.netdev = net_yes;
			अवरोध;
		हाल 'E':
			args.netdev = net_no;
			अवरोध;
		हाल 'f':
			DO_REG(args.fstype_regex);
			अवरोध;
		हाल 'F':
			DO_REG(args.skip_fstype_regex);
			अवरोध;
		हाल 'n':
			DO_REG(args.गाँठ_regex);
			अवरोध;
		हाल 'N':
			DO_REG(args.skip_गाँठ_regex);
			अवरोध;
		हाल 'o':
			DO_REG(args.options_regex);
			अवरोध;
		हाल 'O':
			DO_REG(args.skip_options_regex);
			अवरोध;
		हाल 'p':
			DO_REG(point_regex);
			अवरोध;
		हाल 'P':
			DO_REG(skip_point_regex);
			अवरोध;
		हाल 'i':
			args.mount_type = mount_options;
			अवरोध;
		हाल 's':
			args.mount_type = mount_fstype;
			अवरोध;
		हाल 't':
			args.mount_type = mount_from;
			अवरोध;

		case_RC_COMMON_GETOPT
		}
	}

	जबतक (optind < argc) {
		अगर (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, शून्य);
		अगर (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		मुक्त(real_path);
		real_path = शून्य;
	}
	nodes = find_mounts(&args, &अंक_mounts);
	rc_stringlist_free(args.mounts);

	अगर (doing_unmount)
		mounts = xmalloc(अंक_mounts * माप(*mounts));
	अंक_mounts = 0;
	परिणाम = निकास_त्रुटि;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		अगर (point_regex &&
		    regexec(point_regex, s->मूल्य, 0, शून्य, 0) != 0)
			जारी;
		अगर (skip_point_regex &&
		    regexec(skip_point_regex, s->मूल्य, 0, शून्य, 0) == 0)
			जारी;
		अगर (doing_unmount)
			mounts[अंक_mounts++] = unescape_octal(s->मूल्य);
		अन्यथा अगर (!rc_yesno(दो_पर्या("EINFO_QUIET")))
			म_लिखो("%s\n", s->मूल्य);
		परिणाम = निकास_सफल;
	}
	अगर (!doing_unmount)
		जाओ निकास;

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
	परिणाम = निकास_सफल;
	state = STATE_RUN;
	जबतक (state != STATE_END) चयन (state) {
	हाल STATE_RUN:
		क्रम (unmount_सूचकांक = 0; unmount_सूचकांक < अंक_mounts; ++unmount_सूचकांक) {
			स्थिर अक्षर *candidate = mounts[unmount_सूचकांक];
			पूर्णांक safe_to_unmount = 1;
			क्रम (माप_प्रकार k = 0; safe_to_unmount && k < अंक_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			क्रम (माप_प्रकार k = 0; safe_to_unmount && k < अंक_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			अगर (!safe_to_unmount)
				जारी;
			अगर (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_सूचकांक--] = mounts[--अंक_mounts];
			} अन्यथा {
				अवरोध;
			}
		}
		अगर (अंक_running == RUN_MAX || unmount_सूचकांक >= अंक_mounts) {
			state = STATE_REAP;
			अवरोध;
		}
		rp = running + अंक_running++;
		rp->mntpath = mounts[unmount_सूचकांक];
		rp->last_exec_time = tm_now();
		rp->try_गिनती = 0;
		rp->pid = run_umount(rp->mntpath, umount_args, umount_args_अंक);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_सूचकांक] = mounts[--अंक_mounts];
		अवरोध;
	हाल STATE_REAP:
		flags = (अंक_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = शून्य;
		क्रम (माप_प्रकार i = 0; i < अंक_running; ++i) {
			rp = running + i;
			अगर (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			अगर (rp->pid == pid && pid > 0)
				अवरोध;
			rp = शून्य;
		}
		अगर (rp) {
			अगर ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--अंक_running];
				state = STATE_RUN;
			} अन्यथा अगर (rp->try_गिनती >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--अंक_running];
				परिणाम = निकास_त्रुटि;
			} अन्यथा { /* put into waiting queue */
				rp->pid = -1;
				rp->try_गिनती += 1;
				अंक_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} अन्यथा {
			state = STATE_RETRY;
		}
		अवरोध;
	हाल STATE_RETRY:
		rp = शून्य;
		अग्र_retry = INT64_MAX;
		क्रम (माप_प्रकार i = 0; i < अंक_running; ++i) {
			अगर (running[i].pid > 0)
				जारी;
			अगर (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			अन्यथा
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			अगर (tmp < अग्र_retry) {
				rp = running + i;
				अग्र_retry = tmp;
			}
		}
		अगर (!rp) {
			state = (अंक_mounts > 0) ? STATE_RUN : STATE_END;
			अवरोध;
		}
		now = tm_now();
		अगर ((rp->pid > 0 || rp->fuser_pid > 0) && अग्र_retry > now) {
			int64_t sleep_for = अग्र_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			अगर (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		अगर ((rp->pid == -1 && rp->fuser_pid == -1) || अग्र_retry <= now) {
			अगर (rp->fuser_pid > 0) {
				समाप्त(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, शून्य, 0);
				rp->fuser_pid = -1;
			}
			अगर (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--अंक_running];
				परिणाम = निकास_त्रुटि;
			} अन्यथा { /* retry */
				rp->last_exec_time = tm_now();
				rp->pid = run_umount(rp->mntpath,
					umount_args, umount_args_अंक);
			}
			अंक_waiting -= 1;
			state = STATE_REAP;
		}
		अवरोध;
	शेष: अवरोध;
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

	वापस परिणाम;
}
