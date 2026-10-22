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

#شامل <errno.h>
#شامل <fcntl.h>
#شامل <getopt.h>
#شامل <grp.h>
#شامل <pwd.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/stat.h>
#شامل <sys/types.h>
#شامل <unistd.h>

#شامل "einfo.h"
#شامل "misc.h"
#شامل "selinux.h"
#شامل "_usage.h"
#شامل "helpers.h"

تعریفنوع شمارش {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

ثابت نویسه *applet = پوچ;
ثابت نویسه *extraopts ="path1 [path2] [...]";
ثابت نویسه getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{ "directory",          0, پوچ, 'd'},
	{ "directory-truncate", 0, پوچ, 'D'},
	{ "file",               0, پوچ, 'f'},
	{ "file-truncate",      0, پوچ, 'F'},
	{ "pipe",               0, پوچ, 'p'},
	{ "mode",               1, پوچ, 'm'},
	{ "owner",              1, پوچ, 'o'},
	{ "symlinks",           0, پوچ, 's'},
	{ "writable",           0, پوچ, 'W'},
	longopts_COMMON
};
ثابت نویسه * ثابت longopts_help[] = {
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
ثابت نویسه *usagestring = پوچ;

ایستا صحیح get_dirfd(نویسه *path, bool symlinks)
{
	نویسه *ch, *item, *path_dupe, *str, *linkpath = پوچ;
	صحیح components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	ساختار stat st;

	اگر (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	اگر (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	وقتی (*ch) {
		اگر (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	اگر (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	وقتی (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		اگر (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		اگر (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		اگر (S_ISLNK(st.st_mode) ) {
			اگر (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			اگر (linkpath)
				آزاد(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			اگر (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} وگرنه {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			آزاد(linkpath);
			linkpath = پوچ;
			item = strtok(پوچ, "/");
			components--;
		}

		آزاد(str);
	}
	آزاد(path_dupe);
	آزاد(linkpath);
	بازگشت dirfd;
}

ایستا نویسه *clean_path(نویسه *path)
{
	نویسه *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	وقتی (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		اگر (!*(ch-1))
			بشکن;
		وقتی (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	وقتی ((ch = strrchr(str, '/'))) {
		اگر (ch == str)
			بشکن;
		اگر (!*(ch+1))
			*ch = 0;
		وگرنه
			بشکن;
	}
	بازگشت str;
}

ایستا صحیح do_create(inode_t type, ثابت نویسه *path, صحیح dirfd, ثابت نویسه *name, صحیح flags, mode_t mode)
{
	صحیح mask = umask(0);
	صحیح fd;

	ایستا ثابت mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	اگر (!mode)
		mode = default_mode[type];

	انتخاب (type) {
	حالت inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		بشکن;
	حالت inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		اگر (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			بازگشت -1;
		}

		بشکن;
	حالت inode_fifo:
		einfo("%s: creating fifo", path);

		اگر (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			بازگشت -1;
		}

		بشکن;
	حالت inode_unknown:
		بشکن;
	}
	umask(mask);

	اگر ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		بازگشت -1;
	}
	بازگشت fd;
}

ایستا صحیح do_check(نویسه *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	صحیح flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	ثابت نویسه *name = basename_c(path);
	ساختار stat st;
	صحیح dirfd, fd;

	اگر (writable) {
		اگر (access(path, W_OK) == 0)
			بازگشت 0;
		اگر (errno != ENOENT || type == inode_unknown)
			بازگشت -1;
	}

	اگر (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	اگر ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		بازگشت -1;

	اگر (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		برو err;
	}

	اگر (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		برو err;
	}

	اگر (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		برو err;
	}

	اگر (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		برو err;
	}

	اگر (mode && (st.st_mode & 07777) != mode) {
		اگر ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			برو err;
		}

		اگر (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			برو err;
		}

		einfo("%s: correcting mode", path);
		اگر (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			برو err;
		}
	}

	اگر (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		اگر ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			برو err;
		}

		اگر (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			برو err;
		}

		einfo("%s: correcting owner", path);
		اگر (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			برو err;
		}
	}

	اگر (selinux_on)
		selinux_util_label(path);
	close(fd);

	بازگشت 0;
err:
	close(fd);
	بازگشت -1;
}

ایستا صحیح parse_owner(ساختار passwd **user, ساختار group **group, ثابت نویسه *owner)
{
	نویسه *u = xstrdup(owner), *g = strchr(u, ':');
	صحیح id = 0, retval = 0;

	اگر (g)
		*g++ = '\0';

	اگر (user && *u) {
		اگر (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		وگرنه
			*user = getpwnam(u);
		اگر (*user == پوچ)
			retval = -1;
	}

	اگر (group && g && *g) {
		اگر (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		وگرنه
			*group = getgrnam(g);
		اگر (*group == پوچ)
			retval = -1;
	}

	آزاد(u);
	بازگشت retval;
}

صحیح اصلی(صحیح argc, نویسه **argv)
{
	صحیح opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	ساختار passwd *pw = پوچ;
	ساختار group *gr = پوچ;
	inode_t type = inode_unknown;
	صحیح retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	نویسه *path = پوچ;

	applet = basename_c(argv[0]);
	وقتی ((opt = getopt_long(argc, argv, getoptstring, longopts, (صحیح *) 0)) != -1) {
		انتخاب (opt) {
		حالت 'D':
			trunc = true;
			/* falls through */
		حالت 'd':
			type = inode_dir;
			بشکن;
		حالت 'F':
			trunc = true;
			/* falls through */
		حالت 'f':
			type = inode_file;
			بشکن;
		حالت 'p':
			type = inode_fifo;
			بشکن;
		حالت 'm':
			اگر (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			بشکن;
		حالت 'o':
			chowner = true;
			اگر (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			بشکن;
		حالت 's':
#ifndef O_PATH
			symlinks = true;
#endif
			بشکن;
		حالت 'W':
			writable = true;
			بشکن;

		case_RC_COMMON_GETOPT
		}
	}

	اگر (optind >= argc)
		usage(EXIT_FAILURE);

	اگر (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	اگر (gr)
		gid = gr->gr_gid;

	اگر (selinux_util_open() == 1)
		selinux_on = true;

	وقتی (optind < argc) {
		path = clean_path(argv[optind]);
		اگر (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		آزاد(path);
	}

	اگر (selinux_on)
		selinux_util_close();

	بازگشت retval;
}
