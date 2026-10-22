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

#ifndef __HELPERS_H__
#define __HELPERS_H__

#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <dirent.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <errno.h>

#define ERRX करा { ख_लिखो (मानक_त्रुटि, "out of memory\n"); निकास (1); } पर्यंत (0)

#define UNCONST(a)		((शून्य *)(uintptr_t)(स्थिरांक शून्य *)(a))

#define RC_UNUSED __attribute__((__unused__))
#define RC_NORETURN __attribute__((__noreturn__))
#define RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#define ARRAY_SIZE(a) (आकार(a) / आकार((a)[0]))

#ifndef HAVE_STRLCPY
#    define strlcpy(dst, src, size) snprintf(dst, size, "%s", src)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					      \
	करा {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		जर ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} पर्यंत (/* CONSTCOND */ 0)
#endif

RC_UNUSED स्थिर शून्य *xmalloc (माप_प्रकार size)
{
	शून्य *मूल्य = दो_स्मृति(size);

	जर (मूल्य)
		परत (मूल्य);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED स्थिर शून्य *xrealloc(शून्य *ptr, माप_प्रकार size)
{
	शून्य *मूल्य = पुनः_स्मृति(ptr, size);

	जर (मूल्य)
		परत (मूल्य);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED स्थिर अक्षर *xstrdup(स्थिरांक अक्षर *str)
{
	अक्षर *मूल्य;

	जर (!str)
		परत (रिक्त);

	मूल्य = strdup(str);

	जर (मूल्य)
		परत (मूल्य);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED स्थिर खाता *xopen_memstream(अक्षर **ptr, माप_प्रकार *sizeloc)
{
	खाता *f = open_memstream(ptr, sizeloc);
	जर (f)
		परत f;
	ERRX;
}

RC_UNUSED स्थिर शून्य xclose_memstream(खाता *f)
{
	ख_साफ(f);
	जर (ख_त्रुटि(f) || ख_बंद(f) != 0)
		ERRX;
}

#undef ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_UNUSED स्थिर स्थिरांक अक्षर *basename_c(स्थिरांक अक्षर *path)
{
	स्थिरांक अक्षर *slash = म_खोजप(path, '/');

	जर (slash)
		परत (++slash);
	परत (path);
}

RC_UNUSED स्थिर खाता *do_fopenat(पूर्णांक dirfd, स्थिरांक अक्षर *pathname, पूर्णांक mode)
{
	पूर्णांक fd = openat(dirfd, pathname, mode, 0666);
	स्थिरांक अक्षर *fmode;
	खाता *fp;

	जर (fd == -1)
		परत रिक्त;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	जर ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	अन्यथा जर ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	अन्यथा
		fmode = "r";

	जर (!(fp = fdopen(fd, fmode)))
		बंद(fd);
	परत fp;
}

RC_UNUSED स्थिर सूची *do_opendirat(पूर्णांक dirfd, स्थिरांक अक्षर *pathname)
{
	पूर्णांक fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	सूची *dp;

	जर (fd == -1)
		परत रिक्त;

	जर (!(dp = fdopendir(fd)))
		बंद(fd);

	परत dp;
}

RC_UNUSED स्थिर सूची *do_dopendir(पूर्णांक dirfd)
{
	पूर्णांक fd = dup(dirfd);
	सूची *dp;

	जर (fd == -1)
		परत रिक्त;

	जर (!(dp = fdopendir(fd)))
		बंद(fd);

	परत dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) स्थिर पूर्णांक xasprintf(अक्षर **strp, स्थिरांक अक्षर *fmt, ...)
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
	जर (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		बहु_शुरू(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		बहु_पूर्ण(ap);
	}
	जर (len < 0 || len >= memlen) {
		/* Give up! */
		ख_लिखो(मानक_त्रुटि, "xasprintf: unable to format a buffer\n");
		मुक्त(ret);
		निकास(1);
	}
	*strp = ret;
	परत len;
}

RC_UNUSED स्थिर ssize_t xgetline(अक्षर **restrict lineptr, माप_प्रकार *restrict n, खाता *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	जर (ret <= 0)
		परत ret;

	जर ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	परत ret;
}

RC_UNUSED स्थिर अक्षर *env_findvar(स्थिरांक अक्षर *name, अक्षर **envp)
{
	माप_प्रकार len = म_माप(name);

	साठी (अक्षर **var = envp; *var; var++)
		जर (म_भेदन(name, *var, len) == 0 && (*var)[len] == '=')
			परत *var;
	परत रिक्त;
}

RC_UNUSED स्थिर पूर्णांक xopen_nonblock(स्थिरांक अक्षर *filename, पूर्णांक flags, mode_t mode)
{
	पूर्णांक fd = खोलो(filename, flags | O_NONBLOCK, mode);
	जर (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		पूर्णांक saved_errno = त्रुटि_सं;
		बंद(fd);
		fd = -1;
		त्रुटि_सं = saved_errno;
	}
	परत fd;
}

RC_UNUSED स्थिर पूर्णांक open_rwfifo(स्थिरांक अक्षर *path, bool nonblock)
{
	पूर्णांक r, w, err;

	जर (mkfifo(path, 0600) == -1 && त्रुटि_सं != EEXIST)
		परत -1;

	जर ((r = खोलो(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		परत -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	जर ((w = खोलो(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = त्रुटि_सं;
		जा r_err;
	}

	जर (!nonblock) {
		पूर्णांक flags = fcntl(r, F_GETFL);
		जर (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = त्रुटि_सं;
			जा w_err;
		}
	}

	परत r;

w_err:
	बंद(w);
r_err:
	बंद(r);
	त्रुटि_सं = err;
	परत -1;
}

#endif
