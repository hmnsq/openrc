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

#ако defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
static bool
pid_is_exec(pid_t pid, const char *exec)
{
	char *buffer = NULL;
	FILE *fp;
	цяло c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	ако ((fp = fopen(buffer, "r"))) {
		докато ((c = getc(fp)) != EOF && c != '(')
			;
		ако (c == '(') {
			докато ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			ако (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	free(buffer);
	върни retval;
}

static bool
pid_is_argv(pid_t pid, const char *const *argv)
{
	char *cmdline = NULL;
	цяло fd;
	char buffer[PATH_MAX];
	char *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	ако ((fd = open(cmdline, O_RDONLY)) < 0) {
		free(cmdline);
		върни false;
	}
	bytes = read(fd, buffer, sizeof(buffer) - 1);
	close(fd);
	free(cmdline);
	ако (bytes == -1)
		върни false;

	buffer[bytes] = '\0';
	p = buffer;
	докато (*argv) {
		ако ((unsigned)(p - buffer) >= sizeof(buffer))
			върни false;
		ако (strcmp(*argv, p) != 0)
			върни false;
		argv++;
		p += strlen(p) + 1;
	}
	върни true;
}

RC_PIDLIST *
rc_find_pids(const char *exec, const char *const *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	struct dirent *entry;
	FILE *fp;
	цяло rc;
	bool container_pid = false;
	bool openvz_host = false;
	char *line = NULL;
	char my_ns[30] = {0};
	char proc_ns[30] = {0};
	size_t len = 0;
	pid_t p;
	char *buffer = NULL;
	struct stat sb;
	pid_t openrc_pid = 0;
	char *pp;
	RC_PIDLIST *pids = NULL;
	RC_PID *pi;

	ако ((procdir = opendir("/proc")) == NULL)
		върни NULL;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	ако ((pp = getenv("RC_OPENRC_PID"))) {
		ако (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	ако ((fp = fopen("/proc/self/status", "r"))) {
		докато (xgetline(&line, &len, fp) != -1) {
			ако (strncmp(line, "envID:\t0", 8) == 0) {
				openvz_host = true;
				break;
			}
		}
		fclose(fp);
	}

	ако ((rc = readlink("/proc/self/ns/pid", my_ns, sizeof(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	докато ((entry = readdir(procdir)) != NULL) {
		ако (sscanf(entry->d_name, "%d", &p) != 1)
			continue;
		ако (openrc_pid != 0 && openrc_pid == p)
			continue;
		ако (pid != 0 && pid != p)
			continue;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		ако ((rc = readlink(buffer, proc_ns, sizeof(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		free(buffer);
		ако (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			continue;
		ако (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			ако (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				free(buffer);
				continue;
			}
			free(buffer);
		}
		ако (exec && !pid_is_exec(p, exec))
			continue;
		ако (argv &&
		    !pid_is_argv(p, (const char *const *)argv))
			continue;
		/* If this is an OpenVZ host, filter out container processes */
		ако (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			free(buffer);
			ако (!fp)
				continue;
			докато (xgetline(&line, &len, fp) != -1) {
				ако (strncmp(line, "envID:", 6) == 0) {
					container_pid = !(strncmp(line, "envID:\t0", 8) == 0);
					break;
				}
			}
			fclose(fp);
		}
		ако (container_pid)
			continue;
		ако (!pids) {
			pids = xmalloc(sizeof(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(sizeof(*pi));
		pi->pid = p;
		LIST_INSERT_HEAD(pids, pi, entries);
	}
	ако (line != NULL)
		free(line);
	closedir(procdir);
	върни pids;
}

#elif BSD

# ако defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH NULL
#  define _KVM_FLAGS KVM_NO_FILES
# иначе
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  ако defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  иначе
#    define _GET_KINFO_UID(kp) (kp.ki_ruid)
#    define _GET_KINFO_COMM(kp) (kp.ki_comm)
#    define _GET_KINFO_PID(kp) (kp.ki_pid)
#  endif
#  define _KVM_PATH _PATH_DEVNULL
#  define _KVM_FLAGS O_RDONLY
# endif

RC_PIDLIST *
rc_find_pids(const char *exec, const char *const *argv, uid_t uid, pid_t pid)
{
	static kvm_t *kd = NULL;
	char errbuf[_POSIX2_LINE_MAX];
	struct _KINFO_PROC *kp;
	цяло i;
	цяло processes = 0;
	цяло pargc = 0;
	char **pargv;
	RC_PIDLIST *pids = NULL;
	RC_PID *pi;
	pid_t p;
	const char *const *arg;
	цяло match;

	ако ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    NULL, _KVM_FLAGS, errbuf)) == NULL)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		върни NULL;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, sizeof(*kp), &processes);
#иначе
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	ако ((kp == NULL && processes > 0) || (kp != NULL && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		върни NULL;
	}

	ако (exec)
		exec = basename_c(exec);
	за (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		ако (pid != 0 && pid != p)
			continue;
		ако (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			continue;
		ако (exec) {
			ако (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				continue;
		}
		ако (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			ако (!pargv || !*pargv)
				continue;
			arg = argv;
			match = 1;
			докато (*arg && *pargv)
				ако (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					break;
				}
			ако (!match)
				continue;
		}
		ако (!pids) {
			pids = xmalloc(sizeof(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(sizeof(*pi));
		pi->pid = p;
		LIST_INSERT_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	върни pids;
}

#иначе
#  error "Platform not supported!"
#endif

static bool
_match_daemon(const char *svcname, const char *instance, RC_STRINGLIST *match)
{
	char *line = NULL;
	size_t len = 0;
	FILE *fp;
	RC_STRING *m;
	char *daemon;

	xasprintf(&daemon, "%s/%s", svcname, instance);
	fp = do_fopenat(rc_dirfd(RC_DIR_DAEMONS), daemon, O_RDONLY);
	free(daemon);

	ако (!fp)
		върни false;

	докато (xgetline(&line, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    ако (strcmp(line, m->value) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    break;
		    }
		ако (!TAILQ_FIRST(match))
			break;
	}
	fclose(fp);
	free(line);
	ако (TAILQ_FIRST(match))
		върни false;
	върни true;
}

static RC_STRINGLIST *
_match_list(const char *exec, const char *const *argv, const char *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	цяло i = 0;
	char *m;

	ако (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		free(m);
	}

	докато (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		free(m);
	}

	ако (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		free(m);
	}

	върни match;
}

bool
rc_service_daemon_set(const char *service, const char *exec,
    const char *const *argv,
    const char *pidfile, bool started)
{
	char *file = NULL;
	цяло nfiles = 0;
	char oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	struct dirent *d;
	RC_STRINGLIST *match, *renamelist;
	цяло i = 0;
	FILE *fp;
	const char *base = basename_c(service);

	ако (!exec && !pidfile) {
		errno = EINVAL;
		върни false;
	}

	/* Regardless, erase any existing daemon info */
	ако ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		докато ((d = readdir(dp))) {
			ако (d->d_name[0] == '.')
				continue;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			ако (rc_stringlist_find(renamelist, file)) {
				free(file);
				continue;
			}

			nfiles++;

			ако (!*oldfile) {
				ако (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, sizeof(oldfile));
					nfiles--;
				}
			} иначе {
				rename(file, oldfile);
				strlcpy(oldfile, file, sizeof(oldfile));
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
	ако (!started)
		върни true;

	ако (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		ако ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			ако (exec)
				fprintf(fp, "%s", exec);
			докато (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			ако (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		free(file);
	}

	върни retval;
}

bool
rc_service_started_daemon(const char *service,
    const char *exec, const char *const *argv, цяло indx)
{
	const char *base = basename_c(service);
	char *file = NULL;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	struct dirent *d;

	ако (!service || !exec)
		върни false;

	match = _match_list(exec, argv, NULL);

	ако (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		free(file);
	} иначе ако ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		докато ((d = readdir(dp))) {
			ако (d->d_name[0] == '.')
				continue;
			ако ((retval = _match_daemon(base, d->d_name, match)))
				break;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	върни retval;
}

bool
rc_service_daemons_crashed(const char *service)
{
	DIR *dp;
	struct dirent *d;
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	char **argv = NULL;
	char *exec = NULL;
	char *name = NULL;
	char *pidfile = NULL;
	pid_t pid = 0;
	RC_PIDLIST *pids;
	RC_PID *p1;
	RC_PID *p2;
	char *p;
	char *token;
	bool retval = false;
	RC_STRINGLIST *list = NULL;
	RC_STRING *s;
	size_t i;
	char *ch_root;
	char *spidfile;

	ако (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		върни false;

	докато ((d = readdir(dp))) {
		ако (d->d_name[0] == '.')
			continue;

		ако (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			break;

		докато (xgetline(&line, &len, fp) != -1) {
			p = line;
			ако ((token = strsep(&p, "=")) == NULL || !p)
				continue;

			ако (!*p)
				continue;

			ако (strcmp(token, "exec") == 0) {
				ако (exec)
					free(exec);
				exec = xstrdup(p);
			} иначе ако (strncmp(token, "argv_", 5) == 0) {
				ако (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} иначе ако (strcmp(token, "name") == 0) {
				ако (name)
					free(name);
				name = xstrdup(p);
			} иначе ако (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				break;
			}
		}
		fclose(fp);

		ch_root = rc_service_value_get(basename_c(service), "chroot");
		spidfile = pidfile;
		ако (ch_root && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_root, pidfile);
			free(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		ако (pidfile) {
			retval = true;
			ако ((fp = fopen(pidfile, "r"))) {
				ако (fscanf(fp, "%d", &pid) == 1)
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
		} иначе {
			ако (exec) {
				ако (!list)
					list = rc_stringlist_new();
				ако (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				free(exec);
				exec = NULL;
			}

			ако (list) {
				/* We need to flatten our linked list
				   into an array */
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    i++;
				argv = xmalloc(sizeof(char *) * (i + 1));
				i = 0;
				TAILQ_FOREACH(s, list, entries)
				    argv[i++] = s->value;
				argv[i] = NULL;
			}
		}

		ако (!retval) {
			ако (pid != 0) {
				ако (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} иначе ако ((pids = rc_find_pids(exec,
				    (const char *const *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				докато (p1) {
					p2 = LIST_NEXT(p1, entries);
					free(p1);
					p1 = p2;
				}
				free(pids);
			} иначе
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
		ако (retval)
			break;
	}
	closedir(dp);
	free(line);

	върни retval;
}
