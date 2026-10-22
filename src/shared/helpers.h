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

#నిర్వచితం_కాకపోతే __HELPERS_H__
#నిర్వచించు __HELPERS_H__

#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <dirent.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <errno.h>

#నిర్వచించు ERRX చేయి { fprintf (stderr, "out of memory\n"); నిష్క్రమించు (1); } అయితే (0)

#నిర్వచించు UNCONST(a)		((శూన్య *)(uintptr_t)(స్థిరరాశి శూన్య *)(a))

#నిర్వచించు RC_వాడని __attribute__((__unused__))
#నిర్వచించు RC_NORETURN __attribute__((__noreturn__))
#నిర్వచించు RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#నిర్వచించు ARRAY_కొలత(a) (పరిమాణం(a) / పరిమాణం((a)[0]))

#నిర్వచితం_కాకపోతే HAVE_STRLCPY
#    define strlcpy(dst, src, కొలత) snprintf(dst, కొలత, "%s", src)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే timespecsub
#నిర్వచించు	timespecsub(tsp, usp, vsp)					      \
	చేయి {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		ఒకవేళ ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} అయితే (/* CONSTCOND */ 0)
#అయితే_ముగింపు

RC_వాడని స్థిర శూన్య *xmalloc (size_t కొలత)
{
	శూన్య *విలువ = జ్ఞాపకం_ఇవ్వు(కొలత);

	ఒకవేళ (విలువ)
		తిరిగి (విలువ);

	ERRX;
	/* NOTREACHED */
}

RC_వాడని స్థిర శూన్య *xrealloc(శూన్య *ptr, size_t కొలత)
{
	శూన్య *విలువ = realloc(ptr, కొలత);

	ఒకవేళ (విలువ)
		తిరిగి (విలువ);

	ERRX;
	/* NOTREACHED */
}

RC_వాడని స్థిర అక్షరం *xstrdup(స్థిరరాశి అక్షరం *str)
{
	అక్షరం *విలువ;

	ఒకవేళ (!str)
		తిరిగి (ఖాళీ);

	విలువ = strdup(str);

	ఒకవేళ (విలువ)
		తిరిగి (విలువ);

	ERRX;
	/* NOTREACHED */
}

RC_వాడని స్థిర FILE *xopen_memstream(అక్షరం **ptr, size_t *sizeloc)
{
	FILE *f = open_memstream(ptr, sizeloc);
	ఒకవేళ (f)
		తిరిగి f;
	ERRX;
}

RC_వాడని స్థిర శూన్య xclose_memstream(FILE *f)
{
	fflush(f);
	ఒకవేళ (ferror(f) || fclose(f) != 0)
		ERRX;
}

#నిర్వచనం_రద్దు ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_వాడని స్థిర స్థిరరాశి అక్షరం *basename_c(స్థిరరాశి అక్షరం *మార్గం)
{
	స్థిరరాశి అక్షరం *slash = strrchr(మార్గం, '/');

	ఒకవేళ (slash)
		తిరిగి (++slash);
	తిరిగి (మార్గం);
}

RC_వాడని స్థిర FILE *do_fopenat(పూర్ణాంకం dirfd, స్థిరరాశి అక్షరం *pathname, పూర్ణాంకం బహుళకం)
{
	పూర్ణాంకం fd = openat(dirfd, pathname, బహుళకం, 0666);
	స్థిరరాశి అక్షరం *fmode;
	FILE *fp;

	ఒకవేళ (fd == -1)
		తిరిగి ఖాళీ;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	ఒకవేళ ((బహుళకం & O_RDWR) == O_RDWR)
		fmode = "r+";
	లేకపోతే ఒకవేళ ((బహుళకం & O_WRONLY) == O_WRONLY)
		fmode = "w";
	లేకపోతే
		fmode = "r";

	ఒకవేళ (!(fp = fdopen(fd, fmode)))
		close(fd);
	తిరిగి fp;
}

RC_వాడని స్థిర DIR *do_opendirat(పూర్ణాంకం dirfd, స్థిరరాశి అక్షరం *pathname)
{
	పూర్ణాంకం fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	DIR *dp;

	ఒకవేళ (fd == -1)
		తిరిగి ఖాళీ;

	ఒకవేళ (!(dp = fdopendir(fd)))
		close(fd);

	తిరిగి dp;
}

RC_వాడని స్థిర DIR *do_dopendir(పూర్ణాంకం dirfd)
{
	పూర్ణాంకం fd = dup(dirfd);
	DIR *dp;

	ఒకవేళ (fd == -1)
		తిరిగి ఖాళీ;

	ఒకవేళ (!(dp = fdopendir(fd)))
		close(fd);

	తిరిగి dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_వాడని RC_PRINTF(2,3) స్థిర పూర్ణాంకం xasprintf(అక్షరం **strp, స్థిరరాశి అక్షరం *fmt, ...)
{
	va_list ap;
	పూర్ణాంకం len;
	పూర్ణాంకం memlen;
	అక్షరం *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	va_start(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	va_end(ap);
	ఒకవేళ (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		va_start(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		va_end(ap);
	}
	ఒకవేళ (len < 0 || len >= memlen) {
		/* Give up! */
		fprintf(stderr, "xasprintf: unable to format a buffer\n");
		విడుదల(ret);
		నిష్క్రమించు(1);
	}
	*strp = ret;
	తిరిగి len;
}

RC_వాడని స్థిర ssize_t xgetline(అక్షరం **పరిమిత lineptr, size_t *పరిమిత n, FILE *పరిమిత ప్రవాహం)
{
	ssize_t ret = getline(lineptr, n, ప్రవాహం);
	ఒకవేళ (ret <= 0)
		తిరిగి ret;

	ఒకవేళ ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	తిరిగి ret;
}

RC_వాడని స్థిర అక్షరం *env_findvar(స్థిరరాశి అక్షరం *name, అక్షరం **envp)
{
	size_t len = అక్షరశ్రేణి_పొడవు(name);

	కొరకు (అక్షరం **var = envp; *var; var++)
		ఒకవేళ (అక్షరశ్రేణి_పోలిక(name, *var, len) == 0 && (*var)[len] == '=')
			తిరిగి *var;
	తిరిగి ఖాళీ;
}

RC_వాడని స్థిర పూర్ణాంకం xopen_nonblock(స్థిరరాశి అక్షరం *filename, పూర్ణాంకం flags, mode_t బహుళకం)
{
	పూర్ణాంకం fd = open(filename, flags | O_NONBLOCK, బహుళకం);
	ఒకవేళ (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		పూర్ణాంకం saved_errno = errno;
		close(fd);
		fd = -1;
		errno = saved_errno;
	}
	తిరిగి fd;
}

RC_వాడని స్థిర పూర్ణాంకం open_rwfifo(స్థిరరాశి అక్షరం *మార్గం, bool nonblock)
{
	పూర్ణాంకం r, w, err;

	ఒకవేళ (mkfifo(మార్గం, 0600) == -1 && errno != EEXIST)
		తిరిగి -1;

	ఒకవేళ ((r = open(మార్గం, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		తిరిగి -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	ఒకవేళ ((w = open(మార్గం, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = errno;
		వెళ్ళు r_err;
	}

	ఒకవేళ (!nonblock) {
		పూర్ణాంకం flags = fcntl(r, F_GETFL);
		ఒకవేళ (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = errno;
			వెళ్ళు w_err;
		}
	}

	తిరిగి r;

w_err:
	close(w);
r_err:
	close(r);
	errno = err;
	తిరిగి -1;
}

#అయితే_ముగింపు
