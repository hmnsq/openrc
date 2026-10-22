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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <grp.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "selinux.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *extraopts ="path1 [path2] [...]";
ಧ್ರುವ ವರ್ಣ getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{ "directory",          0, ರಿಕ್ತ, 'd'},
	{ "directory-truncate", 0, ರಿಕ್ತ, 'D'},
	{ "file",               0, ರಿಕ್ತ, 'f'},
	{ "file-truncate",      0, ರಿಕ್ತ, 'F'},
	{ "pipe",               0, ರಿಕ್ತ, 'p'},
	{ "mode",               1, ರಿಕ್ತ, 'm'},
	{ "owner",              1, ರಿಕ್ತ, 'o'},
	{ "symlinks",           0, ರಿಕ್ತ, 's'},
	{ "writable",           0, ರಿಕ್ತ, 'W'},
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[] = {
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
ಧ್ರುವ ವರ್ಣ *usagestring = ರಿಕ್ತ;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ get_dirfd(ವರ್ಣ *path, bool symlinks)
{
	ವರ್ಣ *ch, *item, *path_dupe, *str, *linkpath = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	ಸಂರಚನಾ stat st;

	ಯದಿ (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	ಯದಿ (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	ಯಾವತ್ (*ch) {
		ಯದಿ (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	ಯದಿ (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	ಯಾವತ್ (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		ಯದಿ (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		ಯದಿ (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		ಯದಿ (S_ISLNK(st.st_mode) ) {
			ಯದಿ (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			ಯದಿ (linkpath)
				ಮುಕ್ತ(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			ಯದಿ (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} ಅನ್ಯಥಾ {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			ಮುಕ್ತ(linkpath);
			linkpath = ರಿಕ್ತ;
			item = strtok(ರಿಕ್ತ, "/");
			components--;
		}

		ಮುಕ್ತ(str);
	}
	ಮುಕ್ತ(path_dupe);
	ಮುಕ್ತ(linkpath);
	ಪ್ರತ್ಯಾಗಮ dirfd;
}

ಸ್ಥಿರ ವರ್ಣ *clean_path(ವರ್ಣ *path)
{
	ವರ್ಣ *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	ಯಾವತ್ (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		ಯದಿ (!*(ch-1))
			ಭಙ್ಗ;
		ಯಾವತ್ (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	ಯಾವತ್ ((ch = strrchr(str, '/'))) {
		ಯದಿ (ch == str)
			ಭಙ್ಗ;
		ಯದಿ (!*(ch+1))
			*ch = 0;
		ಅನ್ಯಥಾ
			ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ str;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ do_create(inode_t type, ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ dirfd, ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ flags, mode_t mode)
{
	ಪೂರ್ಣಾಙ್ಕ mask = umask(0);
	ಪೂರ್ಣಾಙ್ಕ fd;

	ಸ್ಥಿರ ಧ್ರುವ mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	ಯದಿ (!mode)
		mode = default_mode[type];

	ವಿಕಲ್ಪ (type) {
	ಪಕ್ಷ inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		ಭಙ್ಗ;
	ಪಕ್ಷ inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		ಯದಿ (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			ಪ್ರತ್ಯಾಗಮ -1;
		}

		ಭಙ್ಗ;
	ಪಕ್ಷ inode_fifo:
		einfo("%s: creating fifo", path);

		ಯದಿ (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			ಪ್ರತ್ಯಾಗಮ -1;
		}

		ಭಙ್ಗ;
	ಪಕ್ಷ inode_unknown:
		ಭಙ್ಗ;
	}
	umask(mask);

	ಯದಿ ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	ಪ್ರತ್ಯಾಗಮ fd;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ do_check(ವರ್ಣ *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	ಪೂರ್ಣಾಙ್ಕ flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	ಧ್ರುವ ವರ್ಣ *name = basename_c(path);
	ಸಂರಚನಾ stat st;
	ಪೂರ್ಣಾಙ್ಕ dirfd, fd;

	ಯದಿ (writable) {
		ಯದಿ (access(path, W_OK) == 0)
			ಪ್ರತ್ಯಾಗಮ 0;
		ಯದಿ (errno != ENOENT || type == inode_unknown)
			ಪ್ರತ್ಯಾಗಮ -1;
	}

	ಯದಿ (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	ಯದಿ ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಯದಿ (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		ಗಚ್ಛ err;
	}

	ಯದಿ (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		ಗಚ್ಛ err;
	}

	ಯದಿ (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		ಗಚ್ಛ err;
	}

	ಯದಿ (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		ಗಚ್ಛ err;
	}

	ಯದಿ (mode && (st.st_mode & 07777) != mode) {
		ಯದಿ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			ಗಚ್ಛ err;
		}

		ಯದಿ (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			ಗಚ್ಛ err;
		}

		einfo("%s: correcting mode", path);
		ಯದಿ (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			ಗಚ್ಛ err;
		}
	}

	ಯದಿ (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		ಯದಿ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			ಗಚ್ಛ err;
		}

		ಯದಿ (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			ಗಚ್ಛ err;
		}

		einfo("%s: correcting owner", path);
		ಯದಿ (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			ಗಚ್ಛ err;
		}
	}

	ಯದಿ (selinux_on)
		selinux_util_label(path);
	close(fd);

	ಪ್ರತ್ಯಾಗಮ 0;
err:
	close(fd);
	ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_owner(ಸಂರಚನಾ passwd **user, ಸಂರಚನಾ group **group, ಧ್ರುವ ವರ್ಣ *owner)
{
	ವರ್ಣ *u = xstrdup(owner), *g = strchr(u, ':');
	ಪೂರ್ಣಾಙ್ಕ id = 0, retval = 0;

	ಯದಿ (g)
		*g++ = '\0';

	ಯದಿ (user && *u) {
		ಯದಿ (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		ಅನ್ಯಥಾ
			*user = getpwnam(u);
		ಯದಿ (*user == ರಿಕ್ತ)
			retval = -1;
	}

	ಯದಿ (group && g && *g) {
		ಯದಿ (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		ಅನ್ಯಥಾ
			*group = getgrnam(g);
		ಯದಿ (*group == ರಿಕ್ತ)
			retval = -1;
	}

	ಮುಕ್ತ(u);
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	ಸಂರಚನಾ passwd *pw = ರಿಕ್ತ;
	ಸಂರಚನಾ group *gr = ರಿಕ್ತ;
	inode_t type = inode_unknown;
	ಪೂರ್ಣಾಙ್ಕ retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	ವರ್ಣ *path = ರಿಕ್ತ;

	applet = basename_c(argv[0]);
	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring, longopts, (ಪೂರ್ಣಾಙ್ಕ *) 0)) != -1) {
		ವಿಕಲ್ಪ (opt) {
		ಪಕ್ಷ 'D':
			trunc = true;
			/* falls through */
		ಪಕ್ಷ 'd':
			type = inode_dir;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'F':
			trunc = true;
			/* falls through */
		ಪಕ್ಷ 'f':
			type = inode_file;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'p':
			type = inode_fifo;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'm':
			ಯದಿ (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'o':
			chowner = true;
			ಯದಿ (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			ಭಙ್ಗ;
		ಪಕ್ಷ 's':
#ifndef O_PATH
			symlinks = true;
#endif
			ಭಙ್ಗ;
		ಪಕ್ಷ 'W':
			writable = true;
			ಭಙ್ಗ;

		case_RC_COMMON_GETOPT
		}
	}

	ಯದಿ (optind >= argc)
		usage(EXIT_FAILURE);

	ಯದಿ (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	ಯದಿ (gr)
		gid = gr->gr_gid;

	ಯದಿ (selinux_util_open() == 1)
		selinux_on = true;

	ಯಾವತ್ (optind < argc) {
		path = clean_path(argv[optind]);
		ಯದಿ (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		ಮುಕ್ತ(path);
	}

	ಯದಿ (selinux_on)
		selinux_util_close();

	ಪ್ರತ್ಯಾಗಮ retval;
}
