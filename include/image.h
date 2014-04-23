/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * image.h	Image file handling
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_IMAGE_H_INCLUDED_)
#define _IMAGE_H_INCLUDED_

#include "system.h"
#include <unistd.h>

#define	INVALID		((uint32_t)-1)

/* Forward declaration of the opaque type struct img_s */
struct img_s;

/* Initialize an image handle for a major/minor type file */
struct img_s *img_file(uint32_t major, uint32_t minor);

/* Return the major type of an image */
uint32_t img_major(struct img_s *img);

/* Return the minor type of an image */
uint32_t img_minor(struct img_s *img);

typedef enum {
	IMG_EXISTS,
	IMG_FORMAT,
	DRV_READY,
	DRV_MOTOR_ON,
	DRV_HEAD_AT_TRACK_0,
	DRV_DISK_WRITE_PROTECTED,
	DRV_INDEX,
	DRV_TOTAL_CYLINDERS,
	DRV_TOTAL_HEADS,
	DRV_CURRENT_CYLINDER,
	DRV_SECTORS_PER_TRACK,
	DRV_FIRST_SECTOR_ID,
	DRV_SECTOR_LENGTH,
	DRV_HEADER_LENGTH,
	DRV_ID_INDEX,
	DRV_SEEK
}	IMG_FLAGS;

typedef enum {
	IMG_TYPE_ROM,	/* ROM image */
	IMG_TYPE_BIN,	/* generic binary image */
	IMG_TYPE_TXT,	/* generic text image */
	IMG_TYPE_CAS,	/* cassette image */
	IMG_TYPE_COM,	/* communications (UART or modem) image */
	IMG_TYPE_LPT,	/* line printer image */
	IMG_TYPE_FD,	/* floppy disk image */
	IMG_TYPE_HD,	/* hard disk image */
	IMG_TYPE_MAX
}	IMG_TYPES;

size_t img_get_size(struct img_s *img);

/* max. number of different user data pointers associated with an image */
#define	IMG_MAX_DATA	8
uint32_t img_get_data(struct img_s *img, uint32_t type, void **pdata);
uint32_t img_set_data(struct img_s *img, uint32_t type, void *data);

uint32_t img_get_flag(struct img_s *img, IMG_FLAGS flag);
uint32_t img_set_flag(struct img_s *img, IMG_FLAGS flag, uint32_t state);

uint32_t img_set_callback(struct img_s *img, IMG_FLAGS flag,
	void (*callback)(struct img_s *img, uint32_t param));

void img_set_geometry(struct img_s *img,
	uint32_t cylinders, uint32_t heads, uint32_t sectors_per_track,
	uint32_t sector_length, uint32_t sector0, uint32_t uh, int ah);

struct img_s *img_fopen(const char *filename, uint32_t major, const char *mode);
void img_fclose(struct img_s *img);

/* rename an image to filename.BAK, unlinking an existing backup */
int img_backup(const char *filename, uint32_t major);

off_t img_fseek(struct img_s *img, off_t offs, int whence);
size_t img_fread(struct img_s *img, void *buff, size_t size);
size_t img_fwrite(struct img_s *img, void *buff, size_t size);
size_t img_fprintf(struct img_s *img, const char *fmt, ...);

uint32_t img_read(struct img_s *img, off_t offs, void *buff, size_t size);
uint32_t img_write(struct img_s *img, off_t offs, void *buff, size_t size);

#endif	/* !defined(_IMAGE_H_INCLUDED_) */
