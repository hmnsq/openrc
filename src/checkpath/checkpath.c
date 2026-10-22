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

typedef ga_nanaa {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

const var_na *applet = NULL;
const var_na *extraopts ="path1 [path2] [...]";
const var_na getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
const sa_nracanaa option longopts[] = {
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
const var_na * const longopts_help[] = {
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
const var_na *usagestring = NULL;

static puur_naa_nka get_dirfd(var_na *path, bool symlinks)
{
	var_na *ch, *item, *path_dupe, *str, *linkpath = NULL;
	puur_naa_nka components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	sa_nracanaa stat st;

	ᬬᬾᬦ᭄ (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	ᬬᬾᬦ᭄ (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	ᬲᭂᬤᭂᬮ (*ch) {
		ᬬᬾᬦ᭄ (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	ᬬᬾᬦ᭄ (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	ᬲᭂᬤᭂᬮ (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		ᬬᬾᬦ᭄ (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		ᬬᬾᬦ᭄ (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		ᬬᬾᬦ᭄ (S_ISLNK(st.st_mode) ) {
			ᬬᬾᬦ᭄ (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			ᬬᬾᬦ᭄ (linkpath)
				free(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			ᬬᬾᬦ᭄ (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} ᬮᬶᬬ {
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
	ᬯᬮᬶ dirfd;
}

static var_na *clean_path(var_na *path)
{
	var_na *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	ᬲᭂᬤᭂᬮ (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		ᬬᬾᬦ᭄ (!*(ch-1))
			bha_nga;
		ᬲᭂᬤᭂᬮ (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	ᬲᭂᬤᭂᬮ ((ch = strrchr(str, '/'))) {
		ᬬᬾᬦ᭄ (ch == str)
			bha_nga;
		ᬬᬾᬦ᭄ (!*(ch+1))
			*ch = 0;
		ᬮᬶᬬ
			bha_nga;
	}
	ᬯᬮᬶ str;
}

static puur_naa_nka do_create(inode_t type, const var_na *path, puur_naa_nka dirfd, const var_na *name, puur_naa_nka flags, mode_t mode)
{
	puur_naa_nka mask = umask(0);
	puur_naa_nka fd;

	static const mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	ᬬᬾᬦ᭄ (!mode)
		mode = default_mode[type];

	switch (type) {
	pak_sa inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		bha_nga;
	pak_sa inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		ᬬᬾᬦ᭄ (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			ᬯᬮᬶ -1;
		}

		bha_nga;
	pak_sa inode_fifo:
		einfo("%s: creating fifo", path);

		ᬬᬾᬦ᭄ (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			ᬯᬮᬶ -1;
		}

		bha_nga;
	pak_sa inode_unknown:
		bha_nga;
	}
	umask(mask);

	ᬬᬾᬦ᭄ ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		ᬯᬮᬶ -1;
	}
	ᬯᬮᬶ fd;
}

static puur_naa_nka do_check(var_na *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	puur_naa_nka flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	const var_na *name = basename_c(path);
	sa_nracanaa stat st;
	puur_naa_nka dirfd, fd;

	ᬬᬾᬦ᭄ (writable) {
		ᬬᬾᬦ᭄ (access(path, W_OK) == 0)
			ᬯᬮᬶ 0;
		ᬬᬾᬦ᭄ (errno != ENOENT || type == inode_unknown)
			ᬯᬮᬶ -1;
	}

	ᬬᬾᬦ᭄ (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	ᬬᬾᬦ᭄ ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		ᬯᬮᬶ -1;

	ᬬᬾᬦ᭄ (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		goto err;
	}

	ᬬᬾᬦ᭄ (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		goto err;
	}

	ᬬᬾᬦ᭄ (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		goto err;
	}

	ᬬᬾᬦ᭄ (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		goto err;
	}

	ᬬᬾᬦ᭄ (mode && (st.st_mode & 07777) != mode) {
		ᬬᬾᬦ᭄ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			goto err;
		}

		ᬬᬾᬦ᭄ (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			goto err;
		}

		einfo("%s: correcting mode", path);
		ᬬᬾᬦ᭄ (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			goto err;
		}
	}

	ᬬᬾᬦ᭄ (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		ᬬᬾᬦ᭄ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			goto err;
		}

		ᬬᬾᬦ᭄ (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			goto err;
		}

		einfo("%s: correcting owner", path);
		ᬬᬾᬦ᭄ (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			goto err;
		}
	}

	ᬬᬾᬦ᭄ (selinux_on)
		selinux_util_label(path);
	close(fd);

	ᬯᬮᬶ 0;
err:
	close(fd);
	ᬯᬮᬶ -1;
}

static puur_naa_nka parse_owner(sa_nracanaa passwd **user, sa_nracanaa group **group, const var_na *owner)
{
	var_na *u = xstrdup(owner), *g = strchr(u, ':');
	puur_naa_nka id = 0, retval = 0;

	ᬬᬾᬦ᭄ (g)
		*g++ = '\0';

	ᬬᬾᬦ᭄ (user && *u) {
		ᬬᬾᬦ᭄ (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		ᬮᬶᬬ
			*user = getpwnam(u);
		ᬬᬾᬦ᭄ (*user == NULL)
			retval = -1;
	}

	ᬬᬾᬦ᭄ (group && g && *g) {
		ᬬᬾᬦ᭄ (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		ᬮᬶᬬ
			*group = getgrnam(g);
		ᬬᬾᬦ᭄ (*group == NULL)
			retval = -1;
	}

	free(u);
	ᬯᬮᬶ retval;
}

puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	puur_naa_nka opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	sa_nracanaa passwd *pw = NULL;
	sa_nracanaa group *gr = NULL;
	inode_t type = inode_unknown;
	puur_naa_nka retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	var_na *path = NULL;

	applet = basename_c(argv[0]);
	ᬲᭂᬤᭂᬮ ((opt = getopt_long(argc, argv, getoptstring, longopts, (puur_naa_nka *) 0)) != -1) {
		switch (opt) {
		pak_sa 'D':
			trunc = true;
			/* falls through */
		pak_sa 'd':
			type = inode_dir;
			bha_nga;
		pak_sa 'F':
			trunc = true;
			/* falls through */
		pak_sa 'f':
			type = inode_file;
			bha_nga;
		pak_sa 'p':
			type = inode_fifo;
			bha_nga;
		pak_sa 'm':
			ᬬᬾᬦ᭄ (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			bha_nga;
		pak_sa 'o':
			chowner = true;
			ᬬᬾᬦ᭄ (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			bha_nga;
		pak_sa 's':
#ifndef O_PATH
			symlinks = true;
#endif
			bha_nga;
		pak_sa 'W':
			writable = true;
			bha_nga;

		case_RC_COMMON_GETOPT
		}
	}

	ᬬᬾᬦ᭄ (optind >= argc)
		usage(EXIT_FAILURE);

	ᬬᬾᬦ᭄ (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	ᬬᬾᬦ᭄ (gr)
		gid = gr->gr_gid;

	ᬬᬾᬦ᭄ (selinux_util_open() == 1)
		selinux_on = true;

	ᬲᭂᬤᭂᬮ (optind < argc) {
		path = clean_path(argv[optind]);
		ᬬᬾᬦ᭄ (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		free(path);
	}

	ᬬᬾᬦ᭄ (selinux_on)
		selinux_util_close();

	ᬯᬮᬶ retval;
}
