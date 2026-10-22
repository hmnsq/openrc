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

#அந்தர்பாவ <linux/random.h>
#அந்தர்பாவ <sys/random.h>
#அந்தர்பாவ <sys/ioctl.h>
#அந்தர்பாவ <sys/file.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <poll.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <endian.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "helpers.h"
#அந்தர்பாவ "_usage.h"

#ifndef GRND_INSECURE
#define GRND_INSECURE 0x0004 /* Apparently some headers don't ship with this yet. */
#endif

#define DEFAULT_SEED_DIR "/var/lib/seedrng"
#define CREDITABLE_SEED "seed.credit"
#define NON_CREDITABLE_SEED "seed.no-credit"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
எண்ணிக்கை long_opts {
	/* This has to come first so following values stay in the 0x100+ range. */
	LONGOPT_BASE = 0x100,
	LONGOPT_SEED_DIR,
	LONGOPT_SKIP_CREDIT,
};

மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *extraopts = வெற்று;
மாறிலி எழுத்து getoptstring[] = getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "seed-dir", 1, வெற்று, LONGOPT_SEED_DIR },
	{ "skip-credit", 0, வெற்று, LONGOPT_SKIP_CREDIT },
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
	"Directory for seed files (default: " DEFAULT_SEED_DIR ")",
	"Skip crediting entropy of seeds",
	longopts_help_COMMON
};
மாறிலி எழுத்து *usagestring = வெற்று;

எண்ணிக்கை blake2s_lengths {
	BLAKE2S_BLOCK_LEN = 64,
	BLAKE2S_HASH_LEN = 32,
	BLAKE2_S_சாவி_LEN = 32
};

எண்ணிக்கை seedrng_lengths {
	MAX_SEED_LEN = 512,
	MIN_SEED_LEN = BLAKE2S_HASH_LEN
};

கட்டமைப்பு blake2s_state {
	uint32_t h[8];
	uint32_t t[2];
	uint32_t f[2];
	uint8_t buf[BLAKE2S_BLOCK_LEN];
	unsigned முழு buflen;
	unsigned முழு outlen;
};

#define le32_to_cpup(a) le32toh(*(a))
#define cpu_to_le32(a) htole32(a)
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (அளவு(x) / அளவு((x)[0]))
#endif
#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

நிலையான உள்வரி வெறுமை cpu_to_le32_array(uint32_t *buf, unsigned முழு சொற்கள்)
{
	போது (சொற்கள்--) {
		*buf = cpu_to_le32(*buf);
		++buf;
	}
}

நிலையான உள்வரி வெறுமை le32_to_cpu_array(uint32_t *buf, unsigned முழு சொற்கள்)
{
	போது (சொற்கள்--) {
		*buf = le32_to_cpup(buf);
		++buf;
	}
}

நிலையான உள்வரி uint32_t ror32(uint32_t word, unsigned முழு shift)
{
	திரும்பு (word >> (shift & 31)) | (word << ((-shift) & 31));
}

நிலையான மாறிலி uint32_t blake2s_iv[8] = {
	0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
	0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

நிலையான மாறிலி uint8_t blake2s_sigma[10][16] = {
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

நிலையான வெறுமை blake2s_set_lastblock(கட்டமைப்பு blake2s_state *state)
{
	state->f[0] = -1;
}

நிலையான வெறுமை blake2s_increment_counter(கட்டமைப்பு blake2s_state *state, மாறிலி uint32_t inc)
{
	state->t[0] += inc;
	state->t[1] += (state->t[0] < inc);
}

நிலையான வெறுமை blake2s_init_param(கட்டமைப்பு blake2s_state *state, மாறிலி uint32_t param)
{
	முழு i;

	memset(state, 0, அளவு(*state));
	க்கு (i = 0; i < 8; ++i)
		state->h[i] = blake2s_iv[i];
	state->h[0] ^= param;
}

நிலையான வெறுமை blake2s_init(கட்டமைப்பு blake2s_state *state, மாறிலி size_t outlen)
{
	blake2s_init_param(state, 0x01010000 | outlen);
	state->outlen = outlen;
}

நிலையான வெறுமை blake2s_compress(கட்டமைப்பு blake2s_state *state, மாறிலி uint8_t *block, size_t nblocks, மாறிலி uint32_t inc)
{
	uint32_t m[16];
	uint32_t v[16];
	முழு i;

	போது (nblocks > 0) {
		blake2s_increment_counter(state, inc);
		memcpy(m, block, BLAKE2S_BLOCK_LEN);
		le32_to_cpu_array(m, ARRAY_SIZE(m));
		memcpy(v, state->h, 32);
		v[ 8] = blake2s_iv[0];
		v[ 9] = blake2s_iv[1];
		v[10] = blake2s_iv[2];
		v[11] = blake2s_iv[3];
		v[12] = blake2s_iv[4] ^ state->t[0];
		v[13] = blake2s_iv[5] ^ state->t[1];
		v[14] = blake2s_iv[6] ^ state->f[0];
		v[15] = blake2s_iv[7] ^ state->f[1];

#define G(r, i, a, b, c, d) செய் { \
	a += b + m[blake2s_sigma[r][2 * i + 0]]; \
	d = ror32(d ^ a, 16); \
	c += d; \
	b = ror32(b ^ c, 12); \
	a += b + m[blake2s_sigma[r][2 * i + 1]]; \
	d = ror32(d ^ a, 8); \
	c += d; \
	b = ror32(b ^ c, 7); \
} போது (0)

#define ROUND(r) செய் { \
	G(r, 0, v[0], v[ 4], v[ 8], v[12]); \
	G(r, 1, v[1], v[ 5], v[ 9], v[13]); \
	G(r, 2, v[2], v[ 6], v[10], v[14]); \
	G(r, 3, v[3], v[ 7], v[11], v[15]); \
	G(r, 4, v[0], v[ 5], v[10], v[15]); \
	G(r, 5, v[1], v[ 6], v[11], v[12]); \
	G(r, 6, v[2], v[ 7], v[ 8], v[13]); \
	G(r, 7, v[3], v[ 4], v[ 9], v[14]); \
} போது (0)
		ROUND(0);
		ROUND(1);
		ROUND(2);
		ROUND(3);
		ROUND(4);
		ROUND(5);
		ROUND(6);
		ROUND(7);
		ROUND(8);
		ROUND(9);

#undef G
#undef ROUND

		க்கு (i = 0; i < 8; ++i)
			state->h[i] ^= v[i] ^ v[i + 8];

		block += BLAKE2S_BLOCK_LEN;
		--nblocks;
	}
}

நிலையான வெறுமை blake2s_update(கட்டமைப்பு blake2s_state *state, மாறிலி வெறுமை *inp, size_t inlen)
{
	மாறிலி size_t fill = BLAKE2S_BLOCK_LEN - state->buflen;
	மாறிலி uint8_t *in = inp;

	எனில் (!inlen)
		திரும்பு;
	எனில் (inlen > fill) {
		memcpy(state->buf + state->buflen, in, fill);
		blake2s_compress(state, state->buf, 1, BLAKE2S_BLOCK_LEN);
		state->buflen = 0;
		in += fill;
		inlen -= fill;
	}
	எனில் (inlen > BLAKE2S_BLOCK_LEN) {
		மாறிலி size_t nblocks = DIV_ROUND_UP(inlen, BLAKE2S_BLOCK_LEN);
		blake2s_compress(state, in, nblocks - 1, BLAKE2S_BLOCK_LEN);
		in += BLAKE2S_BLOCK_LEN * (nblocks - 1);
		inlen -= BLAKE2S_BLOCK_LEN * (nblocks - 1);
	}
	memcpy(state->buf + state->buflen, in, inlen);
	state->buflen += inlen;
}

நிலையான வெறுமை blake2s_final(கட்டமைப்பு blake2s_state *state, uint8_t *out)
{
	blake2s_set_lastblock(state);
	memset(state->buf + state->buflen, 0, BLAKE2S_BLOCK_LEN - state->buflen);
	blake2s_compress(state, state->buf, 1, state->buflen);
	cpu_to_le32_array(state->h, ARRAY_SIZE(state->h));
	memcpy(out, state->h, state->outlen);
}

நிலையான ssize_t getrandom_full(வெறுமை *buf, size_t மொத்தம், unsigned முழு flags)
{
	ssize_t ret, total = 0;
	uint8_t *p = buf;

	செய் {
		ret = getrandom(p, மொத்தம், flags);
		எனில் (ret < 0 && errno == EINTR)
			தொடர்;
		இல்லையெனில் எனில் (ret < 0)
			திரும்பு ret;
		total += ret;
		p += ret;
		மொத்தம் -= ret;
	} போது (மொத்தம்);
	திரும்பு total;
}

நிலையான ssize_t read_full(முழு fd, வெறுமை *buf, size_t மொத்தம்)
{
	ssize_t ret, total = 0;
	uint8_t *p = buf;

	செய் {
		ret = பட(fd, p, மொத்தம்);
		எனில் (ret < 0 && errno == EINTR)
			தொடர்;
		இல்லையெனில் எனில் (ret < 0)
			திரும்பு ret;
		இல்லையெனில் எனில் (ret == 0)
			நிறுத்து;
		total += ret;
		p += ret;
		மொத்தம் -= ret;
	} போது (மொத்தம்);
	திரும்பு total;
}

நிலையான ssize_t write_full(முழு fd, மாறிலி வெறுமை *buf, size_t மொத்தம்)
{
	ssize_t ret, total = 0;
	மாறிலி uint8_t *p = buf;

	செய் {
		ret = லிக(fd, p, மொத்தம்);
		எனில் (ret < 0 && errno == EINTR)
			தொடர்;
		இல்லையெனில் எனில் (ret < 0)
			திரும்பு ret;
		total += ret;
		p += ret;
		மொத்தம் -= ret;
	} போது (மொத்தம்);
	திரும்பு total;
}

நிலையான size_t determine_optimal_seed_len(வெறுமை)
{
	size_t ret = 0;
	எழுத்து poolsize_str[11] = { 0 };
	முழு fd = open("/proc/sys/kernel/random/poolsize", O_RDONLY);

	எனில் (fd < 0 || read_full(fd, poolsize_str, அளவு(poolsize_str) - 1) < 0) {
		ewarn("%s: Unable to determine pool size, falling back to %u bits: %s",
		      applet, MIN_SEED_LEN * 8, strerror(errno));
		ret = MIN_SEED_LEN;
	} இல்லையெனில்
		ret = DIV_ROUND_UP(strtoul(poolsize_str, வெற்று, 10), 8);
	எனில் (fd >= 0)
		close(fd);
	எனில் (ret < MIN_SEED_LEN)
		ret = MIN_SEED_LEN;
	இல்லையெனில் எனில் (ret > MAX_SEED_LEN)
		ret = MAX_SEED_LEN;
	திரும்பு ret;
}

நிலையான முழு read_new_seed(uint8_t *seed, size_t len, bool *is_creditable)
{
	ssize_t ret;
	முழு urandom_fd;

	*is_creditable = false;
	ret = getrandom_full(seed, len, GRND_NONBLOCK);
	எனில் (ret == (ssize_t)len) {
		*is_creditable = true;
		திரும்பு 0;
	} இல்லையெனில் எனில் (ret < 0 && errno == ENOSYS) {
		கட்டமைப்பு pollfd random_fd = {
			.fd = open("/dev/random", O_RDONLY),
			.events = POLLIN
		};
		எனில் (random_fd.fd < 0)
			திரும்பு -errno;
		*is_creditable = poll(&random_fd, 1, 0) == 1;
		close(random_fd.fd);
	} இல்லையெனில் எனில் (getrandom_full(seed, len, GRND_INSECURE) == (ssize_t)len)
		திரும்பு 0;
	urandom_fd = open("/dev/urandom", O_RDONLY);
	எனில் (urandom_fd < 0)
		திரும்பு -1;
	ret = read_full(urandom_fd, seed, len);
	எனில் (ret == (ssize_t)len)
		ret = 0;
	இல்லையெனில்
		ret = -errno ? -errno : -EIO;
	close(urandom_fd);
	errno = -ret;
	திரும்பு ret ? -1 : 0;
}

நிலையான முழு seed_rng(uint8_t *seed, size_t len, bool credit)
{
	கட்டமைப்பு {
		முழு entropy_மொத்தம்;
		முழு buf_size;
		uint8_t buffer[MAX_SEED_LEN];
	} req = {
		.entropy_மொத்தம் = credit ? len * 8 : 0,
		.buf_size = len
	};
	முழு random_fd, ret;

	எனில் (len > அளவு(req.buffer)) {
		errno = EFBIG;
		திரும்பு -1;
	}
	memcpy(req.buffer, seed, len);

	random_fd = open("/dev/urandom", O_RDONLY);
	எனில் (random_fd < 0)
		திரும்பு -1;
	ret = ioctl(random_fd, RNDADDENTROPY, &req);
	எனில் (ret)
		ret = -errno ? -errno : -EIO;
	close(random_fd);
	errno = -ret;
	திரும்பு ret ? -1 : 0;
}

நிலையான முழு seed_from_file_if_exists(மாறிலி எழுத்து *filename, முழு dfd, bool credit, கட்டமைப்பு blake2s_state *hash)
{
	uint8_t seed[MAX_SEED_LEN];
	ssize_t seed_len;
	முழு fd = -1, ret = 0;

	fd = openat(dfd, filename, O_RDONLY);
	எனில் (fd < 0 && errno == ENOENT)
		திரும்பு 0;
	இல்லையெனில் எனில் (fd < 0) {
		ret = -errno;
		eerror("%s: Unable to open seed file: %s", applet, strerror(errno));
		செல் out;
	}
	seed_len = read_full(fd, seed, அளவு(seed));
	எனில் (seed_len < 0) {
		ret = -errno;
		eerror("%s: Unable to read seed file: %s", applet, strerror(errno));
		செல் out;
	}
	எனில் ((unlinkat(dfd, filename, 0) < 0 || fsync(dfd) < 0) && seed_len) {
		ret = -errno;
		eerror("%s: Unable to remove seed after reading, so not seeding: %s",
		       applet, strerror(errno));
		செல் out;
	}
	எனில் (!seed_len)
		செல் out;

	blake2s_update(hash, &seed_len, அளவு(seed_len));
	blake2s_update(hash, seed, seed_len);

	einfo("Seeding %zd bits %s crediting", seed_len * 8, credit ? "and" : "without");
	எனில் (seed_rng(seed, seed_len, credit) < 0) {
		ret = -errno;
		eerror("%s: Unable to seed: %s", applet, strerror(errno));
	}

out:
	எனில் (fd >= 0)
		close(fd);
	errno = -ret;
	திரும்பு ret ? -1 : 0;
}

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	நிலையான மாறிலி எழுத்து seedrng_prefix[] = "SeedRNG v1 Old+New Prefix";
	நிலையான மாறிலி எழுத்து seedrng_failure[] = "SeedRNG v1 No New Seed Failure";
	முழு opt, fd, dfd, program_ret = 0;
	மாறிலி எழுத்து *seed_dir = வெற்று;
	uint8_t new_seed[MAX_SEED_LEN];
	size_t new_seed_len;
	bool new_seed_creditable;
	கட்டமைப்பு timespec realtime = { 0 }, boottime = { 0 };
	கட்டமைப்பு blake2s_state hash;
	bool skip_credit = false;

	applet = basename_c(argv[0]);

	போது ((opt = getopt_long(argc, argv, getoptstring, longopts, (முழு *) 0)) != -1)
	{
		தேர்வு (opt) {
		நிலை LONGOPT_SEED_DIR:
			எனில் (!seed_dir)
				seed_dir = optarg;
			நிறுத்து;
		நிலை LONGOPT_SKIP_CREDIT:
			skip_credit = true;
			நிறுத்து;
		case_RC_COMMON_GETOPT
		}
	}
	எனில் (!seed_dir)
		seed_dir = DEFAULT_SEED_DIR;
	எனில் (getuid())
		eerrorx("%s: superuser access is required", applet);
	umask(0077);

	blake2s_init(&hash, BLAKE2S_HASH_LEN);
	blake2s_update(&hash, seedrng_prefix, strlen(seedrng_prefix));
	clock_gettime(CLOCK_REALTIME, &realtime);
	clock_gettime(CLOCK_BOOTTIME, &boottime);
	blake2s_update(&hash, &realtime, அளவு(realtime));
	blake2s_update(&hash, &boottime, அளவு(boottime));

	எனில் (mkdir(seed_dir, 0700) < 0 && errno != EEXIST)
		eerrorx("%s: Unable to create seed directory: %s", applet, strerror(errno));

	dfd = open(seed_dir, O_DIRECTORY | O_RDONLY);
	எனில் (dfd < 0 || flock(dfd, LOCK_EX) < 0)
		eerrorx("%s: Unable to lock seed directory: %s", applet, strerror(errno));

	எனில் (seed_from_file_if_exists(NON_CREDITABLE_SEED, dfd, false, &hash) < 0)
		program_ret |= 1 << 1;
	எனில் (seed_from_file_if_exists(CREDITABLE_SEED, dfd, !skip_credit, &hash) < 0)
		program_ret |= 1 << 2;

	new_seed_len = determine_optimal_seed_len();
	எனில் (read_new_seed(new_seed, new_seed_len, &new_seed_creditable) < 0) {
		eerror("%s: Unable to read new seed: %s", applet, strerror(errno));
		new_seed_len = BLAKE2S_HASH_LEN;
		strncpy((எழுத்து *)new_seed, seedrng_failure, new_seed_len);
		program_ret |= 1 << 3;
	}
	blake2s_update(&hash, &new_seed_len, அளவு(new_seed_len));
	blake2s_update(&hash, new_seed, new_seed_len);
	blake2s_final(&hash, new_seed + new_seed_len - BLAKE2S_HASH_LEN);

	einfo("Saving %zu bits of %s seed for next boot", new_seed_len * 8, new_seed_creditable ? "creditable" : "non-creditable");
	fd = openat(dfd, NON_CREDITABLE_SEED, O_WRONLY | O_CREAT | O_TRUNC, 0400);
	எனில் (fd < 0) {
		eerror("%s: Unable to open seed file for writing: %s", applet, strerror(errno));
		திரும்பு program_ret | (1 << 4);
	}
	எனில் (write_full(fd, new_seed, new_seed_len) != (ssize_t)new_seed_len || fsync(fd) < 0) {
		eerror("%s: Unable to write seed file: %s", applet, strerror(errno));
		திரும்பு program_ret | (1 << 5);
	}
	எனில் (new_seed_creditable && renameat(dfd, NON_CREDITABLE_SEED, dfd, CREDITABLE_SEED) < 0) {
		ewarn("%s: Unable to make new seed creditable: %s", applet, strerror(errno));
		திரும்பு program_ret | (1 << 6);
	}
	திரும்பு program_ret;
}
