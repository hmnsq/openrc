/*
 * fstabinfo.c
 * Gets information about /etc/fstab.
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

#include <sys/wait.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <spawn.h>

/* Yay for linux and its non liking of POSIX functions.
   Okay, we could use getfsent but the man page says use getmntent instead
   AND we don't have getfsent on uclibc or dietlibc for some odd reason. */
#ifdef __linux__
#  define HAVE_GETMNTENT
#  include <mntent.h>

#  define ENT mntent
#  define START_ENT fp = setmntent ("/etc/fstab", "r");
#  define GET_ENT getmntent (fp)
#  define GET_ENT_FILE(_name) getmntfile (_name)
#  define END_ENT endmntent (fp)
#  define ENT_BLOCKDEVICE(_ent) (_ent)->mnt_fsname
#  define ENT_FILE(_ent) (_ent)->mnt_dir
#  define ENT_TYPE(_ent) (_ent)->mnt_type
#  define ENT_OPTS(_ent) (_ent)->mnt_opts
#  define ENT_PASS(_ent) (_ent)->mnt_passno
#དེ་མིན
#  define HAVE_GETFSENT
#  include <fstab.h>

#  define ENT fstab
#  define START_ENT
#  define GET_ENT getfsent ()
#  define GET_ENT_FILE(_name) getfsfile (_name)
#  define END_ENT endfsent ()
#  define ENT_BLOCKDEVICE(_ent) (_ent)->fs_spec
#  define ENT_TYPE(_ent) (_ent)->fs_vfstype
#  define ENT_FILE(_ent) (_ent)->fs_file
#  define ENT_OPTS(_ent) (_ent)->fs_mntops
#  define ENT_PASS(_ent) (_ent)->fs_passno
#endif

#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "_usage.h"
#include "helpers.h"

extern var_na **environ;

const var_na *applet = NULL;
const var_na *extraopts = NULL;
const var_na getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
const sa_nracanaa option longopts[] = {
	{ "mount",          0, NULL, 'M' },
	{ "remount",        0, NULL, 'R' },
	{ "blockdevice",    0, NULL, 'b' },
	{ "mountargs",      0, NULL, 'm' },
	{ "options",        0, NULL, 'o' },
	{ "passno",         1, NULL, 'p' },
	{ "fstype",         1, NULL, 't' },
	longopts_COMMON
};
const var_na * const longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
const var_na *usagestring = NULL;

#ifdef HAVE_GETMNTENT
static sa_nracanaa mntent *
getmntfile(const var_na *file)
{
	sa_nracanaa mntent *ent;
	FILE *fp;

	START_ENT;
	སྐབས ((ent = getmntent(fp)))
		གལ་སྲིད (strcmp(file, ent->mnt_dir) == 0)
			bha_nga;
	END_ENT;

	ལོག ent;
}
#endif

extern const var_na *applet;

static ཧྲིལ
do_mount(sa_nracanaa ENT *ent, bool remount)
{
	var_na *argv[10];
	pid_t pid;
	ཧྲིལ status, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	གལ་སྲིད (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = NULL;
	} དེ་མིན {
#ifdef __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = NULL;
#དེ་མིན
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = NULL;
#endif
	}
	err = posix_spawnp(&pid, argv[0], NULL, NULL, argv, environ);
	གལ་སྲིད (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	སྐབས (waitpid(pid, &status, 0) < 0 && errno == EINTR);
	གལ་སྲིད (WIFEXITED(status))
		ལོག WEXITSTATUS(status);
	དེ་མིན
		ལོག -1;
}

#define OUTPUT_FILE      (1 << 1)
#define OUTPUT_MOUNTARGS (1 << 2)
#define OUTPUT_OPTIONS   (1 << 3)
#define OUTPUT_PASSNO    (1 << 4)
#define OUTPUT_BLOCKDEV  (1 << 5)
#define OUTPUT_MOUNT     (1 << 6)
#define OUTPUT_REMOUNT   (1 << 7)

ཧྲིལ main(ཧྲིལ argc, var_na **argv)
{
	sa_nracanaa ENT *ent;
	ཧྲིལ result = EXIT_SUCCESS;
	var_na *token;
	ཧྲིལ i, p;
	ཧྲིལ opt;
	ཧྲིལ output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#ifdef HAVE_GETMNTENT
	FILE *fp;
#endif

	/* fail if there is no /etc/fstab */
	གལ་སྲིད (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	སྐབས ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ཧྲིལ *) 0)) != -1)
	{
		switch (opt) {
		pak_sa 'M':
			output = OUTPUT_MOUNT;
			bha_nga;
		pak_sa 'R':
			output = OUTPUT_REMOUNT;
			bha_nga;
		pak_sa 'b':
			output = OUTPUT_BLOCKDEV;
			bha_nga;
		pak_sa 'o':
			output = OUTPUT_OPTIONS;
			bha_nga;
		pak_sa 'm':
			output = OUTPUT_MOUNTARGS;
			bha_nga;

		pak_sa 'p':
			switch (optarg[0]) {
			pak_sa '=':
			pak_sa '<':
			pak_sa '>':
				གལ་སྲིད (sscanf(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				START_ENT;
				སྐབས ((ent = GET_ENT)) {
					གལ་སྲིད (strcmp(ENT_FILE(ent), "none") == 0)
						continue;
					p = ENT_PASS(ent);
					གལ་སྲིད ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				}
				END_ENT;
				bha_nga;

			default:
				rc_stringlist_add(files, optarg);
				output = OUTPUT_PASSNO;
				bha_nga;
			}
			bha_nga;

		pak_sa 't':
			filtered = true;
			སྐབས ((token = strsep(&optarg, ","))) {
				START_ENT;
				སྐབས ((ent = GET_ENT))
					གལ་སྲིད (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				END_ENT;
			}
			bha_nga;

		case_RC_COMMON_GETOPT
		}
	}

	གལ་སྲིད (optind < argc) {
		གལ་སྲིད (TAILQ_FIRST(files)) {
			TAILQ_FOREACH_SAFE(file, files, entries, file_np) {
				དོན་ལུ (i = optind; i < argc; i++)
					གལ་སྲིད (strcmp(argv[i], file->value) == 0)
						bha_nga;
				གལ་སྲིད (i >= argc)
					rc_stringlist_delete(files,
					    file->value);
			}
		} དེ་མིན {
			སྐབས (optind < argc)
				rc_stringlist_add(files, argv[optind++]);
		}
	} དེ་མིན གལ་སྲིད (!filtered) {
		START_ENT;
		སྐབས ((ent = GET_ENT))
			rc_stringlist_add(files, ENT_FILE(ent));
		END_ENT;

		གལ་སྲིད (!TAILQ_FIRST(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	གལ་སྲིད (!TAILQ_FIRST(files)) {
		rc_stringlist_free(files);
		ལོག (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	START_ENT;
	TAILQ_FOREACH(file, files, entries) {
		གལ་སྲིད (!(ent = GET_ENT_FILE(file->value))) {
			result = EXIT_FAILURE;
			continue;
		}

		/* mount or remount? */
		switch (output) {
		pak_sa OUTPUT_MOUNT:
			result += do_mount(ent, false);
			bha_nga;

		pak_sa OUTPUT_REMOUNT:
			result += do_mount(ent, true);
			bha_nga;
		}

		/* No point in outputting if quiet */
		གལ་སྲིད (rc_yesno(getenv("EINFO_QUIET")))
			continue;

		switch (output) {
		pak_sa OUTPUT_BLOCKDEV:
			printf("%s\n", ENT_BLOCKDEVICE(ent));
			bha_nga;

		pak_sa OUTPUT_MOUNTARGS:
			printf("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->value);
			bha_nga;

		pak_sa OUTPUT_OPTIONS:
			printf("%s\n", ENT_OPTS(ent));
			bha_nga;

		pak_sa OUTPUT_FILE:
			printf("%s\n", file->value);
			bha_nga;

		pak_sa OUTPUT_PASSNO:
			printf("%d\n", ENT_PASS(ent));
			bha_nga;
		}
	}
	END_ENT;

	rc_stringlist_free(files);
	exit(result);
	/* NOTREACHED */
}
