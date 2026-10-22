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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <dirent.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "librc.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

#ಆದರೆ defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
ಸ್ಥಿರ bool
pid_is_exec(pid_t pid, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *exec)
{
	ಅಕ್ಷರ *ಬಫರ್ = ಖಾಲಿ;
	FILE *fp;
	ಪೂರ್ಣಾಂಕ c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&ಬಫರ್, "/proc/%d/stat", pid);
	ಆದರೆ ((fp = fopen(ಬಫರ್, "r"))) {
		ಆಗ ((c = getc(fp)) != EOF && c != '(')
			;
		ಆದರೆ (c == '(') {
			ಆಗ ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			ಆದರೆ (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	ಬಿಡುಗಡೆ(ಬಫರ್);
	ಹಿಂತಿರುಗಿ retval;
}

ಸ್ಥಿರ bool
pid_is_argv(pid_t pid, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv)
{
	ಅಕ್ಷರ *cmdline = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ fd;
	ಅಕ್ಷರ ಬಫರ್[PATH_MAX];
	ಅಕ್ಷರ *p;
	ssize_t ಬೈಟುಗಳು;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	ಆದರೆ ((fd = open(cmdline, O_RDONLY)) < 0) {
		ಬಿಡುಗಡೆ(cmdline);
		ಹಿಂತಿರುಗಿ false;
	}
	ಬೈಟುಗಳು = ಪಠ(fd, ಬಫರ್, ಗಾತ್ರ(ಬಫರ್) - 1);
	close(fd);
	ಬಿಡುಗಡೆ(cmdline);
	ಆದರೆ (ಬೈಟುಗಳು == -1)
		ಹಿಂತಿರುಗಿ false;

	ಬಫರ್[ಬೈಟುಗಳು] = '\0';
	p = ಬಫರ್;
	ಆಗ (*argv) {
		ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)(p - ಬಫರ್) >= ಗಾತ್ರ(ಬಫರ್))
			ಹಿಂತಿರುಗಿ false;
		ಆದರೆ (strcmp(*argv, p) != 0)
			ಹಿಂತಿರುಗಿ false;
		argv++;
		p += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p) + 1;
	}
	ಹಿಂತಿರುಗಿ true;
}

RC_PIDLIST *
rc_ಕಂಡುಹಿಡಿ_pids(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *exec, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	ರಚನೆ dirent *ನಮೂದು;
	FILE *fp;
	ಪೂರ್ಣಾಂಕ rc;
	bool container_pid = false;
	bool openvz_host = false;
	ಅಕ್ಷರ *ಸಾಲು = ಖಾಲಿ;
	ಅಕ್ಷರ my_ns[30] = {0};
	ಅಕ್ಷರ proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	ಅಕ್ಷರ *ಬಫರ್ = ಖಾಲಿ;
	ರಚನೆ stat sb;
	pid_t openrc_pid = 0;
	ಅಕ್ಷರ *pp;
	RC_PIDLIST *pids = ಖಾಲಿ;
	RC_PID *pi;

	ಆದರೆ ((procdir = opendir("/proc")) == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	ಆದರೆ ((pp = getenv("RC_OPENRC_PID"))) {
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	ಆದರೆ ((fp = fopen("/proc/self/status", "r"))) {
		ಆಗ (xgetline(&ಸಾಲು, &len, fp) != -1) {
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ಸಾಲು, "envID:\t0", 8) == 0) {
				openvz_host = true;
				ನಿಲ್ಲಿಸು;
			}
		}
		fclose(fp);
	}

	ಆದರೆ ((rc = readlink("/proc/self/ns/pid", my_ns, ಗಾತ್ರ(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	ಆಗ ((ನಮೂದು = readdir(procdir)) != ಖಾಲಿ) {
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(ನಮೂದು->d_name, "%d", &p) != 1)
			ಮುಂದುವರಿ;
		ಆದರೆ (openrc_pid != 0 && openrc_pid == p)
			ಮುಂದುವರಿ;
		ಆದರೆ (pid != 0 && pid != p)
			ಮುಂದುವರಿ;
		xasprintf(&ಬಫರ್, "/proc/%d/ns/pid", p);
		ಆದರೆ ((rc = readlink(ಬಫರ್, proc_ns, ಗಾತ್ರ(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		ಬಿಡುಗಡೆ(ಬಫರ್);
		ಆದರೆ (pid == 0 && ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(my_ns) && ಅಕ್ಷರಸರಣಿ_ಉದ್ದ (proc_ns) && strcmp(my_ns, proc_ns))
			ಮುಂದುವರಿ;
		ಆದರೆ (uid) {
			xasprintf(&ಬಫರ್, "/proc/%d", p);
			ಆದರೆ (stat(ಬಫರ್, &sb) != 0 || sb.st_uid != uid) {
				ಬಿಡುಗಡೆ(ಬಫರ್);
				ಮುಂದುವರಿ;
			}
			ಬಿಡುಗಡೆ(ಬಫರ್);
		}
		ಆದರೆ (exec && !pid_is_exec(p, exec))
			ಮುಂದುವರಿ;
		ಆದರೆ (argv &&
		    !pid_is_argv(p, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *)argv))
			ಮುಂದುವರಿ;
		/* If this is an OpenVZ host, filter out container processes */
		ಆದರೆ (openvz_host) {
			xasprintf(&ಬಫರ್, "/proc/%d/status", p);
			fp = fopen(ಬಫರ್, "r");
			ಬಿಡುಗಡೆ(ಬಫರ್);
			ಆದರೆ (!fp)
				ಮುಂದುವರಿ;
			ಆಗ (xgetline(&ಸಾಲು, &len, fp) != -1) {
				ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ಸಾಲು, "envID:", 6) == 0) {
					container_pid = !(ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ಸಾಲು, "envID:\t0", 8) == 0);
					ನಿಲ್ಲಿಸು;
				}
			}
			fclose(fp);
		}
		ಆದರೆ (container_pid)
			ಮುಂದುವರಿ;
		ಆದರೆ (!pids) {
			pids = xmalloc(ಗಾತ್ರ(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(ಗಾತ್ರ(*pi));
		pi->pid = p;
		LIST_ಸೇರಿಸು_ತಲೆ(pids, pi, entries);
	}
	ಆದರೆ (ಸಾಲು != ಖಾಲಿ)
		ಬಿಡುಗಡೆ(ಸಾಲು);
	closedir(procdir);
	ಹಿಂತಿರುಗಿ pids;
}

#ಅಥವಾಆದರೆ BSD

# ಆದರೆ defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define KVM_ಮಾರ್ಗ ಖಾಲಿ
#  define _KVM_FLAGS KVM_NO_FILES
# ಇಲ್ಲದಿದ್ದರೆ
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  ಆದರೆ defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  ಇಲ್ಲದಿದ್ದರೆ
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define KVM_ಮಾರ್ಗ _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_ಕಂಡುಹಿಡಿ_pids(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *exec, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv, uid_t uid, pid_t pid)
{
	ಸ್ಥಿರ kvm_t *kd = ಖಾಲಿ;
	ಅಕ್ಷರ errbuf[_POSIX2_LINE_MAX];
	ರಚನೆ _KINFO_PROC *kp;
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ processes = 0;
	ಪೂರ್ಣಾಂಕ pargc = 0;
	ಅಕ್ಷರ **pargv;
	RC_PIDLIST *pids = ಖಾಲಿ;
	RC_PID *pi;
	pid_t p;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *arg;
	ಪೂರ್ಣಾಂಕ ಹೊಂದಿಕೆ;

	ಆದರೆ ((kd = kvm_openfiles(KVM_ಮಾರ್ಗ, KVM_ಮಾರ್ಗ,
		    ಖಾಲಿ, _KVM_FLAGS, errbuf)) == ಖಾಲಿ)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, ಗಾತ್ರ(*kp), &processes);
#ಇಲ್ಲದಿದ್ದರೆ
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ ((kp == ಖಾಲಿ && processes > 0) || (kp != ಖಾಲಿ && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (exec)
		exec = basename_c(exec);
	ಗಾಗಿ (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		ಆದರೆ (pid != 0 && pid != p)
			ಮುಂದುವರಿ;
		ಆದರೆ (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			ಮುಂದುವರಿ;
		ಆದರೆ (exec) {
			ಆದರೆ (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				ಮುಂದುವರಿ;
		}
		ಆದರೆ (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			ಆದರೆ (!pargv || !*pargv)
				ಮುಂದುವರಿ;
			arg = argv;
			ಹೊಂದಿಕೆ = 1;
			ಆಗ (*arg && *pargv)
				ಆದರೆ (strcmp(*arg++, *pargv++) != 0) {
					ಹೊಂದಿಕೆ = 0;
					ನಿಲ್ಲಿಸು;
				}
			ಆದರೆ (!ಹೊಂದಿಕೆ)
				ಮುಂದುವರಿ;
		}
		ಆದರೆ (!pids) {
			pids = xmalloc(ಗಾತ್ರ(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(ಗಾತ್ರ(*pi));
		pi->pid = p;
		LIST_ಸೇರಿಸು_ತಲೆ(pids, pi, entries);
	}
	kvm_close(kd);

	ಹಿಂತಿರುಗಿ pids;
}

#ಇಲ್ಲದಿದ್ದರೆ
#  error "Platform not supported!"
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ bool
ಹೊಂದಿಕೆ_daemon(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *svcname, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *instance, RC_STRINGLIST *ಹೊಂದಿಕೆ)
{
	ಅಕ್ಷರ *ಸಾಲು = ಖಾಲಿ;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	ಅಕ್ಷರ *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	ಬಿಡುಗಡೆ(daemon);

	ಆದರೆ (!fp)
		ಹಿಂತಿರುಗಿ false;

	ಆಗ (xgetline(&ಸಾಲು, &len, fp) != -1) {
		TAILQ_FOREACH(m, ಹೊಂದಿಕೆ, entries)
		    ಆದರೆ (strcmp(ಸಾಲು, m->ಮೌಲ್ಯ) == 0) {
			    TAILQ_REMOVE(ಹೊಂದಿಕೆ, m, entries);
			    ನಿಲ್ಲಿಸು;
		    }
		ಆದರೆ (!TAILQ_ಮೊದಲ(ಹೊಂದಿಕೆ))
			ನಿಲ್ಲಿಸು;
	}
	fclose(fp);
	ಬಿಡುಗಡೆ(ಸಾಲು);
	ಆದರೆ (TAILQ_ಮೊದಲ(ಹೊಂದಿಕೆ))
		ಹಿಂತಿರುಗಿ false;
	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ RC_STRINGLIST *
ಹೊಂದಿಕೆ_list(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *exec, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pidfile)
{
	RC_STRINGLIST *ಹೊಂದಿಕೆ = rc_stringlist_new();
	ಪೂರ್ಣಾಂಕ i = 0;
	ಅಕ್ಷರ *m;

	ಆದರೆ (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_ಕೂಡಿಸು(ಹೊಂದಿಕೆ, m);
		ಬಿಡುಗಡೆ(m);
	}

	ಆಗ (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_ಕೂಡಿಸು(ಹೊಂದಿಕೆ, m);
		ಬಿಡುಗಡೆ(m);
	}

	ಆದರೆ (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_ಕೂಡಿಸು(ಹೊಂದಿಕೆ, m);
		ಬಿಡುಗಡೆ(m);
	}

	ಹಿಂತಿರುಗಿ ಹೊಂದಿಕೆ;
}

bool
rc_service_daemon_ಗಣ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *exec,
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv,
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pidfile, bool started)
{
	ಅಕ್ಷರ *file = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ nfiles = 0;
	ಅಕ್ಷರ oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	ರಚನೆ dirent *d;
	RC_STRINGLIST *ಹೊಂದಿಕೆ, *renamelist;
	ಪೂರ್ಣಾಂಕ i = 0;
	FILE *fp;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ತಳ = basename_c(service);

	ಆದರೆ (!exec && !pidfile) {
		errno = EINVAL;
		ಹಿಂತಿರುಗಿ false;
	}

	/* Regardless, erase any existing daemon info */
	ಆದರೆ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), ತಳ))) {
		ಹೊಂದಿಕೆ = ಹೊಂದಿಕೆ_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		ಆಗ ((d = readdir(dp))) {
			ಆದರೆ (d->d_name[0] == '.')
				ಮುಂದುವರಿ;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), ತಳ, d->d_name);
			ಆದರೆ (rc_stringlist_ಕಂಡುಹಿಡಿ(renamelist, file)) {
				ಬಿಡುಗಡೆ(file);
				ಮುಂದುವರಿ;
			}

			nfiles++;

			ಆದರೆ (!*oldfile) {
				ಆದರೆ (ಹೊಂದಿಕೆ_daemon(ತಳ, d->d_name, ಹೊಂದಿಕೆ)) {
					unlink(file);
					strlcpy(oldfile, file, ಗಾತ್ರ(oldfile));
					nfiles--;
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				rename(file, oldfile);
				strlcpy(oldfile, file, ಗಾತ್ರ(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_ಕೂಡಿಸು(renamelist, oldfile);
			}
			ಬಿಡುಗಡೆ(file);
		}
		closedir(dp);
		rc_stringlist_free(ಹೊಂದಿಕೆ);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	ಆದರೆ (!started)
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (mkdirat(rc_dirfd(RC_DIR_DAEMONS), ತಳ, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), ತಳ, nfiles + 1);
		ಆದರೆ ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			ಆದರೆ (exec)
				fprintf(fp, "%s", exec);
			ಆಗ (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			ಆದರೆ (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		ಬಿಡುಗಡೆ(file);
	}

	ಹಿಂತಿರುಗಿ retval;
}

bool
rc_service_started_daemon(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service,
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *exec, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv, ಪೂರ್ಣಾಂಕ indx)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ತಳ = basename_c(service);
	ಅಕ್ಷರ *file = ಖಾಲಿ;
	RC_STRINGLIST *ಹೊಂದಿಕೆ;
	bool retval = false;
	DIR *dp;
	ರಚನೆ dirent *d;

	ಆದರೆ (!service || !exec)
		ಹಿಂತಿರುಗಿ false;

	ಹೊಂದಿಕೆ = ಹೊಂದಿಕೆ_list(exec, argv, ಖಾಲಿ);

	ಆದರೆ (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = ಹೊಂದಿಕೆ_daemon(ತಳ, file, ಹೊಂದಿಕೆ);
		ಬಿಡುಗಡೆ(file);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), ತಳ))) {
		ಆಗ ((d = readdir(dp))) {
			ಆದರೆ (d->d_name[0] == '.')
				ಮುಂದುವರಿ;
			ಆದರೆ ((retval = ಹೊಂದಿಕೆ_daemon(ತಳ, d->d_name, ಹೊಂದಿಕೆ)))
				ನಿಲ್ಲಿಸು;
		}
		closedir(dp);
	}

	rc_stringlist_free(ಹೊಂದಿಕೆ);
	ಹಿಂತಿರುಗಿ retval;
}

bool
rc_service_daemons_crashed(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	DIR *dp;
	ರಚನೆ dirent *d;
	FILE *fp;
	ಅಕ್ಷರ *ಸಾಲು = ಖಾಲಿ;
	size_t len = 0;
	ಅಕ್ಷರ **argv = ಖಾಲಿ;
	ಅಕ್ಷರ *exec = ಖಾಲಿ;
	ಅಕ್ಷರ *name = ಖಾಲಿ;
	ಅಕ್ಷರ *pidfile = ಖಾಲಿ;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *token;
	bool retval = false;
	RC_STRINGLIST *list = ಖಾಲಿ;
	RC_STRING *s;
	size_t i;
	ಅಕ್ಷರ *ch_ಬೇರು;
	ಅಕ್ಷರ *spidfile;

	ಆದರೆ (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		ಹಿಂತಿರುಗಿ false;

	ಆಗ ((d = readdir(dp))) {
		ಆದರೆ (d->d_name[0] == '.')
			ಮುಂದುವರಿ;

		ಆದರೆ (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			ನಿಲ್ಲಿಸು;

		ಆಗ (xgetline(&ಸಾಲು, &len, fp) != -1) {
			p = ಸಾಲು;
			ಆದರೆ ((token = strsep(&p, "=")) == ಖಾಲಿ || !p)
				ಮುಂದುವರಿ;

			ಆದರೆ (!*p)
				ಮುಂದುವರಿ;

			ಆದರೆ (strcmp(token, "exec") == 0) {
				ಆದರೆ (exec)
					ಬಿಡುಗಡೆ(exec);
				exec = xstrdup(p);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(token, "argv_", 5) == 0) {
				ಆದರೆ (!list)
					list = rc_stringlist_new();
				rc_stringlist_ಕೂಡಿಸು(list, p);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(token, "name") == 0) {
				ಆದರೆ (name)
					ಬಿಡುಗಡೆ(name);
				name = xstrdup(p);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				ನಿಲ್ಲಿಸು;
			}
		}
		fclose(fp);

		ch_ಬೇರು = rc_service_ಮೌಲ್ಯ_get(basename_c(service), "chroot");
		spidfile = pidfile;
		ಆದರೆ (ch_ಬೇರು && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_ಬೇರು, pidfile);
			ಬಿಡುಗಡೆ(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		ಆದರೆ (pidfile) {
			retval = true;
			ಆದರೆ ((fp = fopen(pidfile, "r"))) {
				ಆದರೆ (fscanf(fp, "%d", &pid) == 1)
					retval = false;
				fclose(fp);
			}
			ಬಿಡುಗಡೆ(pidfile);
			pidfile = ಖಾಲಿ;

			/* We have the pid, so no need to match
			   on exec or name */
			ಬಿಡುಗಡೆ(exec);
			exec = ಖಾಲಿ;
			ಬಿಡುಗಡೆ(name);
			name = ಖಾಲಿ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (exec) {
				ಆದರೆ (!list)
					list = rc_stringlist_new();
				ಆದರೆ (!TAILQ_ಮೊದಲ(list))
					rc_stringlist_ಕೂಡಿಸು(list, exec);

				ಬಿಡುಗಡೆ(exec);
				exec = ಖಾಲಿ;
			}

			ಆದರೆ (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(ಗಾತ್ರ(ಅಕ್ಷರ *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->ಮೌಲ್ಯ;
				argv[i] = ಖಾಲಿ;
			}
		}

		ಆದರೆ (!retval) {
			ಆದರೆ (pid != 0) {
				ಆದರೆ (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((pids = rc_ಕಂಡುಹಿಡಿ_pids(exec,
				    (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *)argv,
				    0, pid)))
			{
				p1 = LIST_ಮೊದಲ(pids);
				ಆಗ (p1) {
					p2 = LIST_ಮುಂದಿನ(p1, entries);
					ಬಿಡುಗಡೆ(p1);
					p1 = p2;
				}
				ಬಿಡುಗಡೆ(pids);
			} ಇಲ್ಲದಿದ್ದರೆ
				retval = true;
		}
		rc_stringlist_free(list);
		list = ಖಾಲಿ;
		ಬಿಡುಗಡೆ(argv);
		argv = ಖಾಲಿ;
		ಬಿಡುಗಡೆ(exec);
		exec = ಖಾಲಿ;
		ಬಿಡುಗಡೆ(name);
		name = ಖಾಲಿ;
		ಆದರೆ (retval)
			ನಿಲ್ಲಿಸು;
	}
	closedir(dp);
	ಬಿಡುಗಡೆ(ಸಾಲು);

	ಹಿಂತಿರುಗಿ retval;
}
