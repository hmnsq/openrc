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

#define RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_in_plugin = false;

प्रकारनिर्देश संरचना plugin
{
	वर्ण *name;
	शून्य *handle;
	पूर्णाङ्क (*hook)(RC_HOOK, ध्रुव वर्ण *);
	TAILQ_ENTRY(plugin) entries;
} PLUGIN;
TAILQ_HEAD(, plugin) plugins;

शून्य
rc_plugin_load(शून्य)
{
	सूची *dp;
	संरचना dirent *d;
	PLUGIN *plugin;
	वर्ण *file = रिक्त;
	शून्य *h;
	सङ्घ {
		शून्य *sym;
		पूर्णाङ्क (*func)(RC_HOOK, ध्रुव वर्ण *);
	} fptr;

	/* Don't load plugins if we're in one */
	यदि (rc_in_plugin)
		प्रत्यागम;

	TAILQ_INIT(&plugins);

	यदि (!(dp = सूची_खोलो(RC_PLUGINDIR)))
		प्रत्यागम;

	यावत् ((d = सूची_पढ़ो(dp))) {
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
			TAILQ_जोड़ो_TAIL(&plugins, plugin, entries);
		}
	}
	बंद_सूची(dp);
}

शून्य
rc_plugin_run(RC_HOOK hook, ध्रुव वर्ण *मूल्य)
{
	PLUGIN *plugin;
	संरचना sigaction sa;
	sigset_t empty;
	sigset_t full;
	sigset_t old;
	पूर्णाङ्क pfd[2];
	pid_t pid;
	वर्ण *buffer;
	वर्ण *token;
	वर्ण *p;
	ssize_t nr;
	पूर्णाङ्क retval;

	/* Don't run plugins if we're in one */
	यदि (rc_in_plugin)
		प्रत्यागम;

	/* We need to block signals until we have forked */
	स_रखो(&sa, 0, परिमाण(sa));
	sa.sa_handler = संक_पूर्व;
	sigemptyset(&sa.sa_mask);
	sigemptyset(&empty);
	sigfillset(&full);

	TAILQ_FOREACH(plugin, &plugins, entries) {
		/* We create a pipe so that plugins can affect our environment
		 * vars, which in turn influence our scripts.
		 * O_CLOEXEC to stop any scripts from inheriting us.
		 * This is actually quite important as without this, the splash
		 * plugin will probably hang when running in silent mode. */
		यदि (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", म_त्रुटि(त्रुटि_सं));
			प्रत्यागम;
		}

		sigprocmask(SIG_SETMASK, &full, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		यदि ((pid = विभाजन()) == -1) {
			eerror("fork: %s", म_त्रुटि(त्रुटि_सं));
			भङ्ग;
		}

		यदि (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, रिक्त);
			sigaction(SIGHUP,  &sa, रिक्त);
			sigaction(संक_विघ्न,  &sa, रिक्त);
			sigaction(SIGQUIT, &sa, रिक्त);
			sigaction(संक_इति, &sa, रिक्त);
			sigaction(SIGUSR1, &sa, रिक्त);
			sigaction(SIGWINCH, &sa, रिक्त);
			sigprocmask(SIG_SETMASK, &old, रिक्त);

			rc_in_plugin = true;
			बंद(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, मूल्य);
			ख_बंद(rc_environ_fd);
			rc_environ_fd = रिक्त;

			/* Just in case the plugin sets this to false */
			rc_in_plugin = true;
			निकास(retval);
		}

		sigprocmask(SIG_SETMASK, &old, रिक्त);
		बंद(pfd[1]);
		buffer = xmalloc(परिमाण(वर्ण) * बफ_मान);
		स_रखो(buffer, 0, बफ_मान);

		यावत् ((nr = पठ(pfd[0], buffer, बफ_मान)) > 0) {
			p = buffer;
			यावत् (*p && p - buffer < nr) {
				token = strsep(&p, "=");
				यदि (token) {
					unsetenv(token);
					यदि (*p) {
						setenv(token, p, 1);
						p += म_माप(p) + 1;
					} अन्यथा
						p++;
				}
			}
		}

		मुक्त(buffer);
		बंद(pfd[0]);

		rc_waitpid(pid);
	}
}

शून्य
rc_plugin_unload(शून्य)
{
	PLUGIN *plugin = TAILQ_FIRST(&plugins);
	PLUGIN *next;

	यावत् (plugin) {
		next = TAILQ_NEXT(plugin, entries);
		dlclose(plugin->handle);
		मुक्त(plugin->name);
		मुक्त(plugin);
		plugin = next;
	}
	TAILQ_INIT(&plugins);
}
