/*
 * checkpath.c
 * Checks for the existence of a file or directory and creates it
 * if necessary. It can also correct its ownership.
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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <grp.h>
#అన్తర్భావ <pwd.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "selinux.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "helpers.h"

రకనిర్వచనం గణన {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *extraopts ="path1 [path2] [...]";
స్థిరరాశి అక్షరం getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "directory",          0, ఖాళీ, 'd'},
	{ "directory-truncate", 0, ఖాళీ, 'D'},
	{ "file",               0, ఖాళీ, 'f'},
	{ "file-truncate",      0, ఖాళీ, 'F'},
	{ "pipe",               0, ఖాళీ, 'p'},
	{ "mode",               1, ఖాళీ, 'm'},
	{ "owner",              1, ఖాళీ, 'o'},
	{ "symlinks",           0, ఖాళీ, 's'},
	{ "writable",           0, ఖాళీ, 'W'},
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
	"Create a directory if not exists",
	"Create/empty directory",
	"Create a file if not exists",
	"Truncate file",
	"Create a named pipe (FIFO) if not exists",
	"Mode to check",
	"Owner to check (user:group)",
	"follow symbolic links (irrelevant on linux)",
	"Check whether the path is writable or not",
	longopts_help_COMMON
};
స్థిరరాశి అక్షరం *usagestring = ఖాళీ;

స్థిర పూర్ణాంకం get_dirfd(అక్షరం *మార్గం, bool symlinks)
{
	అక్షరం *ch, *అంశం, *మార్గం_dupe, *str, *linkpath = ఖాళీ;
	పూర్ణాంకం components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	నిర్మాణం stat st;

	ఒకవేళ (!మార్గం || *మార్గం != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	ఒకవేళ (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = మార్గం;
	అయితే (*ch) {
		ఒకవేళ (*ch == '/')
			components++;
		ch++;
	}
	మార్గం_dupe = xstrdup(మార్గం);
	అంశం = strtok(మార్గం_dupe, "/");
#నిర్వచితమైతే O_మార్గం
	flags |= O_మార్గం;
#అయితే_ముగింపు
	ఒకవేళ (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	అయితే (dirfd > 0 && అంశం && components > 1) {
		str = xstrdup(linkpath ? linkpath : అంశం);
		new_dirfd = openat(dirfd, str, flags);
		ఒకవేళ (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, మార్గం, str, strerror(errno));
		ఒకవేళ (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, మార్గం, అంశం, strerror(errno));
		ఒకవేళ (S_ISLNK(st.st_mode) ) {
			ఒకవేళ (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, మార్గం, str);
			linksize = st.st_size+1;
			ఒకవేళ (linkpath)
				విడుదల(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			ఒకవేళ (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} లేకపోతే {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			విడుదల(linkpath);
			linkpath = ఖాళీ;
			అంశం = strtok(ఖాళీ, "/");
			components--;
		}

		విడుదల(str);
	}
	విడుదల(మార్గం_dupe);
	విడుదల(linkpath);
	తిరిగి dirfd;
}

స్థిర అక్షరం *clean_మార్గం(అక్షరం *మార్గం)
{
	అక్షరం *ch, *ch2, *str;

	str = xmalloc(అక్షరశ్రేణి_పొడవు(మార్గం) + 1);
	ch = మార్గం;
	ch2 = str;
	అయితే (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		ఒకవేళ (!*(ch-1))
			ఆపు;
		అయితే (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	అయితే ((ch = strrchr(str, '/'))) {
		ఒకవేళ (ch == str)
			ఆపు;
		ఒకవేళ (!*(ch+1))
			*ch = 0;
		లేకపోతే
			ఆపు;
	}
	తిరిగి str;
}

స్థిర పూర్ణాంకం do_create(inode_t type, స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం dirfd, స్థిరరాశి అక్షరం *name, పూర్ణాంకం flags, mode_t బహుళకం)
{
	పూర్ణాంకం ముసుగు = umask(0);
	పూర్ణాంకం fd;

	స్థిర స్థిరరాశి mode_t default_బహుళకం[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	ఒకవేళ (!బహుళకం)
		బహుళకం = default_బహుళకం[type];

	ఎంపిక (type) {
	స్థితి inode_file:
		einfo("%s: creating file", మార్గం);
		flags |= O_CREAT;
		ఆపు;
	స్థితి inode_dir:
		einfo("%s: creating directory", మార్గం);

		/* We do not recursively create parents */
		ఒకవేళ (mkdirat(dirfd, name, బహుళకం) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			తిరిగి -1;
		}

		ఆపు;
	స్థితి inode_fifo:
		einfo("%s: creating fifo", మార్గం);

		ఒకవేళ (mkfifoat(dirfd, name, బహుళకం) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			తిరిగి -1;
		}

		ఆపు;
	స్థితి inode_unknown:
		ఆపు;
	}
	umask(ముసుగు);

	ఒకవేళ ((fd = openat(dirfd, name, flags, బహుళకం)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		తిరిగి -1;
	}
	తిరిగి fd;
}

స్థిర పూర్ణాంకం do_check(అక్షరం *మార్గం, uid_t uid, gid_t gid, mode_t బహుళకం, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	పూర్ణాంకం flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	స్థిరరాశి అక్షరం *name = basename_c(మార్గం);
	నిర్మాణం stat st;
	పూర్ణాంకం dirfd, fd;

	ఒకవేళ (writable) {
		ఒకవేళ (access(మార్గం, W_OK) == 0)
			తిరిగి 0;
		ఒకవేళ (errno != ENOENT || type == inode_unknown)
			తిరిగి -1;
	}

	ఒకవేళ (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(మార్గం, symlinks);
	ఒకవేళ ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, మార్గం, dirfd, name, flags, బహుళకం)) == -1)
		తిరిగి -1;

	ఒకవేళ (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", మార్గం, strerror(errno));
		వెళ్ళు err;
	}

	ఒకవేళ (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", మార్గం);
		వెళ్ళు err;
	}

	ఒకవేళ (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", మార్గం);
		వెళ్ళు err;
	}

	ఒకవేళ (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", మార్గం);
		వెళ్ళు err;
	}

	ఒకవేళ (బహుళకం && (st.st_mode & 07777) != బహుళకం) {
		ఒకవేళ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, మార్గం);
			వెళ్ళు err;
		}

		ఒకవేళ (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, మార్గం);
			వెళ్ళు err;
		}

		einfo("%s: correcting mode", మార్గం);
		ఒకవేళ (fchmod(fd, బహుళకం)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			వెళ్ళు err;
		}
	}

	ఒకవేళ (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		ఒకవేళ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, మార్గం);
			వెళ్ళు err;
		}

		ఒకవేళ (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, మార్గం);
			వెళ్ళు err;
		}

		einfo("%s: correcting owner", మార్గం);
		ఒకవేళ (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			వెళ్ళు err;
		}
	}

	ఒకవేళ (selinux_on)
		selinux_util_label(మార్గం);
	close(fd);

	తిరిగి 0;
err:
	close(fd);
	తిరిగి -1;
}

స్థిర పూర్ణాంకం parse_owner(నిర్మాణం passwd **user, నిర్మాణం group **group, స్థిరరాశి అక్షరం *owner)
{
	అక్షరం *u = xstrdup(owner), *g = strchr(u, ':');
	పూర్ణాంకం id = 0, retval = 0;

	ఒకవేళ (g)
		*g++ = '\0';

	ఒకవేళ (user && *u) {
		ఒకవేళ (అక్షరశ్రేణి_చదువు(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		లేకపోతే
			*user = getpwnam(u);
		ఒకవేళ (*user == ఖాళీ)
			retval = -1;
	}

	ఒకవేళ (group && g && *g) {
		ఒకవేళ (అక్షరశ్రేణి_చదువు(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		లేకపోతే
			*group = getgrnam(g);
		ఒకవేళ (*group == ఖాళీ)
			retval = -1;
	}

	విడుదల(u);
	తిరిగి retval;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	పూర్ణాంకం opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t బహుళకం = 0;
	నిర్మాణం passwd *pw = ఖాళీ;
	నిర్మాణం group *gr = ఖాళీ;
	inode_t type = inode_unknown;
	పూర్ణాంకం retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	అక్షరం *మార్గం = ఖాళీ;

	applet = basename_c(argv[0]);
	అయితే ((opt = getopt_long(argc, argv, getoptstring, longopts, (పూర్ణాంకం *) 0)) != -1) {
		ఎంపిక (opt) {
		స్థితి 'D':
			trunc = true;
			/* falls through */
		స్థితి 'd':
			type = inode_dir;
			ఆపు;
		స్థితి 'F':
			trunc = true;
			/* falls through */
		స్థితి 'f':
			type = inode_file;
			ఆపు;
		స్థితి 'p':
			type = inode_fifo;
			ఆపు;
		స్థితి 'm':
			ఒకవేళ (parse_బహుళకం(&బహుళకం, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			ఆపు;
		స్థితి 'o':
			chowner = true;
			ఒకవేళ (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			ఆపు;
		స్థితి 's':
#నిర్వచితం_కాకపోతే O_మార్గం
			symlinks = true;
#అయితే_ముగింపు
			ఆపు;
		స్థితి 'W':
			writable = true;
			ఆపు;

		case_RC_COMMON_GETOPT
		}
	}

	ఒకవేళ (optind >= argc)
		usage(EXIT_FAILURE);

	ఒకవేళ (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	ఒకవేళ (gr)
		gid = gr->gr_gid;

	ఒకవేళ (selinux_util_open() == 1)
		selinux_on = true;

	అయితే (optind < argc) {
		మార్గం = clean_మార్గం(argv[optind]);
		ఒకవేళ (do_check(మార్గం, uid, gid, బహుళకం, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		విడుదల(మార్గం);
	}

	ఒకవేళ (selinux_on)
		selinux_util_close();

	తిరిగి retval;
}
