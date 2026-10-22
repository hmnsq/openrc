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

#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <getopt.h>
#ضمن <grp.h>
#ضمن <pwd.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/stat.h>
#ضمن <sys/types.h>
#ضمن <unistd.h>

#ضمن "einfo.h"
#ضمن "misc.h"
#ضمن "selinux.h"
#ضمن "_usage.h"
#ضمن "helpers.h"

تعريفنوع تعداد {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

ثابت حرف *applet = عدم;
ثابت حرف *extraopts ="path1 [path2] [...]";
ثابت حرف getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "directory",          0, عدم, 'd'},
	{ "directory-truncate", 0, عدم, 'D'},
	{ "file",               0, عدم, 'f'},
	{ "file-truncate",      0, عدم, 'F'},
	{ "pipe",               0, عدم, 'p'},
	{ "mode",               1, عدم, 'm'},
	{ "owner",              1, عدم, 'o'},
	{ "symlinks",           0, عدم, 's'},
	{ "writable",           0, عدم, 'W'},
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
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
ثابت حرف *usagestring = عدم;

ساكن صحيح get_dirfd(حرف *path, bool symlinks)
{
	حرف *ch, *item, *path_dupe, *str, *linkpath = عدم;
	صحيح components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	بنية stat st;

	ܐܢ (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	ܐܢ (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	ܟܡܐ (*ch) {
		ܐܢ (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	ܐܢ (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	ܟܡܐ (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		ܐܢ (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		ܐܢ (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		ܐܢ (S_ISLNK(st.st_mode) ) {
			ܐܢ (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			ܐܢ (linkpath)
				حرر(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			ܐܢ (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} ܐܚܪܢܐ {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			حرر(linkpath);
			linkpath = عدم;
			item = strtok(عدم, "/");
			components--;
		}

		حرر(str);
	}
	حرر(path_dupe);
	حرر(linkpath);
	ܦܢܐ dirfd;
}

ساكن حرف *clean_path(حرف *path)
{
	حرف *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	ܟܡܐ (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		ܐܢ (!*(ch-1))
			اكسر;
		ܟܡܐ (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	ܟܡܐ ((ch = strrchr(str, '/'))) {
		ܐܢ (ch == str)
			اكسر;
		ܐܢ (!*(ch+1))
			*ch = 0;
		ܐܚܪܢܐ
			اكسر;
	}
	ܦܢܐ str;
}

ساكن صحيح do_create(inode_t type, ثابت حرف *path, صحيح dirfd, ثابت حرف *name, صحيح flags, mode_t mode)
{
	صحيح mask = umask(0);
	صحيح fd;

	ساكن ثابت mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	ܐܢ (!mode)
		mode = default_mode[type];

	بدل (type) {
	حالة inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		اكسر;
	حالة inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		ܐܢ (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			ܦܢܐ -1;
		}

		اكسر;
	حالة inode_fifo:
		einfo("%s: creating fifo", path);

		ܐܢ (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			ܦܢܐ -1;
		}

		اكسر;
	حالة inode_unknown:
		اكسر;
	}
	umask(mask);

	ܐܢ ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		ܦܢܐ -1;
	}
	ܦܢܐ fd;
}

ساكن صحيح do_check(حرف *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	صحيح flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	ثابت حرف *name = basename_c(path);
	بنية stat st;
	صحيح dirfd, fd;

	ܐܢ (writable) {
		ܐܢ (access(path, W_OK) == 0)
			ܦܢܐ 0;
		ܐܢ (errno != ENOENT || type == inode_unknown)
			ܦܢܐ -1;
	}

	ܐܢ (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	ܐܢ ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		ܦܢܐ -1;

	ܐܢ (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		اذهب err;
	}

	ܐܢ (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		اذهب err;
	}

	ܐܢ (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		اذهب err;
	}

	ܐܢ (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		اذهب err;
	}

	ܐܢ (mode && (st.st_mode & 07777) != mode) {
		ܐܢ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			اذهب err;
		}

		ܐܢ (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			اذهب err;
		}

		einfo("%s: correcting mode", path);
		ܐܢ (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			اذهب err;
		}
	}

	ܐܢ (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		ܐܢ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			اذهب err;
		}

		ܐܢ (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			اذهب err;
		}

		einfo("%s: correcting owner", path);
		ܐܢ (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			اذهب err;
		}
	}

	ܐܢ (selinux_on)
		selinux_util_label(path);
	close(fd);

	ܦܢܐ 0;
err:
	close(fd);
	ܦܢܐ -1;
}

ساكن صحيح parse_owner(بنية passwd **user, بنية group **group, ثابت حرف *owner)
{
	حرف *u = xstrdup(owner), *g = strchr(u, ':');
	صحيح id = 0, retval = 0;

	ܐܢ (g)
		*g++ = '\0';

	ܐܢ (user && *u) {
		ܐܢ (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		ܐܚܪܢܐ
			*user = getpwnam(u);
		ܐܢ (*user == عدم)
			retval = -1;
	}

	ܐܢ (group && g && *g) {
		ܐܢ (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		ܐܚܪܢܐ
			*group = getgrnam(g);
		ܐܢ (*group == عدم)
			retval = -1;
	}

	حرر(u);
	ܦܢܐ retval;
}

صحيح main(صحيح argc, حرف **argv)
{
	صحيح opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	بنية passwd *pw = عدم;
	بنية group *gr = عدم;
	inode_t type = inode_unknown;
	صحيح retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	حرف *path = عدم;

	applet = basename_c(argv[0]);
	ܟܡܐ ((opt = getopt_long(argc, argv, getoptstring, longopts, (صحيح *) 0)) != -1) {
		بدل (opt) {
		حالة 'D':
			trunc = true;
			/* falls through */
		حالة 'd':
			type = inode_dir;
			اكسر;
		حالة 'F':
			trunc = true;
			/* falls through */
		حالة 'f':
			type = inode_file;
			اكسر;
		حالة 'p':
			type = inode_fifo;
			اكسر;
		حالة 'm':
			ܐܢ (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			اكسر;
		حالة 'o':
			chowner = true;
			ܐܢ (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			اكسر;
		حالة 's':
#ifndef O_PATH
			symlinks = true;
#endif
			اكسر;
		حالة 'W':
			writable = true;
			اكسر;

		case_RC_COMMON_GETOPT
		}
	}

	ܐܢ (optind >= argc)
		usage(EXIT_FAILURE);

	ܐܢ (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	ܐܢ (gr)
		gid = gr->gr_gid;

	ܐܢ (selinux_util_open() == 1)
		selinux_on = true;

	ܟܡܐ (optind < argc) {
		path = clean_path(argv[optind]);
		ܐܢ (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		حرر(path);
	}

	ܐܢ (selinux_on)
		selinux_util_close();

	ܦܢܐ retval;
}
