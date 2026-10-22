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

#جيڪڏهن_وصف_نه_ٿيل __HELPERS_H__
#وصف_ڪريو __HELPERS_H__

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

#وصف_ڪريو ERRX افعل { fprintf (stderr, "out of memory\n"); ٻاهر_نڪرو (1); } طالما (0)

#وصف_ڪريو UNCONST(a)		((فراغ *)(uintptr_t)(ثابت فراغ *)(a))

#وصف_ڪريو RC_اڻ_استعمال_ٿيل __attribute__((__unused__))
#وصف_ڪريو RC_NORETURN __attribute__((__noreturn__))
#وصف_ڪريو RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#وصف_ڪريو ARRAY_سائيز(a) (حجم(a) / حجم((a)[0]))

#جيڪڏهن_وصف_نه_ٿيل HAVE_STRLCPY
#    define strlcpy(dst, src, سائيز) snprintf(dst, سائيز, "%s", src)
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل timespecsub
#وصف_ڪريو	timespecsub(tsp, usp, vsp)					      \
	افعل {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		إذا ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} طالما (/* CONSTCOND */ 0)
#جيڪڏهن_پورو

RC_اڻ_استعمال_ٿيل ساكن فراغ *xmalloc (size_t سائيز)
{
	فراغ *قدر = يادگيري_ڏيو(سائيز);

	إذا (قدر)
		أرجع (قدر);

	ERRX;
	/* NOTREACHED */
}

RC_اڻ_استعمال_ٿيل ساكن فراغ *xrealloc(فراغ *ptr, size_t سائيز)
{
	فراغ *قدر = realloc(ptr, سائيز);

	إذا (قدر)
		أرجع (قدر);

	ERRX;
	/* NOTREACHED */
}

RC_اڻ_استعمال_ٿيل ساكن حرف *xstrdup(ثابت حرف *str)
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

RC_اڻ_استعمال_ٿيل ساكن FILE *xopen_memstream(حرف **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	إذا (f)
		أرجع f;
	ERRX;
}

RC_اڻ_استعمال_ٿيل ساكن فراغ xclose_memstream(FILE *f)
{
	fflush(f);
	إذا (ferror(f) || fclose(f) != 0)
		ERRX;
}

#وصف_رد ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_اڻ_استعمال_ٿيل ساكن ثابت حرف *basename_c(ثابت حرف *رستو)
{
	ثابت حرف *slash = strrchr(رستو, '/');

	إذا (slash)
		أرجع (++slash);
	أرجع (رستو);
}

RC_اڻ_استعمال_ٿيل ساكن FILE *do_fopenat(صحيح dirfd, ثابت حرف *pathname, صحيح گهڻائي)
{
	صحيح fd = openat(dirfd, pathname, گهڻائي, 0666);
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
	إذا ((گهڻائي & O_RDWR) == O_RDWR)
		fmode = "r+";
	وإلا إذا ((گهڻائي & O_WRONLY) == O_WRONLY)
		fmode = "w";
	وإلا
		fmode = "r";

	إذا (!(fp = fdopen(fd, fmode)))
		close(fd);
	أرجع fp;
}

RC_اڻ_استعمال_ٿيل ساكن DIR *do_opendirat(صحيح dirfd, ثابت حرف *pathname)
{
	صحيح fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	إذا (fd == -1)
		أرجع عدم;

	إذا (!(dp = fdopendir(fd)))
		close(fd);

	أرجع dp;
}

RC_اڻ_استعمال_ٿيل ساكن DIR *do_dopendir(صحيح dirfd)
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
RC_اڻ_استعمال_ٿيل RC_PRINTF(2,3) ساكن صحيح xasprintf(حرف **strp, ثابت حرف *fmt, ...)
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
		ٻاهر_نڪرو(1);
	}
	*strp = ret;
	أرجع len;
}

RC_اڻ_استعمال_ٿيل ساكن ssize_t xgetline(حرف **محدود lineptr, size_t *محدود n, FILE *محدود وهڪرو)
{
	ssize_t ret = getline(lineptr, n, وهڪرو);
	إذا (ret <= 0)
		أرجع ret;

	إذا ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	أرجع ret;
}

RC_اڻ_استعمال_ٿيل ساكن حرف *env_findvar(ثابت حرف *name, حرف **envp)
{
	size_t len = لڙي_ڊيگهه(name);

	لكل (حرف **var = envp; *var; var++)
		إذا (لڙي_ڀيٽ(name, *var, len) == 0 && (*var)[len] == '=')
			أرجع *var;
	أرجع عدم;
}

RC_اڻ_استعمال_ٿيل ساكن صحيح xopen_nonblock(ثابت حرف *filename, صحيح flags, mode_t گهڻائي)
{
	صحيح fd = open(filename, flags | O_NONBLOCK, گهڻائي);
	إذا (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		صحيح saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	أرجع fd;
}

RC_اڻ_استعمال_ٿيل ساكن صحيح open_rwfifo(ثابت حرف *رستو, bool nonblock)
{
	صحيح r, w, err;

	إذا (mkfifo(رستو, 0600) == -1 && errno != EEXIST)
		أرجع -1;

	إذا ((r = open(رستو, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		أرجع -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	إذا ((w = open(رستو, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
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

#جيڪڏهن_پورو
