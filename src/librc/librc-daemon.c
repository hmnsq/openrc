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

#यदि defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
स्थिर bool
pid_is_exec(pid_t pid, ध्रुव वर्ण *exec)
{
	वर्ण *बफर = रिक्त;
	FILE *fp;
	पूर्णाङ्क c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&बफर, "/proc/%d/stat", pid);
	यदि ((fp = fopen(बफर, "r"))) {
		यावत् ((c = getc(fp)) != EOF && c != '(')
			;
		यदि (c == '(') {
			यावत् ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			यदि (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	मुक्त(बफर);
	प्रत्यागम retval;
}

स्थिर bool
pid_is_argv(pid_t pid, ध्रुव वर्ण *ध्रुव *argv)
{
	वर्ण *cmdline = रिक्त;
	पूर्णाङ्क fd;
	वर्ण बफर[PATH_MAX];
	वर्ण *p;
	ssize_t अष्टकानि;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	यदि ((fd = उद्घाटय(cmdline, O_RDONLY)) < 0) {
		मुक्त(cmdline);
		प्रत्यागम false;
	}
	अष्टकानि = पठ(fd, बफर, परिमाण(बफर) - 1);
	close(fd);
	मुक्त(cmdline);
	यदि (अष्टकानि == -1)
		प्रत्यागम false;

	बफर[अष्टकानि] = '\0';
	p = बफर;
	यावत् (*argv) {
		यदि ((अचिह्नित)(p - बफर) >= परिमाण(बफर))
			प्रत्यागम false;
		यदि (वाक्यं_तुलय(*argv, p) != 0)
			प्रत्यागम false;
		argv++;
		p += वाक्यदैर्घ्यम्(p) + 1;
	}
	प्रत्यागम true;
}

RC_PIDLIST *
rc_find_pids(ध्रुव वर्ण *exec, ध्रुव वर्ण *ध्रुव *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	संरचना dirent *प्रविष्टिः;
	FILE *fp;
	पूर्णाङ्क rc;
	bool container_pid = false;
	bool openvz_host = false;
	वर्ण *पङ्क्तिः = रिक्त;
	वर्ण my_ns[30] = {0};
	वर्ण proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	वर्ण *बफर = रिक्त;
	संरचना stat sb;
	pid_t openrc_pid = 0;
	वर्ण *pp;
	RC_PIDLIST *pids = रिक्त;
	RC_PID *pi;

	यदि ((procdir = opendir("/proc")) == रिक्त)
		प्रत्यागम रिक्त;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	यदि ((pp = getenv("RC_OPENRC_PID"))) {
		यदि (वाक्यात्_पठ(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	यदि ((fp = fopen("/proc/self/status", "r"))) {
		यावत् (xgetline(&पङ्क्तिः, &len, fp) != -1) {
			यदि (वाक्ये_तुलय(पङ्क्तिः, "envID:\t0", 8) == 0) {
				openvz_host = true;
				भङ्ग;
			}
		}
		fclose(fp);
	}

	यदि ((rc = readlink("/proc/self/ns/pid", my_ns, परिमाण(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	यावत् ((प्रविष्टिः = readdir(procdir)) != रिक्त) {
		यदि (वाक्यात्_पठ(प्रविष्टिः->d_name, "%d", &p) != 1)
			अनुवर्त;
		यदि (openrc_pid != 0 && openrc_pid == p)
			अनुवर्त;
		यदि (pid != 0 && pid != p)
			अनुवर्त;
		xasprintf(&बफर, "/proc/%d/ns/pid", p);
		यदि ((rc = readlink(बफर, proc_ns, परिमाण(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		मुक्त(बफर);
		यदि (pid == 0 && वाक्यदैर्घ्यम्(my_ns) && वाक्यदैर्घ्यम् (proc_ns) && वाक्यं_तुलय(my_ns, proc_ns))
			अनुवर्त;
		यदि (uid) {
			xasprintf(&बफर, "/proc/%d", p);
			यदि (stat(बफर, &sb) != 0 || sb.st_uid != uid) {
				मुक्त(बफर);
				अनुवर्त;
			}
			मुक्त(बफर);
		}
		यदि (exec && !pid_is_exec(p, exec))
			अनुवर्त;
		यदि (argv &&
		    !pid_is_argv(p, (ध्रुव वर्ण *ध्रुव *)argv))
			अनुवर्त;
		/* If this is an OpenVZ host, filter out container processes */
		यदि (openvz_host) {
			xasprintf(&बफर, "/proc/%d/status", p);
			fp = fopen(बफर, "r");
			मुक्त(बफर);
			यदि (!fp)
				अनुवर्त;
			यावत् (xgetline(&पङ्क्तिः, &len, fp) != -1) {
				यदि (वाक्ये_तुलय(पङ्क्तिः, "envID:", 6) == 0) {
					container_pid = !(वाक्ये_तुलय(पङ्क्तिः, "envID:\t0", 8) == 0);
					भङ्ग;
				}
			}
			fclose(fp);
		}
		यदि (container_pid)
			अनुवर्त;
		यदि (!pids) {
			pids = xmalloc(परिमाण(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(परिमाण(*pi));
		pi->pid = p;
		LIST_निवेशय_शीर्ष(pids, pi, entries);
	}
	यदि (पङ्क्तिः != रिक्त)
		मुक्त(पङ्क्तिः);
	closedir(procdir);
	प्रत्यागम pids;
}

#नोचेद्यदि BSD

# यदि defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define KVM_मार्गः रिक्त
#  define _KVM_FLAGS KVM_NO_FILES
# अन्यथा
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  यदि defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  अन्यथा
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define KVM_मार्गः _PATH_DEVNULL
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
	पूर्णाङ्क मेलनम्;

	यदि ((kd = kvm_openfiles(KVM_मार्गः, KVM_मार्गः,
		    रिक्त, _KVM_FLAGS, errbuf)) == रिक्त)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		प्रत्यागम रिक्त;
	}

#यदि_परिभाषितम् _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, परिमाण(*kp), &processes);
#अन्यथा
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#यदि_अन्तः
	यदि ((kp == रिक्त && processes > 0) || (kp != रिक्त && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		प्रत्यागम रिक्त;
	}

	यदि (exec)
		exec = basename_c(exec);
	प्रति (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		यदि (pid != 0 && pid != p)
			अनुवर्त;
		यदि (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			अनुवर्त;
		यदि (exec) {
			यदि (!_GET_KINFO_COMM(kp[i]) ||
			    वाक्यं_तुलय(exec, _GET_KINFO_COMM(kp[i])) != 0)
				अनुवर्त;
		}
		यदि (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			यदि (!pargv || !*pargv)
				अनुवर्त;
			arg = argv;
			मेलनम् = 1;
			यावत् (*arg && *pargv)
				यदि (वाक्यं_तुलय(*arg++, *pargv++) != 0) {
					मेलनम् = 0;
					भङ्ग;
				}
			यदि (!मेलनम्)
				अनुवर्त;
		}
		यदि (!pids) {
			pids = xmalloc(परिमाण(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(परिमाण(*pi));
		pi->pid = p;
		LIST_निवेशय_शीर्ष(pids, pi, entries);
	}
	kvm_close(kd);

	प्रत्यागम pids;
}

#अन्यथा
#  error "Platform not supported!"
#यदि_अन्तः

स्थिर bool
मेलनम्_daemon(ध्रुव वर्ण *svcname, ध्रुव वर्ण *instance, RC_STRINGLIST *मेलनम्)
{
	वर्ण *पङ्क्तिः = रिक्त;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	वर्ण *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	मुक्त(daemon);

	यदि (!fp)
		प्रत्यागम false;

	यावत् (xgetline(&पङ्क्तिः, &len, fp) != -1) {
		TAILQ_FOREACH(m, मेलनम्, entries)
		    यदि (वाक्यं_तुलय(पङ्क्तिः, m->मानम्) == 0) {
			    TAILQ_REMOVE(मेलनम्, m, entries);
			    भङ्ग;
		    }
		यदि (!TAILQ_FIRST(मेलनम्))
			भङ्ग;
	}
	fclose(fp);
	मुक्त(पङ्क्तिः);
	यदि (TAILQ_FIRST(मेलनम्))
		प्रत्यागम false;
	प्रत्यागम true;
}

स्थिर RC_STRINGLIST *
मेलनम्_list(ध्रुव वर्ण *exec, ध्रुव वर्ण *ध्रुव *argv, ध्रुव वर्ण *pidfile)
{
	RC_STRINGLIST *मेलनम् = rc_stringlist_new();
	पूर्णाङ्क i = 0;
	वर्ण *m;

	यदि (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_सङ्कलय(मेलनम्, m);
		मुक्त(m);
	}

	यावत् (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_सङ्कलय(मेलनम्, m);
		मुक्त(m);
	}

	यदि (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_सङ्कलय(मेलनम्, m);
		मुक्त(m);
	}

	प्रत्यागम मेलनम्;
}

bool
rc_service_daemon_समुच्चयः(ध्रुव वर्ण *service, ध्रुव वर्ण *exec,
    ध्रुव वर्ण *ध्रुव *argv,
    ध्रुव वर्ण *pidfile, bool started)
{
	वर्ण *file = रिक्त;
	पूर्णाङ्क nfiles = 0;
	वर्ण oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	संरचना dirent *d;
	RC_STRINGLIST *मेलनम्, *renamelist;
	पूर्णाङ्क i = 0;
	FILE *fp;
	ध्रुव वर्ण *आधारः = basename_c(service);

	यदि (!exec && !pidfile) {
		errno = EINVAL;
		प्रत्यागम false;
	}

	/* Regardless, erase any existing daemon info */
	यदि ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), आधारः))) {
		मेलनम् = मेलनम्_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		यावत् ((d = readdir(dp))) {
			यदि (d->d_name[0] == '.')
				अनुवर्त;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), आधारः, d->d_name);
			यदि (rc_stringlist_find(renamelist, file)) {
				मुक्त(file);
				अनुवर्त;
			}

			nfiles++;

			यदि (!*oldfile) {
				यदि (मेलनम्_daemon(आधारः, d->d_name, मेलनम्)) {
					unlink(file);
					strlcpy(oldfile, file, परिमाण(oldfile));
					nfiles--;
				}
			} अन्यथा {
				rename(file, oldfile);
				strlcpy(oldfile, file, परिमाण(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_सङ्कलय(renamelist, oldfile);
			}
			मुक्त(file);
		}
		closedir(dp);
		rc_stringlist_free(मेलनम्);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	यदि (!started)
		प्रत्यागम true;

	यदि (mkdirat(rc_dirfd(RC_DIR_DAEMONS), आधारः, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), आधारः, nfiles + 1);
		यदि ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			यदि (exec)
				fprintf(fp, "%s", exec);
			यावत् (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			यदि (pidfile)
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
	ध्रुव वर्ण *आधारः = basename_c(service);
	वर्ण *file = रिक्त;
	RC_STRINGLIST *मेलनम्;
	bool retval = false;
	DIR *dp;
	संरचना dirent *d;

	यदि (!service || !exec)
		प्रत्यागम false;

	मेलनम् = मेलनम्_list(exec, argv, रिक्त);

	यदि (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = मेलनम्_daemon(आधारः, file, मेलनम्);
		मुक्त(file);
	} अन्यथा यदि ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), आधारः))) {
		यावत् ((d = readdir(dp))) {
			यदि (d->d_name[0] == '.')
				अनुवर्त;
			यदि ((retval = मेलनम्_daemon(आधारः, d->d_name, मेलनम्)))
				भङ्ग;
		}
		closedir(dp);
	}

	rc_stringlist_free(मेलनम्);
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

	यदि (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		प्रत्यागम false;

	यावत् ((d = readdir(dp))) {
		यदि (d->d_name[0] == '.')
			अनुवर्त;

		यदि (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			भङ्ग;

		यावत् (xgetline(&पङ्क्तिः, &len, fp) != -1) {
			p = पङ्क्तिः;
			यदि ((token = strsep(&p, "=")) == रिक्त || !p)
				अनुवर्त;

			यदि (!*p)
				अनुवर्त;

			यदि (वाक्यं_तुलय(token, "exec") == 0) {
				यदि (exec)
					मुक्त(exec);
				exec = xstrdup(p);
			} अन्यथा यदि (वाक्ये_तुलय(token, "argv_", 5) == 0) {
				यदि (!list)
					list = rc_stringlist_new();
				rc_stringlist_सङ्कलय(list, p);
			} अन्यथा यदि (वाक्यं_तुलय(token, "name") == 0) {
				यदि (name)
					मुक्त(name);
				name = xstrdup(p);
			} अन्यथा यदि (वाक्यं_तुलय(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				भङ्ग;
			}
		}
		fclose(fp);

		ch_मूलम् = rc_service_मानम्_get(basename_c(service), "chroot");
		spidfile = pidfile;
		यदि (ch_मूलम् && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_मूलम्, pidfile);
			मुक्त(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		यदि (pidfile) {
			retval = true;
			यदि ((fp = fopen(pidfile, "r"))) {
				यदि (fscanf(fp, "%d", &pid) == 1)
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
		} अन्यथा {
			यदि (exec) {
				यदि (!list)
					list = rc_stringlist_new();
				यदि (!TAILQ_FIRST(list))
					rc_stringlist_सङ्कलय(list, exec);

				मुक्त(exec);
				exec = रिक्त;
			}

			यदि (list) {
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

		यदि (!retval) {
			यदि (pid != 0) {
				यदि (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} अन्यथा यदि ((pids = rc_find_pids(exec,
				    (ध्रुव वर्ण *ध्रुव *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				यावत् (p1) {
					p2 = LIST_अग्र(p1, entries);
					मुक्त(p1);
					p1 = p2;
				}
				मुक्त(pids);
			} अन्यथा
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
		यदि (retval)
			भङ्ग;
	}
	closedir(dp);
	मुक्त(पङ्क्तिः);

	प्रत्यागम retval;
}
