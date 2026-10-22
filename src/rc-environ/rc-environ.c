#شامل <einfo.h>
#شامل <errno.h>
#شامل <getopt.h>
#شامل <rc.h>
#شامل <queue.h>
#شامل <string.h>

#شامل "_usage.h"
#شامل "helpers.h"

ثابت نویسه *applet;
ثابت نویسه *extraopts;
ثابت نویسه *usagestring = "rc-environ [-s <service> ...] [-r <runlevel> ...] [-en0]\n";
ثابت نویسه getoptstring[] = "ens:r:0" getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{ "export",          no_argument, پوچ, 'e' },
	{ "no-escape",       no_argument, پوچ, 'n' },
	{ "null",            no_argument, پوچ, '0' },
	{ "service",   required_argument, پوچ, 's' },
	{ "runlevel",  required_argument, پوچ, 'r' },
	longopts_COMMON
};

ثابت نویسه *ثابت longopts_help[] = {
	"Prepend \"export\" to printed variables",
	"Do not perform shell sensitive escape on variable values",
	"End each output line with NUL, not newline",
	"Add service to the list of environments to print",
	"Add services in runlevel to the list of environments to print",
	longopts_help_COMMON
};

ایستا صحیح
set_environment(ثابت نویسه *service, نویسه *vars[])
{
	RC_DEPTREE *deptree = _rc_deptree_load(0, پوچ);
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service, "reexport");
	RC_STRING *reexport;

	اگر (!(rc_service_state(service) & RC_SERVICE_STARTING)) {
		eerror("service %s is not in the starting state", service);
		بازگشت 1;
	}

	برای (ثابت نویسه *var; (var = *vars); vars++) {
		size_t name_len = strcspn(var, "=");

		TAILQ_FOREACH(reexport, reexports, entries) {
			اگر (strncmp(reexport->مقدار, var, name_len) != 0)
				ادامه;
			اگر (var[name_len] == '=')
				rc_service_putenv(service, var);
			وگرنه
				rc_service_setenv(service, var, getenv(var));
			برو next;
		}

		ewarn("service %s does not reexport %.*s, skipping", service, (صحیح)name_len, var);
next:;
	}

	rc_stringlist_free(reexports);
	rc_deptree_free(deptree);
	بازگشت 0;
}

ایستا تهی escape_variable(ثابت نویسه *مقدار) {
	نویسه ch;

	printf("$'");
	برای (ثابت نویسه *p = مقدار; *p; p++) {
		انتخاب ((ch = *p)) {
		حالت '\a': ch = 'a'; بشکن;
		حالت '\b': ch = 'b'; بشکن;
		حالت '\f': ch = 'f'; بشکن;
		حالت '\n': ch = 'n'; بشکن;
		حالت '\r': ch = 'r'; بشکن;
		حالت '\t': ch = 't'; بشکن;
		حالت '\v': ch = 'v'; بشکن;
		/* \e is non-standard. */
		حالت 0x1b: ch = 'e'; بشکن;
		حالت '"':
		حالت '\'':
		حالت '\\':
			بشکن;
		پیشفرض:
			putchar(ch);
			ادامه;
		}
		printf("\\%c", ch);
	}
	printf("\'");
}

ایستا صحیح
print_environment(RC_STRINGLIST *services, bool escape, bool export, نویسه sep)
{
	/* from POSIX.2024 Shell Command Language 2.2, excluding `=` */
	ایستا ثابت نویسه special_chars[] = "|&;<>()$`\\\"' \t\n*?[]^-!#~%{},";
	RC_DEPTREE *deptree = _rc_deptree_load(0, پوچ);
	RC_STRING *service;
	نویسه *مقدار = پوچ;
	صحیح ret = 0;

	اگر (!services)
		services = rc_services_in_state(RC_SERVICE_STARTED);

	TAILQ_FOREACH(service, services, entries) {
		RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service->مقدار, "reexport");
		RC_STRING *reexport;

		TAILQ_FOREACH(reexport, reexports, entries) {
			اگر (rc_service_getenv(service->مقدار, reexport->مقدار, &مقدار) == -1) {
				ewarn("%s: failed to read variable '%s' for '%s': %s",
					applet, reexport->مقدار, service->مقدار, strerror(errno));
				ret = 1;
				ادامه;
			}

			اگر (export)
				fputs("export ", stdout);
			printf("%s=", reexport->مقدار);
			اگر (escape && strpbrk(مقدار, special_chars))
				escape_variable(مقدار);
			وگرنه
				fputs(مقدار, stdout);
			fputc(sep, stdout);
		}

		rc_stringlist_free(reexports);
	}

	آزاد(مقدار);
	rc_deptree_free(deptree);
	بازگشت ret;
}

صحیح اصلی(صحیح argc, نویسه **argv) {
	RC_STRINGLIST *services = پوچ, *in_runlevel;
	bool export = false, escape = true;
	نویسه sep = '\n';

	برای (صحیح opt; (opt = getopt_long(argc, argv, getoptstring, longopts, پوچ)) != -1;) {
		انتخاب (opt) {
		حالت 'e': export = true; بشکن;
		حالت 'n': escape = false; بشکن;
		حالت '0': sep = '\0'; بشکن;
		حالت 'r':
			اگر (!services)
				services = rc_stringlist_new();
			in_runlevel = rc_services_in_runlevel(optarg);
			TAILQ_CONCAT(services, in_runlevel, entries);
			rc_stringlist_free(in_runlevel);
			بشکن;
		حالت 's':
			اگر (!services)
				services = rc_stringlist_new();
			rc_stringlist_addu(services, optarg);
			بشکن;
		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;

	اگر (!argc)
		بازگشت print_environment(services, escape, export, sep);

	بازگشت set_environment(argv[0], &argv[1]);
}
