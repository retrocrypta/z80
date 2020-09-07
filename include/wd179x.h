/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * wd179x.h	Western Digital 179x floppy disk controller
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_WD179X_H_INCLUDED_)
#define _WD179X_H_INCLUDED_

#include "system.h"
#include "timer.h"
#include "image.h"
#include "crc.h"
#include "floppy.h"

/***************************************************************************
 *
 *	Constants and enumerations
 *
 ***************************************************************************/

/* max. number of chips per system */
#define	WD179X_MAX	2

#define WD179X_IRQ_CLR	0
#define WD179X_IRQ_SET	1
#define WD179X_DRQ_CLR	2
#define WD179X_DRQ_SET	3

/* enumeration to specify the type of FDC; there are subtle differences */
enum {
	WD_TYPE_177X = 0,
	WD_TYPE_179X = 1,
	WD_TYPE_MB8877 = 1	/* duplicate constant until this is implemented */
};


typedef struct ifc_wd179x_s {
	/* types of chips */
	int wd_type[WD179X_MAX];

	/* callback to clear and set IRQ and DRQ lines */
	void (*callback[WD179X_MAX])(uint32_t param);
}	ifc_wd179x_t;

/***************************************************************************
 *
 *	Prototypes
 *
 ***************************************************************************/

int wd179x_init(uint32_t num, ifc_wd179x_t *config);
void wd179x_reset(uint32_t chip);
void wd179x_stop(void);

/*
 * The following are not strictly part of the wd179x
 * hardware/emulation but will be put here for now until
 * the flopdrv code has been finalised
 */
void wd179x_set_drive(uint32_t chip, uint8_t drive);	/* set drive wd179x is accessing */
void wd179x_set_side(uint32_t chip, uint8_t side);	/* set side wd179x is accessing */
void wd179x_set_density(uint32_t chip, uint8_t den);	/* set density */

uint8_t wd179x_0_status_r(uint32_t offset);
uint8_t wd179x_0_track_r(uint32_t offset);
uint8_t wd179x_0_sector_r(uint32_t offset);
uint8_t wd179x_0_data_r(uint32_t offset);

void wd179x_0_command_w(uint32_t offset, uint8_t data);
void wd179x_0_track_w(uint32_t offset, uint8_t data);
void wd179x_0_sector_w(uint32_t offset, uint8_t data);
void wd179x_0_data_w(uint32_t offset, uint8_t data);

uint8_t wd179x_1_status_r(uint32_t offset);
uint8_t wd179x_1_track_r(uint32_t offset);
uint8_t wd179x_1_sector_r(uint32_t offset);
uint8_t wd179x_1_data_r(uint32_t offset);

void wd179x_1_command_w(uint32_t offset, uint8_t data);
void wd179x_1_track_w(uint32_t offset, uint8_t data);
void wd179x_1_sector_w(uint32_t offset, uint8_t data);
void wd179x_1_data_w(uint32_t offset, uint8_t data);

uint8_t wd179x_0_r(uint32_t offset);
void wd179x_0_w(uint32_t offset, uint8_t data);

uint8_t wd179x_1_r(uint32_t offset);
void wd179x_1_w(uint32_t offset, uint8_t data);

#endif /* !defined(_WD179X_H_INCLUDED_) */


