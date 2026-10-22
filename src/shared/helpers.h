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

#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <dirent.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <errno.h>

#define ERRX কুরু { fprintf (stderr, "out of memory\n"); exit (1); } যাবত্ (0)

#define UNCONST(a)		((শূন্য *)(uintptr_t)(ধ্রুব শূন্য *)(a))

#define RC_UNUSED __attribute__((__unused__))
#define RC_NORETURN __attribute__((__noreturn__))
#define RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#define ARRAY_SIZE(a) (পরিমাণ(a) / পরিমাণ((a)[0]))

#ifndef HAVE_STRLCPY
#    define strlcpy(dst, src, size) snprintf(dst, size, "%s", src)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					      \
	কুরু {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		যদি ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} যাবত্ (/* CONSTCOND */ 0)
#endif

RC_UNUSED স্থির শূন্য *xmalloc (size_t size)
{
	শূন্য *মান = malloc(size);

	যদি (মান)
		প্রত্যাগম (মান);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED স্থির শূন্য *xrealloc(শূন্য *ptr, size_t size)
{
	শূন্য *মান = realloc(ptr, size);

	যদি (মান)
		প্রত্যাগম (মান);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED স্থির বর্ণ *xstrdup(ধ্রুব বর্ণ *str)
{
	বর্ণ *মান;

	যদি (!str)
		প্রত্যাগম (রিক্ত);

	মান = strdup(str);

	যদি (মান)
		প্রত্যাগম (মান);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED স্থির FILE *xopen_memstream(বর্ণ **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	যদি (f)
		প্রত্যাগম f;
	ERRX;
}

RC_UNUSED স্থির শূন্য xclose_memstream(FILE *f)
{
	fflush(f);
	যদি (ferror(f) || fclose(f) != 0)
		ERRX;
}

#undef ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_UNUSED স্থির ধ্রুব বর্ণ *basename_c(ধ্রুব বর্ণ *path)
{
	ধ্রুব বর্ণ *slash = strrchr(path, '/');

	যদি (slash)
		প্রত্যাগম (++slash);
	প্রত্যাগম (path);
}

RC_UNUSED স্থির FILE *do_fopenat(পূর্ণাঙ্ক dirfd, ধ্রুব বর্ণ *pathname, পূর্ণাঙ্ক mode)
{
	পূর্ণাঙ্ক fd = openat(dirfd, pathname, mode, 0666);
	ধ্রুব বর্ণ *fmode;
	FILE *fp;

	যদি (fd == -1)
		প্রত্যাগম রিক্ত;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	যদি ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	অন্যথা যদি ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	অন্যথা
		fmode = "r";

	যদি (!(fp = fdopen(fd, fmode)))
		close(fd);
	প্রত্যাগম fp;
}

RC_UNUSED স্থির DIR *do_opendirat(পূর্ণাঙ্ক dirfd, ধ্রুব বর্ণ *pathname)
{
	পূর্ণাঙ্ক fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	যদি (fd == -1)
		প্রত্যাগম রিক্ত;

	যদি (!(dp = fdopendir(fd)))
		close(fd);

	প্রত্যাগম dp;
}

RC_UNUSED স্থির DIR *do_dopendir(পূর্ণাঙ্ক dirfd)
{
	পূর্ণাঙ্ক fd = dup(dirfd);
	DIR *dp;

	যদি (fd == -1)
		প্রত্যাগম রিক্ত;

	যদি (!(dp = fdopendir(fd)))
		close(fd);

	প্রত্যাগম dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) স্থির পূর্ণাঙ্ক xasprintf(বর্ণ **strp, ধ্রুব বর্ণ *fmt, ...)
{
	va_list ap;
	পূর্ণাঙ্ক len;
	পূর্ণাঙ্ক memlen;
	বর্ণ *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	যদি (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	যদি (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		মুক্ত(ret);
		exit(1);
	}
	*strp = ret;
	প্রত্যাগম len;
}

RC_UNUSED স্থির ssize_t xgetline(বর্ণ **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	যদি (ret <= 0)
		প্রত্যাগম ret;

	যদি ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	প্রত্যাগম ret;
}

RC_UNUSED স্থির বর্ণ *env_findvar(ধ্রুব বর্ণ *name, বর্ণ **envp)
{
	size_t len = strlen(name);

	প্রতি (বর্ণ **var = envp; *var; var++)
		যদি (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			প্রত্যাগম *var;
	প্রত্যাগম রিক্ত;
}

RC_UNUSED স্থির পূর্ণাঙ্ক xopen_nonblock(ধ্রুব বর্ণ *filename, পূর্ণাঙ্ক flags, mode_t mode)
{
	পূর্ণাঙ্ক fd = open(filename, flags | O_NONBLOCK, mode);
	যদি (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		পূর্ণাঙ্ক saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	প্রত্যাগম fd;
}

RC_UNUSED স্থির পূর্ণাঙ্ক open_rwfifo(ধ্রুব বর্ণ *path, bool nonblock)
{
	পূর্ণাঙ্ক r, w, err;

	যদি (mkfifo(path, 0600) == -1 && errno != EEXIST)
		প্রত্যাগম -1;

	যদি ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		প্রত্যাগম -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	যদি ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		গচ্ছ r_err;
	}

	যদি (!nonblock) {
		পূর্ণাঙ্ক flags = fcntl(r, F_GETFL);
		যদি (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			গচ্ছ w_err;
		}
	}

	প্রত্যাগম r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	প্রত্যাগম -1;
}

#endif
