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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <grp.h>
#அந்தர்பாவ <pwd.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "selinux.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "helpers.h"

வகைவரை எண்ணிக்கை {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *extraopts ="path1 [path2] [...]";
மாறிலி எழுத்து getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "directory",          0, வெற்று, 'd'},
	{ "directory-truncate", 0, வெற்று, 'D'},
	{ "file",               0, வெற்று, 'f'},
	{ "file-truncate",      0, வெற்று, 'F'},
	{ "pipe",               0, வெற்று, 'p'},
	{ "mode",               1, வெற்று, 'm'},
	{ "owner",              1, வெற்று, 'o'},
	{ "symlinks",           0, வெற்று, 's'},
	{ "writable",           0, வெற்று, 'W'},
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
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
மாறிலி எழுத்து *usagestring = வெற்று;

நிலையான முழு get_dirfd(எழுத்து *path, bool symlinks)
{
	எழுத்து *ch, *item, *path_dupe, *str, *linkpath = வெற்று;
	முழு components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	கட்டமைப்பு stat st;

	எனில் (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	எனில் (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	போது (*ch) {
		எனில் (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	எனில் (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	போது (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		எனில் (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		எனில் (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		எனில் (S_ISLNK(st.st_mode) ) {
			எனில் (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			எனில் (linkpath)
				விடுவி(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			எனில் (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} இல்லையெனில் {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			விடுவி(linkpath);
			linkpath = வெற்று;
			item = strtok(வெற்று, "/");
			components--;
		}

		விடுவி(str);
	}
	விடுவி(path_dupe);
	விடுவி(linkpath);
	திரும்பு dirfd;
}

நிலையான எழுத்து *clean_path(எழுத்து *path)
{
	எழுத்து *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	போது (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		எனில் (!*(ch-1))
			நிறுத்து;
		போது (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	போது ((ch = strrchr(str, '/'))) {
		எனில் (ch == str)
			நிறுத்து;
		எனில் (!*(ch+1))
			*ch = 0;
		இல்லையெனில்
			நிறுத்து;
	}
	திரும்பு str;
}

நிலையான முழு do_create(inode_t type, மாறிலி எழுத்து *path, முழு dirfd, மாறிலி எழுத்து *name, முழு flags, mode_t mode)
{
	முழு mask = umask(0);
	முழு fd;

	நிலையான மாறிலி mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	எனில் (!mode)
		mode = default_mode[type];

	தேர்வு (type) {
	நிலை inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		நிறுத்து;
	நிலை inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		எனில் (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			திரும்பு -1;
		}

		நிறுத்து;
	நிலை inode_fifo:
		einfo("%s: creating fifo", path);

		எனில் (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			திரும்பு -1;
		}

		நிறுத்து;
	நிலை inode_unknown:
		நிறுத்து;
	}
	umask(mask);

	எனில் ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		திரும்பு -1;
	}
	திரும்பு fd;
}

நிலையான முழு do_check(எழுத்து *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	முழு flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	மாறிலி எழுத்து *name = basename_c(path);
	கட்டமைப்பு stat st;
	முழு dirfd, fd;

	எனில் (writable) {
		எனில் (access(path, W_OK) == 0)
			திரும்பு 0;
		எனில் (errno != ENOENT || type == inode_unknown)
			திரும்பு -1;
	}

	எனில் (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	எனில் ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		திரும்பு -1;

	எனில் (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		செல் err;
	}

	எனில் (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		செல் err;
	}

	எனில் (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		செல் err;
	}

	எனில் (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		செல் err;
	}

	எனில் (mode && (st.st_mode & 07777) != mode) {
		எனில் ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			செல் err;
		}

		எனில் (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			செல் err;
		}

		einfo("%s: correcting mode", path);
		எனில் (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			செல் err;
		}
	}

	எனில் (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		எனில் ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			செல் err;
		}

		எனில் (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			செல் err;
		}

		einfo("%s: correcting owner", path);
		எனில் (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			செல் err;
		}
	}

	எனில் (selinux_on)
		selinux_util_label(path);
	close(fd);

	திரும்பு 0;
err:
	close(fd);
	திரும்பு -1;
}

நிலையான முழு parse_owner(கட்டமைப்பு passwd **user, கட்டமைப்பு group **group, மாறிலி எழுத்து *owner)
{
	எழுத்து *u = xstrdup(owner), *g = strchr(u, ':');
	முழு id = 0, retval = 0;

	எனில் (g)
		*g++ = '\0';

	எனில் (user && *u) {
		எனில் (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		இல்லையெனில்
			*user = getpwnam(u);
		எனில் (*user == வெற்று)
			retval = -1;
	}

	எனில் (group && g && *g) {
		எனில் (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		இல்லையெனில்
			*group = getgrnam(g);
		எனில் (*group == வெற்று)
			retval = -1;
	}

	விடுவி(u);
	திரும்பு retval;
}

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	முழு opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	கட்டமைப்பு passwd *pw = வெற்று;
	கட்டமைப்பு group *gr = வெற்று;
	inode_t type = inode_unknown;
	முழு retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	எழுத்து *path = வெற்று;

	applet = basename_c(argv[0]);
	போது ((opt = getopt_long(argc, argv, getoptstring, longopts, (முழு *) 0)) != -1) {
		தேர்வு (opt) {
		நிலை 'D':
			trunc = true;
			/* falls through */
		நிலை 'd':
			type = inode_dir;
			நிறுத்து;
		நிலை 'F':
			trunc = true;
			/* falls through */
		நிலை 'f':
			type = inode_file;
			நிறுத்து;
		நிலை 'p':
			type = inode_fifo;
			நிறுத்து;
		நிலை 'm':
			எனில் (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			நிறுத்து;
		நிலை 'o':
			chowner = true;
			எனில் (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			நிறுத்து;
		நிலை 's':
#ifndef O_PATH
			symlinks = true;
#endif
			நிறுத்து;
		நிலை 'W':
			writable = true;
			நிறுத்து;

		case_RC_COMMON_GETOPT
		}
	}

	எனில் (optind >= argc)
		usage(EXIT_FAILURE);

	எனில் (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	எனில் (gr)
		gid = gr->gr_gid;

	எனில் (selinux_util_open() == 1)
		selinux_on = true;

	போது (optind < argc) {
		path = clean_path(argv[optind]);
		எனில் (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		விடுவி(path);
	}

	எனில் (selinux_on)
		selinux_util_close();

	திரும்பு retval;
}
