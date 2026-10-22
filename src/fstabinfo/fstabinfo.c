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

#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <spawn.h>

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
#അല്ലെങ്കിൽ
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

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "_usage.h"
#അന്തര്ഭാവ "helpers.h"

ബാഹ്യ അക്ഷരം **environ;

സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;
സ്ഥിരം അക്ഷരം *extraopts = ശൂന്യ;
സ്ഥിരം അക്ഷരം getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
സ്ഥിരം ഘടന option longopts[] = {
	{ "mount",          0, ശൂന്യ, 'M' },
	{ "remount",        0, ശൂന്യ, 'R' },
	{ "blockdevice",    0, ശൂന്യ, 'b' },
	{ "mountargs",      0, ശൂന്യ, 'm' },
	{ "options",        0, ശൂന്യ, 'o' },
	{ "passno",         1, ശൂന്യ, 'p' },
	{ "fstype",         1, ശൂന്യ, 't' },
	longopts_COMMON
};
സ്ഥിരം അക്ഷരം * സ്ഥിരം longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
സ്ഥിരം അക്ഷരം *usagestring = ശൂന്യ;

#ifdef HAVE_GETMNTENT
സ്ഥിതം ഘടന mntent *
getmntfile(സ്ഥിരം അക്ഷരം *file)
{
	ഘടന mntent *ent;
	FILE *fp;

	START_ENT;
	ആകുമ്പോൾ ((ent = getmntent(fp)))
		എങ്കിൽ (strcmp(file, ent->mnt_dir) == 0)
			നിർത്തുക;
	END_ENT;

	തിരികെ ent;
}
#endif

ബാഹ്യ സ്ഥിരം അക്ഷരം *applet;

സ്ഥിതം പൂർണ്ണസംഖ്യ
do_mount(ഘടന ENT *ent, bool remount)
{
	അക്ഷരം *argv[10];
	pid_t pid;
	പൂർണ്ണസംഖ്യ status, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	എങ്കിൽ (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = ശൂന്യ;
	} അല്ലെങ്കിൽ {
#ifdef __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = ശൂന്യ;
#അല്ലെങ്കിൽ
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = ശൂന്യ;
#endif
	}
	err = posix_spawnp(&pid, argv[0], ശൂന്യ, ശൂന്യ, argv, environ);
	എങ്കിൽ (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	ആകുമ്പോൾ (waitpid(pid, &status, 0) < 0 && errno == EINTR);
	എങ്കിൽ (WIFEXITED(status))
		തിരികെ WEXITSTATUS(status);
	അല്ലെങ്കിൽ
		തിരികെ -1;
}

#define OUTPUT_FILE      (1 << 1)
#define OUTPUT_MOUNTARGS (1 << 2)
#define OUTPUT_OPTIONS   (1 << 3)
#define OUTPUT_PASSNO    (1 << 4)
#define OUTPUT_BLOCKDEV  (1 << 5)
#define OUTPUT_MOUNT     (1 << 6)
#define OUTPUT_REMOUNT   (1 << 7)

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	ഘടന ENT *ent;
	പൂർണ്ണസംഖ്യ result = EXIT_SUCCESS;
	അക്ഷരം *token;
	പൂർണ്ണസംഖ്യ i, p;
	പൂർണ്ണസംഖ്യ opt;
	പൂർണ്ണസംഖ്യ output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#ifdef HAVE_GETMNTENT
	FILE *fp;
#endif

	/* fail if there is no /etc/fstab */
	എങ്കിൽ (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	ആകുമ്പോൾ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (പൂർണ്ണസംഖ്യ *) 0)) != -1)
	{
		തിരഞ്ഞെടുപ്പ് (opt) {
		അവസ്ഥ 'M':
			output = OUTPUT_MOUNT;
			നിർത്തുക;
		അവസ്ഥ 'R':
			output = OUTPUT_REMOUNT;
			നിർത്തുക;
		അവസ്ഥ 'b':
			output = OUTPUT_BLOCKDEV;
			നിർത്തുക;
		അവസ്ഥ 'o':
			output = OUTPUT_OPTIONS;
			നിർത്തുക;
		അവസ്ഥ 'm':
			output = OUTPUT_MOUNTARGS;
			നിർത്തുക;

		അവസ്ഥ 'p':
			തിരഞ്ഞെടുപ്പ് (optarg[0]) {
			അവസ്ഥ '=':
			അവസ്ഥ '<':
			അവസ്ഥ '>':
				എങ്കിൽ (sscanf(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				START_ENT;
				ആകുമ്പോൾ ((ent = GET_ENT)) {
					എങ്കിൽ (strcmp(ENT_FILE(ent), "none") == 0)
						തുടരുക;
					p = ENT_PASS(ent);
					എങ്കിൽ ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				}
				END_ENT;
				നിർത്തുക;

			സ്ഥിരസ്ഥിതി:
				rc_stringlist_add(files, optarg);
				output = OUTPUT_PASSNO;
				നിർത്തുക;
			}
			നിർത്തുക;

		അവസ്ഥ 't':
			filtered = true;
			ആകുമ്പോൾ ((token = strsep(&optarg, ","))) {
				START_ENT;
				ആകുമ്പോൾ ((ent = GET_ENT))
					എങ്കിൽ (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				END_ENT;
			}
			നിർത്തുക;

		case_RC_COMMON_GETOPT
		}
	}

	എങ്കിൽ (optind < argc) {
		എങ്കിൽ (TAILQ_FIRST(files)) {
			TAILQ_FOREACH_SAFE(file, files, entries, file_np) {
				വേണ്ടി (i = optind; i < argc; i++)
					എങ്കിൽ (strcmp(argv[i], file->മൂല്യം) == 0)
						നിർത്തുക;
				എങ്കിൽ (i >= argc)
					rc_stringlist_delete(files,
					    file->മൂല്യം);
			}
		} അല്ലെങ്കിൽ {
			ആകുമ്പോൾ (optind < argc)
				rc_stringlist_add(files, argv[optind++]);
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (!filtered) {
		START_ENT;
		ആകുമ്പോൾ ((ent = GET_ENT))
			rc_stringlist_add(files, ENT_FILE(ent));
		END_ENT;

		എങ്കിൽ (!TAILQ_FIRST(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	എങ്കിൽ (!TAILQ_FIRST(files)) {
		rc_stringlist_free(files);
		തിരികെ (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	START_ENT;
	TAILQ_FOREACH(file, files, entries) {
		എങ്കിൽ (!(ent = GET_ENT_FILE(file->മൂല്യം))) {
			result = EXIT_FAILURE;
			തുടരുക;
		}

		/* mount or remount? */
		തിരഞ്ഞെടുപ്പ് (output) {
		അവസ്ഥ OUTPUT_MOUNT:
			result += do_mount(ent, false);
			നിർത്തുക;

		അവസ്ഥ OUTPUT_REMOUNT:
			result += do_mount(ent, true);
			നിർത്തുക;
		}

		/* No point in outputting if quiet */
		എങ്കിൽ (rc_yesno(getenv("EINFO_QUIET")))
			തുടരുക;

		തിരഞ്ഞെടുപ്പ് (output) {
		അവസ്ഥ OUTPUT_BLOCKDEV:
			printf("%s\n", ENT_BLOCKDEVICE(ent));
			നിർത്തുക;

		അവസ്ഥ OUTPUT_MOUNTARGS:
			printf("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->മൂല്യം);
			നിർത്തുക;

		അവസ്ഥ OUTPUT_OPTIONS:
			printf("%s\n", ENT_OPTS(ent));
			നിർത്തുക;

		അവസ്ഥ OUTPUT_FILE:
			printf("%s\n", file->മൂല്യം);
			നിർത്തുക;

		അവസ്ഥ OUTPUT_PASSNO:
			printf("%d\n", ENT_PASS(ent));
			നിർത്തുക;
		}
	}
	END_ENT;

	rc_stringlist_free(files);
	exit(result);
	/* NOTREACHED */
}
