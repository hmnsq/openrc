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

#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <getopt.h>
#ضمن <grp.h>
#ضمن <pwd.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/stat.h>
#ضمن <sys/types.h>
#ضمن <unistd.h>

#ضمن "einfo.h"
#ضمن "misc.h"
#ضمن "selinux.h"
#ضمن "_usage.h"
#ضمن "helpers.h"

تعريفنوع تعداد {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

ثابت حرف *applet = عدم;
ثابت حرف *extraopts ="path1 [path2] [...]";
ثابت حرف getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "directory",          0, عدم, 'd'},
	{ "directory-truncate", 0, عدم, 'D'},
	{ "file",               0, عدم, 'f'},
	{ "file-truncate",      0, عدم, 'F'},
	{ "pipe",               0, عدم, 'p'},
	{ "mode",               1, عدم, 'm'},
	{ "owner",              1, عدم, 'o'},
	{ "symlinks",           0, عدم, 's'},
	{ "writable",           0, عدم, 'W'},
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
ثابت حرف *usagestring = عدم;

ساكن صحيح get_dirfd(حرف *رستو, bool symlinks)
{
	حرف *ch, *شي, *رستو_dupe, *str, *linkpath = عدم;
	صحيح components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	بنية stat st;

	إذا (!رستو || *رستو != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	إذا (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = رستو;
	طالما (*ch) {
		إذا (*ch == '/')
			components++;
		ch++;
	}
	رستو_dupe = xstrdup(رستو);
	شي = strtok(رستو_dupe, "/");
#جيڪڏهن_وصف_ٿيل O_رستو
	flags |= O_رستو;
#جيڪڏهن_پورو
	إذا (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	طالما (dirfd > 0 && شي && components > 1) {
		str = xstrdup(linkpath ? linkpath : شي);
		new_dirfd = openat(dirfd, str, flags);
		إذا (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, رستو, str, strerror(errno));
		إذا (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, رستو, شي, strerror(errno));
		إذا (S_ISLNK(st.st_mode) ) {
			إذا (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, رستو, str);
			linksize = st.st_size+1;
			إذا (linkpath)
				حرر(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			إذا (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} وإلا {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			حرر(linkpath);
			linkpath = عدم;
			شي = strtok(عدم, "/");
			components--;
		}

		حرر(str);
	}
	حرر(رستو_dupe);
	حرر(linkpath);
	أرجع dirfd;
}

ساكن حرف *clean_رستو(حرف *رستو)
{
	حرف *ch, *ch2, *str;

	str = xmalloc(لڙي_ڊيگهه(رستو) + 1);
	ch = رستو;
	ch2 = str;
	طالما (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		إذا (!*(ch-1))
			اكسر;
		طالما (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	طالما ((ch = strrchr(str, '/'))) {
		إذا (ch == str)
			اكسر;
		إذا (!*(ch+1))
			*ch = 0;
		وإلا
			اكسر;
	}
	أرجع str;
}

ساكن صحيح do_create(inode_t type, ثابت حرف *رستو, صحيح dirfd, ثابت حرف *name, صحيح flags, mode_t گهڻائي)
{
	صحيح نقاب = umask(0);
	صحيح fd;

	ساكن ثابت mode_t default_گهڻائي[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	إذا (!گهڻائي)
		گهڻائي = default_گهڻائي[type];

	بدل (type) {
	حالة inode_file:
		einfo("%s: creating file", رستو);
		flags |= O_CREAT;
		اكسر;
	حالة inode_dir:
		einfo("%s: creating directory", رستو);

		/* We do not recursively create parents */
		إذا (mkdirat(dirfd, name, گهڻائي) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			أرجع -1;
		}

		اكسر;
	حالة inode_fifo:
		einfo("%s: creating fifo", رستو);

		إذا (mkfifoat(dirfd, name, گهڻائي) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			أرجع -1;
		}

		اكسر;
	حالة inode_unknown:
		اكسر;
	}
	umask(نقاب);

	إذا ((fd = openat(dirfd, name, flags, گهڻائي)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		أرجع -1;
	}
	أرجع fd;
}

ساكن صحيح do_check(حرف *رستو, uid_t uid, gid_t gid, mode_t گهڻائي, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	صحيح flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	ثابت حرف *name = basename_c(رستو);
	بنية stat st;
	صحيح dirfd, fd;

	إذا (writable) {
		إذا (access(رستو, W_OK) == 0)
			أرجع 0;
		إذا (errno != ENOENT || type == inode_unknown)
			أرجع -1;
	}

	إذا (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(رستو, symlinks);
	إذا ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, رستو, dirfd, name, flags, گهڻائي)) == -1)
		أرجع -1;

	إذا (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", رستو, strerror(errno));
		اذهب err;
	}

	إذا (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", رستو);
		اذهب err;
	}

	إذا (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", رستو);
		اذهب err;
	}

	إذا (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", رستو);
		اذهب err;
	}

	إذا (گهڻائي && (st.st_mode & 07777) != گهڻائي) {
		إذا ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, رستو);
			اذهب err;
		}

		إذا (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, رستو);
			اذهب err;
		}

		einfo("%s: correcting mode", رستو);
		إذا (fchmod(fd, گهڻائي)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			اذهب err;
		}
	}

	إذا (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		إذا ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, رستو);
			اذهب err;
		}

		إذا (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, رستو);
			اذهب err;
		}

		einfo("%s: correcting owner", رستو);
		إذا (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			اذهب err;
		}
	}

	إذا (selinux_on)
		selinux_util_label(رستو);
	close(fd);

	أرجع 0;
err:
	close(fd);
	أرجع -1;
}

ساكن صحيح parse_owner(بنية passwd **user, بنية group **group, ثابت حرف *owner)
{
	حرف *u = xstrdup(owner), *g = strchr(u, ':');
	صحيح id = 0, retval = 0;

	إذا (g)
		*g++ = '\0';

	إذا (user && *u) {
		إذا (لڙي_مان_پڙهو(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		وإلا
			*user = getpwnam(u);
		إذا (*user == عدم)
			retval = -1;
	}

	إذا (group && g && *g) {
		إذا (لڙي_مان_پڙهو(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		وإلا
			*group = getgrnam(g);
		إذا (*group == عدم)
			retval = -1;
	}

	حرر(u);
	أرجع retval;
}

صحيح مکيه(صحيح argc, حرف **argv)
{
	صحيح opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t گهڻائي = 0;
	بنية passwd *pw = عدم;
	بنية group *gr = عدم;
	inode_t type = inode_unknown;
	صحيح retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	حرف *رستو = عدم;

	applet = basename_c(argv[0]);
	طالما ((opt = getopt_long(argc, argv, getoptstring, longopts, (صحيح *) 0)) != -1) {
		بدل (opt) {
		حالة 'D':
			trunc = true;
			/* falls through */
		حالة 'd':
			type = inode_dir;
			اكسر;
		حالة 'F':
			trunc = true;
			/* falls through */
		حالة 'f':
			type = inode_file;
			اكسر;
		حالة 'p':
			type = inode_fifo;
			اكسر;
		حالة 'm':
			إذا (parse_گهڻائي(&گهڻائي, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			اكسر;
		حالة 'o':
			chowner = true;
			إذا (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			اكسر;
		حالة 's':
#جيڪڏهن_وصف_نه_ٿيل O_رستو
			symlinks = true;
#جيڪڏهن_پورو
			اكسر;
		حالة 'W':
			writable = true;
			اكسر;

		case_RC_COMMON_GETOPT
		}
	}

	إذا (optind >= argc)
		usage(EXIT_FAILURE);

	إذا (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	إذا (gr)
		gid = gr->gr_gid;

	إذا (selinux_util_open() == 1)
		selinux_on = true;

	طالما (optind < argc) {
		رستو = clean_رستو(argv[optind]);
		إذا (do_check(رستو, uid, gid, گهڻائي, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		حرر(رستو);
	}

	إذا (selinux_on)
		selinux_util_close();

	أرجع retval;
}
