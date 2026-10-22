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

#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <spawn.h>

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
#નહીંતર
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

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "_usage.h"
#અન્તર્ભાવ "helpers.h"

બાહ્ય અક્ષર **environ;

અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *extraopts = ખાલી;
અચલ અક્ષર getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{ "mount",          0, ખાલી, 'M' },
	{ "remount",        0, ખાલી, 'R' },
	{ "blockdevice",    0, ખાલી, 'b' },
	{ "mountargs",      0, ખાલી, 'm' },
	{ "options",        0, ખાલી, 'o' },
	{ "passno",         1, ખાલી, 'p' },
	{ "fstype",         1, ખાલી, 't' },
	longopts_COMMON
};
અચલ અક્ષર * અચલ longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
અચલ અક્ષર *usagestring = ખાલી;

#ifdef HAVE_GETMNTENT
સ્થિર સંરચના mntent *
getmntfile(અચલ અક્ષર *file)
{
	સંરચના mntent *ent;
	FILE *fp;

	START_ENT;
	જ્યારે ((ent = getmntent(fp)))
		જો (strcmp(file, ent->mnt_dir) == 0)
			તોડો;
	END_ENT;

	પાછા ent;
}
#endif

બાહ્ય અચલ અક્ષર *applet;

સ્થિર પૂર્ણાંક
do_mount(સંરચના ENT *ent, bool remount)
{
	અક્ષર *argv[10];
	pid_t pid;
	પૂર્ણાંક status, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	જો (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = ખાલી;
	} નહીંતર {
#ifdef __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = ખાલી;
#નહીંતર
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = ખાલી;
#endif
	}
	err = posix_spawnp(&pid, argv[0], ખાલી, ખાલી, argv, environ);
	જો (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	જ્યારે (waitpid(pid, &status, 0) < 0 && errno == EINTR);
	જો (WIFEXITED(status))
		પાછા WEXITSTATUS(status);
	નહીંતર
		પાછા -1;
}

#define OUTPUT_FILE      (1 << 1)
#define OUTPUT_MOUNTARGS (1 << 2)
#define OUTPUT_OPTIONS   (1 << 3)
#define OUTPUT_PASSNO    (1 << 4)
#define OUTPUT_BLOCKDEV  (1 << 5)
#define OUTPUT_MOUNT     (1 << 6)
#define OUTPUT_REMOUNT   (1 << 7)

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	સંરચના ENT *ent;
	પૂર્ણાંક result = EXIT_SUCCESS;
	અક્ષર *token;
	પૂર્ણાંક i, p;
	પૂર્ણાંક opt;
	પૂર્ણાંક output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#ifdef HAVE_GETMNTENT
	FILE *fp;
#endif

	/* fail if there is no /etc/fstab */
	જો (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	જ્યારે ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (પૂર્ણાંક *) 0)) != -1)
	{
		પસંદગી (opt) {
		સ્થિતિ 'M':
			output = OUTPUT_MOUNT;
			તોડો;
		સ્થિતિ 'R':
			output = OUTPUT_REMOUNT;
			તોડો;
		સ્થિતિ 'b':
			output = OUTPUT_BLOCKDEV;
			તોડો;
		સ્થિતિ 'o':
			output = OUTPUT_OPTIONS;
			તોડો;
		સ્થિતિ 'm':
			output = OUTPUT_MOUNTARGS;
			તોડો;

		સ્થિતિ 'p':
			પસંદગી (optarg[0]) {
			સ્થિતિ '=':
			સ્થિતિ '<':
			સ્થિતિ '>':
				જો (sscanf(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				START_ENT;
				જ્યારે ((ent = GET_ENT)) {
					જો (strcmp(ENT_FILE(ent), "none") == 0)
						ચાલુ;
					p = ENT_PASS(ent);
					જો ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				}
				END_ENT;
				તોડો;

			મૂળભૂત:
				rc_stringlist_add(files, optarg);
				output = OUTPUT_PASSNO;
				તોડો;
			}
			તોડો;

		સ્થિતિ 't':
			filtered = true;
			જ્યારે ((token = strsep(&optarg, ","))) {
				START_ENT;
				જ્યારે ((ent = GET_ENT))
					જો (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				END_ENT;
			}
			તોડો;

		case_RC_COMMON_GETOPT
		}
	}

	જો (optind < argc) {
		જો (TAILQ_FIRST(files)) {
			TAILQ_FOREACH_SAFE(file, files, entries, file_np) {
				માટે (i = optind; i < argc; i++)
					જો (strcmp(argv[i], file->કિંમત) == 0)
						તોડો;
				જો (i >= argc)
					rc_stringlist_delete(files,
					    file->કિંમત);
			}
		} નહીંતર {
			જ્યારે (optind < argc)
				rc_stringlist_add(files, argv[optind++]);
		}
	} નહીંતર જો (!filtered) {
		START_ENT;
		જ્યારે ((ent = GET_ENT))
			rc_stringlist_add(files, ENT_FILE(ent));
		END_ENT;

		જો (!TAILQ_FIRST(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	જો (!TAILQ_FIRST(files)) {
		rc_stringlist_free(files);
		પાછા (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	START_ENT;
	TAILQ_FOREACH(file, files, entries) {
		જો (!(ent = GET_ENT_FILE(file->કિંમત))) {
			result = EXIT_FAILURE;
			ચાલુ;
		}

		/* mount or remount? */
		પસંદગી (output) {
		સ્થિતિ OUTPUT_MOUNT:
			result += do_mount(ent, false);
			તોડો;

		સ્થિતિ OUTPUT_REMOUNT:
			result += do_mount(ent, true);
			તોડો;
		}

		/* No point in outputting if quiet */
		જો (rc_yesno(getenv("EINFO_QUIET")))
			ચાલુ;

		પસંદગી (output) {
		સ્થિતિ OUTPUT_BLOCKDEV:
			printf("%s\n", ENT_BLOCKDEVICE(ent));
			તોડો;

		સ્થિતિ OUTPUT_MOUNTARGS:
			printf("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->કિંમત);
			તોડો;

		સ્થિતિ OUTPUT_OPTIONS:
			printf("%s\n", ENT_OPTS(ent));
			તોડો;

		સ્થિતિ OUTPUT_FILE:
			printf("%s\n", file->કિંમત);
			તોડો;

		સ્થિતિ OUTPUT_PASSNO:
			printf("%d\n", ENT_PASS(ent));
			તોડો;
		}
	}
	END_ENT;

	rc_stringlist_free(files);
	exit(result);
	/* NOTREACHED */
}
