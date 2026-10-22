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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <grp.h>
#અન્તર્ભાવ <pwd.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "misc.h"
#અન્તર્ભાવ "selinux.h"
#અન્તર્ભાવ "_usage.h"
#અન્તર્ભાવ "helpers.h"

પ્રકારવ્યાખ્યા ગણના {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *extraopts ="path1 [path2] [...]";
અચલ અક્ષર getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{ "directory",          0, ખાલી, 'd'},
	{ "directory-truncate", 0, ખાલી, 'D'},
	{ "file",               0, ખાલી, 'f'},
	{ "file-truncate",      0, ખાલી, 'F'},
	{ "pipe",               0, ખાલી, 'p'},
	{ "mode",               1, ખાલી, 'm'},
	{ "owner",              1, ખાલી, 'o'},
	{ "symlinks",           0, ખાલી, 's'},
	{ "writable",           0, ખાલી, 'W'},
	longopts_COMMON
};
અચલ અક્ષર * અચલ longopts_help[] = {
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
અચલ અક્ષર *usagestring = ખાલી;

સ્થિર પૂર્ણાંક get_dirfd(અક્ષર *path, bool symlinks)
{
	અક્ષર *ch, *item, *path_dupe, *str, *linkpath = ખાલી;
	પૂર્ણાંક components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	સંરચના stat st;

	જો (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	જો (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	જ્યારે (*ch) {
		જો (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	જો (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	જ્યારે (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		જો (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		જો (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		જો (S_ISLNK(st.st_mode) ) {
			જો (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			જો (linkpath)
				મુક્ત(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			જો (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} નહીંતર {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			મુક્ત(linkpath);
			linkpath = ખાલી;
			item = strtok(ખાલી, "/");
			components--;
		}

		મુક્ત(str);
	}
	મુક્ત(path_dupe);
	મુક્ત(linkpath);
	પાછા dirfd;
}

સ્થિર અક્ષર *clean_path(અક્ષર *path)
{
	અક્ષર *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	જ્યારે (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		જો (!*(ch-1))
			તોડો;
		જ્યારે (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	જ્યારે ((ch = strrchr(str, '/'))) {
		જો (ch == str)
			તોડો;
		જો (!*(ch+1))
			*ch = 0;
		નહીંતર
			તોડો;
	}
	પાછા str;
}

સ્થિર પૂર્ણાંક do_create(inode_t type, અચલ અક્ષર *path, પૂર્ણાંક dirfd, અચલ અક્ષર *name, પૂર્ણાંક flags, mode_t mode)
{
	પૂર્ણાંક mask = umask(0);
	પૂર્ણાંક fd;

	સ્થિર અચલ mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	જો (!mode)
		mode = default_mode[type];

	પસંદગી (type) {
	સ્થિતિ inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		તોડો;
	સ્થિતિ inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		જો (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			પાછા -1;
		}

		તોડો;
	સ્થિતિ inode_fifo:
		einfo("%s: creating fifo", path);

		જો (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			પાછા -1;
		}

		તોડો;
	સ્થિતિ inode_unknown:
		તોડો;
	}
	umask(mask);

	જો ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		પાછા -1;
	}
	પાછા fd;
}

સ્થિર પૂર્ણાંક do_check(અક્ષર *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	પૂર્ણાંક flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	અચલ અક્ષર *name = basename_c(path);
	સંરચના stat st;
	પૂર્ણાંક dirfd, fd;

	જો (writable) {
		જો (access(path, W_OK) == 0)
			પાછા 0;
		જો (errno != ENOENT || type == inode_unknown)
			પાછા -1;
	}

	જો (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	જો ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		પાછા -1;

	જો (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		જાઓ err;
	}

	જો (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		જાઓ err;
	}

	જો (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		જાઓ err;
	}

	જો (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		જાઓ err;
	}

	જો (mode && (st.st_mode & 07777) != mode) {
		જો ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			જાઓ err;
		}

		જો (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			જાઓ err;
		}

		einfo("%s: correcting mode", path);
		જો (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			જાઓ err;
		}
	}

	જો (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		જો ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			જાઓ err;
		}

		જો (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			જાઓ err;
		}

		einfo("%s: correcting owner", path);
		જો (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			જાઓ err;
		}
	}

	જો (selinux_on)
		selinux_util_label(path);
	close(fd);

	પાછા 0;
err:
	close(fd);
	પાછા -1;
}

સ્થિર પૂર્ણાંક parse_owner(સંરચના passwd **user, સંરચના group **group, અચલ અક્ષર *owner)
{
	અક્ષર *u = xstrdup(owner), *g = strchr(u, ':');
	પૂર્ણાંક id = 0, retval = 0;

	જો (g)
		*g++ = '\0';

	જો (user && *u) {
		જો (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		નહીંતર
			*user = getpwnam(u);
		જો (*user == ખાલી)
			retval = -1;
	}

	જો (group && g && *g) {
		જો (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		નહીંતર
			*group = getgrnam(g);
		જો (*group == ખાલી)
			retval = -1;
	}

	મુક્ત(u);
	પાછા retval;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	પૂર્ણાંક opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	સંરચના passwd *pw = ખાલી;
	સંરચના group *gr = ખાલી;
	inode_t type = inode_unknown;
	પૂર્ણાંક retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	અક્ષર *path = ખાલી;

	applet = basename_c(argv[0]);
	જ્યારે ((opt = getopt_long(argc, argv, getoptstring, longopts, (પૂર્ણાંક *) 0)) != -1) {
		પસંદગી (opt) {
		સ્થિતિ 'D':
			trunc = true;
			/* falls through */
		સ્થિતિ 'd':
			type = inode_dir;
			તોડો;
		સ્થિતિ 'F':
			trunc = true;
			/* falls through */
		સ્થિતિ 'f':
			type = inode_file;
			તોડો;
		સ્થિતિ 'p':
			type = inode_fifo;
			તોડો;
		સ્થિતિ 'm':
			જો (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			તોડો;
		સ્થિતિ 'o':
			chowner = true;
			જો (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			તોડો;
		સ્થિતિ 's':
#ifndef O_PATH
			symlinks = true;
#endif
			તોડો;
		સ્થિતિ 'W':
			writable = true;
			તોડો;

		case_RC_COMMON_GETOPT
		}
	}

	જો (optind >= argc)
		usage(EXIT_FAILURE);

	જો (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	જો (gr)
		gid = gr->gr_gid;

	જો (selinux_util_open() == 1)
		selinux_on = true;

	જ્યારે (optind < argc) {
		path = clean_path(argv[optind]);
		જો (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		મુક્ત(path);
	}

	જો (selinux_on)
		selinux_util_close();

	પાછા retval;
}
