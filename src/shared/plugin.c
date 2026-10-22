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

#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <dirent.h>
#అన్తర్భావ <dlfcn.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "rc_exec.h"
#అన్తర్భావ "plugin.h"
#అన్తర్భావ "helpers.h"

#నిర్వచించు RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_లోపల_plugin = false;

రకనిర్వచనం నిర్మాణం plugin
{
	అక్షరం *name;
	శూన్య *handle;
	పూర్ణాంకం (*hook)(RC_HOOK, స్థిరరాశి అక్షరం *);
	TAILQ_నమోదు(plugin) entries;
} PLUGIN;
TAILQ_తల(, plugin) plugins;

శూన్య
rc_plugin_ఎక్కించు(శూన్య)
{
	DIR *dp;
	నిర్మాణం dirent *d;
	PLUGIN *plugin;
	అక్షరం *file = ఖాళీ;
	శూన్య *h;
	సంఘం {
		శూన్య *sym;
		పూర్ణాంకం (*func)(RC_HOOK, స్థిరరాశి అక్షరం *);
	} fptr;

	/* Don't load plugins if we're in one */
	ఒకవేళ (rc_లోపల_plugin)
		తిరిగి;

	TAILQ_INIT(&plugins);

	ఒకవేళ (!(dp = opendir(RC_PLUGINDIR)))
		తిరిగి;

	అయితే ((d = readdir(dp))) {
		ఒకవేళ (d->d_name[0] == '.')
			కొనసాగు;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		విడుదల(file);
		ఒకవేళ (h == ఖాళీ) {
			eerror("dlopen: %s", dlerror());
			కొనసాగు;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		ఒకవేళ (fptr.func == ఖాళీ) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} లేకపోతే {
			plugin = xmalloc(పరిమాణం(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_చొప్పించు_తోక(&plugins, plugin, entries);
		}
	}
	closedir(dp);
}

శూన్య
rc_plugin_నిరంతరం(RC_HOOK hook, స్థిరరాశి అక్షరం *విలువ)
{
	PLUGIN *plugin;
	నిర్మాణం sigaction sa;
	sigset_t ఖాళీలు;
	sigset_t నిండిన;
	sigset_t old;
	పూర్ణాంకం pfd[2];
	pid_t pid;
	అక్షరం *బఫరు;
	అక్షరం *token;
	అక్షరం *p;
	ssize_t nr;
	పూర్ణాంకం retval;

	/* Don't run plugins if we're in one */
	ఒకవేళ (rc_లోపల_plugin)
		తిరిగి;

	/* We need to block signals until we have forked */
	memset(&sa, 0, పరిమాణం(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigemptyset(&ఖాళీలు);
	sigfillset(&నిండిన);

	TAILQ_FOREACH(plugin, &plugins, entries) {
		/* We create a pipe so that plugins can affect our environment
		 * vars, which in turn influence our scripts.
		 * O_CLOEXEC to stop any scripts from inheriting us.
		 * This is actually quite important as without this, the splash
		 * plugin will probably hang when running in silent mode. */
		ఒకవేళ (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", strerror(errno));
			తిరిగి;
		}

		sigprocmask(SIG_SETMASK, &నిండిన, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		ఒకవేళ ((pid = fork()) == -1) {
			eerror("fork: %s", strerror(errno));
			ఆపు;
		}

		ఒకవేళ (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, ఖాళీ);
			sigaction(SIGHUP,  &sa, ఖాళీ);
			sigaction(SIGINT,  &sa, ఖాళీ);
			sigaction(SIGQUIT, &sa, ఖాళీ);
			sigaction(SIGTERM, &sa, ఖాళీ);
			sigaction(SIGUSR1, &sa, ఖాళీ);
			sigaction(SIGWINCH, &sa, ఖాళీ);
			sigprocmask(SIG_SETMASK, &old, ఖాళీ);

			rc_లోపల_plugin = true;
			close(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, విలువ);
			fclose(rc_environ_fd);
			rc_environ_fd = ఖాళీ;

			/* Just in case the plugin sets this to false */
			rc_లోపల_plugin = true;
			నిష్క్రమించు(retval);
		}

		sigprocmask(SIG_SETMASK, &old, ఖాళీ);
		close(pfd[1]);
		బఫరు = xmalloc(పరిమాణం(అక్షరం) * BUFSIZ);
		memset(బఫరు, 0, BUFSIZ);

		అయితే ((nr = పఠ(pfd[0], బఫరు, BUFSIZ)) > 0) {
			p = బఫరు;
			అయితే (*p && p - బఫరు < nr) {
				token = strsep(&p, "=");
				ఒకవేళ (token) {
					unsetenv(token);
					ఒకవేళ (*p) {
						setenv(token, p, 1);
						p += అక్షరశ్రేణి_పొడవు(p) + 1;
					} లేకపోతే
						p++;
				}
			}
		}

		విడుదల(బఫరు);
		close(pfd[0]);

		rc_waitpid(pid);
	}
}

శూన్య
rc_plugin_unload(శూన్య)
{
	PLUGIN *plugin = TAILQ_మొదటి(&plugins);
	PLUGIN *తదుపరి;

	అయితే (plugin) {
		తదుపరి = TAILQ_తదుపరి(plugin, entries);
		dlclose(plugin->handle);
		విడుదల(plugin->name);
		విడుదల(plugin);
		plugin = తదుపరి;
	}
	TAILQ_INIT(&plugins);
}
