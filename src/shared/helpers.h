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

#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <dirent.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <errno.h>

#define ERRX કરો { fprintf (stderr, "out of memory\n"); exit (1); } જ્યારે (0)

#define UNCONST(a)		((શૂન્ય *)(uintptr_t)(અચલ શૂન્ય *)(a))

#define RC_UNUSED __attribute__((__unused__))
#define RC_NORETURN __attribute__((__noreturn__))
#define RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#define ARRAY_SIZE(a) (કદ(a) / કદ((a)[0]))

#ifndef HAVE_STRLCPY
#    define strlcpy(dst, src, size) snprintf(dst, size, "%s", src)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					      \
	કરો {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		જો ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} જ્યારે (/* CONSTCOND */ 0)
#endif

RC_UNUSED સ્થિર શૂન્ય *xmalloc (size_t size)
{
	શૂન્ય *કિંમત = malloc(size);

	જો (કિંમત)
		પાછા (કિંમત);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED સ્થિર શૂન્ય *xrealloc(શૂન્ય *ptr, size_t size)
{
	શૂન્ય *કિંમત = realloc(ptr, size);

	જો (કિંમત)
		પાછા (કિંમત);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED સ્થિર અક્ષર *xstrdup(અચલ અક્ષર *str)
{
	અક્ષર *કિંમત;

	જો (!str)
		પાછા (ખાલી);

	કિંમત = strdup(str);

	જો (કિંમત)
		પાછા (કિંમત);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED સ્થિર FILE *xopen_memstream(અક્ષર **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	જો (f)
		પાછા f;
	ERRX;
}

RC_UNUSED સ્થિર શૂન્ય xclose_memstream(FILE *f)
{
	fflush(f);
	જો (ferror(f) || fclose(f) != 0)
		ERRX;
}

#undef ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_UNUSED સ્થિર અચલ અક્ષર *basename_c(અચલ અક્ષર *path)
{
	અચલ અક્ષર *slash = strrchr(path, '/');

	જો (slash)
		પાછા (++slash);
	પાછા (path);
}

RC_UNUSED સ્થિર FILE *do_fopenat(પૂર્ણાંક dirfd, અચલ અક્ષર *pathname, પૂર્ણાંક mode)
{
	પૂર્ણાંક fd = openat(dirfd, pathname, mode, 0666);
	અચલ અક્ષર *fmode;
	FILE *fp;

	જો (fd == -1)
		પાછા ખાલી;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	જો ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	નહીંતર જો ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	નહીંતર
		fmode = "r";

	જો (!(fp = fdopen(fd, fmode)))
		close(fd);
	પાછા fp;
}

RC_UNUSED સ્થિર DIR *do_opendirat(પૂર્ણાંક dirfd, અચલ અક્ષર *pathname)
{
	પૂર્ણાંક fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	જો (fd == -1)
		પાછા ખાલી;

	જો (!(dp = fdopendir(fd)))
		close(fd);

	પાછા dp;
}

RC_UNUSED સ્થિર DIR *do_dopendir(પૂર્ણાંક dirfd)
{
	પૂર્ણાંક fd = dup(dirfd);
	DIR *dp;

	જો (fd == -1)
		પાછા ખાલી;

	જો (!(dp = fdopendir(fd)))
		close(fd);

	પાછા dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) સ્થિર પૂર્ણાંક xasprintf(અક્ષર **strp, અચલ અક્ષર *fmt, ...)
{
	va_list ap;
	પૂર્ણાંક len;
	પૂર્ણાંક memlen;
	અક્ષર *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	જો (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	જો (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		મુક્ત(ret);
		exit(1);
	}
	*strp = ret;
	પાછા len;
}

RC_UNUSED સ્થિર ssize_t xgetline(અક્ષર **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	જો (ret <= 0)
		પાછા ret;

	જો ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	પાછા ret;
}

RC_UNUSED સ્થિર અક્ષર *env_findvar(અચલ અક્ષર *name, અક્ષર **envp)
{
	size_t len = strlen(name);

	માટે (અક્ષર **var = envp; *var; var++)
		જો (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			પાછા *var;
	પાછા ખાલી;
}

RC_UNUSED સ્થિર પૂર્ણાંક xopen_nonblock(અચલ અક્ષર *filename, પૂર્ણાંક flags, mode_t mode)
{
	પૂર્ણાંક fd = open(filename, flags | O_NONBLOCK, mode);
	જો (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		પૂર્ણાંક saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	પાછા fd;
}

RC_UNUSED સ્થિર પૂર્ણાંક open_rwfifo(અચલ અક્ષર *path, bool nonblock)
{
	પૂર્ણાંક r, w, err;

	જો (mkfifo(path, 0600) == -1 && errno != EEXIST)
		પાછા -1;

	જો ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		પાછા -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	જો ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		જાઓ r_err;
	}

	જો (!nonblock) {
		પૂર્ણાંક flags = fcntl(r, F_GETFL);
		જો (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			જાઓ w_err;
		}
	}

	પાછા r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	પાછા -1;
}

#endif
