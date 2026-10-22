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

#ಅನ್ತರ್ಭಾವ <linux/random.h>
#ಅನ್ತರ್ಭಾವ <sys/random.h>
#ಅನ್ತರ್ಭಾವ <sys/ioctl.h>
#ಅನ್ತರ್ಭಾವ <sys/file.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <poll.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <endian.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"

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
ಗಣನಾ long_opts {
	/* This has to come first so following values stay in the 0x100+ range. */
	LONGOPT_BASE = 0x100,
	LONGOPT_SEED_DIR,
	LONGOPT_SKIP_CREDIT,
};

ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *extraopts = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ getoptstring[] = getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{ "seed-dir", 1, ರಿಕ್ತ, LONGOPT_SEED_DIR },
	{ "skip-credit", 0, ರಿಕ್ತ, LONGOPT_SKIP_CREDIT },
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[] = {
	"Directory for seed files (default: " DEFAULT_SEED_DIR ")",
	"Skip crediting entropy of seeds",
	longopts_help_COMMON
};
ಧ್ರುವ ವರ್ಣ *usagestring = ರಿಕ್ತ;

ಗಣನಾ blake2s_lengths {
	BLAKE2S_BLOCK_LEN = 64,
	BLAKE2S_HASH_LEN = 32,
	BLAKE2_S_ಕೀಲಿ_LEN = 32
};

ಗಣನಾ seedrng_lengths {
	MAX_SEED_LEN = 512,
	MIN_SEED_LEN = BLAKE2S_HASH_LEN
};

ಸಂರಚನಾ blake2s_state {
	uint32_t h[8];
	uint32_t t[2];
	uint32_t f[2];
	uint8_t buf[BLAKE2S_BLOCK_LEN];
	unsigned ಪೂರ್ಣಾಙ್ಕ buflen;
	unsigned ಪೂರ್ಣಾಙ್ಕ outlen;
};

#define le32_to_cpup(a) le32toh(*(a))
#define cpu_to_le32(a) htole32(a)
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (ಪರಿಮಾಣ(x) / ಪರಿಮಾಣ((x)[0]))
#endif
#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ cpu_to_le32_array(uint32_t *buf, unsigned ಪೂರ್ಣಾಙ್ಕ ಪದಗಳು)
{
	ಯಾವತ್ (ಪದಗಳು--) {
		*buf = cpu_to_le32(*buf);
		++buf;
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ le32_to_cpu_array(uint32_t *buf, unsigned ಪೂರ್ಣಾಙ್ಕ ಪದಗಳು)
{
	ಯಾವತ್ (ಪದಗಳು--) {
		*buf = le32_to_cpup(buf);
		++buf;
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ uint32_t ror32(uint32_t word, unsigned ಪೂರ್ಣಾಙ್ಕ shift)
{
	ಪ್ರತ್ಯಾಗಮ (word >> (shift & 31)) | (word << ((-shift) & 31));
}

ಸ್ಥಿರ ಧ್ರುವ uint32_t blake2s_iv[8] = {
	0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
	0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

ಸ್ಥಿರ ಧ್ರುವ uint8_t blake2s_sigma[10][16] = {
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

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_set_lastblock(ಸಂರಚನಾ blake2s_state *state)
{
	state->f[0] = -1;
}

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_increment_counter(ಸಂರಚನಾ blake2s_state *state, ಧ್ರುವ uint32_t inc)
{
	state->t[0] += inc;
	state->t[1] += (state->t[0] < inc);
}

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_init_param(ಸಂರಚನಾ blake2s_state *state, ಧ್ರುವ uint32_t param)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	memset(state, 0, ಪರಿಮಾಣ(*state));
	ಪ್ರತಿ (i = 0; i < 8; ++i)
		state->h[i] = blake2s_iv[i];
	state->h[0] ^= param;
}

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_init(ಸಂರಚನಾ blake2s_state *state, ಧ್ರುವ size_t outlen)
{
	blake2s_init_param(state, 0x01010000 | outlen);
	state->outlen = outlen;
}

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_compress(ಸಂರಚನಾ blake2s_state *state, ಧ್ರುವ uint8_t *block, size_t nblocks, ಧ್ರುವ uint32_t inc)
{
	uint32_t m[16];
	uint32_t v[16];
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯಾವತ್ (nblocks > 0) {
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

#define G(r, i, a, b, c, d) ಕುರು { \
	a += b + m[blake2s_sigma[r][2 * i + 0]]; \
	d = ror32(d ^ a, 16); \
	c += d; \
	b = ror32(b ^ c, 12); \
	a += b + m[blake2s_sigma[r][2 * i + 1]]; \
	d = ror32(d ^ a, 8); \
	c += d; \
	b = ror32(b ^ c, 7); \
} ಯಾವತ್ (0)

#define ROUND(r) ಕುರು { \
	G(r, 0, v[0], v[ 4], v[ 8], v[12]); \
	G(r, 1, v[1], v[ 5], v[ 9], v[13]); \
	G(r, 2, v[2], v[ 6], v[10], v[14]); \
	G(r, 3, v[3], v[ 7], v[11], v[15]); \
	G(r, 4, v[0], v[ 5], v[10], v[15]); \
	G(r, 5, v[1], v[ 6], v[11], v[12]); \
	G(r, 6, v[2], v[ 7], v[ 8], v[13]); \
	G(r, 7, v[3], v[ 4], v[ 9], v[14]); \
} ಯಾವತ್ (0)
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

		ಪ್ರತಿ (i = 0; i < 8; ++i)
			state->h[i] ^= v[i] ^ v[i + 8];

		block += BLAKE2S_BLOCK_LEN;
		--nblocks;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_update(ಸಂರಚನಾ blake2s_state *state, ಧ್ರುವ ಶೂನ್ಯ *inp, size_t inlen)
{
	ಧ್ರುವ size_t fill = BLAKE2S_BLOCK_LEN - state->buflen;
	ಧ್ರುವ uint8_t *in = inp;

	ಯದಿ (!inlen)
		ಪ್ರತ್ಯಾಗಮ;
	ಯದಿ (inlen > fill) {
		memcpy(state->buf + state->buflen, in, fill);
		blake2s_compress(state, state->buf, 1, BLAKE2S_BLOCK_LEN);
		state->buflen = 0;
		in += fill;
		inlen -= fill;
	}
	ಯದಿ (inlen > BLAKE2S_BLOCK_LEN) {
		ಧ್ರುವ size_t nblocks = DIV_ROUND_UP(inlen, BLAKE2S_BLOCK_LEN);
		blake2s_compress(state, in, nblocks - 1, BLAKE2S_BLOCK_LEN);
		in += BLAKE2S_BLOCK_LEN * (nblocks - 1);
		inlen -= BLAKE2S_BLOCK_LEN * (nblocks - 1);
	}
	memcpy(state->buf + state->buflen, in, inlen);
	state->buflen += inlen;
}

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_final(ಸಂರಚನಾ blake2s_state *state, uint8_t *out)
{
	blake2s_set_lastblock(state);
	memset(state->buf + state->buflen, 0, BLAKE2S_BLOCK_LEN - state->buflen);
	blake2s_compress(state, state->buf, 1, state->buflen);
	cpu_to_le32_array(state->h, ARRAY_SIZE(state->h));
	memcpy(out, state->h, state->outlen);
}

ಸ್ಥಿರ ssize_t getrandom_full(ಶೂನ್ಯ *buf, size_t ಎಣಿಕೆ, unsigned ಪೂರ್ಣಾಙ್ಕ flags)
{
	ssize_t ret, total = 0;
	uint8_t *p = buf;

	ಕುರು {
		ret = getrandom(p, ಎಣಿಕೆ, flags);
		ಯದಿ (ret < 0 && errno == EINTR)
			ಅನುವರ್ತ;
		ಅನ್ಯಥಾ ಯದಿ (ret < 0)
			ಪ್ರತ್ಯಾಗಮ ret;
		total += ret;
		p += ret;
		ಎಣಿಕೆ -= ret;
	} ಯಾವತ್ (ಎಣಿಕೆ);
	ಪ್ರತ್ಯಾಗಮ total;
}

ಸ್ಥಿರ ssize_t read_full(ಪೂರ್ಣಾಙ್ಕ fd, ಶೂನ್ಯ *buf, size_t ಎಣಿಕೆ)
{
	ssize_t ret, total = 0;
	uint8_t *p = buf;

	ಕುರು {
		ret = ಪಠ(fd, p, ಎಣಿಕೆ);
		ಯದಿ (ret < 0 && errno == EINTR)
			ಅನುವರ್ತ;
		ಅನ್ಯಥಾ ಯದಿ (ret < 0)
			ಪ್ರತ್ಯಾಗಮ ret;
		ಅನ್ಯಥಾ ಯದಿ (ret == 0)
			ಭಙ್ಗ;
		total += ret;
		p += ret;
		ಎಣಿಕೆ -= ret;
	} ಯಾವತ್ (ಎಣಿಕೆ);
	ಪ್ರತ್ಯಾಗಮ total;
}

ಸ್ಥಿರ ssize_t write_full(ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ ಶೂನ್ಯ *buf, size_t ಎಣಿಕೆ)
{
	ssize_t ret, total = 0;
	ಧ್ರುವ uint8_t *p = buf;

	ಕುರು {
		ret = ಲಿಖ(fd, p, ಎಣಿಕೆ);
		ಯದಿ (ret < 0 && errno == EINTR)
			ಅನುವರ್ತ;
		ಅನ್ಯಥಾ ಯದಿ (ret < 0)
			ಪ್ರತ್ಯಾಗಮ ret;
		total += ret;
		p += ret;
		ಎಣಿಕೆ -= ret;
	} ಯಾವತ್ (ಎಣಿಕೆ);
	ಪ್ರತ್ಯಾಗಮ total;
}

ಸ್ಥಿರ size_t determine_optimal_seed_len(ಶೂನ್ಯ)
{
	size_t ret = 0;
	ವರ್ಣ poolsize_str[11] = { 0 };
	ಪೂರ್ಣಾಙ್ಕ fd = open("/proc/sys/kernel/random/poolsize", O_RDONLY);

	ಯದಿ (fd < 0 || read_full(fd, poolsize_str, ಪರಿಮಾಣ(poolsize_str) - 1) < 0) {
		ewarn("%s: Unable to determine pool size, falling back to %u bits: %s",
		      applet, MIN_SEED_LEN * 8, strerror(errno));
		ret = MIN_SEED_LEN;
	} ಅನ್ಯಥಾ
		ret = DIV_ROUND_UP(strtoul(poolsize_str, ರಿಕ್ತ, 10), 8);
	ಯದಿ (fd >= 0)
		close(fd);
	ಯದಿ (ret < MIN_SEED_LEN)
		ret = MIN_SEED_LEN;
	ಅನ್ಯಥಾ ಯದಿ (ret > MAX_SEED_LEN)
		ret = MAX_SEED_LEN;
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ read_new_seed(uint8_t *seed, size_t len, bool *is_creditable)
{
	ssize_t ret;
	ಪೂರ್ಣಾಙ್ಕ urandom_fd;

	*is_creditable = false;
	ret = getrandom_full(seed, len, GRND_NONBLOCK);
	ಯದಿ (ret == (ssize_t)len) {
		*is_creditable = true;
		ಪ್ರತ್ಯಾಗಮ 0;
	} ಅನ್ಯಥಾ ಯದಿ (ret < 0 && errno == ENOSYS) {
		ಸಂರಚನಾ pollfd random_fd = {
			.fd = open("/dev/random", O_RDONLY),
			.events = POLLIN
		};
		ಯದಿ (random_fd.fd < 0)
			ಪ್ರತ್ಯಾಗಮ -errno;
		*is_creditable = poll(&random_fd, 1, 0) == 1;
		close(random_fd.fd);
	} ಅನ್ಯಥಾ ಯದಿ (getrandom_full(seed, len, GRND_INSECURE) == (ssize_t)len)
		ಪ್ರತ್ಯಾಗಮ 0;
	urandom_fd = open("/dev/urandom", O_RDONLY);
	ಯದಿ (urandom_fd < 0)
		ಪ್ರತ್ಯಾಗಮ -1;
	ret = read_full(urandom_fd, seed, len);
	ಯದಿ (ret == (ssize_t)len)
		ret = 0;
	ಅನ್ಯಥಾ
		ret = -errno ? -errno : -EIO;
	close(urandom_fd);
	errno = -ret;
	ಪ್ರತ್ಯಾಗಮ ret ? -1 : 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ seed_rng(uint8_t *seed, size_t len, bool credit)
{
	ಸಂರಚನಾ {
		ಪೂರ್ಣಾಙ್ಕ entropy_ಎಣಿಕೆ;
		ಪೂರ್ಣಾಙ್ಕ buf_size;
		uint8_t buffer[MAX_SEED_LEN];
	} req = {
		.entropy_ಎಣಿಕೆ = credit ? len * 8 : 0,
		.buf_size = len
	};
	ಪೂರ್ಣಾಙ್ಕ random_fd, ret;

	ಯದಿ (len > ಪರಿಮಾಣ(req.buffer)) {
		errno = EFBIG;
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	memcpy(req.buffer, seed, len);

	random_fd = open("/dev/urandom", O_RDONLY);
	ಯದಿ (random_fd < 0)
		ಪ್ರತ್ಯಾಗಮ -1;
	ret = ioctl(random_fd, RNDADDENTROPY, &req);
	ಯದಿ (ret)
		ret = -errno ? -errno : -EIO;
	close(random_fd);
	errno = -ret;
	ಪ್ರತ್ಯಾಗಮ ret ? -1 : 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ seed_from_file_if_exists(ಧ್ರುವ ವರ್ಣ *filename, ಪೂರ್ಣಾಙ್ಕ dfd, bool credit, ಸಂರಚನಾ blake2s_state *hash)
{
	uint8_t seed[MAX_SEED_LEN];
	ssize_t seed_len;
	ಪೂರ್ಣಾಙ್ಕ fd = -1, ret = 0;

	fd = openat(dfd, filename, O_RDONLY);
	ಯದಿ (fd < 0 && errno == ENOENT)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಅನ್ಯಥಾ ಯದಿ (fd < 0) {
		ret = -errno;
		eerror("%s: Unable to open seed file: %s", applet, strerror(errno));
		ಗಚ್ಛ out;
	}
	seed_len = read_full(fd, seed, ಪರಿಮಾಣ(seed));
	ಯದಿ (seed_len < 0) {
		ret = -errno;
		eerror("%s: Unable to read seed file: %s", applet, strerror(errno));
		ಗಚ್ಛ out;
	}
	ಯದಿ ((unlinkat(dfd, filename, 0) < 0 || fsync(dfd) < 0) && seed_len) {
		ret = -errno;
		eerror("%s: Unable to remove seed after reading, so not seeding: %s",
		       applet, strerror(errno));
		ಗಚ್ಛ out;
	}
	ಯದಿ (!seed_len)
		ಗಚ್ಛ out;

	blake2s_update(hash, &seed_len, ಪರಿಮಾಣ(seed_len));
	blake2s_update(hash, seed, seed_len);

	einfo("Seeding %zd bits %s crediting", seed_len * 8, credit ? "and" : "without");
	ಯದಿ (seed_rng(seed, seed_len, credit) < 0) {
		ret = -errno;
		eerror("%s: Unable to seed: %s", applet, strerror(errno));
	}

out:
	ಯದಿ (fd >= 0)
		close(fd);
	errno = -ret;
	ಪ್ರತ್ಯಾಗಮ ret ? -1 : 0;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ seedrng_prefix[] = "SeedRNG v1 Old+New Prefix";
	ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ seedrng_failure[] = "SeedRNG v1 No New Seed Failure";
	ಪೂರ್ಣಾಙ್ಕ opt, fd, dfd, program_ret = 0;
	ಧ್ರುವ ವರ್ಣ *seed_dir = ರಿಕ್ತ;
	uint8_t new_seed[MAX_SEED_LEN];
	size_t new_seed_len;
	bool new_seed_creditable;
	ಸಂರಚನಾ timespec realtime = { 0 }, boottime = { 0 };
	ಸಂರಚನಾ blake2s_state hash;
	bool skip_credit = false;

	applet = basename_c(argv[0]);

	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring, longopts, (ಪೂರ್ಣಾಙ್ಕ *) 0)) != -1)
	{
		ವಿಕಲ್ಪ (opt) {
		ಪಕ್ಷ LONGOPT_SEED_DIR:
			ಯದಿ (!seed_dir)
				seed_dir = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ LONGOPT_SKIP_CREDIT:
			skip_credit = true;
			ಭಙ್ಗ;
		case_RC_COMMON_GETOPT
		}
	}
	ಯದಿ (!seed_dir)
		seed_dir = DEFAULT_SEED_DIR;
	ಯದಿ (getuid())
		eerrorx("%s: superuser access is required", applet);
	umask(0077);

	blake2s_init(&hash, BLAKE2S_HASH_LEN);
	blake2s_update(&hash, seedrng_prefix, strlen(seedrng_prefix));
	clock_gettime(CLOCK_REALTIME, &realtime);
	clock_gettime(CLOCK_BOOTTIME, &boottime);
	blake2s_update(&hash, &realtime, ಪರಿಮಾಣ(realtime));
	blake2s_update(&hash, &boottime, ಪರಿಮಾಣ(boottime));

	ಯದಿ (mkdir(seed_dir, 0700) < 0 && errno != EEXIST)
		eerrorx("%s: Unable to create seed directory: %s", applet, strerror(errno));

	dfd = open(seed_dir, O_DIRECTORY | O_RDONLY);
	ಯದಿ (dfd < 0 || flock(dfd, LOCK_EX) < 0)
		eerrorx("%s: Unable to lock seed directory: %s", applet, strerror(errno));

	ಯದಿ (seed_from_file_if_exists(NON_CREDITABLE_SEED, dfd, false, &hash) < 0)
		program_ret |= 1 << 1;
	ಯದಿ (seed_from_file_if_exists(CREDITABLE_SEED, dfd, !skip_credit, &hash) < 0)
		program_ret |= 1 << 2;

	new_seed_len = determine_optimal_seed_len();
	ಯದಿ (read_new_seed(new_seed, new_seed_len, &new_seed_creditable) < 0) {
		eerror("%s: Unable to read new seed: %s", applet, strerror(errno));
		new_seed_len = BLAKE2S_HASH_LEN;
		strncpy((ವರ್ಣ *)new_seed, seedrng_failure, new_seed_len);
		program_ret |= 1 << 3;
	}
	blake2s_update(&hash, &new_seed_len, ಪರಿಮಾಣ(new_seed_len));
	blake2s_update(&hash, new_seed, new_seed_len);
	blake2s_final(&hash, new_seed + new_seed_len - BLAKE2S_HASH_LEN);

	einfo("Saving %zu bits of %s seed for next boot", new_seed_len * 8, new_seed_creditable ? "creditable" : "non-creditable");
	fd = openat(dfd, NON_CREDITABLE_SEED, O_WRONLY | O_CREAT | O_TRUNC, 0400);
	ಯದಿ (fd < 0) {
		eerror("%s: Unable to open seed file for writing: %s", applet, strerror(errno));
		ಪ್ರತ್ಯಾಗಮ program_ret | (1 << 4);
	}
	ಯದಿ (write_full(fd, new_seed, new_seed_len) != (ssize_t)new_seed_len || fsync(fd) < 0) {
		eerror("%s: Unable to write seed file: %s", applet, strerror(errno));
		ಪ್ರತ್ಯಾಗಮ program_ret | (1 << 5);
	}
	ಯದಿ (new_seed_creditable && renameat(dfd, NON_CREDITABLE_SEED, dfd, CREDITABLE_SEED) < 0) {
		ewarn("%s: Unable to make new seed creditable: %s", applet, strerror(errno));
		ಪ್ರತ್ಯಾಗಮ program_ret | (1 << 6);
	}
	ಪ್ರತ್ಯಾಗಮ program_ret;
}
