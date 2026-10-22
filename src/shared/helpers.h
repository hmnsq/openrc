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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __HELPERS_H__
#ವ್ಯಾಖ್ಯಾನಿಸು __HELPERS_H__

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

#ವ್ಯಾಖ್ಯಾನಿಸು ERRX ಮಾಡು { fprintf (stderr, "out of memory\n"); ನಿರ್ಗಮಿಸು (1); } ಆಗ (0)

#ವ್ಯಾಖ್ಯಾನಿಸು UNCONST(a)		((ಶೂನ್ಯ *)(uintptr_t)(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *)(a))

#ವ್ಯಾಖ್ಯಾನಿಸು RC_ಬಳಸದ __attribute__((__unused__))
#ವ್ಯಾಖ್ಯಾನಿಸು RC_NORETURN __attribute__((__noreturn__))
#ವ್ಯಾಖ್ಯಾನಿಸು RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#ವ್ಯಾಖ್ಯಾನಿಸು ARRAY_ಗಾತ್ರ(a) (ಗಾತ್ರ(a) / ಗಾತ್ರ((a)[0]))

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ HAVE_STRLCPY
#    define strlcpy(dst, src, ಗಾತ್ರ_2) snprintf(dst, ಗಾತ್ರ_2, "%s", src)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ timespecsub
#ವ್ಯಾಖ್ಯಾನಿಸು	timespecsub(tsp, usp, vsp)					      \
	ಮಾಡು {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		ಆದರೆ ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} ಆಗ (/* CONSTCOND */ 0)
#ಆದರೆ_ಅಂತ್ಯ

RC_ಬಳಸದ ಸ್ಥಿರ ಶೂನ್ಯ *xmalloc (size_t ಗಾತ್ರ_2)
{
	ಶೂನ್ಯ *ಮೌಲ್ಯ = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2);

	ಆದರೆ (ಮೌಲ್ಯ)
		ಹಿಂತಿರುಗಿ (ಮೌಲ್ಯ);

	ERRX;
	/* NOTREACHED */
}

RC_ಬಳಸದ ಸ್ಥಿರ ಶೂನ್ಯ *xrealloc(ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_2)
{
	ಶೂನ್ಯ *ಮೌಲ್ಯ = realloc(ptr, ಗಾತ್ರ_2);

	ಆದರೆ (ಮೌಲ್ಯ)
		ಹಿಂತಿರುಗಿ (ಮೌಲ್ಯ);

	ERRX;
	/* NOTREACHED */
}

RC_ಬಳಸದ ಸ್ಥಿರ ಅಕ್ಷರ *xstrdup(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಅಕ್ಷರ *ಮೌಲ್ಯ;

	ಆದರೆ (!str)
		ಹಿಂತಿರುಗಿ (ಖಾಲಿ);

	ಮೌಲ್ಯ = strdup(str);

	ಆದರೆ (ಮೌಲ್ಯ)
		ಹಿಂತಿರುಗಿ (ಮೌಲ್ಯ);

	ERRX;
	/* NOTREACHED */
}

RC_ಬಳಸದ ಸ್ಥಿರ FILE *xopen_memstream(ಅಕ್ಷರ **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	ಆದರೆ (f)
		ಹಿಂತಿರುಗಿ f;
	ERRX;
}

RC_ಬಳಸದ ಸ್ಥಿರ ಶೂನ್ಯ xclose_memstream(FILE *f)
{
	fflush(f);
	ಆದರೆ (ferror(f) || fclose(f) != 0)
		ERRX;
}

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_ಬಳಸದ ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *basename_c(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *slash = strrchr(ಮಾರ್ಗ, '/');

	ಆದರೆ (slash)
		ಹಿಂತಿರುಗಿ (++slash);
	ಹಿಂತಿರುಗಿ (ಮಾರ್ಗ);
}

RC_ಬಳಸದ ಸ್ಥಿರ FILE *do_fopenat(ಪೂರ್ಣಾಂಕ dirfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pathname, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ)
{
	ಪೂರ್ಣಾಂಕ fd = openat(dirfd, pathname, ಬಹುಲಕ, 0666);
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmode;
	FILE *fp;

	ಆದರೆ (fd == -1)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	ಆದರೆ ((ಬಹುಲಕ & O_RDWR) == O_RDWR)
		fmode = "r+";
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((ಬಹುಲಕ & O_WRONLY) == O_WRONLY)
		fmode = "w";
	ಇಲ್ಲದಿದ್ದರೆ
		fmode = "r";

	ಆದರೆ (!(fp = fdopen(fd, fmode)))
		close(fd);
	ಹಿಂತಿರುಗಿ fp;
}

RC_ಬಳಸದ ಸ್ಥಿರ DIR *do_opendirat(ಪೂರ್ಣಾಂಕ dirfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pathname)
{
	ಪೂರ್ಣಾಂಕ fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	ಆದರೆ (fd == -1)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (!(dp = fdopendir(fd)))
		close(fd);

	ಹಿಂತಿರುಗಿ dp;
}

RC_ಬಳಸದ ಸ್ಥಿರ DIR *do_dopendir(ಪೂರ್ಣಾಂಕ dirfd)
{
	ಪೂರ್ಣಾಂಕ fd = dup(dirfd);
	DIR *dp;

	ಆದರೆ (fd == -1)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (!(dp = fdopendir(fd)))
		close(fd);

	ಹಿಂತಿರುಗಿ dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_ಬಳಸದ RC_PRINTF(2,3) ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xasprintf(ಅಕ್ಷರ **strp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಂಕ len;
	ಪೂರ್ಣಾಂಕ memlen;
	ಅಕ್ಷರ *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	ಆದರೆ (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	ಆದರೆ (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		ಬಿಡುಗಡೆ(ret);
		ನಿರ್ಗಮಿಸು(1);
	}
	*strp = ret;
	ಹಿಂತಿರುಗಿ len;
}

RC_ಬಳಸದ ಸ್ಥಿರ ssize_t xgetline(ಅಕ್ಷರ **ಸೀಮಿತ lineptr, size_t *ಸೀಮಿತ n, FILE *ಸೀಮಿತ ಹರಿವು)
{
	ssize_t ret = getline(lineptr, n, ಹರಿವು);
	ಆದರೆ (ret <= 0)
		ಹಿಂತಿರುಗಿ ret;

	ಆದರೆ ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	ಹಿಂತಿರುಗಿ ret;
}

RC_ಬಳಸದ ಸ್ಥಿರ ಅಕ್ಷರ *env_findvar(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಅಕ್ಷರ **envp)
{
	size_t len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name);

	ಗಾಗಿ (ಅಕ್ಷರ **var = envp; *var; var++)
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(name, *var, len) == 0 && (*var)[len] == '=')
			ಹಿಂತಿರುಗಿ *var;
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

RC_ಬಳಸದ ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xopen_nonblock(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename, ಪೂರ್ಣಾಂಕ flags, mode_t ಬಹುಲಕ)
{
	ಪೂರ್ಣಾಂಕ fd = open(filename, flags | O_NONBLOCK, ಬಹುಲಕ);
	ಆದರೆ (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		ಪೂರ್ಣಾಂಕ saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	ಹಿಂತಿರುಗಿ fd;
}

RC_ಬಳಸದ ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ open_rwfifo(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, bool nonblock)
{
	ಪೂರ್ಣಾಂಕ r, w, err;

	ಆದರೆ (mkfifo(ಮಾರ್ಗ, 0600) == -1 && errno != EEXIST)
		ಹಿಂತಿರುಗಿ -1;

	ಆದರೆ ((r = open(ಮಾರ್ಗ, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		ಹಿಂತಿರುಗಿ -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	ಆದರೆ ((w = open(ಮಾರ್ಗ, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		ಹೋಗು r_err;
	}

	ಆದರೆ (!nonblock) {
		ಪೂರ್ಣಾಂಕ flags = fcntl(r, F_GETFL);
		ಆದರೆ (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			ಹೋಗು w_err;
		}
	}

	ಹಿಂತಿರುಗಿ r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	ಹಿಂತಿರುಗಿ -1;
}

#ಆದರೆ_ಅಂತ್ಯ
