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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <grp.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "selinux.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts ="path1 [path2] [...]";
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{ "directory",          0, ಖಾಲಿ, 'd'},
	{ "directory-truncate", 0, ಖಾಲಿ, 'D'},
	{ "file",               0, ಖಾಲಿ, 'f'},
	{ "file-truncate",      0, ಖಾಲಿ, 'F'},
	{ "pipe",               0, ಖಾಲಿ, 'p'},
	{ "mode",               1, ಖಾಲಿ, 'm'},
	{ "owner",              1, ಖಾಲಿ, 'o'},
	{ "symlinks",           0, ಖಾಲಿ, 's'},
	{ "writable",           0, ಖಾಲಿ, 'W'},
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[] = {
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
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ಖಾಲಿ;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_dirfd(ಅಕ್ಷರ *ಮಾರ್ಗ, bool symlinks)
{
	ಅಕ್ಷರ *ch, *ಅಂಶ, *ಮಾರ್ಗ_dupe, *str, *linkpath = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	ರಚನೆ stat st;

	ಆದರೆ (!ಮಾರ್ಗ || *ಮಾರ್ಗ != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	ಆದರೆ (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = ಮಾರ್ಗ;
	ಆಗ (*ch) {
		ಆದರೆ (*ch == '/')
			components++;
		ch++;
	}
	ಮಾರ್ಗ_dupe = xstrdup(ಮಾರ್ಗ);
	ಅಂಶ = strtok(ಮಾರ್ಗ_dupe, "/");
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_ಮಾರ್ಗ
	flags |= O_ಮಾರ್ಗ;
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	ಆಗ (dirfd > 0 && ಅಂಶ && components > 1) {
		str = xstrdup(linkpath ? linkpath : ಅಂಶ);
		new_dirfd = openat(dirfd, str, flags);
		ಆದರೆ (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, ಮಾರ್ಗ, str, strerror(errno));
		ಆದರೆ (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, ಮಾರ್ಗ, ಅಂಶ, strerror(errno));
		ಆದರೆ (S_ISLNK(st.st_mode) ) {
			ಆದರೆ (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, ಮಾರ್ಗ, str);
			linksize = st.st_size+1;
			ಆದರೆ (linkpath)
				ಬಿಡುಗಡೆ(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			ಆದರೆ (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			ಬಿಡುಗಡೆ(linkpath);
			linkpath = ಖಾಲಿ;
			ಅಂಶ = strtok(ಖಾಲಿ, "/");
			components--;
		}

		ಬಿಡುಗಡೆ(str);
	}
	ಬಿಡುಗಡೆ(ಮಾರ್ಗ_dupe);
	ಬಿಡುಗಡೆ(linkpath);
	ಹಿಂತಿರುಗಿ dirfd;
}

ಸ್ಥಿರ ಅಕ್ಷರ *clean_ಮಾರ್ಗ(ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಅಕ್ಷರ *ch, *ch2, *str;

	str = xmalloc(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮಾರ್ಗ) + 1);
	ch = ಮಾರ್ಗ;
	ch2 = str;
	ಆಗ (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		ಆದರೆ (!*(ch-1))
			ನಿಲ್ಲಿಸು;
		ಆಗ (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	ಆಗ ((ch = strrchr(str, '/'))) {
		ಆದರೆ (ch == str)
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (!*(ch+1))
			*ch = 0;
		ಇಲ್ಲದಿದ್ದರೆ
			ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ str;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ do_create(inode_t type, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ dirfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ flags, mode_t ಬಹುಲಕ)
{
	ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ = umask(0);
	ಪೂರ್ಣಾಂಕ fd;

	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ mode_t default_ಬಹುಲಕ[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	ಆದರೆ (!ಬಹುಲಕ)
		ಬಹುಲಕ = default_ಬಹುಲಕ[type];

	ಆಯ್ಕೆ (type) {
	ಸ್ಥಿತಿ inode_file:
		einfo("%s: creating file", ಮಾರ್ಗ);
		flags |= O_CREAT;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ inode_dir:
		einfo("%s: creating directory", ಮಾರ್ಗ);

		/* We do not recursively create parents */
		ಆದರೆ (mkdirat(dirfd, name, ಬಹುಲಕ) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			ಹಿಂತಿರುಗಿ -1;
		}

		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ inode_fifo:
		einfo("%s: creating fifo", ಮಾರ್ಗ);

		ಆದರೆ (mkfifoat(dirfd, name, ಬಹುಲಕ) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			ಹಿಂತಿರುಗಿ -1;
		}

		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ inode_unknown:
		ನಿಲ್ಲಿಸು;
	}
	umask(ಮುಖವಾಡ);

	ಆದರೆ ((fd = openat(dirfd, name, flags, ಬಹುಲಕ)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		ಹಿಂತಿರುಗಿ -1;
	}
	ಹಿಂತಿರುಗಿ fd;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ do_check(ಅಕ್ಷರ *ಮಾರ್ಗ, uid_t uid, gid_t gid, mode_t ಬಹುಲಕ, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	ಪೂರ್ಣಾಂಕ flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = basename_c(ಮಾರ್ಗ);
	ರಚನೆ stat st;
	ಪೂರ್ಣಾಂಕ dirfd, fd;

	ಆದರೆ (writable) {
		ಆದರೆ (access(ಮಾರ್ಗ, W_OK) == 0)
			ಹಿಂತಿರುಗಿ 0;
		ಆದರೆ (errno != ENOENT || type == inode_unknown)
			ಹಿಂತಿರುಗಿ -1;
	}

	ಆದರೆ (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(ಮಾರ್ಗ, symlinks);
	ಆದರೆ ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, ಮಾರ್ಗ, dirfd, name, flags, ಬಹುಲಕ)) == -1)
		ಹಿಂತಿರುಗಿ -1;

	ಆದರೆ (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", ಮಾರ್ಗ, strerror(errno));
		ಹೋಗು err;
	}

	ಆದರೆ (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", ಮಾರ್ಗ);
		ಹೋಗು err;
	}

	ಆದರೆ (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", ಮಾರ್ಗ);
		ಹೋಗು err;
	}

	ಆದರೆ (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", ಮಾರ್ಗ);
		ಹೋಗು err;
	}

	ಆದರೆ (ಬಹುಲಕ && (st.st_mode & 07777) != ಬಹುಲಕ) {
		ಆದರೆ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, ಮಾರ್ಗ);
			ಹೋಗು err;
		}

		ಆದರೆ (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, ಮಾರ್ಗ);
			ಹೋಗು err;
		}

		einfo("%s: correcting mode", ಮಾರ್ಗ);
		ಆದರೆ (fchmod(fd, ಬಹುಲಕ)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			ಹೋಗು err;
		}
	}

	ಆದರೆ (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		ಆದರೆ ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, ಮಾರ್ಗ);
			ಹೋಗು err;
		}

		ಆದರೆ (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, ಮಾರ್ಗ);
			ಹೋಗು err;
		}

		einfo("%s: correcting owner", ಮಾರ್ಗ);
		ಆದರೆ (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			ಹೋಗು err;
		}
	}

	ಆದರೆ (selinux_on)
		selinux_util_label(ಮಾರ್ಗ);
	close(fd);

	ಹಿಂತಿರುಗಿ 0;
err:
	close(fd);
	ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_owner(ರಚನೆ passwd **user, ರಚನೆ group **group, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *owner)
{
	ಅಕ್ಷರ *u = xstrdup(owner), *g = strchr(u, ':');
	ಪೂರ್ಣಾಂಕ id = 0, retval = 0;

	ಆದರೆ (g)
		*g++ = '\0';

	ಆದರೆ (user && *u) {
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		ಇಲ್ಲದಿದ್ದರೆ
			*user = getpwnam(u);
		ಆದರೆ (*user == ಖಾಲಿ)
			retval = -1;
	}

	ಆದರೆ (group && g && *g) {
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		ಇಲ್ಲದಿದ್ದರೆ
			*group = getgrnam(g);
		ಆದರೆ (*group == ಖಾಲಿ)
			retval = -1;
	}

	ಬಿಡುಗಡೆ(u);
	ಹಿಂತಿರುಗಿ retval;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t ಬಹುಲಕ = 0;
	ರಚನೆ passwd *pw = ಖಾಲಿ;
	ರಚನೆ group *gr = ಖಾಲಿ;
	inode_t type = inode_unknown;
	ಪೂರ್ಣಾಂಕ retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	ಅಕ್ಷರ *ಮಾರ್ಗ = ಖಾಲಿ;

	applet = basename_c(argv[0]);
	ಆಗ ((opt = getopt_long(argc, argv, getoptstring, longopts, (ಪೂರ್ಣಾಂಕ *) 0)) != -1) {
		ಆಯ್ಕೆ (opt) {
		ಸ್ಥಿತಿ 'D':
			trunc = true;
			/* falls through */
		ಸ್ಥಿತಿ 'd':
			type = inode_dir;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'F':
			trunc = true;
			/* falls through */
		ಸ್ಥಿತಿ 'f':
			type = inode_file;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'p':
			type = inode_fifo;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'm':
			ಆದರೆ (parse_ಬಹುಲಕ(&ಬಹುಲಕ, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'o':
			chowner = true;
			ಆದರೆ (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 's':
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ O_ಮಾರ್ಗ
			symlinks = true;
#ಆದರೆ_ಅಂತ್ಯ
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'W':
			writable = true;
			ನಿಲ್ಲಿಸು;

		case_RC_COMMON_GETOPT
		}
	}

	ಆದರೆ (optind >= argc)
		usage(EXIT_FAILURE);

	ಆದರೆ (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	ಆದರೆ (gr)
		gid = gr->gr_gid;

	ಆದರೆ (selinux_util_open() == 1)
		selinux_on = true;

	ಆಗ (optind < argc) {
		ಮಾರ್ಗ = clean_ಮಾರ್ಗ(argv[optind]);
		ಆದರೆ (do_check(ಮಾರ್ಗ, uid, gid, ಬಹುಲಕ, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		ಬಿಡುಗಡೆ(ಮಾರ್ಗ);
	}

	ಆದರೆ (selinux_on)
		selinux_util_close();

	ಹಿಂತಿರುಗಿ retval;
}
