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

#define ERRX করো { fprintf (stderr, "out of memory\n"); exit (1); } যখন (0)

#define UNCONST(a)		((শূন্য *)(uintptr_t)(ধ্রুবক শূন্য *)(a))

#define RC_UNUSED __attribute__((__unused__))
#define RC_NORETURN __attribute__((__noreturn__))
#define RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#define ARRAY_আকার(a) (আকার(a) / আকার((a)[0]))

#ifndef HAVE_STRLCPY
#    define strlcpy(dst, src, আকার_2) snprintf(dst, আকার_2, "%s", src)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					      \
	করো {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		যদি ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} যখন (/* CONSTCOND */ 0)
#endif

RC_UNUSED স্থির শূন্য *xmalloc (size_t আকার_2)
{
	শূন্য *মান = malloc(আকার_2);

	যদি (মান)
		ফেরত (মান);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED স্থির শূন্য *xrealloc(শূন্য *ptr, size_t আকার_2)
{
	শূন্য *মান = realloc(ptr, আকার_2);

	যদি (মান)
		ফেরত (মান);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED স্থির অক্ষর *xstrdup(ধ্রুবক অক্ষর *str)
{
	অক্ষর *মান;

	যদি (!str)
		ফেরত (ফাঁকা);

	মান = strdup(str);

	যদি (মান)
		ফেরত (মান);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED স্থির FILE *xopen_memstream(অক্ষর **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	যদি (f)
		ফেরত f;
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
RC_UNUSED স্থির ধ্রুবক অক্ষর *basename_c(ধ্রুবক অক্ষর *path)
{
	ধ্রুবক অক্ষর *slash = strrchr(path, '/');

	যদি (slash)
		ফেরত (++slash);
	ফেরত (path);
}

RC_UNUSED স্থির FILE *do_fopenat(পূর্ণসংখ্যা dirfd, ধ্রুবক অক্ষর *pathname, পূর্ণসংখ্যা mode)
{
	পূর্ণসংখ্যা fd = openat(dirfd, pathname, mode, 0666);
	ধ্রুবক অক্ষর *fmode;
	FILE *fp;

	যদি (fd == -1)
		ফেরত ফাঁকা;

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
	ফেরত fp;
}

RC_UNUSED স্থির DIR *do_opendirat(পূর্ণসংখ্যা dirfd, ধ্রুবক অক্ষর *pathname)
{
	পূর্ণসংখ্যা fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	যদি (fd == -1)
		ফেরত ফাঁকা;

	যদি (!(dp = fdopendir(fd)))
		close(fd);

	ফেরত dp;
}

RC_UNUSED স্থির DIR *do_dopendir(পূর্ণসংখ্যা dirfd)
{
	পূর্ণসংখ্যা fd = dup(dirfd);
	DIR *dp;

	যদি (fd == -1)
		ফেরত ফাঁকা;

	যদি (!(dp = fdopendir(fd)))
		close(fd);

	ফেরত dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) স্থির পূর্ণসংখ্যা xasprintf(অক্ষর **strp, ধ্রুবক অক্ষর *fmt, ...)
{
	va_list ap;
	পূর্ণসংখ্যা len;
	পূর্ণসংখ্যা memlen;
	অক্ষর *ret;

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
	ফেরত len;
}

RC_UNUSED স্থির ssize_t xgetline(অক্ষর **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	যদি (ret <= 0)
		ফেরত ret;

	যদি ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	ফেরত ret;
}

RC_UNUSED স্থির অক্ষর *env_findvar(ধ্রুবক অক্ষর *name, অক্ষর **envp)
{
	size_t len = strlen(name);

	জন্য (অক্ষর **var = envp; *var; var++)
		যদি (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			ফেরত *var;
	ফেরত ফাঁকা;
}

RC_UNUSED স্থির পূর্ণসংখ্যা xopen_nonblock(ধ্রুবক অক্ষর *filename, পূর্ণসংখ্যা flags, mode_t mode)
{
	পূর্ণসংখ্যা fd = open(filename, flags | O_NONBLOCK, mode);
	যদি (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		পূর্ণসংখ্যা saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	ফেরত fd;
}

RC_UNUSED স্থির পূর্ণসংখ্যা open_rwfifo(ধ্রুবক অক্ষর *path, bool nonblock)
{
	পূর্ণসংখ্যা r, w, err;

	যদি (mkfifo(path, 0600) == -1 && errno != EEXIST)
		ফেরত -1;

	যদি ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		ফেরত -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	যদি ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		যাও r_err;
	}

	যদি (!nonblock) {
		পূর্ণসংখ্যা flags = fcntl(r, F_GETFL);
		যদি (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			যাও w_err;
		}
	}

	ফেরত r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	ফেরত -1;
}

#endif
