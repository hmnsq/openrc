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

#شامل <stdarg.h>
#شامل <stdbool.h>
#شامل <stddef.h>
#شامل <stdint.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/stat.h>
#شامل <dirent.h>
#شامل <unistd.h>
#شامل <fcntl.h>
#شامل <errno.h>

#define ERRX انجامبده { fprintf (stderr, "out of memory\n"); exit (1); } وقتی (0)

#define UNCONST(a)		((تهی *)(uintptr_t)(ثابت تهی *)(a))

#define RC_UNUSED __attribute__((__unused__))
#define RC_NORETURN __attribute__((__noreturn__))
#define RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#define ARRAY_SIZE(a) (اندازه(a) / اندازه((a)[0]))

#ifndef HAVE_STRLCPY
#    define strlcpy(dst, src, size) snprintf(dst, size, "%s", src)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					      \
	انجامبده {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		اگر ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} وقتی (/* CONSTCOND */ 0)
#endif

RC_UNUSED ایستا تهی *xmalloc (size_t size)
{
	تهی *مقدار = malloc(size);

	اگر (مقدار)
		بازگشت (مقدار);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED ایستا تهی *xrealloc(تهی *ptr, size_t size)
{
	تهی *مقدار = realloc(ptr, size);

	اگر (مقدار)
		بازگشت (مقدار);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED ایستا نویسه *xstrdup(ثابت نویسه *str)
{
	نویسه *مقدار;

	اگر (!str)
		بازگشت (پوچ);

	مقدار = strdup(str);

	اگر (مقدار)
		بازگشت (مقدار);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED ایستا FILE *xopen_memstream(نویسه **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	اگر (f)
		بازگشت f;
	ERRX;
}

RC_UNUSED ایستا تهی xclose_memstream(FILE *f)
{
	fflush(f);
	اگر (ferror(f) || fclose(f) != 0)
		ERRX;
}

#undef ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_UNUSED ایستا ثابت نویسه *basename_c(ثابت نویسه *path)
{
	ثابت نویسه *slash = strrchr(path, '/');

	اگر (slash)
		بازگشت (++slash);
	بازگشت (path);
}

RC_UNUSED ایستا FILE *do_fopenat(صحیح dirfd, ثابت نویسه *pathname, صحیح mode)
{
	صحیح fd = openat(dirfd, pathname, mode, 0666);
	ثابت نویسه *fmode;
	FILE *fp;

	اگر (fd == -1)
		بازگشت پوچ;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	اگر ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	وگرنه اگر ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	وگرنه
		fmode = "r";

	اگر (!(fp = fdopen(fd, fmode)))
		close(fd);
	بازگشت fp;
}

RC_UNUSED ایستا DIR *do_opendirat(صحیح dirfd, ثابت نویسه *pathname)
{
	صحیح fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	اگر (fd == -1)
		بازگشت پوچ;

	اگر (!(dp = fdopendir(fd)))
		close(fd);

	بازگشت dp;
}

RC_UNUSED ایستا DIR *do_dopendir(صحیح dirfd)
{
	صحیح fd = dup(dirfd);
	DIR *dp;

	اگر (fd == -1)
		بازگشت پوچ;

	اگر (!(dp = fdopendir(fd)))
		close(fd);

	بازگشت dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) ایستا صحیح xasprintf(نویسه **strp, ثابت نویسه *fmt, ...)
{
	va_list ap;
	صحیح len;
	صحیح memlen;
	نویسه *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	اگر (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	اگر (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		آزاد(ret);
		exit(1);
	}
	*strp = ret;
	بازگشت len;
}

RC_UNUSED ایستا ssize_t xgetline(نویسه **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	اگر (ret <= 0)
		بازگشت ret;

	اگر ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	بازگشت ret;
}

RC_UNUSED ایستا نویسه *env_findvar(ثابت نویسه *name, نویسه **envp)
{
	size_t len = strlen(name);

	برای (نویسه **var = envp; *var; var++)
		اگر (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			بازگشت *var;
	بازگشت پوچ;
}

RC_UNUSED ایستا صحیح xopen_nonblock(ثابت نویسه *filename, صحیح flags, mode_t mode)
{
	صحیح fd = open(filename, flags | O_NONBLOCK, mode);
	اگر (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		صحیح saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	بازگشت fd;
}

RC_UNUSED ایستا صحیح open_rwfifo(ثابت نویسه *path, bool nonblock)
{
	صحیح r, w, err;

	اگر (mkfifo(path, 0600) == -1 && errno != EEXIST)
		بازگشت -1;

	اگر ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		بازگشت -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	اگر ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		برو r_err;
	}

	اگر (!nonblock) {
		صحیح flags = fcntl(r, F_GETFL);
		اگر (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			برو w_err;
		}
	}

	بازگشت r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	بازگشت -1;
}

#endif
