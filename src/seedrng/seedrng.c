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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ GRND_INSECURE
#ವ್ಯಾಖ್ಯಾನಿಸು GRND_INSECURE 0x0004 /* Apparently some headers don't ship with this yet. */
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು DEFAULT_ಬೀಜ_DIR "/var/lib/seedrng"
#ವ್ಯಾಖ್ಯಾನಿಸು CREDITABLE_ಬೀಜ "seed.credit"
#ವ್ಯಾಖ್ಯಾನಿಸು NON_CREDITABLE_ಬೀಜ "seed.no-credit"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ಎಣಿಕೆ long_opts {
	/* This has to come first so following values stay in the 0x100+ range. */
	LONGOPT_ತಳ = 0x100,
	LONGOPT_ಬೀಜ_DIR,
	LONGOPT_SKIP_CREDIT,
};

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{ "seed-dir", 1, ಖಾಲಿ, LONGOPT_ಬೀಜ_DIR },
	{ "skip-credit", 0, ಖಾಲಿ, LONGOPT_SKIP_CREDIT },
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[] = {
	"Directory for seed files (default: " DEFAULT_ಬೀಜ_DIR ")",
	"Skip crediting entropy of seeds",
	longopts_help_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ಖಾಲಿ;

ಎಣಿಕೆ blake2s_lengths {
	BLAKE2_S_ಖಂಡ_LEN = 64,
	BLAKE2S_HASH_LEN = 32,
	BLAKE2_S_ಕೀಲಿ_LEN = 32
};

ಎಣಿಕೆ seedrng_lengths {
	MAX_ಬೀಜ_LEN = 512,
	MIN_ಬೀಜ_LEN = BLAKE2S_HASH_LEN
};

ರಚನೆ blake2s_state {
	uint32_t h[8];
	uint32_t t[2];
	uint32_t f[2];
	uint8_t buf[BLAKE2_S_ಖಂಡ_LEN];
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ buflen;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ outlen;
};

#ವ್ಯಾಖ್ಯಾನಿಸು le32_ಗಮ್ಯ_cpup(a) le32toh(*(a))
#ವ್ಯಾಖ್ಯಾನಿಸು cpu_ಗಮ್ಯ_le32(a) htole32(a)
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ARRAY_ಗಾತ್ರ
#ವ್ಯಾಖ್ಯಾನಿಸು ARRAY_ಗಾತ್ರ(x) (ಗಾತ್ರ(x) / ಗಾತ್ರ((x)[0]))
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ DIV_ಸುತ್ತು_ಮೇಲೆ
#ವ್ಯಾಖ್ಯಾನಿಸು DIV_ಸುತ್ತು_ಮೇಲೆ(n, d) (((n) + (d) - 1) / (d))
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ cpu_ಗಮ್ಯ_le32_array(uint32_t *buf, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಪದಗಳು)
{
	ಆಗ (ಪದಗಳು--) {
		*buf = cpu_ಗಮ್ಯ_le32(*buf);
		++buf;
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ le32_ಗಮ್ಯ_cpu_array(uint32_t *buf, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಪದಗಳು)
{
	ಆಗ (ಪದಗಳು--) {
		*buf = le32_ಗಮ್ಯ_cpup(buf);
		++buf;
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint32_t ror32(uint32_t ಶಬ್ದ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ shift)
{
	ಹಿಂತಿರುಗಿ (ಶಬ್ದ >> (shift & 31)) | (ಶಬ್ದ << ((-shift) & 31));
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint32_t blake2s_iv[8] = {
	0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
	0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t blake2s_sigma[10][16] = {
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

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_ಗಣ_lastblock(ರಚನೆ blake2s_state *state)
{
	state->f[0] = -1;
}

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_increment_counter(ರಚನೆ blake2s_state *state, ಸ್ಥಿರಾಂಕ uint32_t inc)
{
	state->t[0] += inc;
	state->t[1] += (state->t[0] < inc);
}

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_init_param(ರಚನೆ blake2s_state *state, ಸ್ಥಿರಾಂಕ uint32_t param)
{
	ಪೂರ್ಣಾಂಕ i;

	memset(state, 0, ಗಾತ್ರ(*state));
	ಗಾಗಿ (i = 0; i < 8; ++i)
		state->h[i] = blake2s_iv[i];
	state->h[0] ^= param;
}

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_init(ರಚನೆ blake2s_state *state, ಸ್ಥಿರಾಂಕ size_t outlen)
{
	blake2s_init_param(state, 0x01010000 | outlen);
	state->outlen = outlen;
}

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_compress(ರಚನೆ blake2s_state *state, ಸ್ಥಿರಾಂಕ uint8_t *ಖಂಡ, size_t nblocks, ಸ್ಥಿರಾಂಕ uint32_t inc)
{
	uint32_t m[16];
	uint32_t v[16];
	ಪೂರ್ಣಾಂಕ i;

	ಆಗ (nblocks > 0) {
		blake2s_increment_counter(state, inc);
		ಸ್ಮರಣೆ_ನಕಲು(m, ಖಂಡ, BLAKE2_S_ಖಂಡ_LEN);
		le32_ಗಮ್ಯ_cpu_array(m, ARRAY_ಗಾತ್ರ(m));
		ಸ್ಮರಣೆ_ನಕಲು(v, state->h, 32);
		v[ 8] = blake2s_iv[0];
		v[ 9] = blake2s_iv[1];
		v[10] = blake2s_iv[2];
		v[11] = blake2s_iv[3];
		v[12] = blake2s_iv[4] ^ state->t[0];
		v[13] = blake2s_iv[5] ^ state->t[1];
		v[14] = blake2s_iv[6] ^ state->f[0];
		v[15] = blake2s_iv[7] ^ state->f[1];

#ವ್ಯಾಖ್ಯಾನಿಸು G(r, i, a, b, c, d) ಮಾಡು { \
	a += b + m[blake2s_sigma[r][2 * i + 0]]; \
	d = ror32(d ^ a, 16); \
	c += d; \
	b = ror32(b ^ c, 12); \
	a += b + m[blake2s_sigma[r][2 * i + 1]]; \
	d = ror32(d ^ a, 8); \
	c += d; \
	b = ror32(b ^ c, 7); \
} ಆಗ (0)

#ವ್ಯಾಖ್ಯಾನಿಸು ಸುತ್ತು(r) ಮಾಡು { \
	G(r, 0, v[0], v[ 4], v[ 8], v[12]); \
	G(r, 1, v[1], v[ 5], v[ 9], v[13]); \
	G(r, 2, v[2], v[ 6], v[10], v[14]); \
	G(r, 3, v[3], v[ 7], v[11], v[15]); \
	G(r, 4, v[0], v[ 5], v[10], v[15]); \
	G(r, 5, v[1], v[ 6], v[11], v[12]); \
	G(r, 6, v[2], v[ 7], v[ 8], v[13]); \
	G(r, 7, v[3], v[ 4], v[ 9], v[14]); \
} ಆಗ (0)
		ಸುತ್ತು(0);
		ಸುತ್ತು(1);
		ಸುತ್ತು(2);
		ಸುತ್ತು(3);
		ಸುತ್ತು(4);
		ಸುತ್ತು(5);
		ಸುತ್ತು(6);
		ಸುತ್ತು(7);
		ಸುತ್ತು(8);
		ಸುತ್ತು(9);

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು G
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಸುತ್ತು

		ಗಾಗಿ (i = 0; i < 8; ++i)
			state->h[i] ^= v[i] ^ v[i + 8];

		ಖಂಡ += BLAKE2_S_ಖಂಡ_LEN;
		--nblocks;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_update(ರಚನೆ blake2s_state *state, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *inp, size_t inlen)
{
	ಸ್ಥಿರಾಂಕ size_t fill = BLAKE2_S_ಖಂಡ_LEN - state->buflen;
	ಸ್ಥಿರಾಂಕ uint8_t *ಒಳ = inp;

	ಆದರೆ (!inlen)
		ಹಿಂತಿರುಗಿ;
	ಆದರೆ (inlen > fill) {
		ಸ್ಮರಣೆ_ನಕಲು(state->buf + state->buflen, ಒಳ, fill);
		blake2s_compress(state, state->buf, 1, BLAKE2_S_ಖಂಡ_LEN);
		state->buflen = 0;
		ಒಳ += fill;
		inlen -= fill;
	}
	ಆದರೆ (inlen > BLAKE2_S_ಖಂಡ_LEN) {
		ಸ್ಥಿರಾಂಕ size_t nblocks = DIV_ಸುತ್ತು_ಮೇಲೆ(inlen, BLAKE2_S_ಖಂಡ_LEN);
		blake2s_compress(state, ಒಳ, nblocks - 1, BLAKE2_S_ಖಂಡ_LEN);
		ಒಳ += BLAKE2_S_ಖಂಡ_LEN * (nblocks - 1);
		inlen -= BLAKE2_S_ಖಂಡ_LEN * (nblocks - 1);
	}
	ಸ್ಮರಣೆ_ನಕಲು(state->buf + state->buflen, ಒಳ, inlen);
	state->buflen += inlen;
}

ಸ್ಥಿರ ಶೂನ್ಯ blake2s_final(ರಚನೆ blake2s_state *state, uint8_t *ಹೊರಹೋಗು)
{
	blake2s_ಗಣ_lastblock(state);
	memset(state->buf + state->buflen, 0, BLAKE2_S_ಖಂಡ_LEN - state->buflen);
	blake2s_compress(state, state->buf, 1, state->buflen);
	cpu_ಗಮ್ಯ_le32_array(state->h, ARRAY_ಗಾತ್ರ(state->h));
	ಸ್ಮರಣೆ_ನಕಲು(ಹೊರಹೋಗು, state->h, state->outlen);
}

ಸ್ಥಿರ ssize_t getrandom_ತುಂಬಿದ(ಶೂನ್ಯ *buf, size_t ಎಣಿಕೆ_2, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags)
{
	ssize_t ret, ಒಟ್ಟು = 0;
	uint8_t *p = buf;

	ಮಾಡು {
		ret = getrandom(p, ಎಣಿಕೆ_2, flags);
		ಆದರೆ (ret < 0 && errno == EINTR)
			ಮುಂದುವರಿ;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ret < 0)
			ಹಿಂತಿರುಗಿ ret;
		ಒಟ್ಟು += ret;
		p += ret;
		ಎಣಿಕೆ_2 -= ret;
	} ಆಗ (ಎಣಿಕೆ_2);
	ಹಿಂತಿರುಗಿ ಒಟ್ಟು;
}

ಸ್ಥಿರ ssize_t read_ತುಂಬಿದ(ಪೂರ್ಣಾಂಕ fd, ಶೂನ್ಯ *buf, size_t ಎಣಿಕೆ_2)
{
	ssize_t ret, ಒಟ್ಟು = 0;
	uint8_t *p = buf;

	ಮಾಡು {
		ret = ಪಠ(fd, p, ಎಣಿಕೆ_2);
		ಆದರೆ (ret < 0 && errno == EINTR)
			ಮುಂದುವರಿ;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ret < 0)
			ಹಿಂತಿರುಗಿ ret;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ret == 0)
			ನಿಲ್ಲಿಸು;
		ಒಟ್ಟು += ret;
		p += ret;
		ಎಣಿಕೆ_2 -= ret;
	} ಆಗ (ಎಣಿಕೆ_2);
	ಹಿಂತಿರುಗಿ ಒಟ್ಟು;
}

ಸ್ಥಿರ ssize_t write_ತುಂಬಿದ(ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *buf, size_t ಎಣಿಕೆ_2)
{
	ssize_t ret, ಒಟ್ಟು = 0;
	ಸ್ಥಿರಾಂಕ uint8_t *p = buf;

	ಮಾಡು {
		ret = ಲಿಖ(fd, p, ಎಣಿಕೆ_2);
		ಆದರೆ (ret < 0 && errno == EINTR)
			ಮುಂದುವರಿ;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ret < 0)
			ಹಿಂತಿರುಗಿ ret;
		ಒಟ್ಟು += ret;
		p += ret;
		ಎಣಿಕೆ_2 -= ret;
	} ಆಗ (ಎಣಿಕೆ_2);
	ಹಿಂತಿರುಗಿ ಒಟ್ಟು;
}

ಸ್ಥಿರ size_t determine_optimal_ಬೀಜ_len(ಶೂನ್ಯ)
{
	size_t ret = 0;
	ಅಕ್ಷರ poolsize_str[11] = { 0 };
	ಪೂರ್ಣಾಂಕ fd = open("/proc/sys/kernel/random/poolsize", O_RDONLY);

	ಆದರೆ (fd < 0 || read_ತುಂಬಿದ(fd, poolsize_str, ಗಾತ್ರ(poolsize_str) - 1) < 0) {
		ewarn("%s: Unable to determine pool size, falling back to %u bits: %s",
		      applet, MIN_ಬೀಜ_LEN * 8, strerror(errno));
		ret = MIN_ಬೀಜ_LEN;
	} ಇಲ್ಲದಿದ್ದರೆ
		ret = DIV_ಸುತ್ತು_ಮೇಲೆ(strtoul(poolsize_str, ಖಾಲಿ, 10), 8);
	ಆದರೆ (fd >= 0)
		close(fd);
	ಆದರೆ (ret < MIN_ಬೀಜ_LEN)
		ret = MIN_ಬೀಜ_LEN;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ret > MAX_ಬೀಜ_LEN)
		ret = MAX_ಬೀಜ_LEN;
	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ read_new_ಬೀಜ(uint8_t *ಬೀಜ, size_t len, bool *is_creditable)
{
	ssize_t ret;
	ಪೂರ್ಣಾಂಕ urandom_fd;

	*is_creditable = false;
	ret = getrandom_ತುಂಬಿದ(ಬೀಜ, len, GRND_NONBLOCK);
	ಆದರೆ (ret == (ssize_t)len) {
		*is_creditable = true;
		ಹಿಂತಿರುಗಿ 0;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ret < 0 && errno == ENOSYS) {
		ರಚನೆ pollfd random_fd = {
			.fd = open("/dev/random", O_RDONLY),
			.events = POLLIN
		};
		ಆದರೆ (random_fd.fd < 0)
			ಹಿಂತಿರುಗಿ -errno;
		*is_creditable = poll(&random_fd, 1, 0) == 1;
		close(random_fd.fd);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (getrandom_ತುಂಬಿದ(ಬೀಜ, len, GRND_INSECURE) == (ssize_t)len)
		ಹಿಂತಿರುಗಿ 0;
	urandom_fd = open("/dev/urandom", O_RDONLY);
	ಆದರೆ (urandom_fd < 0)
		ಹಿಂತಿರುಗಿ -1;
	ret = read_ತುಂಬಿದ(urandom_fd, ಬೀಜ, len);
	ಆದರೆ (ret == (ssize_t)len)
		ret = 0;
	ಇಲ್ಲದಿದ್ದರೆ
		ret = -errno ? -errno : -EIO;
	close(urandom_fd);
	errno = -ret;
	ಹಿಂತಿರುಗಿ ret ? -1 : 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಬೀಜ_rng(uint8_t *ಬೀಜ, size_t len, bool credit)
{
	ರಚನೆ {
		ಪೂರ್ಣಾಂಕ entropy_ಎಣಿಕೆ;
		ಪೂರ್ಣಾಂಕ buf_ಗಾತ್ರ;
		uint8_t ಬಫರ್[MAX_ಬೀಜ_LEN];
	} req = {
		.entropy_ಎಣಿಕೆ = credit ? len * 8 : 0,
		.buf_ಗಾತ್ರ = len
	};
	ಪೂರ್ಣಾಂಕ random_fd, ret;

	ಆದರೆ (len > ಗಾತ್ರ(req.ಬಫರ್)) {
		errno = EFBIG;
		ಹಿಂತಿರುಗಿ -1;
	}
	ಸ್ಮರಣೆ_ನಕಲು(req.ಬಫರ್, ಬೀಜ, len);

	random_fd = open("/dev/urandom", O_RDONLY);
	ಆದರೆ (random_fd < 0)
		ಹಿಂತಿರುಗಿ -1;
	ret = ioctl(random_fd, RNDADDENTROPY, &req);
	ಆದರೆ (ret)
		ret = -errno ? -errno : -EIO;
	close(random_fd);
	errno = -ret;
	ಹಿಂತಿರುಗಿ ret ? -1 : 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಬೀಜ_from_file_if_exists(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename, ಪೂರ್ಣಾಂಕ dfd, bool credit, ರಚನೆ blake2s_state *hash)
{
	uint8_t ಬೀಜ[MAX_ಬೀಜ_LEN];
	ssize_t ಬೀಜ_len;
	ಪೂರ್ಣಾಂಕ fd = -1, ret = 0;

	fd = openat(dfd, filename, O_RDONLY);
	ಆದರೆ (fd < 0 && errno == ENOENT)
		ಹಿಂತಿರುಗಿ 0;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (fd < 0) {
		ret = -errno;
		eerror("%s: Unable to open seed file: %s", applet, strerror(errno));
		ಹೋಗು ಹೊರಹೋಗು;
	}
	ಬೀಜ_len = read_ತುಂಬಿದ(fd, ಬೀಜ, ಗಾತ್ರ(ಬೀಜ));
	ಆದರೆ (ಬೀಜ_len < 0) {
		ret = -errno;
		eerror("%s: Unable to read seed file: %s", applet, strerror(errno));
		ಹೋಗು ಹೊರಹೋಗು;
	}
	ಆದರೆ ((unlinkat(dfd, filename, 0) < 0 || fsync(dfd) < 0) && ಬೀಜ_len) {
		ret = -errno;
		eerror("%s: Unable to remove seed after reading, so not seeding: %s",
		       applet, strerror(errno));
		ಹೋಗು ಹೊರಹೋಗು;
	}
	ಆದರೆ (!ಬೀಜ_len)
		ಹೋಗು ಹೊರಹೋಗು;

	blake2s_update(hash, &ಬೀಜ_len, ಗಾತ್ರ(ಬೀಜ_len));
	blake2s_update(hash, ಬೀಜ, ಬೀಜ_len);

	einfo("Seeding %zd bits %s crediting", ಬೀಜ_len * 8, credit ? "and" : "without");
	ಆದರೆ (ಬೀಜ_rng(ಬೀಜ, ಬೀಜ_len, credit) < 0) {
		ret = -errno;
		eerror("%s: Unable to seed: %s", applet, strerror(errno));
	}

ಹೊರಹೋಗು:
	ಆದರೆ (fd >= 0)
		close(fd);
	errno = -ret;
	ಹಿಂತಿರುಗಿ ret ? -1 : 0;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ seedrng_ಪೂರ್ವಪ್ರತ್ಯಯ[] = "SeedRNG v1 Old+New Prefix";
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ seedrng_failure[] = "SeedRNG v1 No New Seed Failure";
	ಪೂರ್ಣಾಂಕ opt, fd, dfd, ಕಾರ್ಯಕ್ರಮ_ret = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬೀಜ_dir = ಖಾಲಿ;
	uint8_t new_ಬೀಜ[MAX_ಬೀಜ_LEN];
	size_t new_ಬೀಜ_len;
	bool new_ಬೀಜ_creditable;
	ರಚನೆ timespec realtime = { 0 }, boottime = { 0 };
	ರಚನೆ blake2s_state hash;
	bool skip_credit = false;

	applet = basename_c(argv[0]);

	ಆಗ ((opt = getopt_long(argc, argv, getoptstring, longopts, (ಪೂರ್ಣಾಂಕ *) 0)) != -1)
	{
		ಆಯ್ಕೆ (opt) {
		ಸ್ಥಿತಿ LONGOPT_ಬೀಜ_DIR:
			ಆದರೆ (!ಬೀಜ_dir)
				ಬೀಜ_dir = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ LONGOPT_SKIP_CREDIT:
			skip_credit = true;
			ನಿಲ್ಲಿಸು;
		case_RC_COMMON_GETOPT
		}
	}
	ಆದರೆ (!ಬೀಜ_dir)
		ಬೀಜ_dir = DEFAULT_ಬೀಜ_DIR;
	ಆದರೆ (getuid())
		eerrorx("%s: superuser access is required", applet);
	umask(0077);

	blake2s_init(&hash, BLAKE2S_HASH_LEN);
	blake2s_update(&hash, seedrng_ಪೂರ್ವಪ್ರತ್ಯಯ, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(seedrng_ಪೂರ್ವಪ್ರತ್ಯಯ));
	clock_gettime(CLOCK_REALTIME, &realtime);
	clock_gettime(CLOCK_BOOTTIME, &boottime);
	blake2s_update(&hash, &realtime, ಗಾತ್ರ(realtime));
	blake2s_update(&hash, &boottime, ಗಾತ್ರ(boottime));

	ಆದರೆ (mkdir(ಬೀಜ_dir, 0700) < 0 && errno != EEXIST)
		eerrorx("%s: Unable to create seed directory: %s", applet, strerror(errno));

	dfd = open(ಬೀಜ_dir, O_DIRECTORY | O_RDONLY);
	ಆದರೆ (dfd < 0 || flock(dfd, LOCK_EX) < 0)
		eerrorx("%s: Unable to lock seed directory: %s", applet, strerror(errno));

	ಆದರೆ (ಬೀಜ_from_file_if_exists(NON_CREDITABLE_ಬೀಜ, dfd, false, &hash) < 0)
		ಕಾರ್ಯಕ್ರಮ_ret |= 1 << 1;
	ಆದರೆ (ಬೀಜ_from_file_if_exists(CREDITABLE_ಬೀಜ, dfd, !skip_credit, &hash) < 0)
		ಕಾರ್ಯಕ್ರಮ_ret |= 1 << 2;

	new_ಬೀಜ_len = determine_optimal_ಬೀಜ_len();
	ಆದರೆ (read_new_ಬೀಜ(new_ಬೀಜ, new_ಬೀಜ_len, &new_ಬೀಜ_creditable) < 0) {
		eerror("%s: Unable to read new seed: %s", applet, strerror(errno));
		new_ಬೀಜ_len = BLAKE2S_HASH_LEN;
		strncpy((ಅಕ್ಷರ *)new_ಬೀಜ, seedrng_failure, new_ಬೀಜ_len);
		ಕಾರ್ಯಕ್ರಮ_ret |= 1 << 3;
	}
	blake2s_update(&hash, &new_ಬೀಜ_len, ಗಾತ್ರ(new_ಬೀಜ_len));
	blake2s_update(&hash, new_ಬೀಜ, new_ಬೀಜ_len);
	blake2s_final(&hash, new_ಬೀಜ + new_ಬೀಜ_len - BLAKE2S_HASH_LEN);

	einfo("Saving %zu bits of %s seed for next boot", new_ಬೀಜ_len * 8, new_ಬೀಜ_creditable ? "creditable" : "non-creditable");
	fd = openat(dfd, NON_CREDITABLE_ಬೀಜ, O_WRONLY | O_CREAT | O_TRUNC, 0400);
	ಆದರೆ (fd < 0) {
		eerror("%s: Unable to open seed file for writing: %s", applet, strerror(errno));
		ಹಿಂತಿರುಗಿ ಕಾರ್ಯಕ್ರಮ_ret | (1 << 4);
	}
	ಆದರೆ (write_ತುಂಬಿದ(fd, new_ಬೀಜ, new_ಬೀಜ_len) != (ssize_t)new_ಬೀಜ_len || fsync(fd) < 0) {
		eerror("%s: Unable to write seed file: %s", applet, strerror(errno));
		ಹಿಂತಿರುಗಿ ಕಾರ್ಯಕ್ರಮ_ret | (1 << 5);
	}
	ಆದರೆ (new_ಬೀಜ_creditable && renameat(dfd, NON_CREDITABLE_ಬೀಜ, dfd, CREDITABLE_ಬೀಜ) < 0) {
		ewarn("%s: Unable to make new seed creditable: %s", applet, strerror(errno));
		ಹಿಂತಿರುಗಿ ಕಾರ್ಯಕ್ರಮ_ret | (1 << 6);
	}
	ಹಿಂತಿರುಗಿ ಕಾರ್ಯಕ್ರಮ_ret;
}
