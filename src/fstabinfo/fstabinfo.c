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

বাহ্য বর্ণ **environ;

ধ্রুব বর্ণ *applet = রিক্ত;
ধ্রুব বর্ণ *extraopts = রিক্ত;
ধ্রুব বর্ণ getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
ধ্রুব সংরচনা option longopts[] = {
	{ "mount",          0, রিক্ত, 'M' },
	{ "remount",        0, রিক্ত, 'R' },
	{ "blockdevice",    0, রিক্ত, 'b' },
	{ "mountargs",      0, রিক্ত, 'm' },
	{ "options",        0, রিক্ত, 'o' },
	{ "passno",         1, রিক্ত, 'p' },
	{ "fstype",         1, রিক্ত, 't' },
	longopts_COMMON
};
ধ্রুব বর্ণ * ধ্রুব longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
ধ্রুব বর্ণ *usagestring = রিক্ত;

#ifdef HAVE_GETMNTENT
স্থির সংরচনা mntent *
getmntfile(ধ্রুব বর্ণ *file)
{
	সংরচনা mntent *ent;
	FILE *fp;

	START_ENT;
	যাবত্ ((ent = getmntent(fp)))
		যদি (strcmp(file, ent->mnt_dir) == 0)
			ভঙ্গ;
	END_ENT;

	প্রত্যাগম ent;
}
#endif

বাহ্য ধ্রুব বর্ণ *applet;

স্থির পূর্ণাঙ্ক
do_mount(সংরচনা ENT *ent, bool remount)
{
	বর্ণ *argv[10];
	pid_t pid;
	পূর্ণাঙ্ক status, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	যদি (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = রিক্ত;
	} অন্যথা {
#ifdef __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = রিক্ত;
#অন্যথা
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = রিক্ত;
#endif
	}
	err = posix_spawnp(&pid, argv[0], রিক্ত, রিক্ত, argv, environ);
	যদি (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	যাবত্ (waitpid(pid, &status, 0) < 0 && errno == EINTR);
	যদি (WIFEXITED(status))
		প্রত্যাগম WEXITSTATUS(status);
	অন্যথা
		প্রত্যাগম -1;
}

#define OUTPUT_FILE      (1 << 1)
#define OUTPUT_MOUNTARGS (1 << 2)
#define OUTPUT_OPTIONS   (1 << 3)
#define OUTPUT_PASSNO    (1 << 4)
#define OUTPUT_BLOCKDEV  (1 << 5)
#define OUTPUT_MOUNT     (1 << 6)
#define OUTPUT_REMOUNT   (1 << 7)

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	সংরচনা ENT *ent;
	পূর্ণাঙ্ক result = EXIT_SUCCESS;
	বর্ণ *token;
	পূর্ণাঙ্ক i, p;
	পূর্ণাঙ্ক opt;
	পূর্ণাঙ্ক output = OUTPUT_FILE;
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
	যাবত্ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (পূর্ণাঙ্ক *) 0)) != -1)
	{
		বিকল্প (opt) {
		পক্ষ 'M':
			output = OUTPUT_MOUNT;
			ভঙ্গ;
		পক্ষ 'R':
			output = OUTPUT_REMOUNT;
			ভঙ্গ;
		পক্ষ 'b':
			output = OUTPUT_BLOCKDEV;
			ভঙ্গ;
		পক্ষ 'o':
			output = OUTPUT_OPTIONS;
			ভঙ্গ;
		পক্ষ 'm':
			output = OUTPUT_MOUNTARGS;
			ভঙ্গ;

		পক্ষ 'p':
			বিকল্প (optarg[0]) {
			পক্ষ '=':
			পক্ষ '<':
			পক্ষ '>':
				যদি (sscanf(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				START_ENT;
				যাবত্ ((ent = GET_ENT)) {
					যদি (strcmp(ENT_FILE(ent), "none") == 0)
						অনুবর্ত;
					p = ENT_PASS(ent);
					যদি ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				}
				END_ENT;
				ভঙ্গ;

			স্বভাব:
				rc_stringlist_add(files, optarg);
				output = OUTPUT_PASSNO;
				ভঙ্গ;
			}
			ভঙ্গ;

		পক্ষ 't':
			filtered = true;
			যাবত্ ((token = strsep(&optarg, ","))) {
				START_ENT;
				যাবত্ ((ent = GET_ENT))
					যদি (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				END_ENT;
			}
			ভঙ্গ;

		case_RC_COMMON_GETOPT
		}
	}

	যদি (optind < argc) {
		যদি (TAILQ_FIRST(files)) {
			TAILQ_FOREACH_SAFE(file, files, entries, file_np) {
				প্রতি (i = optind; i < argc; i++)
					যদি (strcmp(argv[i], file->মান) == 0)
						ভঙ্গ;
				যদি (i >= argc)
					rc_stringlist_delete(files,
					    file->মান);
			}
		} অন্যথা {
			যাবত্ (optind < argc)
				rc_stringlist_add(files, argv[optind++]);
		}
	} অন্যথা যদি (!filtered) {
		START_ENT;
		যাবত্ ((ent = GET_ENT))
			rc_stringlist_add(files, ENT_FILE(ent));
		END_ENT;

		যদি (!TAILQ_FIRST(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	যদি (!TAILQ_FIRST(files)) {
		rc_stringlist_free(files);
		প্রত্যাগম (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	START_ENT;
	TAILQ_FOREACH(file, files, entries) {
		যদি (!(ent = GET_ENT_FILE(file->মান))) {
			result = EXIT_FAILURE;
			অনুবর্ত;
		}

		/* mount or remount? */
		বিকল্প (output) {
		পক্ষ OUTPUT_MOUNT:
			result += do_mount(ent, false);
			ভঙ্গ;

		পক্ষ OUTPUT_REMOUNT:
			result += do_mount(ent, true);
			ভঙ্গ;
		}

		/* No point in outputting if quiet */
		যদি (rc_yesno(getenv("EINFO_QUIET")))
			অনুবর্ত;

		বিকল্প (output) {
		পক্ষ OUTPUT_BLOCKDEV:
			printf("%s\n", ENT_BLOCKDEVICE(ent));
			ভঙ্গ;

		পক্ষ OUTPUT_MOUNTARGS:
			printf("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->মান);
			ভঙ্গ;

		পক্ষ OUTPUT_OPTIONS:
			printf("%s\n", ENT_OPTS(ent));
			ভঙ্গ;

		পক্ষ OUTPUT_FILE:
			printf("%s\n", file->মান);
			ভঙ্গ;

		পক্ষ OUTPUT_PASSNO:
			printf("%d\n", ENT_PASS(ent));
			ভঙ্গ;
		}
	}
	END_ENT;

	rc_stringlist_free(files);
	exit(result);
	/* NOTREACHED */
}
