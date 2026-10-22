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

#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/wait.h>
#অন্তর্ভাৱ <dirent.h>
#অন্তর্ভাৱ <dlfcn.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <signal.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <unistd.h>

#অন্তর্ভাৱ "einfo.h"
#অন্তর্ভাৱ "queue.h"
#অন্তর্ভাৱ "rc.h"
#অন্তর্ভাৱ "rc_exec.h"
#অন্তর্ভাৱ "plugin.h"
#অন্তর্ভাৱ "helpers.h"

#define RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_in_plugin = false;

প্রকারনির্দেশ সংরচনা plugin
{
	ৱর্ণ *name;
	শূন্য *handle;
	পূর্ণাঙ্ক (*hook)(RC_HOOK, ধ্রুৱ ৱর্ণ *);
	TAILQ_ENTRY(plugin) entries;
} PLUGIN;
TAILQ_HEAD(, plugin) plugins;

শূন্য
rc_plugin_load(শূন্য)
{
	DIR *dp;
	সংরচনা dirent *d;
	PLUGIN *plugin;
	ৱর্ণ *file = রিক্ত;
	শূন্য *h;
	সঙ্ঘ {
		শূন্য *sym;
		পূর্ণাঙ্ক (*func)(RC_HOOK, ধ্রুৱ ৱর্ণ *);
	} fptr;

	/* Don't load plugins if we're in one */
	যদি (rc_in_plugin)
		ঘূৰাই;

	TAILQ_INIT(&plugins);

	যদি (!(dp = opendir(RC_PLUGINDIR)))
		ঘূৰাই;

	যেতিয়ালৈ ((d = readdir(dp))) {
		যদি (d->d_name[0] == '.')
			অনুৱর্ত;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		মুক্ত(file);
		যদি (h == রিক্ত) {
			eerror("dlopen: %s", dlerror());
			অনুৱর্ত;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		যদি (fptr.func == রিক্ত) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} অন্যথা {
			plugin = xmalloc(পরিমাণ(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_ঢোকাও_TAIL(&plugins, plugin, entries);
		}
	}
	closedir(dp);
}

শূন্য
rc_plugin_run(RC_HOOK hook, ধ্রুৱ ৱর্ণ *মান)
{
	PLUGIN *plugin;
	সংরচনা sigaction sa;
	sigset_t empty;
	sigset_t full;
	sigset_t old;
	পূর্ণাঙ্ক pfd[2];
	pid_t pid;
	ৱর্ণ *buffer;
	ৱর্ণ *token;
	ৱর্ণ *p;
	ssize_t nr;
	পূর্ণাঙ্ক retval;

	/* Don't run plugins if we're in one */
	যদি (rc_in_plugin)
		ঘূৰাই;

	/* We need to block signals until we have forked */
	memset(&sa, 0, পরিমাণ(sa));
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
		যদি (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", strerror(errno));
			ঘূৰাই;
		}

		sigprocmask(SIG_SETMASK, &full, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		যদি ((pid = fork()) == -1) {
			eerror("fork: %s", strerror(errno));
			ভঙ্গ;
		}

		যদি (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, রিক্ত);
			sigaction(SIGHUP,  &sa, রিক্ত);
			sigaction(SIGINT,  &sa, রিক্ত);
			sigaction(SIGQUIT, &sa, রিক্ত);
			sigaction(SIGTERM, &sa, রিক্ত);
			sigaction(SIGUSR1, &sa, রিক্ত);
			sigaction(SIGWINCH, &sa, রিক্ত);
			sigprocmask(SIG_SETMASK, &old, রিক্ত);

			rc_in_plugin = true;
			close(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, মান);
			fclose(rc_environ_fd);
			rc_environ_fd = রিক্ত;

			/* Just in case the plugin sets this to false */
			rc_in_plugin = true;
			exit(retval);
		}

		sigprocmask(SIG_SETMASK, &old, রিক্ত);
		close(pfd[1]);
		buffer = xmalloc(পরিমাণ(ৱর্ণ) * BUFSIZ);
		memset(buffer, 0, BUFSIZ);

		যেতিয়ালৈ ((nr = পঠ(pfd[0], buffer, BUFSIZ)) > 0) {
			p = buffer;
			যেতিয়ালৈ (*p && p - buffer < nr) {
				token = strsep(&p, "=");
				যদি (token) {
					unsetenv(token);
					যদি (*p) {
						setenv(token, p, 1);
						p += strlen(p) + 1;
					} অন্যথা
						p++;
				}
			}
		}

		মুক্ত(buffer);
		close(pfd[0]);

		rc_waitpid(pid);
	}
}

শূন্য
rc_plugin_unload(শূন্য)
{
	PLUGIN *plugin = TAILQ_FIRST(&plugins);
	PLUGIN *next;

	যেতিয়ালৈ (plugin) {
		next = TAILQ_NEXT(plugin, entries);
		dlclose(plugin->handle);
		মুক্ত(plugin->name);
		মুক্ত(plugin);
		plugin = next;
	}
	TAILQ_INIT(&plugins);
}
