#ଅନ୍ତର୍ଭାଵ <einfo.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <rc.h>
#ଅନ୍ତର୍ଭାଵ <queue.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ଅଚଳ ଅକ୍ଷର *applet;
ଅଚଳ ଅକ୍ଷର *extraopts;
ଅଚଳ ଅକ୍ଷର *usagestring = "rc-environ [-s <service> ...] [-r <runlevel> ...] [-en0]\n";
ଅଚଳ ଅକ୍ଷର getoptstring[] = "ens:r:0" getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "export",          no_argument, ଖାଲି, 'e' },
	{ "no-escape",       no_argument, ଖାଲି, 'n' },
	{ "null",            no_argument, ଖାଲି, '0' },
	{ "service",   required_argument, ଖାଲି, 's' },
	{ "runlevel",  required_argument, ଖାଲି, 'r' },
	longopts_COMMON
};

ଅଚଳ ଅକ୍ଷର *ଅଚଳ longopts_help[] = {
	"Prepend \"export\" to printed variables",
	"Do not perform shell sensitive escape on variable values",
	"End each output line with NUL, not newline",
	"Add service to the list of environments to print",
	"Add services in runlevel to the list of environments to print",
	longopts_help_COMMON
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
set_environment(ଅଚଳ ଅକ୍ଷର *service, ଅକ୍ଷର *vars[])
{
	RC_DEPTREE *deptree = _rc_deptree_load(0, ଖାଲି);
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service, "reexport");
	RC_STRING *reexport;

	ଯଦି (!(rc_service_state(service) & RC_SERVICE_STARTING)) {
		eerror("service %s is not in the starting state", service);
		ଫେରନ୍ତୁ 1;
	}

	ପାଇଁ (ଅଚଳ ଅକ୍ଷର *var; (var = *vars); vars++) {
		size_t name_len = strcspn(var, "=");

		TAILQ_FOREACH(reexport, reexports, entries) {
			ଯଦି (strncmp(reexport->ମୂଲ୍ୟ, var, name_len) != 0)
				ଜାରି;
			ଯଦି (var[name_len] == '=')
				rc_service_putenv(service, var);
			ଅନ୍ୟଥା
				rc_service_setenv(service, var, getenv(var));
			ଯାଅ next;
		}

		ewarn("service %s does not reexport %.*s, skipping", service, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)name_len, var);
next:;
	}

	rc_stringlist_free(reexports);
	rc_deptree_free(deptree);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ escape_variable(ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ) {
	ଅକ୍ଷର ch;

	printf("$'");
	ପାଇଁ (ଅଚଳ ଅକ୍ଷର *p = ମୂଲ୍ୟ; *p; p++) {
		ବାଛ ((ch = *p)) {
		ସ୍ଥିତି '\a': ch = 'a'; ଭାଙ୍ଗ;
		ସ୍ଥିତି '\b': ch = 'b'; ଭାଙ୍ଗ;
		ସ୍ଥିତି '\f': ch = 'f'; ଭାଙ୍ଗ;
		ସ୍ଥିତି '\n': ch = 'n'; ଭାଙ୍ଗ;
		ସ୍ଥିତି '\r': ch = 'r'; ଭାଙ୍ଗ;
		ସ୍ଥିତି '\t': ch = 't'; ଭାଙ୍ଗ;
		ସ୍ଥିତି '\v': ch = 'v'; ଭାଙ୍ଗ;
		/* \e is non-standard. */
		ସ୍ଥିତି 0x1b: ch = 'e'; ଭାଙ୍ଗ;
		ସ୍ଥିତି '"':
		ସ୍ଥିତି '\'':
		ସ୍ଥିତି '\\':
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			putchar(ch);
			ଜାରି;
		}
		printf("\\%c", ch);
	}
	printf("\'");
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
print_environment(RC_STRINGLIST *services, bool escape, bool export, ଅକ୍ଷର sep)
{
	/* from POSIX.2024 Shell Command Language 2.2, excluding `=` */
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର special_chars[] = "|&;<>()$`\\\"' \t\n*?[]^-!#~%{},";
	RC_DEPTREE *deptree = _rc_deptree_load(0, ଖାଲି);
	RC_STRING *service;
	ଅକ୍ଷର *ମୂଲ୍ୟ = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	ଯଦି (!services)
		services = rc_services_in_state(RC_SERVICE_STARTED);

	TAILQ_FOREACH(service, services, entries) {
		RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service->ମୂଲ୍ୟ, "reexport");
		RC_STRING *reexport;

		TAILQ_FOREACH(reexport, reexports, entries) {
			ଯଦି (rc_service_getenv(service->ମୂଲ୍ୟ, reexport->ମୂଲ୍ୟ, &ମୂଲ୍ୟ) == -1) {
				ewarn("%s: failed to read variable '%s' for '%s': %s",
					applet, reexport->ମୂଲ୍ୟ, service->ମୂଲ୍ୟ, strerror(errno));
				ret = 1;
				ଜାରି;
			}

			ଯଦି (export)
				fputs("export ", stdout);
			printf("%s=", reexport->ମୂଲ୍ୟ);
			ଯଦି (escape && strpbrk(ମୂଲ୍ୟ, special_chars))
				escape_variable(ମୂଲ୍ୟ);
			ଅନ୍ୟଥା
				fputs(ମୂଲ୍ୟ, stdout);
			fputc(sep, stdout);
		}

		rc_stringlist_free(reexports);
	}

	ମୁକ୍ତ(ମୂଲ୍ୟ);
	rc_deptree_free(deptree);
	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) {
	RC_STRINGLIST *services = ଖାଲି, *in_runlevel;
	bool export = false, escape = true;
	ଅକ୍ଷର sep = '\n';

	ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt; (opt = getopt_long(argc, argv, getoptstring, longopts, ଖାଲି)) != -1;) {
		ବାଛ (opt) {
		ସ୍ଥିତି 'e': export = true; ଭାଙ୍ଗ;
		ସ୍ଥିତି 'n': escape = false; ଭାଙ୍ଗ;
		ସ୍ଥିତି '0': sep = '\0'; ଭାଙ୍ଗ;
		ସ୍ଥିତି 'r':
			ଯଦି (!services)
				services = rc_stringlist_new();
			in_runlevel = rc_services_in_runlevel(optarg);
			TAILQ_CONCAT(services, in_runlevel, entries);
			rc_stringlist_free(in_runlevel);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 's':
			ଯଦି (!services)
				services = rc_stringlist_new();
			rc_stringlist_addu(services, optarg);
			ଭାଙ୍ଗ;
		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;

	ଯଦି (!argc)
		ଫେରନ୍ତୁ print_environment(services, escape, export, sep);

	ଫେରନ୍ତୁ set_environment(argv[0], &argv[1]);
}
