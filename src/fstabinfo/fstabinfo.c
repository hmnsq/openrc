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

#ضمن <sys/wait.h>
#ضمن <errno.h>
#ضمن <getopt.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <unistd.h>
#ضمن <stdbool.h>
#ضمن <spawn.h>

/* Yay for linux and its non liking of POSIX functions.
   Okay, we could use getfsent but the man page says use getmntent instead
   AND we don't have getfsent on uclibc or dietlibc for some odd reason. */
#که_تعریف_شوی __linux__
#  define HAVE_GETMNTENT
#  include <mntent.h>

#  define ENT mntent
#  define پیل_ENT fp = setmntent ("/etc/fstab", "r");
#  define GET_ENT getmntent (fp)
#  define GET_ENT_FILE(_name) getmntfile (_name)
#  define پای_ټکی_ENT endmntent (fp)
#  define ENT_BLOCKDEVICE(_ent) (_ent)->mnt_fsname
#  define ENT_FILE(_ent) (_ent)->mnt_dir
#  define ENT_TYPE(_ent) (_ent)->mnt_type
#  define ENT_OPTS(_ent) (_ent)->mnt_opts
#  define ENT_PASS(_ent) (_ent)->mnt_passno
#وإلا
#  define HAVE_GETFSENT
#  include <fstab.h>

#  define ENT fstab
#  define پیل_ENT
#  define GET_ENT getfsent ()
#  define GET_ENT_FILE(_name) getfsfile (_name)
#  define پای_ټکی_ENT endfsent ()
#  define ENT_BLOCKDEVICE(_ent) (_ent)->fs_spec
#  define ENT_TYPE(_ent) (_ent)->fs_vfstype
#  define ENT_FILE(_ent) (_ent)->fs_file
#  define ENT_OPTS(_ent) (_ent)->fs_mntops
#  define ENT_PASS(_ent) (_ent)->fs_passno
#د_که_پای

#ضمن "einfo.h"
#ضمن "queue.h"
#ضمن "rc.h"
#ضمن "_usage.h"
#ضمن "helpers.h"

خارجي حرف **environ;

ثابت حرف *applet = عدم;
ثابت حرف *extraopts = عدم;
ثابت حرف getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "mount",          0, عدم, 'M' },
	{ "remount",        0, عدم, 'R' },
	{ "blockdevice",    0, عدم, 'b' },
	{ "mountargs",      0, عدم, 'm' },
	{ "options",        0, عدم, 'o' },
	{ "passno",         1, عدم, 'p' },
	{ "fstype",         1, عدم, 't' },
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
ثابت حرف *usagestring = عدم;

#که_تعریف_شوی HAVE_GETMNTENT
ساكن بنية mntent *
getmntfile(ثابت حرف *file)
{
	بنية mntent *ent;
	FILE *fp;

	پیل_ENT;
	طالما ((ent = getmntent(fp)))
		إذا (strcmp(file, ent->mnt_dir) == 0)
			اكسر;
	پای_ټکی_ENT;

	أرجع ent;
}
#د_که_پای

خارجي ثابت حرف *applet;

ساكن صحيح
do_mount(بنية ENT *ent, bool remount)
{
	حرف *argv[10];
	pid_t pid;
	صحيح حالت, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	إذا (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = عدم;
	} وإلا {
#که_تعریف_شوی __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = عدم;
#وإلا
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = عدم;
#د_که_پای
	}
	err = posix_spawnp(&pid, argv[0], عدم, عدم, argv, environ);
	إذا (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	طالما (waitpid(pid, &حالت, 0) < 0 && errno == EINTR);
	إذا (WIFEXITED(حالت))
		أرجع WEXITSTATUS(حالت);
	وإلا
		أرجع -1;
}

#تعریف_کړه OUTPUT_FILE      (1 << 1)
#تعریف_کړه OUTPUT_MOUNTARGS (1 << 2)
#تعریف_کړه OUTPUT_OPTIONS   (1 << 3)
#تعریف_کړه OUTPUT_PASSNO    (1 << 4)
#تعریف_کړه OUTPUT_BLOCKDEV  (1 << 5)
#تعریف_کړه OUTPUT_MOUNT     (1 << 6)
#تعریف_کړه OUTPUT_REMOUNT   (1 << 7)

صحيح اصلي(صحيح argc, حرف **argv)
{
	بنية ENT *ent;
	صحيح پایله = EXIT_SUCCESS;
	حرف *token;
	صحيح i, p;
	صحيح opt;
	صحيح output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#که_تعریف_شوی HAVE_GETMNTENT
	FILE *fp;
#د_که_پای

	/* fail if there is no /etc/fstab */
	إذا (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	طالما ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
		حالة 'M':
			output = OUTPUT_MOUNT;
			اكسر;
		حالة 'R':
			output = OUTPUT_REMOUNT;
			اكسر;
		حالة 'b':
			output = OUTPUT_BLOCKDEV;
			اكسر;
		حالة 'o':
			output = OUTPUT_OPTIONS;
			اكسر;
		حالة 'm':
			output = OUTPUT_MOUNTARGS;
			اكسر;

		حالة 'p':
			بدل (optarg[0]) {
			حالة '=':
			حالة '<':
			حالة '>':
				إذا (له_مزي_ولوله(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				پیل_ENT;
				طالما ((ent = GET_ENT)) {
					إذا (strcmp(ENT_FILE(ent), "none") == 0)
						استمر;
					p = ENT_PASS(ent);
					إذا ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_جمع_کړه(files,
						    ENT_FILE(ent));
				}
				پای_ټکی_ENT;
				اكسر;

			افتراضي:
				rc_stringlist_جمع_کړه(files, optarg);
				output = OUTPUT_PASSNO;
				اكسر;
			}
			اكسر;

		حالة 't':
			filtered = true;
			طالما ((token = strsep(&optarg, ","))) {
				پیل_ENT;
				طالما ((ent = GET_ENT))
					إذا (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_جمع_کړه(files,
						    ENT_FILE(ent));
				پای_ټکی_ENT;
			}
			اكسر;

		case_RC_COMMON_GETOPT
		}
	}

	إذا (optind < argc) {
		إذا (TAILQ_لومړی(files)) {
			TAILQ_FOREACH_خوندي(file, files, entries, file_np) {
				لكل (i = optind; i < argc; i++)
					إذا (strcmp(argv[i], file->ارزښت) == 0)
						اكسر;
				إذا (i >= argc)
					rc_stringlist_delete(files,
					    file->ارزښت);
			}
		} وإلا {
			طالما (optind < argc)
				rc_stringlist_جمع_کړه(files, argv[optind++]);
		}
	} وإلا إذا (!filtered) {
		پیل_ENT;
		طالما ((ent = GET_ENT))
			rc_stringlist_جمع_کړه(files, ENT_FILE(ent));
		پای_ټکی_ENT;

		إذا (!TAILQ_لومړی(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	إذا (!TAILQ_لومړی(files)) {
		rc_stringlist_free(files);
		أرجع (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	پیل_ENT;
	TAILQ_FOREACH(file, files, entries) {
		إذا (!(ent = GET_ENT_FILE(file->ارزښت))) {
			پایله = EXIT_FAILURE;
			استمر;
		}

		/* mount or remount? */
		بدل (output) {
		حالة OUTPUT_MOUNT:
			پایله += do_mount(ent, false);
			اكسر;

		حالة OUTPUT_REMOUNT:
			پایله += do_mount(ent, true);
			اكسر;
		}

		/* No point in outputting if quiet */
		إذا (rc_yesno(getenv("EINFO_QUIET")))
			استمر;

		بدل (output) {
		حالة OUTPUT_BLOCKDEV:
			چاپ_کړه("%s\n", ENT_BLOCKDEVICE(ent));
			اكسر;

		حالة OUTPUT_MOUNTARGS:
			چاپ_کړه("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->ارزښت);
			اكسر;

		حالة OUTPUT_OPTIONS:
			چاپ_کړه("%s\n", ENT_OPTS(ent));
			اكسر;

		حالة OUTPUT_FILE:
			چاپ_کړه("%s\n", file->ارزښت);
			اكسر;

		حالة OUTPUT_PASSNO:
			چاپ_کړه("%d\n", ENT_PASS(ent));
			اكسر;
		}
	}
	پای_ټکی_ENT;

	rc_stringlist_free(files);
	وتل(پایله);
	/* NOTREACHED */
}
