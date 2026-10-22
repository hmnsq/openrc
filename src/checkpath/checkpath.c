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

	ᰀᰧ (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	ᰀᰧ (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	ᰀᰬ (*ch) {
		ᰀᰧ (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	ᰀᰧ (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	ᰀᰬ (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		ᰀᰧ (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		ᰀᰧ (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		ᰀᰧ (S_ISLNK(st.st_mode) ) {
			ᰀᰧ (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			ᰀᰧ (linkpath)
				free(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			ᰀᰧ (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} ᰀᰧᰀ {
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
	ᰀᰧᰌ dirfd;
}

static var_na *clean_path(var_na *path)
{
	var_na *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	ᰀᰬ (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		ᰀᰧ (!*(ch-1))
			bha_nga;
		ᰀᰬ (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	ᰀᰬ ((ch = strrchr(str, '/'))) {
		ᰀᰧ (ch == str)
			bha_nga;
		ᰀᰧ (!*(ch+1))
			*ch = 0;
		ᰀᰧᰀ
			bha_nga;
	}
	ᰀᰧᰌ str;
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

	ᰀᰧ (!mode)
		mode = default_mode[type];

	switch (type) {
	pak_sa inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		bha_nga;
	pak_sa inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		ᰀᰧ (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			ᰀᰧᰌ -1;
		}

		bha_nga;
	pak_sa inode_fifo:
		einfo("%s: creating fifo", path);

		ᰀᰧ (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			ᰀᰧᰌ -1;
		}

		bha_nga;
	pak_sa inode_unknown:
		bha_nga;
	}
	umask(mask);

	ᰀᰧ ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		ᰀᰧᰌ -1;
	}
	ᰀᰧᰌ fd;
}

static puur_naa_nka do_check(var_na *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	puur_naa_nka flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	const var_na *name = basename_c(path);
	sa_nracanaa stat st;
	puur_naa_nka dirfd, fd;

	ᰀᰧ (writable) {
		ᰀᰧ (access(path, W_OK) == 0)
			ᰀᰧᰌ 0;
		ᰀᰧ (errno != ENOENT || type == inode_unknown)
			ᰀᰧᰌ -1;
	}

	ᰀᰧ (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	ᰀᰧ ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		ᰀᰧᰌ -1;

	ᰀᰧ (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		goto err;
	}

	ᰀᰧ (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		goto err;
	}

	ᰀᰧ (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		goto err;
	}

	ᰀᰧ (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		goto err;
	}

	ᰀᰧ (mode && (st.st_mode & 07777) != mode) {
		ᰀᰧ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			goto err;
		}

		ᰀᰧ (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			goto err;
		}

		einfo("%s: correcting mode", path);
		ᰀᰧ (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			goto err;
		}
	}

	ᰀᰧ (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		ᰀᰧ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			goto err;
		}

		ᰀᰧ (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			goto err;
		}

		einfo("%s: correcting owner", path);
		ᰀᰧ (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			goto err;
		}
	}

	ᰀᰧ (selinux_on)
		selinux_util_label(path);
	close(fd);

	ᰀᰧᰌ 0;
err:
	close(fd);
	ᰀᰧᰌ -1;
}

static puur_naa_nka parse_owner(sa_nracanaa passwd **user, sa_nracanaa group **group, const var_na *owner)
{
	var_na *u = xstrdup(owner), *g = strchr(u, ':');
	puur_naa_nka id = 0, retval = 0;

	ᰀᰧ (g)
		*g++ = '\0';

	ᰀᰧ (user && *u) {
		ᰀᰧ (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		ᰀᰧᰀ
			*user = getpwnam(u);
		ᰀᰧ (*user == NULL)
			retval = -1;
	}

	ᰀᰧ (group && g && *g) {
		ᰀᰧ (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		ᰀᰧᰀ
			*group = getgrnam(g);
		ᰀᰧ (*group == NULL)
			retval = -1;
	}

	free(u);
	ᰀᰧᰌ retval;
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
	ᰀᰬ ((opt = getopt_long(argc, argv, getoptstring, longopts, (puur_naa_nka *) 0)) != -1) {
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
			ᰀᰧ (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			bha_nga;
		pak_sa 'o':
			chowner = true;
			ᰀᰧ (parse_owner(&pw, &gr, optarg) != 0)
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

	ᰀᰧ (optind >= argc)
		usage(EXIT_FAILURE);

	ᰀᰧ (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	ᰀᰧ (gr)
		gid = gr->gr_gid;

	ᰀᰧ (selinux_util_open() == 1)
		selinux_on = true;

	ᰀᰬ (optind < argc) {
		path = clean_path(argv[optind]);
		ᰀᰧ (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		free(path);
	}

	ᰀᰧ (selinux_on)
		selinux_util_close();

	ᰀᰧᰌ retval;
}
