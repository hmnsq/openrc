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

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dirent.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dlfcn.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc_exec.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "plugin.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_ਵਿੱਚ_plugin = false;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ plugin
{
	ਅੱਖਰ *name;
	ਖਾਲੀ *handle;
	ਪੂਰਨ (*hook)(RC_HOOK, ਅਚਲ ਅੱਖਰ *);
	TAILQ_ਇੰਦਰਾਜ(plugin) entries;
} PLUGIN;
TAILQ_ਸਿਰ(, plugin) plugins;

ਖਾਲੀ
rc_plugin_ਲੱਦੋ(ਖਾਲੀ)
{
	DIR *dp;
	ਬਣਤਰ dirent *d;
	PLUGIN *plugin;
	ਅੱਖਰ *file = ਨਿਰਾ;
	ਖਾਲੀ *h;
	ਸੰਘ {
		ਖਾਲੀ *sym;
		ਪੂਰਨ (*func)(RC_HOOK, ਅਚਲ ਅੱਖਰ *);
	} fptr;

	/* Don't load plugins if we're in one */
	ਜੇ (rc_ਵਿੱਚ_plugin)
		ਵਾਪਸ;

	TAILQ_INIT(&plugins);

	ਜੇ (!(dp = opendir(RC_PLUGINDIR)))
		ਵਾਪਸ;

	ਜਦੋਂ ((d = readdir(dp))) {
		ਜੇ (d->d_name[0] == '.')
			ਜਾਰੀ;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		ਮੁਕਤ(file);
		ਜੇ (h == ਨਿਰਾ) {
			eerror("dlopen: %s", dlerror());
			ਜਾਰੀ;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		ਜੇ (fptr.func == ਨਿਰਾ) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} ਨਹੀਂਤਾਂ {
			plugin = xmalloc(ਆਕਾਰ(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_ਪਾਓ_ਪੂਛ(&plugins, plugin, entries);
		}
	}
	closedir(dp);
}

ਖਾਲੀ
rc_plugin_ਲੜੀਵਾਰ(RC_HOOK hook, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	PLUGIN *plugin;
	ਬਣਤਰ sigaction sa;
	sigset_t ਖਾਲੀ_ਥਾਵਾਂ;
	sigset_t ਭਰਿਆ;
	sigset_t old;
	ਪੂਰਨ pfd[2];
	pid_t pid;
	ਅੱਖਰ *ਬਫਰ;
	ਅੱਖਰ *token;
	ਅੱਖਰ *p;
	ssize_t nr;
	ਪੂਰਨ retval;

	/* Don't run plugins if we're in one */
	ਜੇ (rc_ਵਿੱਚ_plugin)
		ਵਾਪਸ;

	/* We need to block signals until we have forked */
	memset(&sa, 0, ਆਕਾਰ(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigemptyset(&ਖਾਲੀ_ਥਾਵਾਂ);
	sigfillset(&ਭਰਿਆ);

	TAILQ_FOREACH(plugin, &plugins, entries) {
		/* We create a pipe so that plugins can affect our environment
		 * vars, which in turn influence our scripts.
		 * O_CLOEXEC to stop any scripts from inheriting us.
		 * This is actually quite important as without this, the splash
		 * plugin will probably hang when running in silent mode. */
		ਜੇ (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", strerror(errno));
			ਵਾਪਸ;
		}

		sigprocmask(SIG_SETMASK, &ਭਰਿਆ, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		ਜੇ ((pid = fork()) == -1) {
			eerror("fork: %s", strerror(errno));
			ਰੋਕੋ;
		}

		ਜੇ (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, ਨਿਰਾ);
			sigaction(SIGHUP,  &sa, ਨਿਰਾ);
			sigaction(SIGINT,  &sa, ਨਿਰਾ);
			sigaction(SIGQUIT, &sa, ਨਿਰਾ);
			sigaction(SIGTERM, &sa, ਨਿਰਾ);
			sigaction(SIGUSR1, &sa, ਨਿਰਾ);
			sigaction(SIGWINCH, &sa, ਨਿਰਾ);
			sigprocmask(SIG_SETMASK, &old, ਨਿਰਾ);

			rc_ਵਿੱਚ_plugin = true;
			close(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, ਮੁੱਲ);
			fclose(rc_environ_fd);
			rc_environ_fd = ਨਿਰਾ;

			/* Just in case the plugin sets this to false */
			rc_ਵਿੱਚ_plugin = true;
			ਬਾਹਰ_ਨਿਕਲੋ(retval);
		}

		sigprocmask(SIG_SETMASK, &old, ਨਿਰਾ);
		close(pfd[1]);
		ਬਫਰ = xmalloc(ਆਕਾਰ(ਅੱਖਰ) * BUFSIZ);
		memset(ਬਫਰ, 0, BUFSIZ);

		ਜਦੋਂ ((nr = ਪਠ(pfd[0], ਬਫਰ, BUFSIZ)) > 0) {
			p = ਬਫਰ;
			ਜਦੋਂ (*p && p - ਬਫਰ < nr) {
				token = strsep(&p, "=");
				ਜੇ (token) {
					unsetenv(token);
					ਜੇ (*p) {
						setenv(token, p, 1);
						p += ਲੜੀ_ਲੰਬਾਈ(p) + 1;
					} ਨਹੀਂਤਾਂ
						p++;
				}
			}
		}

		ਮੁਕਤ(ਬਫਰ);
		close(pfd[0]);

		rc_waitpid(pid);
	}
}

ਖਾਲੀ
rc_plugin_unload(ਖਾਲੀ)
{
	PLUGIN *plugin = TAILQ_ਪਹਿਲਾ(&plugins);
	PLUGIN *ਅਗਲਾ;

	ਜਦੋਂ (plugin) {
		ਅਗਲਾ = TAILQ_ਅਗਲਾ(plugin, entries);
		dlclose(plugin->handle);
		ਮੁਕਤ(plugin->name);
		ਮੁਕਤ(plugin);
		plugin = ਅਗਲਾ;
	}
	TAILQ_INIT(&plugins);
}
