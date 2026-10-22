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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <grp.h>
#अन्तर्भाव <pwd.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "selinux.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"

प्रकारनिर्देश गणना {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

ध्रुव वर्ण *applet = रिक्त;
ध्रुव वर्ण *extraopts ="path1 [path2] [...]";
ध्रुव वर्ण getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
ध्रुव संरचना option longopts[] = {
	{ "directory",          0, रिक्त, 'd'},
	{ "directory-truncate", 0, रिक्त, 'D'},
	{ "file",               0, रिक्त, 'f'},
	{ "file-truncate",      0, रिक्त, 'F'},
	{ "pipe",               0, रिक्त, 'p'},
	{ "mode",               1, रिक्त, 'm'},
	{ "owner",              1, रिक्त, 'o'},
	{ "symlinks",           0, रिक्त, 's'},
	{ "writable",           0, रिक्त, 'W'},
	longopts_COMMON
};
ध्रुव वर्ण * ध्रुव longopts_help[] = {
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
ध्रुव वर्ण *usagestring = रिक्त;

स्थिर पूर्णाङ्क get_dirfd(वर्ण *path, bool symlinks)
{
	वर्ण *ch, *item, *path_dupe, *str, *linkpath = रिक्त;
	पूर्णाङ्क components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	संरचना stat st;

	सचे (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	सचे (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = path;
	याव (*ch) {
		सचे (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	सचे (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	याव (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		सचे (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, strerror(errno));
		सचे (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, strerror(errno));
		सचे (S_ISLNK(st.st_mode) ) {
			सचे (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			सचे (linkpath)
				मुक्त(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			सचे (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} अञ्ञथा {
			/* now walk down the directory path */
			close(dirfd);
			dirfd = new_dirfd;
			मुक्त(linkpath);
			linkpath = रिक्त;
			item = strtok(रिक्त, "/");
			components--;
		}

		मुक्त(str);
	}
	मुक्त(path_dupe);
	मुक्त(linkpath);
	पटिनिवत्त dirfd;
}

स्थिर वर्ण *clean_path(वर्ण *path)
{
	वर्ण *ch, *ch2, *str;

	str = xmalloc(strlen(path) + 1);
	ch = path;
	ch2 = str;
	याव (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		सचे (!*(ch-1))
			भङ्ग;
		याव (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	याव ((ch = strrchr(str, '/'))) {
		सचे (ch == str)
			भङ्ग;
		सचे (!*(ch+1))
			*ch = 0;
		अञ्ञथा
			भङ्ग;
	}
	पटिनिवत्त str;
}

स्थिर पूर्णाङ्क do_create(inode_t type, ध्रुव वर्ण *path, पूर्णाङ्क dirfd, ध्रुव वर्ण *name, पूर्णाङ्क flags, mode_t mode)
{
	पूर्णाङ्क mask = umask(0);
	पूर्णाङ्क fd;

	स्थिर ध्रुव mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	सचे (!mode)
		mode = default_mode[type];

	विकल्प (type) {
	पक्ष inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		भङ्ग;
	पक्ष inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		सचे (mkdirat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			पटिनिवत्त -1;
		}

		भङ्ग;
	पक्ष inode_fifo:
		einfo("%s: creating fifo", path);

		सचे (mkfifoat(dirfd, name, mode) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			पटिनिवत्त -1;
		}

		भङ्ग;
	पक्ष inode_unknown:
		भङ्ग;
	}
	umask(mask);

	सचे ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		पटिनिवत्त -1;
	}
	पटिनिवत्त fd;
}

स्थिर पूर्णाङ्क do_check(वर्ण *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	पूर्णाङ्क flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	ध्रुव वर्ण *name = basename_c(path);
	संरचना stat st;
	पूर्णाङ्क dirfd, fd;

	सचे (writable) {
		सचे (access(path, W_OK) == 0)
			पटिनिवत्त 0;
		सचे (errno != ENOENT || type == inode_unknown)
			पटिनिवत्त -1;
	}

	सचे (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	सचे ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		पटिनिवत्त -1;

	सचे (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, strerror(errno));
		गच्छ err;
	}

	सचे (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		गच्छ err;
	}

	सचे (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		गच्छ err;
	}

	सचे (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		गच्छ err;
	}

	सचे (mode && (st.st_mode & 07777) != mode) {
		सचे ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			गच्छ err;
		}

		सचे (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			गच्छ err;
		}

		einfo("%s: correcting mode", path);
		सचे (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			गच्छ err;
		}
	}

	सचे (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		सचे ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			गच्छ err;
		}

		सचे (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			गच्छ err;
		}

		einfo("%s: correcting owner", path);
		सचे (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			गच्छ err;
		}
	}

	सचे (selinux_on)
		selinux_util_label(path);
	close(fd);

	पटिनिवत्त 0;
err:
	close(fd);
	पटिनिवत्त -1;
}

स्थिर पूर्णाङ्क parse_owner(संरचना passwd **user, संरचना group **group, ध्रुव वर्ण *owner)
{
	वर्ण *u = xstrdup(owner), *g = strchr(u, ':');
	पूर्णाङ्क id = 0, retval = 0;

	सचे (g)
		*g++ = '\0';

	सचे (user && *u) {
		सचे (sscanf(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		अञ्ञथा
			*user = getpwnam(u);
		सचे (*user == रिक्त)
			retval = -1;
	}

	सचे (group && g && *g) {
		सचे (sscanf(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		अञ्ञथा
			*group = getgrnam(g);
		सचे (*group == रिक्त)
			retval = -1;
	}

	मुक्त(u);
	पटिनिवत्त retval;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	संरचना passwd *pw = रिक्त;
	संरचना group *gr = रिक्त;
	inode_t type = inode_unknown;
	पूर्णाङ्क retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	वर्ण *path = रिक्त;

	applet = basename_c(argv[0]);
	याव ((opt = getopt_long(argc, argv, getoptstring, longopts, (पूर्णाङ्क *) 0)) != -1) {
		विकल्प (opt) {
		पक्ष 'D':
			trunc = true;
			/* falls through */
		पक्ष 'd':
			type = inode_dir;
			भङ्ग;
		पक्ष 'F':
			trunc = true;
			/* falls through */
		पक्ष 'f':
			type = inode_file;
			भङ्ग;
		पक्ष 'p':
			type = inode_fifo;
			भङ्ग;
		पक्ष 'm':
			सचे (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			भङ्ग;
		पक्ष 'o':
			chowner = true;
			सचे (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			भङ्ग;
		पक्ष 's':
#ifndef O_PATH
			symlinks = true;
#endif
			भङ्ग;
		पक्ष 'W':
			writable = true;
			भङ्ग;

		case_RC_COMMON_GETOPT
		}
	}

	सचे (optind >= argc)
		usage(EXIT_FAILURE);

	सचे (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	सचे (gr)
		gid = gr->gr_gid;

	सचे (selinux_util_open() == 1)
		selinux_on = true;

	याव (optind < argc) {
		path = clean_path(argv[optind]);
		सचे (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		मुक्त(path);
	}

	सचे (selinux_on)
		selinux_util_close();

	पटिनिवत्त retval;
}
