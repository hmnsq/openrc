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

#define ERRX do { fprintf (stderr, "out of memory\n"); exit (1); } докато (0)

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
		ако ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} докато (/* CONSTCOND */ 0)
#endif

RC_UNUSED static void *xmalloc (size_t size)
{
	void *value = malloc(size);

	ако (value)
		върни (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static void *xrealloc(void *ptr, size_t size)
{
	void *value = realloc(ptr, size);

	ако (value)
		върни (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static char *xstrdup(const char *str)
{
	char *value;

	ако (!str)
		върни (NULL);

	value = strdup(str);

	ако (value)
		върни (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static FILE *xopen_memstream(char **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	ако (f)
		върни f;
	ERRX;
}

RC_UNUSED static void xclose_memstream(FILE *f)
{
	fflush(f);
	ако (ferror(f) || fclose(f) != 0)
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

	ако (slash)
		върни (++slash);
	върни (path);
}

RC_UNUSED static FILE *do_fopenat(цяло dirfd, const char *pathname, цяло mode)
{
	цяло fd = openat(dirfd, pathname, mode, 0666);
	const char *fmode;
	FILE *fp;

	ако (fd == -1)
		върни NULL;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	ако ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	иначе ако ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	иначе
		fmode = "r";

	ако (!(fp = fdopen(fd, fmode)))
		close(fd);
	върни fp;
}

RC_UNUSED static DIR *do_opendirat(цяло dirfd, const char *pathname)
{
	цяло fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	ако (fd == -1)
		върни NULL;

	ако (!(dp = fdopendir(fd)))
		close(fd);

	върни dp;
}

RC_UNUSED static DIR *do_dopendir(цяло dirfd)
{
	цяло fd = dup(dirfd);
	DIR *dp;

	ако (fd == -1)
		върни NULL;

	ако (!(dp = fdopendir(fd)))
		close(fd);

	върни dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) static цяло xasprintf(char **strp, const char *fmt, ...)
{
	va_list ap;
	цяло len;
	цяло memlen;
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
	ако (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	ако (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		free(ret);
		exit(1);
	}
	*strp = ret;
	върни len;
}

RC_UNUSED static ssize_t xgetline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	ако (ret <= 0)
		върни ret;

	ако ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	върни ret;
}

RC_UNUSED static char *env_findvar(const char *name, char **envp)
{
	size_t len = strlen(name);

	за (char **var = envp; *var; var++)
		ако (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			върни *var;
	върни NULL;
}

RC_UNUSED static цяло xopen_nonblock(const char *filename, цяло flags, mode_t mode)
{
	цяло fd = open(filename, flags | O_NONBLOCK, mode);
	ако (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		цяло saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	върни fd;
}

RC_UNUSED static цяло open_rwfifo(const char *path, bool nonblock)
{
	цяло r, w, err;

	ако (mkfifo(path, 0600) == -1 && errno != EEXIST)
		върни -1;

	ако ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		върни -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	ако ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		goto r_err;
	}

	ако (!nonblock) {
		цяло flags = fcntl(r, F_GETFL);
		ако (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			goto w_err;
		}
	}

	върни r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	върни -1;
}

#endif
