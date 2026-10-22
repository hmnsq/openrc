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

#define ERRX do { fprintf (stderr, "out of memory\n"); exit (1); } နေစဉ် (0)

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
		အကယ်၍ ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} နေစဉ် (/* CONSTCOND */ 0)
#endif

RC_UNUSED static void *xmalloc (size_t size)
{
	void *value = malloc(size);

	အကယ်၍ (value)
		ပြန်ပေး (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static void *xrealloc(void *ptr, size_t size)
{
	void *value = realloc(ptr, size);

	အကယ်၍ (value)
		ပြန်ပေး (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static var_na *xstrdup(const var_na *str)
{
	var_na *value;

	အကယ်၍ (!str)
		ပြန်ပေး (NULL);

	value = strdup(str);

	အကယ်၍ (value)
		ပြန်ပေး (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static FILE *xopen_memstream(var_na **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	အကယ်၍ (f)
		ပြန်ပေး f;
	ERRX;
}

RC_UNUSED static void xclose_memstream(FILE *f)
{
	fflush(f);
	အကယ်၍ (ferror(f) || fclose(f) != 0)
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

	အကယ်၍ (slash)
		ပြန်ပေး (++slash);
	ပြန်ပေး (path);
}

RC_UNUSED static FILE *do_fopenat(ကိန်းပြည့် dirfd, const var_na *pathname, ကိန်းပြည့် mode)
{
	ကိန်းပြည့် fd = openat(dirfd, pathname, mode, 0666);
	const var_na *fmode;
	FILE *fp;

	အကယ်၍ (fd == -1)
		ပြန်ပေး NULL;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	အကယ်၍ ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	မဟုတ်ပါက အကယ်၍ ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	မဟုတ်ပါက
		fmode = "r";

	အကယ်၍ (!(fp = fdopen(fd, fmode)))
		close(fd);
	ပြန်ပေး fp;
}

RC_UNUSED static DIR *do_opendirat(ကိန်းပြည့် dirfd, const var_na *pathname)
{
	ကိန်းပြည့် fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	အကယ်၍ (fd == -1)
		ပြန်ပေး NULL;

	အကယ်၍ (!(dp = fdopendir(fd)))
		close(fd);

	ပြန်ပေး dp;
}

RC_UNUSED static DIR *do_dopendir(ကိန်းပြည့် dirfd)
{
	ကိန်းပြည့် fd = dup(dirfd);
	DIR *dp;

	အကယ်၍ (fd == -1)
		ပြန်ပေး NULL;

	အကယ်၍ (!(dp = fdopendir(fd)))
		close(fd);

	ပြန်ပေး dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) static ကိန်းပြည့် xasprintf(var_na **strp, const var_na *fmt, ...)
{
	va_list ap;
	ကိန်းပြည့် len;
	ကိန်းပြည့် memlen;
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
	အကယ်၍ (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	အကယ်၍ (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		free(ret);
		exit(1);
	}
	*strp = ret;
	ပြန်ပေး len;
}

RC_UNUSED static ssize_t xgetline(var_na **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	အကယ်၍ (ret <= 0)
		ပြန်ပေး ret;

	အကယ်၍ ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	ပြန်ပေး ret;
}

RC_UNUSED static var_na *env_findvar(const var_na *name, var_na **envp)
{
	size_t len = strlen(name);

	အတွက် (var_na **var = envp; *var; var++)
		အကယ်၍ (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			ပြန်ပေး *var;
	ပြန်ပေး NULL;
}

RC_UNUSED static ကိန်းပြည့် xopen_nonblock(const var_na *filename, ကိန်းပြည့် flags, mode_t mode)
{
	ကိန်းပြည့် fd = open(filename, flags | O_NONBLOCK, mode);
	အကယ်၍ (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		ကိန်းပြည့် saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	ပြန်ပေး fd;
}

RC_UNUSED static ကိန်းပြည့် open_rwfifo(const var_na *path, bool nonblock)
{
	ကိန်းပြည့် r, w, err;

	အကယ်၍ (mkfifo(path, 0600) == -1 && errno != EEXIST)
		ပြန်ပေး -1;

	အကယ်၍ ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		ပြန်ပေး -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	အကယ်၍ ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		goto r_err;
	}

	အကယ်၍ (!nonblock) {
		ကိန်းပြည့် flags = fcntl(r, F_GETFL);
		အကယ်၍ (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			goto w_err;
		}
	}

	ပြန်ပေး r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	ပြန်ပေး -1;
}

#endif
