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
#यदि_अन्तः

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

#परिभाषय UMOUNT_तर्काः_MAX 16
#परिभाषय सततम्_MAX 32
#परिभाषय TRY_MAX 3
#परिभाषय TRY_DELAY_MS 1000

संरचना सततम्_queue {
	ध्रुव वर्ण *mntpath;
	int64_t चरमः_exec_time;
	int64_t fuser_exec_time;
	pid_t pid;
	pid_t fuser_pid;
	पूर्णाङ्क try_गणनम्;
	पूर्णाङ्क fuser_stdoutfd;
};

प्रकारनिर्देश गणना {
	mount_from,
	mount_गन्तव्यम्,
	mount_fstype,
	mount_options
} mount_type;

प्रकारनिर्देश गणना {
	net_ignore,
	net_yes,
	net_no
} net_opts;

संरचना तर्काः;
प्रकारनिर्देश पूर्णाङ्क process_func_t(RC_STRINGLIST *, संरचना तर्काः *,
    वर्ण *, वर्ण *, वर्ण *, वर्ण *, पूर्णाङ्क);

संरचना तर्काः {
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
process_mount(RC_STRINGLIST *list, संरचना तर्काः *तर्काः,
    वर्ण *from, वर्ण *गन्तव्यम्, वर्ण *fstype, वर्ण *options,
    पूर्णाङ्क netdev)
{
	वर्ण *p;
	RC_STRING *s;

	errno = ENOENT;

#यदि_परिभाषितम् __linux__
	/* Skip the really silly rootfs */
	यदि (वाक्यं_तुलय(fstype, "rootfs") == 0)
		प्रत्यागम -1;
#यदि_अन्तः

	यदि (तर्काः->netdev == net_yes &&
	    (netdev != -1 || TAILQ_FIRST(तर्काः->mounts)))
	{
		यदि (netdev != 0)
			प्रत्यागम 1;
	} अन्यथा यदि (तर्काः->netdev == net_no &&
	    (netdev != -1 || TAILQ_FIRST(तर्काः->mounts)))
	{
		यदि (netdev != 1)
			प्रत्यागम 1;
	} अन्यथा {
		यदि (तर्काः->ग्रन्थिः_regex &&
		    regexec(तर्काः->ग्रन्थिः_regex, from, 0, रिक्त, 0) != 0)
			प्रत्यागम 1;
		यदि (तर्काः->skip_ग्रन्थिः_regex &&
		    regexec(तर्काः->skip_ग्रन्थिः_regex, from, 0, रिक्त, 0) == 0)
			प्रत्यागम 1;

		यदि (तर्काः->fstype_regex &&
		    regexec(तर्काः->fstype_regex, fstype, 0, रिक्त, 0) != 0)
			प्रत्यागम -1;
		यदि (तर्काः->skip_fstype_regex &&
		    regexec(तर्काः->skip_fstype_regex, fstype, 0, रिक्त, 0) == 0)
			प्रत्यागम -1;

		यदि (तर्काः->options_regex &&
		    regexec(तर्काः->options_regex, options, 0, रिक्त, 0) != 0)
			प्रत्यागम -1;
		यदि (तर्काः->skip_options_regex &&
		    regexec(तर्काः->skip_options_regex, options, 0, रिक्त, 0) == 0)
			प्रत्यागम -1;
	}

	यदि (TAILQ_FIRST(तर्काः->mounts)) {
		TAILQ_FOREACH(s, तर्काः->mounts, entries)
		    यदि (वाक्यं_तुलय(s->मानम्, गन्तव्यम्) == 0)
			    भङ्ग;
		यदि (!s)
			प्रत्यागम -1;
	}

	विकल्प (तर्काः->mount_type) {
	पक्ष mount_from:
		p = from;
		भङ्ग;
	पक्ष mount_गन्तव्यम्:
		p = गन्तव्यम्;
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

	यदि (p) {
		errno = 0;
		rc_stringlist_सङ्कलय(list, p);
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
#यदि_परिभाषितम् MNT_NOSYMFOLLOW
	{ MNT_NOSYMFOLLOW,  "nosymfollow" },
#यदि_अन्तः
	{ MNT_QUOTA,        "with quotas" },
	{ MNT_RDONLY,       "read-only" },
	{ MNT_SYNCHRONOUS,  "synchronous" },
	{ MNT_UNION,        "union" },
#यदि_परिभाषितम् MNT_NOCLUSTERR
	{ MNT_NOCLUSTERR,   "noclusterr" },
#यदि_अन्तः
#यदि_परिभाषितम् MNT_NOCLUSTERW
	{ MNT_NOCLUSTERW,   "noclusterw" },
#यदि_अन्तः
#यदि_परिभाषितम् MNT_SUIDDIR
	{ MNT_SUIDDIR,      "suiddir" },
#यदि_अन्तः
	{ MNT_SOFTDEP,      "soft-updates" },
#यदि_परिभाषितम् MNT_MULTILABEL
	{ MNT_MULTILABEL,   "multilabel" },
#यदि_अन्तः
#यदि_परिभाषितम् MNT_ACLS
	{ MNT_ACLS,         "acls" },
#यदि_अन्तः
#यदि_परिभाषितम् MNT_GJOURNAL
	{ MNT_GJOURNAL,     "gjournal" },
#यदि_अन्तः
	{ 0, रिक्त }
};

स्थिर RC_STRINGLIST *
find_mounts(संरचना तर्काः *तर्काः, size_t *अङ्कः_mounts)
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
		eerrorx("getmntinfo: %s", strerror (errno));

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

		*अङ्कः_mounts += 0 == तर्काः->process(list, तर्काः,
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
	FILE *fp;

	यदि (!(fp = setmntent("/etc/fstab", "r")))
		प्रत्यागम रिक्त;
	यावत् ((ent = getmntent(fp)))
		यदि (वाक्यं_तुलय(file, ent->mnt_dir) == 0)
			भङ्ग;
	endmntent(fp);

	प्रत्यागम ent;
}

स्थिर RC_STRINGLIST *
find_mounts(संरचना तर्काः *तर्काः, size_t *अङ्कः_mounts)
{
	FILE *fp;
	वर्ण *बफर;
	size_t आकार;
	वर्ण *p;
	वर्ण *from;
	वर्ण *गन्तव्यम्;
	वर्ण *fst;
	वर्ण *opts;
	संरचना mntent *ent;
	पूर्णाङ्क netdev;
	RC_STRINGLIST *list;

	यदि ((fp = fopen(procmounts, "r")) == रिक्त)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	बफर = रिक्त;
	यावत् (xgetline(&बफर, &आकार, fp) != -1) {
		netdev = -1;
		p = बफर;
		from = strsep(&p, " ");
		गन्तव्यम् = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		यदि ((ent = getmntfile(गन्तव्यम्))) {
			यदि (वाक्ये_अन्विष(ent->mnt_opts, "_netdev"))
				netdev = 0;
			अन्यथा
				netdev = 1;
		}

		*अङ्कः_mounts += 0 == तर्काः->process(list, तर्काः,
			from, गन्तव्यम्, fst, opts, netdev);

		मुक्त(बफर);
		बफर = रिक्त;
	}
	मुक्त(बफर);
	fclose(fp);

	प्रत्यागम list;
}

#अन्यथा
#  error "Operating system not supported!"
#यदि_अन्तः

स्थिर पूर्णाङ्क is_उपसर्गः(ध्रुव वर्ण *needle, ध्रुव वर्ण *hay)
{
	size_t nlen = वाक्यदैर्घ्यम्(needle);
	यदि (वाक्ये_तुलय(needle, hay, nlen) == 0 && hay[nlen] == '/')
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

स्थिर पूर्णाङ्क check_is_mounted(RC_STRINGLIST *list RC_अप्रयुक्तम्, संरचना तर्काः *तर्काः,
    वर्ण *from RC_अप्रयुक्तम्, वर्ण *गन्तव्यम्, वर्ण *fstype RC_अप्रयुक्तम्,
    वर्ण *options RC_अप्रयुक्तम्, पूर्णाङ्क netdev RC_अप्रयुक्तम्)
{
	प्रत्यागम वाक्यं_तुलय(तर्काः->check_mntpath, unescape_octal(गन्तव्यम्)) == 0 ? 0 : -1;
}

स्थिर पूर्णाङ्क is_mounted(ध्रुव वर्ण *mntpath)
{
	size_t अङ्कः_mounts = 0;
	संरचना तर्काः तर्काः = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = find_mounts(&तर्काः, &अङ्कः_mounts);
	rc_stringlist_free(l);
	प्रत्यागम अङ्कः_mounts > 0;
}

स्थिर pid_t सततम्_umount(ध्रुव वर्ण *mntpath,
	ध्रुव वर्ण **umount_तर्काः, पूर्णाङ्क umount_तर्काः_अङ्कः)
{
	संरचना exec_तर्काः तर्काः;
	संरचना exec_परिणाम res;
	ध्रुव वर्ण *argv[UMOUNT_तर्काः_MAX + 3];
	पूर्णाङ्क k, i = 0;

	argv[i++] = "umount";
	प्रति (k = 0; k < umount_तर्काः_अङ्कः; ++k)
		argv[i++] = umount_तर्काः[k];
	argv[i++] = mntpath;
	argv[i++] = रिक्त;

	तर्काः = exec_init(argv);
	तर्काः.redirect_stdout = तर्काः.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&तर्काः);
	यदि (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	प्रत्यागम res.pid;
}

स्थिर शून्य fuser_सततम्(संरचना सततम्_queue *rp, ध्रुव वर्ण *fuser_opt)
{
	स्थिर पूर्णाङ्क fuser_exec_failed = 0;
	ध्रुव वर्ण *argv[] = { "fuser", fuser_opt, rp->mntpath, रिक्त };
	संरचना exec_परिणाम res;
	संरचना exec_तर्काः तर्काः;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	यदि (fuser_exec_failed)
		प्रत्यागम;

	तर्काः = exec_init(argv);
	तर्काः.redirect_stdout = EXEC_MKPIPE;
	तर्काः.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&तर्काः);
	यदि (res.pid < 0) {
		fuser_exec_failed = 1;
	} अन्यथा {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_time = tm_now();
	}
}

स्थिर पूर्णाङ्क fuser_decide(संरचना सततम्_queue *rp,
	ध्रुव वर्ण *fuser_opt, ध्रुव वर्ण *fuser_kill_उपसर्गः)
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
	close(rp->fuser_stdoutfd);
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
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	यदि (वाक्ये_अन्विष(buf, selfpid) || (rc_pid && वाक्ये_अन्विष(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		प्रत्यागम -1;
	} अन्यथा यदि (वाक्यं_तुलय(buf, "  ") == 0) {
		यदि (rp->try_गणनम् >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			प्रत्यागम -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_गणनम् = TRY_MAX;
		प्रत्यागम 0;
	} अन्यथा {
		वर्ण sig[32];
		ध्रुव वर्ण *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, रिक्त
		};
		संरचना exec_परिणाम res;
		संरचना exec_तर्काः तर्काः = exec_init(argv);
		तर्काः.redirect_stdout = तर्काः.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, परिमाण sig, "%s%s", fuser_kill_उपसर्गः,
			rp->try_गणनम् == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&तर्काः);
		यदि (res.pid > 0)
			rc_waitpid(res.pid);
		प्रत्यागम 0;
	}
}

स्थिर regex_t *
get_regex(ध्रुव वर्ण *string)
{
	regex_t *reg = xmalloc(परिमाण (*reg));
	पूर्णाङ्क परिणाम;
	वर्ण बफर[256];

	यदि ((परिणाम = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(परिणाम, reg, बफर, परिमाण(बफर));
		eerrorx("%s: invalid regex `%s'", applet, बफर);
	}

	प्रत्यागम reg;
}

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv)
{
	संरचना तर्काः तर्काः;
	regex_t *point_regex = रिक्त;
	regex_t *skip_point_regex = रिक्त;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	वर्ण *real_मार्गः = रिक्त;
	पूर्णाङ्क opt;
	पूर्णाङ्क परिणाम;
	वर्ण *this_मार्गः, *argv0;
	ध्रुव वर्ण *tmps;
	ध्रुव वर्ण *fuser_opt, *fuser_kill_उपसर्गः;
	size_t अङ्कः_mounts = 0;
	size_t unmount_सूचकांक;
	पूर्णाङ्क doing_unmount = 0;
	pid_t pid;
	पूर्णाङ्क स्थितिः, flags;
	int64_t tmp, अग्र_retry, now;
	int64_t rc_fuser_timeout = -1;
	ध्रुव वर्ण *umount_तर्काः[UMOUNT_तर्काः_MAX];
	पूर्णाङ्क umount_तर्काः_अङ्कः = 0;
	ध्रुव वर्ण **mounts = रिक्त;
	संरचना सततम्_queue running[सततम्_MAX] = {0};
	संरचना सततम्_queue *rp;
	size_t अङ्कः_running = 0, अङ्कः_प्रतीक्षा = 0;
	गणना { STATE_सततम्, STATE_REAP, STATE_RETRY, STATE_अन्तः } state;

#परिभाषय DO_REG(_var)							      \
	यदि (_var) मुक्त(_var);						      \
	_var = get_regex(optarg);
#परिभाषय REG_FREE(_var)							      \
	यदि (_var) { regfree(_var); मुक्त(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	स्मृतिं_पूरय (&तर्काः, 0, परिमाण(तर्काः));
	तर्काः.mount_type = mount_गन्तव्यम्;
	तर्काः.netdev = net_ignore;
	तर्काः.mounts = rc_stringlist_new();
	तर्काः.process = process_mount;

	यदि (वाक्यं_तुलय(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		यावत् (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			यदि (वाक्यं_तुलय(tmps, "--") == 0)
				भङ्ग;
			यदि (umount_तर्काः_अङ्कः >= (पूर्णाङ्क)ARRAY_आकार(umount_तर्काः))
				eerrorx("%s: Too many umount arguments", applet);
			umount_तर्काः[umount_तर्काः_अङ्कः++] = tmps;
		}

		tmps = rc_conf_मानम्("rc_fuser_timeout");
		यदि (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		यदि (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		यदि (!tmps || वाक्यं_तुलय(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_उपसर्गः = "-";
		} अन्यथा {
			fuser_opt = "-cm";
			fuser_kill_उपसर्गः = "-s";
		}
	}

	यावत् ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णाङ्क *) 0)) != -1)
	{
		विकल्प (opt) {
		पक्ष 'e':
			तर्काः.netdev = net_yes;
			भङ्ग;
		पक्ष 'E':
			तर्काः.netdev = net_no;
			भङ्ग;
		पक्ष 'f':
			DO_REG(तर्काः.fstype_regex);
			भङ्ग;
		पक्ष 'F':
			DO_REG(तर्काः.skip_fstype_regex);
			भङ्ग;
		पक्ष 'n':
			DO_REG(तर्काः.ग्रन्थिः_regex);
			भङ्ग;
		पक्ष 'N':
			DO_REG(तर्काः.skip_ग्रन्थिः_regex);
			भङ्ग;
		पक्ष 'o':
			DO_REG(तर्काः.options_regex);
			भङ्ग;
		पक्ष 'O':
			DO_REG(तर्काः.skip_options_regex);
			भङ्ग;
		पक्ष 'p':
			DO_REG(point_regex);
			भङ्ग;
		पक्ष 'P':
			DO_REG(skip_point_regex);
			भङ्ग;
		पक्ष 'i':
			तर्काः.mount_type = mount_options;
			भङ्ग;
		पक्ष 's':
			तर्काः.mount_type = mount_fstype;
			भङ्ग;
		पक्ष 't':
			तर्काः.mount_type = mount_from;
			भङ्ग;

		case_RC_COMMON_GETOPT
		}
	}

	यावत् (optind < argc) {
		यदि (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_मार्गः = argv[optind++];
		real_मार्गः = realpath(this_मार्गः, रिक्त);
		यदि (real_मार्गः)
			this_मार्गः = real_मार्गः;
		rc_stringlist_सङ्कलय(तर्काः.mounts, this_मार्गः);
		मुक्त(real_मार्गः);
		real_मार्गः = रिक्त;
	}
	nodes = find_mounts(&तर्काः, &अङ्कः_mounts);
	rc_stringlist_free(तर्काः.mounts);

	यदि (doing_unmount)
		mounts = xmalloc(अङ्कः_mounts * परिमाण(*mounts));
	अङ्कः_mounts = 0;
	परिणाम = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_REVERSE(s, nodes, rc_stringlist, entries) {
		यदि (point_regex &&
		    regexec(point_regex, s->मानम्, 0, रिक्त, 0) != 0)
			अनुवर्त;
		यदि (skip_point_regex &&
		    regexec(skip_point_regex, s->मानम्, 0, रिक्त, 0) == 0)
			अनुवर्त;
		यदि (doing_unmount)
			mounts[अङ्कः_mounts++] = unescape_octal(s->मानम्);
		अन्यथा यदि (!rc_yesno(getenv("EINFO_QUIET")))
			प्रारूप_लिख("%s\n", s->मानम्);
		परिणाम = EXIT_SUCCESS;
	}
	यदि (!doing_unmount)
		गच्छ निर्गच्छ;

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
	परिणाम = EXIT_SUCCESS;
	state = STATE_सततम्;
	यावत् (state != STATE_अन्तः) विकल्प (state) {
	पक्ष STATE_सततम्:
		प्रति (unmount_सूचकांक = 0; unmount_सूचकांक < अङ्कः_mounts; ++unmount_सूचकांक) {
			ध्रुव वर्ण *candidate = mounts[unmount_सूचकांक];
			पूर्णाङ्क safe_गन्तव्यम्_unmount = 1;
			प्रति (size_t k = 0; safe_गन्तव्यम्_unmount && k < अङ्कः_running; ++k)
				safe_गन्तव्यम्_unmount = !is_उपसर्गः(candidate, running[k].mntpath);
			प्रति (size_t k = 0; safe_गन्तव्यम्_unmount && k < अङ्कः_mounts; ++k)
				safe_गन्तव्यम्_unmount = !is_उपसर्गः(candidate, mounts[k]);
			यदि (!safe_गन्तव्यम्_unmount)
				अनुवर्त;
			यदि (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_सूचकांक--] = mounts[--अङ्कः_mounts];
			} अन्यथा {
				भङ्ग;
			}
		}
		यदि (अङ्कः_running == सततम्_MAX || unmount_सूचकांक >= अङ्कः_mounts) {
			state = STATE_REAP;
			भङ्ग;
		}
		rp = running + अङ्कः_running++;
		rp->mntpath = mounts[unmount_सूचकांक];
		rp->चरमः_exec_time = tm_now();
		rp->try_गणनम् = 0;
		rp->pid = सततम्_umount(rp->mntpath, umount_तर्काः, umount_तर्काः_अङ्कः);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_time = -1;
		mounts[unmount_सूचकांक] = mounts[--अङ्कः_mounts];
		भङ्ग;
	पक्ष STATE_REAP:
		flags = (अङ्कः_प्रतीक्षा > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &स्थितिः, flags);
		rp = रिक्त;
		प्रति (size_t i = 0; i < अङ्कः_running; ++i) {
			rp = running + i;
			यदि (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			यदि (rp->pid == pid && pid > 0)
				भङ्ग;
			rp = रिक्त;
		}
		यदि (rp) {
			यदि ((WIFEXITED(स्थितिः) && WEXITSTATUS(स्थितिः) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--अङ्कः_running];
				state = STATE_सततम्;
			} अन्यथा यदि (rp->try_गणनम् >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--अङ्कः_running];
				परिणाम = EXIT_FAILURE;
			} अन्यथा { /* put into waiting queue */
				rp->pid = -1;
				rp->try_गणनम् += 1;
				अङ्कः_प्रतीक्षा += 1;
				fuser_सततम्(rp, fuser_opt);
			}
		} अन्यथा {
			state = STATE_RETRY;
		}
		भङ्ग;
	पक्ष STATE_RETRY:
		rp = रिक्त;
		अग्र_retry = INT64_MAX;
		प्रति (size_t i = 0; i < अङ्कः_running; ++i) {
			यदि (running[i].pid > 0)
				अनुवर्त;
			यदि (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_time + rc_fuser_timeout;
			अन्यथा
				tmp = running[i].चरमः_exec_time + TRY_DELAY_MS;
			यदि (tmp < अग्र_retry) {
				rp = running + i;
				अग्र_retry = tmp;
			}
		}
		यदि (!rp) {
			state = (अङ्कः_mounts > 0) ? STATE_सततम् : STATE_अन्तः;
			भङ्ग;
		}
		now = tm_now();
		यदि ((rp->pid > 0 || rp->fuser_pid > 0) && अग्र_retry > now) {
			int64_t sleep_for = अग्र_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			यदि (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		यदि ((rp->pid == -1 && rp->fuser_pid == -1) || अग्र_retry <= now) {
			यदि (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, रिक्त, 0);
				rp->fuser_pid = -1;
			}
			यदि (fuser_decide(rp, fuser_opt, fuser_kill_उपसर्गः) < 0) { /* abort */
				*rp = running[--अङ्कः_running];
				परिणाम = EXIT_FAILURE;
			} अन्यथा { /* retry */
				rp->चरमः_exec_time = tm_now();
				rp->pid = सततम्_umount(rp->mntpath,
					umount_तर्काः, umount_तर्काः_अङ्कः);
			}
			अङ्कः_प्रतीक्षा -= 1;
			state = STATE_REAP;
		}
		भङ्ग;
	स्वभाव: भङ्ग;
	}

निर्गच्छ:
	मुक्त(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(तर्काः.fstype_regex);
	REG_FREE(तर्काः.skip_fstype_regex);
	REG_FREE(तर्काः.ग्रन्थिः_regex);
	REG_FREE(तर्काः.skip_ग्रन्थिः_regex);
	REG_FREE(तर्काः.options_regex);
	REG_FREE(तर्काः.skip_options_regex);
	REG_FREE(point_regex);
	REG_FREE(skip_point_regex);

	प्रत्यागम परिणाम;
}
