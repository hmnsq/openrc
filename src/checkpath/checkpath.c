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

स्थिर पूर्णाङ्क get_dirfd(वर्ण *मार्गः, bool symlinks)
{
	वर्ण *ch, *item, *मार्गः_dupe, *str, *linkpath = रिक्त;
	पूर्णाङ्क components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	संरचना stat st;

	यदि (!मार्गः || *मार्गः != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	यदि (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, strerror(errno));
	ch = मार्गः;
	यावत् (*ch) {
		यदि (*ch == '/')
			components++;
		ch++;
	}
	मार्गः_dupe = xstrdup(मार्गः);
	item = strtok(मार्गः_dupe, "/");
#यदि_परिभाषितम् O_मार्गः
	flags |= O_मार्गः;
#यदि_अन्तः
	यदि (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	यावत् (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		यदि (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, मार्गः, str, strerror(errno));
		यदि (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, मार्गः, item, strerror(errno));
		यदि (S_ISLNK(st.st_mode) ) {
			यदि (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, मार्गः, str);
			linksize = st.st_size+1;
			यदि (linkpath)
				मुक्त(linkpath);
			linkpath = xmalloc(linksize);
			स्मृतिं_पूरय(linkpath, 0, linksize);
			यदि (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			/*
			 * now follow the symlink.
			 */
			close(new_dirfd);
		} अन्यथा {
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
	मुक्त(मार्गः_dupe);
	मुक्त(linkpath);
	प्रत्यागम dirfd;
}

स्थिर वर्ण *clean_मार्गः(वर्ण *मार्गः)
{
	वर्ण *ch, *ch2, *str;

	str = xmalloc(वाक्यदैर्घ्यम्(मार्गः) + 1);
	ch = मार्गः;
	ch2 = str;
	यावत् (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		यदि (!*(ch-1))
			भङ्ग;
		यावत् (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	यावत् ((ch = strrchr(str, '/'))) {
		यदि (ch == str)
			भङ्ग;
		यदि (!*(ch+1))
			*ch = 0;
		अन्यथा
			भङ्ग;
	}
	प्रत्यागम str;
}

स्थिर पूर्णाङ्क do_create(inode_t type, ध्रुव वर्ण *मार्गः, पूर्णाङ्क dirfd, ध्रुव वर्ण *name, पूर्णाङ्क flags, mode_t बहुलकम्)
{
	पूर्णाङ्क आवरणम् = umask(0);
	पूर्णाङ्क fd;

	स्थिर ध्रुव mode_t default_बहुलकम्[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	यदि (!बहुलकम्)
		बहुलकम् = default_बहुलकम्[type];

	विकल्प (type) {
	पक्ष inode_file:
		einfo("%s: creating file", मार्गः);
		flags |= O_CREAT;
		भङ्ग;
	पक्ष inode_dir:
		einfo("%s: creating directory", मार्गः);

		/* We do not recursively create parents */
		यदि (mkdirat(dirfd, name, बहुलकम्) == -1 && errno != EEXIST) {
			eerror("%s: mkdirat: %s", applet, strerror (errno));
			प्रत्यागम -1;
		}

		भङ्ग;
	पक्ष inode_fifo:
		einfo("%s: creating fifo", मार्गः);

		यदि (mkfifoat(dirfd, name, बहुलकम्) == -1 && errno != EEXIST) {
			eerror("%s: mkfifo: %s", applet, strerror (errno));
			प्रत्यागम -1;
		}

		भङ्ग;
	पक्ष inode_unknown:
		भङ्ग;
	}
	umask(आवरणम्);

	यदि ((fd = openat(dirfd, name, flags, बहुलकम्)) == -1) {
		eerror("%s: open: %s", applet, strerror(errno));
		प्रत्यागम -1;
	}
	प्रत्यागम fd;
}

स्थिर पूर्णाङ्क do_check(वर्ण *मार्गः, uid_t uid, gid_t gid, mode_t बहुलकम्, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	पूर्णाङ्क flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	ध्रुव वर्ण *name = basename_c(मार्गः);
	संरचना stat st;
	पूर्णाङ्क dirfd, fd;

	यदि (writable) {
		यदि (access(मार्गः, W_OK) == 0)
			प्रत्यागम 0;
		यदि (errno != ENOENT || type == inode_unknown)
			प्रत्यागम -1;
	}

	यदि (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(मार्गः, symlinks);
	यदि ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, मार्गः, dirfd, name, flags, बहुलकम्)) == -1)
		प्रत्यागम -1;

	यदि (fstat(fd, &st) == -1) {
		eerror("fstat: %s: %s", मार्गः, strerror(errno));
		गच्छ err;
	}

	यदि (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", मार्गः);
		गच्छ err;
	}

	यदि (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", मार्गः);
		गच्छ err;
	}

	यदि (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", मार्गः);
		गच्छ err;
	}

	यदि (बहुलकम् && (st.st_mode & 07777) != बहुलकम्) {
		यदि ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, मार्गः);
			गच्छ err;
		}

		यदि (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, मार्गः);
			गच्छ err;
		}

		einfo("%s: correcting mode", मार्गः);
		यदि (fchmod(fd, बहुलकम्)) {
			eerror("%s: chmod: %s", applet, strerror(errno));
			गच्छ err;
		}
	}

	यदि (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		यदि ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, मार्गः);
			गच्छ err;
		}

		यदि (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, मार्गः);
			गच्छ err;
		}

		einfo("%s: correcting owner", मार्गः);
		यदि (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, strerror(errno));
			गच्छ err;
		}
	}

	यदि (selinux_on)
		selinux_util_label(मार्गः);
	close(fd);

	प्रत्यागम 0;
err:
	close(fd);
	प्रत्यागम -1;
}

स्थिर पूर्णाङ्क parse_owner(संरचना passwd **user, संरचना group **group, ध्रुव वर्ण *owner)
{
	वर्ण *u = xstrdup(owner), *g = strchr(u, ':');
	पूर्णाङ्क id = 0, retval = 0;

	यदि (g)
		*g++ = '\0';

	यदि (user && *u) {
		यदि (वाक्यात्_पठ(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		अन्यथा
			*user = getpwnam(u);
		यदि (*user == रिक्त)
			retval = -1;
	}

	यदि (group && g && *g) {
		यदि (वाक्यात्_पठ(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		अन्यथा
			*group = getgrnam(g);
		यदि (*group == रिक्त)
			retval = -1;
	}

	मुक्त(u);
	प्रत्यागम retval;
}

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t बहुलकम् = 0;
	संरचना passwd *pw = रिक्त;
	संरचना group *gr = रिक्त;
	inode_t type = inode_unknown;
	पूर्णाङ्क retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	वर्ण *मार्गः = रिक्त;

	applet = basename_c(argv[0]);
	यावत् ((opt = getopt_long(argc, argv, getoptstring, longopts, (पूर्णाङ्क *) 0)) != -1) {
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
			यदि (parse_बहुलकम्(&बहुलकम्, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			भङ्ग;
		पक्ष 'o':
			chowner = true;
			यदि (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			भङ्ग;
		पक्ष 's':
#यदि_अपरिभाषितम् O_मार्गः
			symlinks = true;
#यदि_अन्तः
			भङ्ग;
		पक्ष 'W':
			writable = true;
			भङ्ग;

		case_RC_COMMON_GETOPT
		}
	}

	यदि (optind >= argc)
		usage(EXIT_FAILURE);

	यदि (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	यदि (gr)
		gid = gr->gr_gid;

	यदि (selinux_util_open() == 1)
		selinux_on = true;

	यावत् (optind < argc) {
		मार्गः = clean_मार्गः(argv[optind]);
		यदि (do_check(मार्गः, uid, gid, बहुलकम्, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
		मुक्त(मार्गः);
	}

	यदि (selinux_on)
		selinux_util_close();

	प्रत्यागम retval;
}
