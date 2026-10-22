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

#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <spawn.h>

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
#अन्यथा
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

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"

बाह्य वर्ण **environ;

ध्रुव वर्ण *applet = रिक्त;
ध्रुव वर्ण *extraopts = रिक्त;
ध्रुव वर्ण getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
ध्रुव संरचना option longopts[] = {
	{ "mount",          0, रिक्त, 'M' },
	{ "remount",        0, रिक्त, 'R' },
	{ "blockdevice",    0, रिक्त, 'b' },
	{ "mountargs",      0, रिक्त, 'm' },
	{ "options",        0, रिक्त, 'o' },
	{ "passno",         1, रिक्त, 'p' },
	{ "fstype",         1, रिक्त, 't' },
	longopts_COMMON
};
ध्रुव वर्ण * ध्रुव longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
ध्रुव वर्ण *usagestring = रिक्त;

#ifdef HAVE_GETMNTENT
स्थिर संरचना mntent *
getmntfile(ध्रुव वर्ण *file)
{
	संरचना mntent *ent;
	FILE *fp;

	START_ENT;
	यावत् ((ent = getmntent(fp)))
		यदि (strcmp(file, ent->mnt_dir) == 0)
			भङ्ग;
	END_ENT;

	प्रत्यागम ent;
}
#endif

बाह्य ध्रुव वर्ण *applet;

स्थिर पूर्णाङ्क
do_mount(संरचना ENT *ent, bool remount)
{
	वर्ण *argv[10];
	pid_t pid;
	पूर्णाङ्क status, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	यदि (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = रिक्त;
	} अन्यथा {
#ifdef __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = रिक्त;
#अन्यथा
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = रिक्त;
#endif
	}
	err = posix_spawnp(&pid, argv[0], रिक्त, रिक्त, argv, environ);
	यदि (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	यावत् (waitpid(pid, &status, 0) < 0 && errno == EINTR);
	यदि (WIFEXITED(status))
		प्रत्यागम WEXITSTATUS(status);
	अन्यथा
		प्रत्यागम -1;
}

#define OUTPUT_FILE      (1 << 1)
#define OUTPUT_MOUNTARGS (1 << 2)
#define OUTPUT_OPTIONS   (1 << 3)
#define OUTPUT_PASSNO    (1 << 4)
#define OUTPUT_BLOCKDEV  (1 << 5)
#define OUTPUT_MOUNT     (1 << 6)
#define OUTPUT_REMOUNT   (1 << 7)

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	संरचना ENT *ent;
	पूर्णाङ्क result = EXIT_SUCCESS;
	वर्ण *token;
	पूर्णाङ्क i, p;
	पूर्णाङ्क opt;
	पूर्णाङ्क output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#ifdef HAVE_GETMNTENT
	FILE *fp;
#endif

	/* fail if there is no /etc/fstab */
	यदि (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	यावत् ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णाङ्क *) 0)) != -1)
	{
		विकल्प (opt) {
		पक्ष 'M':
			output = OUTPUT_MOUNT;
			भङ्ग;
		पक्ष 'R':
			output = OUTPUT_REMOUNT;
			भङ्ग;
		पक्ष 'b':
			output = OUTPUT_BLOCKDEV;
			भङ्ग;
		पक्ष 'o':
			output = OUTPUT_OPTIONS;
			भङ्ग;
		पक्ष 'm':
			output = OUTPUT_MOUNTARGS;
			भङ्ग;

		पक्ष 'p':
			विकल्प (optarg[0]) {
			पक्ष '=':
			पक्ष '<':
			पक्ष '>':
				यदि (sscanf(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				START_ENT;
				यावत् ((ent = GET_ENT)) {
					यदि (strcmp(ENT_FILE(ent), "none") == 0)
						अनुवर्त;
					p = ENT_PASS(ent);
					यदि ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				}
				END_ENT;
				भङ्ग;

			स्वभाव:
				rc_stringlist_add(files, optarg);
				output = OUTPUT_PASSNO;
				भङ्ग;
			}
			भङ्ग;

		पक्ष 't':
			filtered = true;
			यावत् ((token = strsep(&optarg, ","))) {
				START_ENT;
				यावत् ((ent = GET_ENT))
					यदि (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				END_ENT;
			}
			भङ्ग;

		case_RC_COMMON_GETOPT
		}
	}

	यदि (optind < argc) {
		यदि (TAILQ_FIRST(files)) {
			TAILQ_FOREACH_SAFE(file, files, entries, file_np) {
				प्रति (i = optind; i < argc; i++)
					यदि (strcmp(argv[i], file->मूल्य) == 0)
						भङ्ग;
				यदि (i >= argc)
					rc_stringlist_delete(files,
					    file->मूल्य);
			}
		} अन्यथा {
			यावत् (optind < argc)
				rc_stringlist_add(files, argv[optind++]);
		}
	} अन्यथा यदि (!filtered) {
		START_ENT;
		यावत् ((ent = GET_ENT))
			rc_stringlist_add(files, ENT_FILE(ent));
		END_ENT;

		यदि (!TAILQ_FIRST(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	यदि (!TAILQ_FIRST(files)) {
		rc_stringlist_free(files);
		प्रत्यागम (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	START_ENT;
	TAILQ_FOREACH(file, files, entries) {
		यदि (!(ent = GET_ENT_FILE(file->मूल्य))) {
			result = EXIT_FAILURE;
			अनुवर्त;
		}

		/* mount or remount? */
		विकल्प (output) {
		पक्ष OUTPUT_MOUNT:
			result += do_mount(ent, false);
			भङ्ग;

		पक्ष OUTPUT_REMOUNT:
			result += do_mount(ent, true);
			भङ्ग;
		}

		/* No point in outputting if quiet */
		यदि (rc_yesno(getenv("EINFO_QUIET")))
			अनुवर्त;

		विकल्प (output) {
		पक्ष OUTPUT_BLOCKDEV:
			printf("%s\n", ENT_BLOCKDEVICE(ent));
			भङ्ग;

		पक्ष OUTPUT_MOUNTARGS:
			printf("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->मूल्य);
			भङ्ग;

		पक्ष OUTPUT_OPTIONS:
			printf("%s\n", ENT_OPTS(ent));
			भङ्ग;

		पक्ष OUTPUT_FILE:
			printf("%s\n", file->मूल्य);
			भङ्ग;

		पक्ष OUTPUT_PASSNO:
			printf("%d\n", ENT_PASS(ent));
			भङ्ग;
		}
	}
	END_ENT;

	rc_stringlist_free(files);
	exit(result);
	/* NOTREACHED */
}
