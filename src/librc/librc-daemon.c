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

#אם defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
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
	אם ((fp = fopen(buffer, "r"))) {
		כלעוד ((c = getc(fp)) != EOF && c != '(')
			;
		אם (c == '(') {
			כלעוד ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			אם (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	حرر(buffer);
	החזר retval;
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
	אם ((fd = open(cmdline, O_RDONLY)) < 0) {
		حرر(cmdline);
		החזר false;
	}
	bytes = اقرأ(fd, buffer, حجم(buffer) - 1);
	close(fd);
	حرر(cmdline);
	אם (bytes == -1)
		החזר false;

	buffer[bytes] = '\0';
	p = buffer;
	כלעוד (*argv) {
		אם ((unsigned)(p - buffer) >= حجم(buffer))
			החזר false;
		אם (strcmp(*argv, p) != 0)
			החזר false;
		argv++;
		p += strlen(p) + 1;
	}
	החזר true;
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
	حرف *line = عدم;
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

	אם ((procdir = opendir("/proc")) == عدم)
		החזר عدم;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	אם ((pp = getenv("RC_OPENRC_PID"))) {
		אם (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	אם ((fp = fopen("/proc/self/status", "r"))) {
		כלעוד (xgetline(&line, &len, fp) != -1) {
			אם (strncmp(line, "envID:\t0", 8) == 0) {
				openvz_host = true;
				اكسر;
			}
		}
		fclose(fp);
	}

	אם ((rc = readlink("/proc/self/ns/pid", my_ns, حجم(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	כלעוד ((entry = readdir(procdir)) != عدم) {
		אם (sscanf(entry->d_name, "%d", &p) != 1)
			استمر;
		אם (openrc_pid != 0 && openrc_pid == p)
			استمر;
		אם (pid != 0 && pid != p)
			استمر;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		אם ((rc = readlink(buffer, proc_ns, حجم(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		حرر(buffer);
		אם (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			استمر;
		אם (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			אם (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				حرر(buffer);
				استمر;
			}
			حرر(buffer);
		}
		אם (exec && !pid_is_exec(p, exec))
			استمر;
		אם (argv &&
		    !pid_is_argv(p, (ثابت حرف *ثابت *)argv))
			استمر;
		/* If this is an OpenVZ host, filter out container processes */
		אם (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			حرر(buffer);
			אם (!fp)
				استمر;
			כלעוד (xgetline(&line, &len, fp) != -1) {
				אם (strncmp(line, "envID:", 6) == 0) {
					container_pid = !(strncmp(line, "envID:\t0", 8) == 0);
					اكسر;
				}
			}
			fclose(fp);
		}
		אם (container_pid)
			استمر;
		אם (!pids) {
			pids = xmalloc(حجم(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(حجم(*pi));
		pi->pid = p;
		LIST_INSERT_HEAD(pids, pi, entries);
	}
	אם (line != عدم)
		حرر(line);
	closedir(procdir);
	החזר pids;
}

#وإلاإذا BSD

# אם defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH عدم
#  define _KVM_FLAGS KVM_NO_FILES
# אחרת
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  אם defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  אחרת
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

	אם ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    عدم, _KVM_FLAGS, errbuf)) == عدم)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		החזר عدم;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, حجم(*kp), &processes);
#אחרת
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	אם ((kp == عدم && processes > 0) || (kp != عدم && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		החזר عدم;
	}

	אם (exec)
		exec = basename_c(exec);
	עבור (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		אם (pid != 0 && pid != p)
			استمر;
		אם (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			استمر;
		אם (exec) {
			אם (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				استمر;
		}
		אם (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			אם (!pargv || !*pargv)
				استمر;
			arg = argv;
			match = 1;
			כלעוד (*arg && *pargv)
				אם (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					اكسر;
				}
			אם (!match)
				استمر;
		}
		אם (!pids) {
			pids = xmalloc(حجم(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(حجم(*pi));
		pi->pid = p;
		LIST_INSERT_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	החזר pids;
}

#אחרת
#  error "Platform not supported!"
#endif

ساكن bool
_match_daemon(ثابت حرف *svcname, ثابت حرف *instance, RC_STRINGLIST *match)
{
	حرف *line = عدم;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	حرف *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	حرر(daemon);

	אם (!fp)
		החזר false;

	כלעוד (xgetline(&line, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    אם (strcmp(line, m->value) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    اكسر;
		    }
		אם (!TAILQ_FIRST(match))
			اكسر;
	}
	fclose(fp);
	حرر(line);
	אם (TAILQ_FIRST(match))
		החזר false;
	החזר true;
}

ساكن RC_STRINGLIST *
_match_list(ثابت حرف *exec, ثابت حرف *ثابت *argv, ثابت حرف *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	صحيح i = 0;
	حرف *m;

	אם (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		حرر(m);
	}

	כלעוד (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		حرر(m);
	}

	אם (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		حرر(m);
	}

	החזר match;
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

	אם (!exec && !pidfile) {
		errno = EINVAL;
		החזר false;
	}

	/* Regardless, erase any existing daemon info */
	אם ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		כלעוד ((d = readdir(dp))) {
			אם (d->d_name[0] == '.')
				استمر;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			אם (rc_stringlist_find(renamelist, file)) {
				حرر(file);
				استمر;
			}

			nfiles++;

			אם (!*oldfile) {
				אם (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, حجم(oldfile));
					nfiles--;
				}
			} אחרת {
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
	אם (!started)
		החזר true;

	אם (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		אם ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			אם (exec)
				fprintf(fp, "%s", exec);
			כלעוד (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			אם (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		حرر(file);
	}

	החזר retval;
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

	אם (!service || !exec)
		החזר false;

	match = _match_list(exec, argv, عدم);

	אם (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		حرر(file);
	} אחרת אם ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		כלעוד ((d = readdir(dp))) {
			אם (d->d_name[0] == '.')
				استمر;
			אם ((retval = _match_daemon(base, d->d_name, match)))
				اكسر;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	החזר retval;
}

bool
rc_service_daemons_crashed(ثابت حرف *service)
{
	DIR *dp;
	بنية dirent *d;
	FILE *fp;
	حرف *line = عدم;
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
	حرف *ch_root;
	حرف *spidfile;

	אם (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		החזר false;

	כלעוד ((d = readdir(dp))) {
		אם (d->d_name[0] == '.')
			استمر;

		אם (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			اكسر;

		כלעוד (xgetline(&line, &len, fp) != -1) {
			p = line;
			אם ((token = strsep(&p, "=")) == عدم || !p)
				استمر;

			אם (!*p)
				استمر;

			אם (strcmp(token, "exec") == 0) {
				אם (exec)
					حرر(exec);
				exec = xstrdup(p);
			} אחרת אם (strncmp(token, "argv_", 5) == 0) {
				אם (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} אחרת אם (strcmp(token, "name") == 0) {
				אם (name)
					حرر(name);
				name = xstrdup(p);
			} אחרת אם (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				اكسر;
			}
		}
		fclose(fp);

		ch_root = rc_service_value_get(basename_c(service), "chroot");
		spidfile = pidfile;
		אם (ch_root && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_root, pidfile);
			حرر(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		אם (pidfile) {
			retval = true;
			אם ((fp = fopen(pidfile, "r"))) {
				אם (fscanf(fp, "%d", &pid) == 1)
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
		} אחרת {
			אם (exec) {
				אם (!list)
					list = rc_stringlist_new();
				אם (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				حرر(exec);
				exec = عدم;
			}

			אם (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(حجم(حرف *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->value;
				argv[i] = عدم;
			}
		}

		אם (!retval) {
			אם (pid != 0) {
				אם (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} אחרת אם ((pids = rc_find_pids(exec,
				    (ثابت حرف *ثابت *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				כלעוד (p1) {
					p2 = LIST_NEXT(p1, entries);
					حرر(p1);
					p1 = p2;
				}
				حرر(pids);
			} אחרת
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
		אם (retval)
			اكسر;
	}
	closedir(dp);
	حرر(line);

	החזר retval;
}
