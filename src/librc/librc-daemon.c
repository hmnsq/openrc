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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <dirent.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "librc.h"
#അന്തര്ഭാവ "helpers.h"

#എങ്കിൽ defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
സ്ഥിതം bool
pid_is_exec(pid_t pid, സ്ഥിരം അക്ഷരം *exec)
{
	അക്ഷരം *buffer = ശൂന്യ;
	FILE *fp;
	പൂർണ്ണസംഖ്യ c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	എങ്കിൽ ((fp = fopen(buffer, "r"))) {
		ആകുമ്പോൾ ((c = getc(fp)) != EOF && c != '(')
			;
		എങ്കിൽ (c == '(') {
			ആകുമ്പോൾ ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			എങ്കിൽ (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	സ്വതന്ത്രം(buffer);
	തിരികെ retval;
}

സ്ഥിതം bool
pid_is_argv(pid_t pid, സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv)
{
	അക്ഷരം *cmdline = ശൂന്യ;
	പൂർണ്ണസംഖ്യ fd;
	അക്ഷരം buffer[PATH_MAX];
	അക്ഷരം *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	എങ്കിൽ ((fd = open(cmdline, O_RDONLY)) < 0) {
		സ്വതന്ത്രം(cmdline);
		തിരികെ false;
	}
	bytes = പഠ(fd, buffer, വലുപ്പം(buffer) - 1);
	close(fd);
	സ്വതന്ത്രം(cmdline);
	എങ്കിൽ (bytes == -1)
		തിരികെ false;

	buffer[bytes] = '\0';
	p = buffer;
	ആകുമ്പോൾ (*argv) {
		എങ്കിൽ ((unsigned)(p - buffer) >= വലുപ്പം(buffer))
			തിരികെ false;
		എങ്കിൽ (strcmp(*argv, p) != 0)
			തിരികെ false;
		argv++;
		p += strlen(p) + 1;
	}
	തിരികെ true;
}

RC_PIDLIST *
rc_find_pids(സ്ഥിരം അക്ഷരം *exec, സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	ഘടന dirent *entry;
	FILE *fp;
	പൂർണ്ണസംഖ്യ rc;
	bool container_pid = false;
	bool openvz_host = false;
	അക്ഷരം *വരി = ശൂന്യ;
	അക്ഷരം my_ns[30] = {0};
	അക്ഷരം proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	അക്ഷരം *buffer = ശൂന്യ;
	ഘടന stat sb;
	pid_t openrc_pid = 0;
	അക്ഷരം *pp;
	RC_PIDLIST *pids = ശൂന്യ;
	RC_PID *pi;

	എങ്കിൽ ((procdir = opendir("/proc")) == ശൂന്യ)
		തിരികെ ശൂന്യ;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	എങ്കിൽ ((pp = getenv("RC_OPENRC_PID"))) {
		എങ്കിൽ (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	എങ്കിൽ ((fp = fopen("/proc/self/status", "r"))) {
		ആകുമ്പോൾ (xgetline(&വരി, &len, fp) != -1) {
			എങ്കിൽ (strncmp(വരി, "envID:\t0", 8) == 0) {
				openvz_host = true;
				നിർത്തുക;
			}
		}
		fclose(fp);
	}

	എങ്കിൽ ((rc = readlink("/proc/self/ns/pid", my_ns, വലുപ്പം(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	ആകുമ്പോൾ ((entry = readdir(procdir)) != ശൂന്യ) {
		എങ്കിൽ (sscanf(entry->d_name, "%d", &p) != 1)
			തുടരുക;
		എങ്കിൽ (openrc_pid != 0 && openrc_pid == p)
			തുടരുക;
		എങ്കിൽ (pid != 0 && pid != p)
			തുടരുക;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		എങ്കിൽ ((rc = readlink(buffer, proc_ns, വലുപ്പം(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		സ്വതന്ത്രം(buffer);
		എങ്കിൽ (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			തുടരുക;
		എങ്കിൽ (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			എങ്കിൽ (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				സ്വതന്ത്രം(buffer);
				തുടരുക;
			}
			സ്വതന്ത്രം(buffer);
		}
		എങ്കിൽ (exec && !pid_is_exec(p, exec))
			തുടരുക;
		എങ്കിൽ (argv &&
		    !pid_is_argv(p, (സ്ഥിരം അക്ഷരം *സ്ഥിരം *)argv))
			തുടരുക;
		/* If this is an OpenVZ host, filter out container processes */
		എങ്കിൽ (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			സ്വതന്ത്രം(buffer);
			എങ്കിൽ (!fp)
				തുടരുക;
			ആകുമ്പോൾ (xgetline(&വരി, &len, fp) != -1) {
				എങ്കിൽ (strncmp(വരി, "envID:", 6) == 0) {
					container_pid = !(strncmp(വരി, "envID:\t0", 8) == 0);
					നിർത്തുക;
				}
			}
			fclose(fp);
		}
		എങ്കിൽ (container_pid)
			തുടരുക;
		എങ്കിൽ (!pids) {
			pids = xmalloc(വലുപ്പം(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(വലുപ്പം(*pi));
		pi->pid = p;
		LIST_തിരുകുക_HEAD(pids, pi, entries);
	}
	എങ്കിൽ (വരി != ശൂന്യ)
		സ്വതന്ത്രം(വരി);
	closedir(procdir);
	തിരികെ pids;
}

#അല്ലെങ്കിലെങ്കിൽ BSD

# എങ്കിൽ defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH ശൂന്യ
#  define _KVM_FLAGS KVM_NO_FILES
# അല്ലെങ്കിൽ
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  എങ്കിൽ defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  അല്ലെങ്കിൽ
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define _KVM_PATH _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_find_pids(സ്ഥിരം അക്ഷരം *exec, സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv, uid_t uid, pid_t pid)
{
	സ്ഥിതം kvm_t *kd = ശൂന്യ;
	അക്ഷരം errbuf[_POSIX2_LINE_MAX];
	ഘടന _KINFO_PROC *kp;
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ processes = 0;
	പൂർണ്ണസംഖ്യ pargc = 0;
	അക്ഷരം **pargv;
	RC_PIDLIST *pids = ശൂന്യ;
	RC_PID *pi;
	pid_t p;
	സ്ഥിരം അക്ഷരം *സ്ഥിരം *arg;
	പൂർണ്ണസംഖ്യ match;

	എങ്കിൽ ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    ശൂന്യ, _KVM_FLAGS, errbuf)) == ശൂന്യ)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		തിരികെ ശൂന്യ;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, വലുപ്പം(*kp), &processes);
#അല്ലെങ്കിൽ
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	എങ്കിൽ ((kp == ശൂന്യ && processes > 0) || (kp != ശൂന്യ && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (exec)
		exec = basename_c(exec);
	വേണ്ടി (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		എങ്കിൽ (pid != 0 && pid != p)
			തുടരുക;
		എങ്കിൽ (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			തുടരുക;
		എങ്കിൽ (exec) {
			എങ്കിൽ (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				തുടരുക;
		}
		എങ്കിൽ (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			എങ്കിൽ (!pargv || !*pargv)
				തുടരുക;
			arg = argv;
			match = 1;
			ആകുമ്പോൾ (*arg && *pargv)
				എങ്കിൽ (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					നിർത്തുക;
				}
			എങ്കിൽ (!match)
				തുടരുക;
		}
		എങ്കിൽ (!pids) {
			pids = xmalloc(വലുപ്പം(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(വലുപ്പം(*pi));
		pi->pid = p;
		LIST_തിരുകുക_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	തിരികെ pids;
}

#അല്ലെങ്കിൽ
#  error "Platform not supported!"
#endif

സ്ഥിതം bool
_match_daemon(സ്ഥിരം അക്ഷരം *svcname, സ്ഥിരം അക്ഷരം *instance, RC_STRINGLIST *match)
{
	അക്ഷരം *വരി = ശൂന്യ;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	അക്ഷരം *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	സ്വതന്ത്രം(daemon);

	എങ്കിൽ (!fp)
		തിരികെ false;

	ആകുമ്പോൾ (xgetline(&വരി, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    എങ്കിൽ (strcmp(വരി, m->മൂല്യം) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    നിർത്തുക;
		    }
		എങ്കിൽ (!TAILQ_FIRST(match))
			നിർത്തുക;
	}
	fclose(fp);
	സ്വതന്ത്രം(വരി);
	എങ്കിൽ (TAILQ_FIRST(match))
		തിരികെ false;
	തിരികെ true;
}

സ്ഥിതം RC_STRINGLIST *
_match_list(സ്ഥിരം അക്ഷരം *exec, സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv, സ്ഥിരം അക്ഷരം *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	പൂർണ്ണസംഖ്യ i = 0;
	അക്ഷരം *m;

	എങ്കിൽ (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		സ്വതന്ത്രം(m);
	}

	ആകുമ്പോൾ (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		സ്വതന്ത്രം(m);
	}

	എങ്കിൽ (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		സ്വതന്ത്രം(m);
	}

	തിരികെ match;
}

bool
rc_service_daemon_set(സ്ഥിരം അക്ഷരം *service, സ്ഥിരം അക്ഷരം *exec,
    സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv,
    സ്ഥിരം അക്ഷരം *pidfile, bool started)
{
	അക്ഷരം *file = ശൂന്യ;
	പൂർണ്ണസംഖ്യ nfiles = 0;
	അക്ഷരം oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	ഘടന dirent *d;
	RC_STRINGLIST *match, *renamelist;
	പൂർണ്ണസംഖ്യ i = 0;
	FILE *fp;
	സ്ഥിരം അക്ഷരം *base = basename_c(service);

	എങ്കിൽ (!exec && !pidfile) {
		errno = EINVAL;
		തിരികെ false;
	}

	/* Regardless, erase any existing daemon info */
	എങ്കിൽ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		ആകുമ്പോൾ ((d = readdir(dp))) {
			എങ്കിൽ (d->d_name[0] == '.')
				തുടരുക;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			എങ്കിൽ (rc_stringlist_find(renamelist, file)) {
				സ്വതന്ത്രം(file);
				തുടരുക;
			}

			nfiles++;

			എങ്കിൽ (!*oldfile) {
				എങ്കിൽ (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, വലുപ്പം(oldfile));
					nfiles--;
				}
			} അല്ലെങ്കിൽ {
				rename(file, oldfile);
				strlcpy(oldfile, file, വലുപ്പം(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_add(renamelist, oldfile);
			}
			സ്വതന്ത്രം(file);
		}
		closedir(dp);
		rc_stringlist_free(match);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	എങ്കിൽ (!started)
		തിരികെ true;

	എങ്കിൽ (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		എങ്കിൽ ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			എങ്കിൽ (exec)
				fprintf(fp, "%s", exec);
			ആകുമ്പോൾ (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			എങ്കിൽ (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		സ്വതന്ത്രം(file);
	}

	തിരികെ retval;
}

bool
rc_service_started_daemon(സ്ഥിരം അക്ഷരം *service,
    സ്ഥിരം അക്ഷരം *exec, സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv, പൂർണ്ണസംഖ്യ indx)
{
	സ്ഥിരം അക്ഷരം *base = basename_c(service);
	അക്ഷരം *file = ശൂന്യ;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	ഘടന dirent *d;

	എങ്കിൽ (!service || !exec)
		തിരികെ false;

	match = _match_list(exec, argv, ശൂന്യ);

	എങ്കിൽ (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		സ്വതന്ത്രം(file);
	} അല്ലെങ്കിൽ എങ്കിൽ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		ആകുമ്പോൾ ((d = readdir(dp))) {
			എങ്കിൽ (d->d_name[0] == '.')
				തുടരുക;
			എങ്കിൽ ((retval = _match_daemon(base, d->d_name, match)))
				നിർത്തുക;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	തിരികെ retval;
}

bool
rc_service_daemons_crashed(സ്ഥിരം അക്ഷരം *service)
{
	DIR *dp;
	ഘടന dirent *d;
	FILE *fp;
	അക്ഷരം *വരി = ശൂന്യ;
	size_t len = 0;
	അക്ഷരം **argv = ശൂന്യ;
	അക്ഷരം *exec = ശൂന്യ;
	അക്ഷരം *name = ശൂന്യ;
	അക്ഷരം *pidfile = ശൂന്യ;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	അക്ഷരം *p;
	അക്ഷരം *token;
	bool retval = false;
	RC_STRINGLIST *list = ശൂന്യ;
	RC_STRING *s;
	size_t i;
	അക്ഷരം *ch_വേര്;
	അക്ഷരം *spidfile;

	എങ്കിൽ (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		തിരികെ false;

	ആകുമ്പോൾ ((d = readdir(dp))) {
		എങ്കിൽ (d->d_name[0] == '.')
			തുടരുക;

		എങ്കിൽ (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			നിർത്തുക;

		ആകുമ്പോൾ (xgetline(&വരി, &len, fp) != -1) {
			p = വരി;
			എങ്കിൽ ((token = strsep(&p, "=")) == ശൂന്യ || !p)
				തുടരുക;

			എങ്കിൽ (!*p)
				തുടരുക;

			എങ്കിൽ (strcmp(token, "exec") == 0) {
				എങ്കിൽ (exec)
					സ്വതന്ത്രം(exec);
				exec = xstrdup(p);
			} അല്ലെങ്കിൽ എങ്കിൽ (strncmp(token, "argv_", 5) == 0) {
				എങ്കിൽ (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(token, "name") == 0) {
				എങ്കിൽ (name)
					സ്വതന്ത്രം(name);
				name = xstrdup(p);
			} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				നിർത്തുക;
			}
		}
		fclose(fp);

		ch_വേര് = rc_service_മൂല്യം_get(basename_c(service), "chroot");
		spidfile = pidfile;
		എങ്കിൽ (ch_വേര് && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_വേര്, pidfile);
			സ്വതന്ത്രം(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		എങ്കിൽ (pidfile) {
			retval = true;
			എങ്കിൽ ((fp = fopen(pidfile, "r"))) {
				എങ്കിൽ (fscanf(fp, "%d", &pid) == 1)
					retval = false;
				fclose(fp);
			}
			സ്വതന്ത്രം(pidfile);
			pidfile = ശൂന്യ;

			/* We have the pid, so no need to match
			   on exec or name */
			സ്വതന്ത്രം(exec);
			exec = ശൂന്യ;
			സ്വതന്ത്രം(name);
			name = ശൂന്യ;
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (exec) {
				എങ്കിൽ (!list)
					list = rc_stringlist_new();
				എങ്കിൽ (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				സ്വതന്ത്രം(exec);
				exec = ശൂന്യ;
			}

			എങ്കിൽ (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(വലുപ്പം(അക്ഷരം *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->മൂല്യം;
				argv[i] = ശൂന്യ;
			}
		}

		എങ്കിൽ (!retval) {
			എങ്കിൽ (pid != 0) {
				എങ്കിൽ (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} അല്ലെങ്കിൽ എങ്കിൽ ((pids = rc_find_pids(exec,
				    (സ്ഥിരം അക്ഷരം *സ്ഥിരം *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				ആകുമ്പോൾ (p1) {
					p2 = LIST_NEXT(p1, entries);
					സ്വതന്ത്രം(p1);
					p1 = p2;
				}
				സ്വതന്ത്രം(pids);
			} അല്ലെങ്കിൽ
				retval = true;
		}
		rc_stringlist_free(list);
		list = ശൂന്യ;
		സ്വതന്ത്രം(argv);
		argv = ശൂന്യ;
		സ്വതന്ത്രം(exec);
		exec = ശൂന്യ;
		സ്വതന്ത്രം(name);
		name = ശൂന്യ;
		എങ്കിൽ (retval)
			നിർത്തുക;
	}
	closedir(dp);
	സ്വതന്ത്രം(വരി);

	തിരികെ retval;
}
