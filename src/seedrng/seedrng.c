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

#અન્તર્ભાવ <linux/random.h>
#અન્તર્ભાવ <sys/random.h>
#અન્તર્ભાવ <sys/ioctl.h>
#અન્તર્ભાવ <sys/file.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <poll.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <endian.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "helpers.h"
#અન્તર્ભાવ "_usage.h"

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
ગણના long_opts {
	/* This has to come first so following values stay in the 0x100+ range. */
	LONGOPT_BASE = 0x100,
	LONGOPT_SEED_DIR,
	LONGOPT_SKIP_CREDIT,
};

અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *extraopts = ખાલી;
અચલ અક્ષર getoptstring[] = getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{ "seed-dir", 1, ખાલી, LONGOPT_SEED_DIR },
	{ "skip-credit", 0, ખાલી, LONGOPT_SKIP_CREDIT },
	longopts_COMMON
};
અચલ અક્ષર * અચલ longopts_help[] = {
	"Directory for seed files (default: " DEFAULT_SEED_DIR ")",
	"Skip crediting entropy of seeds",
	longopts_help_COMMON
};
અચલ અક્ષર *usagestring = ખાલી;

ગણના blake2s_lengths {
	BLAKE2S_BLOCK_LEN = 64,
	BLAKE2S_HASH_LEN = 32,
	BLAKE2_S_ચાવી_LEN = 32
};

ગણના seedrng_lengths {
	MAX_SEED_LEN = 512,
	MIN_SEED_LEN = BLAKE2S_HASH_LEN
};

સંરચના blake2s_state {
	uint32_t h[8];
	uint32_t t[2];
	uint32_t f[2];
	uint8_t buf[BLAKE2S_BLOCK_LEN];
	unsigned પૂર્ણાંક buflen;
	unsigned પૂર્ણાંક outlen;
};

#define le32_to_cpup(a) le32toh(*(a))
#define cpu_to_le32(a) htole32(a)
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (કદ(x) / કદ((x)[0]))
#endif
#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

સ્થિર અંતઃરેખ શૂન્ય cpu_to_le32_array(uint32_t *buf, unsigned પૂર્ણાંક શબ્દો)
{
	જ્યારે (શબ્દો--) {
		*buf = cpu_to_le32(*buf);
		++buf;
	}
}

સ્થિર અંતઃરેખ શૂન્ય le32_to_cpu_array(uint32_t *buf, unsigned પૂર્ણાંક શબ્દો)
{
	જ્યારે (શબ્દો--) {
		*buf = le32_to_cpup(buf);
		++buf;
	}
}

સ્થિર અંતઃરેખ uint32_t ror32(uint32_t word, unsigned પૂર્ણાંક shift)
{
	પાછા (word >> (shift & 31)) | (word << ((-shift) & 31));
}

સ્થિર અચલ uint32_t blake2s_iv[8] = {
	0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
	0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

સ્થિર અચલ uint8_t blake2s_sigma[10][16] = {
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

સ્થિર શૂન્ય blake2s_set_lastblock(સંરચના blake2s_state *state)
{
	state->f[0] = -1;
}

સ્થિર શૂન્ય blake2s_increment_counter(સંરચના blake2s_state *state, અચલ uint32_t inc)
{
	state->t[0] += inc;
	state->t[1] += (state->t[0] < inc);
}

સ્થિર શૂન્ય blake2s_init_param(સંરચના blake2s_state *state, અચલ uint32_t param)
{
	પૂર્ણાંક i;

	memset(state, 0, કદ(*state));
	માટે (i = 0; i < 8; ++i)
		state->h[i] = blake2s_iv[i];
	state->h[0] ^= param;
}

સ્થિર શૂન્ય blake2s_init(સંરચના blake2s_state *state, અચલ size_t outlen)
{
	blake2s_init_param(state, 0x01010000 | outlen);
	state->outlen = outlen;
}

સ્થિર શૂન્ય blake2s_compress(સંરચના blake2s_state *state, અચલ uint8_t *block, size_t nblocks, અચલ uint32_t inc)
{
	uint32_t m[16];
	uint32_t v[16];
	પૂર્ણાંક i;

	જ્યારે (nblocks > 0) {
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

#define G(r, i, a, b, c, d) કરો { \
	a += b + m[blake2s_sigma[r][2 * i + 0]]; \
	d = ror32(d ^ a, 16); \
	c += d; \
	b = ror32(b ^ c, 12); \
	a += b + m[blake2s_sigma[r][2 * i + 1]]; \
	d = ror32(d ^ a, 8); \
	c += d; \
	b = ror32(b ^ c, 7); \
} જ્યારે (0)

#define ROUND(r) કરો { \
	G(r, 0, v[0], v[ 4], v[ 8], v[12]); \
	G(r, 1, v[1], v[ 5], v[ 9], v[13]); \
	G(r, 2, v[2], v[ 6], v[10], v[14]); \
	G(r, 3, v[3], v[ 7], v[11], v[15]); \
	G(r, 4, v[0], v[ 5], v[10], v[15]); \
	G(r, 5, v[1], v[ 6], v[11], v[12]); \
	G(r, 6, v[2], v[ 7], v[ 8], v[13]); \
	G(r, 7, v[3], v[ 4], v[ 9], v[14]); \
} જ્યારે (0)
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

		માટે (i = 0; i < 8; ++i)
			state->h[i] ^= v[i] ^ v[i + 8];

		block += BLAKE2S_BLOCK_LEN;
		--nblocks;
	}
}

સ્થિર શૂન્ય blake2s_update(સંરચના blake2s_state *state, અચલ શૂન્ય *inp, size_t inlen)
{
	અચલ size_t fill = BLAKE2S_BLOCK_LEN - state->buflen;
	અચલ uint8_t *in = inp;

	જો (!inlen)
		પાછા;
	જો (inlen > fill) {
		memcpy(state->buf + state->buflen, in, fill);
		blake2s_compress(state, state->buf, 1, BLAKE2S_BLOCK_LEN);
		state->buflen = 0;
		in += fill;
		inlen -= fill;
	}
	જો (inlen > BLAKE2S_BLOCK_LEN) {
		અચલ size_t nblocks = DIV_ROUND_UP(inlen, BLAKE2S_BLOCK_LEN);
		blake2s_compress(state, in, nblocks - 1, BLAKE2S_BLOCK_LEN);
		in += BLAKE2S_BLOCK_LEN * (nblocks - 1);
		inlen -= BLAKE2S_BLOCK_LEN * (nblocks - 1);
	}
	memcpy(state->buf + state->buflen, in, inlen);
	state->buflen += inlen;
}

સ્થિર શૂન્ય blake2s_final(સંરચના blake2s_state *state, uint8_t *out)
{
	blake2s_set_lastblock(state);
	memset(state->buf + state->buflen, 0, BLAKE2S_BLOCK_LEN - state->buflen);
	blake2s_compress(state, state->buf, 1, state->buflen);
	cpu_to_le32_array(state->h, ARRAY_SIZE(state->h));
	memcpy(out, state->h, state->outlen);
}

સ્થિર ssize_t getrandom_full(શૂન્ય *buf, size_t ગણતરી, unsigned પૂર્ણાંક flags)
{
	ssize_t ret, total = 0;
	uint8_t *p = buf;

	કરો {
		ret = getrandom(p, ગણતરી, flags);
		જો (ret < 0 && errno == EINTR)
			ચાલુ;
		નહીંતર જો (ret < 0)
			પાછા ret;
		total += ret;
		p += ret;
		ગણતરી -= ret;
	} જ્યારે (ગણતરી);
	પાછા total;
}

સ્થિર ssize_t read_full(પૂર્ણાંક fd, શૂન્ય *buf, size_t ગણતરી)
{
	ssize_t ret, total = 0;
	uint8_t *p = buf;

	કરો {
		ret = પઠ(fd, p, ગણતરી);
		જો (ret < 0 && errno == EINTR)
			ચાલુ;
		નહીંતર જો (ret < 0)
			પાછા ret;
		નહીંતર જો (ret == 0)
			તોડો;
		total += ret;
		p += ret;
		ગણતરી -= ret;
	} જ્યારે (ગણતરી);
	પાછા total;
}

સ્થિર ssize_t write_full(પૂર્ણાંક fd, અચલ શૂન્ય *buf, size_t ગણતરી)
{
	ssize_t ret, total = 0;
	અચલ uint8_t *p = buf;

	કરો {
		ret = લિખ(fd, p, ગણતરી);
		જો (ret < 0 && errno == EINTR)
			ચાલુ;
		નહીંતર જો (ret < 0)
			પાછા ret;
		total += ret;
		p += ret;
		ગણતરી -= ret;
	} જ્યારે (ગણતરી);
	પાછા total;
}

સ્થિર size_t determine_optimal_seed_len(શૂન્ય)
{
	size_t ret = 0;
	અક્ષર poolsize_str[11] = { 0 };
	પૂર્ણાંક fd = open("/proc/sys/kernel/random/poolsize", O_RDONLY);

	જો (fd < 0 || read_full(fd, poolsize_str, કદ(poolsize_str) - 1) < 0) {
		ewarn("%s: Unable to determine pool size, falling back to %u bits: %s",
		      applet, MIN_SEED_LEN * 8, strerror(errno));
		ret = MIN_SEED_LEN;
	} નહીંતર
		ret = DIV_ROUND_UP(strtoul(poolsize_str, ખાલી, 10), 8);
	જો (fd >= 0)
		close(fd);
	જો (ret < MIN_SEED_LEN)
		ret = MIN_SEED_LEN;
	નહીંતર જો (ret > MAX_SEED_LEN)
		ret = MAX_SEED_LEN;
	પાછા ret;
}

સ્થિર પૂર્ણાંક read_new_seed(uint8_t *seed, size_t len, bool *is_creditable)
{
	ssize_t ret;
	પૂર્ણાંક urandom_fd;

	*is_creditable = false;
	ret = getrandom_full(seed, len, GRND_NONBLOCK);
	જો (ret == (ssize_t)len) {
		*is_creditable = true;
		પાછા 0;
	} નહીંતર જો (ret < 0 && errno == ENOSYS) {
		સંરચના pollfd random_fd = {
			.fd = open("/dev/random", O_RDONLY),
			.events = POLLIN
		};
		જો (random_fd.fd < 0)
			પાછા -errno;
		*is_creditable = poll(&random_fd, 1, 0) == 1;
		close(random_fd.fd);
	} નહીંતર જો (getrandom_full(seed, len, GRND_INSECURE) == (ssize_t)len)
		પાછા 0;
	urandom_fd = open("/dev/urandom", O_RDONLY);
	જો (urandom_fd < 0)
		પાછા -1;
	ret = read_full(urandom_fd, seed, len);
	જો (ret == (ssize_t)len)
		ret = 0;
	નહીંતર
		ret = -errno ? -errno : -EIO;
	close(urandom_fd);
	errno = -ret;
	પાછા ret ? -1 : 0;
}

સ્થિર પૂર્ણાંક seed_rng(uint8_t *seed, size_t len, bool credit)
{
	સંરચના {
		પૂર્ણાંક entropy_ગણતરી;
		પૂર્ણાંક buf_size;
		uint8_t buffer[MAX_SEED_LEN];
	} req = {
		.entropy_ગણતરી = credit ? len * 8 : 0,
		.buf_size = len
	};
	પૂર્ણાંક random_fd, ret;

	જો (len > કદ(req.buffer)) {
		errno = EFBIG;
		પાછા -1;
	}
	memcpy(req.buffer, seed, len);

	random_fd = open("/dev/urandom", O_RDONLY);
	જો (random_fd < 0)
		પાછા -1;
	ret = ioctl(random_fd, RNDADDENTROPY, &req);
	જો (ret)
		ret = -errno ? -errno : -EIO;
	close(random_fd);
	errno = -ret;
	પાછા ret ? -1 : 0;
}

સ્થિર પૂર્ણાંક seed_from_file_if_exists(અચલ અક્ષર *filename, પૂર્ણાંક dfd, bool credit, સંરચના blake2s_state *hash)
{
	uint8_t seed[MAX_SEED_LEN];
	ssize_t seed_len;
	પૂર્ણાંક fd = -1, ret = 0;

	fd = openat(dfd, filename, O_RDONLY);
	જો (fd < 0 && errno == ENOENT)
		પાછા 0;
	નહીંતર જો (fd < 0) {
		ret = -errno;
		eerror("%s: Unable to open seed file: %s", applet, strerror(errno));
		જાઓ out;
	}
	seed_len = read_full(fd, seed, કદ(seed));
	જો (seed_len < 0) {
		ret = -errno;
		eerror("%s: Unable to read seed file: %s", applet, strerror(errno));
		જાઓ out;
	}
	જો ((unlinkat(dfd, filename, 0) < 0 || fsync(dfd) < 0) && seed_len) {
		ret = -errno;
		eerror("%s: Unable to remove seed after reading, so not seeding: %s",
		       applet, strerror(errno));
		જાઓ out;
	}
	જો (!seed_len)
		જાઓ out;

	blake2s_update(hash, &seed_len, કદ(seed_len));
	blake2s_update(hash, seed, seed_len);

	einfo("Seeding %zd bits %s crediting", seed_len * 8, credit ? "and" : "without");
	જો (seed_rng(seed, seed_len, credit) < 0) {
		ret = -errno;
		eerror("%s: Unable to seed: %s", applet, strerror(errno));
	}

out:
	જો (fd >= 0)
		close(fd);
	errno = -ret;
	પાછા ret ? -1 : 0;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	સ્થિર અચલ અક્ષર seedrng_prefix[] = "SeedRNG v1 Old+New Prefix";
	સ્થિર અચલ અક્ષર seedrng_failure[] = "SeedRNG v1 No New Seed Failure";
	પૂર્ણાંક opt, fd, dfd, program_ret = 0;
	અચલ અક્ષર *seed_dir = ખાલી;
	uint8_t new_seed[MAX_SEED_LEN];
	size_t new_seed_len;
	bool new_seed_creditable;
	સંરચના timespec realtime = { 0 }, boottime = { 0 };
	સંરચના blake2s_state hash;
	bool skip_credit = false;

	applet = basename_c(argv[0]);

	જ્યારે ((opt = getopt_long(argc, argv, getoptstring, longopts, (પૂર્ણાંક *) 0)) != -1)
	{
		પસંદગી (opt) {
		સ્થિતિ LONGOPT_SEED_DIR:
			જો (!seed_dir)
				seed_dir = optarg;
			તોડો;
		સ્થિતિ LONGOPT_SKIP_CREDIT:
			skip_credit = true;
			તોડો;
		case_RC_COMMON_GETOPT
		}
	}
	જો (!seed_dir)
		seed_dir = DEFAULT_SEED_DIR;
	જો (getuid())
		eerrorx("%s: superuser access is required", applet);
	umask(0077);

	blake2s_init(&hash, BLAKE2S_HASH_LEN);
	blake2s_update(&hash, seedrng_prefix, strlen(seedrng_prefix));
	clock_gettime(CLOCK_REALTIME, &realtime);
	clock_gettime(CLOCK_BOOTTIME, &boottime);
	blake2s_update(&hash, &realtime, કદ(realtime));
	blake2s_update(&hash, &boottime, કદ(boottime));

	જો (mkdir(seed_dir, 0700) < 0 && errno != EEXIST)
		eerrorx("%s: Unable to create seed directory: %s", applet, strerror(errno));

	dfd = open(seed_dir, O_DIRECTORY | O_RDONLY);
	જો (dfd < 0 || flock(dfd, LOCK_EX) < 0)
		eerrorx("%s: Unable to lock seed directory: %s", applet, strerror(errno));

	જો (seed_from_file_if_exists(NON_CREDITABLE_SEED, dfd, false, &hash) < 0)
		program_ret |= 1 << 1;
	જો (seed_from_file_if_exists(CREDITABLE_SEED, dfd, !skip_credit, &hash) < 0)
		program_ret |= 1 << 2;

	new_seed_len = determine_optimal_seed_len();
	જો (read_new_seed(new_seed, new_seed_len, &new_seed_creditable) < 0) {
		eerror("%s: Unable to read new seed: %s", applet, strerror(errno));
		new_seed_len = BLAKE2S_HASH_LEN;
		strncpy((અક્ષર *)new_seed, seedrng_failure, new_seed_len);
		program_ret |= 1 << 3;
	}
	blake2s_update(&hash, &new_seed_len, કદ(new_seed_len));
	blake2s_update(&hash, new_seed, new_seed_len);
	blake2s_final(&hash, new_seed + new_seed_len - BLAKE2S_HASH_LEN);

	einfo("Saving %zu bits of %s seed for next boot", new_seed_len * 8, new_seed_creditable ? "creditable" : "non-creditable");
	fd = openat(dfd, NON_CREDITABLE_SEED, O_WRONLY | O_CREAT | O_TRUNC, 0400);
	જો (fd < 0) {
		eerror("%s: Unable to open seed file for writing: %s", applet, strerror(errno));
		પાછા program_ret | (1 << 4);
	}
	જો (write_full(fd, new_seed, new_seed_len) != (ssize_t)new_seed_len || fsync(fd) < 0) {
		eerror("%s: Unable to write seed file: %s", applet, strerror(errno));
		પાછા program_ret | (1 << 5);
	}
	જો (new_seed_creditable && renameat(dfd, NON_CREDITABLE_SEED, dfd, CREDITABLE_SEED) < 0) {
		ewarn("%s: Unable to make new seed creditable: %s", applet, strerror(errno));
		પાછા program_ret | (1 << 6);
	}
	પાછા program_ret;
}
