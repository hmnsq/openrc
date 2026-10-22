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

#شامل <linux/random.h>
#شامل <sys/random.h>
#شامل <sys/ioctl.h>
#شامل <sys/file.h>
#شامل <sys/stat.h>
#شامل <sys/types.h>
#شامل <fcntl.h>
#شامل <getopt.h>
#شامل <poll.h>
#شامل <unistd.h>
#شامل <time.h>
#شامل <errno.h>
#شامل <endian.h>
#شامل <stdbool.h>
#شامل <stddef.h>
#شامل <stdint.h>
#شامل <string.h>
#شامل <stdio.h>
#شامل <stdlib.h>

#شامل "einfo.h"
#شامل "helpers.h"
#شامل "_usage.h"

#اگر_غیر_تعریف_شدہ GRND_INSECURE
#تعریف_کرو GRND_INSECURE 0x0004 /* Apparently some headers don't ship with this yet. */
#اگر_ختم

#تعریف_کرو DEFAULT_بیج_DIR "/var/lib/seedrng"
#تعریف_کرو CREDITABLE_بیج "seed.credit"
#تعریف_کرو NON_CREDITABLE_بیج "seed.no-credit"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
شمار long_opts {
	/* This has to come first so following values stay in the 0x100+ range. */
	LONGOPT_بنیاد = 0x100,
	LONGOPT_بیج_DIR,
	LONGOPT_SKIP_CREDIT,
};

ثابت حرف *applet = تہی;
ثابت حرف *extraopts = تہی;
ثابت حرف getoptstring[] = getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{ "seed-dir", 1, تہی, LONGOPT_بیج_DIR },
	{ "skip-credit", 0, تہی, LONGOPT_SKIP_CREDIT },
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
	"Directory for seed files (default: " DEFAULT_بیج_DIR ")",
	"Skip crediting entropy of seeds",
	longopts_help_COMMON
};
ثابت حرف *usagestring = تہی;

شمار blake2s_lengths {
	BLAKE2_S_بلاک_LEN = 64,
	BLAKE2S_HASH_LEN = 32,
	BLAKE2_S_کلید_LEN = 32
};

شمار seedrng_lengths {
	MAX_بیج_LEN = 512,
	MIN_بیج_LEN = BLAKE2S_HASH_LEN
};

ساخت blake2s_state {
	uint32_t h[8];
	uint32_t t[2];
	uint32_t f[2];
	uint8_t buf[BLAKE2_S_بلاک_LEN];
	غیرعلامتی صحیح buflen;
	غیرعلامتی صحیح outlen;
};

#تعریف_کرو le32_مقصد_cpup(a) le32toh(*(a))
#تعریف_کرو cpu_مقصد_le32(a) htole32(a)
#اگر_غیر_تعریف_شدہ ARRAY_سائز
#تعریف_کرو ARRAY_سائز(x) (حجم(x) / حجم((x)[0]))
#اگر_ختم
#اگر_غیر_تعریف_شدہ DIV_باری_اوپر
#تعریف_کرو DIV_باری_اوپر(n, d) (((n) + (d) - 1) / (d))
#اگر_ختم

ساکن اندرونی خالی cpu_مقصد_le32_array(uint32_t *buf, غیرعلامتی صحیح الفاظ)
{
	جب (الفاظ--) {
		*buf = cpu_مقصد_le32(*buf);
		++buf;
	}
}

ساکن اندرونی خالی le32_مقصد_cpu_array(uint32_t *buf, غیرعلامتی صحیح الفاظ)
{
	جب (الفاظ--) {
		*buf = le32_مقصد_cpup(buf);
		++buf;
	}
}

ساکن اندرونی uint32_t ror32(uint32_t لفظ, غیرعلامتی صحیح shift)
{
	واپس (لفظ >> (shift & 31)) | (لفظ << ((-shift) & 31));
}

ساکن ثابت uint32_t blake2s_iv[8] = {
	0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
	0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

ساکن ثابت uint8_t blake2s_sigma[10][16] = {
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

ساکن خالی blake2s_سیٹ_lastblock(ساخت blake2s_state *state)
{
	state->f[0] = -1;
}

ساکن خالی blake2s_increment_شمار(ساخت blake2s_state *state, ثابت uint32_t inc)
{
	state->t[0] += inc;
	state->t[1] += (state->t[0] < inc);
}

ساکن خالی blake2s_init_param(ساخت blake2s_state *state, ثابت uint32_t param)
{
	صحیح i;

	memset(state, 0, حجم(*state));
	برائے (i = 0; i < 8; ++i)
		state->h[i] = blake2s_iv[i];
	state->h[0] ^= param;
}

ساکن خالی blake2s_init(ساخت blake2s_state *state, ثابت size_t outlen)
{
	blake2s_init_param(state, 0x01010000 | outlen);
	state->outlen = outlen;
}

ساکن خالی blake2s_compress(ساخت blake2s_state *state, ثابت uint8_t *بلاک, size_t nblocks, ثابت uint32_t inc)
{
	uint32_t m[16];
	uint32_t v[16];
	صحیح i;

	جب (nblocks > 0) {
		blake2s_increment_شمار(state, inc);
		حافظہ_نقل(m, بلاک, BLAKE2_S_بلاک_LEN);
		le32_مقصد_cpu_array(m, ARRAY_سائز(m));
		حافظہ_نقل(v, state->h, 32);
		v[ 8] = blake2s_iv[0];
		v[ 9] = blake2s_iv[1];
		v[10] = blake2s_iv[2];
		v[11] = blake2s_iv[3];
		v[12] = blake2s_iv[4] ^ state->t[0];
		v[13] = blake2s_iv[5] ^ state->t[1];
		v[14] = blake2s_iv[6] ^ state->f[0];
		v[15] = blake2s_iv[7] ^ state->f[1];

#تعریف_کرو G(r, i, a, b, c, d) کرو { \
	a += b + m[blake2s_sigma[r][2 * i + 0]]; \
	d = ror32(d ^ a, 16); \
	c += d; \
	b = ror32(b ^ c, 12); \
	a += b + m[blake2s_sigma[r][2 * i + 1]]; \
	d = ror32(d ^ a, 8); \
	c += d; \
	b = ror32(b ^ c, 7); \
} جب (0)

#تعریف_کرو باری(r) کرو { \
	G(r, 0, v[0], v[ 4], v[ 8], v[12]); \
	G(r, 1, v[1], v[ 5], v[ 9], v[13]); \
	G(r, 2, v[2], v[ 6], v[10], v[14]); \
	G(r, 3, v[3], v[ 7], v[11], v[15]); \
	G(r, 4, v[0], v[ 5], v[10], v[15]); \
	G(r, 5, v[1], v[ 6], v[11], v[12]); \
	G(r, 6, v[2], v[ 7], v[ 8], v[13]); \
	G(r, 7, v[3], v[ 4], v[ 9], v[14]); \
} جب (0)
		باری(0);
		باری(1);
		باری(2);
		باری(3);
		باری(4);
		باری(5);
		باری(6);
		باری(7);
		باری(8);
		باری(9);

#تعریف_منسوخ G
#تعریف_منسوخ باری

		برائے (i = 0; i < 8; ++i)
			state->h[i] ^= v[i] ^ v[i + 8];

		بلاک += BLAKE2_S_بلاک_LEN;
		--nblocks;
	}
}

ساکن خالی blake2s_update(ساخت blake2s_state *state, ثابت خالی *inp, size_t inlen)
{
	ثابت size_t fill = BLAKE2_S_بلاک_LEN - state->buflen;
	ثابت uint8_t *اندر = inp;

	اگر (!inlen)
		واپس;
	اگر (inlen > fill) {
		حافظہ_نقل(state->buf + state->buflen, اندر, fill);
		blake2s_compress(state, state->buf, 1, BLAKE2_S_بلاک_LEN);
		state->buflen = 0;
		اندر += fill;
		inlen -= fill;
	}
	اگر (inlen > BLAKE2_S_بلاک_LEN) {
		ثابت size_t nblocks = DIV_باری_اوپر(inlen, BLAKE2_S_بلاک_LEN);
		blake2s_compress(state, اندر, nblocks - 1, BLAKE2_S_بلاک_LEN);
		اندر += BLAKE2_S_بلاک_LEN * (nblocks - 1);
		inlen -= BLAKE2_S_بلاک_LEN * (nblocks - 1);
	}
	حافظہ_نقل(state->buf + state->buflen, اندر, inlen);
	state->buflen += inlen;
}

ساکن خالی blake2s_final(ساخت blake2s_state *state, uint8_t *برآمد)
{
	blake2s_سیٹ_lastblock(state);
	memset(state->buf + state->buflen, 0, BLAKE2_S_بلاک_LEN - state->buflen);
	blake2s_compress(state, state->buf, 1, state->buflen);
	cpu_مقصد_le32_array(state->h, ARRAY_سائز(state->h));
	حافظہ_نقل(برآمد, state->h, state->outlen);
}

ساکن ssize_t getrandom_بھرا(خالی *buf, size_t گنتی, غیرعلامتی صحیح flags)
{
	ssize_t ret, کل = 0;
	uint8_t *p = buf;

	کرو {
		ret = getrandom(p, گنتی, flags);
		اگر (ret < 0 && errno == EINTR)
			جاری;
		ورنہ اگر (ret < 0)
			واپس ret;
		کل += ret;
		p += ret;
		گنتی -= ret;
	} جب (گنتی);
	واپس کل;
}

ساکن ssize_t read_بھرا(صحیح fd, خالی *buf, size_t گنتی)
{
	ssize_t ret, کل = 0;
	uint8_t *p = buf;

	کرو {
		ret = اقرأ(fd, p, گنتی);
		اگر (ret < 0 && errno == EINTR)
			جاری;
		ورنہ اگر (ret < 0)
			واپس ret;
		ورنہ اگر (ret == 0)
			توڑو;
		کل += ret;
		p += ret;
		گنتی -= ret;
	} جب (گنتی);
	واپس کل;
}

ساکن ssize_t write_بھرا(صحیح fd, ثابت خالی *buf, size_t گنتی)
{
	ssize_t ret, کل = 0;
	ثابت uint8_t *p = buf;

	کرو {
		ret = اكتب(fd, p, گنتی);
		اگر (ret < 0 && errno == EINTR)
			جاری;
		ورنہ اگر (ret < 0)
			واپس ret;
		کل += ret;
		p += ret;
		گنتی -= ret;
	} جب (گنتی);
	واپس کل;
}

ساکن size_t determine_optimal_بیج_len(خالی)
{
	size_t ret = 0;
	حرف poolsize_str[11] = { 0 };
	صحیح fd = open("/proc/sys/kernel/random/poolsize", O_RDONLY);

	اگر (fd < 0 || read_بھرا(fd, poolsize_str, حجم(poolsize_str) - 1) < 0) {
		ewarn("%s: Unable to determine pool size, falling back to %u bits: %s",
		      applet, MIN_بیج_LEN * 8, strerror(errno));
		ret = MIN_بیج_LEN;
	} ورنہ
		ret = DIV_باری_اوپر(strtoul(poolsize_str, تہی, 10), 8);
	اگر (fd >= 0)
		close(fd);
	اگر (ret < MIN_بیج_LEN)
		ret = MIN_بیج_LEN;
	ورنہ اگر (ret > MAX_بیج_LEN)
		ret = MAX_بیج_LEN;
	واپس ret;
}

ساکن صحیح read_new_بیج(uint8_t *بیج, size_t len, bool *is_creditable)
{
	ssize_t ret;
	صحیح urandom_fd;

	*is_creditable = false;
	ret = getrandom_بھرا(بیج, len, GRND_NONBLOCK);
	اگر (ret == (ssize_t)len) {
		*is_creditable = true;
		واپس 0;
	} ورنہ اگر (ret < 0 && errno == ENOSYS) {
		ساخت pollfd random_fd = {
			.fd = open("/dev/random", O_RDONLY),
			.events = POLLIN
		};
		اگر (random_fd.fd < 0)
			واپس -errno;
		*is_creditable = poll(&random_fd, 1, 0) == 1;
		close(random_fd.fd);
	} ورنہ اگر (getrandom_بھرا(بیج, len, GRND_INSECURE) == (ssize_t)len)
		واپس 0;
	urandom_fd = open("/dev/urandom", O_RDONLY);
	اگر (urandom_fd < 0)
		واپس -1;
	ret = read_بھرا(urandom_fd, بیج, len);
	اگر (ret == (ssize_t)len)
		ret = 0;
	ورنہ
		ret = -errno ? -errno : -EIO;
	close(urandom_fd);
	errno = -ret;
	واپس ret ? -1 : 0;
}

ساکن صحیح بیج_rng(uint8_t *بیج, size_t len, bool credit)
{
	ساخت {
		صحیح entropy_گنتی;
		صحیح buf_سائز;
		uint8_t بفر[MAX_بیج_LEN];
	} req = {
		.entropy_گنتی = credit ? len * 8 : 0,
		.buf_سائز = len
	};
	صحیح random_fd, ret;

	اگر (len > حجم(req.بفر)) {
		errno = EFBIG;
		واپس -1;
	}
	حافظہ_نقل(req.بفر, بیج, len);

	random_fd = open("/dev/urandom", O_RDONLY);
	اگر (random_fd < 0)
		واپس -1;
	ret = ioctl(random_fd, RNDADDENTROPY, &req);
	اگر (ret)
		ret = -errno ? -errno : -EIO;
	close(random_fd);
	errno = -ret;
	واپس ret ? -1 : 0;
}

ساکن صحیح بیج_from_file_if_exists(ثابت حرف *filename, صحیح dfd, bool credit, ساخت blake2s_state *hash)
{
	uint8_t بیج[MAX_بیج_LEN];
	ssize_t بیج_len;
	صحیح fd = -1, ret = 0;

	fd = openat(dfd, filename, O_RDONLY);
	اگر (fd < 0 && errno == ENOENT)
		واپس 0;
	ورنہ اگر (fd < 0) {
		ret = -errno;
		eerror("%s: Unable to open seed file: %s", applet, strerror(errno));
		جاؤ برآمد;
	}
	بیج_len = read_بھرا(fd, بیج, حجم(بیج));
	اگر (بیج_len < 0) {
		ret = -errno;
		eerror("%s: Unable to read seed file: %s", applet, strerror(errno));
		جاؤ برآمد;
	}
	اگر ((unlinkat(dfd, filename, 0) < 0 || fsync(dfd) < 0) && بیج_len) {
		ret = -errno;
		eerror("%s: Unable to remove seed after reading, so not seeding: %s",
		       applet, strerror(errno));
		جاؤ برآمد;
	}
	اگر (!بیج_len)
		جاؤ برآمد;

	blake2s_update(hash, &بیج_len, حجم(بیج_len));
	blake2s_update(hash, بیج, بیج_len);

	einfo("Seeding %zd bits %s crediting", بیج_len * 8, credit ? "and" : "without");
	اگر (بیج_rng(بیج, بیج_len, credit) < 0) {
		ret = -errno;
		eerror("%s: Unable to seed: %s", applet, strerror(errno));
	}

برآمد:
	اگر (fd >= 0)
		close(fd);
	errno = -ret;
	واپس ret ? -1 : 0;
}

صحیح مین(صحیح argc, حرف **argv)
{
	ساکن ثابت حرف seedrng_سابقہ[] = "SeedRNG v1 Old+New Prefix";
	ساکن ثابت حرف seedrng_failure[] = "SeedRNG v1 No New Seed Failure";
	صحیح opt, fd, dfd, پروگرام_ret = 0;
	ثابت حرف *بیج_dir = تہی;
	uint8_t new_بیج[MAX_بیج_LEN];
	size_t new_بیج_len;
	bool new_بیج_creditable;
	ساخت timespec realtime = { 0 }, boottime = { 0 };
	ساخت blake2s_state hash;
	bool skip_credit = false;

	applet = basename_c(argv[0]);

	جب ((opt = getopt_long(argc, argv, getoptstring, longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
		حالت LONGOPT_بیج_DIR:
			اگر (!بیج_dir)
				بیج_dir = optarg;
			توڑو;
		حالت LONGOPT_SKIP_CREDIT:
			skip_credit = true;
			توڑو;
		case_RC_COMMON_GETOPT
		}
	}
	اگر (!بیج_dir)
		بیج_dir = DEFAULT_بیج_DIR;
	اگر (getuid())
		eerrorx("%s: superuser access is required", applet);
	umask(0077);

	blake2s_init(&hash, BLAKE2S_HASH_LEN);
	blake2s_update(&hash, seedrng_سابقہ, لڑی_لمبائی(seedrng_سابقہ));
	clock_gettime(CLOCK_REALTIME, &realtime);
	clock_gettime(CLOCK_BOOTTIME, &boottime);
	blake2s_update(&hash, &realtime, حجم(realtime));
	blake2s_update(&hash, &boottime, حجم(boottime));

	اگر (mkdir(بیج_dir, 0700) < 0 && errno != EEXIST)
		eerrorx("%s: Unable to create seed directory: %s", applet, strerror(errno));

	dfd = open(بیج_dir, O_DIRECTORY | O_RDONLY);
	اگر (dfd < 0 || flock(dfd, LOCK_EX) < 0)
		eerrorx("%s: Unable to lock seed directory: %s", applet, strerror(errno));

	اگر (بیج_from_file_if_exists(NON_CREDITABLE_بیج, dfd, false, &hash) < 0)
		پروگرام_ret |= 1 << 1;
	اگر (بیج_from_file_if_exists(CREDITABLE_بیج, dfd, !skip_credit, &hash) < 0)
		پروگرام_ret |= 1 << 2;

	new_بیج_len = determine_optimal_بیج_len();
	اگر (read_new_بیج(new_بیج, new_بیج_len, &new_بیج_creditable) < 0) {
		eerror("%s: Unable to read new seed: %s", applet, strerror(errno));
		new_بیج_len = BLAKE2S_HASH_LEN;
		strncpy((حرف *)new_بیج, seedrng_failure, new_بیج_len);
		پروگرام_ret |= 1 << 3;
	}
	blake2s_update(&hash, &new_بیج_len, حجم(new_بیج_len));
	blake2s_update(&hash, new_بیج, new_بیج_len);
	blake2s_final(&hash, new_بیج + new_بیج_len - BLAKE2S_HASH_LEN);

	einfo("Saving %zu bits of %s seed for next boot", new_بیج_len * 8, new_بیج_creditable ? "creditable" : "non-creditable");
	fd = openat(dfd, NON_CREDITABLE_بیج, O_WRONLY | O_CREAT | O_TRUNC, 0400);
	اگر (fd < 0) {
		eerror("%s: Unable to open seed file for writing: %s", applet, strerror(errno));
		واپس پروگرام_ret | (1 << 4);
	}
	اگر (write_بھرا(fd, new_بیج, new_بیج_len) != (ssize_t)new_بیج_len || fsync(fd) < 0) {
		eerror("%s: Unable to write seed file: %s", applet, strerror(errno));
		واپس پروگرام_ret | (1 << 5);
	}
	اگر (new_بیج_creditable && renameat(dfd, NON_CREDITABLE_بیج, dfd, CREDITABLE_بیج) < 0) {
		ewarn("%s: Unable to make new seed creditable: %s", applet, strerror(errno));
		واپس پروگرام_ret | (1 << 6);
	}
	واپس پروگرام_ret;
}
