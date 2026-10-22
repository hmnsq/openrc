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

#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <spawn.h>

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
#இல்லையெனில்
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

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "helpers.h"

வெளி எழுத்து **environ;

மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *extraopts = வெற்று;
மாறிலி எழுத்து getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "mount",          0, வெற்று, 'M' },
	{ "remount",        0, வெற்று, 'R' },
	{ "blockdevice",    0, வெற்று, 'b' },
	{ "mountargs",      0, வெற்று, 'm' },
	{ "options",        0, வெற்று, 'o' },
	{ "passno",         1, வெற்று, 'p' },
	{ "fstype",         1, வெற்று, 't' },
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
மாறிலி எழுத்து *usagestring = வெற்று;

#ifdef HAVE_GETMNTENT
நிலையான கட்டமைப்பு mntent *
getmntfile(மாறிலி எழுத்து *file)
{
	கட்டமைப்பு mntent *ent;
	FILE *fp;

	START_ENT;
	போது ((ent = getmntent(fp)))
		எனில் (strcmp(file, ent->mnt_dir) == 0)
			நிறுத்து;
	END_ENT;

	திரும்பு ent;
}
#endif

வெளி மாறிலி எழுத்து *applet;

நிலையான முழு
do_mount(கட்டமைப்பு ENT *ent, bool remount)
{
	எழுத்து *argv[10];
	pid_t pid;
	முழு status, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	எனில் (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = வெற்று;
	} இல்லையெனில் {
#ifdef __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = வெற்று;
#இல்லையெனில்
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = வெற்று;
#endif
	}
	err = posix_spawnp(&pid, argv[0], வெற்று, வெற்று, argv, environ);
	எனில் (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	போது (waitpid(pid, &status, 0) < 0 && errno == EINTR);
	எனில் (WIFEXITED(status))
		திரும்பு WEXITSTATUS(status);
	இல்லையெனில்
		திரும்பு -1;
}

#define OUTPUT_FILE      (1 << 1)
#define OUTPUT_MOUNTARGS (1 << 2)
#define OUTPUT_OPTIONS   (1 << 3)
#define OUTPUT_PASSNO    (1 << 4)
#define OUTPUT_BLOCKDEV  (1 << 5)
#define OUTPUT_MOUNT     (1 << 6)
#define OUTPUT_REMOUNT   (1 << 7)

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	கட்டமைப்பு ENT *ent;
	முழு result = EXIT_SUCCESS;
	எழுத்து *token;
	முழு i, p;
	முழு opt;
	முழு output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#ifdef HAVE_GETMNTENT
	FILE *fp;
#endif

	/* fail if there is no /etc/fstab */
	எனில் (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	போது ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (முழு *) 0)) != -1)
	{
		தேர்வு (opt) {
		நிலை 'M':
			output = OUTPUT_MOUNT;
			நிறுத்து;
		நிலை 'R':
			output = OUTPUT_REMOUNT;
			நிறுத்து;
		நிலை 'b':
			output = OUTPUT_BLOCKDEV;
			நிறுத்து;
		நிலை 'o':
			output = OUTPUT_OPTIONS;
			நிறுத்து;
		நிலை 'm':
			output = OUTPUT_MOUNTARGS;
			நிறுத்து;

		நிலை 'p':
			தேர்வு (optarg[0]) {
			நிலை '=':
			நிலை '<':
			நிலை '>':
				எனில் (sscanf(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				START_ENT;
				போது ((ent = GET_ENT)) {
					எனில் (strcmp(ENT_FILE(ent), "none") == 0)
						தொடர்;
					p = ENT_PASS(ent);
					எனில் ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				}
				END_ENT;
				நிறுத்து;

			இயல்பு:
				rc_stringlist_add(files, optarg);
				output = OUTPUT_PASSNO;
				நிறுத்து;
			}
			நிறுத்து;

		நிலை 't':
			filtered = true;
			போது ((token = strsep(&optarg, ","))) {
				START_ENT;
				போது ((ent = GET_ENT))
					எனில் (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				END_ENT;
			}
			நிறுத்து;

		case_RC_COMMON_GETOPT
		}
	}

	எனில் (optind < argc) {
		எனில் (TAILQ_FIRST(files)) {
			TAILQ_FOREACH_SAFE(file, files, entries, file_np) {
				க்கு (i = optind; i < argc; i++)
					எனில் (strcmp(argv[i], file->மதிப்பு) == 0)
						நிறுத்து;
				எனில் (i >= argc)
					rc_stringlist_delete(files,
					    file->மதிப்பு);
			}
		} இல்லையெனில் {
			போது (optind < argc)
				rc_stringlist_add(files, argv[optind++]);
		}
	} இல்லையெனில் எனில் (!filtered) {
		START_ENT;
		போது ((ent = GET_ENT))
			rc_stringlist_add(files, ENT_FILE(ent));
		END_ENT;

		எனில் (!TAILQ_FIRST(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	எனில் (!TAILQ_FIRST(files)) {
		rc_stringlist_free(files);
		திரும்பு (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	START_ENT;
	TAILQ_FOREACH(file, files, entries) {
		எனில் (!(ent = GET_ENT_FILE(file->மதிப்பு))) {
			result = EXIT_FAILURE;
			தொடர்;
		}

		/* mount or remount? */
		தேர்வு (output) {
		நிலை OUTPUT_MOUNT:
			result += do_mount(ent, false);
			நிறுத்து;

		நிலை OUTPUT_REMOUNT:
			result += do_mount(ent, true);
			நிறுத்து;
		}

		/* No point in outputting if quiet */
		எனில் (rc_yesno(getenv("EINFO_QUIET")))
			தொடர்;

		தேர்வு (output) {
		நிலை OUTPUT_BLOCKDEV:
			printf("%s\n", ENT_BLOCKDEVICE(ent));
			நிறுத்து;

		நிலை OUTPUT_MOUNTARGS:
			printf("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->மதிப்பு);
			நிறுத்து;

		நிலை OUTPUT_OPTIONS:
			printf("%s\n", ENT_OPTS(ent));
			நிறுத்து;

		நிலை OUTPUT_FILE:
			printf("%s\n", file->மதிப்பு);
			நிறுத்து;

		நிலை OUTPUT_PASSNO:
			printf("%d\n", ENT_PASS(ent));
			நிறுத்து;
		}
	}
	END_ENT;

	rc_stringlist_free(files);
	exit(result);
	/* NOTREACHED */
}
