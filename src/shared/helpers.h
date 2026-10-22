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

#define ERRX कुरु { fprintf (stderr, "out of memory\n"); exit (1); } यावत् (0)

#define UNCONST(a)		((शून्य *)(uintptr_t)(ध्रुव शून्य *)(a))

#define RC_UNUSED __attribute__((__unused__))
#define RC_NORETURN __attribute__((__noreturn__))
#define RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#define ARRAY_SIZE(a) (परिमाण(a) / परिमाण((a)[0]))

#ifndef HAVE_STRLCPY
#    define strlcpy(dst, src, size) snprintf(dst, size, "%s", src)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					      \
	कुरु {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		यदि ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} यावत् (/* CONSTCOND */ 0)
#endif

RC_UNUSED स्थिर शून्य *xmalloc (size_t size)
{
	शून्य *मूल्य = malloc(size);

	यदि (मूल्य)
		प्रत्यागम (मूल्य);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED स्थिर शून्य *xrealloc(शून्य *ptr, size_t size)
{
	शून्य *मूल्य = realloc(ptr, size);

	यदि (मूल्य)
		प्रत्यागम (मूल्य);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED स्थिर वर्ण *xstrdup(ध्रुव वर्ण *str)
{
	वर्ण *मूल्य;

	यदि (!str)
		प्रत्यागम (रिक्त);

	मूल्य = strdup(str);

	यदि (मूल्य)
		प्रत्यागम (मूल्य);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED स्थिर FILE *xopen_memstream(वर्ण **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	यदि (f)
		प्रत्यागम f;
	ERRX;
}

RC_UNUSED स्थिर शून्य xclose_memstream(FILE *f)
{
	fflush(f);
	यदि (ferror(f) || fclose(f) != 0)
		ERRX;
}

#undef ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_UNUSED स्थिर ध्रुव वर्ण *basename_c(ध्रुव वर्ण *path)
{
	ध्रुव वर्ण *slash = strrchr(path, '/');

	यदि (slash)
		प्रत्यागम (++slash);
	प्रत्यागम (path);
}

RC_UNUSED स्थिर FILE *do_fopenat(पूर्णाङ्क dirfd, ध्रुव वर्ण *pathname, पूर्णाङ्क mode)
{
	पूर्णाङ्क fd = openat(dirfd, pathname, mode, 0666);
	ध्रुव वर्ण *fmode;
	FILE *fp;

	यदि (fd == -1)
		प्रत्यागम रिक्त;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	यदि ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	अन्यथा यदि ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	अन्यथा
		fmode = "r";

	यदि (!(fp = fdopen(fd, fmode)))
		close(fd);
	प्रत्यागम fp;
}

RC_UNUSED स्थिर DIR *do_opendirat(पूर्णाङ्क dirfd, ध्रुव वर्ण *pathname)
{
	पूर्णाङ्क fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	यदि (fd == -1)
		प्रत्यागम रिक्त;

	यदि (!(dp = fdopendir(fd)))
		close(fd);

	प्रत्यागम dp;
}

RC_UNUSED स्थिर DIR *do_dopendir(पूर्णाङ्क dirfd)
{
	पूर्णाङ्क fd = dup(dirfd);
	DIR *dp;

	यदि (fd == -1)
		प्रत्यागम रिक्त;

	यदि (!(dp = fdopendir(fd)))
		close(fd);

	प्रत्यागम dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) स्थिर पूर्णाङ्क xasprintf(वर्ण **strp, ध्रुव वर्ण *fmt, ...)
{
	va_list ap;
	पूर्णाङ्क len;
	पूर्णाङ्क memlen;
	वर्ण *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	यदि (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	यदि (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		मुक्त(ret);
		exit(1);
	}
	*strp = ret;
	प्रत्यागम len;
}

RC_UNUSED स्थिर ssize_t xgetline(वर्ण **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	यदि (ret <= 0)
		प्रत्यागम ret;

	यदि ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	प्रत्यागम ret;
}

RC_UNUSED स्थिर वर्ण *env_findvar(ध्रुव वर्ण *name, वर्ण **envp)
{
	size_t len = strlen(name);

	प्रति (वर्ण **var = envp; *var; var++)
		यदि (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			प्रत्यागम *var;
	प्रत्यागम रिक्त;
}

RC_UNUSED स्थिर पूर्णाङ्क xopen_nonblock(ध्रुव वर्ण *filename, पूर्णाङ्क flags, mode_t mode)
{
	पूर्णाङ्क fd = open(filename, flags | O_NONBLOCK, mode);
	यदि (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		पूर्णाङ्क saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	प्रत्यागम fd;
}

RC_UNUSED स्थिर पूर्णाङ्क open_rwfifo(ध्रुव वर्ण *path, bool nonblock)
{
	पूर्णाङ्क r, w, err;

	यदि (mkfifo(path, 0600) == -1 && errno != EEXIST)
		प्रत्यागम -1;

	यदि ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		प्रत्यागम -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	यदि ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		गच्छ r_err;
	}

	यदि (!nonblock) {
		पूर्णाङ्क flags = fcntl(r, F_GETFL);
		यदि (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			गच्छ w_err;
		}
	}

	प्रत्यागम r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	प्रत्यागम -1;
}

#endif
