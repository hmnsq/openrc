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

#ಯದಿ defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
ಸ್ಥಿರ bool
pid_is_exec(pid_t pid, ಧ್ರುವ ವರ್ಣ *exec)
{
	ವರ್ಣ *buffer = ರಿಕ್ತ;
	FILE *fp;
	ಪೂರ್ಣಾಙ್ಕ c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	ಯದಿ ((fp = fopen(buffer, "r"))) {
		ಯಾವತ್ ((c = getc(fp)) != EOF && c != '(')
			;
		ಯದಿ (c == '(') {
			ಯಾವತ್ ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			ಯದಿ (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	ಮುಕ್ತ(buffer);
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಸ್ಥಿರ bool
pid_is_argv(pid_t pid, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv)
{
	ವರ್ಣ *cmdline = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ fd;
	ವರ್ಣ buffer[PATH_MAX];
	ವರ್ಣ *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	ಯದಿ ((fd = open(cmdline, O_RDONLY)) < 0) {
		ಮುಕ್ತ(cmdline);
		ಪ್ರತ್ಯಾಗಮ false;
	}
	bytes = ಪಠ(fd, buffer, ಪರಿಮಾಣ(buffer) - 1);
	close(fd);
	ಮುಕ್ತ(cmdline);
	ಯದಿ (bytes == -1)
		ಪ್ರತ್ಯಾಗಮ false;

	buffer[bytes] = '\0';
	p = buffer;
	ಯಾವತ್ (*argv) {
		ಯದಿ ((unsigned)(p - buffer) >= ಪರಿಮಾಣ(buffer))
			ಪ್ರತ್ಯಾಗಮ false;
		ಯದಿ (strcmp(*argv, p) != 0)
			ಪ್ರತ್ಯಾಗಮ false;
		argv++;
		p += strlen(p) + 1;
	}
	ಪ್ರತ್ಯಾಗಮ true;
}

RC_PIDLIST *
rc_find_pids(ಧ್ರುವ ವರ್ಣ *exec, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	ಸಂರಚನಾ dirent *entry;
	FILE *fp;
	ಪೂರ್ಣಾಙ್ಕ rc;
	bool container_pid = false;
	bool openvz_host = false;
	ವರ್ಣ *ಸಾಲು = ರಿಕ್ತ;
	ವರ್ಣ my_ns[30] = {0};
	ವರ್ಣ proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	ವರ್ಣ *buffer = ರಿಕ್ತ;
	ಸಂರಚನಾ stat sb;
	pid_t openrc_pid = 0;
	ವರ್ಣ *pp;
	RC_PIDLIST *pids = ರಿಕ್ತ;
	RC_PID *pi;

	ಯದಿ ((procdir = opendir("/proc")) == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	ಯದಿ ((pp = getenv("RC_OPENRC_PID"))) {
		ಯದಿ (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	ಯದಿ ((fp = fopen("/proc/self/status", "r"))) {
		ಯಾವತ್ (xgetline(&ಸಾಲು, &len, fp) != -1) {
			ಯದಿ (strncmp(ಸಾಲು, "envID:\t0", 8) == 0) {
				openvz_host = true;
				ಭಙ್ಗ;
			}
		}
		fclose(fp);
	}

	ಯದಿ ((rc = readlink("/proc/self/ns/pid", my_ns, ಪರಿಮಾಣ(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	ಯಾವತ್ ((entry = readdir(procdir)) != ರಿಕ್ತ) {
		ಯದಿ (sscanf(entry->d_name, "%d", &p) != 1)
			ಅನುವರ್ತ;
		ಯದಿ (openrc_pid != 0 && openrc_pid == p)
			ಅನುವರ್ತ;
		ಯದಿ (pid != 0 && pid != p)
			ಅನುವರ್ತ;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		ಯದಿ ((rc = readlink(buffer, proc_ns, ಪರಿಮಾಣ(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		ಮುಕ್ತ(buffer);
		ಯದಿ (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			ಅನುವರ್ತ;
		ಯದಿ (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			ಯದಿ (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				ಮುಕ್ತ(buffer);
				ಅನುವರ್ತ;
			}
			ಮುಕ್ತ(buffer);
		}
		ಯದಿ (exec && !pid_is_exec(p, exec))
			ಅನುವರ್ತ;
		ಯದಿ (argv &&
		    !pid_is_argv(p, (ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *)argv))
			ಅನುವರ್ತ;
		/* If this is an OpenVZ host, filter out container processes */
		ಯದಿ (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			ಮುಕ್ತ(buffer);
			ಯದಿ (!fp)
				ಅನುವರ್ತ;
			ಯಾವತ್ (xgetline(&ಸಾಲು, &len, fp) != -1) {
				ಯದಿ (strncmp(ಸಾಲು, "envID:", 6) == 0) {
					container_pid = !(strncmp(ಸಾಲು, "envID:\t0", 8) == 0);
					ಭಙ್ಗ;
				}
			}
			fclose(fp);
		}
		ಯದಿ (container_pid)
			ಅನುವರ್ತ;
		ಯದಿ (!pids) {
			pids = xmalloc(ಪರಿಮಾಣ(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(ಪರಿಮಾಣ(*pi));
		pi->pid = p;
		LIST_ಸೇರಿಸು_HEAD(pids, pi, entries);
	}
	ಯದಿ (ಸಾಲು != ರಿಕ್ತ)
		ಮುಕ್ತ(ಸಾಲು);
	closedir(procdir);
	ಪ್ರತ್ಯಾಗಮ pids;
}

#ನೋಚೇದ್ಯದಿ BSD

# ಯದಿ defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH ರಿಕ್ತ
#  define _KVM_FLAGS KVM_NO_FILES
# ಅನ್ಯಥಾ
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  ಯದಿ defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  ಅನ್ಯಥಾ
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define _KVM_PATH _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_find_pids(ಧ್ರುವ ವರ್ಣ *exec, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv, uid_t uid, pid_t pid)
{
	ಸ್ಥಿರ kvm_t *kd = ರಿಕ್ತ;
	ವರ್ಣ errbuf[_POSIX2_LINE_MAX];
	ಸಂರಚನಾ _KINFO_PROC *kp;
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ processes = 0;
	ಪೂರ್ಣಾಙ್ಕ pargc = 0;
	ವರ್ಣ **pargv;
	RC_PIDLIST *pids = ರಿಕ್ತ;
	RC_PID *pi;
	pid_t p;
	ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *arg;
	ಪೂರ್ಣಾಙ್ಕ match;

	ಯದಿ ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    ರಿಕ್ತ, _KVM_FLAGS, errbuf)) == ರಿಕ್ತ)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, ಪರಿಮಾಣ(*kp), &processes);
#ಅನ್ಯಥಾ
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	ಯದಿ ((kp == ರಿಕ್ತ && processes > 0) || (kp != ರಿಕ್ತ && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (exec)
		exec = basename_c(exec);
	ಪ್ರತಿ (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		ಯದಿ (pid != 0 && pid != p)
			ಅನುವರ್ತ;
		ಯದಿ (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			ಅನುವರ್ತ;
		ಯದಿ (exec) {
			ಯದಿ (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				ಅನುವರ್ತ;
		}
		ಯದಿ (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			ಯದಿ (!pargv || !*pargv)
				ಅನುವರ್ತ;
			arg = argv;
			match = 1;
			ಯಾವತ್ (*arg && *pargv)
				ಯದಿ (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					ಭಙ್ಗ;
				}
			ಯದಿ (!match)
				ಅನುವರ್ತ;
		}
		ಯದಿ (!pids) {
			pids = xmalloc(ಪರಿಮಾಣ(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(ಪರಿಮಾಣ(*pi));
		pi->pid = p;
		LIST_ಸೇರಿಸು_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	ಪ್ರತ್ಯಾಗಮ pids;
}

#ಅನ್ಯಥಾ
#  error "Platform not supported!"
#endif

ಸ್ಥಿರ bool
_match_daemon(ಧ್ರುವ ವರ್ಣ *svcname, ಧ್ರುವ ವರ್ಣ *instance, RC_STRINGLIST *match)
{
	ವರ್ಣ *ಸಾಲು = ರಿಕ್ತ;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	ವರ್ಣ *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	ಮುಕ್ತ(daemon);

	ಯದಿ (!fp)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯಾವತ್ (xgetline(&ಸಾಲು, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    ಯದಿ (strcmp(ಸಾಲು, m->ಮೌಲ್ಯ) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    ಭಙ್ಗ;
		    }
		ಯದಿ (!TAILQ_FIRST(match))
			ಭಙ್ಗ;
	}
	fclose(fp);
	ಮುಕ್ತ(ಸಾಲು);
	ಯದಿ (TAILQ_FIRST(match))
		ಪ್ರತ್ಯಾಗಮ false;
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ RC_STRINGLIST *
_match_list(ಧ್ರುವ ವರ್ಣ *exec, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv, ಧ್ರುವ ವರ್ಣ *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	ಪೂರ್ಣಾಙ್ಕ i = 0;
	ವರ್ಣ *m;

	ಯದಿ (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		ಮುಕ್ತ(m);
	}

	ಯಾವತ್ (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		ಮುಕ್ತ(m);
	}

	ಯದಿ (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		ಮುಕ್ತ(m);
	}

	ಪ್ರತ್ಯಾಗಮ match;
}

bool
rc_service_daemon_set(ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ ವರ್ಣ *exec,
    ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv,
    ಧ್ರುವ ವರ್ಣ *pidfile, bool started)
{
	ವರ್ಣ *file = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ nfiles = 0;
	ವರ್ಣ oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	ಸಂರಚನಾ dirent *d;
	RC_STRINGLIST *match, *renamelist;
	ಪೂರ್ಣಾಙ್ಕ i = 0;
	FILE *fp;
	ಧ್ರುವ ವರ್ಣ *base = basename_c(service);

	ಯದಿ (!exec && !pidfile) {
		errno = EINVAL;
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* Regardless, erase any existing daemon info */
	ಯದಿ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		ಯಾವತ್ ((d = readdir(dp))) {
			ಯದಿ (d->d_name[0] == '.')
				ಅನುವರ್ತ;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			ಯದಿ (rc_stringlist_find(renamelist, file)) {
				ಮುಕ್ತ(file);
				ಅನುವರ್ತ;
			}

			nfiles++;

			ಯದಿ (!*oldfile) {
				ಯದಿ (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, ಪರಿಮಾಣ(oldfile));
					nfiles--;
				}
			} ಅನ್ಯಥಾ {
				rename(file, oldfile);
				strlcpy(oldfile, file, ಪರಿಮಾಣ(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_add(renamelist, oldfile);
			}
			ಮುಕ್ತ(file);
		}
		closedir(dp);
		rc_stringlist_free(match);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	ಯದಿ (!started)
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		ಯದಿ ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			ಯದಿ (exec)
				fprintf(fp, "%s", exec);
			ಯಾವತ್ (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			ಯದಿ (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		ಮುಕ್ತ(file);
	}

	ಪ್ರತ್ಯಾಗಮ retval;
}

bool
rc_service_started_daemon(ಧ್ರುವ ವರ್ಣ *service,
    ಧ್ರುವ ವರ್ಣ *exec, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv, ಪೂರ್ಣಾಙ್ಕ indx)
{
	ಧ್ರುವ ವರ್ಣ *base = basename_c(service);
	ವರ್ಣ *file = ರಿಕ್ತ;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	ಸಂರಚನಾ dirent *d;

	ಯದಿ (!service || !exec)
		ಪ್ರತ್ಯಾಗಮ false;

	match = _match_list(exec, argv, ರಿಕ್ತ);

	ಯದಿ (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		ಮುಕ್ತ(file);
	} ಅನ್ಯಥಾ ಯದಿ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		ಯಾವತ್ ((d = readdir(dp))) {
			ಯದಿ (d->d_name[0] == '.')
				ಅನುವರ್ತ;
			ಯದಿ ((retval = _match_daemon(base, d->d_name, match)))
				ಭಙ್ಗ;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	ಪ್ರತ್ಯಾಗಮ retval;
}

bool
rc_service_daemons_crashed(ಧ್ರುವ ವರ್ಣ *service)
{
	DIR *dp;
	ಸಂರಚನಾ dirent *d;
	FILE *fp;
	ವರ್ಣ *ಸಾಲು = ರಿಕ್ತ;
	size_t len = 0;
	ವರ್ಣ **argv = ರಿಕ್ತ;
	ವರ್ಣ *exec = ರಿಕ್ತ;
	ವರ್ಣ *name = ರಿಕ್ತ;
	ವರ್ಣ *pidfile = ರಿಕ್ತ;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	ವರ್ಣ *p;
	ವರ್ಣ *token;
	bool retval = false;
	RC_STRINGLIST *list = ರಿಕ್ತ;
	RC_STRING *s;
	size_t i;
	ವರ್ಣ *ch_ಬೇರು;
	ವರ್ಣ *spidfile;

	ಯದಿ (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		ಪ್ರತ್ಯಾಗಮ false;

	ಯಾವತ್ ((d = readdir(dp))) {
		ಯದಿ (d->d_name[0] == '.')
			ಅನುವರ್ತ;

		ಯದಿ (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			ಭಙ್ಗ;

		ಯಾವತ್ (xgetline(&ಸಾಲು, &len, fp) != -1) {
			p = ಸಾಲು;
			ಯದಿ ((token = strsep(&p, "=")) == ರಿಕ್ತ || !p)
				ಅನುವರ್ತ;

			ಯದಿ (!*p)
				ಅನುವರ್ತ;

			ಯದಿ (strcmp(token, "exec") == 0) {
				ಯದಿ (exec)
					ಮುಕ್ತ(exec);
				exec = xstrdup(p);
			} ಅನ್ಯಥಾ ಯದಿ (strncmp(token, "argv_", 5) == 0) {
				ಯದಿ (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} ಅನ್ಯಥಾ ಯದಿ (strcmp(token, "name") == 0) {
				ಯದಿ (name)
					ಮುಕ್ತ(name);
				name = xstrdup(p);
			} ಅನ್ಯಥಾ ಯದಿ (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				ಭಙ್ಗ;
			}
		}
		fclose(fp);

		ch_ಬೇರು = rc_service_ಮೌಲ್ಯ_get(basename_c(service), "chroot");
		spidfile = pidfile;
		ಯದಿ (ch_ಬೇರು && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_ಬೇರು, pidfile);
			ಮುಕ್ತ(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		ಯದಿ (pidfile) {
			retval = true;
			ಯದಿ ((fp = fopen(pidfile, "r"))) {
				ಯದಿ (fscanf(fp, "%d", &pid) == 1)
					retval = false;
				fclose(fp);
			}
			ಮುಕ್ತ(pidfile);
			pidfile = ರಿಕ್ತ;

			/* We have the pid, so no need to match
			   on exec or name */
			ಮುಕ್ತ(exec);
			exec = ರಿಕ್ತ;
			ಮುಕ್ತ(name);
			name = ರಿಕ್ತ;
		} ಅನ್ಯಥಾ {
			ಯದಿ (exec) {
				ಯದಿ (!list)
					list = rc_stringlist_new();
				ಯದಿ (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				ಮುಕ್ತ(exec);
				exec = ರಿಕ್ತ;
			}

			ಯದಿ (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(ಪರಿಮಾಣ(ವರ್ಣ *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->ಮೌಲ್ಯ;
				argv[i] = ರಿಕ್ತ;
			}
		}

		ಯದಿ (!retval) {
			ಯದಿ (pid != 0) {
				ಯದಿ (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} ಅನ್ಯಥಾ ಯದಿ ((pids = rc_find_pids(exec,
				    (ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				ಯಾವತ್ (p1) {
					p2 = LIST_NEXT(p1, entries);
					ಮುಕ್ತ(p1);
					p1 = p2;
				}
				ಮುಕ್ತ(pids);
			} ಅನ್ಯಥಾ
				retval = true;
		}
		rc_stringlist_free(list);
		list = ರಿಕ್ತ;
		ಮುಕ್ತ(argv);
		argv = ರಿಕ್ತ;
		ಮುಕ್ತ(exec);
		exec = ರಿಕ್ತ;
		ಮುಕ್ತ(name);
		name = ರಿಕ್ತ;
		ಯದಿ (retval)
			ಭಙ್ಗ;
	}
	closedir(dp);
	ಮುಕ್ತ(ಸಾಲು);

	ಪ್ರತ್ಯಾಗಮ retval;
}
