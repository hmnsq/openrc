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

#అన్తర్భావ <linux/random.h>
#అన్తర్భావ <sys/random.h>
#అన్తర్భావ <sys/ioctl.h>
#అన్తర్భావ <sys/file.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <poll.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <endian.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "helpers.h"
#అన్తర్భావ "_usage.h"

#నిర్వచితం_కాకపోతే GRND_INSECURE
#నిర్వచించు GRND_INSECURE 0x0004 /* Apparently some headers don't ship with this yet. */
#అయితే_ముగింపు

#నిర్వచించు DEFAULT_విత్తనం_DIR "/var/lib/seedrng"
#నిర్వచించు CREDITABLE_విత్తనం "seed.credit"
#నిర్వచించు NON_CREDITABLE_విత్తనం "seed.no-credit"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
గణన long_opts {
	/* This has to come first so following values stay in the 0x100+ range. */
	LONGOPT_పునాది = 0x100,
	LONGOPT_విత్తనం_DIR,
	LONGOPT_SKIP_CREDIT,
};

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *extraopts = ఖాళీ;
స్థిరరాశి అక్షరం getoptstring[] = getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "seed-dir", 1, ఖాళీ, LONGOPT_విత్తనం_DIR },
	{ "skip-credit", 0, ఖాళీ, LONGOPT_SKIP_CREDIT },
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
	"Directory for seed files (default: " DEFAULT_విత్తనం_DIR ")",
	"Skip crediting entropy of seeds",
	longopts_help_COMMON
};
స్థిరరాశి అక్షరం *usagestring = ఖాళీ;

గణన blake2s_lengths {
	BLAKE2_S_ఖండం_LEN = 64,
	BLAKE2S_HASH_LEN = 32,
	BLAKE2_S_కీలకం_LEN = 32
};

గణన seedrng_lengths {
	MAX_విత్తనం_LEN = 512,
	MIN_విత్తనం_LEN = BLAKE2S_HASH_LEN
};

నిర్మాణం blake2s_state {
	uint32_t h[8];
	uint32_t t[2];
	uint32_t f[2];
	uint8_t buf[BLAKE2_S_ఖండం_LEN];
	గుర్తులేని పూర్ణాంకం buflen;
	గుర్తులేని పూర్ణాంకం outlen;
};

#నిర్వచించు le32_గమ్యం_cpup(a) le32toh(*(a))
#నిర్వచించు cpu_గమ్యం_le32(a) htole32(a)
#నిర్వచితం_కాకపోతే ARRAY_కొలత
#నిర్వచించు ARRAY_కొలత(x) (పరిమాణం(x) / పరిమాణం((x)[0]))
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే DIV_విడత_పైకి
#నిర్వచించు DIV_విడత_పైకి(n, d) (((n) + (d) - 1) / (d))
#అయితే_ముగింపు

స్థిర అంతఃవరుస శూన్య cpu_గమ్యం_le32_array(uint32_t *buf, గుర్తులేని పూర్ణాంకం పదాలు)
{
	అయితే (పదాలు--) {
		*buf = cpu_గమ్యం_le32(*buf);
		++buf;
	}
}

స్థిర అంతఃవరుస శూన్య le32_గమ్యం_cpu_array(uint32_t *buf, గుర్తులేని పూర్ణాంకం పదాలు)
{
	అయితే (పదాలు--) {
		*buf = le32_గమ్యం_cpup(buf);
		++buf;
	}
}

స్థిర అంతఃవరుస uint32_t ror32(uint32_t మాట, గుర్తులేని పూర్ణాంకం shift)
{
	తిరిగి (మాట >> (shift & 31)) | (మాట << ((-shift) & 31));
}

స్థిర స్థిరరాశి uint32_t blake2s_iv[8] = {
	0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
	0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

స్థిర స్థిరరాశి uint8_t blake2s_sigma[10][16] = {
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

స్థిర శూన్య blake2s_సమితి_lastblock(నిర్మాణం blake2s_state *state)
{
	state->f[0] = -1;
}

స్థిర శూన్య blake2s_increment_counter(నిర్మాణం blake2s_state *state, స్థిరరాశి uint32_t inc)
{
	state->t[0] += inc;
	state->t[1] += (state->t[0] < inc);
}

స్థిర శూన్య blake2s_init_param(నిర్మాణం blake2s_state *state, స్థిరరాశి uint32_t param)
{
	పూర్ణాంకం i;

	memset(state, 0, పరిమాణం(*state));
	కొరకు (i = 0; i < 8; ++i)
		state->h[i] = blake2s_iv[i];
	state->h[0] ^= param;
}

స్థిర శూన్య blake2s_init(నిర్మాణం blake2s_state *state, స్థిరరాశి size_t outlen)
{
	blake2s_init_param(state, 0x01010000 | outlen);
	state->outlen = outlen;
}

స్థిర శూన్య blake2s_compress(నిర్మాణం blake2s_state *state, స్థిరరాశి uint8_t *ఖండం, size_t nblocks, స్థిరరాశి uint32_t inc)
{
	uint32_t m[16];
	uint32_t v[16];
	పూర్ణాంకం i;

	అయితే (nblocks > 0) {
		blake2s_increment_counter(state, inc);
		జ్ఞాపకం_నకలు(m, ఖండం, BLAKE2_S_ఖండం_LEN);
		le32_గమ్యం_cpu_array(m, ARRAY_కొలత(m));
		జ్ఞాపకం_నకలు(v, state->h, 32);
		v[ 8] = blake2s_iv[0];
		v[ 9] = blake2s_iv[1];
		v[10] = blake2s_iv[2];
		v[11] = blake2s_iv[3];
		v[12] = blake2s_iv[4] ^ state->t[0];
		v[13] = blake2s_iv[5] ^ state->t[1];
		v[14] = blake2s_iv[6] ^ state->f[0];
		v[15] = blake2s_iv[7] ^ state->f[1];

#నిర్వచించు G(r, i, a, b, c, d) చేయి { \
	a += b + m[blake2s_sigma[r][2 * i + 0]]; \
	d = ror32(d ^ a, 16); \
	c += d; \
	b = ror32(b ^ c, 12); \
	a += b + m[blake2s_sigma[r][2 * i + 1]]; \
	d = ror32(d ^ a, 8); \
	c += d; \
	b = ror32(b ^ c, 7); \
} అయితే (0)

#నిర్వచించు విడత(r) చేయి { \
	G(r, 0, v[0], v[ 4], v[ 8], v[12]); \
	G(r, 1, v[1], v[ 5], v[ 9], v[13]); \
	G(r, 2, v[2], v[ 6], v[10], v[14]); \
	G(r, 3, v[3], v[ 7], v[11], v[15]); \
	G(r, 4, v[0], v[ 5], v[10], v[15]); \
	G(r, 5, v[1], v[ 6], v[11], v[12]); \
	G(r, 6, v[2], v[ 7], v[ 8], v[13]); \
	G(r, 7, v[3], v[ 4], v[ 9], v[14]); \
} అయితే (0)
		విడత(0);
		విడత(1);
		విడత(2);
		విడత(3);
		విడత(4);
		విడత(5);
		విడత(6);
		విడత(7);
		విడత(8);
		విడత(9);

#నిర్వచనం_రద్దు G
#నిర్వచనం_రద్దు విడత

		కొరకు (i = 0; i < 8; ++i)
			state->h[i] ^= v[i] ^ v[i + 8];

		ఖండం += BLAKE2_S_ఖండం_LEN;
		--nblocks;
	}
}

స్థిర శూన్య blake2s_update(నిర్మాణం blake2s_state *state, స్థిరరాశి శూన్య *inp, size_t inlen)
{
	స్థిరరాశి size_t fill = BLAKE2_S_ఖండం_LEN - state->buflen;
	స్థిరరాశి uint8_t *లోపల = inp;

	ఒకవేళ (!inlen)
		తిరిగి;
	ఒకవేళ (inlen > fill) {
		జ్ఞాపకం_నకలు(state->buf + state->buflen, లోపల, fill);
		blake2s_compress(state, state->buf, 1, BLAKE2_S_ఖండం_LEN);
		state->buflen = 0;
		లోపల += fill;
		inlen -= fill;
	}
	ఒకవేళ (inlen > BLAKE2_S_ఖండం_LEN) {
		స్థిరరాశి size_t nblocks = DIV_విడత_పైకి(inlen, BLAKE2_S_ఖండం_LEN);
		blake2s_compress(state, లోపల, nblocks - 1, BLAKE2_S_ఖండం_LEN);
		లోపల += BLAKE2_S_ఖండం_LEN * (nblocks - 1);
		inlen -= BLAKE2_S_ఖండం_LEN * (nblocks - 1);
	}
	జ్ఞాపకం_నకలు(state->buf + state->buflen, లోపల, inlen);
	state->buflen += inlen;
}

స్థిర శూన్య blake2s_final(నిర్మాణం blake2s_state *state, uint8_t *బహిర్గతం)
{
	blake2s_సమితి_lastblock(state);
	memset(state->buf + state->buflen, 0, BLAKE2_S_ఖండం_LEN - state->buflen);
	blake2s_compress(state, state->buf, 1, state->buflen);
	cpu_గమ్యం_le32_array(state->h, ARRAY_కొలత(state->h));
	జ్ఞాపకం_నకలు(బహిర్గతం, state->h, state->outlen);
}

స్థిర ssize_t getrandom_నిండిన(శూన్య *buf, size_t లెక్క, గుర్తులేని పూర్ణాంకం flags)
{
	ssize_t ret, మొత్తం = 0;
	uint8_t *p = buf;

	చేయి {
		ret = getrandom(p, లెక్క, flags);
		ఒకవేళ (ret < 0 && errno == EINTR)
			కొనసాగు;
		లేకపోతే ఒకవేళ (ret < 0)
			తిరిగి ret;
		మొత్తం += ret;
		p += ret;
		లెక్క -= ret;
	} అయితే (లెక్క);
	తిరిగి మొత్తం;
}

స్థిర ssize_t read_నిండిన(పూర్ణాంకం fd, శూన్య *buf, size_t లెక్క)
{
	ssize_t ret, మొత్తం = 0;
	uint8_t *p = buf;

	చేయి {
		ret = పఠ(fd, p, లెక్క);
		ఒకవేళ (ret < 0 && errno == EINTR)
			కొనసాగు;
		లేకపోతే ఒకవేళ (ret < 0)
			తిరిగి ret;
		లేకపోతే ఒకవేళ (ret == 0)
			ఆపు;
		మొత్తం += ret;
		p += ret;
		లెక్క -= ret;
	} అయితే (లెక్క);
	తిరిగి మొత్తం;
}

స్థిర ssize_t write_నిండిన(పూర్ణాంకం fd, స్థిరరాశి శూన్య *buf, size_t లెక్క)
{
	ssize_t ret, మొత్తం = 0;
	స్థిరరాశి uint8_t *p = buf;

	చేయి {
		ret = లిఖ(fd, p, లెక్క);
		ఒకవేళ (ret < 0 && errno == EINTR)
			కొనసాగు;
		లేకపోతే ఒకవేళ (ret < 0)
			తిరిగి ret;
		మొత్తం += ret;
		p += ret;
		లెక్క -= ret;
	} అయితే (లెక్క);
	తిరిగి మొత్తం;
}

స్థిర size_t determine_optimal_విత్తనం_len(శూన్య)
{
	size_t ret = 0;
	అక్షరం poolsize_str[11] = { 0 };
	పూర్ణాంకం fd = open("/proc/sys/kernel/random/poolsize", O_RDONLY);

	ఒకవేళ (fd < 0 || read_నిండిన(fd, poolsize_str, పరిమాణం(poolsize_str) - 1) < 0) {
		ewarn("%s: Unable to determine pool size, falling back to %u bits: %s",
		      applet, MIN_విత్తనం_LEN * 8, strerror(errno));
		ret = MIN_విత్తనం_LEN;
	} లేకపోతే
		ret = DIV_విడత_పైకి(strtoul(poolsize_str, ఖాళీ, 10), 8);
	ఒకవేళ (fd >= 0)
		close(fd);
	ఒకవేళ (ret < MIN_విత్తనం_LEN)
		ret = MIN_విత్తనం_LEN;
	లేకపోతే ఒకవేళ (ret > MAX_విత్తనం_LEN)
		ret = MAX_విత్తనం_LEN;
	తిరిగి ret;
}

స్థిర పూర్ణాంకం read_new_విత్తనం(uint8_t *విత్తనం, size_t len, bool *is_creditable)
{
	ssize_t ret;
	పూర్ణాంకం urandom_fd;

	*is_creditable = false;
	ret = getrandom_నిండిన(విత్తనం, len, GRND_NONBLOCK);
	ఒకవేళ (ret == (ssize_t)len) {
		*is_creditable = true;
		తిరిగి 0;
	} లేకపోతే ఒకవేళ (ret < 0 && errno == ENOSYS) {
		నిర్మాణం pollfd random_fd = {
			.fd = open("/dev/random", O_RDONLY),
			.events = POLLIN
		};
		ఒకవేళ (random_fd.fd < 0)
			తిరిగి -errno;
		*is_creditable = poll(&random_fd, 1, 0) == 1;
		close(random_fd.fd);
	} లేకపోతే ఒకవేళ (getrandom_నిండిన(విత్తనం, len, GRND_INSECURE) == (ssize_t)len)
		తిరిగి 0;
	urandom_fd = open("/dev/urandom", O_RDONLY);
	ఒకవేళ (urandom_fd < 0)
		తిరిగి -1;
	ret = read_నిండిన(urandom_fd, విత్తనం, len);
	ఒకవేళ (ret == (ssize_t)len)
		ret = 0;
	లేకపోతే
		ret = -errno ? -errno : -EIO;
	close(urandom_fd);
	errno = -ret;
	తిరిగి ret ? -1 : 0;
}

స్థిర పూర్ణాంకం విత్తనం_rng(uint8_t *విత్తనం, size_t len, bool credit)
{
	నిర్మాణం {
		పూర్ణాంకం entropy_లెక్క;
		పూర్ణాంకం buf_కొలత;
		uint8_t బఫరు[MAX_విత్తనం_LEN];
	} req = {
		.entropy_లెక్క = credit ? len * 8 : 0,
		.buf_కొలత = len
	};
	పూర్ణాంకం random_fd, ret;

	ఒకవేళ (len > పరిమాణం(req.బఫరు)) {
		errno = EFBIG;
		తిరిగి -1;
	}
	జ్ఞాపకం_నకలు(req.బఫరు, విత్తనం, len);

	random_fd = open("/dev/urandom", O_RDONLY);
	ఒకవేళ (random_fd < 0)
		తిరిగి -1;
	ret = ioctl(random_fd, RNDADDENTROPY, &req);
	ఒకవేళ (ret)
		ret = -errno ? -errno : -EIO;
	close(random_fd);
	errno = -ret;
	తిరిగి ret ? -1 : 0;
}

స్థిర పూర్ణాంకం విత్తనం_from_file_if_exists(స్థిరరాశి అక్షరం *filename, పూర్ణాంకం dfd, bool credit, నిర్మాణం blake2s_state *hash)
{
	uint8_t విత్తనం[MAX_విత్తనం_LEN];
	ssize_t విత్తనం_len;
	పూర్ణాంకం fd = -1, ret = 0;

	fd = openat(dfd, filename, O_RDONLY);
	ఒకవేళ (fd < 0 && errno == ENOENT)
		తిరిగి 0;
	లేకపోతే ఒకవేళ (fd < 0) {
		ret = -errno;
		eerror("%s: Unable to open seed file: %s", applet, strerror(errno));
		వెళ్ళు బహిర్గతం;
	}
	విత్తనం_len = read_నిండిన(fd, విత్తనం, పరిమాణం(విత్తనం));
	ఒకవేళ (విత్తనం_len < 0) {
		ret = -errno;
		eerror("%s: Unable to read seed file: %s", applet, strerror(errno));
		వెళ్ళు బహిర్గతం;
	}
	ఒకవేళ ((unlinkat(dfd, filename, 0) < 0 || fsync(dfd) < 0) && విత్తనం_len) {
		ret = -errno;
		eerror("%s: Unable to remove seed after reading, so not seeding: %s",
		       applet, strerror(errno));
		వెళ్ళు బహిర్గతం;
	}
	ఒకవేళ (!విత్తనం_len)
		వెళ్ళు బహిర్గతం;

	blake2s_update(hash, &విత్తనం_len, పరిమాణం(విత్తనం_len));
	blake2s_update(hash, విత్తనం, విత్తనం_len);

	einfo("Seeding %zd bits %s crediting", విత్తనం_len * 8, credit ? "and" : "without");
	ఒకవేళ (విత్తనం_rng(విత్తనం, విత్తనం_len, credit) < 0) {
		ret = -errno;
		eerror("%s: Unable to seed: %s", applet, strerror(errno));
	}

బహిర్గతం:
	ఒకవేళ (fd >= 0)
		close(fd);
	errno = -ret;
	తిరిగి ret ? -1 : 0;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	స్థిర స్థిరరాశి అక్షరం seedrng_ఉపసర్గ[] = "SeedRNG v1 Old+New Prefix";
	స్థిర స్థిరరాశి అక్షరం seedrng_failure[] = "SeedRNG v1 No New Seed Failure";
	పూర్ణాంకం opt, fd, dfd, కార్యక్రమం_ret = 0;
	స్థిరరాశి అక్షరం *విత్తనం_dir = ఖాళీ;
	uint8_t new_విత్తనం[MAX_విత్తనం_LEN];
	size_t new_విత్తనం_len;
	bool new_విత్తనం_creditable;
	నిర్మాణం timespec realtime = { 0 }, boottime = { 0 };
	నిర్మాణం blake2s_state hash;
	bool skip_credit = false;

	applet = basename_c(argv[0]);

	అయితే ((opt = getopt_long(argc, argv, getoptstring, longopts, (పూర్ణాంకం *) 0)) != -1)
	{
		ఎంపిక (opt) {
		స్థితి LONGOPT_విత్తనం_DIR:
			ఒకవేళ (!విత్తనం_dir)
				విత్తనం_dir = optarg;
			ఆపు;
		స్థితి LONGOPT_SKIP_CREDIT:
			skip_credit = true;
			ఆపు;
		case_RC_COMMON_GETOPT
		}
	}
	ఒకవేళ (!విత్తనం_dir)
		విత్తనం_dir = DEFAULT_విత్తనం_DIR;
	ఒకవేళ (getuid())
		eerrorx("%s: superuser access is required", applet);
	umask(0077);

	blake2s_init(&hash, BLAKE2S_HASH_LEN);
	blake2s_update(&hash, seedrng_ఉపసర్గ, అక్షరశ్రేణి_పొడవు(seedrng_ఉపసర్గ));
	clock_gettime(CLOCK_REALTIME, &realtime);
	clock_gettime(CLOCK_BOOTTIME, &boottime);
	blake2s_update(&hash, &realtime, పరిమాణం(realtime));
	blake2s_update(&hash, &boottime, పరిమాణం(boottime));

	ఒకవేళ (mkdir(విత్తనం_dir, 0700) < 0 && errno != EEXIST)
		eerrorx("%s: Unable to create seed directory: %s", applet, strerror(errno));

	dfd = open(విత్తనం_dir, O_DIRECTORY | O_RDONLY);
	ఒకవేళ (dfd < 0 || flock(dfd, LOCK_EX) < 0)
		eerrorx("%s: Unable to lock seed directory: %s", applet, strerror(errno));

	ఒకవేళ (విత్తనం_from_file_if_exists(NON_CREDITABLE_విత్తనం, dfd, false, &hash) < 0)
		కార్యక్రమం_ret |= 1 << 1;
	ఒకవేళ (విత్తనం_from_file_if_exists(CREDITABLE_విత్తనం, dfd, !skip_credit, &hash) < 0)
		కార్యక్రమం_ret |= 1 << 2;

	new_విత్తనం_len = determine_optimal_విత్తనం_len();
	ఒకవేళ (read_new_విత్తనం(new_విత్తనం, new_విత్తనం_len, &new_విత్తనం_creditable) < 0) {
		eerror("%s: Unable to read new seed: %s", applet, strerror(errno));
		new_విత్తనం_len = BLAKE2S_HASH_LEN;
		strncpy((అక్షరం *)new_విత్తనం, seedrng_failure, new_విత్తనం_len);
		కార్యక్రమం_ret |= 1 << 3;
	}
	blake2s_update(&hash, &new_విత్తనం_len, పరిమాణం(new_విత్తనం_len));
	blake2s_update(&hash, new_విత్తనం, new_విత్తనం_len);
	blake2s_final(&hash, new_విత్తనం + new_విత్తనం_len - BLAKE2S_HASH_LEN);

	einfo("Saving %zu bits of %s seed for next boot", new_విత్తనం_len * 8, new_విత్తనం_creditable ? "creditable" : "non-creditable");
	fd = openat(dfd, NON_CREDITABLE_విత్తనం, O_WRONLY | O_CREAT | O_TRUNC, 0400);
	ఒకవేళ (fd < 0) {
		eerror("%s: Unable to open seed file for writing: %s", applet, strerror(errno));
		తిరిగి కార్యక్రమం_ret | (1 << 4);
	}
	ఒకవేళ (write_నిండిన(fd, new_విత్తనం, new_విత్తనం_len) != (ssize_t)new_విత్తనం_len || fsync(fd) < 0) {
		eerror("%s: Unable to write seed file: %s", applet, strerror(errno));
		తిరిగి కార్యక్రమం_ret | (1 << 5);
	}
	ఒకవేళ (new_విత్తనం_creditable && renameat(dfd, NON_CREDITABLE_విత్తనం, dfd, CREDITABLE_విత్తనం) < 0) {
		ewarn("%s: Unable to make new seed creditable: %s", applet, strerror(errno));
		తిరిగి కార్యక్రమం_ret | (1 << 6);
	}
	తిరిగి కార్యక్రమం_ret;
}
