/* ed:set tabstop=8 noexpandtab: */
/**************************************************************************
 *
 * floppy.c	Floppy disk drive emulation
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 **************************************************************************/
#include "floppy.h"

/**************************************************************************
 *
 * Terms used throughout this file:
 *
 * Cylinder	- one position of a drive's head
 * Track	- one head's data on one cylinder
 * Head		- head number (e.g. side 0 or 1 for a double sided floppy)
 * Sector	- data range on a track
 *
 * A disk (image) is made up of one or more cylinders
 * A cylinder is made up of one or more tracks
 * A track is made up of one or more sectors
 *
 **************************************************************************/

#if	1
#define	P(x)	printf x
#else
#define	P(x)
#endif

#define	DMK_CACHE_TRACK	0

static uint32_t sec_len[4] = {0x080, 0x100, 0x200, 0x400};

/*
 * Use with img_get/set_data for the type of data associated with image.
 */
typedef enum {
	FDD_DATA_JV3,
	FDD_DATA_DMK,
	FDD_DATA_DDAM
}	IMG_DATA;

/**************************************************************************
 *
 *	JV3 image format
 *
 **************************************************************************/
#define	JV3_SECTORS	2901
#define	JV3_HEADER_SIZE	(3*JV3_SECTORS+1)

#define JV3_DENSITY	(1<<7)
#define JV3_DAM_SHIFT	5
#define JV3_DAM_MASK	(3<<JV3_DAM_SHIFT)
#define JV3_DAM_DD_STD	(0<<JV3_DAM_SHIFT)
#define JV3_DAM_DD_DEL	(1<<JV3_DAM_SHIFT)
#define JV3_DAM_SD_STD	(2<<JV3_DAM_SHIFT)
#define JV3_DAM_SD_DEL	(3<<JV3_DAM_SHIFT)
#define JV3_SIDE	(1<<4)
#define JV3_ERROR	(1<<3)
#define JV3_NONIBM	(1<<2)
#define JV3_SIZE_SHIFT	0
#define JV3_SIZE_MASK	(3<<JV3_SIZE_SHIFT)
#define JV3_SIZE_256	(0<<JV3_SIZE_SHIFT)
#define JV3_SIZE_128	(1<<JV3_SIZE_SHIFT)
#define JV3_SIZE_1024	(2<<JV3_SIZE_SHIFT)
#define JV3_SIZE_512	(3<<JV3_SIZE_SHIFT)

typedef struct jv3_header_s {
	uint8_t table[3*JV3_SECTORS];
	uint8_t wrprot;
}	jv3_header_t;

typedef struct jv3_dam_s {
	size_t size;		/* size, converted from flag length code */
	off_t offs;		/* offset in file image */
	off_t oflg;		/* offset of flag in header */
	uint8_t c;		/* cylinder number */
	uint8_t h;		/* head number */
	uint8_t r;		/* record number (sector) */
	uint8_t n;		/* length code and flags(!) */
}	jv3_dam_t;

typedef struct jv3_track_s {
	uint32_t sectors;	/* sectors in this track */
	jv3_dam_t sector[64];	/* sector info */
}	jv3_track_t;

#define	JV3_MAX_HEADERS	4
typedef struct jv3_s {
	uint32_t headers;
	uint32_t wrprot;
	uint32_t cylinders;
	uint32_t heads;
	jv3_header_t hdr[JV3_MAX_HEADERS];
	jv3_track_t track[256][2];
}	jv3_t;

static int jv3_setup(struct img_s *img, jv3_t **pjv3)
{
	jv3_t *jv3;
	uint32_t drive;
	uint32_t cyl, sec, head, flg;
	uint32_t cyl_min, cyl_max;
	uint32_t sec_min, sec_max;
	size_t len_min, len_max;
	off_t offs;
	size_t size;
	int i, valid = 0;

	/* see if we already have a dmk struct */
	img_get_data(img, FDD_DATA_JV3, (void *)&jv3);
	if (NULL != jv3) {
		*pjv3 = jv3;
		return 0;
	}

	drive = img_minor(img);

	/* allocate a new dmk struct */
	jv3 = malloc(sizeof(jv3_t));
	if (NULL == jv3)
		return -1;

	memset(jv3, 0, sizeof(*jv3));

	/* read existing header, if any */
	if (JV3_HEADER_SIZE != img_read(img, 0, jv3->hdr, JV3_HEADER_SIZE)) {
		valid = 0;
	} else {
		/* got existing JV3 header */
		cyl_min = 0xff; cyl_max = 0x00;
		sec_min = 0xff; sec_max = 0x00;
		len_min = 0xffff; len_max = 0x0000;
		jv3->heads = 1;
		offs = JV3_HEADER_SIZE;
		for (i = 0; i < JV3_SECTORS; i++) {
			jv3_track_t *tp;
			jv3_dam_t *dp;

			cyl = jv3->hdr[0].table[3*i+0];
			sec = jv3->hdr[0].table[3*i+1];
			flg = jv3->hdr[0].table[3*i+2];
			if (cyl == 0xff)
				break;

			switch (flg & JV3_SIZE_MASK) {
			case JV3_SIZE_256:
				size = 256;
				break;
			case JV3_SIZE_128:
				size = 128;
				break;
			case JV3_SIZE_1024:
				size = 1024;
				break;
			default:
				size = 512;
				break;
			}

			if (cyl < cyl_min)
				cyl_min = cyl;
			if (cyl > cyl_max)
				cyl_max = cyl;
			if (sec < sec_min)
				sec_min = sec;
			if (sec > sec_max)
				sec_max = sec;
			if (size < len_min)
				len_min = size;
			if (size > len_max)
				len_max = size;
			if (flg & JV3_SIDE) {
				head = 1;
				jv3->heads = 2;
			} else {
				head = 0;
			}

			/* track info pointer */
			tp = &jv3->track[cyl][head];

			if (cyl + 1 > jv3->cylinders)
				jv3->cylinders = cyl + 1;

			/* sector info pointer */
			dp = &tp->sector[tp->sectors];

			/* increment sectors in this track */
			tp->sectors += 1;

			dp->size = size;
			dp->offs = offs;
			dp->oflg = 3 * i + 2;
			/* set data address mark status */
			dp->c = cyl;
			dp->h = (flg & JV3_SIDE) ? 1 : 0;
			dp->r = sec;
			dp->n = flg ^ 1;	/* toggle bit 0 */

			LOG((1,"JV3","#%x setup C:%02x H:%x R:%02x N:%02x @ 0x%x\n",
				drive, dp->c, dp->h, dp->r, dp->n, (uint32_t)offs));
			offs += size;
		}
	}

	if (0 == valid) {
		/* create a new JV3 header with no sectors */
		memset(jv3->hdr, 0xff, sizeof(jv3->hdr));
		jv3->wrprot = 0;
	}

	/* set the image data to our jv3 struct */
	if (0 != img_set_data(img, FDD_DATA_JV3, jv3)) {
		free(jv3);
		return -1;
	}
	*pjv3 = jv3;

	img_set_flag(img, DRV_TOTAL_HEADS, jv3->heads);
	img_set_flag(img, DRV_TOTAL_CYLINDERS, jv3->cylinders);
	img_set_flag(img, IMG_FORMAT, FDD_FORMAT_JV3);

	return 0;
}

static int jv3_set_geometry(struct img_s *img)
{
	jv3_t *jv3;

	if (0 != jv3_setup(img, &jv3))
		return -1;

	/* TODO: mark deleted data address mark sectors? */

	return 0;
}

/**
 * @brief jv3_get_next_id - get the next sector ID from a spinning floppy
 *
 * This function reads the next sector ID (i.e. address mark) from a
 * "spinning" floppy in the JV3 format.
 */
static int jv3_get_next_id(struct img_s *img, uint32_t head, fdd_chrn_id_t *id,
	uint32_t den)
{
	jv3_t *jv3; 
	jv3_track_t *tp;
	jv3_dam_t *dp;
	uint32_t drive, cyl, i, idx;

	if (0 != jv3_setup(img, &jv3))
		return -1;

	drive = img_minor(img);
	cyl = img_get_flag(img, DRV_CURRENT_CYLINDER);
	i = img_get_flag(img, DRV_ID_INDEX);

	if (cyl >= jv3->cylinders)
		return -1;
	if (head >= jv3->heads)
		return -1;

	tp = &jv3->track[cyl][head];

	idx = 0;
	i += 1;
	if (i >= tp->sectors) {
		idx = 1;
		i = 0;
	}
	img_set_flag(img, DRV_ID_INDEX, i);
	img_set_flag(img, DRV_INDEX, idx);
	dp = &tp->sector[i];

	id->C = dp->c;
	id->H = dp->h;
	id->R = dp->r;
	id->N = dp->n & 0x03;
	id->data_id = id->R;
	id->flags = 0;

	switch (dp->n & JV3_DAM_MASK) {
	case JV3_DAM_SD_STD:
	case JV3_DAM_DD_STD:
		id->flags &= ~ID_FLAG_DELETED_DATA;
		break;
	case JV3_DAM_SD_DEL:
	case JV3_DAM_DD_DEL:
		id->flags |= ID_FLAG_DELETED_DATA;
		break;
	}

	LOG((1,"JV3","#%x/%cD next id C:%02x H:%02x R:%02x N:%02x SPT:%02x%s\n",
		drive, DEN_FM_LO == den ? 'S' : 'D',
		id->C, id->H, id->R, id->N, tp->sectors,
		id->flags & ID_FLAG_DELETED_DATA ? " DDAM" : ""));

	return 0;
}

/**
 * @brief jv3_read_sector - JV3 format read sector function
 *
 * This assumes the JV3 image type.
 */
static int jv3_read_sector(struct img_s *img, uint32_t head, uint32_t sec,
	void *buff, size_t size, fdd_chrn_id_t *id, uint32_t den)
{
	jv3_t *jv3;
	jv3_track_t *tp;
	jv3_dam_t *dp;
	uint32_t drive, cyl;
	off_t offs;
	uint32_t i;

	if (0 != jv3_setup(img, &jv3))
		return -1;

	drive = img_minor(img);
	cyl = img_get_flag(img, DRV_CURRENT_CYLINDER);

	if (cyl >= jv3->cylinders)
		return -1;
	if (head >= jv3->heads)
		return -1;

	tp = &jv3->track[cyl][head];
	for (i = 0, dp = tp->sector; i < tp->sectors; i++, dp++)
		if (dp->r == sec)
			break;

	/* record not found? */
	if (i >= tp->sectors) {
		dp = &tp->sector[0];
		LOG((1,"JV3","#%x read sector %02x not found SPT:%02x\n",
			drive, sec, tp->sectors));
		id->C = dp->c;
		id->H = dp->h;
		id->R = dp->r;
		id->N = dp->n & 0x03;
		id->data_id = dp->r;
		id->flags = 0;
		return -1;
	}

	id->C = dp->c;
	id->H = dp->h;
	id->R = dp->r;
	id->N = dp->n & 0x03;
	id->data_id = dp->r;
	id->flags = 0;

	LOG((1,"JV3","#%x/%cD read C:%02x H:%x R:%02x N:%02x SPT:%02x\n",
		drive, DEN_FM_LO == den ? 'S' : 'D',
		id->C, id->H, id->R, id->N, tp->sectors));

	switch (dp->n & JV3_DAM_MASK) {
	case JV3_DAM_SD_STD:
	case JV3_DAM_DD_STD:
		id->flags &= ~ID_FLAG_DELETED_DATA;
		break;
	case JV3_DAM_SD_DEL:
	case JV3_DAM_DD_DEL:
		id->flags |= ID_FLAG_DELETED_DATA;
		break;
	}

	offs = dp->offs;

	if (size <= dp->size) {
		if (size == img_read(img, offs, buff, size))
			return 0;
	} else {
		memset(buff, 0xff, size);
		if (size == img_read(img, offs, buff, dp->size))
			return 0;
	}

	return -1;
}

/**
 * @brief jv3_write_sector - JV3 format write sector function
 *
 * This assumes the JV3 image type.
 */
static int jv3_write_sector(struct img_s *img, uint32_t head, uint32_t sec,
	void *buff, size_t size, uint32_t den, uint32_t ddam)
{
	jv3_t *jv3;
	jv3_track_t *tp;
	jv3_dam_t *dp;
	uint32_t drive, cyl;
	off_t offs;
	uint8_t n[1];
	uint32_t i;

	if (0 != jv3_setup(img, &jv3))
		return -1;

	drive = img_minor(img);
	cyl = img_get_flag(img, DRV_CURRENT_CYLINDER);

	if (cyl >= jv3->cylinders)
		return -1;
	if (head >= jv3->heads)
		return -1;

	tp = &jv3->track[cyl][head];
	for (i = 0; i < tp->sectors; i++)
		if (tp->sector[i].r == sec)
			break;
	/* record not found? */
	if (i >= tp->sectors) {
		LOG((1,"JV3","#%x/%cD write sector %02x not found SPT:%02x\n",
			drive, DEN_FM_LO == den ? 'S' : 'D',
			sec, tp->sectors));
		return -1;
	}
	dp = &tp->sector[i];

	LOG((1,"JV3","#%x/%cD write C:%02x H:%x R:%02x N:%02x SPT:%02x\n",
		drive, DEN_FM_LO == den ? 'S' : 'D',
		dp->c, dp->h, dp->r, dp->n & 0x03, tp->sectors));

	dp->n &= ~JV3_DAM_MASK;
	switch (den) {
	case DEN_FM_LO:
		if (ddam)
			dp->n |= JV3_DAM_SD_DEL;
		else
			dp->n |= JV3_DAM_SD_STD;
		break;
	case DEN_MFM_LO:
		if (ddam)
			dp->n |= JV3_DAM_DD_DEL;
		else
			dp->n |= JV3_DAM_DD_STD;
		break;
	}
	/* write the (new) flag for this sector */
	n[0] = dp->n ^ 1;
	img_write(img, dp->oflg, n, 1);

	offs = dp->offs;

	if (size <= dp->size) {
		if (size == img_write(img, offs, buff, size))
			return 0;
	} else {
		if (size == img_write(img, offs, buff, dp->size))
			return 0;
	}

	return -1;
}

/**
 * @brief jv3_read_track - JV3 format read an entire track
 *
 * This assumes the JV3 image type.
 */
static int jv3_read_track(struct img_s *img, uint32_t cyl, uint32_t head,
	void *buff, size_t size, uint32_t den)
{
	jv3_t *jv3;
	jv3_track_t *tp;
	jv3_dam_t *dp;
	uint8_t *dst;
	uint32_t crc = 0xffff;
	size_t left, count;
	uint32_t i;
	int state;

	if (0 != jv3_setup(img, &jv3))
		return -1;

	if (cyl >= jv3->cylinders)
		return -1;
	if (head >= jv3->heads)
		return -1;

	state = 0;
	count = 11;
	i = 0;
	dst = buff;
	left = size;
	tp = &jv3->track[cyl][head];
	dp = tp->sector;
	if (den == DEN_FM_LO) {
		/* fill in a single density track */
		while (left-- > 0) {
			switch (state) {
			case 0:		/* pre AM gap 0xff */
				*dst++ = 0xff;
				if (--count == 0) {
					count = 8;
					state++;
				}
				break;
			case 1:		/* pre AM gap 0x00 */
				*dst++ = 0x00;
				if (--count == 0)
					state++;
				break;
			case 2:		/* AM (address mark) */
				crc = 0xffff;
				crc = calc_crc(crc, 0xfe);
				*dst++ = 0xfe;
				state++;
				break;
			case 3: 	/* cylinder number */
				crc = calc_crc(crc, dp->c);
				*dst++ = dp->c;
				state++;
				break;
			case 4: 	/* head number */
				crc = calc_crc(crc, dp->h);
				*dst++ = dp->h;
				state++;
				break;
			case 5: 	/* record number */
				crc = calc_crc(crc, dp->r);
				*dst++ = dp->r;
				state++;
				break;
			case 6: 	/* sector length */
				crc = calc_crc(crc, dp->n & 0x03);
				*dst++ = dp->n & 0x03;
				state++;
				break;
			case 7: 	/* AM CRC high */
				*dst++ = crc / 256;
				state++;
				break;
			case 8: 	/* AM CRC low */
				*dst++ = crc % 256;
				count = 8;
				state++;
				break;
			case 9:		/* pre DAM gap 0xff */
				*dst++ = 0xff;
				if (--count == 0) {
					count = 8;
					state++;
				}
				break;
			case 10:	/* pre DAM gap 0x00 */
				*dst++ = 0x00;
				if (--count == 0) {
					crc = 0xffff;
					state++;
				}
				break;
			case 11:	/* DAM */
				switch (dp->n & JV3_DAM_MASK) {
				case JV3_DAM_SD_STD:
					crc = calc_crc(crc, 0xfb);
					*dst++ = 0xfb;
					break;
				case JV3_DAM_SD_DEL:
					crc = calc_crc(crc, 0xfa);
					*dst++ = 0xfa;
					break;
				case JV3_DAM_DD_DEL:
					crc = calc_crc(crc, 0xf8);
					*dst++ = 0xf8;
					break;
				}
				state++;
				count = 0;
				if (left < dp->size)
					img_read(img, dp->offs, dst, dp->size);
				else
					img_read(img, dp->offs, dst, left);
				break;
			case 12:	/* sector data */
				crc = calc_crc(crc, *dst++);
				if (++count == dp->size)
					state++;
				break;
			case 13:	/* DAM CRC high */
				*dst++ = crc / 256;
				state++;
				break;
			case 14:	/* DAM CRC low */
				*dst++ = crc % 256;
				i++;
				if (i >= tp->sectors) {
					state++;
					break;
				}
				/* next sector */
				count = 11;
				state = 0;
				dp = &tp->sector[i];
				break;
			case 15:	/* pad track with 0xff */
				*dst++ = 0xff;
				break;
			}
		}
	} else {
		/* fill in a double density track */
		while (left-- > 0) {
			switch (state) {
			case 0:		/* pre AM gap 0x4e */
				*dst++ = 0x4e;
				if (--count == 0) {
					count = 8;
					state++;
				}
				break;
			case 1:		/* pre AM gap 0x00 */
				*dst++ = 0x00;
				if (--count == 0) {
					crc = 0xffff;
					count = 3;
					state++;
				}
				break;
			case 2:		/* reset CRC 0xf5 (read as 0xa1) */
				*dst++ = 0xa1;
				crc = calc_crc(crc, 0xa1);
				if (--count == 0)
					state++;
				break;
			case 3:		/* AM (address mark) */
				crc = calc_crc(crc, 0xfe);
				*dst++ = 0xfe;
				state++;
				break;
			case 4: 	/* cylinder number */
				crc = calc_crc(crc, dp->c);
				*dst++ = dp->c;
				state++;
				break;
			case 5: 	/* head number */
				crc = calc_crc(crc, dp->h);
				*dst++ = dp->h;
				state++;
				break;
			case 6: 	/* record number */
				crc = calc_crc(crc, dp->r);
				*dst++ = dp->r;
				state++;
				break;
			case 7: 	/* sector length */
				crc = calc_crc(crc, dp->n & 0x03);
				*dst++ = dp->n & 0x03;
				state++;
				break;
			case 8: 	/* AM CRC high */
				*dst++ = crc / 256;
				state++;
				break;
			case 9: 	/* AM CRC low */
				*dst++ = crc % 256;
				count = 8;
				state++;
				break;
			case 10:	/* pre DAM gap 0x4e */
				*dst++ = 0x4e;
				if (--count == 0) {
					count = 8;
					state++;
				}
				break;
			case 11:	/* pre DAM gap 0x00 */
				*dst++ = 0x00;
				if (--count == 0) {
					crc = 0xffff;
					count = 3;
					state++;
				}
				break;
			case 12:	/* reset CRC 0xf5 (read as 0xa1) */
				*dst++ = 0xa1;
				crc = calc_crc(crc, 0xa1);
				if (--count == 0)
					state++;
				break;
			case 13:	/* DAM */
				switch (dp->n & JV3_DAM_MASK) {
				case JV3_DAM_DD_STD:
					crc = calc_crc(crc, 0xfb);
					*dst++ = 0xfb;
					break;
				case JV3_DAM_SD_DEL:
					crc = calc_crc(crc, 0xf8);
					*dst++ = 0xf8;
					break;
				case JV3_DAM_DD_DEL:
					crc = calc_crc(crc, 0xf8);
					*dst++ = 0xf8;
					break;
				}
				state++;
				count = 0;
				if (left < dp->size)
					img_read(img, dp->offs, dst, left);
				else
					img_read(img, dp->offs, dst, dp->size);
				break;
			case 14:	/* sector data */
				crc = calc_crc(crc, *dst++);
				if (++count == dp->size)
					state++;
				break;
			case 15:	/* DAM CRC high */
				*dst++ = crc / 256;
				state++;
				break;
			case 16:	/* DAM CRC low */
				*dst++ = crc % 256;
				i++;
				if (i >= tp->sectors) {
					state++;
					break;
				}
				/* next sector */
				count = 11;
				state = 0;
				dp = &tp->sector[i];
				break;
			case 17:	/* pad track with 0x4e */
				*dst++ = 0x4e;
				break;
			}
		}
	}
	return 0;
}

/**
 * @brief jv3_write_track - JV3 format write track function
 *
 * This function writes an entire track from the track buffer.
 */
static int jv3_write_track(struct img_s *img, uint32_t cyl, uint32_t head,
	void *buff, size_t size, uint32_t den)
{
	(void)img;
	(void)cyl;
	(void)head;
	(void)buff;
	(void)size;
	return -1;
}

/**************************************************************************
 *
 *	DMK image format
 *
 **************************************************************************/
#define	DMK_TRACK_SIZE		0x1900
#define	DMK_IDAM_DDEN		0x8000
#define	DMK_IDAM_SIZE		0x80

#define	DMK_FLAG_SSIDE		0x10
#define	DMK_FLAG_SD1BYTE	0x40
#define	DMK_FLAG_IGNDEN		0x80

#define	DMK_INC(p,dden) do { \
	if (dden) { \
		p += 1; \
	} else if (dmk->flags & DMK_FLAG_SD1BYTE) { \
		p += 1; \
	} else { \
		p += 2; \
	} \
} while (0)

#define	DMK_DUP(p,dden) do { \
	if (DMK_IDAM_DDEN != dd && 0 == (dmk->flags & DMK_FLAG_SD1BYTE)) \
		dst[p+1] = dst[p]; \
} while (0)

typedef struct dmk_s {
	uint8_t wprot;			/* write protected floppy if 0xff */
	uint8_t cylinders;		/* total number of cylinders */
	uint8_t cyllen[2];		/* track length in LSB,MSB */
	uint8_t flags;			/* flags (single side, SS one byte, ignore density) */
	uint8_t rsvd[7];		/* reserved for future extensions */
	uint8_t realdsk[4];		/* should be 0 for virtual, 0x12345678 for real fdd */
	uint8_t track[DMK_TRACK_SIZE];
}	dmk_t;

#define	DMK_HEADER_SIZE	16

#if	DMK_CACHE_TRACK
/* offsets into dmk.rsvd for local use */
#define	DMK_TRACK_BUFF	0
#define	DMK_SIDE_BUFF	1
#endif

static int dmk_setup(struct img_s *img, dmk_t **pdmk)
{
	dmk_t *dmk;
	uint32_t cyllen, heads;
	int valid = 1;

	/* see if we already have a dmk struct */
	img_get_data(img, FDD_DATA_DMK, (void *)&dmk);
	if (NULL != dmk) {
		*pdmk = dmk;
		return 0;
	}

	/* allocate a new dmk struct */
	dmk = malloc(sizeof(dmk_t));
	if (NULL == dmk)
		return -1;

	/* read existing header, if any */
	if (DMK_HEADER_SIZE != img_read(img, 0, dmk, DMK_HEADER_SIZE)) {
		valid = 0;
	} else {
		/* got existing DMK header */
		cyllen = dmk->cyllen[0] + 256 * dmk->cyllen[1];

		/* what is a sane minimum track length? */
		if (cyllen < 1024)
			valid = 0;
		/* this is certainly a wrong track length */
		if (cyllen > 0x7fff)
			valid = 0;

		/* reallocate the dmk struct to the track length */
		dmk = realloc(dmk, DMK_HEADER_SIZE + cyllen);
		if (NULL == dmk)
			return -1;
		memset(dmk->rsvd, 0xff, sizeof(dmk->rsvd));
		LOG((3,"FDD","DMK setup %u cylinders, %u heads, cyllen %#x\n",
			dmk->cylinders, (dmk->flags & DMK_FLAG_SSIDE) ? 1 : 2,
				cyllen));
	}

	if (0 == valid) {
		/* create a new DMK header */
		cyllen = DMK_TRACK_SIZE;
		dmk->wprot = 0;
		dmk->cylinders = 0;
		dmk->flags = 0;
		memset(dmk->rsvd, 0xff, sizeof(dmk->rsvd));
		dmk->cyllen[0] = cyllen % 256;
		dmk->cyllen[1] = cyllen / 256;
		memset(dmk->realdsk, 0, sizeof(dmk->realdsk));
		/* fill the idam and track buffers */
		memset(dmk->track, 0xff, cyllen);
		memset(dmk->track, 0x00, DMK_IDAM_SIZE);
	}

	/* set the image data to our dmk struct */
	if (0 != img_set_data(img, FDD_DATA_DMK, dmk)) {
		free(dmk);
		return -1;
	}
	*pdmk = dmk;

	heads = (dmk->flags & DMK_FLAG_SSIDE) ? 1 : 2;
	img_set_flag(img, DRV_TOTAL_HEADS, heads);
	img_set_flag(img, DRV_TOTAL_CYLINDERS, dmk->cylinders);
	img_set_flag(img, IMG_FORMAT, FDD_FORMAT_DMK);

	return 0;
}

int dmk_get_track(struct img_s *img, dmk_t *dmk, uint32_t cyl, uint32_t head)
{
	uint32_t drive, cyllen, heads;
	off_t offs;

#if	DMK_CACHE_TRACK
	/* is it the buffered track number? */
	if (cyl == dmk->rsvd[DMK_TRACK_BUFF] &&
		head == dmk->rsvd[DMK_SIDE_BUFF])
		return 0;
#endif

	drive = img_minor(img);
	cyllen = dmk->cyllen[0] + 256 * dmk->cyllen[1];
	heads = (dmk->flags & DMK_FLAG_SSIDE) ? 1 : 2;

	LOG((3,"DMK","#%x load track %u, head %u into buffer\n",
		drive, cyl, head));

	offs = DMK_HEADER_SIZE + (off_t)cyllen * (cyl * heads + head);
	memset(dmk->track, 0xff, cyllen);
	memset(dmk->track, 0x00, DMK_IDAM_SIZE);
	if (cyllen != img_read(img, offs, dmk->track, cyllen))
		return -1;

#if	DMK_CACHE_TRACK
	dmk->rsvd[DMK_TRACK_BUFF] = cyl;
	dmk->rsvd[DMK_SIDE_BUFF] = head;
#endif
	return 0;
}

int dmk_put_track(struct img_s *img, dmk_t *dmk, uint32_t cyl, uint32_t head)
{
	uint32_t drive, cyllen, heads;
	off_t offs;

	drive = img_minor(img);
	cyllen = dmk->cyllen[0] + 256 * dmk->cyllen[1];
	heads = (dmk->flags & DMK_FLAG_SSIDE) ? 1 : 2;

	LOG((3,"DMK","#%x write track %u, head %u from buffer\n",
		drive, cyl, head));

	offs = DMK_HEADER_SIZE + (off_t)cyllen * (cyl * heads + head);
	if (cyllen != img_write(img, offs, dmk->track, cyllen))
		return -1;

#if	DMK_CACHE_TRACK
	dmk->rsvd[DMK_TRACK_BUFF] = cyl;
	dmk->rsvd[DMK_SIDE_BUFF] = head;
#endif
	return 0;
}

int dmk_set_geometry(struct img_s *img)
{
	dmk_t *dmk;
	uint32_t drive, cyllen, heads, cyl, head, i, dp, dd, c, h, r, n, len;
	uint32_t sec_min, sec_max;
	uint32_t len_min, len_max;
	uint8_t *p;

	if (0 != dmk_setup(img, &dmk))
		return -1;

	drive = img_minor(img);
	heads = (dmk->flags & DMK_FLAG_SSIDE) ? 1 : 2;

	cyllen = dmk->cyllen[0] + 256 * dmk->cyllen[1];
	len_min = 0xffff;
	len_max = 0x0000;
	sec_min = 0xff;
	sec_max = 0x00;
	for (cyl = 0; cyl < dmk->cylinders; cyl++) {
		for (head = 0; head < heads; head++) {
			if (0 != dmk_get_track(img, dmk, cyl, head))
				continue;
			p = dmk->track;
			for (i = 0; i < DMK_IDAM_SIZE; i += 2) {
				dp = p[i] + 256 * p[i+1];
				if (0 == dp)
					break;
				dd = dp & DMK_IDAM_DDEN;
				dp = dp & ~DMK_IDAM_DDEN;
				/* must have address mark here */
				if (0xfe != p[dp])
					break;
				DMK_INC(dp,dd);
				c = p[dp]; DMK_INC(dp,dd);
				h = p[dp]; DMK_INC(dp,dd);
				r = p[dp]; DMK_INC(dp,dd);
				n = p[dp]; DMK_INC(dp,dd);

				/* sector length in bytes */
				len = 1 << (7 + (n & 3));
				if (r < sec_min)
					sec_min = r;
				if (r > sec_max)
					sec_max = r;
				if (len < len_min)
					len_min = len;
				if (len > len_max)
					len_max = len;
			}
		}
	}
	LOG((3,"DMK", "#%x cylinders %u, heads %u\n",
		drive, dmk->cylinders, heads));
	img_set_flag(img, DRV_TOTAL_CYLINDERS, dmk->cylinders);
	img_set_flag(img, DRV_TOTAL_HEADS, heads);
	img_set_flag(img, DRV_FIRST_SECTOR_ID, sec_min);
	img_set_flag(img, DRV_SECTORS_PER_TRACK, sec_max + 1 - sec_min);
	LOG((3,"DMK", "#%x found sector numbers from %u to %u\n",
		drive, sec_min, sec_max));
	if (len_min == len_max) {
		LOG((3,"DMK", "#%x found sector length 0x%04x\n",
			drive, len_min));
		img_set_flag(img, DRV_SECTOR_LENGTH, len_min);
	} else {
		LOG((3,"DMK", "#%x sector lengths from 0x%04x to 0x%04x\n",
			drive, len_min, len_max));
	}
	return 0;
}

/**
 * @brief dmk_get_next_id - get the next sector ID from a DMK format image
 *
 * This function reads the next sector ID (i.e. address mark) from a
 * "spinning" floppy in the DMK format. It does this by scanning the
 * track's IDAM for the next address mark, returning the values found
 * there.
 */
static int dmk_get_next_id(struct img_s *img, uint32_t head,
	fdd_chrn_id_t *id, uint32_t den)
{
	uint32_t drive, cyllen, cyl, crc;
	uint32_t i, ip, idx, dp, dd;
	uint8_t *src;
	dmk_t *dmk;

	if (NULL == img)
		return -1;

	if (0 != dmk_setup(img, &dmk))
		return -1;
	cyl = img_get_flag(img, DRV_CURRENT_CYLINDER);

	if (0 != dmk_get_track(img, dmk, cyl, head))
		return -1;

	drive = img_minor(img);
	cyllen = dmk->cyllen[0] + 256 * dmk->cyllen[1];

	ip = img_get_flag(img, DRV_ID_INDEX);
	for (i = ip + 1; i != ip; i++) {
		idx =  0;
		src = dmk->track;

		dp = src[2*i+0] + 256 * src[2*i+1];
		/* reached end of IDAM? */
		if (0 == dp) {
			/* set 'index pulse' */
			idx = 1;
			/* start over at IDAM inex 0 */
			i = 0;
			dp = src[2*i+0] + 256 * src[2*i+1];
		}
		/* mask double density flag */
		dd = dp & DMK_IDAM_DDEN;
		/* mask track offset */
		dp = dp & ~DMK_IDAM_DDEN;

		/* no address mark here? */
		if (src[dp] != 0xfe)
			continue;

		crc = 0xffff;
		if (dd) {
			/* double density CRC has three times 0xa1 included */
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
		}
		/* skip over AM */
		crc = calc_crc(crc, src[dp]);
		DMK_INC(dp,dd);

		/* copy address mark fields */
		id->C = src[dp]; DMK_INC(dp,dd);
		id->H = src[dp]; DMK_INC(dp,dd);
		id->R = src[dp]; DMK_INC(dp,dd);
		id->N = src[dp]; DMK_INC(dp,dd);
		id->data_id = id->R;
		id->flags = 0;

		crc = calc_crc(crc, id->C);
		crc = calc_crc(crc, id->H);
		crc = calc_crc(crc, id->R);
		crc = calc_crc(crc, id->N);

		/* verify CRC */
		if (src[dp] != (crc / 256))
			id->flags |= ID_FLAG_CRC_ERROR_IN_ID_FIELD;
		DMK_INC(dp,dd);
		if (src[dp] != (crc % 256))
			id->flags |= ID_FLAG_CRC_ERROR_IN_ID_FIELD;
		DMK_INC(dp,dd);

		/* scan for DAM */
		while (dp < cyllen) {
			if (src[dp] >= 0xf8 && src[dp] <= 0xfb)
				break;
			DMK_INC(dp,dd);
		}
		/* DAM not found, if dp reaches cyllen */
		if (dp >= cyllen)
			return -1;

		if (src[dp] != 0xfb)
			id->flags |= ID_FLAG_DELETED_DATA;
		/* skip DAM */
		DMK_INC(dp,dd);

		LOG((1,"DMK","#%x next id #%02x C:%02x H:%02x R:%02x N:%02x%s\n",
			drive, i, id->C, id->H, id->R, id->N,
			id->flags & ID_FLAG_DELETED_DATA ? " DDAM" : ""));

		img_set_flag(img, DRV_ID_INDEX, i);
		img_set_flag(img, DRV_INDEX, idx);
		return 0;
	}
	/* no valid address mark found */
	return -1;
}

/**
 * @brief dmk_read_sector - DMK format read sector function
 *
 * This function reads a sector from the track buffer of a
 * DMK format image.
 */
static int dmk_read_sector(struct img_s *img, uint32_t head, uint32_t sec,
	void *buff, size_t size, fdd_chrn_id_t *id, uint32_t den)
{
	uint32_t drive, cyllen, cyl, crc;
	uint8_t *src, *dst = (uint8_t *)buff;
	dmk_t *dmk;
	int i;

	if (NULL == img)
		return -1;

	if (0 != dmk_setup(img, &dmk))
		return -1;
	drive = img_minor(img);
	cyllen = dmk->cyllen[0] + 256 * dmk->cyllen[1];
	cyl = img_get_flag(img, DRV_CURRENT_CYLINDER);

	if (0 != dmk_get_track(img, dmk, cyl, head))
		return -1;
	src = dmk->track;

	/* track should be in the buffer; scan address marks */
	for (i = 0; i < DMK_IDAM_SIZE; i += 2) {
		uint32_t dp, dd, c, h, r, n, dam;

		dp = src[i] + 256 * src[i+1];
		if (0 == dp) {
			LOG((3,"DMK","#%x IDAM end at offs %u\n", drive, i));
			break;
		}
		dd = dp & DMK_IDAM_DDEN;
		dp = dp & ~DMK_IDAM_DDEN;

		if (0 == (dmk->flags & DMK_FLAG_IGNDEN) &&
			(DMK_IDAM_DDEN == dd) != (DEN_MFM_LO == den)) {
			LOG((3,"DMK","#%x IDAM density mismatch (want %cD)\n",
				drive, DEN_FM_LO == den ? 'S' : 'D'));
			continue;
		}

		if (dp >= cyllen) {
			LOG((3,"DMK","#%x IDAM pointer range 0x%04x (cyllen 0x%04x)\n",
				drive, dp, cyllen));
			continue;
		}
		/* track should contain an AM here */
		if (src[dp] != 0xfe) {
			LOG((3,"DMK","#%x IDAM pointer not ad 0xfe (found 0x%02x\n",
				drive, src[dp]));
			continue;
		}

		crc = 0xffff;
		if (DMK_IDAM_DDEN == dd) {
			/* double density CRC has three times 0xa1 included */
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
		}
		crc = calc_crc(crc, src[dp]);
		/* skip over AM */
		DMK_INC(dp,dd);

		/* get track, head, sector, length bytes from address field */
		id->C = c = src[dp];
		crc = calc_crc(crc, c);
		DMK_INC(dp,dd);

		id->H = h = src[dp];
		crc = calc_crc(crc, h);
		DMK_INC(dp,dd);

		id->R = r = src[dp];
		crc = calc_crc(crc, r);
		DMK_INC(dp,dd);

		id->N = n = src[dp];
		crc = calc_crc(crc, n);
		DMK_INC(dp,dd);

		id->data_id = id->R;
		id->flags = 0;

		/* verify CRC */
		if (src[dp] != (crc / 256))
			id->flags |= ID_FLAG_CRC_ERROR_IN_ID_FIELD;
		DMK_INC(dp,dd);
		if (src[dp] != (crc % 256))
			id->flags |= ID_FLAG_CRC_ERROR_IN_ID_FIELD;
		DMK_INC(dp,dd);

		/* compare sector number */
		if (r != sec)
			continue;

		/* scan for DAM */
		while (dp < cyllen) {
			if (src[dp] >= 0xf8 && src[dp] <= 0xfb)
				break;
			DMK_INC(dp,dd);
		}
		/* DAM not found, if dp reaches cyllen */
		if (dp >= cyllen)
			return -1;
		dam = src[dp];
		if (dam != 0xfb)
			id->flags |= ID_FLAG_DELETED_DATA;

		crc = 0xffff;
		if (DMK_IDAM_DDEN == dd) {
			/* double density CRC has three times 0xa1 included */
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
		}
		crc = calc_crc(crc, src[dp]);
		/* skip DAM */
		DMK_INC(dp,dd);

		LOG((3,"DMK","#%x read cyl %02x, sec %02x" \
			" C:%02x H:%02x R:%02x N:%02x DAM:%02x\n",
			drive, cyl, sec, c, h, r, n, dam));

		/* make sector length from length byte */
		n = sec_len[n & 3];
		/* copy smaller of sector length and size bytes */
		while (n > 0 && size > 0) {
			*dst++ = src[dp];
			crc = calc_crc(crc, src[dp]);
			DMK_INC(dp,dd);
			size--;
			n--;
		}

		/* verify CRC */
		if (src[dp] != (crc / 256))
			id->flags |= ID_FLAG_CRC_ERROR_IN_DATA_FIELD;
		DMK_INC(dp,dd);
		if (src[dp] != (crc % 256))
			id->flags |= ID_FLAG_CRC_ERROR_IN_DATA_FIELD;
		DMK_INC(dp,dd);

		/* pad with 0xff? */
		while (size > 0) {
			*dst++ = 0xff;
			size--;
		}
		return 0;
	}

	/* sector not found */
	return -1;
}

/**
 * @brief dmk_write_sector - DMK format write sector function
 *
 * This function writes a sector into the track buffer of a
 * DMK format image.
 */
static int dmk_write_sector(struct img_s *img, uint32_t head, uint32_t sec,
	void *buff, size_t size, uint32_t den, uint32_t ddam)
{
	uint32_t cyllen, cyl;
	uint32_t crc, dam, dp, dd, c, h, r, n;
	uint8_t *dst, *src = (uint8_t *)buff;
	dmk_t *dmk;
	int i;

	if (NULL == img)
		return -1;

	if (0 != dmk_setup(img, &dmk))
		return -1;
	cyllen = dmk->cyllen[0] + 256 * dmk->cyllen[1];
	cyl = img_get_flag(img, DRV_CURRENT_CYLINDER);

	if (0 != dmk_get_track(img, dmk, cyl, head))
		return -1;
	dst = dmk->track;

	/* track should be in the buffer; scan address marks */
	for (i = 0; i < DMK_IDAM_SIZE; i += 2) {

		dp = dst[i] + 256 * dst[i+1];
		if (0 == dp) {
			LOG((3,"FDD","IDAM end at offs %u\n", i));
			break;
		}
		dd = dp & DMK_IDAM_DDEN;
		dp = dp & ~DMK_IDAM_DDEN;

		/* compare matching density */
		if (0 == (dmk->flags & DMK_FLAG_IGNDEN) &&
			(DMK_IDAM_DDEN == dd) != (DEN_MFM_LO == den)) {
			LOG((3,"FDD","IDAM density mismatch (want %cD)\n",
				DEN_FM_LO == den ? 'S' : 'D'));
			continue;
		}

		if (dp < DMK_IDAM_SIZE || dp >= cyllen) {
			LOG((3,"FDD","IDAM pointer range 0x%04x (cyllen 0x%04x)\n",
				dp, cyllen));
			continue;
		}
		/* track should contain an AM here */
		if (dst[dp] != 0xfe) {
			LOG((3,"FDD","IDAM pointer not ad 0xfe (found 0x%02x)\n",
				dst[dp]));
			continue;
		}

		crc = 0xffff;
		if (DMK_IDAM_DDEN == dd) {
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
		}
		crc = calc_crc(crc, dst[dp]);
		/* skip AM */
		DMK_INC(dp,dd);

		/* get cylinder, head, record, length from address field */
		c = dst[dp];
		crc = calc_crc(crc, dst[dp]);
		DMK_INC(dp,dd);

		h = dst[dp];
		crc = calc_crc(crc, dst[dp]);
		DMK_INC(dp,dd);

		r = dst[dp];
		crc = calc_crc(crc, dst[dp]);
		DMK_INC(dp,dd);

		n = dst[dp];
		crc = calc_crc(crc, dst[dp]);
		DMK_INC(dp,dd);

		/* skip CRC */
		DMK_INC(dp,dd);
		DMK_INC(dp,dd);

		/* compare sector number */
		if (r != sec)
			continue;

		LOG((3,"FDD","DMK write cyl %u, sec %u, head %u, ddam %u\n",
			cyl, sec, head, ddam));

		/* scan for DAM */
		while (dp < cyllen) {
			if (dst[dp] >= 0xf8 && dst[dp] <= 0xfb)
				break;
			DMK_INC(dp,dd);
		}

		/* DAM not found if dp reaches cyllen */
		if (dp >= cyllen)
			return -1;
		dam = ddam ? 0xf8 : 0xfb;

		/* start over with new CRC */
		crc = 0xffff;
		if (DMK_IDAM_DDEN == dd) {
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
		}

		/* write new data address mark */
		dst[dp] = dam;
		DMK_DUP(dp,dd);
		crc = calc_crc(crc, dam);
		/* skip DAM */
		DMK_INC(dp,dd);

		/* make sector length from length byte */
		n = sec_len[n & 3];
		/* copy smaller of sector length and size bytes */
		while (n > 0 && size > 0) {
			dst[dp] = *src;
			DMK_DUP(dp,dd);
			crc = calc_crc(crc, *src);
			DMK_INC(dp,dd);
			src++;
			size--;
			n--;
		}

		/* update data CRC */
		dst[dp] = crc / 256;
		DMK_DUP(dp,dd);
		DMK_INC(dp,dd);

		dst[dp] = crc % 256;
		DMK_DUP(dp,dd);
		DMK_INC(dp,dd);

		if (0 != dmk_put_track(img, dmk, cyl, head))
			return -1;
		return 0;
	}

	/* sector not found */
	return -1;
}

/**
 * @brief dmk_read_track - DMK format read track function
 *
 * This function reads an entire track into the track buffer.
 */
static int dmk_read_track(struct img_s *img, uint32_t cyl, uint32_t head,
	void *buff, uint32_t size, uint32_t den)
{
	uint32_t cyllen, heads;
	uint8_t *dst = buff, *src;
	dmk_t *dmk;
	off_t offs;

	if (0 != dmk_setup(img, &dmk))
		return -1;
	heads = (dmk->flags & DMK_FLAG_SSIDE) ? 1 : 2;
	cyllen = dmk->cyllen[0] + 256 * dmk->cyllen[1];

	offs = DMK_HEADER_SIZE + (off_t)cyllen * (cyl * heads + head);

#if	DMK_CACHE_TRACK
	dmk->rsvd[DMK_TRACK_BUFF] = 0xff;
	dmk->rsvd[DMK_SIDE_BUFF] = 0xff;
#endif

	/* zap idam and track before trying to read it from the image */
	memset(dmk->track, 0xff, cyllen);
	memset(dmk->track, 0x00, DMK_IDAM_SIZE);
	if (cyllen != img_read(img, offs, dmk->track, cyllen))
		return -1;
	src = dmk->track + DMK_IDAM_SIZE;
	cyllen -= DMK_IDAM_SIZE;
	while (cyllen > 0 && size > 0) {
		*dst++ = *src++;
		cyllen--;
		if (den == DEN_FM_LO && 0 == (dmk->flags & DMK_FLAG_SD1BYTE)) {
			src++;
			cyllen--;
		}
		size--;
	}
	while (size > 0) {
		*dst++ = 0xff;
		size--;
	}
#if	DMK_CACHE_TRACK
	dmk->rsvd[DMK_TRACK_BUFF] = cyl;
	dmk->rsvd[DMK_SIDE_BUFF] = head;
#endif
	return 0;
}

/**
 * @brief dmk_write_track - DMK format write track function
 *
 * This function writes an entire track from the track buffer.
 */
static int dmk_write_track(struct img_s *img, uint32_t cyl, uint32_t head,
	void *buff, size_t size, uint32_t den)
{
	uint32_t cyllen, dd, dp, d0, ip, c, h, r, n, crc;
	uint8_t *src;
	dmk_t *dmk;
	int state;

	if (0 != dmk_setup(img, &dmk))
		return -1;

	cyllen = dmk->cyllen[0] + 256 * dmk->cyllen[1];

	if (cyl >= dmk->cylinders) {
		dmk->cylinders = cyl + 1;
		memset(dmk->rsvd, 0x00, sizeof(dmk->rsvd));
		img_write(img, 0, dmk, DMK_HEADER_SIZE);
		memset(dmk->rsvd, 0xff, sizeof(dmk->rsvd));
		img_set_flag(img, DRV_TOTAL_CYLINDERS, dmk->cylinders);
		img_set_flag(img, DRV_CURRENT_CYLINDER, cyl);
	}

	/* zap idam and track before trying to read it from the image */
	memset(dmk->track, 0xff, cyllen);
	memset(dmk->track, 0x00, DMK_IDAM_SIZE);

	src = buff;
	state = 0;
	dd = DEN_FM_LO == den ? 0 : DMK_IDAM_DDEN;
	ip = 0;			/* idam pointer */
	dp = DMK_IDAM_SIZE;	/* data pointer */
	d0 = 0;			/* data pointer of current 0xfe */
	c = h = r = n = 0;
	crc = 0xffff;
	while (dp < cyllen && size > 0) {
		dmk->track[dp] = *src;
		switch (state) {
		case 0:	/* looking for address mark (0xfe) */
			switch (*src) {
			case 0xf5:	/* DD reset CRC */
				dmk->track[dp] = 0xa1;
				break;
			case 0xfe:	/* address mark */
				crc = 0xffff;
				if (DEN_FM_LO != den) {
					crc = calc_crc(crc, 0xa1);
					crc = calc_crc(crc, 0xa1);
					crc = calc_crc(crc, 0xa1);
				}
				crc = calc_crc(crc, *src);
				d0 = dp;
				state++;
				break;
			}
			break;
		case 1:	/* get track number */
			c = *src;
			crc = calc_crc(crc, c);
			state++;
			break;
		case 2:	/* get head number */
			h = *src;
			crc = calc_crc(crc, h);
			state++;
			break;
		case 3:	/* get sector number */
			r = *src;
			crc = calc_crc(crc, r);
			state++;
			break;
		case 4:	/* get sector length code */
			n = *src;
			crc = calc_crc(crc, n);
			state++;
			n = 1 << (n & 3);
			break;
		case 5:	/* write address mark CRC high byte */
			if (0xf7 == *src) {
				dmk->track[dp] = crc / 256;
				state++;
				src--;
				size++;
			}
			break;
		case 6:	/* write address mark CRC low byte */
			dmk->track[dp] = crc % 256;
			state++;
			break;
		case 7:	/* now look for a data address mark (0xf8-0xfb) */
			switch (*src) {
			case 0xf5:	/* DD reset CRC */
				dmk->track[dp] = 0xa1;
				break;
			case 0xf8:
			case 0xf9:
			case 0xfa:
			case 0xfb:
				crc = 0xffff;
				if (DEN_FM_LO != den) {
					crc = calc_crc(crc, 0xa1);
					crc = calc_crc(crc, 0xa1);
					crc = calc_crc(crc, 0xa1);
				}
				crc = calc_crc(crc, *src);
				state++;
				break;
			}
			break;
		case 8:	/* skip sector data */
			crc = calc_crc(crc, *src);
			if (0 == --n)
				state++;
			break;
		case 9:	/* write data CRC high byte */
			if (0xf7 == *src) {
				dmk->track[dp] = crc / 256;
				state++;
				src--;
				size++;
			}
			break;
		case 10:/* write data CRC low byte */
			dmk->track[dp] = crc % 256;
			state++;
			break;
		case 11:/* look for next sector */
			d0 |= dd;
			dmk->track[ip+0] = d0 % 256;
			dmk->track[ip+1] = d0 / 256;
			ip += 2;
			/* abort if idam size reached */
			if (ip >= DMK_IDAM_SIZE)
				size = 1;
			state = 0;
			break;
		}
		size--;
		src++;
		dp++;
		if (0 == dd && 0 == (dmk->flags & DMK_FLAG_SD1BYTE)) {
			if (dp < cyllen) {
				/* duplicate byte for single density */
				dmk->track[dp] = dmk->track[dp-1];
				dp++;
			}
		}
	}

	/* pad track with 0xff to cyllen */
	while (dp < cyllen)
		dmk->track[dp++] = 0xff;

	if (0 != dmk_put_track(img, dmk, cyl, head))
		return -1;

#if	DMK_CACHE_TRACK
	dmk->rsvd[DMK_TRACK_BUFF] = cyl;
	dmk->rsvd[DMK_SIDE_BUFF] = head;
#endif
	return 0;
}

/**************************************************************************
 *
 *	DEF image format (default)
 *
 **************************************************************************/
#define	DEF_LUMPS	192
#define	DEF_GRANULES	8
#define	DEF_SECTORS	5
#define	DEF_MAX_DDAM	(DEF_LUMPS*DEF_GRANULES*DEF_SECTORS)

typedef struct ddam_s {
	uint32_t bits[(DEF_MAX_DDAM+31)/32];
}	ddam_t;

static int def_setup(struct img_s *img, ddam_t **pddam)
{
	ddam_t *ddam;

	/* see if we already have a ddam struct */
	img_get_data(img, FDD_DATA_DDAM, (void *)&ddam);
	if (NULL != ddam) {
		*pddam = ddam;
		return 0;
	}

	/* allocate a new ddam struct */
	ddam = (ddam_t *)calloc(1, sizeof(ddam_t));
	if (NULL == ddam)
		return -1;

	if (0 != img_set_data(img, FDD_DATA_DDAM, ddam)) {
		free(ddam);
		return -1;
	}
	*pddam = ddam;

	return 0;
}

/**
 * @brief def_get_next_id - get the next sector ID from a spinning floppy
 *
 * This function reads the next sector ID (i.e. address mark) from a
 * "spinning" floppy in the default format, where the system driver
 * defined the geometry. It does this by counting an index between
 * 0 and sectors_per_track - 1, and faking the address mark fields
 * for the current index.
 */
static int def_get_next_id(struct img_s *img, uint32_t head,
	fdd_chrn_id_t *id, uint32_t den)
{
	uint32_t heads, cyl, spt, sn0, len, sec, idx, lba;

	if (NULL == img)
		return -1;

	heads = img_get_flag(img, DRV_TOTAL_HEADS);
	cyl = img_get_flag(img, DRV_CURRENT_CYLINDER);
	spt = img_get_flag(img, DRV_SECTORS_PER_TRACK);
	sn0 = img_get_flag(img, DRV_FIRST_SECTOR_ID);
	len = img_get_flag(img, DRV_SECTOR_LENGTH);
	sec = img_get_flag(img, DRV_ID_INDEX);

	if (cyl >= img_get_flag(img, DRV_TOTAL_CYLINDERS))
		return -1;

	if (sec == (spt - 1) || 0 == spt) {
		idx = 1;
	} else {
		idx = 0;
	}
	img_set_flag(img, DRV_INDEX, idx);

	sec = (sec + 1) % (spt > 0 ? spt : 1);
	img_set_flag(img, DRV_ID_INDEX, sec);

	/* adjust for non zero-based sector numbers */
	sec += sn0;

	id->C = cyl;
	id->H = head;
	id->R = sec;
	id->N = (len >> 8) & 7;
	id->data_id = sec;
	id->flags = 0;

	lba = (cyl * heads + head) * spt + sec - sn0;

	if (0 != fdd_get_ddam(img, lba))
		id->flags |= ID_FLAG_DELETED_DATA;

	LOG((1,"DEF","next id #%02x C:%02x H:%02x R:%02x N:%02x%s\n",
		sec - sn0, id->C, id->H, id->R, id->N,
		id->flags & ID_FLAG_DELETED_DATA ? " DDAM" : ""));

	if (0 == spt)
		return -1;
	return 0;
}

/**
 * @brief def_read_sector - default read sector function
 *
 * This assumes the default image type with disk geometry
 * defined by the system driver.
 */
static int def_read_sector(struct img_s *img, uint32_t head, uint32_t sec,
	void *buff, size_t size, fdd_chrn_id_t *id, uint32_t den)
{
	uint32_t heads, cyl, spt, sn0, len, lba;
	off_t offs;

	if (NULL == img)
		return -1;
	cyl = img_get_flag(img, DRV_CURRENT_CYLINDER);
	heads = img_get_flag(img, DRV_TOTAL_HEADS);
	spt = img_get_flag(img, DRV_SECTORS_PER_TRACK);
	sn0 = img_get_flag(img, DRV_FIRST_SECTOR_ID);
	len = img_get_flag(img, DRV_SECTOR_LENGTH);

	if (head >= heads)
		return -1;

	if (cyl >= img_get_flag(img, DRV_TOTAL_CYLINDERS))
		return -1;

	if (sec > spt)
		return -1;

	lba = (heads * cyl + head) * spt + sec - sn0;
	LOG((1,"DEF","read C:%02x H:%x R:%02x SPT:%02x (LBA:%x) size:0x%x\n",
		cyl, head, sec, spt, lba, size));
	id->C = cyl;
	id->H = head;
	id->R = sec;
	id->N = len >> 8;
	id->data_id = sec;
	id->flags = 0;

	if (0 != fdd_get_ddam(img, lba))
		id->flags |= ID_FLAG_DELETED_DATA;

	offs = (off_t)len * lba;

	if (size == img_read(img, offs, buff, size))
		return 0;
	return -1;
}

/**
 * @brief def_write_sector - default write sector function
 *
 * This assumes the default image type with disk geometry
 * defined by the system driver.
 */
static int def_write_sector(struct img_s *img, uint32_t head, uint32_t sec,
	void *buff, size_t size, uint32_t den, uint32_t ddam)
{
	uint32_t cyl, heads, spt, sn0, len, lba;
	off_t offs;

	cyl = img_get_flag(img, DRV_CURRENT_CYLINDER);
	heads = img_get_flag(img, DRV_TOTAL_HEADS);
	spt = img_get_flag(img, DRV_SECTORS_PER_TRACK);
	sn0 = img_get_flag(img, DRV_FIRST_SECTOR_ID);
	len = img_get_flag(img, DRV_SECTOR_LENGTH);

	if (head >= heads)
		return -1;

	if (cyl >= img_get_flag(img, DRV_TOTAL_CYLINDERS))
		return -1;

	if (sec > spt)
		return -1;

	lba = (heads * cyl + head) * spt + sec - sn0;
	LOG((1,"DEF","write C:%02x H:%x R:%02x SPT:%02x (LBA:%x) size:0x%x\n",
		cyl, head, sec, spt, lba, size));
	fdd_set_ddam(img, lba, ddam);

	offs = (off_t)len * lba;
	if (size == img_write(img, offs, buff, size))
		return 0;
	return -1;
}

/**
 * @brief fdd_set_ddam - set the deleted address mark status of a sector
 *
 *	Sets the "deleted data address mark" flag for a specific sector.
 *	Allocates an internal ddam_t on the first call.
 *
 * @param img img handle
 * @param lba logical block address (disk relative sector)
 * @param del DDAM state
 * @result Nothing
 */
void fdd_set_ddam(struct img_s *img, uint32_t lba, int del)
{
	ddam_t *ddam;

	if (NULL == img)
		return;

	if (0 != def_setup(img, &ddam))
		return;

	/* disk relative sector number */
	if (lba >= DEF_MAX_DDAM)
		return;

	if (del)
		ddam->bits[lba / 32] |= 1ul << (lba % 32);
	else
		ddam->bits[lba / 32] &= ~(1ul << (lba % 32));
}

/**
 * @brief fdd_get_ddam - get the deleted address mark status of a sector
 *	img handle, logical block addres (disk relative sector)
 *
 *	Returns the "deleted data address mark" flag for a specific sector.
 *
 * @param img img handle
 * @param lba  logical block address (disk relative sector)
 * @result DDAM flag
 */
int fdd_get_ddam(struct img_s *img, uint32_t lba)
{
	ddam_t *ddam;

	if (NULL == img)
		return -1;

	if (0 != def_setup(img, &ddam))
		return -1;

	/* disk relative sector number */
	if (lba >= DEF_MAX_DDAM)
		return 0;

	return (ddam->bits[lba / 32] & 1ul << (lba % 32)) ? 1 : 0;
}


uint32_t fdd_get_datarate_in_us(uint32_t density)
{
	switch (density) {
	case DEN_FM_LO:
		return 1000000 / 125000;
	case DEN_FM_HI:
		return 1000000 / 250000;
	case DEN_MFM_LO:
		return 1000000 / 250000;
	case DEN_MFM_HI:
		return 1000000 / 500000;
	}
	return 1;
}

int fdd_seek(struct img_s *img, int direction)
{
	img_set_flag(img, DRV_SEEK, direction);
	return 0;
}

/**
 * @brief fdd_get_next_id - get the next sector ID from a spinning floppy
 *
 * This function reads the next sector ID (i.e. address mark) from a "spinning" floppy.
 */
int fdd_get_next_id(struct img_s *img, uint32_t head, fdd_chrn_id_t *id, uint32_t den)
{
	uint32_t fmt;

	fmt = img_get_flag(img, IMG_FORMAT);
	switch (fmt) {
	case FDD_FORMAT_JV1:
		break;
	case FDD_FORMAT_JV3:
		return jv3_get_next_id(img, head, id, den);
	case FDD_FORMAT_DMK:
		return dmk_get_next_id(img, head, id, den);
	default:
		return def_get_next_id(img, head, id, den);
	}
	return -1;
}

int fdd_read_sector_data(struct img_s *img, uint32_t head, uint32_t sec,
	void *buff, size_t size, fdd_chrn_id_t *id, uint32_t den)
{
	uint32_t fmt;

	fmt = img_get_flag(img, IMG_FORMAT);
	switch (fmt) {
	case FDD_FORMAT_JV1:
		break;
	case FDD_FORMAT_JV3:
		return jv3_read_sector(img, head, sec, buff, size, id, den);
	case FDD_FORMAT_DMK:
		return dmk_read_sector(img, head, sec, buff, size, id, den);
	default:
		return def_read_sector(img, head, sec, buff, size, id, den);
	}
	return -1;
}

int fdd_write_sector_data(struct img_s *img, uint32_t head, uint32_t sec,
	void *buff, size_t size, uint32_t den, uint32_t ddam)
{
	uint32_t fmt;

	fmt = img_get_flag(img, IMG_FORMAT);
	switch (fmt) {
	case FDD_FORMAT_JV1:
		break;
	case FDD_FORMAT_JV3:
		return jv3_write_sector(img, head, sec, buff, size, den, ddam);
	case FDD_FORMAT_DMK:
		return dmk_write_sector(img, head, sec, buff, size, den, ddam);
	default:
		return def_write_sector(img, head, sec, buff, size, den, ddam);
	}
	return -1;
}

int fdd_read_track(struct img_s *img, uint32_t cyl, uint32_t head,
	void *buff, size_t size, uint32_t den)
{
	uint32_t fmt;

	if (NULL == img)
		return -1;
	fmt = img_get_flag(img, IMG_FORMAT);
	switch (fmt) {
	case FDD_FORMAT_JV1:
		break;
	case FDD_FORMAT_JV3:
		return jv3_read_track(img, cyl, head, buff, size, den);
	case FDD_FORMAT_DMK:
		return dmk_read_track(img, cyl, head, buff, size, den);
	}
	return -1;
}

int fdd_write_track(struct img_s *img, uint32_t cyl, uint32_t head,
	void *buff, size_t size, uint32_t den)
{
	uint32_t fmt;

	if (NULL == img)
		return -1;
	fmt = img_get_flag(img, IMG_FORMAT);
	switch (fmt) {
	case FDD_FORMAT_JV1:
		break;
	case FDD_FORMAT_JV3:
		return jv3_write_track(img, cyl, head, buff, size, den);
		break;
	case FDD_FORMAT_DMK:
		return dmk_write_track(img, cyl, head, buff, size, den);
	}
	return -1;
}

int fdd_find_format(struct img_s *img)
{
	uint8_t *buff;
	jv3_header_t *jv3;
	dmk_t *dmk;
	uint32_t drive;
	uint32_t offs;
	uint8_t sec_min, sec_max;
	uint8_t cyl_min, cyl_max;
	uint8_t heads;
	size_t size;

	if (NULL == img)
		return -1;

	if (0 == img_get_flag(img, IMG_EXISTS))
		return -1;

	buff = malloc(0x10000);

	drive = img_minor(img);
	img_set_geometry(img, 1, 1, 1, 256, 0, 0, 0);
	img_read(img, 0, buff, 0x10000);

	/* try JV3 format */
	jv3 = (jv3_header_t *)buff;
	sec_min = 0xff; sec_max = 0x00;
	cyl_min = 0xff; cyl_max = 0x00;
	size = JV3_HEADER_SIZE;
	heads = 1;
	for (offs = 0; offs < JV3_SECTORS; offs++) {
		uint8_t cyl, sec, flg;
		cyl = jv3->table[3*offs + 0];
		sec = jv3->table[3*offs + 1];
		flg = jv3->table[3*offs + 2];
		if (cyl == 0xff)
			break;
		if (cyl < cyl_min)
			cyl_min = cyl;
		if (cyl > cyl_max)
			cyl_max = cyl;
		if (sec < sec_min)
			sec_min = sec;
		if (sec > sec_max)
			sec_max = sec;
		if (flg & JV3_SIDE)
			heads = 2;
		switch (flg & JV3_SIZE_MASK) {
		case JV3_SIZE_256:
			size += 256;
			break;
		case JV3_SIZE_128:
			size += 128;
			break;
		case JV3_SIZE_1024:
			size += 1024;
			break;
		case JV3_SIZE_512:
			size += 512;
			break;
		}
	}
	if (size == img_get_size(img)) {
		free(buff);
		LOG((3,"JV3","#%x found %u heads, cylinders %u to %u, sectors %02x to %02x\n",
			drive, heads, cyl_min, cyl_max, sec_min, sec_max));
		jv3_set_geometry(img);
		return 0;
	}

	/* try DMK format */
	dmk = (dmk_t *)buff;
	if (dmk->cylinders < 0xe0) {
		uint32_t cyllen;
		uint32_t heads;

		cyllen = dmk->cyllen[0] + 256 * dmk->cyllen[1];
		heads = (dmk->flags & DMK_FLAG_SSIDE) ? 1 : 2;
		/* possible track length? */
		if (cyllen > 1024 && cyllen < 32768) {
			size = img_get_size(img) - DMK_HEADER_SIZE;
			/* check if the size matches */
			LOG((3,"DMK","#%x found cyllen %u, heads %u, size %u\n",
				drive, cyllen, heads, (uint32_t)size));
			/* if we have a whole number of cylinders... */
			if (0 == (size % cyllen)) {
				dmk_set_geometry(img);
				free(buff);
				return 0;
			}
		}
	}

	free(buff);
	return -1;
}
