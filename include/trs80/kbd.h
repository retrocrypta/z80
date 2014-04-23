/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * kbd.h	TRS-80 keyboard emulation
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_KBD_H_INCLUDED_)
#define	_KBD_H_INCLUDED_

#include "osd.h"

/** @brief reset the keyboard */
extern void trs80_kbd_reset(void);
/** @brief keyboard matrix */
extern uint8_t *trs80_kbd_map(void);
/** @brief callback for the OSD when a key is pressed */
extern void trs80_key_dn(void *cookie, osd_key_t *key);
/** @brief callback for the OSD when a key is released */
extern void trs80_key_up(void *cookie, osd_key_t *key);

#endif	/* !defined(_KBD_H_INCLUDED_) */
