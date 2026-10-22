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

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <grp.h>
#অন্তর্ভাব <pwd.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "misc.h"
#অন্তর্ভাব "selinux.h"
#অন্তর্ভাব "_usage.h"
#অন্তর্ভাব "helpers.h"

প্রকারলেখ গণনা {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

ধ্রুবক অক্ষর *applet = ফাঁকা;
ধ্রুবক অক্ষর *extraopts ="path1 [path2] [...]";
ধ্রুবক অক্ষর getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
ধ্রুবক কাঠামো option longopts[] = {
	{ "directory",          0, ফাঁকা, 'd'},
	{ "directory-truncate", 0, ফাঁকা, 'D'},
	{ "file",               0, ফাঁকা, 'f'},
	{ "file-truncate",      0, ফাঁকা, 'F'},
	{ "pipe",               0, ফাঁকা, 'p'},
	{ "mode",               1, ফাঁকা, 'm'},
	{ "owner",              1, ফাঁকা, 'o'},
	{ "symlinks",           0, ফাঁকা, 's'},
	{ "writable",           0, ফাঁকা, 'W'},
	longopts_COMMON
};
ধ্রুবক অক্ষর * ধ্রুবক longopts_help[] = {
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
ধ্রুবক অক্ষর *usagestring = ফাঁকা;

স্থির পূর্ণসংখ্যা get_dirfd(অক্ষর *path, bool symlinks)
{
	অক্ষর *ch, *item, *path_dupe, *str, *linkpath = ফাঁকা;
	পূর্ণসংখ্যা components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	কাঠামো stat st;

	যদি (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	যদি (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	যখন (*ch) {
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
	যখন (dirfd > 0 && item && components > 1) {
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
			linkpath = ফাঁকা;
			item = strtok(ফাঁকা, "/");
			components--;
		}

		মুক্ত(str);
	}
	মুক্ত(path_dupe);
	মুক্ত(linkpath);
	ফেরত dirfd;
}

স্থির অক্ষর *clean_path(অক্ষর *path)
{
	অক্ষর *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	যখন (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		যদি (!*(ch-1))
			বিরতি;
		যখন (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	যখন ((ch = strrchr(str, '/'))) {
		যদি (ch == str)
			বিরতি;
		যদি (!*(ch+1))
			*ch = 0;
		অন্যথা
			বিরতি;
	}
	ফেরত str;
}

স্থির পূর্ণসংখ্যা do_create(inode_t type, ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা dirfd, ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা flags, mode_t mode)
{
	পূর্ণসংখ্যা mask = umask(0);
	পূর্ণসংখ্যা fd;

	স্থির ধ্রুবক mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	যদি (!mode)
		mode = default_mode[type];

	নির্বাচন (type) {
	ক্ষেত্র inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		বিরতি;
	ক্ষেত্র inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		যদি (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			ফেরত -1;
		}

		বিরতি;
	ক্ষেত্র inode_fifo:
		einfo("%s: creating fifo", path);

		যদি (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			ফেরত -1;
		}

		বিরতি;
	ক্ষেত্র inode_unknown:
		বিরতি;
	}
	umask(mask);

	যদি ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		ফেরত -1;
	}
	ফেরত fd;
}

স্থির পূর্ণসংখ্যা do_check(অক্ষর *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	পূর্ণসংখ্যা flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	ধ্রুবক অক্ষর *name = basename_c(path);
	কাঠামো stat st;
	পূর্ণসংখ্যা dirfd, fd;

	যদি (writable) {
		যদি (access(path, W_OK) == 0)
			ফেরত 0;
		যদি (errno != ENOENT || type == inode_unknown)
			ফেরত -1;
	}

	যদি (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	যদি ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		ফেরত -1;

	যদি (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		যাও err;
	}

	যদি (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		যাও err;
	}

	যদি (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		যাও err;
	}

	যদি (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		যাও err;
	}

	যদি (mode && (st.st_mode & 07777) != mode) {
		যদি ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			যাও err;
		}

		যদি (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			যাও err;
		}

		einfo("%s: correcting mode", path);
		যদি (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			যাও err;
		}
	}

	যদি (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		যদি ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			যাও err;
		}

		যদি (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			যাও err;
		}

		einfo("%s: correcting owner", path);
		যদি (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			যাও err;
		}
	}

	যদি (selinux_on)
		selinux_util_label(path);
	close(fd);

	ফেরত 0;
err:
	close(fd);
	ফেরত -1;
}

স্থির পূর্ণসংখ্যা parse_owner(কাঠামো passwd **user, কাঠামো group **group, ধ্রুবক অক্ষর *owner)
{
	অক্ষর *u = xstrdup(owner), *g = strchr(u, ':');
	পূর্ণসংখ্যা id = 0, retval = 0;

	যদি (g)
		*g++ = '\0';

	যদি (user && *u) {
		যদি (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		অন্যথা
			*user = getpwnam(u);
		যদি (*user == ফাঁকা)
			retval = -1;
	}

	যদি (group && g && *g) {
		যদি (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		অন্যথা
			*group = getgrnam(g);
		যদি (*group == ফাঁকা)
			retval = -1;
	}

	মুক্ত(u);
	ফেরত retval;
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	পূর্ণসংখ্যা opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	কাঠামো passwd *pw = ফাঁকা;
	কাঠামো group *gr = ফাঁকা;
	inode_t type = inode_unknown;
	পূর্ণসংখ্যা retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	অক্ষর *path = ফাঁকা;

	applet = basename_c(argv[0]);
	যখন ((opt = getopt_long(argc, argv, getoptstring, longopts, (পূর্ণসংখ্যা *) 0)) != -1) {
		নির্বাচন (opt) {
		ক্ষেত্র 'D':
			trunc = true;
			/* falls through */
		ক্ষেত্র 'd':
			type = inode_dir;
			বিরতি;
		ক্ষেত্র 'F':
			trunc = true;
			/* falls through */
		ক্ষেত্র 'f':
			type = inode_file;
			বিরতি;
		ক্ষেত্র 'p':
			type = inode_fifo;
			বিরতি;
		ক্ষেত্র 'm':
			যদি (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			বিরতি;
		ক্ষেত্র 'o':
			chowner = true;
			যদি (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			বিরতি;
		ক্ষেত্র 's':
#ifndef O_PATH
			symlinks = true;
#endif
			বিরতি;
		ক্ষেত্র 'W':
			writable = true;
			বিরতি;

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

	যখন (optind < argc) {
		path = clean_path(argv[optind]);
		যদি (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		মুক্ত(path);
	}

	যদি (selinux_on)
		selinux_util_close();

	ফেরত retval;
}
