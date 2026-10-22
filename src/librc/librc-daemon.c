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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dirent.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "librc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

#ਜੇ defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
ਸਥਿਰ bool
pid_is_exec(pid_t pid, ਅਚਲ ਅੱਖਰ *exec)
{
	ਅੱਖਰ *ਬਫਰ = ਨਿਰਾ;
	FILE *fp;
	ਪੂਰਨ c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&ਬਫਰ, "/proc/%d/stat", pid);
	ਜੇ ((fp = fopen(ਬਫਰ, "r"))) {
		ਜਦੋਂ ((c = getc(fp)) != EOF && c != '(')
			;
		ਜੇ (c == '(') {
			ਜਦੋਂ ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			ਜੇ (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	ਮੁਕਤ(ਬਫਰ);
	ਵਾਪਸ retval;
}

ਸਥਿਰ bool
pid_is_argv(pid_t pid, ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv)
{
	ਅੱਖਰ *cmdline = ਨਿਰਾ;
	ਪੂਰਨ fd;
	ਅੱਖਰ ਬਫਰ[PATH_MAX];
	ਅੱਖਰ *p;
	ssize_t ਬਾਈਟਾਂ;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	ਜੇ ((fd = open(cmdline, O_RDONLY)) < 0) {
		ਮੁਕਤ(cmdline);
		ਵਾਪਸ false;
	}
	ਬਾਈਟਾਂ = ਪਠ(fd, ਬਫਰ, ਆਕਾਰ(ਬਫਰ) - 1);
	close(fd);
	ਮੁਕਤ(cmdline);
	ਜੇ (ਬਾਈਟਾਂ == -1)
		ਵਾਪਸ false;

	ਬਫਰ[ਬਾਈਟਾਂ] = '\0';
	p = ਬਫਰ;
	ਜਦੋਂ (*argv) {
		ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(p - ਬਫਰ) >= ਆਕਾਰ(ਬਫਰ))
			ਵਾਪਸ false;
		ਜੇ (strcmp(*argv, p) != 0)
			ਵਾਪਸ false;
		argv++;
		p += ਲੜੀ_ਲੰਬਾਈ(p) + 1;
	}
	ਵਾਪਸ true;
}

RC_PIDLIST *
rc_ਲੱਭੋ_pids(ਅਚਲ ਅੱਖਰ *exec, ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	ਬਣਤਰ dirent *ਇੰਦਰਾਜ;
	FILE *fp;
	ਪੂਰਨ rc;
	bool container_pid = false;
	bool openvz_host = false;
	ਅੱਖਰ *ਸਤਰ = ਨਿਰਾ;
	ਅੱਖਰ my_ns[30] = {0};
	ਅੱਖਰ proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	ਅੱਖਰ *ਬਫਰ = ਨਿਰਾ;
	ਬਣਤਰ stat sb;
	pid_t openrc_pid = 0;
	ਅੱਖਰ *pp;
	RC_PIDLIST *pids = ਨਿਰਾ;
	RC_PID *pi;

	ਜੇ ((procdir = opendir("/proc")) == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	ਜੇ ((pp = getenv("RC_OPENRC_PID"))) {
		ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	ਜੇ ((fp = fopen("/proc/self/status", "r"))) {
		ਜਦੋਂ (xgetline(&ਸਤਰ, &len, fp) != -1) {
			ਜੇ (ਲੜੀ_ਤੁਲਨਾ(ਸਤਰ, "envID:\t0", 8) == 0) {
				openvz_host = true;
				ਰੋਕੋ;
			}
		}
		fclose(fp);
	}

	ਜੇ ((rc = readlink("/proc/self/ns/pid", my_ns, ਆਕਾਰ(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	ਜਦੋਂ ((ਇੰਦਰਾਜ = readdir(procdir)) != ਨਿਰਾ) {
		ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(ਇੰਦਰਾਜ->d_name, "%d", &p) != 1)
			ਜਾਰੀ;
		ਜੇ (openrc_pid != 0 && openrc_pid == p)
			ਜਾਰੀ;
		ਜੇ (pid != 0 && pid != p)
			ਜਾਰੀ;
		xasprintf(&ਬਫਰ, "/proc/%d/ns/pid", p);
		ਜੇ ((rc = readlink(ਬਫਰ, proc_ns, ਆਕਾਰ(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		ਮੁਕਤ(ਬਫਰ);
		ਜੇ (pid == 0 && ਲੜੀ_ਲੰਬਾਈ(my_ns) && ਲੜੀ_ਲੰਬਾਈ (proc_ns) && strcmp(my_ns, proc_ns))
			ਜਾਰੀ;
		ਜੇ (uid) {
			xasprintf(&ਬਫਰ, "/proc/%d", p);
			ਜੇ (stat(ਬਫਰ, &sb) != 0 || sb.st_uid != uid) {
				ਮੁਕਤ(ਬਫਰ);
				ਜਾਰੀ;
			}
			ਮੁਕਤ(ਬਫਰ);
		}
		ਜੇ (exec && !pid_is_exec(p, exec))
			ਜਾਰੀ;
		ਜੇ (argv &&
		    !pid_is_argv(p, (ਅਚਲ ਅੱਖਰ *ਅਚਲ *)argv))
			ਜਾਰੀ;
		/* If this is an OpenVZ host, filter out container processes */
		ਜੇ (openvz_host) {
			xasprintf(&ਬਫਰ, "/proc/%d/status", p);
			fp = fopen(ਬਫਰ, "r");
			ਮੁਕਤ(ਬਫਰ);
			ਜੇ (!fp)
				ਜਾਰੀ;
			ਜਦੋਂ (xgetline(&ਸਤਰ, &len, fp) != -1) {
				ਜੇ (ਲੜੀ_ਤੁਲਨਾ(ਸਤਰ, "envID:", 6) == 0) {
					container_pid = !(ਲੜੀ_ਤੁਲਨਾ(ਸਤਰ, "envID:\t0", 8) == 0);
					ਰੋਕੋ;
				}
			}
			fclose(fp);
		}
		ਜੇ (container_pid)
			ਜਾਰੀ;
		ਜੇ (!pids) {
			pids = xmalloc(ਆਕਾਰ(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(ਆਕਾਰ(*pi));
		pi->pid = p;
		LIST_ਪਾਓ_ਸਿਰ(pids, pi, entries);
	}
	ਜੇ (ਸਤਰ != ਨਿਰਾ)
		ਮੁਕਤ(ਸਤਰ);
	closedir(procdir);
	ਵਾਪਸ pids;
}

#ਜਾਂਜੇ BSD

# ਜੇ defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define KVM_ਰਸਤਾ ਨਿਰਾ
#  define _KVM_FLAGS KVM_NO_FILES
# ਨਹੀਂਤਾਂ
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  ਜੇ defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  ਨਹੀਂਤਾਂ
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define KVM_ਰਸਤਾ _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_ਲੱਭੋ_pids(ਅਚਲ ਅੱਖਰ *exec, ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv, uid_t uid, pid_t pid)
{
	ਸਥਿਰ kvm_t *kd = ਨਿਰਾ;
	ਅੱਖਰ errbuf[_POSIX2_LINE_MAX];
	ਬਣਤਰ _KINFO_PROC *kp;
	ਪੂਰਨ i;
	ਪੂਰਨ processes = 0;
	ਪੂਰਨ pargc = 0;
	ਅੱਖਰ **pargv;
	RC_PIDLIST *pids = ਨਿਰਾ;
	RC_PID *pi;
	pid_t p;
	ਅਚਲ ਅੱਖਰ *ਅਚਲ *arg;
	ਪੂਰਨ ਮੇਲ;

	ਜੇ ((kd = kvm_openfiles(KVM_ਰਸਤਾ, KVM_ਰਸਤਾ,
		    ਨਿਰਾ, _KVM_FLAGS, errbuf)) == ਨਿਰਾ)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		ਵਾਪਸ ਨਿਰਾ;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, ਆਕਾਰ(*kp), &processes);
#ਨਹੀਂਤਾਂ
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#ਜੇ_ਅੰਤ
	ਜੇ ((kp == ਨਿਰਾ && processes > 0) || (kp != ਨਿਰਾ && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (exec)
		exec = basename_c(exec);
	ਲਈ (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		ਜੇ (pid != 0 && pid != p)
			ਜਾਰੀ;
		ਜੇ (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			ਜਾਰੀ;
		ਜੇ (exec) {
			ਜੇ (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				ਜਾਰੀ;
		}
		ਜੇ (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			ਜੇ (!pargv || !*pargv)
				ਜਾਰੀ;
			arg = argv;
			ਮੇਲ = 1;
			ਜਦੋਂ (*arg && *pargv)
				ਜੇ (strcmp(*arg++, *pargv++) != 0) {
					ਮੇਲ = 0;
					ਰੋਕੋ;
				}
			ਜੇ (!ਮੇਲ)
				ਜਾਰੀ;
		}
		ਜੇ (!pids) {
			pids = xmalloc(ਆਕਾਰ(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(ਆਕਾਰ(*pi));
		pi->pid = p;
		LIST_ਪਾਓ_ਸਿਰ(pids, pi, entries);
	}
	kvm_close(kd);

	ਵਾਪਸ pids;
}

#ਨਹੀਂਤਾਂ
#  error "Platform not supported!"
#ਜੇ_ਅੰਤ

ਸਥਿਰ bool
ਮੇਲ_daemon(ਅਚਲ ਅੱਖਰ *svcname, ਅਚਲ ਅੱਖਰ *instance, RC_STRINGLIST *ਮੇਲ)
{
	ਅੱਖਰ *ਸਤਰ = ਨਿਰਾ;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	ਅੱਖਰ *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	ਮੁਕਤ(daemon);

	ਜੇ (!fp)
		ਵਾਪਸ false;

	ਜਦੋਂ (xgetline(&ਸਤਰ, &len, fp) != -1) {
		TAILQ_FOREACH(m, ਮੇਲ, entries)
		    ਜੇ (strcmp(ਸਤਰ, m->ਮੁੱਲ) == 0) {
			    TAILQ_REMOVE(ਮੇਲ, m, entries);
			    ਰੋਕੋ;
		    }
		ਜੇ (!TAILQ_ਪਹਿਲਾ(ਮੇਲ))
			ਰੋਕੋ;
	}
	fclose(fp);
	ਮੁਕਤ(ਸਤਰ);
	ਜੇ (TAILQ_ਪਹਿਲਾ(ਮੇਲ))
		ਵਾਪਸ false;
	ਵਾਪਸ true;
}

ਸਥਿਰ RC_STRINGLIST *
ਮੇਲ_list(ਅਚਲ ਅੱਖਰ *exec, ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv, ਅਚਲ ਅੱਖਰ *pidfile)
{
	RC_STRINGLIST *ਮੇਲ = rc_stringlist_new();
	ਪੂਰਨ i = 0;
	ਅੱਖਰ *m;

	ਜੇ (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_ਜੋੜੋ(ਮੇਲ, m);
		ਮੁਕਤ(m);
	}

	ਜਦੋਂ (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_ਜੋੜੋ(ਮੇਲ, m);
		ਮੁਕਤ(m);
	}

	ਜੇ (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_ਜੋੜੋ(ਮੇਲ, m);
		ਮੁਕਤ(m);
	}

	ਵਾਪਸ ਮੇਲ;
}

bool
rc_service_daemon_ਸੈੱਟ(ਅਚਲ ਅੱਖਰ *service, ਅਚਲ ਅੱਖਰ *exec,
    ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv,
    ਅਚਲ ਅੱਖਰ *pidfile, bool started)
{
	ਅੱਖਰ *file = ਨਿਰਾ;
	ਪੂਰਨ nfiles = 0;
	ਅੱਖਰ oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	ਬਣਤਰ dirent *d;
	RC_STRINGLIST *ਮੇਲ, *renamelist;
	ਪੂਰਨ i = 0;
	FILE *fp;
	ਅਚਲ ਅੱਖਰ *ਅਧਾਰ = basename_c(service);

	ਜੇ (!exec && !pidfile) {
		errno = EINVAL;
		ਵਾਪਸ false;
	}

	/* Regardless, erase any existing daemon info */
	ਜੇ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), ਅਧਾਰ))) {
		ਮੇਲ = ਮੇਲ_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		ਜਦੋਂ ((d = readdir(dp))) {
			ਜੇ (d->d_name[0] == '.')
				ਜਾਰੀ;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), ਅਧਾਰ, d->d_name);
			ਜੇ (rc_stringlist_ਲੱਭੋ(renamelist, file)) {
				ਮੁਕਤ(file);
				ਜਾਰੀ;
			}

			nfiles++;

			ਜੇ (!*oldfile) {
				ਜੇ (ਮੇਲ_daemon(ਅਧਾਰ, d->d_name, ਮੇਲ)) {
					unlink(file);
					strlcpy(oldfile, file, ਆਕਾਰ(oldfile));
					nfiles--;
				}
			} ਨਹੀਂਤਾਂ {
				rename(file, oldfile);
				strlcpy(oldfile, file, ਆਕਾਰ(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_ਜੋੜੋ(renamelist, oldfile);
			}
			ਮੁਕਤ(file);
		}
		closedir(dp);
		rc_stringlist_free(ਮੇਲ);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	ਜੇ (!started)
		ਵਾਪਸ true;

	ਜੇ (mkdirat(rc_dirfd(RC_DIR_DAEMONS), ਅਧਾਰ, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), ਅਧਾਰ, nfiles + 1);
		ਜੇ ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			ਜੇ (exec)
				fprintf(fp, "%s", exec);
			ਜਦੋਂ (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			ਜੇ (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		ਮੁਕਤ(file);
	}

	ਵਾਪਸ retval;
}

bool
rc_service_started_daemon(ਅਚਲ ਅੱਖਰ *service,
    ਅਚਲ ਅੱਖਰ *exec, ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv, ਪੂਰਨ indx)
{
	ਅਚਲ ਅੱਖਰ *ਅਧਾਰ = basename_c(service);
	ਅੱਖਰ *file = ਨਿਰਾ;
	RC_STRINGLIST *ਮੇਲ;
	bool retval = false;
	DIR *dp;
	ਬਣਤਰ dirent *d;

	ਜੇ (!service || !exec)
		ਵਾਪਸ false;

	ਮੇਲ = ਮੇਲ_list(exec, argv, ਨਿਰਾ);

	ਜੇ (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = ਮੇਲ_daemon(ਅਧਾਰ, file, ਮੇਲ);
		ਮੁਕਤ(file);
	} ਨਹੀਂਤਾਂ ਜੇ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), ਅਧਾਰ))) {
		ਜਦੋਂ ((d = readdir(dp))) {
			ਜੇ (d->d_name[0] == '.')
				ਜਾਰੀ;
			ਜੇ ((retval = ਮੇਲ_daemon(ਅਧਾਰ, d->d_name, ਮੇਲ)))
				ਰੋਕੋ;
		}
		closedir(dp);
	}

	rc_stringlist_free(ਮੇਲ);
	ਵਾਪਸ retval;
}

bool
rc_service_daemons_crashed(ਅਚਲ ਅੱਖਰ *service)
{
	DIR *dp;
	ਬਣਤਰ dirent *d;
	FILE *fp;
	ਅੱਖਰ *ਸਤਰ = ਨਿਰਾ;
	size_t len = 0;
	ਅੱਖਰ **argv = ਨਿਰਾ;
	ਅੱਖਰ *exec = ਨਿਰਾ;
	ਅੱਖਰ *name = ਨਿਰਾ;
	ਅੱਖਰ *pidfile = ਨਿਰਾ;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	ਅੱਖਰ *p;
	ਅੱਖਰ *token;
	bool retval = false;
	RC_STRINGLIST *list = ਨਿਰਾ;
	RC_STRING *s;
	size_t i;
	ਅੱਖਰ *ch_ਜੜ੍ਹ;
	ਅੱਖਰ *spidfile;

	ਜੇ (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		ਵਾਪਸ false;

	ਜਦੋਂ ((d = readdir(dp))) {
		ਜੇ (d->d_name[0] == '.')
			ਜਾਰੀ;

		ਜੇ (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			ਰੋਕੋ;

		ਜਦੋਂ (xgetline(&ਸਤਰ, &len, fp) != -1) {
			p = ਸਤਰ;
			ਜੇ ((token = strsep(&p, "=")) == ਨਿਰਾ || !p)
				ਜਾਰੀ;

			ਜੇ (!*p)
				ਜਾਰੀ;

			ਜੇ (strcmp(token, "exec") == 0) {
				ਜੇ (exec)
					ਮੁਕਤ(exec);
				exec = xstrdup(p);
			} ਨਹੀਂਤਾਂ ਜੇ (ਲੜੀ_ਤੁਲਨਾ(token, "argv_", 5) == 0) {
				ਜੇ (!list)
					list = rc_stringlist_new();
				rc_stringlist_ਜੋੜੋ(list, p);
			} ਨਹੀਂਤਾਂ ਜੇ (strcmp(token, "name") == 0) {
				ਜੇ (name)
					ਮੁਕਤ(name);
				name = xstrdup(p);
			} ਨਹੀਂਤਾਂ ਜੇ (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				ਰੋਕੋ;
			}
		}
		fclose(fp);

		ch_ਜੜ੍ਹ = rc_service_ਮੁੱਲ_get(basename_c(service), "chroot");
		spidfile = pidfile;
		ਜੇ (ch_ਜੜ੍ਹ && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_ਜੜ੍ਹ, pidfile);
			ਮੁਕਤ(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		ਜੇ (pidfile) {
			retval = true;
			ਜੇ ((fp = fopen(pidfile, "r"))) {
				ਜੇ (fscanf(fp, "%d", &pid) == 1)
					retval = false;
				fclose(fp);
			}
			ਮੁਕਤ(pidfile);
			pidfile = ਨਿਰਾ;

			/* We have the pid, so no need to match
			   on exec or name */
			ਮੁਕਤ(exec);
			exec = ਨਿਰਾ;
			ਮੁਕਤ(name);
			name = ਨਿਰਾ;
		} ਨਹੀਂਤਾਂ {
			ਜੇ (exec) {
				ਜੇ (!list)
					list = rc_stringlist_new();
				ਜੇ (!TAILQ_ਪਹਿਲਾ(list))
					rc_stringlist_ਜੋੜੋ(list, exec);

				ਮੁਕਤ(exec);
				exec = ਨਿਰਾ;
			}

			ਜੇ (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(ਆਕਾਰ(ਅੱਖਰ *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->ਮੁੱਲ;
				argv[i] = ਨਿਰਾ;
			}
		}

		ਜੇ (!retval) {
			ਜੇ (pid != 0) {
				ਜੇ (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} ਨਹੀਂਤਾਂ ਜੇ ((pids = rc_ਲੱਭੋ_pids(exec,
				    (ਅਚਲ ਅੱਖਰ *ਅਚਲ *)argv,
				    0, pid)))
			{
				p1 = LIST_ਪਹਿਲਾ(pids);
				ਜਦੋਂ (p1) {
					p2 = LIST_ਅਗਲਾ(p1, entries);
					ਮੁਕਤ(p1);
					p1 = p2;
				}
				ਮੁਕਤ(pids);
			} ਨਹੀਂਤਾਂ
				retval = true;
		}
		rc_stringlist_free(list);
		list = ਨਿਰਾ;
		ਮੁਕਤ(argv);
		argv = ਨਿਰਾ;
		ਮੁਕਤ(exec);
		exec = ਨਿਰਾ;
		ਮੁਕਤ(name);
		name = ਨਿਰਾ;
		ਜੇ (retval)
			ਰੋਕੋ;
	}
	closedir(dp);
	ਮੁਕਤ(ਸਤਰ);

	ਵਾਪਸ retval;
}
