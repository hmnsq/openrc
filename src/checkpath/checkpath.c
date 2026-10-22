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

अचर वर्ण *applet = रिक्त;
अचर वर्ण *extraopts ="path1 [path2] [...]";
अचर वर्ण getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
अचर संरचना option longopts[] = {
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
अचर वर्ण * अचर longopts_help[] = {
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
अचर वर्ण *usagestring = रिक्त;

स्थिर पूर्णाङ्क get_dirfd(वर्ण *path, bool symlinks)
{
	वर्ण *ch, *item, *path_dupe, *str, *linkpath = रिक्त;
	पूर्णाङ्क components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	संरचना stat st;

	यदि (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	यदि (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, म_त्रुटि(त्रुटि_सं));
	ch = path;
	जबसम्म (*ch) {
		यदि (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = म_मोहर(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	यदि (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	जबसम्म (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		यदि (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, म_त्रुटि(त्रुटि_सं));
		यदि (ख_स्थिति(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, म_त्रुटि(त्रुटि_सं));
		यदि (S_ISLNK(st.st_mode) ) {
			यदि (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			यदि (linkpath)
				मुक्त(linkpath);
			linkpath = xmalloc(linksize);
			स_रखो(linkpath, 0, linksize);
			यदि (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			बंद(new_dirfd);
		} अन्यथा {
			/* now walk down the directory path */
			बंद(dirfd);
			dirfd = new_dirfd;
			मुक्त(linkpath);
			linkpath = रिक्त;
			item = म_मोहर(रिक्त, "/");
			components--;
		}

		मुक्त(str);
	}
	मुक्त(path_dupe);
	मुक्त(linkpath);
	फर्काऊ dirfd;
}

स्थिर वर्ण *clean_path(वर्ण *path)
{
	वर्ण *ch, *ch2, *str;

	str = xmalloc(म_माप(path) + 1);
	ch = path;
	ch2 = str;
	जबसम्म (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		यदि (!*(ch-1))
			रोक;
		जबसम्म (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	जबसम्म ((ch = म_खोजप(str, '/'))) {
		यदि (ch == str)
			रोक;
		यदि (!*(ch+1))
			*ch = 0;
		अन्यथा
			रोक;
	}
	फर्काऊ str;
}

स्थिर पूर्णाङ्क do_create(inode_t type, अचर वर्ण *path, पूर्णाङ्क dirfd, अचर वर्ण *name, पूर्णाङ्क flags, mode_t mode)
{
	पूर्णाङ्क mask = umask(0);
	पूर्णाङ्क fd;

	स्थिर अचर mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	यदि (!mode)
		mode = default_mode[type];

	विकल्प (type) {
	पक्ष inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		रोक;
	पक्ष inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		यदि (mkdirat(dirfd, name, mode) == -1 && त्रुटि_सं != EEXIST) {
			eerror("%s: mkdirat: %s", applet, म_त्रुटि (त्रुटि_सं));
			फर्काऊ -1;
		}

		रोक;
	पक्ष inode_fifo:
		einfo("%s: creating fifo", path);

		यदि (mkfifoat(dirfd, name, mode) == -1 && त्रुटि_सं != EEXIST) {
			eerror("%s: mkfifo: %s", applet, म_त्रुटि (त्रुटि_सं));
			फर्काऊ -1;
		}

		रोक;
	पक्ष inode_unknown:
		रोक;
	}
	umask(mask);

	यदि ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, म_त्रुटि(त्रुटि_सं));
		फर्काऊ -1;
	}
	फर्काऊ fd;
}

स्थिर पूर्णाङ्क do_check(वर्ण *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	पूर्णाङ्क flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	अचर वर्ण *name = basename_c(path);
	संरचना stat st;
	पूर्णाङ्क dirfd, fd;

	यदि (writable) {
		यदि (access(path, W_OK) == 0)
			फर्काऊ 0;
		यदि (त्रुटि_सं != ENOENT || type == inode_unknown)
			फर्काऊ -1;
	}

	यदि (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	यदि ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		फर्काऊ -1;

	यदि (ख_स्थिति(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, म_त्रुटि(त्रुटि_सं));
		गच्छ err;
	}

	यदि (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		गच्छ err;
	}

	यदि (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		गच्छ err;
	}

	यदि (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		गच्छ err;
	}

	यदि (mode && (st.st_mode & 07777) != mode) {
		यदि ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			गच्छ err;
		}

		यदि (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			गच्छ err;
		}

		einfo("%s: correcting mode", path);
		यदि (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, म_त्रुटि(त्रुटि_सं));
			गच्छ err;
		}
	}

	यदि (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		यदि ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			गच्छ err;
		}

		यदि (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			गच्छ err;
		}

		einfo("%s: correcting owner", path);
		यदि (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, म_त्रुटि(त्रुटि_सं));
			गच्छ err;
		}
	}

	यदि (selinux_on)
		selinux_util_label(path);
	बंद(fd);

	फर्काऊ 0;
err:
	बंद(fd);
	फर्काऊ -1;
}

स्थिर पूर्णाङ्क parse_owner(संरचना passwd **user, संरचना group **group, अचर वर्ण *owner)
{
	वर्ण *u = xstrdup(owner), *g = म_अक्षर(u, ':');
	पूर्णाङ्क id = 0, retval = 0;

	यदि (g)
		*g++ = '\0';

	यदि (user && *u) {
		यदि (माला_पूछो(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		अन्यथा
			*user = getpwnam(u);
		यदि (*user == रिक्त)
			retval = -1;
	}

	यदि (group && g && *g) {
		यदि (माला_पूछो(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		अन्यथा
			*group = getgrnam(g);
		यदि (*group == रिक्त)
			retval = -1;
	}

	मुक्त(u);
	फर्काऊ retval;
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
	पूर्णाङ्क retval = निकास_सफल;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	वर्ण *path = रिक्त;

	applet = basename_c(argv[0]);
	जबसम्म ((opt = getopt_long(argc, argv, getoptstring, longopts, (पूर्णाङ्क *) 0)) != -1) {
		विकल्प (opt) {
		पक्ष 'D':
			trunc = true;
			/* falls through */
		पक्ष 'd':
			type = inode_dir;
			रोक;
		पक्ष 'F':
			trunc = true;
			/* falls through */
		पक्ष 'f':
			type = inode_file;
			रोक;
		पक्ष 'p':
			type = inode_fifo;
			रोक;
		पक्ष 'm':
			यदि (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			रोक;
		पक्ष 'o':
			chowner = true;
			यदि (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			रोक;
		पक्ष 's':
#ifndef O_PATH
			symlinks = true;
#endif
			रोक;
		पक्ष 'W':
			writable = true;
			रोक;

		case_RC_COMMON_GETOPT
		}
	}

	यदि (optind >= argc)
		usage(निकास_त्रुटि);

	यदि (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	यदि (gr)
		gid = gr->gr_gid;

	यदि (selinux_util_open() == 1)
		selinux_on = true;

	जबसम्म (optind < argc) {
		path = clean_path(argv[optind]);
		यदि (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = निकास_त्रुटि;
		optind++;
		मुक्त(path);
	}

	यदि (selinux_on)
		selinux_util_close();

	फर्काऊ retval;
}
