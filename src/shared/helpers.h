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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __HELPERS_H__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __HELPERS_H__

#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dirent.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERRX ਕਰੋ { fprintf (stderr, "out of memory\n"); ਬਾਹਰ_ਨਿਕਲੋ (1); } ਜਦੋਂ (0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNCONST(a)		((ਖਾਲੀ *)(uintptr_t)(ਅਚਲ ਖਾਲੀ *)(a))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_ਅਣਵਰਤਿਆ __attribute__((__unused__))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_NORETURN __attribute__((__noreturn__))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ARRAY_ਮਾਪ(a) (ਆਕਾਰ(a) / ਆਕਾਰ((a)[0]))

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_STRLCPY
#    define strlcpy(dst, src, ਮਾਪ) snprintf(dst, ਮਾਪ, "%s", src)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ timespecsub
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	timespecsub(tsp, usp, vsp)					      \
	ਕਰੋ {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		ਜੇ ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} ਜਦੋਂ (/* CONSTCOND */ 0)
#ਜੇ_ਅੰਤ

RC_ਅਣਵਰਤਿਆ ਸਥਿਰ ਖਾਲੀ *xmalloc (size_t ਮਾਪ)
{
	ਖਾਲੀ *ਮੁੱਲ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ);

	ਜੇ (ਮੁੱਲ)
		ਵਾਪਸ (ਮੁੱਲ);

	ERRX;
	/* NOTREACHED */
}

RC_ਅਣਵਰਤਿਆ ਸਥਿਰ ਖਾਲੀ *xrealloc(ਖਾਲੀ *ptr, size_t ਮਾਪ)
{
	ਖਾਲੀ *ਮੁੱਲ = realloc(ptr, ਮਾਪ);

	ਜੇ (ਮੁੱਲ)
		ਵਾਪਸ (ਮੁੱਲ);

	ERRX;
	/* NOTREACHED */
}

RC_ਅਣਵਰਤਿਆ ਸਥਿਰ ਅੱਖਰ *xstrdup(ਅਚਲ ਅੱਖਰ *str)
{
	ਅੱਖਰ *ਮੁੱਲ;

	ਜੇ (!str)
		ਵਾਪਸ (ਨਿਰਾ);

	ਮੁੱਲ = strdup(str);

	ਜੇ (ਮੁੱਲ)
		ਵਾਪਸ (ਮੁੱਲ);

	ERRX;
	/* NOTREACHED */
}

RC_ਅਣਵਰਤਿਆ ਸਥਿਰ FILE *xopen_memstream(ਅੱਖਰ **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	ਜੇ (f)
		ਵਾਪਸ f;
	ERRX;
}

RC_ਅਣਵਰਤਿਆ ਸਥਿਰ ਖਾਲੀ xclose_memstream(FILE *f)
{
	fflush(f);
	ਜੇ (ferror(f) || fclose(f) != 0)
		ERRX;
}

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_ਅਣਵਰਤਿਆ ਸਥਿਰ ਅਚਲ ਅੱਖਰ *basename_c(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਅਚਲ ਅੱਖਰ *slash = strrchr(ਰਸਤਾ, '/');

	ਜੇ (slash)
		ਵਾਪਸ (++slash);
	ਵਾਪਸ (ਰਸਤਾ);
}

RC_ਅਣਵਰਤਿਆ ਸਥਿਰ FILE *do_fopenat(ਪੂਰਨ dirfd, ਅਚਲ ਅੱਖਰ *pathname, ਪੂਰਨ ਬਹੁਲਕ)
{
	ਪੂਰਨ fd = openat(dirfd, pathname, ਬਹੁਲਕ, 0666);
	ਅਚਲ ਅੱਖਰ *fmode;
	FILE *fp;

	ਜੇ (fd == -1)
		ਵਾਪਸ ਨਿਰਾ;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	ਜੇ ((ਬਹੁਲਕ & O_RDWR) == O_RDWR)
		fmode = "r+";
	ਨਹੀਂਤਾਂ ਜੇ ((ਬਹੁਲਕ & O_WRONLY) == O_WRONLY)
		fmode = "w";
	ਨਹੀਂਤਾਂ
		fmode = "r";

	ਜੇ (!(fp = fdopen(fd, fmode)))
		close(fd);
	ਵਾਪਸ fp;
}

RC_ਅਣਵਰਤਿਆ ਸਥਿਰ DIR *do_opendirat(ਪੂਰਨ dirfd, ਅਚਲ ਅੱਖਰ *pathname)
{
	ਪੂਰਨ fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	ਜੇ (fd == -1)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (!(dp = fdopendir(fd)))
		close(fd);

	ਵਾਪਸ dp;
}

RC_ਅਣਵਰਤਿਆ ਸਥਿਰ DIR *do_dopendir(ਪੂਰਨ dirfd)
{
	ਪੂਰਨ fd = dup(dirfd);
	DIR *dp;

	ਜੇ (fd == -1)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (!(dp = fdopendir(fd)))
		close(fd);

	ਵਾਪਸ dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_ਅਣਵਰਤਿਆ RC_PRINTF(2,3) ਸਥਿਰ ਪੂਰਨ xasprintf(ਅੱਖਰ **strp, ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	va_list ap;
	ਪੂਰਨ len;
	ਪੂਰਨ memlen;
	ਅੱਖਰ *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	ਜੇ (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	ਜੇ (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		ਮੁਕਤ(ret);
		ਬਾਹਰ_ਨਿਕਲੋ(1);
	}
	*strp = ret;
	ਵਾਪਸ len;
}

RC_ਅਣਵਰਤਿਆ ਸਥਿਰ ssize_t xgetline(ਅੱਖਰ **ਸੀਮਿਤ lineptr, size_t *ਸੀਮਿਤ n, FILE *ਸੀਮਿਤ ਧਾਰਾ)
{
	ssize_t ret = getline(lineptr, n, ਧਾਰਾ);
	ਜੇ (ret <= 0)
		ਵਾਪਸ ret;

	ਜੇ ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	ਵਾਪਸ ret;
}

RC_ਅਣਵਰਤਿਆ ਸਥਿਰ ਅੱਖਰ *env_findvar(ਅਚਲ ਅੱਖਰ *name, ਅੱਖਰ **envp)
{
	size_t len = ਲੜੀ_ਲੰਬਾਈ(name);

	ਲਈ (ਅੱਖਰ **var = envp; *var; var++)
		ਜੇ (ਲੜੀ_ਤੁਲਨਾ(name, *var, len) == 0 && (*var)[len] == '=')
			ਵਾਪਸ *var;
	ਵਾਪਸ ਨਿਰਾ;
}

RC_ਅਣਵਰਤਿਆ ਸਥਿਰ ਪੂਰਨ xopen_nonblock(ਅਚਲ ਅੱਖਰ *filename, ਪੂਰਨ flags, mode_t ਬਹੁਲਕ)
{
	ਪੂਰਨ fd = open(filename, flags | O_NONBLOCK, ਬਹੁਲਕ);
	ਜੇ (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		ਪੂਰਨ saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	ਵਾਪਸ fd;
}

RC_ਅਣਵਰਤਿਆ ਸਥਿਰ ਪੂਰਨ open_rwfifo(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, bool nonblock)
{
	ਪੂਰਨ r, w, err;

	ਜੇ (mkfifo(ਰਸਤਾ, 0600) == -1 && errno != EEXIST)
		ਵਾਪਸ -1;

	ਜੇ ((r = open(ਰਸਤਾ, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		ਵਾਪਸ -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	ਜੇ ((w = open(ਰਸਤਾ, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		ਜਾਓ r_err;
	}

	ਜੇ (!nonblock) {
		ਪੂਰਨ flags = fcntl(r, F_GETFL);
		ਜੇ (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			ਜਾਓ w_err;
		}
	}

	ਵਾਪਸ r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	ਵਾਪਸ -1;
}

#ਜੇ_ਅੰਤ
