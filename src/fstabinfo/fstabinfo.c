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

#شامل <sys/wait.h>
#شامل <errno.h>
#شامل <getopt.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <unistd.h>
#شامل <stdbool.h>
#شامل <spawn.h>

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
#وگرنه
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

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "_usage.h"
#شامل "helpers.h"

بیرونی نویسه **environ;

ثابت نویسه *applet = پوچ;
ثابت نویسه *extraopts = پوچ;
ثابت نویسه getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{ "mount",          0, پوچ, 'M' },
	{ "remount",        0, پوچ, 'R' },
	{ "blockdevice",    0, پوچ, 'b' },
	{ "mountargs",      0, پوچ, 'm' },
	{ "options",        0, پوچ, 'o' },
	{ "passno",         1, پوچ, 'p' },
	{ "fstype",         1, پوچ, 't' },
	longopts_COMMON
};
ثابت نویسه * ثابت longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
ثابت نویسه *usagestring = پوچ;

#ifdef HAVE_GETMNTENT
ایستا ساختار mntent *
getmntfile(ثابت نویسه *file)
{
	ساختار mntent *ent;
	FILE *fp;

	START_ENT;
	وقتی ((ent = getmntent(fp)))
		اگر (strcmp(file, ent->mnt_dir) == 0)
			بشکن;
	END_ENT;

	بازگشت ent;
}
#endif

بیرونی ثابت نویسه *applet;

ایستا صحیح
do_mount(ساختار ENT *ent, bool remount)
{
	نویسه *argv[10];
	pid_t pid;
	صحیح status, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	اگر (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = پوچ;
	} وگرنه {
#ifdef __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = پوچ;
#وگرنه
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = پوچ;
#endif
	}
	err = posix_spawnp(&pid, argv[0], پوچ, پوچ, argv, environ);
	اگر (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	وقتی (waitpid(pid, &status, 0) < 0 && errno == EINTR);
	اگر (WIFEXITED(status))
		بازگشت WEXITSTATUS(status);
	وگرنه
		بازگشت -1;
}

#define OUTPUT_FILE      (1 << 1)
#define OUTPUT_MOUNTARGS (1 << 2)
#define OUTPUT_OPTIONS   (1 << 3)
#define OUTPUT_PASSNO    (1 << 4)
#define OUTPUT_BLOCKDEV  (1 << 5)
#define OUTPUT_MOUNT     (1 << 6)
#define OUTPUT_REMOUNT   (1 << 7)

صحیح اصلی(صحیح argc, نویسه **argv)
{
	ساختار ENT *ent;
	صحیح result = EXIT_SUCCESS;
	نویسه *token;
	صحیح i, p;
	صحیح opt;
	صحیح output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#ifdef HAVE_GETMNTENT
	FILE *fp;
#endif

	/* fail if there is no /etc/fstab */
	اگر (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	وقتی ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
		حالت 'M':
			output = OUTPUT_MOUNT;
			بشکن;
		حالت 'R':
			output = OUTPUT_REMOUNT;
			بشکن;
		حالت 'b':
			output = OUTPUT_BLOCKDEV;
			بشکن;
		حالت 'o':
			output = OUTPUT_OPTIONS;
			بشکن;
		حالت 'm':
			output = OUTPUT_MOUNTARGS;
			بشکن;

		حالت 'p':
			انتخاب (optarg[0]) {
			حالت '=':
			حالت '<':
			حالت '>':
				اگر (sscanf(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				START_ENT;
				وقتی ((ent = GET_ENT)) {
					اگر (strcmp(ENT_FILE(ent), "none") == 0)
						ادامه;
					p = ENT_PASS(ent);
					اگر ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				}
				END_ENT;
				بشکن;

			پیشفرض:
				rc_stringlist_add(files, optarg);
				output = OUTPUT_PASSNO;
				بشکن;
			}
			بشکن;

		حالت 't':
			filtered = true;
			وقتی ((token = strsep(&optarg, ","))) {
				START_ENT;
				وقتی ((ent = GET_ENT))
					اگر (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				END_ENT;
			}
			بشکن;

		case_RC_COMMON_GETOPT
		}
	}

	اگر (optind < argc) {
		اگر (TAILQ_FIRST(files)) {
			TAILQ_FOREACH_SAFE(file, files, entries, file_np) {
				برای (i = optind; i < argc; i++)
					اگر (strcmp(argv[i], file->مقدار) == 0)
						بشکن;
				اگر (i >= argc)
					rc_stringlist_delete(files,
					    file->مقدار);
			}
		} وگرنه {
			وقتی (optind < argc)
				rc_stringlist_add(files, argv[optind++]);
		}
	} وگرنه اگر (!filtered) {
		START_ENT;
		وقتی ((ent = GET_ENT))
			rc_stringlist_add(files, ENT_FILE(ent));
		END_ENT;

		اگر (!TAILQ_FIRST(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	اگر (!TAILQ_FIRST(files)) {
		rc_stringlist_free(files);
		بازگشت (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	START_ENT;
	TAILQ_FOREACH(file, files, entries) {
		اگر (!(ent = GET_ENT_FILE(file->مقدار))) {
			result = EXIT_FAILURE;
			ادامه;
		}

		/* mount or remount? */
		انتخاب (output) {
		حالت OUTPUT_MOUNT:
			result += do_mount(ent, false);
			بشکن;

		حالت OUTPUT_REMOUNT:
			result += do_mount(ent, true);
			بشکن;
		}

		/* No point in outputting if quiet */
		اگر (rc_yesno(getenv("EINFO_QUIET")))
			ادامه;

		انتخاب (output) {
		حالت OUTPUT_BLOCKDEV:
			printf("%s\n", ENT_BLOCKDEVICE(ent));
			بشکن;

		حالت OUTPUT_MOUNTARGS:
			printf("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->مقدار);
			بشکن;

		حالت OUTPUT_OPTIONS:
			printf("%s\n", ENT_OPTS(ent));
			بشکن;

		حالت OUTPUT_FILE:
			printf("%s\n", file->مقدار);
			بشکن;

		حالت OUTPUT_PASSNO:
			printf("%d\n", ENT_PASS(ent));
			بشکن;
		}
	}
	END_ENT;

	rc_stringlist_free(files);
	exit(result);
	/* NOTREACHED */
}
