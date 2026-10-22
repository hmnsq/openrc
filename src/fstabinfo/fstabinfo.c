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

#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <spawn.h>

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
#ଅନ୍ୟଥା
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

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ବାହ୍ୟ ଅକ୍ଷର **environ;

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *extraopts = ଖାଲି;
ଅଚଳ ଅକ୍ଷର getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "mount",          0, ଖାଲି, 'M' },
	{ "remount",        0, ଖାଲି, 'R' },
	{ "blockdevice",    0, ଖାଲି, 'b' },
	{ "mountargs",      0, ଖାଲି, 'm' },
	{ "options",        0, ଖାଲି, 'o' },
	{ "passno",         1, ଖାଲି, 'p' },
	{ "fstype",         1, ଖାଲି, 't' },
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
ଅଚଳ ଅକ୍ଷର *usagestring = ଖାଲି;

#ifdef HAVE_GETMNTENT
ସ୍ଥିର ସଂରଚନା mntent *
getmntfile(ଅଚଳ ଅକ୍ଷର *file)
{
	ସଂରଚନା mntent *ent;
	FILE *fp;

	START_ENT;
	ଯେତେବେଳେ ((ent = getmntent(fp)))
		ଯଦି (strcmp(file, ent->mnt_dir) == 0)
			ଭାଙ୍ଗ;
	END_ENT;

	ଫେରନ୍ତୁ ent;
}
#endif

ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *applet;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
do_mount(ସଂରଚନା ENT *ent, bool remount)
{
	ଅକ୍ଷର *argv[10];
	pid_t pid;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	ଯଦି (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = ଖାଲି;
	} ଅନ୍ୟଥା {
#ifdef __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = ଖାଲି;
#ଅନ୍ୟଥା
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = ଖାଲି;
#endif
	}
	err = posix_spawnp(&pid, argv[0], ଖାଲି, ଖାଲି, argv, environ);
	ଯଦି (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	ଯେତେବେଳେ (waitpid(pid, &status, 0) < 0 && errno == EINTR);
	ଯଦି (WIFEXITED(status))
		ଫେରନ୍ତୁ WEXITSTATUS(status);
	ଅନ୍ୟଥା
		ଫେରନ୍ତୁ -1;
}

#define OUTPUT_FILE      (1 << 1)
#define OUTPUT_MOUNTARGS (1 << 2)
#define OUTPUT_OPTIONS   (1 << 3)
#define OUTPUT_PASSNO    (1 << 4)
#define OUTPUT_BLOCKDEV  (1 << 5)
#define OUTPUT_MOUNT     (1 << 6)
#define OUTPUT_REMOUNT   (1 << 7)

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ସଂରଚନା ENT *ent;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result = EXIT_SUCCESS;
	ଅକ୍ଷର *token;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#ifdef HAVE_GETMNTENT
	FILE *fp;
#endif

	/* fail if there is no /etc/fstab */
	ଯଦି (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) 0)) != -1)
	{
		ବାଛ (opt) {
		ସ୍ଥିତି 'M':
			output = OUTPUT_MOUNT;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'R':
			output = OUTPUT_REMOUNT;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'b':
			output = OUTPUT_BLOCKDEV;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'o':
			output = OUTPUT_OPTIONS;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'm':
			output = OUTPUT_MOUNTARGS;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'p':
			ବାଛ (optarg[0]) {
			ସ୍ଥିତି '=':
			ସ୍ଥିତି '<':
			ସ୍ଥିତି '>':
				ଯଦି (sscanf(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				START_ENT;
				ଯେତେବେଳେ ((ent = GET_ENT)) {
					ଯଦି (strcmp(ENT_FILE(ent), "none") == 0)
						ଜାରି;
					p = ENT_PASS(ent);
					ଯଦି ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				}
				END_ENT;
				ଭାଙ୍ଗ;

			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				rc_stringlist_add(files, optarg);
				output = OUTPUT_PASSNO;
				ଭାଙ୍ଗ;
			}
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 't':
			filtered = true;
			ଯେତେବେଳେ ((token = strsep(&optarg, ","))) {
				START_ENT;
				ଯେତେବେଳେ ((ent = GET_ENT))
					ଯଦି (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				END_ENT;
			}
			ଭାଙ୍ଗ;

		case_RC_COMMON_GETOPT
		}
	}

	ଯଦି (optind < argc) {
		ଯଦି (TAILQ_FIRST(files)) {
			TAILQ_FOREACH_SAFE(file, files, entries, file_np) {
				ପାଇଁ (i = optind; i < argc; i++)
					ଯଦି (strcmp(argv[i], file->ମୂଲ୍ୟ) == 0)
						ଭାଙ୍ଗ;
				ଯଦି (i >= argc)
					rc_stringlist_delete(files,
					    file->ମୂଲ୍ୟ);
			}
		} ଅନ୍ୟଥା {
			ଯେତେବେଳେ (optind < argc)
				rc_stringlist_add(files, argv[optind++]);
		}
	} ଅନ୍ୟଥା ଯଦି (!filtered) {
		START_ENT;
		ଯେତେବେଳେ ((ent = GET_ENT))
			rc_stringlist_add(files, ENT_FILE(ent));
		END_ENT;

		ଯଦି (!TAILQ_FIRST(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	ଯଦି (!TAILQ_FIRST(files)) {
		rc_stringlist_free(files);
		ଫେରନ୍ତୁ (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	START_ENT;
	TAILQ_FOREACH(file, files, entries) {
		ଯଦି (!(ent = GET_ENT_FILE(file->ମୂଲ୍ୟ))) {
			result = EXIT_FAILURE;
			ଜାରି;
		}

		/* mount or remount? */
		ବାଛ (output) {
		ସ୍ଥିତି OUTPUT_MOUNT:
			result += do_mount(ent, false);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି OUTPUT_REMOUNT:
			result += do_mount(ent, true);
			ଭାଙ୍ଗ;
		}

		/* No point in outputting if quiet */
		ଯଦି (rc_yesno(getenv("EINFO_QUIET")))
			ଜାରି;

		ବାଛ (output) {
		ସ୍ଥିତି OUTPUT_BLOCKDEV:
			printf("%s\n", ENT_BLOCKDEVICE(ent));
			ଭାଙ୍ଗ;

		ସ୍ଥିତି OUTPUT_MOUNTARGS:
			printf("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->ମୂଲ୍ୟ);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି OUTPUT_OPTIONS:
			printf("%s\n", ENT_OPTS(ent));
			ଭାଙ୍ଗ;

		ସ୍ଥିତି OUTPUT_FILE:
			printf("%s\n", file->ମୂଲ୍ୟ);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି OUTPUT_PASSNO:
			printf("%d\n", ENT_PASS(ent));
			ଭାଙ୍ଗ;
		}
	}
	END_ENT;

	rc_stringlist_free(files);
	exit(result);
	/* NOTREACHED */
}
