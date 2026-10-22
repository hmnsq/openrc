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

#ضمن <sys/types.h>
#ضمن <sys/wait.h>
#ضمن <dirent.h>
#ضمن <dlfcn.h>
#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <signal.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <unistd.h>

#ضمن "einfo.h"
#ضمن "queue.h"
#ضمن "rc.h"
#ضمن "rc_exec.h"
#ضمن "plugin.h"
#ضمن "helpers.h"

#تعریف_کن RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_درون_plugin = false;

تعريفنوع بنية plugin
{
	حرف *name;
	فراغ *handle;
	صحيح (*hook)(RC_HOOK, ثابت حرف *);
	TAILQ_مدخل(plugin) entries;
} PLUGIN;
TAILQ_سر(, plugin) plugins;

فراغ
rc_plugin_بارگیری(فراغ)
{
	DIR *dp;
	بنية dirent *d;
	PLUGIN *plugin;
	حرف *file = عدم;
	فراغ *h;
	اتحاد {
		فراغ *sym;
		صحيح (*func)(RC_HOOK, ثابت حرف *);
	} fptr;

	/* Don't load plugins if we're in one */
	إذا (rc_درون_plugin)
		أرجع;

	TAILQ_INIT(&plugins);

	إذا (!(dp = opendir(RC_PLUGINDIR)))
		أرجع;

	طالما ((d = readdir(dp))) {
		إذا (d->d_name[0] == '.')
			استمر;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		حرر(file);
		إذا (h == عدم) {
			eerror("dlopen: %s", dlerror());
			استمر;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		إذا (fptr.func == عدم) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} وإلا {
			plugin = xmalloc(حجم(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_درج_دم(&plugins, plugin, entries);
		}
	}
	closedir(dp);
}

فراغ
rc_plugin_پیاپی(RC_HOOK hook, ثابت حرف *مقدار)
{
	PLUGIN *plugin;
	بنية sigaction sa;
	sigset_t خالی;
	sigset_t پر;
	sigset_t old;
	صحيح pfd[2];
	pid_t pid;
	حرف *میانگیر;
	حرف *token;
	حرف *p;
	ssize_t nr;
	صحيح retval;

	/* Don't run plugins if we're in one */
	إذا (rc_درون_plugin)
		أرجع;

	/* We need to block signals until we have forked */
	memset(&sa, 0, حجم(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigemptyset(&خالی);
	sigfillset(&پر);

	TAILQ_FOREACH(plugin, &plugins, entries) {
		/* We create a pipe so that plugins can affect our environment
		 * vars, which in turn influence our scripts.
		 * O_CLOEXEC to stop any scripts from inheriting us.
		 * This is actually quite important as without this, the splash
		 * plugin will probably hang when running in silent mode. */
		إذا (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", strerror(errno));
			أرجع;
		}

		sigprocmask(SIG_SETMASK, &پر, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		إذا ((pid = fork()) == -1) {
			eerror("fork: %s", strerror(errno));
			اكسر;
		}

		إذا (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, عدم);
			sigaction(SIGHUP,  &sa, عدم);
			sigaction(SIGINT,  &sa, عدم);
			sigaction(SIGQUIT, &sa, عدم);
			sigaction(SIGTERM, &sa, عدم);
			sigaction(SIGUSR1, &sa, عدم);
			sigaction(SIGWINCH, &sa, عدم);
			sigprocmask(SIG_SETMASK, &old, عدم);

			rc_درون_plugin = true;
			close(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, مقدار);
			fclose(rc_environ_fd);
			rc_environ_fd = عدم;

			/* Just in case the plugin sets this to false */
			rc_درون_plugin = true;
			خروج(retval);
		}

		sigprocmask(SIG_SETMASK, &old, عدم);
		close(pfd[1]);
		میانگیر = xmalloc(حجم(حرف) * BUFSIZ);
		memset(میانگیر, 0, BUFSIZ);

		طالما ((nr = اقرأ(pfd[0], میانگیر, BUFSIZ)) > 0) {
			p = میانگیر;
			طالما (*p && p - میانگیر < nr) {
				token = strsep(&p, "=");
				إذا (token) {
					unsetenv(token);
					إذا (*p) {
						setenv(token, p, 1);
						p += طول_رشته(p) + 1;
					} وإلا
						p++;
				}
			}
		}

		حرر(میانگیر);
		close(pfd[0]);

		rc_waitpid(pid);
	}
}

فراغ
rc_plugin_unload(فراغ)
{
	PLUGIN *plugin = TAILQ_اولی(&plugins);
	PLUGIN *بعدی;

	طالما (plugin) {
		بعدی = TAILQ_بعدی(plugin, entries);
		dlclose(plugin->handle);
		حرر(plugin->name);
		حرر(plugin);
		plugin = بعدی;
	}
	TAILQ_INIT(&plugins);
}
