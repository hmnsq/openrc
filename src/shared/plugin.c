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

#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <dirent.h>
#ଅନ୍ତର୍ଭାଵ <dlfcn.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "rc_exec.h"
#ଅନ୍ତର୍ଭାଵ "plugin.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

#define RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_in_plugin = false;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା plugin
{
	ଅକ୍ଷର *name;
	ଶୂନ୍ୟ *handle;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*hook)(RC_HOOK, ଅଚଳ ଅକ୍ଷର *);
	TAILQ_ENTRY(plugin) entries;
} PLUGIN;
TAILQ_HEAD(, plugin) plugins;

ଶୂନ୍ୟ
rc_plugin_load(ଶୂନ୍ୟ)
{
	DIR *dp;
	ସଂରଚନା dirent *d;
	PLUGIN *plugin;
	ଅକ୍ଷର *file = ଖାଲି;
	ଶୂନ୍ୟ *h;
	ସଂଘ {
		ଶୂନ୍ୟ *sym;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*func)(RC_HOOK, ଅଚଳ ଅକ୍ଷର *);
	} fptr;

	/* Don't load plugins if we're in one */
	ଯଦି (rc_in_plugin)
		ଫେରନ୍ତୁ;

	TAILQ_INIT(&plugins);

	ଯଦି (!(dp = opendir(RC_PLUGINDIR)))
		ଫେରନ୍ତୁ;

	ଯେତେବେଳେ ((d = readdir(dp))) {
		ଯଦି (d->d_name[0] == '.')
			ଜାରି;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		ମୁକ୍ତ(file);
		ଯଦି (h == ଖାଲି) {
			eerror("dlopen: %s", dlerror());
			ଜାରି;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		ଯଦି (fptr.func == ଖାଲି) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} ଅନ୍ୟଥା {
			plugin = xmalloc(ଆକାର(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_ଭର୍ତ୍ତି_TAIL(&plugins, plugin, entries);
		}
	}
	closedir(dp);
}

ଶୂନ୍ୟ
rc_plugin_run(RC_HOOK hook, ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	PLUGIN *plugin;
	ସଂରଚନା sigaction sa;
	sigset_t empty;
	sigset_t full;
	sigset_t old;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pfd[2];
	pid_t pid;
	ଅକ୍ଷର *buffer;
	ଅକ୍ଷର *token;
	ଅକ୍ଷର *p;
	ssize_t nr;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;

	/* Don't run plugins if we're in one */
	ଯଦି (rc_in_plugin)
		ଫେରନ୍ତୁ;

	/* We need to block signals until we have forked */
	memset(&sa, 0, ଆକାର(sa));
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
		ଯଦି (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", strerror(errno));
			ଫେରନ୍ତୁ;
		}

		sigprocmask(SIG_SETMASK, &full, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		ଯଦି ((pid = fork()) == -1) {
			eerror("fork: %s", strerror(errno));
			ଭାଙ୍ଗ;
		}

		ଯଦି (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, ଖାଲି);
			sigaction(SIGHUP,  &sa, ଖାଲି);
			sigaction(SIGINT,  &sa, ଖାଲି);
			sigaction(SIGQUIT, &sa, ଖାଲି);
			sigaction(SIGTERM, &sa, ଖାଲି);
			sigaction(SIGUSR1, &sa, ଖାଲି);
			sigaction(SIGWINCH, &sa, ଖାଲି);
			sigprocmask(SIG_SETMASK, &old, ଖାଲି);

			rc_in_plugin = true;
			close(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, ମୂଲ୍ୟ);
			fclose(rc_environ_fd);
			rc_environ_fd = ଖାଲି;

			/* Just in case the plugin sets this to false */
			rc_in_plugin = true;
			exit(retval);
		}

		sigprocmask(SIG_SETMASK, &old, ଖାଲି);
		close(pfd[1]);
		buffer = xmalloc(ଆକାର(ଅକ୍ଷର) * BUFSIZ);
		memset(buffer, 0, BUFSIZ);

		ଯେତେବେଳେ ((nr = ପଠ(pfd[0], buffer, BUFSIZ)) > 0) {
			p = buffer;
			ଯେତେବେଳେ (*p && p - buffer < nr) {
				token = strsep(&p, "=");
				ଯଦି (token) {
					unsetenv(token);
					ଯଦି (*p) {
						setenv(token, p, 1);
						p += strlen(p) + 1;
					} ଅନ୍ୟଥା
						p++;
				}
			}
		}

		ମୁକ୍ତ(buffer);
		close(pfd[0]);

		rc_waitpid(pid);
	}
}

ଶୂନ୍ୟ
rc_plugin_unload(ଶୂନ୍ୟ)
{
	PLUGIN *plugin = TAILQ_FIRST(&plugins);
	PLUGIN *next;

	ଯେତେବେଳେ (plugin) {
		next = TAILQ_NEXT(plugin, entries);
		dlclose(plugin->handle);
		ମୁକ୍ତ(plugin->name);
		ମୁକ୍ତ(plugin);
		plugin = next;
	}
	TAILQ_INIT(&plugins);
}
