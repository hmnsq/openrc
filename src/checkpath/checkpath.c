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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <grp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pwd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "selinux.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ *extraopts ="path1 [path2] [...]";
ਅਚਲ ਅੱਖਰ getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "directory",          0, ਨਿਰਾ, 'd'},
	{ "directory-truncate", 0, ਨਿਰਾ, 'D'},
	{ "file",               0, ਨਿਰਾ, 'f'},
	{ "file-truncate",      0, ਨਿਰਾ, 'F'},
	{ "pipe",               0, ਨਿਰਾ, 'p'},
	{ "mode",               1, ਨਿਰਾ, 'm'},
	{ "owner",              1, ਨਿਰਾ, 'o'},
	{ "symlinks",           0, ਨਿਰਾ, 's'},
	{ "writable",           0, ਨਿਰਾ, 'W'},
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
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
ਅਚਲ ਅੱਖਰ *usagestring = ਨਿਰਾ;

ਸਥਿਰ ਪੂਰਨ get_dirfd(ਅੱਖਰ *ਰਸਤਾ, bool symlinks)
{
	ਅੱਖਰ *ch, *ਵਸਤੂ, *ਰਸਤਾ_dupe, *str, *linkpath = ਨਿਰਾ;
	ਪੂਰਨ components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	ਬਣਤਰ stat st;

	ਜੇ (!ਰਸਤਾ || *ਰਸਤਾ != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	ਜੇ (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = ਰਸਤਾ;
	ਜਦੋਂ (*ch) {
		ਜੇ (*ch == '/')
			components++;
		ch++;
	}
	ਰਸਤਾ_dupe = xstrdup(ਰਸਤਾ);
	ਵਸਤੂ = strtok(ਰਸਤਾ_dupe, "/");
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_ਰਸਤਾ
	flags |= O_ਰਸਤਾ;
#ਜੇ_ਅੰਤ
	ਜੇ (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	ਜਦੋਂ (dirfd > 0 && ਵਸਤੂ && components > 1) {
		str = xstrdup(linkpath ? linkpath : ਵਸਤੂ);
		new_dirfd = openat(dirfd, str, flags);
		ਜੇ (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, ਰਸਤਾ, str, strerror(errno));
		ਜੇ (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, ਰਸਤਾ, ਵਸਤੂ, strerror(errno));
		ਜੇ (S_ISLNK(st.st_mode) ) {
			ਜੇ (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, ਰਸਤਾ, str);
			linksize = st.st_size+1;
			ਜੇ (linkpath)
				ਮੁਕਤ(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			ਜੇ (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} ਨਹੀਂਤਾਂ {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			ਮੁਕਤ(linkpath);
			linkpath = ਨਿਰਾ;
			ਵਸਤੂ = strtok(ਨਿਰਾ, "/");
			components--;
		}

		ਮੁਕਤ(str);
	}
	ਮੁਕਤ(ਰਸਤਾ_dupe);
	ਮੁਕਤ(linkpath);
	ਵਾਪਸ dirfd;
}

ਸਥਿਰ ਅੱਖਰ *clean_ਰਸਤਾ(ਅੱਖਰ *ਰਸਤਾ)
{
	ਅੱਖਰ *ch, *ch2, *str;

	str = xmalloc(ਲੜੀ_ਲੰਬਾਈ(ਰਸਤਾ) + 1);
	ch = ਰਸਤਾ;
	ch2 = str;
	ਜਦੋਂ (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		ਜੇ (!*(ch-1))
			ਰੋਕੋ;
		ਜਦੋਂ (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	ਜਦੋਂ ((ch = strrchr(str, '/'))) {
		ਜੇ (ch == str)
			ਰੋਕੋ;
		ਜੇ (!*(ch+1))
			*ch = 0;
		ਨਹੀਂਤਾਂ
			ਰੋਕੋ;
	}
	ਵਾਪਸ str;
}

ਸਥਿਰ ਪੂਰਨ do_create(inode_t type, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ dirfd, ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ flags, mode_t ਬਹੁਲਕ)
{
	ਪੂਰਨ ਨਕਾਬ = umask(0);
	ਪੂਰਨ fd;

	ਸਥਿਰ ਅਚਲ mode_t default_ਬਹੁਲਕ[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	ਜੇ (!ਬਹੁਲਕ)
		ਬਹੁਲਕ = default_ਬਹੁਲਕ[type];

	ਚੋਣ (type) {
	ਹਾਲਤ inode_file:
		einfo("%s: creating file", ਰਸਤਾ);
		flags |= O_CREAT;
		ਰੋਕੋ;
	ਹਾਲਤ inode_dir:
		einfo("%s: creating directory", ਰਸਤਾ);

		/* We do not recursively create parents */
		ਜੇ (mkdirat(dirfd, name, ਬਹੁਲਕ) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			ਵਾਪਸ -1;
		}

		ਰੋਕੋ;
	ਹਾਲਤ inode_fifo:
		einfo("%s: creating fifo", ਰਸਤਾ);

		ਜੇ (mkfifoat(dirfd, name, ਬਹੁਲਕ) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			ਵਾਪਸ -1;
		}

		ਰੋਕੋ;
	ਹਾਲਤ inode_unknown:
		ਰੋਕੋ;
	}
	umask(ਨਕਾਬ);

	ਜੇ ((fd = openat(dirfd, name, flags, ਬਹੁਲਕ)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		ਵਾਪਸ -1;
	}
	ਵਾਪਸ fd;
}

ਸਥਿਰ ਪੂਰਨ do_check(ਅੱਖਰ *ਰਸਤਾ, uid_t uid, gid_t gid, mode_t ਬਹੁਲਕ, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	ਪੂਰਨ flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	ਅਚਲ ਅੱਖਰ *name = basename_c(ਰਸਤਾ);
	ਬਣਤਰ stat st;
	ਪੂਰਨ dirfd, fd;

	ਜੇ (writable) {
		ਜੇ (access(ਰਸਤਾ, W_OK) == 0)
			ਵਾਪਸ 0;
		ਜੇ (errno != ENOENT || type == inode_unknown)
			ਵਾਪਸ -1;
	}

	ਜੇ (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(ਰਸਤਾ, symlinks);
	ਜੇ ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, ਰਸਤਾ, dirfd, name, flags, ਬਹੁਲਕ)) == -1)
		ਵਾਪਸ -1;

	ਜੇ (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", ਰਸਤਾ, strerror(errno));
		ਜਾਓ err;
	}

	ਜੇ (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", ਰਸਤਾ);
		ਜਾਓ err;
	}

	ਜੇ (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", ਰਸਤਾ);
		ਜਾਓ err;
	}

	ਜੇ (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", ਰਸਤਾ);
		ਜਾਓ err;
	}

	ਜੇ (ਬਹੁਲਕ && (st.st_mode & 07777) != ਬਹੁਲਕ) {
		ਜੇ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, ਰਸਤਾ);
			ਜਾਓ err;
		}

		ਜੇ (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, ਰਸਤਾ);
			ਜਾਓ err;
		}

		einfo("%s: correcting mode", ਰਸਤਾ);
		ਜੇ (fchmod(fd, ਬਹੁਲਕ)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			ਜਾਓ err;
		}
	}

	ਜੇ (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		ਜੇ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, ਰਸਤਾ);
			ਜਾਓ err;
		}

		ਜੇ (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, ਰਸਤਾ);
			ਜਾਓ err;
		}

		einfo("%s: correcting owner", ਰਸਤਾ);
		ਜੇ (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			ਜਾਓ err;
		}
	}

	ਜੇ (selinux_on)
		selinux_util_label(ਰਸਤਾ);
	close(fd);

	ਵਾਪਸ 0;
err:
	close(fd);
	ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ parse_owner(ਬਣਤਰ passwd **user, ਬਣਤਰ group **group, ਅਚਲ ਅੱਖਰ *owner)
{
	ਅੱਖਰ *u = xstrdup(owner), *g = strchr(u, ':');
	ਪੂਰਨ id = 0, retval = 0;

	ਜੇ (g)
		*g++ = '\0';

	ਜੇ (user && *u) {
		ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		ਨਹੀਂਤਾਂ
			*user = getpwnam(u);
		ਜੇ (*user == ਨਿਰਾ)
			retval = -1;
	}

	ਜੇ (group && g && *g) {
		ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		ਨਹੀਂਤਾਂ
			*group = getgrnam(g);
		ਜੇ (*group == ਨਿਰਾ)
			retval = -1;
	}

	ਮੁਕਤ(u);
	ਵਾਪਸ retval;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਪੂਰਨ opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t ਬਹੁਲਕ = 0;
	ਬਣਤਰ passwd *pw = ਨਿਰਾ;
	ਬਣਤਰ group *gr = ਨਿਰਾ;
	inode_t type = inode_unknown;
	ਪੂਰਨ retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	ਅੱਖਰ *ਰਸਤਾ = ਨਿਰਾ;

	applet = basename_c(argv[0]);
	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring, longopts, (ਪੂਰਨ *) 0)) != -1) {
		ਚੋਣ (opt) {
		ਹਾਲਤ 'D':
			trunc = true;
			/* falls through */
		ਹਾਲਤ 'd':
			type = inode_dir;
			ਰੋਕੋ;
		ਹਾਲਤ 'F':
			trunc = true;
			/* falls through */
		ਹਾਲਤ 'f':
			type = inode_file;
			ਰੋਕੋ;
		ਹਾਲਤ 'p':
			type = inode_fifo;
			ਰੋਕੋ;
		ਹਾਲਤ 'm':
			ਜੇ (parse_ਬਹੁਲਕ(&ਬਹੁਲਕ, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			ਰੋਕੋ;
		ਹਾਲਤ 'o':
			chowner = true;
			ਜੇ (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			ਰੋਕੋ;
		ਹਾਲਤ 's':
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ O_ਰਸਤਾ
			symlinks = true;
#ਜੇ_ਅੰਤ
			ਰੋਕੋ;
		ਹਾਲਤ 'W':
			writable = true;
			ਰੋਕੋ;

		case_RC_COMMON_GETOPT
		}
	}

	ਜੇ (optind >= argc)
		usage(EXIT_FAILURE);

	ਜੇ (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	ਜੇ (gr)
		gid = gr->gr_gid;

	ਜੇ (selinux_util_open() == 1)
		selinux_on = true;

	ਜਦੋਂ (optind < argc) {
		ਰਸਤਾ = clean_ਰਸਤਾ(argv[optind]);
		ਜੇ (do_check(ਰਸਤਾ, uid, gid, ਬਹੁਲਕ, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		ਮੁਕਤ(ਰਸਤਾ);
	}

	ਜੇ (selinux_on)
		selinux_util_close();

	ਵਾਪਸ retval;
}
