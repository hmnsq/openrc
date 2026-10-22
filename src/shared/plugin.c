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

#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <dirent.h>
#અન્તર્ભાવ <dlfcn.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "rc_exec.h"
#અન્તર્ભાવ "plugin.h"
#અન્તર્ભાવ "helpers.h"

#define RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_in_plugin = false;

પ્રકારવ્યાખ્યા સંરચના plugin
{
	અક્ષર *name;
	શૂન્ય *handle;
	પૂર્ણાંક (*hook)(RC_HOOK, અચલ અક્ષર *);
	TAILQ_ENTRY(plugin) entries;
} PLUGIN;
TAILQ_HEAD(, plugin) plugins;

શૂન્ય
rc_plugin_load(શૂન્ય)
{
	DIR *dp;
	સંરચના dirent *d;
	PLUGIN *plugin;
	અક્ષર *file = ખાલી;
	શૂન્ય *h;
	સંઘ {
		શૂન્ય *sym;
		પૂર્ણાંક (*func)(RC_HOOK, અચલ અક્ષર *);
	} fptr;

	/* Don't load plugins if we're in one */
	જો (rc_in_plugin)
		પાછા;

	TAILQ_INIT(&plugins);

	જો (!(dp = opendir(RC_PLUGINDIR)))
		પાછા;

	જ્યારે ((d = readdir(dp))) {
		જો (d->d_name[0] == '.')
			ચાલુ;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		મુક્ત(file);
		જો (h == ખાલી) {
			eerror("dlopen: %s", dlerror());
			ચાલુ;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		જો (fptr.func == ખાલી) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} નહીંતર {
			plugin = xmalloc(કદ(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_ઉમેરો_TAIL(&plugins, plugin, entries);
		}
	}
	closedir(dp);
}

શૂન્ય
rc_plugin_run(RC_HOOK hook, અચલ અક્ષર *કિંમત)
{
	PLUGIN *plugin;
	સંરચના sigaction sa;
	sigset_t empty;
	sigset_t full;
	sigset_t old;
	પૂર્ણાંક pfd[2];
	pid_t pid;
	અક્ષર *buffer;
	અક્ષર *token;
	અક્ષર *p;
	ssize_t nr;
	પૂર્ણાંક retval;

	/* Don't run plugins if we're in one */
	જો (rc_in_plugin)
		પાછા;

	/* We need to block signals until we have forked */
	memset(&sa, 0, કદ(sa));
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
		જો (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", strerror(errno));
			પાછા;
		}

		sigprocmask(SIG_SETMASK, &full, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		જો ((pid = fork()) == -1) {
			eerror("fork: %s", strerror(errno));
			તોડો;
		}

		જો (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, ખાલી);
			sigaction(SIGHUP,  &sa, ખાલી);
			sigaction(SIGINT,  &sa, ખાલી);
			sigaction(SIGQUIT, &sa, ખાલી);
			sigaction(SIGTERM, &sa, ખાલી);
			sigaction(SIGUSR1, &sa, ખાલી);
			sigaction(SIGWINCH, &sa, ખાલી);
			sigprocmask(SIG_SETMASK, &old, ખાલી);

			rc_in_plugin = true;
			close(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, કિંમત);
			fclose(rc_environ_fd);
			rc_environ_fd = ખાલી;

			/* Just in case the plugin sets this to false */
			rc_in_plugin = true;
			exit(retval);
		}

		sigprocmask(SIG_SETMASK, &old, ખાલી);
		close(pfd[1]);
		buffer = xmalloc(કદ(અક્ષર) * BUFSIZ);
		memset(buffer, 0, BUFSIZ);

		જ્યારે ((nr = પઠ(pfd[0], buffer, BUFSIZ)) > 0) {
			p = buffer;
			જ્યારે (*p && p - buffer < nr) {
				token = strsep(&p, "=");
				જો (token) {
					unsetenv(token);
					જો (*p) {
						setenv(token, p, 1);
						p += strlen(p) + 1;
					} નહીંતર
						p++;
				}
			}
		}

		મુક્ત(buffer);
		close(pfd[0]);

		rc_waitpid(pid);
	}
}

શૂન્ય
rc_plugin_unload(શૂન્ય)
{
	PLUGIN *plugin = TAILQ_FIRST(&plugins);
	PLUGIN *next;

	જ્યારે (plugin) {
		next = TAILQ_NEXT(plugin, entries);
		dlclose(plugin->handle);
		મુક્ત(plugin->name);
		મુક્ત(plugin);
		plugin = next;
	}
	TAILQ_INIT(&plugins);
}
