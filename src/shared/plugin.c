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

#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <dirent.h>
#അന്തര്ഭാവ <dlfcn.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "rc_exec.h"
#അന്തര്ഭാവ "plugin.h"
#അന്തര്ഭാവ "helpers.h"

#define RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_in_plugin = false;

തരനിർവചനം ഘടന plugin
{
	അക്ഷരം *name;
	ശൂന്യം *handle;
	പൂർണ്ണസംഖ്യ (*hook)(RC_HOOK, സ്ഥിരം അക്ഷരം *);
	TAILQ_ENTRY(plugin) entries;
} PLUGIN;
TAILQ_HEAD(, plugin) plugins;

ശൂന്യം
rc_plugin_load(ശൂന്യം)
{
	DIR *dp;
	ഘടന dirent *d;
	PLUGIN *plugin;
	അക്ഷരം *file = ശൂന്യ;
	ശൂന്യം *h;
	സംഘം {
		ശൂന്യം *sym;
		പൂർണ്ണസംഖ്യ (*func)(RC_HOOK, സ്ഥിരം അക്ഷരം *);
	} fptr;

	/* Don't load plugins if we're in one */
	എങ്കിൽ (rc_in_plugin)
		തിരികെ;

	TAILQ_INIT(&plugins);

	എങ്കിൽ (!(dp = opendir(RC_PLUGINDIR)))
		തിരികെ;

	ആകുമ്പോൾ ((d = readdir(dp))) {
		എങ്കിൽ (d->d_name[0] == '.')
			തുടരുക;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		സ്വതന്ത്രം(file);
		എങ്കിൽ (h == ശൂന്യ) {
			eerror("dlopen: %s", dlerror());
			തുടരുക;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		എങ്കിൽ (fptr.func == ശൂന്യ) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} അല്ലെങ്കിൽ {
			plugin = xmalloc(വലുപ്പം(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_തിരുകുക_TAIL(&plugins, plugin, entries);
		}
	}
	closedir(dp);
}

ശൂന്യം
rc_plugin_run(RC_HOOK hook, സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	PLUGIN *plugin;
	ഘടന sigaction sa;
	sigset_t empty;
	sigset_t full;
	sigset_t old;
	പൂർണ്ണസംഖ്യ pfd[2];
	pid_t pid;
	അക്ഷരം *buffer;
	അക്ഷരം *token;
	അക്ഷരം *p;
	ssize_t nr;
	പൂർണ്ണസംഖ്യ retval;

	/* Don't run plugins if we're in one */
	എങ്കിൽ (rc_in_plugin)
		തിരികെ;

	/* We need to block signals until we have forked */
	memset(&sa, 0, വലുപ്പം(sa));
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
		എങ്കിൽ (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", strerror(errno));
			തിരികെ;
		}

		sigprocmask(SIG_SETMASK, &full, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		എങ്കിൽ ((pid = fork()) == -1) {
			eerror("fork: %s", strerror(errno));
			നിർത്തുക;
		}

		എങ്കിൽ (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, ശൂന്യ);
			sigaction(SIGHUP,  &sa, ശൂന്യ);
			sigaction(SIGINT,  &sa, ശൂന്യ);
			sigaction(SIGQUIT, &sa, ശൂന്യ);
			sigaction(SIGTERM, &sa, ശൂന്യ);
			sigaction(SIGUSR1, &sa, ശൂന്യ);
			sigaction(SIGWINCH, &sa, ശൂന്യ);
			sigprocmask(SIG_SETMASK, &old, ശൂന്യ);

			rc_in_plugin = true;
			close(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, മൂല്യം);
			fclose(rc_environ_fd);
			rc_environ_fd = ശൂന്യ;

			/* Just in case the plugin sets this to false */
			rc_in_plugin = true;
			exit(retval);
		}

		sigprocmask(SIG_SETMASK, &old, ശൂന്യ);
		close(pfd[1]);
		buffer = xmalloc(വലുപ്പം(അക്ഷരം) * BUFSIZ);
		memset(buffer, 0, BUFSIZ);

		ആകുമ്പോൾ ((nr = പഠ(pfd[0], buffer, BUFSIZ)) > 0) {
			p = buffer;
			ആകുമ്പോൾ (*p && p - buffer < nr) {
				token = strsep(&p, "=");
				എങ്കിൽ (token) {
					unsetenv(token);
					എങ്കിൽ (*p) {
						setenv(token, p, 1);
						p += strlen(p) + 1;
					} അല്ലെങ്കിൽ
						p++;
				}
			}
		}

		സ്വതന്ത്രം(buffer);
		close(pfd[0]);

		rc_waitpid(pid);
	}
}

ശൂന്യം
rc_plugin_unload(ശൂന്യം)
{
	PLUGIN *plugin = TAILQ_FIRST(&plugins);
	PLUGIN *next;

	ആകുമ്പോൾ (plugin) {
		next = TAILQ_NEXT(plugin, entries);
		dlclose(plugin->handle);
		സ്വതന്ത്രം(plugin->name);
		സ്വതന്ത്രം(plugin);
		plugin = next;
	}
	TAILQ_INIT(&plugins);
}
