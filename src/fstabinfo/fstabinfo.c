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

बाह्य अक्षर **environ;

स्थिरांक अक्षर *applet = रिक्त;
स्थिरांक अक्षर *extraopts = रिक्त;
स्थिरांक अक्षर getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
स्थिरांक संरचना option longopts[] = {
	{ "mount",          0, रिक्त, 'M' },
	{ "remount",        0, रिक्त, 'R' },
	{ "blockdevice",    0, रिक्त, 'b' },
	{ "mountargs",      0, रिक्त, 'm' },
	{ "options",        0, रिक्त, 'o' },
	{ "passno",         1, रिक्त, 'p' },
	{ "fstype",         1, रिक्त, 't' },
	longopts_COMMON
};
स्थिरांक अक्षर * स्थिरांक longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
स्थिरांक अक्षर *usagestring = रिक्त;

#ifdef HAVE_GETMNTENT
स्थिर संरचना mntent *
getmntfile(स्थिरांक अक्षर *file)
{
	संरचना mntent *ent;
	खाता *fp;

	START_ENT;
	पर्यंत ((ent = getmntent(fp)))
		जर (म_भेद(file, ent->mnt_dir) == 0)
			थांबा;
	END_ENT;

	परत ent;
}
#endif

बाह्य स्थिरांक अक्षर *applet;

स्थिर पूर्णांक
do_mount(संरचना ENT *ent, bool remount)
{
	अक्षर *argv[10];
	pid_t pid;
	पूर्णांक status, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	जर (!remount) {
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
	जर (err)
		eerrorx("%s: posix_spawnp: %s", applet, म_त्रुटि(err));
	पर्यंत (waitpid(pid, &status, 0) < 0 && त्रुटि_सं == EINTR);
	जर (WIFEXITED(status))
		परत WEXITSTATUS(status);
	अन्यथा
		परत -1;
}

#define OUTPUT_FILE      (1 << 1)
#define OUTPUT_MOUNTARGS (1 << 2)
#define OUTPUT_OPTIONS   (1 << 3)
#define OUTPUT_PASSNO    (1 << 4)
#define OUTPUT_BLOCKDEV  (1 << 5)
#define OUTPUT_MOUNT     (1 << 6)
#define OUTPUT_REMOUNT   (1 << 7)

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	संरचना ENT *ent;
	पूर्णांक result = निकास_सफल;
	अक्षर *token;
	पूर्णांक i, p;
	पूर्णांक opt;
	पूर्णांक output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#ifdef HAVE_GETMNTENT
	खाता *fp;
#endif

	/* fail if there is no /etc/fstab */
	जर (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	पर्यंत ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		निवड (opt) {
		स्थिती 'M':
			output = OUTPUT_MOUNT;
			थांबा;
		स्थिती 'R':
			output = OUTPUT_REMOUNT;
			थांबा;
		स्थिती 'b':
			output = OUTPUT_BLOCKDEV;
			थांबा;
		स्थिती 'o':
			output = OUTPUT_OPTIONS;
			थांबा;
		स्थिती 'm':
			output = OUTPUT_MOUNTARGS;
			थांबा;

		स्थिती 'p':
			निवड (optarg[0]) {
			स्थिती '=':
			स्थिती '<':
			स्थिती '>':
				जर (माला_पूछो(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				START_ENT;
				पर्यंत ((ent = GET_ENT)) {
					जर (म_भेद(ENT_FILE(ent), "none") == 0)
						सुरूठेवा;
					p = ENT_PASS(ent);
					जर ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				}
				END_ENT;
				थांबा;

			पूर्वनिर्धारित:
				rc_stringlist_add(files, optarg);
				output = OUTPUT_PASSNO;
				थांबा;
			}
			थांबा;

		स्थिती 't':
			filtered = true;
			पर्यंत ((token = strsep(&optarg, ","))) {
				START_ENT;
				पर्यंत ((ent = GET_ENT))
					जर (म_भेद(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				END_ENT;
			}
			थांबा;

		case_RC_COMMON_GETOPT
		}
	}

	जर (optind < argc) {
		जर (TAILQ_FIRST(files)) {
			TAILQ_FOREACH_SAFE(file, files, entries, file_np) {
				साठी (i = optind; i < argc; i++)
					जर (म_भेद(argv[i], file->मूल्य) == 0)
						थांबा;
				जर (i >= argc)
					rc_stringlist_delete(files,
					    file->मूल्य);
			}
		} अन्यथा {
			पर्यंत (optind < argc)
				rc_stringlist_add(files, argv[optind++]);
		}
	} अन्यथा जर (!filtered) {
		START_ENT;
		पर्यंत ((ent = GET_ENT))
			rc_stringlist_add(files, ENT_FILE(ent));
		END_ENT;

		जर (!TAILQ_FIRST(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	जर (!TAILQ_FIRST(files)) {
		rc_stringlist_free(files);
		परत (निकास_त्रुटि);
	}

	/* Ensure we always display something */
	START_ENT;
	TAILQ_FOREACH(file, files, entries) {
		जर (!(ent = GET_ENT_FILE(file->मूल्य))) {
			result = निकास_त्रुटि;
			सुरूठेवा;
		}

		/* mount or remount? */
		निवड (output) {
		स्थिती OUTPUT_MOUNT:
			result += do_mount(ent, false);
			थांबा;

		स्थिती OUTPUT_REMOUNT:
			result += do_mount(ent, true);
			थांबा;
		}

		/* No point in outputting if quiet */
		जर (rc_yesno(दो_पर्या("EINFO_QUIET")))
			सुरूठेवा;

		निवड (output) {
		स्थिती OUTPUT_BLOCKDEV:
			म_लिखो("%s\n", ENT_BLOCKDEVICE(ent));
			थांबा;

		स्थिती OUTPUT_MOUNTARGS:
			म_लिखो("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->मूल्य);
			थांबा;

		स्थिती OUTPUT_OPTIONS:
			म_लिखो("%s\n", ENT_OPTS(ent));
			थांबा;

		स्थिती OUTPUT_FILE:
			म_लिखो("%s\n", file->मूल्य);
			थांबा;

		स्थिती OUTPUT_PASSNO:
			म_लिखो("%d\n", ENT_PASS(ent));
			थांबा;
		}
	}
	END_ENT;

	rc_stringlist_free(files);
	निकास(result);
	/* NOTREACHED */
}
