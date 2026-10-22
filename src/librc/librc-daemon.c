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

#ضمن <errno.h>
#ضمن <dirent.h>
#ضمن <fcntl.h>
#ضمن <limits.h>
#ضمن <signal.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/stat.h>
#ضمن <unistd.h>

#ضمن "queue.h"
#ضمن "librc.h"
#ضمن "helpers.h"

#إذا defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
ساكن bool
pid_is_exec(pid_t pid, ثابت حرف *exec)
{
	حرف *buffer = عدم;
	FILE *fp;
	صحيح c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	إذا ((fp = fopen(buffer, "r"))) {
		طالما ((c = getc(fp)) != EOF && c != '(')
			;
		إذا (c == '(') {
			طالما ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			إذا (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	حرر(buffer);
	أرجع retval;
}

ساكن bool
pid_is_argv(pid_t pid, ثابت حرف *ثابت *argv)
{
	حرف *cmdline = عدم;
	صحيح fd;
	حرف buffer[PATH_MAX];
	حرف *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	إذا ((fd = open(cmdline, O_RDONLY)) < 0) {
		حرر(cmdline);
		أرجع false;
	}
	bytes = اقرأ(fd, buffer, حجم(buffer) - 1);
	close(fd);
	حرر(cmdline);
	إذا (bytes == -1)
		أرجع false;

	buffer[bytes] = '\0';
	p = buffer;
	طالما (*argv) {
		إذا ((unsigned)(p - buffer) >= حجم(buffer))
			أرجع false;
		إذا (strcmp(*argv, p) != 0)
			أرجع false;
		argv++;
		p += strlen(p) + 1;
	}
	أرجع true;
}

RC_PIDLIST *
rc_find_pids(ثابت حرف *exec, ثابت حرف *ثابت *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	بنية dirent *entry;
	FILE *fp;
	صحيح rc;
	bool container_pid = false;
	bool openvz_host = false;
	حرف *قۇر = عدم;
	حرف my_ns[30] = {0};
	حرف proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	حرف *buffer = عدم;
	بنية stat sb;
	pid_t openrc_pid = 0;
	حرف *pp;
	RC_PIDLIST *pids = عدم;
	RC_PID *pi;

	إذا ((procdir = opendir("/proc")) == عدم)
		أرجع عدم;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	إذا ((pp = getenv("RC_OPENRC_PID"))) {
		إذا (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	إذا ((fp = fopen("/proc/self/status", "r"))) {
		طالما (xgetline(&قۇر, &len, fp) != -1) {
			إذا (strncmp(قۇر, "envID:\t0", 8) == 0) {
				openvz_host = true;
				اكسر;
			}
		}
		fclose(fp);
	}

	إذا ((rc = readlink("/proc/self/ns/pid", my_ns, حجم(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	طالما ((entry = readdir(procdir)) != عدم) {
		إذا (sscanf(entry->d_name, "%d", &p) != 1)
			استمر;
		إذا (openrc_pid != 0 && openrc_pid == p)
			استمر;
		إذا (pid != 0 && pid != p)
			استمر;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		إذا ((rc = readlink(buffer, proc_ns, حجم(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		حرر(buffer);
		إذا (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			استمر;
		إذا (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			إذا (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				حرر(buffer);
				استمر;
			}
			حرر(buffer);
		}
		إذا (exec && !pid_is_exec(p, exec))
			استمر;
		إذا (argv &&
		    !pid_is_argv(p, (ثابت حرف *ثابت *)argv))
			استمر;
		/* If this is an OpenVZ host, filter out container processes */
		إذا (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			حرر(buffer);
			إذا (!fp)
				استمر;
			طالما (xgetline(&قۇر, &len, fp) != -1) {
				إذا (strncmp(قۇر, "envID:", 6) == 0) {
					container_pid = !(strncmp(قۇر, "envID:\t0", 8) == 0);
					اكسر;
				}
			}
			fclose(fp);
		}
		إذا (container_pid)
			استمر;
		إذا (!pids) {
			pids = xmalloc(حجم(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(حجم(*pi));
		pi->pid = p;
		LIST_قىستۇرۇش_HEAD(pids, pi, entries);
	}
	إذا (قۇر != عدم)
		حرر(قۇر);
	closedir(procdir);
	أرجع pids;
}

#وإلاإذا BSD

# إذا defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH عدم
#  define _KVM_FLAGS KVM_NO_FILES
# وإلا
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  إذا defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  وإلا
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define _KVM_PATH _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_find_pids(ثابت حرف *exec, ثابت حرف *ثابت *argv, uid_t uid, pid_t pid)
{
	ساكن kvm_t *kd = عدم;
	حرف errbuf[_POSIX2_LINE_MAX];
	بنية _KINFO_PROC *kp;
	صحيح i;
	صحيح processes = 0;
	صحيح pargc = 0;
	حرف **pargv;
	RC_PIDLIST *pids = عدم;
	RC_PID *pi;
	pid_t p;
	ثابت حرف *ثابت *arg;
	صحيح match;

	إذا ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    عدم, _KVM_FLAGS, errbuf)) == عدم)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		أرجع عدم;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, حجم(*kp), &processes);
#وإلا
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	إذا ((kp == عدم && processes > 0) || (kp != عدم && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		أرجع عدم;
	}

	إذا (exec)
		exec = basename_c(exec);
	لكل (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		إذا (pid != 0 && pid != p)
			استمر;
		إذا (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			استمر;
		إذا (exec) {
			إذا (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				استمر;
		}
		إذا (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			إذا (!pargv || !*pargv)
				استمر;
			arg = argv;
			match = 1;
			طالما (*arg && *pargv)
				إذا (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					اكسر;
				}
			إذا (!match)
				استمر;
		}
		إذا (!pids) {
			pids = xmalloc(حجم(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(حجم(*pi));
		pi->pid = p;
		LIST_قىستۇرۇش_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	أرجع pids;
}

#وإلا
#  error "Platform not supported!"
#endif

ساكن bool
_match_daemon(ثابت حرف *svcname, ثابت حرف *instance, RC_STRINGLIST *match)
{
	حرف *قۇر = عدم;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	حرف *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	حرر(daemon);

	إذا (!fp)
		أرجع false;

	طالما (xgetline(&قۇر, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    إذا (strcmp(قۇر, m->قىممەت) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    اكسر;
		    }
		إذا (!TAILQ_FIRST(match))
			اكسر;
	}
	fclose(fp);
	حرر(قۇر);
	إذا (TAILQ_FIRST(match))
		أرجع false;
	أرجع true;
}

ساكن RC_STRINGLIST *
_match_list(ثابت حرف *exec, ثابت حرف *ثابت *argv, ثابت حرف *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	صحيح i = 0;
	حرف *m;

	إذا (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		حرر(m);
	}

	طالما (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		حرر(m);
	}

	إذا (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		حرر(m);
	}

	أرجع match;
}

bool
rc_service_daemon_set(ثابت حرف *service, ثابت حرف *exec,
    ثابت حرف *ثابت *argv,
    ثابت حرف *pidfile, bool started)
{
	حرف *file = عدم;
	صحيح nfiles = 0;
	حرف oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	بنية dirent *d;
	RC_STRINGLIST *match, *renamelist;
	صحيح i = 0;
	FILE *fp;
	ثابت حرف *base = basename_c(service);

	إذا (!exec && !pidfile) {
		errno = EINVAL;
		أرجع false;
	}

	/* Regardless, erase any existing daemon info */
	إذا ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		طالما ((d = readdir(dp))) {
			إذا (d->d_name[0] == '.')
				استمر;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			إذا (rc_stringlist_find(renamelist, file)) {
				حرر(file);
				استمر;
			}

			nfiles++;

			إذا (!*oldfile) {
				إذا (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, حجم(oldfile));
					nfiles--;
				}
			} وإلا {
				rename(file, oldfile);
				strlcpy(oldfile, file, حجم(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_add(renamelist, oldfile);
			}
			حرر(file);
		}
		closedir(dp);
		rc_stringlist_free(match);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	إذا (!started)
		أرجع true;

	إذا (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		إذا ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			إذا (exec)
				fprintf(fp, "%s", exec);
			طالما (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			إذا (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		حرر(file);
	}

	أرجع retval;
}

bool
rc_service_started_daemon(ثابت حرف *service,
    ثابت حرف *exec, ثابت حرف *ثابت *argv, صحيح indx)
{
	ثابت حرف *base = basename_c(service);
	حرف *file = عدم;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	بنية dirent *d;

	إذا (!service || !exec)
		أرجع false;

	match = _match_list(exec, argv, عدم);

	إذا (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		حرر(file);
	} وإلا إذا ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		طالما ((d = readdir(dp))) {
			إذا (d->d_name[0] == '.')
				استمر;
			إذا ((retval = _match_daemon(base, d->d_name, match)))
				اكسر;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	أرجع retval;
}

bool
rc_service_daemons_crashed(ثابت حرف *service)
{
	DIR *dp;
	بنية dirent *d;
	FILE *fp;
	حرف *قۇر = عدم;
	size_t len = 0;
	حرف **argv = عدم;
	حرف *exec = عدم;
	حرف *name = عدم;
	حرف *pidfile = عدم;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	حرف *p;
	حرف *token;
	bool retval = false;
	RC_STRINGLIST *list = عدم;
	RC_STRING *s;
	size_t i;
	حرف *ch_يىلتىز;
	حرف *spidfile;

	إذا (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		أرجع false;

	طالما ((d = readdir(dp))) {
		إذا (d->d_name[0] == '.')
			استمر;

		إذا (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			اكسر;

		طالما (xgetline(&قۇر, &len, fp) != -1) {
			p = قۇر;
			إذا ((token = strsep(&p, "=")) == عدم || !p)
				استمر;

			إذا (!*p)
				استمر;

			إذا (strcmp(token, "exec") == 0) {
				إذا (exec)
					حرر(exec);
				exec = xstrdup(p);
			} وإلا إذا (strncmp(token, "argv_", 5) == 0) {
				إذا (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} وإلا إذا (strcmp(token, "name") == 0) {
				إذا (name)
					حرر(name);
				name = xstrdup(p);
			} وإلا إذا (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				اكسر;
			}
		}
		fclose(fp);

		ch_يىلتىز = rc_service_قىممەت_get(basename_c(service), "chroot");
		spidfile = pidfile;
		إذا (ch_يىلتىز && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_يىلتىز, pidfile);
			حرر(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		إذا (pidfile) {
			retval = true;
			إذا ((fp = fopen(pidfile, "r"))) {
				إذا (fscanf(fp, "%d", &pid) == 1)
					retval = false;
				fclose(fp);
			}
			حرر(pidfile);
			pidfile = عدم;

			/* We have the pid, so no need to match
			   on exec or name */
			حرر(exec);
			exec = عدم;
			حرر(name);
			name = عدم;
		} وإلا {
			إذا (exec) {
				إذا (!list)
					list = rc_stringlist_new();
				إذا (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				حرر(exec);
				exec = عدم;
			}

			إذا (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(حجم(حرف *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->قىممەت;
				argv[i] = عدم;
			}
		}

		إذا (!retval) {
			إذا (pid != 0) {
				إذا (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} وإلا إذا ((pids = rc_find_pids(exec,
				    (ثابت حرف *ثابت *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				طالما (p1) {
					p2 = LIST_NEXT(p1, entries);
					حرر(p1);
					p1 = p2;
				}
				حرر(pids);
			} وإلا
				retval = true;
		}
		rc_stringlist_free(list);
		list = عدم;
		حرر(argv);
		argv = عدم;
		حرر(exec);
		exec = عدم;
		حرر(name);
		name = عدم;
		إذا (retval)
			اكسر;
	}
	closedir(dp);
	حرر(قۇر);

	أرجع retval;
}
