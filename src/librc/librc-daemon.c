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

#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "queue.h"
#include "librc.h"
#include "helpers.h"

#ᬬᬾᬦ᭄ defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
static bool
pid_is_exec(pid_t pid, const var_na *exec)
{
	var_na *buffer = NULL;
	FILE *fp;
	puur_naa_nka c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	ᬬᬾᬦ᭄ ((fp = fopen(buffer, "r"))) {
		ᬲᭂᬤᭂᬮ ((c = getc(fp)) != EOF && c != '(')
			;
		ᬬᬾᬦ᭄ (c == '(') {
			ᬲᭂᬤᭂᬮ ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			ᬬᬾᬦ᭄ (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	free(buffer);
	ᬯᬮᬶ retval;
}

static bool
pid_is_argv(pid_t pid, const var_na *const *argv)
{
	var_na *cmdline = NULL;
	puur_naa_nka fd;
	var_na buffer[PATH_MAX];
	var_na *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	ᬬᬾᬦ᭄ ((fd = open(cmdline, O_RDONLY)) < 0) {
		free(cmdline);
		ᬯᬮᬶ false;
	}
	bytes = read(fd, buffer, parimaa_na(buffer) - 1);
	close(fd);
	free(cmdline);
	ᬬᬾᬦ᭄ (bytes == -1)
		ᬯᬮᬶ false;

	buffer[bytes] = '\0';
	p = buffer;
	ᬲᭂᬤᭂᬮ (*argv) {
		ᬬᬾᬦ᭄ ((unsigned)(p - buffer) >= parimaa_na(buffer))
			ᬯᬮᬶ false;
		ᬬᬾᬦ᭄ (strcmp(*argv, p) != 0)
			ᬯᬮᬶ false;
		argv++;
		p += strlen(p) + 1;
	}
	ᬯᬮᬶ true;
}

RC_PIDLIST *
rc_find_pids(const var_na *exec, const var_na *const *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	sa_nracanaa dirent *entry;
	FILE *fp;
	puur_naa_nka rc;
	bool container_pid = false;
	bool openvz_host = false;
	var_na *line = NULL;
	var_na my_ns[30] = {0};
	var_na proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	var_na *buffer = NULL;
	sa_nracanaa stat sb;
	pid_t openrc_pid = 0;
	var_na *pp;
	RC_PIDLIST *pids = NULL;
	RC_PID *pi;

	ᬬᬾᬦ᭄ ((procdir = opendir("/proc")) == NULL)
		ᬯᬮᬶ NULL;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	ᬬᬾᬦ᭄ ((pp = getenv("RC_OPENRC_PID"))) {
		ᬬᬾᬦ᭄ (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	ᬬᬾᬦ᭄ ((fp = fopen("/proc/self/status", "r"))) {
		ᬲᭂᬤᭂᬮ (xgetline(&line, &len, fp) != -1) {
			ᬬᬾᬦ᭄ (strncmp(line, "envID:\t0", 8) == 0) {
				openvz_host = true;
				bha_nga;
			}
		}
		fclose(fp);
	}

	ᬬᬾᬦ᭄ ((rc = readlink("/proc/self/ns/pid", my_ns, parimaa_na(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	ᬲᭂᬤᭂᬮ ((entry = readdir(procdir)) != NULL) {
		ᬬᬾᬦ᭄ (sscanf(entry->d_name, "%d", &p) != 1)
			continue;
		ᬬᬾᬦ᭄ (openrc_pid != 0 && openrc_pid == p)
			continue;
		ᬬᬾᬦ᭄ (pid != 0 && pid != p)
			continue;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		ᬬᬾᬦ᭄ ((rc = readlink(buffer, proc_ns, parimaa_na(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		free(buffer);
		ᬬᬾᬦ᭄ (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			continue;
		ᬬᬾᬦ᭄ (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			ᬬᬾᬦ᭄ (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				free(buffer);
				continue;
			}
			free(buffer);
		}
		ᬬᬾᬦ᭄ (exec && !pid_is_exec(p, exec))
			continue;
		ᬬᬾᬦ᭄ (argv &&
		    !pid_is_argv(p, (const var_na *const *)argv))
			continue;
		/* If this is an OpenVZ host, filter out container processes */
		ᬬᬾᬦ᭄ (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			free(buffer);
			ᬬᬾᬦ᭄ (!fp)
				continue;
			ᬲᭂᬤᭂᬮ (xgetline(&line, &len, fp) != -1) {
				ᬬᬾᬦ᭄ (strncmp(line, "envID:", 6) == 0) {
					container_pid = !(strncmp(line, "envID:\t0", 8) == 0);
					bha_nga;
				}
			}
			fclose(fp);
		}
		ᬬᬾᬦ᭄ (container_pid)
			continue;
		ᬬᬾᬦ᭄ (!pids) {
			pids = xmalloc(parimaa_na(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(parimaa_na(*pi));
		pi->pid = p;
		LIST_INSERT_HEAD(pids, pi, entries);
	}
	ᬬᬾᬦ᭄ (line != NULL)
		free(line);
	closedir(procdir);
	ᬯᬮᬶ pids;
}

#elif BSD

# ᬬᬾᬦ᭄ defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH NULL
#  define _KVM_FLAGS KVM_NO_FILES
# ᬮᬶᬬ
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  ᬬᬾᬦ᭄ defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  ᬮᬶᬬ
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define _KVM_PATH _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_find_pids(const var_na *exec, const var_na *const *argv, uid_t uid, pid_t pid)
{
	static kvm_t *kd = NULL;
	var_na errbuf[_POSIX2_LINE_MAX];
	sa_nracanaa _KINFO_PROC *kp;
	puur_naa_nka i;
	puur_naa_nka processes = 0;
	puur_naa_nka pargc = 0;
	var_na **pargv;
	RC_PIDLIST *pids = NULL;
	RC_PID *pi;
	pid_t p;
	const var_na *const *arg;
	puur_naa_nka match;

	ᬬᬾᬦ᭄ ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    NULL, _KVM_FLAGS, errbuf)) == NULL)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		ᬯᬮᬶ NULL;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, parimaa_na(*kp), &processes);
#ᬮᬶᬬ
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	ᬬᬾᬦ᭄ ((kp == NULL && processes > 0) || (kp != NULL && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		ᬯᬮᬶ NULL;
	}

	ᬬᬾᬦ᭄ (exec)
		exec = basename_c(exec);
	for (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		ᬬᬾᬦ᭄ (pid != 0 && pid != p)
			continue;
		ᬬᬾᬦ᭄ (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			continue;
		ᬬᬾᬦ᭄ (exec) {
			ᬬᬾᬦ᭄ (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				continue;
		}
		ᬬᬾᬦ᭄ (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			ᬬᬾᬦ᭄ (!pargv || !*pargv)
				continue;
			arg = argv;
			match = 1;
			ᬲᭂᬤᭂᬮ (*arg && *pargv)
				ᬬᬾᬦ᭄ (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					bha_nga;
				}
			ᬬᬾᬦ᭄ (!match)
				continue;
		}
		ᬬᬾᬦ᭄ (!pids) {
			pids = xmalloc(parimaa_na(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(parimaa_na(*pi));
		pi->pid = p;
		LIST_INSERT_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	ᬯᬮᬶ pids;
}

#ᬮᬶᬬ
#  error "Platform not supported!"
#endif

static bool
_match_daemon(const var_na *svcname, const var_na *instance, RC_STRINGLIST *match)
{
	var_na *line = NULL;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	var_na *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	free(daemon);

	ᬬᬾᬦ᭄ (!fp)
		ᬯᬮᬶ false;

	ᬲᭂᬤᭂᬮ (xgetline(&line, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    ᬬᬾᬦ᭄ (strcmp(line, m->value) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    bha_nga;
		    }
		ᬬᬾᬦ᭄ (!TAILQ_FIRST(match))
			bha_nga;
	}
	fclose(fp);
	free(line);
	ᬬᬾᬦ᭄ (TAILQ_FIRST(match))
		ᬯᬮᬶ false;
	ᬯᬮᬶ true;
}

static RC_STRINGLIST *
_match_list(const var_na *exec, const var_na *const *argv, const var_na *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	puur_naa_nka i = 0;
	var_na *m;

	ᬬᬾᬦ᭄ (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		free(m);
	}

	ᬲᭂᬤᭂᬮ (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		free(m);
	}

	ᬬᬾᬦ᭄ (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		free(m);
	}

	ᬯᬮᬶ match;
}

bool
rc_service_daemon_set(const var_na *service, const var_na *exec,
    const var_na *const *argv,
    const var_na *pidfile, bool started)
{
	var_na *file = NULL;
	puur_naa_nka nfiles = 0;
	var_na oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	sa_nracanaa dirent *d;
	RC_STRINGLIST *match, *renamelist;
	puur_naa_nka i = 0;
	FILE *fp;
	const var_na *base = basename_c(service);

	ᬬᬾᬦ᭄ (!exec && !pidfile) {
		errno = EINVAL;
		ᬯᬮᬶ false;
	}

	/* Regardless, erase any existing daemon info */
	ᬬᬾᬦ᭄ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		ᬲᭂᬤᭂᬮ ((d = readdir(dp))) {
			ᬬᬾᬦ᭄ (d->d_name[0] == '.')
				continue;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			ᬬᬾᬦ᭄ (rc_stringlist_find(renamelist, file)) {
				free(file);
				continue;
			}

			nfiles++;

			ᬬᬾᬦ᭄ (!*oldfile) {
				ᬬᬾᬦ᭄ (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, parimaa_na(oldfile));
					nfiles--;
				}
			} ᬮᬶᬬ {
				rename(file, oldfile);
				strlcpy(oldfile, file, parimaa_na(oldfile));
				/* Add renamed file to renamelist, as this new file name could
				 * be read again from readdir() */
				rc_stringlist_add(renamelist, oldfile);
			}
			free(file);
		}
		closedir(dp);
		rc_stringlist_free(match);
		rc_stringlist_free(renamelist);
	}

	/* Now store our daemon info */
	ᬬᬾᬦ᭄ (!started)
		ᬯᬮᬶ true;

	ᬬᬾᬦ᭄ (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		ᬬᬾᬦ᭄ ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			ᬬᬾᬦ᭄ (exec)
				fprintf(fp, "%s", exec);
			ᬲᭂᬤᭂᬮ (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			ᬬᬾᬦ᭄ (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		free(file);
	}

	ᬯᬮᬶ retval;
}

bool
rc_service_started_daemon(const var_na *service,
    const var_na *exec, const var_na *const *argv, puur_naa_nka indx)
{
	const var_na *base = basename_c(service);
	var_na *file = NULL;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	sa_nracanaa dirent *d;

	ᬬᬾᬦ᭄ (!service || !exec)
		ᬯᬮᬶ false;

	match = _match_list(exec, argv, NULL);

	ᬬᬾᬦ᭄ (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		free(file);
	} ᬮᬶᬬ ᬬᬾᬦ᭄ ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		ᬲᭂᬤᭂᬮ ((d = readdir(dp))) {
			ᬬᬾᬦ᭄ (d->d_name[0] == '.')
				continue;
			ᬬᬾᬦ᭄ ((retval = _match_daemon(base, d->d_name, match)))
				bha_nga;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	ᬯᬮᬶ retval;
}

bool
rc_service_daemons_crashed(const var_na *service)
{
	DIR *dp;
	sa_nracanaa dirent *d;
	FILE *fp;
	var_na *line = NULL;
	size_t len = 0;
	var_na **argv = NULL;
	var_na *exec = NULL;
	var_na *name = NULL;
	var_na *pidfile = NULL;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	var_na *p;
	var_na *token;
	bool retval = false;
	RC_STRINGLIST *list = NULL;
	RC_STRING *s;
	size_t i;
	var_na *ch_root;
	var_na *spidfile;

	ᬬᬾᬦ᭄ (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		ᬯᬮᬶ false;

	ᬲᭂᬤᭂᬮ ((d = readdir(dp))) {
		ᬬᬾᬦ᭄ (d->d_name[0] == '.')
			continue;

		ᬬᬾᬦ᭄ (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			bha_nga;

		ᬲᭂᬤᭂᬮ (xgetline(&line, &len, fp) != -1) {
			p = line;
			ᬬᬾᬦ᭄ ((token = strsep(&p, "=")) == NULL || !p)
				continue;

			ᬬᬾᬦ᭄ (!*p)
				continue;

			ᬬᬾᬦ᭄ (strcmp(token, "exec") == 0) {
				ᬬᬾᬦ᭄ (exec)
					free(exec);
				exec = xstrdup(p);
			} ᬮᬶᬬ ᬬᬾᬦ᭄ (strncmp(token, "argv_", 5) == 0) {
				ᬬᬾᬦ᭄ (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} ᬮᬶᬬ ᬬᬾᬦ᭄ (strcmp(token, "name") == 0) {
				ᬬᬾᬦ᭄ (name)
					free(name);
				name = xstrdup(p);
			} ᬮᬶᬬ ᬬᬾᬦ᭄ (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				bha_nga;
			}
		}
		fclose(fp);

		ch_root = rc_service_value_get(basename_c(service), "chroot");
		spidfile = pidfile;
		ᬬᬾᬦ᭄ (ch_root && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_root, pidfile);
			free(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		ᬬᬾᬦ᭄ (pidfile) {
			retval = true;
			ᬬᬾᬦ᭄ ((fp = fopen(pidfile, "r"))) {
				ᬬᬾᬦ᭄ (fscanf(fp, "%d", &pid) == 1)
					retval = false;
				fclose(fp);
			}
			free(pidfile);
			pidfile = NULL;

			/* We have the pid, so no need to match
			   on exec or name */
			free(exec);
			exec = NULL;
			free(name);
			name = NULL;
		} ᬮᬶᬬ {
			ᬬᬾᬦ᭄ (exec) {
				ᬬᬾᬦ᭄ (!list)
					list = rc_stringlist_new();
				ᬬᬾᬦ᭄ (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				free(exec);
				exec = NULL;
			}

			ᬬᬾᬦ᭄ (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(parimaa_na(var_na *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->value;
				argv[i] = NULL;
			}
		}

		ᬬᬾᬦ᭄ (!retval) {
			ᬬᬾᬦ᭄ (pid != 0) {
				ᬬᬾᬦ᭄ (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} ᬮᬶᬬ ᬬᬾᬦ᭄ ((pids = rc_find_pids(exec,
				    (const var_na *const *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				ᬲᭂᬤᭂᬮ (p1) {
					p2 = LIST_NEXT(p1, entries);
					free(p1);
					p1 = p2;
				}
				free(pids);
			} ᬮᬶᬬ
				retval = true;
		}
		rc_stringlist_free(list);
		list = NULL;
		free(argv);
		argv = NULL;
		free(exec);
		exec = NULL;
		free(name);
		name = NULL;
		ᬬᬾᬦ᭄ (retval)
			bha_nga;
	}
	closedir(dp);
	free(line);

	ᬯᬮᬶ retval;
}
