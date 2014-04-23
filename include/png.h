/*****************************************************************************
 *
 * pnh.h	Portable Network Graphics functions
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 *****************************************************************************/
#if !defined(_PNG_H_INCLUDED_)
#define	_PNG_H_INCLUDED_

#include "system.h"
#include "blit.h"
#include <zlib.h>

#ifndef	O_BINARY
#define	O_BINARY	0
#endif

#define	COLOR_GRAYSCALE	0
#define	COLOR_RGBTRIPLE	2
#define	COLOR_PALETTE	3
#define	COLOR_GRAYALPHA	4
#define	COLOR_RGBALPHA	6

/** @brief structure of the common part of PNG and MNG context (at least) */
typedef	struct xng_s {
	/** @brief cookie passed down to input or output function */
	void *cookie;

	/** @brief input function to read from e.g. a file */
	int (*input)(void *cookie, uint8_t *data, int size);

	/** @brief output function to write to e.g. a file */
	int (*output)(void *cookie, uint8_t *data, int size);

	/** @brief ISO CRC of current PNG or MNG element */
	uint32_t crc;
}	xng_t;


/** @brief structure of a PNG context */
typedef	struct png_s {
	/** @brief common part of structure shared with MNG */
	xng_t x;

	/** @brief bytes per scanline */
	uint32_t stride;

	/** @brief image width in pixels */
	uint32_t w;

	/** @brief image height in pixels */
	uint32_t h;

	/** @brief image bits per pixel */
	uint32_t bpp;

	/** @brief color mode */
	uint8_t color;

	/** @brief depth of the elements of a color */
	uint8_t depth;

	/** @brief compression type */
	uint8_t compression;

	/** @brief filter method */
	uint8_t filter;

	/** @brief interlaced flag */
	uint8_t interlace;

	/** @brief physical pixel dimension x */
	uint32_t px;

	/** @brief physical pixel dimension y */
	uint32_t py;

	/** @brief unit for the dimensions */
	uint8_t unit;

	/** @brief time stamp C,Y,M,D,H,M,S */
	uint8_t time[7];

	/** @brief size of the sRGB chunk */
	size_t srgb_size;

	/** @brief sRGB data */
	uint8_t srgb[1];

	/** @brief gAMA data (iamge gamma) */
	uint32_t gamma;

	/** @brief size of the background color */
	size_t bkgd_size;

	/** @brief background color */
	uint8_t bkgd[6];

	/** @brief size of the transparency map */
	size_t trns_size;

	/** @brief transparency map */
	uint8_t trns[256];

	/** @brief size of the (compressed) image data */
	size_t isize;

	/** @brief offset into the (compressed) image data while reading */
	size_t ioffs;

	/** @brief compressed image data */
	uint8_t *idat;

	/** @brief offset into the uncompressed image data while reading */
	size_t offs;

	/** @brief image bitmap size in bytes */
	size_t size;

	/** @brief image bitmap data */
	uint8_t *img;

	/** @brief size of the palette (number of entries) */
	size_t pal_size;

	/** @brief palette entries (used only if color mode is COLOR_PALETTE) */
	uint8_t pal[3*256];

	/** @brief comment string */
	char *comment;

	/** @brief author string */
	char *author;

}	png_t;

/** @brief convert a red, green, blue triple to a PNG color word */
#define	PNG_RGB(r,g,b)	((((uint32_t)(uint8_t)(r))<<16)|(((uint32_t)(uint8_t)(g))<<8)|((uint32_t)(uint8_t)(b)))

/** @brief reset the CRC32 */
extern void isocrc_reset(uint32_t *pcrc);

/** @brief append a byte to the CRC32 */
extern void isocrc_byte(uint32_t *pcrc, uint8_t b);

/** @brief append a number of bytes to the CRC32 */
extern void isocrc_bytes(uint32_t *pcrc, uint8_t *buff, size_t size);

/** @brief get the cookie from a xng_t struct */
extern void *xng_get_cookie(void *ptr);

/** @brief write a PNG/MNG size header */
extern int xng_write_size(void *ptr, uint32_t size);

/** @brief read a PNG/MNG size header */
extern int xng_read_size(void *ptr, uint32_t *psize);

/** @brief write a PNG/MNG string */
extern int xng_write_string(void *ptr, const char *src);

/** @brief read a PNG/MNG string */
extern int xng_read_string(void *ptr, char *dst, uint32_t size);

/** @brief write a PNG/MNG array of bytes */
extern int xng_write_bytes(void *ptr, uint8_t *buff, uint32_t size);

/** @brief read a PNG/MNG array of bytes */
extern int xng_read_bytes(void *ptr, uint8_t *buff, uint32_t size);

/** @brief write a PNG/MNG byte */
extern int xng_write_byte(void *ptr, uint8_t b);

/** @brief read a PNG/MNG byte */
extern int xng_read_byte(void *ptr, uint8_t *pbyte);

/** @brief write a PNG/MNG unsigned 16 bit integer */
extern int xng_write_uint16(void *ptr, uint16_t val);

/** @brief read a PNG/MNG unsigned 16 bit integer */
extern int xng_read_uint16(void *ptr, uint16_t *pval);

/** @brief write a PNG/MNG unsigned integer */
extern int xng_write_uint(void *ptr, uint32_t val);

/** @brief read a PNG/MNG unsigned integer */
extern int xng_read_uint(void *ptr, uint32_t *pval);

/** @brief write a PNG/MNG signed 32 bit integer */
extern int xng_write_int(void *ptr, int32_t i);

/** @brief read a PNG/MNG signed 32 bit integer */
extern int xng_read_int(void *ptr, int32_t *pval);

/** @brief write the current PNG/MNG CRC32 */
extern int xng_write_crc(void *ptr);

/** @brief read the PNG/MNG CRC32 and compare with the calculated crc */
extern int xng_read_crc(void *ptr);

/** @brief read a PNG in an PNG or MNG stream */
extern png_t *png_read_stream(void *cookie, int (*input)(void *, uint8_t *,int));

/** @brief read a PNG file and setup a handler to write it on png_finish() */
extern png_t *png_read(const char *filename,
	void *cookie, int (*output)(void *cookie, uint8_t *data, int size));

/** @brief create a fresh PNG context and setup a handler to write it on png_finish() */
extern png_t *png_create(int w, int h, int color, int depth,
	void *cookie, int (*output)(void *cookie, uint8_t *data, int size));

/** @brief set a palette entry */
extern int png_set_palette(png_t *png, int idx, int color);

/** @brief finish a PNG image (read or created) and write to output */
extern int png_finish(png_t *png);

/** @brief finish a PNG image to be written to a MNG stream */
extern int png_finish_mng(png_t *png);

/** @brief discard a PNG image without writing it to an output */
extern int png_discard(png_t *png);

/** @brief put a pixel into a png_t context */
extern int png_put_pixel(png_t *png, int x, int y, int color, int alpha);

/**
 * @brief get a pixel from a png_t context
 *
 * @param png pointer to a png_t context
 * @param x x coordinate of the pixel (0 .. png->w - 1)
 * @param y y coordinate of the pixel (0 .. png->h - 1)
 * @param color pointer to color value to return
 * @param alpha pointer to alpha value to return
 * @result returns 0 on success, -1 on error
 */
extern int png_get_pixel(png_t *png, int x, int y, int *color, int *alpha);

/** @brief bit block transfer from 1bpp grayscale source */
extern int png_blit_from_gray1(png_t *png, int dx, int dy, int sx, int sy,
	int w, int h, uint8_t *src, int stride, uint32_t *colors, int alpha);

/** @brief bit block transfer from 2bpp grayscale source */
extern int png_blit_from_gray2(png_t *png, int dx, int dy, int sx, int sy,
	int w, int h, uint8_t *src, int stride, uint32_t *colors, int alpha);

/** @brief bit block transfer from 4bpp grayscale source */
extern int png_blit_from_gray4(png_t *png, int dx, int dy, int sx, int sy,
	int w, int h, uint8_t *src, int stride, uint32_t *colors, int alpha);

/** @brief bit block transfer from 8bpp grayscale source */
extern int png_blit_from_gray8(png_t *png, int dx, int dy, int sx, int sy,
	int w, int h, uint8_t *src, int stride, uint32_t *colors, int alpha);

/** @brief bit block transfer from 16bpp grayscale source */
extern int png_blit_from_gray16(png_t *png, int dx, int dy, int sx, int sy,
	int w, int h, uint8_t *src, int stride, uint32_t *colors, int alpha);

/** @brief bit block transfer from 1bpp paletteized source (2 colors) */
extern int png_blit_from_pal1(png_t *png, int dx, int dy, int sx, int sy,
	int w, int h, uint8_t *src, int stride, uint32_t *colors, int alpha);

/** @brief bit block transfer from 2bpp paletteized source (4 colors) */
extern int png_blit_from_pal2(png_t *png, int dx, int dy, int sx, int sy,
	int w, int h, uint8_t *src, int stride, uint32_t *colors, int alpha);

/** @brief bit block transfer from 4bpp paletteized source (16 colors) */
extern int png_blit_from_pal4(png_t *png, int dx, int dy, int sx, int sy,
	int w, int h, uint8_t *src, int stride, uint32_t *colors, int alpha);

/** @brief bit block transfer from 8bpp paletteized source (256 colors) */
extern int png_blit_from_pal8(png_t *png, int dx, int dy, int sx, int sy,
	int w, int h, uint8_t *src, int stride, uint32_t *colors, int alpha);

/** @brief bit block transfer from 8bpp grayscale + alpha source */
extern int png_blit_from_gray8a(png_t *png, int dx, int dy, int sx, int sy,
	int w, int h, uint8_t *src, int stride, uint32_t *colors, int alpha);

/** @brief bit block transfer from 16bpp grayscale + alpha source */
extern int png_blit_from_gray16a(png_t *png, int dx, int dy, int sx, int sy,
	int w, int h, uint8_t *src, int stride, uint32_t *colors, int alpha);

/** @brief bit block transfer from rgb8 source (8:8:8 = 24bpp) */
extern int png_blit_from_rgb8(png_t *png, int dx, int dy, int sx, int sy,
	int w, int h, uint8_t *src, int stride, uint32_t *colors, int alpha);

/** @brief bit block transfer from rgba8 source (8:8:8:8 = 32bpp) */
extern int png_blit_from_rgba8(png_t *png, int dx, int dy, int sx, int sy,
	int w, int h, uint8_t *src, int stride, uint32_t *colors, int alpha);

#endif	/* !defined(_PNG_H_INCLUDED_) */
