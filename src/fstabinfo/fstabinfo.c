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

#समावेश <sys/wait.h>
#समावेश <errno.h>
#समावेश <getopt.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <unistd.h>
#समावेश <stdbool.h>
#समावेश <spawn.h>

/* Yay for linux and its non liking of POSIX functions.
   Okay, we could use getfsent but the man page says use getmntent instead
   AND we don't have getfsent on uclibc or dietlibc for some odd reason. */
#अगर_घोषित __linux__
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
#पूर्ण_अगर

#समावेश "einfo.h"
#समावेश "queue.h"
#समावेश "rc.h"
#समावेश "_usage.h"
#समावेश "helpers.h"

बाह्य अक्षर **environ;

स्थिर अक्षर *applet = शून्य;
स्थिर अक्षर *extraopts = शून्य;
स्थिर अक्षर getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "mount",          0, शून्य, 'M' },
	{ "remount",        0, शून्य, 'R' },
	{ "blockdevice",    0, शून्य, 'b' },
	{ "mountargs",      0, शून्य, 'm' },
	{ "options",        0, शून्य, 'o' },
	{ "passno",         1, शून्य, 'p' },
	{ "fstype",         1, शून्य, 't' },
	longopts_COMMON
};
स्थिर अक्षर * स्थिर longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
स्थिर अक्षर *usagestring = शून्य;

#अगर_घोषित HAVE_GETMNTENT
अटल काष्ठा mntent *
getmntfile(स्थिर अक्षर *file)
{
	काष्ठा mntent *ent;
	खाता *fp;

	START_ENT;
	जबतक ((ent = getmntent(fp)))
		अगर (म_भेद(file, ent->mnt_dir) == 0)
			अवरोध;
	END_ENT;

	वापस ent;
}
#पूर्ण_अगर

बाह्य स्थिर अक्षर *applet;

अटल पूर्णांक
do_mount(काष्ठा ENT *ent, bool remount)
{
	अक्षर *argv[10];
	pid_t pid;
	पूर्णांक status, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	अगर (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = शून्य;
	} अन्यथा {
#अगर_घोषित __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = शून्य;
#अन्यथा
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = शून्य;
#पूर्ण_अगर
	}
	err = posix_spawnp(&pid, argv[0], शून्य, शून्य, argv, environ);
	अगर (err)
		eerrorx("%s: posix_spawnp: %s", applet, म_त्रुटि(err));
	जबतक (waitpid(pid, &status, 0) < 0 && त्रुटि_सं == EINTR);
	अगर (WIFEXITED(status))
		वापस WEXITSTATUS(status);
	अन्यथा
		वापस -1;
}

#घोषणा OUTPUT_FILE      (1 << 1)
#घोषणा OUTPUT_MOUNTARGS (1 << 2)
#घोषणा OUTPUT_OPTIONS   (1 << 3)
#घोषणा OUTPUT_PASSNO    (1 << 4)
#घोषणा OUTPUT_BLOCKDEV  (1 << 5)
#घोषणा OUTPUT_MOUNT     (1 << 6)
#घोषणा OUTPUT_REMOUNT   (1 << 7)

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	काष्ठा ENT *ent;
	पूर्णांक परिणाम = निकास_सफल;
	अक्षर *token;
	पूर्णांक i, p;
	पूर्णांक opt;
	पूर्णांक output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#अगर_घोषित HAVE_GETMNTENT
	खाता *fp;
#पूर्ण_अगर

	/* fail if there is no /etc/fstab */
	अगर (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	जबतक ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		चयन (opt) {
		हाल 'M':
			output = OUTPUT_MOUNT;
			अवरोध;
		हाल 'R':
			output = OUTPUT_REMOUNT;
			अवरोध;
		हाल 'b':
			output = OUTPUT_BLOCKDEV;
			अवरोध;
		हाल 'o':
			output = OUTPUT_OPTIONS;
			अवरोध;
		हाल 'm':
			output = OUTPUT_MOUNTARGS;
			अवरोध;

		हाल 'p':
			चयन (optarg[0]) {
			हाल '=':
			हाल '<':
			हाल '>':
				अगर (माला_पूछो(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				START_ENT;
				जबतक ((ent = GET_ENT)) {
					अगर (म_भेद(ENT_FILE(ent), "none") == 0)
						जारी;
					p = ENT_PASS(ent);
					अगर ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				}
				END_ENT;
				अवरोध;

			शेष:
				rc_stringlist_add(files, optarg);
				output = OUTPUT_PASSNO;
				अवरोध;
			}
			अवरोध;

		हाल 't':
			filtered = true;
			जबतक ((token = strsep(&optarg, ","))) {
				START_ENT;
				जबतक ((ent = GET_ENT))
					अगर (म_भेद(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_add(files,
						    ENT_FILE(ent));
				END_ENT;
			}
			अवरोध;

		case_RC_COMMON_GETOPT
		}
	}

	अगर (optind < argc) {
		अगर (TAILQ_FIRST(files)) {
			TAILQ_FOREACH_SAFE(file, files, entries, file_np) {
				क्रम (i = optind; i < argc; i++)
					अगर (म_भेद(argv[i], file->मूल्य) == 0)
						अवरोध;
				अगर (i >= argc)
					rc_stringlist_delete(files,
					    file->मूल्य);
			}
		} अन्यथा {
			जबतक (optind < argc)
				rc_stringlist_add(files, argv[optind++]);
		}
	} अन्यथा अगर (!filtered) {
		START_ENT;
		जबतक ((ent = GET_ENT))
			rc_stringlist_add(files, ENT_FILE(ent));
		END_ENT;

		अगर (!TAILQ_FIRST(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	अगर (!TAILQ_FIRST(files)) {
		rc_stringlist_free(files);
		वापस (निकास_त्रुटि);
	}

	/* Ensure we always display something */
	START_ENT;
	TAILQ_FOREACH(file, files, entries) {
		अगर (!(ent = GET_ENT_FILE(file->मूल्य))) {
			परिणाम = निकास_त्रुटि;
			जारी;
		}

		/* mount or remount? */
		चयन (output) {
		हाल OUTPUT_MOUNT:
			परिणाम += do_mount(ent, false);
			अवरोध;

		हाल OUTPUT_REMOUNT:
			परिणाम += do_mount(ent, true);
			अवरोध;
		}

		/* No point in outputting if quiet */
		अगर (rc_yesno(दो_पर्या("EINFO_QUIET")))
			जारी;

		चयन (output) {
		हाल OUTPUT_BLOCKDEV:
			म_लिखो("%s\n", ENT_BLOCKDEVICE(ent));
			अवरोध;

		हाल OUTPUT_MOUNTARGS:
			म_लिखो("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->मूल्य);
			अवरोध;

		हाल OUTPUT_OPTIONS:
			म_लिखो("%s\n", ENT_OPTS(ent));
			अवरोध;

		हाल OUTPUT_FILE:
			म_लिखो("%s\n", file->मूल्य);
			अवरोध;

		हाल OUTPUT_PASSNO:
			म_लिखो("%d\n", ENT_PASS(ent));
			अवरोध;
		}
	}
	END_ENT;

	rc_stringlist_free(files);
	निकास(परिणाम);
	/* NOTREACHED */
}
