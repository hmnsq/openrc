#include <einfo.h>
#include <errno.h>
#include <getopt.h>
#include <rc.h>
#include <queue.h>
#include <string.h>

#include "_usage.h"
#include "helpers.h"

const var_na *applet;
const var_na *extraopts;
const var_na *usagestring = "rc-environ [-s <service> ...] [-r <runlevel> ...] [-en0]\n";
const var_na getoptstring[] = "ens:r:0" getoptstring_COMMON;
const sa_nracanaa option longopts[] = {
	{ "export",          no_argument, NULL, 'e' },
	{ "no-escape",       no_argument, NULL, 'n' },
	{ "null",            no_argument, NULL, '0' },
	{ "service",   required_argument, NULL, 's' },
	{ "runlevel",  required_argument, NULL, 'r' },
	longopts_COMMON
};

const var_na *const longopts_help[] = {
	"Prepend \"export\" to printed variables",
	"Do not perform shell sensitive escape on variable values",
	"End each output line with NUL, not newline",
	"Add service to the list of environments to print",
	"Add services in runlevel to the list of environments to print",
	longopts_help_COMMON
};

static ຈຳນວນເຕັມ
set_environment(const var_na *service, var_na *vars[])
{
	RC_DEPTREE *deptree = _rc_deptree_load(0, NULL);
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service, "reexport");
	RC_STRING *reexport;

	ຖ້າ (!(rc_service_state(service) & RC_SERVICE_STARTING)) {
		eerror("service %s is not in the starting state", service);
		ສົ່ງຄືນ 1;
	}

	ສຳລັບ (const var_na *var; (var = *vars); vars++) {
		size_t name_len = strcspn(var, "=");

		TAILQ_FOREACH(reexport, reexports, entries) {
			ຖ້າ (strncmp(reexport->value, var, name_len) != 0)
				continue;
			ຖ້າ (var[name_len] == '=')
				rc_service_putenv(service, var);
			ບໍ່ດັ່ງນັ້ນ
				rc_service_setenv(service, var, getenv(var));
			goto next;
		}

		ewarn("service %s does not reexport %.*s, skipping", service, (ຈຳນວນເຕັມ)name_len, var);
next:;
	}

	rc_stringlist_free(reexports);
	rc_deptree_free(deptree);
	ສົ່ງຄືນ 0;
}

static void escape_variable(const var_na *value) {
	var_na ch;

	printf("$'");
	ສຳລັບ (const var_na *p = value; *p; p++) {
		switch ((ch = *p)) {
		pak_sa '\a': ch = 'a'; bha_nga;
		pak_sa '\b': ch = 'b'; bha_nga;
		pak_sa '\f': ch = 'f'; bha_nga;
		pak_sa '\n': ch = 'n'; bha_nga;
		pak_sa '\r': ch = 'r'; bha_nga;
		pak_sa '\t': ch = 't'; bha_nga;
		pak_sa '\v': ch = 'v'; bha_nga;
		/* \e is non-standard. */
		pak_sa 0x1b: ch = 'e'; bha_nga;
		pak_sa '"':
		pak_sa '\'':
		pak_sa '\\':
			bha_nga;
		default:
			putchar(ch);
			continue;
		}
		printf("\\%c", ch);
	}
	printf("\'");
}

static ຈຳນວນເຕັມ
print_environment(RC_STRINGLIST *services, bool escape, bool export, var_na sep)
{
	/* from POSIX.2024 Shell Command Language 2.2, excluding `=` */
	static const var_na special_chars[] = "|&;<>()$`\\\"' \t\n*?[]^-!#~%{},";
	RC_DEPTREE *deptree = _rc_deptree_load(0, NULL);
	RC_STRING *service;
	var_na *value = NULL;
	ຈຳນວນເຕັມ ret = 0;

	ຖ້າ (!services)
		services = rc_services_in_state(RC_SERVICE_STARTED);

	TAILQ_FOREACH(service, services, entries) {
		RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service->value, "reexport");
		RC_STRING *reexport;

		TAILQ_FOREACH(reexport, reexports, entries) {
			ຖ້າ (rc_service_getenv(service->value, reexport->value, &value) == -1) {
				ewarn("%s: failed to read variable '%s' for '%s': %s",
					applet, reexport->value, service->value, strerror(errno));
				ret = 1;
				continue;
			}

			ຖ້າ (export)
				fputs("export ", stdout);
			printf("%s=", reexport->value);
			ຖ້າ (escape && strpbrk(value, special_chars))
				escape_variable(value);
			ບໍ່ດັ່ງນັ້ນ
				fputs(value, stdout);
			fputc(sep, stdout);
		}

		rc_stringlist_free(reexports);
	}

	free(value);
	rc_deptree_free(deptree);
	ສົ່ງຄືນ ret;
}

ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na **argv) {
	RC_STRINGLIST *services = NULL, *in_runlevel;
	bool export = false, escape = true;
	var_na sep = '\n';

	ສຳລັບ (ຈຳນວນເຕັມ opt; (opt = getopt_long(argc, argv, getoptstring, longopts, NULL)) != -1;) {
		switch (opt) {
		pak_sa 'e': export = true; bha_nga;
		pak_sa 'n': escape = false; bha_nga;
		pak_sa '0': sep = '\0'; bha_nga;
		pak_sa 'r':
			ຖ້າ (!services)
				services = rc_stringlist_new();
			in_runlevel = rc_services_in_runlevel(optarg);
			TAILQ_CONCAT(services, in_runlevel, entries);
			rc_stringlist_free(in_runlevel);
			bha_nga;
		pak_sa 's':
			ຖ້າ (!services)
				services = rc_stringlist_new();
			rc_stringlist_addu(services, optarg);
			bha_nga;
		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;

	ຖ້າ (!argc)
		ສົ່ງຄືນ print_environment(services, escape, export, sep);

	ສົ່ງຄືນ set_environment(argv[0], &argv[1]);
}
