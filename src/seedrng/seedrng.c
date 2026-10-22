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

#که_نه_تعریف_شوی GRND_INSECURE
#تعریف_کړه GRND_INSECURE 0x0004 /* Apparently some headers don't ship with this yet. */
#د_که_پای

#تعریف_کړه DEFAULT_تخم_DIR "/var/lib/seedrng"
#تعریف_کړه CREDITABLE_تخم "seed.credit"
#تعریف_کړه NON_CREDITABLE_تخم "seed.no-credit"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
تعداد long_opts {
	/* This has to come first so following values stay in the 0x100+ range. */
	LONGOPT_بنسټ = 0x100,
	LONGOPT_تخم_DIR,
	LONGOPT_SKIP_CREDIT,
};

ثابت حرف *applet = عدم;
ثابت حرف *extraopts = عدم;
ثابت حرف getoptstring[] = getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "seed-dir", 1, عدم, LONGOPT_تخم_DIR },
	{ "skip-credit", 0, عدم, LONGOPT_SKIP_CREDIT },
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
	"Directory for seed files (default: " DEFAULT_تخم_DIR ")",
	"Skip crediting entropy of seeds",
	longopts_help_COMMON
};
ثابت حرف *usagestring = عدم;

تعداد blake2s_lengths {
	BLAKE2_S_بلاک_LEN = 64,
	BLAKE2S_HASH_LEN = 32,
	BLAKE2_S_کلی_LEN = 32
};

تعداد seedrng_lengths {
	MAX_تخم_LEN = 512,
	MIN_تخم_LEN = BLAKE2S_HASH_LEN
};

بنية blake2s_state {
	uint32_t h[8];
	uint32_t t[2];
	uint32_t f[2];
	uint8_t buf[BLAKE2_S_بلاک_LEN];
	بینښه صحيح buflen;
	بینښه صحيح outlen;
};

#تعریف_کړه le32_منزل_cpup(a) le32toh(*(a))
#تعریف_کړه cpu_منزل_le32(a) htole32(a)
#که_نه_تعریف_شوی ARRAY_اندازه
#تعریف_کړه ARRAY_اندازه(x) (حجم(x) / حجم((x)[0]))
#د_که_پای
#که_نه_تعریف_شوی DIV_پړاو_پورته
#تعریف_کړه DIV_پړاو_پورته(n, d) (((n) + (d) - 1) / (d))
#د_که_پای

ساكن مضمن فراغ cpu_منزل_le32_array(uint32_t *buf, بینښه صحيح ټکي)
{
	طالما (ټکي--) {
		*buf = cpu_منزل_le32(*buf);
		++buf;
	}
}

ساكن مضمن فراغ le32_منزل_cpu_array(uint32_t *buf, بینښه صحيح ټکي)
{
	طالما (ټکي--) {
		*buf = le32_منزل_cpup(buf);
		++buf;
	}
}

ساكن مضمن uint32_t ror32(uint32_t کلمه, بینښه صحيح shift)
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

ساكن فراغ blake2s_سېټ_lastblock(بنية blake2s_state *state)
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

ساكن فراغ blake2s_compress(بنية blake2s_state *state, ثابت uint8_t *بلاک, size_t nblocks, ثابت uint32_t inc)
{
	uint32_t m[16];
	uint32_t v[16];
	صحيح i;

	طالما (nblocks > 0) {
		blake2s_increment_counter(state, inc);
		حافظه_نقل(m, بلاک, BLAKE2_S_بلاک_LEN);
		le32_منزل_cpu_array(m, ARRAY_اندازه(m));
		حافظه_نقل(v, state->h, 32);
		v[ 8] = blake2s_iv[0];
		v[ 9] = blake2s_iv[1];
		v[10] = blake2s_iv[2];
		v[11] = blake2s_iv[3];
		v[12] = blake2s_iv[4] ^ state->t[0];
		v[13] = blake2s_iv[5] ^ state->t[1];
		v[14] = blake2s_iv[6] ^ state->f[0];
		v[15] = blake2s_iv[7] ^ state->f[1];

#تعریف_کړه G(r, i, a, b, c, d) افعل { \
	a += b + m[blake2s_sigma[r][2 * i + 0]]; \
	d = ror32(d ^ a, 16); \
	c += d; \
	b = ror32(b ^ c, 12); \
	a += b + m[blake2s_sigma[r][2 * i + 1]]; \
	d = ror32(d ^ a, 8); \
	c += d; \
	b = ror32(b ^ c, 7); \
} طالما (0)

#تعریف_کړه پړاو(r) افعل { \
	G(r, 0, v[0], v[ 4], v[ 8], v[12]); \
	G(r, 1, v[1], v[ 5], v[ 9], v[13]); \
	G(r, 2, v[2], v[ 6], v[10], v[14]); \
	G(r, 3, v[3], v[ 7], v[11], v[15]); \
	G(r, 4, v[0], v[ 5], v[10], v[15]); \
	G(r, 5, v[1], v[ 6], v[11], v[12]); \
	G(r, 6, v[2], v[ 7], v[ 8], v[13]); \
	G(r, 7, v[3], v[ 4], v[ 9], v[14]); \
} طالما (0)
		پړاو(0);
		پړاو(1);
		پړاو(2);
		پړاو(3);
		پړاو(4);
		پړاو(5);
		پړاو(6);
		پړاو(7);
		پړاو(8);
		پړاو(9);

#تعریف_لغوه G
#تعریف_لغوه پړاو

		لكل (i = 0; i < 8; ++i)
			state->h[i] ^= v[i] ^ v[i + 8];

		بلاک += BLAKE2_S_بلاک_LEN;
		--nblocks;
	}
}

ساكن فراغ blake2s_update(بنية blake2s_state *state, ثابت فراغ *inp, size_t inlen)
{
	ثابت size_t fill = BLAKE2_S_بلاک_LEN - state->buflen;
	ثابت uint8_t *دننه = inp;

	إذا (!inlen)
		أرجع;
	إذا (inlen > fill) {
		حافظه_نقل(state->buf + state->buflen, دننه, fill);
		blake2s_compress(state, state->buf, 1, BLAKE2_S_بلاک_LEN);
		state->buflen = 0;
		دننه += fill;
		inlen -= fill;
	}
	إذا (inlen > BLAKE2_S_بلاک_LEN) {
		ثابت size_t nblocks = DIV_پړاو_پورته(inlen, BLAKE2_S_بلاک_LEN);
		blake2s_compress(state, دننه, nblocks - 1, BLAKE2_S_بلاک_LEN);
		دننه += BLAKE2_S_بلاک_LEN * (nblocks - 1);
		inlen -= BLAKE2_S_بلاک_LEN * (nblocks - 1);
	}
	حافظه_نقل(state->buf + state->buflen, دننه, inlen);
	state->buflen += inlen;
}

ساكن فراغ blake2s_final(بنية blake2s_state *state, uint8_t *بهر)
{
	blake2s_سېټ_lastblock(state);
	memset(state->buf + state->buflen, 0, BLAKE2_S_بلاک_LEN - state->buflen);
	blake2s_compress(state, state->buf, 1, state->buflen);
	cpu_منزل_le32_array(state->h, ARRAY_اندازه(state->h));
	حافظه_نقل(بهر, state->h, state->outlen);
}

ساكن ssize_t getrandom_ډک(فراغ *buf, size_t شمېر, بینښه صحيح flags)
{
	ssize_t ret, ټول = 0;
	uint8_t *p = buf;

	افعل {
		ret = getrandom(p, شمېر, flags);
		إذا (ret < 0 && errno == EINTR)
			استمر;
		وإلا إذا (ret < 0)
			أرجع ret;
		ټول += ret;
		p += ret;
		شمېر -= ret;
	} طالما (شمېر);
	أرجع ټول;
}

ساكن ssize_t read_ډک(صحيح fd, فراغ *buf, size_t شمېر)
{
	ssize_t ret, ټول = 0;
	uint8_t *p = buf;

	افعل {
		ret = اقرأ(fd, p, شمېر);
		إذا (ret < 0 && errno == EINTR)
			استمر;
		وإلا إذا (ret < 0)
			أرجع ret;
		وإلا إذا (ret == 0)
			اكسر;
		ټول += ret;
		p += ret;
		شمېر -= ret;
	} طالما (شمېر);
	أرجع ټول;
}

ساكن ssize_t write_ډک(صحيح fd, ثابت فراغ *buf, size_t شمېر)
{
	ssize_t ret, ټول = 0;
	ثابت uint8_t *p = buf;

	افعل {
		ret = اكتب(fd, p, شمېر);
		إذا (ret < 0 && errno == EINTR)
			استمر;
		وإلا إذا (ret < 0)
			أرجع ret;
		ټول += ret;
		p += ret;
		شمېر -= ret;
	} طالما (شمېر);
	أرجع ټول;
}

ساكن size_t determine_optimal_تخم_len(فراغ)
{
	size_t ret = 0;
	حرف poolsize_str[11] = { 0 };
	صحيح fd = open("/proc/sys/kernel/random/poolsize", O_RDONLY);

	إذا (fd < 0 || read_ډک(fd, poolsize_str, حجم(poolsize_str) - 1) < 0) {
		ewarn("%s: Unable to determine pool size, falling back to %u bits: %s",
		      applet, MIN_تخم_LEN * 8, strerror(errno));
		ret = MIN_تخم_LEN;
	} وإلا
		ret = DIV_پړاو_پورته(strtoul(poolsize_str, عدم, 10), 8);
	إذا (fd >= 0)
		close(fd);
	إذا (ret < MIN_تخم_LEN)
		ret = MIN_تخم_LEN;
	وإلا إذا (ret > MAX_تخم_LEN)
		ret = MAX_تخم_LEN;
	أرجع ret;
}

ساكن صحيح read_new_تخم(uint8_t *تخم, size_t len, bool *is_creditable)
{
	ssize_t ret;
	صحيح urandom_fd;

	*is_creditable = false;
	ret = getrandom_ډک(تخم, len, GRND_NONBLOCK);
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
	} وإلا إذا (getrandom_ډک(تخم, len, GRND_INSECURE) == (ssize_t)len)
		أرجع 0;
	urandom_fd = open("/dev/urandom", O_RDONLY);
	إذا (urandom_fd < 0)
		أرجع -1;
	ret = read_ډک(urandom_fd, تخم, len);
	إذا (ret == (ssize_t)len)
		ret = 0;
	وإلا
		ret = -errno ? -errno : -EIO;
	close(urandom_fd);
	errno = -ret;
	أرجع ret ? -1 : 0;
}

ساكن صحيح تخم_rng(uint8_t *تخم, size_t len, bool credit)
{
	بنية {
		صحيح entropy_شمېر;
		صحيح buf_اندازه;
		uint8_t بفر[MAX_تخم_LEN];
	} req = {
		.entropy_شمېر = credit ? len * 8 : 0,
		.buf_اندازه = len
	};
	صحيح random_fd, ret;

	إذا (len > حجم(req.بفر)) {
		errno = EFBIG;
		أرجع -1;
	}
	حافظه_نقل(req.بفر, تخم, len);

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

ساكن صحيح تخم_from_file_if_exists(ثابت حرف *filename, صحيح dfd, bool credit, بنية blake2s_state *hash)
{
	uint8_t تخم[MAX_تخم_LEN];
	ssize_t تخم_len;
	صحيح fd = -1, ret = 0;

	fd = openat(dfd, filename, O_RDONLY);
	إذا (fd < 0 && errno == ENOENT)
		أرجع 0;
	وإلا إذا (fd < 0) {
		ret = -errno;
		eerror("%s: Unable to open seed file: %s", applet, strerror(errno));
		اذهب بهر;
	}
	تخم_len = read_ډک(fd, تخم, حجم(تخم));
	إذا (تخم_len < 0) {
		ret = -errno;
		eerror("%s: Unable to read seed file: %s", applet, strerror(errno));
		اذهب بهر;
	}
	إذا ((unlinkat(dfd, filename, 0) < 0 || fsync(dfd) < 0) && تخم_len) {
		ret = -errno;
		eerror("%s: Unable to remove seed after reading, so not seeding: %s",
		       applet, strerror(errno));
		اذهب بهر;
	}
	إذا (!تخم_len)
		اذهب بهر;

	blake2s_update(hash, &تخم_len, حجم(تخم_len));
	blake2s_update(hash, تخم, تخم_len);

	einfo("Seeding %zd bits %s crediting", تخم_len * 8, credit ? "and" : "without");
	إذا (تخم_rng(تخم, تخم_len, credit) < 0) {
		ret = -errno;
		eerror("%s: Unable to seed: %s", applet, strerror(errno));
	}

بهر:
	إذا (fd >= 0)
		close(fd);
	errno = -ret;
	أرجع ret ? -1 : 0;
}

صحيح اصلي(صحيح argc, حرف **argv)
{
	ساكن ثابت حرف seedrng_مختاړی[] = "SeedRNG v1 Old+New Prefix";
	ساكن ثابت حرف seedrng_failure[] = "SeedRNG v1 No New Seed Failure";
	صحيح opt, fd, dfd, پروګرام_ret = 0;
	ثابت حرف *تخم_dir = عدم;
	uint8_t new_تخم[MAX_تخم_LEN];
	size_t new_تخم_len;
	bool new_تخم_creditable;
	بنية timespec realtime = { 0 }, boottime = { 0 };
	بنية blake2s_state hash;
	bool skip_credit = false;

	applet = basename_c(argv[0]);

	طالما ((opt = getopt_long(argc, argv, getoptstring, longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
		حالة LONGOPT_تخم_DIR:
			إذا (!تخم_dir)
				تخم_dir = optarg;
			اكسر;
		حالة LONGOPT_SKIP_CREDIT:
			skip_credit = true;
			اكسر;
		case_RC_COMMON_GETOPT
		}
	}
	إذا (!تخم_dir)
		تخم_dir = DEFAULT_تخم_DIR;
	إذا (getuid())
		eerrorx("%s: superuser access is required", applet);
	umask(0077);

	blake2s_init(&hash, BLAKE2S_HASH_LEN);
	blake2s_update(&hash, seedrng_مختاړی, مزي_اوږدوالی(seedrng_مختاړی));
	clock_gettime(CLOCK_REALTIME, &realtime);
	clock_gettime(CLOCK_BOOTTIME, &boottime);
	blake2s_update(&hash, &realtime, حجم(realtime));
	blake2s_update(&hash, &boottime, حجم(boottime));

	إذا (mkdir(تخم_dir, 0700) < 0 && errno != EEXIST)
		eerrorx("%s: Unable to create seed directory: %s", applet, strerror(errno));

	dfd = open(تخم_dir, O_DIRECTORY | O_RDONLY);
	إذا (dfd < 0 || flock(dfd, LOCK_EX) < 0)
		eerrorx("%s: Unable to lock seed directory: %s", applet, strerror(errno));

	إذا (تخم_from_file_if_exists(NON_CREDITABLE_تخم, dfd, false, &hash) < 0)
		پروګرام_ret |= 1 << 1;
	إذا (تخم_from_file_if_exists(CREDITABLE_تخم, dfd, !skip_credit, &hash) < 0)
		پروګرام_ret |= 1 << 2;

	new_تخم_len = determine_optimal_تخم_len();
	إذا (read_new_تخم(new_تخم, new_تخم_len, &new_تخم_creditable) < 0) {
		eerror("%s: Unable to read new seed: %s", applet, strerror(errno));
		new_تخم_len = BLAKE2S_HASH_LEN;
		strncpy((حرف *)new_تخم, seedrng_failure, new_تخم_len);
		پروګرام_ret |= 1 << 3;
	}
	blake2s_update(&hash, &new_تخم_len, حجم(new_تخم_len));
	blake2s_update(&hash, new_تخم, new_تخم_len);
	blake2s_final(&hash, new_تخم + new_تخم_len - BLAKE2S_HASH_LEN);

	einfo("Saving %zu bits of %s seed for next boot", new_تخم_len * 8, new_تخم_creditable ? "creditable" : "non-creditable");
	fd = openat(dfd, NON_CREDITABLE_تخم, O_WRONLY | O_CREAT | O_TRUNC, 0400);
	إذا (fd < 0) {
		eerror("%s: Unable to open seed file for writing: %s", applet, strerror(errno));
		أرجع پروګرام_ret | (1 << 4);
	}
	إذا (write_ډک(fd, new_تخم, new_تخم_len) != (ssize_t)new_تخم_len || fsync(fd) < 0) {
		eerror("%s: Unable to write seed file: %s", applet, strerror(errno));
		أرجع پروګرام_ret | (1 << 5);
	}
	إذا (new_تخم_creditable && renameat(dfd, NON_CREDITABLE_تخم, dfd, CREDITABLE_تخم) < 0) {
		ewarn("%s: Unable to make new seed creditable: %s", applet, strerror(errno));
		أرجع پروګرام_ret | (1 << 6);
	}
	أرجع پروګرام_ret;
}
