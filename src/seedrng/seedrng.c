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

#अन्तर्भाव <linux/random.h>
#अन्तर्भाव <sys/random.h>
#अन्तर्भाव <sys/ioctl.h>
#अन्तर्भाव <sys/file.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <poll.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <endian.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "helpers.h"
#अन्तर्भाव "_usage.h"

#यदि_अपरिभाषितम् GRND_INSECURE
#परिभाषय GRND_INSECURE 0x0004 /* Apparently some headers don't ship with this yet. */
#यदि_अन्तः

#परिभाषय DEFAULT_बीजम्_DIR "/var/lib/seedrng"
#परिभाषय CREDITABLE_बीजम् "seed.credit"
#परिभाषय NON_CREDITABLE_बीजम् "seed.no-credit"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
गणना long_opts {
	/* This has to come first so following values stay in the 0x100+ range. */
	LONGOPT_आधारः = 0x100,
	LONGOPT_बीजम्_DIR,
	LONGOPT_SKIP_CREDIT,
};

ध्रुव वर्ण *applet = रिक्त;
ध्रुव वर्ण *extraopts = रिक्त;
ध्रुव वर्ण getoptstring[] = getoptstring_COMMON;
ध्रुव संरचना option longopts[] = {
	{ "seed-dir", 1, रिक्त, LONGOPT_बीजम्_DIR },
	{ "skip-credit", 0, रिक्त, LONGOPT_SKIP_CREDIT },
	longopts_COMMON
};
ध्रुव वर्ण * ध्रुव longopts_help[] = {
	"Directory for seed files (default: " DEFAULT_बीजम्_DIR ")",
	"Skip crediting entropy of seeds",
	longopts_help_COMMON
};
ध्रुव वर्ण *usagestring = रिक्त;

गणना blake2s_lengths {
	BLAKE2_S_खण्डम्_LEN = 64,
	BLAKE2S_HASH_LEN = 32,
	BLAKE2_S_कुञ्चिका_LEN = 32
};

गणना seedrng_lengths {
	MAX_बीजम्_LEN = 512,
	MIN_बीजम्_LEN = BLAKE2S_HASH_LEN
};

संरचना blake2s_state {
	uint32_t h[8];
	uint32_t t[2];
	uint32_t f[2];
	uint8_t buf[BLAKE2_S_खण्डम्_LEN];
	अचिह्नित पूर्णाङ्क buflen;
	अचिह्नित पूर्णाङ्क outlen;
};

#परिभाषय le32_गन्तव्यम्_cpup(a) le32toh(*(a))
#परिभाषय cpu_गन्तव्यम्_le32(a) htole32(a)
#यदि_अपरिभाषितम् ARRAY_आकार
#परिभाषय ARRAY_आकार(x) (परिमाण(x) / परिमाण((x)[0]))
#यदि_अन्तः
#यदि_अपरिभाषितम् DIV_आवर्तनम्_ऊर्ध्वम्
#परिभाषय DIV_आवर्तनम्_ऊर्ध्वम्(n, d) (((n) + (d) - 1) / (d))
#यदि_अन्तः

स्थिर अन्तःस्थ शून्य cpu_गन्तव्यम्_le32_array(uint32_t *buf, अचिह्नित पूर्णाङ्क शब्दाः)
{
	यावत् (शब्दाः--) {
		*buf = cpu_गन्तव्यम्_le32(*buf);
		++buf;
	}
}

स्थिर अन्तःस्थ शून्य le32_गन्तव्यम्_cpu_array(uint32_t *buf, अचिह्नित पूर्णाङ्क शब्दाः)
{
	यावत् (शब्दाः--) {
		*buf = le32_गन्तव्यम्_cpup(buf);
		++buf;
	}
}

स्थिर अन्तःस्थ uint32_t ror32(uint32_t word, अचिह्नित पूर्णाङ्क shift)
{
	प्रत्यागम (word >> (shift & 31)) | (word << ((-shift) & 31));
}

स्थिर ध्रुव uint32_t blake2s_iv[8] = {
	0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
	0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

स्थिर ध्रुव uint8_t blake2s_sigma[10][16] = {
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

स्थिर शून्य blake2s_समुच्चयः_lastblock(संरचना blake2s_state *state)
{
	state->f[0] = -1;
}

स्थिर शून्य blake2s_increment_गणक(संरचना blake2s_state *state, ध्रुव uint32_t inc)
{
	state->t[0] += inc;
	state->t[1] += (state->t[0] < inc);
}

स्थिर शून्य blake2s_init_param(संरचना blake2s_state *state, ध्रुव uint32_t param)
{
	पूर्णाङ्क i;

	स्मृतिं_पूरय(state, 0, परिमाण(*state));
	प्रति (i = 0; i < 8; ++i)
		state->h[i] = blake2s_iv[i];
	state->h[0] ^= param;
}

स्थिर शून्य blake2s_init(संरचना blake2s_state *state, ध्रुव size_t outlen)
{
	blake2s_init_param(state, 0x01010000 | outlen);
	state->outlen = outlen;
}

स्थिर शून्य blake2s_compress(संरचना blake2s_state *state, ध्रुव uint8_t *खण्डम्, size_t nblocks, ध्रुव uint32_t inc)
{
	uint32_t m[16];
	uint32_t v[16];
	पूर्णाङ्क i;

	यावत् (nblocks > 0) {
		blake2s_increment_गणक(state, inc);
		स्मृतिं_प्रतिलिख(m, खण्डम्, BLAKE2_S_खण्डम्_LEN);
		le32_गन्तव्यम्_cpu_array(m, ARRAY_आकार(m));
		स्मृतिं_प्रतिलिख(v, state->h, 32);
		v[ 8] = blake2s_iv[0];
		v[ 9] = blake2s_iv[1];
		v[10] = blake2s_iv[2];
		v[11] = blake2s_iv[3];
		v[12] = blake2s_iv[4] ^ state->t[0];
		v[13] = blake2s_iv[5] ^ state->t[1];
		v[14] = blake2s_iv[6] ^ state->f[0];
		v[15] = blake2s_iv[7] ^ state->f[1];

#परिभाषय G(r, i, a, b, c, d) कुरु { \
	a += b + m[blake2s_sigma[r][2 * i + 0]]; \
	d = ror32(d ^ a, 16); \
	c += d; \
	b = ror32(b ^ c, 12); \
	a += b + m[blake2s_sigma[r][2 * i + 1]]; \
	d = ror32(d ^ a, 8); \
	c += d; \
	b = ror32(b ^ c, 7); \
} यावत् (0)

#परिभाषय आवर्तनम्(r) कुरु { \
	G(r, 0, v[0], v[ 4], v[ 8], v[12]); \
	G(r, 1, v[1], v[ 5], v[ 9], v[13]); \
	G(r, 2, v[2], v[ 6], v[10], v[14]); \
	G(r, 3, v[3], v[ 7], v[11], v[15]); \
	G(r, 4, v[0], v[ 5], v[10], v[15]); \
	G(r, 5, v[1], v[ 6], v[11], v[12]); \
	G(r, 6, v[2], v[ 7], v[ 8], v[13]); \
	G(r, 7, v[3], v[ 4], v[ 9], v[14]); \
} यावत् (0)
		आवर्तनम्(0);
		आवर्तनम्(1);
		आवर्तनम्(2);
		आवर्तनम्(3);
		आवर्तनम्(4);
		आवर्तनम्(5);
		आवर्तनम्(6);
		आवर्तनम्(7);
		आवर्तनम्(8);
		आवर्तनम्(9);

#अपरिभाषय G
#अपरिभाषय आवर्तनम्

		प्रति (i = 0; i < 8; ++i)
			state->h[i] ^= v[i] ^ v[i + 8];

		खण्डम् += BLAKE2_S_खण्डम्_LEN;
		--nblocks;
	}
}

स्थिर शून्य blake2s_update(संरचना blake2s_state *state, ध्रुव शून्य *inp, size_t inlen)
{
	ध्रुव size_t fill = BLAKE2_S_खण्डम्_LEN - state->buflen;
	ध्रुव uint8_t *अन्तर् = inp;

	यदि (!inlen)
		प्रत्यागम;
	यदि (inlen > fill) {
		स्मृतिं_प्रतिलिख(state->buf + state->buflen, अन्तर्, fill);
		blake2s_compress(state, state->buf, 1, BLAKE2_S_खण्डम्_LEN);
		state->buflen = 0;
		अन्तर् += fill;
		inlen -= fill;
	}
	यदि (inlen > BLAKE2_S_खण्डम्_LEN) {
		ध्रुव size_t nblocks = DIV_आवर्तनम्_ऊर्ध्वम्(inlen, BLAKE2_S_खण्डम्_LEN);
		blake2s_compress(state, अन्तर्, nblocks - 1, BLAKE2_S_खण्डम्_LEN);
		अन्तर् += BLAKE2_S_खण्डम्_LEN * (nblocks - 1);
		inlen -= BLAKE2_S_खण्डम्_LEN * (nblocks - 1);
	}
	स्मृतिं_प्रतिलिख(state->buf + state->buflen, अन्तर्, inlen);
	state->buflen += inlen;
}

स्थिर शून्य blake2s_final(संरचना blake2s_state *state, uint8_t *बहिर्गमनम्)
{
	blake2s_समुच्चयः_lastblock(state);
	स्मृतिं_पूरय(state->buf + state->buflen, 0, BLAKE2_S_खण्डम्_LEN - state->buflen);
	blake2s_compress(state, state->buf, 1, state->buflen);
	cpu_गन्तव्यम्_le32_array(state->h, ARRAY_आकार(state->h));
	स्मृतिं_प्रतिलिख(बहिर्गमनम्, state->h, state->outlen);
}

स्थिर ssize_t getrandom_पूर्णम्(शून्य *buf, size_t गणनम्, अचिह्नित पूर्णाङ्क flags)
{
	ssize_t ret, योग = 0;
	uint8_t *p = buf;

	कुरु {
		ret = getrandom(p, गणनम्, flags);
		यदि (ret < 0 && errno == EINTR)
			अनुवर्त;
		अन्यथा यदि (ret < 0)
			प्रत्यागम ret;
		योग += ret;
		p += ret;
		गणनम् -= ret;
	} यावत् (गणनम्);
	प्रत्यागम योग;
}

स्थिर ssize_t read_पूर्णम्(पूर्णाङ्क fd, शून्य *buf, size_t गणनम्)
{
	ssize_t ret, योग = 0;
	uint8_t *p = buf;

	कुरु {
		ret = पठ(fd, p, गणनम्);
		यदि (ret < 0 && errno == EINTR)
			अनुवर्त;
		अन्यथा यदि (ret < 0)
			प्रत्यागम ret;
		अन्यथा यदि (ret == 0)
			भङ्ग;
		योग += ret;
		p += ret;
		गणनम् -= ret;
	} यावत् (गणनम्);
	प्रत्यागम योग;
}

स्थिर ssize_t write_पूर्णम्(पूर्णाङ्क fd, ध्रुव शून्य *buf, size_t गणनम्)
{
	ssize_t ret, योग = 0;
	ध्रुव uint8_t *p = buf;

	कुरु {
		ret = लिख(fd, p, गणनम्);
		यदि (ret < 0 && errno == EINTR)
			अनुवर्त;
		अन्यथा यदि (ret < 0)
			प्रत्यागम ret;
		योग += ret;
		p += ret;
		गणनम् -= ret;
	} यावत् (गणनम्);
	प्रत्यागम योग;
}

स्थिर size_t determine_optimal_बीजम्_len(शून्य)
{
	size_t ret = 0;
	वर्ण poolsize_str[11] = { 0 };
	पूर्णाङ्क fd = उद्घाटय("/proc/sys/kernel/random/poolsize", O_RDONLY);

	यदि (fd < 0 || read_पूर्णम्(fd, poolsize_str, परिमाण(poolsize_str) - 1) < 0) {
		ewarn("%s: Unable to determine pool size, falling back to %u bits: %s",
		      applet, MIN_बीजम्_LEN * 8, strerror(errno));
		ret = MIN_बीजम्_LEN;
	} अन्यथा
		ret = DIV_आवर्तनम्_ऊर्ध्वम्(strtoul(poolsize_str, रिक्त, 10), 8);
	यदि (fd >= 0)
		close(fd);
	यदि (ret < MIN_बीजम्_LEN)
		ret = MIN_बीजम्_LEN;
	अन्यथा यदि (ret > MAX_बीजम्_LEN)
		ret = MAX_बीजम्_LEN;
	प्रत्यागम ret;
}

स्थिर पूर्णाङ्क read_new_बीजम्(uint8_t *बीजम्, size_t len, bool *is_creditable)
{
	ssize_t ret;
	पूर्णाङ्क urandom_fd;

	*is_creditable = false;
	ret = getrandom_पूर्णम्(बीजम्, len, GRND_NONBLOCK);
	यदि (ret == (ssize_t)len) {
		*is_creditable = true;
		प्रत्यागम 0;
	} अन्यथा यदि (ret < 0 && errno == ENOSYS) {
		संरचना pollfd random_fd = {
			.fd = उद्घाटय("/dev/random", O_RDONLY),
			.events = POLLIN
		};
		यदि (random_fd.fd < 0)
			प्रत्यागम -errno;
		*is_creditable = poll(&random_fd, 1, 0) == 1;
		close(random_fd.fd);
	} अन्यथा यदि (getrandom_पूर्णम्(बीजम्, len, GRND_INSECURE) == (ssize_t)len)
		प्रत्यागम 0;
	urandom_fd = उद्घाटय("/dev/urandom", O_RDONLY);
	यदि (urandom_fd < 0)
		प्रत्यागम -1;
	ret = read_पूर्णम्(urandom_fd, बीजम्, len);
	यदि (ret == (ssize_t)len)
		ret = 0;
	अन्यथा
		ret = -errno ? -errno : -EIO;
	close(urandom_fd);
	errno = -ret;
	प्रत्यागम ret ? -1 : 0;
}

स्थिर पूर्णाङ्क बीजम्_rng(uint8_t *बीजम्, size_t len, bool credit)
{
	संरचना {
		पूर्णाङ्क entropy_गणनम्;
		पूर्णाङ्क buf_आकार;
		uint8_t बफर[MAX_बीजम्_LEN];
	} req = {
		.entropy_गणनम् = credit ? len * 8 : 0,
		.buf_आकार = len
	};
	पूर्णाङ्क random_fd, ret;

	यदि (len > परिमाण(req.बफर)) {
		errno = EFBIG;
		प्रत्यागम -1;
	}
	स्मृतिं_प्रतिलिख(req.बफर, बीजम्, len);

	random_fd = उद्घाटय("/dev/urandom", O_RDONLY);
	यदि (random_fd < 0)
		प्रत्यागम -1;
	ret = ioctl(random_fd, RNDADDENTROPY, &req);
	यदि (ret)
		ret = -errno ? -errno : -EIO;
	close(random_fd);
	errno = -ret;
	प्रत्यागम ret ? -1 : 0;
}

स्थिर पूर्णाङ्क बीजम्_from_file_if_exists(ध्रुव वर्ण *filename, पूर्णाङ्क dfd, bool credit, संरचना blake2s_state *hash)
{
	uint8_t बीजम्[MAX_बीजम्_LEN];
	ssize_t बीजम्_len;
	पूर्णाङ्क fd = -1, ret = 0;

	fd = openat(dfd, filename, O_RDONLY);
	यदि (fd < 0 && errno == ENOENT)
		प्रत्यागम 0;
	अन्यथा यदि (fd < 0) {
		ret = -errno;
		eerror("%s: Unable to open seed file: %s", applet, strerror(errno));
		गच्छ बहिर्गमनम्;
	}
	बीजम्_len = read_पूर्णम्(fd, बीजम्, परिमाण(बीजम्));
	यदि (बीजम्_len < 0) {
		ret = -errno;
		eerror("%s: Unable to read seed file: %s", applet, strerror(errno));
		गच्छ बहिर्गमनम्;
	}
	यदि ((unlinkat(dfd, filename, 0) < 0 || fsync(dfd) < 0) && बीजम्_len) {
		ret = -errno;
		eerror("%s: Unable to remove seed after reading, so not seeding: %s",
		       applet, strerror(errno));
		गच्छ बहिर्गमनम्;
	}
	यदि (!बीजम्_len)
		गच्छ बहिर्गमनम्;

	blake2s_update(hash, &बीजम्_len, परिमाण(बीजम्_len));
	blake2s_update(hash, बीजम्, बीजम्_len);

	einfo("Seeding %zd bits %s crediting", बीजम्_len * 8, credit ? "and" : "without");
	यदि (बीजम्_rng(बीजम्, बीजम्_len, credit) < 0) {
		ret = -errno;
		eerror("%s: Unable to seed: %s", applet, strerror(errno));
	}

बहिर्गमनम्:
	यदि (fd >= 0)
		close(fd);
	errno = -ret;
	प्रत्यागम ret ? -1 : 0;
}

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv)
{
	स्थिर ध्रुव वर्ण seedrng_उपसर्गः[] = "SeedRNG v1 Old+New Prefix";
	स्थिर ध्रुव वर्ण seedrng_failure[] = "SeedRNG v1 No New Seed Failure";
	पूर्णाङ्क opt, fd, dfd, क्रमादेशः_ret = 0;
	ध्रुव वर्ण *बीजम्_dir = रिक्त;
	uint8_t new_बीजम्[MAX_बीजम्_LEN];
	size_t new_बीजम्_len;
	bool new_बीजम्_creditable;
	संरचना timespec realtime = { 0 }, boottime = { 0 };
	संरचना blake2s_state hash;
	bool skip_credit = false;

	applet = basename_c(argv[0]);

	यावत् ((opt = getopt_long(argc, argv, getoptstring, longopts, (पूर्णाङ्क *) 0)) != -1)
	{
		विकल्प (opt) {
		पक्ष LONGOPT_बीजम्_DIR:
			यदि (!बीजम्_dir)
				बीजम्_dir = optarg;
			भङ्ग;
		पक्ष LONGOPT_SKIP_CREDIT:
			skip_credit = true;
			भङ्ग;
		case_RC_COMMON_GETOPT
		}
	}
	यदि (!बीजम्_dir)
		बीजम्_dir = DEFAULT_बीजम्_DIR;
	यदि (getuid())
		eerrorx("%s: superuser access is required", applet);
	umask(0077);

	blake2s_init(&hash, BLAKE2S_HASH_LEN);
	blake2s_update(&hash, seedrng_उपसर्गः, वाक्यदैर्घ्यम्(seedrng_उपसर्गः));
	clock_gettime(CLOCK_REALTIME, &realtime);
	clock_gettime(CLOCK_BOOTTIME, &boottime);
	blake2s_update(&hash, &realtime, परिमाण(realtime));
	blake2s_update(&hash, &boottime, परिमाण(boottime));

	यदि (mkdir(बीजम्_dir, 0700) < 0 && errno != EEXIST)
		eerrorx("%s: Unable to create seed directory: %s", applet, strerror(errno));

	dfd = उद्घाटय(बीजम्_dir, O_DIRECTORY | O_RDONLY);
	यदि (dfd < 0 || flock(dfd, LOCK_EX) < 0)
		eerrorx("%s: Unable to lock seed directory: %s", applet, strerror(errno));

	यदि (बीजम्_from_file_if_exists(NON_CREDITABLE_बीजम्, dfd, false, &hash) < 0)
		क्रमादेशः_ret |= 1 << 1;
	यदि (बीजम्_from_file_if_exists(CREDITABLE_बीजम्, dfd, !skip_credit, &hash) < 0)
		क्रमादेशः_ret |= 1 << 2;

	new_बीजम्_len = determine_optimal_बीजम्_len();
	यदि (read_new_बीजम्(new_बीजम्, new_बीजम्_len, &new_बीजम्_creditable) < 0) {
		eerror("%s: Unable to read new seed: %s", applet, strerror(errno));
		new_बीजम्_len = BLAKE2S_HASH_LEN;
		strncpy((वर्ण *)new_बीजम्, seedrng_failure, new_बीजम्_len);
		क्रमादेशः_ret |= 1 << 3;
	}
	blake2s_update(&hash, &new_बीजम्_len, परिमाण(new_बीजम्_len));
	blake2s_update(&hash, new_बीजम्, new_बीजम्_len);
	blake2s_final(&hash, new_बीजम् + new_बीजम्_len - BLAKE2S_HASH_LEN);

	einfo("Saving %zu bits of %s seed for next boot", new_बीजम्_len * 8, new_बीजम्_creditable ? "creditable" : "non-creditable");
	fd = openat(dfd, NON_CREDITABLE_बीजम्, O_WRONLY | O_CREAT | O_TRUNC, 0400);
	यदि (fd < 0) {
		eerror("%s: Unable to open seed file for writing: %s", applet, strerror(errno));
		प्रत्यागम क्रमादेशः_ret | (1 << 4);
	}
	यदि (write_पूर्णम्(fd, new_बीजम्, new_बीजम्_len) != (ssize_t)new_बीजम्_len || fsync(fd) < 0) {
		eerror("%s: Unable to write seed file: %s", applet, strerror(errno));
		प्रत्यागम क्रमादेशः_ret | (1 << 5);
	}
	यदि (new_बीजम्_creditable && renameat(dfd, NON_CREDITABLE_बीजम्, dfd, CREDITABLE_बीजम्) < 0) {
		ewarn("%s: Unable to make new seed creditable: %s", applet, strerror(errno));
		प्रत्यागम क्रमादेशः_ret | (1 << 6);
	}
	प्रत्यागम क्रमादेशः_ret;
}
