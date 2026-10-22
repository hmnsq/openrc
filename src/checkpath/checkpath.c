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

#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <getopt.h>
#অন্তর্ভাৱ <grp.h>
#অন্তর্ভাৱ <pwd.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <unistd.h>

#অন্তর্ভাৱ "einfo.h"
#অন্তর্ভাৱ "misc.h"
#অন্তর্ভাৱ "selinux.h"
#অন্তর্ভাৱ "_usage.h"
#অন্তর্ভাৱ "helpers.h"

প্রকারনির্দেশ গণনা {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

ধ্রুৱ ৱর্ণ *applet = রিক্ত;
ধ্রুৱ ৱর্ণ *extraopts ="path1 [path2] [...]";
ধ্রুৱ ৱর্ণ getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
ধ্রুৱ সংরচনা option longopts[] = {
	{ "directory",          0, রিক্ত, 'd'},
	{ "directory-truncate", 0, রিক্ত, 'D'},
	{ "file",               0, রিক্ত, 'f'},
	{ "file-truncate",      0, রিক্ত, 'F'},
	{ "pipe",               0, রিক্ত, 'p'},
	{ "mode",               1, রিক্ত, 'm'},
	{ "owner",              1, রিক্ত, 'o'},
	{ "symlinks",           0, রিক্ত, 's'},
	{ "writable",           0, রিক্ত, 'W'},
	longopts_COMMON
};
ধ্রুৱ ৱর্ণ * ধ্রুৱ longopts_help[] = {
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
ধ্রুৱ ৱর্ণ *usagestring = রিক্ত;

স্থির পূর্ণাঙ্ক get_dirfd(ৱর্ণ *path, bool symlinks)
{
	ৱর্ণ *ch, *item, *path_dupe, *str, *linkpath = রিক্ত;
	পূর্ণাঙ্ক components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	সংরচনা stat st;

	যদি (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	যদি (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	যেতিয়ালৈ (*ch) {
		যদি (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	যদি (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	যেতিয়ালৈ (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		যদি (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		যদি (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		যদি (S_ISLNK(st.st_mode) ) {
			যদি (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			যদি (linkpath)
				মুক্ত(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			যদি (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} অন্যথা {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			মুক্ত(linkpath);
			linkpath = রিক্ত;
			item = strtok(রিক্ত, "/");
			components--;
		}

		মুক্ত(str);
	}
	মুক্ত(path_dupe);
	মুক্ত(linkpath);
	ঘূৰাই dirfd;
}

স্থির ৱর্ণ *clean_path(ৱর্ণ *path)
{
	ৱর্ণ *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	যেতিয়ালৈ (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		যদি (!*(ch-1))
			ভঙ্গ;
		যেতিয়ালৈ (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	যেতিয়ালৈ ((ch = strrchr(str, '/'))) {
		যদি (ch == str)
			ভঙ্গ;
		যদি (!*(ch+1))
			*ch = 0;
		অন্যথা
			ভঙ্গ;
	}
	ঘূৰাই str;
}

স্থির পূর্ণাঙ্ক do_create(inode_t type, ধ্রুৱ ৱর্ণ *path, পূর্ণাঙ্ক dirfd, ধ্রুৱ ৱর্ণ *name, পূর্ণাঙ্ক flags, mode_t mode)
{
	পূর্ণাঙ্ক mask = umask(0);
	পূর্ণাঙ্ক fd;

	স্থির ধ্রুৱ mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	যদি (!mode)
		mode = default_mode[type];

	ৱিকল্প (type) {
	পক্ষ inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		ভঙ্গ;
	পক্ষ inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		যদি (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			ঘূৰাই -1;
		}

		ভঙ্গ;
	পক্ষ inode_fifo:
		einfo("%s: creating fifo", path);

		যদি (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			ঘূৰাই -1;
		}

		ভঙ্গ;
	পক্ষ inode_unknown:
		ভঙ্গ;
	}
	umask(mask);

	যদি ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		ঘূৰাই -1;
	}
	ঘূৰাই fd;
}

স্থির পূর্ণাঙ্ক do_check(ৱর্ণ *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	পূর্ণাঙ্ক flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	ধ্রুৱ ৱর্ণ *name = basename_c(path);
	সংরচনা stat st;
	পূর্ণাঙ্ক dirfd, fd;

	যদি (writable) {
		যদি (access(path, W_OK) == 0)
			ঘূৰাই 0;
		যদি (errno != ENOENT || type == inode_unknown)
			ঘূৰাই -1;
	}

	যদি (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	যদি ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		ঘূৰাই -1;

	যদি (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		গচ্ছ err;
	}

	যদি (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		গচ্ছ err;
	}

	যদি (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		গচ্ছ err;
	}

	যদি (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		গচ্ছ err;
	}

	যদি (mode && (st.st_mode & 07777) != mode) {
		যদি ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			গচ্ছ err;
		}

		যদি (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			গচ্ছ err;
		}

		einfo("%s: correcting mode", path);
		যদি (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			গচ্ছ err;
		}
	}

	যদি (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		যদি ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			গচ্ছ err;
		}

		যদি (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			গচ্ছ err;
		}

		einfo("%s: correcting owner", path);
		যদি (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			গচ্ছ err;
		}
	}

	যদি (selinux_on)
		selinux_util_label(path);
	close(fd);

	ঘূৰাই 0;
err:
	close(fd);
	ঘূৰাই -1;
}

স্থির পূর্ণাঙ্ক parse_owner(সংরচনা passwd **user, সংরচনা group **group, ধ্রুৱ ৱর্ণ *owner)
{
	ৱর্ণ *u = xstrdup(owner), *g = strchr(u, ':');
	পূর্ণাঙ্ক id = 0, retval = 0;

	যদি (g)
		*g++ = '\0';

	যদি (user && *u) {
		যদি (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		অন্যথা
			*user = getpwnam(u);
		যদি (*user == রিক্ত)
			retval = -1;
	}

	যদি (group && g && *g) {
		যদি (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		অন্যথা
			*group = getgrnam(g);
		যদি (*group == রিক্ত)
			retval = -1;
	}

	মুক্ত(u);
	ঘূৰাই retval;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
{
	পূর্ণাঙ্ক opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	সংরচনা passwd *pw = রিক্ত;
	সংরচনা group *gr = রিক্ত;
	inode_t type = inode_unknown;
	পূর্ণাঙ্ক retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	ৱর্ণ *path = রিক্ত;

	applet = basename_c(argv[0]);
	যেতিয়ালৈ ((opt = getopt_long(argc, argv, getoptstring, longopts, (পূর্ণাঙ্ক *) 0)) != -1) {
		ৱিকল্প (opt) {
		পক্ষ 'D':
			trunc = true;
			/* falls through */
		পক্ষ 'd':
			type = inode_dir;
			ভঙ্গ;
		পক্ষ 'F':
			trunc = true;
			/* falls through */
		পক্ষ 'f':
			type = inode_file;
			ভঙ্গ;
		পক্ষ 'p':
			type = inode_fifo;
			ভঙ্গ;
		পক্ষ 'm':
			যদি (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			ভঙ্গ;
		পক্ষ 'o':
			chowner = true;
			যদি (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			ভঙ্গ;
		পক্ষ 's':
#ifndef O_PATH
			symlinks = true;
#endif
			ভঙ্গ;
		পক্ষ 'W':
			writable = true;
			ভঙ্গ;

		case_RC_COMMON_GETOPT
		}
	}

	যদি (optind >= argc)
		usage(EXIT_FAILURE);

	যদি (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	যদি (gr)
		gid = gr->gr_gid;

	যদি (selinux_util_open() == 1)
		selinux_on = true;

	যেতিয়ালৈ (optind < argc) {
		path = clean_path(argv[optind]);
		যদি (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		মুক্ত(path);
	}

	যদি (selinux_on)
		selinux_util_close();

	ঘূৰাই retval;
}
