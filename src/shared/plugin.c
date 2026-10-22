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

#تعریف_کرو RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_اندر_plugin = false;

قسمتعریف ساخت plugin
{
	حرف *name;
	خالی *handle;
	صحیح (*hook)(RC_HOOK, ثابت حرف *);
	TAILQ_اندراج(plugin) entries;
} PLUGIN;
TAILQ_سر(, plugin) plugins;

خالی
rc_plugin_بھرو(خالی)
{
	DIR *dp;
	ساخت dirent *d;
	PLUGIN *plugin;
	حرف *file = تہی;
	خالی *h;
	اتحاد {
		خالی *sym;
		صحیح (*func)(RC_HOOK, ثابت حرف *);
	} fptr;

	/* Don't load plugins if we're in one */
	اگر (rc_اندر_plugin)
		واپس;

	TAILQ_INIT(&plugins);

	اگر (!(dp = opendir(RC_PLUGINDIR)))
		واپس;

	جب ((d = readdir(dp))) {
		اگر (d->d_name[0] == '.')
			جاری;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		آزاد(file);
		اگر (h == تہی) {
			eerror("dlopen: %s", dlerror());
			جاری;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		اگر (fptr.func == تہی) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} ورنہ {
			plugin = xmalloc(حجم(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_داخل_کرو_دم(&plugins, plugin, entries);
		}
	}
	closedir(dp);
}

خالی
rc_plugin_تسلسل(RC_HOOK hook, ثابت حرف *قدر)
{
	PLUGIN *plugin;
	ساخت sigaction sa;
	sigset_t خالی_خانے;
	sigset_t بھرا;
	sigset_t old;
	صحیح pfd[2];
	pid_t pid;
	حرف *بفر;
	حرف *token;
	حرف *p;
	ssize_t nr;
	صحیح retval;

	/* Don't run plugins if we're in one */
	اگر (rc_اندر_plugin)
		واپس;

	/* We need to block signals until we have forked */
	memset(&sa, 0, حجم(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigemptyset(&خالی_خانے);
	sigfillset(&بھرا);

	TAILQ_FOREACH(plugin, &plugins, entries) {
		/* We create a pipe so that plugins can affect our environment
		 * vars, which in turn influence our scripts.
		 * O_CLOEXEC to stop any scripts from inheriting us.
		 * This is actually quite important as without this, the splash
		 * plugin will probably hang when running in silent mode. */
		اگر (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", strerror(errno));
			واپس;
		}

		sigprocmask(SIG_SETMASK, &بھرا, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		اگر ((pid = fork()) == -1) {
			eerror("fork: %s", strerror(errno));
			توڑو;
		}

		اگر (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, تہی);
			sigaction(SIGHUP,  &sa, تہی);
			sigaction(SIGINT,  &sa, تہی);
			sigaction(SIGQUIT, &sa, تہی);
			sigaction(SIGTERM, &sa, تہی);
			sigaction(SIGUSR1, &sa, تہی);
			sigaction(SIGWINCH, &sa, تہی);
			sigprocmask(SIG_SETMASK, &old, تہی);

			rc_اندر_plugin = true;
			close(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, قدر);
			fclose(rc_environ_fd);
			rc_environ_fd = تہی;

			/* Just in case the plugin sets this to false */
			rc_اندر_plugin = true;
			باہر_نکلو(retval);
		}

		sigprocmask(SIG_SETMASK, &old, تہی);
		close(pfd[1]);
		بفر = xmalloc(حجم(حرف) * BUFSIZ);
		memset(بفر, 0, BUFSIZ);

		جب ((nr = اقرأ(pfd[0], بفر, BUFSIZ)) > 0) {
			p = بفر;
			جب (*p && p - بفر < nr) {
				token = strsep(&p, "=");
				اگر (token) {
					unsetenv(token);
					اگر (*p) {
						setenv(token, p, 1);
						p += لڑی_لمبائی(p) + 1;
					} ورنہ
						p++;
				}
			}
		}

		آزاد(بفر);
		close(pfd[0]);

		rc_waitpid(pid);
	}
}

خالی
rc_plugin_unload(خالی)
{
	PLUGIN *plugin = TAILQ_پہلا(&plugins);
	PLUGIN *اگلا;

	جب (plugin) {
		اگلا = TAILQ_اگلا(plugin, entries);
		dlclose(plugin->handle);
		آزاد(plugin->name);
		آزاد(plugin);
		plugin = اگلا;
	}
	TAILQ_INIT(&plugins);
}
