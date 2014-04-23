/* ed:set tabstop=8 noexpandtab: */
/*****************************************************************************
 *
 * image.c	Image file handling
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 *****************************************************************************/
#include "image.h"

#define	IMG_DEBUG	1

#if	IMG_DEBUG
#define	LL	3
#else
#define	LL	7
#endif

/* Forward declaration of the opaque type img_t */
#define	IMG_TAG 0x484f4654

typedef struct img_s {
	struct img_s *next;
	char filename[FILENAME_MAX];
	uint32_t tag;
	uint32_t major;
	uint32_t minor;
	FILE *fp;
	uint32_t exists;
	uint32_t format;
	uint32_t motor_on;
	uint32_t drive_ready;
	uint32_t write_protect;
	uint32_t sector_length;
	uint32_t header_length;
	uint32_t size;
	uint32_t total_sectors;
	uint32_t total_cylinders;
	uint32_t total_sides;
	uint32_t sectors_per_track;
	uint32_t first_sector_id;
	uint32_t current_cylinder;
	uint32_t index;
	uint32_t id;
	uint32_t last_seek;
	void *data[8];

	void (*drive_ready_callback)(struct img_s *, uint32_t);
}	img_t;

static img_t *images = NULL;

/*****************************************************************************
 * @brief search list of major/minor handles forn an initialized image
 *	major (type) and minor (node) to identify an image
 *
 * Searches the list of known major/minor handles for an already
 * initiliazed image and returns that if found. Otherwise allocates
 * and initializes an image handle of specified major/minor type. 
 *
 * @param major major number of device
 * @param minor minor number of device
 * @result image handle
 *****************************************************************************/

img_t *img_file(uint32_t major, uint32_t minor)
{
	img_t *img;
	struct stat st;

	for (img = images; NULL != img; img = img->next)
		if (major == img->major && minor == img->minor)
			break;

	if (NULL != img)
		return img;

	/* allocate a new img_t */
	img = malloc(sizeof(img_t));
	if (NULL == img) {
		LOG((1,"IMG","out of memory allocating '%d/%d'\n",
			major, minor));
		return NULL;
	}
	memset(img, 0, sizeof(*img));
	img->tag = IMG_TAG;
	img->major = major;
	img->minor = minor;
	switch (img->major) {
	case IMG_TYPE_ROM:
		snprintf(img->filename, sizeof(img->filename),
			"%s/rom/rom%u.img",
			sys_get_name(), img->minor);
		img->exists = (0 == stat(img->filename, &st));
		if (0 == img->exists) {
			snprintf(img->filename, sizeof(img->filename),
				"%s/rom%u.img",
				sys_get_name(), img->minor);
			img->exists = (0 == stat(img->filename, &st));
		}
		break;
	case IMG_TYPE_CAS:
		snprintf(img->filename, sizeof(img->filename),
			"%s/cas/cas%u.img",
			sys_get_name(), img->minor);
		img->exists = (0 == stat(img->filename, &st));
		if (0 == img->exists) {
			snprintf(img->filename, sizeof(img->filename),
				"%s/cas%u.img",
				sys_get_name(), img->minor);
			img->exists = (0 == stat(img->filename, &st));
		}
		break;
	case IMG_TYPE_FD:
		snprintf(img->filename, sizeof(img->filename),
			"%s/fd/fd%u.img",
			sys_get_name(), img->minor);
		img->exists = (0 == stat(img->filename, &st));
		if (0 == img->exists) {
			snprintf(img->filename, sizeof(img->filename),
				"%s/fd%u.img",
				sys_get_name(), img->minor);
			img->exists = (0 == stat(img->filename, &st));
		}
		break;
	case IMG_TYPE_HD:
		snprintf(img->filename, sizeof(img->filename),
			"%s/hd/hd%u.img",
			sys_get_name(), img->minor);
		img->exists = (0 == stat(img->filename, &st));
		if (0 == img->exists) {
			snprintf(img->filename, sizeof(img->filename),
				"%s/hd%u.img",
				sys_get_name(), img->minor);
			img->exists = (0 == stat(img->filename, &st));
		}
		break;
	}
	LOG((1,"IMG","image '%s' setup (exists:%u)\n",
		img->filename, img->exists));

	img->next = images;
	images = img;
	return img;
}

/*****************************************************************************
 * img_major
 * Entry:
 *	img handle
 * Return:
 *	major number
 * Description:
 *	Returns the major number of an image handle
 *****************************************************************************/

uint32_t img_major(img_t *img)
{
	if (NULL == img)
		return INVALID;

	if (img->tag != IMG_TAG)
		return INVALID;

	return img->major;
}

/*****************************************************************************
 * img_minor
 * Entry:
 *	img handle
 * Return:
 *	minor number
 * Description:
 *	Returns the minor number of an image handle
 *****************************************************************************/

uint32_t img_minor(img_t *img)
{
	if (NULL == img)
		return INVALID;

	if (img->tag != IMG_TAG)
		return INVALID;

	return img->minor;
}

/*****************************************************************************
 * img_open
 * Entry:
 *	img handle
 * Return:
 *	Nothing
 * Description:
 *	Opens the file associated with an image, if it is not already
 *	open. Sets some fields of the image structure depending on
 *	previously set values (detects possible geometry if not set).
 *****************************************************************************/

void img_open(img_t *img)
{
	struct stat st;

	if (NULL == img)
		return;

	if (img->tag != IMG_TAG)
		return;

	if (NULL != img->fp)
		return;

	img->fp = fopen(img->filename, "r+b");
	LOG((LL,"IMG","image '%s' %sopen\n",
		img->filename, NULL == img->fp ? "not " : ""));
	if (NULL == img->fp)
		return;

	if (0 != fstat(fileno(img->fp), &st)) {
		LOG((1,"IMG","image '%s' fstat(%d) failed (%s)\n",
			img->filename, fileno(img->fp), strerror(errno)));
		return;
	}
	img->size = st.st_size;

	if (img->major == IMG_TYPE_ROM || img->major == IMG_TYPE_CAS)
		return;

	if (0 == img->total_cylinders &&
	    0 == img->total_sides &&
	    0 == img->sectors_per_track &&
	    0 == img->sector_length) {

		if (strcasecmp(sys_get_name(), "trs80") &&
			strcasecmp(sys_get_name(), "cgenie")) {
			/* commonly used formats (PC, Amstrad) */
			img->sector_length = 512;
			img->first_sector_id = 1;

			img->total_sectors = img->size / img->sector_length;
			/* detect common formats and set geometry */
			switch (img->total_sectors) {
			case 40*1*9:
				img->total_cylinders = 40;
				img->total_sides = 1;
				img->sectors_per_track = 9;
				LOG((LL,"IMG","image '%s' 180K 40/1/9\n",
					img->filename));
				return;
			case 40*2*9:
				img->total_cylinders = 40;
				img->total_sides = 2;
				img->sectors_per_track = 9;
				LOG((LL,"IMG","image '%s' 360K 40/2/9\n",
					img->filename));
				return;
			case 80*2*9:
				img->total_cylinders = 80;
				img->total_sides = 2;
				img->sectors_per_track = 9;
				LOG((LL,"IMG","image '%s' 720K 80/2/9\n",
					img->filename));
				return;
			case 80*2*15:
				img->total_cylinders = 80;
				img->total_sides = 2;
				img->sectors_per_track = 15;
				LOG((LL,"IMG","image '%s' 1.2M 80/2/15\n",
					img->filename));
				return;
			case 80*2*18:
				img->total_cylinders = 80;
				img->total_sides = 2;
				img->sectors_per_track = 18;
				LOG((LL,"IMG","image '%s' 1.44M 80/2/18\n",
					img->filename));
				return;
			}
		} else {
			/* commonly used format (TRS-80, Colour Genie) */
			img->sector_length = 256;
			img->first_sector_id = 0;

			/* header = max lumps * max granules * sectors per lump */
			img->header_length = 192 * 8 * 5;
			img->total_sectors = (img->size - img->header_length) /
				img->sector_length;

			switch (img->total_sectors) {
			case 40*1*10:
				img->total_cylinders = 40;
				img->total_sides = 1;
				img->sectors_per_track = 10;
				LOG((LL,"IMG","image '%s' 40 track SS/SD\n",
					img->filename));
				return;
			case 40*2*10:	/* could as well be 80*1*10 */
				img->total_cylinders = 40;
				img->total_sides = 2;
				img->sectors_per_track = 10;
				LOG((LL,"IMG","image '%s' 40 track DS/SD\n",
					img->filename));
				return;
			case 40*1*18:
				img->total_cylinders = 40;
				img->total_sides = 1;
				img->sectors_per_track = 18;
				LOG((LL,"IMG","image '%s' 40 track SS/DD\n",
					img->filename));
				return;
			case 40*2*18:	/* could as well be 80*1*18 */
				img->total_cylinders = 40;
				img->total_sides = 2;
				img->sectors_per_track = 18;
				LOG((LL,"IMG","image '%s' 40 track DS/DD\n",
					img->filename));
				return;
			case 80*2*10:
				img->total_cylinders = 80;
				img->total_sides = 2;
				img->sectors_per_track = 10;
				LOG((LL,"IMG","image '%s' 80 track DS/SD\n",
					img->filename));
				return;
			case 80*2*18:
				img->total_cylinders = 80;
				img->total_sides = 2;
				img->sectors_per_track = 18;
				LOG((LL,"IMG","image '%s' 80 track DS/DD\n",
					img->filename));
				return;
			}
		}

		img->header_length = 0;
		img->total_cylinders = 0;
		img->total_sides = 0;
		img->sectors_per_track = 0;
		img->sector_length = 0;
	} else {
		if (0 != img->sector_length)
			img->total_sectors = img->size / img->sector_length;
	}
}

/*****************************************************************************
 * img_close
 * Entry:
 *	img handle
 * Return:
 *	Nothing
 * Description:
 *	Closes the file associated with an image.
 *****************************************************************************/

void img_close(img_t *img)
{
	if (NULL == img)
		return;

	if (img->tag != IMG_TAG)
		return;

	if (NULL != img->fp) {
		fclose(img->fp);
		img->fp = NULL;
	}
	LOG((LL,"IMG","image '%s' %s\n", img->filename, "closed"));
}

/*****************************************************************************
 * img_set_geometry
 * Entry:
 *	img handle, geometry parameters
 * Return:
 *	Nothing
 * Description:
 *	Sets all supported geometry parameters through one call.
 *****************************************************************************/

void img_set_geometry(struct img_s *img,
	uint32_t cylinders, uint32_t heads, uint32_t sectors_per_track,
	uint32_t sector_length, uint32_t sector0, uint32_t header_length, int ah)
{
	img_set_flag(img, DRV_TOTAL_CYLINDERS, cylinders);
	img_set_flag(img, DRV_TOTAL_HEADS, heads);
	img_set_flag(img, DRV_SECTORS_PER_TRACK, sectors_per_track);
	img_set_flag(img, DRV_SECTOR_LENGTH, sector_length);
	img_set_flag(img, DRV_FIRST_SECTOR_ID, sector0);
	img_set_flag(img, DRV_HEADER_LENGTH, header_length);
	/* FIXME: what is ah? */
	(void)ah;
}

/*****************************************************************************
 * img_read
 * Entry:
 *	img handle, file offset, buffer, size
 * Return:
 *	number of bytes gotten
 * Description:
 *	Seeks to the specified offset inside an image's file and
 *	tries to read the specified size (bytes) to buffer. The
 *	value returned is the actual number of bytes read.
 *****************************************************************************/

uint32_t img_read(img_t *img, off_t offs, void *buff, size_t size)
{
	size_t done;

	if (NULL == img)
		return 0;

	if (img->tag != IMG_TAG)
		return 0;

	if (NULL == img->fp)
		img_open(img);

	if (NULL == img->fp)
		return 0;

	offs += img->header_length;

	if (0 != fseek(img->fp, offs, SEEK_SET)) {
		LOG((1,"IMG", "seek error '%d/%d' (%s)\n",
			img->major, img->minor, strerror(errno)));
		return 0;
	}
	done = fread(buff, 1, size, img->fp);
	if (size != done) {
		LOG((1,"IMG", "read error '%d/%d' 0x%x of 0x%x @0x%llx (%s)\n",
			img->major, img->minor, done, size, (uint64_t)offs,
			strerror(errno)));
		return 0;
	}
	LOG((LL,"IMG","image '%s' read 0x%x at 0x%llx\n",
		img->filename, done, (uint64_t)offs));
	return done;
}

/*****************************************************************************
 * img_write
 * Entry:
 *	img handle, file offset, buffer, size
 * Return:
 *	number of bytes written
 * Description:
 *	Seeks to the specified offset inside an image's file and
 *	tries to write the specified size (bytes) from buffer. The
 *	value returned is the actual number of bytes written.
 *	If buff is specified as NULL, an amount of 'size' bytes of
 *	zeroes is written to the file.
 *****************************************************************************/

uint32_t img_write(img_t *img, off_t offs, void *buff, size_t size)
{
	void *tempbuff = NULL;
	size_t done;

	if (NULL == img)
		return 0;

	if (img->tag != IMG_TAG)
		return 0;

	if (NULL == img->fp)
		img_open(img);

	if (NULL == img->fp)
		return 0;

	offs += img->header_length;
	if (0 != fseek(img->fp, offs, SEEK_SET)) {
		LOG((1,"IMG", "seek error '%d/%d' (%s)\n",
			img->major, img->minor, strerror(errno)));
		return 0;
	}
	if (NULL == buff) {
		buff = tempbuff = malloc(size);
		if (NULL == buff) {
			LOG((1,"IMG", "memory problem 0x%x temp buffer for '%d/%d' (%s)\n",
				size, img->major, img->minor, strerror(errno)));
		}
		memset(buff, 0, size);
	}
	done = fwrite(buff, 1, size, img->fp);
	if (size != done) {
		LOG((1,"IMG", "write error '%d/%d' 0x%x of 0x%x @0x%llx (%s)\n",
			img->major, img->minor, done, size, (uint64_t)offs,
			strerror(errno)));
	}
	if (NULL != tempbuff)
		free(tempbuff);
	LOG((LL,"IMG","image '%s' write 0x%x @0x%llx\n",
		img->filename, done, (uint64_t)offs));
	return done;
}

/*****************************************************************************
 * img_seek
 * Entry:
 *	img handle, direction
 * Return:
 *	Nothing
 * Description:
 *	Does a logical seek for the current_cylinder field of an image
 *	by adding the value of 'direction' to it and clipping on
 *	both ends (0 and total_cylinders-1).
 *****************************************************************************/

void img_seek(img_t *img, int direction)
{
	if (NULL == img)
		return;

	if (img->tag != IMG_TAG)
		return;

	if (NULL == img->fp)
		img_open(img);

	img->current_cylinder += direction;

	if (img->current_cylinder > 255)
		img->current_cylinder = 255;

	LOG((LL,"IMG","image '%s' seek %+d to track %d\n",
		img->filename, direction, img->current_cylinder));
}

/*****************************************************************************
 * img_fopen
 * Entry:
 *	filename, type of file, open mode
 * Return:
 *	image handle
 * Description:
 *	Scans the list of already opened image handles for the
 *	specified filename and returns that handle if found. Otherwise
 *	opens the specified filename in the specfied mode (mode same
 *	as fopen()), where the default file path is determined by the
 *	type of file through configurable search paths.
 * TODO: configurable path names
 *****************************************************************************/

img_t *img_fopen(const char *filename, uint32_t major, const char *mode)
{
	img_t *img;
	struct stat st;

	for (img = images; NULL != img; img = img->next)
		if (0 == strcmp(filename, img->filename))
			break;

	if (NULL != img)
		return img;

	/* allocate a new img_t */
	img = malloc(sizeof(img_t));
	if (NULL == img) {
		LOG((1,"IMG","out of memory allocating '%s'\n",
			filename));
		return NULL;
	}
	memset(img, 0, sizeof(*img));
	img->tag = IMG_TAG;
	img->major = major;
	img->minor = 0;
	switch (major) {
	case IMG_TYPE_ROM: /* ROM image */
		snprintf(img->filename, sizeof(img->filename),
			"%s/rom/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_BIN: /* binary image */
		snprintf(img->filename, sizeof(img->filename),
			"%s/bin/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_TXT: /* text image */
		snprintf(img->filename, sizeof(img->filename),
			"%s/txt/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_CAS: /* cassette image */
		snprintf(img->filename, sizeof(img->filename),
			"%s/cas/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_COM: /* COM: image */
		snprintf(img->filename, sizeof(img->filename),
			"%s/com/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_LPT: /* LPT: image */
		snprintf(img->filename, sizeof(img->filename),
			"%s/lpt/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_FD: /* floppy disk image */
		snprintf(img->filename, sizeof(img->filename),
			"%s/fd/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_HD: /* hard disk image */
		snprintf(img->filename, sizeof(img->filename),
			"%s/hd/%s", sys_get_name(), filename);
		break;
	default:
		LOG((1,"IMG","file type %d for '%s' not supported\n",
			major, filename));
		free(img);
		return NULL;
	}
	img->exists = (0 == stat(img->filename, &st));
	LOG((1,"IMG","image '%s' setup (exists:%d)\n",
		img->filename, img->exists));
	/* set image size */
	img->size = st.st_size;

	/* open for read only, (not read+write) requires existing image */
	if (mode[0] == 'r' && mode[1] != '+') {
		if (0 == img->exists) {
			free(img);
			return NULL;
		}
	}
	img->fp = fopen(img->filename, mode);
	img->next = images;
	images = img;
	return img;
}

/*****************************************************************************
 * img_fclose
 * Entry:
 *	img handle
 * Return:
 *	Nothing
 * Description:
 *	Close the file associated with an image handle and
 *	removes the handle from the list.
 *****************************************************************************/

void img_fclose(img_t *img)
{
	img_t *i;

	if (NULL == img)
		return;

	if (img->tag != IMG_TAG)
		return;

	if (NULL != img->fp) {
		fclose(img->fp);
		img->fp = NULL;
	}

	if (img == images) {
		images = img->next;
		free(img);
		return;
	}
	for (i = images; NULL != i; i = i->next)
		if (img == i->next)
			break;
	if (NULL != i)
		i->next = i->next->next;
	free(img);
}

/*****************************************************************************
 * img_backup
 * Entry:
 *	filename, major number
 * Return:
 *	zero on success
 * Description:
 *	Rename an existing file to filename.BAK,
 *	unlinking an existing backup.
 *****************************************************************************/

int img_backup(const char *filename, uint32_t major)
{
	char pathname[FILENAME_MAX];
	char backup[FILENAME_MAX];

	switch (major) {
	case IMG_TYPE_ROM: /* ROM image */
		snprintf(pathname, sizeof(pathname),
			"%s/rom/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_BIN: /* binary image */
		snprintf(pathname, sizeof(pathname),
			"%s/bin/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_TXT: /* text image */
		snprintf(pathname, sizeof(pathname),
			"%s/txt/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_CAS: /* cassette image */
		snprintf(pathname, sizeof(pathname),
			"%s/cas/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_COM: /* COM: image */
		snprintf(pathname, sizeof(pathname),
			"%s/com/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_LPT: /* LPT: image */
		snprintf(pathname, sizeof(pathname),
			"%s/lpt/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_FD: /* floppy disk image */
		snprintf(pathname, sizeof(pathname),
			"%s/fd/%s", sys_get_name(), filename);
		break;
	case IMG_TYPE_HD: /* hard disk image */
		snprintf(pathname, sizeof(pathname),
			"%s/hd/%s", sys_get_name(), filename);
		break;
	default:
		LOG((1,"IMG","file type %d for '%s' not supported\n",
			major, filename));
		return -1;
	}

	snprintf(backup, sizeof(backup), "%s.BAK", pathname);
	if (unlink(backup) < 0) {
		LOG((1,"IMG","no backup '%s' found\n", backup));
	}
	if (rename(pathname, backup) < 0) {
		LOG((1,"IMG","could not rename '%s' to '%s'\n", pathname, backup));
		return -1;
	}
	return 0;
}

/*****************************************************************************
 * img_fseek
 * Entry:
 *	img handle, offset, meaning of offset
 * Return:
 *	Nothing
 * Description:
 *	Seeks to the specified offset in the file associated with an
 *	image, just like fseeko() does.
 *****************************************************************************/

off_t img_fseek(img_t *img, off_t offs, int whence)
{
	if (NULL == img)
		return (off_t)INVALID;

	if (NULL == img->fp)
		img_open(img);

	if (NULL == img->fp)
		return (off_t)INVALID;
	return (off_t)fseek(img->fp, offs, whence);
}

/*****************************************************************************
 * img_fread
 * Entry:
 *	img handle, buffer, size
 * Return:
 *	size gotten
 * Description:
 *	Tries to read size bytes from the file handle associated with an
 *	image to buff. Returns the number of bytes actually read.
 *****************************************************************************/

size_t img_fread(img_t *img, void *buff, size_t size)
{
	if (NULL == img)
		return (size_t)INVALID;

	if (NULL == img->fp)
		img_open(img);

	if (NULL == img->fp)
		return (size_t)INVALID;

	return fread(buff, 1, size, img->fp);
}

/*****************************************************************************
 * img_fwrite
 * Entry:
 *	img handle, buffer, size
 * Return:
 *	size written
 * Description:
 *	Tries to read write size bytes from buff to the file handle
 * 	associated with an image. Returns the number of bytes actually read.
 *****************************************************************************/

size_t img_fwrite(img_t *img, void *buff, size_t size)
{
	if (NULL == img)
		return (size_t)INVALID;

	if (NULL == img->fp)
		img_open(img);

	if (NULL == img->fp)
		return (size_t)INVALID;

	return fwrite(buff, 1, size, img->fp);
}

/*****************************************************************************
 * img_fprintf
 * Entry:
 *	img handle, format string, optional arguments
 * Return:
 *	size written
 * Description:
 *	Prints a formatted string to an image file.
 *****************************************************************************/

size_t img_fprintf(img_t *img, const char *fmt, ...)
{
	img_t *i = (img_t *)img;
	va_list ap;
	size_t size;

	if (NULL == i)
		return (size_t)0;

	if (NULL == img->fp)
		img_open(img);

	if (NULL == img->fp)
		return (size_t)0;

	va_start(ap, fmt);
	size = vfprintf(img->fp, fmt, ap);
	va_end(ap);

	return size;
}

size_t img_get_size(img_t *img)
{
	img_t *i = (img_t *)img;

	if (NULL == i)
		return INVALID;

	return img->size;
}

/**
 * @brief img_get_data - return data pointer associated with image
 *
 * Returns a specific data pointer associated with an image.
 * @param img pointer to an image handle
 * @param pdata pointer to a pointer to some data
 * @result 0 on success, or INVALID
 */
uint32_t img_get_data(img_t *img, uint32_t type, void **pdata)
{
	img_t *i = (img_t *)img;

	if (NULL == i)
		return INVALID;

	if (img->tag != IMG_TAG)
		return INVALID;

	if (NULL == pdata)
		return INVALID;

	if (type >= IMG_MAX_DATA)
		return INVALID;

	*pdata = img->data[type];
	return 0;
}

/*****************************************************************************
 * img_set_data
 * Entry:
 *	img handle, data pointer
 * Return:
 *	0 on success, or INVALID
 * Description:
 *	Sets a specific data pointer associated with an image.
 *****************************************************************************/
uint32_t img_set_data(img_t *img, uint32_t type, void *data)
{
	img_t *i = (img_t *)img;

	if (NULL == i)
		return INVALID;

	if (img->tag != IMG_TAG)
		return INVALID;

	if (type >= IMG_MAX_DATA)
		return INVALID;

	img->data[type] = data;
	return 0;
}

/*****************************************************************************
 * img_get_flag
 * Entry:
 *	img handle, flag type
 * Return:
 *	flag value
 * Description:
 *	Returns a specific flag value associated with an image.
 *****************************************************************************/

uint32_t img_get_flag(img_t *img, IMG_FLAGS flag)
{
	img_t *i = (img_t *)img;

	if (NULL == i)
		return 0;

	if (img->tag != IMG_TAG)
		return 0;

	switch (flag) {
	case IMG_EXISTS:
		return img->exists;

	case IMG_FORMAT:
		return img->format;

	case DRV_MOTOR_ON:
		return img->motor_on ? 1 : 0;

	case DRV_READY:
		return img->drive_ready ? 1 : 0;

	case DRV_HEAD_AT_TRACK_0:
		return img->current_cylinder == 0 ? 1 : 0;

	case DRV_DISK_WRITE_PROTECTED:
		return img->write_protect;

	case DRV_INDEX:
		return img->index;

	case DRV_TOTAL_CYLINDERS:
		return img->total_cylinders;

	case DRV_TOTAL_HEADS:
		return img->total_sides;

	case DRV_CURRENT_CYLINDER:
		return img->current_cylinder;

	case DRV_SECTORS_PER_TRACK:
		return img->sectors_per_track;

	case DRV_FIRST_SECTOR_ID:
		return img->first_sector_id;

	case DRV_SECTOR_LENGTH:
		return img->sector_length;

	case DRV_HEADER_LENGTH:
		return img->header_length;

	case DRV_ID_INDEX:
		return img->id;

	case DRV_SEEK:
		return img->last_seek;

	default:
		return 0;
	}
}

/*****************************************************************************
 * img_get_flag
 * Entry:
 *	img handle, flag type, new flag value
 * Return:
 *	old flag value
 * Description:
 *	Set a specific flag value associated with an image and returns
 *	the previous value for convenience.
 *****************************************************************************/

uint32_t img_set_flag(img_t *img, IMG_FLAGS flag, uint32_t new_state)
{
	img_t *i = (img_t *)img;
	uint32_t old_state = 0;

	if (NULL == i)
		return 0;

	if (img->tag != IMG_TAG)
		return 0;

	switch (flag) {
	case IMG_EXISTS:
		old_state = img->exists;
		/* won't set existance flag */
		break;

	case IMG_FORMAT:
		old_state = img->format;
		img->format = new_state;
		break;

	case DRV_READY:
		old_state = img->drive_ready;
		img->drive_ready = new_state;
		if (old_state != new_state && NULL != img->drive_ready_callback)
			(*img->drive_ready_callback)(img, new_state);
		break;

	case DRV_MOTOR_ON:
		old_state = img->motor_on;
		img->motor_on = new_state;
		if (0 != new_state && NULL == img->fp)
			img_open(img);
		break;

	case DRV_HEAD_AT_TRACK_0:
		old_state = img->current_cylinder == 0 ? 1 : 0;
		/* won't set track 0 flag */
		break;

	case DRV_DISK_WRITE_PROTECTED:
		old_state = img->write_protect;
		img->write_protect = new_state;
		break;

	case DRV_INDEX:
		old_state = img->index;
		img->index = new_state;
		break;

	case DRV_TOTAL_CYLINDERS:
		old_state = img->total_cylinders;
		img->total_cylinders = new_state;
		break;

	case DRV_TOTAL_HEADS:
		old_state = img->total_sides;
		img->total_sides = new_state;
		break;

	case DRV_CURRENT_CYLINDER:
		old_state = img->current_cylinder;
		img->current_cylinder = new_state;
		break;

	case DRV_SECTORS_PER_TRACK:
		old_state = img->sectors_per_track;
		img->sectors_per_track = new_state;
		break;

	case DRV_FIRST_SECTOR_ID:
		old_state = img->first_sector_id;
		img->first_sector_id = new_state;
		break;

	case DRV_SECTOR_LENGTH:
		old_state = img->sector_length;
		img->sector_length = new_state;
		break;

	case DRV_HEADER_LENGTH:
		old_state = img->header_length;
		img->header_length = new_state;
		break;

	case DRV_ID_INDEX:
		old_state = img->id;
		img->id= new_state;
		break;

	case DRV_SEEK:
		old_state = img->last_seek;
		img->last_seek = new_state;
		img_seek(img, (int32_t)new_state);
		break;

	default:
		old_state = 0;
	}

	return old_state;
}

/*****************************************************************************
 * img_set_callback
 * Entry:
 *	img handle, flag type, callback
 * Return:
 *	0 on success, else INVALID
 * Description:
 *	Set a callback that will will be called whenever 'flag' is
 *	changed. This is currently supported for DRV_READY only.
 *****************************************************************************/

uint32_t img_set_callback(img_t *img, IMG_FLAGS flag,
	void (*callback)(img_t *, uint32_t state))
{
	if (NULL == img)
		return INVALID;

	if (img->tag != IMG_TAG)
		return INVALID;

	switch (flag) {
	case IMG_EXISTS:
		return INVALID;

	case IMG_FORMAT:
		return INVALID;

	case DRV_READY:
		img->drive_ready_callback = callback;
		return 0;

	case DRV_MOTOR_ON:
		return INVALID;

	case DRV_HEAD_AT_TRACK_0:
		return INVALID;

	case DRV_DISK_WRITE_PROTECTED:
		return INVALID;

	case DRV_INDEX:
		return INVALID;

	case DRV_TOTAL_CYLINDERS:
		return INVALID;

	case DRV_TOTAL_HEADS:
		return INVALID;

	case DRV_CURRENT_CYLINDER:
		return INVALID;

	case DRV_SECTORS_PER_TRACK:
		return INVALID;

	case DRV_FIRST_SECTOR_ID:
		return INVALID;

	case DRV_SECTOR_LENGTH:
		return INVALID;

	case DRV_HEADER_LENGTH:
		return INVALID;

	case DRV_ID_INDEX:
		return INVALID;

	case DRV_SEEK:
		return INVALID;
	}

	return INVALID;
}
