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

static 정수 get_dirfd(char *path, bool symlinks)
{
	char *ch, *item, *path_dupe, *str, *linkpath = NULL;
	정수 components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	struct stat st;

	만약 (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	만약 (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	동안 (*ch) {
		만약 (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	만약 (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	동안 (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		만약 (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		만약 (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		만약 (S_ISLNK(st.st_mode) ) {
			만약 (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			만약 (linkpath)
				free(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			만약 (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} 아니면 {
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
	반환 dirfd;
}

static char *clean_path(char *path)
{
	char *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	동안 (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		만약 (!*(ch-1))
			break;
		동안 (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	동안 ((ch = strrchr(str, '/'))) {
		만약 (ch == str)
			break;
		만약 (!*(ch+1))
			*ch = 0;
		아니면
			break;
	}
	반환 str;
}

static 정수 do_create(inode_t type, const char *path, 정수 dirfd, const char *name, 정수 flags, mode_t mode)
{
	정수 mask = umask(0);
	정수 fd;

	static const mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	만약 (!mode)
		mode = default_mode[type];

	switch (type) {
	case inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		break;
	case inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		만약 (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			반환 -1;
		}

		break;
	case inode_fifo:
		einfo("%s: creating fifo", path);

		만약 (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			반환 -1;
		}

		break;
	case inode_unknown:
		break;
	}
	umask(mask);

	만약 ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		반환 -1;
	}
	반환 fd;
}

static 정수 do_check(char *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	정수 flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	const char *name = basename_c(path);
	struct stat st;
	정수 dirfd, fd;

	만약 (writable) {
		만약 (access(path, W_OK) == 0)
			반환 0;
		만약 (errno != ENOENT || type == inode_unknown)
			반환 -1;
	}

	만약 (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	만약 ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		반환 -1;

	만약 (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		goto err;
	}

	만약 (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		goto err;
	}

	만약 (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		goto err;
	}

	만약 (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		goto err;
	}

	만약 (mode && (st.st_mode & 07777) != mode) {
		만약 ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			goto err;
		}

		만약 (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			goto err;
		}

		einfo("%s: correcting mode", path);
		만약 (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			goto err;
		}
	}

	만약 (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		만약 ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			goto err;
		}

		만약 (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			goto err;
		}

		einfo("%s: correcting owner", path);
		만약 (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			goto err;
		}
	}

	만약 (selinux_on)
		selinux_util_label(path);
	close(fd);

	반환 0;
err:
	close(fd);
	반환 -1;
}

static 정수 parse_owner(struct passwd **user, struct group **group, const char *owner)
{
	char *u = xstrdup(owner), *g = strchr(u, ':');
	정수 id = 0, retval = 0;

	만약 (g)
		*g++ = '\0';

	만약 (user && *u) {
		만약 (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		아니면
			*user = getpwnam(u);
		만약 (*user == NULL)
			retval = -1;
	}

	만약 (group && g && *g) {
		만약 (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		아니면
			*group = getgrnam(g);
		만약 (*group == NULL)
			retval = -1;
	}

	free(u);
	반환 retval;
}

정수 main(정수 argc, char **argv)
{
	정수 opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	struct passwd *pw = NULL;
	struct group *gr = NULL;
	inode_t type = inode_unknown;
	정수 retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	char *path = NULL;

	applet = basename_c(argv[0]);
	동안 ((opt = getopt_long(argc, argv, getoptstring, longopts, (정수 *) 0)) != -1) {
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
			만약 (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			break;
		case 'o':
			chowner = true;
			만약 (parse_owner(&pw, &gr, optarg) != 0)
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

	만약 (optind >= argc)
		usage(EXIT_FAILURE);

	만약 (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	만약 (gr)
		gid = gr->gr_gid;

	만약 (selinux_util_open() == 1)
		selinux_on = true;

	동안 (optind < argc) {
		path = clean_path(argv[optind]);
		만약 (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		free(path);
	}

	만약 (selinux_on)
		selinux_util_close();

	반환 retval;
}
