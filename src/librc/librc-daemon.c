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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <dirent.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "librc.h"
#அந்தர்பாவ "helpers.h"

#எனில் defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
நிலையான bool
pid_is_exec(pid_t pid, மாறிலி எழுத்து *exec)
{
	எழுத்து *buffer = வெற்று;
	FILE *fp;
	முழு c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	எனில் ((fp = fopen(buffer, "r"))) {
		போது ((c = getc(fp)) != EOF && c != '(')
			;
		எனில் (c == '(') {
			போது ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			எனில் (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	விடுவி(buffer);
	திரும்பு retval;
}

நிலையான bool
pid_is_argv(pid_t pid, மாறிலி எழுத்து *மாறிலி *argv)
{
	எழுத்து *cmdline = வெற்று;
	முழு fd;
	எழுத்து buffer[PATH_MAX];
	எழுத்து *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	எனில் ((fd = open(cmdline, O_RDONLY)) < 0) {
		விடுவி(cmdline);
		திரும்பு false;
	}
	bytes = பட(fd, buffer, அளவு(buffer) - 1);
	close(fd);
	விடுவி(cmdline);
	எனில் (bytes == -1)
		திரும்பு false;

	buffer[bytes] = '\0';
	p = buffer;
	போது (*argv) {
		எனில் ((unsigned)(p - buffer) >= அளவு(buffer))
			திரும்பு false;
		எனில் (strcmp(*argv, p) != 0)
			திரும்பு false;
		argv++;
		p += strlen(p) + 1;
	}
	திரும்பு true;
}

RC_PIDLIST *
rc_find_pids(மாறிலி எழுத்து *exec, மாறிலி எழுத்து *மாறிலி *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	கட்டமைப்பு dirent *entry;
	FILE *fp;
	முழு rc;
	bool container_pid = false;
	bool openvz_host = false;
	எழுத்து *வரி = வெற்று;
	எழுத்து my_ns[30] = {0};
	எழுத்து proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	எழுத்து *buffer = வெற்று;
	கட்டமைப்பு stat sb;
	pid_t openrc_pid = 0;
	எழுத்து *pp;
	RC_PIDLIST *pids = வெற்று;
	RC_PID *pi;

	எனில் ((procdir = opendir("/proc")) == வெற்று)
		திரும்பு வெற்று;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	எனில் ((pp = getenv("RC_OPENRC_PID"))) {
		எனில் (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	எனில் ((fp = fopen("/proc/self/status", "r"))) {
		போது (xgetline(&வரி, &len, fp) != -1) {
			எனில் (strncmp(வரி, "envID:\t0", 8) == 0) {
				openvz_host = true;
				நிறுத்து;
			}
		}
		fclose(fp);
	}

	எனில் ((rc = readlink("/proc/self/ns/pid", my_ns, அளவு(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	போது ((entry = readdir(procdir)) != வெற்று) {
		எனில் (sscanf(entry->d_name, "%d", &p) != 1)
			தொடர்;
		எனில் (openrc_pid != 0 && openrc_pid == p)
			தொடர்;
		எனில் (pid != 0 && pid != p)
			தொடர்;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		எனில் ((rc = readlink(buffer, proc_ns, அளவு(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		விடுவி(buffer);
		எனில் (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			தொடர்;
		எனில் (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			எனில் (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				விடுவி(buffer);
				தொடர்;
			}
			விடுவி(buffer);
		}
		எனில் (exec && !pid_is_exec(p, exec))
			தொடர்;
		எனில் (argv &&
		    !pid_is_argv(p, (மாறிலி எழுத்து *மாறிலி *)argv))
			தொடர்;
		/* If this is an OpenVZ host, filter out container processes */
		எனில் (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			விடுவி(buffer);
			எனில் (!fp)
				தொடர்;
			போது (xgetline(&வரி, &len, fp) != -1) {
				எனில் (strncmp(வரி, "envID:", 6) == 0) {
					container_pid = !(strncmp(வரி, "envID:\t0", 8) == 0);
					நிறுத்து;
				}
			}
			fclose(fp);
		}
		எனில் (container_pid)
			தொடர்;
		எனில் (!pids) {
			pids = xmalloc(அளவு(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(அளவு(*pi));
		pi->pid = p;
		LIST_செருகு_HEAD(pids, pi, entries);
	}
	எனில் (வரி != வெற்று)
		விடுவி(வரி);
	closedir(procdir);
	திரும்பு pids;
}

#அல்லதெனில் BSD

# எனில் defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH வெற்று
#  define _KVM_FLAGS KVM_NO_FILES
# இல்லையெனில்
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  எனில் defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  இல்லையெனில்
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define _KVM_PATH _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_find_pids(மாறிலி எழுத்து *exec, மாறிலி எழுத்து *மாறிலி *argv, uid_t uid, pid_t pid)
{
	நிலையான kvm_t *kd = வெற்று;
	எழுத்து errbuf[_POSIX2_LINE_MAX];
	கட்டமைப்பு _KINFO_PROC *kp;
	முழு i;
	முழு processes = 0;
	முழு pargc = 0;
	எழுத்து **pargv;
	RC_PIDLIST *pids = வெற்று;
	RC_PID *pi;
	pid_t p;
	மாறிலி எழுத்து *மாறிலி *arg;
	முழு match;

	எனில் ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    வெற்று, _KVM_FLAGS, errbuf)) == வெற்று)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		திரும்பு வெற்று;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, அளவு(*kp), &processes);
#இல்லையெனில்
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	எனில் ((kp == வெற்று && processes > 0) || (kp != வெற்று && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		திரும்பு வெற்று;
	}

	எனில் (exec)
		exec = basename_c(exec);
	க்கு (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		எனில் (pid != 0 && pid != p)
			தொடர்;
		எனில் (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			தொடர்;
		எனில் (exec) {
			எனில் (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				தொடர்;
		}
		எனில் (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			எனில் (!pargv || !*pargv)
				தொடர்;
			arg = argv;
			match = 1;
			போது (*arg && *pargv)
				எனில் (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					நிறுத்து;
				}
			எனில் (!match)
				தொடர்;
		}
		எனில் (!pids) {
			pids = xmalloc(அளவு(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(அளவு(*pi));
		pi->pid = p;
		LIST_செருகு_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	திரும்பு pids;
}

#இல்லையெனில்
#  error "Platform not supported!"
#endif

நிலையான bool
_match_daemon(மாறிலி எழுத்து *svcname, மாறிலி எழுத்து *instance, RC_STRINGLIST *match)
{
	எழுத்து *வரி = வெற்று;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	எழுத்து *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	விடுவி(daemon);

	எனில் (!fp)
		திரும்பு false;

	போது (xgetline(&வரி, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    எனில் (strcmp(வரி, m->மதிப்பு) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    நிறுத்து;
		    }
		எனில் (!TAILQ_FIRST(match))
			நிறுத்து;
	}
	fclose(fp);
	விடுவி(வரி);
	எனில் (TAILQ_FIRST(match))
		திரும்பு false;
	திரும்பு true;
}

நிலையான RC_STRINGLIST *
_match_list(மாறிலி எழுத்து *exec, மாறிலி எழுத்து *மாறிலி *argv, மாறிலி எழுத்து *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	முழு i = 0;
	எழுத்து *m;

	எனில் (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		விடுவி(m);
	}

	போது (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		விடுவி(m);
	}

	எனில் (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		விடுவி(m);
	}

	திரும்பு match;
}

bool
rc_service_daemon_set(மாறிலி எழுத்து *service, மாறிலி எழுத்து *exec,
    மாறிலி எழுத்து *மாறிலி *argv,
    மாறிலி எழுத்து *pidfile, bool started)
{
	எழுத்து *file = வெற்று;
	முழு nfiles = 0;
	எழுத்து oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	கட்டமைப்பு dirent *d;
	RC_STRINGLIST *match, *renamelist;
	முழு i = 0;
	FILE *fp;
	மாறிலி எழுத்து *base = basename_c(service);

	எனில் (!exec && !pidfile) {
		errno = EINVAL;
		திரும்பு false;
	}

	/* Regardless, erase any existing daemon info */
	எனில் ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		போது ((d = readdir(dp))) {
			எனில் (d->d_name[0] == '.')
				தொடர்;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			எனில் (rc_stringlist_find(renamelist, file)) {
				விடுவி(file);
				தொடர்;
			}

			nfiles++;

			எனில் (!*oldfile) {
				எனில் (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, அளவு(oldfile));
					nfiles--;
				}
			} இல்லையெனில் {
				rename(file, oldfile);
				strlcpy(oldfile, file, அளவு(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_add(renamelist, oldfile);
			}
			விடுவி(file);
		}
		closedir(dp);
		rc_stringlist_free(match);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	எனில் (!started)
		திரும்பு true;

	எனில் (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		எனில் ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			எனில் (exec)
				fprintf(fp, "%s", exec);
			போது (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			எனில் (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		விடுவி(file);
	}

	திரும்பு retval;
}

bool
rc_service_started_daemon(மாறிலி எழுத்து *service,
    மாறிலி எழுத்து *exec, மாறிலி எழுத்து *மாறிலி *argv, முழு indx)
{
	மாறிலி எழுத்து *base = basename_c(service);
	எழுத்து *file = வெற்று;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	கட்டமைப்பு dirent *d;

	எனில் (!service || !exec)
		திரும்பு false;

	match = _match_list(exec, argv, வெற்று);

	எனில் (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		விடுவி(file);
	} இல்லையெனில் எனில் ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		போது ((d = readdir(dp))) {
			எனில் (d->d_name[0] == '.')
				தொடர்;
			எனில் ((retval = _match_daemon(base, d->d_name, match)))
				நிறுத்து;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	திரும்பு retval;
}

bool
rc_service_daemons_crashed(மாறிலி எழுத்து *service)
{
	DIR *dp;
	கட்டமைப்பு dirent *d;
	FILE *fp;
	எழுத்து *வரி = வெற்று;
	size_t len = 0;
	எழுத்து **argv = வெற்று;
	எழுத்து *exec = வெற்று;
	எழுத்து *name = வெற்று;
	எழுத்து *pidfile = வெற்று;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	எழுத்து *p;
	எழுத்து *token;
	bool retval = false;
	RC_STRINGLIST *list = வெற்று;
	RC_STRING *s;
	size_t i;
	எழுத்து *ch_வேர்;
	எழுத்து *spidfile;

	எனில் (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		திரும்பு false;

	போது ((d = readdir(dp))) {
		எனில் (d->d_name[0] == '.')
			தொடர்;

		எனில் (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			நிறுத்து;

		போது (xgetline(&வரி, &len, fp) != -1) {
			p = வரி;
			எனில் ((token = strsep(&p, "=")) == வெற்று || !p)
				தொடர்;

			எனில் (!*p)
				தொடர்;

			எனில் (strcmp(token, "exec") == 0) {
				எனில் (exec)
					விடுவி(exec);
				exec = xstrdup(p);
			} இல்லையெனில் எனில் (strncmp(token, "argv_", 5) == 0) {
				எனில் (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} இல்லையெனில் எனில் (strcmp(token, "name") == 0) {
				எனில் (name)
					விடுவி(name);
				name = xstrdup(p);
			} இல்லையெனில் எனில் (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				நிறுத்து;
			}
		}
		fclose(fp);

		ch_வேர் = rc_service_மதிப்பு_get(basename_c(service), "chroot");
		spidfile = pidfile;
		எனில் (ch_வேர் && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_வேர், pidfile);
			விடுவி(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		எனில் (pidfile) {
			retval = true;
			எனில் ((fp = fopen(pidfile, "r"))) {
				எனில் (fscanf(fp, "%d", &pid) == 1)
					retval = false;
				fclose(fp);
			}
			விடுவி(pidfile);
			pidfile = வெற்று;

			/* We have the pid, so no need to match
			   on exec or name */
			விடுவி(exec);
			exec = வெற்று;
			விடுவி(name);
			name = வெற்று;
		} இல்லையெனில் {
			எனில் (exec) {
				எனில் (!list)
					list = rc_stringlist_new();
				எனில் (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				விடுவி(exec);
				exec = வெற்று;
			}

			எனில் (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(அளவு(எழுத்து *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->மதிப்பு;
				argv[i] = வெற்று;
			}
		}

		எனில் (!retval) {
			எனில் (pid != 0) {
				எனில் (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} இல்லையெனில் எனில் ((pids = rc_find_pids(exec,
				    (மாறிலி எழுத்து *மாறிலி *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				போது (p1) {
					p2 = LIST_NEXT(p1, entries);
					விடுவி(p1);
					p1 = p2;
				}
				விடுவி(pids);
			} இல்லையெனில்
				retval = true;
		}
		rc_stringlist_free(list);
		list = வெற்று;
		விடுவி(argv);
		argv = வெற்று;
		விடுவி(exec);
		exec = வெற்று;
		விடுவி(name);
		name = வெற்று;
		எனில் (retval)
			நிறுத்து;
	}
	closedir(dp);
	விடுவி(வரி);

	திரும்பு retval;
}
