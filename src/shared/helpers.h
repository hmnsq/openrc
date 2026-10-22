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

#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <dirent.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <errno.h>

#define ERRX ചെയ്യുക { fprintf (stderr, "out of memory\n"); exit (1); } ആകുമ്പോൾ (0)

#define UNCONST(a)		((ശൂന്യം *)(uintptr_t)(സ്ഥിരം ശൂന്യം *)(a))

#define RC_UNUSED __attribute__((__unused__))
#define RC_NORETURN __attribute__((__noreturn__))
#define RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#define ARRAY_SIZE(a) (വലുപ്പം(a) / വലുപ്പം((a)[0]))

#ifndef HAVE_STRLCPY
#    define strlcpy(dst, src, size) snprintf(dst, size, "%s", src)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					      \
	ചെയ്യുക {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		എങ്കിൽ ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} ആകുമ്പോൾ (/* CONSTCOND */ 0)
#endif

RC_UNUSED സ്ഥിതം ശൂന്യം *xmalloc (size_t size)
{
	ശൂന്യം *മൂല്യം = malloc(size);

	എങ്കിൽ (മൂല്യം)
		തിരികെ (മൂല്യം);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED സ്ഥിതം ശൂന്യം *xrealloc(ശൂന്യം *ptr, size_t size)
{
	ശൂന്യം *മൂല്യം = realloc(ptr, size);

	എങ്കിൽ (മൂല്യം)
		തിരികെ (മൂല്യം);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED സ്ഥിതം അക്ഷരം *xstrdup(സ്ഥിരം അക്ഷരം *str)
{
	അക്ഷരം *മൂല്യം;

	എങ്കിൽ (!str)
		തിരികെ (ശൂന്യ);

	മൂല്യം = strdup(str);

	എങ്കിൽ (മൂല്യം)
		തിരികെ (മൂല്യം);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED സ്ഥിതം FILE *xopen_memstream(അക്ഷരം **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	എങ്കിൽ (f)
		തിരികെ f;
	ERRX;
}

RC_UNUSED സ്ഥിതം ശൂന്യം xclose_memstream(FILE *f)
{
	fflush(f);
	എങ്കിൽ (ferror(f) || fclose(f) != 0)
		ERRX;
}

#undef ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_UNUSED സ്ഥിതം സ്ഥിരം അക്ഷരം *basename_c(സ്ഥിരം അക്ഷരം *path)
{
	സ്ഥിരം അക്ഷരം *slash = strrchr(path, '/');

	എങ്കിൽ (slash)
		തിരികെ (++slash);
	തിരികെ (path);
}

RC_UNUSED സ്ഥിതം FILE *do_fopenat(പൂർണ്ണസംഖ്യ dirfd, സ്ഥിരം അക്ഷരം *pathname, പൂർണ്ണസംഖ്യ mode)
{
	പൂർണ്ണസംഖ്യ fd = openat(dirfd, pathname, mode, 0666);
	സ്ഥിരം അക്ഷരം *fmode;
	FILE *fp;

	എങ്കിൽ (fd == -1)
		തിരികെ ശൂന്യ;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	എങ്കിൽ ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	അല്ലെങ്കിൽ എങ്കിൽ ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	അല്ലെങ്കിൽ
		fmode = "r";

	എങ്കിൽ (!(fp = fdopen(fd, fmode)))
		close(fd);
	തിരികെ fp;
}

RC_UNUSED സ്ഥിതം DIR *do_opendirat(പൂർണ്ണസംഖ്യ dirfd, സ്ഥിരം അക്ഷരം *pathname)
{
	പൂർണ്ണസംഖ്യ fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	എങ്കിൽ (fd == -1)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (!(dp = fdopendir(fd)))
		close(fd);

	തിരികെ dp;
}

RC_UNUSED സ്ഥിതം DIR *do_dopendir(പൂർണ്ണസംഖ്യ dirfd)
{
	പൂർണ്ണസംഖ്യ fd = dup(dirfd);
	DIR *dp;

	എങ്കിൽ (fd == -1)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (!(dp = fdopendir(fd)))
		close(fd);

	തിരികെ dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) സ്ഥിതം പൂർണ്ണസംഖ്യ xasprintf(അക്ഷരം **strp, സ്ഥിരം അക്ഷരം *fmt, ...)
{
	va_list ap;
	പൂർണ്ണസംഖ്യ len;
	പൂർണ്ണസംഖ്യ memlen;
	അക്ഷരം *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	എങ്കിൽ (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	എങ്കിൽ (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		സ്വതന്ത്രം(ret);
		exit(1);
	}
	*strp = ret;
	തിരികെ len;
}

RC_UNUSED സ്ഥിതം ssize_t xgetline(അക്ഷരം **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	എങ്കിൽ (ret <= 0)
		തിരികെ ret;

	എങ്കിൽ ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	തിരികെ ret;
}

RC_UNUSED സ്ഥിതം അക്ഷരം *env_findvar(സ്ഥിരം അക്ഷരം *name, അക്ഷരം **envp)
{
	size_t len = strlen(name);

	വേണ്ടി (അക്ഷരം **var = envp; *var; var++)
		എങ്കിൽ (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			തിരികെ *var;
	തിരികെ ശൂന്യ;
}

RC_UNUSED സ്ഥിതം പൂർണ്ണസംഖ്യ xopen_nonblock(സ്ഥിരം അക്ഷരം *filename, പൂർണ്ണസംഖ്യ flags, mode_t mode)
{
	പൂർണ്ണസംഖ്യ fd = open(filename, flags | O_NONBLOCK, mode);
	എങ്കിൽ (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		പൂർണ്ണസംഖ്യ saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	തിരികെ fd;
}

RC_UNUSED സ്ഥിതം പൂർണ്ണസംഖ്യ open_rwfifo(സ്ഥിരം അക്ഷരം *path, bool nonblock)
{
	പൂർണ്ണസംഖ്യ r, w, err;

	എങ്കിൽ (mkfifo(path, 0600) == -1 && errno != EEXIST)
		തിരികെ -1;

	എങ്കിൽ ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		തിരികെ -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	എങ്കിൽ ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		പോകുക r_err;
	}

	എങ്കിൽ (!nonblock) {
		പൂർണ്ണസംഖ്യ flags = fcntl(r, F_GETFL);
		എങ്കിൽ (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			പോകുക w_err;
		}
	}

	തിരികെ r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	തിരികെ -1;
}

#endif
