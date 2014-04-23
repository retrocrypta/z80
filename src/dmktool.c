/* ed:set tabstop=8 noexpandtab: */
/**************************************************************************
 *
 * dmktool.c	Tool to list and extract files from images in DMK format
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

/** @brief TRACKSIZE - DMK absolute maximum tracksize (useful is 0x1900) */
#define	TRACKSIZE	0x8000
/** @brief IDAM_DDEN - DMK flag for double density sector in the IDAM */
#define	IDAM_DDEN	0x8000
/** @brief IDAM_SIZE - DMK # of entries in the IDAM (index data address mark) */
#define	IDAM_SIZE	0x80

/** @brief FLAG_SSIDE - DMK flag bit to indicate single sided image */
#define	FLAG_SSIDE	0x10
/** @brief FLAG_SSIDE - DMK flag bit to indicate 1 byte per SD data (else 2) */
#define	FLAG_SD1BYTE	0x40
/** @brief FLAG_SSIDE - DMK flag bit to indicate ignore density */
#define	FLAG_IGNDEN	0x80

/** @brief single density sectors per track */
#define	SD_SPT		10
/** @brief double density sectors per track */
#define	DD_SPT		18
#define	MAX_HEADS	2

#define	PRINTABLE(ch) ((ch) < 32 || (ch) > 126 ? '.' : (ch))

/** @brief DMK header structure (16 bytes), followed by track data */
typedef struct dmk_s {
	/** @brief write protected */
	uint8_t wprot;
	/** @brief number of tracks */
	uint8_t tracks;
	/** @brief length of each track (LSB first) */
	uint8_t trklen[2];
	/** @brief global flags */
	uint8_t flags;
	/** @brief reserved data */
	uint8_t reserved[7];
	/** @brief real disk drive (like "A:") */
	uint8_t realdsk[4];
	/** @brief track data (more to folloe) */
	uint8_t track[TRACKSIZE];
}	dmk_t;

#define	DMK_HEADER_SIZE	(sizeof(dmk_t) - TRACKSIZE)

/** @brief structure to pass information about a DAM (data address mark) */
typedef struct dam_s {
	/** @brief offset into track's data */
	uint32_t dp;
	/** @brief cylinder */
	uint8_t c;
	/** @brief head */
	uint8_t h;
	/** @brief record */
	uint8_t r;
	/** @brief length code (0x80 << n) */
	uint8_t n;
#define	CHRN_DOUBLE_DENSITY	0x01
#define	CHRN_AM_MISSING		0x02
#define	CHRN_AM_BAD_CRC		0x04
#define	CHRN_DAM_MISSING	0x08
#define	CHRN_DAM_BAD_CRC	0x10
#define	CHRN_DAM_DELETED	0x20
#define	CHRN_UNUSED_40		0x40
#define	CHRN_EMPTY		0x80
	/** @brief AM and DAM status */
	uint8_t flags;
}	dam_t;

/** @brief structure to keep the status of a track after sanity checks */
typedef struct {
	int8_t perfect;
	int8_t am_good;
	int8_t am_bad;
	int8_t am_crc_bad;
	int8_t dam_missing;
	int8_t dam_crc_bad;
	int8_t sector_count;
	dam_t sector[IDAM_SIZE/2];
}	track_stat_t;

/** @brief format of a GAT sector (granule allocation table) */
typedef struct {
	uint8_t bit[192];		/* GAT bit masks (max. 192) */
	char name_date[32];		/* disk name and date */
	uint8_t sys_tsa[32];	/* /SYS files track, sector, grans */
}	gat_t;

/** @brief dmk - global track buffer */
dmk_t dmk;

#define TRACKS		(dmk.tracks)
#define SIDES		(dmk.flags & FLAG_SSIDE ? 1 : 2)
#define TRACK_LEN	(256ul * dmk.trklen[1] + dmk.trklen[0])

/** @brief track_stat - global track status for current DMK image */
track_stat_t track_stat[256][MAX_HEADS];

/* FIXME */
#define	SPT (SIDES * track_stat[1][0].sector_count)

/** @brief gpl - granules per lump */
int gpl = 8;

/** @brief spg - sectors per granule */
int spg = 5;

/** @brief ddsl - disk directory start lump */
int ddsl = 0;

/** @brief ddga - disk directory granule allocation */
int ddga = 0;

/** @brief t0s - track 0 sectors */
int t0s = 10;

/** @brief dir_lba - disk directory start sector */
int dir_lba = 0;

/** @brief dir_len - length of disk directory */
int dir_len = 0;

/** @brief filtered output if non-zero */
int filter = 0;

/** @brief verify integrity if non-zero */
int verify = 0;

/** @brief be verbose if non-zero */
int verbose = 0;

/** @brief pathname of the export directory if non-NULL */
char *export = NULL;

int info(int level, const char *fmt, ...)
{
	va_list ap;
	int length;

	if (level > verbose)
		return 0;
	va_start(ap, fmt);
	length = vfprintf(stdout, fmt, ap);
	va_end(ap);
	return length;
}

/* Accelerator table to compute the CRC eight bits at a time */
unsigned short const crc16[256] = {
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
  0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
  0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
  0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
  0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
  0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
  0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
  0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
  0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
  0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
  0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
  0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
  0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
  0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
  0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
  0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
  0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
  0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
  0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
  0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
  0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
  0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
  0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

/* @brief CALC_CRC1a - calculate CRC16 using polynome */
unsigned short CALC_CRC1a(unsigned short crc, uint8_t byte)
{
	int i = 8;
	unsigned short b = byte << 8;
	while (i--) {
		crc = (crc << 1) ^ (((crc ^ b) & 0x8000) ? 0x1021 : 0);
		b <<= 1;
	}
	return crc;
}

/* @brief CALC_CRC1b - calculate CRC16 using table lookup */
#define CALC_CRC1b(crc, c) ((((crc)<<8)^crc16[(((crc)>>8)^(c))&0xff])&0xffff)

#ifndef calc_crc
#define calc_crc CALC_CRC1b
#endif

static const char *token[128+26+1] = {
	/* TRS-80 and Colour genie shared tokens */
	"END","FOR","RESET","SET","CLS","CMD","RANDOM","NEXT",
	"DATA","INPUT","DIM","READ","LET","GOTO","RUN","IF",
	"RESTORE","GOSUB","RETURN","REM","STOP","ELSE","TRON","TROFF",
	"DEFSTR","DEFINT","DEFSNG","DEFDBL","LINE","EDIT","ERROR","RESUME",
	"OUT","ON","OPEN","FIELD","GET","PUT","CLOSE","LOAD",
	"MERGE","NAME","KILL","LSET","RSET","SAVE","SYSTEM","LPRINT",
	"DEF","POKE","PRINT","CONT","LIST","LLIST","DELETE","AUTO",
	"CLEAR","CLOAD","CSAVE","NEW","TAB(","TO","FN", "USING",
	"VARPTR","USR","ERL","ERR","STRING$","INSTR","CHECK","TIME$",
	"MEM","INKEY$","THEN","NOT","STEP","+","-","*",
	"/","[","AND","OR",">","=","<","SGN",
	"INT","ABS","FRE","INP","POS","SQR","RND","LOG",
	"EXP","COS","SIN","TAN","ATN","PEEK","CVI","CVS",
	"CVD","EOF","LOC","LOF","MKI$","MKS$","MKD$","CINT",
	"CSNG","CDBL","FIX","LEN","STR$","VAL","ASC","CHR$",
	"LEFT$","RIGHT$","MID$","'","","","","",
	/* Colour Genie specific tokens (after 0xff) */
	"COLOUR","FCOLOU",/* (sic!) */
	"KEYPAD","JOY","PLOT","FGR","LGR","FCLS",
	"PLAY","CIRCLE","SCALE","SHAPE","NSHAPE","XSHAPE","PAINT","CPOINT",
	"NPLOT","SOUND","CHAR","RENUM","SWAP","FKEY","CALL","VERIFY",
	"BGRD","NBGRD", NULL
};

const char *detokenize(uint8_t *src, int maxsize)
{
	static char buff[8192];
	char *dst = buff;
	int string = 0;

	while (0x00 != *src && maxsize-- > 0) {
		if (string) {
			if ('"' == *src)
				string = 0;
			if (*src >= 32 && *src < 127)
				*dst++ = *src;
			else
				dst += sprintf(dst, "\\%03o", *src);
		} else {
			if (0xff == *src) {
				src++;
				if (*src < 0x80 + 26)
					dst += sprintf(dst, "%s", token[*src]);
				else
					dst += sprintf(dst, "<ff%02x>", *src);
			} else if (*src > 0x7f) {
				dst += sprintf(dst, "%s", token[*src & 0x7f]);
			} else {
				*dst++ = *src;
				if ('"' == *src)
					string = 1;
			}
		}
		src++;
	}
	*dst = '\0';
	return buff;
}

/**
 * @brief DMKINC - increment DMK format data pointer
 * Increment data pointer 'dp' once or twice, depending on
 * double-density flag 'dd', and dmk.flags bit FLAGS_SD1BYTE
 */
#define	DMKINC(dp,dd) \
	dp += ((dd) ? 1 : (dmk.flags & FLAG_SD1BYTE) ? 1 : 2)


/**
 * @brief dmk_init - initialize the DMK header in global 'dmk'
 */
int dmk_init(FILE *fp)
{
	fseek(fp, 0, SEEK_SET);

	if (DMK_HEADER_SIZE != fread(&dmk, 1, DMK_HEADER_SIZE, fp)) {
		perror("read DMK header");
		exit(1);
	}
	memset(dmk.reserved, 0xff, sizeof(dmk.reserved));

	return 0;
}

/**
 * @brief dmk_get_track - get a track 'trk' from side 'side' into the buffer
 */
int dmk_get_track(FILE *fp, int trk, int side)
{
	off_t offs;

	if (trk == dmk.reserved[0] && side == dmk.reserved[1])
		return 0;

	offs = DMK_HEADER_SIZE + TRACK_LEN * (trk * SIDES + side);

	info(1,"get track: %u, side: %u, offs: %04x\n",
			trk, side, (unsigned)offs);

	if (0 != fseek(fp, offs, SEEK_SET))
		return -1;

	if (TRACK_LEN != fread(dmk.track, 1, TRACK_LEN, fp)) {
		memset(dmk.track, 0xff, sizeof(dmk.track));
		memset(dmk.track, 0x00, IDAM_SIZE);
		return -1;
	}

	dmk.reserved[0] = trk;
	dmk.reserved[1] = side;

	return 0;
}

/**
 * @brief dmk_get_sector - copy data for a specific sector from track buffer
 */
int dmk_get_sector(FILE *fp, int trk, int side, int sec, void *buff)
{
	track_stat_t *ts;
	int i;

	if (trk >= TRACKS)
		return -1;

	ts = &track_stat[trk][side];

	info(1,"C:%02x H:%x R:%02x\n", trk, side, sec);
	if (0 != dmk_get_track(fp, trk, side)) {
		return -1;
	}
		
	ts = &track_stat[trk][side];
	/* now find sector number matching the lba (zero based!?) */
	for (i =0; i < ts->sector_count; i++) {
		dam_t *dam = (dam_t *)&ts->sector[sec];
		if (dam->r == sec) {
			/* copy raw data to buff */
			memcpy(buff, &dmk.track[dam->dp], 256);
			return 0;
		}
	}
	return -1;
}

/**
 * @brief newdos_get_lba_sector - copy data for a logical block address sector
 */
int newdos_get_lba_sector(FILE *fp, int lba, void *buff,
	int *ptrk, int *pside, int *psec)
{
	int trk, side, sec;
	track_stat_t *ts;

	trk = 0;
	side = 0;
	info(1,"get lba:%04x\n", lba);
	while (lba > 0) {
		ts = &track_stat[trk][side];
		if (lba < ts->sector_count)
			break;
		lba -= ts->sector_count;
		if (++side >= SIDES) {
			side = 0;
			trk++;
		}
		if (trk >= TRACKS) {
			/* FIXME: set some error "out of bounds" */
			info(1," - out of bounds\n");
			return -1;
		}
	}

	ts = &track_stat[trk][side];

	/* got track, side, and track stat pointe in ts */
	if (0 != dmk_get_track(fp, trk, side)) {
		/* FIXME: set some error "track read error" */
		info(1," - track read error\n");
		return -1;
	}
	info(1," C:%02x H:%x R:%02x", trk, side, lba);
		
	/* now find sector number matching the lba (zero based!?) */
	for (sec = 0; sec < ts->sector_count; sec++) {
		dam_t *dam = (dam_t *)&ts->sector[sec];
		if (dam->r == lba) {
			info(1," DP:%04x ok\n", dam->dp);
			if (NULL != ptrk)
				*ptrk = trk;
			if (NULL != pside)
				*pside = side;
			if (NULL != psec)
				*psec = lba;
			/* copy raw data to buff */
			memcpy(buff, &dmk.track[dam->dp], 256);
			return 0;
		}
	}
	info(1," - sector not found (%d)\n", lba);
	/* FIXME: set some error number "sector not found" */
	return -1;
}

/**
 * @brief dmk_scan_sectors - scan a track's sectors and update a track_stat_t*
 */
int dmk_scan_sectors(track_stat_t *ts)
{
	uint8_t *p;
	int ip;
	unsigned long dp;
	unsigned long dd;
	unsigned long len;
	unsigned short crc, c;
	dam_t *dam;

	memset(ts, 0, sizeof(*ts));
	for (ip = 0; ip < IDAM_SIZE; ip += 2) {
		p = dmk.track;
		dam = &ts->sector[ip/2];

		dp = p[ip+0] + 256 * p[ip+1];

		/* end of entries? */
		if (0 == dp)
			break;
		dd = dp & IDAM_DDEN;
		dp = dp & ~IDAM_DDEN;

		info(1,"#%02x @%04x", ip/2, dp);
		crc = 0xffff;
		if (IDAM_DDEN == dd) {
			dam->flags |= CHRN_DOUBLE_DENSITY;
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
		}
			
		if (p[dp] != 0xfe) {
			info(1," - not pointing to AM (0xfe)\n");
			dam->flags |= CHRN_AM_MISSING;
			ts->am_bad += 1;
			continue;
		}
		ts->am_good += 1;

		crc = calc_crc(crc, p[dp]);
		DMKINC(dp,dd);

		dam->c = p[dp];
		crc = calc_crc(crc, p[dp]);
		DMKINC(dp,dd);

		dam->h = p[dp];
		crc = calc_crc(crc, p[dp]);
		DMKINC(dp,dd);

		dam->r = p[dp];
		crc = calc_crc(crc, p[dp]);
		DMKINC(dp,dd);

		dam->n = p[dp];
		crc = calc_crc(crc, p[dp]);
		DMKINC(dp,dd);

		len = 1 << (7 + (dam->n & 3));

		info(1," C:%02x H:%02x R:%02x N:%02x",
			dam->c, dam->h, dam->r, dam->n);

		/* get CRC from track */
		c = 256 * p[dp];
		DMKINC(dp,dd);

		c = c | p[dp];
		DMKINC(dp,dd);

		if (c == crc) {
			info(1," AM-CRC:good");
			dam->flags &= ~CHRN_AM_BAD_CRC;
		} else {
			info(1," AM-CRC:bad (%04x != %04x)", c, crc);
			dam->flags |= CHRN_AM_BAD_CRC;
			ts->am_crc_bad += 1;
		}

		for (/* */; dp < TRACK_LEN; dp++) {
			if (p[dp] >= 0xf8 && p[dp] <= 0xfb)
				break;
		}
		if (dp >= TRACK_LEN) {
			info(1," no DAM!\n");
			dam->flags |= CHRN_DAM_MISSING;
			ts->dam_missing += 1;
			continue;
		}
		crc = 0xffff;
		if (IDAM_DDEN == dd) {
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
			crc = calc_crc(crc, 0xa1);
		}
		switch (p[dp]) {
		case 0xf8:
		case 0xf9:
		case 0xfa:
			dam->flags |= CHRN_DAM_DELETED;
			break;
		case 0xfb:
			dam->flags &= ~CHRN_DAM_DELETED;
			break;
		}
		crc = calc_crc(crc, p[dp]);
		info(1," DAM:%02x", p[dp]);
		DMKINC(dp,dd);
		dam->dp = dp;

		/* assume empty sector (all E5) */
		dam->flags |= CHRN_EMPTY;

		/* scan sector data and add into CRC */
		while (len-- > 0) {
			if (0 != (dam->flags & CHRN_EMPTY) && 0xe5 != p[dp])
				dam->flags &= ~CHRN_EMPTY;
			crc = calc_crc(crc, p[dp]);
			DMKINC(dp,dd);
		}

		/* get CRC from track */
		c = 256 * p[dp];
		DMKINC(dp,dd);

		c = c | p[dp];
		DMKINC(dp,dd);

		if (c == crc) {
			info(1," DAM-CRC:good");
			dam->flags &= ~CHRN_DAM_BAD_CRC;
		} else {
			info(1," DAM-CRC:bad (%04x != %04x)", c, crc);
			dam->flags |= CHRN_DAM_BAD_CRC;
			ts->dam_crc_bad += 1;
		}

		if (dam->flags & CHRN_EMPTY)
			info(1," empty");
		info(1,"\n");
	}
	ts->sector_count = ip / 2;
	info(1,"\n");

	if (0 == ts->am_bad && 0 == ts->am_crc_bad && 0 == ts->dam_crc_bad)
		ts->perfect = 1;
	return 0;
}

/**
 * @brief dmk_scan_tracks - scan all tracks from a DMK file image
 */
int dmk_scan_tracks(FILE *fp)
{
	int trk, side, errors;
	track_stat_t *ts;

	errors = 0;
	for (trk = 0; trk < TRACKS; trk++) {
		for (side = 0; side < SIDES; side++) {
			ts = &track_stat[trk][side];
			if (0 != dmk_get_track(fp, trk, side)) {
				info(0,"Track %02x side %x is missing\n",
					trk, side);
				errors += 1;
			}
			if (0 != dmk_scan_sectors(ts)) {
				info(0,"Track %02x side %x is corrupted\n",
					trk, side);
				return 1;
			}
			if (ts->perfect)
				continue;
			errors += 1;
			info(0,"Track %02x side %x has errors\n", trk, side);
		}
	}
	if (errors)
		info(0,"%d", errors);
	else
		info(0,"No");
	info(0," errors found in DMK image\n");
	return 0;
}

/**
 * @brief qsort_sector - quick sort callback to compare two dma_t* sectors
 */
int qsort_sector(const void *p1, const void *p2)
{
	const dam_t *dam1 = (const dam_t *)p1;
	const dam_t *dam2 = (const dam_t *)p2;
	return dam1->r - dam2->r;

}

/**
 * @brief dmk_sort_by_sector - sort a track_stat_t* by its sector numbers
 */
int dmk_sort_by_sector(track_stat_t *ts)
{
	qsort(ts->sector, ts->sector_count, sizeof(dam_t),
		qsort_sector);
	return 0;
}

/**
 * @brief newdos_extent_s - structure of an extent in a NEWDOS80 directory entry
 */
typedef struct newdos_extent_s {
	uint8_t reccnt;
	uint8_t recbeg;
	uint8_t lump;
	uint8_t gran_cnt;
}	newdos_extent_t;

/**
 * @brief newdos_dirent_s  - structur of a NEWDOS80 directory entry
 */
typedef struct newdos_dirent_s {
	uint8_t fl0;
	uint8_t fl1;
	uint8_t grans_per_extent;
	uint8_t eof[2];
	char filename[8];
	char extension[3];
	uint8_t pw1[2];
	uint8_t pw2[2];
	newdos_extent_t ext[3];
}	newdos_dirent_t;

#define	DIRENT_FL0_L	0x07
#define	DIRENT_FL0_I	0x08
#define	DIRENT_FL0_U	0x10
#define	DIRENT_FL0_5	0x20
#define	DIRENT_FL0_S	0x40

#define	DIRENT_FL1_HIT	0x20	/* entry has a HIT symbol */

/**
 * @brief newdos_flags_str - format dirent_t flags like NEWDOS80 (broken)
 */
const char *newdos_flags_str(newdos_dirent_t *de)
{
	static char flags[12+1];
	flags[ 0] = de->fl0 & DIRENT_FL0_S ? 'S' : '.';
	flags[ 1] = de->fl0 & DIRENT_FL0_I ? 'I' : '.';
	flags[ 2] = de->fl0 & DIRENT_FL0_U ? 'U' : '.';

	if (de->fl0 & DIRENT_FL0_S) {
		flags[ 3] = de->fl1 & 0x80 ? 'E' : '.';
		flags[ 4] = de->fl1 & 0x40 ? 'C' : '.';
		flags[ 5] = de->fl1 & 0x01 ? '0' : '.';
		flags[ 6] = de->fl1 & 0x02 ? '1' : '.';
		flags[ 7] = de->fl1 & 0x04 ? '2' : '.';
		flags[ 8] = de->fl0 & 0x08 ? '3' : '.';
		flags[ 9] = de->fl0 & 0x20 ? 'U' : '.';
		flags[10] = de->fl0 & 0x40 ? 'A' : '.';
	} else {
		flags[ 3] = '.';
		flags[ 4] = '.';
		flags[ 5] = '.';
		flags[ 6] = '.';
		flags[ 7] = '.';
		flags[ 8] = '.';
		flags[ 9] = '.';
		flags[10] = '.';
	}

	flags[11] = '0' + (de->fl0 & DIRENT_FL0_L);

	flags[12] = '\0';
	return flags;
}

/**
 * @brief binstr - format a value as string of 1s and 0s
 */
const char *binstr(int val, int bits)
{
	static char buff[32+1];
	int i;

	if (bits > 32)
		bits = 32;
	for (i = 0; i < bits; i++, val >>= 1)
		buff[i] = '0' + (val & 1);
	buff[bits] = '\0';
	return buff;
}

/**
 * @brief dmk_filename_ext - return filename and extension from a newdos_dirent_t
 */
const char *dmk_filename_ext(newdos_dirent_t *de, char delim)
{
	static char name[8+1+3+1];
	char *dst, *src;
	int i;

	dst = name;
	for (i = 0, src = de->filename; i < 8; i++) {
		if (*src == ' ')
			break;
		*dst++ = *src++;
	}
	if (*de->extension != ' ') {
		*dst++ = delim;
		for (i = 0, src = de->extension; i < 3; i++) {
			if (*src == ' ')
				break;
			*dst++ = *src++;
		}
	}
	*dst = '\0';
	return name;
}

/**
 * @brief verify_bas - verify an in-memory image for valid BASIC format
 */
int verify_bas(uint8_t *buff, int *psize)
{
	int state, offs;
	int addr = 0;	/* load address */
	int line = 0;	/* line number */
	int blen = 0;	/* line length, as counted */
	int lcnt = 0;	/* number of lines */

	if (0xff != *buff) {
		info(0,"BASIC data does not start with 0xff (0x%02x)\n",
			*buff);
		return -1;
	}
	info(2,"*** verify BAS\n");
	state = 0;
	for (offs = 0; offs < *psize; offs++) {
		switch (state) {
		case 0:	/* expect 0xff */
			state = 1;
			break;
		case 1:	/* memory address low */
			addr = buff[offs];
			state = 2;
			break;
		case 2:	/* memory address high */
			addr = 256 * buff[offs] + addr;
			state = 3;
			/* end of program when addr is zero */
			if (0 == addr) {
				if (offs + 1 < *psize) {
					info(1,"  data after program (0x%x)\n",
						*psize - 1 - offs);
				}
				/* force termination */
				*psize = offs + 1;
			}
			break;
		case 3:	/* line number low */
			line = buff[offs];
			state = 4;
			break;
		case 4:	/* line number high */
			line = 256 * buff[offs] + line;
			state = 5;
			info(2,"  line %5d at 0x%04x", line, addr);
			blen = 0;
			break;
		case 5:	/* data until 0x00 */
			blen += 1;
			if (0x00 != buff[offs])
				break;
			info(2," - 0x%02x bytes\n", blen);
			state = 1;	/* next address */
			lcnt++;
			break;
		}
	}
	info(2,"  %d lines\n", lcnt);
	return 0;
}

/**
 * @brief verify_cmd - verify an in-memory image for valid BINARY format
 */
int verify_cmd(uint8_t *buff, int *psize)
{
	int state, offs;
	int blen = 0;	/* block length */
	int addr = 0;	/* block offset */
	int chain = 0;	/* block offset after loading previous block */
	int badr = 0;	/* entry address */

	info(2,"*** verify CMD\n");
	state = 0;
	chain = 0;
	for (offs = 0; offs < *psize; offs++) {
		switch (state) {
		case 0:	/* expect block type */
			switch (buff[offs]) {
			case 0x01:	/* data block */
				state = 1;
				break;
			case 0x02:	/* entry block */
				state = 2;
				break;
			default:	/* everything else is a comment!? */
				if (chain)
					info(2," - %04x\n", chain - 1);
				chain = 0;
				info(1,"  comment block type 0x%02x\n",
						buff[offs]);
				state = 3;
				break;
			}
			break;
		case 1:	/* data block length */
			blen = buff[offs];
			state = 4;
			break;
		case 2:	/* entry block length */
			blen = buff[offs];
			state = 5;
			if (2 != blen) {
				if (chain)
					info(2," - %04x\n", chain - 1);
				chain = 0;
				info(1,"  bogus entry block length (%02x)\n",
					blen);
				/* force length 2 */
				blen = 2;
				buff[offs] = 0x02;
			}
			break;
		case 3:	/* comment block length */
			blen = buff[offs];
			state = 6;
			break;
		case 4:	/* data block address low */
			blen--;
			addr = buff[offs];
			state = 7;
			break;
		case 5:	/* entry point low */
			blen--;
			badr = buff[offs];
			state = 8;
			break;
		case 6:	/* skip comment */
			if (--blen <= 0)
				state = 0;
			break;
		case 7:	/* data block address high */
			blen--;
			addr = 256 * buff[offs] + addr;
			state = 9;
			if (blen <= 0)
				blen += 256;
			if (addr != chain) {
				if (chain)
					info(2," - %04x\n", chain - 1);
				chain = 0;
				info(1,"  data at %04x", addr);
			}
			chain = addr;
			break;
		case 8:	/* entry point high */
			blen--;
			badr = 256 * buff[offs] + badr;
			if (chain)
				info(2," - %04x\n", chain - 1);
			chain = 0;
			info(1,"  entry point at 0x%04x\n", badr);
			if (offs + 1 < *psize)
				info(1,"  data after entry point (0x%x)\n",
					*psize - 1 - offs);
			/* force termination */
			*psize = offs + 1;
			break;
		case 9:
			chain += 1;
			if (--blen <= 0)
				state = 0;
			break;
		}
	}
	if (chain)
		info(2," - %04x\n", chain - 1);
	return 0;
}

/**
 * @brief verify_txt - verify an in-memory image for text end-of-file code(s)
 */
int verify_txt(uint8_t *buff, int *psize)
{
	int offs;
	uint8_t *dst;

	dst = memchr(buff, '\003', *psize);
	offs = dst ? (int)(dst - buff) : *psize;
	if (offs + 1 < *psize) {
		info(1,"  data after end-of-file code ^C (0x%x)\n",
			*psize - 1 - offs);
		*psize = offs + 1;
	}
	dst = memchr(buff, '\000', *psize);
	offs = dst ? (int)(dst - buff) : *psize;
	if (offs + 1 < *psize) {
		info(1,"  data after end-of-file code ^@ (0x%x)\n",
			*psize - 1 - offs);
		*psize = offs + 1;
	}
	return 0;
}

/**
 * @brief hdexump_bas - dump an in-memory image in BASIC format
 */
int hexdump_bas(uint8_t *buff, int size)
{
	int state, offs;
	int addr = 0;	/* load address */
	int line = 0;	/* line number */
	int blen = 0;	/* line length, as counted */
	int lcnt = 0;	/* number of lines */

	state = 0;
	for (offs = 0; offs < size; offs++) {
		switch (state) {
		case 0:	/* expect 0xff */
			state = 1;
			break;
		case 1:	/* memory address low */
			addr = buff[offs];
			state = 2;
			break;
		case 2:	/* memory address high */
			addr = 256 * buff[offs] + addr;
			state = 3;
			/* end of program when addr is zero */
			if (0 == addr)
				offs = size;
			break;
		case 3:	/* line number low */
			line = buff[offs];
			state = 4;
			break;
		case 4:	/* line number high */
			line = 256 * buff[offs] + line;
			info(0,"%05x:(%04x) %5d", offs, addr, line);
			blen = 0;
			state = 5;
			break;
		case 5:	/* data until 0x00 */
			blen += 1;
			if (0x00 != buff[offs])
				break;
			info(0," %s\n",
				detokenize(buff + offs - blen + 1, blen));
			state = 1;	/* next address */
			lcnt++;
			break;
		}
	}
	return 0;
}

/**
 * @brief hexdump_cmd - do a hexdump of a buffer in BINARY format
 */
int hexdump_cmd(uint8_t *buff, int size)
{
	int state, offs;
	int blen = 0;	/* block length */
	int addr = 0;	/* block offset */
	int chain = 0;	/* block offset after loading previous block */
	int badr = 0;	/* entry address */
	int bcnt = 0;	/* block count */
	int i;

	state = 0;
	chain = 0;
	for (offs = 0; offs < size; offs++) {
		switch (state) {
		case 0:	/* expect block type */
			switch (buff[offs]) {
			case 0x01:	/* data block */
				info(0,"%05x: %02x", offs, buff[offs]);
				state = 1;
				break;
			case 0x02:	/* entry block */
				info(0,"%05x: %02x", offs, buff[offs]);
				state = 2;
				break;
			default:	/* everything else is a comment!? */
				info(0,"%05x: %02x", offs, buff[offs]);
				state = 3;
				break;
			}
			break;
		case 1:	/* data block length */
			blen = buff[offs];
			state = 4;
			info(0," %02x",
				buff[offs]);
			break;
		case 2:	/* entry block length */
			blen = buff[offs];
			state = 5;
			if (2 != blen) {
				/* force length 2 */
				blen = 2;
			}
			info(0," %02x",
				buff[offs]);
			break;
		case 3:	/* comment block length */
			blen = buff[offs];
			bcnt = 0;
			state = 6;
			info(0," %02x (comment length %03x)\n",
				buff[offs], blen);
			break;
		case 4:	/* data block address low */
			blen--;
			addr = buff[offs];
			state = 7;
			info(0," %02x", buff[offs]);
			break;
		case 5:	/* entry point low */
			blen--;
			badr = buff[offs];
			state = 8;
			info(0," %02x", buff[offs]);
			break;
		case 6:	/* skip comment */
			if (0 == (bcnt & 15))
				info(0,"%05x:(%04x)", offs, bcnt);
			info(0," %02x", buff[offs]);
			bcnt++;
			if (0 == (bcnt & 15))
				info(0,"\n");
			if (--blen <= 0) {
				if (0 != (bcnt & 15))
					info(0,"\n");
				state = 0;
			}
			break;
		case 7:	/* data block address high */
			blen--;
			addr = 256 * buff[offs] + addr;
			state = 9;
			bcnt = 0;
			if (blen <= 0)
				blen += 256;
			info(0," %02x (data length %03x @ %04x)\n",
				buff[offs], blen, addr);
			break;
		case 8:	/* entry point high */
			blen--;
			badr = 256 * buff[offs] + badr;
			bcnt = 0;
			info(0," %02x (entry point %04x)\n",
				buff[offs], badr);
			offs = size;
			break;
		case 9:
			if (0 == (bcnt & 15))
				info(0,"%05x:(%04x)", offs, addr + bcnt);
			info(0," %02x", buff[offs]);
			bcnt++;
			if (0 == (bcnt & 15)) {
				info(0," - ");
				for (i = -15; i <= 0; i++)
					info(0,"%c", PRINTABLE(buff[offs+i]));
				info(0,"\n");
			}
			if (--blen <= 0) {
				if (0 != (bcnt & 15)) {
					info(0,"%-*s - ", 3*(16-(bcnt&15)), "");
					for (i = -(bcnt&15); i <= 0; i++)
						info(0,"%c", PRINTABLE(buff[offs+i]));
					info(0,"\n");
				}
				state = 0;
			}
			break;
		}
	}
	return 0;
}

/**
 * @brief hexdump_raw - do a raw hexdump of a buffer of size 'size'
 */
int hexdump_raw(uint8_t *buff, int size)
{
	int y, x;

	for (y = 0; y < size; y += 16) {
		info(0,"%05x:", y);
		for (x = 0; x < 16; x++)
			info(0," %02x", buff[y+x]);
		info(0," - ");
		for (x = 0; x < 16; x++)
			info(0,"%c", PRINTABLE(buff[y+x]));
		info(0,"\n");
	}
	return 0;
}

/**
 * @brief newdos_read_file - read a file defined by a newdos_dirent_t* to memory
 */
int newdos_read_file(FILE *fp, newdos_dirent_t *de, int skip_dir)
{
	newdos_extent_t *ex;
	int eof, reccnt;
	int lump, gran, ngran, lba;
	int e;
	int fileoffs, filesize;
	uint8_t *filebuff;

	eof = de->eof[0] + 256 * de->eof[1];

	for (e = 0, ex = de->ext, filesize = 0; e < 3; e++, ex++) {
		if (0xff == ex->lump)
			break;
		reccnt = ex->reccnt;
		filesize += 256 * reccnt;
	}

	filebuff = malloc(filesize);

	for (e = 0, ex = de->ext, fileoffs = 0; e < 3; e++, ex++) {
		if (0xff == ex->lump)
			break;

		reccnt = ex->reccnt;

		/* translate NEWDOS80 lump and gran into trk, sec */
		lump = ex->lump;
		gran = ex->gran_cnt / 32;
		ngran = ex->gran_cnt % 32;
		/*
	 	 * lba = (lump * granules_per_lump + gran) *
	 	 *       sectors_per_granule + t0s;
	 	 */
		lba = ((lump * gpl) + gran) * spg + t0s;

		while (reccnt > 0) {
			if (skip_dir && lba == dir_lba) {
				info(1,"  skip directory at lba:%04x + %02x\n",
					dir_lba, dir_len);
				lba += dir_len;
			}
			newdos_get_lba_sector(fp, lba,
				&filebuff[fileoffs], NULL, NULL, NULL);
			reccnt--;
			fileoffs += 256;
			lba++;
		}
	}
	/* maybe strip some bytes of the last record */
	if (0 != eof)
		filesize = filesize - 256 + eof;

	if (verify > 0) {
		if (!memcmp(de->extension, "BAS", 3)) {
			/* verify BASIC format */
			verify_bas(filebuff, &filesize);
		} else if (!memcmp(de->extension, "CMD", 3)) {
			/* verify binary format */
			verify_cmd(filebuff, &filesize);
		} else if (!memcmp(de->extension, "ASM", 3)) {
			/* verify end-of-file character (^C or ^@) */
			verify_txt(filebuff, &filesize);
		}
	}
	
	if (export) {
		char path[FILENAME_MAX+1];
		FILE *fo;

		if (strcmp(export, "-")) {
			sprintf(path, "%s/%s",
				export, dmk_filename_ext(de, '.'));
			fo = fopen(path, "wb");
			if (NULL == fo) {
				perror(path);
				exit(1);
			}
		} else {
			fo = stdout;
		}
		fwrite(filebuff, 1, filesize, fo);
		if (fo != stdout) {
			fclose(fo);
			info(1,"  wrote %d (0x%x) bytes to %s\n",
				filesize, filesize, path);
		}
	} else if (filter) {
		if (!memcmp(de->extension, "BAS", 3)) {
			/* dump BASIC format */
			hexdump_bas(filebuff, filesize);
		} else if (!memcmp(de->extension, "CMD", 3)) {
			/* dump BINARY format */
			hexdump_cmd(filebuff, filesize);
		} else if (!memcmp(de->extension, "ASM", 3)) {
			/* dump TEXT format */
			hexdump_raw(filebuff, filesize);
		} else {
			/* dump raw */
			hexdump_raw(filebuff, filesize);
		}
	} else if (verbose) {
		/* dump raw */
		hexdump_raw(filebuff, filesize);
	}
	free(filebuff);
	return 0;
}

/**
 * @brief newdos_list_entries - list directory entries for one sector
 */
int newdos_list_entries(FILE *fp, void *gat, void *hit, void *dir)
{
	newdos_dirent_t *de;
	newdos_extent_t *ex;
	int eof, rec;
	int lump, gran, ngran, lba;
	int i, e;

	/* suppress unused warnings */
	(void)gat;
	(void)hit;

	for (i = 0; i < 256; i += 32) {
		de = (newdos_dirent_t *)((uint8_t *)dir + i);

		/* skip empty slots */
		if (0 == (de->fl0 & DIRENT_FL0_U))
			continue;

		ex = de->ext;

		eof = de->eof[0] + 256 * de->eof[1];

		for (e = 0; e < 3; e++, ex++) {
			if (0xff == ex->lump)
				break;
			rec = ex->reccnt;
	
			/* translate NEWDOS80 lump and gran into trk, sec */
			lump = ex->lump;
			gran = ex->gran_cnt / 32;
			ngran = ex->gran_cnt % 32;

			info(0,"%-15s %5d/%03d %3d %5d %5d %s %02x%02x %02x/%x/%02x\n",
				e ? "      extent" : dmk_filename_ext(de, '/'),
				eof ? rec - 1 : rec, eof,
				256,	/* how? */
				rec,
				ngran + 1,
				newdos_flags_str(de), de->fl0, de->fl1,
				lump, gran, ngran);

			/*
		 	 * lba = (lump * granules_per_lump + gran) *
		 	 *       sectors_per_granule + t0s;
		 	 */
			lba = ((lump * gpl) + gran) *
				spg + t0s;

			info(1,"  lump:%02x gran:%x ngran:%02x -> lba:%04x\n",
				lump, gran, ngran, lba);
		}

		newdos_read_file(fp, de, (de->fl0 & DIRENT_FL0_S) ? 0 : 1);
	}

	return 0;
}

/**
 * @brief newdos_find_directory - scan track list for (probable) directory start
 */
int newdos_find_directory(FILE *fp)
{
	int trk = 0, side = 0, sec = 0;
	int lump, gran, lba;
	track_stat_t *ts;

	trk = 0;
	side = 0;
	sec = 0;
	lba = 0;
	t0s = 0;

	/* find (probable) GAT sector */
	ts = &track_stat[trk][side];
	while (trk < TRACKS) {
		/* sort sectors in ascending order */
		dmk_sort_by_sector(ts);

		if (0 != dmk_get_track(fp, trk, side))
			break;

		if (0 == trk)
			t0s += ts->sector_count;

		for (sec = 0; sec < ts->sector_count; sec++) {
			if (0 != (ts->sector[sec].flags & CHRN_DAM_DELETED))
				break;
		}

		if (sec < ts->sector_count &&
			0 != (ts->sector[sec].flags & CHRN_DAM_DELETED))
			break;

		if (++side >= SIDES) {
			side = 0;
			++trk;
		}
		ts = &track_stat[trk][side];
		lba += ts->sector_count;
	}
	if (!(ts->sector[sec].flags & CHRN_DAM_DELETED))
		return -1;

	info(1,"track 0 sectors:%02x\n", t0s);

	dir_lba = lba + sec;

	/* adjust lba: add sector, subtract track 0 sectors, skip GAT and HIT */
	lba = lba + sec - t0s + 2;

	/* now lba should point to the sector containing BOOT/SYS */
	lump = lba / spg / gpl;
	gran = (lba / spg) % gpl;
	info(1,"directory at lba:%04x lump:%02x gran:%x (C:%02x H:%x R:%02x)\n",
		lba, lump, gran, trk, side, sec);
	ddsl = lump;

	/* unadjust lba: undo above adjustment */
	lba = lba - sec + t0s - 2;

	/* continue scanning for deleted data address marks */
	while (trk < TRACKS) {
		/* sort sectors in ascending order */
		dmk_sort_by_sector(ts);

		if (0 != dmk_get_track(fp, trk, side))
			break;

		for (/* */; sec < ts->sector_count; sec++) {
			if (0 == (ts->sector[sec].flags & CHRN_DAM_DELETED))
				break;
		}

		if (sec < ts->sector_count &&
			0 == (ts->sector[sec].flags & CHRN_DAM_DELETED))
			break;

		if (++side >= SIDES) {
			side = 0;
			++trk;
		}
		ts = &track_stat[trk][side];
		lba += ts->sector_count;
		sec = 0;
	}
	dir_len = lba + sec - dir_lba;
	/* now dir_len should be in whole lumps!? */
	lump = dir_len / spg / gpl;
	gran = (dir_len / spg) % gpl;
	info(1,"          length:%04x sec, lumps:%02x, grans:%02x (C:%02x H:%02x R:%02x)\n",
		dir_len, lump, gran, trk, side, sec);

	return 0;
}

/**
 * @brief dmk_list_directory - list the entire NEWDOS80 directory of an image
 */
int dmk_list_directory(FILE *fp)
{
	uint8_t buff[256];
	uint8_t gat[256];
	uint8_t hit[256];
	uint8_t dir[256];
	int trk, side, sec, id;
	int lump, gran, lba, ds;
	track_stat_t *ts;

	lba = 0;
	ds = 0;	/* directory relative sector */
	if (0 == ddsl) {
		newdos_find_directory(fp);
	}
	
	info(0,"FILENAME/EXT          EOF LRL  RECS GRANS SIUEC....UAL FL01 LUMP/GRAN/COUNT\n");
	/* translate NEWDOS80 lump and gran into real trk, sec */
	lump = ddsl;
	gran = 0;
	/*
	 * lba = (lump * granules_per_lump + gran) *
	 *       sectors_per_granule + t0s;
	 */
	lba = ((lump * gpl) + gran) * spg + t0s;

	while (0 == newdos_get_lba_sector(fp, lba, buff, &trk, &side, &sec)) {
		ts = &track_stat[trk][side];
		for (id = 0; id < ts->sector_count; id++)
			if (ts->sector[id].r == sec)
				break;
		if (!(ts->sector[id].flags & CHRN_DAM_DELETED))
			break;
		switch (ds) {
		case 0:	/* GAT mask sector */
			memcpy(gat, buff, 256);
			ds++;
			break;
		case 1:	/* HIT sector */
			memcpy(hit, buff, 256);
			ds++;
			break;
		default:
			memcpy(dir, buff, 256);
			newdos_list_entries(fp, gat, hit, dir);
			ds++;
			break;
		}
		lba++;
	}
	return 0;
}

int main(int argc, char **argv)
{
	FILE *fp;
	int i;

	if (argc < 2) {
		printf("usage: %s [options] dmk-file\n", argv[0]);
		printf("options can be one or more of:\n");
		printf("-gpl n\tset granules per lump (def.: %d)\n", gpl);
		printf("-ddsl n\tdisk directory start lump\n");
		printf("-ddga n\tdisk directory granule allocation\n");
		printf("-t0 n\tset sectors in track 0 (def.: %d)\n", t0s);
		printf("-f\tfiltered output of BAS/CMD/ASM files\n");
		printf("-v\tbe (more) verbose\n");
		printf("-V\tverify file contents format (BAS,CMD)\n");
		printf("-x dir\texport files to directory dir\n");
		exit(1);
	}
	i = 1;
	while (argv[i][0] == '-') {
		if (!strcmp(argv[i], "-gpl")) {
			++i;
			gpl = strtoul(argv[i], NULL, 0);
		} else if (!strcmp(argv[i], "-spg")) {
			++i;
			spg = strtoul(argv[i], NULL, 0);
		} else if (!strcmp(argv[i], "-ddsl")) {
			++i;
			ddsl = strtoul(argv[i], NULL, 0);
		} else if (!strcmp(argv[i], "-ddga")) {
			++i;
			ddga = strtoul(argv[i], NULL, 0);
		} else if (!strcmp(argv[i], "-t0")) {
			++i;
			t0s = strtoul(argv[i], NULL, 0);
		} else if (!strcmp(argv[i], "-f")) {
			filter++;
		} else if (!strcmp(argv[i], "-V")) {
			verify++;
		} else if (!strcmp(argv[i], "-v")) {
			verbose++;
		} else if (!strcmp(argv[i], "-x")) {
			struct stat st;
			++i;
			export = strdup(argv[i]);
			if (0 != stat(export, &st))
				mkdir(export, 0777);
		}
		++i;
	}

	fp = fopen(argv[i], "r+b");
	if (NULL == fp) {
		fp = fopen(argv[i], "rb");
		if (NULL == fp) {
			perror(argv[1]);
			return 1;
		}
	}

	if (0 != dmk_init(fp))
		return 1;

	info(1,"success: tracks %d, trklen: %d (0x%04x)\n",
		TRACKS, TRACK_LEN, TRACK_LEN);

	dmk_scan_tracks(fp);

	dmk_list_directory(fp);

	return 0;
}
