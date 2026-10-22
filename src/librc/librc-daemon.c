/*
 * librc-daemon
 * Finds PID for given daemon criteria
*/

/*
 * Copyright (c) 2007-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#అన్తర్భావ <errno.h>
#అన్తర్భావ <dirent.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "queue.h"
#అన్తర్భావ "librc.h"
#అన్తర్భావ "helpers.h"

#ఒకవేళ defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
స్థిర bool
pid_is_exec(pid_t pid, స్థిరరాశి అక్షరం *exec)
{
	అక్షరం *బఫరు = ఖాళీ;
	FILE *fp;
	పూర్ణాంకం c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&బఫరు, "/proc/%d/stat", pid);
	ఒకవేళ ((fp = fopen(బఫరు, "r"))) {
		అయితే ((c = getc(fp)) != EOF && c != '(')
			;
		ఒకవేళ (c == '(') {
			అయితే ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			ఒకవేళ (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	విడుదల(బఫరు);
	తిరిగి retval;
}

స్థిర bool
pid_is_argv(pid_t pid, స్థిరరాశి అక్షరం *స్థిరరాశి *argv)
{
	అక్షరం *cmdline = ఖాళీ;
	పూర్ణాంకం fd;
	అక్షరం బఫరు[PATH_MAX];
	అక్షరం *p;
	ssize_t బైట్లు;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	ఒకవేళ ((fd = open(cmdline, O_RDONLY)) < 0) {
		విడుదల(cmdline);
		తిరిగి false;
	}
	బైట్లు = పఠ(fd, బఫరు, పరిమాణం(బఫరు) - 1);
	close(fd);
	విడుదల(cmdline);
	ఒకవేళ (బైట్లు == -1)
		తిరిగి false;

	బఫరు[బైట్లు] = '\0';
	p = బఫరు;
	అయితే (*argv) {
		ఒకవేళ ((గుర్తులేని)(p - బఫరు) >= పరిమాణం(బఫరు))
			తిరిగి false;
		ఒకవేళ (strcmp(*argv, p) != 0)
			తిరిగి false;
		argv++;
		p += అక్షరశ్రేణి_పొడవు(p) + 1;
	}
	తిరిగి true;
}

RC_PIDLIST *
rc_కనుగొను_pids(స్థిరరాశి అక్షరం *exec, స్థిరరాశి అక్షరం *స్థిరరాశి *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	నిర్మాణం dirent *నమోదు_2;
	FILE *fp;
	పూర్ణాంకం rc;
	bool container_pid = false;
	bool openvz_host = false;
	అక్షరం *పంక్తి = ఖాళీ;
	అక్షరం my_ns[30] = {0};
	అక్షరం proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	అక్షరం *బఫరు = ఖాళీ;
	నిర్మాణం stat sb;
	pid_t openrc_pid = 0;
	అక్షరం *pp;
	RC_PIDLIST *pids = ఖాళీ;
	RC_PID *pi;

	ఒకవేళ ((procdir = opendir("/proc")) == ఖాళీ)
		తిరిగి ఖాళీ;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	ఒకవేళ ((pp = getenv("RC_OPENRC_PID"))) {
		ఒకవేళ (అక్షరశ్రేణి_చదువు(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	ఒకవేళ ((fp = fopen("/proc/self/status", "r"))) {
		అయితే (xgetline(&పంక్తి, &len, fp) != -1) {
			ఒకవేళ (అక్షరశ్రేణి_పోలిక(పంక్తి, "envID:\t0", 8) == 0) {
				openvz_host = true;
				ఆపు;
			}
		}
		fclose(fp);
	}

	ఒకవేళ ((rc = readlink("/proc/self/ns/pid", my_ns, పరిమాణం(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	అయితే ((నమోదు_2 = readdir(procdir)) != ఖాళీ) {
		ఒకవేళ (అక్షరశ్రేణి_చదువు(నమోదు_2->d_name, "%d", &p) != 1)
			కొనసాగు;
		ఒకవేళ (openrc_pid != 0 && openrc_pid == p)
			కొనసాగు;
		ఒకవేళ (pid != 0 && pid != p)
			కొనసాగు;
		xasprintf(&బఫరు, "/proc/%d/ns/pid", p);
		ఒకవేళ ((rc = readlink(బఫరు, proc_ns, పరిమాణం(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		విడుదల(బఫరు);
		ఒకవేళ (pid == 0 && అక్షరశ్రేణి_పొడవు(my_ns) && అక్షరశ్రేణి_పొడవు (proc_ns) && strcmp(my_ns, proc_ns))
			కొనసాగు;
		ఒకవేళ (uid) {
			xasprintf(&బఫరు, "/proc/%d", p);
			ఒకవేళ (stat(బఫరు, &sb) != 0 || sb.st_uid != uid) {
				విడుదల(బఫరు);
				కొనసాగు;
			}
			విడుదల(బఫరు);
		}
		ఒకవేళ (exec && !pid_is_exec(p, exec))
			కొనసాగు;
		ఒకవేళ (argv &&
		    !pid_is_argv(p, (స్థిరరాశి అక్షరం *స్థిరరాశి *)argv))
			కొనసాగు;
		/* If this is an OpenVZ host, filter out container processes */
		ఒకవేళ (openvz_host) {
			xasprintf(&బఫరు, "/proc/%d/status", p);
			fp = fopen(బఫరు, "r");
			విడుదల(బఫరు);
			ఒకవేళ (!fp)
				కొనసాగు;
			అయితే (xgetline(&పంక్తి, &len, fp) != -1) {
				ఒకవేళ (అక్షరశ్రేణి_పోలిక(పంక్తి, "envID:", 6) == 0) {
					container_pid = !(అక్షరశ్రేణి_పోలిక(పంక్తి, "envID:\t0", 8) == 0);
					ఆపు;
				}
			}
			fclose(fp);
		}
		ఒకవేళ (container_pid)
			కొనసాగు;
		ఒకవేళ (!pids) {
			pids = xmalloc(పరిమాణం(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(పరిమాణం(*pi));
		pi->pid = p;
		LIST_చొప్పించు_తల(pids, pi, entries);
	}
	ఒకవేళ (పంక్తి != ఖాళీ)
		విడుదల(పంక్తి);
	closedir(procdir);
	తిరిగి pids;
}

#లేదావేళ BSD

# ఒకవేళ defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define KVM_మార్గం ఖాళీ
#  define _KVM_FLAGS KVM_NO_FILES
# లేకపోతే
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  ఒకవేళ defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  లేకపోతే
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define KVM_మార్గం _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_కనుగొను_pids(స్థిరరాశి అక్షరం *exec, స్థిరరాశి అక్షరం *స్థిరరాశి *argv, uid_t uid, pid_t pid)
{
	స్థిర kvm_t *kd = ఖాళీ;
	అక్షరం errbuf[_POSIX2_LINE_MAX];
	నిర్మాణం _KINFO_PROC *kp;
	పూర్ణాంకం i;
	పూర్ణాంకం processes = 0;
	పూర్ణాంకం pargc = 0;
	అక్షరం **pargv;
	RC_PIDLIST *pids = ఖాళీ;
	RC_PID *pi;
	pid_t p;
	స్థిరరాశి అక్షరం *స్థిరరాశి *arg;
	పూర్ణాంకం సరిపోలిక;

	ఒకవేళ ((kd = kvm_openfiles(KVM_మార్గం, KVM_మార్గం,
		    ఖాళీ, _KVM_FLAGS, errbuf)) == ఖాళీ)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		తిరిగి ఖాళీ;
	}

#నిర్వచితమైతే _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, పరిమాణం(*kp), &processes);
#లేకపోతే
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#అయితే_ముగింపు
	ఒకవేళ ((kp == ఖాళీ && processes > 0) || (kp != ఖాళీ && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (exec)
		exec = basename_c(exec);
	కొరకు (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		ఒకవేళ (pid != 0 && pid != p)
			కొనసాగు;
		ఒకవేళ (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			కొనసాగు;
		ఒకవేళ (exec) {
			ఒకవేళ (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				కొనసాగు;
		}
		ఒకవేళ (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			ఒకవేళ (!pargv || !*pargv)
				కొనసాగు;
			arg = argv;
			సరిపోలిక = 1;
			అయితే (*arg && *pargv)
				ఒకవేళ (strcmp(*arg++, *pargv++) != 0) {
					సరిపోలిక = 0;
					ఆపు;
				}
			ఒకవేళ (!సరిపోలిక)
				కొనసాగు;
		}
		ఒకవేళ (!pids) {
			pids = xmalloc(పరిమాణం(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(పరిమాణం(*pi));
		pi->pid = p;
		LIST_చొప్పించు_తల(pids, pi, entries);
	}
	kvm_close(kd);

	తిరిగి pids;
}

#లేకపోతే
#  error "Platform not supported!"
#అయితే_ముగింపు

స్థిర bool
సరిపోలిక_daemon(స్థిరరాశి అక్షరం *svcname, స్థిరరాశి అక్షరం *instance, RC_STRINGLIST *సరిపోలిక)
{
	అక్షరం *పంక్తి = ఖాళీ;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	అక్షరం *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	విడుదల(daemon);

	ఒకవేళ (!fp)
		తిరిగి false;

	అయితే (xgetline(&పంక్తి, &len, fp) != -1) {
		TAILQ_FOREACH(m, సరిపోలిక, entries)
		    ఒకవేళ (strcmp(పంక్తి, m->విలువ) == 0) {
			    TAILQ_REMOVE(సరిపోలిక, m, entries);
			    ఆపు;
		    }
		ఒకవేళ (!TAILQ_మొదటి(సరిపోలిక))
			ఆపు;
	}
	fclose(fp);
	విడుదల(పంక్తి);
	ఒకవేళ (TAILQ_మొదటి(సరిపోలిక))
		తిరిగి false;
	తిరిగి true;
}

స్థిర RC_STRINGLIST *
సరిపోలిక_list(స్థిరరాశి అక్షరం *exec, స్థిరరాశి అక్షరం *స్థిరరాశి *argv, స్థిరరాశి అక్షరం *pidfile)
{
	RC_STRINGLIST *సరిపోలిక = rc_stringlist_new();
	పూర్ణాంకం i = 0;
	అక్షరం *m;

	ఒకవేళ (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_కూడు(సరిపోలిక, m);
		విడుదల(m);
	}

	అయితే (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_కూడు(సరిపోలిక, m);
		విడుదల(m);
	}

	ఒకవేళ (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_కూడు(సరిపోలిక, m);
		విడుదల(m);
	}

	తిరిగి సరిపోలిక;
}

bool
rc_service_daemon_సమితి(స్థిరరాశి అక్షరం *service, స్థిరరాశి అక్షరం *exec,
    స్థిరరాశి అక్షరం *స్థిరరాశి *argv,
    స్థిరరాశి అక్షరం *pidfile, bool started)
{
	అక్షరం *file = ఖాళీ;
	పూర్ణాంకం nfiles = 0;
	అక్షరం oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	నిర్మాణం dirent *d;
	RC_STRINGLIST *సరిపోలిక, *renamelist;
	పూర్ణాంకం i = 0;
	FILE *fp;
	స్థిరరాశి అక్షరం *పునాది = basename_c(service);

	ఒకవేళ (!exec && !pidfile) {
		errno = EINVAL;
		తిరిగి false;
	}

	/* Regardless, erase any existing daemon info */
	ఒకవేళ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), పునాది))) {
		సరిపోలిక = సరిపోలిక_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		అయితే ((d = readdir(dp))) {
			ఒకవేళ (d->d_name[0] == '.')
				కొనసాగు;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), పునాది, d->d_name);
			ఒకవేళ (rc_stringlist_కనుగొను(renamelist, file)) {
				విడుదల(file);
				కొనసాగు;
			}

			nfiles++;

			ఒకవేళ (!*oldfile) {
				ఒకవేళ (సరిపోలిక_daemon(పునాది, d->d_name, సరిపోలిక)) {
					unlink(file);
					strlcpy(oldfile, file, పరిమాణం(oldfile));
					nfiles--;
				}
			} లేకపోతే {
				rename(file, oldfile);
				strlcpy(oldfile, file, పరిమాణం(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_కూడు(renamelist, oldfile);
			}
			విడుదల(file);
		}
		closedir(dp);
		rc_stringlist_free(సరిపోలిక);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	ఒకవేళ (!started)
		తిరిగి true;

	ఒకవేళ (mkdirat(rc_dirfd(RC_DIR_DAEMONS), పునాది, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), పునాది, nfiles + 1);
		ఒకవేళ ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			ఒకవేళ (exec)
				fprintf(fp, "%s", exec);
			అయితే (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			ఒకవేళ (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		విడుదల(file);
	}

	తిరిగి retval;
}

bool
rc_service_started_daemon(స్థిరరాశి అక్షరం *service,
    స్థిరరాశి అక్షరం *exec, స్థిరరాశి అక్షరం *స్థిరరాశి *argv, పూర్ణాంకం indx)
{
	స్థిరరాశి అక్షరం *పునాది = basename_c(service);
	అక్షరం *file = ఖాళీ;
	RC_STRINGLIST *సరిపోలిక;
	bool retval = false;
	DIR *dp;
	నిర్మాణం dirent *d;

	ఒకవేళ (!service || !exec)
		తిరిగి false;

	సరిపోలిక = సరిపోలిక_list(exec, argv, ఖాళీ);

	ఒకవేళ (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = సరిపోలిక_daemon(పునాది, file, సరిపోలిక);
		విడుదల(file);
	} లేకపోతే ఒకవేళ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), పునాది))) {
		అయితే ((d = readdir(dp))) {
			ఒకవేళ (d->d_name[0] == '.')
				కొనసాగు;
			ఒకవేళ ((retval = సరిపోలిక_daemon(పునాది, d->d_name, సరిపోలిక)))
				ఆపు;
		}
		closedir(dp);
	}

	rc_stringlist_free(సరిపోలిక);
	తిరిగి retval;
}

bool
rc_service_daemons_crashed(స్థిరరాశి అక్షరం *service)
{
	DIR *dp;
	నిర్మాణం dirent *d;
	FILE *fp;
	అక్షరం *పంక్తి = ఖాళీ;
	size_t len = 0;
	అక్షరం **argv = ఖాళీ;
	అక్షరం *exec = ఖాళీ;
	అక్షరం *name = ఖాళీ;
	అక్షరం *pidfile = ఖాళీ;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	అక్షరం *p;
	అక్షరం *token;
	bool retval = false;
	RC_STRINGLIST *list = ఖాళీ;
	RC_STRING *s;
	size_t i;
	అక్షరం *ch_మూలం;
	అక్షరం *spidfile;

	ఒకవేళ (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		తిరిగి false;

	అయితే ((d = readdir(dp))) {
		ఒకవేళ (d->d_name[0] == '.')
			కొనసాగు;

		ఒకవేళ (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			ఆపు;

		అయితే (xgetline(&పంక్తి, &len, fp) != -1) {
			p = పంక్తి;
			ఒకవేళ ((token = strsep(&p, "=")) == ఖాళీ || !p)
				కొనసాగు;

			ఒకవేళ (!*p)
				కొనసాగు;

			ఒకవేళ (strcmp(token, "exec") == 0) {
				ఒకవేళ (exec)
					విడుదల(exec);
				exec = xstrdup(p);
			} లేకపోతే ఒకవేళ (అక్షరశ్రేణి_పోలిక(token, "argv_", 5) == 0) {
				ఒకవేళ (!list)
					list = rc_stringlist_new();
				rc_stringlist_కూడు(list, p);
			} లేకపోతే ఒకవేళ (strcmp(token, "name") == 0) {
				ఒకవేళ (name)
					విడుదల(name);
				name = xstrdup(p);
			} లేకపోతే ఒకవేళ (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				ఆపు;
			}
		}
		fclose(fp);

		ch_మూలం = rc_service_విలువ_get(basename_c(service), "chroot");
		spidfile = pidfile;
		ఒకవేళ (ch_మూలం && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_మూలం, pidfile);
			విడుదల(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		ఒకవేళ (pidfile) {
			retval = true;
			ఒకవేళ ((fp = fopen(pidfile, "r"))) {
				ఒకవేళ (fscanf(fp, "%d", &pid) == 1)
					retval = false;
				fclose(fp);
			}
			విడుదల(pidfile);
			pidfile = ఖాళీ;

			/* We have the pid, so no need to match
			   on exec or name */
			విడుదల(exec);
			exec = ఖాళీ;
			విడుదల(name);
			name = ఖాళీ;
		} లేకపోతే {
			ఒకవేళ (exec) {
				ఒకవేళ (!list)
					list = rc_stringlist_new();
				ఒకవేళ (!TAILQ_మొదటి(list))
					rc_stringlist_కూడు(list, exec);

				విడుదల(exec);
				exec = ఖాళీ;
			}

			ఒకవేళ (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(పరిమాణం(అక్షరం *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->విలువ;
				argv[i] = ఖాళీ;
			}
		}

		ఒకవేళ (!retval) {
			ఒకవేళ (pid != 0) {
				ఒకవేళ (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} లేకపోతే ఒకవేళ ((pids = rc_కనుగొను_pids(exec,
				    (స్థిరరాశి అక్షరం *స్థిరరాశి *)argv,
				    0, pid)))
			{
				p1 = LIST_మొదటి(pids);
				అయితే (p1) {
					p2 = LIST_తదుపరి(p1, entries);
					విడుదల(p1);
					p1 = p2;
				}
				విడుదల(pids);
			} లేకపోతే
				retval = true;
		}
		rc_stringlist_free(list);
		list = ఖాళీ;
		విడుదల(argv);
		argv = ఖాళీ;
		విడుదల(exec);
		exec = ఖాళీ;
		విడుదల(name);
		name = ఖాళీ;
		ఒకవేళ (retval)
			ఆపు;
	}
	closedir(dp);
	విడుదల(పంక్తి);

	తిరిగి retval;
}
