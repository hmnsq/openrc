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

#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <dirent.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <errno.h>

#define ERRX செய் { fprintf (stderr, "out of memory\n"); exit (1); } போது (0)

#define UNCONST(a)		((வெறுமை *)(uintptr_t)(மாறிலி வெறுமை *)(a))

#define RC_UNUSED __attribute__((__unused__))
#define RC_NORETURN __attribute__((__noreturn__))
#define RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#define ARRAY_SIZE(a) (அளவு(a) / அளவு((a)[0]))

#ifndef HAVE_STRLCPY
#    define strlcpy(dst, src, size) snprintf(dst, size, "%s", src)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					      \
	செய் {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		எனில் ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} போது (/* CONSTCOND */ 0)
#endif

RC_UNUSED நிலையான வெறுமை *xmalloc (size_t size)
{
	வெறுமை *மதிப்பு = malloc(size);

	எனில் (மதிப்பு)
		திரும்பு (மதிப்பு);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED நிலையான வெறுமை *xrealloc(வெறுமை *ptr, size_t size)
{
	வெறுமை *மதிப்பு = realloc(ptr, size);

	எனில் (மதிப்பு)
		திரும்பு (மதிப்பு);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED நிலையான எழுத்து *xstrdup(மாறிலி எழுத்து *str)
{
	எழுத்து *மதிப்பு;

	எனில் (!str)
		திரும்பு (வெற்று);

	மதிப்பு = strdup(str);

	எனில் (மதிப்பு)
		திரும்பு (மதிப்பு);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED நிலையான FILE *xopen_memstream(எழுத்து **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	எனில் (f)
		திரும்பு f;
	ERRX;
}

RC_UNUSED நிலையான வெறுமை xclose_memstream(FILE *f)
{
	fflush(f);
	எனில் (ferror(f) || fclose(f) != 0)
		ERRX;
}

#undef ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_UNUSED நிலையான மாறிலி எழுத்து *basename_c(மாறிலி எழுத்து *path)
{
	மாறிலி எழுத்து *slash = strrchr(path, '/');

	எனில் (slash)
		திரும்பு (++slash);
	திரும்பு (path);
}

RC_UNUSED நிலையான FILE *do_fopenat(முழு dirfd, மாறிலி எழுத்து *pathname, முழு mode)
{
	முழு fd = openat(dirfd, pathname, mode, 0666);
	மாறிலி எழுத்து *fmode;
	FILE *fp;

	எனில் (fd == -1)
		திரும்பு வெற்று;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	எனில் ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	இல்லையெனில் எனில் ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	இல்லையெனில்
		fmode = "r";

	எனில் (!(fp = fdopen(fd, fmode)))
		close(fd);
	திரும்பு fp;
}

RC_UNUSED நிலையான DIR *do_opendirat(முழு dirfd, மாறிலி எழுத்து *pathname)
{
	முழு fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	எனில் (fd == -1)
		திரும்பு வெற்று;

	எனில் (!(dp = fdopendir(fd)))
		close(fd);

	திரும்பு dp;
}

RC_UNUSED நிலையான DIR *do_dopendir(முழு dirfd)
{
	முழு fd = dup(dirfd);
	DIR *dp;

	எனில் (fd == -1)
		திரும்பு வெற்று;

	எனில் (!(dp = fdopendir(fd)))
		close(fd);

	திரும்பு dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) நிலையான முழு xasprintf(எழுத்து **strp, மாறிலி எழுத்து *fmt, ...)
{
	va_list ap;
	முழு len;
	முழு memlen;
	எழுத்து *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	எனில் (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	எனில் (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		விடுவி(ret);
		exit(1);
	}
	*strp = ret;
	திரும்பு len;
}

RC_UNUSED நிலையான ssize_t xgetline(எழுத்து **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	எனில் (ret <= 0)
		திரும்பு ret;

	எனில் ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	திரும்பு ret;
}

RC_UNUSED நிலையான எழுத்து *env_findvar(மாறிலி எழுத்து *name, எழுத்து **envp)
{
	size_t len = strlen(name);

	க்கு (எழுத்து **var = envp; *var; var++)
		எனில் (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			திரும்பு *var;
	திரும்பு வெற்று;
}

RC_UNUSED நிலையான முழு xopen_nonblock(மாறிலி எழுத்து *filename, முழு flags, mode_t mode)
{
	முழு fd = open(filename, flags | O_NONBLOCK, mode);
	எனில் (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		முழு saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	திரும்பு fd;
}

RC_UNUSED நிலையான முழு open_rwfifo(மாறிலி எழுத்து *path, bool nonblock)
{
	முழு r, w, err;

	எனில் (mkfifo(path, 0600) == -1 && errno != EEXIST)
		திரும்பு -1;

	எனில் ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		திரும்பு -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	எனில் ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		செல் r_err;
	}

	எனில் (!nonblock) {
		முழு flags = fcntl(r, F_GETFL);
		எனில் (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			செல் w_err;
		}
	}

	திரும்பு r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	திரும்பு -1;
}

#endif
