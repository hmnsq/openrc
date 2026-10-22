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

#ਅਨ੍ਤਰ੍ਭਾਵ <linux/random.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/random.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/ioctl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/file.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <poll.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <endian.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ GRND_INSECURE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GRND_INSECURE 0x0004 /* Apparently some headers don't ship with this yet. */
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_ਬੀਜ_DIR "/var/lib/seedrng"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CREDITABLE_ਬੀਜ "seed.credit"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NON_CREDITABLE_ਬੀਜ "seed.no-credit"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ਗਿਣਤੀ long_opts {
	/* This has to come first so following values stay in the 0x100+ range. */
	LONGOPT_ਅਧਾਰ = 0x100,
	LONGOPT_ਬੀਜ_DIR,
	LONGOPT_SKIP_CREDIT,
};

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ *extraopts = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ getoptstring[] = getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "seed-dir", 1, ਨਿਰਾ, LONGOPT_ਬੀਜ_DIR },
	{ "skip-credit", 0, ਨਿਰਾ, LONGOPT_SKIP_CREDIT },
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
	"Directory for seed files (default: " DEFAULT_ਬੀਜ_DIR ")",
	"Skip crediting entropy of seeds",
	longopts_help_COMMON
};
ਅਚਲ ਅੱਖਰ *usagestring = ਨਿਰਾ;

ਗਿਣਤੀ blake2s_lengths {
	BLAKE2_S_ਖੰਡ_LEN = 64,
	BLAKE2S_HASH_LEN = 32,
	BLAKE2_S_ਕੁੰਜੀ_LEN = 32
};

ਗਿਣਤੀ seedrng_lengths {
	MAX_ਬੀਜ_LEN = 512,
	MIN_ਬੀਜ_LEN = BLAKE2S_HASH_LEN
};

ਬਣਤਰ blake2s_state {
	uint32_t h[8];
	uint32_t t[2];
	uint32_t f[2];
	uint8_t buf[BLAKE2_S_ਖੰਡ_LEN];
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ buflen;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ outlen;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ le32_ਮੰਜ਼ਿਲ_cpup(a) le32toh(*(a))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cpu_ਮੰਜ਼ਿਲ_le32(a) htole32(a)
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ARRAY_ਮਾਪ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ARRAY_ਮਾਪ(x) (ਆਕਾਰ(x) / ਆਕਾਰ((x)[0]))
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DIV_ਗੇੜਾ_ਉੱਪਰ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DIV_ਗੇੜਾ_ਉੱਪਰ(n, d) (((n) + (d) - 1) / (d))
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ cpu_ਮੰਜ਼ਿਲ_le32_array(uint32_t *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸ਼ਬਦ)
{
	ਜਦੋਂ (ਸ਼ਬਦ--) {
		*buf = cpu_ਮੰਜ਼ਿਲ_le32(*buf);
		++buf;
	}
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ le32_ਮੰਜ਼ਿਲ_cpu_array(uint32_t *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸ਼ਬਦ)
{
	ਜਦੋਂ (ਸ਼ਬਦ--) {
		*buf = le32_ਮੰਜ਼ਿਲ_cpup(buf);
		++buf;
	}
}

ਸਥਿਰ ਸਤਰਅੰਦਰ uint32_t ror32(uint32_t ਸ਼ਬਦ_2, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ shift)
{
	ਵਾਪਸ (ਸ਼ਬਦ_2 >> (shift & 31)) | (ਸ਼ਬਦ_2 << ((-shift) & 31));
}

ਸਥਿਰ ਅਚਲ uint32_t blake2s_iv[8] = {
	0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
	0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

ਸਥਿਰ ਅਚਲ uint8_t blake2s_sigma[10][16] = {
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

ਸਥਿਰ ਖਾਲੀ blake2s_ਸੈੱਟ_lastblock(ਬਣਤਰ blake2s_state *state)
{
	state->f[0] = -1;
}

ਸਥਿਰ ਖਾਲੀ blake2s_increment_counter(ਬਣਤਰ blake2s_state *state, ਅਚਲ uint32_t inc)
{
	state->t[0] += inc;
	state->t[1] += (state->t[0] < inc);
}

ਸਥਿਰ ਖਾਲੀ blake2s_init_param(ਬਣਤਰ blake2s_state *state, ਅਚਲ uint32_t param)
{
	ਪੂਰਨ i;

	memset(state, 0, ਆਕਾਰ(*state));
	ਲਈ (i = 0; i < 8; ++i)
		state->h[i] = blake2s_iv[i];
	state->h[0] ^= param;
}

ਸਥਿਰ ਖਾਲੀ blake2s_init(ਬਣਤਰ blake2s_state *state, ਅਚਲ size_t outlen)
{
	blake2s_init_param(state, 0x01010000 | outlen);
	state->outlen = outlen;
}

ਸਥਿਰ ਖਾਲੀ blake2s_compress(ਬਣਤਰ blake2s_state *state, ਅਚਲ uint8_t *ਖੰਡ, size_t nblocks, ਅਚਲ uint32_t inc)
{
	uint32_t m[16];
	uint32_t v[16];
	ਪੂਰਨ i;

	ਜਦੋਂ (nblocks > 0) {
		blake2s_increment_counter(state, inc);
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(m, ਖੰਡ, BLAKE2_S_ਖੰਡ_LEN);
		le32_ਮੰਜ਼ਿਲ_cpu_array(m, ARRAY_ਮਾਪ(m));
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(v, state->h, 32);
		v[ 8] = blake2s_iv[0];
		v[ 9] = blake2s_iv[1];
		v[10] = blake2s_iv[2];
		v[11] = blake2s_iv[3];
		v[12] = blake2s_iv[4] ^ state->t[0];
		v[13] = blake2s_iv[5] ^ state->t[1];
		v[14] = blake2s_iv[6] ^ state->f[0];
		v[15] = blake2s_iv[7] ^ state->f[1];

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ G(r, i, a, b, c, d) ਕਰੋ { \
	a += b + m[blake2s_sigma[r][2 * i + 0]]; \
	d = ror32(d ^ a, 16); \
	c += d; \
	b = ror32(b ^ c, 12); \
	a += b + m[blake2s_sigma[r][2 * i + 1]]; \
	d = ror32(d ^ a, 8); \
	c += d; \
	b = ror32(b ^ c, 7); \
} ਜਦੋਂ (0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਗੇੜਾ(r) ਕਰੋ { \
	G(r, 0, v[0], v[ 4], v[ 8], v[12]); \
	G(r, 1, v[1], v[ 5], v[ 9], v[13]); \
	G(r, 2, v[2], v[ 6], v[10], v[14]); \
	G(r, 3, v[3], v[ 7], v[11], v[15]); \
	G(r, 4, v[0], v[ 5], v[10], v[15]); \
	G(r, 5, v[1], v[ 6], v[11], v[12]); \
	G(r, 6, v[2], v[ 7], v[ 8], v[13]); \
	G(r, 7, v[3], v[ 4], v[ 9], v[14]); \
} ਜਦੋਂ (0)
		ਗੇੜਾ(0);
		ਗੇੜਾ(1);
		ਗੇੜਾ(2);
		ਗੇੜਾ(3);
		ਗੇੜਾ(4);
		ਗੇੜਾ(5);
		ਗੇੜਾ(6);
		ਗੇੜਾ(7);
		ਗੇੜਾ(8);
		ਗੇੜਾ(9);

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ G
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਗੇੜਾ

		ਲਈ (i = 0; i < 8; ++i)
			state->h[i] ^= v[i] ^ v[i + 8];

		ਖੰਡ += BLAKE2_S_ਖੰਡ_LEN;
		--nblocks;
	}
}

ਸਥਿਰ ਖਾਲੀ blake2s_update(ਬਣਤਰ blake2s_state *state, ਅਚਲ ਖਾਲੀ *inp, size_t inlen)
{
	ਅਚਲ size_t fill = BLAKE2_S_ਖੰਡ_LEN - state->buflen;
	ਅਚਲ uint8_t *ਵਿੱਚ = inp;

	ਜੇ (!inlen)
		ਵਾਪਸ;
	ਜੇ (inlen > fill) {
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(state->buf + state->buflen, ਵਿੱਚ, fill);
		blake2s_compress(state, state->buf, 1, BLAKE2_S_ਖੰਡ_LEN);
		state->buflen = 0;
		ਵਿੱਚ += fill;
		inlen -= fill;
	}
	ਜੇ (inlen > BLAKE2_S_ਖੰਡ_LEN) {
		ਅਚਲ size_t nblocks = DIV_ਗੇੜਾ_ਉੱਪਰ(inlen, BLAKE2_S_ਖੰਡ_LEN);
		blake2s_compress(state, ਵਿੱਚ, nblocks - 1, BLAKE2_S_ਖੰਡ_LEN);
		ਵਿੱਚ += BLAKE2_S_ਖੰਡ_LEN * (nblocks - 1);
		inlen -= BLAKE2_S_ਖੰਡ_LEN * (nblocks - 1);
	}
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(state->buf + state->buflen, ਵਿੱਚ, inlen);
	state->buflen += inlen;
}

ਸਥਿਰ ਖਾਲੀ blake2s_final(ਬਣਤਰ blake2s_state *state, uint8_t *ਬਾਹਰਲਾ)
{
	blake2s_ਸੈੱਟ_lastblock(state);
	memset(state->buf + state->buflen, 0, BLAKE2_S_ਖੰਡ_LEN - state->buflen);
	blake2s_compress(state, state->buf, 1, state->buflen);
	cpu_ਮੰਜ਼ਿਲ_le32_array(state->h, ARRAY_ਮਾਪ(state->h));
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ, state->h, state->outlen);
}

ਸਥਿਰ ssize_t getrandom_ਭਰਿਆ(ਖਾਲੀ *buf, size_t ਗਿਣਤੀ_2, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags)
{
	ssize_t ret, ਕੁੱਲ = 0;
	uint8_t *p = buf;

	ਕਰੋ {
		ret = getrandom(p, ਗਿਣਤੀ_2, flags);
		ਜੇ (ret < 0 && errno == EINTR)
			ਜਾਰੀ;
		ਨਹੀਂਤਾਂ ਜੇ (ret < 0)
			ਵਾਪਸ ret;
		ਕੁੱਲ += ret;
		p += ret;
		ਗਿਣਤੀ_2 -= ret;
	} ਜਦੋਂ (ਗਿਣਤੀ_2);
	ਵਾਪਸ ਕੁੱਲ;
}

ਸਥਿਰ ssize_t read_ਭਰਿਆ(ਪੂਰਨ fd, ਖਾਲੀ *buf, size_t ਗਿਣਤੀ_2)
{
	ssize_t ret, ਕੁੱਲ = 0;
	uint8_t *p = buf;

	ਕਰੋ {
		ret = ਪਠ(fd, p, ਗਿਣਤੀ_2);
		ਜੇ (ret < 0 && errno == EINTR)
			ਜਾਰੀ;
		ਨਹੀਂਤਾਂ ਜੇ (ret < 0)
			ਵਾਪਸ ret;
		ਨਹੀਂਤਾਂ ਜੇ (ret == 0)
			ਰੋਕੋ;
		ਕੁੱਲ += ret;
		p += ret;
		ਗਿਣਤੀ_2 -= ret;
	} ਜਦੋਂ (ਗਿਣਤੀ_2);
	ਵਾਪਸ ਕੁੱਲ;
}

ਸਥਿਰ ssize_t write_ਭਰਿਆ(ਪੂਰਨ fd, ਅਚਲ ਖਾਲੀ *buf, size_t ਗਿਣਤੀ_2)
{
	ssize_t ret, ਕੁੱਲ = 0;
	ਅਚਲ uint8_t *p = buf;

	ਕਰੋ {
		ret = ਲਿਖ(fd, p, ਗਿਣਤੀ_2);
		ਜੇ (ret < 0 && errno == EINTR)
			ਜਾਰੀ;
		ਨਹੀਂਤਾਂ ਜੇ (ret < 0)
			ਵਾਪਸ ret;
		ਕੁੱਲ += ret;
		p += ret;
		ਗਿਣਤੀ_2 -= ret;
	} ਜਦੋਂ (ਗਿਣਤੀ_2);
	ਵਾਪਸ ਕੁੱਲ;
}

ਸਥਿਰ size_t determine_optimal_ਬੀਜ_len(ਖਾਲੀ)
{
	size_t ret = 0;
	ਅੱਖਰ poolsize_str[11] = { 0 };
	ਪੂਰਨ fd = open("/proc/sys/kernel/random/poolsize", O_RDONLY);

	ਜੇ (fd < 0 || read_ਭਰਿਆ(fd, poolsize_str, ਆਕਾਰ(poolsize_str) - 1) < 0) {
		ewarn("%s: Unable to determine pool size, falling back to %u bits: %s",
		      applet, MIN_ਬੀਜ_LEN * 8, strerror(errno));
		ret = MIN_ਬੀਜ_LEN;
	} ਨਹੀਂਤਾਂ
		ret = DIV_ਗੇੜਾ_ਉੱਪਰ(strtoul(poolsize_str, ਨਿਰਾ, 10), 8);
	ਜੇ (fd >= 0)
		close(fd);
	ਜੇ (ret < MIN_ਬੀਜ_LEN)
		ret = MIN_ਬੀਜ_LEN;
	ਨਹੀਂਤਾਂ ਜੇ (ret > MAX_ਬੀਜ_LEN)
		ret = MAX_ਬੀਜ_LEN;
	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ read_new_ਬੀਜ(uint8_t *ਬੀਜ, size_t len, bool *is_creditable)
{
	ssize_t ret;
	ਪੂਰਨ urandom_fd;

	*is_creditable = false;
	ret = getrandom_ਭਰਿਆ(ਬੀਜ, len, GRND_NONBLOCK);
	ਜੇ (ret == (ssize_t)len) {
		*is_creditable = true;
		ਵਾਪਸ 0;
	} ਨਹੀਂਤਾਂ ਜੇ (ret < 0 && errno == ENOSYS) {
		ਬਣਤਰ pollfd random_fd = {
			.fd = open("/dev/random", O_RDONLY),
			.events = POLLIN
		};
		ਜੇ (random_fd.fd < 0)
			ਵਾਪਸ -errno;
		*is_creditable = poll(&random_fd, 1, 0) == 1;
		close(random_fd.fd);
	} ਨਹੀਂਤਾਂ ਜੇ (getrandom_ਭਰਿਆ(ਬੀਜ, len, GRND_INSECURE) == (ssize_t)len)
		ਵਾਪਸ 0;
	urandom_fd = open("/dev/urandom", O_RDONLY);
	ਜੇ (urandom_fd < 0)
		ਵਾਪਸ -1;
	ret = read_ਭਰਿਆ(urandom_fd, ਬੀਜ, len);
	ਜੇ (ret == (ssize_t)len)
		ret = 0;
	ਨਹੀਂਤਾਂ
		ret = -errno ? -errno : -EIO;
	close(urandom_fd);
	errno = -ret;
	ਵਾਪਸ ret ? -1 : 0;
}

ਸਥਿਰ ਪੂਰਨ ਬੀਜ_rng(uint8_t *ਬੀਜ, size_t len, bool credit)
{
	ਬਣਤਰ {
		ਪੂਰਨ entropy_ਗਿਣਤੀ;
		ਪੂਰਨ buf_ਮਾਪ;
		uint8_t ਬਫਰ[MAX_ਬੀਜ_LEN];
	} req = {
		.entropy_ਗਿਣਤੀ = credit ? len * 8 : 0,
		.buf_ਮਾਪ = len
	};
	ਪੂਰਨ random_fd, ret;

	ਜੇ (len > ਆਕਾਰ(req.ਬਫਰ)) {
		errno = EFBIG;
		ਵਾਪਸ -1;
	}
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(req.ਬਫਰ, ਬੀਜ, len);

	random_fd = open("/dev/urandom", O_RDONLY);
	ਜੇ (random_fd < 0)
		ਵਾਪਸ -1;
	ret = ioctl(random_fd, RNDADDENTROPY, &req);
	ਜੇ (ret)
		ret = -errno ? -errno : -EIO;
	close(random_fd);
	errno = -ret;
	ਵਾਪਸ ret ? -1 : 0;
}

ਸਥਿਰ ਪੂਰਨ ਬੀਜ_from_file_if_exists(ਅਚਲ ਅੱਖਰ *filename, ਪੂਰਨ dfd, bool credit, ਬਣਤਰ blake2s_state *hash)
{
	uint8_t ਬੀਜ[MAX_ਬੀਜ_LEN];
	ssize_t ਬੀਜ_len;
	ਪੂਰਨ fd = -1, ret = 0;

	fd = openat(dfd, filename, O_RDONLY);
	ਜੇ (fd < 0 && errno == ENOENT)
		ਵਾਪਸ 0;
	ਨਹੀਂਤਾਂ ਜੇ (fd < 0) {
		ret = -errno;
		eerror("%s: Unable to open seed file: %s", applet, strerror(errno));
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	ਬੀਜ_len = read_ਭਰਿਆ(fd, ਬੀਜ, ਆਕਾਰ(ਬੀਜ));
	ਜੇ (ਬੀਜ_len < 0) {
		ret = -errno;
		eerror("%s: Unable to read seed file: %s", applet, strerror(errno));
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	ਜੇ ((unlinkat(dfd, filename, 0) < 0 || fsync(dfd) < 0) && ਬੀਜ_len) {
		ret = -errno;
		eerror("%s: Unable to remove seed after reading, so not seeding: %s",
		       applet, strerror(errno));
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	ਜੇ (!ਬੀਜ_len)
		ਜਾਓ ਬਾਹਰਲਾ;

	blake2s_update(hash, &ਬੀਜ_len, ਆਕਾਰ(ਬੀਜ_len));
	blake2s_update(hash, ਬੀਜ, ਬੀਜ_len);

	einfo("Seeding %zd bits %s crediting", ਬੀਜ_len * 8, credit ? "and" : "without");
	ਜੇ (ਬੀਜ_rng(ਬੀਜ, ਬੀਜ_len, credit) < 0) {
		ret = -errno;
		eerror("%s: Unable to seed: %s", applet, strerror(errno));
	}

ਬਾਹਰਲਾ:
	ਜੇ (fd >= 0)
		close(fd);
	errno = -ret;
	ਵਾਪਸ ret ? -1 : 0;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ seedrng_ਅਗੇਤਰ[] = "SeedRNG v1 Old+New Prefix";
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ seedrng_failure[] = "SeedRNG v1 No New Seed Failure";
	ਪੂਰਨ opt, fd, dfd, ਪ੍ਰੋਗਰਾਮ_ret = 0;
	ਅਚਲ ਅੱਖਰ *ਬੀਜ_dir = ਨਿਰਾ;
	uint8_t new_ਬੀਜ[MAX_ਬੀਜ_LEN];
	size_t new_ਬੀਜ_len;
	bool new_ਬੀਜ_creditable;
	ਬਣਤਰ timespec realtime = { 0 }, boottime = { 0 };
	ਬਣਤਰ blake2s_state hash;
	bool skip_credit = false;

	applet = basename_c(argv[0]);

	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring, longopts, (ਪੂਰਨ *) 0)) != -1)
	{
		ਚੋਣ (opt) {
		ਹਾਲਤ LONGOPT_ਬੀਜ_DIR:
			ਜੇ (!ਬੀਜ_dir)
				ਬੀਜ_dir = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ LONGOPT_SKIP_CREDIT:
			skip_credit = true;
			ਰੋਕੋ;
		case_RC_COMMON_GETOPT
		}
	}
	ਜੇ (!ਬੀਜ_dir)
		ਬੀਜ_dir = DEFAULT_ਬੀਜ_DIR;
	ਜੇ (getuid())
		eerrorx("%s: superuser access is required", applet);
	umask(0077);

	blake2s_init(&hash, BLAKE2S_HASH_LEN);
	blake2s_update(&hash, seedrng_ਅਗੇਤਰ, ਲੜੀ_ਲੰਬਾਈ(seedrng_ਅਗੇਤਰ));
	clock_gettime(CLOCK_REALTIME, &realtime);
	clock_gettime(CLOCK_BOOTTIME, &boottime);
	blake2s_update(&hash, &realtime, ਆਕਾਰ(realtime));
	blake2s_update(&hash, &boottime, ਆਕਾਰ(boottime));

	ਜੇ (mkdir(ਬੀਜ_dir, 0700) < 0 && errno != EEXIST)
		eerrorx("%s: Unable to create seed directory: %s", applet, strerror(errno));

	dfd = open(ਬੀਜ_dir, O_DIRECTORY | O_RDONLY);
	ਜੇ (dfd < 0 || flock(dfd, LOCK_EX) < 0)
		eerrorx("%s: Unable to lock seed directory: %s", applet, strerror(errno));

	ਜੇ (ਬੀਜ_from_file_if_exists(NON_CREDITABLE_ਬੀਜ, dfd, false, &hash) < 0)
		ਪ੍ਰੋਗਰਾਮ_ret |= 1 << 1;
	ਜੇ (ਬੀਜ_from_file_if_exists(CREDITABLE_ਬੀਜ, dfd, !skip_credit, &hash) < 0)
		ਪ੍ਰੋਗਰਾਮ_ret |= 1 << 2;

	new_ਬੀਜ_len = determine_optimal_ਬੀਜ_len();
	ਜੇ (read_new_ਬੀਜ(new_ਬੀਜ, new_ਬੀਜ_len, &new_ਬੀਜ_creditable) < 0) {
		eerror("%s: Unable to read new seed: %s", applet, strerror(errno));
		new_ਬੀਜ_len = BLAKE2S_HASH_LEN;
		strncpy((ਅੱਖਰ *)new_ਬੀਜ, seedrng_failure, new_ਬੀਜ_len);
		ਪ੍ਰੋਗਰਾਮ_ret |= 1 << 3;
	}
	blake2s_update(&hash, &new_ਬੀਜ_len, ਆਕਾਰ(new_ਬੀਜ_len));
	blake2s_update(&hash, new_ਬੀਜ, new_ਬੀਜ_len);
	blake2s_final(&hash, new_ਬੀਜ + new_ਬੀਜ_len - BLAKE2S_HASH_LEN);

	einfo("Saving %zu bits of %s seed for next boot", new_ਬੀਜ_len * 8, new_ਬੀਜ_creditable ? "creditable" : "non-creditable");
	fd = openat(dfd, NON_CREDITABLE_ਬੀਜ, O_WRONLY | O_CREAT | O_TRUNC, 0400);
	ਜੇ (fd < 0) {
		eerror("%s: Unable to open seed file for writing: %s", applet, strerror(errno));
		ਵਾਪਸ ਪ੍ਰੋਗਰਾਮ_ret | (1 << 4);
	}
	ਜੇ (write_ਭਰਿਆ(fd, new_ਬੀਜ, new_ਬੀਜ_len) != (ssize_t)new_ਬੀਜ_len || fsync(fd) < 0) {
		eerror("%s: Unable to write seed file: %s", applet, strerror(errno));
		ਵਾਪਸ ਪ੍ਰੋਗਰਾਮ_ret | (1 << 5);
	}
	ਜੇ (new_ਬੀਜ_creditable && renameat(dfd, NON_CREDITABLE_ਬੀਜ, dfd, CREDITABLE_ਬੀਜ) < 0) {
		ewarn("%s: Unable to make new seed creditable: %s", applet, strerror(errno));
		ਵਾਪਸ ਪ੍ਰੋਗਰਾਮ_ret | (1 << 6);
	}
	ਵਾਪਸ ਪ੍ਰੋਗਰਾਮ_ret;
}
