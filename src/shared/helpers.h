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

#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <dirent.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <errno.h>

#define ERRX गर { ख_लिखो (मानक_त्रुटि, "out of memory\n"); निकास (1); } जबसम्म (0)

#define UNCONST(a)		((शून्य *)(uintptr_t)(अचर शून्य *)(a))

#define RC_UNUSED __attribute__((__unused__))
#define RC_NORETURN __attribute__((__noreturn__))
#define RC_PRINTF(a, b)  __attribute__((__format__(__printf__, a, b)))

#define ARRAY_SIZE(a) (परिमाण(a) / परिमाण((a)[0]))

#ifndef HAVE_STRLCPY
#    define strlcpy(dst, src, size) snprintf(dst, size, "%s", src)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					      \
	गर {								      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		      \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	      \
		यदि ((vsp)->tv_nsec < 0) {				      \
			(vsp)->tv_sec--;				      \
			(vsp)->tv_nsec += 1000000000L;			      \
		}							      \
	} जबसम्म (/* CONSTCOND */ 0)
#endif

RC_UNUSED स्थिर शून्य *xmalloc (माप_प्रकार size)
{
	शून्य *मान = दो_स्मृति(size);

	यदि (मान)
		फर्काऊ (मान);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED स्थिर शून्य *xrealloc(शून्य *ptr, माप_प्रकार size)
{
	शून्य *मान = पुनः_स्मृति(ptr, size);

	यदि (मान)
		फर्काऊ (मान);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED स्थिर वर्ण *xstrdup(अचर वर्ण *str)
{
	वर्ण *मान;

	यदि (!str)
		फर्काऊ (रिक्त);

	मान = strdup(str);

	यदि (मान)
		फर्काऊ (मान);

	ERRX;
	/* NOTREACHED */
}

RC_UNUSED स्थिर खाता *xopen_memstream(वर्ण **ptr, माप_प्रकार *sizeloc)
{
	खाता *f = open_memstream(ptr, sizeloc);
	यदि (f)
		फर्काऊ f;
	ERRX;
}

RC_UNUSED स्थिर शून्य xclose_memstream(खाता *f)
{
	ख_साफ(f);
	यदि (ख_त्रुटि(f) || ख_बंद(f) != 0)
		ERRX;
}

#undef ERRX

/*
 * basename_c never modifies the argument. As such, if there is a trailing
 * slash then an empty string is returned.
 */
RC_UNUSED स्थिर अचर वर्ण *basename_c(अचर वर्ण *path)
{
	अचर वर्ण *slash = म_खोजप(path, '/');

	यदि (slash)
		फर्काऊ (++slash);
	फर्काऊ (path);
}

RC_UNUSED स्थिर खाता *do_fopenat(पूर्णाङ्क dirfd, अचर वर्ण *pathname, पूर्णाङ्क mode)
{
	पूर्णाङ्क fd = openat(dirfd, pathname, mode, 0666);
	अचर वर्ण *fmode;
	खाता *fp;

	यदि (fd == -1)
		फर्काऊ रिक्त;

	/* O_CREAT and O_TRUNC in modes 'a', 'w+', 'a+' don't
	 * really matter after the file has been opened.
	 *
	 * Some implementations have O_RDONLY defined to zero,
	 * thus making it impossible to detect, so, we default
	 * to "r" if no other mask fully matches. */
	यदि ((mode & O_RDWR) == O_RDWR)
		fmode = "r+";
	अन्यथा यदि ((mode & O_WRONLY) == O_WRONLY)
		fmode = "w";
	अन्यथा
		fmode = "r";

	यदि (!(fp = fdopen(fd, fmode)))
		बंद(fd);
	फर्काऊ fp;
}

RC_UNUSED स्थिर सूची *do_opendirat(पूर्णाङ्क dirfd, अचर वर्ण *pathname)
{
	पूर्णाङ्क fd = openat(dirfd, pathname, O_RDONLY | O_DIRECTORY);
	सूची *dp;

	यदि (fd == -1)
		फर्काऊ रिक्त;

	यदि (!(dp = fdopendir(fd)))
		बंद(fd);

	फर्काऊ dp;
}

RC_UNUSED स्थिर सूची *do_dopendir(पूर्णाङ्क dirfd)
{
	पूर्णाङ्क fd = dup(dirfd);
	सूची *dp;

	यदि (fd == -1)
		फर्काऊ रिक्त;

	यदि (!(dp = fdopendir(fd)))
		बंद(fd);

	फर्काऊ dp;
}

/*
 * This is an OpenRC specific version of the asprintf() function.
 * We do this to avoid defining the _GNU_SOURCE feature test macro on
 * glibc systems and to ensure that we have a consistent function across
 * platforms. This also allows us to call our xmalloc and xrealloc
 * functions to handle memory allocation.
 * this function was originally written by Mike Frysinger.
 */
RC_UNUSED RC_PRINTF(2,3) स्थिर पूर्णाङ्क xasprintf(वर्ण **strp, अचर वर्ण *fmt, ...)
{
	बहु_सूची ap;
	पूर्णाङ्क len;
	पूर्णाङ्क memlen;
	वर्ण *ret;

	/*
	 * Start with a buffer size that should cover the vast majority of uses
	 * (path construction).
	 */
	memlen = 4096;
	ret = xmalloc(memlen);

	बहु_शुरू(ap, fmt);
	len = vsnprintf(ret, memlen, fmt, ap);
	बहु_पूर्ण(ap);
	यदि (len >= memlen) {
		/*
		 * Output was truncated, so increase buffer to exactly what we need.
		 */
		memlen = len + 1;
		ret = xrealloc(ret, memlen);
		बहु_शुरू(ap, fmt);
		len = vsnprintf(ret, len + 1, fmt, ap);
		बहु_पूर्ण(ap);
	}
	यदि (len < 0 || len >= memlen) {
		/* Give up! */
		ख_लिखो(मानक_त्रुटि, "xasprintf: unable to format a buffer\n");
		मुक्त(ret);
		निकास(1);
	}
	*strp = ret;
	फर्काऊ len;
}

RC_UNUSED स्थिर ssize_t xgetline(वर्ण **restrict lineptr, माप_प्रकार *restrict n, खाता *restrict stream)
{
	ssize_t ret = getline(lineptr, n, stream);
	यदि (ret <= 0)
		फर्काऊ ret;

	यदि ((*lineptr)[ret - 1] == '\n')
		(*lineptr)[--ret] = '\0';

	फर्काऊ ret;
}

RC_UNUSED स्थिर वर्ण *env_findvar(अचर वर्ण *name, वर्ण **envp)
{
	माप_प्रकार len = म_माप(name);

	लागि (वर्ण **var = envp; *var; var++)
		यदि (म_भेदन(name, *var, len) == 0 && (*var)[len] == '=')
			फर्काऊ *var;
	फर्काऊ रिक्त;
}

RC_UNUSED स्थिर पूर्णाङ्क xopen_nonblock(अचर वर्ण *filename, पूर्णाङ्क flags, mode_t mode)
{
	पूर्णाङ्क fd = खोलो(filename, flags | O_NONBLOCK, mode);
	यदि (fd >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		पूर्णाङ्क saved_errno = त्रुटि_सं;
		बंद(fd);
		fd = -1;
		त्रुटि_सं = saved_errno;
	}
	फर्काऊ fd;
}

RC_UNUSED स्थिर पूर्णाङ्क open_rwfifo(अचर वर्ण *path, bool nonblock)
{
	पूर्णाङ्क r, w, err;

	यदि (mkfifo(path, 0600) == -1 && त्रुटि_सं != EEXIST)
		फर्काऊ -1;

	यदि ((r = खोलो(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)
		फर्काऊ -1;

	/* Leak an open writeable fd for the life of the process
	 * to keep the read end valid after a client disconnects.
	 * On linux, opening O_RDWR would solve this, but posix
	 * leaves that behaviour undefined, and for example, hurd
	 * does not properly handle it. */
	यदि ((w = खोलो(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		err = त्रुटि_सं;
		गच्छ r_err;
	}

	यदि (!nonblock) {
		पूर्णाङ्क flags = fcntl(r, F_GETFL);
		यदि (flags == -1 || fcntl(r, F_SETFL, flags & ~O_NONBLOCK) == -1) {
			err = त्रुटि_सं;
			गच्छ w_err;
		}
	}

	फर्काऊ r;

w_err:
	बंद(w);
r_err:
	बंद(r);
	त्रुटि_सं = err;
	फर्काऊ -1;
}

#endif
