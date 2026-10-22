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

#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <dirent.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <limits.h>
#অন্তর্ভাৱ <signal.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <unistd.h>

#অন্তর্ভাৱ "queue.h"
#অন্তর্ভাৱ "librc.h"
#অন্তর্ভাৱ "helpers.h"

#যদি defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
স্থির bool
pid_is_exec(pid_t pid, ধ্রুৱ ৱর্ণ *exec)
{
	ৱর্ণ *buffer = রিক্ত;
	FILE *fp;
	পূর্ণাঙ্ক c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	যদি ((fp = fopen(buffer, "r"))) {
		যেতিয়ালৈ ((c = getc(fp)) != EOF && c != '(')
			;
		যদি (c == '(') {
			যেতিয়ালৈ ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			যদি (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	মুক্ত(buffer);
	ঘূৰাই retval;
}

স্থির bool
pid_is_argv(pid_t pid, ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv)
{
	ৱর্ণ *cmdline = রিক্ত;
	পূর্ণাঙ্ক fd;
	ৱর্ণ buffer[PATH_MAX];
	ৱর্ণ *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	যদি ((fd = open(cmdline, O_RDONLY)) < 0) {
		মুক্ত(cmdline);
		ঘূৰাই false;
	}
	bytes = পঠ(fd, buffer, পরিমাণ(buffer) - 1);
	close(fd);
	মুক্ত(cmdline);
	যদি (bytes == -1)
		ঘূৰাই false;

	buffer[bytes] = '\0';
	p = buffer;
	যেতিয়ালৈ (*argv) {
		যদি ((unsigned)(p - buffer) >= পরিমাণ(buffer))
			ঘূৰাই false;
		যদি (strcmp(*argv, p) != 0)
			ঘূৰাই false;
		argv++;
		p += strlen(p) + 1;
	}
	ঘূৰাই true;
}

RC_PIDLIST *
rc_find_pids(ধ্রুৱ ৱর্ণ *exec, ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	সংরচনা dirent *entry;
	FILE *fp;
	পূর্ণাঙ্ক rc;
	bool container_pid = false;
	bool openvz_host = false;
	ৱর্ণ *সাৰি = রিক্ত;
	ৱর্ণ my_ns[30] = {0};
	ৱর্ণ proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	ৱর্ণ *buffer = রিক্ত;
	সংরচনা stat sb;
	pid_t openrc_pid = 0;
	ৱর্ণ *pp;
	RC_PIDLIST *pids = রিক্ত;
	RC_PID *pi;

	যদি ((procdir = opendir("/proc")) == রিক্ত)
		ঘূৰাই রিক্ত;

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
		যেতিয়ালৈ (xgetline(&সাৰি, &len, fp) != -1) {
			যদি (strncmp(সাৰি, "envID:\t0", 8) == 0) {
				openvz_host = true;
				ভঙ্গ;
			}
		}
		fclose(fp);
	}

	যদি ((rc = readlink("/proc/self/ns/pid", my_ns, পরিমাণ(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	যেতিয়ালৈ ((entry = readdir(procdir)) != রিক্ত) {
		যদি (sscanf(entry->d_name, "%d", &p) != 1)
			অনুৱর্ত;
		যদি (openrc_pid != 0 && openrc_pid == p)
			অনুৱর্ত;
		যদি (pid != 0 && pid != p)
			অনুৱর্ত;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		যদি ((rc = readlink(buffer, proc_ns, পরিমাণ(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		মুক্ত(buffer);
		যদি (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			অনুৱর্ত;
		যদি (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			যদি (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				মুক্ত(buffer);
				অনুৱর্ত;
			}
			মুক্ত(buffer);
		}
		যদি (exec && !pid_is_exec(p, exec))
			অনুৱর্ত;
		যদি (argv &&
		    !pid_is_argv(p, (ধ্রুৱ ৱর্ণ *ধ্রুৱ *)argv))
			অনুৱর্ত;
		/* If this is an OpenVZ host, filter out container processes */
		যদি (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			মুক্ত(buffer);
			যদি (!fp)
				অনুৱর্ত;
			যেতিয়ালৈ (xgetline(&সাৰি, &len, fp) != -1) {
				যদি (strncmp(সাৰি, "envID:", 6) == 0) {
					container_pid = !(strncmp(সাৰি, "envID:\t0", 8) == 0);
					ভঙ্গ;
				}
			}
			fclose(fp);
		}
		যদি (container_pid)
			অনুৱর্ত;
		যদি (!pids) {
			pids = xmalloc(পরিমাণ(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(পরিমাণ(*pi));
		pi->pid = p;
		LIST_ঢোকাও_HEAD(pids, pi, entries);
	}
	যদি (সাৰি != রিক্ত)
		মুক্ত(সাৰি);
	closedir(procdir);
	ঘূৰাই pids;
}

#নোচেদ্যদি BSD

# যদি defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH রিক্ত
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
rc_find_pids(ধ্রুৱ ৱর্ণ *exec, ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv, uid_t uid, pid_t pid)
{
	স্থির kvm_t *kd = রিক্ত;
	ৱর্ণ errbuf[_POSIX2_LINE_MAX];
	সংরচনা _KINFO_PROC *kp;
	পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক processes = 0;
	পূর্ণাঙ্ক pargc = 0;
	ৱর্ণ **pargv;
	RC_PIDLIST *pids = রিক্ত;
	RC_PID *pi;
	pid_t p;
	ধ্রুৱ ৱর্ণ *ধ্রুৱ *arg;
	পূর্ণাঙ্ক match;

	যদি ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    রিক্ত, _KVM_FLAGS, errbuf)) == রিক্ত)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		ঘূৰাই রিক্ত;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, পরিমাণ(*kp), &processes);
#অন্যথা
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	যদি ((kp == রিক্ত && processes > 0) || (kp != রিক্ত && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		ঘূৰাই রিক্ত;
	}

	যদি (exec)
		exec = basename_c(exec);
	বাবে (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		যদি (pid != 0 && pid != p)
			অনুৱর্ত;
		যদি (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			অনুৱর্ত;
		যদি (exec) {
			যদি (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				অনুৱর্ত;
		}
		যদি (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			যদি (!pargv || !*pargv)
				অনুৱর্ত;
			arg = argv;
			match = 1;
			যেতিয়ালৈ (*arg && *pargv)
				যদি (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					ভঙ্গ;
				}
			যদি (!match)
				অনুৱর্ত;
		}
		যদি (!pids) {
			pids = xmalloc(পরিমাণ(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(পরিমাণ(*pi));
		pi->pid = p;
		LIST_ঢোকাও_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	ঘূৰাই pids;
}

#অন্যথা
#  error "Platform not supported!"
#endif

স্থির bool
_match_daemon(ধ্রুৱ ৱর্ণ *svcname, ধ্রুৱ ৱর্ণ *instance, RC_STRINGLIST *match)
{
	ৱর্ণ *সাৰি = রিক্ত;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	ৱর্ণ *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	মুক্ত(daemon);

	যদি (!fp)
		ঘূৰাই false;

	যেতিয়ালৈ (xgetline(&সাৰি, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    যদি (strcmp(সাৰি, m->মান) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    ভঙ্গ;
		    }
		যদি (!TAILQ_FIRST(match))
			ভঙ্গ;
	}
	fclose(fp);
	মুক্ত(সাৰি);
	যদি (TAILQ_FIRST(match))
		ঘূৰাই false;
	ঘূৰাই true;
}

স্থির RC_STRINGLIST *
_match_list(ধ্রুৱ ৱর্ণ *exec, ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv, ধ্রুৱ ৱর্ণ *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	পূর্ণাঙ্ক i = 0;
	ৱর্ণ *m;

	যদি (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		মুক্ত(m);
	}

	যেতিয়ালৈ (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		মুক্ত(m);
	}

	যদি (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		মুক্ত(m);
	}

	ঘূৰাই match;
}

bool
rc_service_daemon_set(ধ্রুৱ ৱর্ণ *service, ধ্রুৱ ৱর্ণ *exec,
    ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv,
    ধ্রুৱ ৱর্ণ *pidfile, bool started)
{
	ৱর্ণ *file = রিক্ত;
	পূর্ণাঙ্ক nfiles = 0;
	ৱর্ণ oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	সংরচনা dirent *d;
	RC_STRINGLIST *match, *renamelist;
	পূর্ণাঙ্ক i = 0;
	FILE *fp;
	ধ্রুৱ ৱর্ণ *base = basename_c(service);

	যদি (!exec && !pidfile) {
		errno = EINVAL;
		ঘূৰাই false;
	}

	/* Regardless, erase any existing daemon info */
	যদি ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		যেতিয়ালৈ ((d = readdir(dp))) {
			যদি (d->d_name[0] == '.')
				অনুৱর্ত;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			যদি (rc_stringlist_find(renamelist, file)) {
				মুক্ত(file);
				অনুৱর্ত;
			}

			nfiles++;

			যদি (!*oldfile) {
				যদি (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, পরিমাণ(oldfile));
					nfiles--;
				}
			} অন্যথা {
				rename(file, oldfile);
				strlcpy(oldfile, file, পরিমাণ(oldfile));
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
		ঘূৰাই true;

	যদি (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		যদি ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			যদি (exec)
				fprintf(fp, "%s", exec);
			যেতিয়ালৈ (argv && argv[i]) {
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

	ঘূৰাই retval;
}

bool
rc_service_started_daemon(ধ্রুৱ ৱর্ণ *service,
    ধ্রুৱ ৱর্ণ *exec, ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv, পূর্ণাঙ্ক indx)
{
	ধ্রুৱ ৱর্ণ *base = basename_c(service);
	ৱর্ণ *file = রিক্ত;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	সংরচনা dirent *d;

	যদি (!service || !exec)
		ঘূৰাই false;

	match = _match_list(exec, argv, রিক্ত);

	যদি (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		মুক্ত(file);
	} অন্যথা যদি ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		যেতিয়ালৈ ((d = readdir(dp))) {
			যদি (d->d_name[0] == '.')
				অনুৱর্ত;
			যদি ((retval = _match_daemon(base, d->d_name, match)))
				ভঙ্গ;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	ঘূৰাই retval;
}

bool
rc_service_daemons_crashed(ধ্রুৱ ৱর্ণ *service)
{
	DIR *dp;
	সংরচনা dirent *d;
	FILE *fp;
	ৱর্ণ *সাৰি = রিক্ত;
	size_t len = 0;
	ৱর্ণ **argv = রিক্ত;
	ৱর্ণ *exec = রিক্ত;
	ৱর্ণ *name = রিক্ত;
	ৱর্ণ *pidfile = রিক্ত;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	ৱর্ণ *p;
	ৱর্ণ *token;
	bool retval = false;
	RC_STRINGLIST *list = রিক্ত;
	RC_STRING *s;
	size_t i;
	ৱর্ণ *ch_মূল;
	ৱর্ণ *spidfile;

	যদি (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		ঘূৰাই false;

	যেতিয়ালৈ ((d = readdir(dp))) {
		যদি (d->d_name[0] == '.')
			অনুৱর্ত;

		যদি (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			ভঙ্গ;

		যেতিয়ালৈ (xgetline(&সাৰি, &len, fp) != -1) {
			p = সাৰি;
			যদি ((token = strsep(&p, "=")) == রিক্ত || !p)
				অনুৱর্ত;

			যদি (!*p)
				অনুৱর্ত;

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
				ভঙ্গ;
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
			pidfile = রিক্ত;

			/* We have the pid, so no need to match
			   on exec or name */
			মুক্ত(exec);
			exec = রিক্ত;
			মুক্ত(name);
			name = রিক্ত;
		} অন্যথা {
			যদি (exec) {
				যদি (!list)
					list = rc_stringlist_new();
				যদি (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				মুক্ত(exec);
				exec = রিক্ত;
			}

			যদি (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(পরিমাণ(ৱর্ণ *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->মান;
				argv[i] = রিক্ত;
			}
		}

		যদি (!retval) {
			যদি (pid != 0) {
				যদি (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} অন্যথা যদি ((pids = rc_find_pids(exec,
				    (ধ্রুৱ ৱর্ণ *ধ্রুৱ *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				যেতিয়ালৈ (p1) {
					p2 = LIST_NEXT(p1, entries);
					মুক্ত(p1);
					p1 = p2;
				}
				মুক্ত(pids);
			} অন্যথা
				retval = true;
		}
		rc_stringlist_free(list);
		list = রিক্ত;
		মুক্ত(argv);
		argv = রিক্ত;
		মুক্ত(exec);
		exec = রিক্ত;
		মুক্ত(name);
		name = রিক্ত;
		যদি (retval)
			ভঙ্গ;
	}
	closedir(dp);
	মুক্ত(সাৰি);

	ঘূৰাই retval;
}
