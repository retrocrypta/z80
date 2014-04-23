/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * cas.h	TRS80 cassette emulation.
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_CAS_H_INCLUDED_)
#define _CAS_H_INCLUDED_

#include "system.h"
#include "image.h"
#include "timer.h"

#ifdef	__cplusplus
extern "C" {
#endif

int trs80_cas_init(void);
void trs80_cas_stop(void);

uint8_t trs80_cas_r(uint32_t offset);
void trs80_cas_w(uint32_t offset, uint8_t data);

#ifdef	__cplusplus
}
#endif

#endif	/* !defined(_CAS_H_INCLUDED_) */
