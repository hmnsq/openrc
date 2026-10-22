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
#інакше
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

extern char **environ;

const char *applet = NULL;
const char *extraopts = NULL;
const char getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
const struct option longopts[] = {
	{ "mount",          0, NULL, 'M' },
	{ "remount",        0, NULL, 'R' },
	{ "blockdevice",    0, NULL, 'b' },
	{ "mountargs",      0, NULL, 'm' },
	{ "options",        0, NULL, 'o' },
	{ "passno",         1, NULL, 'p' },
	{ "fstype",         1, NULL, 't' },
	longopts_COMMON
};
const char * const longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
const char *usagestring = NULL;

#ifdef HAVE_GETMNTENT
static struct mntent *
getmntfile(const char *file)
{
	struct mntent *ent;
	FILE *fp;

	START_ENT;
	поки ((ent = getmntent(fp)))
		якщо (strcmp(file, ent->mnt_dir) == 0)
			break;
	END_ENT;

	повернути ent;
}
#endif

extern const char *applet;

static ціле
do_mount(struct ENT *ent, bool remount)
{
	char *argv[10];
	pid_t pid;
	ціле status, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	якщо (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = NULL;
	} інакше {
#ifdef __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = NULL;
#інакше
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = NULL;
#endif
	}
	err = posix_spawnp(&pid, argv[0], NULL, NULL, argv, environ);
	якщо (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	поки (waitpid(pid, &status, 0) < 0 && errno == EINTR);
	якщо (WIFEXITED(status))
		повернути WEXITSTATUS(status);
	інакше
		повернути -1;
}

#define OUTPUT_FILE      (1 << 1)
#define OUTPUT_MOUNTARGS (1 << 2)
#define OUTPUT_OPTIONS   (1 << 3)
#define OUTPUT_PASSNO    (1 << 4)
#define OUTPUT_BLOCKDEV  (1 << 5)
#define OUTPUT_MOUNT     (1 << 6)
#define OUTPUT_REMOUNT   (1 << 7)

ціле main(ціле argc, char **argv)
{
	struct ENT *ent;
	ціле result = EXIT_SUCCESS;
	char *token;
	ціле i, p;
	ціле opt;
	ціле output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#ifdef HAVE_GETMNTENT
	FILE *fp;
#endif

	/* fail if there is no /etc/fstab */
	якщо (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	поки ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ціле *) 0)) != -1)
	{
		switch (opt) {
		case 'M':
			output = OUTPUT_MOUNT;
			break;
		case 'R':
			output = OUTPUT_REMOUNT;
			break;
		case 'b':
			output = OUTPUT_BLOCKDEV;
			break;
		case 'o':
			output = OUTPUT_OPTIONS;
			break;
		case 'm':
			output = OUTPUT_MOUNTARGS;
			break;

		case 'p':
			switch (optarg[0]) {
			case '=':
			case '<':
			case '>':
				якщо (sscanf(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				START_ENT;
				поки ((ent = GET_ENT)) {
					якщо (strcmp(ENT_FILE(ent), "none") == 0)
						continue;
					p = ENT_PASS(ent);
					якщо ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				}
				END_ENT;
				break;

			default:
				rc_stringlist_add(files, optarg);
				output = OUTPUT_PASSNO;
				break;
			}
			break;

		case 't':
			filtered = true;
			поки ((token = strsep(&optarg, ","))) {
				START_ENT;
				поки ((ent = GET_ENT))
					якщо (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				END_ENT;
			}
			break;

		case_RC_COMMON_GETOPT
		}
	}

	якщо (optind < argc) {
		якщо (TAILQ_FIRST(files)) {
			TAILQ_FOREACH_SAFE(file, files, entries, file_np) {
				для (i = optind; i < argc; i++)
					якщо (strcmp(argv[i], file->value) == 0)
						break;
				якщо (i >= argc)
					rc_stringlist_delete(files,
					    file->value);
			}
		} інакше {
			поки (optind < argc)
				rc_stringlist_add(files, argv[optind++]);
		}
	} інакше якщо (!filtered) {
		START_ENT;
		поки ((ent = GET_ENT))
			rc_stringlist_add(files, ENT_FILE(ent));
		END_ENT;

		якщо (!TAILQ_FIRST(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	якщо (!TAILQ_FIRST(files)) {
		rc_stringlist_free(files);
		повернути (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	START_ENT;
	TAILQ_FOREACH(file, files, entries) {
		якщо (!(ent = GET_ENT_FILE(file->value))) {
			result = EXIT_FAILURE;
			continue;
		}

		/* mount or remount? */
		switch (output) {
		case OUTPUT_MOUNT:
			result += do_mount(ent, false);
			break;

		case OUTPUT_REMOUNT:
			result += do_mount(ent, true);
			break;
		}

		/* No point in outputting if quiet */
		якщо (rc_yesno(getenv("EINFO_QUIET")))
			continue;

		switch (output) {
		case OUTPUT_BLOCKDEV:
			printf("%s\n", ENT_BLOCKDEVICE(ent));
			break;

		case OUTPUT_MOUNTARGS:
			printf("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->value);
			break;

		case OUTPUT_OPTIONS:
			printf("%s\n", ENT_OPTS(ent));
			break;

		case OUTPUT_FILE:
			printf("%s\n", file->value);
			break;

		case OUTPUT_PASSNO:
			printf("%d\n", ENT_PASS(ent));
			break;
		}
	}
	END_ENT;

	rc_stringlist_free(files);
	exit(result);
	/* NOTREACHED */
}
