#ਅਨ੍ਤਰ੍ਭਾਵ <einfo.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <rc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <queue.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਅਚਲ ਅੱਖਰ *applet;
ਅਚਲ ਅੱਖਰ *extraopts;
ਅਚਲ ਅੱਖਰ *usagestring = "rc-environ [-s <service> ...] [-r <runlevel> ...] [-en0]\n";
ਅਚਲ ਅੱਖਰ getoptstring[] = "ens:r:0" getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "export",          no_argument, ਨਿਰਾ, 'e' },
	{ "no-escape",       no_argument, ਨਿਰਾ, 'n' },
	{ "null",            no_argument, ਨਿਰਾ, '0' },
	{ "service",   required_argument, ਨਿਰਾ, 's' },
	{ "runlevel",  required_argument, ਨਿਰਾ, 'r' },
	longopts_COMMON
};

ਅਚਲ ਅੱਖਰ *ਅਚਲ longopts_help[] = {
	"Prepend \"export\" to printed variables",
	"Do not perform shell sensitive escape on variable values",
	"End each output line with NUL, not newline",
	"Add service to the list of environments to print",
	"Add services in runlevel to the list of environments to print",
	longopts_help_COMMON
};

ਸਥਿਰ ਪੂਰਨ
ਸੈੱਟ_environment(ਅਚਲ ਅੱਖਰ *service, ਅੱਖਰ *vars[])
{
	RC_DEPTREE *deptree = rc_deptree_ਲੱਦੋ_2(0, ਨਿਰਾ);
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service, "reexport");
	RC_STRING *reexport;

	ਜੇ (!(rc_service_state(service) & RC_SERVICE_STARTING)) {
		eerror("service %s is not in the starting state", service);
		ਵਾਪਸ 1;
	}

	ਲਈ (ਅਚਲ ਅੱਖਰ *var; (var = *vars); vars++) {
		size_t name_len = strcspn(var, "=");

		TAILQ_FOREACH(reexport, reexports, entries) {
			ਜੇ (ਲੜੀ_ਤੁਲਨਾ(reexport->ਮੁੱਲ, var, name_len) != 0)
				ਜਾਰੀ;
			ਜੇ (var[name_len] == '=')
				rc_service_putenv(service, var);
			ਨਹੀਂਤਾਂ
				rc_service_setenv(service, var, getenv(var));
			ਜਾਓ ਅਗਲਾ;
		}

		ewarn("service %s does not reexport %.*s, skipping", service, (ਪੂਰਨ)name_len, var);
ਅਗਲਾ:;
	}

	rc_stringlist_free(reexports);
	rc_deptree_free(deptree);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ escape_variable(ਅਚਲ ਅੱਖਰ *ਮੁੱਲ) {
	ਅੱਖਰ ch;

	ਲਿਖੋ("$'");
	ਲਈ (ਅਚਲ ਅੱਖਰ *p = ਮੁੱਲ; *p; p++) {
		ਚੋਣ ((ch = *p)) {
		ਹਾਲਤ '\a': ch = 'a'; ਰੋਕੋ;
		ਹਾਲਤ '\b': ch = 'b'; ਰੋਕੋ;
		ਹਾਲਤ '\f': ch = 'f'; ਰੋਕੋ;
		ਹਾਲਤ '\n': ch = 'n'; ਰੋਕੋ;
		ਹਾਲਤ '\r': ch = 'r'; ਰੋਕੋ;
		ਹਾਲਤ '\t': ch = 't'; ਰੋਕੋ;
		ਹਾਲਤ '\v': ch = 'v'; ਰੋਕੋ;
		/* \e is non-standard. */
		ਹਾਲਤ 0x1b: ch = 'e'; ਰੋਕੋ;
		ਹਾਲਤ '"':
		ਹਾਲਤ '\'':
		ਹਾਲਤ '\\':
			ਰੋਕੋ;
		ਮੂਲ:
			ਅੱਖਰ_ਲਿਖੋ(ch);
			ਜਾਰੀ;
		}
		ਲਿਖੋ("\\%c", ch);
	}
	ਲਿਖੋ("\'");
}

ਸਥਿਰ ਪੂਰਨ
print_environment(RC_STRINGLIST *services, bool escape, bool export, ਅੱਖਰ sep)
{
	/* from POSIX.2024 Shell Command Language 2.2, excluding `=` */
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ special_chars[] = "|&;<>()$`\\\"' \t\n*?[]^-!#~%{},";
	RC_DEPTREE *deptree = rc_deptree_ਲੱਦੋ_2(0, ਨਿਰਾ);
	RC_STRING *service;
	ਅੱਖਰ *ਮੁੱਲ = ਨਿਰਾ;
	ਪੂਰਨ ret = 0;

	ਜੇ (!services)
		services = rc_services_ਵਿੱਚ_state(RC_SERVICE_STARTED);

	TAILQ_FOREACH(service, services, entries) {
		RC_STRINGLIST *reexports = rc_deptree_depend(deptree, service->ਮੁੱਲ, "reexport");
		RC_STRING *reexport;

		TAILQ_FOREACH(reexport, reexports, entries) {
			ਜੇ (rc_service_getenv(service->ਮੁੱਲ, reexport->ਮੁੱਲ, &ਮੁੱਲ) == -1) {
				ewarn("%s: failed to read variable '%s' for '%s': %s",
					applet, reexport->ਮੁੱਲ, service->ਮੁੱਲ, strerror(errno));
				ret = 1;
				ਜਾਰੀ;
			}

			ਜੇ (export)
				fputs("export ", stdout);
			ਲਿਖੋ("%s=", reexport->ਮੁੱਲ);
			ਜੇ (escape && strpbrk(ਮੁੱਲ, special_chars))
				escape_variable(ਮੁੱਲ);
			ਨਹੀਂਤਾਂ
				fputs(ਮੁੱਲ, stdout);
			fputc(sep, stdout);
		}

		rc_stringlist_free(reexports);
	}

	ਮੁਕਤ(ਮੁੱਲ);
	rc_deptree_free(deptree);
	ਵਾਪਸ ret;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv) {
	RC_STRINGLIST *services = ਨਿਰਾ, *ਵਿੱਚ_runlevel;
	bool export = false, escape = true;
	ਅੱਖਰ sep = '\n';

	ਲਈ (ਪੂਰਨ opt; (opt = getopt_long(argc, argv, getoptstring, longopts, ਨਿਰਾ)) != -1;) {
		ਚੋਣ (opt) {
		ਹਾਲਤ 'e': export = true; ਰੋਕੋ;
		ਹਾਲਤ 'n': escape = false; ਰੋਕੋ;
		ਹਾਲਤ '0': sep = '\0'; ਰੋਕੋ;
		ਹਾਲਤ 'r':
			ਜੇ (!services)
				services = rc_stringlist_new();
			ਵਿੱਚ_runlevel = rc_services_ਵਿੱਚ_runlevel(optarg);
			TAILQ_CONCAT(services, ਵਿੱਚ_runlevel, entries);
			rc_stringlist_free(ਵਿੱਚ_runlevel);
			ਰੋਕੋ;
		ਹਾਲਤ 's':
			ਜੇ (!services)
				services = rc_stringlist_new();
			rc_stringlist_addu(services, optarg);
			ਰੋਕੋ;
		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;

	ਜੇ (!argc)
		ਵਾਪਸ print_environment(services, escape, export, sep);

	ਵਾਪਸ ਸੈੱਟ_environment(argv[0], &argv[1]);
}
