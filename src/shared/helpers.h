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

#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <dirent.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>

#define ERRX କର { fprintf (stderr, "out of memory\n"); exit (1); } ଯେତେବେଳେ (0)

#define UNCONST(a)		((ଶୂନ୍ୟ *)(uintptr_t)(ଅଚଳ ଶୂନ୍ୟ *)(a))

#define RC_UNUSED __attribute__((__unused__))
#define RC_NORETURN __attribute__((__noreturn__))
#define RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#define ARRAY_SIZE(a) (ଆକାର(a) / ଆକାର((a)[0]))

#ifndef HAVE_STRLCPY
#    define strlcpy(dst, src, size) snprintf(dst, size, "%s", src)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					      \
	କର {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		ଯଦି ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} ଯେତେବେଳେ (/* CONSTCOND */ 0)
#endif

RC_UNUSED ସ୍ଥିର ଶୂନ୍ୟ *xmalloc (size_t size)
{
	ଶୂନ୍ୟ *ମୂଲ୍ୟ = malloc(size);

	ଯଦି (ମୂଲ୍ୟ)
		ଫେରନ୍ତୁ (ମୂଲ୍ୟ);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED ସ୍ଥିର ଶୂନ୍ୟ *xrealloc(ଶୂନ୍ୟ *ptr, size_t size)
{
	ଶୂନ୍ୟ *ମୂଲ୍ୟ = realloc(ptr, size);

	ଯଦି (ମୂଲ୍ୟ)
		ଫେରନ୍ତୁ (ମୂଲ୍ୟ);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED ସ୍ଥିର ଅକ୍ଷର *xstrdup(ଅଚଳ ଅକ୍ଷର *str)
{
	ଅକ୍ଷର *ମୂଲ୍ୟ;

	ଯଦି (!str)
		ଫେରନ୍ତୁ (ଖାଲି);

	ମୂଲ୍ୟ = strdup(str);

	ଯଦି (ମୂଲ୍ୟ)
		ଫେରନ୍ତୁ (ମୂଲ୍ୟ);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED ସ୍ଥିର FILE *xopen_memstream(ଅକ୍ଷର **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	ଯଦି (f)
		ଫେରନ୍ତୁ f;
	ERRX;
}

RC_UNUSED ସ୍ଥିର ଶୂନ୍ୟ xclose_memstream(FILE *f)
{
	fflush(f);
	ଯଦି (ferror(f) || fclose(f) != 0)
		ERRX;
}

#undef ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_UNUSED ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *basename_c(ଅଚଳ ଅକ୍ଷର *path)
{
	ଅଚଳ ଅକ୍ଷର *slash = strrchr(path, '/');

	ଯଦି (slash)
		ଫେରନ୍ତୁ (++slash);
	ଫେରନ୍ତୁ (path);
}

RC_UNUSED ସ୍ଥିର FILE *do_fopenat(ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd, ଅଚଳ ଅକ୍ଷର *pathname, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = openat(dirfd, pathname, mode, 0666);
	ଅଚଳ ଅକ୍ଷର *fmode;
	FILE *fp;

	ଯଦି (fd == -1)
		ଫେରନ୍ତୁ ଖାଲି;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	ଯଦି ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	ଅନ୍ୟଥା ଯଦି ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	ଅନ୍ୟଥା
		fmode = "r";

	ଯଦି (!(fp = fdopen(fd, fmode)))
		close(fd);
	ଫେରନ୍ତୁ fp;
}

RC_UNUSED ସ୍ଥିର DIR *do_opendirat(ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd, ଅଚଳ ଅକ୍ଷର *pathname)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	ଯଦି (fd == -1)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (!(dp = fdopendir(fd)))
		close(fd);

	ଫେରନ୍ତୁ dp;
}

RC_UNUSED ସ୍ଥିର DIR *do_dopendir(ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = dup(dirfd);
	DIR *dp;

	ଯଦି (fd == -1)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (!(dp = fdopendir(fd)))
		close(fd);

	ଫେରନ୍ତୁ dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xasprintf(ଅକ୍ଷର **strp, ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	va_list ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା memlen;
	ଅକ୍ଷର *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	ଯଦି (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	ଯଦି (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		ମୁକ୍ତ(ret);
		exit(1);
	}
	*strp = ret;
	ଫେରନ୍ତୁ len;
}

RC_UNUSED ସ୍ଥିର ssize_t xgetline(ଅକ୍ଷର **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	ଯଦି (ret <= 0)
		ଫେରନ୍ତୁ ret;

	ଯଦି ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	ଫେରନ୍ତୁ ret;
}

RC_UNUSED ସ୍ଥିର ଅକ୍ଷର *env_findvar(ଅଚଳ ଅକ୍ଷର *name, ଅକ୍ଷର **envp)
{
	size_t len = strlen(name);

	ପାଇଁ (ଅକ୍ଷର **var = envp; *var; var++)
		ଯଦି (strncmp(name, *var, len) == 0 && (*var)[len] == '=')
			ଫେରନ୍ତୁ *var;
	ଫେରନ୍ତୁ ଖାଲି;
}

RC_UNUSED ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xopen_nonblock(ଅଚଳ ଅକ୍ଷର *filename, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, mode_t mode)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = open(filename, flags | O_NONBLOCK, mode);
	ଯଦି (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	ଫେରନ୍ତୁ fd;
}

RC_UNUSED ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା open_rwfifo(ଅଚଳ ଅକ୍ଷର *path, bool nonblock)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା r, w, err;

	ଯଦି (mkfifo(path, 0600) == -1 && errno != EEXIST)
		ଫେରନ୍ତୁ -1;

	ଯଦି ((r = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		ଫେରନ୍ତୁ -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	ଯଦି ((w = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		ଯାଅ r_err;
	}

	ଯଦି (!nonblock) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = fcntl(r, F_GETFL);
		ଯଦି (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			ଯାଅ w_err;
		}
	}

	ଫେରନ୍ତୁ r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	ଫେରନ୍ତୁ -1;
}

#endif
