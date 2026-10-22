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

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spawn.h>

/* Yay for linux and its non liking of POSIX functions.
   Okay, we could use getfsent but the man page says use getmntent instead
   AND we don't have getfsent on uclibc or dietlibc for some odd reason. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
#  define HAVE_GETMNTENT
#  include <mntent.h>

#  define ENT mntent
#  define ਸ਼ੁਰੂ_ENT fp = setmntent ("/etc/fstab", "r");
#  define GET_ENT getmntent (fp)
#  define GET_ENT_FILE(_name) getmntfile (_name)
#  define ਸਿਰਾ_ENT endmntent (fp)
#  define ENT_BLOCKDEVICE(_ent) (_ent)->mnt_fsname
#  define ENT_FILE(_ent) (_ent)->mnt_dir
#  define ENT_TYPE(_ent) (_ent)->mnt_type
#  define ENT_OPTS(_ent) (_ent)->mnt_opts
#  define ENT_PASS(_ent) (_ent)->mnt_passno
#ਨਹੀਂਤਾਂ
#  define HAVE_GETFSENT
#  include <fstab.h>

#  define ENT fstab
#  define ਸ਼ੁਰੂ_ENT
#  define GET_ENT getfsent ()
#  define GET_ENT_FILE(_name) getfsfile (_name)
#  define ਸਿਰਾ_ENT endfsent ()
#  define ENT_BLOCKDEVICE(_ent) (_ent)->fs_spec
#  define ENT_TYPE(_ent) (_ent)->fs_vfstype
#  define ENT_FILE(_ent) (_ent)->fs_file
#  define ENT_OPTS(_ent) (_ent)->fs_mntops
#  define ENT_PASS(_ent) (_ent)->fs_passno
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਬਾਹਰੀ ਅੱਖਰ **environ;

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ *extraopts = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "mount",          0, ਨਿਰਾ, 'M' },
	{ "remount",        0, ਨਿਰਾ, 'R' },
	{ "blockdevice",    0, ਨਿਰਾ, 'b' },
	{ "mountargs",      0, ਨਿਰਾ, 'm' },
	{ "options",        0, ਨਿਰਾ, 'o' },
	{ "passno",         1, ਨਿਰਾ, 'p' },
	{ "fstype",         1, ਨਿਰਾ, 't' },
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
ਅਚਲ ਅੱਖਰ *usagestring = ਨਿਰਾ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_GETMNTENT
ਸਥਿਰ ਬਣਤਰ mntent *
getmntfile(ਅਚਲ ਅੱਖਰ *file)
{
	ਬਣਤਰ mntent *ent;
	FILE *fp;

	ਸ਼ੁਰੂ_ENT;
	ਜਦੋਂ ((ent = getmntent(fp)))
		ਜੇ (strcmp(file, ent->mnt_dir) == 0)
			ਰੋਕੋ;
	ਸਿਰਾ_ENT;

	ਵਾਪਸ ent;
}
#ਜੇ_ਅੰਤ

ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *applet;

ਸਥਿਰ ਪੂਰਨ
do_mount(ਬਣਤਰ ENT *ent, bool remount)
{
	ਅੱਖਰ *argv[10];
	pid_t pid;
	ਪੂਰਨ ਸਥਿਤੀ, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	ਜੇ (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = ਨਿਰਾ;
	} ਨਹੀਂਤਾਂ {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = ਨਿਰਾ;
#ਨਹੀਂਤਾਂ
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = ਨਿਰਾ;
#ਜੇ_ਅੰਤ
	}
	err = posix_spawnp(&pid, argv[0], ਨਿਰਾ, ਨਿਰਾ, argv, environ);
	ਜੇ (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	ਜਦੋਂ (waitpid(pid, &ਸਥਿਤੀ, 0) < 0 && errno == EINTR);
	ਜੇ (WIFEXITED(ਸਥਿਤੀ))
		ਵਾਪਸ WEXITSTATUS(ਸਥਿਤੀ);
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ -1;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OUTPUT_FILE      (1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OUTPUT_MOUNTARGS (1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OUTPUT_OPTIONS   (1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OUTPUT_PASSNO    (1 << 4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OUTPUT_BLOCKDEV  (1 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OUTPUT_MOUNT     (1 << 6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OUTPUT_REMOUNT   (1 << 7)

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਬਣਤਰ ENT *ent;
	ਪੂਰਨ ਨਤੀਜਾ = EXIT_SUCCESS;
	ਅੱਖਰ *token;
	ਪੂਰਨ i, p;
	ਪੂਰਨ opt;
	ਪੂਰਨ output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_GETMNTENT
	FILE *fp;
#ਜੇ_ਅੰਤ

	/* fail if there is no /etc/fstab */
	ਜੇ (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ਪੂਰਨ *) 0)) != -1)
	{
		ਚੋਣ (opt) {
		ਹਾਲਤ 'M':
			output = OUTPUT_MOUNT;
			ਰੋਕੋ;
		ਹਾਲਤ 'R':
			output = OUTPUT_REMOUNT;
			ਰੋਕੋ;
		ਹਾਲਤ 'b':
			output = OUTPUT_BLOCKDEV;
			ਰੋਕੋ;
		ਹਾਲਤ 'o':
			output = OUTPUT_OPTIONS;
			ਰੋਕੋ;
		ਹਾਲਤ 'm':
			output = OUTPUT_MOUNTARGS;
			ਰੋਕੋ;

		ਹਾਲਤ 'p':
			ਚੋਣ (optarg[0]) {
			ਹਾਲਤ '=':
			ਹਾਲਤ '<':
			ਹਾਲਤ '>':
				ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				ਸ਼ੁਰੂ_ENT;
				ਜਦੋਂ ((ent = GET_ENT)) {
					ਜੇ (strcmp(ENT_FILE(ent), "none") == 0)
						ਜਾਰੀ;
					p = ENT_PASS(ent);
					ਜੇ ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_ਜੋੜੋ(files,
						    ENT_FILE(ent));
				}
				ਸਿਰਾ_ENT;
				ਰੋਕੋ;

			ਮੂਲ:
				rc_stringlist_ਜੋੜੋ(files, optarg);
				output = OUTPUT_PASSNO;
				ਰੋਕੋ;
			}
			ਰੋਕੋ;

		ਹਾਲਤ 't':
			filtered = true;
			ਜਦੋਂ ((token = strsep(&optarg, ","))) {
				ਸ਼ੁਰੂ_ENT;
				ਜਦੋਂ ((ent = GET_ENT))
					ਜੇ (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_ਜੋੜੋ(files,
						    ENT_FILE(ent));
				ਸਿਰਾ_ENT;
			}
			ਰੋਕੋ;

		case_RC_COMMON_GETOPT
		}
	}

	ਜੇ (optind < argc) {
		ਜੇ (TAILQ_ਪਹਿਲਾ(files)) {
			TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(file, files, entries, file_np) {
				ਲਈ (i = optind; i < argc; i++)
					ਜੇ (strcmp(argv[i], file->ਮੁੱਲ) == 0)
						ਰੋਕੋ;
				ਜੇ (i >= argc)
					rc_stringlist_delete(files,
					    file->ਮੁੱਲ);
			}
		} ਨਹੀਂਤਾਂ {
			ਜਦੋਂ (optind < argc)
				rc_stringlist_ਜੋੜੋ(files, argv[optind++]);
		}
	} ਨਹੀਂਤਾਂ ਜੇ (!filtered) {
		ਸ਼ੁਰੂ_ENT;
		ਜਦੋਂ ((ent = GET_ENT))
			rc_stringlist_ਜੋੜੋ(files, ENT_FILE(ent));
		ਸਿਰਾ_ENT;

		ਜੇ (!TAILQ_ਪਹਿਲਾ(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	ਜੇ (!TAILQ_ਪਹਿਲਾ(files)) {
		rc_stringlist_free(files);
		ਵਾਪਸ (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	ਸ਼ੁਰੂ_ENT;
	TAILQ_FOREACH(file, files, entries) {
		ਜੇ (!(ent = GET_ENT_FILE(file->ਮੁੱਲ))) {
			ਨਤੀਜਾ = EXIT_FAILURE;
			ਜਾਰੀ;
		}

		/* mount or remount? */
		ਚੋਣ (output) {
		ਹਾਲਤ OUTPUT_MOUNT:
			ਨਤੀਜਾ += do_mount(ent, false);
			ਰੋਕੋ;

		ਹਾਲਤ OUTPUT_REMOUNT:
			ਨਤੀਜਾ += do_mount(ent, true);
			ਰੋਕੋ;
		}

		/* No point in outputting if quiet */
		ਜੇ (rc_yesno(getenv("EINFO_QUIET")))
			ਜਾਰੀ;

		ਚੋਣ (output) {
		ਹਾਲਤ OUTPUT_BLOCKDEV:
			ਲਿਖੋ("%s\n", ENT_BLOCKDEVICE(ent));
			ਰੋਕੋ;

		ਹਾਲਤ OUTPUT_MOUNTARGS:
			ਲਿਖੋ("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->ਮੁੱਲ);
			ਰੋਕੋ;

		ਹਾਲਤ OUTPUT_OPTIONS:
			ਲਿਖੋ("%s\n", ENT_OPTS(ent));
			ਰੋਕੋ;

		ਹਾਲਤ OUTPUT_FILE:
			ਲਿਖੋ("%s\n", file->ਮੁੱਲ);
			ਰੋਕੋ;

		ਹਾਲਤ OUTPUT_PASSNO:
			ਲਿਖੋ("%d\n", ENT_PASS(ent));
			ਰੋਕੋ;
		}
	}
	ਸਿਰਾ_ENT;

	rc_stringlist_free(files);
	ਬਾਹਰ_ਨਿਕਲੋ(ਨਤੀਜਾ);
	/* NOTREACHED */
}
