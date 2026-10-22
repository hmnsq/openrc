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

#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <spawn.h>

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
#ಅನ್ಯಥಾ
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

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಬಾಹ್ಯ ವರ್ಣ **environ;

ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *extraopts = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{ "mount",          0, ರಿಕ್ತ, 'M' },
	{ "remount",        0, ರಿಕ್ತ, 'R' },
	{ "blockdevice",    0, ರಿಕ್ತ, 'b' },
	{ "mountargs",      0, ರಿಕ್ತ, 'm' },
	{ "options",        0, ರಿಕ್ತ, 'o' },
	{ "passno",         1, ರಿಕ್ತ, 'p' },
	{ "fstype",         1, ರಿಕ್ತ, 't' },
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
ಧ್ರುವ ವರ್ಣ *usagestring = ರಿಕ್ತ;

#ifdef HAVE_GETMNTENT
ಸ್ಥಿರ ಸಂರಚನಾ mntent *
getmntfile(ಧ್ರುವ ವರ್ಣ *file)
{
	ಸಂರಚನಾ mntent *ent;
	FILE *fp;

	START_ENT;
	ಯಾವತ್ ((ent = getmntent(fp)))
		ಯದಿ (strcmp(file, ent->mnt_dir) == 0)
			ಭಙ್ಗ;
	END_ENT;

	ಪ್ರತ್ಯಾಗಮ ent;
}
#endif

ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *applet;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
do_mount(ಸಂರಚನಾ ENT *ent, bool remount)
{
	ವರ್ಣ *argv[10];
	pid_t pid;
	ಪೂರ್ಣಾಙ್ಕ status, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	ಯದಿ (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = ರಿಕ್ತ;
	} ಅನ್ಯಥಾ {
#ifdef __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = ರಿಕ್ತ;
#ಅನ್ಯಥಾ
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = ರಿಕ್ತ;
#endif
	}
	err = posix_spawnp(&pid, argv[0], ರಿಕ್ತ, ರಿಕ್ತ, argv, environ);
	ಯದಿ (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	ಯಾವತ್ (waitpid(pid, &status, 0) < 0 && errno == EINTR);
	ಯದಿ (WIFEXITED(status))
		ಪ್ರತ್ಯಾಗಮ WEXITSTATUS(status);
	ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ -1;
}

#define OUTPUT_FILE      (1 << 1)
#define OUTPUT_MOUNTARGS (1 << 2)
#define OUTPUT_OPTIONS   (1 << 3)
#define OUTPUT_PASSNO    (1 << 4)
#define OUTPUT_BLOCKDEV  (1 << 5)
#define OUTPUT_MOUNT     (1 << 6)
#define OUTPUT_REMOUNT   (1 << 7)

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಸಂರಚನಾ ENT *ent;
	ಪೂರ್ಣಾಙ್ಕ result = EXIT_SUCCESS;
	ವರ್ಣ *token;
	ಪೂರ್ಣಾಙ್ಕ i, p;
	ಪೂರ್ಣಾಙ್ಕ opt;
	ಪೂರ್ಣಾಙ್ಕ output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#ifdef HAVE_GETMNTENT
	FILE *fp;
#endif

	/* fail if there is no /etc/fstab */
	ಯದಿ (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಙ್ಕ *) 0)) != -1)
	{
		ವಿಕಲ್ಪ (opt) {
		ಪಕ್ಷ 'M':
			output = OUTPUT_MOUNT;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'R':
			output = OUTPUT_REMOUNT;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'b':
			output = OUTPUT_BLOCKDEV;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'o':
			output = OUTPUT_OPTIONS;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'm':
			output = OUTPUT_MOUNTARGS;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'p':
			ವಿಕಲ್ಪ (optarg[0]) {
			ಪಕ್ಷ '=':
			ಪಕ್ಷ '<':
			ಪಕ್ಷ '>':
				ಯದಿ (sscanf(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				START_ENT;
				ಯಾವತ್ ((ent = GET_ENT)) {
					ಯದಿ (strcmp(ENT_FILE(ent), "none") == 0)
						ಅನುವರ್ತ;
					p = ENT_PASS(ent);
					ಯದಿ ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				}
				END_ENT;
				ಭಙ್ಗ;

			ಸ್ವಭಾವ:
				rc_stringlist_add(files, optarg);
				output = OUTPUT_PASSNO;
				ಭಙ್ಗ;
			}
			ಭಙ್ಗ;

		ಪಕ್ಷ 't':
			filtered = true;
			ಯಾವತ್ ((token = strsep(&optarg, ","))) {
				START_ENT;
				ಯಾವತ್ ((ent = GET_ENT))
					ಯದಿ (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				END_ENT;
			}
			ಭಙ್ಗ;

		case_RC_COMMON_GETOPT
		}
	}

	ಯದಿ (optind < argc) {
		ಯದಿ (TAILQ_FIRST(files)) {
			TAILQ_FOREACH_SAFE(file, files, entries, file_np) {
				ಪ್ರತಿ (i = optind; i < argc; i++)
					ಯದಿ (strcmp(argv[i], file->ಮೌಲ್ಯ) == 0)
						ಭಙ್ಗ;
				ಯದಿ (i >= argc)
					rc_stringlist_delete(files,
					    file->ಮೌಲ್ಯ);
			}
		} ಅನ್ಯಥಾ {
			ಯಾವತ್ (optind < argc)
				rc_stringlist_add(files, argv[optind++]);
		}
	} ಅನ್ಯಥಾ ಯದಿ (!filtered) {
		START_ENT;
		ಯಾವತ್ ((ent = GET_ENT))
			rc_stringlist_add(files, ENT_FILE(ent));
		END_ENT;

		ಯದಿ (!TAILQ_FIRST(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	ಯದಿ (!TAILQ_FIRST(files)) {
		rc_stringlist_free(files);
		ಪ್ರತ್ಯಾಗಮ (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	START_ENT;
	TAILQ_FOREACH(file, files, entries) {
		ಯದಿ (!(ent = GET_ENT_FILE(file->ಮೌಲ್ಯ))) {
			result = EXIT_FAILURE;
			ಅನುವರ್ತ;
		}

		/* mount or remount? */
		ವಿಕಲ್ಪ (output) {
		ಪಕ್ಷ OUTPUT_MOUNT:
			result += do_mount(ent, false);
			ಭಙ್ಗ;

		ಪಕ್ಷ OUTPUT_REMOUNT:
			result += do_mount(ent, true);
			ಭಙ್ಗ;
		}

		/* No point in outputting if quiet */
		ಯದಿ (rc_yesno(getenv("EINFO_QUIET")))
			ಅನುವರ್ತ;

		ವಿಕಲ್ಪ (output) {
		ಪಕ್ಷ OUTPUT_BLOCKDEV:
			printf("%s\n", ENT_BLOCKDEVICE(ent));
			ಭಙ್ಗ;

		ಪಕ್ಷ OUTPUT_MOUNTARGS:
			printf("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->ಮೌಲ್ಯ);
			ಭಙ್ಗ;

		ಪಕ್ಷ OUTPUT_OPTIONS:
			printf("%s\n", ENT_OPTS(ent));
			ಭಙ್ಗ;

		ಪಕ್ಷ OUTPUT_FILE:
			printf("%s\n", file->ಮೌಲ್ಯ);
			ಭಙ್ಗ;

		ಪಕ್ಷ OUTPUT_PASSNO:
			printf("%d\n", ENT_PASS(ent));
			ಭಙ್ಗ;
		}
	}
	END_ENT;

	rc_stringlist_free(files);
	exit(result);
	/* NOTREACHED */
}
