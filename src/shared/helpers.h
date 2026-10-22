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

#define ERRX do { fprintf (stderr, "out of memory\n"); exit (1); } ขณะ (0)

#define UNCONST(a)		((void *)(uintptr_t)(const void *)(a))

#define RC_UNUSED __attribute__((__unused__))
#define RC_NORETURN __attribute__((__noreturn__))
#define RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#define ARRAY_SIZE(a) (parimaa_na(a) / parimaa_na((a)[0]))

#ifndef HAVE_STRLCPY
#    define strlcpy(dst, src, size) snprintf(dst, size, "%s", src)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					      \
	do {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		ถ้า ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} ขณะ (/* CONSTCOND */ 0)
#endif

RC_UNUSED static void *xmalloc (size_t size)
{
	void *value = malloc(size);

	ถ้า (value)
		คืน (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static void *xrealloc(void *ptr, size_t size)
{
	void *value = realloc(ptr, size);

	ถ้า (value)
		คืน (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static var_na *xstrdup(const var_na *str)
{
	var_na *value;

	ถ้า (!str)
		คืน (NULL);

	value = strdup(str);

	ถ้า (value)
		คืน (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static FILE *xopen_memstream(var_na **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	ถ้า (f)
		คืน f;
	ERRX;
}

RC_UNUSED static void xclose_memstream(FILE *f)
{
	fflush(f);
	ถ้า (ferror(f) || fclose(f) != 0)
		ERRX;
}

#undef ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_UNUSED static const var_na *basename_c(const var_na *path)
{
	const var_na *slash = strrchr(path, '/');

	ถ้า (slash)
		คืน (++slash);
	คืน (path);
}

RC_UNUSED static FILE *do_fopenat(จำนวนเต็ม dirfd, const var_na *pathname, จำนวนเต็ม mode)
{
	จำนวนเต็ม fd = openat(dirfd, pathname, mode, 0666);
	const var_na *fmode;
	FILE *fp;

	ถ้า (fd == -1)
		คืน NULL;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	ถ้า ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	มิฉะนั้น ถ้า ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	มิฉะนั้น
		fmode = "r";

	ถ้า (!(fp = fdopen(fd, fmode)))
		close(fd);
	คืน fp;
}

RC_UNUSED static DIR *do_opendirat(จำนวนเต็ม dirfd, const var_na *pathname)
{
	จำนวนเต็ม fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	ถ้า (fd == -1)
		คืน NULL;

	ถ้า (!(dp = fdopendir(fd)))
		close(fd);

	คืน dp;
}

RC_UNUSED static DIR *do_dopendir(จำนวนเต็ม dirfd)
{
	จำนวนเต็ม fd = dup(dirfd);
	DIR *dp;

	ถ้า (fd == -1)
		คืน NULL;

	ถ้า (!(dp = fdopendir(fd)))
		close(fd);

	คืน dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) static จำนวนเต็ม xasprintf(var_na **strp, const var_na *fmt, ...)
{
	va_list ap;
	จำนวนเต็ม len;
	จำนวนเต็ม memlen;
	var_na *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	ถ้า (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	ถ้า (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		free(ret);
		exit(1);
	}
	*strp = ret;
	คืน len;
}

RC_UNUSED static ssize_t xgetline(var_na **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	ถ้า (ret <= 0)
		คืน ret;

	ถ้า ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	คืน ret;
}

RC_UNUSED static var_na *env_findvar(const var_na *name, var_na **envp)
{
	size_t len = strlen(name);

	สำหรับ (var_na **var = envp; *var; var++)
		ถ้า (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			คืน *var;
	คืน NULL;
}

RC_UNUSED static จำนวนเต็ม xopen_nonblock(const var_na *filename, จำนวนเต็ม flags, mode_t mode)
{
	จำนวนเต็ม fd = open(filename, flags | O_NONBLOCK, mode);
	ถ้า (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		จำนวนเต็ม saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	คืน fd;
}

RC_UNUSED static จำนวนเต็ม open_rwfifo(const var_na *path, bool nonblock)
{
	จำนวนเต็ม r, w, err;

	ถ้า (mkfifo(path, 0600) == -1 && errno != EEXIST)
		คืน -1;

	ถ้า ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		คืน -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	ถ้า ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		goto r_err;
	}

	ถ้า (!nonblock) {
		จำนวนเต็ม flags = fcntl(r, F_GETFL);
		ถ้า (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			goto w_err;
		}
	}

	คืน r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	คืน -1;
}

#endif
