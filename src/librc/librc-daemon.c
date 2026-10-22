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

#जर defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
स्थिर bool
pid_is_exec(pid_t pid, स्थिरांक अक्षर *exec)
{
	अक्षर *buffer = रिक्त;
	खाता *fp;
	पूर्णांक c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	जर ((fp = ख_खोलो(buffer, "r"))) {
		पर्यंत ((c = अ_लो(fp)) != खातापूर्ण && c != '(')
			;
		जर (c == '(') {
			पर्यंत ((c = अ_लो(fp)) != खातापूर्ण && c == *exec)
				exec++;
			जर (c == ')' && *exec == '\0')
				retval = true;
		}
		ख_बंद(fp);
	}
	मुक्त(buffer);
	परत retval;
}

स्थिर bool
pid_is_argv(pid_t pid, स्थिरांक अक्षर *स्थिरांक *argv)
{
	अक्षर *cmdline = रिक्त;
	पूर्णांक fd;
	अक्षर buffer[PATH_MAX];
	अक्षर *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	जर ((fd = खोलो(cmdline, O_RDONLY)) < 0) {
		मुक्त(cmdline);
		परत false;
	}
	bytes = पठ(fd, buffer, आकार(buffer) - 1);
	बंद(fd);
	मुक्त(cmdline);
	जर (bytes == -1)
		परत false;

	buffer[bytes] = '\0';
	p = buffer;
	पर्यंत (*argv) {
		जर ((अचिन्हित)(p - buffer) >= आकार(buffer))
			परत false;
		जर (म_भेद(*argv, p) != 0)
			परत false;
		argv++;
		p += म_माप(p) + 1;
	}
	परत true;
}

RC_PIDLIST *
rc_find_pids(स्थिरांक अक्षर *exec, स्थिरांक अक्षर *स्थिरांक *argv, uid_t uid, pid_t pid)
{
	सूची *procdir;
	संरचना dirent *entry;
	खाता *fp;
	पूर्णांक rc;
	bool container_pid = false;
	bool openvz_host = false;
	अक्षर *ओळ = रिक्त;
	अक्षर my_ns[30] = {0};
	अक्षर proc_ns[30] = {0};
	माप_प्रकार len = 0;
	pid_t p;
	अक्षर *buffer = रिक्त;
	संरचना stat sb;
	pid_t openrc_pid = 0;
	अक्षर *pp;
	RC_PIDLIST *pids = रिक्त;
	RC_PID *pi;

	जर ((procdir = सूची_खोलो("/proc")) == रिक्त)
		परत रिक्त;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	जर ((pp = दो_पर्या("RC_OPENRC_PID"))) {
		जर (माला_पूछो(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	जर ((fp = ख_खोलो("/proc/self/status", "r"))) {
		पर्यंत (xgetline(&ओळ, &len, fp) != -1) {
			जर (म_भेदन(ओळ, "envID:\t0", 8) == 0) {
				openvz_host = true;
				थांबा;
			}
		}
		ख_बंद(fp);
	}

	जर ((rc = readlink("/proc/self/ns/pid", my_ns, आकार(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	पर्यंत ((entry = सूची_पढ़ो(procdir)) != रिक्त) {
		जर (माला_पूछो(entry->d_name, "%d", &p) != 1)
			सुरूठेवा;
		जर (openrc_pid != 0 && openrc_pid == p)
			सुरूठेवा;
		जर (pid != 0 && pid != p)
			सुरूठेवा;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		जर ((rc = readlink(buffer, proc_ns, आकार(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		मुक्त(buffer);
		जर (pid == 0 && म_माप(my_ns) && म_माप (proc_ns) && म_भेद(my_ns, proc_ns))
			सुरूठेवा;
		जर (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			जर (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				मुक्त(buffer);
				सुरूठेवा;
			}
			मुक्त(buffer);
		}
		जर (exec && !pid_is_exec(p, exec))
			सुरूठेवा;
		जर (argv &&
		    !pid_is_argv(p, (स्थिरांक अक्षर *स्थिरांक *)argv))
			सुरूठेवा;
		/* If this is an OpenVZ host, filter out container processes */
		जर (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = ख_खोलो(buffer, "r");
			मुक्त(buffer);
			जर (!fp)
				सुरूठेवा;
			पर्यंत (xgetline(&ओळ, &len, fp) != -1) {
				जर (म_भेदन(ओळ, "envID:", 6) == 0) {
					container_pid = !(म_भेदन(ओळ, "envID:\t0", 8) == 0);
					थांबा;
				}
			}
			ख_बंद(fp);
		}
		जर (container_pid)
			सुरूठेवा;
		जर (!pids) {
			pids = xmalloc(आकार(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(आकार(*pi));
		pi->pid = p;
		LIST_घाला_HEAD(pids, pi, entries);
	}
	जर (ओळ != रिक्त)
		मुक्त(ओळ);
	बंद_सूची(procdir);
	परत pids;
}

#किंवाजर BSD

# जर defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH रिक्त
#  define _KVM_FLAGS KVM_NO_FILES
# अन्यथा
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  जर defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  अन्यथा
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define _KVM_PATH _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_find_pids(स्थिरांक अक्षर *exec, स्थिरांक अक्षर *स्थिरांक *argv, uid_t uid, pid_t pid)
{
	स्थिर kvm_t *kd = रिक्त;
	अक्षर errbuf[_POSIX2_LINE_MAX];
	संरचना _KINFO_PROC *kp;
	पूर्णांक i;
	पूर्णांक processes = 0;
	पूर्णांक pargc = 0;
	अक्षर **pargv;
	RC_PIDLIST *pids = रिक्त;
	RC_PID *pi;
	pid_t p;
	स्थिरांक अक्षर *स्थिरांक *arg;
	पूर्णांक match;

	जर ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    रिक्त, _KVM_FLAGS, errbuf)) == रिक्त)
	{
		ख_लिखो(मानक_त्रुटि, "kvm_open: %s\n", errbuf);
		परत रिक्त;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, आकार(*kp), &processes);
#अन्यथा
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	जर ((kp == रिक्त && processes > 0) || (kp != रिक्त && processes < 0)) {
		ख_लिखो(मानक_त्रुटि, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		परत रिक्त;
	}

	जर (exec)
		exec = basename_c(exec);
	साठी (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		जर (pid != 0 && pid != p)
			सुरूठेवा;
		जर (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			सुरूठेवा;
		जर (exec) {
			जर (!_GET_KINFO_COMM(kp[i]) ||
			    म_भेद(exec, _GET_KINFO_COMM(kp[i])) != 0)
				सुरूठेवा;
		}
		जर (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			जर (!pargv || !*pargv)
				सुरूठेवा;
			arg = argv;
			match = 1;
			पर्यंत (*arg && *pargv)
				जर (म_भेद(*arg++, *pargv++) != 0) {
					match = 0;
					थांबा;
				}
			जर (!match)
				सुरूठेवा;
		}
		जर (!pids) {
			pids = xmalloc(आकार(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(आकार(*pi));
		pi->pid = p;
		LIST_घाला_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	परत pids;
}

#अन्यथा
#  error "Platform not supported!"
#endif

स्थिर bool
_match_daemon(स्थिरांक अक्षर *svcname, स्थिरांक अक्षर *instance, RC_STRINGLIST *match)
{
	अक्षर *ओळ = रिक्त;
	माप_प्रकार len = 0;
	खाता *fp;
	RC_STRING *m;
	अक्षर *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	मुक्त(daemon);

	जर (!fp)
		परत false;

	पर्यंत (xgetline(&ओळ, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    जर (म_भेद(ओळ, m->मूल्य) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    थांबा;
		    }
		जर (!TAILQ_FIRST(match))
			थांबा;
	}
	ख_बंद(fp);
	मुक्त(ओळ);
	जर (TAILQ_FIRST(match))
		परत false;
	परत true;
}

स्थिर RC_STRINGLIST *
_match_list(स्थिरांक अक्षर *exec, स्थिरांक अक्षर *स्थिरांक *argv, स्थिरांक अक्षर *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	पूर्णांक i = 0;
	अक्षर *m;

	जर (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		मुक्त(m);
	}

	पर्यंत (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		मुक्त(m);
	}

	जर (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		मुक्त(m);
	}

	परत match;
}

bool
rc_service_daemon_set(स्थिरांक अक्षर *service, स्थिरांक अक्षर *exec,
    स्थिरांक अक्षर *स्थिरांक *argv,
    स्थिरांक अक्षर *pidfile, bool started)
{
	अक्षर *file = रिक्त;
	पूर्णांक nfiles = 0;
	अक्षर oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	सूची *dp;
	संरचना dirent *d;
	RC_STRINGLIST *match, *renamelist;
	पूर्णांक i = 0;
	खाता *fp;
	स्थिरांक अक्षर *base = basename_c(service);

	जर (!exec && !pidfile) {
		त्रुटि_सं = EINVAL;
		परत false;
	}

	/* Regardless, erase any existing daemon info */
	जर ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		पर्यंत ((d = सूची_पढ़ो(dp))) {
			जर (d->d_name[0] == '.')
				सुरूठेवा;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			जर (rc_stringlist_find(renamelist, file)) {
				मुक्त(file);
				सुरूठेवा;
			}

			nfiles++;

			जर (!*oldfile) {
				जर (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, आकार(oldfile));
					nfiles--;
				}
			} अन्यथा {
				नाम(file, oldfile);
				strlcpy(oldfile, file, आकार(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_add(renamelist, oldfile);
			}
			मुक्त(file);
		}
		बंद_सूची(dp);
		rc_stringlist_free(match);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	जर (!started)
		परत true;

	जर (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || त्रुटि_सं == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		जर ((fp = ख_खोलो(file, "w"))) {
			ख_लिखो(fp, "exec=");
			जर (exec)
				ख_लिखो(fp, "%s", exec);
			पर्यंत (argv && argv[i]) {
				ख_लिखो(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			ख_लिखो(fp, "\npidfile=");
			जर (pidfile)
				ख_लिखो(fp, "%s", pidfile);
			ख_लिखो(fp, "\n");
			ख_बंद(fp);
			retval = true;
		}
		मुक्त(file);
	}

	परत retval;
}

bool
rc_service_started_daemon(स्थिरांक अक्षर *service,
    स्थिरांक अक्षर *exec, स्थिरांक अक्षर *स्थिरांक *argv, पूर्णांक indx)
{
	स्थिरांक अक्षर *base = basename_c(service);
	अक्षर *file = रिक्त;
	RC_STRINGLIST *match;
	bool retval = false;
	सूची *dp;
	संरचना dirent *d;

	जर (!service || !exec)
		परत false;

	match = _match_list(exec, argv, रिक्त);

	जर (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		मुक्त(file);
	} अन्यथा जर ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		पर्यंत ((d = सूची_पढ़ो(dp))) {
			जर (d->d_name[0] == '.')
				सुरूठेवा;
			जर ((retval = _match_daemon(base, d->d_name, match)))
				थांबा;
		}
		बंद_सूची(dp);
	}

	rc_stringlist_free(match);
	परत retval;
}

bool
rc_service_daemons_crashed(स्थिरांक अक्षर *service)
{
	सूची *dp;
	संरचना dirent *d;
	खाता *fp;
	अक्षर *ओळ = रिक्त;
	माप_प्रकार len = 0;
	अक्षर **argv = रिक्त;
	अक्षर *exec = रिक्त;
	अक्षर *name = रिक्त;
	अक्षर *pidfile = रिक्त;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	अक्षर *p;
	अक्षर *token;
	bool retval = false;
	RC_STRINGLIST *list = रिक्त;
	RC_STRING *s;
	माप_प्रकार i;
	अक्षर *ch_मूळ;
	अक्षर *spidfile;

	जर (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		परत false;

	पर्यंत ((d = सूची_पढ़ो(dp))) {
		जर (d->d_name[0] == '.')
			सुरूठेवा;

		जर (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			थांबा;

		पर्यंत (xgetline(&ओळ, &len, fp) != -1) {
			p = ओळ;
			जर ((token = strsep(&p, "=")) == रिक्त || !p)
				सुरूठेवा;

			जर (!*p)
				सुरूठेवा;

			जर (म_भेद(token, "exec") == 0) {
				जर (exec)
					मुक्त(exec);
				exec = xstrdup(p);
			} अन्यथा जर (म_भेदन(token, "argv_", 5) == 0) {
				जर (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} अन्यथा जर (म_भेद(token, "name") == 0) {
				जर (name)
					मुक्त(name);
				name = xstrdup(p);
			} अन्यथा जर (म_भेद(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				थांबा;
			}
		}
		ख_बंद(fp);

		ch_मूळ = rc_service_मूल्य_get(basename_c(service), "chroot");
		spidfile = pidfile;
		जर (ch_मूळ && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_मूळ, pidfile);
			मुक्त(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		जर (pidfile) {
			retval = true;
			जर ((fp = ख_खोलो(pidfile, "r"))) {
				जर (ख_पूछो(fp, "%d", &pid) == 1)
					retval = false;
				ख_बंद(fp);
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
			जर (exec) {
				जर (!list)
					list = rc_stringlist_new();
				जर (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				मुक्त(exec);
				exec = रिक्त;
			}

			जर (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(आकार(अक्षर *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->मूल्य;
				argv[i] = रिक्त;
			}
		}

		जर (!retval) {
			जर (pid != 0) {
				जर (समाप्त(pid, 0) == -1 && त्रुटि_सं == ESRCH)
					retval = true;
			} अन्यथा जर ((pids = rc_find_pids(exec,
				    (स्थिरांक अक्षर *स्थिरांक *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				पर्यंत (p1) {
					p2 = LIST_NEXT(p1, entries);
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
		जर (retval)
			थांबा;
	}
	बंद_सूची(dp);
	मुक्त(ओळ);

	परत retval;
}
