#include <einfo.h>
#include <errno.h>
#include <getopt.h>
#include <rc.h>
#include <queue.h>
#include <string.h>

#include "_usage.h"
#include "helpers.h"

const char *applet;
const char *extraopts;
const char *usagestring = "rc-environ [-s <service> ...] [-r <runlevel> ...] [-en0]\n";
const char getoptstring[] = "ens:r:0" getoptstring_COMMON;
const struct option longopts[] = {
	{ "export",          no_argument, NULL, 'e' },
	{ "no-escape",       no_argument, NULL, 'n' },
	{ "null",            no_argument, NULL, '0' },
	{ "service",   required_argument, NULL, 's' },
	{ "runlevel",  required_argument, NULL, 'r' },
	longopts_COMMON
};

const char *const longopts_help[] = {
	"Prepend \"export\" to printed variables",
	"Do not perform shell sensitive escape on variable values",
	"End each output line with NUL, not newline",
	"Add service to the list of environments to print",
	"Add services in runlevel to the list of environments to print",
	longopts_help_COMMON
};

static 정수
set_environment(const char *service, char *vars[])
{
	RC_DEPTREE *deptree = _rc_deptree_load(0, NULL);
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service, "reexport");
	RC_STRING *reexport;

	만약 (!(rc_service_state(service) & RC_SERVICE_STARTING)) {
		eerror("service %s is not in the starting state", service);
		반환 1;
	}

	위해 (const char *var; (var = *vars); vars++) {
		size_t name_len = strcspn(var, "=");

		TAILQ_FOREACH(reexport, reexports, entries) {
			만약 (strncmp(reexport->value, var, name_len) != 0)
				continue;
			만약 (var[name_len] == '=')
				rc_service_putenv(service, var);
			아니면
				rc_service_setenv(service, var, getenv(var));
			goto next;
		}

		ewarn("service %s does not reexport %.*s, skipping", service, (정수)name_len, var);
next:;
	}

	rc_stringlist_free(reexports);
	rc_deptree_free(deptree);
	반환 0;
}

static void escape_variable(const char *value) {
	char ch;

	printf("$'");
	위해 (const char *p = value; *p; p++) {
		switch ((ch = *p)) {
		case '\a': ch = 'a'; break;
		case '\b': ch = 'b'; break;
		case '\f': ch = 'f'; break;
		case '\n': ch = 'n'; break;
		case '\r': ch = 'r'; break;
		case '\t': ch = 't'; break;
		case '\v': ch = 'v'; break;
		/* \e is non-standard. */
		case 0x1b: ch = 'e'; break;
		case '"':
		case '\'':
		case '\\':
			break;
		default:
			putchar(ch);
			continue;
		}
		printf("\\%c", ch);
	}
	printf("\'");
}

static 정수
print_environment(RC_STRINGLIST *services, bool escape, bool export, char sep)
{
	/* from POSIX.2024 Shell Command Language 2.2, excluding `=` */
	static const char special_chars[] = "|&;<>()$`\\\"' \t\n*?[]^-!#~%{},";
	RC_DEPTREE *deptree = _rc_deptree_load(0, NULL);
	RC_STRING *service;
	char *value = NULL;
	정수 ret = 0;

	만약 (!services)
		services = rc_services_in_state(RC_SERVICE_STARTED);

	TAILQ_FOREACH(service, services, entries) {
		RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service->value, "reexport");
		RC_STRING *reexport;

		TAILQ_FOREACH(reexport, reexports, entries) {
			만약 (rc_service_getenv(service->value, reexport->value, &value) == -1) {
				ewarn("%s: failed to read variable '%s' for '%s': %s",
					applet, reexport->value, service->value, strerror(errno));
				ret = 1;
				continue;
			}

			만약 (export)
				fputs("export ", stdout);
			printf("%s=", reexport->value);
			만약 (escape && strpbrk(value, special_chars))
				escape_variable(value);
			아니면
				fputs(value, stdout);
			fputc(sep, stdout);
		}

		rc_stringlist_free(reexports);
	}

	free(value);
	rc_deptree_free(deptree);
	반환 ret;
}

정수 main(정수 argc, char **argv) {
	RC_STRINGLIST *services = NULL, *in_runlevel;
	bool export = false, escape = true;
	char sep = '\n';

	위해 (정수 opt; (opt = getopt_long(argc, argv, getoptstring, longopts, NULL)) != -1;) {
		switch (opt) {
		case 'e': export = true; break;
		case 'n': escape = false; break;
		case '0': sep = '\0'; break;
		case 'r':
			만약 (!services)
				services = rc_stringlist_new();
			in_runlevel = rc_services_in_runlevel(optarg);
			TAILQ_CONCAT(services, in_runlevel, entries);
			rc_stringlist_free(in_runlevel);
			break;
		case 's':
			만약 (!services)
				services = rc_stringlist_new();
			rc_stringlist_addu(services, optarg);
			break;
		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;

	만약 (!argc)
		반환 print_environment(services, escape, export, sep);

	반환 set_environment(argv[0], &argv[1]);
}
