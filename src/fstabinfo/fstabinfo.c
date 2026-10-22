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

#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <spawn.h>

/* Yay for linux and its non liking of POSIX functions.
   Okay, we could use getfsent but the man page says use getmntent instead
   AND we don't have getfsent on uclibc or dietlibc for some odd reason. */
#నిర్వచితమైతే __linux__
#  define HAVE_GETMNTENT
#  include <mntent.h>

#  define ENT mntent
#  define ప్రారంభం_ENT fp = setmntent ("/etc/fstab", "r");
#  define GET_ENT getmntent (fp)
#  define GET_ENT_FILE(_name) getmntfile (_name)
#  define చివర_ENT endmntent (fp)
#  define ENT_BLOCKDEVICE(_ent) (_ent)->mnt_fsname
#  define ENT_FILE(_ent) (_ent)->mnt_dir
#  define ENT_TYPE(_ent) (_ent)->mnt_type
#  define ENT_OPTS(_ent) (_ent)->mnt_opts
#  define ENT_PASS(_ent) (_ent)->mnt_passno
#లేకపోతే
#  define HAVE_GETFSENT
#  include <fstab.h>

#  define ENT fstab
#  define ప్రారంభం_ENT
#  define GET_ENT getfsent ()
#  define GET_ENT_FILE(_name) getfsfile (_name)
#  define చివర_ENT endfsent ()
#  define ENT_BLOCKDEVICE(_ent) (_ent)->fs_spec
#  define ENT_TYPE(_ent) (_ent)->fs_vfstype
#  define ENT_FILE(_ent) (_ent)->fs_file
#  define ENT_OPTS(_ent) (_ent)->fs_mntops
#  define ENT_PASS(_ent) (_ent)->fs_passno
#అయితే_ముగింపు

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "helpers.h"

బాహ్య అక్షరం **environ;

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *extraopts = ఖాళీ;
స్థిరరాశి అక్షరం getoptstring[] = "MRbmop:t:" getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "mount",          0, ఖాళీ, 'M' },
	{ "remount",        0, ఖాళీ, 'R' },
	{ "blockdevice",    0, ఖాళీ, 'b' },
	{ "mountargs",      0, ఖాళీ, 'm' },
	{ "options",        0, ఖాళీ, 'o' },
	{ "passno",         1, ఖాళీ, 'p' },
	{ "fstype",         1, ఖాళీ, 't' },
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
	"Mounts the filesystem from the mountpoint",
	"Remounts the filesystem based on the information in fstab",
	"Extract the block device",
	"Show arguments needed to mount the entry",
	"Extract the options field",
	"Extract or query the pass number field",
	"List entries with matching file system type",
	longopts_help_COMMON
};
స్థిరరాశి అక్షరం *usagestring = ఖాళీ;

#నిర్వచితమైతే HAVE_GETMNTENT
స్థిర నిర్మాణం mntent *
getmntfile(స్థిరరాశి అక్షరం *file)
{
	నిర్మాణం mntent *ent;
	FILE *fp;

	ప్రారంభం_ENT;
	అయితే ((ent = getmntent(fp)))
		ఒకవేళ (strcmp(file, ent->mnt_dir) == 0)
			ఆపు;
	చివర_ENT;

	తిరిగి ent;
}
#అయితే_ముగింపు

బాహ్య స్థిరరాశి అక్షరం *applet;

స్థిర పూర్ణాంకం
do_mount(నిర్మాణం ENT *ent, bool remount)
{
	అక్షరం *argv[10];
	pid_t pid;
	పూర్ణాంకం స్థితి_2, err;

	argv[0] = UNCONST("mount");
	argv[1] = UNCONST("-o");
	argv[2] = ENT_OPTS(ent);
	argv[3] = UNCONST("-t");
	argv[4] = ENT_TYPE(ent);
	ఒకవేళ (!remount) {
		argv[5] = ENT_BLOCKDEVICE(ent);
		argv[6] = ENT_FILE(ent);
		argv[7] = ఖాళీ;
	} లేకపోతే {
#నిర్వచితమైతే __linux__
		argv[5] = UNCONST("-o");
		argv[6] = UNCONST("remount");
		argv[7] = ENT_BLOCKDEVICE(ent);
		argv[8] = ENT_FILE(ent);
		argv[9] = ఖాళీ;
#లేకపోతే
		argv[5] = UNCONST("-u");
		argv[6] = ENT_BLOCKDEVICE(ent);
		argv[7] = ENT_FILE(ent);
		argv[8] = ఖాళీ;
#అయితే_ముగింపు
	}
	err = posix_spawnp(&pid, argv[0], ఖాళీ, ఖాళీ, argv, environ);
	ఒకవేళ (err)
		eerrorx("%s: posix_spawnp: %s", applet, strerror(err));
	అయితే (waitpid(pid, &స్థితి_2, 0) < 0 && errno == EINTR);
	ఒకవేళ (WIFEXITED(స్థితి_2))
		తిరిగి WEXITSTATUS(స్థితి_2);
	లేకపోతే
		తిరిగి -1;
}

#నిర్వచించు OUTPUT_FILE      (1 << 1)
#నిర్వచించు OUTPUT_MOUNTARGS (1 << 2)
#నిర్వచించు OUTPUT_OPTIONS   (1 << 3)
#నిర్వచించు OUTPUT_PASSNO    (1 << 4)
#నిర్వచించు OUTPUT_BLOCKDEV  (1 << 5)
#నిర్వచించు OUTPUT_MOUNT     (1 << 6)
#నిర్వచించు OUTPUT_REMOUNT   (1 << 7)

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	నిర్మాణం ENT *ent;
	పూర్ణాంకం ఫలితం = EXIT_SUCCESS;
	అక్షరం *token;
	పూర్ణాంకం i, p;
	పూర్ణాంకం opt;
	పూర్ణాంకం output = OUTPUT_FILE;
	RC_STRINGLIST *files = rc_stringlist_new();
	RC_STRING *file, *file_np;
	bool filtered = false;

#నిర్వచితమైతే HAVE_GETMNTENT
	FILE *fp;
#అయితే_ముగింపు

	/* fail if there is no /etc/fstab */
	ఒకవేళ (access("/etc/fstab", F_OK) != 0)
		eerrorx("/etc/fstab does not exist");
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	అయితే ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (పూర్ణాంకం *) 0)) != -1)
	{
		ఎంపిక (opt) {
		స్థితి 'M':
			output = OUTPUT_MOUNT;
			ఆపు;
		స్థితి 'R':
			output = OUTPUT_REMOUNT;
			ఆపు;
		స్థితి 'b':
			output = OUTPUT_BLOCKDEV;
			ఆపు;
		స్థితి 'o':
			output = OUTPUT_OPTIONS;
			ఆపు;
		స్థితి 'm':
			output = OUTPUT_MOUNTARGS;
			ఆపు;

		స్థితి 'p':
			ఎంపిక (optarg[0]) {
			స్థితి '=':
			స్థితి '<':
			స్థితి '>':
				ఒకవేళ (అక్షరశ్రేణి_చదువు(optarg + 1, "%d", &i) != 1)
					eerrorx("%s: invalid passno %s",
					    argv[0], optarg + 1);

				filtered = true;
				opt = optarg[0];
				ప్రారంభం_ENT;
				అయితే ((ent = GET_ENT)) {
					ఒకవేళ (strcmp(ENT_FILE(ent), "none") == 0)
						కొనసాగు;
					p = ENT_PASS(ent);
					ఒకవేళ ((opt == '=' && i == p) ||
					    (opt == '<' && i > p && p != 0) ||
					    (opt == '>' && i < p && p != 0))
						rc_stringlist_కూడు(files,
						    ENT_FILE(ent));
				}
				చివర_ENT;
				ఆపు;

			అప్రమేయ:
				rc_stringlist_కూడు(files, optarg);
				output = OUTPUT_PASSNO;
				ఆపు;
			}
			ఆపు;

		స్థితి 't':
			filtered = true;
			అయితే ((token = strsep(&optarg, ","))) {
				ప్రారంభం_ENT;
				అయితే ((ent = GET_ENT))
					ఒకవేళ (strcmp(token, ENT_TYPE(ent)) == 0)
						rc_stringlist_కూడు(files,
						    ENT_FILE(ent));
				చివర_ENT;
			}
			ఆపు;

		case_RC_COMMON_GETOPT
		}
	}

	ఒకవేళ (optind < argc) {
		ఒకవేళ (TAILQ_మొదటి(files)) {
			TAILQ_FOREACH_సురక్షిత(file, files, entries, file_np) {
				కొరకు (i = optind; i < argc; i++)
					ఒకవేళ (strcmp(argv[i], file->విలువ) == 0)
						ఆపు;
				ఒకవేళ (i >= argc)
					rc_stringlist_delete(files,
					    file->విలువ);
			}
		} లేకపోతే {
			అయితే (optind < argc)
				rc_stringlist_కూడు(files, argv[optind++]);
		}
	} లేకపోతే ఒకవేళ (!filtered) {
		ప్రారంభం_ENT;
		అయితే ((ent = GET_ENT))
			rc_stringlist_కూడు(files, ENT_FILE(ent));
		చివర_ENT;

		ఒకవేళ (!TAILQ_మొదటి(files))
			eerrorx("%s: empty fstab", argv[0]);
	}

	ఒకవేళ (!TAILQ_మొదటి(files)) {
		rc_stringlist_free(files);
		తిరిగి (EXIT_FAILURE);
	}

	/* Ensure we always display something */
	ప్రారంభం_ENT;
	TAILQ_FOREACH(file, files, entries) {
		ఒకవేళ (!(ent = GET_ENT_FILE(file->విలువ))) {
			ఫలితం = EXIT_FAILURE;
			కొనసాగు;
		}

		/* mount or remount? */
		ఎంపిక (output) {
		స్థితి OUTPUT_MOUNT:
			ఫలితం += do_mount(ent, false);
			ఆపు;

		స్థితి OUTPUT_REMOUNT:
			ఫలితం += do_mount(ent, true);
			ఆపు;
		}

		/* No point in outputting if quiet */
		ఒకవేళ (rc_yesno(getenv("EINFO_QUIET")))
			కొనసాగు;

		ఎంపిక (output) {
		స్థితి OUTPUT_BLOCKDEV:
			రాయి("%s\n", ENT_BLOCKDEVICE(ent));
			ఆపు;

		స్థితి OUTPUT_MOUNTARGS:
			రాయి("-o %s -t %s %s %s\n",
			    ENT_OPTS(ent),
			    ENT_TYPE(ent),
			    ENT_BLOCKDEVICE(ent),
			    file->విలువ);
			ఆపు;

		స్థితి OUTPUT_OPTIONS:
			రాయి("%s\n", ENT_OPTS(ent));
			ఆపు;

		స్థితి OUTPUT_FILE:
			రాయి("%s\n", file->విలువ);
			ఆపు;

		స్థితి OUTPUT_PASSNO:
			రాయి("%d\n", ENT_PASS(ent));
			ఆపు;
		}
	}
	చివర_ENT;

	rc_stringlist_free(files);
	నిష్క్రమించు(ఫలితం);
	/* NOTREACHED */
}
