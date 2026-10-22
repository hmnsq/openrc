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

#ವ್ಯಾಖ್ಯಾನಿಸು RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_ಒಳ_plugin = false;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ plugin
{
	ಅಕ್ಷರ *name;
	ಶೂನ್ಯ *handle;
	ಪೂರ್ಣಾಂಕ (*hook)(RC_HOOK, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
	TAILQ_ನಮೂದು(plugin) entries;
} PLUGIN;
TAILQ_ತಲೆ(, plugin) plugins;

ಶೂನ್ಯ
rc_plugin_ತುಂಬು(ಶೂನ್ಯ)
{
	DIR *dp;
	ರಚನೆ dirent *d;
	PLUGIN *plugin;
	ಅಕ್ಷರ *file = ಖಾಲಿ;
	ಶೂನ್ಯ *h;
	ಒಕ್ಕೂಟ {
		ಶೂನ್ಯ *sym;
		ಪೂರ್ಣಾಂಕ (*func)(RC_HOOK, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
	} fptr;

	/* Don't load plugins if we're in one */
	ಆದರೆ (rc_ಒಳ_plugin)
		ಹಿಂತಿರುಗಿ;

	TAILQ_INIT(&plugins);

	ಆದರೆ (!(dp = opendir(RC_PLUGINDIR)))
		ಹಿಂತಿರುಗಿ;

	ಆಗ ((d = readdir(dp))) {
		ಆದರೆ (d->d_name[0] == '.')
			ಮುಂದುವರಿ;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		ಬಿಡುಗಡೆ(file);
		ಆದರೆ (h == ಖಾಲಿ) {
			eerror("dlopen: %s", dlerror());
			ಮುಂದುವರಿ;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		ಆದರೆ (fptr.func == ಖಾಲಿ) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} ಇಲ್ಲದಿದ್ದರೆ {
			plugin = xmalloc(ಗಾತ್ರ(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_ಸೇರಿಸು_ಬಾಲ(&plugins, plugin, entries);
		}
	}
	closedir(dp);
}

ಶೂನ್ಯ
rc_plugin_ಸತತ(RC_HOOK hook, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	PLUGIN *plugin;
	ರಚನೆ sigaction sa;
	sigset_t ಖಾಲಿಗಳು;
	sigset_t ತುಂಬಿದ;
	sigset_t old;
	ಪೂರ್ಣಾಂಕ pfd[2];
	pid_t pid;
	ಅಕ್ಷರ *ಬಫರ್;
	ಅಕ್ಷರ *token;
	ಅಕ್ಷರ *p;
	ssize_t nr;
	ಪೂರ್ಣಾಂಕ retval;

	/* Don't run plugins if we're in one */
	ಆದರೆ (rc_ಒಳ_plugin)
		ಹಿಂತಿರುಗಿ;

	/* We need to block signals until we have forked */
	memset(&sa, 0, ಗಾತ್ರ(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigemptyset(&ಖಾಲಿಗಳು);
	sigfillset(&ತುಂಬಿದ);

	TAILQ_FOREACH(plugin, &plugins, entries) {
		/* We create a pipe so that plugins can affect our environment
		 * vars, which in turn influence our scripts.
		 * O_CLOEXEC to stop any scripts from inheriting us.
		 * This is actually quite important as without this, the splash
		 * plugin will probably hang when running in silent mode. */
		ಆದರೆ (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", strerror(errno));
			ಹಿಂತಿರುಗಿ;
		}

		sigprocmask(SIG_SETMASK, &ತುಂಬಿದ, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		ಆದರೆ ((pid = fork()) == -1) {
			eerror("fork: %s", strerror(errno));
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, ಖಾಲಿ);
			sigaction(SIGHUP,  &sa, ಖಾಲಿ);
			sigaction(SIGINT,  &sa, ಖಾಲಿ);
			sigaction(SIGQUIT, &sa, ಖಾಲಿ);
			sigaction(SIGTERM, &sa, ಖಾಲಿ);
			sigaction(SIGUSR1, &sa, ಖಾಲಿ);
			sigaction(SIGWINCH, &sa, ಖಾಲಿ);
			sigprocmask(SIG_SETMASK, &old, ಖಾಲಿ);

			rc_ಒಳ_plugin = true;
			close(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, ಮೌಲ್ಯ);
			fclose(rc_environ_fd);
			rc_environ_fd = ಖಾಲಿ;

			/* Just in case the plugin sets this to false */
			rc_ಒಳ_plugin = true;
			ನಿರ್ಗಮಿಸು(retval);
		}

		sigprocmask(SIG_SETMASK, &old, ಖಾಲಿ);
		close(pfd[1]);
		ಬಫರ್ = xmalloc(ಗಾತ್ರ(ಅಕ್ಷರ) * BUFSIZ);
		memset(ಬಫರ್, 0, BUFSIZ);

		ಆಗ ((nr = ಪಠ(pfd[0], ಬಫರ್, BUFSIZ)) > 0) {
			p = ಬಫರ್;
			ಆಗ (*p && p - ಬಫರ್ < nr) {
				token = strsep(&p, "=");
				ಆದರೆ (token) {
					unsetenv(token);
					ಆದರೆ (*p) {
						setenv(token, p, 1);
						p += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p) + 1;
					} ಇಲ್ಲದಿದ್ದರೆ
						p++;
				}
			}
		}

		ಬಿಡುಗಡೆ(ಬಫರ್);
		close(pfd[0]);

		rc_waitpid(pid);
	}
}

ಶೂನ್ಯ
rc_plugin_unload(ಶೂನ್ಯ)
{
	PLUGIN *plugin = TAILQ_ಮೊದಲ(&plugins);
	PLUGIN *ಮುಂದಿನ;

	ಆಗ (plugin) {
		ಮುಂದಿನ = TAILQ_ಮುಂದಿನ(plugin, entries);
		dlclose(plugin->handle);
		ಬಿಡುಗಡೆ(plugin->name);
		ಬಿಡುಗಡೆ(plugin);
		plugin = ಮುಂದಿನ;
	}
	TAILQ_INIT(&plugins);
}
