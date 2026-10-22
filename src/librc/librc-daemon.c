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

#만약 defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
static bool
pid_is_exec(pid_t pid, const char *exec)
{
	char *buffer = NULL;
	FILE *fp;
	정수 c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	만약 ((fp = fopen(buffer, "r"))) {
		동안 ((c = getc(fp)) != EOF && c != '(')
			;
		만약 (c == '(') {
			동안 ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			만약 (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	free(buffer);
	반환 retval;
}

static bool
pid_is_argv(pid_t pid, const char *const *argv)
{
	char *cmdline = NULL;
	정수 fd;
	char buffer[PATH_MAX];
	char *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	만약 ((fd = open(cmdline, O_RDONLY)) < 0) {
		free(cmdline);
		반환 false;
	}
	bytes = read(fd, buffer, sizeof(buffer) - 1);
	close(fd);
	free(cmdline);
	만약 (bytes == -1)
		반환 false;

	buffer[bytes] = '\0';
	p = buffer;
	동안 (*argv) {
		만약 ((unsigned)(p - buffer) >= sizeof(buffer))
			반환 false;
		만약 (strcmp(*argv, p) != 0)
			반환 false;
		argv++;
		p += strlen(p) + 1;
	}
	반환 true;
}

RC_PIDLIST *
rc_find_pids(const char *exec, const char *const *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	struct dirent *entry;
	FILE *fp;
	정수 rc;
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

	만약 ((procdir = opendir("/proc")) == NULL)
		반환 NULL;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	만약 ((pp = getenv("RC_OPENRC_PID"))) {
		만약 (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	만약 ((fp = fopen("/proc/self/status", "r"))) {
		동안 (xgetline(&line, &len, fp) != -1) {
			만약 (strncmp(line, "envID:\t0", 8) == 0) {
				openvz_host = true;
				break;
			}
		}
		fclose(fp);
	}

	만약 ((rc = readlink("/proc/self/ns/pid", my_ns, sizeof(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	동안 ((entry = readdir(procdir)) != NULL) {
		만약 (sscanf(entry->d_name, "%d", &p) != 1)
			continue;
		만약 (openrc_pid != 0 && openrc_pid == p)
			continue;
		만약 (pid != 0 && pid != p)
			continue;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		만약 ((rc = readlink(buffer, proc_ns, sizeof(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		free(buffer);
		만약 (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			continue;
		만약 (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			만약 (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				free(buffer);
				continue;
			}
			free(buffer);
		}
		만약 (exec && !pid_is_exec(p, exec))
			continue;
		만약 (argv &&
		    !pid_is_argv(p, (const char *const *)argv))
			continue;
		/* If this is an OpenVZ host, filter out container processes */
		만약 (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			free(buffer);
			만약 (!fp)
				continue;
			동안 (xgetline(&line, &len, fp) != -1) {
				만약 (strncmp(line, "envID:", 6) == 0) {
					container_pid = !(strncmp(line, "envID:\t0", 8) == 0);
					break;
				}
			}
			fclose(fp);
		}
		만약 (container_pid)
			continue;
		만약 (!pids) {
			pids = xmalloc(sizeof(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(sizeof(*pi));
		pi->pid = p;
		LIST_INSERT_HEAD(pids, pi, entries);
	}
	만약 (line != NULL)
		free(line);
	closedir(procdir);
	반환 pids;
}

#elif BSD

# 만약 defined(__NetBSD__) || defined(__OpenBSD__)
#  define _KVM_GETPROC2
#  define _KINFO_PROC kinfo_proc2
#  define _KVM_GETARGV kvm_getargv2
#  define _GET_KINFO_UID(kp) (kp.p_ruid)
#  define _GET_KINFO_COMM(kp) (kp.p_comm)
#  define _GET_KINFO_PID(kp) (kp.p_pid)
#  define _KVM_PATH NULL
#  define _KVM_FLAGS KVM_NO_FILES
# 아니면
#  ifndef KERN_PROC_PROC
#    define KERN_PROC_PROC KERN_PROC_ALL
#  endif
#  define _KINFO_PROC kinfo_proc
#  define _KVM_GETARGV kvm_getargv
#  만약 defined(__DragonFly__)
#    define _GET_KINFO_UID(kp) (kp.kp_ruid)
#    define _GET_KINFO_COMM(kp) (kp.kp_comm)
#    define _GET_KINFO_PID(kp) (kp.kp_pid)
#  아니면
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
	정수 i;
	정수 processes = 0;
	정수 pargc = 0;
	char **pargv;
	RC_PIDLIST *pids = NULL;
	RC_PID *pi;
	pid_t p;
	const char *const *arg;
	정수 match;

	만약 ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    NULL, _KVM_FLAGS, errbuf)) == NULL)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		반환 NULL;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, sizeof(*kp), &processes);
#아니면
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	만약 ((kp == NULL && processes > 0) || (kp != NULL && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		반환 NULL;
	}

	만약 (exec)
		exec = basename_c(exec);
	위해 (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		만약 (pid != 0 && pid != p)
			continue;
		만약 (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			continue;
		만약 (exec) {
			만약 (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				continue;
		}
		만약 (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			만약 (!pargv || !*pargv)
				continue;
			arg = argv;
			match = 1;
			동안 (*arg && *pargv)
				만약 (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					break;
				}
			만약 (!match)
				continue;
		}
		만약 (!pids) {
			pids = xmalloc(sizeof(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(sizeof(*pi));
		pi->pid = p;
		LIST_INSERT_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	반환 pids;
}

#아니면
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

	만약 (!fp)
		반환 false;

	동안 (xgetline(&line, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    만약 (strcmp(line, m->value) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    break;
		    }
		만약 (!TAILQ_FIRST(match))
			break;
	}
	fclose(fp);
	free(line);
	만약 (TAILQ_FIRST(match))
		반환 false;
	반환 true;
}

static RC_STRINGLIST *
_match_list(const char *exec, const char *const *argv, const char *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	정수 i = 0;
	char *m;

	만약 (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		free(m);
	}

	동안 (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		free(m);
	}

	만약 (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		free(m);
	}

	반환 match;
}

bool
rc_service_daemon_set(const char *service, const char *exec,
    const char *const *argv,
    const char *pidfile, bool started)
{
	char *file = NULL;
	정수 nfiles = 0;
	char oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	struct dirent *d;
	RC_STRINGLIST *match, *renamelist;
	정수 i = 0;
	FILE *fp;
	const char *base = basename_c(service);

	만약 (!exec && !pidfile) {
		errno = EINVAL;
		반환 false;
	}

	/* Regardless, erase any existing daemon info */
	만약 ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		동안 ((d = readdir(dp))) {
			만약 (d->d_name[0] == '.')
				continue;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			만약 (rc_stringlist_find(renamelist, file)) {
				free(file);
				continue;
			}

			nfiles++;

			만약 (!*oldfile) {
				만약 (_match_daemon(base, d->d_name, match)) {
					unlink(file);
					strlcpy(oldfile, file, sizeof(oldfile));
					nfiles--;
				}
			} 아니면 {
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
	만약 (!started)
		반환 true;

	만약 (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		만약 ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			만약 (exec)
				fprintf(fp, "%s", exec);
			동안 (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			만약 (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		free(file);
	}

	반환 retval;
}

bool
rc_service_started_daemon(const char *service,
    const char *exec, const char *const *argv, 정수 indx)
{
	const char *base = basename_c(service);
	char *file = NULL;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	struct dirent *d;

	만약 (!service || !exec)
		반환 false;

	match = _match_list(exec, argv, NULL);

	만약 (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		free(file);
	} 아니면 만약 ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		동안 ((d = readdir(dp))) {
			만약 (d->d_name[0] == '.')
				continue;
			만약 ((retval = _match_daemon(base, d->d_name, match)))
				break;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	반환 retval;
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

	만약 (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		반환 false;

	동안 ((d = readdir(dp))) {
		만약 (d->d_name[0] == '.')
			continue;

		만약 (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			break;

		동안 (xgetline(&line, &len, fp) != -1) {
			p = line;
			만약 ((token = strsep(&p, "=")) == NULL || !p)
				continue;

			만약 (!*p)
				continue;

			만약 (strcmp(token, "exec") == 0) {
				만약 (exec)
					free(exec);
				exec = xstrdup(p);
			} 아니면 만약 (strncmp(token, "argv_", 5) == 0) {
				만약 (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} 아니면 만약 (strcmp(token, "name") == 0) {
				만약 (name)
					free(name);
				name = xstrdup(p);
			} 아니면 만약 (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				break;
			}
		}
		fclose(fp);

		ch_root = rc_service_value_get(basename_c(service), "chroot");
		spidfile = pidfile;
		만약 (ch_root && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_root, pidfile);
			free(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		만약 (pidfile) {
			retval = true;
			만약 ((fp = fopen(pidfile, "r"))) {
				만약 (fscanf(fp, "%d", &pid) == 1)
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
		} 아니면 {
			만약 (exec) {
				만약 (!list)
					list = rc_stringlist_new();
				만약 (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				free(exec);
				exec = NULL;
			}

			만약 (list) {
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

		만약 (!retval) {
			만약 (pid != 0) {
				만약 (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} 아니면 만약 ((pids = rc_find_pids(exec,
				    (const char *const *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				동안 (p1) {
					p2 = LIST_NEXT(p1, entries);
					free(p1);
					p1 = p2;
				}
				free(pids);
			} 아니면
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
		만약 (retval)
			break;
	}
	closedir(dp);
	free(line);

	반환 retval;
}
