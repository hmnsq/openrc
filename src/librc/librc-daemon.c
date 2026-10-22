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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <dirent.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "librc.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

#ଯଦି defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
ସ୍ଥିର bool
pid_is_exec(pid_t pid, ଅଚଳ ଅକ୍ଷର *exec)
{
	ଅକ୍ଷର *buffer = ଖାଲି;
	FILE *fp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	ଯଦି ((fp = fopen(buffer, "r"))) {
		ଯେତେବେଳେ ((c = getc(fp)) != EOF && c != '(')
			;
		ଯଦି (c == '(') {
			ଯେତେବେଳେ ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			ଯଦି (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	ମୁକ୍ତ(buffer);
	ଫେରନ୍ତୁ retval;
}

ସ୍ଥିର bool
pid_is_argv(pid_t pid, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv)
{
	ଅକ୍ଷର *cmdline = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
	ଅକ୍ଷର buffer[PATH_MAX];
	ଅକ୍ଷର *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	ଯଦି ((fd = open(cmdline, O_RDONLY)) < 0) {
		ମୁକ୍ତ(cmdline);
		ଫେରନ୍ତୁ false;
	}
	bytes = ପଠ(fd, buffer, ଆକାର(buffer) - 1);
	close(fd);
	ମୁକ୍ତ(cmdline);
	ଯଦି (bytes == -1)
		ଫେରନ୍ତୁ false;

	buffer[bytes] = '\0';
	p = buffer;
	ଯେତେବେଳେ (*argv) {
		ଯଦି ((unsigned)(p - buffer) >= ଆକାର(buffer))
			ଫେରନ୍ତୁ false;
		ଯଦି (strcmp(*argv, p) != 0)
			ଫେରନ୍ତୁ false;
		argv++;
		p += strlen(p) + 1;
	}
	ଫେରନ୍ତୁ true;
}

RC_PIDLIST *
rc_find_pids(ଅଚଳ ଅକ୍ଷର *exec, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	ସଂରଚନା dirent *entry;
	FILE *fp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	bool container_pid = false;
	bool openvz_host = false;
	ଅକ୍ଷର *ଧାଡ଼ି = ଖାଲି;
	ଅକ୍ଷର my_ns[30] = {0};
	ଅକ୍ଷର proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	ଅକ୍ଷର *buffer = ଖାଲି;
	ସଂରଚନା stat sb;
	pid_t openrc_pid = 0;
	ଅକ୍ଷର *pp;
	RC_PIDLIST *pids = ଖାଲି;
	RC_PID *pi;

	ଯଦି ((procdir = opendir("/proc")) == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	ଯଦି ((pp = getenv("RC_OPENRC_PID"))) {
		ଯଦି (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	ଯଦି ((fp = fopen("/proc/self/status", "r"))) {
		ଯେତେବେଳେ (xgetline(&ଧାଡ଼ି, &len, fp) != -1) {
			ଯଦି (strncmp(ଧାଡ଼ି, "envID:\t0", 8) == 0) {
				openvz_host = true;
				ଭାଙ୍ଗ;
			}
		}
		fclose(fp);
	}

	ଯଦି ((rc = readlink("/proc/self/ns/pid", my_ns, ଆକାର(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	ଯେତେବେଳେ ((entry = readdir(procdir)) != ଖାଲି) {
		ଯଦି (sscanf(entry->d_name, "%d", &p) != 1)
			ଜାରି;
		ଯଦି (openrc_pid != 0 && openrc_pid == p)
			ଜାରି;
		ଯଦି (pid != 0 && pid != p)
			ଜାରି;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		ଯଦି ((rc = readlink(buffer, proc_ns, ଆକାର(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		ମୁକ୍ତ(buffer);
		ଯଦି (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			ଜାରି;
		ଯଦି (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			ଯଦି (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				ମୁକ୍ତ(buffer);
				ଜାରି;
			}
			ମୁକ୍ତ(buffer);
		}
		ଯଦି (exec && !pid_is_exec(p, exec))
			ଜାରି;
		ଯଦି (argv &&
		    !pid_is_argv(p, (ଅଚଳ ଅକ୍ଷର *ଅଚଳ *)argv))
			ଜାରି;
		/* If this is an OpenVZ host, filter out container processes */
		ଯଦି (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			ମୁକ୍ତ(buffer);
			ଯଦି (!fp)
				ଜାରି;
			ଯେତେବେଳେ (xgetline(&ଧାଡ଼ି, &len, fp) != -1) {
				ଯଦି (strncmp(ଧାଡ଼ି, "envID:", 6) == 0) {
					container_pid = !(strncmp(ଧାଡ଼ି, "envID:\t0", 8) == 0);
					ଭାଙ୍ଗ;
				}
			}
			fclose(fp);
		}
		ଯଦି (container_pid)
			ଜାରି;
		ଯଦି (!pids) {
			pids = xmalloc(ଆକାର(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(ଆକାର(*pi));
		pi->pid = p;
		LIST_ଭର୍ତ୍ତି_HEAD(pids, pi, entries);
	}
	ଯଦି (ଧାଡ଼ି != ଖାଲି)
		ମୁକ୍ତ(ଧାଡ଼ି);
	closedir(procdir);
	ଫେରନ୍ତୁ pids;
}

#ନଚେତଯଦି BSD

# ଯଦି defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH ଖାଲି
#  define _KVM_FLAGS KVM_NO_FILES
# ଅନ୍ୟଥା
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  ଯଦି defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  ଅନ୍ୟଥା
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define _KVM_PATH _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_find_pids(ଅଚଳ ଅକ୍ଷର *exec, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv, uid_t uid, pid_t pid)
{
	ସ୍ଥିର kvm_t *kd = ଖାଲି;
	ଅକ୍ଷର errbuf[_POSIX2_LINE_MAX];
	ସଂରଚନା _KINFO_PROC *kp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା processes = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pargc = 0;
	ଅକ୍ଷର **pargv;
	RC_PIDLIST *pids = ଖାଲି;
	RC_PID *pi;
	pid_t p;
	ଅଚଳ ଅକ୍ଷର *ଅଚଳ *arg;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା match;

	ଯଦି ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    ଖାଲି, _KVM_FLAGS, errbuf)) == ଖାଲି)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		ଫେରନ୍ତୁ ଖାଲି;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, ଆକାର(*kp), &processes);
#ଅନ୍ୟଥା
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	ଯଦି ((kp == ଖାଲି && processes > 0) || (kp != ଖାଲି && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (exec)
		exec = basename_c(exec);
	ପାଇଁ (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		ଯଦି (pid != 0 && pid != p)
			ଜାରି;
		ଯଦି (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			ଜାରି;
		ଯଦି (exec) {
			ଯଦି (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				ଜାରି;
		}
		ଯଦି (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			ଯଦି (!pargv || !*pargv)
				ଜାରି;
			arg = argv;
			match = 1;
			ଯେତେବେଳେ (*arg && *pargv)
				ଯଦି (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					ଭାଙ୍ଗ;
				}
			ଯଦି (!match)
				ଜାରି;
		}
		ଯଦି (!pids) {
			pids = xmalloc(ଆକାର(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(ଆକାର(*pi));
		pi->pid = p;
		LIST_ଭର୍ତ୍ତି_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	ଫେରନ୍ତୁ pids;
}

#ଅନ୍ୟଥା
#  error "Platform not supported!"
#endif

ସ୍ଥିର bool
_match_daemon(ଅଚଳ ଅକ୍ଷର *svcname, ଅଚଳ ଅକ୍ଷର *instance, RC_STRINGLIST *match)
{
	ଅକ୍ଷର *ଧାଡ଼ି = ଖାଲି;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	ଅକ୍ଷର *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	ମୁକ୍ତ(daemon);

	ଯଦି (!fp)
		ଫେରନ୍ତୁ false;

	ଯେତେବେଳେ (xgetline(&ଧାଡ଼ି, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    ଯଦି (strcmp(ଧାଡ଼ି, m->ମୂଲ୍ୟ) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    ଭାଙ୍ଗ;
		    }
		ଯଦି (!TAILQ_FIRST(match))
			ଭାଙ୍ଗ;
	}
	fclose(fp);
	ମୁକ୍ତ(ଧାଡ଼ି);
	ଯଦି (TAILQ_FIRST(match))
		ଫେରନ୍ତୁ false;
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର RC_STRINGLIST *
_match_list(ଅଚଳ ଅକ୍ଷର *exec, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv, ଅଚଳ ଅକ୍ଷର *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
	ଅକ୍ଷର *m;

	ଯଦି (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		ମୁକ୍ତ(m);
	}

	ଯେତେବେଳେ (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		ମୁକ୍ତ(m);
	}

	ଯଦି (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		ମୁକ୍ତ(m);
	}

	ଫେରନ୍ତୁ match;
}

bool
rc_service_daemon_set(ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ ଅକ୍ଷର *exec,
    ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv,
    ଅଚଳ ଅକ୍ଷର *pidfile, bool started)
{
	ଅକ୍ଷର *file = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfiles = 0;
	ଅକ୍ଷର oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	ସଂରଚନା dirent *d;
	RC_STRINGLIST *match, *renamelist;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
	FILE *fp;
	ଅଚଳ ଅକ୍ଷର *base = basename_c(service);

	ଯଦି (!exec && !pidfile) {
		errno = EINVAL;
		ଫେରନ୍ତୁ false;
	}

	/* Regardless, erase any existing daemon info */
	ଯଦି ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		ଯେତେବେଳେ ((d = readdir(dp))) {
			ଯଦି (d->d_name[0] == '.')
				ଜାରି;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			ଯଦି (rc_stringlist_find(renamelist, file)) {
				ମୁକ୍ତ(file);
				ଜାରି;
			}

			nfiles++;

			ଯଦି (!*oldfile) {
				ଯଦି (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, ଆକାର(oldfile));
					nfiles--;
				}
			} ଅନ୍ୟଥା {
				rename(file, oldfile);
				strlcpy(oldfile, file, ଆକାର(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_add(renamelist, oldfile);
			}
			ମୁକ୍ତ(file);
		}
		closedir(dp);
		rc_stringlist_free(match);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	ଯଦି (!started)
		ଫେରନ୍ତୁ true;

	ଯଦି (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		ଯଦି ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			ଯଦି (exec)
				fprintf(fp, "%s", exec);
			ଯେତେବେଳେ (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			ଯଦି (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		ମୁକ୍ତ(file);
	}

	ଫେରନ୍ତୁ retval;
}

bool
rc_service_started_daemon(ଅଚଳ ଅକ୍ଷର *service,
    ଅଚଳ ଅକ୍ଷର *exec, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv, ପୂର୍ଣ୍ଣସଂଖ୍ୟା indx)
{
	ଅଚଳ ଅକ୍ଷର *base = basename_c(service);
	ଅକ୍ଷର *file = ଖାଲି;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	ସଂରଚନା dirent *d;

	ଯଦି (!service || !exec)
		ଫେରନ୍ତୁ false;

	match = _match_list(exec, argv, ଖାଲି);

	ଯଦି (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		ମୁକ୍ତ(file);
	} ଅନ୍ୟଥା ଯଦି ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		ଯେତେବେଳେ ((d = readdir(dp))) {
			ଯଦି (d->d_name[0] == '.')
				ଜାରି;
			ଯଦି ((retval = _match_daemon(base, d->d_name, match)))
				ଭାଙ୍ଗ;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	ଫେରନ୍ତୁ retval;
}

bool
rc_service_daemons_crashed(ଅଚଳ ଅକ୍ଷର *service)
{
	DIR *dp;
	ସଂରଚନା dirent *d;
	FILE *fp;
	ଅକ୍ଷର *ଧାଡ଼ି = ଖାଲି;
	size_t len = 0;
	ଅକ୍ଷର **argv = ଖାଲି;
	ଅକ୍ଷର *exec = ଖାଲି;
	ଅକ୍ଷର *name = ଖାଲି;
	ଅକ୍ଷର *pidfile = ଖାଲି;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *token;
	bool retval = false;
	RC_STRINGLIST *list = ଖାଲି;
	RC_STRING *s;
	size_t i;
	ଅକ୍ଷର *ch_ମୂଳ;
	ଅକ୍ଷର *spidfile;

	ଯଦି (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		ଫେରନ୍ତୁ false;

	ଯେତେବେଳେ ((d = readdir(dp))) {
		ଯଦି (d->d_name[0] == '.')
			ଜାରି;

		ଯଦି (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			ଭାଙ୍ଗ;

		ଯେତେବେଳେ (xgetline(&ଧାଡ଼ି, &len, fp) != -1) {
			p = ଧାଡ଼ି;
			ଯଦି ((token = strsep(&p, "=")) == ଖାଲି || !p)
				ଜାରି;

			ଯଦି (!*p)
				ଜାରି;

			ଯଦି (strcmp(token, "exec") == 0) {
				ଯଦି (exec)
					ମୁକ୍ତ(exec);
				exec = xstrdup(p);
			} ଅନ୍ୟଥା ଯଦି (strncmp(token, "argv_", 5) == 0) {
				ଯଦି (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} ଅନ୍ୟଥା ଯଦି (strcmp(token, "name") == 0) {
				ଯଦି (name)
					ମୁକ୍ତ(name);
				name = xstrdup(p);
			} ଅନ୍ୟଥା ଯଦି (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				ଭାଙ୍ଗ;
			}
		}
		fclose(fp);

		ch_ମୂଳ = rc_service_ମୂଲ୍ୟ_get(basename_c(service), "chroot");
		spidfile = pidfile;
		ଯଦି (ch_ମୂଳ && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_ମୂଳ, pidfile);
			ମୁକ୍ତ(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		ଯଦି (pidfile) {
			retval = true;
			ଯଦି ((fp = fopen(pidfile, "r"))) {
				ଯଦି (fscanf(fp, "%d", &pid) == 1)
					retval = false;
				fclose(fp);
			}
			ମୁକ୍ତ(pidfile);
			pidfile = ଖାଲି;

			/* We have the pid, so no need to match
			   on exec or name */
			ମୁକ୍ତ(exec);
			exec = ଖାଲି;
			ମୁକ୍ତ(name);
			name = ଖାଲି;
		} ଅନ୍ୟଥା {
			ଯଦି (exec) {
				ଯଦି (!list)
					list = rc_stringlist_new();
				ଯଦି (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				ମୁକ୍ତ(exec);
				exec = ଖାଲି;
			}

			ଯଦି (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(ଆକାର(ଅକ୍ଷର *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->ମୂଲ୍ୟ;
				argv[i] = ଖାଲି;
			}
		}

		ଯଦି (!retval) {
			ଯଦି (pid != 0) {
				ଯଦି (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} ଅନ୍ୟଥା ଯଦି ((pids = rc_find_pids(exec,
				    (ଅଚଳ ଅକ୍ଷର *ଅଚଳ *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				ଯେତେବେଳେ (p1) {
					p2 = LIST_NEXT(p1, entries);
					ମୁକ୍ତ(p1);
					p1 = p2;
				}
				ମୁକ୍ତ(pids);
			} ଅନ୍ୟଥା
				retval = true;
		}
		rc_stringlist_free(list);
		list = ଖାଲି;
		ମୁକ୍ତ(argv);
		argv = ଖାଲି;
		ମୁକ୍ତ(exec);
		exec = ଖାଲି;
		ମୁକ୍ତ(name);
		name = ଖାଲି;
		ଯଦି (retval)
			ଭାଙ୍ଗ;
	}
	closedir(dp);
	ମୁକ୍ତ(ଧାଡ଼ି);

	ଫେରନ୍ତୁ retval;
}
