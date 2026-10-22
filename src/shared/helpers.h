/*
 * helpers.h
 * This is private to us and not for user consumption
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

#अगर_अघोषित __HELPERS_H__
#घोषणा __HELPERS_H__

#समावेश <stdarg.h>
#समावेश <stdbool.h>
#समावेश <stddef.h>
#समावेश <stdint.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <sys/stat.h>
#समावेश <dirent.h>
#समावेश <unistd.h>
#समावेश <fcntl.h>
#समावेश <errno.h>

#घोषणा ERRX करो { ख_लिखो (मानक_त्रुटि, "out of memory\n"); निकास (1); } जबतक (0)

#घोषणा UNCONST(a)		((व्योम *)(uintptr_t)(स्थिर व्योम *)(a))

#घोषणा RC_UNUSED __attribute__((__unused__))
#घोषणा RC_NORETURN __attribute__((__noreturn__))
#घोषणा RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#घोषणा ARRAY_आकार(a) (माप(a) / माप((a)[0]))

#अगर_अघोषित HAVE_STRLCPY
#    define strlcpy(dst, src, आकार) snprintf(dst, आकार, "%s", src)
#पूर्ण_अगर

#अगर_अघोषित timespecsub
#घोषणा	timespecsub(tsp, usp, vsp)					      \
	करो {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		अगर ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} जबतक (/* CONSTCOND */ 0)
#पूर्ण_अगर

RC_UNUSED अटल व्योम *xmalloc (माप_प्रकार आकार)
{
	व्योम *मूल्य = दो_स्मृति(आकार);

	अगर (मूल्य)
		वापस (मूल्य);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED अटल व्योम *xrealloc(व्योम *ptr, माप_प्रकार आकार)
{
	व्योम *मूल्य = पुनः_स्मृति(ptr, आकार);

	अगर (मूल्य)
		वापस (मूल्य);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED अटल अक्षर *xstrdup(स्थिर अक्षर *str)
{
	अक्षर *मूल्य;

	अगर (!str)
		वापस (शून्य);

	मूल्य = strdup(str);

	अगर (मूल्य)
		वापस (मूल्य);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED अटल खाता *xopen_memstream(अक्षर **ptr, माप_प्रकार *sizeloc)
{
	खाता *f = open_memstream(ptr, sizeloc);
	अगर (f)
		वापस f;
	ERRX;
}

RC_UNUSED अटल व्योम xclose_memstream(खाता *f)
{
	ख_साफ(f);
	अगर (ख_त्रुटि(f) || ख_बंद(f) != 0)
		ERRX;
}

#अघोषित ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_UNUSED अटल स्थिर अक्षर *basename_c(स्थिर अक्षर *path)
{
	स्थिर अक्षर *slash = म_खोजप(path, '/');

	अगर (slash)
		वापस (++slash);
	वापस (path);
}

RC_UNUSED अटल खाता *do_fopenat(पूर्णांक dirfd, स्थिर अक्षर *pathname, पूर्णांक mode)
{
	पूर्णांक fd = openat(dirfd, pathname, mode, 0666);
	स्थिर अक्षर *fmode;
	खाता *fp;

	अगर (fd == -1)
		वापस शून्य;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	अगर ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	अन्यथा अगर ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	अन्यथा
		fmode = "r";

	अगर (!(fp = fdopen(fd, fmode)))
		बंद(fd);
	वापस fp;
}

RC_UNUSED अटल सूची *do_opendirat(पूर्णांक dirfd, स्थिर अक्षर *pathname)
{
	पूर्णांक fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	सूची *dp;

	अगर (fd == -1)
		वापस शून्य;

	अगर (!(dp = fdopendir(fd)))
		बंद(fd);

	वापस dp;
}

RC_UNUSED अटल सूची *do_dopendir(पूर्णांक dirfd)
{
	पूर्णांक fd = dup(dirfd);
	सूची *dp;

	अगर (fd == -1)
		वापस शून्य;

	अगर (!(dp = fdopendir(fd)))
		बंद(fd);

	वापस dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) अटल पूर्णांक xasprintf(अक्षर **strp, स्थिर अक्षर *fmt, ...)
{
	बहु_सूची ap;
	पूर्णांक len;
	पूर्णांक memlen;
	अक्षर *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	बहु_शुरू(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	बहु_पूर्ण(ap);
	अगर (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		बहु_शुरू(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		बहु_पूर्ण(ap);
	}
	अगर (len < 0 || len >= memlen) {
		/* Give up! */
		ख_लिखो(मानक_त्रुटि, "xasprintf: unable to format a buffer\n");
		मुक्त(ret);
		निकास(1);
	}
	*strp = ret;
	वापस len;
}

RC_UNUSED अटल ssize_t xgetline(अक्षर **restrict lineptr, माप_प्रकार *restrict n, खाता *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	अगर (ret <= 0)
		वापस ret;

	अगर ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	वापस ret;
}

RC_UNUSED अटल अक्षर *env_findvar(स्थिर अक्षर *name, अक्षर **envp)
{
	माप_प्रकार len = म_माप(name);

	क्रम (अक्षर **var = envp; *var; var++)
		अगर (म_भेदन(name, *var, len) == 0 && (*var)[len] == '=')
			वापस *var;
	वापस शून्य;
}

RC_UNUSED अटल पूर्णांक xopen_nonblock(स्थिर अक्षर *filename, पूर्णांक flags, mode_t mode)
{
	पूर्णांक fd = खोलो(filename, flags | O_NONBLOCK, mode);
	अगर (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		पूर्णांक saved_errno = त्रुटि_सं;
		बंद(fd);
		fd = -1;
		त्रुटि_सं = saved_errno;
	}
	वापस fd;
}

RC_UNUSED अटल पूर्णांक open_rwfifo(स्थिर अक्षर *path, bool nonblock)
{
	पूर्णांक r, w, err;

	अगर (mkfifo(path, 0600) == -1 && त्रुटि_सं != EEXIST)
		वापस -1;

	अगर ((r = खोलो(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		वापस -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	अगर ((w = खोलो(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = त्रुटि_सं;
		जाओ r_err;
	}

	अगर (!nonblock) {
		पूर्णांक flags = fcntl(r, F_GETFL);
		अगर (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = त्रुटि_सं;
			जाओ w_err;
		}
	}

	वापस r;

w_err:
	बंद(w);
r_err:
	बंद(r);
	त्रुटि_सं = err;
	वापस -1;
}

#पूर्ण_अगर
