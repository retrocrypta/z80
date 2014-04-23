/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * fdc.h	TRS80 floppy disk controller emulation
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_FDC_H_INCLUDED_)
#define _FDC_H_INCLUDED_

#include "system.h"
#include "osd.h"
#include "z80.h"
#include "image.h"
#include "wd179x.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern int trs80_fdc_init(void);
extern void trs80_fdc_reset(void);
extern void trs80_fdc_stop(void);

extern uint8_t trs80_irq_status_r(uint32_t offset);

extern void trs80_motors_w(uint32_t offset, uint8_t data);

extern void trs80_timer_interrupt(void);
extern void trs80_fdc_interrupt(void);

extern void trs80_fdc_callback(uint32_t event);

#ifdef	__cplusplus
}
#endif

#endif /* !defined(_FDC_H_INCLUDED_) */
