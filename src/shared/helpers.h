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

#اگر_غیر_تعریف_شدہ __HELPERS_H__
#تعریف_کرو __HELPERS_H__

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

#تعریف_کرو ERRX کرو { fprintf (stderr, "out of memory\n"); باہر_نکلو (1); } جب (0)

#تعریف_کرو UNCONST(a)		((خالی *)(uintptr_t)(ثابت خالی *)(a))

#تعریف_کرو RC_غیر_مستعمل __attribute__((__unused__))
#تعریف_کرو RC_NORETURN __attribute__((__noreturn__))
#تعریف_کرو RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#تعریف_کرو ARRAY_سائز(a) (حجم(a) / حجم((a)[0]))

#اگر_غیر_تعریف_شدہ HAVE_STRLCPY
#    define strlcpy(dst, src, سائز) snprintf(dst, سائز, "%s", src)
#اگر_ختم

#اگر_غیر_تعریف_شدہ timespecsub
#تعریف_کرو	timespecsub(tsp, usp, vsp)					      \
	کرو {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		اگر ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} جب (/* CONSTCOND */ 0)
#اگر_ختم

RC_غیر_مستعمل ساکن خالی *xmalloc (size_t سائز)
{
	خالی *قدر = حافظہ_دو(سائز);

	اگر (قدر)
		واپس (قدر);

	ERRX;
	/* NOTREACHED */
}

RC_غیر_مستعمل ساکن خالی *xrealloc(خالی *ptr, size_t سائز)
{
	خالی *قدر = realloc(ptr, سائز);

	اگر (قدر)
		واپس (قدر);

	ERRX;
	/* NOTREACHED */
}

RC_غیر_مستعمل ساکن حرف *xstrdup(ثابت حرف *str)
{
	حرف *قدر;

	اگر (!str)
		واپس (تہی);

	قدر = strdup(str);

	اگر (قدر)
		واپس (قدر);

	ERRX;
	/* NOTREACHED */
}

RC_غیر_مستعمل ساکن FILE *xopen_memstream(حرف **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	اگر (f)
		واپس f;
	ERRX;
}

RC_غیر_مستعمل ساکن خالی xclose_memstream(FILE *f)
{
	fflush(f);
	اگر (ferror(f) || fclose(f) != 0)
		ERRX;
}

#تعریف_منسوخ ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_غیر_مستعمل ساکن ثابت حرف *basename_c(ثابت حرف *راستہ)
{
	ثابت حرف *slash = strrchr(راستہ, '/');

	اگر (slash)
		واپس (++slash);
	واپس (راستہ);
}

RC_غیر_مستعمل ساکن FILE *do_fopenat(صحیح dirfd, ثابت حرف *pathname, صحیح منوال)
{
	صحیح fd = openat(dirfd, pathname, منوال, 0666);
	ثابت حرف *fmode;
	FILE *fp;

	اگر (fd == -1)
		واپس تہی;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	اگر ((منوال & O_RDWR) == O_RDWR)
		fmode = "r+";
	ورنہ اگر ((منوال & O_WRONLY) == O_WRONLY)
		fmode = "w";
	ورنہ
		fmode = "r";

	اگر (!(fp = fdopen(fd, fmode)))
		close(fd);
	واپس fp;
}

RC_غیر_مستعمل ساکن DIR *do_opendirat(صحیح dirfd, ثابت حرف *pathname)
{
	صحیح fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	اگر (fd == -1)
		واپس تہی;

	اگر (!(dp = fdopendir(fd)))
		close(fd);

	واپس dp;
}

RC_غیر_مستعمل ساکن DIR *do_dopendir(صحیح dirfd)
{
	صحیح fd = dup(dirfd);
	DIR *dp;

	اگر (fd == -1)
		واپس تہی;

	اگر (!(dp = fdopendir(fd)))
		close(fd);

	واپس dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_غیر_مستعمل RC_PRINTF(2,3) ساکن صحیح xasprintf(حرف **strp, ثابت حرف *fmt, ...)
{
	va_list ap;
	صحیح len;
	صحیح memlen;
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
		باہر_نکلو(1);
	}
	*strp = ret;
	واپس len;
}

RC_غیر_مستعمل ساکن ssize_t xgetline(حرف **محدود lineptr, size_t *محدود n, FILE *محدود دھارا)
{
	ssize_t ret = getline(lineptr, n, دھارا);
	اگر (ret <= 0)
		واپس ret;

	اگر ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	واپس ret;
}

RC_غیر_مستعمل ساکن حرف *env_findvar(ثابت حرف *name, حرف **envp)
{
	size_t len = لڑی_لمبائی(name);

	برائے (حرف **var = envp; *var; var++)
		اگر (لڑی_موازنہ(name, *var, len) == 0 && (*var)[len] == '=')
			واپس *var;
	واپس تہی;
}

RC_غیر_مستعمل ساکن صحیح xopen_nonblock(ثابت حرف *filename, صحیح flags, mode_t منوال)
{
	صحیح fd = open(filename, flags | O_NONBLOCK, منوال);
	اگر (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		صحیح saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	واپس fd;
}

RC_غیر_مستعمل ساکن صحیح open_rwfifo(ثابت حرف *راستہ, bool nonblock)
{
	صحیح r, w, err;

	اگر (mkfifo(راستہ, 0600) == -1 && errno != EEXIST)
		واپس -1;

	اگر ((r = open(راستہ, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		واپس -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	اگر ((w = open(راستہ, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		جاؤ r_err;
	}

	اگر (!nonblock) {
		صحیح flags = fcntl(r, F_GETFL);
		اگر (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			جاؤ w_err;
		}
	}

	واپس r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	واپس -1;
}

#اگر_ختم
