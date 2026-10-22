/*
 * seedrng.c
 * Seed kernel RNG from seed file, based on code from:
 * https://git.zx2c4.com/seedrng/about/
 */

/*
 * Copyright (c) 2022-2023 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ضمن <linux/random.h>
#ضمن <sys/random.h>
#ضمن <sys/ioctl.h>
#ضمن <sys/file.h>
#ضمن <sys/stat.h>
#ضمن <sys/types.h>
#ضمن <fcntl.h>
#ضمن <getopt.h>
#ضمن <poll.h>
#ضمن <unistd.h>
#ضمن <time.h>
#ضمن <errno.h>
#ضمن <endian.h>
#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdint.h>
#ضمن <string.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>

#ضمن "einfo.h"
#ضمن "helpers.h"
#ضمن "_usage.h"

#اگر_تعریف_نشده GRND_INSECURE
#تعریف_کن GRND_INSECURE 0x0004 /* Apparently some headers don't ship with this yet. */
#پایان_اگر

#تعریف_کن DEFAULT_دانه_DIR "/var/lib/seedrng"
#تعریف_کن CREDITABLE_دانه "seed.credit"
#تعریف_کن NON_CREDITABLE_دانه "seed.no-credit"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
تعداد long_opts {
	/* This has to come first so following values stay in the 0x100+ range. */
	LONGOPT_پایه = 0x100,
	LONGOPT_دانه_DIR,
	LONGOPT_SKIP_CREDIT,
};

ثابت حرف *applet = عدم;
ثابت حرف *extraopts = عدم;
ثابت حرف getoptstring[] = getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "seed-dir", 1, عدم, LONGOPT_دانه_DIR },
	{ "skip-credit", 0, عدم, LONGOPT_SKIP_CREDIT },
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
	"Directory for seed files (default: " DEFAULT_دانه_DIR ")",
	"Skip crediting entropy of seeds",
	longopts_help_COMMON
};
ثابت حرف *usagestring = عدم;

تعداد blake2s_lengths {
	BLAKE2_S_بلوک_LEN = 64,
	BLAKE2S_HASH_LEN = 32,
	BLAKE2_S_کلید_LEN = 32
};

تعداد seedrng_lengths {
	MAX_دانه_LEN = 512,
	MIN_دانه_LEN = BLAKE2S_HASH_LEN
};

بنية blake2s_state {
	uint32_t h[8];
	uint32_t t[2];
	uint32_t f[2];
	uint8_t buf[BLAKE2_S_بلوک_LEN];
	بیعلامت صحيح buflen;
	بیعلامت صحيح outlen;
};

#تعریف_کن le32_مقصد_cpup(a) le32toh(*(a))
#تعریف_کن cpu_مقصد_le32(a) htole32(a)
#اگر_تعریف_نشده ARRAY_اندازه
#تعریف_کن ARRAY_اندازه(x) (حجم(x) / حجم((x)[0]))
#پایان_اگر
#اگر_تعریف_نشده DIV_دور_به_بالا
#تعریف_کن DIV_دور_به_بالا(n, d) (((n) + (d) - 1) / (d))
#پایان_اگر

ساكن مضمن فراغ cpu_مقصد_le32_array(uint32_t *buf, بیعلامت صحيح واژگان)
{
	طالما (واژگان--) {
		*buf = cpu_مقصد_le32(*buf);
		++buf;
	}
}

ساكن مضمن فراغ le32_مقصد_cpu_array(uint32_t *buf, بیعلامت صحيح واژگان)
{
	طالما (واژگان--) {
		*buf = le32_مقصد_cpup(buf);
		++buf;
	}
}

ساكن مضمن uint32_t ror32(uint32_t کلمه, بیعلامت صحيح shift)
{
	أرجع (کلمه >> (shift & 31)) | (کلمه << ((-shift) & 31));
}

ساكن ثابت uint32_t blake2s_iv[8] = {
	0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
	0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

ساكن ثابت uint8_t blake2s_sigma[10][16] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 },
	{ 11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4 },
	{ 7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8 },
	{ 9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13 },
	{ 2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9 },
	{ 12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11 },
	{ 13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10 },
	{ 6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5 },
	{ 10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0 },
};

ساكن فراغ blake2s_مجموعه_lastblock(بنية blake2s_state *state)
{
	state->f[0] = -1;
}

ساكن فراغ blake2s_increment_counter(بنية blake2s_state *state, ثابت uint32_t inc)
{
	state->t[0] += inc;
	state->t[1] += (state->t[0] < inc);
}

ساكن فراغ blake2s_init_param(بنية blake2s_state *state, ثابت uint32_t param)
{
	صحيح i;

	memset(state, 0, حجم(*state));
	لكل (i = 0; i < 8; ++i)
		state->h[i] = blake2s_iv[i];
	state->h[0] ^= param;
}

ساكن فراغ blake2s_init(بنية blake2s_state *state, ثابت size_t outlen)
{
	blake2s_init_param(state, 0x01010000 | outlen);
	state->outlen = outlen;
}

ساكن فراغ blake2s_compress(بنية blake2s_state *state, ثابت uint8_t *بلوک, size_t nblocks, ثابت uint32_t inc)
{
	uint32_t m[16];
	uint32_t v[16];
	صحيح i;

	طالما (nblocks > 0) {
		blake2s_increment_counter(state, inc);
		کپی_حافظه(m, بلوک, BLAKE2_S_بلوک_LEN);
		le32_مقصد_cpu_array(m, ARRAY_اندازه(m));
		کپی_حافظه(v, state->h, 32);
		v[ 8] = blake2s_iv[0];
		v[ 9] = blake2s_iv[1];
		v[10] = blake2s_iv[2];
		v[11] = blake2s_iv[3];
		v[12] = blake2s_iv[4] ^ state->t[0];
		v[13] = blake2s_iv[5] ^ state->t[1];
		v[14] = blake2s_iv[6] ^ state->f[0];
		v[15] = blake2s_iv[7] ^ state->f[1];

#تعریف_کن G(r, i, a, b, c, d) افعل { \
	a += b + m[blake2s_sigma[r][2 * i + 0]]; \
	d = ror32(d ^ a, 16); \
	c += d; \
	b = ror32(b ^ c, 12); \
	a += b + m[blake2s_sigma[r][2 * i + 1]]; \
	d = ror32(d ^ a, 8); \
	c += d; \
	b = ror32(b ^ c, 7); \
} طالما (0)

#تعریف_کن دور(r) افعل { \
	G(r, 0, v[0], v[ 4], v[ 8], v[12]); \
	G(r, 1, v[1], v[ 5], v[ 9], v[13]); \
	G(r, 2, v[2], v[ 6], v[10], v[14]); \
	G(r, 3, v[3], v[ 7], v[11], v[15]); \
	G(r, 4, v[0], v[ 5], v[10], v[15]); \
	G(r, 5, v[1], v[ 6], v[11], v[12]); \
	G(r, 6, v[2], v[ 7], v[ 8], v[13]); \
	G(r, 7, v[3], v[ 4], v[ 9], v[14]); \
} طالما (0)
		دور(0);
		دور(1);
		دور(2);
		دور(3);
		دور(4);
		دور(5);
		دور(6);
		دور(7);
		دور(8);
		دور(9);

#لغو_تعریف G
#لغو_تعریف دور

		لكل (i = 0; i < 8; ++i)
			state->h[i] ^= v[i] ^ v[i + 8];

		بلوک += BLAKE2_S_بلوک_LEN;
		--nblocks;
	}
}

ساكن فراغ blake2s_update(بنية blake2s_state *state, ثابت فراغ *inp, size_t inlen)
{
	ثابت size_t fill = BLAKE2_S_بلوک_LEN - state->buflen;
	ثابت uint8_t *درون = inp;

	إذا (!inlen)
		أرجع;
	إذا (inlen > fill) {
		کپی_حافظه(state->buf + state->buflen, درون, fill);
		blake2s_compress(state, state->buf, 1, BLAKE2_S_بلوک_LEN);
		state->buflen = 0;
		درون += fill;
		inlen -= fill;
	}
	إذا (inlen > BLAKE2_S_بلوک_LEN) {
		ثابت size_t nblocks = DIV_دور_به_بالا(inlen, BLAKE2_S_بلوک_LEN);
		blake2s_compress(state, درون, nblocks - 1, BLAKE2_S_بلوک_LEN);
		درون += BLAKE2_S_بلوک_LEN * (nblocks - 1);
		inlen -= BLAKE2_S_بلوک_LEN * (nblocks - 1);
	}
	کپی_حافظه(state->buf + state->buflen, درون, inlen);
	state->buflen += inlen;
}

ساكن فراغ blake2s_final(بنية blake2s_state *state, uint8_t *بیرون)
{
	blake2s_مجموعه_lastblock(state);
	memset(state->buf + state->buflen, 0, BLAKE2_S_بلوک_LEN - state->buflen);
	blake2s_compress(state, state->buf, 1, state->buflen);
	cpu_مقصد_le32_array(state->h, ARRAY_اندازه(state->h));
	کپی_حافظه(بیرون, state->h, state->outlen);
}

ساكن ssize_t getrandom_پر(فراغ *buf, size_t شمار, بیعلامت صحيح flags)
{
	ssize_t ret, مجموع = 0;
	uint8_t *p = buf;

	افعل {
		ret = getrandom(p, شمار, flags);
		إذا (ret < 0 && errno == EINTR)
			استمر;
		وإلا إذا (ret < 0)
			أرجع ret;
		مجموع += ret;
		p += ret;
		شمار -= ret;
	} طالما (شمار);
	أرجع مجموع;
}

ساكن ssize_t read_پر(صحيح fd, فراغ *buf, size_t شمار)
{
	ssize_t ret, مجموع = 0;
	uint8_t *p = buf;

	افعل {
		ret = اقرأ(fd, p, شمار);
		إذا (ret < 0 && errno == EINTR)
			استمر;
		وإلا إذا (ret < 0)
			أرجع ret;
		وإلا إذا (ret == 0)
			اكسر;
		مجموع += ret;
		p += ret;
		شمار -= ret;
	} طالما (شمار);
	أرجع مجموع;
}

ساكن ssize_t write_پر(صحيح fd, ثابت فراغ *buf, size_t شمار)
{
	ssize_t ret, مجموع = 0;
	ثابت uint8_t *p = buf;

	افعل {
		ret = اكتب(fd, p, شمار);
		إذا (ret < 0 && errno == EINTR)
			استمر;
		وإلا إذا (ret < 0)
			أرجع ret;
		مجموع += ret;
		p += ret;
		شمار -= ret;
	} طالما (شمار);
	أرجع مجموع;
}

ساكن size_t determine_optimal_دانه_len(فراغ)
{
	size_t ret = 0;
	حرف poolsize_str[11] = { 0 };
	صحيح fd = open("/proc/sys/kernel/random/poolsize", O_RDONLY);

	إذا (fd < 0 || read_پر(fd, poolsize_str, حجم(poolsize_str) - 1) < 0) {
		ewarn("%s: Unable to determine pool size, falling back to %u bits: %s",
		      applet, MIN_دانه_LEN * 8, strerror(errno));
		ret = MIN_دانه_LEN;
	} وإلا
		ret = DIV_دور_به_بالا(strtoul(poolsize_str, عدم, 10), 8);
	إذا (fd >= 0)
		close(fd);
	إذا (ret < MIN_دانه_LEN)
		ret = MIN_دانه_LEN;
	وإلا إذا (ret > MAX_دانه_LEN)
		ret = MAX_دانه_LEN;
	أرجع ret;
}

ساكن صحيح read_new_دانه(uint8_t *دانه, size_t len, bool *is_creditable)
{
	ssize_t ret;
	صحيح urandom_fd;

	*is_creditable = false;
	ret = getrandom_پر(دانه, len, GRND_NONBLOCK);
	إذا (ret == (ssize_t)len) {
		*is_creditable = true;
		أرجع 0;
	} وإلا إذا (ret < 0 && errno == ENOSYS) {
		بنية pollfd random_fd = {
			.fd = open("/dev/random", O_RDONLY),
			.events = POLLIN
		};
		إذا (random_fd.fd < 0)
			أرجع -errno;
		*is_creditable = poll(&random_fd, 1, 0) == 1;
		close(random_fd.fd);
	} وإلا إذا (getrandom_پر(دانه, len, GRND_INSECURE) == (ssize_t)len)
		أرجع 0;
	urandom_fd = open("/dev/urandom", O_RDONLY);
	إذا (urandom_fd < 0)
		أرجع -1;
	ret = read_پر(urandom_fd, دانه, len);
	إذا (ret == (ssize_t)len)
		ret = 0;
	وإلا
		ret = -errno ? -errno : -EIO;
	close(urandom_fd);
	errno = -ret;
	أرجع ret ? -1 : 0;
}

ساكن صحيح دانه_rng(uint8_t *دانه, size_t len, bool credit)
{
	بنية {
		صحيح entropy_شمار;
		صحيح buf_اندازه;
		uint8_t میانگیر[MAX_دانه_LEN];
	} req = {
		.entropy_شمار = credit ? len * 8 : 0,
		.buf_اندازه = len
	};
	صحيح random_fd, ret;

	إذا (len > حجم(req.میانگیر)) {
		errno = EFBIG;
		أرجع -1;
	}
	کپی_حافظه(req.میانگیر, دانه, len);

	random_fd = open("/dev/urandom", O_RDONLY);
	إذا (random_fd < 0)
		أرجع -1;
	ret = ioctl(random_fd, RNDADDENTROPY, &req);
	إذا (ret)
		ret = -errno ? -errno : -EIO;
	close(random_fd);
	errno = -ret;
	أرجع ret ? -1 : 0;
}

ساكن صحيح دانه_from_file_if_exists(ثابت حرف *filename, صحيح dfd, bool credit, بنية blake2s_state *hash)
{
	uint8_t دانه[MAX_دانه_LEN];
	ssize_t دانه_len;
	صحيح fd = -1, ret = 0;

	fd = openat(dfd, filename, O_RDONLY);
	إذا (fd < 0 && errno == ENOENT)
		أرجع 0;
	وإلا إذا (fd < 0) {
		ret = -errno;
		eerror("%s: Unable to open seed file: %s", applet, strerror(errno));
		اذهب بیرون;
	}
	دانه_len = read_پر(fd, دانه, حجم(دانه));
	إذا (دانه_len < 0) {
		ret = -errno;
		eerror("%s: Unable to read seed file: %s", applet, strerror(errno));
		اذهب بیرون;
	}
	إذا ((unlinkat(dfd, filename, 0) < 0 || fsync(dfd) < 0) && دانه_len) {
		ret = -errno;
		eerror("%s: Unable to remove seed after reading, so not seeding: %s",
		       applet, strerror(errno));
		اذهب بیرون;
	}
	إذا (!دانه_len)
		اذهب بیرون;

	blake2s_update(hash, &دانه_len, حجم(دانه_len));
	blake2s_update(hash, دانه, دانه_len);

	einfo("Seeding %zd bits %s crediting", دانه_len * 8, credit ? "and" : "without");
	إذا (دانه_rng(دانه, دانه_len, credit) < 0) {
		ret = -errno;
		eerror("%s: Unable to seed: %s", applet, strerror(errno));
	}

بیرون:
	إذا (fd >= 0)
		close(fd);
	errno = -ret;
	أرجع ret ? -1 : 0;
}

صحيح اصلی(صحيح argc, حرف **argv)
{
	ساكن ثابت حرف seedrng_پیشوند[] = "SeedRNG v1 Old+New Prefix";
	ساكن ثابت حرف seedrng_failure[] = "SeedRNG v1 No New Seed Failure";
	صحيح opt, fd, dfd, برنامه_ret = 0;
	ثابت حرف *دانه_dir = عدم;
	uint8_t new_دانه[MAX_دانه_LEN];
	size_t new_دانه_len;
	bool new_دانه_creditable;
	بنية timespec realtime = { 0 }, boottime = { 0 };
	بنية blake2s_state hash;
	bool skip_credit = false;

	applet = basename_c(argv[0]);

	طالما ((opt = getopt_long(argc, argv, getoptstring, longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
		حالة LONGOPT_دانه_DIR:
			إذا (!دانه_dir)
				دانه_dir = optarg;
			اكسر;
		حالة LONGOPT_SKIP_CREDIT:
			skip_credit = true;
			اكسر;
		case_RC_COMMON_GETOPT
		}
	}
	إذا (!دانه_dir)
		دانه_dir = DEFAULT_دانه_DIR;
	إذا (getuid())
		eerrorx("%s: superuser access is required", applet);
	umask(0077);

	blake2s_init(&hash, BLAKE2S_HASH_LEN);
	blake2s_update(&hash, seedrng_پیشوند, طول_رشته(seedrng_پیشوند));
	clock_gettime(CLOCK_REALTIME, &realtime);
	clock_gettime(CLOCK_BOOTTIME, &boottime);
	blake2s_update(&hash, &realtime, حجم(realtime));
	blake2s_update(&hash, &boottime, حجم(boottime));

	إذا (mkdir(دانه_dir, 0700) < 0 && errno != EEXIST)
		eerrorx("%s: Unable to create seed directory: %s", applet, strerror(errno));

	dfd = open(دانه_dir, O_DIRECTORY | O_RDONLY);
	إذا (dfd < 0 || flock(dfd, LOCK_EX) < 0)
		eerrorx("%s: Unable to lock seed directory: %s", applet, strerror(errno));

	إذا (دانه_from_file_if_exists(NON_CREDITABLE_دانه, dfd, false, &hash) < 0)
		برنامه_ret |= 1 << 1;
	إذا (دانه_from_file_if_exists(CREDITABLE_دانه, dfd, !skip_credit, &hash) < 0)
		برنامه_ret |= 1 << 2;

	new_دانه_len = determine_optimal_دانه_len();
	إذا (read_new_دانه(new_دانه, new_دانه_len, &new_دانه_creditable) < 0) {
		eerror("%s: Unable to read new seed: %s", applet, strerror(errno));
		new_دانه_len = BLAKE2S_HASH_LEN;
		strncpy((حرف *)new_دانه, seedrng_failure, new_دانه_len);
		برنامه_ret |= 1 << 3;
	}
	blake2s_update(&hash, &new_دانه_len, حجم(new_دانه_len));
	blake2s_update(&hash, new_دانه, new_دانه_len);
	blake2s_final(&hash, new_دانه + new_دانه_len - BLAKE2S_HASH_LEN);

	einfo("Saving %zu bits of %s seed for next boot", new_دانه_len * 8, new_دانه_creditable ? "creditable" : "non-creditable");
	fd = openat(dfd, NON_CREDITABLE_دانه, O_WRONLY | O_CREAT | O_TRUNC, 0400);
	إذا (fd < 0) {
		eerror("%s: Unable to open seed file for writing: %s", applet, strerror(errno));
		أرجع برنامه_ret | (1 << 4);
	}
	إذا (write_پر(fd, new_دانه, new_دانه_len) != (ssize_t)new_دانه_len || fsync(fd) < 0) {
		eerror("%s: Unable to write seed file: %s", applet, strerror(errno));
		أرجع برنامه_ret | (1 << 5);
	}
	إذا (new_دانه_creditable && renameat(dfd, NON_CREDITABLE_دانه, dfd, CREDITABLE_دانه) < 0) {
		ewarn("%s: Unable to make new seed creditable: %s", applet, strerror(errno));
		أرجع برنامه_ret | (1 << 6);
	}
	أرجع برنامه_ret;
}
