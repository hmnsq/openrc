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
ساکن bool
pid_is_exec(pid_t pid, ثابت حرف *exec)
{
	حرف *بفر = تہی;
	FILE *fp;
	صحیح c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&بفر, "/proc/%d/stat", pid);
	اگر ((fp = fopen(بفر, "r"))) {
		جب ((c = getc(fp)) != EOF && c != '(')
			;
		اگر (c == '(') {
			جب ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			اگر (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	آزاد(بفر);
	واپس retval;
}

ساکن bool
pid_is_argv(pid_t pid, ثابت حرف *ثابت *argv)
{
	حرف *cmdline = تہی;
	صحیح fd;
	حرف بفر[PATH_MAX];
	حرف *p;
	ssize_t بائٹیں;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	اگر ((fd = open(cmdline, O_RDONLY)) < 0) {
		آزاد(cmdline);
		واپس false;
	}
	بائٹیں = اقرأ(fd, بفر, حجم(بفر) - 1);
	close(fd);
	آزاد(cmdline);
	اگر (بائٹیں == -1)
		واپس false;

	بفر[بائٹیں] = '\0';
	p = بفر;
	جب (*argv) {
		اگر ((غیرعلامتی)(p - بفر) >= حجم(بفر))
			واپس false;
		اگر (strcmp(*argv, p) != 0)
			واپس false;
		argv++;
		p += لڑی_لمبائی(p) + 1;
	}
	واپس true;
}

RC_PIDLIST *
rc_ڈھونڈو_pids(ثابت حرف *exec, ثابت حرف *ثابت *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	ساخت dirent *اندراج_2;
	FILE *fp;
	صحیح rc;
	bool container_pid = false;
	bool openvz_host = false;
	حرف *سطر = تہی;
	حرف my_ns[30] = {0};
	حرف proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	حرف *بفر = تہی;
	ساخت stat sb;
	pid_t openrc_pid = 0;
	حرف *pp;
	RC_PIDLIST *pids = تہی;
	RC_PID *pi;

	اگر ((procdir = opendir("/proc")) == تہی)
		واپس تہی;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	اگر ((pp = getenv("RC_OPENRC_PID"))) {
		اگر (لڑی_سے_پڑھو(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	اگر ((fp = fopen("/proc/self/status", "r"))) {
		جب (xgetline(&سطر, &len, fp) != -1) {
			اگر (لڑی_موازنہ(سطر, "envID:\t0", 8) == 0) {
				openvz_host = true;
				توڑو;
			}
		}
		fclose(fp);
	}

	اگر ((rc = readlink("/proc/self/ns/pid", my_ns, حجم(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	جب ((اندراج_2 = readdir(procdir)) != تہی) {
		اگر (لڑی_سے_پڑھو(اندراج_2->d_name, "%d", &p) != 1)
			جاری;
		اگر (openrc_pid != 0 && openrc_pid == p)
			جاری;
		اگر (pid != 0 && pid != p)
			جاری;
		xasprintf(&بفر, "/proc/%d/ns/pid", p);
		اگر ((rc = readlink(بفر, proc_ns, حجم(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		آزاد(بفر);
		اگر (pid == 0 && لڑی_لمبائی(my_ns) && لڑی_لمبائی (proc_ns) && strcmp(my_ns, proc_ns))
			جاری;
		اگر (uid) {
			xasprintf(&بفر, "/proc/%d", p);
			اگر (stat(بفر, &sb) != 0 || sb.st_uid != uid) {
				آزاد(بفر);
				جاری;
			}
			آزاد(بفر);
		}
		اگر (exec && !pid_is_exec(p, exec))
			جاری;
		اگر (argv &&
		    !pid_is_argv(p, (ثابت حرف *ثابت *)argv))
			جاری;
		/* If this is an OpenVZ host, filter out container processes */
		اگر (openvz_host) {
			xasprintf(&بفر, "/proc/%d/status", p);
			fp = fopen(بفر, "r");
			آزاد(بفر);
			اگر (!fp)
				جاری;
			جب (xgetline(&سطر, &len, fp) != -1) {
				اگر (لڑی_موازنہ(سطر, "envID:", 6) == 0) {
					container_pid = !(لڑی_موازنہ(سطر, "envID:\t0", 8) == 0);
					توڑو;
				}
			}
			fclose(fp);
		}
		اگر (container_pid)
			جاری;
		اگر (!pids) {
			pids = xmalloc(حجم(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(حجم(*pi));
		pi->pid = p;
		LIST_داخل_کرو_سر(pids, pi, entries);
	}
	اگر (سطر != تہی)
		آزاد(سطر);
	closedir(procdir);
	واپس pids;
}

#وگرنہ_اگر BSD

# اگر defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define KVM_راستہ تہی
#  define _KVM_FLAGS KVM_NO_FILES
# ورنہ
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  اگر defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  ورنہ
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define KVM_راستہ _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_ڈھونڈو_pids(ثابت حرف *exec, ثابت حرف *ثابت *argv, uid_t uid, pid_t pid)
{
	ساکن kvm_t *kd = تہی;
	حرف errbuf[_POSIX2_LINE_MAX];
	ساخت _KINFO_PROC *kp;
	صحیح i;
	صحیح processes = 0;
	صحیح pargc = 0;
	حرف **pargv;
	RC_PIDLIST *pids = تہی;
	RC_PID *pi;
	pid_t p;
	ثابت حرف *ثابت *arg;
	صحیح مطابقت;

	اگر ((kd = kvm_openfiles(KVM_راستہ, KVM_راستہ,
		    تہی, _KVM_FLAGS, errbuf)) == تہی)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		واپس تہی;
	}

#اگر_تعریف_شدہ _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, حجم(*kp), &processes);
#ورنہ
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#اگر_ختم
	اگر ((kp == تہی && processes > 0) || (kp != تہی && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		واپس تہی;
	}

	اگر (exec)
		exec = basename_c(exec);
	برائے (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		اگر (pid != 0 && pid != p)
			جاری;
		اگر (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			جاری;
		اگر (exec) {
			اگر (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				جاری;
		}
		اگر (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			اگر (!pargv || !*pargv)
				جاری;
			arg = argv;
			مطابقت = 1;
			جب (*arg && *pargv)
				اگر (strcmp(*arg++, *pargv++) != 0) {
					مطابقت = 0;
					توڑو;
				}
			اگر (!مطابقت)
				جاری;
		}
		اگر (!pids) {
			pids = xmalloc(حجم(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(حجم(*pi));
		pi->pid = p;
		LIST_داخل_کرو_سر(pids, pi, entries);
	}
	kvm_close(kd);

	واپس pids;
}

#ورنہ
#  error "Platform not supported!"
#اگر_ختم

ساکن bool
مطابقت_daemon(ثابت حرف *svcname, ثابت حرف *instance, RC_STRINGLIST *مطابقت)
{
	حرف *سطر = تہی;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	حرف *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	آزاد(daemon);

	اگر (!fp)
		واپس false;

	جب (xgetline(&سطر, &len, fp) != -1) {
		TAILQ_FOREACH(m, مطابقت, entries)
		    اگر (strcmp(سطر, m->قدر) == 0) {
			    TAILQ_REMOVE(مطابقت, m, entries);
			    توڑو;
		    }
		اگر (!TAILQ_پہلا(مطابقت))
			توڑو;
	}
	fclose(fp);
	آزاد(سطر);
	اگر (TAILQ_پہلا(مطابقت))
		واپس false;
	واپس true;
}

ساکن RC_STRINGLIST *
مطابقت_list(ثابت حرف *exec, ثابت حرف *ثابت *argv, ثابت حرف *pidfile)
{
	RC_STRINGLIST *مطابقت = rc_stringlist_new();
	صحیح i = 0;
	حرف *m;

	اگر (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_جوڑو(مطابقت, m);
		آزاد(m);
	}

	جب (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_جوڑو(مطابقت, m);
		آزاد(m);
	}

	اگر (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_جوڑو(مطابقت, m);
		آزاد(m);
	}

	واپس مطابقت;
}

bool
rc_service_daemon_سیٹ(ثابت حرف *service, ثابت حرف *exec,
    ثابت حرف *ثابت *argv,
    ثابت حرف *pidfile, bool started)
{
	حرف *file = تہی;
	صحیح nfiles = 0;
	حرف oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	ساخت dirent *d;
	RC_STRINGLIST *مطابقت, *renamelist;
	صحیح i = 0;
	FILE *fp;
	ثابت حرف *بنیاد = basename_c(service);

	اگر (!exec && !pidfile) {
		errno = EINVAL;
		واپس false;
	}

	/* Regardless, erase any existing daemon info */
	اگر ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), بنیاد))) {
		مطابقت = مطابقت_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		جب ((d = readdir(dp))) {
			اگر (d->d_name[0] == '.')
				جاری;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), بنیاد, d->d_name);
			اگر (rc_stringlist_ڈھونڈو(renamelist, file)) {
				آزاد(file);
				جاری;
			}

			nfiles++;

			اگر (!*oldfile) {
				اگر (مطابقت_daemon(بنیاد, d->d_name, مطابقت)) {
					unlink(file);
					strlcpy(oldfile, file, حجم(oldfile));
					nfiles--;
				}
			} ورنہ {
				rename(file, oldfile);
				strlcpy(oldfile, file, حجم(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_جوڑو(renamelist, oldfile);
			}
			آزاد(file);
		}
		closedir(dp);
		rc_stringlist_free(مطابقت);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	اگر (!started)
		واپس true;

	اگر (mkdirat(rc_dirfd(RC_DIR_DAEMONS), بنیاد, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), بنیاد, nfiles + 1);
		اگر ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			اگر (exec)
				fprintf(fp, "%s", exec);
			جب (argv && argv[i]) {
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

	واپس retval;
}

bool
rc_service_started_daemon(ثابت حرف *service,
    ثابت حرف *exec, ثابت حرف *ثابت *argv, صحیح indx)
{
	ثابت حرف *بنیاد = basename_c(service);
	حرف *file = تہی;
	RC_STRINGLIST *مطابقت;
	bool retval = false;
	DIR *dp;
	ساخت dirent *d;

	اگر (!service || !exec)
		واپس false;

	مطابقت = مطابقت_list(exec, argv, تہی);

	اگر (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = مطابقت_daemon(بنیاد, file, مطابقت);
		آزاد(file);
	} ورنہ اگر ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), بنیاد))) {
		جب ((d = readdir(dp))) {
			اگر (d->d_name[0] == '.')
				جاری;
			اگر ((retval = مطابقت_daemon(بنیاد, d->d_name, مطابقت)))
				توڑو;
		}
		closedir(dp);
	}

	rc_stringlist_free(مطابقت);
	واپس retval;
}

bool
rc_service_daemons_crashed(ثابت حرف *service)
{
	DIR *dp;
	ساخت dirent *d;
	FILE *fp;
	حرف *سطر = تہی;
	size_t len = 0;
	حرف **argv = تہی;
	حرف *exec = تہی;
	حرف *name = تہی;
	حرف *pidfile = تہی;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	حرف *p;
	حرف *token;
	bool retval = false;
	RC_STRINGLIST *list = تہی;
	RC_STRING *s;
	size_t i;
	حرف *ch_جڑ;
	حرف *spidfile;

	اگر (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		واپس false;

	جب ((d = readdir(dp))) {
		اگر (d->d_name[0] == '.')
			جاری;

		اگر (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			توڑو;

		جب (xgetline(&سطر, &len, fp) != -1) {
			p = سطر;
			اگر ((token = strsep(&p, "=")) == تہی || !p)
				جاری;

			اگر (!*p)
				جاری;

			اگر (strcmp(token, "exec") == 0) {
				اگر (exec)
					آزاد(exec);
				exec = xstrdup(p);
			} ورنہ اگر (لڑی_موازنہ(token, "argv_", 5) == 0) {
				اگر (!list)
					list = rc_stringlist_new();
				rc_stringlist_جوڑو(list, p);
			} ورنہ اگر (strcmp(token, "name") == 0) {
				اگر (name)
					آزاد(name);
				name = xstrdup(p);
			} ورنہ اگر (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				توڑو;
			}
		}
		fclose(fp);

		ch_جڑ = rc_service_قدر_get(basename_c(service), "chroot");
		spidfile = pidfile;
		اگر (ch_جڑ && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_جڑ, pidfile);
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
			pidfile = تہی;

			/* We have the pid, so no need to match
			   on exec or name */
			آزاد(exec);
			exec = تہی;
			آزاد(name);
			name = تہی;
		} ورنہ {
			اگر (exec) {
				اگر (!list)
					list = rc_stringlist_new();
				اگر (!TAILQ_پہلا(list))
					rc_stringlist_جوڑو(list, exec);

				آزاد(exec);
				exec = تہی;
			}

			اگر (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(حجم(حرف *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->قدر;
				argv[i] = تہی;
			}
		}

		اگر (!retval) {
			اگر (pid != 0) {
				اگر (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} ورنہ اگر ((pids = rc_ڈھونڈو_pids(exec,
				    (ثابت حرف *ثابت *)argv,
				    0, pid)))
			{
				p1 = LIST_پہلا(pids);
				جب (p1) {
					p2 = LIST_اگلا(p1, entries);
					آزاد(p1);
					p1 = p2;
				}
				آزاد(pids);
			} ورنہ
				retval = true;
		}
		rc_stringlist_free(list);
		list = تہی;
		آزاد(argv);
		argv = تہی;
		آزاد(exec);
		exec = تہی;
		آزاد(name);
		name = تہی;
		اگر (retval)
			توڑو;
	}
	closedir(dp);
	آزاد(سطر);

	واپس retval;
}
