/*****************************************************************************
 *
 *  osd_font.c	Operating System font handling
 *
 * Copyright Juergen Buchmueller <pullmoll@t-online.de>
 * Copyright 2015 Andreas Müller <schnitzeltony@googlemail.com>
 *
 *****************************************************************************/
#include "osd_font.h"

/**
 * @brief allocate a font handle
 *
 * @param pfont pointer to the osd_font_t structure to fill
 * @param count maximum number of characters
 * @param width width of the font in pixels
 * @param height height of the font in pixels
 * @param depth color depth in bits when rendering (2/4/16/256 colors)
 * @result returns 0 on success, -1 on error
 */
int32_t osd_font_alloc(osd_font_t **pfont, 
	int32_t count, int32_t width, int32_t height, osd_font_depth depth)
{
	osd_font_t *font;

	if (*pfont)
		osd_font_free(pfont);

	font = calloc(1, sizeof(osd_font_t));
	if (NULL == font)
		osd_die("calloc(%s,%d) failed\n", "font", sizeof(osd_font_t));
	
	font->pixels = calloc(width * height, count);
	if (NULL == font->pixels)
		osd_die("calloc(%s,%d) failed\n", "font pixels", width * height);
	
	font->count = count;
	font->width = width;
	font->height = height;
	font->bpp = (uint32_t)depth;
	
	*pfont = font;
	return 0;
}

/**
 * @brief free a font handle
 *
 * @param pfont pointer to the osd_font_t structure to free
 */
void osd_font_free(osd_font_t **pfont)
{
	osd_font_t *font;

	if(NULL == pfont)
		return;
	if(NULL == *pfont)
		return;
	font = *pfont;
	
	free(font->pixels);
	free(font);
	*pfont = NULL;
}


static __inline int get_pixel_1bpp(const void *data, uint32_t stride, int x, int y)
{
	const uint8_t *pb = data + stride * y + x / 8;
	return (*pb >> (7 - (x % 8))) & 0x01;
}

static __inline int get_pixel_2bpp(const void *data, uint32_t stride, int x, int y)
{
	const uint8_t *pb = data + stride * y + x / 4;
	return (*pb >> (3 - (x % 4))) & 0x03;
}

static __inline int get_pixel_4bpp(const void *data, uint32_t stride, int x, int y)
{
	const uint8_t *pb = data + stride * y + x / 2;
	return (*pb >> (1 - (x % 2))) & 0x0F;
}

static __inline int get_pixel_8bpp(const void *data, uint32_t stride, int x, int y)
{
	const uint8_t *pb = data + stride * y + x;
	return *pb;
}

/**
 * @brief render font data->pixel 
 *
 * @param pfont pointer to the osd_font_t structure to fill
 * @param data font data to render
 * @param code number of the first character to render
 * @param count number of characters to render
 * @param aligned_top = 1: for widths  not multiple of 8: upper bit contain pixels
 * @result returns 0 on success, -1 on error
 */
int32_t osd_render_font(osd_font_t *font, const void *data,
	uint32_t code, uint32_t count, uint32_t aligned_top)
{
	uint8_t *dest, pixel;
	uint32_t ch, x, y;
	uint32_t stride;
	uint32_t xoffset = 0;
	
	if (NULL == font || code+count > font->count)
		return -1;

	stride = 1 + ((font->width-1) / 8);
	dest = font->pixels + code * font->height * font->width;
	if (aligned_top)
		xoffset = (8 - (font->width * font->bpp) % 8) / font->bpp;
	
	/* render font data into our array of pixels (=palette values) */
	switch (font->bpp) {
		case 1:
			for (ch=0; ch<count; ch++) {
				for (y=0; y<font->height; y++) {
					for (x = xoffset; x < font->width + xoffset; x++) {
						pixel = get_pixel_1bpp(data, 
							stride, 
							x, 
							y + ch * font->height);
						*dest = pixel;
						dest++;
					}
				}
			}
			break;
		case 2:
			for (ch=0; ch<count; ch++) {
				for (y=0; y<font->height; y++) {
					for (x = xoffset; x < font->width + xoffset; x++) {
						pixel = get_pixel_2bpp(data, 
							stride, 
							x, 
							y + ch * font->height);
						*dest = pixel;
						dest++;
					}
				}
			}
			break;
		case 4:
			for (ch=0; ch<count; ch++) {
				for (y=0; y<font->height; y++) {
					for (x = xoffset; x < font->width + xoffset; x++) {
						pixel = get_pixel_4bpp(data, 
							stride, 
							x, 
							y + ch * font->height);
						*dest = pixel;
						dest++;
					}
				}
			}
			break;
		case 8:
			for (ch=0; ch<count; ch++) {
				for (y=0; y<font->height; y++) {
					for (x=0; x<font->width; x++) {
						pixel = get_pixel_8bpp(data, 
							stride, 
							x, 
							y + (ch + code) * font->height);
						*dest = pixel;
						dest++;
					}
				}
			}
			break;
	}
	return 0;
}

/**
 * @brief blit character to bitmap 
 *
 * @param font source font
 * @param dst destination bitmap to blit to
 * @param colors which must contain 2^bpp entries first is background
 * @param x position of upper left corner in target bitmap (unscaled)
 * @param y position of upper left corner in target bitmap (unscaled)
 * @param code character number to blit
 * @param scale value > 1 to enlarge character
 * @param drawbackground = 0: dont't draw background
 * @result returns 0 on success, -1 on error
 */
int32_t osd_font_blit(osd_font_t *font, osd_bitmap_t *dst, uint32_t *colors,
	uint32_t x, uint32_t y, uint32_t code, uint32_t scale, uint32_t drawbackground)
{
	SDL_Surface *dst_surface = NULL;
	SDL_Rect dst_rect;
	uint32_t x1, y1;
	uint8_t *src, pixel;
	
	if (NULL == font || 
		NULL == dst || 
		NULL == colors || 
		code >= font->count)
		return -1;
	
	dst_surface = dst->surface;
	if (NULL == dst_surface)
		return -1;
		
	src = font->pixels + code * font->height * font->width;

	/* pixel rect */
	dst_rect.x = x;
	dst_rect.y = y;
	dst_rect.w = scale;
	dst_rect.h = scale;
	for (y1=0; y1<font->height; y1++) {
		for (x1=0; x1<font->width; x1++) {
			pixel = *src;
			if (drawbackground || pixel != 0) {
				SDL_FillRect(dst_surface, &dst_rect, colors[pixel]);
			}
			dst_rect.x += scale;
			src++;
		}
		dst_rect.x -= font->width * scale;
		dst_rect.y += scale;
	}
	/* dirty rect in bitmap */
	dst_rect.x = x;
	dst_rect.y = y;
	dst_rect.w = font->width * scale;
	dst_rect.h = font->height * scale;
	osd_bitmap_dirty(dst, &dst_rect);
	return 0;
}
