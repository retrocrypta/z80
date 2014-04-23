/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * fdc.c	Colour Genie floppy disk controller emulation
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#include "cgenie/fdc.h"

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
	uint8_t tracks;	/* total number of cylinders (tracks is used as synonymous) */
	uint8_t spt;    /* sectors per track (both sides on DS media!) */
	uint8_t gatm;   /* GAT mask (number of meaningful bits per byte in GAT) */
	uint8_t p7;     /* ???? always zero */
	uint8_t flags;  /* ???? some flags (bit 6 = density) */
	uint8_t gpl;    /* Granules Per Lump (5 is the default) */
	uint8_t ddga;   /* Disk Directory Granule Allocation (dir granules) */
	uint8_t pad[6];
}	pdrive_t;

/* Standard drive formats, taken from the Coulour Genie DOS ROM */
static pdrive_t pdrive_list[] = {
	/* CMD"<0=A" 40 cylinders, SS, SD */
	{20, 40, 0x07, 40, 10, 2, 0, 0x00, 5, 2},

	/* CMD"<0=B" 40 cylinders, DS, SD */
	{20, 40, 0x07, 40, 20, 4, 0, 0x40, 5, 4},

	/* CMD"<0=C" 40 cylinders, SS, DD */
	{24, 48, 0x53, 40, 18, 3, 0, 0x03, 5, 3},

	/* CMD"<0=D" 40 cylinders, DS, DD */
	{24, 48, 0x53, 40, 36, 6, 0, 0x43, 5, 6},

	/* CMD"<0=E" 40 cylinders, SS, SD */
	{20, 40, 0x07, 40, 10, 2, 0, 0x04, 5, 2},

	/* CMD"<0=F" 40 cylinders, DS, SD */
	{20, 40, 0x07, 40, 20, 4, 0, 0x44, 5, 4},

	/* CMD"<0=G" 40 cylinders, SS, DD */
	{24, 48, 0x53, 40, 18, 3, 0, 0x07, 5, 3},

	/* CMD"<0=H" 40 cylinders, DS, DD */
	{24, 48, 0x53, 40, 36, 6, 0, 0x47, 5, 6},

	/* CMD"<0=I" 80 cylinders, SS, SD */
	{40, 80, 0x07, 80, 10, 2, 0, 0x00, 5, 2},

	/* CMD"<0=J" 80 cylinders, DS, SD */
	{40, 80, 0x07, 80, 20, 4, 0, 0x40, 5, 4},

	/* CMD"<0=K" 80 cylinders, SS, DD */
	{48, 96, 0x53, 80, 18, 3, 0, 0x03, 5, 3},

	/* CMD"<0=L" 80 cylinders, DS, DD */
	{48, 96, 0x53, 80, 36, 6, 0, 0x43, 5, 6},

	/* FAKE! CMD"<0=x" 213 cylinders, DS, DD */
	{17, 192, 0x53, 213, 36, 8, 0, 0x43, 5, 8},

	/* end-of-list */
	{0,  0,   0x00, 0,   0,  0, 0, 0x00, 0, 0}
};

static ifc_wd179x_t cgenie_wd179x = {
	{ WD_TYPE_179X },
	{ cgenie_fdc_callback }
};


#define	PDRIVE_BASE	0x5a71

static int init_drive(uint32_t drive)
{
	int i, j, k, found, format;
	pdrive_t *pd;
	const pdrive_t *ps;
	struct img_s *img;
	fdd_chrn_id_t id;
	uint8_t buff[16];
	uint8_t cylinders, sides, spt, t, h, s;
	uint32_t den = DEN_FM_LO;
	uint32_t dir_sector = 0;
	uint32_t dir_length = 0;

	LOG((LL,"CGENIE","checking drive #%d\n", drive));
	img = img_file(IMG_TYPE_FD, drive);

	/* A floppy isn't manditory, so continue if none */
	if (0 == img_get_flag(img, IMG_EXISTS))
		return -1;

	format = 0;
	if (0 == fdd_find_format(img)) {
		cylinders = img_get_flag(img, DRV_TOTAL_CYLINDERS);
		sides = img_get_flag(img, DRV_TOTAL_HEADS);
		spt = img_get_flag(img, DRV_SECTORS_PER_TRACK);
		LOG((LL,"CGENIE","#%x found format %02x/%x/%02x\n",
			drive, cylinders, sides, spt));
		format = 1;
	}

	/* determine geometry from disk contents */
	for (ps = pdrive_list, found = -1, i = 0; ps->ddsl; ps++, i++) {

		/* Setup a geometry so we can access the image */
		cylinders = ps->tracks;
		sides = (ps->spt == 20 || ps->spt == 36) ? 2 : 1;
		spt = ps->spt / sides;
		den = (ps->steprt & 0x40) ? DEN_MFM_LO : DEN_FM_LO;
		img_set_geometry(img, cylinders, sides, spt, 256, 0, 0, 0);

		img_set_flag(img, DRV_CURRENT_CYLINDER, 1);
		if (0 != fdd_read_sector_data(img, 0, 0, buff, 16, &id, den))
			continue;

		LOG((LL,"CGENIE","#%x buff: %02x %02x %02x %02x\n",
			drive, buff[0], buff[1], buff[2], buff[3]));
		/* find an entry with matching DDSL */
		if (memcmp(buff, "\x00\xfe", 2) || buff[2] != ps->ddsl)
			continue;
		LOG((LL,"CGENIE","#%x checking format CMD\"<%x=%c\"\n",
			drive, drive, 'A' + i));

		dir_sector = ps->ddsl * ps->gatm * ps->gpl + ps->spt;
		dir_length = ps->ddga * ps->gpl;

		/*
		 * Scan the directory for DIR/SYS or NCW1983/JHL files.
		 * Look into sectors 2 and 3 first entry relative
		 * to the DDSL (disk directory start lump).
		 */
		for (j = 2; j < 4; j++) {
			char *name = (char *)buff + 5;

			k = dir_sector + j;
			s = k % spt;
			h = (k / spt) % sides;
			t = k / spt / sides;
			LOG((LL,"CGENIE","#%x LBA:%04x T:%02x H:%x S:%x\n",
				drive, k, t, h, s));
			img_set_flag(img, DRV_CURRENT_CYLINDER, t);
			if (0 != fdd_read_sector_data(img, h, s, buff, 16, &id, den))
				break;
			LOG((LL,"CGENIE","#%x name: '%-11.11s'\n",
				drive, name));
			if (0 == memcmp(name, "DIR     SYS", 11)) {
				found = i;
				break;
			}
			if (0 == memcmp(name, "NCW1983 JHL", 11)) {
				found = i;
				break;
			}
		}
		if (found >= 0 && format == 0)
			break;
	}
	img_set_flag(img, DRV_CURRENT_CYLINDER, 0);
	fdd_read_sector_data(img, 0, 0, buff, 16, &id, den);
	if (found < 0) {
		LOG((LL,"CGENIE","#%x geometry is unknown\n",
			drive));
		return -1;
	}

	ps = &pdrive_list[found];
	cylinders = ps->tracks;
	sides = (ps->flags & 0x40) ? 2 : 1;
	spt = ps->spt / sides;
	den = (ps->steprt & 0x40) ? DEN_MFM_LO : DEN_FM_LO;
	dir_sector = ps->ddsl * ps->gatm * ps->gpl + ps->spt;
	dir_length = ps->ddga * ps->gpl;

	LOG((1,"CGENIE","#%x geometry is CMD\"<%c\"\n",
		drive, 'A'+found));

	/* XXX: patch mem to contain useful pdrive parameters */
	pd = (pdrive_t *)&mem[PDRIVE_BASE + drive * 10];
	pd->ddsl = ps->ddsl;
	pd->gatl = ps->gatl;
	pd->steprt = ps->steprt;
	pd->tracks = ps->tracks;
	pd->spt = ps->spt;	
	pd->gatm = ps->gatm;
	pd->p7 = ps->p7;
	pd->flags = ps->flags;
	pd->gpl = ps->gpl;	
	pd->ddga = ps->ddga;
	/* set geometry to what we found */
	img_set_geometry(img, cylinders, sides, spt, 256, 0, 0, 0);

	LOG((LL,"CGENIE","#%x dir sectors %d +%d sectors\n",
		drive, dir_sector, dir_length));
	/*
	 * Mark directory sectors with deleted data address mark
	 * assumption: dir_sector is a sector offset
	 */
	for (i = 0; i < dir_length; i++)
		fdd_set_ddam(img, dir_sector + i, 1);

	return 0;
}

int cgenie_fdc_init(void)
{
	pdrive_t *pd;
	uint32_t drive;

	/* Initialize the WD179X emulation */
	if (0 == fdc_enabled)
		wd179x_init(1, &cgenie_wd179x);

	for (drive = 0; drive < 4; drive++)
		init_drive(drive);
	for (drive = 0; drive < 4; drive++) {
		pd = (pdrive_t *)&mem[PDRIVE_BASE + drive * 10];
		LOG((LL,"CGENIE","pd%x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			drive,
			pd->ddsl, pd->gatl, pd->steprt, pd->tracks, pd->spt,
			pd->gatm, pd->p7, pd->flags, pd->gpl, pd->ddga));
	}

	fdc_enabled += 1;
	return 0;
}

void cgenie_fdc_reset(void)
{
	/* Reset the WD179X emulation */
	wd179x_reset(0);
	fdc_enabled = 1;
	irq_status = 0;
}

void cgenie_fdc_stop(void)
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

uint8_t cgenie_irq_status_r(uint32_t offset)
{
	uint8_t data = irq_status;
	irq_status &= ~(IRQ_TIMER | IRQ_FDC);
	LOG((LL,"CGFDC","irq_status_r 0x%02X\n", data));
	return data;
}

void cgenie_timer_interrupt(void)
{
	irq_status |= IRQ_TIMER;
	z80_interrupt(&z80, 2);
}

void cgenie_fdc_interrupt(void)
{
	irq_status |= IRQ_FDC;
	z80_interrupt(&z80, 2);
}

void cgenie_fdc_callback(uint32_t event)
{
	/* If disc hardware is not enabled, do not cause an interrupt */
	if (0 == fdc_enabled)
		return;

	switch (event) {
	case WD179X_IRQ_CLR:
		irq_status &= ~IRQ_FDC;
		break;
	case WD179X_IRQ_SET:
		cgenie_fdc_interrupt();
		break;
	}
}

void cgenie_motors_w(uint32_t offset, uint8_t data)
{
	uint8_t drive = 255;
	uint8_t head = 0;

	if (fdc_enabled < 2) {
		LOG((LL,"CGENIE","re-init PDRIVE params\n"));
		cgenie_fdc_init();
	}

	LOG((LL,"CGENIE","motor_w 0x%02X\n", data));

	if (data & 1)
		drive = 0;
	if (data & 2)
		drive = 1;
	if (data & 4)
		drive = 2;
	if (data & 8)
		drive = 3;

	if (drive > 3)
		return;

	/* mask head select bit */
	head = (data >> 4) & 1;

	wd179x_set_drive(0, drive);
	wd179x_set_side(0, head);
}
