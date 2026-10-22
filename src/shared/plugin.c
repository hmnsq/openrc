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

#شامل <sys/types.h>
#شامل <sys/wait.h>
#شامل <dirent.h>
#شامل <dlfcn.h>
#شامل <errno.h>
#شامل <fcntl.h>
#شامل <signal.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <unistd.h>

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "rc_exec.h"
#شامل "plugin.h"
#شامل "helpers.h"

#define RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_in_plugin = false;

تعریفنوع ساختار plugin
{
	نویسه *name;
	تهی *handle;
	صحیح (*hook)(RC_HOOK, ثابت نویسه *);
	TAILQ_ENTRY(plugin) entries;
} PLUGIN;
TAILQ_HEAD(, plugin) plugins;

تهی
rc_plugin_load(تهی)
{
	DIR *dp;
	ساختار dirent *d;
	PLUGIN *plugin;
	نویسه *file = پوچ;
	تهی *h;
	اجتماع {
		تهی *sym;
		صحیح (*func)(RC_HOOK, ثابت نویسه *);
	} fptr;

	/* Don't load plugins if we're in one */
	اگر (rc_in_plugin)
		بازگشت;

	TAILQ_INIT(&plugins);

	اگر (!(dp = opendir(RC_PLUGINDIR)))
		بازگشت;

	وقتی ((d = readdir(dp))) {
		اگر (d->d_name[0] == '.')
			ادامه;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		آزاد(file);
		اگر (h == پوچ) {
			eerror("dlopen: %s", dlerror());
			ادامه;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		اگر (fptr.func == پوچ) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} وگرنه {
			plugin = xmalloc(اندازه(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_درج_TAIL(&plugins, plugin, entries);
		}
	}
	closedir(dp);
}

تهی
rc_plugin_run(RC_HOOK hook, ثابت نویسه *مقدار)
{
	PLUGIN *plugin;
	ساختار sigaction sa;
	sigset_t empty;
	sigset_t full;
	sigset_t old;
	صحیح pfd[2];
	pid_t pid;
	نویسه *buffer;
	نویسه *token;
	نویسه *p;
	ssize_t nr;
	صحیح retval;

	/* Don't run plugins if we're in one */
	اگر (rc_in_plugin)
		بازگشت;

	/* We need to block signals until we have forked */
	memset(&sa, 0, اندازه(sa));
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
		اگر (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", strerror(errno));
			بازگشت;
		}

		sigprocmask(SIG_SETMASK, &full, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		اگر ((pid = fork()) == -1) {
			eerror("fork: %s", strerror(errno));
			بشکن;
		}

		اگر (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, پوچ);
			sigaction(SIGHUP,  &sa, پوچ);
			sigaction(SIGINT,  &sa, پوچ);
			sigaction(SIGQUIT, &sa, پوچ);
			sigaction(SIGTERM, &sa, پوچ);
			sigaction(SIGUSR1, &sa, پوچ);
			sigaction(SIGWINCH, &sa, پوچ);
			sigprocmask(SIG_SETMASK, &old, پوچ);

			rc_in_plugin = true;
			close(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, مقدار);
			fclose(rc_environ_fd);
			rc_environ_fd = پوچ;

			/* Just in case the plugin sets this to false */
			rc_in_plugin = true;
			exit(retval);
		}

		sigprocmask(SIG_SETMASK, &old, پوچ);
		close(pfd[1]);
		buffer = xmalloc(اندازه(نویسه) * BUFSIZ);
		memset(buffer, 0, BUFSIZ);

		وقتی ((nr = اقرأ(pfd[0], buffer, BUFSIZ)) > 0) {
			p = buffer;
			وقتی (*p && p - buffer < nr) {
				token = strsep(&p, "=");
				اگر (token) {
					unsetenv(token);
					اگر (*p) {
						setenv(token, p, 1);
						p += strlen(p) + 1;
					} وگرنه
						p++;
				}
			}
		}

		آزاد(buffer);
		close(pfd[0]);

		rc_waitpid(pid);
	}
}

تهی
rc_plugin_unload(تهی)
{
	PLUGIN *plugin = TAILQ_FIRST(&plugins);
	PLUGIN *next;

	وقتی (plugin) {
		next = TAILQ_NEXT(plugin, entries);
		dlclose(plugin->handle);
		آزاد(plugin->name);
		آزاد(plugin);
		plugin = next;
	}
	TAILQ_INIT(&plugins);
}
