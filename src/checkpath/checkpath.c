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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <grp.h>
#ଅନ୍ତର୍ଭାଵ <pwd.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "selinux.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *extraopts ="path1 [path2] [...]";
ଅଚଳ ଅକ୍ଷର getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "directory",          0, ଖାଲି, 'd'},
	{ "directory-truncate", 0, ଖାଲି, 'D'},
	{ "file",               0, ଖାଲି, 'f'},
	{ "file-truncate",      0, ଖାଲି, 'F'},
	{ "pipe",               0, ଖାଲି, 'p'},
	{ "mode",               1, ଖାଲି, 'm'},
	{ "owner",              1, ଖାଲି, 'o'},
	{ "symlinks",           0, ଖାଲି, 's'},
	{ "writable",           0, ଖାଲି, 'W'},
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[] = {
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
ଅଚଳ ଅକ୍ଷର *usagestring = ଖାଲି;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_dirfd(ଅକ୍ଷର *path, bool symlinks)
{
	ଅକ୍ଷର *ch, *item, *path_dupe, *str, *linkpath = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	ସଂରଚନା stat st;

	ଯଦି (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	ଯଦି (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	ଯେତେବେଳେ (*ch) {
		ଯଦି (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	ଯଦି (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	ଯେତେବେଳେ (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		ଯଦି (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		ଯଦି (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		ଯଦି (S_ISLNK(st.st_mode) ) {
			ଯଦି (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			ଯଦି (linkpath)
				ମୁକ୍ତ(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			ଯଦି (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} ଅନ୍ୟଥା {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			ମୁକ୍ତ(linkpath);
			linkpath = ଖାଲି;
			item = strtok(ଖାଲି, "/");
			components--;
		}

		ମୁକ୍ତ(str);
	}
	ମୁକ୍ତ(path_dupe);
	ମୁକ୍ତ(linkpath);
	ଫେରନ୍ତୁ dirfd;
}

ସ୍ଥିର ଅକ୍ଷର *clean_path(ଅକ୍ଷର *path)
{
	ଅକ୍ଷର *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	ଯେତେବେଳେ (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		ଯଦି (!*(ch-1))
			ଭାଙ୍ଗ;
		ଯେତେବେଳେ (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	ଯେତେବେଳେ ((ch = strrchr(str, '/'))) {
		ଯଦି (ch == str)
			ଭାଙ୍ଗ;
		ଯଦି (!*(ch+1))
			*ch = 0;
		ଅନ୍ୟଥା
			ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ str;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_create(inode_t type, ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd, ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, mode_t mode)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask = umask(0);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

	ସ୍ଥିର ଅଚଳ mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	ଯଦି (!mode)
		mode = default_mode[type];

	ବାଛ (type) {
	ସ୍ଥିତି inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		ଯଦି (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			ଫେରନ୍ତୁ -1;
		}

		ଭାଙ୍ଗ;
	ସ୍ଥିତି inode_fifo:
		einfo("%s: creating fifo", path);

		ଯଦି (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			ଫେରନ୍ତୁ -1;
		}

		ଭାଙ୍ଗ;
	ସ୍ଥିତି inode_unknown:
		ଭାଙ୍ଗ;
	}
	umask(mask);

	ଯଦି ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		ଫେରନ୍ତୁ -1;
	}
	ଫେରନ୍ତୁ fd;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_check(ଅକ୍ଷର *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	ଅଚଳ ଅକ୍ଷର *name = basename_c(path);
	ସଂରଚନା stat st;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd, fd;

	ଯଦି (writable) {
		ଯଦି (access(path, W_OK) == 0)
			ଫେରନ୍ତୁ 0;
		ଯଦି (errno != ENOENT || type == inode_unknown)
			ଫେରନ୍ତୁ -1;
	}

	ଯଦି (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	ଯଦି ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		ଫେରନ୍ତୁ -1;

	ଯଦି (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		ଯାଅ err;
	}

	ଯଦି (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		ଯାଅ err;
	}

	ଯଦି (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		ଯାଅ err;
	}

	ଯଦି (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		ଯାଅ err;
	}

	ଯଦି (mode && (st.st_mode & 07777) != mode) {
		ଯଦି ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			ଯାଅ err;
		}

		ଯଦି (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			ଯାଅ err;
		}

		einfo("%s: correcting mode", path);
		ଯଦି (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			ଯାଅ err;
		}
	}

	ଯଦି (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		ଯଦି ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			ଯାଅ err;
		}

		ଯଦି (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			ଯାଅ err;
		}

		einfo("%s: correcting owner", path);
		ଯଦି (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			ଯାଅ err;
		}
	}

	ଯଦି (selinux_on)
		selinux_util_label(path);
	close(fd);

	ଫେରନ୍ତୁ 0;
err:
	close(fd);
	ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_owner(ସଂରଚନା passwd **user, ସଂରଚନା group **group, ଅଚଳ ଅକ୍ଷର *owner)
{
	ଅକ୍ଷର *u = xstrdup(owner), *g = strchr(u, ':');
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା id = 0, retval = 0;

	ଯଦି (g)
		*g++ = '\0';

	ଯଦି (user && *u) {
		ଯଦି (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		ଅନ୍ୟଥା
			*user = getpwnam(u);
		ଯଦି (*user == ଖାଲି)
			retval = -1;
	}

	ଯଦି (group && g && *g) {
		ଯଦି (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		ଅନ୍ୟଥା
			*group = getgrnam(g);
		ଯଦି (*group == ଖାଲି)
			retval = -1;
	}

	ମୁକ୍ତ(u);
	ଫେରନ୍ତୁ retval;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	ସଂରଚନା passwd *pw = ଖାଲି;
	ସଂରଚନା group *gr = ଖାଲି;
	inode_t type = inode_unknown;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	ଅକ୍ଷର *path = ଖାଲି;

	applet = basename_c(argv[0]);
	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring, longopts, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) 0)) != -1) {
		ବାଛ (opt) {
		ସ୍ଥିତି 'D':
			trunc = true;
			/* falls through */
		ସ୍ଥିତି 'd':
			type = inode_dir;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'F':
			trunc = true;
			/* falls through */
		ସ୍ଥିତି 'f':
			type = inode_file;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'p':
			type = inode_fifo;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'm':
			ଯଦି (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'o':
			chowner = true;
			ଯଦି (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 's':
#ifndef O_PATH
			symlinks = true;
#endif
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'W':
			writable = true;
			ଭାଙ୍ଗ;

		case_RC_COMMON_GETOPT
		}
	}

	ଯଦି (optind >= argc)
		usage(EXIT_FAILURE);

	ଯଦି (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	ଯଦି (gr)
		gid = gr->gr_gid;

	ଯଦି (selinux_util_open() == 1)
		selinux_on = true;

	ଯେତେବେଳେ (optind < argc) {
		path = clean_path(argv[optind]);
		ଯଦି (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		ମୁକ୍ତ(path);
	}

	ଯଦି (selinux_on)
		selinux_util_close();

	ଫେରନ୍ତୁ retval;
}
