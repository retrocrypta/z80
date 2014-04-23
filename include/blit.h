/*****************************************************************************
 *
 * png.h	Blitter functions
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 *****************************************************************************/
#if !defined(_BLIT_H_INCLUDED_)
#define	_BLIT_H_INCLUDED_

#include "system.h"

/******************************************************************************
 *
 *	GRAY1 source
 *
 ******************************************************************************/
extern int blit_gray1_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray1_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	GRAY2 source
 *
 ******************************************************************************/
extern int blit_gray2_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray2_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	GRAY4 source
 *
 ******************************************************************************/
extern int blit_gray4_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray4_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	GRAY8 source
 *
 ******************************************************************************/
extern int blit_gray8_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray8_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	GRAY16 source
 *
 ******************************************************************************/
extern int blit_gray16_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_gray16_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	PAL1 source
 *
 ******************************************************************************/
extern int blit_pal1_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal1_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal1_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal1_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal1_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal1_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal1_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal1_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal1_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal1_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal1_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal1_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal1_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal1_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal1_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	PAL2 source
 *
 ******************************************************************************/
extern int blit_pal2_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal2_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal2_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal2_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal2_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal2_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal2_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal2_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal2_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal2_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal2_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal2_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal2_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal2_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal2_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	PAL4 source
 *
 ******************************************************************************/
extern int blit_pal4_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal4_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal4_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal4_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal4_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal4_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal4_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal4_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal4_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal4_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal4_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal4_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal4_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal4_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal4_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	PAL8 source
 *
 ******************************************************************************/
extern int blit_pal8_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal8_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal8_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal8_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal8_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal8_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal8_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal8_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal8_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal8_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal8_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_pal8_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal8_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal8_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_pal8_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	GRAY8 + ALPHA source
 *
 ******************************************************************************/
extern int blit_graya8_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya8_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	GRAY16 + ALPHA source
 *
 ******************************************************************************/
extern int blit_graya16_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_graya16_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	RGB8 source
 *
 ******************************************************************************/
extern int blit_rgb8_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb8_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb8_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb8_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb8_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb8_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb8_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb8_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgb8_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgb8_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgb8_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgb8_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb8_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb8_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb8_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	RGB16 source
 *
 ******************************************************************************/
extern int blit_rgb16_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb16_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb16_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb16_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb16_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb16_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb16_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb16_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgb16_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgb16_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgb16_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgb16_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb16_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb16_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgb16_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	RGBA8 source
 *
 ******************************************************************************/
extern int blit_rgba8_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba8_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba8_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba8_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba8_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba8_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba8_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba8_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgba8_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgba8_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgba8_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgba8_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba8_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba8_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba8_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

/******************************************************************************
 *
 *	RGBA16 source
 *
 ******************************************************************************/
extern int blit_rgba16_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba16_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba16_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba16_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba16_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba16_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba16_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba16_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgba16_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgba16_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgba16_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t));

extern int blit_rgba16_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba16_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba16_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

extern int blit_rgba16_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha);

#endif	/* !defined(_BLIT_H_INCLUDED_) */
