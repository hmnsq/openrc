#ضمن <einfo.h>
#ضمن <errno.h>
#ضمن <getopt.h>
#ضمن <rc.h>
#ضمن <queue.h>
#ضمن <string.h>

#ضمن "_usage.h"
#ضمن "helpers.h"

ثابت حرف *applet;
ثابت حرف *extraopts;
ثابت حرف *usagestring = "rc-environ [-s <service> ...] [-r <runlevel> ...] [-en0]\n";
ثابت حرف getoptstring[] = "ens:r:0" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "export",          no_argument, عدم, 'e' },
	{ "no-escape",       no_argument, عدم, 'n' },
	{ "null",            no_argument, عدم, '0' },
	{ "service",   required_argument, عدم, 's' },
	{ "runlevel",  required_argument, عدم, 'r' },
	longopts_COMMON
};

ثابت حرف *ثابت longopts_help[] = {
	"Prepend \"export\" to printed variables",
	"Do not perform shell sensitive escape on variable values",
	"End each output line with NUL, not newline",
	"Add service to the list of environments to print",
	"Add services in runlevel to the list of environments to print",
	longopts_help_COMMON
};

ساكن صحيح
set_environment(ثابت حرف *service, حرف *vars[])
{
	RC_DEPTREE *deptree = _rc_deptree_load(0, عدم);
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service, "reexport");
	RC_STRING *reexport;

	إذا (!(rc_service_state(service) & RC_SERVICE_STARTING)) {
		eerror("service %s is not in the starting state", service);
		أرجع 1;
	}

	لكل (ثابت حرف *var; (var = *vars); vars++) {
		size_t name_len = strcspn(var, "=");

		TAILQ_FOREACH(reexport, reexports, entries) {
			إذا (strncmp(reexport->قيمة, var, name_len) != 0)
				استمر;
			إذا (var[name_len] == '=')
				rc_service_putenv(service, var);
			وإلا
				rc_service_setenv(service, var, getenv(var));
			اذهب next;
		}

		ewarn("service %s does not reexport %.*s, skipping", service, (صحيح)name_len, var);
next:;
	}

	rc_stringlist_free(reexports);
	rc_deptree_free(deptree);
	أرجع 0;
}

ساكن فراغ escape_variable(ثابت حرف *قيمة) {
	حرف ch;

	printf("$'");
	لكل (ثابت حرف *p = قيمة; *p; p++) {
		بدل ((ch = *p)) {
		حالة '\a': ch = 'a'; اكسر;
		حالة '\b': ch = 'b'; اكسر;
		حالة '\f': ch = 'f'; اكسر;
		حالة '\n': ch = 'n'; اكسر;
		حالة '\r': ch = 'r'; اكسر;
		حالة '\t': ch = 't'; اكسر;
		حالة '\v': ch = 'v'; اكسر;
		/* \e is non-standard. */
		حالة 0x1b: ch = 'e'; اكسر;
		حالة '"':
		حالة '\'':
		حالة '\\':
			اكسر;
		افتراضي:
			putchar(ch);
			استمر;
		}
		printf("\\%c", ch);
	}
	printf("\'");
}

ساكن صحيح
اطبع_environment(RC_STRINGLIST *services, bool escape, bool export, حرف sep)
{
	/* from POSIX.2024 Shell Command Language 2.2, excluding `=` */
	ساكن ثابت حرف special_chars[] = "|&;<>()$`\\\"' \t\n*?[]^-!#~%{},";
	RC_DEPTREE *deptree = _rc_deptree_load(0, عدم);
	RC_STRING *service;
	حرف *قيمة = عدم;
	صحيح ret = 0;

	إذا (!services)
		services = rc_services_in_state(RC_SERVICE_STARTED);

	TAILQ_FOREACH(service, services, entries) {
		RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service->قيمة, "reexport");
		RC_STRING *reexport;

		TAILQ_FOREACH(reexport, reexports, entries) {
			إذا (rc_service_getenv(service->قيمة, reexport->قيمة, &قيمة) == -1) {
				ewarn("%s: failed to read variable '%s' for '%s': %s",
					applet, reexport->قيمة, service->قيمة, strerror(errno));
				ret = 1;
				استمر;
			}

			إذا (export)
				fputs("export ", stdout);
			printf("%s=", reexport->قيمة);
			إذا (escape && strpbrk(قيمة, special_chars))
				escape_variable(قيمة);
			وإلا
				fputs(قيمة, stdout);
			fputc(sep, stdout);
		}

		rc_stringlist_free(reexports);
	}

	حرر(قيمة);
	rc_deptree_free(deptree);
	أرجع ret;
}

صحيح main(صحيح argc, حرف **argv) {
	RC_STRINGLIST *services = عدم, *in_runlevel;
	bool export = false, escape = true;
	حرف sep = '\n';

	لكل (صحيح opt; (opt = getopt_long(argc, argv, getoptstring, longopts, عدم)) != -1;) {
		بدل (opt) {
		حالة 'e': export = true; اكسر;
		حالة 'n': escape = false; اكسر;
		حالة '0': sep = '\0'; اكسر;
		حالة 'r':
			إذا (!services)
				services = rc_stringlist_new();
			in_runlevel = rc_services_in_runlevel(optarg);
			TAILQ_CONCAT(services, in_runlevel, entries);
			rc_stringlist_free(in_runlevel);
			اكسر;
		حالة 's':
			إذا (!services)
				services = rc_stringlist_new();
			rc_stringlist_addu(services, optarg);
			اكسر;
		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;

	إذا (!argc)
		أرجع اطبع_environment(services, escape, export, sep);

	أرجع set_environment(argv[0], &argv[1]);
}
