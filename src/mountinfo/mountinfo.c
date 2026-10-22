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

#എങ്കിൽ defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#അല്ലെങ്കിലെങ്കിൽ defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#അല്ലെങ്കിലെങ്കിൽ defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#അല്ലെങ്കിലെങ്കിൽ defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#endif

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <regex.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "rc_exec.h"
#അന്തര്ഭാവ "timeutils.h"
#അന്തര്ഭാവ "_usage.h"
#അന്തര്ഭാവ "helpers.h"

സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;
സ്ഥിരം അക്ഷരം *procmounts = "/proc/mounts";
സ്ഥിരം അക്ഷരം *extraopts = "[mount1] [mount2] ...";
സ്ഥിരം അക്ഷരം getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
സ്ഥിരം ഘടന option longopts[] = {
	{ "fstype-regex",        1, ശൂന്യ, 'f'},
	{ "skip-fstype-regex",   1, ശൂന്യ, 'F'},
	{ "node-regex",          1, ശൂന്യ, 'n'},
	{ "skip-node-regex",     1, ശൂന്യ, 'N'},
	{ "options-regex",       1, ശൂന്യ, 'o'},
	{ "skip-options-regex",  1, ശൂന്യ, 'O'},
	{ "point-regex",         1, ശൂന്യ, 'p'},
	{ "skip-point-regex",    1, ശൂന്യ, 'P'},
	{ "options",             0, ശൂന്യ, 'i'},
	{ "fstype",              0, ശൂന്യ, 's'},
	{ "node",                0, ശൂന്യ, 't'},
	{ "netdev",              0, ശൂന്യ, 'e'},
	{ "nonetdev",            0, ശൂന്യ, 'E'},
	longopts_COMMON
};
സ്ഥിരം അക്ഷരം * സ്ഥിരം longopts_help[] = {
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
സ്ഥിരം അക്ഷരം *usagestring = ശൂന്യ;

#define UMOUNT_ARGS_MAX 16
#define RUN_MAX 32
#define TRY_MAX 3
#define TRY_DELAY_MS 1000

ഘടന run_queue {
	സ്ഥിരം അക്ഷരം *mntpath;
	int64_t last_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	പൂർണ്ണസംഖ്യ try_എണ്ണം;
	പൂർണ്ണസംഖ്യ fuser_stdoutfd;
};

തരനിർവചനം എണ്ണൽ {
	mount_from,
	mount_to,
	mount_fstype,
	mount_options
} mount_type;

തരനിർവചനം എണ്ണൽ {
	net_ignore,
	net_yes,
	net_no
} net_opts;

ഘടന args;
തരനിർവചനം പൂർണ്ണസംഖ്യ process_func_t(RC_STRINGLIST *, ഘടന args *,
    അക്ഷരം *, അക്ഷരം *, അക്ഷരം *, അക്ഷരം *, പൂർണ്ണസംഖ്യ);

ഘടന args {
	process_func_t *process;
	regex_t *കണ്ണി_regex;
	regex_t *skip_കണ്ണി_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	സ്ഥിരം അക്ഷരം *check_mntpath;
};

സ്ഥിതം പൂർണ്ണസംഖ്യ
process_mount(RC_STRINGLIST *list, ഘടന args *args,
    അക്ഷരം *from, അക്ഷരം *to, അക്ഷരം *fstype, അക്ഷരം *options,
    പൂർണ്ണസംഖ്യ netdev)
{
	അക്ഷരം *p;
	RC_STRING *s;

	errno = ENOENT;

#ifdef __linux__
	/* Skip the really silly rootfs */
	എങ്കിൽ (strcmp(fstype, "rootfs") == 0)
		തിരികെ -1;
#endif

	എങ്കിൽ (args->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		എങ്കിൽ (netdev != 0)
			തിരികെ 1;
	} അല്ലെങ്കിൽ എങ്കിൽ (args->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(args->mounts)))
	{
		എങ്കിൽ (netdev != 1)
			തിരികെ 1;
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (args->കണ്ണി_regex &&
		    regexec(args->കണ്ണി_regex, from, 0, ശൂന്യ, 0) != 0)
			തിരികെ 1;
		എങ്കിൽ (args->skip_കണ്ണി_regex &&
		    regexec(args->skip_കണ്ണി_regex, from, 0, ശൂന്യ, 0) == 0)
			തിരികെ 1;

		എങ്കിൽ (args->fstype_regex &&
		    regexec(args->fstype_regex, fstype, 0, ശൂന്യ, 0) != 0)
			തിരികെ -1;
		എങ്കിൽ (args->skip_fstype_regex &&
		    regexec(args->skip_fstype_regex, fstype, 0, ശൂന്യ, 0) == 0)
			തിരികെ -1;

		എങ്കിൽ (args->options_regex &&
		    regexec(args->options_regex, options, 0, ശൂന്യ, 0) != 0)
			തിരികെ -1;
		എങ്കിൽ (args->skip_options_regex &&
		    regexec(args->skip_options_regex, options, 0, ശൂന്യ, 0) == 0)
			തിരികെ -1;
	}

	എങ്കിൽ (TAILQ_FIRST(args->mounts)) {
		TAILQ_FOREACH(s, args->mounts, entries)
		    എങ്കിൽ (strcmp(s->മൂല്യം, to) == 0)
			    നിർത്തുക;
		എങ്കിൽ (!s)
			തിരികെ -1;
	}

	തിരഞ്ഞെടുപ്പ് (args->mount_type) {
	അവസ്ഥ mount_from:
		p = from;
		നിർത്തുക;
	അവസ്ഥ mount_to:
		p = to;
		നിർത്തുക;
	അവസ്ഥ mount_fstype:
		p = fstype;
		നിർത്തുക;
	അവസ്ഥ mount_options:
		p = options;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		p = ശൂന്യ;
		errno = EINVAL;
		നിർത്തുക;
	}

	എങ്കിൽ (p) {
		errno = 0;
		rc_stringlist_add(list, p);
		തിരികെ 0;
	}

	തിരികെ -1;
}

#എങ്കിൽ defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
സ്ഥിതം ഘടന opt {
	പൂർണ്ണസംഖ്യ o_opt;
	സ്ഥിരം അക്ഷരം *o_name;
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
	{ 0, ശൂന്യ }
};

സ്ഥിതം RC_STRINGLIST *
find_mounts(ഘടന args *args, size_t *അക്കം_mounts)
{
	ഘടന statfs *mnts;
	പൂർണ്ണസംഖ്യ nmnts;
	പൂർണ്ണസംഖ്യ i;
	RC_STRINGLIST *list;
	അക്ഷരം *options = ശൂന്യ;
	uint64_t flags;
	ഘടന opt *o;
	പൂർണ്ണസംഖ്യ netdev;
	അക്ഷരം *tmp;

	എങ്കിൽ ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	വേണ്ടി (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		വേണ്ടി (o = optnames; flags && o->o_opt; o++) {
			എങ്കിൽ (flags & o->o_opt) {
				എങ്കിൽ (o->o_opt == MNT_LOCAL)
					netdev = 1;
				എങ്കിൽ (!options)
					options = xstrdup(o->o_name);
				അല്ലെങ്കിൽ {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					സ്വതന്ത്രം(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*അക്കം_mounts += 0 == args->process(list, args,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		സ്വതന്ത്രം(options);
		options = ശൂന്യ;
	}

	തിരികെ list;
}

#അല്ലെങ്കിലെങ്കിൽ defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
സ്ഥിതം ഘടന mntent *
getmntfile(സ്ഥിരം അക്ഷരം *file)
{
	ഘടന mntent *ent = ശൂന്യ;
	FILE *fp;

	എങ്കിൽ (!(fp = setmntent("/etc/fstab", "r")))
		തിരികെ ശൂന്യ;
	ആകുമ്പോൾ ((ent = getmntent(fp)))
		എങ്കിൽ (strcmp(file, ent->mnt_dir) == 0)
			നിർത്തുക;
	endmntent(fp);

	തിരികെ ent;
}

സ്ഥിതം RC_STRINGLIST *
find_mounts(ഘടന args *args, size_t *അക്കം_mounts)
{
	FILE *fp;
	അക്ഷരം *buffer;
	size_t size;
	അക്ഷരം *p;
	അക്ഷരം *from;
	അക്ഷരം *to;
	അക്ഷരം *fst;
	അക്ഷരം *opts;
	ഘടന mntent *ent;
	പൂർണ്ണസംഖ്യ netdev;
	RC_STRINGLIST *list;

	എങ്കിൽ ((fp = fopen(procmounts, "r")) == ശൂന്യ)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	buffer = ശൂന്യ;
	ആകുമ്പോൾ (xgetline(&buffer, &size, fp) != -1) {
		netdev = -1;
		p = buffer;
		from = strsep(&p, " ");
		to = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		എങ്കിൽ ((ent = getmntfile(to))) {
			എങ്കിൽ (strstr(ent->mnt_opts, "_netdev"))
				netdev = 0;
			അല്ലെങ്കിൽ
				netdev = 1;
		}

		*അക്കം_mounts += 0 == args->process(list, args,
			from, to, fst, opts, netdev);

		സ്വതന്ത്രം(buffer);
		buffer = ശൂന്യ;
	}
	സ്വതന്ത്രം(buffer);
	fclose(fp);

	തിരികെ list;
}

#അല്ലെങ്കിൽ
#  error "Operating system not supported!"
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ is_prefix(സ്ഥിരം അക്ഷരം *needle, സ്ഥിരം അക്ഷരം *hay)
{
	size_t nlen = strlen(needle);
	എങ്കിൽ (strncmp(needle, hay, nlen) == 0 && hay[nlen] == '/')
		തിരികെ true;
	തിരികെ false;
}

സ്ഥിതം അക്ഷരം *unescape_octal(അക്ഷരം *beg)
{
	പൂർണ്ണസംഖ്യ n, i;
	അക്ഷരം *w = beg, *r = beg;
	ആകുമ്പോൾ (*r) {
		എങ്കിൽ (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} അല്ലെങ്കിൽ {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			വേണ്ടി (i = n = 0; i < 3; ++i) {
				എങ്കിൽ (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} അല്ലെങ്കിൽ {
					നിർത്തുക;
				}
			}
			എങ്കിൽ (n)
				*w++ = n;
		}
	}
	*w = '\0';
	തിരികെ beg;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ check_is_mounted(RC_STRINGLIST *list RC_UNUSED, ഘടന args *args,
    അക്ഷരം *from RC_UNUSED, അക്ഷരം *to, അക്ഷരം *fstype RC_UNUSED,
    അക്ഷരം *options RC_UNUSED, പൂർണ്ണസംഖ്യ netdev RC_UNUSED)
{
	തിരികെ strcmp(args->check_mntpath, unescape_octal(to)) == 0 ? 0 : -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ is_mounted(സ്ഥിരം അക്ഷരം *mntpath)
{
	size_t അക്കം_mounts = 0;
	ഘടന args args = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&args, &അക്കം_mounts);
	rc_stringlist_free(l);
	തിരികെ അക്കം_mounts > 0;
}

സ്ഥിതം pid_t run_umount(സ്ഥിരം അക്ഷരം *mntpath,
	സ്ഥിരം അക്ഷരം **umount_args, പൂർണ്ണസംഖ്യ umount_args_അക്കം)
{
	ഘടന exec_args args;
	ഘടന exec_result res;
	സ്ഥിരം അക്ഷരം *argv[UMOUNT_ARGS_MAX + 3];
	പൂർണ്ണസംഖ്യ k, i = 0;

	argv[i++] = "umount";
	വേണ്ടി (k = 0; k < umount_args_അക്കം; ++k)
		argv[i++] = umount_args[k];
	argv[i++] = mntpath;
	argv[i++] = ശൂന്യ;

	args = exec_init(argv);
	args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	എങ്കിൽ (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	തിരികെ res.pid;
}

സ്ഥിതം ശൂന്യം fuser_run(ഘടന run_queue *rp, സ്ഥിരം അക്ഷരം *fuser_opt)
{
	സ്ഥിതം പൂർണ്ണസംഖ്യ fuser_exec_failed = 0;
	സ്ഥിരം അക്ഷരം *argv[] = { "fuser", fuser_opt, rp->mntpath, ശൂന്യ };
	ഘടന exec_result res;
	ഘടന exec_args args;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	എങ്കിൽ (fuser_exec_failed)
		തിരികെ;

	args = exec_init(argv);
	args.redirect_stdout = EXEC_MKPIPE;
	args.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&args);
	എങ്കിൽ (res.pid < 0) {
		fuser_exec_failed = 1;
	} അല്ലെങ്കിൽ {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

സ്ഥിതം പൂർണ്ണസംഖ്യ fuser_decide(ഘടന run_queue *rp,
	സ്ഥിരം അക്ഷരം *fuser_opt, സ്ഥിരം അക്ഷരം *fuser_kill_prefix)
{
	സ്ഥിതം bool has_rc_pid = false;
	സ്ഥിതം അക്ഷരം *rc_pid;
	അക്ഷരം buf[1<<12];
	അക്ഷരം selfpid[64];
	പൂർണ്ണസംഖ്യ read_maybe_truncated;
	ssize_t n;

	എങ്കിൽ (rp->fuser_stdoutfd < 0)
		തിരികെ 0;

	buf[0] = ' ';
	n = പഠ(rp->fuser_stdoutfd, buf + 1, വലുപ്പം buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == വലുപ്പം buf - 3);
	എങ്കിൽ (n < 0 || read_maybe_truncated)
		തിരികെ 0;
	ആകുമ്പോൾ (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, വലുപ്പം selfpid, " %lld ", (long long)getpid());

	എങ്കിൽ (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	എങ്കിൽ (strstr(buf, selfpid) || (rc_pid && strstr(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		തിരികെ -1;
	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(buf, "  ") == 0) {
		എങ്കിൽ (rp->try_എണ്ണം >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			തിരികെ -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_എണ്ണം = TRY_MAX;
		തിരികെ 0;
	} അല്ലെങ്കിൽ {
		അക്ഷരം sig[32];
		സ്ഥിരം അക്ഷരം *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, ശൂന്യ
		};
		ഘടന exec_result res;
		ഘടന exec_args args = exec_init(argv);
		args.redirect_stdout = args.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, വലുപ്പം sig, "%s%s", fuser_kill_prefix,
			rp->try_എണ്ണം == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&args);
		എങ്കിൽ (res.pid > 0)
			rc_waitpid(res.pid);
		തിരികെ 0;
	}
}

സ്ഥിതം regex_t *
get_regex(സ്ഥിരം അക്ഷരം *string)
{
	regex_t *reg = xmalloc(വലുപ്പം (*reg));
	പൂർണ്ണസംഖ്യ result;
	അക്ഷരം buffer[256];

	എങ്കിൽ ((result = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(result, reg, buffer, വലുപ്പം(buffer));
		eerrorx("%s: invalid regex `%s'", applet, buffer);
	}

	തിരികെ reg;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	ഘടന args args;
	regex_t *point_regex = ശൂന്യ;
	regex_t *skip_point_regex = ശൂന്യ;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	അക്ഷരം *real_path = ശൂന്യ;
	പൂർണ്ണസംഖ്യ opt;
	പൂർണ്ണസംഖ്യ result;
	അക്ഷരം *this_path, *argv0;
	സ്ഥിരം അക്ഷരം *tmps;
	സ്ഥിരം അക്ഷരം *fuser_opt, *fuser_kill_prefix;
	size_t അക്കം_mounts = 0;
	size_t unmount_index;
	പൂർണ്ണസംഖ്യ doing_unmount = 0;
	pid_t pid;
	പൂർണ്ണസംഖ്യ status, flags;
	int64_t tmp, next_retry, now;
	int64_t rc_fuser_timeout = -1;
	സ്ഥിരം അക്ഷരം *umount_args[UMOUNT_ARGS_MAX];
	പൂർണ്ണസംഖ്യ umount_args_അക്കം = 0;
	സ്ഥിരം അക്ഷരം **mounts = ശൂന്യ;
	ഘടന run_queue running[RUN_MAX] = {0};
	ഘടന run_queue *rp;
	size_t അക്കം_running = 0, അക്കം_waiting = 0;
	എണ്ണൽ { STATE_RUN, STATE_REAP, STATE_RETRY, STATE_END } state;

#define DO_REG(_var)							      \
	എങ്കിൽ (_var) സ്വതന്ത്രം(_var);						      \
	_var = get_regex(optarg);
#define REG_FREE(_var)							      \
	എങ്കിൽ (_var) { regfree(_var); സ്വതന്ത്രം(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&args, 0, വലുപ്പം(args));
	args.mount_type = mount_to;
	args.netdev = net_ignore;
	args.mounts = rc_stringlist_new();
	args.process = process_mount;

	എങ്കിൽ (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		ആകുമ്പോൾ (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			എങ്കിൽ (strcmp(tmps, "--") == 0)
				നിർത്തുക;
			എങ്കിൽ (umount_args_അക്കം >= (പൂർണ്ണസംഖ്യ)ARRAY_SIZE(umount_args))
				eerrorx("%s: Too many umount arguments", applet);
			umount_args[umount_args_അക്കം++] = tmps;
		}

		tmps = rc_conf_മൂല്യം("rc_fuser_timeout");
		എങ്കിൽ (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		എങ്കിൽ (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		എങ്കിൽ (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_prefix = "-";
		} അല്ലെങ്കിൽ {
			fuser_opt = "-cm";
			fuser_kill_prefix = "-s";
		}
	}

	ആകുമ്പോൾ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (പൂർണ്ണസംഖ്യ *) 0)) != -1)
	{
		തിരഞ്ഞെടുപ്പ് (opt) {
		അവസ്ഥ 'e':
			args.netdev = net_yes;
			നിർത്തുക;
		അവസ്ഥ 'E':
			args.netdev = net_no;
			നിർത്തുക;
		അവസ്ഥ 'f':
			DO_REG(args.fstype_regex);
			നിർത്തുക;
		അവസ്ഥ 'F':
			DO_REG(args.skip_fstype_regex);
			നിർത്തുക;
		അവസ്ഥ 'n':
			DO_REG(args.കണ്ണി_regex);
			നിർത്തുക;
		അവസ്ഥ 'N':
			DO_REG(args.skip_കണ്ണി_regex);
			നിർത്തുക;
		അവസ്ഥ 'o':
			DO_REG(args.options_regex);
			നിർത്തുക;
		അവസ്ഥ 'O':
			DO_REG(args.skip_options_regex);
			നിർത്തുക;
		അവസ്ഥ 'p':
			DO_REG(point_regex);
			നിർത്തുക;
		അവസ്ഥ 'P':
			DO_REG(skip_point_regex);
			നിർത്തുക;
		അവസ്ഥ 'i':
			args.mount_type = mount_options;
			നിർത്തുക;
		അവസ്ഥ 's':
			args.mount_type = mount_fstype;
			നിർത്തുക;
		അവസ്ഥ 't':
			args.mount_type = mount_from;
			നിർത്തുക;

		case_RC_COMMON_GETOPT
		}
	}

	ആകുമ്പോൾ (optind < argc) {
		എങ്കിൽ (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_path = argv[optind++];
		real_path = realpath(this_path, ശൂന്യ);
		എങ്കിൽ (real_path)
			this_path = real_path;
		rc_stringlist_add(args.mounts, this_path);
		സ്വതന്ത്രം(real_path);
		real_path = ശൂന്യ;
	}
	nodes = find_mounts(&args, &അക്കം_mounts);
	rc_stringlist_free(args.mounts);

	എങ്കിൽ (doing_unmount)
		mounts = xmalloc(അക്കം_mounts * വലുപ്പം(*mounts));
	അക്കം_mounts = 0;
	result = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		എങ്കിൽ (point_regex &&
		    regexec(point_regex, s->മൂല്യം, 0, ശൂന്യ, 0) != 0)
			തുടരുക;
		എങ്കിൽ (skip_point_regex &&
		    regexec(skip_point_regex, s->മൂല്യം, 0, ശൂന്യ, 0) == 0)
			തുടരുക;
		എങ്കിൽ (doing_unmount)
			mounts[അക്കം_mounts++] = unescape_octal(s->മൂല്യം);
		അല്ലെങ്കിൽ എങ്കിൽ (!rc_yesno(getenv("EINFO_QUIET")))
			printf("%s\n", s->മൂല്യം);
		result = EXIT_SUCCESS;
	}
	എങ്കിൽ (!doing_unmount)
		പോകുക exit;

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
	ആകുമ്പോൾ (state != STATE_END) തിരഞ്ഞെടുപ്പ് (state) {
	അവസ്ഥ STATE_RUN:
		വേണ്ടി (unmount_index = 0; unmount_index < അക്കം_mounts; ++unmount_index) {
			സ്ഥിരം അക്ഷരം *candidate = mounts[unmount_index];
			പൂർണ്ണസംഖ്യ safe_to_unmount = 1;
			വേണ്ടി (size_t k = 0; safe_to_unmount && k < അക്കം_running; ++k)
				safe_to_unmount = !is_prefix(candidate, running[k].mntpath);
			വേണ്ടി (size_t k = 0; safe_to_unmount && k < അക്കം_mounts; ++k)
				safe_to_unmount = !is_prefix(candidate, mounts[k]);
			എങ്കിൽ (!safe_to_unmount)
				തുടരുക;
			എങ്കിൽ (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_index--] = mounts[--അക്കം_mounts];
			} അല്ലെങ്കിൽ {
				നിർത്തുക;
			}
		}
		എങ്കിൽ (അക്കം_running == RUN_MAX || unmount_index >= അക്കം_mounts) {
			state = STATE_REAP;
			നിർത്തുക;
		}
		rp = running + അക്കം_running++;
		rp->mntpath = mounts[unmount_index];
		rp->last_exec_time = tm_now();
		rp->try_എണ്ണം = 0;
		rp->pid = run_umount(rp->mntpath, umount_args, umount_args_അക്കം);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_index] = mounts[--അക്കം_mounts];
		നിർത്തുക;
	അവസ്ഥ STATE_REAP:
		flags = (അക്കം_waiting > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &status, flags);
		rp = ശൂന്യ;
		വേണ്ടി (size_t i = 0; i < അക്കം_running; ++i) {
			rp = running + i;
			എങ്കിൽ (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			എങ്കിൽ (rp->pid == pid && pid > 0)
				നിർത്തുക;
			rp = ശൂന്യ;
		}
		എങ്കിൽ (rp) {
			എങ്കിൽ ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--അക്കം_running];
				state = STATE_RUN;
			} അല്ലെങ്കിൽ എങ്കിൽ (rp->try_എണ്ണം >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--അക്കം_running];
				result = EXIT_FAILURE;
			} അല്ലെങ്കിൽ { /* put into waiting queue */
				rp->pid = -1;
				rp->try_എണ്ണം += 1;
				അക്കം_waiting += 1;
				fuser_run(rp, fuser_opt);
			}
		} അല്ലെങ്കിൽ {
			state = STATE_RETRY;
		}
		നിർത്തുക;
	അവസ്ഥ STATE_RETRY:
		rp = ശൂന്യ;
		next_retry = INT64_MAX;
		വേണ്ടി (size_t i = 0; i < അക്കം_running; ++i) {
			എങ്കിൽ (running[i].pid > 0)
				തുടരുക;
			എങ്കിൽ (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			അല്ലെങ്കിൽ
				tmp = running[i].last_exec_time + TRY_DELAY_MS;
			എങ്കിൽ (tmp < next_retry) {
				rp = running + i;
				next_retry = tmp;
			}
		}
		എങ്കിൽ (!rp) {
			state = (അക്കം_mounts > 0) ? STATE_RUN : STATE_END;
			നിർത്തുക;
		}
		now = tm_now();
		എങ്കിൽ ((rp->pid > 0 || rp->fuser_pid > 0) && next_retry > now) {
			int64_t sleep_for = next_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			എങ്കിൽ (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		എങ്കിൽ ((rp->pid == -1 && rp->fuser_pid == -1) || next_retry <= now) {
			എങ്കിൽ (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, ശൂന്യ, 0);
				rp->fuser_pid = -1;
			}
			എങ്കിൽ (fuser_decide(rp, fuser_opt, fuser_kill_prefix) < 0) { /* abort */
				*rp = running[--അക്കം_running];
				result = EXIT_FAILURE;
			} അല്ലെങ്കിൽ { /* retry */
				rp->last_exec_time = tm_now();
				rp->pid = run_umount(rp->mntpath,
					umount_args, umount_args_അക്കം);
			}
			അക്കം_waiting -= 1;
			state = STATE_REAP;
		}
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി: നിർത്തുക;
	}

exit:
	സ്വതന്ത്രം(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(args.fstype_regex);
	REG_FREE(args.skip_fstype_regex);
	REG_FREE(args.കണ്ണി_regex);
	REG_FREE(args.skip_കണ്ണി_regex);
	REG_FREE(args.options_regex);
	REG_FREE(args.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	തിരികെ result;
}
