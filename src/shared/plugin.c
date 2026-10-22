/*
 * plugin.c
 * Simple plugin handler
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

#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <dirent.h>
#ಅನ್ತರ್ಭಾವ <dlfcn.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "rc_exec.h"
#ಅನ್ತರ್ಭಾವ "plugin.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

#define RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_in_plugin = false;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ plugin
{
	ವರ್ಣ *name;
	ಶೂನ್ಯ *handle;
	ಪೂರ್ಣಾಙ್ಕ (*hook)(RC_HOOK, ಧ್ರುವ ವರ್ಣ *);
	TAILQ_ENTRY(plugin) entries;
} PLUGIN;
TAILQ_HEAD(, plugin) plugins;

ಶೂನ್ಯ
rc_plugin_load(ಶೂನ್ಯ)
{
	DIR *dp;
	ಸಂರಚನಾ dirent *d;
	PLUGIN *plugin;
	ವರ್ಣ *file = ರಿಕ್ತ;
	ಶೂನ್ಯ *h;
	ಸಙ್ಘ {
		ಶೂನ್ಯ *sym;
		ಪೂರ್ಣಾಙ್ಕ (*func)(RC_HOOK, ಧ್ರುವ ವರ್ಣ *);
	} fptr;

	/* Don't load plugins if we're in one */
	ಯದಿ (rc_in_plugin)
		ಪ್ರತ್ಯಾಗಮ;

	TAILQ_INIT(&plugins);

	ಯದಿ (!(dp = opendir(RC_PLUGINDIR)))
		ಪ್ರತ್ಯಾಗಮ;

	ಯಾವತ್ ((d = readdir(dp))) {
		ಯದಿ (d->d_name[0] == '.')
			ಅನುವರ್ತ;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		ಮುಕ್ತ(file);
		ಯದಿ (h == ರಿಕ್ತ) {
			eerror("dlopen: %s", dlerror());
			ಅನುವರ್ತ;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		ಯದಿ (fptr.func == ರಿಕ್ತ) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} ಅನ್ಯಥಾ {
			plugin = xmalloc(ಪರಿಮಾಣ(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_ಸೇರಿಸು_TAIL(&plugins, plugin, entries);
		}
	}
	closedir(dp);
}

ಶೂನ್ಯ
rc_plugin_run(RC_HOOK hook, ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	PLUGIN *plugin;
	ಸಂರಚನಾ sigaction sa;
	sigset_t empty;
	sigset_t full;
	sigset_t old;
	ಪೂರ್ಣಾಙ್ಕ pfd[2];
	pid_t pid;
	ವರ್ಣ *buffer;
	ವರ್ಣ *token;
	ವರ್ಣ *p;
	ssize_t nr;
	ಪೂರ್ಣಾಙ್ಕ retval;

	/* Don't run plugins if we're in one */
	ಯದಿ (rc_in_plugin)
		ಪ್ರತ್ಯಾಗಮ;

	/* We need to block signals until we have forked */
	memset(&sa, 0, ಪರಿಮಾಣ(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigemptyset(&empty);
	sigfillset(&full);

	TAILQ_FOREACH(plugin, &plugins, entries) {
		/* We create a pipe so that plugins can affect our environment
		 * vars, which in turn influence our scripts.
		 * O_CLOEXEC to stop any scripts from inheriting us.
		 * This is actually quite important as without this, the splash
		 * plugin will probably hang when running in silent mode. */
		ಯದಿ (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", strerror(errno));
			ಪ್ರತ್ಯಾಗಮ;
		}

		sigprocmask(SIG_SETMASK, &full, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		ಯದಿ ((pid = fork()) == -1) {
			eerror("fork: %s", strerror(errno));
			ಭಙ್ಗ;
		}

		ಯದಿ (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, ರಿಕ್ತ);
			sigaction(SIGHUP,  &sa, ರಿಕ್ತ);
			sigaction(SIGINT,  &sa, ರಿಕ್ತ);
			sigaction(SIGQUIT, &sa, ರಿಕ್ತ);
			sigaction(SIGTERM, &sa, ರಿಕ್ತ);
			sigaction(SIGUSR1, &sa, ರಿಕ್ತ);
			sigaction(SIGWINCH, &sa, ರಿಕ್ತ);
			sigprocmask(SIG_SETMASK, &old, ರಿಕ್ತ);

			rc_in_plugin = true;
			close(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, ಮೌಲ್ಯ);
			fclose(rc_environ_fd);
			rc_environ_fd = ರಿಕ್ತ;

			/* Just in case the plugin sets this to false */
			rc_in_plugin = true;
			exit(retval);
		}

		sigprocmask(SIG_SETMASK, &old, ರಿಕ್ತ);
		close(pfd[1]);
		buffer = xmalloc(ಪರಿಮಾಣ(ವರ್ಣ) * BUFSIZ);
		memset(buffer, 0, BUFSIZ);

		ಯಾವತ್ ((nr = ಪಠ(pfd[0], buffer, BUFSIZ)) > 0) {
			p = buffer;
			ಯಾವತ್ (*p && p - buffer < nr) {
				token = strsep(&p, "=");
				ಯದಿ (token) {
					unsetenv(token);
					ಯದಿ (*p) {
						setenv(token, p, 1);
						p += strlen(p) + 1;
					} ಅನ್ಯಥಾ
						p++;
				}
			}
		}

		ಮುಕ್ತ(buffer);
		close(pfd[0]);

		rc_waitpid(pid);
	}
}

ಶೂನ್ಯ
rc_plugin_unload(ಶೂನ್ಯ)
{
	PLUGIN *plugin = TAILQ_FIRST(&plugins);
	PLUGIN *next;

	ಯಾವತ್ (plugin) {
		next = TAILQ_NEXT(plugin, entries);
		dlclose(plugin->handle);
		ಮುಕ್ತ(plugin->name);
		ಮುಕ್ತ(plugin);
		plugin = next;
	}
	TAILQ_INIT(&plugins);
}
