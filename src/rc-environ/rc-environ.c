#अन्तर्भाव <einfo.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <rc.h>
#अन्तर्भाव <queue.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"

स्थिरांक अक्षर *applet;
स्थिरांक अक्षर *extraopts;
स्थिरांक अक्षर *usagestring = "rc-environ [-s <service> ...] [-r <runlevel> ...] [-en0]\n";
स्थिरांक अक्षर getoptstring[] = "ens:r:0" getoptstring_COMMON;
स्थिरांक संरचना option longopts[] = {
	{ "export",          no_argument, रिक्त, 'e' },
	{ "no-escape",       no_argument, रिक्त, 'n' },
	{ "null",            no_argument, रिक्त, '0' },
	{ "service",   required_argument, रिक्त, 's' },
	{ "runlevel",  required_argument, रिक्त, 'r' },
	longopts_COMMON
};

स्थिरांक अक्षर *स्थिरांक longopts_help[] = {
	"Prepend \"export\" to printed variables",
	"Do not perform shell sensitive escape on variable values",
	"End each output line with NUL, not newline",
	"Add service to the list of environments to print",
	"Add services in runlevel to the list of environments to print",
	longopts_help_COMMON
};

स्थिर पूर्णांक
set_environment(स्थिरांक अक्षर *service, अक्षर *vars[])
{
	RC_DEPTREE *deptree = _rc_deptree_load(0, रिक्त);
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service, "reexport");
	RC_STRING *reexport;

	जर (!(rc_service_state(service) & RC_SERVICE_STARTING)) {
		eerror("service %s is not in the starting state", service);
		परत 1;
	}

	साठी (स्थिरांक अक्षर *var; (var = *vars); vars++) {
		माप_प्रकार name_len = म_खोज(var, "=");

		TAILQ_FOREACH(reexport, reexports, entries) {
			जर (म_भेदन(reexport->मूल्य, var, name_len) != 0)
				सुरूठेवा;
			जर (var[name_len] == '=')
				rc_service_putenv(service, var);
			अन्यथा
				rc_service_setenv(service, var, दो_पर्या(var));
			जा next;
		}

		ewarn("service %s does not reexport %.*s, skipping", service, (पूर्णांक)name_len, var);
next:;
	}

	rc_stringlist_free(reexports);
	rc_deptree_free(deptree);
	परत 0;
}

स्थिर शून्य escape_variable(स्थिरांक अक्षर *मूल्य) {
	अक्षर ch;

	म_लिखो("$'");
	साठी (स्थिरांक अक्षर *p = मूल्य; *p; p++) {
		निवड ((ch = *p)) {
		स्थिती '\a': ch = 'a'; थांबा;
		स्थिती '\b': ch = 'b'; थांबा;
		स्थिती '\f': ch = 'f'; थांबा;
		स्थिती '\n': ch = 'n'; थांबा;
		स्थिती '\r': ch = 'r'; थांबा;
		स्थिती '\t': ch = 't'; थांबा;
		स्थिती '\v': ch = 'v'; थांबा;
		/* \e is non-standard. */
		स्थिती 0x1b: ch = 'e'; थांबा;
		स्थिती '"':
		स्थिती '\'':
		स्थिती '\\':
			थांबा;
		पूर्वनिर्धारित:
			अक्षर_दो(ch);
			सुरूठेवा;
		}
		म_लिखो("\\%c", ch);
	}
	म_लिखो("\'");
}

स्थिर पूर्णांक
print_environment(RC_STRINGLIST *services, bool escape, bool export, अक्षर sep)
{
	/* from POSIX.2024 Shell Command Language 2.2, excluding `=` */
	स्थिर स्थिरांक अक्षर special_chars[] = "|&;<>()$`\\\"' \t\n*?[]^-!#~%{},";
	RC_DEPTREE *deptree = _rc_deptree_load(0, रिक्त);
	RC_STRING *service;
	अक्षर *मूल्य = रिक्त;
	पूर्णांक ret = 0;

	जर (!services)
		services = rc_services_in_state(RC_SERVICE_STARTED);

	TAILQ_FOREACH(service, services, entries) {
		RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service->मूल्य, "reexport");
		RC_STRING *reexport;

		TAILQ_FOREACH(reexport, reexports, entries) {
			जर (rc_service_getenv(service->मूल्य, reexport->मूल्य, &मूल्य) == -1) {
				ewarn("%s: failed to read variable '%s' for '%s': %s",
					applet, reexport->मूल्य, service->मूल्य, म_त्रुटि(त्रुटि_सं));
				ret = 1;
				सुरूठेवा;
			}

			जर (export)
				ख_माला_दो("export ", मानक_निकास);
			म_लिखो("%s=", reexport->मूल्य);
			जर (escape && strpbrk(मूल्य, special_chars))
				escape_variable(मूल्य);
			अन्यथा
				ख_माला_दो(मूल्य, मानक_निकास);
			ख_अक्षर_दो(sep, मानक_निकास);
		}

		rc_stringlist_free(reexports);
	}

	मुक्त(मूल्य);
	rc_deptree_free(deptree);
	परत ret;
}

पूर्णांक main(पूर्णांक argc, अक्षर **argv) {
	RC_STRINGLIST *services = रिक्त, *in_runlevel;
	bool export = false, escape = true;
	अक्षर sep = '\n';

	साठी (पूर्णांक opt; (opt = getopt_long(argc, argv, getoptstring, longopts, रिक्त)) != -1;) {
		निवड (opt) {
		स्थिती 'e': export = true; थांबा;
		स्थिती 'n': escape = false; थांबा;
		स्थिती '0': sep = '\0'; थांबा;
		स्थिती 'r':
			जर (!services)
				services = rc_stringlist_new();
			in_runlevel = rc_services_in_runlevel(optarg);
			TAILQ_CONCAT(services, in_runlevel, entries);
			rc_stringlist_free(in_runlevel);
			थांबा;
		स्थिती 's':
			जर (!services)
				services = rc_stringlist_new();
			rc_stringlist_addu(services, optarg);
			थांबा;
		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;

	जर (!argc)
		परत print_environment(services, escape, export, sep);

	परत set_environment(argv[0], &argv[1]);
}
