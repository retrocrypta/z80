/*****************************************************************************
 *
 *  osd_bitmap.c Operating System bitmap/surface handling
 *
 * Copyright Juergen Buchmueller <pullmoll@t-online.de>
 * Copyright 2015 Andreas Müller <schnitzeltony@googlemail.com>
 *
 *****************************************************************************/
#include "osd_bitmap.h"

/**
 * @brief allocate a bitmap handle
 *
 * @param bitmap pointer to the osd_bitmap_t structure to fill
 * @param width width of the bitmap in pixels
 * @param height height of the bitmap in pixels
 * @param depth number of bits per pixel (1, 4, 8, 15, 16, 24, 32)
 * @result returns 0 on success, -1 on error
 */
int32_t osd_bitmap_alloc(osd_bitmap_t **pbitmap, 
	SDL_Window *window, int32_t width, int32_t height)
{
	osd_bitmap_t *bitmap;
	SDL_Surface *surfacetmp;
	int32_t rmask, gmask, bmask, amask;

	if (*pbitmap)
		osd_bitmap_free(pbitmap);

	bitmap = calloc(1, sizeof(osd_bitmap_t));
	if (NULL == bitmap)
		osd_die("calloc(%s,%d) failed\n", "bitmap", sizeof(osd_bitmap_t));

	/* the only way to create a window compatible surface is to create a dummy
	 * suface of correct size and convert that to window's format */
#if	(SDL_BYTEORDER==SDL_BIG_ENDIAN)
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
#endif
	surfacetmp = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);
	if (NULL == surfacetmp)
		return -1;
	bitmap->surface = SDL_ConvertSurface(surfacetmp, SDL_GetWindowSurface(window)->format, 0);
	if (NULL == bitmap->surface)
		return -1;
	SDL_FreeSurface(surfacetmp);

	*pbitmap = bitmap;
	return 0;
}

/**
 * @brief allocate a bitmap handle
 *
 * @param bitmap bitmap
 * @param renderer renderer creating a texture for
 */

int32_t osd_bitmap_create_texture(osd_bitmap_t *bitmap, SDL_Renderer *renderer)
{
	if (NULL == bitmap || NULL == bitmap->surface)
		return -1;

	if (bitmap->texture) {
		SDL_DestroyTexture(bitmap->texture);
	}

	bitmap->texture = SDL_CreateTexture(renderer,
		bitmap->surface->format->format, 
		SDL_TEXTUREACCESS_STREAMING, 
		bitmap->surface->w, bitmap->surface->h );

	return 0;
}

/**
 * @brief free a bitmap handle
 *
 * @param bitmap pointer to the osd_bitmap_t structure to free
 */
void osd_bitmap_free(osd_bitmap_t **pbitmap)
{
	osd_bitmap_t *bitmap;

	if (NULL == pbitmap)
		return;
	if (NULL == *pbitmap)
		return;
	bitmap = *pbitmap;
	if (bitmap->dirty) {
		free(bitmap->dirty);
	}
	if (bitmap->surface) {
		SDL_FreeSurface(bitmap->surface);
	}
	if (bitmap->texture) {
		SDL_DestroyTexture(bitmap->texture);
	}
	free(bitmap);
	*pbitmap = NULL;
}

/**
 * @brief render surface to texture
 *
 * @param bitmap pointer to the osd_bitmap_t structure to render
 */
void osd_bitmap_render(osd_bitmap_t *bitmap, SDL_Renderer* renderer)
{
    void *pixels;
    int pitch;
    uint32_t n;
	SDL_Rect rect;
	
	if (NULL == bitmap->texture)
		return;
	
	rect.x = rect.y = 0;
	rect.w = bitmap->surface->w;
	rect.h = bitmap->surface->h;

	SDL_LockTexture(bitmap->texture, &rect, &pixels, &pitch);
	memcpy(pixels, bitmap->surface->pixels, bitmap->surface->pitch * bitmap->surface->h );
	SDL_UnlockTexture(bitmap->texture);

	SDL_RenderCopy(renderer, frame->texture, &rect, &rect);

	/*for (n = 0; n < bitmap->dirty_count; n++) {
			SDL_RenderCopy(renderer, frame->texture, &bitmap->dirty[n], &bitmap->dirty[n]);
	}
	bitmap->dirty_count = 0;*/
}

/**
 * @brief draw a horizontal line of pixels of one color in a bitmap
 *
 * The destination bitmap may be scaled
 *
 * @param bitmap pointer to the bitmap to set a pixel in
 * @param x x coordinate of the vertical line
 * @param y y coordinate of the vertical line
 * @param l length of the line
 * @param w line width
 * @param pixel pixel value
 */
void osd_bitmap_hline(osd_bitmap_t *bitmap, 
	int32_t x, int32_t y, int32_t l, int32_t w, uint32_t pixel)
{
	SDL_Rect dst;

	dst.x = x;
	dst.y = y;
	dst.w = l;
	dst.h = w;
	SDL_FillRect(bitmap->surface, &dst, pixel);
	osd_bitmap_dirty(bitmap, &dst);
}

/**
 * @brief draw a vertical line of pixels of one color in a bitmap
 *
 * The destination bitmap may be scaled
 *
 * @param bitmap pointer to the bitmap to set a pixel in
 * @param x x coordinate of the vertical line
 * @param y y coordinate of the vertical line
 * @param l length of the line
 * @param w line width
 * @param pixel pixel value
 */
void osd_bitmap_vline(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t l, int32_t w, uint32_t pixel)
{
	SDL_Rect dst;

	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = l;
	SDL_FillRect(bitmap->surface, &dst, pixel);
	osd_bitmap_dirty(bitmap, &dst);
}

/**
 * @brief frame a rectangular shape of pixels in a bitmap
 *
 * The destination bitmap may be scaled
 *
 * @param bitmap pointer to the bitmap to set a pixel in
 * @param x left x coordinate of the rectangle
 * @param y top y coordinate of the rectangle
 * @param w width of the rectangle
 * @param h height of the rectangle
 * @param corners 1 draw hard corners
 * @param lw line width of the rectangle
 * @param tl pixel value for the top and left lines
 * @param br pixel value for the bottom and right lines
 */
void osd_bitmap_framerect(osd_bitmap_t *bitmap, 
	int32_t x, int32_t y, int32_t w, int32_t h,
	int32_t corners, uint32_t lw, uint32_t tl, uint32_t br)
{
	if (corners) {
		osd_bitmap_hline(bitmap, x, y, w, lw, tl);
		osd_bitmap_vline(bitmap, x, y, h, lw, tl);
		osd_bitmap_hline(bitmap, x, y + h - lw, w, lw, br);
		osd_bitmap_vline(bitmap, x + w - lw, y, h, lw, br);
	} else {
		osd_bitmap_hline(bitmap, x + lw, y, w - 2 * lw, lw, tl);
		osd_bitmap_vline(bitmap, x, y + lw, h - 2 * lw, lw, tl);
		osd_bitmap_hline(bitmap, x + lw, y + h - lw, w - 2 * lw, lw, br);
		osd_bitmap_vline(bitmap, x + w - lw, y + lw, h - 2 * lw, lw, br);
	}
}

/**
 * @brief fill a rectangular shape of pixels in a bitmap
 *
 * The destination bitmap may be scaled
 *
 * @param bitmap pointer to the bitmap to set a pixel in
 * @param x left x coordinate of the rectangle
 * @param y top y coordinate of the rectangle
 * @param w width of the rectangle
 * @param h height of the rectangle
 * @param pixel pixel value
 */
void osd_bitmap_fillrect(osd_bitmap_t *bitmap, 
	int32_t x, int32_t y, int32_t w, int32_t h, uint32_t pixel)
{
	SDL_Surface *dst_surface = NULL;
	SDL_Rect dst;

	dst_surface = (SDL_Surface *)bitmap->surface;
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;
	SDL_FillRect(dst_surface, &dst, pixel);
	osd_bitmap_dirty(bitmap, &dst);
}

uint32_t oss_bitmap_get_pix_val(osd_bitmap_t *bitmap, 
	uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return SDL_MapRGBA(bitmap->surface->format, r, g, b, a);
}
