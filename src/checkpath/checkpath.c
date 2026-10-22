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

static целое get_dirfd(char *path, bool symlinks)
{
	char *ch, *item, *path_dupe, *str, *linkpath = NULL;
	целое components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	struct stat st;

	если (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	если (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	пока (*ch) {
		если (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	если (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	пока (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		если (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		если (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		если (S_ISLNK(st.st_mode) ) {
			если (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			если (linkpath)
				free(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			если (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} иначе {
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
	вернуть dirfd;
}

static char *clean_path(char *path)
{
	char *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	пока (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		если (!*(ch-1))
			break;
		пока (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	пока ((ch = strrchr(str, '/'))) {
		если (ch == str)
			break;
		если (!*(ch+1))
			*ch = 0;
		иначе
			break;
	}
	вернуть str;
}

static целое do_create(inode_t type, const char *path, целое dirfd, const char *name, целое flags, mode_t mode)
{
	целое mask = umask(0);
	целое fd;

	static const mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	если (!mode)
		mode = default_mode[type];

	switch (type) {
	case inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		break;
	case inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		если (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			вернуть -1;
		}

		break;
	case inode_fifo:
		einfo("%s: creating fifo", path);

		если (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			вернуть -1;
		}

		break;
	case inode_unknown:
		break;
	}
	umask(mask);

	если ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		вернуть -1;
	}
	вернуть fd;
}

static целое do_check(char *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	целое flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	const char *name = basename_c(path);
	struct stat st;
	целое dirfd, fd;

	если (writable) {
		если (access(path, W_OK) == 0)
			вернуть 0;
		если (errno != ENOENT || type == inode_unknown)
			вернуть -1;
	}

	если (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	если ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		вернуть -1;

	если (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		goto err;
	}

	если (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		goto err;
	}

	если (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		goto err;
	}

	если (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		goto err;
	}

	если (mode && (st.st_mode & 07777) != mode) {
		если ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			goto err;
		}

		если (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			goto err;
		}

		einfo("%s: correcting mode", path);
		если (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			goto err;
		}
	}

	если (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		если ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			goto err;
		}

		если (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			goto err;
		}

		einfo("%s: correcting owner", path);
		если (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			goto err;
		}
	}

	если (selinux_on)
		selinux_util_label(path);
	close(fd);

	вернуть 0;
err:
	close(fd);
	вернуть -1;
}

static целое parse_owner(struct passwd **user, struct group **group, const char *owner)
{
	char *u = xstrdup(owner), *g = strchr(u, ':');
	целое id = 0, retval = 0;

	если (g)
		*g++ = '\0';

	если (user && *u) {
		если (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		иначе
			*user = getpwnam(u);
		если (*user == NULL)
			retval = -1;
	}

	если (group && g && *g) {
		если (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		иначе
			*group = getgrnam(g);
		если (*group == NULL)
			retval = -1;
	}

	free(u);
	вернуть retval;
}

целое основной(целое argc, char **argv)
{
	целое opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	struct passwd *pw = NULL;
	struct group *gr = NULL;
	inode_t type = inode_unknown;
	целое retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	char *path = NULL;

	applet = basename_c(argv[0]);
	пока ((opt = getopt_long(argc, argv, getoptstring, longopts, (целое *) 0)) != -1) {
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
			если (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			break;
		case 'o':
			chowner = true;
			если (parse_owner(&pw, &gr, optarg) != 0)
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

	если (optind >= argc)
		usage(EXIT_FAILURE);

	если (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	если (gr)
		gid = gr->gr_gid;

	если (selinux_util_open() == 1)
		selinux_on = true;

	пока (optind < argc) {
		path = clean_path(argv[optind]);
		если (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		free(path);
	}

	если (selinux_on)
		selinux_util_close();

	вернуть retval;
}
