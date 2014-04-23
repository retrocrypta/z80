/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * crc.h	CRC-16 (CCITT) for WD179x et. al.
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_CRC_H_INCLUDED_)
#define _CRC_H_INCLUDED_

#include "system.h"

uint16_t CALC_CRC1a(uint16_t crc, uint8_t c);
uint16_t CALC_CRC1b(uint16_t crc, uint8_t c);

/* Use the fast method with table lookup */
#ifndef calc_crc
#define	calc_crc CALC_CRC1b
#endif

#endif /* !defined(_CRC_H_INCLUDED_) */


