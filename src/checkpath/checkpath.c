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

#شامل <errno.h>
#شامل <fcntl.h>
#شامل <getopt.h>
#شامل <grp.h>
#شامل <pwd.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/stat.h>
#شامل <sys/types.h>
#شامل <unistd.h>

#شامل "einfo.h"
#شامل "misc.h"
#شامل "selinux.h"
#شامل "_usage.h"
#شامل "helpers.h"

قسمتعریف شمار {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

ثابت حرف *applet = تہی;
ثابت حرف *extraopts ="path1 [path2] [...]";
ثابت حرف getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{ "directory",          0, تہی, 'd'},
	{ "directory-truncate", 0, تہی, 'D'},
	{ "file",               0, تہی, 'f'},
	{ "file-truncate",      0, تہی, 'F'},
	{ "pipe",               0, تہی, 'p'},
	{ "mode",               1, تہی, 'm'},
	{ "owner",              1, تہی, 'o'},
	{ "symlinks",           0, تہی, 's'},
	{ "writable",           0, تہی, 'W'},
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
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
ثابت حرف *usagestring = تہی;

ساکن صحیح get_dirfd(حرف *راستہ, bool symlinks)
{
	حرف *ch, *شے, *راستہ_dupe, *str, *linkpath = تہی;
	صحیح components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	ساخت stat st;

	اگر (!راستہ || *راستہ != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	اگر (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = راستہ;
	جب (*ch) {
		اگر (*ch == '/')
			components++;
		ch++;
	}
	راستہ_dupe = xstrdup(راستہ);
	شے = strtok(راستہ_dupe, "/");
#اگر_تعریف_شدہ O_راستہ
	flags |= O_راستہ;
#اگر_ختم
	اگر (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	جب (dirfd > 0 && شے && components > 1) {
		str = xstrdup(linkpath ? linkpath : شے);
		new_dirfd = openat(dirfd, str, flags);
		اگر (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, راستہ, str, strerror(errno));
		اگر (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, راستہ, شے, strerror(errno));
		اگر (S_ISLNK(st.st_mode) ) {
			اگر (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, راستہ, str);
			linksize = st.st_size+1;
			اگر (linkpath)
				آزاد(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			اگر (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} ورنہ {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			آزاد(linkpath);
			linkpath = تہی;
			شے = strtok(تہی, "/");
			components--;
		}

		آزاد(str);
	}
	آزاد(راستہ_dupe);
	آزاد(linkpath);
	واپس dirfd;
}

ساکن حرف *clean_راستہ(حرف *راستہ)
{
	حرف *ch, *ch2, *str;

	str = xmalloc(لڑی_لمبائی(راستہ) + 1);
	ch = راستہ;
	ch2 = str;
	جب (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		اگر (!*(ch-1))
			توڑو;
		جب (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	جب ((ch = strrchr(str, '/'))) {
		اگر (ch == str)
			توڑو;
		اگر (!*(ch+1))
			*ch = 0;
		ورنہ
			توڑو;
	}
	واپس str;
}

ساکن صحیح do_create(inode_t type, ثابت حرف *راستہ, صحیح dirfd, ثابت حرف *name, صحیح flags, mode_t منوال)
{
	صحیح نقاب = umask(0);
	صحیح fd;

	ساکن ثابت mode_t default_منوال[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	اگر (!منوال)
		منوال = default_منوال[type];

	انتخاب (type) {
	حالت inode_file:
		einfo("%s: creating file", راستہ);
		flags |= O_CREAT;
		توڑو;
	حالت inode_dir:
		einfo("%s: creating directory", راستہ);

		/* We do not recursively create parents */
		اگر (mkdirat(dirfd, name, منوال) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			واپس -1;
		}

		توڑو;
	حالت inode_fifo:
		einfo("%s: creating fifo", راستہ);

		اگر (mkfifoat(dirfd, name, منوال) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			واپس -1;
		}

		توڑو;
	حالت inode_unknown:
		توڑو;
	}
	umask(نقاب);

	اگر ((fd = openat(dirfd, name, flags, منوال)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		واپس -1;
	}
	واپس fd;
}

ساکن صحیح do_check(حرف *راستہ, uid_t uid, gid_t gid, mode_t منوال, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	صحیح flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	ثابت حرف *name = basename_c(راستہ);
	ساخت stat st;
	صحیح dirfd, fd;

	اگر (writable) {
		اگر (access(راستہ, W_OK) == 0)
			واپس 0;
		اگر (errno != ENOENT || type == inode_unknown)
			واپس -1;
	}

	اگر (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(راستہ, symlinks);
	اگر ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, راستہ, dirfd, name, flags, منوال)) == -1)
		واپس -1;

	اگر (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", راستہ, strerror(errno));
		جاؤ err;
	}

	اگر (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", راستہ);
		جاؤ err;
	}

	اگر (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", راستہ);
		جاؤ err;
	}

	اگر (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", راستہ);
		جاؤ err;
	}

	اگر (منوال && (st.st_mode & 07777) != منوال) {
		اگر ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, راستہ);
			جاؤ err;
		}

		اگر (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, راستہ);
			جاؤ err;
		}

		einfo("%s: correcting mode", راستہ);
		اگر (fchmod(fd, منوال)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			جاؤ err;
		}
	}

	اگر (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		اگر ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, راستہ);
			جاؤ err;
		}

		اگر (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, راستہ);
			جاؤ err;
		}

		einfo("%s: correcting owner", راستہ);
		اگر (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			جاؤ err;
		}
	}

	اگر (selinux_on)
		selinux_util_label(راستہ);
	close(fd);

	واپس 0;
err:
	close(fd);
	واپس -1;
}

ساکن صحیح parse_owner(ساخت passwd **user, ساخت group **group, ثابت حرف *owner)
{
	حرف *u = xstrdup(owner), *g = strchr(u, ':');
	صحیح id = 0, retval = 0;

	اگر (g)
		*g++ = '\0';

	اگر (user && *u) {
		اگر (لڑی_سے_پڑھو(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		ورنہ
			*user = getpwnam(u);
		اگر (*user == تہی)
			retval = -1;
	}

	اگر (group && g && *g) {
		اگر (لڑی_سے_پڑھو(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		ورنہ
			*group = getgrnam(g);
		اگر (*group == تہی)
			retval = -1;
	}

	آزاد(u);
	واپس retval;
}

صحیح مین(صحیح argc, حرف **argv)
{
	صحیح opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t منوال = 0;
	ساخت passwd *pw = تہی;
	ساخت group *gr = تہی;
	inode_t type = inode_unknown;
	صحیح retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	حرف *راستہ = تہی;

	applet = basename_c(argv[0]);
	جب ((opt = getopt_long(argc, argv, getoptstring, longopts, (صحیح *) 0)) != -1) {
		انتخاب (opt) {
		حالت 'D':
			trunc = true;
			/* falls through */
		حالت 'd':
			type = inode_dir;
			توڑو;
		حالت 'F':
			trunc = true;
			/* falls through */
		حالت 'f':
			type = inode_file;
			توڑو;
		حالت 'p':
			type = inode_fifo;
			توڑو;
		حالت 'm':
			اگر (parse_منوال(&منوال, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			توڑو;
		حالت 'o':
			chowner = true;
			اگر (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			توڑو;
		حالت 's':
#اگر_غیر_تعریف_شدہ O_راستہ
			symlinks = true;
#اگر_ختم
			توڑو;
		حالت 'W':
			writable = true;
			توڑو;

		case_RC_COMMON_GETOPT
		}
	}

	اگر (optind >= argc)
		usage(EXIT_FAILURE);

	اگر (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	اگر (gr)
		gid = gr->gr_gid;

	اگر (selinux_util_open() == 1)
		selinux_on = true;

	جب (optind < argc) {
		راستہ = clean_راستہ(argv[optind]);
		اگر (do_check(راستہ, uid, gid, منوال, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		آزاد(راستہ);
	}

	اگر (selinux_on)
		selinux_util_close();

	واپس retval;
}
