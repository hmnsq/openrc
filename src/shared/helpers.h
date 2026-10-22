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

#define ERRX do { fprintf (stderr, "out of memory\n"); exit (1); } අතරතුර (0)

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
		නම් ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} අතරතුර (/* CONSTCOND */ 0)
#endif

RC_UNUSED static void *xmalloc (size_t size)
{
	void *value = malloc(size);

	නම් (value)
		ආපසු (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static void *xrealloc(void *ptr, size_t size)
{
	void *value = realloc(ptr, size);

	නම් (value)
		ආපසු (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static var_na *xstrdup(const var_na *str)
{
	var_na *value;

	නම් (!str)
		ආපසු (NULL);

	value = strdup(str);

	නම් (value)
		ආපසු (value);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED static FILE *xopen_memstream(var_na **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	නම් (f)
		ආපසු f;
	ERRX;
}

RC_UNUSED static void xclose_memstream(FILE *f)
{
	fflush(f);
	නම් (ferror(f) || fclose(f) != 0)
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

	නම් (slash)
		ආපසු (++slash);
	ආපසු (path);
}

RC_UNUSED static FILE *do_fopenat(පූර්ණ_සංඛ්‍යා dirfd, const var_na *pathname, පූර්ණ_සංඛ්‍යා mode)
{
	පූර්ණ_සංඛ්‍යා fd = openat(dirfd, pathname, mode, 0666);
	const var_na *fmode;
	FILE *fp;

	නම් (fd == -1)
		ආපසු NULL;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	නම් ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	නැතහොත් නම් ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	නැතහොත්
		fmode = "r";

	නම් (!(fp = fdopen(fd, fmode)))
		close(fd);
	ආපසු fp;
}

RC_UNUSED static DIR *do_opendirat(පූර්ණ_සංඛ්‍යා dirfd, const var_na *pathname)
{
	පූර්ණ_සංඛ්‍යා fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	නම් (fd == -1)
		ආපසු NULL;

	නම් (!(dp = fdopendir(fd)))
		close(fd);

	ආපසු dp;
}

RC_UNUSED static DIR *do_dopendir(පූර්ණ_සංඛ්‍යා dirfd)
{
	පූර්ණ_සංඛ්‍යා fd = dup(dirfd);
	DIR *dp;

	නම් (fd == -1)
		ආපසු NULL;

	නම් (!(dp = fdopendir(fd)))
		close(fd);

	ආපසු dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) static පූර්ණ_සංඛ්‍යා xasprintf(var_na **strp, const var_na *fmt, ...)
{
	va_list ap;
	පූර්ණ_සංඛ්‍යා len;
	පූර්ණ_සංඛ්‍යා memlen;
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
	නම් (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	නම් (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		free(ret);
		exit(1);
	}
	*strp = ret;
	ආපසු len;
}

RC_UNUSED static ssize_t xgetline(var_na **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	නම් (ret <= 0)
		ආපසු ret;

	නම් ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	ආපසු ret;
}

RC_UNUSED static var_na *env_findvar(const var_na *name, var_na **envp)
{
	size_t len = strlen(name);

	සඳහා (var_na **var = envp; *var; var++)
		නම් (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			ආපසු *var;
	ආපසු NULL;
}

RC_UNUSED static පූර්ණ_සංඛ්‍යා xopen_nonblock(const var_na *filename, පූර්ණ_සංඛ්‍යා flags, mode_t mode)
{
	පූර්ණ_සංඛ්‍යා fd = open(filename, flags | O_NONBLOCK, mode);
	නම් (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		පූර්ණ_සංඛ්‍යා saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	ආපසු fd;
}

RC_UNUSED static පූර්ණ_සංඛ්‍යා open_rwfifo(const var_na *path, bool nonblock)
{
	පූර්ණ_සංඛ්‍යා r, w, err;

	නම් (mkfifo(path, 0600) == -1 && errno != EEXIST)
		ආපසු -1;

	නම් ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		ආපසු -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	නම් ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		goto r_err;
	}

	නම් (!nonblock) {
		පූර්ණ_සංඛ්‍යා flags = fcntl(r, F_GETFL);
		නම් (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			goto w_err;
		}
	}

	ආපසු r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	ආපසු -1;
}

#endif
