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

static ціле get_dirfd(char *path, bool symlinks)
{
	char *ch, *item, *path_dupe, *str, *linkpath = NULL;
	ціле components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	struct stat st;

	якщо (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	якщо (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	поки (*ch) {
		якщо (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	якщо (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	поки (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		якщо (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		якщо (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		якщо (S_ISLNK(st.st_mode) ) {
			якщо (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			якщо (linkpath)
				free(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			якщо (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} інакше {
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
	повернути dirfd;
}

static char *clean_path(char *path)
{
	char *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	поки (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		якщо (!*(ch-1))
			break;
		поки (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	поки ((ch = strrchr(str, '/'))) {
		якщо (ch == str)
			break;
		якщо (!*(ch+1))
			*ch = 0;
		інакше
			break;
	}
	повернути str;
}

static ціле do_create(inode_t type, const char *path, ціле dirfd, const char *name, ціле flags, mode_t mode)
{
	ціле mask = umask(0);
	ціле fd;

	static const mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	якщо (!mode)
		mode = default_mode[type];

	switch (type) {
	case inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		break;
	case inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		якщо (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			повернути -1;
		}

		break;
	case inode_fifo:
		einfo("%s: creating fifo", path);

		якщо (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			повернути -1;
		}

		break;
	case inode_unknown:
		break;
	}
	umask(mask);

	якщо ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		повернути -1;
	}
	повернути fd;
}

static ціле do_check(char *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	ціле flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	const char *name = basename_c(path);
	struct stat st;
	ціле dirfd, fd;

	якщо (writable) {
		якщо (access(path, W_OK) == 0)
			повернути 0;
		якщо (errno != ENOENT || type == inode_unknown)
			повернути -1;
	}

	якщо (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	якщо ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		повернути -1;

	якщо (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		goto err;
	}

	якщо (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		goto err;
	}

	якщо (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		goto err;
	}

	якщо (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		goto err;
	}

	якщо (mode && (st.st_mode & 07777) != mode) {
		якщо ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			goto err;
		}

		якщо (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			goto err;
		}

		einfo("%s: correcting mode", path);
		якщо (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			goto err;
		}
	}

	якщо (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		якщо ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			goto err;
		}

		якщо (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			goto err;
		}

		einfo("%s: correcting owner", path);
		якщо (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			goto err;
		}
	}

	якщо (selinux_on)
		selinux_util_label(path);
	close(fd);

	повернути 0;
err:
	close(fd);
	повернути -1;
}

static ціле parse_owner(struct passwd **user, struct group **group, const char *owner)
{
	char *u = xstrdup(owner), *g = strchr(u, ':');
	ціле id = 0, retval = 0;

	якщо (g)
		*g++ = '\0';

	якщо (user && *u) {
		якщо (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		інакше
			*user = getpwnam(u);
		якщо (*user == NULL)
			retval = -1;
	}

	якщо (group && g && *g) {
		якщо (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		інакше
			*group = getgrnam(g);
		якщо (*group == NULL)
			retval = -1;
	}

	free(u);
	повернути retval;
}

ціле main(ціле argc, char **argv)
{
	ціле opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	struct passwd *pw = NULL;
	struct group *gr = NULL;
	inode_t type = inode_unknown;
	ціле retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	char *path = NULL;

	applet = basename_c(argv[0]);
	поки ((opt = getopt_long(argc, argv, getoptstring, longopts, (ціле *) 0)) != -1) {
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
			якщо (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			break;
		case 'o':
			chowner = true;
			якщо (parse_owner(&pw, &gr, optarg) != 0)
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

	якщо (optind >= argc)
		usage(EXIT_FAILURE);

	якщо (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	якщо (gr)
		gid = gr->gr_gid;

	якщо (selinux_util_open() == 1)
		selinux_on = true;

	поки (optind < argc) {
		path = clean_path(argv[optind]);
		якщо (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		free(path);
	}

	якщо (selinux_on)
		selinux_util_close();

	повернути retval;
}
