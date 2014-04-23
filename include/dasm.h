/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * crc.h	DASM-16 (CCITT) for WD179x et. al.
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_DASM_H_INCLUDED_)
#define _DASM_H_INCLUDED_

#include "system.h"

typedef enum {
	DASM_NONE,
	DASM_CODE,
	DASM_ENTRY,
	DASM_BYTES,
	DASM_WORDS,
	DASM_ASCII,
	DASM_SPACE
}	dasm_type_t;

typedef struct {
	uint16_t addr;
	dasm_type_t type;
	char *comment;
}	dasm_t;

#endif /* !defined(_DASM_H_INCLUDED_) */
