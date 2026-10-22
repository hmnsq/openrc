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

#define ERRX do { fprintf (stderr, "out of memory\n"); exit (1); } trong_khi (0)

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
		nếu ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} trong_khi (/* CONSTCOND */ 0)
#endif

RC_UNUSED static void *xmalloc (size_t size)
{
	void *value = malloc(size);

	nếu (value)
		trả_về (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static void *xrealloc(void *ptr, size_t size)
{
	void *value = realloc(ptr, size);

	nếu (value)
		trả_về (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static char *xstrdup(const char *str)
{
	char *value;

	nếu (!str)
		trả_về (NULL);

	value = strdup(str);

	nếu (value)
		trả_về (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static FILE *xopen_memstream(char **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	nếu (f)
		trả_về f;
	ERRX;
}

RC_UNUSED static void xclose_memstream(FILE *f)
{
	fflush(f);
	nếu (ferror(f) || fclose(f) != 0)
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

	nếu (slash)
		trả_về (++slash);
	trả_về (path);
}

RC_UNUSED static FILE *do_fopenat(số_nguyên dirfd, const char *pathname, số_nguyên mode)
{
	số_nguyên fd = openat(dirfd, pathname, mode, 0666);
	const char *fmode;
	FILE *fp;

	nếu (fd == -1)
		trả_về NULL;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	nếu ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	không_thì nếu ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	không_thì
		fmode = "r";

	nếu (!(fp = fdopen(fd, fmode)))
		close(fd);
	trả_về fp;
}

RC_UNUSED static DIR *do_opendirat(số_nguyên dirfd, const char *pathname)
{
	số_nguyên fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	nếu (fd == -1)
		trả_về NULL;

	nếu (!(dp = fdopendir(fd)))
		close(fd);

	trả_về dp;
}

RC_UNUSED static DIR *do_dopendir(số_nguyên dirfd)
{
	số_nguyên fd = dup(dirfd);
	DIR *dp;

	nếu (fd == -1)
		trả_về NULL;

	nếu (!(dp = fdopendir(fd)))
		close(fd);

	trả_về dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) static số_nguyên xasprintf(char **strp, const char *fmt, ...)
{
	va_list ap;
	số_nguyên len;
	số_nguyên memlen;
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
	nếu (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	nếu (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		free(ret);
		exit(1);
	}
	*strp = ret;
	trả_về len;
}

RC_UNUSED static ssize_t xgetline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	nếu (ret <= 0)
		trả_về ret;

	nếu ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	trả_về ret;
}

RC_UNUSED static char *env_findvar(const char *name, char **envp)
{
	size_t len = strlen(name);

	cho (char **var = envp; *var; var++)
		nếu (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			trả_về *var;
	trả_về NULL;
}

RC_UNUSED static số_nguyên xopen_nonblock(const char *filename, số_nguyên flags, mode_t mode)
{
	số_nguyên fd = open(filename, flags | O_NONBLOCK, mode);
	nếu (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		số_nguyên saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	trả_về fd;
}

RC_UNUSED static số_nguyên open_rwfifo(const char *path, bool nonblock)
{
	số_nguyên r, w, err;

	nếu (mkfifo(path, 0600) == -1 && errno != EEXIST)
		trả_về -1;

	nếu ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		trả_về -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	nếu ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		goto r_err;
	}

	nếu (!nonblock) {
		số_nguyên flags = fcntl(r, F_GETFL);
		nếu (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			goto w_err;
		}
	}

	trả_về r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	trả_về -1;
}

#endif
