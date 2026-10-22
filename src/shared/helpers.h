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

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define ERRX do { fprintf (stderr, "out of memory\n"); exit (1); } 当 (0)

#define UNCONST(a)		((void *)(uintptr_t)(const void *)(a))

#define RC_UNUSED __attribute__((__unused__))
#define RC_NORETURN __attribute__((__noreturn__))
#define RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#ifndef HAVE_STRLCPY
#    define strlcpy(dst, src, size) snprintf(dst, size, "%s", src)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					      \
	do {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		如果 ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} 当 (/* CONSTCOND */ 0)
#endif

RC_UNUSED static void *xmalloc (size_t size)
{
	void *value = malloc(size);

	如果 (value)
		返回 (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static void *xrealloc(void *ptr, size_t size)
{
	void *value = realloc(ptr, size);

	如果 (value)
		返回 (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static char *xstrdup(const char *str)
{
	char *value;

	如果 (!str)
		返回 (NULL);

	value = strdup(str);

	如果 (value)
		返回 (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static FILE *xopen_memstream(char **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	如果 (f)
		返回 f;
	ERRX;
}

RC_UNUSED static void xclose_memstream(FILE *f)
{
	fflush(f);
	如果 (ferror(f) || fclose(f) != 0)
		ERRX;
}

#undef ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_UNUSED static const char *basename_c(const char *path)
{
	const char *slash = strrchr(path, '/');

	如果 (slash)
		返回 (++slash);
	返回 (path);
}

RC_UNUSED static FILE *do_fopenat(整数 dirfd, const char *pathname, 整数 mode)
{
	整数 fd = openat(dirfd, pathname, mode, 0666);
	const char *fmode;
	FILE *fp;

	如果 (fd == -1)
		返回 NULL;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	如果 ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	否则 如果 ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	否则
		fmode = "r";

	如果 (!(fp = fdopen(fd, fmode)))
		close(fd);
	返回 fp;
}

RC_UNUSED static DIR *do_opendirat(整数 dirfd, const char *pathname)
{
	整数 fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	如果 (fd == -1)
		返回 NULL;

	如果 (!(dp = fdopendir(fd)))
		close(fd);

	返回 dp;
}

RC_UNUSED static DIR *do_dopendir(整数 dirfd)
{
	整数 fd = dup(dirfd);
	DIR *dp;

	如果 (fd == -1)
		返回 NULL;

	如果 (!(dp = fdopendir(fd)))
		close(fd);

	返回 dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) static 整数 xasprintf(char **strp, const char *fmt, ...)
{
	va_list ap;
	整数 len;
	整数 memlen;
	char *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	如果 (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	如果 (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		free(ret);
		exit(1);
	}
	*strp = ret;
	返回 len;
}

RC_UNUSED static ssize_t xgetline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	如果 (ret <= 0)
		返回 ret;

	如果 ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	返回 ret;
}

RC_UNUSED static char *env_findvar(const char *name, char **envp)
{
	size_t len = strlen(name);

	为 (char **var = envp; *var; var++)
		如果 (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			返回 *var;
	返回 NULL;
}

RC_UNUSED static 整数 xopen_nonblock(const char *filename, 整数 flags, mode_t mode)
{
	整数 fd = open(filename, flags | O_NONBLOCK, mode);
	如果 (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		整数 saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	返回 fd;
}

RC_UNUSED static 整数 open_rwfifo(const char *path, bool nonblock)
{
	整数 r, w, err;

	如果 (mkfifo(path, 0600) == -1 && errno != EEXIST)
		返回 -1;

	如果 ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		返回 -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	如果 ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		goto r_err;
	}

	如果 (!nonblock) {
		整数 flags = fcntl(r, F_GETFL);
		如果 (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			goto w_err;
		}
	}

	返回 r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	返回 -1;
}

#endif
