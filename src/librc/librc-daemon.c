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

#जइ defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
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
	जइ ((fp = fopen(buffer, "r"))) {
		जाव ((c = getc(fp)) != EOF && c != '(')
			;
		जइ (c == '(') {
			जाव ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			जइ (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	मुक्त(buffer);
	प्रत्यागम retval;
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
	जइ ((fd = open(cmdline, O_RDONLY)) < 0) {
		मुक्त(cmdline);
		प्रत्यागम false;
	}
	bytes = पठ(fd, buffer, परिमाण(buffer) - 1);
	close(fd);
	मुक्त(cmdline);
	जइ (bytes == -1)
		प्रत्यागम false;

	buffer[bytes] = '\0';
	p = buffer;
	जाव (*argv) {
		जइ ((unsigned)(p - buffer) >= परिमाण(buffer))
			प्रत्यागम false;
		जइ (strcmp(*argv, p) != 0)
			प्रत्यागम false;
		argv++;
		p += strlen(p) + 1;
	}
	प्रत्यागम true;
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

	जइ ((procdir = opendir("/proc")) == रिक्त)
		प्रत्यागम रिक्त;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	जइ ((pp = getenv("RC_OPENRC_PID"))) {
		जइ (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	जइ ((fp = fopen("/proc/self/status", "r"))) {
		जाव (xgetline(&पङ्क्तिः, &len, fp) != -1) {
			जइ (strncmp(पङ्क्तिः, "envID:\t0", 8) == 0) {
				openvz_host = true;
				भङ्ग;
			}
		}
		fclose(fp);
	}

	जइ ((rc = readlink("/proc/self/ns/pid", my_ns, परिमाण(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	जाव ((entry = readdir(procdir)) != रिक्त) {
		जइ (sscanf(entry->d_name, "%d", &p) != 1)
			अनुवर्त;
		जइ (openrc_pid != 0 && openrc_pid == p)
			अनुवर्त;
		जइ (pid != 0 && pid != p)
			अनुवर्त;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		जइ ((rc = readlink(buffer, proc_ns, परिमाण(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		मुक्त(buffer);
		जइ (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			अनुवर्त;
		जइ (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			जइ (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				मुक्त(buffer);
				अनुवर्त;
			}
			मुक्त(buffer);
		}
		जइ (exec && !pid_is_exec(p, exec))
			अनुवर्त;
		जइ (argv &&
		    !pid_is_argv(p, (ध्रुव वर्ण *ध्रुव *)argv))
			अनुवर्त;
		/* If this is an OpenVZ host, filter out container processes */
		जइ (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			मुक्त(buffer);
			जइ (!fp)
				अनुवर्त;
			जाव (xgetline(&पङ्क्तिः, &len, fp) != -1) {
				जइ (strncmp(पङ्क्तिः, "envID:", 6) == 0) {
					container_pid = !(strncmp(पङ्क्तिः, "envID:\t0", 8) == 0);
					भङ्ग;
				}
			}
			fclose(fp);
		}
		जइ (container_pid)
			अनुवर्त;
		जइ (!pids) {
			pids = xmalloc(परिमाण(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(परिमाण(*pi));
		pi->pid = p;
		LIST_निवेशय_HEAD(pids, pi, entries);
	}
	जइ (पङ्क्तिः != रिक्त)
		मुक्त(पङ्क्तिः);
	closedir(procdir);
	प्रत्यागम pids;
}

#नोचेद्यदि BSD

# जइ defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH रिक्त
#  define _KVM_FLAGS KVM_NO_FILES
# अण्णहा
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  जइ defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  अण्णहा
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

	जइ ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    रिक्त, _KVM_FLAGS, errbuf)) == रिक्त)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		प्रत्यागम रिक्त;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, परिमाण(*kp), &processes);
#अण्णहा
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	जइ ((kp == रिक्त && processes > 0) || (kp != रिक्त && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		प्रत्यागम रिक्त;
	}

	जइ (exec)
		exec = basename_c(exec);
	प्रति (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		जइ (pid != 0 && pid != p)
			अनुवर्त;
		जइ (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			अनुवर्त;
		जइ (exec) {
			जइ (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				अनुवर्त;
		}
		जइ (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			जइ (!pargv || !*pargv)
				अनुवर्त;
			arg = argv;
			match = 1;
			जाव (*arg && *pargv)
				जइ (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					भङ्ग;
				}
			जइ (!match)
				अनुवर्त;
		}
		जइ (!pids) {
			pids = xmalloc(परिमाण(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(परिमाण(*pi));
		pi->pid = p;
		LIST_निवेशय_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	प्रत्यागम pids;
}

#अण्णहा
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

	जइ (!fp)
		प्रत्यागम false;

	जाव (xgetline(&पङ्क्तिः, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    जइ (strcmp(पङ्क्तिः, m->मानम्) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    भङ्ग;
		    }
		जइ (!TAILQ_FIRST(match))
			भङ्ग;
	}
	fclose(fp);
	मुक्त(पङ्क्तिः);
	जइ (TAILQ_FIRST(match))
		प्रत्यागम false;
	प्रत्यागम true;
}

स्थिर RC_STRINGLIST *
_match_list(ध्रुव वर्ण *exec, ध्रुव वर्ण *ध्रुव *argv, ध्रुव वर्ण *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	पूर्णाङ्क i = 0;
	वर्ण *m;

	जइ (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		मुक्त(m);
	}

	जाव (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		मुक्त(m);
	}

	जइ (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		मुक्त(m);
	}

	प्रत्यागम match;
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

	जइ (!exec && !pidfile) {
		errno = EINVAL;
		प्रत्यागम false;
	}

	/* Regardless, erase any existing daemon info */
	जइ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		जाव ((d = readdir(dp))) {
			जइ (d->d_name[0] == '.')
				अनुवर्त;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			जइ (rc_stringlist_find(renamelist, file)) {
				मुक्त(file);
				अनुवर्त;
			}

			nfiles++;

			जइ (!*oldfile) {
				जइ (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, परिमाण(oldfile));
					nfiles--;
				}
			} अण्णहा {
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
	जइ (!started)
		प्रत्यागम true;

	जइ (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		जइ ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			जइ (exec)
				fprintf(fp, "%s", exec);
			जाव (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			जइ (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		मुक्त(file);
	}

	प्रत्यागम retval;
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

	जइ (!service || !exec)
		प्रत्यागम false;

	match = _match_list(exec, argv, रिक्त);

	जइ (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		मुक्त(file);
	} अण्णहा जइ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		जाव ((d = readdir(dp))) {
			जइ (d->d_name[0] == '.')
				अनुवर्त;
			जइ ((retval = _match_daemon(base, d->d_name, match)))
				भङ्ग;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	प्रत्यागम retval;
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

	जइ (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		प्रत्यागम false;

	जाव ((d = readdir(dp))) {
		जइ (d->d_name[0] == '.')
			अनुवर्त;

		जइ (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			भङ्ग;

		जाव (xgetline(&पङ्क्तिः, &len, fp) != -1) {
			p = पङ्क्तिः;
			जइ ((token = strsep(&p, "=")) == रिक्त || !p)
				अनुवर्त;

			जइ (!*p)
				अनुवर्त;

			जइ (strcmp(token, "exec") == 0) {
				जइ (exec)
					मुक्त(exec);
				exec = xstrdup(p);
			} अण्णहा जइ (strncmp(token, "argv_", 5) == 0) {
				जइ (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} अण्णहा जइ (strcmp(token, "name") == 0) {
				जइ (name)
					मुक्त(name);
				name = xstrdup(p);
			} अण्णहा जइ (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				भङ्ग;
			}
		}
		fclose(fp);

		ch_मूलम् = rc_service_मानम्_get(basename_c(service), "chroot");
		spidfile = pidfile;
		जइ (ch_मूलम् && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_मूलम्, pidfile);
			मुक्त(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		जइ (pidfile) {
			retval = true;
			जइ ((fp = fopen(pidfile, "r"))) {
				जइ (fscanf(fp, "%d", &pid) == 1)
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
		} अण्णहा {
			जइ (exec) {
				जइ (!list)
					list = rc_stringlist_new();
				जइ (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				मुक्त(exec);
				exec = रिक्त;
			}

			जइ (list) {
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

		जइ (!retval) {
			जइ (pid != 0) {
				जइ (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} अण्णहा जइ ((pids = rc_find_pids(exec,
				    (ध्रुव वर्ण *ध्रुव *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				जाव (p1) {
					p2 = LIST_NEXT(p1, entries);
					मुक्त(p1);
					p1 = p2;
				}
				मुक्त(pids);
			} अण्णहा
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
		जइ (retval)
			भङ्ग;
	}
	closedir(dp);
	मुक्त(पङ्क्तिः);

	प्रत्यागम retval;
}
