/***************************************************************************************
 *
 * osd_font.h	Operating System font handling
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 * Copyright 2015 by Andreas Müller <schnitzeltony@googlemail.com>
 *
 ***************************************************************************************/
#if !defined(_OSD_FONT_H_INCLUDED_)
#define	_OSD_FONT_H_INCLUDED_

#include "system.h"
#include "osd_bitmap.h"

/** @brief Defines the osd_font_depth type */
typedef enum {
	FONT_DEPTH_1BPP = 1,
	FONT_DEPTH_2BPP = 2,
	FONT_DEPTH_4BPP = 4,
	FONT_DEPTH_8BPP = 8
} osd_font_depth;


/** @brief Defines the osd_font_t type */
typedef struct osd_font_s {
	/** @brief 8bit pixel field rendered to (osd_render_font) or from (osd_font_blit) */
	uint8_t* pixels;
	/** @brief count characters */
	uint32_t count;
	/** @brief character width */
	uint32_t width;
	/** @brief character height */
	uint32_t height;
	/** @brief bits per font pixel */
	uint32_t bpp;
} osd_font_t;

extern int32_t osd_font_alloc(osd_font_t **pfont, 
	int32_t count, int32_t width, int32_t height, osd_font_depth depth);

extern void osd_font_free(osd_font_t **pfont);

extern int32_t osd_render_font(osd_font_t *font, const void *data,
	uint32_t code, uint32_t count, uint32_t aligned_top);

extern int32_t osd_font_blit(osd_font_t *font, osd_bitmap_t *dst, uint32_t *colors,
	uint32_t x, uint32_t y, uint32_t code, uint32_t scale, uint32_t drawbackground);

#endif	/* !defined(_OSD_FONT_H_INCLUDED_) */
