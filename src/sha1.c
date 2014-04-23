/* ed:set tabstop=8 noexpandtab: */
/*****************************************************************************
 *
 * sha1.c	SHA1 implementation
 *
 *	Test vectors from FIPS 180-1 are:
 *	"abc", 3
 *	A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
 *	"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56
 *	84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
 * 	A million repetitions of "a"
 *	34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
 *****************************************************************************/
#include "sha1.h"

int xdigit(char ch)
{
	switch (ch) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return ch - '0';
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return ch - 'a' + 10;
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return ch - 'A' + 10;
	}
	return -1;
}

/*
 * blk0() and blk() perform the initial expand.
 * I got the idea of expanding during the round function from SSLeay
 */
#if (SDL_BYTEORDER==SDL_LIL_ENDIAN)
# define blk0(i) \
	(block->l[i] = \
		(rol(block->l[i],24) & 0xff00ff00) | \
		(rol(block->l[i], 8) & 0x00ff00ff))
#else
# define blk0(i) \
		block->l[i]
#endif

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

#define blk(i) \
	(block->l[i&15] = \
		rol( \
			block->l[(i+13)&15] ^ \
			block->l[(i+8)&15] ^ \
			block->l[(i+2)&15] ^ \
			block->l[i&15], \
		1) \
	)

/*
 * (R0+R1), R2, R3, R4 are the different operations (rounds) used in SHA1
 */
#define R0(v,w,x,y,z,i) \
	z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5a827999 + rol(v,5); \
	w = rol(w,30);

#define R1(v,w,x,y,z,i) \
	z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5a827999  +rol(v,5); \
	w = rol(w,30);

#define R2(v,w,x,y,z,i) \
	z += (w ^ x ^ y) + blk(i) + 0x6ed9eba1 + rol(v,5); \
	w = rol(w,30);

#define R3(v,w,x,y,z,i) \
	z += (((w | x) & y) | (w & x)) + blk(i) + 0x8f1bbcdc + rol(v,5); \
	w = rol(w,30);

#define R4(v,w,x,y,z,i) \
	z += (w ^ x ^ y) + blk(i) + 0xca62c1d6 + rol(v,5); \
	w = rol(w,30);

typedef union {
	uint8_t c[64];
	uint32_t l[16];
}	CHAR64LONG16;

/* Hash a single 512-bit block. This is the core of the algorithm.  */
void sha1_process(sha1_state_t *pss, const uint8_t data[64])
{
	uint32_t a = pss->state[0];
	uint32_t b = pss->state[1];
	uint32_t c = pss->state[2];
	uint32_t d = pss->state[3];
	uint32_t e = pss->state[4];
	CHAR64LONG16 workspace, *block = &workspace;

	(void)memcpy(block, data, 64);

	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

	/* Add the working vars back into context.state[] */
	pss->state[0] += a;
	pss->state[1] += b;
	pss->state[2] += c;
	pss->state[3] += d;
	pss->state[4] += e;
}


/**
 * @brief Initialize new context
 *
 * @param pss pointer to a SHA1 state to initialize
 */
void sha1_init(sha1_state_t *pss)
{
	/* SHA1 initialization constants */
	pss->state[0] = 0x67452301;
	pss->state[1] = 0xefcdab89;
	pss->state[2] = 0x98badcfe;
	pss->state[3] = 0x10325476;
	pss->state[4] = 0xc3d2e1f0;

	pss->count[0] = 0;
	pss->count[1] = 0;
}

/**
 * @brief Append data to a SHA1 context
 *
 * @param pss pointer to a SHA1 state to append to
 * @param data pointer to data to append
 * @param nbytes number of bytes of data
 */
void sha1_append(sha1_state_t *pss, const void *data, size_t nbytes)
{
	size_t nbits = nbytes * 8;
	const uint8_t *p = (const uint8_t *)data;
	size_t i, offs;

	offs = pss->count[0];
	if ((pss->count[0] += nbits) < offs)
		pss->count[1] += (nbytes >> 29) + 1;
	offs = (offs / 8) & 63;
	if ((offs + nbytes) > 63) {
		i = 64 - offs;
		memcpy(&pss->accu[offs], p, i);
		sha1_process(pss, pss->accu);
		for (/* */; i + 63 < nbytes; i += 64)
			sha1_process(pss, &p[i]);
		offs = 0;
	} else {
		i = 0;
	}
	memcpy(&pss->accu[offs], &p[i], nbytes - i);
}

/**
 * @brief Add padding and return the message digest.
 *
 * @param pss pointer to a SHA1 state to finish
 * @param pd pointer to a SHA1 digest to receive the result
 */
void sha1_finish(sha1_state_t *pss, sha1_digest_t *pd)
{
	uint8_t finalcount[8];
	size_t i;

	for (i = 0; i < 8; i++)
		finalcount[i] = (uint8_t)(pss->count[(i/4)^1] >> (((i^3)&3)*8));

	sha1_append(pss, (const void *)"\200", 1);
	while ((pss->count[0] & 504) != 448)
		sha1_append(pss, (const void *)"\0", 1);

	sha1_append(pss, finalcount, 8);  /* Should cause a SHA1Transform() */

	if (NULL != pd) {
		for (i = 0; i < SHA1SIZE; i++)
			pd->b[i] = (uint8_t)(pss->state[i/4] >> (((i^3)&3)*8));
	}
}

/**
 * @brief Return a hex string representation for a SHA1 digest
 *
 * @param pd pointer to a SHA1 digest to convert
 * @result returns a pointer to a string buffer
 */
const char *sha1_hexstr(const sha1_digest_t *pd)
{
	static char hexstr[2*sizeof(pd->b)+1];
	char *dst;
	size_t i;

	dst = hexstr;
	for (i = 0; i < sizeof(pd->b); i++)
		dst += sprintf(dst, "%02x", pd->b[i]);
	return hexstr;
}


/**
 * @brief Convert a hex string representation into a SHA1 digest
 *
 * @param pd pointer to a SHA1 digest to convert
 * @param src pointer to a string of 40 hex digits
 * @result returns 0 on success, -1 on error
 */
int sha1_strhex(sha1_digest_t *pd, const char *src)
{
	size_t i;

	for (i = 0; i < sizeof(pd->b); i++) {
		if (*src == '\0')
			return -1;
		pd->b[i] = 16 * xdigit(*src++);
		if (*src == '\0')
			return -1;
		pd->b[i] |= xdigit(*src++);
	}
	return 0;
}
