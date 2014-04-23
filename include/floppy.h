/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * floppy.h	Floppy disk drive emulation
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_FLOPPY_H_INCLUDED_)
#define _FLOPPY_H_INCLUDED_

#include "system.h"
#include "image.h"
#include "crc.h"

#define	DEN_FM_LO	0
#define	DEN_FM_HI	1
#define	DEN_MFM_LO	2
#define	DEN_MFM_HI	3

#define	FDD_FORMAT_JV1	1
#define	FDD_FORMAT_JV3	3
#define	FDD_FORMAT_DMK	4

/* floppy disk drive cylinde/head/record/length and ID structure */
typedef struct fdd_chrn_id_s {
	uint8_t C;
	uint8_t H;
	uint8_t R;
	uint8_t N;
	uint8_t data_id;
	uint8_t flags;
#define	ID_FLAG_DELETED_DATA 0x01
#define	ID_FLAG_CRC_ERROR_IN_ID_FIELD 0x02
#define	ID_FLAG_CRC_ERROR_IN_DATA_FIELD 0x04
#define	ID_FLAG_AM_MISSING 0x08
}	fdd_chrn_id_t;

uint32_t fdd_get_datarate_in_us(uint32_t density);

/* Get/set the deleted data address mark flag bits */
void fdd_set_ddam(struct img_s *img, uint32_t lba, int state);
int fdd_get_ddam(struct img_s *img, uint32_t lba);

int fdd_seek(struct img_s *img, int direction);

int fdd_get_next_id(struct img_s *img, uint32_t head, fdd_chrn_id_t *id, uint32_t den);

int fdd_read_track(struct img_s *img, uint32_t cyl, uint32_t head, void *buff, size_t size, uint32_t den);

int fdd_write_track(struct img_s *img, uint32_t cyl, uint32_t head, void *buff, size_t size, uint32_t den);

int fdd_read_sector_data(struct img_s *img, uint32_t head, uint32_t sector,
	void *buff, size_t size, fdd_chrn_id_t *id, uint32_t den);

int fdd_write_sector_data(struct img_s *img, uint32_t head, uint32_t sector,
	void *buff, size_t size, uint32_t den, uint32_t ddam);

int fdd_find_format(struct img_s *img);

#endif	/* !defined(_FLOPPY_H_INCLUDED_) */
