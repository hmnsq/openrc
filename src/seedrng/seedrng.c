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

#समावेश <linux/random.h>
#समावेश <sys/random.h>
#समावेश <sys/ioctl.h>
#समावेश <sys/file.h>
#समावेश <sys/stat.h>
#समावेश <sys/types.h>
#समावेश <fcntl.h>
#समावेश <getopt.h>
#समावेश <poll.h>
#समावेश <unistd.h>
#समावेश <time.h>
#समावेश <errno.h>
#समावेश <endian.h>
#समावेश <stdbool.h>
#समावेश <stddef.h>
#समावेश <stdint.h>
#समावेश <string.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>

#समावेश "einfo.h"
#समावेश "helpers.h"
#समावेश "_usage.h"

#अगर_अघोषित GRND_INSECURE
#घोषणा GRND_INSECURE 0x0004 /* Apparently some headers don't ship with this yet. */
#पूर्ण_अगर

#घोषणा DEFAULT_SEED_DIR "/var/lib/seedrng"
#घोषणा CREDITABLE_SEED "seed.credit"
#घोषणा NON_CREDITABLE_SEED "seed.no-credit"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
क्रमागत long_opts {
	/* This has to come first so following values stay in the 0x100+ range. */
	LONGOPT_BASE = 0x100,
	LONGOPT_SEED_DIR,
	LONGOPT_SKIP_CREDIT,
};

स्थिर अक्षर *applet = शून्य;
स्थिर अक्षर *extraopts = शून्य;
स्थिर अक्षर getoptstring[] = getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "seed-dir", 1, शून्य, LONGOPT_SEED_DIR },
	{ "skip-credit", 0, शून्य, LONGOPT_SKIP_CREDIT },
	longopts_COMMON
};
स्थिर अक्षर * स्थिर longopts_help[] = {
	"Directory for seed files (default: " DEFAULT_SEED_DIR ")",
	"Skip crediting entropy of seeds",
	longopts_help_COMMON
};
स्थिर अक्षर *usagestring = शून्य;

क्रमागत blake2s_lengths {
	BLAKE2S_BLOCK_LEN = 64,
	BLAKE2S_HASH_LEN = 32,
	BLAKE2_S_कुंजी_LEN = 32
};

क्रमागत seedrng_lengths {
	MAX_SEED_LEN = 512,
	MIN_SEED_LEN = BLAKE2S_HASH_LEN
};

काष्ठा blake2s_state {
	uint32_t h[8];
	uint32_t t[2];
	uint32_t f[2];
	uint8_t buf[BLAKE2S_BLOCK_LEN];
	अचिन्हित पूर्णांक buflen;
	अचिन्हित पूर्णांक outlen;
};

#घोषणा le32_to_cpup(a) le32toh(*(a))
#घोषणा cpu_to_le32(a) htole32(a)
#अगर_अघोषित ARRAY_आकार
#घोषणा ARRAY_आकार(x) (माप(x) / माप((x)[0]))
#पूर्ण_अगर
#अगर_अघोषित DIV_ROUND_UP
#घोषणा DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#पूर्ण_अगर

अटल अंतरभूत व्योम cpu_to_le32_array(uint32_t *buf, अचिन्हित पूर्णांक शब्दगण)
{
	जबतक (शब्दगण--) {
		*buf = cpu_to_le32(*buf);
		++buf;
	}
}

अटल अंतरभूत व्योम le32_to_cpu_array(uint32_t *buf, अचिन्हित पूर्णांक शब्दगण)
{
	जबतक (शब्दगण--) {
		*buf = le32_to_cpup(buf);
		++buf;
	}
}

अटल अंतरभूत uint32_t ror32(uint32_t word, अचिन्हित पूर्णांक shift)
{
	वापस (word >> (shift & 31)) | (word << ((-shift) & 31));
}

अटल स्थिर uint32_t blake2s_iv[8] = {
	0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
	0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

अटल स्थिर uint8_t blake2s_sigma[10][16] = {
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

अटल व्योम blake2s_set_lastblock(काष्ठा blake2s_state *state)
{
	state->f[0] = -1;
}

अटल व्योम blake2s_increment_गणक(काष्ठा blake2s_state *state, स्थिर uint32_t inc)
{
	state->t[0] += inc;
	state->t[1] += (state->t[0] < inc);
}

अटल व्योम blake2s_init_param(काष्ठा blake2s_state *state, स्थिर uint32_t param)
{
	पूर्णांक i;

	स_रखो(state, 0, माप(*state));
	क्रम (i = 0; i < 8; ++i)
		state->h[i] = blake2s_iv[i];
	state->h[0] ^= param;
}

अटल व्योम blake2s_init(काष्ठा blake2s_state *state, स्थिर माप_प्रकार outlen)
{
	blake2s_init_param(state, 0x01010000 | outlen);
	state->outlen = outlen;
}

अटल व्योम blake2s_compress(काष्ठा blake2s_state *state, स्थिर uint8_t *block, माप_प्रकार nblocks, स्थिर uint32_t inc)
{
	uint32_t m[16];
	uint32_t v[16];
	पूर्णांक i;

	जबतक (nblocks > 0) {
		blake2s_increment_गणक(state, inc);
		स_नकल(m, block, BLAKE2S_BLOCK_LEN);
		le32_to_cpu_array(m, ARRAY_आकार(m));
		स_नकल(v, state->h, 32);
		v[ 8] = blake2s_iv[0];
		v[ 9] = blake2s_iv[1];
		v[10] = blake2s_iv[2];
		v[11] = blake2s_iv[3];
		v[12] = blake2s_iv[4] ^ state->t[0];
		v[13] = blake2s_iv[5] ^ state->t[1];
		v[14] = blake2s_iv[6] ^ state->f[0];
		v[15] = blake2s_iv[7] ^ state->f[1];

#घोषणा G(r, i, a, b, c, d) करो { \
	a += b + m[blake2s_sigma[r][2 * i + 0]]; \
	d = ror32(d ^ a, 16); \
	c += d; \
	b = ror32(b ^ c, 12); \
	a += b + m[blake2s_sigma[r][2 * i + 1]]; \
	d = ror32(d ^ a, 8); \
	c += d; \
	b = ror32(b ^ c, 7); \
} जबतक (0)

#घोषणा ROUND(r) करो { \
	G(r, 0, v[0], v[ 4], v[ 8], v[12]); \
	G(r, 1, v[1], v[ 5], v[ 9], v[13]); \
	G(r, 2, v[2], v[ 6], v[10], v[14]); \
	G(r, 3, v[3], v[ 7], v[11], v[15]); \
	G(r, 4, v[0], v[ 5], v[10], v[15]); \
	G(r, 5, v[1], v[ 6], v[11], v[12]); \
	G(r, 6, v[2], v[ 7], v[ 8], v[13]); \
	G(r, 7, v[3], v[ 4], v[ 9], v[14]); \
} जबतक (0)
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

#अघोषित G
#अघोषित ROUND

		क्रम (i = 0; i < 8; ++i)
			state->h[i] ^= v[i] ^ v[i + 8];

		block += BLAKE2S_BLOCK_LEN;
		--nblocks;
	}
}

अटल व्योम blake2s_update(काष्ठा blake2s_state *state, स्थिर व्योम *inp, माप_प्रकार inlen)
{
	स्थिर माप_प्रकार fill = BLAKE2S_BLOCK_LEN - state->buflen;
	स्थिर uint8_t *in = inp;

	अगर (!inlen)
		वापस;
	अगर (inlen > fill) {
		स_नकल(state->buf + state->buflen, in, fill);
		blake2s_compress(state, state->buf, 1, BLAKE2S_BLOCK_LEN);
		state->buflen = 0;
		in += fill;
		inlen -= fill;
	}
	अगर (inlen > BLAKE2S_BLOCK_LEN) {
		स्थिर माप_प्रकार nblocks = DIV_ROUND_UP(inlen, BLAKE2S_BLOCK_LEN);
		blake2s_compress(state, in, nblocks - 1, BLAKE2S_BLOCK_LEN);
		in += BLAKE2S_BLOCK_LEN * (nblocks - 1);
		inlen -= BLAKE2S_BLOCK_LEN * (nblocks - 1);
	}
	स_नकल(state->buf + state->buflen, in, inlen);
	state->buflen += inlen;
}

अटल व्योम blake2s_final(काष्ठा blake2s_state *state, uint8_t *out)
{
	blake2s_set_lastblock(state);
	स_रखो(state->buf + state->buflen, 0, BLAKE2S_BLOCK_LEN - state->buflen);
	blake2s_compress(state, state->buf, 1, state->buflen);
	cpu_to_le32_array(state->h, ARRAY_आकार(state->h));
	स_नकल(out, state->h, state->outlen);
}

अटल ssize_t getrandom_full(व्योम *buf, माप_प्रकार गिनती, अचिन्हित पूर्णांक flags)
{
	ssize_t ret, योग = 0;
	uint8_t *p = buf;

	करो {
		ret = getrandom(p, गिनती, flags);
		अगर (ret < 0 && त्रुटि_सं == EINTR)
			जारी;
		अन्यथा अगर (ret < 0)
			वापस ret;
		योग += ret;
		p += ret;
		गिनती -= ret;
	} जबतक (गिनती);
	वापस योग;
}

अटल ssize_t read_full(पूर्णांक fd, व्योम *buf, माप_प्रकार गिनती)
{
	ssize_t ret, योग = 0;
	uint8_t *p = buf;

	करो {
		ret = पढ़ो(fd, p, गिनती);
		अगर (ret < 0 && त्रुटि_सं == EINTR)
			जारी;
		अन्यथा अगर (ret < 0)
			वापस ret;
		अन्यथा अगर (ret == 0)
			अवरोध;
		योग += ret;
		p += ret;
		गिनती -= ret;
	} जबतक (गिनती);
	वापस योग;
}

अटल ssize_t write_full(पूर्णांक fd, स्थिर व्योम *buf, माप_प्रकार गिनती)
{
	ssize_t ret, योग = 0;
	स्थिर uint8_t *p = buf;

	करो {
		ret = ग_लिखो(fd, p, गिनती);
		अगर (ret < 0 && त्रुटि_सं == EINTR)
			जारी;
		अन्यथा अगर (ret < 0)
			वापस ret;
		योग += ret;
		p += ret;
		गिनती -= ret;
	} जबतक (गिनती);
	वापस योग;
}

अटल माप_प्रकार determine_optimal_seed_len(व्योम)
{
	माप_प्रकार ret = 0;
	अक्षर poolsize_str[11] = { 0 };
	पूर्णांक fd = खोलो("/proc/sys/kernel/random/poolsize", O_RDONLY);

	अगर (fd < 0 || read_full(fd, poolsize_str, माप(poolsize_str) - 1) < 0) {
		ewarn("%s: Unable to determine pool size, falling back to %u bits: %s",
		      applet, MIN_SEED_LEN * 8, म_त्रुटि(त्रुटि_सं));
		ret = MIN_SEED_LEN;
	} अन्यथा
		ret = DIV_ROUND_UP(म_से_अदीर्घ(poolsize_str, शून्य, 10), 8);
	अगर (fd >= 0)
		बंद(fd);
	अगर (ret < MIN_SEED_LEN)
		ret = MIN_SEED_LEN;
	अन्यथा अगर (ret > MAX_SEED_LEN)
		ret = MAX_SEED_LEN;
	वापस ret;
}

अटल पूर्णांक read_new_seed(uint8_t *seed, माप_प्रकार len, bool *is_creditable)
{
	ssize_t ret;
	पूर्णांक urandom_fd;

	*is_creditable = false;
	ret = getrandom_full(seed, len, GRND_NONBLOCK);
	अगर (ret == (ssize_t)len) {
		*is_creditable = true;
		वापस 0;
	} अन्यथा अगर (ret < 0 && त्रुटि_सं == ENOSYS) {
		काष्ठा pollfd random_fd = {
			.fd = खोलो("/dev/random", O_RDONLY),
			.events = POLLIN
		};
		अगर (random_fd.fd < 0)
			वापस -त्रुटि_सं;
		*is_creditable = poll(&random_fd, 1, 0) == 1;
		बंद(random_fd.fd);
	} अन्यथा अगर (getrandom_full(seed, len, GRND_INSECURE) == (ssize_t)len)
		वापस 0;
	urandom_fd = खोलो("/dev/urandom", O_RDONLY);
	अगर (urandom_fd < 0)
		वापस -1;
	ret = read_full(urandom_fd, seed, len);
	अगर (ret == (ssize_t)len)
		ret = 0;
	अन्यथा
		ret = -त्रुटि_सं ? -त्रुटि_सं : -EIO;
	बंद(urandom_fd);
	त्रुटि_सं = -ret;
	वापस ret ? -1 : 0;
}

अटल पूर्णांक seed_rng(uint8_t *seed, माप_प्रकार len, bool credit)
{
	काष्ठा {
		पूर्णांक entropy_गिनती;
		पूर्णांक buf_आकार;
		uint8_t बफर[MAX_SEED_LEN];
	} req = {
		.entropy_गिनती = credit ? len * 8 : 0,
		.buf_आकार = len
	};
	पूर्णांक random_fd, ret;

	अगर (len > माप(req.बफर)) {
		त्रुटि_सं = EFBIG;
		वापस -1;
	}
	स_नकल(req.बफर, seed, len);

	random_fd = खोलो("/dev/urandom", O_RDONLY);
	अगर (random_fd < 0)
		वापस -1;
	ret = ioctl(random_fd, RNDADDENTROPY, &req);
	अगर (ret)
		ret = -त्रुटि_सं ? -त्रुटि_सं : -EIO;
	बंद(random_fd);
	त्रुटि_सं = -ret;
	वापस ret ? -1 : 0;
}

अटल पूर्णांक seed_from_file_if_exists(स्थिर अक्षर *filename, पूर्णांक dfd, bool credit, काष्ठा blake2s_state *hash)
{
	uint8_t seed[MAX_SEED_LEN];
	ssize_t seed_len;
	पूर्णांक fd = -1, ret = 0;

	fd = openat(dfd, filename, O_RDONLY);
	अगर (fd < 0 && त्रुटि_सं == ENOENT)
		वापस 0;
	अन्यथा अगर (fd < 0) {
		ret = -त्रुटि_सं;
		eerror("%s: Unable to open seed file: %s", applet, म_त्रुटि(त्रुटि_सं));
		जाओ out;
	}
	seed_len = read_full(fd, seed, माप(seed));
	अगर (seed_len < 0) {
		ret = -त्रुटि_सं;
		eerror("%s: Unable to read seed file: %s", applet, म_त्रुटि(त्रुटि_सं));
		जाओ out;
	}
	अगर ((unlinkat(dfd, filename, 0) < 0 || fsync(dfd) < 0) && seed_len) {
		ret = -त्रुटि_सं;
		eerror("%s: Unable to remove seed after reading, so not seeding: %s",
		       applet, म_त्रुटि(त्रुटि_सं));
		जाओ out;
	}
	अगर (!seed_len)
		जाओ out;

	blake2s_update(hash, &seed_len, माप(seed_len));
	blake2s_update(hash, seed, seed_len);

	einfo("Seeding %zd bits %s crediting", seed_len * 8, credit ? "and" : "without");
	अगर (seed_rng(seed, seed_len, credit) < 0) {
		ret = -त्रुटि_सं;
		eerror("%s: Unable to seed: %s", applet, म_त्रुटि(त्रुटि_सं));
	}

out:
	अगर (fd >= 0)
		बंद(fd);
	त्रुटि_सं = -ret;
	वापस ret ? -1 : 0;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	अटल स्थिर अक्षर seedrng_prefix[] = "SeedRNG v1 Old+New Prefix";
	अटल स्थिर अक्षर seedrng_failure[] = "SeedRNG v1 No New Seed Failure";
	पूर्णांक opt, fd, dfd, program_ret = 0;
	स्थिर अक्षर *seed_dir = शून्य;
	uint8_t new_seed[MAX_SEED_LEN];
	माप_प्रकार new_seed_len;
	bool new_seed_creditable;
	काष्ठा timespec realtime = { 0 }, boottime = { 0 };
	काष्ठा blake2s_state hash;
	bool skip_credit = false;

	applet = basename_c(argv[0]);

	जबतक ((opt = getopt_long(argc, argv, getoptstring, longopts, (पूर्णांक *) 0)) != -1)
	{
		चयन (opt) {
		हाल LONGOPT_SEED_DIR:
			अगर (!seed_dir)
				seed_dir = optarg;
			अवरोध;
		हाल LONGOPT_SKIP_CREDIT:
			skip_credit = true;
			अवरोध;
		case_RC_COMMON_GETOPT
		}
	}
	अगर (!seed_dir)
		seed_dir = DEFAULT_SEED_DIR;
	अगर (getuid())
		eerrorx("%s: superuser access is required", applet);
	umask(0077);

	blake2s_init(&hash, BLAKE2S_HASH_LEN);
	blake2s_update(&hash, seedrng_prefix, म_माप(seedrng_prefix));
	clock_gettime(CLOCK_REALTIME, &realtime);
	clock_gettime(CLOCK_BOOTTIME, &boottime);
	blake2s_update(&hash, &realtime, माप(realtime));
	blake2s_update(&hash, &boottime, माप(boottime));

	अगर (सूची_गढ़ो(seed_dir, 0700) < 0 && त्रुटि_सं != EEXIST)
		eerrorx("%s: Unable to create seed directory: %s", applet, म_त्रुटि(त्रुटि_सं));

	dfd = खोलो(seed_dir, O_DIRECTORY | O_RDONLY);
	अगर (dfd < 0 || flock(dfd, LOCK_EX) < 0)
		eerrorx("%s: Unable to lock seed directory: %s", applet, म_त्रुटि(त्रुटि_सं));

	अगर (seed_from_file_if_exists(NON_CREDITABLE_SEED, dfd, false, &hash) < 0)
		program_ret |= 1 << 1;
	अगर (seed_from_file_if_exists(CREDITABLE_SEED, dfd, !skip_credit, &hash) < 0)
		program_ret |= 1 << 2;

	new_seed_len = determine_optimal_seed_len();
	अगर (read_new_seed(new_seed, new_seed_len, &new_seed_creditable) < 0) {
		eerror("%s: Unable to read new seed: %s", applet, म_त्रुटि(त्रुटि_सं));
		new_seed_len = BLAKE2S_HASH_LEN;
		म_नकलन((अक्षर *)new_seed, seedrng_failure, new_seed_len);
		program_ret |= 1 << 3;
	}
	blake2s_update(&hash, &new_seed_len, माप(new_seed_len));
	blake2s_update(&hash, new_seed, new_seed_len);
	blake2s_final(&hash, new_seed + new_seed_len - BLAKE2S_HASH_LEN);

	einfo("Saving %zu bits of %s seed for next boot", new_seed_len * 8, new_seed_creditable ? "creditable" : "non-creditable");
	fd = openat(dfd, NON_CREDITABLE_SEED, O_WRONLY | O_CREAT | O_TRUNC, 0400);
	अगर (fd < 0) {
		eerror("%s: Unable to open seed file for writing: %s", applet, म_त्रुटि(त्रुटि_सं));
		वापस program_ret | (1 << 4);
	}
	अगर (write_full(fd, new_seed, new_seed_len) != (ssize_t)new_seed_len || fsync(fd) < 0) {
		eerror("%s: Unable to write seed file: %s", applet, म_त्रुटि(त्रुटि_सं));
		वापस program_ret | (1 << 5);
	}
	अगर (new_seed_creditable && renameat(dfd, NON_CREDITABLE_SEED, dfd, CREDITABLE_SEED) < 0) {
		ewarn("%s: Unable to make new seed creditable: %s", applet, म_त्रुटि(त्रुटि_सं));
		वापस program_ret | (1 << 6);
	}
	वापस program_ret;
}
