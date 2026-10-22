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

#অন্তর্ভাব <sys/wait.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <spawn.h>

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
#অন্যথা
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

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "_usage.h"
#অন্তর্ভাব "helpers.h"

বাহ্যিক অক্ষর **environ;

ধ্রুবক অক্ষর *applet = ফাঁকা;
ধ্রুবক অক্ষর *extraopts = ফাঁকা;
ধ্রুবক অক্ষর getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
ধ্রুবক কাঠামো option longopts[] = {
	{ "mount",          0, ফাঁকা, 'M' },
	{ "remount",        0, ফাঁকা, 'R' },
	{ "blockdevice",    0, ফাঁকা, 'b' },
	{ "mountargs",      0, ফাঁকা, 'm' },
	{ "options",        0, ফাঁকা, 'o' },
	{ "passno",         1, ফাঁকা, 'p' },
	{ "fstype",         1, ফাঁকা, 't' },
	longopts_COMMON
};
ধ্রুবক অক্ষর * ধ্রুবক longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
ধ্রুবক অক্ষর *usagestring = ফাঁকা;

#ifdef HAVE_GETMNTENT
স্থির কাঠামো mntent *
getmntfile(ধ্রুবক অক্ষর *file)
{
	কাঠামো mntent *ent;
	FILE *fp;

	START_ENT;
	যখন ((ent = getmntent(fp)))
		যদি (strcmp(file, ent->mnt_dir) == 0)
			বিরতি;
	END_ENT;

	ফেরত ent;
}
#endif

বাহ্যিক ধ্রুবক অক্ষর *applet;

স্থির পূর্ণসংখ্যা
do_mount(কাঠামো ENT *ent, bool remount)
{
	অক্ষর *argv[10];
	pid_t pid;
	পূর্ণসংখ্যা status, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	যদি (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = ফাঁকা;
	} অন্যথা {
#ifdef __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = ফাঁকা;
#অন্যথা
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = ফাঁকা;
#endif
	}
	err = posix_spawnp(&pid, argv[0], ফাঁকা, ফাঁকা, argv, environ);
	যদি (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	যখন (waitpid(pid, &status, 0) < 0 && errno == EINTR);
	যদি (WIFEXITED(status))
		ফেরত WEXITSTATUS(status);
	অন্যথা
		ফেরত -1;
}

#define OUTPUT_FILE      (1 << 1)
#define OUTPUT_MOUNTARGS (1 << 2)
#define OUTPUT_OPTIONS   (1 << 3)
#define OUTPUT_PASSNO    (1 << 4)
#define OUTPUT_BLOCKDEV  (1 << 5)
#define OUTPUT_MOUNT     (1 << 6)
#define OUTPUT_REMOUNT   (1 << 7)

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	কাঠামো ENT *ent;
	পূর্ণসংখ্যা ফলাফল = EXIT_SUCCESS;
	অক্ষর *token;
	পূর্ণসংখ্যা i, p;
	পূর্ণসংখ্যা opt;
	পূর্ণসংখ্যা output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#ifdef HAVE_GETMNTENT
	FILE *fp;
#endif

	/* fail if there is no /etc/fstab */
	যদি (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	যখন ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (পূর্ণসংখ্যা *) 0)) != -1)
	{
		নির্বাচন (opt) {
		ক্ষেত্র 'M':
			output = OUTPUT_MOUNT;
			বিরতি;
		ক্ষেত্র 'R':
			output = OUTPUT_REMOUNT;
			বিরতি;
		ক্ষেত্র 'b':
			output = OUTPUT_BLOCKDEV;
			বিরতি;
		ক্ষেত্র 'o':
			output = OUTPUT_OPTIONS;
			বিরতি;
		ক্ষেত্র 'm':
			output = OUTPUT_MOUNTARGS;
			বিরতি;

		ক্ষেত্র 'p':
			নির্বাচন (optarg[0]) {
			ক্ষেত্র '=':
			ক্ষেত্র '<':
			ক্ষেত্র '>':
				যদি (sscanf(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				START_ENT;
				যখন ((ent = GET_ENT)) {
					যদি (strcmp(ENT_FILE(ent), "none") == 0)
						চালু;
					p = ENT_PASS(ent);
					যদি ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				}
				END_ENT;
				বিরতি;

			পূর্বনির্ধারিত:
				rc_stringlist_add(files, optarg);
				output = OUTPUT_PASSNO;
				বিরতি;
			}
			বিরতি;

		ক্ষেত্র 't':
			filtered = true;
			যখন ((token = strsep(&optarg, ","))) {
				START_ENT;
				যখন ((ent = GET_ENT))
					যদি (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				END_ENT;
			}
			বিরতি;

		case_RC_COMMON_GETOPT
		}
	}

	যদি (optind < argc) {
		যদি (TAILQ_FIRST(files)) {
			TAILQ_FOREACH_SAFE(file, files, entries, file_np) {
				জন্য (i = optind; i < argc; i++)
					যদি (strcmp(argv[i], file->মান) == 0)
						বিরতি;
				যদি (i >= argc)
					rc_stringlist_delete(files,
					    file->মান);
			}
		} অন্যথা {
			যখন (optind < argc)
				rc_stringlist_add(files, argv[optind++]);
		}
	} অন্যথা যদি (!filtered) {
		START_ENT;
		যখন ((ent = GET_ENT))
			rc_stringlist_add(files, ENT_FILE(ent));
		END_ENT;

		যদি (!TAILQ_FIRST(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	যদি (!TAILQ_FIRST(files)) {
		rc_stringlist_free(files);
		ফেরত (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	START_ENT;
	TAILQ_FOREACH(file, files, entries) {
		যদি (!(ent = GET_ENT_FILE(file->মান))) {
			ফলাফল = EXIT_FAILURE;
			চালু;
		}

		/* mount or remount? */
		নির্বাচন (output) {
		ক্ষেত্র OUTPUT_MOUNT:
			ফলাফল += do_mount(ent, false);
			বিরতি;

		ক্ষেত্র OUTPUT_REMOUNT:
			ফলাফল += do_mount(ent, true);
			বিরতি;
		}

		/* No point in outputting if quiet */
		যদি (rc_yesno(getenv("EINFO_QUIET")))
			চালু;

		নির্বাচন (output) {
		ক্ষেত্র OUTPUT_BLOCKDEV:
			printf("%s\n", ENT_BLOCKDEVICE(ent));
			বিরতি;

		ক্ষেত্র OUTPUT_MOUNTARGS:
			printf("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->মান);
			বিরতি;

		ক্ষেত্র OUTPUT_OPTIONS:
			printf("%s\n", ENT_OPTS(ent));
			বিরতি;

		ক্ষেত্র OUTPUT_FILE:
			printf("%s\n", file->মান);
			বিরতি;

		ক্ষেত্র OUTPUT_PASSNO:
			printf("%d\n", ENT_PASS(ent));
			বিরতি;
		}
	}
	END_ENT;

	rc_stringlist_free(files);
	exit(ফলাফল);
	/* NOTREACHED */
}
