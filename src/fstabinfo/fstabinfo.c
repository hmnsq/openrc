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
#اگر_تعریف_شدہ __linux__
#  define HAVE_GETMNTENT
#  include <mntent.h>

#  define ENT mntent
#  define آغاز_ENT fp = setmntent ("/etc/fstab", "r");
#  define GET_ENT getmntent (fp)
#  define GET_ENT_FILE(_name) getmntfile (_name)
#  define انتہا_ENT endmntent (fp)
#  define ENT_BLOCKDEVICE(_ent) (_ent)->mnt_fsname
#  define ENT_FILE(_ent) (_ent)->mnt_dir
#  define ENT_TYPE(_ent) (_ent)->mnt_type
#  define ENT_OPTS(_ent) (_ent)->mnt_opts
#  define ENT_PASS(_ent) (_ent)->mnt_passno
#ورنہ
#  define HAVE_GETFSENT
#  include <fstab.h>

#  define ENT fstab
#  define آغاز_ENT
#  define GET_ENT getfsent ()
#  define GET_ENT_FILE(_name) getfsfile (_name)
#  define انتہا_ENT endfsent ()
#  define ENT_BLOCKDEVICE(_ent) (_ent)->fs_spec
#  define ENT_TYPE(_ent) (_ent)->fs_vfstype
#  define ENT_FILE(_ent) (_ent)->fs_file
#  define ENT_OPTS(_ent) (_ent)->fs_mntops
#  define ENT_PASS(_ent) (_ent)->fs_passno
#اگر_ختم

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "_usage.h"
#شامل "helpers.h"

خارجی حرف **environ;

ثابت حرف *applet = تہی;
ثابت حرف *extraopts = تہی;
ثابت حرف getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{ "mount",          0, تہی, 'M' },
	{ "remount",        0, تہی, 'R' },
	{ "blockdevice",    0, تہی, 'b' },
	{ "mountargs",      0, تہی, 'm' },
	{ "options",        0, تہی, 'o' },
	{ "passno",         1, تہی, 'p' },
	{ "fstype",         1, تہی, 't' },
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
ثابت حرف *usagestring = تہی;

#اگر_تعریف_شدہ HAVE_GETMNTENT
ساکن ساخت mntent *
getmntfile(ثابت حرف *file)
{
	ساخت mntent *ent;
	FILE *fp;

	آغاز_ENT;
	جب ((ent = getmntent(fp)))
		اگر (strcmp(file, ent->mnt_dir) == 0)
			توڑو;
	انتہا_ENT;

	واپس ent;
}
#اگر_ختم

خارجی ثابت حرف *applet;

ساکن صحیح
do_mount(ساخت ENT *ent, bool remount)
{
	حرف *argv[10];
	pid_t pid;
	صحیح حالت_2, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	اگر (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = تہی;
	} ورنہ {
#اگر_تعریف_شدہ __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = تہی;
#ورنہ
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = تہی;
#اگر_ختم
	}
	err = posix_spawnp(&pid, argv[0], تہی, تہی, argv, environ);
	اگر (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	جب (waitpid(pid, &حالت_2, 0) < 0 && errno == EINTR);
	اگر (WIFEXITED(حالت_2))
		واپس WEXITSTATUS(حالت_2);
	ورنہ
		واپس -1;
}

#تعریف_کرو OUTPUT_FILE      (1 << 1)
#تعریف_کرو OUTPUT_MOUNTARGS (1 << 2)
#تعریف_کرو OUTPUT_OPTIONS   (1 << 3)
#تعریف_کرو OUTPUT_PASSNO    (1 << 4)
#تعریف_کرو OUTPUT_BLOCKDEV  (1 << 5)
#تعریف_کرو OUTPUT_MOUNT     (1 << 6)
#تعریف_کرو OUTPUT_REMOUNT   (1 << 7)

صحیح مین(صحیح argc, حرف **argv)
{
	ساخت ENT *ent;
	صحیح نتیجہ = EXIT_SUCCESS;
	حرف *token;
	صحیح i, p;
	صحیح opt;
	صحیح output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#اگر_تعریف_شدہ HAVE_GETMNTENT
	FILE *fp;
#اگر_ختم

	/* fail if there is no /etc/fstab */
	اگر (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	جب ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
		حالت 'M':
			output = OUTPUT_MOUNT;
			توڑو;
		حالت 'R':
			output = OUTPUT_REMOUNT;
			توڑو;
		حالت 'b':
			output = OUTPUT_BLOCKDEV;
			توڑو;
		حالت 'o':
			output = OUTPUT_OPTIONS;
			توڑو;
		حالت 'm':
			output = OUTPUT_MOUNTARGS;
			توڑو;

		حالت 'p':
			انتخاب (optarg[0]) {
			حالت '=':
			حالت '<':
			حالت '>':
				اگر (لڑی_سے_پڑھو(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				آغاز_ENT;
				جب ((ent = GET_ENT)) {
					اگر (strcmp(ENT_FILE(ent), "none") == 0)
						جاری;
					p = ENT_PASS(ent);
					اگر ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_جوڑو(files,
						    ENT_FILE(ent));
				}
				انتہا_ENT;
				توڑو;

			طےشدہ:
				rc_stringlist_جوڑو(files, optarg);
				output = OUTPUT_PASSNO;
				توڑو;
			}
			توڑو;

		حالت 't':
			filtered = true;
			جب ((token = strsep(&optarg, ","))) {
				آغاز_ENT;
				جب ((ent = GET_ENT))
					اگر (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_جوڑو(files,
						    ENT_FILE(ent));
				انتہا_ENT;
			}
			توڑو;

		case_RC_COMMON_GETOPT
		}
	}

	اگر (optind < argc) {
		اگر (TAILQ_پہلا(files)) {
			TAILQ_FOREACH_محفوظ(file, files, entries, file_np) {
				برائے (i = optind; i < argc; i++)
					اگر (strcmp(argv[i], file->قدر) == 0)
						توڑو;
				اگر (i >= argc)
					rc_stringlist_delete(files,
					    file->قدر);
			}
		} ورنہ {
			جب (optind < argc)
				rc_stringlist_جوڑو(files, argv[optind++]);
		}
	} ورنہ اگر (!filtered) {
		آغاز_ENT;
		جب ((ent = GET_ENT))
			rc_stringlist_جوڑو(files, ENT_FILE(ent));
		انتہا_ENT;

		اگر (!TAILQ_پہلا(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	اگر (!TAILQ_پہلا(files)) {
		rc_stringlist_free(files);
		واپس (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	آغاز_ENT;
	TAILQ_FOREACH(file, files, entries) {
		اگر (!(ent = GET_ENT_FILE(file->قدر))) {
			نتیجہ = EXIT_FAILURE;
			جاری;
		}

		/* mount or remount? */
		انتخاب (output) {
		حالت OUTPUT_MOUNT:
			نتیجہ += do_mount(ent, false);
			توڑو;

		حالت OUTPUT_REMOUNT:
			نتیجہ += do_mount(ent, true);
			توڑو;
		}

		/* No point in outputting if quiet */
		اگر (rc_yesno(getenv("EINFO_QUIET")))
			جاری;

		انتخاب (output) {
		حالت OUTPUT_BLOCKDEV:
			لکھو("%s\n", ENT_BLOCKDEVICE(ent));
			توڑو;

		حالت OUTPUT_MOUNTARGS:
			لکھو("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->قدر);
			توڑو;

		حالت OUTPUT_OPTIONS:
			لکھو("%s\n", ENT_OPTS(ent));
			توڑو;

		حالت OUTPUT_FILE:
			لکھو("%s\n", file->قدر);
			توڑو;

		حالت OUTPUT_PASSNO:
			لکھو("%d\n", ENT_PASS(ent));
			توڑو;
		}
	}
	انتہا_ENT;

	rc_stringlist_free(files);
	باہر_نکلو(نتیجہ);
	/* NOTREACHED */
}
