#समावेश <einfo.h>
#समावेश <errno.h>
#समावेश <getopt.h>
#समावेश <rc.h>
#समावेश <queue.h>
#समावेश <string.h>

#समावेश "_usage.h"
#समावेश "helpers.h"

स्थिर अक्षर *applet;
स्थिर अक्षर *extraopts;
स्थिर अक्षर *usagestring = "rc-environ [-s <service> ...] [-r <runlevel> ...] [-en0]\n";
स्थिर अक्षर getoptstring[] = "ens:r:0" getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "export",          no_argument, शून्य, 'e' },
	{ "no-escape",       no_argument, शून्य, 'n' },
	{ "null",            no_argument, शून्य, '0' },
	{ "service",   required_argument, शून्य, 's' },
	{ "runlevel",  required_argument, शून्य, 'r' },
	longopts_COMMON
};

स्थिर अक्षर *स्थिर longopts_help[] = {
	"Prepend \"export\" to printed variables",
	"Do not perform shell sensitive escape on variable values",
	"End each output line with NUL, not newline",
	"Add service to the list of environments to print",
	"Add services in runlevel to the list of environments to print",
	longopts_help_COMMON
};

अटल पूर्णांक
set_environment(स्थिर अक्षर *service, अक्षर *vars[])
{
	RC_DEPTREE *deptree = _rc_deptree_load(0, शून्य);
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service, "reexport");
	RC_STRING *reexport;

	अगर (!(rc_service_state(service) & RC_SERVICE_STARTING)) {
		eerror("service %s is not in the starting state", service);
		वापस 1;
	}

	क्रम (स्थिर अक्षर *var; (var = *vars); vars++) {
		माप_प्रकार name_len = म_खोज(var, "=");

		TAILQ_FOREACH(reexport, reexports, entries) {
			अगर (म_भेदन(reexport->मूल्य, var, name_len) != 0)
				जारी;
			अगर (var[name_len] == '=')
				rc_service_putenv(service, var);
			अन्यथा
				rc_service_setenv(service, var, दो_पर्या(var));
			जाओ अग्र;
		}

		ewarn("service %s does not reexport %.*s, skipping", service, (पूर्णांक)name_len, var);
अग्र:;
	}

	rc_stringlist_free(reexports);
	rc_deptree_free(deptree);
	वापस 0;
}

अटल व्योम escape_variable(स्थिर अक्षर *मूल्य) {
	अक्षर ch;

	म_लिखो("$'");
	क्रम (स्थिर अक्षर *p = मूल्य; *p; p++) {
		चयन ((ch = *p)) {
		हाल '\a': ch = 'a'; अवरोध;
		हाल '\b': ch = 'b'; अवरोध;
		हाल '\f': ch = 'f'; अवरोध;
		हाल '\n': ch = 'n'; अवरोध;
		हाल '\r': ch = 'r'; अवरोध;
		हाल '\t': ch = 't'; अवरोध;
		हाल '\v': ch = 'v'; अवरोध;
		/* \e is non-standard. */
		हाल 0x1b: ch = 'e'; अवरोध;
		हाल '"':
		हाल '\'':
		हाल '\\':
			अवरोध;
		शेष:
			अक्षर_दो(ch);
			जारी;
		}
		म_लिखो("\\%c", ch);
	}
	म_लिखो("\'");
}

अटल पूर्णांक
print_environment(RC_STRINGLIST *services, bool escape, bool export, अक्षर sep)
{
	/* from POSIX.2024 Shell Command Language 2.2, excluding `=` */
	अटल स्थिर अक्षर special_chars[] = "|&;<>()$`\\\"' \t\n*?[]^-!#~%{},";
	RC_DEPTREE *deptree = _rc_deptree_load(0, शून्य);
	RC_STRING *service;
	अक्षर *मूल्य = शून्य;
	पूर्णांक ret = 0;

	अगर (!services)
		services = rc_services_in_state(RC_SERVICE_STARTED);

	TAILQ_FOREACH(service, services, entries) {
		RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service->मूल्य, "reexport");
		RC_STRING *reexport;

		TAILQ_FOREACH(reexport, reexports, entries) {
			अगर (rc_service_getenv(service->मूल्य, reexport->मूल्य, &मूल्य) == -1) {
				ewarn("%s: failed to read variable '%s' for '%s': %s",
					applet, reexport->मूल्य, service->मूल्य, म_त्रुटि(त्रुटि_सं));
				ret = 1;
				जारी;
			}

			अगर (export)
				ख_माला_दो("export ", मानक_निकास);
			म_लिखो("%s=", reexport->मूल्य);
			अगर (escape && strpbrk(मूल्य, special_chars))
				escape_variable(मूल्य);
			अन्यथा
				ख_माला_दो(मूल्य, मानक_निकास);
			ख_अक्षर_दो(sep, मानक_निकास);
		}

		rc_stringlist_free(reexports);
	}

	मुक्त(मूल्य);
	rc_deptree_free(deptree);
	वापस ret;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv) {
	RC_STRINGLIST *services = शून्य, *in_runlevel;
	bool export = false, escape = true;
	अक्षर sep = '\n';

	क्रम (पूर्णांक opt; (opt = getopt_long(argc, argv, getoptstring, longopts, शून्य)) != -1;) {
		चयन (opt) {
		हाल 'e': export = true; अवरोध;
		हाल 'n': escape = false; अवरोध;
		हाल '0': sep = '\0'; अवरोध;
		हाल 'r':
			अगर (!services)
				services = rc_stringlist_new();
			in_runlevel = rc_services_in_runlevel(optarg);
			TAILQ_CONCAT(services, in_runlevel, entries);
			rc_stringlist_free(in_runlevel);
			अवरोध;
		हाल 's':
			अगर (!services)
				services = rc_stringlist_new();
			rc_stringlist_addu(services, optarg);
			अवरोध;
		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;

	अगर (!argc)
		वापस print_environment(services, escape, export, sep);

	वापस set_environment(argv[0], &argv[1]);
}
