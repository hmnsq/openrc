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

#если defined(__linux__) || (defined (__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
static bool
pid_is_exec(pid_t pid, const char *exec)
{
	char *buffer = NULL;
	FILE *fp;
	целое c;
	bool retval = false;

	exec = basename_c(exec);
	xasprintf(&buffer, "/proc/%d/stat", pid);
	если ((fp = fopen(buffer, "r"))) {
		пока ((c = getc(fp)) != EOF && c != '(')
			;
		если (c == '(') {
			пока ((c = getc(fp)) != EOF && c == *exec)
				exec++;
			если (c == ')' && *exec == '\0')
				retval = true;
		}
		fclose(fp);
	}
	free(buffer);
	вернуть retval;
}

static bool
pid_is_argv(pid_t pid, const char *const *argv)
{
	char *cmdline = NULL;
	целое fd;
	char buffer[PATH_MAX];
	char *p;
	ssize_t bytes;

	xasprintf(&cmdline, "/proc/%u/cmdline", pid);
	если ((fd = open(cmdline, O_RDONLY)) < 0) {
		free(cmdline);
		вернуть false;
	}
	bytes = read(fd, buffer, sizeof(buffer) - 1);
	close(fd);
	free(cmdline);
	если (bytes == -1)
		вернуть false;

	buffer[bytes] = '\0';
	p = buffer;
	пока (*argv) {
		если ((unsigned)(p - buffer) >= sizeof(buffer))
			вернуть false;
		если (strcmp(*argv, p) != 0)
			вернуть false;
		argv++;
		p += strlen(p) + 1;
	}
	вернуть true;
}

RC_PIDLIST *
rc_find_pids(const char *exec, const char *const *argv, uid_t uid, pid_t pid)
{
	DIR *procdir;
	struct dirent *entry;
	FILE *fp;
	целое rc;
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

	если ((procdir = opendir("/proc")) == NULL)
		вернуть NULL;

	/*
	  We never match RC_OPENRC_PID if present so we avoid the below
	  scenario

	  /etc/init.d/ntpd stop does
	  start-stop-daemon --stop --name ntpd
	  catching /etc/init.d/ntpd stop

	  nasty
	*/

	если ((pp = getenv("RC_OPENRC_PID"))) {
		если (sscanf(pp, "%d", &openrc_pid) != 1)
			openrc_pid = 0;
	}

	/*
	If /proc/self/status contains EnvID: 0, then we are an OpenVZ host,
	and we will need to filter out processes that are inside containers
	from our list of pids.
	*/

	если ((fp = fopen("/proc/self/status", "r"))) {
		пока (xgetline(&line, &len, fp) != -1) {
			если (strncmp(line, "envID:\t0", 8) == 0) {
				openvz_host = true;
				break;
			}
		}
		fclose(fp);
	}

	если ((rc = readlink("/proc/self/ns/pid", my_ns, sizeof(my_ns)-1)) <= 0)
		my_ns[0] = '\0';

	пока ((entry = readdir(procdir)) != NULL) {
		если (sscanf(entry->d_name, "%d", &p) != 1)
			continue;
		если (openrc_pid != 0 && openrc_pid == p)
			continue;
		если (pid != 0 && pid != p)
			continue;
		xasprintf(&buffer, "/proc/%d/ns/pid", p);
		если ((rc = readlink(buffer, proc_ns, sizeof(proc_ns)-1)) <= 0)
			proc_ns[0] = '\0';
		free(buffer);
		если (pid == 0 && strlen(my_ns) && strlen (proc_ns) && strcmp(my_ns, proc_ns))
			continue;
		если (uid) {
			xasprintf(&buffer, "/proc/%d", p);
			если (stat(buffer, &sb) != 0 || sb.st_uid != uid) {
				free(buffer);
				continue;
			}
			free(buffer);
		}
		если (exec && !pid_is_exec(p, exec))
			continue;
		если (argv &&
		    !pid_is_argv(p, (const char *const *)argv))
			continue;
		/* If this is an OpenVZ host, filter out container processes */
		если (openvz_host) {
			xasprintf(&buffer, "/proc/%d/status", p);
			fp = fopen(buffer, "r");
			free(buffer);
			если (!fp)
				continue;
			пока (xgetline(&line, &len, fp) != -1) {
				если (strncmp(line, "envID:", 6) == 0) {
					container_pid = !(strncmp(line, "envID:\t0", 8) == 0);
					break;
				}
			}
			fclose(fp);
		}
		если (container_pid)
			continue;
		если (!pids) {
			pids = xmalloc(sizeof(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(sizeof(*pi));
		pi->pid = p;
		LIST_INSERT_HEAD(pids, pi, entries);
	}
	если (line != NULL)
		free(line);
	closedir(procdir);
	вернуть pids;
}

#elif BSD

# если defined(__NetBSD__) || defined(__OpenBSD__)
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
#  если defined(__DragonFly__)
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
	целое i;
	целое processes = 0;
	целое pargc = 0;
	char **pargv;
	RC_PIDLIST *pids = NULL;
	RC_PID *pi;
	pid_t p;
	const char *const *arg;
	целое match;

	если ((kd = kvm_openfiles(_KVM_PATH, _KVM_PATH,
		    NULL, _KVM_FLAGS, errbuf)) == NULL)
	{
		fprintf(stderr, "kvm_open: %s\n", errbuf);
		вернуть NULL;
	}

#ifdef _KVM_GETPROC2
	kp = kvm_getproc2(kd, KERN_PROC_ALL, 0, sizeof(*kp), &processes);
#иначе
	kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &processes);
#endif
	если ((kp == NULL && processes > 0) || (kp != NULL && processes < 0)) {
		fprintf(stderr, "kvm_getprocs: %s\n", kvm_geterr(kd));
		kvm_close(kd);
		вернуть NULL;
	}

	если (exec)
		exec = basename_c(exec);
	для (i = 0; i < processes; i++) {
		p = _GET_KINFO_PID(kp[i]);
		если (pid != 0 && pid != p)
			continue;
		если (uid != 0 && uid != _GET_KINFO_UID(kp[i]))
			continue;
		если (exec) {
			если (!_GET_KINFO_COMM(kp[i]) ||
			    strcmp(exec, _GET_KINFO_COMM(kp[i])) != 0)
				continue;
		}
		если (argv && *argv) {
			pargv = _KVM_GETARGV(kd, &kp[i], pargc);
			если (!pargv || !*pargv)
				continue;
			arg = argv;
			match = 1;
			пока (*arg && *pargv)
				если (strcmp(*arg++, *pargv++) != 0) {
					match = 0;
					break;
				}
			если (!match)
				continue;
		}
		если (!pids) {
			pids = xmalloc(sizeof(*pids));
			LIST_INIT(pids);
		}
		pi = xmalloc(sizeof(*pi));
		pi->pid = p;
		LIST_INSERT_HEAD(pids, pi, entries);
	}
	kvm_close(kd);

	вернуть pids;
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

	если (!fp)
		вернуть false;

	пока (xgetline(&line, &len, fp) != -1) {
		TAILQ_FOREACH(m, match, entries)
		    если (strcmp(line, m->value) == 0) {
			    TAILQ_REMOVE(match, m, entries);
			    break;
		    }
		если (!TAILQ_FIRST(match))
			break;
	}
	fclose(fp);
	free(line);
	если (TAILQ_FIRST(match))
		вернуть false;
	вернуть true;
}

static RC_STRINGLIST *
_match_list(const char *exec, const char *const *argv, const char *pidfile)
{
	RC_STRINGLIST *match = rc_stringlist_new();
	целое i = 0;
	char *m;

	если (exec) {
		xasprintf(&m, "exec=%s", exec);
		rc_stringlist_add(match, m);
		free(m);
	}

	пока (argv && argv[i]) {
		xasprintf(&m, "argv_0=%s", argv[i++]);
		rc_stringlist_add(match, m);
		free(m);
	}

	если (pidfile) {
		xasprintf(&m, "pidfile=%s", pidfile);
		rc_stringlist_add(match, m);
		free(m);
	}

	вернуть match;
}

bool
rc_service_daemon_set(const char *service, const char *exec,
    const char *const *argv,
    const char *pidfile, bool started)
{
	char *file = NULL;
	целое nfiles = 0;
	char oldfile[PATH_MAX] = { '\0' };
	bool retval = false;
	DIR *dp;
	struct dirent *d;
	RC_STRINGLIST *match, *renamelist;
	целое i = 0;
	FILE *fp;
	const char *base = basename_c(service);

	если (!exec && !pidfile) {
		errno = EINVAL;
		вернуть false;
	}

	/* Regardless, erase any existing daemon info */
	если ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		match = _match_list(exec, argv, pidfile);
		renamelist = rc_stringlist_new();
		пока ((d = readdir(dp))) {
			если (d->d_name[0] == '.')
				continue;

			xasprintf(&file, "%s/daemons/%s/%s", rc_svcdir(), base, d->d_name);
			если (rc_stringlist_find(renamelist, file)) {
				free(file);
				continue;
			}

			nfiles++;

			если (!*oldfile) {
				если (_match_daemon(base, d->d_name, match)) {
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
	если (!started)
		вернуть true;

	если (mkdirat(rc_dirfd(RC_DIR_DAEMONS), base, 0755) == 0 || errno == EEXIST) {
		xasprintf(&file, "%s/daemons/%s/%03d", rc_svcdir(), base, nfiles + 1);
		если ((fp = fopen(file, "w"))) {
			fprintf(fp, "exec=");
			если (exec)
				fprintf(fp, "%s", exec);
			пока (argv && argv[i]) {
				fprintf(fp, "\nargv_%d=%s", i, argv[i]);
				i++;
			}
			fprintf(fp, "\npidfile=");
			если (pidfile)
				fprintf(fp, "%s", pidfile);
			fprintf(fp, "\n");
			fclose(fp);
			retval = true;
		}
		free(file);
	}

	вернуть retval;
}

bool
rc_service_started_daemon(const char *service,
    const char *exec, const char *const *argv, целое indx)
{
	const char *base = basename_c(service);
	char *file = NULL;
	RC_STRINGLIST *match;
	bool retval = false;
	DIR *dp;
	struct dirent *d;

	если (!service || !exec)
		вернуть false;

	match = _match_list(exec, argv, NULL);

	если (indx > 0) {
		xasprintf(&file, "%03d", indx);
		retval = _match_daemon(base, file, match);
		free(file);
	} иначе если ((dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), base))) {
		пока ((d = readdir(dp))) {
			если (d->d_name[0] == '.')
				continue;
			если ((retval = _match_daemon(base, d->d_name, match)))
				break;
		}
		closedir(dp);
	}

	rc_stringlist_free(match);
	вернуть retval;
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

	если (!(dp = do_opendirat(rc_dirfd(RC_DIR_DAEMONS), basename_c(service))))
		вернуть false;

	пока ((d = readdir(dp))) {
		если (d->d_name[0] == '.')
			continue;

		если (!(fp = do_fopenat(dirfd(dp), d->d_name, O_RDONLY)))
			break;

		пока (xgetline(&line, &len, fp) != -1) {
			p = line;
			если ((token = strsep(&p, "=")) == NULL || !p)
				continue;

			если (!*p)
				continue;

			если (strcmp(token, "exec") == 0) {
				если (exec)
					free(exec);
				exec = xstrdup(p);
			} иначе если (strncmp(token, "argv_", 5) == 0) {
				если (!list)
					list = rc_stringlist_new();
				rc_stringlist_add(list, p);
			} иначе если (strcmp(token, "name") == 0) {
				если (name)
					free(name);
				name = xstrdup(p);
			} иначе если (strcmp(token, "pidfile") == 0) {
				pidfile = xstrdup(p);
				break;
			}
		}
		fclose(fp);

		ch_root = rc_service_value_get(basename_c(service), "chroot");
		spidfile = pidfile;
		если (ch_root && pidfile) {
			xasprintf(&spidfile, "%s%s", ch_root, pidfile);
			free(pidfile);
			pidfile = spidfile;
		}

		pid = 0;
		если (pidfile) {
			retval = true;
			если ((fp = fopen(pidfile, "r"))) {
				если (fscanf(fp, "%d", &pid) == 1)
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
			если (exec) {
				если (!list)
					list = rc_stringlist_new();
				если (!TAILQ_FIRST(list))
					rc_stringlist_add(list, exec);

				free(exec);
				exec = NULL;
			}

			если (list) {
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

		если (!retval) {
			если (pid != 0) {
				если (kill(pid, 0) == -1 && errno == ESRCH)
					retval = true;
			} иначе если ((pids = rc_find_pids(exec,
				    (const char *const *)argv,
				    0, pid)))
			{
				p1 = LIST_FIRST(pids);
				пока (p1) {
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
		если (retval)
			break;
	}
	closedir(dp);
	free(line);

	вернуть retval;
}
