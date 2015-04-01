/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * mc6845.h	Motorola 6845 Cathod Ray Tube Controller
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_MC6845_H_INCLUDED_)
#define _MC6845_H_INCLUDED_

#include "system.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *
 *	Constants
 *
 ****************************************************************************/

/*
 * maximum number of chips supported per system
 */
#define	MC6845_MAX	2

/*
 * since this code is used to support emulations of pseudo-M6845 chips,
 * that support functionality simlar to 6845's but are not true M6845 chips,
 * we need to have a concept of a type
 */
#define M6845_TYPE_GENUINE	0
#define M6845_TYPE_PC1512	1

/****************************************************************************
 *
 * Type definitions
 *
 ****************************************************************************/

typedef struct mc6845_cursor_s {
	uint32_t pos;
	uint32_t on;
	uint32_t top;
	uint32_t bottom;
}	mc6845_cursor_t;

typedef struct ifc6845_s {
	uint32_t type;
	uint32_t freq;
	void (*cursor_changed)(uint32_t chip, mc6845_cursor_t *old);
	uint32_t (*video_addr_changed)(uint32_t chip, uint32_t frame_base_old, uint32_t frame_base_new);
}	ifc6845_t;

/****************************************************************************
 *
 * Externs
 *
 ****************************************************************************/

/****************************************************************************
 *
 *	Prototypes
 *
 ****************************************************************************/

extern int32_t mc6845_init(uint32_t num, ifc6845_t *config);

void mc6845_set_clock(uint32_t chip, uint32_t freq);

uint32_t mc6845_get_char_columns(uint32_t chip);
uint32_t mc6845_get_char_height(uint32_t chip);
uint32_t mc6845_get_char_lines(uint32_t chip);
uint32_t mc6845_get_start(uint32_t chip);
uint32_t mc6845_get_horz_sync(uint32_t chip);
uint32_t mc6845_get_vert_sync(uint32_t chip);
uint32_t mc6845_get_type(uint32_t chip);

/*
 * cursor off
 * cursor on
 * cursor 16 frames on/off
 * cursor 32 frames on/off 
 * start line
 * end line
 */
void mc6845_get_cursor(uint32_t chip, mc6845_cursor_t *cursor);

uint8_t mc6845_r(uint32_t chip, uint32_t offset);
int32_t mc6845_w(uint32_t chip, uint32_t offset, uint8_t data);

uint8_t mc6845_0_r(uint32_t offset);
uint8_t mc6845_1_r(uint32_t offset);
void mc6845_0_w(uint32_t offset, uint8_t data);
void mc6845_1_w(uint32_t offset, uint8_t data);
	
#ifdef __cplusplus
}
#endif

#endif /* !defined(_MC6845_H_INCLUDED_) */
