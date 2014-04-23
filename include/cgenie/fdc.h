/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * cgeniefdc.h	Colour Genie EG2000 floppy disk controller
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_CGFDC_H_INCLUDED_)
#define _CGFDC_H_INCLUDED_

#include "system.h"
#include "osd.h"
#include "z80.h"
#include "image.h"
#include "wd179x.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern int cgenie_fdc_init(void);
extern void cgenie_fdc_reset(void);
extern void cgenie_fdc_stop(void);

extern uint8_t cgenie_irq_status_r(uint32_t offset);

extern void cgenie_motors_w(uint32_t offset, uint8_t data);

extern void cgenie_timer_interrupt(void);
extern void cgenie_fdc_interrupt(void);

extern void cgenie_fdc_callback(uint32_t event);

#ifdef	__cplusplus
}
#endif

#endif /* !defined(_CGFDC_H_INCLUDED_) */
