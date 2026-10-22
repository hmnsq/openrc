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

#അന്തര്ഭാവ <linux/random.h>
#അന്തര്ഭാവ <sys/random.h>
#അന്തര്ഭാവ <sys/ioctl.h>
#അന്തര്ഭാവ <sys/file.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <poll.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <endian.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "helpers.h"
#അന്തര്ഭാവ "_usage.h"

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
എണ്ണൽ long_opts {
	/* This has to come first so following values stay in the 0x100+ range. */
	LONGOPT_BASE = 0x100,
	LONGOPT_SEED_DIR,
	LONGOPT_SKIP_CREDIT,
};

സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;
സ്ഥിരം അക്ഷരം *extraopts = ശൂന്യ;
സ്ഥിരം അക്ഷരം getoptstring[] = getoptstring_COMMON;
സ്ഥിരം ഘടന option longopts[] = {
	{ "seed-dir", 1, ശൂന്യ, LONGOPT_SEED_DIR },
	{ "skip-credit", 0, ശൂന്യ, LONGOPT_SKIP_CREDIT },
	longopts_COMMON
};
സ്ഥിരം അക്ഷരം * സ്ഥിരം longopts_help[] = {
	"Directory for seed files (default: " DEFAULT_SEED_DIR ")",
	"Skip crediting entropy of seeds",
	longopts_help_COMMON
};
സ്ഥിരം അക്ഷരം *usagestring = ശൂന്യ;

എണ്ണൽ blake2s_lengths {
	BLAKE2S_BLOCK_LEN = 64,
	BLAKE2S_HASH_LEN = 32,
	BLAKE2_S_താക്കോൽ_LEN = 32
};

എണ്ണൽ seedrng_lengths {
	MAX_SEED_LEN = 512,
	MIN_SEED_LEN = BLAKE2S_HASH_LEN
};

ഘടന blake2s_state {
	uint32_t h[8];
	uint32_t t[2];
	uint32_t f[2];
	uint8_t buf[BLAKE2S_BLOCK_LEN];
	unsigned പൂർണ്ണസംഖ്യ buflen;
	unsigned പൂർണ്ണസംഖ്യ outlen;
};

#define le32_to_cpup(a) le32toh(*(a))
#define cpu_to_le32(a) htole32(a)
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (വലുപ്പം(x) / വലുപ്പം((x)[0]))
#endif
#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

സ്ഥിതം വരിയിൽ ശൂന്യം cpu_to_le32_array(uint32_t *buf, unsigned പൂർണ്ണസംഖ്യ വാക്കുകൾ)
{
	ആകുമ്പോൾ (വാക്കുകൾ--) {
		*buf = cpu_to_le32(*buf);
		++buf;
	}
}

സ്ഥിതം വരിയിൽ ശൂന്യം le32_to_cpu_array(uint32_t *buf, unsigned പൂർണ്ണസംഖ്യ വാക്കുകൾ)
{
	ആകുമ്പോൾ (വാക്കുകൾ--) {
		*buf = le32_to_cpup(buf);
		++buf;
	}
}

സ്ഥിതം വരിയിൽ uint32_t ror32(uint32_t word, unsigned പൂർണ്ണസംഖ്യ shift)
{
	തിരികെ (word >> (shift & 31)) | (word << ((-shift) & 31));
}

സ്ഥിതം സ്ഥിരം uint32_t blake2s_iv[8] = {
	0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
	0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

സ്ഥിതം സ്ഥിരം uint8_t blake2s_sigma[10][16] = {
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

സ്ഥിതം ശൂന്യം blake2s_set_lastblock(ഘടന blake2s_state *state)
{
	state->f[0] = -1;
}

സ്ഥിതം ശൂന്യം blake2s_increment_counter(ഘടന blake2s_state *state, സ്ഥിരം uint32_t inc)
{
	state->t[0] += inc;
	state->t[1] += (state->t[0] < inc);
}

സ്ഥിതം ശൂന്യം blake2s_init_param(ഘടന blake2s_state *state, സ്ഥിരം uint32_t param)
{
	പൂർണ്ണസംഖ്യ i;

	memset(state, 0, വലുപ്പം(*state));
	വേണ്ടി (i = 0; i < 8; ++i)
		state->h[i] = blake2s_iv[i];
	state->h[0] ^= param;
}

സ്ഥിതം ശൂന്യം blake2s_init(ഘടന blake2s_state *state, സ്ഥിരം size_t outlen)
{
	blake2s_init_param(state, 0x01010000 | outlen);
	state->outlen = outlen;
}

സ്ഥിതം ശൂന്യം blake2s_compress(ഘടന blake2s_state *state, സ്ഥിരം uint8_t *block, size_t nblocks, സ്ഥിരം uint32_t inc)
{
	uint32_t m[16];
	uint32_t v[16];
	പൂർണ്ണസംഖ്യ i;

	ആകുമ്പോൾ (nblocks > 0) {
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

#define G(r, i, a, b, c, d) ചെയ്യുക { \
	a += b + m[blake2s_sigma[r][2 * i + 0]]; \
	d = ror32(d ^ a, 16); \
	c += d; \
	b = ror32(b ^ c, 12); \
	a += b + m[blake2s_sigma[r][2 * i + 1]]; \
	d = ror32(d ^ a, 8); \
	c += d; \
	b = ror32(b ^ c, 7); \
} ആകുമ്പോൾ (0)

#define ROUND(r) ചെയ്യുക { \
	G(r, 0, v[0], v[ 4], v[ 8], v[12]); \
	G(r, 1, v[1], v[ 5], v[ 9], v[13]); \
	G(r, 2, v[2], v[ 6], v[10], v[14]); \
	G(r, 3, v[3], v[ 7], v[11], v[15]); \
	G(r, 4, v[0], v[ 5], v[10], v[15]); \
	G(r, 5, v[1], v[ 6], v[11], v[12]); \
	G(r, 6, v[2], v[ 7], v[ 8], v[13]); \
	G(r, 7, v[3], v[ 4], v[ 9], v[14]); \
} ആകുമ്പോൾ (0)
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

		വേണ്ടി (i = 0; i < 8; ++i)
			state->h[i] ^= v[i] ^ v[i + 8];

		block += BLAKE2S_BLOCK_LEN;
		--nblocks;
	}
}

സ്ഥിതം ശൂന്യം blake2s_update(ഘടന blake2s_state *state, സ്ഥിരം ശൂന്യം *inp, size_t inlen)
{
	സ്ഥിരം size_t fill = BLAKE2S_BLOCK_LEN - state->buflen;
	സ്ഥിരം uint8_t *in = inp;

	എങ്കിൽ (!inlen)
		തിരികെ;
	എങ്കിൽ (inlen > fill) {
		memcpy(state->buf + state->buflen, in, fill);
		blake2s_compress(state, state->buf, 1, BLAKE2S_BLOCK_LEN);
		state->buflen = 0;
		in += fill;
		inlen -= fill;
	}
	എങ്കിൽ (inlen > BLAKE2S_BLOCK_LEN) {
		സ്ഥിരം size_t nblocks = DIV_ROUND_UP(inlen, BLAKE2S_BLOCK_LEN);
		blake2s_compress(state, in, nblocks - 1, BLAKE2S_BLOCK_LEN);
		in += BLAKE2S_BLOCK_LEN * (nblocks - 1);
		inlen -= BLAKE2S_BLOCK_LEN * (nblocks - 1);
	}
	memcpy(state->buf + state->buflen, in, inlen);
	state->buflen += inlen;
}

സ്ഥിതം ശൂന്യം blake2s_final(ഘടന blake2s_state *state, uint8_t *out)
{
	blake2s_set_lastblock(state);
	memset(state->buf + state->buflen, 0, BLAKE2S_BLOCK_LEN - state->buflen);
	blake2s_compress(state, state->buf, 1, state->buflen);
	cpu_to_le32_array(state->h, ARRAY_SIZE(state->h));
	memcpy(out, state->h, state->outlen);
}

സ്ഥിതം ssize_t getrandom_full(ശൂന്യം *buf, size_t എണ്ണം, unsigned പൂർണ്ണസംഖ്യ flags)
{
	ssize_t ret, total = 0;
	uint8_t *p = buf;

	ചെയ്യുക {
		ret = getrandom(p, എണ്ണം, flags);
		എങ്കിൽ (ret < 0 && errno == EINTR)
			തുടരുക;
		അല്ലെങ്കിൽ എങ്കിൽ (ret < 0)
			തിരികെ ret;
		total += ret;
		p += ret;
		എണ്ണം -= ret;
	} ആകുമ്പോൾ (എണ്ണം);
	തിരികെ total;
}

സ്ഥിതം ssize_t read_full(പൂർണ്ണസംഖ്യ fd, ശൂന്യം *buf, size_t എണ്ണം)
{
	ssize_t ret, total = 0;
	uint8_t *p = buf;

	ചെയ്യുക {
		ret = പഠ(fd, p, എണ്ണം);
		എങ്കിൽ (ret < 0 && errno == EINTR)
			തുടരുക;
		അല്ലെങ്കിൽ എങ്കിൽ (ret < 0)
			തിരികെ ret;
		അല്ലെങ്കിൽ എങ്കിൽ (ret == 0)
			നിർത്തുക;
		total += ret;
		p += ret;
		എണ്ണം -= ret;
	} ആകുമ്പോൾ (എണ്ണം);
	തിരികെ total;
}

സ്ഥിതം ssize_t write_full(പൂർണ്ണസംഖ്യ fd, സ്ഥിരം ശൂന്യം *buf, size_t എണ്ണം)
{
	ssize_t ret, total = 0;
	സ്ഥിരം uint8_t *p = buf;

	ചെയ്യുക {
		ret = ലിഖ(fd, p, എണ്ണം);
		എങ്കിൽ (ret < 0 && errno == EINTR)
			തുടരുക;
		അല്ലെങ്കിൽ എങ്കിൽ (ret < 0)
			തിരികെ ret;
		total += ret;
		p += ret;
		എണ്ണം -= ret;
	} ആകുമ്പോൾ (എണ്ണം);
	തിരികെ total;
}

സ്ഥിതം size_t determine_optimal_seed_len(ശൂന്യം)
{
	size_t ret = 0;
	അക്ഷരം poolsize_str[11] = { 0 };
	പൂർണ്ണസംഖ്യ fd = open("/proc/sys/kernel/random/poolsize", O_RDONLY);

	എങ്കിൽ (fd < 0 || read_full(fd, poolsize_str, വലുപ്പം(poolsize_str) - 1) < 0) {
		ewarn("%s: Unable to determine pool size, falling back to %u bits: %s",
		      applet, MIN_SEED_LEN * 8, strerror(errno));
		ret = MIN_SEED_LEN;
	} അല്ലെങ്കിൽ
		ret = DIV_ROUND_UP(strtoul(poolsize_str, ശൂന്യ, 10), 8);
	എങ്കിൽ (fd >= 0)
		close(fd);
	എങ്കിൽ (ret < MIN_SEED_LEN)
		ret = MIN_SEED_LEN;
	അല്ലെങ്കിൽ എങ്കിൽ (ret > MAX_SEED_LEN)
		ret = MAX_SEED_LEN;
	തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ read_new_seed(uint8_t *seed, size_t len, bool *is_creditable)
{
	ssize_t ret;
	പൂർണ്ണസംഖ്യ urandom_fd;

	*is_creditable = false;
	ret = getrandom_full(seed, len, GRND_NONBLOCK);
	എങ്കിൽ (ret == (ssize_t)len) {
		*is_creditable = true;
		തിരികെ 0;
	} അല്ലെങ്കിൽ എങ്കിൽ (ret < 0 && errno == ENOSYS) {
		ഘടന pollfd random_fd = {
			.fd = open("/dev/random", O_RDONLY),
			.events = POLLIN
		};
		എങ്കിൽ (random_fd.fd < 0)
			തിരികെ -errno;
		*is_creditable = poll(&random_fd, 1, 0) == 1;
		close(random_fd.fd);
	} അല്ലെങ്കിൽ എങ്കിൽ (getrandom_full(seed, len, GRND_INSECURE) == (ssize_t)len)
		തിരികെ 0;
	urandom_fd = open("/dev/urandom", O_RDONLY);
	എങ്കിൽ (urandom_fd < 0)
		തിരികെ -1;
	ret = read_full(urandom_fd, seed, len);
	എങ്കിൽ (ret == (ssize_t)len)
		ret = 0;
	അല്ലെങ്കിൽ
		ret = -errno ? -errno : -EIO;
	close(urandom_fd);
	errno = -ret;
	തിരികെ ret ? -1 : 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ seed_rng(uint8_t *seed, size_t len, bool credit)
{
	ഘടന {
		പൂർണ്ണസംഖ്യ entropy_എണ്ണം;
		പൂർണ്ണസംഖ്യ buf_size;
		uint8_t buffer[MAX_SEED_LEN];
	} req = {
		.entropy_എണ്ണം = credit ? len * 8 : 0,
		.buf_size = len
	};
	പൂർണ്ണസംഖ്യ random_fd, ret;

	എങ്കിൽ (len > വലുപ്പം(req.buffer)) {
		errno = EFBIG;
		തിരികെ -1;
	}
	memcpy(req.buffer, seed, len);

	random_fd = open("/dev/urandom", O_RDONLY);
	എങ്കിൽ (random_fd < 0)
		തിരികെ -1;
	ret = ioctl(random_fd, RNDADDENTROPY, &req);
	എങ്കിൽ (ret)
		ret = -errno ? -errno : -EIO;
	close(random_fd);
	errno = -ret;
	തിരികെ ret ? -1 : 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ seed_from_file_if_exists(സ്ഥിരം അക്ഷരം *filename, പൂർണ്ണസംഖ്യ dfd, bool credit, ഘടന blake2s_state *hash)
{
	uint8_t seed[MAX_SEED_LEN];
	ssize_t seed_len;
	പൂർണ്ണസംഖ്യ fd = -1, ret = 0;

	fd = openat(dfd, filename, O_RDONLY);
	എങ്കിൽ (fd < 0 && errno == ENOENT)
		തിരികെ 0;
	അല്ലെങ്കിൽ എങ്കിൽ (fd < 0) {
		ret = -errno;
		eerror("%s: Unable to open seed file: %s", applet, strerror(errno));
		പോകുക out;
	}
	seed_len = read_full(fd, seed, വലുപ്പം(seed));
	എങ്കിൽ (seed_len < 0) {
		ret = -errno;
		eerror("%s: Unable to read seed file: %s", applet, strerror(errno));
		പോകുക out;
	}
	എങ്കിൽ ((unlinkat(dfd, filename, 0) < 0 || fsync(dfd) < 0) && seed_len) {
		ret = -errno;
		eerror("%s: Unable to remove seed after reading, so not seeding: %s",
		       applet, strerror(errno));
		പോകുക out;
	}
	എങ്കിൽ (!seed_len)
		പോകുക out;

	blake2s_update(hash, &seed_len, വലുപ്പം(seed_len));
	blake2s_update(hash, seed, seed_len);

	einfo("Seeding %zd bits %s crediting", seed_len * 8, credit ? "and" : "without");
	എങ്കിൽ (seed_rng(seed, seed_len, credit) < 0) {
		ret = -errno;
		eerror("%s: Unable to seed: %s", applet, strerror(errno));
	}

out:
	എങ്കിൽ (fd >= 0)
		close(fd);
	errno = -ret;
	തിരികെ ret ? -1 : 0;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	സ്ഥിതം സ്ഥിരം അക്ഷരം seedrng_prefix[] = "SeedRNG v1 Old+New Prefix";
	സ്ഥിതം സ്ഥിരം അക്ഷരം seedrng_failure[] = "SeedRNG v1 No New Seed Failure";
	പൂർണ്ണസംഖ്യ opt, fd, dfd, program_ret = 0;
	സ്ഥിരം അക്ഷരം *seed_dir = ശൂന്യ;
	uint8_t new_seed[MAX_SEED_LEN];
	size_t new_seed_len;
	bool new_seed_creditable;
	ഘടന timespec realtime = { 0 }, boottime = { 0 };
	ഘടന blake2s_state hash;
	bool skip_credit = false;

	applet = basename_c(argv[0]);

	ആകുമ്പോൾ ((opt = getopt_long(argc, argv, getoptstring, longopts, (പൂർണ്ണസംഖ്യ *) 0)) != -1)
	{
		തിരഞ്ഞെടുപ്പ് (opt) {
		അവസ്ഥ LONGOPT_SEED_DIR:
			എങ്കിൽ (!seed_dir)
				seed_dir = optarg;
			നിർത്തുക;
		അവസ്ഥ LONGOPT_SKIP_CREDIT:
			skip_credit = true;
			നിർത്തുക;
		case_RC_COMMON_GETOPT
		}
	}
	എങ്കിൽ (!seed_dir)
		seed_dir = DEFAULT_SEED_DIR;
	എങ്കിൽ (getuid())
		eerrorx("%s: superuser access is required", applet);
	umask(0077);

	blake2s_init(&hash, BLAKE2S_HASH_LEN);
	blake2s_update(&hash, seedrng_prefix, strlen(seedrng_prefix));
	clock_gettime(CLOCK_REALTIME, &realtime);
	clock_gettime(CLOCK_BOOTTIME, &boottime);
	blake2s_update(&hash, &realtime, വലുപ്പം(realtime));
	blake2s_update(&hash, &boottime, വലുപ്പം(boottime));

	എങ്കിൽ (mkdir(seed_dir, 0700) < 0 && errno != EEXIST)
		eerrorx("%s: Unable to create seed directory: %s", applet, strerror(errno));

	dfd = open(seed_dir, O_DIRECTORY | O_RDONLY);
	എങ്കിൽ (dfd < 0 || flock(dfd, LOCK_EX) < 0)
		eerrorx("%s: Unable to lock seed directory: %s", applet, strerror(errno));

	എങ്കിൽ (seed_from_file_if_exists(NON_CREDITABLE_SEED, dfd, false, &hash) < 0)
		program_ret |= 1 << 1;
	എങ്കിൽ (seed_from_file_if_exists(CREDITABLE_SEED, dfd, !skip_credit, &hash) < 0)
		program_ret |= 1 << 2;

	new_seed_len = determine_optimal_seed_len();
	എങ്കിൽ (read_new_seed(new_seed, new_seed_len, &new_seed_creditable) < 0) {
		eerror("%s: Unable to read new seed: %s", applet, strerror(errno));
		new_seed_len = BLAKE2S_HASH_LEN;
		strncpy((അക്ഷരം *)new_seed, seedrng_failure, new_seed_len);
		program_ret |= 1 << 3;
	}
	blake2s_update(&hash, &new_seed_len, വലുപ്പം(new_seed_len));
	blake2s_update(&hash, new_seed, new_seed_len);
	blake2s_final(&hash, new_seed + new_seed_len - BLAKE2S_HASH_LEN);

	einfo("Saving %zu bits of %s seed for next boot", new_seed_len * 8, new_seed_creditable ? "creditable" : "non-creditable");
	fd = openat(dfd, NON_CREDITABLE_SEED, O_WRONLY | O_CREAT | O_TRUNC, 0400);
	എങ്കിൽ (fd < 0) {
		eerror("%s: Unable to open seed file for writing: %s", applet, strerror(errno));
		തിരികെ program_ret | (1 << 4);
	}
	എങ്കിൽ (write_full(fd, new_seed, new_seed_len) != (ssize_t)new_seed_len || fsync(fd) < 0) {
		eerror("%s: Unable to write seed file: %s", applet, strerror(errno));
		തിരികെ program_ret | (1 << 5);
	}
	എങ്കിൽ (new_seed_creditable && renameat(dfd, NON_CREDITABLE_SEED, dfd, CREDITABLE_SEED) < 0) {
		ewarn("%s: Unable to make new seed creditable: %s", applet, strerror(errno));
		തിരികെ program_ret | (1 << 6);
	}
	തിരികെ program_ret;
}
