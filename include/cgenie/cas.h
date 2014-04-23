/***************************************************************************************
 *
 * cas.h	Colour Genie cassette emulation.
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

int cgenie_cas_init(void);
void cgenie_cas_stop(void);

uint8_t cgenie_cas_r(uint32_t offset);
void cgenie_cas_w(uint32_t offset, uint8_t data);

#ifdef	__cplusplus
}
#endif

#endif	/* !defined(_CAS_H_INCLUDED_) */
