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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
#  define HAVE_GETMNTENT
#  include <mntent.h>

#  define ENT mntent
#  define ಆರಂಭ_ENT fp = setmntent ("/etc/fstab", "r");
#  define GET_ENT getmntent (fp)
#  define GET_ENT_FILE(_name) getmntfile (_name)
#  define ಕೊನೆ_ENT endmntent (fp)
#  define ENT_BLOCKDEVICE(_ent) (_ent)->mnt_fsname
#  define ENT_FILE(_ent) (_ent)->mnt_dir
#  define ENT_TYPE(_ent) (_ent)->mnt_type
#  define ENT_OPTS(_ent) (_ent)->mnt_opts
#  define ENT_PASS(_ent) (_ent)->mnt_passno
#ಇಲ್ಲದಿದ್ದರೆ
#  define HAVE_GETFSENT
#  include <fstab.h>

#  define ENT fstab
#  define ಆರಂಭ_ENT
#  define GET_ENT getfsent ()
#  define GET_ENT_FILE(_name) getfsfile (_name)
#  define ಕೊನೆ_ENT endfsent ()
#  define ENT_BLOCKDEVICE(_ent) (_ent)->fs_spec
#  define ENT_TYPE(_ent) (_ent)->fs_vfstype
#  define ENT_FILE(_ent) (_ent)->fs_file
#  define ENT_OPTS(_ent) (_ent)->fs_mntops
#  define ENT_PASS(_ent) (_ent)->fs_passno
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಬಾಹ್ಯ ಅಕ್ಷರ **environ;

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{ "mount",          0, ಖಾಲಿ, 'M' },
	{ "remount",        0, ಖಾಲಿ, 'R' },
	{ "blockdevice",    0, ಖಾಲಿ, 'b' },
	{ "mountargs",      0, ಖಾಲಿ, 'm' },
	{ "options",        0, ಖಾಲಿ, 'o' },
	{ "passno",         1, ಖಾಲಿ, 'p' },
	{ "fstype",         1, ಖಾಲಿ, 't' },
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ಖಾಲಿ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_GETMNTENT
ಸ್ಥಿರ ರಚನೆ mntent *
getmntfile(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file)
{
	ರಚನೆ mntent *ent;
	FILE *fp;

	ಆರಂಭ_ENT;
	ಆಗ ((ent = getmntent(fp)))
		ಆದರೆ (strcmp(file, ent->mnt_dir) == 0)
			ನಿಲ್ಲಿಸು;
	ಕೊನೆ_ENT;

	ಹಿಂತಿರುಗಿ ent;
}
#ಆದರೆ_ಅಂತ್ಯ

ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
do_mount(ರಚನೆ ENT *ent, bool remount)
{
	ಅಕ್ಷರ *argv[10];
	pid_t pid;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	ಆದರೆ (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = ಖಾಲಿ;
	} ಇಲ್ಲದಿದ್ದರೆ {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = ಖಾಲಿ;
#ಇಲ್ಲದಿದ್ದರೆ
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
	}
	err = posix_spawnp(&pid, argv[0], ಖಾಲಿ, ಖಾಲಿ, argv, environ);
	ಆದರೆ (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	ಆಗ (waitpid(pid, &ಸ್ಥಿತಿ_2, 0) < 0 && errno == EINTR);
	ಆದರೆ (WIFEXITED(ಸ್ಥಿತಿ_2))
		ಹಿಂತಿರುಗಿ WEXITSTATUS(ಸ್ಥಿತಿ_2);
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ -1;
}

#ವ್ಯಾಖ್ಯಾನಿಸು OUTPUT_FILE      (1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು OUTPUT_MOUNTARGS (1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು OUTPUT_OPTIONS   (1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು OUTPUT_PASSNO    (1 << 4)
#ವ್ಯಾಖ್ಯಾನಿಸು OUTPUT_BLOCKDEV  (1 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು OUTPUT_MOUNT     (1 << 6)
#ವ್ಯಾಖ್ಯಾನಿಸು OUTPUT_REMOUNT   (1 << 7)

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ರಚನೆ ENT *ent;
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ = EXIT_SUCCESS;
	ಅಕ್ಷರ *token;
	ಪೂರ್ಣಾಂಕ i, p;
	ಪೂರ್ಣಾಂಕ opt;
	ಪೂರ್ಣಾಂಕ output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_GETMNTENT
	FILE *fp;
#ಆದರೆ_ಅಂತ್ಯ

	/* fail if there is no /etc/fstab */
	ಆದರೆ (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	ಆಗ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಂಕ *) 0)) != -1)
	{
		ಆಯ್ಕೆ (opt) {
		ಸ್ಥಿತಿ 'M':
			output = OUTPUT_MOUNT;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'R':
			output = OUTPUT_REMOUNT;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'b':
			output = OUTPUT_BLOCKDEV;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'o':
			output = OUTPUT_OPTIONS;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'm':
			output = OUTPUT_MOUNTARGS;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'p':
			ಆಯ್ಕೆ (optarg[0]) {
			ಸ್ಥಿತಿ '=':
			ಸ್ಥಿತಿ '<':
			ಸ್ಥಿತಿ '>':
				ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				ಆರಂಭ_ENT;
				ಆಗ ((ent = GET_ENT)) {
					ಆದರೆ (strcmp(ENT_FILE(ent), "none") == 0)
						ಮುಂದುವರಿ;
					p = ENT_PASS(ent);
					ಆದರೆ ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_ಕೂಡಿಸು(files,
						    ENT_FILE(ent));
				}
				ಕೊನೆ_ENT;
				ನಿಲ್ಲಿಸು;

			ಪೂರ್ವನಿಯೋಜಿತ:
				rc_stringlist_ಕೂಡಿಸು(files, optarg);
				output = OUTPUT_PASSNO;
				ನಿಲ್ಲಿಸು;
			}
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 't':
			filtered = true;
			ಆಗ ((token = strsep(&optarg, ","))) {
				ಆರಂಭ_ENT;
				ಆಗ ((ent = GET_ENT))
					ಆದರೆ (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_ಕೂಡಿಸು(files,
						    ENT_FILE(ent));
				ಕೊನೆ_ENT;
			}
			ನಿಲ್ಲಿಸು;

		case_RC_COMMON_GETOPT
		}
	}

	ಆದರೆ (optind < argc) {
		ಆದರೆ (TAILQ_ಮೊದಲ(files)) {
			TAILQ_FOREACH_ಸುರಕ್ಷಿತ(file, files, entries, file_np) {
				ಗಾಗಿ (i = optind; i < argc; i++)
					ಆದರೆ (strcmp(argv[i], file->ಮೌಲ್ಯ) == 0)
						ನಿಲ್ಲಿಸು;
				ಆದರೆ (i >= argc)
					rc_stringlist_delete(files,
					    file->ಮೌಲ್ಯ);
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆಗ (optind < argc)
				rc_stringlist_ಕೂಡಿಸು(files, argv[optind++]);
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!filtered) {
		ಆರಂಭ_ENT;
		ಆಗ ((ent = GET_ENT))
			rc_stringlist_ಕೂಡಿಸು(files, ENT_FILE(ent));
		ಕೊನೆ_ENT;

		ಆದರೆ (!TAILQ_ಮೊದಲ(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	ಆದರೆ (!TAILQ_ಮೊದಲ(files)) {
		rc_stringlist_free(files);
		ಹಿಂತಿರುಗಿ (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	ಆರಂಭ_ENT;
	TAILQ_FOREACH(file, files, entries) {
		ಆದರೆ (!(ent = GET_ENT_FILE(file->ಮೌಲ್ಯ))) {
			ಫಲಿತಾಂಶ = EXIT_FAILURE;
			ಮುಂದುವರಿ;
		}

		/* mount or remount? */
		ಆಯ್ಕೆ (output) {
		ಸ್ಥಿತಿ OUTPUT_MOUNT:
			ಫಲಿತಾಂಶ += do_mount(ent, false);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ OUTPUT_REMOUNT:
			ಫಲಿತಾಂಶ += do_mount(ent, true);
			ನಿಲ್ಲಿಸು;
		}

		/* No point in outputting if quiet */
		ಆದರೆ (rc_yesno(getenv("EINFO_QUIET")))
			ಮುಂದುವರಿ;

		ಆಯ್ಕೆ (output) {
		ಸ್ಥಿತಿ OUTPUT_BLOCKDEV:
			ಬರೆ("%s\n", ENT_BLOCKDEVICE(ent));
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ OUTPUT_MOUNTARGS:
			ಬರೆ("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->ಮೌಲ್ಯ);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ OUTPUT_OPTIONS:
			ಬರೆ("%s\n", ENT_OPTS(ent));
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ OUTPUT_FILE:
			ಬರೆ("%s\n", file->ಮೌಲ್ಯ);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ OUTPUT_PASSNO:
			ಬರೆ("%d\n", ENT_PASS(ent));
			ನಿಲ್ಲಿಸು;
		}
	}
	ಕೊನೆ_ENT;

	rc_stringlist_free(files);
	ನಿರ್ಗಮಿಸು(ಫಲಿತಾಂಶ);
	/* NOTREACHED */
}
