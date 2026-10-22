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

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <dirent.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "librc.h"
#অন্তর্ভাব "helpers.h"

#যদি defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
স্থির bool
pid_is_exec(pid_t pid, ধ্রুবক অক্ষর *exec)
{
	অক্ষর *বাফার = ফাঁকা;
	FILE *fp;
	পূর্ণসংখ্যা c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&বাফার, "/proc/%d/stat", pid);
	যদি ((fp = fopen(বাফার, "r"))) {
		যখন ((c = getc(fp)) != EOF && c != '(')
			;
		যদি (c == '(') {
			যখন ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			যদি (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	মুক্ত(বাফার);
	ফেরত retval;
}

স্থির bool
pid_is_argv(pid_t pid, ধ্রুবক অক্ষর *ধ্রুবক *argv)
{
	অক্ষর *cmdline = ফাঁকা;
	পূর্ণসংখ্যা fd;
	অক্ষর বাফার[PATH_MAX];
	অক্ষর *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	যদি ((fd = open(cmdline, O_RDONLY)) < 0) {
		মুক্ত(cmdline);
		ফেরত false;
	}
	bytes = পঠ(fd, বাফার, আকার(বাফার) - 1);
	close(fd);
	মুক্ত(cmdline);
	যদি (bytes == -1)
		ফেরত false;

	বাফার[bytes] = '\0';
	p = বাফার;
	যখন (*argv) {
		যদি ((unsigned)(p - বাফার) >= আকার(বাফার))
			ফেরত false;
		যদি (strcmp(*argv, p) != 0)
			ফেরত false;
		argv++;
		p += strlen(p) + 1;
	}
	ফেরত true;
}

RC_PIDLIST *
rc_find_pids(ধ্রুবক অক্ষর *exec, ধ্রুবক অক্ষর *ধ্রুবক *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	কাঠামো dirent *entry;
	FILE *fp;
	পূর্ণসংখ্যা rc;
	bool container_pid = false;
	bool openvz_host = false;
	অক্ষর *সারি = ফাঁকা;
	অক্ষর my_ns[30] = {0};
	অক্ষর proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	অক্ষর *বাফার = ফাঁকা;
	কাঠামো stat sb;
	pid_t openrc_pid = 0;
	অক্ষর *pp;
	RC_PIDLIST *pids = ফাঁকা;
	RC_PID *pi;

	যদি ((procdir = opendir("/proc")) == ফাঁকা)
		ফেরত ফাঁকা;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	যদি ((pp = getenv("RC_OPENRC_PID"))) {
		যদি (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	যদি ((fp = fopen("/proc/self/status", "r"))) {
		যখন (xgetline(&সারি, &len, fp) != -1) {
			যদি (strncmp(সারি, "envID:\t0", 8) == 0) {
				openvz_host = true;
				বিরতি;
			}
		}
		fclose(fp);
	}

	যদি ((rc = readlink("/proc/self/ns/pid", my_ns, আকার(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	যখন ((entry = readdir(procdir)) != ফাঁকা) {
		যদি (sscanf(entry->d_name, "%d", &p) != 1)
			চালু;
		যদি (openrc_pid != 0 && openrc_pid == p)
			চালু;
		যদি (pid != 0 && pid != p)
			চালু;
		xasprintf(&বাফার, "/proc/%d/ns/pid", p);
		যদি ((rc = readlink(বাফার, proc_ns, আকার(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		মুক্ত(বাফার);
		যদি (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			চালু;
		যদি (uid) {
			xasprintf(&বাফার, "/proc/%d", p);
			যদি (stat(বাফার, &sb) != 0 || sb.st_uid != uid) {
				মুক্ত(বাফার);
				চালু;
			}
			মুক্ত(বাফার);
		}
		যদি (exec && !pid_is_exec(p, exec))
			চালু;
		যদি (argv &&
		    !pid_is_argv(p, (ধ্রুবক অক্ষর *ধ্রুবক *)argv))
			চালু;
		/* If this is an OpenVZ host, filter out container processes */
		যদি (openvz_host) {
			xasprintf(&বাফার, "/proc/%d/status", p);
			fp = fopen(বাফার, "r");
			মুক্ত(বাফার);
			যদি (!fp)
				চালু;
			যখন (xgetline(&সারি, &len, fp) != -1) {
				যদি (strncmp(সারি, "envID:", 6) == 0) {
					container_pid = !(strncmp(সারি, "envID:\t0", 8) == 0);
					বিরতি;
				}
			}
			fclose(fp);
		}
		যদি (container_pid)
			চালু;
		যদি (!pids) {
			pids = xmalloc(আকার(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(আকার(*pi));
		pi->pid = p;
		LIST_ঢোকাও_শীর্ষ(pids, pi, entries);
	}
	যদি (সারি != ফাঁকা)
		মুক্ত(সারি);
	closedir(procdir);
	ফেরত pids;
}

#নতুবা_যদি BSD

# যদি defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH ফাঁকা
#  define _KVM_FLAGS KVM_NO_FILES
# অন্যথা
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  যদি defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  অন্যথা
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define _KVM_PATH _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_find_pids(ধ্রুবক অক্ষর *exec, ধ্রুবক অক্ষর *ধ্রুবক *argv, uid_t uid, pid_t pid)
{
	স্থির kvm_t *kd = ফাঁকা;
	অক্ষর errbuf[_POSIX2_LINE_MAX];
	কাঠামো _KINFO_PROC *kp;
	পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা processes = 0;
	পূর্ণসংখ্যা pargc = 0;
	অক্ষর **pargv;
	RC_PIDLIST *pids = ফাঁকা;
	RC_PID *pi;
	pid_t p;
	ধ্রুবক অক্ষর *ধ্রুবক *arg;
	পূর্ণসংখ্যা match;

	যদি ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    ফাঁকা, _KVM_FLAGS, errbuf)) == ফাঁকা)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		ফেরত ফাঁকা;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, আকার(*kp), &processes);
#অন্যথা
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	যদি ((kp == ফাঁকা && processes > 0) || (kp != ফাঁকা && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		ফেরত ফাঁকা;
	}

	যদি (exec)
		exec = basename_c(exec);
	জন্য (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		যদি (pid != 0 && pid != p)
			চালু;
		যদি (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			চালু;
		যদি (exec) {
			যদি (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				চালু;
		}
		যদি (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			যদি (!pargv || !*pargv)
				চালু;
			arg = argv;
			match = 1;
			যখন (*arg && *pargv)
				যদি (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					বিরতি;
				}
			যদি (!match)
				চালু;
		}
		যদি (!pids) {
			pids = xmalloc(আকার(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(আকার(*pi));
		pi->pid = p;
		LIST_ঢোকাও_শীর্ষ(pids, pi, entries);
	}
	kvm_close(kd);

	ফেরত pids;
}

#অন্যথা
#  error "Platform not supported!"
#endif

স্থির bool
_match_daemon(ধ্রুবক অক্ষর *svcname, ধ্রুবক অক্ষর *instance, RC_STRINGLIST *match)
{
	অক্ষর *সারি = ফাঁকা;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	অক্ষর *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	মুক্ত(daemon);

	যদি (!fp)
		ফেরত false;

	যখন (xgetline(&সারি, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    যদি (strcmp(সারি, m->মান) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    বিরতি;
		    }
		যদি (!TAILQ_FIRST(match))
			বিরতি;
	}
	fclose(fp);
	মুক্ত(সারি);
	যদি (TAILQ_FIRST(match))
		ফেরত false;
	ফেরত true;
}

স্থির RC_STRINGLIST *
_match_list(ধ্রুবক অক্ষর *exec, ধ্রুবক অক্ষর *ধ্রুবক *argv, ধ্রুবক অক্ষর *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	পূর্ণসংখ্যা i = 0;
	অক্ষর *m;

	যদি (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		মুক্ত(m);
	}

	যখন (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		মুক্ত(m);
	}

	যদি (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		মুক্ত(m);
	}

	ফেরত match;
}

bool
rc_service_daemon_set(ধ্রুবক অক্ষর *service, ধ্রুবক অক্ষর *exec,
    ধ্রুবক অক্ষর *ধ্রুবক *argv,
    ধ্রুবক অক্ষর *pidfile, bool started)
{
	অক্ষর *file = ফাঁকা;
	পূর্ণসংখ্যা nfiles = 0;
	অক্ষর oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	কাঠামো dirent *d;
	RC_STRINGLIST *match, *renamelist;
	পূর্ণসংখ্যা i = 0;
	FILE *fp;
	ধ্রুবক অক্ষর *base = basename_c(service);

	যদি (!exec && !pidfile) {
		errno = EINVAL;
		ফেরত false;
	}

	/* Regardless, erase any existing daemon info */
	যদি ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		যখন ((d = readdir(dp))) {
			যদি (d->d_name[0] == '.')
				চালু;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			যদি (rc_stringlist_find(renamelist, file)) {
				মুক্ত(file);
				চালু;
			}

			nfiles++;

			যদি (!*oldfile) {
				যদি (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, আকার(oldfile));
					nfiles--;
				}
			} অন্যথা {
				rename(file, oldfile);
				strlcpy(oldfile, file, আকার(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_add(renamelist, oldfile);
			}
			মুক্ত(file);
		}
		closedir(dp);
		rc_stringlist_free(match);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	যদি (!started)
		ফেরত true;

	যদি (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		যদি ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			যদি (exec)
				fprintf(fp, "%s", exec);
			যখন (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			যদি (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		মুক্ত(file);
	}

	ফেরত retval;
}

bool
rc_service_started_daemon(ধ্রুবক অক্ষর *service,
    ধ্রুবক অক্ষর *exec, ধ্রুবক অক্ষর *ধ্রুবক *argv, পূর্ণসংখ্যা indx)
{
	ধ্রুবক অক্ষর *base = basename_c(service);
	অক্ষর *file = ফাঁকা;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	কাঠামো dirent *d;

	যদি (!service || !exec)
		ফেরত false;

	match = _match_list(exec, argv, ফাঁকা);

	যদি (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		মুক্ত(file);
	} অন্যথা যদি ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		যখন ((d = readdir(dp))) {
			যদি (d->d_name[0] == '.')
				চালু;
			যদি ((retval = _match_daemon(base, d->d_name, match)))
				বিরতি;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	ফেরত retval;
}

bool
rc_service_daemons_crashed(ধ্রুবক অক্ষর *service)
{
	DIR *dp;
	কাঠামো dirent *d;
	FILE *fp;
	অক্ষর *সারি = ফাঁকা;
	size_t len = 0;
	অক্ষর **argv = ফাঁকা;
	অক্ষর *exec = ফাঁকা;
	অক্ষর *name = ফাঁকা;
	অক্ষর *pidfile = ফাঁকা;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	অক্ষর *p;
	অক্ষর *token;
	bool retval = false;
	RC_STRINGLIST *list = ফাঁকা;
	RC_STRING *s;
	size_t i;
	অক্ষর *ch_মূল;
	অক্ষর *spidfile;

	যদি (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		ফেরত false;

	যখন ((d = readdir(dp))) {
		যদি (d->d_name[0] == '.')
			চালু;

		যদি (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			বিরতি;

		যখন (xgetline(&সারি, &len, fp) != -1) {
			p = সারি;
			যদি ((token = strsep(&p, "=")) == ফাঁকা || !p)
				চালু;

			যদি (!*p)
				চালু;

			যদি (strcmp(token, "exec") == 0) {
				যদি (exec)
					মুক্ত(exec);
				exec = xstrdup(p);
			} অন্যথা যদি (strncmp(token, "argv_", 5) == 0) {
				যদি (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} অন্যথা যদি (strcmp(token, "name") == 0) {
				যদি (name)
					মুক্ত(name);
				name = xstrdup(p);
			} অন্যথা যদি (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				বিরতি;
			}
		}
		fclose(fp);

		ch_মূল = rc_service_মান_get(basename_c(service), "chroot");
		spidfile = pidfile;
		যদি (ch_মূল && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_মূল, pidfile);
			মুক্ত(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		যদি (pidfile) {
			retval = true;
			যদি ((fp = fopen(pidfile, "r"))) {
				যদি (fscanf(fp, "%d", &pid) == 1)
					retval = false;
				fclose(fp);
			}
			মুক্ত(pidfile);
			pidfile = ফাঁকা;

			/* We have the pid, so no need to match
			   on exec or name */
			মুক্ত(exec);
			exec = ফাঁকা;
			মুক্ত(name);
			name = ফাঁকা;
		} অন্যথা {
			যদি (exec) {
				যদি (!list)
					list = rc_stringlist_new();
				যদি (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				মুক্ত(exec);
				exec = ফাঁকা;
			}

			যদি (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(আকার(অক্ষর *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->মান;
				argv[i] = ফাঁকা;
			}
		}

		যদি (!retval) {
			যদি (pid != 0) {
				যদি (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} অন্যথা যদি ((pids = rc_find_pids(exec,
				    (ধ্রুবক অক্ষর *ধ্রুবক *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				যখন (p1) {
					p2 = LIST_পরবর্তী(p1, entries);
					মুক্ত(p1);
					p1 = p2;
				}
				মুক্ত(pids);
			} অন্যথা
				retval = true;
		}
		rc_stringlist_free(list);
		list = ফাঁকা;
		মুক্ত(argv);
		argv = ফাঁকা;
		মুক্ত(exec);
		exec = ফাঁকা;
		মুক্ত(name);
		name = ফাঁকা;
		যদি (retval)
			বিরতি;
	}
	closedir(dp);
	মুক্ত(সারি);

	ফেরত retval;
}
