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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <dirent.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "queue.h"
#अन्तर्भाव "librc.h"
#अन्तर्भाव "helpers.h"

#सचे defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
स्थिर bool
pid_is_exec(pid_t pid, ध्रुव वर्ण *exec)
{
	वर्ण *buffer = रिक्त;
	FILE *fp;
	पूर्णाङ्क c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	सचे ((fp = fopen(buffer, "r"))) {
		याव ((c = getc(fp)) != EOF && c != '(')
			;
		सचे (c == '(') {
			याव ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			सचे (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	मुक्त(buffer);
	पटिनिवत्त retval;
}

स्थिर bool
pid_is_argv(pid_t pid, ध्रुव वर्ण *ध्रुव *argv)
{
	वर्ण *cmdline = रिक्त;
	पूर्णाङ्क fd;
	वर्ण buffer[PATH_MAX];
	वर्ण *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	सचे ((fd = open(cmdline, O_RDONLY)) < 0) {
		मुक्त(cmdline);
		पटिनिवत्त false;
	}
	bytes = पठ(fd, buffer, परिमाण(buffer) - 1);
	close(fd);
	मुक्त(cmdline);
	सचे (bytes == -1)
		पटिनिवत्त false;

	buffer[bytes] = '\0';
	p = buffer;
	याव (*argv) {
		सचे ((unsigned)(p - buffer) >= परिमाण(buffer))
			पटिनिवत्त false;
		सचे (strcmp(*argv, p) != 0)
			पटिनिवत्त false;
		argv++;
		p += strlen(p) + 1;
	}
	पटिनिवत्त true;
}

RC_PIDLIST *
rc_find_pids(ध्रुव वर्ण *exec, ध्रुव वर्ण *ध्रुव *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	संरचना dirent *entry;
	FILE *fp;
	पूर्णाङ्क rc;
	bool container_pid = false;
	bool openvz_host = false;
	वर्ण *पङ्क्तिः = रिक्त;
	वर्ण my_ns[30] = {0};
	वर्ण proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	वर्ण *buffer = रिक्त;
	संरचना stat sb;
	pid_t openrc_pid = 0;
	वर्ण *pp;
	RC_PIDLIST *pids = रिक्त;
	RC_PID *pi;

	सचे ((procdir = opendir("/proc")) == रिक्त)
		पटिनिवत्त रिक्त;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	सचे ((pp = getenv("RC_OPENRC_PID"))) {
		सचे (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	सचे ((fp = fopen("/proc/self/status", "r"))) {
		याव (xgetline(&पङ्क्तिः, &len, fp) != -1) {
			सचे (strncmp(पङ्क्तिः, "envID:\t0", 8) == 0) {
				openvz_host = true;
				भङ्ग;
			}
		}
		fclose(fp);
	}

	सचे ((rc = readlink("/proc/self/ns/pid", my_ns, परिमाण(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	याव ((entry = readdir(procdir)) != रिक्त) {
		सचे (sscanf(entry->d_name, "%d", &p) != 1)
			अनुवर्त;
		सचे (openrc_pid != 0 && openrc_pid == p)
			अनुवर्त;
		सचे (pid != 0 && pid != p)
			अनुवर्त;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		सचे ((rc = readlink(buffer, proc_ns, परिमाण(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		मुक्त(buffer);
		सचे (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			अनुवर्त;
		सचे (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			सचे (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				मुक्त(buffer);
				अनुवर्त;
			}
			मुक्त(buffer);
		}
		सचे (exec && !pid_is_exec(p, exec))
			अनुवर्त;
		सचे (argv &&
		    !pid_is_argv(p, (ध्रुव वर्ण *ध्रुव *)argv))
			अनुवर्त;
		/* If this is an OpenVZ host, filter out container processes */
		सचे (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			मुक्त(buffer);
			सचे (!fp)
				अनुवर्त;
			याव (xgetline(&पङ्क्तिः, &len, fp) != -1) {
				सचे (strncmp(पङ्क्तिः, "envID:", 6) == 0) {
					container_pid = !(strncmp(पङ्क्तिः, "envID:\t0", 8) == 0);
					भङ्ग;
				}
			}
			fclose(fp);
		}
		सचे (container_pid)
			अनुवर्त;
		सचे (!pids) {
			pids = xmalloc(परिमाण(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(परिमाण(*pi));
		pi->pid = p;
		LIST_निवेशय_HEAD(pids, pi, entries);
	}
	सचे (पङ्क्तिः != रिक्त)
		मुक्त(पङ्क्तिः);
	closedir(procdir);
	पटिनिवत्त pids;
}

#नोचेद्यदि BSD

# सचे defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH रिक्त
#  define _KVM_FLAGS KVM_NO_FILES
# अञ्ञथा
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  सचे defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  अञ्ञथा
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define _KVM_PATH _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_find_pids(ध्रुव वर्ण *exec, ध्रुव वर्ण *ध्रुव *argv, uid_t uid, pid_t pid)
{
	स्थिर kvm_t *kd = रिक्त;
	वर्ण errbuf[_POSIX2_LINE_MAX];
	संरचना _KINFO_PROC *kp;
	पूर्णाङ्क i;
	पूर्णाङ्क processes = 0;
	पूर्णाङ्क pargc = 0;
	वर्ण **pargv;
	RC_PIDLIST *pids = रिक्त;
	RC_PID *pi;
	pid_t p;
	ध्रुव वर्ण *ध्रुव *arg;
	पूर्णाङ्क match;

	सचे ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    रिक्त, _KVM_FLAGS, errbuf)) == रिक्त)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		पटिनिवत्त रिक्त;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, परिमाण(*kp), &processes);
#अञ्ञथा
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	सचे ((kp == रिक्त && processes > 0) || (kp != रिक्त && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		पटिनिवत्त रिक्त;
	}

	सचे (exec)
		exec = basename_c(exec);
	अत्थाय (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		सचे (pid != 0 && pid != p)
			अनुवर्त;
		सचे (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			अनुवर्त;
		सचे (exec) {
			सचे (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				अनुवर्त;
		}
		सचे (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			सचे (!pargv || !*pargv)
				अनुवर्त;
			arg = argv;
			match = 1;
			याव (*arg && *pargv)
				सचे (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					भङ्ग;
				}
			सचे (!match)
				अनुवर्त;
		}
		सचे (!pids) {
			pids = xmalloc(परिमाण(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(परिमाण(*pi));
		pi->pid = p;
		LIST_निवेशय_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	पटिनिवत्त pids;
}

#अञ्ञथा
#  error "Platform not supported!"
#endif

स्थिर bool
_match_daemon(ध्रुव वर्ण *svcname, ध्रुव वर्ण *instance, RC_STRINGLIST *match)
{
	वर्ण *पङ्क्तिः = रिक्त;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	वर्ण *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	मुक्त(daemon);

	सचे (!fp)
		पटिनिवत्त false;

	याव (xgetline(&पङ्क्तिः, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    सचे (strcmp(पङ्क्तिः, m->मानम्) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    भङ्ग;
		    }
		सचे (!TAILQ_FIRST(match))
			भङ्ग;
	}
	fclose(fp);
	मुक्त(पङ्क्तिः);
	सचे (TAILQ_FIRST(match))
		पटिनिवत्त false;
	पटिनिवत्त true;
}

स्थिर RC_STRINGLIST *
_match_list(ध्रुव वर्ण *exec, ध्रुव वर्ण *ध्रुव *argv, ध्रुव वर्ण *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	पूर्णाङ्क i = 0;
	वर्ण *m;

	सचे (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		मुक्त(m);
	}

	याव (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		मुक्त(m);
	}

	सचे (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		मुक्त(m);
	}

	पटिनिवत्त match;
}

bool
rc_service_daemon_set(ध्रुव वर्ण *service, ध्रुव वर्ण *exec,
    ध्रुव वर्ण *ध्रुव *argv,
    ध्रुव वर्ण *pidfile, bool started)
{
	वर्ण *file = रिक्त;
	पूर्णाङ्क nfiles = 0;
	वर्ण oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	संरचना dirent *d;
	RC_STRINGLIST *match, *renamelist;
	पूर्णाङ्क i = 0;
	FILE *fp;
	ध्रुव वर्ण *base = basename_c(service);

	सचे (!exec && !pidfile) {
		errno = EINVAL;
		पटिनिवत्त false;
	}

	/* Regardless, erase any existing daemon info */
	सचे ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		याव ((d = readdir(dp))) {
			सचे (d->d_name[0] == '.')
				अनुवर्त;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			सचे (rc_stringlist_find(renamelist, file)) {
				मुक्त(file);
				अनुवर्त;
			}

			nfiles++;

			सचे (!*oldfile) {
				सचे (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, परिमाण(oldfile));
					nfiles--;
				}
			} अञ्ञथा {
				rename(file, oldfile);
				strlcpy(oldfile, file, परिमाण(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_add(renamelist, oldfile);
			}
			मुक्त(file);
		}
		closedir(dp);
		rc_stringlist_free(match);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	सचे (!started)
		पटिनिवत्त true;

	सचे (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		सचे ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			सचे (exec)
				fprintf(fp, "%s", exec);
			याव (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			सचे (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		मुक्त(file);
	}

	पटिनिवत्त retval;
}

bool
rc_service_started_daemon(ध्रुव वर्ण *service,
    ध्रुव वर्ण *exec, ध्रुव वर्ण *ध्रुव *argv, पूर्णाङ्क indx)
{
	ध्रुव वर्ण *base = basename_c(service);
	वर्ण *file = रिक्त;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	संरचना dirent *d;

	सचे (!service || !exec)
		पटिनिवत्त false;

	match = _match_list(exec, argv, रिक्त);

	सचे (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		मुक्त(file);
	} अञ्ञथा सचे ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		याव ((d = readdir(dp))) {
			सचे (d->d_name[0] == '.')
				अनुवर्त;
			सचे ((retval = _match_daemon(base, d->d_name, match)))
				भङ्ग;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	पटिनिवत्त retval;
}

bool
rc_service_daemons_crashed(ध्रुव वर्ण *service)
{
	DIR *dp;
	संरचना dirent *d;
	FILE *fp;
	वर्ण *पङ्क्तिः = रिक्त;
	size_t len = 0;
	वर्ण **argv = रिक्त;
	वर्ण *exec = रिक्त;
	वर्ण *name = रिक्त;
	वर्ण *pidfile = रिक्त;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	वर्ण *p;
	वर्ण *token;
	bool retval = false;
	RC_STRINGLIST *list = रिक्त;
	RC_STRING *s;
	size_t i;
	वर्ण *ch_मूलम्;
	वर्ण *spidfile;

	सचे (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		पटिनिवत्त false;

	याव ((d = readdir(dp))) {
		सचे (d->d_name[0] == '.')
			अनुवर्त;

		सचे (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			भङ्ग;

		याव (xgetline(&पङ्क्तिः, &len, fp) != -1) {
			p = पङ्क्तिः;
			सचे ((token = strsep(&p, "=")) == रिक्त || !p)
				अनुवर्त;

			सचे (!*p)
				अनुवर्त;

			सचे (strcmp(token, "exec") == 0) {
				सचे (exec)
					मुक्त(exec);
				exec = xstrdup(p);
			} अञ्ञथा सचे (strncmp(token, "argv_", 5) == 0) {
				सचे (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} अञ्ञथा सचे (strcmp(token, "name") == 0) {
				सचे (name)
					मुक्त(name);
				name = xstrdup(p);
			} अञ्ञथा सचे (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				भङ्ग;
			}
		}
		fclose(fp);

		ch_मूलम् = rc_service_मानम्_get(basename_c(service), "chroot");
		spidfile = pidfile;
		सचे (ch_मूलम् && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_मूलम्, pidfile);
			मुक्त(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		सचे (pidfile) {
			retval = true;
			सचे ((fp = fopen(pidfile, "r"))) {
				सचे (fscanf(fp, "%d", &pid) == 1)
					retval = false;
				fclose(fp);
			}
			मुक्त(pidfile);
			pidfile = रिक्त;

			/* We have the pid, so no need to match
			   on exec or name */
			मुक्त(exec);
			exec = रिक्त;
			मुक्त(name);
			name = रिक्त;
		} अञ्ञथा {
			सचे (exec) {
				सचे (!list)
					list = rc_stringlist_new();
				सचे (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				मुक्त(exec);
				exec = रिक्त;
			}

			सचे (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(परिमाण(वर्ण *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->मानम्;
				argv[i] = रिक्त;
			}
		}

		सचे (!retval) {
			सचे (pid != 0) {
				सचे (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} अञ्ञथा सचे ((pids = rc_find_pids(exec,
				    (ध्रुव वर्ण *ध्रुव *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				याव (p1) {
					p2 = LIST_NEXT(p1, entries);
					मुक्त(p1);
					p1 = p2;
				}
				मुक्त(pids);
			} अञ्ञथा
				retval = true;
		}
		rc_stringlist_free(list);
		list = रिक्त;
		मुक्त(argv);
		argv = रिक्त;
		मुक्त(exec);
		exec = रिक्त;
		मुक्त(name);
		name = रिक्त;
		सचे (retval)
			भङ्ग;
	}
	closedir(dp);
	मुक्त(पङ्क्तिः);

	पटिनिवत्त retval;
}
