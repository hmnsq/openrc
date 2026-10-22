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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <dirent.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "librc.h"
#અન્તર્ભાવ "helpers.h"

#જો defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
સ્થિર bool
pid_is_exec(pid_t pid, અચલ અક્ષર *exec)
{
	અક્ષર *buffer = ખાલી;
	FILE *fp;
	પૂર્ણાંક c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	જો ((fp = fopen(buffer, "r"))) {
		જ્યારે ((c = getc(fp)) != EOF && c != '(')
			;
		જો (c == '(') {
			જ્યારે ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			જો (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	મુક્ત(buffer);
	પાછા retval;
}

સ્થિર bool
pid_is_argv(pid_t pid, અચલ અક્ષર *અચલ *argv)
{
	અક્ષર *cmdline = ખાલી;
	પૂર્ણાંક fd;
	અક્ષર buffer[PATH_MAX];
	અક્ષર *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	જો ((fd = open(cmdline, O_RDONLY)) < 0) {
		મુક્ત(cmdline);
		પાછા false;
	}
	bytes = પઠ(fd, buffer, કદ(buffer) - 1);
	close(fd);
	મુક્ત(cmdline);
	જો (bytes == -1)
		પાછા false;

	buffer[bytes] = '\0';
	p = buffer;
	જ્યારે (*argv) {
		જો ((unsigned)(p - buffer) >= કદ(buffer))
			પાછા false;
		જો (strcmp(*argv, p) != 0)
			પાછા false;
		argv++;
		p += strlen(p) + 1;
	}
	પાછા true;
}

RC_PIDLIST *
rc_find_pids(અચલ અક્ષર *exec, અચલ અક્ષર *અચલ *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	સંરચના dirent *entry;
	FILE *fp;
	પૂર્ણાંક rc;
	bool container_pid = false;
	bool openvz_host = false;
	અક્ષર *લીટી = ખાલી;
	અક્ષર my_ns[30] = {0};
	અક્ષર proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	અક્ષર *buffer = ખાલી;
	સંરચના stat sb;
	pid_t openrc_pid = 0;
	અક્ષર *pp;
	RC_PIDLIST *pids = ખાલી;
	RC_PID *pi;

	જો ((procdir = opendir("/proc")) == ખાલી)
		પાછા ખાલી;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	જો ((pp = getenv("RC_OPENRC_PID"))) {
		જો (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	જો ((fp = fopen("/proc/self/status", "r"))) {
		જ્યારે (xgetline(&લીટી, &len, fp) != -1) {
			જો (strncmp(લીટી, "envID:\t0", 8) == 0) {
				openvz_host = true;
				તોડો;
			}
		}
		fclose(fp);
	}

	જો ((rc = readlink("/proc/self/ns/pid", my_ns, કદ(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	જ્યારે ((entry = readdir(procdir)) != ખાલી) {
		જો (sscanf(entry->d_name, "%d", &p) != 1)
			ચાલુ;
		જો (openrc_pid != 0 && openrc_pid == p)
			ચાલુ;
		જો (pid != 0 && pid != p)
			ચાલુ;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		જો ((rc = readlink(buffer, proc_ns, કદ(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		મુક્ત(buffer);
		જો (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			ચાલુ;
		જો (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			જો (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				મુક્ત(buffer);
				ચાલુ;
			}
			મુક્ત(buffer);
		}
		જો (exec && !pid_is_exec(p, exec))
			ચાલુ;
		જો (argv &&
		    !pid_is_argv(p, (અચલ અક્ષર *અચલ *)argv))
			ચાલુ;
		/* If this is an OpenVZ host, filter out container processes */
		જો (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			મુક્ત(buffer);
			જો (!fp)
				ચાલુ;
			જ્યારે (xgetline(&લીટી, &len, fp) != -1) {
				જો (strncmp(લીટી, "envID:", 6) == 0) {
					container_pid = !(strncmp(લીટી, "envID:\t0", 8) == 0);
					તોડો;
				}
			}
			fclose(fp);
		}
		જો (container_pid)
			ચાલુ;
		જો (!pids) {
			pids = xmalloc(કદ(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(કદ(*pi));
		pi->pid = p;
		LIST_ઉમેરો_HEAD(pids, pi, entries);
	}
	જો (લીટી != ખાલી)
		મુક્ત(લીટી);
	closedir(procdir);
	પાછા pids;
}

#અથવાજો BSD

# જો defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH ખાલી
#  define _KVM_FLAGS KVM_NO_FILES
# નહીંતર
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  જો defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  નહીંતર
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define _KVM_PATH _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_find_pids(અચલ અક્ષર *exec, અચલ અક્ષર *અચલ *argv, uid_t uid, pid_t pid)
{
	સ્થિર kvm_t *kd = ખાલી;
	અક્ષર errbuf[_POSIX2_LINE_MAX];
	સંરચના _KINFO_PROC *kp;
	પૂર્ણાંક i;
	પૂર્ણાંક processes = 0;
	પૂર્ણાંક pargc = 0;
	અક્ષર **pargv;
	RC_PIDLIST *pids = ખાલી;
	RC_PID *pi;
	pid_t p;
	અચલ અક્ષર *અચલ *arg;
	પૂર્ણાંક match;

	જો ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    ખાલી, _KVM_FLAGS, errbuf)) == ખાલી)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		પાછા ખાલી;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, કદ(*kp), &processes);
#નહીંતર
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	જો ((kp == ખાલી && processes > 0) || (kp != ખાલી && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		પાછા ખાલી;
	}

	જો (exec)
		exec = basename_c(exec);
	માટે (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		જો (pid != 0 && pid != p)
			ચાલુ;
		જો (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			ચાલુ;
		જો (exec) {
			જો (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				ચાલુ;
		}
		જો (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			જો (!pargv || !*pargv)
				ચાલુ;
			arg = argv;
			match = 1;
			જ્યારે (*arg && *pargv)
				જો (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					તોડો;
				}
			જો (!match)
				ચાલુ;
		}
		જો (!pids) {
			pids = xmalloc(કદ(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(કદ(*pi));
		pi->pid = p;
		LIST_ઉમેરો_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	પાછા pids;
}

#નહીંતર
#  error "Platform not supported!"
#endif

સ્થિર bool
_match_daemon(અચલ અક્ષર *svcname, અચલ અક્ષર *instance, RC_STRINGLIST *match)
{
	અક્ષર *લીટી = ખાલી;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	અક્ષર *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	મુક્ત(daemon);

	જો (!fp)
		પાછા false;

	જ્યારે (xgetline(&લીટી, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    જો (strcmp(લીટી, m->કિંમત) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    તોડો;
		    }
		જો (!TAILQ_FIRST(match))
			તોડો;
	}
	fclose(fp);
	મુક્ત(લીટી);
	જો (TAILQ_FIRST(match))
		પાછા false;
	પાછા true;
}

સ્થિર RC_STRINGLIST *
_match_list(અચલ અક્ષર *exec, અચલ અક્ષર *અચલ *argv, અચલ અક્ષર *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	પૂર્ણાંક i = 0;
	અક્ષર *m;

	જો (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		મુક્ત(m);
	}

	જ્યારે (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		મુક્ત(m);
	}

	જો (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		મુક્ત(m);
	}

	પાછા match;
}

bool
rc_service_daemon_set(અચલ અક્ષર *service, અચલ અક્ષર *exec,
    અચલ અક્ષર *અચલ *argv,
    અચલ અક્ષર *pidfile, bool started)
{
	અક્ષર *file = ખાલી;
	પૂર્ણાંક nfiles = 0;
	અક્ષર oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	સંરચના dirent *d;
	RC_STRINGLIST *match, *renamelist;
	પૂર્ણાંક i = 0;
	FILE *fp;
	અચલ અક્ષર *base = basename_c(service);

	જો (!exec && !pidfile) {
		errno = EINVAL;
		પાછા false;
	}

	/* Regardless, erase any existing daemon info */
	જો ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		જ્યારે ((d = readdir(dp))) {
			જો (d->d_name[0] == '.')
				ચાલુ;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			જો (rc_stringlist_find(renamelist, file)) {
				મુક્ત(file);
				ચાલુ;
			}

			nfiles++;

			જો (!*oldfile) {
				જો (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, કદ(oldfile));
					nfiles--;
				}
			} નહીંતર {
				rename(file, oldfile);
				strlcpy(oldfile, file, કદ(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_add(renamelist, oldfile);
			}
			મુક્ત(file);
		}
		closedir(dp);
		rc_stringlist_free(match);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	જો (!started)
		પાછા true;

	જો (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		જો ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			જો (exec)
				fprintf(fp, "%s", exec);
			જ્યારે (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			જો (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		મુક્ત(file);
	}

	પાછા retval;
}

bool
rc_service_started_daemon(અચલ અક્ષર *service,
    અચલ અક્ષર *exec, અચલ અક્ષર *અચલ *argv, પૂર્ણાંક indx)
{
	અચલ અક્ષર *base = basename_c(service);
	અક્ષર *file = ખાલી;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	સંરચના dirent *d;

	જો (!service || !exec)
		પાછા false;

	match = _match_list(exec, argv, ખાલી);

	જો (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		મુક્ત(file);
	} નહીંતર જો ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		જ્યારે ((d = readdir(dp))) {
			જો (d->d_name[0] == '.')
				ચાલુ;
			જો ((retval = _match_daemon(base, d->d_name, match)))
				તોડો;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	પાછા retval;
}

bool
rc_service_daemons_crashed(અચલ અક્ષર *service)
{
	DIR *dp;
	સંરચના dirent *d;
	FILE *fp;
	અક્ષર *લીટી = ખાલી;
	size_t len = 0;
	અક્ષર **argv = ખાલી;
	અક્ષર *exec = ખાલી;
	અક્ષર *name = ખાલી;
	અક્ષર *pidfile = ખાલી;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	અક્ષર *p;
	અક્ષર *token;
	bool retval = false;
	RC_STRINGLIST *list = ખાલી;
	RC_STRING *s;
	size_t i;
	અક્ષર *ch_મૂળ;
	અક્ષર *spidfile;

	જો (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		પાછા false;

	જ્યારે ((d = readdir(dp))) {
		જો (d->d_name[0] == '.')
			ચાલુ;

		જો (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			તોડો;

		જ્યારે (xgetline(&લીટી, &len, fp) != -1) {
			p = લીટી;
			જો ((token = strsep(&p, "=")) == ખાલી || !p)
				ચાલુ;

			જો (!*p)
				ચાલુ;

			જો (strcmp(token, "exec") == 0) {
				જો (exec)
					મુક્ત(exec);
				exec = xstrdup(p);
			} નહીંતર જો (strncmp(token, "argv_", 5) == 0) {
				જો (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} નહીંતર જો (strcmp(token, "name") == 0) {
				જો (name)
					મુક્ત(name);
				name = xstrdup(p);
			} નહીંતર જો (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				તોડો;
			}
		}
		fclose(fp);

		ch_મૂળ = rc_service_કિંમત_get(basename_c(service), "chroot");
		spidfile = pidfile;
		જો (ch_મૂળ && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_મૂળ, pidfile);
			મુક્ત(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		જો (pidfile) {
			retval = true;
			જો ((fp = fopen(pidfile, "r"))) {
				જો (fscanf(fp, "%d", &pid) == 1)
					retval = false;
				fclose(fp);
			}
			મુક્ત(pidfile);
			pidfile = ખાલી;

			/* We have the pid, so no need to match
			   on exec or name */
			મુક્ત(exec);
			exec = ખાલી;
			મુક્ત(name);
			name = ખાલી;
		} નહીંતર {
			જો (exec) {
				જો (!list)
					list = rc_stringlist_new();
				જો (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				મુક્ત(exec);
				exec = ખાલી;
			}

			જો (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(કદ(અક્ષર *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->કિંમત;
				argv[i] = ખાલી;
			}
		}

		જો (!retval) {
			જો (pid != 0) {
				જો (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} નહીંતર જો ((pids = rc_find_pids(exec,
				    (અચલ અક્ષર *અચલ *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				જ્યારે (p1) {
					p2 = LIST_NEXT(p1, entries);
					મુક્ત(p1);
					p1 = p2;
				}
				મુક્ત(pids);
			} નહીંતર
				retval = true;
		}
		rc_stringlist_free(list);
		list = ખાલી;
		મુક્ત(argv);
		argv = ખાલી;
		મુક્ત(exec);
		exec = ખાલી;
		મુક્ત(name);
		name = ખાલી;
		જો (retval)
			તોડો;
	}
	closedir(dp);
	મુક્ત(લીટી);

	પાછા retval;
}
