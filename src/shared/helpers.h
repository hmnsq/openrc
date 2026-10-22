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

#جے_بنا_تعریف __HELPERS_H__
#تعریف_کرو __HELPERS_H__

#ضمن <stdarg.h>
#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdint.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/stat.h>
#ضمن <dirent.h>
#ضمن <unistd.h>
#ضمن <fcntl.h>
#ضمن <errno.h>

#تعریف_کرو ERRX افعل { fprintf (stderr, "out of memory\n"); باہر_نکلو (1); } طالما (0)

#تعریف_کرو UNCONST(a)		((فراغ *)(uintptr_t)(ثابت فراغ *)(a))

#تعریف_کرو RC_بن_ورتیا __attribute__((__unused__))
#تعریف_کرو RC_NORETURN __attribute__((__noreturn__))
#تعریف_کرو RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#تعریف_کرو ARRAY_ناپ(a) (حجم(a) / حجم((a)[0]))

#جے_بنا_تعریف HAVE_STRLCPY
#    define strlcpy(dst, src, ناپ) snprintf(dst, ناپ, "%s", src)
#جے_انت

#جے_بنا_تعریف timespecsub
#تعریف_کرو	timespecsub(tsp, usp, vsp)					      \
	افعل {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		إذا ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} طالما (/* CONSTCOND */ 0)
#جے_انت

RC_بن_ورتیا ساكن فراغ *xmalloc (size_t ناپ)
{
	فراغ *قدر = حافظہ_دیو(ناپ);

	إذا (قدر)
		أرجع (قدر);

	ERRX;
	/* NOTREACHED */
}

RC_بن_ورتیا ساكن فراغ *xrealloc(فراغ *ptr, size_t ناپ)
{
	فراغ *قدر = realloc(ptr, ناپ);

	إذا (قدر)
		أرجع (قدر);

	ERRX;
	/* NOTREACHED */
}

RC_بن_ورتیا ساكن حرف *xstrdup(ثابت حرف *str)
{
	حرف *قدر;

	إذا (!str)
		أرجع (عدم);

	قدر = strdup(str);

	إذا (قدر)
		أرجع (قدر);

	ERRX;
	/* NOTREACHED */
}

RC_بن_ورتیا ساكن FILE *xopen_memstream(حرف **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	إذا (f)
		أرجع f;
	ERRX;
}

RC_بن_ورتیا ساكن فراغ xclose_memstream(FILE *f)
{
	fflush(f);
	إذا (ferror(f) || fclose(f) != 0)
		ERRX;
}

#تعریف_رد ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_بن_ورتیا ساكن ثابت حرف *basename_c(ثابت حرف *رستہ)
{
	ثابت حرف *slash = strrchr(رستہ, '/');

	إذا (slash)
		أرجع (++slash);
	أرجع (رستہ);
}

RC_بن_ورتیا ساكن FILE *do_fopenat(صحيح dirfd, ثابت حرف *pathname, صحيح منوال)
{
	صحيح fd = openat(dirfd, pathname, منوال, 0666);
	ثابت حرف *fmode;
	FILE *fp;

	إذا (fd == -1)
		أرجع عدم;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	إذا ((منوال & O_RDWR) == O_RDWR)
		fmode = "r+";
	وإلا إذا ((منوال & O_WRONLY) == O_WRONLY)
		fmode = "w";
	وإلا
		fmode = "r";

	إذا (!(fp = fdopen(fd, fmode)))
		close(fd);
	أرجع fp;
}

RC_بن_ورتیا ساكن DIR *do_opendirat(صحيح dirfd, ثابت حرف *pathname)
{
	صحيح fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	إذا (fd == -1)
		أرجع عدم;

	إذا (!(dp = fdopendir(fd)))
		close(fd);

	أرجع dp;
}

RC_بن_ورتیا ساكن DIR *do_dopendir(صحيح dirfd)
{
	صحيح fd = dup(dirfd);
	DIR *dp;

	إذا (fd == -1)
		أرجع عدم;

	إذا (!(dp = fdopendir(fd)))
		close(fd);

	أرجع dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_بن_ورتیا RC_PRINTF(2,3) ساكن صحيح xasprintf(حرف **strp, ثابت حرف *fmt, ...)
{
	va_list ap;
	صحيح len;
	صحيح memlen;
	حرف *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	إذا (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	إذا (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		حرر(ret);
		باہر_نکلو(1);
	}
	*strp = ret;
	أرجع len;
}

RC_بن_ورتیا ساكن ssize_t xgetline(حرف **محدود lineptr, size_t *محدود n, FILE *محدود دھارا)
{
	ssize_t ret = getline(lineptr, n, دھارا);
	إذا (ret <= 0)
		أرجع ret;

	إذا ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	أرجع ret;
}

RC_بن_ورتیا ساكن حرف *env_findvar(ثابت حرف *name, حرف **envp)
{
	size_t len = لڑی_لمبائی(name);

	لكل (حرف **var = envp; *var; var++)
		إذا (لڑی_مقابلہ(name, *var, len) == 0 && (*var)[len] == '=')
			أرجع *var;
	أرجع عدم;
}

RC_بن_ورتیا ساكن صحيح xopen_nonblock(ثابت حرف *filename, صحيح flags, mode_t منوال)
{
	صحيح fd = open(filename, flags | O_NONBLOCK, منوال);
	إذا (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		صحيح saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	أرجع fd;
}

RC_بن_ورتیا ساكن صحيح open_rwfifo(ثابت حرف *رستہ, bool nonblock)
{
	صحيح r, w, err;

	إذا (mkfifo(رستہ, 0600) == -1 && errno != EEXIST)
		أرجع -1;

	إذا ((r = open(رستہ, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		أرجع -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	إذا ((w = open(رستہ, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		اذهب r_err;
	}

	إذا (!nonblock) {
		صحيح flags = fcntl(r, F_GETFL);
		إذا (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			اذهب w_err;
		}
	}

	أرجع r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	أرجع -1;
}

#جے_انت
