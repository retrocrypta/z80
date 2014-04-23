/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * wd179x.c	Western Digital 179x floppy disc controller
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#include "wd179x.h"

/***************************************************************************
 *
 *	Parameters
 *
 ****************************************************************************/

#define LL		3	/* General logging */
#define LS		4	/* Logging of each status read */
#define LD		5	/* Logging of each byte during read and write */
#define	LQ		7	/* logging of each DRQ set/reset */

#define DELAY_ERROR	3
#define DELAY_NOTREADY	1
#define DELAY_DATADONE	3


/***************************************************************************
 *
 *	Constants
 *
 ****************************************************************************/

#define TYPE_1		1
#define TYPE_2 	2
#define TYPE_3	3
#define TYPE_4 	4

#define FDC_STEP_RATE   0x03    /* Type I additional flags */
#define FDC_STEP_VERIFY 0x04	/* verify track number */
#define FDC_STEP_HDLOAD 0x08	/* load head */
#define FDC_STEP_UPDATE 0x10	/* update track register */

#define FDC_RESTORE 	0x00	/* Type I commands */
#define FDC_SEEK	0x10
#define FDC_STEP	0x20
#define FDC_STEP_IN 	0x40
#define FDC_STEP_OUT	0x60

#define FDC_MASK_TYPE_1	(FDC_STEP_HDLOAD|FDC_STEP_VERIFY|FDC_STEP_RATE)

/* Type 1 commands status */
#define STA_1_BUSY	0x01	/* controller is busy */
#define STA_1_IPL	0x02	/* index pulse */
#define STA_1_TRACK0	0x04	/* track 0 detected */
#define STA_1_CRC_ERR	0x08	/* CRC error */
#define STA_1_SEEK_ERR	0x10	/* seek error */
#define STA_1_HD_LOADED 0x20	/* head loaded */
#define STA_1_WRITE_PRO 0x40	/* floppy is write protected */
#define STA_1_NOT_READY 0x80	/* controller not ready */

/* Type 2 and 3 additional flags */
#define FDC_DELETED_AM	0x01	/* read/write deleted address mark */
#define FDC_SIDE_CMP_T	0x02	/* side compare track data */
#define FDC_15MS_DELAY	0x04	/* delay 15ms before command */
#define FDC_SIDE_CMP_S	0x08	/* side compare sector data */
#define FDC_MULTI_REC	0x10	/* only for type II commands */

/* Type 2 commands */
#define FDC_READ_SEC	0x80	/* read sector */
#define FDC_WRITE_SEC	0xA0	/* write sector */

#define FDC_MASK_TYPE_2	(FDC_MULTI_REC|FDC_SIDE_CMP_S|FDC_15MS_DELAY|FDC_SIDE_CMP_T|FDC_DELETED_AM)

/* Type 2 commands status */
#define STA_2_BUSY	0x01
#define STA_2_DRQ	0x02
#define STA_2_LOST_DAT	0x04
#define STA_2_CRC_ERR	0x08
#define STA_2_REC_N_FND 0x10
#define STA_2_REC_TYPE	0x20
#define STA_2_WRITE_PRO 0x40
#define STA_2_NOT_READY 0x80

#define FDC_MASK_TYPE_3	(FDC_15MS_DELAY)

/* Type 3 commands */
#define FDC_READ_DAM	0xc0	/* read data address mark */
#define FDC_READ_TRK	0xe0	/* read track */
#define FDC_WRITE_TRK	0xf0	/* write track (format) */

/* Type 4 additional flags */
#define FDC_IM0 	0x01	/* interrupt mode 0 */
#define FDC_IM1 	0x02	/* interrupt mode 1 */
#define FDC_IM2 	0x04	/* interrupt mode 2 */
#define FDC_IM3 	0x08	/* interrupt mode 3 */

#define FDC_MASK_TYPE_4	(FDC_IM3|FDC_IM2|FDC_IM1|FDC_IM0)

/* Type 4 commands */
#define FDC_FORCE_INT	0xd0	/* force interrupt */

/* Density change */
#define	FDC_DEN_SD	0xfe
#define	FDC_DEN_DD	0xff



/**
 * @brief chip_wd179x_s struct - per instance structure of the chip
 */
typedef struct chip_wd179x_s {
	void (*callback)(uint32_t event);	/* callback for IRQ status */
	int wd_type;

	/** @brief value of track register */
	uint8_t track_reg;		
	/** @brief seek to track number */
	uint8_t track_new;		
	/** @brief value of data register */
	uint8_t data;			
	/** @brief last command written */
	uint8_t command;		
	/** @brief last command type */
	uint8_t command_type;		
	/** @brief current sector number */
	uint8_t sector;			

	/** @brief last read command issued */
	uint8_t read_cmd;		
	/** @brief last write command issued */
	uint8_t write_cmd;		
	/** @brief last step direction */
	int8_t direction;		

	/** @brief status register */
	uint8_t status;			
	/** @brief status register data request bit */
	uint8_t status_drq;		

	/** @brief I/O buffer (holds a whole track) */
	uint8_t buffer[8192];		
	/** @brief offset into I/O buffer */
	uint32_t data_offset;		
	/** @brief transfer count from/into I/O buffer */
	int32_t data_count;		

	/** @brief pointers to data after formatting a track */
	uint8_t *fmt_data[128];		
	/** @brief list of DAMs (while writing track) */
	uint8_t dam_list[128][5];	
	/** @brief valid number of dam_list, fmt_data entries */
	int dam_cnt;			
	/** @brief sector length (bytes) */
	uint16_t sector_length;		

	/** @brief ddam of sector found - used when reading */
	uint8_t ddam;			
	/** @brief data id found in the ddam */
	uint8_t sector_data_id;
	/** @brief timer for miscellaneous events */
	tmr_t *misc_timer;
	/** @brief timer for controller busy state */
	tmr_t *busy_timer;
	/** @brief timer for track to track seek events */
	tmr_t *seek_timer;

	/* not specificially WD179x data, but useful for the emulation */

	/** @brief this is the drive currently selected */
	uint8_t drive;			
	/** @brief this is the head currently selected */
	uint8_t head;			
	/** @brief FM / MFM, single / double density */
	uint8_t density;		

}	chip_wd179x_t;


#define TRKSIZE_DD		6144
#define TRKSIZE_SD		3172

#if	neverdef
/** @brief track_DD - structure describing a double density track */
static uint8_t track_DD[][2] = {
	{16, 0x4e}, 	/* 16 * 4E (track lead in)		*/
	{ 8, 0x00}, 	/*  8 * 00 (pre DAM)			*/
	{ 3, 0xf5}, 	/*  3 * F5 (clear CRC)			*/

	{ 1, 0xfe}, 	/* *** sector *** FE (DAM)		*/
	{ 1, 0x80}, 	/*  4 bytes track,head,sector,seclen	*/
	{ 2, 0xf7}, 	/*  2 * F7 (CRC)			*/
	{22, 0x4e}, 	/* 22 * 4E (sector lead in)		*/
	{12, 0x00}, 	/* 12 * 00 (pre AM)			*/
	{ 3, 0xf5}, 	/*  3 * F5 (clear CRC)			*/
	{ 1, 0xfb}, 	/*  1 * FB (AM)				*/
	{ 1, 0x81}, 	/*  x bytes sector data			*/
	{ 2, 0xf7}, 	/*  2 * F7 (CRC)			*/
	{16, 0x4e}, 	/* 16 * 4E (sector lead out)		*/
	{ 8, 0x00}, 	/*  8 * 00 (post sector)		*/
	{ 0, 0x00}, 	/* end of data				*/
};

/** @brief track_SD - structure describing a single density track */
static uint8_t track_SD[][2] = {
	{16, 0xff}, 	/* 16 * FF (track lead in)		*/
	{ 8, 0x00}, 	/*  8 * 00 (pre DAM)			*/
	{ 1, 0xfc}, 	/*  1 * FC (clear CRC)			*/

	{11, 0xff}, 	/* *** sector *** 11 * FF		*/
	{ 6, 0x00}, 	/*  6 * 00 (pre DAM)			*/
	{ 1, 0xfe}, 	/*  1 * FE (DAM)			*/
	{ 1, 0x80}, 	/*  4 bytes track,head,sector,seclen	*/
	{ 2, 0xf7}, 	/*  2 * F7 (CRC)			*/
	{10, 0xff}, 	/* 10 * FF (sector lead in)		*/
	{ 4, 0x00}, 	/*  4 * 00 (pre AM)			*/
	{ 1, 0xfb}, 	/*  1 * FB (AM)				*/
	{ 1, 0x81}, 	/*  x bytes sector data			*/
	{ 2, 0xf7}, 	/*  2 * F7 (CRC)			*/
	{ 0, 0x00}, 	/* end of data				*/
};
#endif


/***************************************************************************

	Prototypes

***************************************************************************/

static void wd179x_complete_command(uint32_t chip, int delay);
static void wd179x_clear_data_request(uint32_t chip);
static void wd179x_set_data_request(uint32_t chip);
static void wd179x_timed_data_request(uint32_t chip);
static void wd179x_timed_read_sector_request(uint32_t chip);
static void wd179x_timed_write_sector_request(uint32_t chip);
static void wd179x_timed_read_track_request(uint32_t chip);
static void wd179x_timed_write_track_request(uint32_t chip);
static void wd179x_timed_seek(uint32_t chip, uint32_t steprate);
static void wd179x_set_irq(uint32_t chip);

static void wd179x_seek_callback(uint32_t chip);
static void wd179x_busy_callback(uint32_t chip);
static void wd179x_misc_timer_callback(uint32_t chip);
static void wd179x_read_sector_callback(uint32_t chip);
static void wd179x_write_sector_callback(uint32_t chip);
static void wd179x_read_track_callback(uint32_t chip);
static void wd179x_write_track_callback(uint32_t chip);

/* one wd controlling multiple drives */
static chip_wd179x_t *chips;
static uint32_t num_chips;

/**************************************************************************/

static struct img_s *wd179x_image(uint8_t drive)
{
	return img_file(IMG_TYPE_FD, drive);
}



/* use this to determine which drive is controlled by WD */
void wd179x_set_drive(uint32_t chip, uint8_t drive)
{
	chip_wd179x_t *w = &chips[chip];

	if (chip >= num_chips)
		return;
	if (w->drive != drive) {
		LOG((LL,"WD179x","#%x set drive %x\n",
			w->drive, drive));
	}
	w->drive = drive;
}



void wd179x_set_side(uint32_t chip, uint8_t head)
{
	chip_wd179x_t *w = &chips[chip];

	if (chip >= num_chips)
		return;
	if (w->head != head) {
		LOG((LL,"WD179x","#%x set side %x\n",
			w->drive, head));
	}
	w->head = head;
}



void wd179x_set_density(uint32_t chip, uint8_t density)
{
	chip_wd179x_t *w = &chips[chip];

	if (chip >= num_chips)
		return;
	if (w->density != density) {
		LOG((LL,"WD179x","#%x set density %x\n",
			w->drive, density));
	}
	w->density = density;
}



static void wd179x_busy_callback(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];

	if (chip >= num_chips)
		return;
	wd179x_set_irq(chip);
	tmr_reset(w->busy_timer, time_never);
}



static void wd179x_set_busy(uint32_t chip, double milliseconds)
{
	chip_wd179x_t *w = &chips[chip];
	int rc;

	if (chip >= num_chips)
		return;
	w->status |= STA_1_BUSY;
	rc = tmr_adjust(w->busy_timer,
		tmr_double_to_time(TIME_IN_MSEC(milliseconds)),
		chip, time_zero);
	if (0 != rc) {
		LOG((LL,"WD179x","#%x tmr_adjust(busy_timer, %dms) failed\n",
			w->drive, milliseconds));
	}
}



static void wd179x_restore(uint32_t chip, uint32_t data)
{
	chip_wd179x_t *w = &chips[chip];
	struct img_s *img;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);
	/* setup step direction */
	w->direction = -1;

	w->track_reg = img_get_flag(img, DRV_CURRENT_CYLINDER);

	/* seek to track 0 */
	w->track_new = 0;

	w->command_type = TYPE_1;

	if (0 == img_get_flag(img, IMG_EXISTS)) {
		w->status |= STA_1_SEEK_ERR;
		w->status &= ~STA_1_BUSY;
		return;
	}

	wd179x_timed_seek(chip, (data & FDC_STEP_RATE) + 1);
}



void wd179x_stop(void)
{
	uint32_t chip;

	LOG((1,"WD179x","Stop %u chips\n", num_chips));

	for (chip = 0; chip < num_chips; chip++) {
		chip_wd179x_t *w = &chips[chip];

		if (NULL != w->misc_timer)
			tmr_remove(w->misc_timer);
		if (NULL != w->busy_timer)
			tmr_remove(w->busy_timer);
		if (NULL != w->seek_timer)
			tmr_remove(w->seek_timer);
	}

	free(chips);
	chips = NULL;

	num_chips = 0;
}

void wd179x_reset(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];
	struct img_s *img;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);

	w->status = STA_1_NOT_READY | STA_1_IPL;
	w->density = DEN_FM_LO;

	img_set_flag(img, DRV_CURRENT_CYLINDER, 5);

	if (NULL != w->misc_timer)
		tmr_remove(w->misc_timer);
	w->misc_timer = tmr_alloc(wd179x_misc_timer_callback, time_never, 0, time_never);

	if (NULL != w->busy_timer)
		tmr_remove(w->busy_timer);
	w->busy_timer = tmr_alloc(wd179x_busy_callback, time_never, chip, time_never);

	if (NULL != w->seek_timer)
		tmr_remove(w->seek_timer);
	w->seek_timer = tmr_alloc(wd179x_seek_callback, time_never, chip, time_never);

	wd179x_restore(chip, 0);
}

int wd179x_init(uint32_t num, ifc_wd179x_t *config)
{
	uint32_t chip;
	LOG((1,"WD179x","Initialize for %u chips\n", num));

	chips = malloc(num * sizeof(chip_wd179x_t));
	if (NULL == chips)
		return -1;
	memset(chips, 0, num * sizeof(chip_wd179x_t));

	num_chips = num;
	for (chip = 0; chip < num_chips; chip++) {
		chip_wd179x_t *w = &chips[chip];

		w->wd_type = config->wd_type[chip];
		w->callback = config->callback[chip];

		wd179x_reset(chip);
	}
	return 0;
}

static void wd179x_write_track(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];
	struct img_s *img;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);
	fdd_write_track(img, w->track_reg, w->head,
		w->buffer, w->data_offset, w->density);
	w->status &= ~STA_2_DRQ;
}



/* read an entire track */
static void wd179x_read_track(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];
	struct img_s *img;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);
	fdd_read_track(img, w->track_reg, w->head,
		w->buffer, sizeof(w->buffer), w->density);

	w->data_offset = 0;
	w->data_count = DEN_MFM_LO == w->density ? TRKSIZE_DD : TRKSIZE_SD;

	w->status &= ~STA_2_DRQ;
	wd179x_set_data_request(chip);
}

/* read the next data address mark */
static void wd179x_read_id(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];
	struct img_s *img;
	fdd_chrn_id_t id;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);

	w->status &= ~(STA_2_CRC_ERR | STA_2_REC_N_FND);

	/* get next id from disc */
	if (0 == fdd_get_next_id(img, w->head, &id, w->density)) {
		uint16_t crc = 0xffff;

		w->data_offset = 0;
		w->data_count = 6;

		/* for MFM */
		/* crc includes three times a1, and once fe (id mark) */
		crc = calc_crc(crc, 0xa1);
		crc = calc_crc(crc, 0xa1);
		crc = calc_crc(crc, 0xa1);

		crc = calc_crc(crc, 0xfe);
		w->buffer[0] = id.C;
		w->buffer[1] = id.H;
		w->buffer[2] = id.R;
		w->buffer[3] = id.N;
		crc = calc_crc(crc, w->buffer[0]);
		crc = calc_crc(crc, w->buffer[1]);
		crc = calc_crc(crc, w->buffer[2]);
		crc = calc_crc(crc, w->buffer[3]);

		/* crc is stored hi-byte followed by lo-byte */
		w->buffer[4] = crc / 256;
		w->buffer[5] = crc % 256;
		LOG((3,"WD179x","#%x read id succeeded (CRC %04x)\n",
			w->drive, crc));

		/* sector set to cylinder number? really? */
		w->sector = id.C;
		w->status |= STA_2_BUSY;

		wd179x_set_data_request(chip);
	} else {
		/* record not found */
		w->status |= STA_2_REC_N_FND;
		/*w->sector = w->track_reg; */
		LOG((3,"WD179x","#%x read id failed\n",
			w->drive));

		wd179x_complete_command(chip, DELAY_ERROR);
	}
}



static int wd179x_find_sector(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];
	struct img_s *img;
	uint32_t revolution_count;
	fdd_chrn_id_t id;

	if (chip >= num_chips)
		return -1;
	img = wd179x_image(w->drive);
	revolution_count = 0;
	w->status &= ~STA_2_REC_N_FND;

	while (revolution_count < 4) {
		 /* index set? */
		if (0 != img_get_flag(img, DRV_INDEX)) {
			/* update revolution count */
			revolution_count++;
		}

		if (0 != fdd_get_next_id(img, w->head, &id, w->density))
			break;

		/* compare track */
		if (id.C != w->track_reg)
			continue;

		/* compare id */
		if (id.R != w->sector)
			continue;

		w->sector_length = 0x80 << id.N;
		w->sector_data_id = id.data_id;

		/* get ddam status */
		w->ddam = id.flags & ID_FLAG_DELETED_DATA;

		/* got record type here */
		LOG((LL,"WD179x","#%x found C:%02x H:%02x R:%02x N:%02x%s\n",
			w->drive, id.C, id.H, id.R, id.N,
			w->ddam ? " DDAM" : ""));
		return 0;
	}

	/* record not found */
	w->status |= STA_2_REC_N_FND;

	LOG((LL,"WD179x","#%x track %02x sector %02x not found!\n",
		w->drive, w->track_reg, w->sector));
	wd179x_complete_command(chip, DELAY_ERROR);

	return -1;
}



/* read a sector */
static void wd179x_read_sector(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];
	struct img_s *img;
	fdd_chrn_id_t id;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);
	w->data_offset = 0;

	if (0 != wd179x_find_sector(chip))
		return;
	w->data_count = w->sector_length;

	/* read data */
	fdd_read_sector_data(img, w->head, w->sector_data_id,
		w->buffer, w->sector_length, &id, w->density);

	w->sector_data_id = id.data_id;
	/* get ddam status */
	w->ddam = id.flags & ID_FLAG_DELETED_DATA;

	wd179x_timed_data_request(chip);

	w->status |= STA_2_BUSY;
}



static void wd179x_set_irq(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];

	if (chip >= num_chips)
		return;
	LOG((3,"WD179x","#%x set IRQ (sta %02x, callback %p)\n",
		w->drive, w->status, w->callback));
	w->status &= ~STA_2_BUSY;

	/* generate an IRQ */
	if (NULL != w->callback)
		(*w->callback) (WD179X_IRQ_SET);
}


static void wd179x_seek_callback(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];
	struct img_s *img;
	int rc;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);

	LOG((1,"WD179x","seek callback track:%02x new:%02x\n",
		w->track_reg, w->track_new));

	fdd_seek(img, w->direction);

	/* update track reg */
	w->track_reg += w->direction;

	/* keep stepping until we reached track_new */
	if (w->track_reg != w->track_new) {
		/* kick the seek timer */
		rc = tmr_adjust(w->seek_timer,
			tmr_double_to_time(TIME_IN_MSEC(1)), chip, time_zero);
		if (0 != rc) {
			LOG((LL,"WD179x","#%x tmr_adjust(busy_timer, %dms) failed\n",
				w->drive, 1));
		}
	} else {
		rc = tmr_reset(w->seek_timer, time_never);
		if (0 != rc) {
			LOG((LL,"WD179x","#%x tmr_adjust(busy_timer, never) failed\n",
				w->drive));
		}
		/* clear busy status */
		w->status &= ~STA_1_BUSY;
	}
}


enum {
	MISCCB_COMMAND,
	MISCCB_DATA,
	MISCCB_READ_SEC,
	MISCCB_WRITE_SEC,
	MISCCB_READ_TRK,
	MISCCB_WRITE_TRK
};



static void wd179x_misc_timer_callback(uint32_t param)
{
	uint32_t chip = param % 16;
	uint32_t callback_type = param / 16;
	chip_wd179x_t *w = &chips[chip];

	if (chip >= num_chips)
		return;

	/* stop timer, but don't allow it to be free'd */
	tmr_reset(w->misc_timer, time_never); 

	switch (callback_type) {
	case MISCCB_COMMAND:	/* command callback */
		wd179x_set_irq(chip);
		break;

	case MISCCB_DATA:	/* data callback */
		wd179x_set_data_request(chip);
		break;

	case MISCCB_READ_SEC:	/* read sector callback */
		wd179x_read_sector_callback(chip);
		break;

	case MISCCB_WRITE_SEC:	/* write sector callback */
		wd179x_write_sector_callback(chip);
		break;

	case MISCCB_READ_TRK:	/* read track callback */
		wd179x_read_track_callback(chip);
		break;

	case MISCCB_WRITE_TRK:	/* write track callback */
		wd179x_write_track_callback(chip);
		break;
	}
}


/*
 * called on error, or when a command is actually completed
 */
static void wd179x_complete_command(uint32_t chip, int delay)
{
	chip_wd179x_t *w = &chips[chip];
	int usecs;
	int rc;

	if (chip >= num_chips)
		return;

	w->data_count = 0;

	/* clear busy bit */
	w->status &= ~STA_2_BUSY;

	usecs = delay * fdd_get_datarate_in_us(w->density);

	/* set new timer */
	rc = tmr_adjust(w->misc_timer,
		tmr_double_to_time(TIME_IN_USEC(usecs)),
		16 * MISCCB_COMMAND + chip, time_zero);
	if (0 != rc) {
		LOG((LL,"WD179x","#%x tmr_adjust(misc_command, %dus) failed\n",
			w->drive, usecs));
	}
}



static void wd179x_write_sector(uint32_t chip)
{
	chip_wd179x_t *w =  &chips[chip];
	struct img_s *img;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);

	/*
	 * At this point, the disc is write enabled, and data
	 * has been transfered into our buffer - now write it to
	 * the disc image or to the real disc
	 */

	/* write data */
	fdd_write_sector_data(img, w->head, w->sector_data_id,
		w->buffer, w->sector_length,
		w->density, w->write_cmd & FDC_DELETED_AM);
}



/*
 * verify the seek operation by looking for a id that
 * has a matching track value
 */
static void wd179x_verify_seek(uint32_t chip)
{
	chip_wd179x_t *w =  &chips[chip];
	struct img_s *img;
	uint32_t revolution_count;
	fdd_chrn_id_t id;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);

	revolution_count = 0;

	LOG((3,"WD179x","#%x seek w/ verify\n", w->drive));

	w->status &= ~STA_1_SEEK_ERR;

	/* must be found within 5 revolutions otherwise error */
	while (revolution_count < 5) {
		/* compare track */
		if (0 == fdd_get_next_id(img, w->head, &id, w->density) &&
			id.C == w->track_reg) {
			LOG((3,"WD179x","#%x seek verify succeeded!\n",
				w->drive));
			return;
		}

		 /* index set? */
		if (0 != img_get_flag(img, DRV_INDEX)) {
			/* update revolution count */
			revolution_count++;
		}
	}

	w->status |= STA_1_SEEK_ERR;

	LOG((3,"WD179x","#%x seek w/ verify failed", w->drive));
}



/* clear a data request */
static void wd179x_clear_data_request(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];

	if (chip >= num_chips)
		return;

	LOG((LQ,"WD179x","#%x clear DRQ (sta %02x, callback %p)\n",
		w->drive, w->status, w->callback));

	w->status &= ~STA_2_DRQ;
	if (NULL != w->callback)
		(*w->callback)(WD179X_DRQ_CLR);
}



/* set data request */
static void wd179x_set_data_request(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];

	if (chip >= num_chips)
		return;

	if (w->status & STA_2_DRQ) {
		LOG((LL,"WD179x","#%x set DRQ - lost data\n",
			w->drive, w->data_count));
		w->status |= STA_2_LOST_DAT;
	}

	LOG((LQ,"WD179x","#%x set DRQ (sta %02x, callback %p)\n",
		w->drive, w->status, w->callback));
	w->status |= STA_2_DRQ;
	if (NULL != w->callback)
		(*w->callback)(WD179X_DRQ_SET);
}



/* callback to initiate read sector */
static void wd179x_read_sector_callback(uint32_t chip)
{
	chip_wd179x_t *w =  &chips[chip];
	struct img_s *img;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);

	LOG((LL,"WD179x","#%x read sector callback\n",
		w->drive));

	if (0 == img_get_flag(img, DRV_READY)) {
		wd179x_complete_command(chip, DELAY_NOTREADY);
	} else {
		wd179x_read_sector(chip);
	}
}



/* callback to initiate write sector */
static void wd179x_write_sector_callback(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];
	struct img_s *img;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);

	LOG((LL,"WD179x","#%x write sector callback\n",
		w->drive));

	if (0 == img_get_flag(img, DRV_READY)) {
		LOG((3,"WD179x","#%x write sector callback - not ready\n",
			w->drive));
		wd179x_complete_command(chip, DELAY_NOTREADY);
		return;
	}

	/* drive write protected? */
	if (img_get_flag(img, DRV_DISK_WRITE_PROTECTED)) {
		LOG((3,"WD179x","#%x write sector callback - write prot\n",
			w->drive));
		w->status |= STA_2_WRITE_PRO;
		wd179x_complete_command(chip, DELAY_ERROR);
		return;
	}

	/* request data */
	w->data_offset = 0;
	w->data_count = w->sector_length;
	w->status &= ~STA_2_DRQ;
	wd179x_set_data_request(chip);
}


/* callback to initiate read track */
static void wd179x_read_track_callback(uint32_t chip)
{
	chip_wd179x_t *w =  &chips[chip];
	struct img_s *img;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);

	LOG((LL,"WD179x","#%x read track callback\n", w->drive));

	if (0 == img_get_flag(img, DRV_READY)) {
		LOG((3,"WD179x","#%x read track callback - not ready\n", w->drive));
		wd179x_complete_command(chip, DELAY_NOTREADY);
	} else {
		wd179x_read_track(chip);
	}
}



/* callback to initiate write track */
static void wd179x_write_track_callback(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];
	struct img_s *img;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);

	LOG((LL,"WD179x","#%x write track callback\n",
		w->drive));

	if (0 == img_get_flag(img, DRV_READY)) {
		LOG((3,"WD179x","#%x write track callback - not ready\n",
			w->drive));
		wd179x_complete_command(chip, DELAY_NOTREADY);
		return;
	}

	/* drive write protected? */
	if (img_get_flag(img, DRV_DISK_WRITE_PROTECTED)) {
		LOG((3,"WD179x","#%x write track callback - write prot\n",
			w->drive));
		w->status |= STA_2_WRITE_PRO;
		wd179x_complete_command(chip, DELAY_ERROR);
		return;
	}

	/* request data */
	w->data_offset = 0;
	w->data_count = DEN_MFM_LO == w->density ? TRKSIZE_DD : TRKSIZE_SD;
	w->status &= ~STA_2_DRQ;
	wd179x_set_data_request(chip);
}


/*
 * setup a timed data request
 * data request will be triggered in a few usecs time
 */
static void wd179x_timed_data_request(uint32_t chip)
{
	chip_wd179x_t *w = &chips[chip];
	int usecs;
	int rc;

	if (chip >= num_chips)
		return;

	LOG((LL,"WD179x","#%x timed data request callback\n",
		w->drive));

	usecs = fdd_get_datarate_in_us(w->density);

	/* set new timer */
	rc = tmr_adjust(w->misc_timer,
		tmr_double_to_time(TIME_IN_USEC(usecs)),
		16 * MISCCB_DATA + chip, time_zero);
	if (0 != rc) {
		LOG((LL,"WD179x","#%x tmr_adjust(misc_data, %dus) failed\n",
			w->drive, usecs));
	}
}



/* setup a timed read sector
 * read sector will be triggered in a few usecs time
 */
static void wd179x_timed_read_sector_request(uint32_t chip)
{
	chip_wd179x_t *w =  &chips[chip];
	int usecs = 20;
	int rc;

	if (chip >= num_chips)
		return;

	LOG((LL,"WD179x","#%x timed read sector request callback\n",
		w->drive));

	/* set new timer */
	rc = tmr_adjust(w->misc_timer,
		tmr_double_to_time(TIME_IN_USEC(usecs)),
		16 * MISCCB_READ_SEC + chip, time_zero);
	if (0 != rc) {
		LOG((LL,"WD179x","#%x tmr_adjust(misc_read_sec, %dus) failed\n",
			w->drive, usecs));
	}
}



/* setup a timed write sector
 * write sector will be triggered in a few usecs time
 */
static void wd179x_timed_write_sector_request(uint32_t chip)
{
	chip_wd179x_t *w =  &chips[chip];
	int usecs = 5;
	int rc;

	if (chip >= num_chips)
		return;

	LOG((LL,"WD179x","#%x timed write sector request callback\n",
		w->drive));

	/* find the desired sector */
	if (0 != wd179x_find_sector(chip))
		return;

	/* set new timer */
	rc = tmr_adjust(w->misc_timer,
		tmr_double_to_time(TIME_IN_USEC(usecs)),
		16 * MISCCB_WRITE_SEC + chip, time_zero);
	if (0 != rc) {
		LOG((LL,"WD179x","#%x tmr_adjust(misc_write_sec, %dus) failed\n",
			w->drive, usecs));
	}
}

/*
 * setup a timed read track
 * read track will be triggered in a few usecs time
 */
static void wd179x_timed_read_track_request(uint32_t chip)
{
	chip_wd179x_t *w =  &chips[chip];
	int usecs = 20;
	int rc;

	if (chip >= num_chips)
		return;

	LOG((LL,"WD179x","#%x timed read track request callback\n",
		w->drive));

	/* set new timer */
	rc = tmr_adjust(w->misc_timer,
		tmr_double_to_time(TIME_IN_USEC(usecs)),
		16 * MISCCB_READ_TRK + chip, time_zero);
	if (0 != rc) {
		LOG((LL,"WD179x","#%x tmr_adjust(misc_read_trk, %dus) failed\n",
			w->drive, usecs));
	}
}

/*
 * setup a timed write track
 * write track will be triggered in a few usecs time
 */
static void wd179x_timed_write_track_request(uint32_t chip)
{
	chip_wd179x_t *w =  &chips[chip];
	int usecs = 20;
	int rc;

	if (chip >= num_chips)
		return;

	LOG((LL,"WD179x","#%x timed write track request callback\n",
		w->drive));

	/* set new timer */
	rc = tmr_adjust(w->misc_timer,
		tmr_double_to_time(TIME_IN_USEC(usecs)),
		16 * MISCCB_WRITE_TRK + chip, time_zero);
	if (0 != rc) {
		LOG((LL,"WD179x","#%x tmr_adjust(misc_write_trk, %dus) failed\n",
			w->drive, usecs));
	}
}

static void wd179x_timed_seek(uint32_t chip, uint32_t steprate)
{
	chip_wd179x_t *w =  &chips[chip];
	int rc;

	if (chip >= num_chips)
		return;

	LOG((LL,"WD179x","#%x timed seek callback track:%02x new:%02x srt:%dms\n",
		w->drive, w->track_reg, w->track_new, steprate));

	if (w->track_new == w->track_reg) {
		w->status &= ~(STA_1_BUSY | STA_1_IPL);
		return;
	}

	/* set busy status */
	w->status |= STA_1_BUSY;

	/* and setup the seek timer */
	rc = tmr_adjust(w->seek_timer,
		tmr_double_to_time(TIME_IN_MSEC(steprate)),
		chip, time_zero);
	if (0 != rc) {
		LOG((LL,"WD179x","#%x tmr_adjust(seek_timer, %dms) failed\n",
			w->drive, steprate));
	}
}



/* read the FDC status register, and clear IRQ line, too */
uint32_t wd179x_status_r(uint32_t chip, uint32_t offset)
{
	chip_wd179x_t *w = &chips[chip];
	struct img_s *img;
	uint32_t result = w->status;

	if (chip >= num_chips)
		return 0x00;
	img = wd179x_image(w->drive);

	if (NULL != w->callback)
		(*w->callback) (WD179X_IRQ_CLR);

	/* type 1 command or force int command? */
	if (w->command_type == TYPE_1 || w->command_type == TYPE_4) {

		/* if disc present toggle index pulse */
		if (0 != img_get_flag(img, IMG_EXISTS)) {
			/* eventually toggle index pulse bit */
			w->status ^= STA_1_IPL;
		} else {
			return 0x00;
		}

		/* set track 0 state */
		w->status &= ~STA_1_TRACK0;
		if (img_get_flag(img, DRV_HEAD_AT_TRACK_0))
			w->status |= STA_1_TRACK0;

		w->status &= ~STA_1_NOT_READY;
		if (0 == img_get_flag(img, DRV_READY))
			w->status |= STA_1_NOT_READY;

		result = w->status;
	}
	
	if (w->data_count < 4) {
		LOG((LS,"WD179x","#%x status_r: %02x (data_count %04x)\n",
			w->drive, result, w->data_count));
	}

	return result;
}



/* read the FDC track register */
uint32_t wd179x_track_r(uint32_t chip, uint32_t offset)
{
	chip_wd179x_t *w = &chips[chip];

	if (chip >= num_chips)
		return 0x00;

	LOG((LL,"WD179x","#%x track_r: %02x\n",
		w->drive, w->track_reg));
	return w->track_reg;
}



/* read the FDC sector register */
uint32_t wd179x_sector_r(uint32_t chip, uint32_t offset)
{
	chip_wd179x_t *w = &chips[chip];

	if (chip >= num_chips)
		return 0x00;

	LOG((LL,"WD179x","#%x sector_r: %02x\n",
		w->drive, w->sector));
	return w->sector;
}



/* read the FDC data register */
uint32_t wd179x_data_r(uint32_t chip, uint32_t offset)
{
	chip_wd179x_t *w = &chips[chip];

	if (chip >= num_chips)
		return 0x00;

	if (0 == w->data_count) {
		LOG((LL,"WD179x","#%x data_r: (no new data) %02x\n",
			w->drive, w->data));
		return w->data;
	}

	/* clear data request */
	wd179x_clear_data_request(chip);

	w->data = w->buffer[w->data_offset++];

	LOG((LD,"WD179x","#%x data_r: %02x (data_count %04x)\n",
		w->drive, w->data, w->data_count));
	w->data_count--;
	if (w->data_count > 0) {
		/* issue a timed data request */
		wd179x_timed_data_request(chip);
		return w->data;
	}

	/* no bytes remaining */
	w->data_offset = 0;

	/* clear ddam type */
	w->status &= ~STA_2_REC_TYPE;

	/* read a sector with ddam set? */
	if (w->command_type == TYPE_2 && w->ddam != 0) {
		/* set it */
		w->status |= STA_2_REC_TYPE;
	}

	LOG((LL,"WD179x","#%x data read completed (sta %02x, data %02x)\n",
		w->drive, w->status, w->data));

	/*
	 * Delay the INTRQ 3 byte times because we need to read
	 * two CRC bytes and compare them with a calculated CRC
	 */
	wd179x_complete_command(chip, DELAY_DATADONE);

	return w->data;
}



/* write the FDC command register */
void wd179x_command_w(uint32_t chip, uint32_t offset, uint32_t data)
{
	chip_wd179x_t *w = &chips[chip];
	struct img_s *img;

	if (chip >= num_chips)
		return;
	img = wd179x_image(w->drive);

#if	0	/* FIXME: the system should run the motor */
	img_set_flag(img, DRV_MOTOR_ON, 1);
#endif
	/* FIXME: drive ready from floppy.c or image.c? */
	img_set_flag(img, DRV_READY, 1);

	/* IRQ also cleared by writing command */
	if (NULL != w->callback)
		(*w->callback)(WD179X_IRQ_CLR);

	w->status &= ~STA_1_SEEK_ERR;
	w->status &= ~STA_1_HD_LOADED;
	w->status &= ~STA_1_WRITE_PRO;

	if (FDC_FORCE_INT == (data & ~FDC_MASK_TYPE_4)) {
		LOG((LL,"WD179x","#%x command_w %02x FORCE_INT\n",
			w->drive, data));
		w->data_count = 0;
		w->data_offset = 0;
		w->status &= ~STA_2_BUSY;
		wd179x_clear_data_request(chip);

		if (data & 0x0f) {
			/* What was with the low bits?
			 * Something with interrupt modes...
			 */
		}

		w->command_type = TYPE_4;
		return;
	}

	if (0 != (data & 0x80)) {

		if (FDC_READ_SEC == (data & ~FDC_MASK_TYPE_2)) {
			LOG((LL,"WD179x","#%x command_w %02x READ_SEC\n",
				w->drive, data));
			w->read_cmd = data;
			w->command = data & ~FDC_MASK_TYPE_2;
			w->command_type = TYPE_2;
			w->status &= ~STA_2_LOST_DAT;
			w->status |= STA_2_BUSY;
			wd179x_clear_data_request(chip);
			if (0 == img_get_flag(img, DRV_READY)) {
				wd179x_complete_command(chip, DELAY_NOTREADY);
			} else {
				wd179x_timed_read_sector_request(chip);
			}
			return;
		}

		if (FDC_WRITE_SEC == (data & ~FDC_MASK_TYPE_2)) {
			LOG((LL,"WD179x","#%x command_w %02x WRITE_SEC\n",
				w->drive, data));
			w->write_cmd = data;
			w->command = data & ~FDC_MASK_TYPE_2;
			w->command_type = TYPE_2;
			w->status &= ~STA_2_LOST_DAT;
			w->status |= STA_2_BUSY;
			wd179x_clear_data_request(chip);
			if (0 == img_get_flag(img, DRV_READY)) {
				wd179x_complete_command(chip, DELAY_NOTREADY);
			} else {
				wd179x_timed_write_sector_request(chip);
			}
			return;
		}

		if (FDC_READ_TRK == (data & ~FDC_MASK_TYPE_3)) {
			LOG((LL,"WD179x","#%x command_w %02x READ_TRK\n",
				w->drive, data));
			w->command = data & ~FDC_MASK_TYPE_3;
			w->command_type = TYPE_3;
			w->status &= ~STA_2_LOST_DAT;
			w->status |= STA_2_BUSY;
			wd179x_clear_data_request(chip);
			if (0 == img_get_flag(img, DRV_READY)) {
				wd179x_complete_command(chip, DELAY_NOTREADY);
			} else {
				wd179x_timed_read_track_request(chip);
			}
			return;
		}

		if (FDC_WRITE_TRK == (data & ~FDC_MASK_TYPE_3)) {
			LOG((LL,"WD179x","#%x command_w %02x WRITE_TRK\n",
				w->drive, data));
			w->command = data & ~FDC_MASK_TYPE_3;
			w->command_type = TYPE_3;
			w->status &= ~STA_2_LOST_DAT;
			w->status |= STA_2_BUSY;
			wd179x_clear_data_request(chip);
			if (0 == img_get_flag(img, DRV_READY)) {
				wd179x_complete_command(chip, DELAY_NOTREADY);
			} else {
				wd179x_timed_write_track_request(chip);
			}
			return;
		}


		if (FDC_READ_DAM == (data & ~FDC_MASK_TYPE_3)) {
			LOG((LL,"WD179x","#%x command_w %02x READ_DAM\n",
				w->drive, data));
			w->command_type = TYPE_3;
			w->status &= ~STA_2_LOST_DAT;
  			wd179x_clear_data_request(chip);

			if (0 == img_get_flag(img, DRV_READY)) {
				wd179x_complete_command(chip, DELAY_NOTREADY);
			} else {
				wd179x_read_id(chip);
			}
			return;
		}

		if (FDC_DEN_SD == data) {
			LOG((LL,"WD179x","#%x command_w %02x SET_SD\n",
				w->drive, data));
			wd179x_set_density(chip, DEN_FM_LO);
			return;
		}

		if (FDC_DEN_DD == data) {
			LOG((LL,"WD179x","#%x command_w %02x SET_DD\n",
				w->drive, data));
			wd179x_set_density(chip, DEN_MFM_LO);
			return;
		}

		LOG((LL,"WD179x","#%x command_w %02x unknown\n",
			w->drive, data));
		return;
	}

	w->status |= STA_1_BUSY;
	
	/* clear CRC error */
	w->status &=~STA_1_CRC_ERR;

	if (FDC_RESTORE == (data & ~FDC_MASK_TYPE_1)) {
		LOG((LL,"WD179x","#%x command_w %02x RESTORE\n",
			w->drive, data));
		wd179x_restore(chip, data);
	}

	if (FDC_SEEK == (data & ~FDC_MASK_TYPE_1)) {
		w->track_new = w->data;
		/* setup step direction */
		if (w->track_reg < w->track_new) {
			LOG((LL,"WD179x","#%x direction: +1\n",
				w->drive));
			w->direction = 1;
		} else if (w->track_reg > w->track_new) {
			LOG((LL,"WD179x","#%x direction: -1\n",
				w->drive));
			w->direction = -1;
		}

		LOG((LL,"WD179x","#%x command_w %02x SEEK (%02x->%02x, %+d)\n",
			w->drive, data,
			w->track_reg, w->track_new, w->direction));
		w->command_type = TYPE_1;

		wd179x_timed_seek(chip, (data & FDC_STEP_RATE) + 1);
	}

	if (FDC_STEP == (data & ~(FDC_STEP_UPDATE | FDC_MASK_TYPE_1))) {
		LOG((LL,"WD179x","#%x command_w %02x STEP dir %+d\n",
			w->drive, data, w->direction));
		w->command_type = TYPE_1;

		fdd_seek(img, w->direction);

		if (0 != (data & FDC_STEP_UPDATE))
			w->track_reg += w->direction;

		wd179x_set_busy(chip, 0.1);
	}

	if (FDC_STEP_IN == (data & ~(FDC_STEP_UPDATE | FDC_MASK_TYPE_1))) {
		LOG((LL,"WD179x","#%x command_w %02x STEP_IN\n",
			w->drive, data));
		w->command_type = TYPE_1;
		w->direction = +1;

		fdd_seek(img, w->direction);

		if (0 != (data & FDC_STEP_UPDATE))
			w->track_reg += w->direction;
		wd179x_set_busy(chip, 0.1);

	}

	if (FDC_STEP_OUT == (data & ~(FDC_STEP_UPDATE | FDC_MASK_TYPE_1))) {
		LOG((LL,"WD179x","#%x command_w %02x STEP_OUT\n",
			w->drive, data));
		w->command_type = TYPE_1;
		w->direction = -1;

		fdd_seek(img, w->direction);

		if (0 != (data & FDC_STEP_UPDATE))
			w->track_reg += w->direction;
		wd179x_set_busy(chip, 0.1);
	}

	/* 0 is enable spin up sequence, 1 is disable spin up sequence */
	if (0 == (data & FDC_STEP_HDLOAD))
		w->status |= STA_1_HD_LOADED;

	if (0 != (data & FDC_STEP_VERIFY)) {
		/* verify seek */
		wd179x_verify_seek(chip);
	}
}



/* write the FDC track register */
void wd179x_track_w(uint32_t chip, uint32_t offset, uint32_t data)
{
	chip_wd179x_t *w = &chips[chip];

	if (chip >= num_chips)
		return;

	LOG((LL,"WD179x","#%x track_w %02x\n",
		w->drive, data));
	w->track_reg = data;
}



/* write the FDC sector register */
void wd179x_sector_w(uint32_t chip, uint32_t offset, uint32_t data)
{
	chip_wd179x_t *w = &chips[chip];

	if (chip >= num_chips)
		return;
	LOG((LL,"WD179x","#%x sector_w %02x\n",
		w->drive, data));
	w->sector = data;
}



/* write the FDC data register */
void wd179x_data_w(uint32_t chip, uint32_t offset, uint32_t data)
{
	chip_wd179x_t *w =  &chips[chip];

	if (chip >= num_chips)
		return;

	w->data = data;

	if (0 == w->data_count) {
		LOG((LL,"WD179x","#%x data_w %02x (not expecting data)\n",
			w->drive, data));
		return;
	}

	/* clear data request */
	wd179x_clear_data_request(chip);

	/* put byte into buffer */
	LOG((LD,"WD179x","#%x data_w %02x\n",
		w->drive, data));
	
	w->buffer[w->data_offset++] = data;
	w->data_count--;
		
	if (w->data_count > 0) {
		wd179x_set_data_request(chip);
		return;
	}

	{
		char *hexdump = malloc(w->data_offset / 16 * 128);
		char *dst = hexdump;
		uint32_t i;
		for (i = 0; i < w->data_offset; i++) {
			if (0 == (i % 16))
				dst += sprintf(dst, "%04x:", i);
			dst += sprintf(dst, " %02x", w->buffer[i]);
			if (15 == (i % 16))
				dst += sprintf(dst, "\n");
		}
		if (0 != (i % 16))
			dst += sprintf(dst, "\n");
		LOG((1,"WD179x","Data buffer:\n%s\n", hexdump));
		free(hexdump);
	}

	switch (w->command) {
	case FDC_WRITE_TRK:
		wd179x_write_track(chip);
		break;

	case FDC_WRITE_SEC:
		wd179x_write_sector(chip);
		break;
	default:
		LOG((3,"WD179x","#%x w->command %02x ** NOT HANDLED! **\n",
			w->drive, w->command));
	}

	w->data_offset = 0;
	wd179x_complete_command(chip, DELAY_DATADONE);
}


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

uint8_t wd179x_0_r(uint32_t offset)
{
	uint8_t data = 0;

	switch(offset & 3) {
	case 0: 
		data = wd179x_status_r(0, 0);
		break;
	case 1: 
		data = wd179x_track_r(0, 1);
		break;
	case 2: 
		data = wd179x_sector_r(0, 2);
		break;
	case 3: 
		data = wd179x_data_r(0, 3);
		break;
	}
	return data;
}


void wd179x_0_w(uint32_t offset, uint8_t data)
{
	switch(offset & 3) {
	case 0:
		wd179x_command_w(0, 0, data);
		break;
	case 1:
		wd179x_track_w(0, 1, data);
		break;
	case 2:
		wd179x_sector_w(0, 2, data);
		break;
	case 3:
		wd179x_data_w(0, 3, data);
		break;
	}
}


uint8_t wd179x_1_r(uint32_t offset)
{
	uint8_t data = 0;

	switch(offset & 3) {
	case 0: 
		data = wd179x_status_r(1, 0);
		break;
	case 1: 
		data = wd179x_track_r(1, 1);
		break;
	case 2: 
		data = wd179x_sector_r(1, 2);
		break;
	case 3: 
		data = wd179x_data_r(1, 3);
		break;
	}
	return data;
}



void wd179x_1_w(uint32_t offset, uint8_t data)
{
	switch(offset & 3) {
	case 0:
		wd179x_command_w(1, 0, data);
		break;
	case 1:
		wd179x_track_w(1, 1, data);
		break;
	case 2:
		wd179x_sector_w(1, 2, data);
		break;
	case 3:
		wd179x_data_w(1, 3, data);
		break;
	}
}
