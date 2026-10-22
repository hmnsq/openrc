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

#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <dirent.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

#define ERRX ಕುರು { fprintf (stderr, "out of memory\n"); exit (1); } ಯಾವತ್ (0)

#define UNCONST(a)		((ಶೂನ್ಯ *)(uintptr_t)(ಧ್ರುವ ಶೂನ್ಯ *)(a))

#define RC_UNUSED __attribute__((__unused__))
#define RC_NORETURN __attribute__((__noreturn__))
#define RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#define ARRAY_SIZE(a) (ಪರಿಮಾಣ(a) / ಪರಿಮಾಣ((a)[0]))

#ifndef HAVE_STRLCPY
#    define strlcpy(dst, src, size) snprintf(dst, size, "%s", src)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					      \
	ಕುರು {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		ಯದಿ ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} ಯಾವತ್ (/* CONSTCOND */ 0)
#endif

RC_UNUSED ಸ್ಥಿರ ಶೂನ್ಯ *xmalloc (size_t size)
{
	ಶೂನ್ಯ *ಮೌಲ್ಯ = malloc(size);

	ಯದಿ (ಮೌಲ್ಯ)
		ಪ್ರತ್ಯಾಗಮ (ಮೌಲ್ಯ);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED ಸ್ಥಿರ ಶೂನ್ಯ *xrealloc(ಶೂನ್ಯ *ptr, size_t size)
{
	ಶೂನ್ಯ *ಮೌಲ್ಯ = realloc(ptr, size);

	ಯದಿ (ಮೌಲ್ಯ)
		ಪ್ರತ್ಯಾಗಮ (ಮೌಲ್ಯ);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED ಸ್ಥಿರ ವರ್ಣ *xstrdup(ಧ್ರುವ ವರ್ಣ *str)
{
	ವರ್ಣ *ಮೌಲ್ಯ;

	ಯದಿ (!str)
		ಪ್ರತ್ಯಾಗಮ (ರಿಕ್ತ);

	ಮೌಲ್ಯ = strdup(str);

	ಯದಿ (ಮೌಲ್ಯ)
		ಪ್ರತ್ಯಾಗಮ (ಮೌಲ್ಯ);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED ಸ್ಥಿರ FILE *xopen_memstream(ವರ್ಣ **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	ಯದಿ (f)
		ಪ್ರತ್ಯಾಗಮ f;
	ERRX;
}

RC_UNUSED ಸ್ಥಿರ ಶೂನ್ಯ xclose_memstream(FILE *f)
{
	fflush(f);
	ಯದಿ (ferror(f) || fclose(f) != 0)
		ERRX;
}

#undef ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_UNUSED ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *basename_c(ಧ್ರುವ ವರ್ಣ *path)
{
	ಧ್ರುವ ವರ್ಣ *slash = strrchr(path, '/');

	ಯದಿ (slash)
		ಪ್ರತ್ಯಾಗಮ (++slash);
	ಪ್ರತ್ಯಾಗಮ (path);
}

RC_UNUSED ಸ್ಥಿರ FILE *do_fopenat(ಪೂರ್ಣಾಙ್ಕ dirfd, ಧ್ರುವ ವರ್ಣ *pathname, ಪೂರ್ಣಾಙ್ಕ mode)
{
	ಪೂರ್ಣಾಙ್ಕ fd = openat(dirfd, pathname, mode, 0666);
	ಧ್ರುವ ವರ್ಣ *fmode;
	FILE *fp;

	ಯದಿ (fd == -1)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	ಯದಿ ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	ಅನ್ಯಥಾ ಯದಿ ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	ಅನ್ಯಥಾ
		fmode = "r";

	ಯದಿ (!(fp = fdopen(fd, fmode)))
		close(fd);
	ಪ್ರತ್ಯಾಗಮ fp;
}

RC_UNUSED ಸ್ಥಿರ DIR *do_opendirat(ಪೂರ್ಣಾಙ್ಕ dirfd, ಧ್ರುವ ವರ್ಣ *pathname)
{
	ಪೂರ್ಣಾಙ್ಕ fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	ಯದಿ (fd == -1)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (!(dp = fdopendir(fd)))
		close(fd);

	ಪ್ರತ್ಯಾಗಮ dp;
}

RC_UNUSED ಸ್ಥಿರ DIR *do_dopendir(ಪೂರ್ಣಾಙ್ಕ dirfd)
{
	ಪೂರ್ಣಾಙ್ಕ fd = dup(dirfd);
	DIR *dp;

	ಯದಿ (fd == -1)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (!(dp = fdopendir(fd)))
		close(fd);

	ಪ್ರತ್ಯಾಗಮ dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ xasprintf(ವರ್ಣ **strp, ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ len;
	ಪೂರ್ಣಾಙ್ಕ memlen;
	ವರ್ಣ *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	ಯದಿ (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	ಯದಿ (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		ಮುಕ್ತ(ret);
		exit(1);
	}
	*strp = ret;
	ಪ್ರತ್ಯಾಗಮ len;
}

RC_UNUSED ಸ್ಥಿರ ssize_t xgetline(ವರ್ಣ **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	ಯದಿ (ret <= 0)
		ಪ್ರತ್ಯಾಗಮ ret;

	ಯದಿ ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	ಪ್ರತ್ಯಾಗಮ ret;
}

RC_UNUSED ಸ್ಥಿರ ವರ್ಣ *env_findvar(ಧ್ರುವ ವರ್ಣ *name, ವರ್ಣ **envp)
{
	size_t len = strlen(name);

	ಪ್ರತಿ (ವರ್ಣ **var = envp; *var; var++)
		ಯದಿ (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			ಪ್ರತ್ಯಾಗಮ *var;
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

RC_UNUSED ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ xopen_nonblock(ಧ್ರುವ ವರ್ಣ *filename, ಪೂರ್ಣಾಙ್ಕ flags, mode_t mode)
{
	ಪೂರ್ಣಾಙ್ಕ fd = open(filename, flags | O_NONBLOCK, mode);
	ಯದಿ (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		ಪೂರ್ಣಾಙ್ಕ saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	ಪ್ರತ್ಯಾಗಮ fd;
}

RC_UNUSED ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ open_rwfifo(ಧ್ರುವ ವರ್ಣ *path, bool nonblock)
{
	ಪೂರ್ಣಾಙ್ಕ r, w, err;

	ಯದಿ (mkfifo(path, 0600) == -1 && errno != EEXIST)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಯದಿ ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		ಪ್ರತ್ಯಾಗಮ -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	ಯದಿ ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		ಗಚ್ಛ r_err;
	}

	ಯದಿ (!nonblock) {
		ಪೂರ್ಣಾಙ್ಕ flags = fcntl(r, F_GETFL);
		ಯದಿ (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			ಗಚ್ಛ w_err;
		}
	}

	ಪ್ರತ್ಯಾಗಮ r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	ಪ್ರತ್ಯಾಗಮ -1;
}

#endif
