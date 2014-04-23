/* ed:set tabstop=8 noexpandtab: */
/*****************************************************************************
 *
 * mc6845.c	Motorola 6845 Cathod Ray Tube Controller
 *
 * Copyright Juergen Buchmueller <pullmoll@t-online.de>
 *
 *****************************************************************************/
#include "mc6845.h"

/*****************************************************************************
 *    This code emulates the functionality of the 6845 chip, and also
 *    supports the functionality of chips related to the 6845
 *
 *    Registers
 *      00 - horizontal total characters
 *      01 - horizontal displayed characters per line
 *      02 - horizontal synch position
 *      03 - horizontal synch width in characters
 *      04 - vertical total lines
 *      05 - vertical total adjust (scan lines)
 *      06 - vertical displayed rows
 *      07 - vertical synch position (character rows)
 *      08 - interlace mode
 *      09 - maximum scan line address
 *      0a - cursor start (scan line)
 *      0b - cursor end (scan line)
 *      0c - start address (MSB)
 *      0d - start address (LSB)
 *      0e - cursor address (MSB) (RW)
 *      0f - cursor address (LSB) (RW)
 *      10 - light pen (MSB) (RO)
 *      11 - light pen (LSB) (RO)
 *
 ****************************************************************************/

#define	CRTC6845_REGS	18

#define REG(x) crtc->reg[x]

#define CRTC6845_COLUMNS (REG(0) + 1)
#define CRTC6845_CHAR_COLUMNS REG(1)
#define	CRTC6845_HORIZONTAL_SYNC (REG(0) + 1 - REG(2))
#define CRTC6845_LINES (REG(4) * CRTC6845_CHAR_HEIGHT + REG(5))
#define CRTC6845_CHAR_LINES REG(6)
#define	CRTC6845_VERTICAL_SYNC (REG(4) + 1 - REG(7))
#define CRTC6845_CHAR_HEIGHT ((REG(9) & 31) + 1)
#define CRTC6845_VIDEO_START (256 * REG(12) + REG(13))
#define CRTC6845_INTERLACE_MODE (REG(8) % 4)
#define CRTC6845_INTERLACE_SIGNAL 1
#define CRTC6845_INTERLACE 3
#define CRTC6845_CURSOR_MODE (REG(10) & 0x60)
#define CRTC6845_CURSOR_OFF 0x20
#define CRTC6845_CURSOR_16FRAMES 0x40
#define CRTC6845_CURSOR_32FRAMES 0x60
#define CRTC6845_SKEW (REG(8) % 16)
#define CRTC6845_CURSOR_POS (256 * REG(14) + REG(15))
#define CRTC6845_CURSOR_TOP (REG(10) % 32)
#define CRTC6845_CURSOR_BOTTOM REG(11)

/***************************************************************************
 *
 *	Type definitions
 *
 ****************************************************************************/

typedef struct {
	ifc6845_t ifc;
	uint8_t reg[CRTC6845_REGS];
	uint8_t ndx;
	int32_t cursor_on;
	int32_t cursor_count;
	void *timer;
}	mc6845_t;

typedef struct regmask_s {
	uint8_t store_mask;
	uint8_t read_mask;
}	regmask_t;

static mc6845_t *mc6845;
static uint32_t num_chips;

/***************************************************************************
 *
 *	Local variables
 *
 ****************************************************************************/

/*-------------------------------------------------*
 * mc6845_reg_mask - an array specifying how
 * much of any given register "registers", per
 * m6845 type
 *-------------------------------------------------*/

static const regmask_t mc6845_reg_mask[2][18] = {
{ /* M6845_TYPE_GENUINE */
	{0xff, 0x00},
	{0xff, 0x00},
	{0xff, 0x00},
	{0xff, 0x00},
	{0x7f, 0x00},
	{0x1f, 0x00},
	{0x7f, 0x00},
	{0x7f, 0x00},
	{0x3f, 0x00},
	{0x1f, 0x00},
	{0x7f, 0x00},
	{0x1f, 0x00},
	{0x3f, 0x3f},
	{0xff, 0xff},
	{0x3f, 0x3f},
	{0xff, 0xff},
	{0xff, 0x3f},
	{0xff, 0xff}
},
{ /* M6845_TYPE_PC1512 */
	{0x00, 0x00},
	{0x00, 0x00},
	{0x00, 0x00},
	{0x00, 0x00},
	{0x00, 0x00},
	{0x00, 0x00},
	{0x00, 0x00},
	{0x00, 0x00},
	{0x00, 0x00},
	{0x1f, 0x00},
	{0x7f, 0x00},
	{0x1f, 0x00},
	{0x3f, 0x3f},
	{0xff, 0xff},
	{0x3f, 0x3f},
	{0xff, 0xff},
	{0xff, 0x3f},
	{0xff, 0xff}
}
};

/*
 * The PC1512 has not got a full MC6845; the first 9
 * registers act as if they had these hardwired values:
 */
static uint8_t pc1512_defaults[] =
{
	113,
	 80,
	 90,
	 10,
	127,
	  6,
	100,
	112,
	  2 
};

/***************************************************************************

	Functions

***************************************************************************/

static void mc6845_timer_callback(uint32_t chip);

int32_t mc6845_init(uint32_t num, ifc6845_t *config)
{
	uint32_t n;

	mc6845 = malloc(num * sizeof(mc6845_t));
	if (NULL == mc6845)
		return -1;
	num_chips = num;

	memset(mc6845, 0, num * sizeof(mc6845_t));
	for (num = 0; num < num_chips; num++) {
		mc6845[num].ifc = *config;
		mc6845[num].timer = tmr_alloc(mc6845_timer_callback,
			time_zero, num, tmr_double_to_time(TIME_IN_HZ(50.0/8)));

		/* Hardwire the values which can't be changed in the PC1512 version */
		if (config->type == M6845_TYPE_PC1512) {
			for (n = 0; n < sizeof(pc1512_defaults); n++)
				mc6845[num].reg[n] = pc1512_defaults[n];
		}
	}

	return 0;
}

#if	neverdef
static uint32_t mc6845_clocks_in_frame(uint32_t chip)
{
	mc6845_t *crtc = &mc6845[chip];
	uint32_t clocks = CRTC6845_COLUMNS * CRTC6845_LINES;

	switch (CRTC6845_INTERLACE_MODE) {
	case CRTC6845_INTERLACE_SIGNAL:
		/* interlace generation of video signals only */
		return clocks / 2;
	case CRTC6845_INTERLACE:
		/* interlace */
		return clocks / 2;
	}
	return clocks;
}
#endif

void mc6845_set_clock(uint32_t chip, uint32_t freq)
{
	mc6845_t *crtc = &mc6845[chip];

	crtc->ifc.freq = freq;
	sys_set_full_refresh();
}

static void mc6845_timer_callback(uint32_t chip)
{
	mc6845_t *crtc = &mc6845[chip];
	mc6845_cursor_t cursor;
	int32_t max;

	if (CRTC6845_CURSOR_MODE == CRTC6845_CURSOR_32FRAMES)
		max = 2;
	else
		max = 1;

	crtc->cursor_count++;
	if (crtc->cursor_count > max) {
		crtc->cursor_count = 0;
		crtc->cursor_on ^= 1;
	}

	mc6845_get_cursor(chip, &cursor);
	if (NULL != crtc->ifc.cursor_changed)
		(*crtc->ifc.cursor_changed)(chip, &cursor);
}

uint32_t mc6845_get_char_columns(uint32_t chip) 
{ 
	mc6845_t *crtc = &mc6845[chip];
	return CRTC6845_CHAR_COLUMNS;
}

uint32_t mc6845_get_char_height(uint32_t chip) 
{
	mc6845_t *crtc = &mc6845[chip];
	return CRTC6845_CHAR_HEIGHT;
}

uint32_t mc6845_get_char_lines(uint32_t chip) 
{ 
	mc6845_t *crtc = &mc6845[chip];
	return CRTC6845_CHAR_LINES;
}

uint32_t mc6845_get_start(uint32_t chip) 
{
	mc6845_t *crtc = &mc6845[chip];
	return CRTC6845_VIDEO_START;
}

uint32_t mc6845_get_horz_sync(uint32_t chip)
{
	mc6845_t *crtc = &mc6845[chip];
	return CRTC6845_HORIZONTAL_SYNC;
}

uint32_t mc6845_get_vert_sync(uint32_t chip)
{
	mc6845_t *crtc = &mc6845[chip];
	return CRTC6845_VERTICAL_SYNC;
}


uint32_t mc6845_get_type(uint32_t chip)
{
	mc6845_t *crtc = &mc6845[chip];
	return crtc->ifc.type;
}

void mc6845_get_cursor(uint32_t chip, mc6845_cursor_t *cursor)
{
	mc6845_t *crtc = &mc6845[chip];

	switch (CRTC6845_CURSOR_MODE) {
	case CRTC6845_CURSOR_OFF:
		cursor->on = 0;
		break;

	case CRTC6845_CURSOR_16FRAMES:
	case CRTC6845_CURSOR_32FRAMES:
		cursor->on = crtc->cursor_on;
		break;

	default:
		cursor->on = 1;
		break;
	}

	cursor->pos = CRTC6845_CURSOR_POS;
	cursor->top = CRTC6845_CURSOR_TOP;
	cursor->bottom = CRTC6845_CURSOR_BOTTOM;
}

uint8_t mc6845_r(uint32_t chip, uint32_t offset)
{
	mc6845_t *crtc = &mc6845[chip];
	uint8_t data = 0xff;

	if (0 == (offset & 1)) {
		data = crtc->ndx;
	} else {
		uint32_t n = crtc->ndx & 0x1f;
		if (n < CRTC6845_REGS)
			data = crtc->reg[n] & mc6845_reg_mask[crtc->ifc.type][n].read_mask;
	}
	return data;
}

int32_t mc6845_w(uint32_t chip, uint32_t offset, uint8_t data)
{
	mc6845_t *crtc = &mc6845[chip];
	mc6845_cursor_t cursor;
	uint8_t mask;
	uint32_t n;

	if (0 == (offset & 1)) {
		/* change the idx register */
		crtc->ndx = data;
		return 0;
	}

	/* write to a 6845 register, if supported */
	n = crtc->ndx & 0x1f;
	if (n >= CRTC6845_REGS)
		return -1;

	mask = mc6845_reg_mask[crtc->ifc.type][n].store_mask;

	LOG((7,"6845","write reg %x data:%02x mask:%02x\n",
		n, data, mask));

	/* Don't zero out bits not covered by the mask. */
	crtc->reg[n] &= ~mask;
	crtc->reg[n] |= (data & mask);

	/* are there special consequences to writing to this register? */
	switch (n) {
	case 0x0a:
	case 0x0b:
	case 0x0e:
	case 0x0f:
		mc6845_get_cursor(chip, &cursor);
		if (NULL != crtc->ifc.cursor_changed)
			(*crtc->ifc.cursor_changed)(chip, &cursor);
		break;

	default:
		sys_set_full_refresh();
		break;
	}
	return 1;
}

uint8_t mc6845_0_r(uint32_t offset) { return mc6845_r(0, offset); }
uint8_t mc6845_1_r(uint32_t offset) { return mc6845_r(1, offset); }
void mc6845_0_w(uint32_t offset, uint8_t data) { mc6845_w(0, offset, data); }
void mc6845_1_w(uint32_t offset, uint8_t data) { mc6845_w(1, offset, data); }
