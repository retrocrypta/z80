/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * fdc.c	TRS-80 floppy disk controller emulation
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#include "trs80/fdc.h"

#define	FDC_DEBUG	1
#if	FDC_DEBUG
#define	LL	3
#else
#define	LL	7
#endif

#define MAX_LUMPS	192
#define MAX_GRANULES	8
#define MAX_SECTORS	5

#define	DDAM_LENGTH (MAX_LUMPS * MAX_GRANULES * MAX_SECTORS)

#define IRQ_TIMER	0x80
#define IRQ_FDC 	0x40
static uint8_t irq_status = 0;
static int fdc_enabled;

typedef struct pdrive_s {
	uint8_t ddsl;   /* Disk Directory Start Lump (lump number of GAT) */
	uint8_t gatl;   /* Granule Allocation Table Length (in sectors) */
	uint8_t steprt; /* step rate and some SD/DD flag ... */
	uint8_t tracks; /* total number of tracks */
	uint8_t spt;    /* sectors per track (both sides on DS media!) */
	uint8_t gatm;   /* GAT mask (number of meaningful bits per byte in GAT) */
	uint8_t p7;     /* ???? always zero */
	uint8_t flags;  /* ???? some flags (bit 6 = density) */
	uint8_t gpl;    /* Granules Per Lump (5 is the default) */
	uint8_t ddga;   /* Disk Directory Granule Allocation (dir granules) */
	uint8_t pad[6];
}	pdrive_t;

static ifc_wd179x_t trs80_wd179x = {
	{ WD_TYPE_179X },
	{ trs80_fdc_callback }
};

int trs80_fdc_init(void)
{
	struct img_s *img;
	pdrive_t pdrive[4], *pd;
	int tracks;
	int heads;
	int spt;
	int lba;
	uint8_t *ddam;
	uint32_t drive;

	ddam = malloc(DDAM_LENGTH);
	if (NULL == ddam) {
		osd_die("failed to alloc ddam buffer\n");
	}

	/* Initialize the WD179X emulation */
	wd179x_init(1, &trs80_wd179x);

	/* setup drive geometries based upon pdrive info on boot drive (0) */ 
	for (drive = 0; drive < 4; drive++) {
		img = img_file(IMG_TYPE_FD, drive);
		if (NULL == img)
			continue;
		if (0 == img_get_flag(img, IMG_EXISTS))
			continue;
		/* see if the floppy code can determine the image format */
		if (0 == fdd_find_format(img))
			continue;
		img_set_geometry(img, 80, 2, 20, 256, 0, DDAM_LENGTH, 0);
		if (0 == drive) {
			img_fseek(img, DDAM_LENGTH + 2 * 256 + drive * 16, SEEK_SET);
			img_fread(img, &pdrive[0], sizeof(pdrive_t));
			img_fread(img, &pdrive[1], sizeof(pdrive_t));
			img_fread(img, &pdrive[2], sizeof(pdrive_t));
			img_fread(img, &pdrive[3], sizeof(pdrive_t));
		}
		pd = &pdrive[drive];
		tracks = pd->tracks;
		heads = pd->spt > 18 ? 2 : 1;
		spt = pd->spt / heads;
		img_set_geometry(img, tracks, heads, spt, 256, 0, DDAM_LENGTH, 0);
		LOG((LL,"TRS80","drive #%d geometry: %d tracks, %d heads, %d spt\n",
			drive, tracks, heads, spt));
		img_fseek(img, 0, SEEK_SET);
		img_fread(img, ddam, DDAM_LENGTH);
		for (lba = 0; lba < tracks * heads * spt; lba++)
			fdd_set_ddam(img, lba, ddam[lba]);
	}

	free(ddam);
	fdc_enabled = 1;
	return 0;
}

void trs80_fdc_reset(void)
{
	/* Reset the WD179X emulation */
	wd179x_reset(0);
	fdc_enabled = 1;
	irq_status = 0;
}

void trs80_fdc_stop(void)
{
	struct img_s *img;
	uint32_t drive;

	for (drive = 0; drive < 4; drive++) {
		img = img_file(IMG_TYPE_FD, drive);
		if (NULL == img)
			continue;
		if (0 == img_get_flag(img, IMG_EXISTS))
			continue;
		img_fclose(img);
	}

	/* Stop the WD179X emulation */
	wd179x_stop();
	fdc_enabled = 0;
}

uint8_t trs80_irq_status_r(uint32_t offset)
{
	uint8_t data = irq_status;
	irq_status &= ~(IRQ_TIMER | IRQ_FDC);

	LOG((LL,"TRS80","irq_status_r 0x%02X\n", data));
	return data;
}

void trs80_timer_interrupt(void)
{
	irq_status |= IRQ_TIMER;
	z80_interrupt(&z80, 2);
}

void trs80_fdc_interrupt(void)
{
	irq_status |= IRQ_FDC;
	z80_interrupt(&z80, 2);
}

void trs80_fdc_callback(uint32_t event)
{
	/* If disc hardware is not enabled, do not cause an interrupt */
	if (0 == fdc_enabled)
		return;

	switch (event) {
	case WD179X_IRQ_CLR:
		irq_status &= ~IRQ_FDC;
		break;
	case WD179X_IRQ_SET:
		trs80_fdc_interrupt();
		break;
	}
}

void trs80_motors_w(uint32_t offset, uint8_t data)
{
	uint8_t drive = 255;
	uint8_t head = 0;

	LOG((LL,"TRS80","motor_w 0x%02X\n", data));

	switch (data) {
	case 1:
		drive = 0;
		break;
	case 2:
		drive = 1;
		break;
	case 4:
		drive = 2;
		break;
	case 8:
		drive = 3;
		break;
	case 1+8:
		drive = 0;
		head = 1;
		break;
	case 2+8:
		drive = 1;
		head = 1;
		break;
	case 4+8:
		drive = 2;
		head = 1;
		break;
	}

	if (drive > 3)
		return;

	wd179x_set_drive(0, drive);
	wd179x_set_side(0, head);
}
