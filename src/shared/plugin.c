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

#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <dirent.h>
#अन्तर्भाव <dlfcn.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "rc_exec.h"
#अन्तर्भाव "plugin.h"
#अन्तर्भाव "helpers.h"

#परिभाषय RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_अन्तर्_plugin = false;

प्रकारनिर्देश संरचना plugin
{
	वर्ण *name;
	शून्य *handle;
	पूर्णाङ्क (*hook)(RC_HOOK, ध्रुव वर्ण *);
	TAILQ_प्रविष्टिः(plugin) entries;
} PLUGIN;
TAILQ_शीर्ष(, plugin) plugins;

शून्य
rc_plugin_आरोपय(शून्य)
{
	DIR *dp;
	संरचना dirent *d;
	PLUGIN *plugin;
	वर्ण *file = रिक्त;
	शून्य *h;
	सङ्घ {
		शून्य *sym;
		पूर्णाङ्क (*func)(RC_HOOK, ध्रुव वर्ण *);
	} fptr;

	/* Don't load plugins if we're in one */
	यदि (rc_अन्तर्_plugin)
		प्रत्यागम;

	TAILQ_INIT(&plugins);

	यदि (!(dp = opendir(RC_PLUGINDIR)))
		प्रत्यागम;

	यावत् ((d = readdir(dp))) {
		यदि (d->d_name[0] == '.')
			अनुवर्त;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		मुक्त(file);
		यदि (h == रिक्त) {
			eerror("dlopen: %s", dlerror());
			अनुवर्त;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		यदि (fptr.func == रिक्त) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} अन्यथा {
			plugin = xmalloc(परिमाण(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_निवेशय_पुच्छ(&plugins, plugin, entries);
		}
	}
	closedir(dp);
}

शून्य
rc_plugin_सततम्(RC_HOOK hook, ध्रुव वर्ण *मानम्)
{
	PLUGIN *plugin;
	संरचना sigaction sa;
	sigset_t रिक्तानि;
	sigset_t पूर्णम्;
	sigset_t old;
	पूर्णाङ्क pfd[2];
	pid_t pid;
	वर्ण *बफर;
	वर्ण *token;
	वर्ण *p;
	ssize_t nr;
	पूर्णाङ्क retval;

	/* Don't run plugins if we're in one */
	यदि (rc_अन्तर्_plugin)
		प्रत्यागम;

	/* We need to block signals until we have forked */
	स्मृतिं_पूरय(&sa, 0, परिमाण(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigemptyset(&रिक्तानि);
	sigfillset(&पूर्णम्);

	TAILQ_FOREACH(plugin, &plugins, entries) {
		/* We create a pipe so that plugins can affect our environment
		 * vars, which in turn influence our scripts.
		 * O_CLOEXEC to stop any scripts from inheriting us.
		 * This is actually quite important as without this, the splash
		 * plugin will probably hang when running in silent mode. */
		यदि (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", strerror(errno));
			प्रत्यागम;
		}

		sigprocmask(SIG_SETMASK, &पूर्णम्, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		यदि ((pid = fork()) == -1) {
			eerror("fork: %s", strerror(errno));
			भङ्ग;
		}

		यदि (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, रिक्त);
			sigaction(SIGHUP,  &sa, रिक्त);
			sigaction(SIGINT,  &sa, रिक्त);
			sigaction(SIGQUIT, &sa, रिक्त);
			sigaction(SIGTERM, &sa, रिक्त);
			sigaction(SIGUSR1, &sa, रिक्त);
			sigaction(SIGWINCH, &sa, रिक्त);
			sigprocmask(SIG_SETMASK, &old, रिक्त);

			rc_अन्तर्_plugin = true;
			close(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, मानम्);
			fclose(rc_environ_fd);
			rc_environ_fd = रिक्त;

			/* Just in case the plugin sets this to false */
			rc_अन्तर्_plugin = true;
			निर्गच्छ(retval);
		}

		sigprocmask(SIG_SETMASK, &old, रिक्त);
		close(pfd[1]);
		बफर = xmalloc(परिमाण(वर्ण) * BUFSIZ);
		स्मृतिं_पूरय(बफर, 0, BUFSIZ);

		यावत् ((nr = पठ(pfd[0], बफर, BUFSIZ)) > 0) {
			p = बफर;
			यावत् (*p && p - बफर < nr) {
				token = strsep(&p, "=");
				यदि (token) {
					unsetenv(token);
					यदि (*p) {
						setenv(token, p, 1);
						p += वाक्यदैर्घ्यम्(p) + 1;
					} अन्यथा
						p++;
				}
			}
		}

		मुक्त(बफर);
		close(pfd[0]);

		rc_waitpid(pid);
	}
}

शून्य
rc_plugin_unload(शून्य)
{
	PLUGIN *plugin = TAILQ_FIRST(&plugins);
	PLUGIN *अग्र;

	यावत् (plugin) {
		अग्र = TAILQ_अग्र(plugin, entries);
		dlclose(plugin->handle);
		मुक्त(plugin->name);
		मुक्त(plugin);
		plugin = अग्र;
	}
	TAILQ_INIT(&plugins);
}
