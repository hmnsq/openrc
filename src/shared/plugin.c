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

#समावेश <sys/types.h>
#समावेश <sys/wait.h>
#समावेश <dirent.h>
#समावेश <dlfcn.h>
#समावेश <errno.h>
#समावेश <fcntl.h>
#समावेश <signal.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <unistd.h>

#समावेश "einfo.h"
#समावेश "queue.h"
#समावेश "rc.h"
#समावेश "rc_exec.h"
#समावेश "plugin.h"
#समावेश "helpers.h"

#घोषणा RC_PLUGIN_HOOK "rc_plugin_hook"

bool rc_in_plugin = false;

प्रकार काष्ठा plugin
{
	अक्षर *name;
	व्योम *handle;
	पूर्णांक (*hook)(RC_HOOK, स्थिर अक्षर *);
	TAILQ_ENTRY(plugin) entries;
} PLUGIN;
TAILQ_शीर्ष(, plugin) plugins;

व्योम
rc_plugin_load(व्योम)
{
	सूची *dp;
	काष्ठा dirent *d;
	PLUGIN *plugin;
	अक्षर *file = शून्य;
	व्योम *h;
	जोड़ {
		व्योम *sym;
		पूर्णांक (*func)(RC_HOOK, स्थिर अक्षर *);
	} fptr;

	/* Don't load plugins if we're in one */
	अगर (rc_in_plugin)
		वापस;

	TAILQ_INIT(&plugins);

	अगर (!(dp = सूची_खोलो(RC_PLUGINDIR)))
		वापस;

	जबतक ((d = सूची_पढ़ो(dp))) {
		अगर (d->d_name[0] == '.')
			जारी;

		xasprintf(&file, RC_PLUGINDIR "/%s",  d->d_name);
		h = dlopen(file, RTLD_LAZY);
		मुक्त(file);
		अगर (h == शून्य) {
			eerror("dlopen: %s", dlerror());
			जारी;
		}

		fptr.sym = dlsym(h, RC_PLUGIN_HOOK);
		अगर (fptr.func == शून्य) {
			eerror("%s: cannot find symbol `%s'",
			    d->d_name, RC_PLUGIN_HOOK);
			dlclose(h);
		} अन्यथा {
			plugin = xmalloc(माप(*plugin));
			plugin->name = xstrdup(d->d_name);
			plugin->handle = h;
			plugin->hook = fptr.func;
			TAILQ_जोड़ो_पुच्छ(&plugins, plugin, entries);
		}
	}
	बंद_सूची(dp);
}

व्योम
rc_plugin_run(RC_HOOK hook, स्थिर अक्षर *मूल्य)
{
	PLUGIN *plugin;
	काष्ठा sigaction sa;
	sigset_t empty;
	sigset_t full;
	sigset_t old;
	पूर्णांक pfd[2];
	pid_t pid;
	अक्षर *बफर;
	अक्षर *token;
	अक्षर *p;
	ssize_t nr;
	पूर्णांक retval;

	/* Don't run plugins if we're in one */
	अगर (rc_in_plugin)
		वापस;

	/* We need to block signals until we have forked */
	स_रखो(&sa, 0, माप(sa));
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
		अगर (pipe2(pfd, O_CLOEXEC) == -1) {
			eerror("pipe2: %s", म_त्रुटि(त्रुटि_सं));
			वापस;
		}

		sigprocmask(SIG_SETMASK, &full, &old);

		/* We run the plugin in a new process so we never crash
		 * or otherwise affected by it */
		अगर ((pid = विभाजन()) == -1) {
			eerror("fork: %s", म_त्रुटि(त्रुटि_सं));
			अवरोध;
		}

		अगर (pid == 0) {
			/* Restore default handlers */
			sigaction(SIGCHLD, &sa, शून्य);
			sigaction(SIGHUP,  &sa, शून्य);
			sigaction(संक_विघ्न,  &sa, शून्य);
			sigaction(SIGQUIT, &sa, शून्य);
			sigaction(संक_इति, &sa, शून्य);
			sigaction(SIGUSR1, &sa, शून्य);
			sigaction(SIGWINCH, &sa, शून्य);
			sigprocmask(SIG_SETMASK, &old, शून्य);

			rc_in_plugin = true;
			बंद(pfd[0]);
			rc_environ_fd = fdopen(pfd[1], "w");
			retval = plugin->hook(hook, मूल्य);
			ख_बंद(rc_environ_fd);
			rc_environ_fd = शून्य;

			/* Just in case the plugin sets this to false */
			rc_in_plugin = true;
			निकास(retval);
		}

		sigprocmask(SIG_SETMASK, &old, शून्य);
		बंद(pfd[1]);
		बफर = xmalloc(माप(अक्षर) * बफ_मान);
		स_रखो(बफर, 0, बफ_मान);

		जबतक ((nr = पढ़ो(pfd[0], बफर, बफ_मान)) > 0) {
			p = बफर;
			जबतक (*p && p - बफर < nr) {
				token = strsep(&p, "=");
				अगर (token) {
					unsetenv(token);
					अगर (*p) {
						setenv(token, p, 1);
						p += म_माप(p) + 1;
					} अन्यथा
						p++;
				}
			}
		}

		मुक्त(बफर);
		बंद(pfd[0]);

		rc_waitpid(pid);
	}
}

व्योम
rc_plugin_unload(व्योम)
{
	PLUGIN *plugin = TAILQ_FIRST(&plugins);
	PLUGIN *अग्र;

	जबतक (plugin) {
		अग्र = TAILQ_अग्र(plugin, entries);
		dlclose(plugin->handle);
		मुक्त(plugin->name);
		मुक्त(plugin);
		plugin = अग्र;
	}
	TAILQ_INIT(&plugins);
}
