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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <grp.h>
#അന്തര്ഭാവ <pwd.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "misc.h"
#അന്തര്ഭാവ "selinux.h"
#അന്തര്ഭാവ "_usage.h"
#അന്തര്ഭാവ "helpers.h"

തരനിർവചനം എണ്ണൽ {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;
സ്ഥിരം അക്ഷരം *extraopts ="path1 [path2] [...]";
സ്ഥിരം അക്ഷരം getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
സ്ഥിരം ഘടന option longopts[] = {
	{ "directory",          0, ശൂന്യ, 'd'},
	{ "directory-truncate", 0, ശൂന്യ, 'D'},
	{ "file",               0, ശൂന്യ, 'f'},
	{ "file-truncate",      0, ശൂന്യ, 'F'},
	{ "pipe",               0, ശൂന്യ, 'p'},
	{ "mode",               1, ശൂന്യ, 'm'},
	{ "owner",              1, ശൂന്യ, 'o'},
	{ "symlinks",           0, ശൂന്യ, 's'},
	{ "writable",           0, ശൂന്യ, 'W'},
	longopts_COMMON
};
സ്ഥിരം അക്ഷരം * സ്ഥിരം longopts_help[] = {
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
സ്ഥിരം അക്ഷരം *usagestring = ശൂന്യ;

സ്ഥിതം പൂർണ്ണസംഖ്യ get_dirfd(അക്ഷരം *path, bool symlinks)
{
	അക്ഷരം *ch, *item, *path_dupe, *str, *linkpath = ശൂന്യ;
	പൂർണ്ണസംഖ്യ components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	ഘടന stat st;

	എങ്കിൽ (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	എങ്കിൽ (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	ആകുമ്പോൾ (*ch) {
		എങ്കിൽ (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	എങ്കിൽ (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	ആകുമ്പോൾ (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		എങ്കിൽ (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		എങ്കിൽ (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		എങ്കിൽ (S_ISLNK(st.st_mode) ) {
			എങ്കിൽ (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			എങ്കിൽ (linkpath)
				സ്വതന്ത്രം(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			എങ്കിൽ (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} അല്ലെങ്കിൽ {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			സ്വതന്ത്രം(linkpath);
			linkpath = ശൂന്യ;
			item = strtok(ശൂന്യ, "/");
			components--;
		}

		സ്വതന്ത്രം(str);
	}
	സ്വതന്ത്രം(path_dupe);
	സ്വതന്ത്രം(linkpath);
	തിരികെ dirfd;
}

സ്ഥിതം അക്ഷരം *clean_path(അക്ഷരം *path)
{
	അക്ഷരം *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	ആകുമ്പോൾ (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		എങ്കിൽ (!*(ch-1))
			നിർത്തുക;
		ആകുമ്പോൾ (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	ആകുമ്പോൾ ((ch = strrchr(str, '/'))) {
		എങ്കിൽ (ch == str)
			നിർത്തുക;
		എങ്കിൽ (!*(ch+1))
			*ch = 0;
		അല്ലെങ്കിൽ
			നിർത്തുക;
	}
	തിരികെ str;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ do_create(inode_t type, സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ dirfd, സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ flags, mode_t mode)
{
	പൂർണ്ണസംഖ്യ mask = umask(0);
	പൂർണ്ണസംഖ്യ fd;

	സ്ഥിതം സ്ഥിരം mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	എങ്കിൽ (!mode)
		mode = default_mode[type];

	തിരഞ്ഞെടുപ്പ് (type) {
	അവസ്ഥ inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		നിർത്തുക;
	അവസ്ഥ inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		എങ്കിൽ (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			തിരികെ -1;
		}

		നിർത്തുക;
	അവസ്ഥ inode_fifo:
		einfo("%s: creating fifo", path);

		എങ്കിൽ (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			തിരികെ -1;
		}

		നിർത്തുക;
	അവസ്ഥ inode_unknown:
		നിർത്തുക;
	}
	umask(mask);

	എങ്കിൽ ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		തിരികെ -1;
	}
	തിരികെ fd;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ do_check(അക്ഷരം *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	പൂർണ്ണസംഖ്യ flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	സ്ഥിരം അക്ഷരം *name = basename_c(path);
	ഘടന stat st;
	പൂർണ്ണസംഖ്യ dirfd, fd;

	എങ്കിൽ (writable) {
		എങ്കിൽ (access(path, W_OK) == 0)
			തിരികെ 0;
		എങ്കിൽ (errno != ENOENT || type == inode_unknown)
			തിരികെ -1;
	}

	എങ്കിൽ (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	എങ്കിൽ ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		തിരികെ -1;

	എങ്കിൽ (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		പോകുക err;
	}

	എങ്കിൽ (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		പോകുക err;
	}

	എങ്കിൽ (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		പോകുക err;
	}

	എങ്കിൽ (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		പോകുക err;
	}

	എങ്കിൽ (mode && (st.st_mode & 07777) != mode) {
		എങ്കിൽ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			പോകുക err;
		}

		എങ്കിൽ (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			പോകുക err;
		}

		einfo("%s: correcting mode", path);
		എങ്കിൽ (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			പോകുക err;
		}
	}

	എങ്കിൽ (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		എങ്കിൽ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			പോകുക err;
		}

		എങ്കിൽ (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			പോകുക err;
		}

		einfo("%s: correcting owner", path);
		എങ്കിൽ (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			പോകുക err;
		}
	}

	എങ്കിൽ (selinux_on)
		selinux_util_label(path);
	close(fd);

	തിരികെ 0;
err:
	close(fd);
	തിരികെ -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_owner(ഘടന passwd **user, ഘടന group **group, സ്ഥിരം അക്ഷരം *owner)
{
	അക്ഷരം *u = xstrdup(owner), *g = strchr(u, ':');
	പൂർണ്ണസംഖ്യ id = 0, retval = 0;

	എങ്കിൽ (g)
		*g++ = '\0';

	എങ്കിൽ (user && *u) {
		എങ്കിൽ (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		അല്ലെങ്കിൽ
			*user = getpwnam(u);
		എങ്കിൽ (*user == ശൂന്യ)
			retval = -1;
	}

	എങ്കിൽ (group && g && *g) {
		എങ്കിൽ (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		അല്ലെങ്കിൽ
			*group = getgrnam(g);
		എങ്കിൽ (*group == ശൂന്യ)
			retval = -1;
	}

	സ്വതന്ത്രം(u);
	തിരികെ retval;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	ഘടന passwd *pw = ശൂന്യ;
	ഘടന group *gr = ശൂന്യ;
	inode_t type = inode_unknown;
	പൂർണ്ണസംഖ്യ retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	അക്ഷരം *path = ശൂന്യ;

	applet = basename_c(argv[0]);
	ആകുമ്പോൾ ((opt = getopt_long(argc, argv, getoptstring, longopts, (പൂർണ്ണസംഖ്യ *) 0)) != -1) {
		തിരഞ്ഞെടുപ്പ് (opt) {
		അവസ്ഥ 'D':
			trunc = true;
			/* falls through */
		അവസ്ഥ 'd':
			type = inode_dir;
			നിർത്തുക;
		അവസ്ഥ 'F':
			trunc = true;
			/* falls through */
		അവസ്ഥ 'f':
			type = inode_file;
			നിർത്തുക;
		അവസ്ഥ 'p':
			type = inode_fifo;
			നിർത്തുക;
		അവസ്ഥ 'm':
			എങ്കിൽ (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			നിർത്തുക;
		അവസ്ഥ 'o':
			chowner = true;
			എങ്കിൽ (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			നിർത്തുക;
		അവസ്ഥ 's':
#ifndef O_PATH
			symlinks = true;
#endif
			നിർത്തുക;
		അവസ്ഥ 'W':
			writable = true;
			നിർത്തുക;

		case_RC_COMMON_GETOPT
		}
	}

	എങ്കിൽ (optind >= argc)
		usage(EXIT_FAILURE);

	എങ്കിൽ (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	എങ്കിൽ (gr)
		gid = gr->gr_gid;

	എങ്കിൽ (selinux_util_open() == 1)
		selinux_on = true;

	ആകുമ്പോൾ (optind < argc) {
		path = clean_path(argv[optind]);
		എങ്കിൽ (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		സ്വതന്ത്രം(path);
	}

	എങ്കിൽ (selinux_on)
		selinux_util_close();

	തിരികെ retval;
}
