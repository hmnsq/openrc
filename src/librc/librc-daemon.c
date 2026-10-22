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

#समावेश <errno.h>
#समावेश <dirent.h>
#समावेश <fcntl.h>
#समावेश <limits.h>
#समावेश <signal.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <sys/stat.h>
#समावेश <unistd.h>

#समावेश "queue.h"
#समावेश "librc.h"
#समावेश "helpers.h"

#अगर defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
अटल bool
pid_is_exec(pid_t pid, स्थिर अक्षर *exec)
{
	अक्षर *बफर = शून्य;
	खाता *fp;
	पूर्णांक c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&बफर, "/proc/%d/stat", pid);
	अगर ((fp = ख_खोलो(बफर, "r"))) {
		जबतक ((c = अ_लो(fp)) != खातापूर्ण && c != '(')
			;
		अगर (c == '(') {
			जबतक ((c = अ_लो(fp)) != खातापूर्ण && c == *exec)
				exec++;
			अगर (c == ')' && *exec == '\0')
				retval = true;
		}
		ख_बंद(fp);
	}
	मुक्त(बफर);
	वापस retval;
}

अटल bool
pid_is_argv(pid_t pid, स्थिर अक्षर *स्थिर *argv)
{
	अक्षर *cmdline = शून्य;
	पूर्णांक fd;
	अक्षर बफर[PATH_MAX];
	अक्षर *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	अगर ((fd = खोलो(cmdline, O_RDONLY)) < 0) {
		मुक्त(cmdline);
		वापस false;
	}
	bytes = पढ़ो(fd, बफर, माप(बफर) - 1);
	बंद(fd);
	मुक्त(cmdline);
	अगर (bytes == -1)
		वापस false;

	बफर[bytes] = '\0';
	p = बफर;
	जबतक (*argv) {
		अगर ((अचिन्हित)(p - बफर) >= माप(बफर))
			वापस false;
		अगर (म_भेद(*argv, p) != 0)
			वापस false;
		argv++;
		p += म_माप(p) + 1;
	}
	वापस true;
}

RC_PIDLIST *
rc_find_pids(स्थिर अक्षर *exec, स्थिर अक्षर *स्थिर *argv, uid_t uid, pid_t pid)
{
	सूची *procdir;
	काष्ठा dirent *entry;
	खाता *fp;
	पूर्णांक rc;
	bool container_pid = false;
	bool openvz_host = false;
	अक्षर *पंक्ति = शून्य;
	अक्षर my_ns[30] = {0};
	अक्षर proc_ns[30] = {0};
	माप_प्रकार len = 0;
	pid_t p;
	अक्षर *बफर = शून्य;
	काष्ठा stat sb;
	pid_t openrc_pid = 0;
	अक्षर *pp;
	RC_PIDLIST *pids = शून्य;
	RC_PID *pi;

	अगर ((procdir = सूची_खोलो("/proc")) == शून्य)
		वापस शून्य;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	अगर ((pp = दो_पर्या("RC_OPENRC_PID"))) {
		अगर (माला_पूछो(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	अगर ((fp = ख_खोलो("/proc/self/status", "r"))) {
		जबतक (xgetline(&पंक्ति, &len, fp) != -1) {
			अगर (म_भेदन(पंक्ति, "envID:\t0", 8) == 0) {
				openvz_host = true;
				अवरोध;
			}
		}
		ख_बंद(fp);
	}

	अगर ((rc = readlink("/proc/self/ns/pid", my_ns, माप(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	जबतक ((entry = सूची_पढ़ो(procdir)) != शून्य) {
		अगर (माला_पूछो(entry->d_name, "%d", &p) != 1)
			जारी;
		अगर (openrc_pid != 0 && openrc_pid == p)
			जारी;
		अगर (pid != 0 && pid != p)
			जारी;
		xasprintf(&बफर, "/proc/%d/ns/pid", p);
		अगर ((rc = readlink(बफर, proc_ns, माप(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		मुक्त(बफर);
		अगर (pid == 0 && म_माप(my_ns) && म_माप (proc_ns) && म_भेद(my_ns, proc_ns))
			जारी;
		अगर (uid) {
			xasprintf(&बफर, "/proc/%d", p);
			अगर (stat(बफर, &sb) != 0 || sb.st_uid != uid) {
				मुक्त(बफर);
				जारी;
			}
			मुक्त(बफर);
		}
		अगर (exec && !pid_is_exec(p, exec))
			जारी;
		अगर (argv &&
		    !pid_is_argv(p, (स्थिर अक्षर *स्थिर *)argv))
			जारी;
		/* If this is an OpenVZ host, filter out container processes */
		अगर (openvz_host) {
			xasprintf(&बफर, "/proc/%d/status", p);
			fp = ख_खोलो(बफर, "r");
			मुक्त(बफर);
			अगर (!fp)
				जारी;
			जबतक (xgetline(&पंक्ति, &len, fp) != -1) {
				अगर (म_भेदन(पंक्ति, "envID:", 6) == 0) {
					container_pid = !(म_भेदन(पंक्ति, "envID:\t0", 8) == 0);
					अवरोध;
				}
			}
			ख_बंद(fp);
		}
		अगर (container_pid)
			जारी;
		अगर (!pids) {
			pids = xmalloc(माप(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(माप(*pi));
		pi->pid = p;
		LIST_जोड़ो_शीर्ष(pids, pi, entries);
	}
	अगर (पंक्ति != शून्य)
		मुक्त(पंक्ति);
	बंद_सूची(procdir);
	वापस pids;
}

#या_अगर BSD

# अगर defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH शून्य
#  define _KVM_FLAGS KVM_NO_FILES
# अन्यथा
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  अगर defined(__DragonFly__)
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
rc_find_pids(स्थिर अक्षर *exec, स्थिर अक्षर *स्थिर *argv, uid_t uid, pid_t pid)
{
	अटल kvm_t *kd = शून्य;
	अक्षर errbuf[_POSIX2_LINE_MAX];
	काष्ठा _KINFO_PROC *kp;
	पूर्णांक i;
	पूर्णांक processes = 0;
	पूर्णांक pargc = 0;
	अक्षर **pargv;
	RC_PIDLIST *pids = शून्य;
	RC_PID *pi;
	pid_t p;
	स्थिर अक्षर *स्थिर *arg;
	पूर्णांक match;

	अगर ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    शून्य, _KVM_FLAGS, errbuf)) == शून्य)
	{
		ख_लिखो(मानक_त्रुटि, "kvm_open: %s\n", errbuf);
		वापस शून्य;
	}

#अगर_घोषित _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, माप(*kp), &processes);
#अन्यथा
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#पूर्ण_अगर
	अगर ((kp == शून्य && processes > 0) || (kp != शून्य && processes < 0)) {
		ख_लिखो(मानक_त्रुटि, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		वापस शून्य;
	}

	अगर (exec)
		exec = basename_c(exec);
	क्रम (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		अगर (pid != 0 && pid != p)
			जारी;
		अगर (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			जारी;
		अगर (exec) {
			अगर (!_GET_KINFO_COMM(kp[i]) ||
			    म_भेद(exec, _GET_KINFO_COMM(kp[i])) != 0)
				जारी;
		}
		अगर (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			अगर (!pargv || !*pargv)
				जारी;
			arg = argv;
			match = 1;
			जबतक (*arg && *pargv)
				अगर (म_भेद(*arg++, *pargv++) != 0) {
					match = 0;
					अवरोध;
				}
			अगर (!match)
				जारी;
		}
		अगर (!pids) {
			pids = xmalloc(माप(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(माप(*pi));
		pi->pid = p;
		LIST_जोड़ो_शीर्ष(pids, pi, entries);
	}
	kvm_close(kd);

	वापस pids;
}

#अन्यथा
#  error "Platform not supported!"
#पूर्ण_अगर

अटल bool
_match_daemon(स्थिर अक्षर *svcname, स्थिर अक्षर *instance, RC_STRINGLIST *match)
{
	अक्षर *पंक्ति = शून्य;
	माप_प्रकार len = 0;
	खाता *fp;
	RC_STRING *m;
	अक्षर *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	मुक्त(daemon);

	अगर (!fp)
		वापस false;

	जबतक (xgetline(&पंक्ति, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    अगर (म_भेद(पंक्ति, m->मूल्य) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    अवरोध;
		    }
		अगर (!TAILQ_FIRST(match))
			अवरोध;
	}
	ख_बंद(fp);
	मुक्त(पंक्ति);
	अगर (TAILQ_FIRST(match))
		वापस false;
	वापस true;
}

अटल RC_STRINGLIST *
_match_list(स्थिर अक्षर *exec, स्थिर अक्षर *स्थिर *argv, स्थिर अक्षर *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	पूर्णांक i = 0;
	अक्षर *m;

	अगर (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		मुक्त(m);
	}

	जबतक (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		मुक्त(m);
	}

	अगर (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		मुक्त(m);
	}

	वापस match;
}

bool
rc_service_daemon_set(स्थिर अक्षर *service, स्थिर अक्षर *exec,
    स्थिर अक्षर *स्थिर *argv,
    स्थिर अक्षर *pidfile, bool started)
{
	अक्षर *file = शून्य;
	पूर्णांक nfiles = 0;
	अक्षर oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	सूची *dp;
	काष्ठा dirent *d;
	RC_STRINGLIST *match, *renamelist;
	पूर्णांक i = 0;
	खाता *fp;
	स्थिर अक्षर *base = basename_c(service);

	अगर (!exec && !pidfile) {
		त्रुटि_सं = EINVAL;
		वापस false;
	}

	/* Regardless, erase any existing daemon info */
	अगर ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		जबतक ((d = सूची_पढ़ो(dp))) {
			अगर (d->d_name[0] == '.')
				जारी;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			अगर (rc_stringlist_find(renamelist, file)) {
				मुक्त(file);
				जारी;
			}

			nfiles++;

			अगर (!*oldfile) {
				अगर (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, माप(oldfile));
					nfiles--;
				}
			} अन्यथा {
				नाम(file, oldfile);
				strlcpy(oldfile, file, माप(oldfile));
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
	अगर (!started)
		वापस true;

	अगर (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || त्रुटि_सं == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		अगर ((fp = ख_खोलो(file, "w"))) {
			ख_लिखो(fp, "exec=");
			अगर (exec)
				ख_लिखो(fp, "%s", exec);
			जबतक (argv && argv[i]) {
				ख_लिखो(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			ख_लिखो(fp, "\npidfile=");
			अगर (pidfile)
				ख_लिखो(fp, "%s", pidfile);
			ख_लिखो(fp, "\n");
			ख_बंद(fp);
			retval = true;
		}
		मुक्त(file);
	}

	वापस retval;
}

bool
rc_service_started_daemon(स्थिर अक्षर *service,
    स्थिर अक्षर *exec, स्थिर अक्षर *स्थिर *argv, पूर्णांक indx)
{
	स्थिर अक्षर *base = basename_c(service);
	अक्षर *file = शून्य;
	RC_STRINGLIST *match;
	bool retval = false;
	सूची *dp;
	काष्ठा dirent *d;

	अगर (!service || !exec)
		वापस false;

	match = _match_list(exec, argv, शून्य);

	अगर (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		मुक्त(file);
	} अन्यथा अगर ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		जबतक ((d = सूची_पढ़ो(dp))) {
			अगर (d->d_name[0] == '.')
				जारी;
			अगर ((retval = _match_daemon(base, d->d_name, match)))
				अवरोध;
		}
		बंद_सूची(dp);
	}

	rc_stringlist_free(match);
	वापस retval;
}

bool
rc_service_daemons_crashed(स्थिर अक्षर *service)
{
	सूची *dp;
	काष्ठा dirent *d;
	खाता *fp;
	अक्षर *पंक्ति = शून्य;
	माप_प्रकार len = 0;
	अक्षर **argv = शून्य;
	अक्षर *exec = शून्य;
	अक्षर *name = शून्य;
	अक्षर *pidfile = शून्य;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	अक्षर *p;
	अक्षर *token;
	bool retval = false;
	RC_STRINGLIST *list = शून्य;
	RC_STRING *s;
	माप_प्रकार i;
	अक्षर *ch_मूल;
	अक्षर *spidfile;

	अगर (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		वापस false;

	जबतक ((d = सूची_पढ़ो(dp))) {
		अगर (d->d_name[0] == '.')
			जारी;

		अगर (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			अवरोध;

		जबतक (xgetline(&पंक्ति, &len, fp) != -1) {
			p = पंक्ति;
			अगर ((token = strsep(&p, "=")) == शून्य || !p)
				जारी;

			अगर (!*p)
				जारी;

			अगर (म_भेद(token, "exec") == 0) {
				अगर (exec)
					मुक्त(exec);
				exec = xstrdup(p);
			} अन्यथा अगर (म_भेदन(token, "argv_", 5) == 0) {
				अगर (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} अन्यथा अगर (म_भेद(token, "name") == 0) {
				अगर (name)
					मुक्त(name);
				name = xstrdup(p);
			} अन्यथा अगर (म_भेद(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				अवरोध;
			}
		}
		ख_बंद(fp);

		ch_मूल = rc_service_मूल्य_get(basename_c(service), "chroot");
		spidfile = pidfile;
		अगर (ch_मूल && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_मूल, pidfile);
			मुक्त(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		अगर (pidfile) {
			retval = true;
			अगर ((fp = ख_खोलो(pidfile, "r"))) {
				अगर (ख_पूछो(fp, "%d", &pid) == 1)
					retval = false;
				ख_बंद(fp);
			}
			मुक्त(pidfile);
			pidfile = शून्य;

			/* We have the pid, so no need to match
			   on exec or name */
			मुक्त(exec);
			exec = शून्य;
			मुक्त(name);
			name = शून्य;
		} अन्यथा {
			अगर (exec) {
				अगर (!list)
					list = rc_stringlist_new();
				अगर (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				मुक्त(exec);
				exec = शून्य;
			}

			अगर (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(माप(अक्षर *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->मूल्य;
				argv[i] = शून्य;
			}
		}

		अगर (!retval) {
			अगर (pid != 0) {
				अगर (समाप्त(pid, 0) == -1 && त्रुटि_सं == ESRCH)
					retval = true;
			} अन्यथा अगर ((pids = rc_find_pids(exec,
				    (स्थिर अक्षर *स्थिर *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				जबतक (p1) {
					p2 = LIST_अग्र(p1, entries);
					मुक्त(p1);
					p1 = p2;
				}
				मुक्त(pids);
			} अन्यथा
				retval = true;
		}
		rc_stringlist_free(list);
		list = शून्य;
		मुक्त(argv);
		argv = शून्य;
		मुक्त(exec);
		exec = शून्य;
		मुक्त(name);
		name = शून्य;
		अगर (retval)
			अवरोध;
	}
	बंद_सूची(dp);
	मुक्त(पंक्ति);

	वापस retval;
}
