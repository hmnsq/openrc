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

#شامل <errno.h>
#شامل <dirent.h>
#شامل <fcntl.h>
#شامل <limits.h>
#شامل <signal.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/stat.h>
#شامل <unistd.h>

#شامل "queue.h"
#شامل "librc.h"
#شامل "helpers.h"

#اگر defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
ایستا bool
pid_is_exec(pid_t pid, ثابت نویسه *exec)
{
	نویسه *buffer = پوچ;
	FILE *fp;
	صحیح c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	اگر ((fp = fopen(buffer, "r"))) {
		وقتی ((c = getc(fp)) != EOF && c != '(')
			;
		اگر (c == '(') {
			وقتی ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			اگر (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	آزاد(buffer);
	بازگشت retval;
}

ایستا bool
pid_is_argv(pid_t pid, ثابت نویسه *ثابت *argv)
{
	نویسه *cmdline = پوچ;
	صحیح fd;
	نویسه buffer[PATH_MAX];
	نویسه *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	اگر ((fd = open(cmdline, O_RDONLY)) < 0) {
		آزاد(cmdline);
		بازگشت false;
	}
	bytes = اقرأ(fd, buffer, اندازه(buffer) - 1);
	close(fd);
	آزاد(cmdline);
	اگر (bytes == -1)
		بازگشت false;

	buffer[bytes] = '\0';
	p = buffer;
	وقتی (*argv) {
		اگر ((unsigned)(p - buffer) >= اندازه(buffer))
			بازگشت false;
		اگر (strcmp(*argv, p) != 0)
			بازگشت false;
		argv++;
		p += strlen(p) + 1;
	}
	بازگشت true;
}

RC_PIDLIST *
rc_find_pids(ثابت نویسه *exec, ثابت نویسه *ثابت *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	ساختار dirent *entry;
	FILE *fp;
	صحیح rc;
	bool container_pid = false;
	bool openvz_host = false;
	نویسه *سطر = پوچ;
	نویسه my_ns[30] = {0};
	نویسه proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	نویسه *buffer = پوچ;
	ساختار stat sb;
	pid_t openrc_pid = 0;
	نویسه *pp;
	RC_PIDLIST *pids = پوچ;
	RC_PID *pi;

	اگر ((procdir = opendir("/proc")) == پوچ)
		بازگشت پوچ;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	اگر ((pp = getenv("RC_OPENRC_PID"))) {
		اگر (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	اگر ((fp = fopen("/proc/self/status", "r"))) {
		وقتی (xgetline(&سطر, &len, fp) != -1) {
			اگر (strncmp(سطر, "envID:\t0", 8) == 0) {
				openvz_host = true;
				بشکن;
			}
		}
		fclose(fp);
	}

	اگر ((rc = readlink("/proc/self/ns/pid", my_ns, اندازه(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	وقتی ((entry = readdir(procdir)) != پوچ) {
		اگر (sscanf(entry->d_name, "%d", &p) != 1)
			ادامه;
		اگر (openrc_pid != 0 && openrc_pid == p)
			ادامه;
		اگر (pid != 0 && pid != p)
			ادامه;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		اگر ((rc = readlink(buffer, proc_ns, اندازه(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		آزاد(buffer);
		اگر (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			ادامه;
		اگر (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			اگر (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				آزاد(buffer);
				ادامه;
			}
			آزاد(buffer);
		}
		اگر (exec && !pid_is_exec(p, exec))
			ادامه;
		اگر (argv &&
		    !pid_is_argv(p, (ثابت نویسه *ثابت *)argv))
			ادامه;
		/* If this is an OpenVZ host, filter out container processes */
		اگر (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			آزاد(buffer);
			اگر (!fp)
				ادامه;
			وقتی (xgetline(&سطر, &len, fp) != -1) {
				اگر (strncmp(سطر, "envID:", 6) == 0) {
					container_pid = !(strncmp(سطر, "envID:\t0", 8) == 0);
					بشکن;
				}
			}
			fclose(fp);
		}
		اگر (container_pid)
			ادامه;
		اگر (!pids) {
			pids = xmalloc(اندازه(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(اندازه(*pi));
		pi->pid = p;
		LIST_درج_HEAD(pids, pi, entries);
	}
	اگر (سطر != پوچ)
		آزاد(سطر);
	closedir(procdir);
	بازگشت pids;
}

#وگرنهاگر BSD

# اگر defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH پوچ
#  define _KVM_FLAGS KVM_NO_FILES
# وگرنه
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  اگر defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  وگرنه
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define _KVM_PATH _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_find_pids(ثابت نویسه *exec, ثابت نویسه *ثابت *argv, uid_t uid, pid_t pid)
{
	ایستا kvm_t *kd = پوچ;
	نویسه errbuf[_POSIX2_LINE_MAX];
	ساختار _KINFO_PROC *kp;
	صحیح i;
	صحیح processes = 0;
	صحیح pargc = 0;
	نویسه **pargv;
	RC_PIDLIST *pids = پوچ;
	RC_PID *pi;
	pid_t p;
	ثابت نویسه *ثابت *arg;
	صحیح match;

	اگر ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    پوچ, _KVM_FLAGS, errbuf)) == پوچ)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		بازگشت پوچ;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, اندازه(*kp), &processes);
#وگرنه
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	اگر ((kp == پوچ && processes > 0) || (kp != پوچ && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		بازگشت پوچ;
	}

	اگر (exec)
		exec = basename_c(exec);
	برای (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		اگر (pid != 0 && pid != p)
			ادامه;
		اگر (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			ادامه;
		اگر (exec) {
			اگر (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				ادامه;
		}
		اگر (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			اگر (!pargv || !*pargv)
				ادامه;
			arg = argv;
			match = 1;
			وقتی (*arg && *pargv)
				اگر (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					بشکن;
				}
			اگر (!match)
				ادامه;
		}
		اگر (!pids) {
			pids = xmalloc(اندازه(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(اندازه(*pi));
		pi->pid = p;
		LIST_درج_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	بازگشت pids;
}

#وگرنه
#  error "Platform not supported!"
#endif

ایستا bool
_match_daemon(ثابت نویسه *svcname, ثابت نویسه *instance, RC_STRINGLIST *match)
{
	نویسه *سطر = پوچ;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	نویسه *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	آزاد(daemon);

	اگر (!fp)
		بازگشت false;

	وقتی (xgetline(&سطر, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    اگر (strcmp(سطر, m->مقدار) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    بشکن;
		    }
		اگر (!TAILQ_FIRST(match))
			بشکن;
	}
	fclose(fp);
	آزاد(سطر);
	اگر (TAILQ_FIRST(match))
		بازگشت false;
	بازگشت true;
}

ایستا RC_STRINGLIST *
_match_list(ثابت نویسه *exec, ثابت نویسه *ثابت *argv, ثابت نویسه *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	صحیح i = 0;
	نویسه *m;

	اگر (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		آزاد(m);
	}

	وقتی (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		آزاد(m);
	}

	اگر (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		آزاد(m);
	}

	بازگشت match;
}

bool
rc_service_daemon_set(ثابت نویسه *service, ثابت نویسه *exec,
    ثابت نویسه *ثابت *argv,
    ثابت نویسه *pidfile, bool started)
{
	نویسه *file = پوچ;
	صحیح nfiles = 0;
	نویسه oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	ساختار dirent *d;
	RC_STRINGLIST *match, *renamelist;
	صحیح i = 0;
	FILE *fp;
	ثابت نویسه *base = basename_c(service);

	اگر (!exec && !pidfile) {
		errno = EINVAL;
		بازگشت false;
	}

	/* Regardless, erase any existing daemon info */
	اگر ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		وقتی ((d = readdir(dp))) {
			اگر (d->d_name[0] == '.')
				ادامه;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			اگر (rc_stringlist_find(renamelist, file)) {
				آزاد(file);
				ادامه;
			}

			nfiles++;

			اگر (!*oldfile) {
				اگر (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, اندازه(oldfile));
					nfiles--;
				}
			} وگرنه {
				rename(file, oldfile);
				strlcpy(oldfile, file, اندازه(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_add(renamelist, oldfile);
			}
			آزاد(file);
		}
		closedir(dp);
		rc_stringlist_free(match);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	اگر (!started)
		بازگشت true;

	اگر (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		اگر ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			اگر (exec)
				fprintf(fp, "%s", exec);
			وقتی (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			اگر (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		آزاد(file);
	}

	بازگشت retval;
}

bool
rc_service_started_daemon(ثابت نویسه *service,
    ثابت نویسه *exec, ثابت نویسه *ثابت *argv, صحیح indx)
{
	ثابت نویسه *base = basename_c(service);
	نویسه *file = پوچ;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	ساختار dirent *d;

	اگر (!service || !exec)
		بازگشت false;

	match = _match_list(exec, argv, پوچ);

	اگر (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		آزاد(file);
	} وگرنه اگر ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		وقتی ((d = readdir(dp))) {
			اگر (d->d_name[0] == '.')
				ادامه;
			اگر ((retval = _match_daemon(base, d->d_name, match)))
				بشکن;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	بازگشت retval;
}

bool
rc_service_daemons_crashed(ثابت نویسه *service)
{
	DIR *dp;
	ساختار dirent *d;
	FILE *fp;
	نویسه *سطر = پوچ;
	size_t len = 0;
	نویسه **argv = پوچ;
	نویسه *exec = پوچ;
	نویسه *name = پوچ;
	نویسه *pidfile = پوچ;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	نویسه *p;
	نویسه *token;
	bool retval = false;
	RC_STRINGLIST *list = پوچ;
	RC_STRING *s;
	size_t i;
	نویسه *ch_ریشه;
	نویسه *spidfile;

	اگر (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		بازگشت false;

	وقتی ((d = readdir(dp))) {
		اگر (d->d_name[0] == '.')
			ادامه;

		اگر (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			بشکن;

		وقتی (xgetline(&سطر, &len, fp) != -1) {
			p = سطر;
			اگر ((token = strsep(&p, "=")) == پوچ || !p)
				ادامه;

			اگر (!*p)
				ادامه;

			اگر (strcmp(token, "exec") == 0) {
				اگر (exec)
					آزاد(exec);
				exec = xstrdup(p);
			} وگرنه اگر (strncmp(token, "argv_", 5) == 0) {
				اگر (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} وگرنه اگر (strcmp(token, "name") == 0) {
				اگر (name)
					آزاد(name);
				name = xstrdup(p);
			} وگرنه اگر (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				بشکن;
			}
		}
		fclose(fp);

		ch_ریشه = rc_service_مقدار_get(basename_c(service), "chroot");
		spidfile = pidfile;
		اگر (ch_ریشه && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_ریشه, pidfile);
			آزاد(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		اگر (pidfile) {
			retval = true;
			اگر ((fp = fopen(pidfile, "r"))) {
				اگر (fscanf(fp, "%d", &pid) == 1)
					retval = false;
				fclose(fp);
			}
			آزاد(pidfile);
			pidfile = پوچ;

			/* We have the pid, so no need to match
			   on exec or name */
			آزاد(exec);
			exec = پوچ;
			آزاد(name);
			name = پوچ;
		} وگرنه {
			اگر (exec) {
				اگر (!list)
					list = rc_stringlist_new();
				اگر (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				آزاد(exec);
				exec = پوچ;
			}

			اگر (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(اندازه(نویسه *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->مقدار;
				argv[i] = پوچ;
			}
		}

		اگر (!retval) {
			اگر (pid != 0) {
				اگر (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} وگرنه اگر ((pids = rc_find_pids(exec,
				    (ثابت نویسه *ثابت *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				وقتی (p1) {
					p2 = LIST_NEXT(p1, entries);
					آزاد(p1);
					p1 = p2;
				}
				آزاد(pids);
			} وگرنه
				retval = true;
		}
		rc_stringlist_free(list);
		list = پوچ;
		آزاد(argv);
		argv = پوچ;
		آزاد(exec);
		exec = پوچ;
		آزاد(name);
		name = پوچ;
		اگر (retval)
			بشکن;
	}
	closedir(dp);
	آزاد(سطر);

	بازگشت retval;
}
