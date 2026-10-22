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

#ఒకవేళ defined(__DragonFly__) || defined(__FreeBSD__)
#  include <sys/ucred.h>
#  include <sys/mount.h>

#  define F_FLAGS f_flags
#లేదావేళ defined(__NetBSD__)
#  include <sys/mount.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#లేదావేళ defined(BSD) && !defined(__GNU__)
#  include <sys/statvfs.h>

#  define statfs statvfs
#  define F_FLAGS f_flag
#లేదావేళ defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
#  include <mntent.h>
#అయితే_ముగింపు

#అన్తర్భావ <errno.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <regex.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "rc_exec.h"
#అన్తర్భావ "timeutils.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "helpers.h"

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *procmounts = "/proc/mounts";
స్థిరరాశి అక్షరం *extraopts = "[mount1] [mount2] ...";
స్థిరరాశి అక్షరం getoptstring[] = "f:F:n:N:o:O:p:P:iste:E:" getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "fstype-regex",        1, ఖాళీ, 'f'},
	{ "skip-fstype-regex",   1, ఖాళీ, 'F'},
	{ "node-regex",          1, ఖాళీ, 'n'},
	{ "skip-node-regex",     1, ఖాళీ, 'N'},
	{ "options-regex",       1, ఖాళీ, 'o'},
	{ "skip-options-regex",  1, ఖాళీ, 'O'},
	{ "point-regex",         1, ఖాళీ, 'p'},
	{ "skip-point-regex",    1, ఖాళీ, 'P'},
	{ "options",             0, ఖాళీ, 'i'},
	{ "fstype",              0, ఖాళీ, 's'},
	{ "node",                0, ఖాళీ, 't'},
	{ "netdev",              0, ఖాళీ, 'e'},
	{ "nonetdev",            0, ఖాళీ, 'E'},
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
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
స్థిరరాశి అక్షరం *usagestring = ఖాళీ;

#నిర్వచించు UMOUNT_వాదనలు_MAX 16
#నిర్వచించు నిరంతరం_MAX 32
#నిర్వచించు TRY_MAX 3
#నిర్వచించు TRY_DELAY_MS 1000

నిర్మాణం నిరంతరం_వరుస {
	స్థిరరాశి అక్షరం *mntpath;
	int64_t చివరి_exec_సమయం;
	int64_t fuser_exec_సమయం;
	pid_t pid;
	pid_t fuser_pid;
	పూర్ణాంకం try_లెక్క;
	పూర్ణాంకం fuser_stdoutfd;
};

రకనిర్వచనం గణన {
	mount_from,
	mount_గమ్యం,
	mount_fstype,
	mount_options
} mount_type;

రకనిర్వచనం గణన {
	net_ignore,
	net_yes,
	net_no
} net_opts;

నిర్మాణం వాదనలు;
రకనిర్వచనం పూర్ణాంకం process_func_t(RC_STRINGLIST *, నిర్మాణం వాదనలు *,
    అక్షరం *, అక్షరం *, అక్షరం *, అక్షరం *, పూర్ణాంకం);

నిర్మాణం వాదనలు {
	process_func_t *process;
	regex_t *కణుపు_regex;
	regex_t *skip_కణుపు_regex;
	regex_t *fstype_regex;
	regex_t *skip_fstype_regex;
	regex_t *options_regex;
	regex_t *skip_options_regex;
	RC_STRINGLIST *mounts;
	mount_type mount_type;
	net_opts netdev;
	స్థిరరాశి అక్షరం *check_mntpath;
};

స్థిర పూర్ణాంకం
process_mount(RC_STRINGLIST *list, నిర్మాణం వాదనలు *వాదనలు,
    అక్షరం *from, అక్షరం *గమ్యం, అక్షరం *fstype, అక్షరం *options,
    పూర్ణాంకం netdev)
{
	అక్షరం *p;
	RC_STRING *s;

	errno = ENOENT;

#నిర్వచితమైతే __linux__
	/* Skip the really silly rootfs */
	ఒకవేళ (strcmp(fstype, "rootfs") == 0)
		తిరిగి -1;
#అయితే_ముగింపు

	ఒకవేళ (వాదనలు->netdev == net_yes &&
	    (netdev != -1 || TAILQ_మొదటి(వాదనలు->mounts)))
	{
		ఒకవేళ (netdev != 0)
			తిరిగి 1;
	} లేకపోతే ఒకవేళ (వాదనలు->netdev == net_no &&
	    (netdev != -1 || TAILQ_మొదటి(వాదనలు->mounts)))
	{
		ఒకవేళ (netdev != 1)
			తిరిగి 1;
	} లేకపోతే {
		ఒకవేళ (వాదనలు->కణుపు_regex &&
		    regexec(వాదనలు->కణుపు_regex, from, 0, ఖాళీ, 0) != 0)
			తిరిగి 1;
		ఒకవేళ (వాదనలు->skip_కణుపు_regex &&
		    regexec(వాదనలు->skip_కణుపు_regex, from, 0, ఖాళీ, 0) == 0)
			తిరిగి 1;

		ఒకవేళ (వాదనలు->fstype_regex &&
		    regexec(వాదనలు->fstype_regex, fstype, 0, ఖాళీ, 0) != 0)
			తిరిగి -1;
		ఒకవేళ (వాదనలు->skip_fstype_regex &&
		    regexec(వాదనలు->skip_fstype_regex, fstype, 0, ఖాళీ, 0) == 0)
			తిరిగి -1;

		ఒకవేళ (వాదనలు->options_regex &&
		    regexec(వాదనలు->options_regex, options, 0, ఖాళీ, 0) != 0)
			తిరిగి -1;
		ఒకవేళ (వాదనలు->skip_options_regex &&
		    regexec(వాదనలు->skip_options_regex, options, 0, ఖాళీ, 0) == 0)
			తిరిగి -1;
	}

	ఒకవేళ (TAILQ_మొదటి(వాదనలు->mounts)) {
		TAILQ_FOREACH(s, వాదనలు->mounts, entries)
		    ఒకవేళ (strcmp(s->విలువ, గమ్యం) == 0)
			    ఆపు;
		ఒకవేళ (!s)
			తిరిగి -1;
	}

	ఎంపిక (వాదనలు->mount_type) {
	స్థితి mount_from:
		p = from;
		ఆపు;
	స్థితి mount_గమ్యం:
		p = గమ్యం;
		ఆపు;
	స్థితి mount_fstype:
		p = fstype;
		ఆపు;
	స్థితి mount_options:
		p = options;
		ఆపు;
	అప్రమేయ:
		p = ఖాళీ;
		errno = EINVAL;
		ఆపు;
	}

	ఒకవేళ (p) {
		errno = 0;
		rc_stringlist_కూడు(list, p);
		తిరిగి 0;
	}

	తిరిగి -1;
}

#ఒకవేళ defined(BSD) && !defined(__GNU__)

/* Translate the mounted options to english
 * This is taken directly from FreeBSD mount.c */
స్థిర నిర్మాణం opt {
	పూర్ణాంకం o_opt;
	స్థిరరాశి అక్షరం *o_name;
} optnames[] = {
	{ MNT_ASYNC,        "asynchronous" },
	{ MNT_EXPORTED,     "NFS exported" },
	{ MNT_LOCAL,        "local" },
	{ MNT_NOATIME,      "noatime" },
	{ MNT_NOEXEC,       "noexec" },
	{ MNT_NOSUID,       "nosuid" },
#నిర్వచితమైతే MNT_NOSYMFOLLOW
	{ MNT_NOSYMFOLLOW,  "nosymfollow" },
#అయితే_ముగింపు
	{ MNT_QUOTA,        "with quotas" },
	{ MNT_RDONLY,       "read-only" },
	{ MNT_SYNCHRONOUS,  "synchronous" },
	{ MNT_UNION,        "union" },
#నిర్వచితమైతే MNT_NOCLUSTERR
	{ MNT_NOCLUSTERR,   "noclusterr" },
#అయితే_ముగింపు
#నిర్వచితమైతే MNT_NOCLUSTERW
	{ MNT_NOCLUSTERW,   "noclusterw" },
#అయితే_ముగింపు
#నిర్వచితమైతే MNT_SUIDDIR
	{ MNT_SUIDDIR,      "suiddir" },
#అయితే_ముగింపు
	{ MNT_SOFTDEP,      "soft-updates" },
#నిర్వచితమైతే MNT_MULTILABEL
	{ MNT_MULTILABEL,   "multilabel" },
#అయితే_ముగింపు
#నిర్వచితమైతే MNT_ACLS
	{ MNT_ACLS,         "acls" },
#అయితే_ముగింపు
#నిర్వచితమైతే MNT_GJOURNAL
	{ MNT_GJOURNAL,     "gjournal" },
#అయితే_ముగింపు
	{ 0, ఖాళీ }
};

స్థిర RC_STRINGLIST *
కనుగొను_mounts(నిర్మాణం వాదనలు *వాదనలు, size_t *అంకె_mounts)
{
	నిర్మాణం statfs *mnts;
	పూర్ణాంకం nmnts;
	పూర్ణాంకం i;
	RC_STRINGLIST *list;
	అక్షరం *options = ఖాళీ;
	uint64_t flags;
	నిర్మాణం opt *o;
	పూర్ణాంకం netdev;
	అక్షరం *tmp;

	ఒకవేళ ((nmnts = getmntinfo(&mnts, MNT_NOWAIT)) == 0)
		eerrorx("getmntinfo: %s", strerror (errno));

	list = rc_stringlist_new();
	కొరకు (i = 0; i < nmnts; i++) {
		netdev = 0;
		flags = mnts[i].F_FLAGS & MNT_VISFLAGMASK;
		కొరకు (o = optnames; flags && o->o_opt; o++) {
			ఒకవేళ (flags & o->o_opt) {
				ఒకవేళ (o->o_opt == MNT_LOCAL)
					netdev = 1;
				ఒకవేళ (!options)
					options = xstrdup(o->o_name);
				లేకపోతే {
					xasprintf(&tmp, "%s,%s", options, o->o_name);
					విడుదల(options);
					options = tmp;
				}
			}
			flags &= ~o->o_opt;
		}

		*అంకె_mounts += 0 == వాదనలు->process(list, వాదనలు,
		    mnts[i].f_mntfromname,
		    mnts[i].f_mntonname,
		    mnts[i].f_fstypename,
		    options,
		    netdev);

		విడుదల(options);
		options = ఖాళీ;
	}

	తిరిగి list;
}

#లేదావేళ defined(__linux__) || (defined(__FreeBSD_kernel__) && \
	defined(__GLIBC__)) || defined(__GNU__)
స్థిర నిర్మాణం mntent *
getmntfile(స్థిరరాశి అక్షరం *file)
{
	నిర్మాణం mntent *ent = ఖాళీ;
	FILE *fp;

	ఒకవేళ (!(fp = setmntent("/etc/fstab", "r")))
		తిరిగి ఖాళీ;
	అయితే ((ent = getmntent(fp)))
		ఒకవేళ (strcmp(file, ent->mnt_dir) == 0)
			ఆపు;
	endmntent(fp);

	తిరిగి ent;
}

స్థిర RC_STRINGLIST *
కనుగొను_mounts(నిర్మాణం వాదనలు *వాదనలు, size_t *అంకె_mounts)
{
	FILE *fp;
	అక్షరం *బఫరు;
	size_t కొలత;
	అక్షరం *p;
	అక్షరం *from;
	అక్షరం *గమ్యం;
	అక్షరం *fst;
	అక్షరం *opts;
	నిర్మాణం mntent *ent;
	పూర్ణాంకం netdev;
	RC_STRINGLIST *list;

	ఒకవేళ ((fp = fopen(procmounts, "r")) == ఖాళీ)
		eerrorx("getmntinfo: %s", strerror(errno));

	list = rc_stringlist_new();

	బఫరు = ఖాళీ;
	అయితే (xgetline(&బఫరు, &కొలత, fp) != -1) {
		netdev = -1;
		p = బఫరు;
		from = strsep(&p, " ");
		గమ్యం = strsep(&p, " ");
		fst = strsep(&p, " ");
		opts = strsep(&p, " ");

		ఒకవేళ ((ent = getmntfile(గమ్యం))) {
			ఒకవేళ (అక్షరశ్రేణి_శోధన(ent->mnt_opts, "_netdev"))
				netdev = 0;
			లేకపోతే
				netdev = 1;
		}

		*అంకె_mounts += 0 == వాదనలు->process(list, వాదనలు,
			from, గమ్యం, fst, opts, netdev);

		విడుదల(బఫరు);
		బఫరు = ఖాళీ;
	}
	విడుదల(బఫరు);
	fclose(fp);

	తిరిగి list;
}

#లేకపోతే
#  error "Operating system not supported!"
#అయితే_ముగింపు

స్థిర పూర్ణాంకం is_ఉపసర్గ(స్థిరరాశి అక్షరం *needle, స్థిరరాశి అక్షరం *hay)
{
	size_t nlen = అక్షరశ్రేణి_పొడవు(needle);
	ఒకవేళ (అక్షరశ్రేణి_పోలిక(needle, hay, nlen) == 0 && hay[nlen] == '/')
		తిరిగి true;
	తిరిగి false;
}

స్థిర అక్షరం *unescape_octal(అక్షరం *beg)
{
	పూర్ణాంకం n, i;
	అక్షరం *w = beg, *r = beg;
	అయితే (*r) {
		ఒకవేళ (*r != '\\' || *++r == '\\') {
			*w++ = *r++;
		} లేకపోతే {
			/* octal. should have at least 3 bytes,
			 * but don't choke on malformed input
			 */
			కొరకు (i = n = 0; i < 3; ++i) {
				ఒకవేళ (*r >= '0' && *r <= '7') {
					n <<= 3;
					n |= *r++ - '0';
				} లేకపోతే {
					ఆపు;
				}
			}
			ఒకవేళ (n)
				*w++ = n;
		}
	}
	*w = '\0';
	తిరిగి beg;
}

స్థిర పూర్ణాంకం check_is_mounted(RC_STRINGLIST *list RC_వాడని, నిర్మాణం వాదనలు *వాదనలు,
    అక్షరం *from RC_వాడని, అక్షరం *గమ్యం, అక్షరం *fstype RC_వాడని,
    అక్షరం *options RC_వాడని, పూర్ణాంకం netdev RC_వాడని)
{
	తిరిగి strcmp(వాదనలు->check_mntpath, unescape_octal(గమ్యం)) == 0 ? 0 : -1;
}

స్థిర పూర్ణాంకం is_mounted(స్థిరరాశి అక్షరం *mntpath)
{
	size_t అంకె_mounts = 0;
	నిర్మాణం వాదనలు వాదనలు = { .process = check_is_mounted, .check_mntpath = mntpath };
	RC_STRINGLIST *l = కనుగొను_mounts(&వాదనలు, &అంకె_mounts);
	rc_stringlist_free(l);
	తిరిగి అంకె_mounts > 0;
}

స్థిర pid_t నిరంతరం_umount(స్థిరరాశి అక్షరం *mntpath,
	స్థిరరాశి అక్షరం **umount_వాదనలు, పూర్ణాంకం umount_వాదనలు_అంకె)
{
	నిర్మాణం exec_వాదనలు వాదనలు;
	నిర్మాణం exec_ఫలితం res;
	స్థిరరాశి అక్షరం *argv[UMOUNT_వాదనలు_MAX + 3];
	పూర్ణాంకం k, i = 0;

	argv[i++] = "umount";
	కొరకు (k = 0; k < umount_వాదనలు_అంకె; ++k)
		argv[i++] = umount_వాదనలు[k];
	argv[i++] = mntpath;
	argv[i++] = ఖాళీ;

	వాదనలు = exec_init(argv);
	వాదనలు.redirect_stdout = వాదనలు.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&వాదనలు);
	ఒకవేళ (res.pid < 0)
		eerrorx("%s: failed to run umount: %s", applet, strerror(errno));
	తిరిగి res.pid;
}

స్థిర శూన్య fuser_నిరంతరం(నిర్మాణం నిరంతరం_వరుస *rp, స్థిరరాశి అక్షరం *fuser_opt)
{
	స్థిర పూర్ణాంకం fuser_exec_failed = 0;
	స్థిరరాశి అక్షరం *argv[] = { "fuser", fuser_opt, rp->mntpath, ఖాళీ };
	నిర్మాణం exec_ఫలితం res;
	నిర్మాణం exec_వాదనలు వాదనలు;

	/* if exec failed, fuser likely doesn't exist. so don't retry */
	ఒకవేళ (fuser_exec_failed)
		తిరిగి;

	వాదనలు = exec_init(argv);
	వాదనలు.redirect_stdout = EXEC_MKPIPE;
	వాదనలు.redirect_stderr = EXEC_DEVNULL;
	res = do_exec(&వాదనలు);
	ఒకవేళ (res.pid < 0) {
		fuser_exec_failed = 1;
	} లేకపోతే {
		rp->fuser_pid = res.pid;
		rp->fuser_stdoutfd = res.proc_stdout;
		rp->fuser_exec_సమయం = tm_now();
	}
}

స్థిర పూర్ణాంకం fuser_decide(నిర్మాణం నిరంతరం_వరుస *rp,
	స్థిరరాశి అక్షరం *fuser_opt, స్థిరరాశి అక్షరం *fuser_kill_ఉపసర్గ)
{
	స్థిర bool has_rc_pid = false;
	స్థిర అక్షరం *rc_pid;
	అక్షరం buf[1<<12];
	అక్షరం selfpid[64];
	పూర్ణాంకం read_maybe_truncated;
	ssize_t n;

	ఒకవేళ (rp->fuser_stdoutfd < 0)
		తిరిగి 0;

	buf[0] = ' ';
	n = పఠ(rp->fuser_stdoutfd, buf + 1, పరిమాణం buf - 3);
	close(rp->fuser_stdoutfd);
	rp->fuser_stdoutfd = -1;
	read_maybe_truncated = (n == పరిమాణం buf - 3);
	ఒకవేళ (n < 0 || read_maybe_truncated)
		తిరిగి 0;
	అయితే (n > 0 && buf[n] == '\n')
		--n;
	buf[n+1] = ' ';
	buf[n+2] = '\0';
	snprintf(selfpid, పరిమాణం selfpid, " %lld ", (దీర్ఘ దీర్ఘ)getpid());

	ఒకవేళ (!has_rc_pid) {
		rc_pid = getenv("RC_PID");
		has_rc_pid = true;
	}

	ఒకవేళ (అక్షరశ్రేణి_శోధన(buf, selfpid) || (rc_pid && అక్షరశ్రేణి_శోధన(buf, rc_pid))) {
		/* lets not kill ourselves */
		eerror("Unmounting %s failed because we are using it", rp->mntpath);
		తిరిగి -1;
	} లేకపోతే ఒకవేళ (strcmp(buf, "  ") == 0) {
		ఒకవేళ (rp->try_లెక్క >= TRY_MAX) {
			eerror("Unmounting %s failed but fuser finds no one using it", rp->mntpath);
			తిరిగి -1;
		}
		/* it's possible that whatever was using the mount stopped
		 * using it now, so allow 1 more retry */
		rp->try_లెక్క = TRY_MAX;
		తిరిగి 0;
	} లేకపోతే {
		అక్షరం sig[32];
		స్థిరరాశి అక్షరం *argv[] = {
			"fuser", sig, "-k", fuser_opt, rp->mntpath, ఖాళీ
		};
		నిర్మాణం exec_ఫలితం res;
		నిర్మాణం exec_వాదనలు వాదనలు = exec_init(argv);
		వాదనలు.redirect_stdout = వాదనలు.redirect_stderr = EXEC_DEVNULL;
		snprintf(sig, పరిమాణం sig, "%s%s", fuser_kill_ఉపసర్గ,
			rp->try_లెక్క == TRY_MAX ? "KILL" : "TERM");
		res = do_exec(&వాదనలు);
		ఒకవేళ (res.pid > 0)
			rc_waitpid(res.pid);
		తిరిగి 0;
	}
}

స్థిర regex_t *
get_regex(స్థిరరాశి అక్షరం *string)
{
	regex_t *reg = xmalloc(పరిమాణం (*reg));
	పూర్ణాంకం ఫలితం;
	అక్షరం బఫరు[256];

	ఒకవేళ ((ఫలితం = regcomp(reg, string, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(ఫలితం, reg, బఫరు, పరిమాణం(బఫరు));
		eerrorx("%s: invalid regex `%s'", applet, బఫరు);
	}

	తిరిగి reg;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	నిర్మాణం వాదనలు వాదనలు;
	regex_t *బిందువు_regex = ఖాళీ;
	regex_t *skip_బిందువు_regex = ఖాళీ;
	RC_STRINGLIST *nodes;
	RC_STRING *s;
	అక్షరం *real_మార్గం = ఖాళీ;
	పూర్ణాంకం opt;
	పూర్ణాంకం ఫలితం;
	అక్షరం *this_మార్గం, *argv0;
	స్థిరరాశి అక్షరం *tmps;
	స్థిరరాశి అక్షరం *fuser_opt, *fuser_kill_ఉపసర్గ;
	size_t అంకె_mounts = 0;
	size_t unmount_సూచీ;
	పూర్ణాంకం doing_unmount = 0;
	pid_t pid;
	పూర్ణాంకం స్థితి_2, flags;
	int64_t tmp, తదుపరి_retry, now;
	int64_t rc_fuser_timeout = -1;
	స్థిరరాశి అక్షరం *umount_వాదనలు[UMOUNT_వాదనలు_MAX];
	పూర్ణాంకం umount_వాదనలు_అంకె = 0;
	స్థిరరాశి అక్షరం **mounts = ఖాళీ;
	నిర్మాణం నిరంతరం_వరుస running[నిరంతరం_MAX] = {0};
	నిర్మాణం నిరంతరం_వరుస *rp;
	size_t అంకె_running = 0, అంకె_నిరీక్షణ = 0;
	గణన { STATE_నిరంతరం, STATE_REAP, STATE_RETRY, STATE_చివర } state;

#నిర్వచించు DO_REG(_var)							      \
	ఒకవేళ (_var) విడుదల(_var);						      \
	_var = get_regex(optarg);
#నిర్వచించు REG_FREE(_var)							      \
	ఒకవేళ (_var) { regfree(_var); విడుదల(_var); }

	argv0 = argv[0];
	applet = basename_c(argv[0]);
	memset (&వాదనలు, 0, పరిమాణం(వాదనలు));
	వాదనలు.mount_type = mount_గమ్యం;
	వాదనలు.netdev = net_ignore;
	వాదనలు.mounts = rc_stringlist_new();
	వాదనలు.process = process_mount;

	ఒకవేళ (strcmp(applet, "do_unmount") == 0) {
		doing_unmount = 1;
		అయితే (argv[1]) {
			/* shift over */
			tmps = argv[1];
			argv[1] = argv0;
			++argv;
			--argc;

			ఒకవేళ (strcmp(tmps, "--") == 0)
				ఆపు;
			ఒకవేళ (umount_వాదనలు_అంకె >= (పూర్ణాంకం)ARRAY_కొలత(umount_వాదనలు))
				eerrorx("%s: Too many umount arguments", applet);
			umount_వాదనలు[umount_వాదనలు_అంకె++] = tmps;
		}

		tmps = rc_conf_విలువ("rc_fuser_timeout");
		ఒకవేళ (tmps && (rc_fuser_timeout = parse_duration(tmps)) < 0)
			ewarn("%s: Invalid rc_fuser_timeout value: `%s`. "
				"Defaulting to 20", applet, tmps);
		ఒకవేళ (rc_fuser_timeout < 0)
			rc_fuser_timeout = 20 * 1000;

		tmps = getenv("RC_UNAME");
		ఒకవేళ (!tmps || strcmp(tmps, "Linux") == 0) {
			fuser_opt = "-m";
			fuser_kill_ఉపసర్గ = "-";
		} లేకపోతే {
			fuser_opt = "-cm";
			fuser_kill_ఉపసర్గ = "-s";
		}
	}

	అయితే ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (పూర్ణాంకం *) 0)) != -1)
	{
		ఎంపిక (opt) {
		స్థితి 'e':
			వాదనలు.netdev = net_yes;
			ఆపు;
		స్థితి 'E':
			వాదనలు.netdev = net_no;
			ఆపు;
		స్థితి 'f':
			DO_REG(వాదనలు.fstype_regex);
			ఆపు;
		స్థితి 'F':
			DO_REG(వాదనలు.skip_fstype_regex);
			ఆపు;
		స్థితి 'n':
			DO_REG(వాదనలు.కణుపు_regex);
			ఆపు;
		స్థితి 'N':
			DO_REG(వాదనలు.skip_కణుపు_regex);
			ఆపు;
		స్థితి 'o':
			DO_REG(వాదనలు.options_regex);
			ఆపు;
		స్థితి 'O':
			DO_REG(వాదనలు.skip_options_regex);
			ఆపు;
		స్థితి 'p':
			DO_REG(బిందువు_regex);
			ఆపు;
		స్థితి 'P':
			DO_REG(skip_బిందువు_regex);
			ఆపు;
		స్థితి 'i':
			వాదనలు.mount_type = mount_options;
			ఆపు;
		స్థితి 's':
			వాదనలు.mount_type = mount_fstype;
			ఆపు;
		స్థితి 't':
			వాదనలు.mount_type = mount_from;
			ఆపు;

		case_RC_COMMON_GETOPT
		}
	}

	అయితే (optind < argc) {
		ఒకవేళ (argv[optind][0] != '/')
			eerrorx("%s: `%s' is not a mount point",
			    argv[0], argv[optind]);
		this_మార్గం = argv[optind++];
		real_మార్గం = realpath(this_మార్గం, ఖాళీ);
		ఒకవేళ (real_మార్గం)
			this_మార్గం = real_మార్గం;
		rc_stringlist_కూడు(వాదనలు.mounts, this_మార్గం);
		విడుదల(real_మార్గం);
		real_మార్గం = ఖాళీ;
	}
	nodes = కనుగొను_mounts(&వాదనలు, &అంకె_mounts);
	rc_stringlist_free(వాదనలు.mounts);

	ఒకవేళ (doing_unmount)
		mounts = xmalloc(అంకె_mounts * పరిమాణం(*mounts));
	అంకె_mounts = 0;
	ఫలితం = EXIT_FAILURE;
	/* We should report the mounts in reverse order to ease unmounting */
	TAILQ_FOREACH_తిరగేయి(s, nodes, rc_stringlist, entries) {
		ఒకవేళ (బిందువు_regex &&
		    regexec(బిందువు_regex, s->విలువ, 0, ఖాళీ, 0) != 0)
			కొనసాగు;
		ఒకవేళ (skip_బిందువు_regex &&
		    regexec(skip_బిందువు_regex, s->విలువ, 0, ఖాళీ, 0) == 0)
			కొనసాగు;
		ఒకవేళ (doing_unmount)
			mounts[అంకె_mounts++] = unescape_octal(s->విలువ);
		లేకపోతే ఒకవేళ (!rc_yesno(getenv("EINFO_QUIET")))
			రాయి("%s\n", s->విలువ);
		ఫలితం = EXIT_SUCCESS;
	}
	ఒకవేళ (!doing_unmount)
		వెళ్ళు నిష్క్రమించు;

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
	ఫలితం = EXIT_SUCCESS;
	state = STATE_నిరంతరం;
	అయితే (state != STATE_చివర) ఎంపిక (state) {
	స్థితి STATE_నిరంతరం:
		కొరకు (unmount_సూచీ = 0; unmount_సూచీ < అంకె_mounts; ++unmount_సూచీ) {
			స్థిరరాశి అక్షరం *candidate = mounts[unmount_సూచీ];
			పూర్ణాంకం సురక్షిత_గమ్యం_unmount = 1;
			కొరకు (size_t k = 0; సురక్షిత_గమ్యం_unmount && k < అంకె_running; ++k)
				సురక్షిత_గమ్యం_unmount = !is_ఉపసర్గ(candidate, running[k].mntpath);
			కొరకు (size_t k = 0; సురక్షిత_గమ్యం_unmount && k < అంకె_mounts; ++k)
				సురక్షిత_గమ్యం_unmount = !is_ఉపసర్గ(candidate, mounts[k]);
			ఒకవేళ (!సురక్షిత_గమ్యం_unmount)
				కొనసాగు;
			ఒకవేళ (!is_mounted(candidate)) {
				/* probably a shared mount and got unmounted, remove */
				mounts[unmount_సూచీ--] = mounts[--అంకె_mounts];
			} లేకపోతే {
				ఆపు;
			}
		}
		ఒకవేళ (అంకె_running == నిరంతరం_MAX || unmount_సూచీ >= అంకె_mounts) {
			state = STATE_REAP;
			ఆపు;
		}
		rp = running + అంకె_running++;
		rp->mntpath = mounts[unmount_సూచీ];
		rp->చివరి_exec_సమయం = tm_now();
		rp->try_లెక్క = 0;
		rp->pid = నిరంతరం_umount(rp->mntpath, umount_వాదనలు, umount_వాదనలు_అంకె);
		rp->fuser_pid = -1;
		rp->fuser_stdoutfd = -1;
		rp->fuser_exec_సమయం = -1;
		mounts[unmount_సూచీ] = mounts[--అంకె_mounts];
		ఆపు;
	స్థితి STATE_REAP:
		flags = (అంకె_నిరీక్షణ > 0) ? WNOHANG : 0;
		pid = waitpid(-1, &స్థితి_2, flags);
		rp = ఖాళీ;
		కొరకు (size_t i = 0; i < అంకె_running; ++i) {
			rp = running + i;
			ఒకవేళ (rp->fuser_pid == pid)
				rp->fuser_pid = -1;
			ఒకవేళ (rp->pid == pid && pid > 0)
				ఆపు;
			rp = ఖాళీ;
		}
		ఒకవేళ (rp) {
			ఒకవేళ ((WIFEXITED(స్థితి_2) && WEXITSTATUS(స్థితి_2) == 0) ||
			    !is_mounted(rp->mntpath)) {
				einfo("Unmounted %s", rp->mntpath);
				*rp = running[--అంకె_running];
				state = STATE_నిరంతరం;
			} లేకపోతే ఒకవేళ (rp->try_లెక్క >= TRY_MAX) {
				eerror("Failed to unmount %s", rp->mntpath);
				*rp = running[--అంకె_running];
				ఫలితం = EXIT_FAILURE;
			} లేకపోతే { /* put into waiting queue */
				rp->pid = -1;
				rp->try_లెక్క += 1;
				అంకె_నిరీక్షణ += 1;
				fuser_నిరంతరం(rp, fuser_opt);
			}
		} లేకపోతే {
			state = STATE_RETRY;
		}
		ఆపు;
	స్థితి STATE_RETRY:
		rp = ఖాళీ;
		తదుపరి_retry = INT64_MAX;
		కొరకు (size_t i = 0; i < అంకె_running; ++i) {
			ఒకవేళ (running[i].pid > 0)
				కొనసాగు;
			ఒకవేళ (running[i].fuser_pid > 0)
				tmp = running[i].fuser_exec_సమయం + rc_fuser_timeout;
			లేకపోతే
				tmp = running[i].చివరి_exec_సమయం + TRY_DELAY_MS;
			ఒకవేళ (tmp < తదుపరి_retry) {
				rp = running + i;
				తదుపరి_retry = tmp;
			}
		}
		ఒకవేళ (!rp) {
			state = (అంకె_mounts > 0) ? STATE_నిరంతరం : STATE_చివర;
			ఆపు;
		}
		now = tm_now();
		ఒకవేళ ((rp->pid > 0 || rp->fuser_pid > 0) && తదుపరి_retry > now) {
			int64_t sleep_for = తదుపరి_retry - now;
			/* a child may become available for reaping *before* we
			 * enter sleep. cap the timeout to stay responsive. */
			ఒకవేళ (sleep_for > 500)
				sleep_for = 500;
			tm_sleep(sleep_for, 0);
			state = STATE_REAP;
			now = tm_now();
		}
		ఒకవేళ ((rp->pid == -1 && rp->fuser_pid == -1) || తదుపరి_retry <= now) {
			ఒకవేళ (rp->fuser_pid > 0) {
				kill(rp->fuser_pid, SIGKILL);
				waitpid(rp->fuser_pid, ఖాళీ, 0);
				rp->fuser_pid = -1;
			}
			ఒకవేళ (fuser_decide(rp, fuser_opt, fuser_kill_ఉపసర్గ) < 0) { /* abort */
				*rp = running[--అంకె_running];
				ఫలితం = EXIT_FAILURE;
			} లేకపోతే { /* retry */
				rp->చివరి_exec_సమయం = tm_now();
				rp->pid = నిరంతరం_umount(rp->mntpath,
					umount_వాదనలు, umount_వాదనలు_అంకె);
			}
			అంకె_నిరీక్షణ -= 1;
			state = STATE_REAP;
		}
		ఆపు;
	అప్రమేయ: ఆపు;
	}

నిష్క్రమించు:
	విడుదల(mounts);
	rc_stringlist_free(nodes);
	REG_FREE(వాదనలు.fstype_regex);
	REG_FREE(వాదనలు.skip_fstype_regex);
	REG_FREE(వాదనలు.కణుపు_regex);
	REG_FREE(వాదనలు.skip_కణుపు_regex);
	REG_FREE(వాదనలు.options_regex);
	REG_FREE(వాదనలు.skip_options_regex);
	REG_FREE(బిందువు_regex);
	REG_FREE(skip_బిందువు_regex);

	తిరిగి ఫలితం;
}
