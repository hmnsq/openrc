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

प्रकारव्याख्या गणना {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

स्थिरांक अक्षर *applet = रिक्त;
स्थिरांक अक्षर *extraopts ="path1 [path2] [...]";
स्थिरांक अक्षर getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
स्थिरांक संरचना option longopts[] = {
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
स्थिरांक अक्षर * स्थिरांक longopts_help[] = {
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
स्थिरांक अक्षर *usagestring = रिक्त;

स्थिर पूर्णांक get_dirfd(अक्षर *path, bool symlinks)
{
	अक्षर *ch, *item, *path_dupe, *str, *linkpath = रिक्त;
	पूर्णांक components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	संरचना stat st;

	जर (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	जर (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, म_त्रुटि(त्रुटि_सं));
	ch = path;
	पर्यंत (*ch) {
		जर (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = म_मोहर(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	जर (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	पर्यंत (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		जर (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, म_त्रुटि(त्रुटि_सं));
		जर (ख_स्थिति(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, म_त्रुटि(त्रुटि_सं));
		जर (S_ISLNK(st.st_mode) ) {
			जर (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			जर (linkpath)
				मुक्त(linkpath);
			linkpath = xmalloc(linksize);
			स_रखो(linkpath, 0, linksize);
			जर (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
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
	परत dirfd;
}

स्थिर अक्षर *clean_path(अक्षर *path)
{
	अक्षर *ch, *ch2, *str;

	str = xmalloc(म_माप(path) + 1);
	ch = path;
	ch2 = str;
	पर्यंत (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		जर (!*(ch-1))
			थांबा;
		पर्यंत (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	पर्यंत ((ch = म_खोजप(str, '/'))) {
		जर (ch == str)
			थांबा;
		जर (!*(ch+1))
			*ch = 0;
		अन्यथा
			थांबा;
	}
	परत str;
}

स्थिर पूर्णांक do_create(inode_t type, स्थिरांक अक्षर *path, पूर्णांक dirfd, स्थिरांक अक्षर *name, पूर्णांक flags, mode_t mode)
{
	पूर्णांक mask = umask(0);
	पूर्णांक fd;

	स्थिर स्थिरांक mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	जर (!mode)
		mode = default_mode[type];

	निवड (type) {
	स्थिती inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		थांबा;
	स्थिती inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		जर (mkdirat(dirfd, name, mode) == -1 && त्रुटि_सं != EEXIST) {
			eerror("%s: mkdirat: %s", applet, म_त्रुटि (त्रुटि_सं));
			परत -1;
		}

		थांबा;
	स्थिती inode_fifo:
		einfo("%s: creating fifo", path);

		जर (mkfifoat(dirfd, name, mode) == -1 && त्रुटि_सं != EEXIST) {
			eerror("%s: mkfifo: %s", applet, म_त्रुटि (त्रुटि_सं));
			परत -1;
		}

		थांबा;
	स्थिती inode_unknown:
		थांबा;
	}
	umask(mask);

	जर ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, म_त्रुटि(त्रुटि_सं));
		परत -1;
	}
	परत fd;
}

स्थिर पूर्णांक do_check(अक्षर *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	पूर्णांक flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	स्थिरांक अक्षर *name = basename_c(path);
	संरचना stat st;
	पूर्णांक dirfd, fd;

	जर (writable) {
		जर (access(path, W_OK) == 0)
			परत 0;
		जर (त्रुटि_सं != ENOENT || type == inode_unknown)
			परत -1;
	}

	जर (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	जर ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		परत -1;

	जर (ख_स्थिति(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, म_त्रुटि(त्रुटि_सं));
		जा err;
	}

	जर (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		जा err;
	}

	जर (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		जा err;
	}

	जर (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		जा err;
	}

	जर (mode && (st.st_mode & 07777) != mode) {
		जर ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			जा err;
		}

		जर (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			जा err;
		}

		einfo("%s: correcting mode", path);
		जर (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, म_त्रुटि(त्रुटि_सं));
			जा err;
		}
	}

	जर (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		जर ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			जा err;
		}

		जर (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			जा err;
		}

		einfo("%s: correcting owner", path);
		जर (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, म_त्रुटि(त्रुटि_सं));
			जा err;
		}
	}

	जर (selinux_on)
		selinux_util_label(path);
	बंद(fd);

	परत 0;
err:
	बंद(fd);
	परत -1;
}

स्थिर पूर्णांक parse_owner(संरचना passwd **user, संरचना group **group, स्थिरांक अक्षर *owner)
{
	अक्षर *u = xstrdup(owner), *g = म_अक्षर(u, ':');
	पूर्णांक id = 0, retval = 0;

	जर (g)
		*g++ = '\0';

	जर (user && *u) {
		जर (माला_पूछो(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		अन्यथा
			*user = getpwnam(u);
		जर (*user == रिक्त)
			retval = -1;
	}

	जर (group && g && *g) {
		जर (माला_पूछो(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		अन्यथा
			*group = getgrnam(g);
		जर (*group == रिक्त)
			retval = -1;
	}

	मुक्त(u);
	परत retval;
}

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	संरचना passwd *pw = रिक्त;
	संरचना group *gr = रिक्त;
	inode_t type = inode_unknown;
	पूर्णांक retval = निकास_सफल;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	अक्षर *path = रिक्त;

	applet = basename_c(argv[0]);
	पर्यंत ((opt = getopt_long(argc, argv, getoptstring, longopts, (पूर्णांक *) 0)) != -1) {
		निवड (opt) {
		स्थिती 'D':
			trunc = true;
			/* falls through */
		स्थिती 'd':
			type = inode_dir;
			थांबा;
		स्थिती 'F':
			trunc = true;
			/* falls through */
		स्थिती 'f':
			type = inode_file;
			थांबा;
		स्थिती 'p':
			type = inode_fifo;
			थांबा;
		स्थिती 'm':
			जर (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			थांबा;
		स्थिती 'o':
			chowner = true;
			जर (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			थांबा;
		स्थिती 's':
#ifndef O_PATH
			symlinks = true;
#endif
			थांबा;
		स्थिती 'W':
			writable = true;
			थांबा;

		case_RC_COMMON_GETOPT
		}
	}

	जर (optind >= argc)
		usage(निकास_त्रुटि);

	जर (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	जर (gr)
		gid = gr->gr_gid;

	जर (selinux_util_open() == 1)
		selinux_on = true;

	पर्यंत (optind < argc) {
		path = clean_path(argv[optind]);
		जर (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = निकास_त्रुटि;
		optind++;
		मुक्त(path);
	}

	जर (selinux_on)
		selinux_util_close();

	परत retval;
}
