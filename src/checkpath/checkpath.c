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

#समावेश <errno.h>
#समावेश <fcntl.h>
#समावेश <getopt.h>
#समावेश <grp.h>
#समावेश <pwd.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <sys/stat.h>
#समावेश <sys/types.h>
#समावेश <unistd.h>

#समावेश "einfo.h"
#समावेश "misc.h"
#समावेश "selinux.h"
#समावेश "_usage.h"
#समावेश "helpers.h"

प्रकार क्रमागत {
	inode_unknown = 0,
	inode_file = 1,
	inode_dir = 2,
	inode_fifo = 3,
} inode_t;

स्थिर अक्षर *applet = शून्य;
स्थिर अक्षर *extraopts ="path1 [path2] [...]";
स्थिर अक्षर getoptstring[] = "dDfFpm:o:sW" getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "directory",          0, शून्य, 'd'},
	{ "directory-truncate", 0, शून्य, 'D'},
	{ "file",               0, शून्य, 'f'},
	{ "file-truncate",      0, शून्य, 'F'},
	{ "pipe",               0, शून्य, 'p'},
	{ "mode",               1, शून्य, 'm'},
	{ "owner",              1, शून्य, 'o'},
	{ "symlinks",           0, शून्य, 's'},
	{ "writable",           0, शून्य, 'W'},
	longopts_COMMON
};
स्थिर अक्षर * स्थिर longopts_help[] = {
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
स्थिर अक्षर *usagestring = शून्य;

अटल पूर्णांक get_dirfd(अक्षर *path, bool symlinks)
{
	अक्षर *ch, *item, *path_dupe, *str, *linkpath = शून्य;
	पूर्णांक components = 0, dirfd, flags = 0, new_dirfd;
	ssize_t linksize;
	काष्ठा stat st;

	अगर (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(AT_FDCWD, "/", O_RDONLY);
	अगर (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s", applet, म_त्रुटि(त्रुटि_सं));
	ch = path;
	जबतक (*ch) {
		अगर (*ch == '/')
			components++;
		ch++;
	}
	path_dupe = xstrdup(path);
	item = म_मोहर(path_dupe, "/");
#अगर_घोषित O_PATH
	flags |= O_PATH;
#पूर्ण_अगर
	अगर (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	जबतक (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		अगर (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str, म_त्रुटि(त्रुटि_सं));
		अगर (ख_स्थिति(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item, म_त्रुटि(त्रुटि_सं));
		अगर (S_ISLNK(st.st_mode) ) {
			अगर (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root", applet, path, str);
			linksize = st.st_size+1;
			अगर (linkpath)
				मुक्त(linkpath);
			linkpath = xmalloc(linksize);
			स_रखो(linkpath, 0, linksize);
			अगर (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
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
			linkpath = शून्य;
			item = म_मोहर(शून्य, "/");
			components--;
		}

		मुक्त(str);
	}
	मुक्त(path_dupe);
	मुक्त(linkpath);
	वापस dirfd;
}

अटल अक्षर *clean_path(अक्षर *path)
{
	अक्षर *ch, *ch2, *str;

	str = xmalloc(म_माप(path) + 1);
	ch = path;
	ch2 = str;
	जबतक (true) {
		*ch2 = *ch;
		ch++;
		ch2++;
		अगर (!*(ch-1))
			अवरोध;
		जबतक (*(ch - 1) == '/' && *ch == '/')
			ch++;
	}
	/* get rid of trailing / characters */
	जबतक ((ch = म_खोजप(str, '/'))) {
		अगर (ch == str)
			अवरोध;
		अगर (!*(ch+1))
			*ch = 0;
		अन्यथा
			अवरोध;
	}
	वापस str;
}

अटल पूर्णांक do_create(inode_t type, स्थिर अक्षर *path, पूर्णांक dirfd, स्थिर अक्षर *name, पूर्णांक flags, mode_t mode)
{
	पूर्णांक mask = umask(0);
	पूर्णांक fd;

	अटल स्थिर mode_t default_mode[] = {
		[inode_file] = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, /* 664 */
		[inode_dir] = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, /* 775 */
		[inode_fifo] = S_IRUSR | S_IWUSR, /* 600 */
		[inode_unknown] = 0,
	};

	अगर (!mode)
		mode = default_mode[type];

	चयन (type) {
	हाल inode_file:
		einfo("%s: creating file", path);
		flags |= O_CREAT;
		अवरोध;
	हाल inode_dir:
		einfo("%s: creating directory", path);

		/* We do not recursively create parents */
		अगर (mkdirat(dirfd, name, mode) == -1 && त्रुटि_सं != EEXIST) {
			eerror("%s: mkdirat: %s", applet, म_त्रुटि (त्रुटि_सं));
			वापस -1;
		}

		अवरोध;
	हाल inode_fifo:
		einfo("%s: creating fifo", path);

		अगर (mkfifoat(dirfd, name, mode) == -1 && त्रुटि_सं != EEXIST) {
			eerror("%s: mkfifo: %s", applet, म_त्रुटि (त्रुटि_सं));
			वापस -1;
		}

		अवरोध;
	हाल inode_unknown:
		अवरोध;
	}
	umask(mask);

	अगर ((fd = openat(dirfd, name, flags, mode)) == -1) {
		eerror("%s: open: %s", applet, म_त्रुटि(त्रुटि_सं));
		वापस -1;
	}
	वापस fd;
}

अटल पूर्णांक do_check(अक्षर *path, uid_t uid, gid_t gid, mode_t mode, inode_t type,
		bool trunc, bool chowner, bool writable, bool symlinks, bool selinux_on)
{
	पूर्णांक flags = O_NDELAY | O_NOCTTY | O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
	स्थिर अक्षर *name = basename_c(path);
	काष्ठा stat st;
	पूर्णांक dirfd, fd;

	अगर (writable) {
		अगर (access(path, W_OK) == 0)
			वापस 0;
		अगर (त्रुटि_सं != ENOENT || type == inode_unknown)
			वापस -1;
	}

	अगर (trunc && type == inode_file)
		flags |= O_TRUNC;

	dirfd = get_dirfd(path, symlinks);
	अगर ((fd = openat(dirfd, name, flags)) == -1 &&
			(fd = do_create(type, path, dirfd, name, flags, mode)) == -1)
		वापस -1;

	अगर (ख_स्थिति(fd, &st) == -1) {
		eerror("fstat: %s: %s", path, म_त्रुटि(त्रुटि_सं));
		जाओ err;
	}

	अगर (type != inode_dir && S_ISDIR(st.st_mode)) {
		eerror("%s: is a directory", path);
		जाओ err;
	}

	अगर (type != inode_file && S_ISREG(st.st_mode)) {
		eerror("%s: is a file", path);
		जाओ err;
	}

	अगर (type != inode_fifo && S_ISFIFO(st.st_mode)) {
		eerror("%s: is a fifo", path);
		जाओ err;
	}

	अगर (mode && (st.st_mode & 07777) != mode) {
		अगर ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chmod: Too many hard links to %s", applet, path);
			जाओ err;
		}

		अगर (S_ISLNK(st.st_mode)) {
			eerror("%s: chmod: %s is a symbolic link", applet, path);
			जाओ err;
		}

		einfo("%s: correcting mode", path);
		अगर (fchmod(fd, mode)) {
			eerror("%s: chmod: %s", applet, म_त्रुटि(त्रुटि_सं));
			जाओ err;
		}
	}

	अगर (chowner && (st.st_uid != uid || st.st_gid != gid)) {
		अगर ((type != inode_dir) && (st.st_nlink > 1)) {
			eerror("%s: chown: Too many hardlinks to %s", applet, path);
			जाओ err;
		}

		अगर (S_ISLNK(st.st_mode)) {
			eerror("%s: chown: %s is a symbolic link", applet, path);
			जाओ err;
		}

		einfo("%s: correcting owner", path);
		अगर (fchown(fd, uid, gid)) {
			eerror("%s: chown: %s", applet, म_त्रुटि(त्रुटि_सं));
			जाओ err;
		}
	}

	अगर (selinux_on)
		selinux_util_label(path);
	बंद(fd);

	वापस 0;
err:
	बंद(fd);
	वापस -1;
}

अटल पूर्णांक parse_owner(काष्ठा passwd **user, काष्ठा group **group, स्थिर अक्षर *owner)
{
	अक्षर *u = xstrdup(owner), *g = म_अक्षर(u, ':');
	पूर्णांक id = 0, retval = 0;

	अगर (g)
		*g++ = '\0';

	अगर (user && *u) {
		अगर (माला_पूछो(u, "%d", &id) == 1)
			*user = getpwuid((uid_t) id);
		अन्यथा
			*user = getpwnam(u);
		अगर (*user == शून्य)
			retval = -1;
	}

	अगर (group && g && *g) {
		अगर (माला_पूछो(g, "%d", &id) == 1)
			*group = getgrgid((gid_t) id);
		अन्यथा
			*group = getgrnam(g);
		अगर (*group == शून्य)
			retval = -1;
	}

	मुक्त(u);
	वापस retval;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	काष्ठा passwd *pw = शून्य;
	काष्ठा group *gr = शून्य;
	inode_t type = inode_unknown;
	पूर्णांक retval = निकास_सफल;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	अक्षर *path = शून्य;

	applet = basename_c(argv[0]);
	जबतक ((opt = getopt_long(argc, argv, getoptstring, longopts, (पूर्णांक *) 0)) != -1) {
		चयन (opt) {
		हाल 'D':
			trunc = true;
			/* falls through */
		हाल 'd':
			type = inode_dir;
			अवरोध;
		हाल 'F':
			trunc = true;
			/* falls through */
		हाल 'f':
			type = inode_file;
			अवरोध;
		हाल 'p':
			type = inode_fifo;
			अवरोध;
		हाल 'm':
			अगर (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode '%s'", applet, optarg);
			अवरोध;
		हाल 'o':
			chowner = true;
			अगर (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner '%s' not found", applet, optarg);
			अवरोध;
		हाल 's':
#अगर_अघोषित O_PATH
			symlinks = true;
#पूर्ण_अगर
			अवरोध;
		हाल 'W':
			writable = true;
			अवरोध;

		case_RC_COMMON_GETOPT
		}
	}

	अगर (optind >= argc)
		usage(निकास_त्रुटि);

	अगर (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	अगर (gr)
		gid = gr->gr_gid;

	अगर (selinux_util_open() == 1)
		selinux_on = true;

	जबतक (optind < argc) {
		path = clean_path(argv[optind]);
		अगर (do_check(path, uid, gid, mode, type, trunc, chowner, writable, symlinks, selinux_on))
			retval = निकास_त्रुटि;
		optind++;
		मुक्त(path);
	}

	अगर (selinux_on)
		selinux_util_close();

	वापस retval;
}
