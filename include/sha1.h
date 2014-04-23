/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * crc.h	SHA1-16 (CCITT) for WD179x et. al.
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_SHA1_H_INCLUDED_)
#define _SHA1_H_INCLUDED_

#include "system.h"

#define	SHA1SIZE	20

/* defines the state of the SHA1 algorithm */
typedef struct sha1_state_s {
	uint8_t accu[64];		/* accumulator */
	uint32_t state[5];		/* digest buffer */
	uint32_t count[2];  	/* message length in bits; LSW, MSW */
}	sha1_state_t;

typedef struct sha1_digest_s {
	uint8_t b[SHA1SIZE];
}	sha1_digest_t;

#ifdef	__cplusplus
extern "C" {
#endif

extern int xdigit(char ch);
extern void sha1_init(sha1_state_t *pss);
extern void sha1_append(sha1_state_t *pss, const void *data, size_t nbytes);
extern void sha1_finish(sha1_state_t *pss, sha1_digest_t *pd);
extern const char *sha1_hexstr(const sha1_digest_t *pd);
extern int sha1_strhex(sha1_digest_t *pd, const char *src);

#ifdef	__cplusplus
}
#endif

#endif /* !defined(_SHA1_H_INCLUDED_) */
