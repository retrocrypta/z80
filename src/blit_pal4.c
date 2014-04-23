/******************************************************************************
 *
 *	PAL4 source
 *
 ******************************************************************************/
#define	SRC_PAL4_INIT do {			\
	soff = (sy + y) * sstride + sx / 2;	\
	sbit = (sx & 1) << 2;			\
	sacc = src[soff] << sbit;		\
	soff++;					\
} while (0)

#define	SRC_PAL4_NEXT do {			\
	if (sbit == 4) {			\
		sbit = 0;			\
		sacc = src[soff];		\
		soff++;				\
	} else {				\
		sbit += 4;			\
		sacc <<= 4;			\
	}					\
} while (0)

#undef	SRC_IDX

#define	SRC_IDX colors[(sacc >> 4) & 15]

/**
 * @brief bit block transfer from gray8 source to gray1 destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_GRAY1_INIT;
		for (x = 0; x < w; x++) {
			dacc = (dacc & ~mask) | ((SRC_IDX << (7-dbit)) & mask);
			SRC_PAL4_NEXT;
			DST_GRAY1_NEXT;
		}
		DST_GRAY1_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from gray8 source to gray2 destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_GRAY2_INIT;
		for (x = 0; x < w; x++) {
			dacc = (dacc & ~mask) | ((SRC_IDX << (6-dbit)) & mask);
			SRC_PAL4_NEXT;
			DST_GRAY2_NEXT;
		}
		DST_GRAY2_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from gray8 source to gray4 destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_GRAY4_INIT;
		for (x = 0; x < w; x++) {
			dacc = (dacc & ~mask) | ((SRC_IDX << (4-dbit)) & mask);
			SRC_PAL4_NEXT;
			DST_GRAY4_NEXT;
		}
		DST_GRAY4_LAST;
	}
	return 0;
}


/**
 * @brief bit block transfer from gray8 source to gray8 destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_GRAY8_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_IDX & mask;
			DST_GRAY8_NEXT;
			SRC_PAL4_NEXT;
		}
	}
	return 0;
}



/**
 * @brief bit block transfer from gray8 source to gray16 destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_GRAY16_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_IDX & mask;
			DST_GRAY16_NEXT;
			SRC_PAL4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray8 source to rgb8 destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_RGB8_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_IDX & mask;
			DST_RGB8_NEXT;
			SRC_PAL4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray8 source to gray16 RGB destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_RGB16_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_IDX & mask;
			DST_RGB16_NEXT;
			SRC_PAL4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray8 source to gray1 paletteized destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t))
{
	int x, y, sbit, dbit, n;
	uint32_t soff, doff, sacc, dacc, mask;
	uint32_t lut[16];

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (n = 0; n < 16; n++)
		lut[n] = find_palette(cookie, 2, colors[n]);
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_PAL1_INIT;
		for (x = 0; x < w; x++) {
			dacc = (dacc & ~mask) | ((lut[SRC_IDX] << (7-dbit)) & mask);
			SRC_PAL4_NEXT;
			DST_PAL1_NEXT;
		}
		DST_PAL1_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from gray8 source to gray2 paletteized destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t))
{
	int x, y, sbit, dbit, n;
	uint32_t soff, doff, sacc, dacc, mask;
	uint32_t lut[16];

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (n = 0; n < 16; n++)
		lut[n] = find_palette(cookie, 4, colors[n]);
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_PAL2_INIT;
		for (x = 0; x < w; x++) {
			dacc = (dacc & ~mask) | ((lut[SRC_IDX] << (6-dbit)) & mask);
			SRC_PAL4_NEXT;
			DST_PAL2_NEXT;
		}
		DST_PAL2_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from gray8 source to gray4 paletteized destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t))
{
	int x, y, sbit, dbit, n;
	uint32_t soff, doff, sacc, dacc, mask;
	uint32_t lut[16];

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (n = 0; n < 16; n++)
		lut[n] = find_palette(cookie, 16, colors[n]);
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_PAL4_INIT;
		for (x = 0; x < w; x++) {
			dacc = (dacc & ~mask) | ((lut[SRC_IDX] << (4-dbit)) & mask);
			SRC_PAL4_NEXT;
			DST_PAL4_NEXT;
		}
		DST_PAL4_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from gray8 source to 8bpp paletteized destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t))
{
	int x, y, sbit, n;
	uint32_t soff, doff, sacc, dacc, mask;
	uint32_t lut[16];

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (n = 0; n < 16; n++)
		lut[n] = find_palette(cookie, 256, colors[n]);
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_PAL8_INIT;
		for (x = 0; x < w; x++) {
			dacc = lut[SRC_IDX] & mask;
			DST_PAL8_NEXT;
			SRC_PAL4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray8 source to gray8 + alpha destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_GRAYA8_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_IDX & mask;
			DST_GRAYA8_NEXT;
			SRC_PAL4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray8 source to  + alpha destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_GRAYA16_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_IDX & mask;
			DST_GRAYA16_NEXT;
			SRC_PAL4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray8 source to rgb8 + alpha destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_RGBA8_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_IDX & mask;
			DST_RGBA8_NEXT;
			SRC_PAL4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray8 source to rgb16 + alpha destination
 *
 * @param dst destination memory
 * @param dx destination x
 * @param dy destination y
 * @param dstride destination memory stride per scanline
 * @param src source memory
 * @param sx source x
 * @param sy source y
 * @param sstride source memory stride per scanline
 * @param w width
 * @param h height
 */
int blit_pal4_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL4_INIT;
		DST_RGBA16_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_IDX & mask;
			DST_RGBA16_NEXT;
			SRC_PAL4_NEXT;
		}
	}
	return 0;
}
