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

#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <dirent.h>
#அந்தர்பாவ <dlfcn.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "rc_exec.h"
#அந்தர்பாவ "plugin.h"
#அந்தர்பாவ "helpers.h"

#define RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_in_plugin = false;

வகைவரை கட்டமைப்பு plugin
{
	எழுத்து *name;
	வெறுமை *handle;
	முழு (*hook)(RC_HOOK, மாறிலி எழுத்து *);
	TAILQ_ENTRY(plugin) entries;
} PLUGIN;
TAILQ_HEAD(, plugin) plugins;

வெறுமை
rc_plugin_load(வெறுமை)
{
	DIR *dp;
	கட்டமைப்பு dirent *d;
	PLUGIN *plugin;
	எழுத்து *file = வெற்று;
	வெறுமை *h;
	ஒன்றிப்பு {
		வெறுமை *sym;
		முழு (*func)(RC_HOOK, மாறிலி எழுத்து *);
	} fptr;

	/* Don't load plugins if we're in one */
	எனில் (rc_in_plugin)
		திரும்பு;

	TAILQ_INIT(&plugins);

	எனில் (!(dp = opendir(RC_PLUGINDIR)))
		திரும்பு;

	போது ((d = readdir(dp))) {
		எனில் (d->d_name[0] == '.')
			தொடர்;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		விடுவி(file);
		எனில் (h == வெற்று) {
			eerror("dlopen: %s", dlerror());
			தொடர்;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		எனில் (fptr.func == வெற்று) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} இல்லையெனில் {
			plugin = xmalloc(அளவு(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_செருகு_TAIL(&plugins, plugin, entries);
		}
	}
	closedir(dp);
}

வெறுமை
rc_plugin_run(RC_HOOK hook, மாறிலி எழுத்து *மதிப்பு)
{
	PLUGIN *plugin;
	கட்டமைப்பு sigaction sa;
	sigset_t empty;
	sigset_t full;
	sigset_t old;
	முழு pfd[2];
	pid_t pid;
	எழுத்து *buffer;
	எழுத்து *token;
	எழுத்து *p;
	ssize_t nr;
	முழு retval;

	/* Don't run plugins if we're in one */
	எனில் (rc_in_plugin)
		திரும்பு;

	/* We need to block signals until we have forked */
	memset(&sa, 0, அளவு(sa));
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
		எனில் (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", strerror(errno));
			திரும்பு;
		}

		sigprocmask(SIG_SETMASK, &full, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		எனில் ((pid = fork()) == -1) {
			eerror("fork: %s", strerror(errno));
			நிறுத்து;
		}

		எனில் (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, வெற்று);
			sigaction(SIGHUP,  &sa, வெற்று);
			sigaction(SIGINT,  &sa, வெற்று);
			sigaction(SIGQUIT, &sa, வெற்று);
			sigaction(SIGTERM, &sa, வெற்று);
			sigaction(SIGUSR1, &sa, வெற்று);
			sigaction(SIGWINCH, &sa, வெற்று);
			sigprocmask(SIG_SETMASK, &old, வெற்று);

			rc_in_plugin = true;
			close(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, மதிப்பு);
			fclose(rc_environ_fd);
			rc_environ_fd = வெற்று;

			/* Just in case the plugin sets this to false */
			rc_in_plugin = true;
			exit(retval);
		}

		sigprocmask(SIG_SETMASK, &old, வெற்று);
		close(pfd[1]);
		buffer = xmalloc(அளவு(எழுத்து) * BUFSIZ);
		memset(buffer, 0, BUFSIZ);

		போது ((nr = பட(pfd[0], buffer, BUFSIZ)) > 0) {
			p = buffer;
			போது (*p && p - buffer < nr) {
				token = strsep(&p, "=");
				எனில் (token) {
					unsetenv(token);
					எனில் (*p) {
						setenv(token, p, 1);
						p += strlen(p) + 1;
					} இல்லையெனில்
						p++;
				}
			}
		}

		விடுவி(buffer);
		close(pfd[0]);

		rc_waitpid(pid);
	}
}

வெறுமை
rc_plugin_unload(வெறுமை)
{
	PLUGIN *plugin = TAILQ_FIRST(&plugins);
	PLUGIN *next;

	போது (plugin) {
		next = TAILQ_NEXT(plugin, entries);
		dlclose(plugin->handle);
		விடுவி(plugin->name);
		விடுவி(plugin);
		plugin = next;
	}
	TAILQ_INIT(&plugins);
}
