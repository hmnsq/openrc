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

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "einfo.h"
#include "misc.h"
#include "selinux.h"
#include "_usage.h"
#include "helpers.h"

typedef enum {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

const char *applet = NULL;
const char *extraopts ="path1 [path2] [...]";
const char getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
const struct option longopts[] = {
	{ "directory",          0, NULL, 'd'},
	{ "directory-truncate", 0, NULL, 'D'},
	{ "file",               0, NULL, 'f'},
	{ "file-truncate",      0, NULL, 'F'},
	{ "pipe",               0, NULL, 'p'},
	{ "mode",               1, NULL, 'm'},
	{ "owner",              1, NULL, 'o'},
	{ "symlinks",           0, NULL, 's'},
	{ "writable",           0, NULL, 'W'},
	longopts_COMMON
};
const char * const longopts_help[] = {
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
const char *usagestring = NULL;

static 整数 get_dirfd(char *path, bool symlinks)
{
	char *ch, *item, *path_dupe, *str, *linkpath = NULL;
	整数 components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	struct stat st;

	如果 (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	如果 (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	当 (*ch) {
		如果 (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	如果 (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	当 (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		如果 (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		如果 (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		如果 (S_ISLNK(st.st_mode) ) {
			如果 (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			如果 (linkpath)
				free(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			如果 (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} 否则 {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			free(linkpath);
			linkpath = NULL;
			item = strtok(NULL, "/");
			components--;
		}

		free(str);
	}
	free(path_dupe);
	free(linkpath);
	返回 dirfd;
}

static char *clean_path(char *path)
{
	char *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	当 (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		如果 (!*(ch-1))
			break;
		当 (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	当 ((ch = strrchr(str, '/'))) {
		如果 (ch == str)
			break;
		如果 (!*(ch+1))
			*ch = 0;
		否则
			break;
	}
	返回 str;
}

static 整数 do_create(inode_t type, const char *path, 整数 dirfd, const char *name, 整数 flags, mode_t mode)
{
	整数 mask = umask(0);
	整数 fd;

	static const mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	如果 (!mode)
		mode = default_mode[type];

	switch (type) {
	case inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		break;
	case inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		如果 (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			返回 -1;
		}

		break;
	case inode_fifo:
		einfo("%s: creating fifo", path);

		如果 (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			返回 -1;
		}

		break;
	case inode_unknown:
		break;
	}
	umask(mask);

	如果 ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		返回 -1;
	}
	返回 fd;
}

static 整数 do_check(char *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	整数 flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	const char *name = basename_c(path);
	struct stat st;
	整数 dirfd, fd;

	如果 (writable) {
		如果 (access(path, W_OK) == 0)
			返回 0;
		如果 (errno != ENOENT || type == inode_unknown)
			返回 -1;
	}

	如果 (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	如果 ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		返回 -1;

	如果 (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		goto err;
	}

	如果 (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		goto err;
	}

	如果 (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		goto err;
	}

	如果 (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		goto err;
	}

	如果 (mode && (st.st_mode & 07777) != mode) {
		如果 ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			goto err;
		}

		如果 (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			goto err;
		}

		einfo("%s: correcting mode", path);
		如果 (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			goto err;
		}
	}

	如果 (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		如果 ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			goto err;
		}

		如果 (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			goto err;
		}

		einfo("%s: correcting owner", path);
		如果 (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			goto err;
		}
	}

	如果 (selinux_on)
		selinux_util_label(path);
	close(fd);

	返回 0;
err:
	close(fd);
	返回 -1;
}

static 整数 parse_owner(struct passwd **user, struct group **group, const char *owner)
{
	char *u = xstrdup(owner), *g = strchr(u, ':');
	整数 id = 0, retval = 0;

	如果 (g)
		*g++ = '\0';

	如果 (user && *u) {
		如果 (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		否则
			*user = getpwnam(u);
		如果 (*user == NULL)
			retval = -1;
	}

	如果 (group && g && *g) {
		如果 (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		否则
			*group = getgrnam(g);
		如果 (*group == NULL)
			retval = -1;
	}

	free(u);
	返回 retval;
}

整数 主要(整数 argc, char **argv)
{
	整数 opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	struct passwd *pw = NULL;
	struct group *gr = NULL;
	inode_t type = inode_unknown;
	整数 retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	char *path = NULL;

	applet = basename_c(argv[0]);
	当 ((opt = getopt_long(argc, argv, getoptstring, longopts, (整数 *) 0)) != -1) {
		switch (opt) {
		case 'D':
			trunc = true;
			/* falls through */
		case 'd':
			type = inode_dir;
			break;
		case 'F':
			trunc = true;
			/* falls through */
		case 'f':
			type = inode_file;
			break;
		case 'p':
			type = inode_fifo;
			break;
		case 'm':
			如果 (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			break;
		case 'o':
			chowner = true;
			如果 (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			break;
		case 's':
#ifndef O_PATH
			symlinks = true;
#endif
			break;
		case 'W':
			writable = true;
			break;

		case_RC_COMMON_GETOPT
		}
	}

	如果 (optind >= argc)
		usage(EXIT_FAILURE);

	如果 (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	如果 (gr)
		gid = gr->gr_gid;

	如果 (selinux_util_open() == 1)
		selinux_on = true;

	当 (optind < argc) {
		path = clean_path(argv[optind]);
		如果 (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		free(path);
	}

	如果 (selinux_on)
		selinux_util_close();

	返回 retval;
}
