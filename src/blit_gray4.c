/******************************************************************************
 *
 *	GRAY4 source
 *
 ******************************************************************************/
#define	SRC_GRAY4_INIT do {			\
	soff = (sy + y) * sstride + sx / 2;	\
	sbit = (sx & 1) << 2;			\
	sacc = src[soff] << sbit;		\
	soff++;					\
} while (0)

#define	SRC_GRAY4_NEXT do {			\
	if (sbit == 4) {			\
		sbit = 0;			\
		sacc = src[soff];		\
		soff++;				\
	} else {				\
		sbit += 4;			\
		sacc <<= 4;			\
	}					\
} while (0)

#undef	SRC_GRAY

#define	SRC_GRAY colors[(sacc >> 4) & 15]

/**
 * @brief bit block transfer from gray4 source to gray1 destination
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
int blit_gray4_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_GRAY1_INIT;
		for (x = 0; x < w; x++) {
			dacc = (dacc & ~mask) | ((SRC_GRAY << (7-dbit)) & mask);
			SRC_GRAY4_NEXT;
			DST_GRAY1_NEXT;
		}
		DST_GRAY1_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from gray4 source to gray2 destination
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
int blit_gray4_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_GRAY2_INIT;
		for (x = 0; x < w; x++) {
			dacc = (dacc & ~mask) | ((SRC_GRAY << (6-dbit)) & mask);
			SRC_GRAY4_NEXT;
			DST_GRAY2_NEXT;
		}
		DST_GRAY2_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from gray4 source to gray4 destination
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
int blit_gray4_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_GRAY4_INIT;
		for (x = 0; x < w; x++) {
			dacc = (dacc & ~mask) | ((SRC_GRAY << (4-dbit)) & mask);
			SRC_GRAY4_NEXT;
			DST_GRAY4_NEXT;
		}
		DST_GRAY4_LAST;
	}
	return 0;
}


/**
 * @brief bit block transfer from gray4 source to gray8 destination
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
int blit_gray4_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_GRAY8_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_GRAY & mask;
			DST_GRAY8_NEXT;
			SRC_GRAY4_NEXT;
		}
	}
	return 0;
}



/**
 * @brief bit block transfer from gray4 source to gray16 destination
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
int blit_gray4_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_GRAY16_INIT;
		for (x = 0; x < w; x++) {
			dacc = colors[(sacc>>6)&1] & mask;
			DST_GRAY16_NEXT;
			SRC_GRAY4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray4 source to rgb8 destination
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
int blit_gray4_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_RGB8_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_GRAY & mask;
			DST_RGB8_NEXT;
			SRC_GRAY4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray4 source to gray16 RGB destination
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
int blit_gray4_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_RGB16_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_GRAY & mask;
			DST_RGB16_NEXT;
			SRC_GRAY4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray4 source to gray1 paletteized destination
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
int blit_gray4_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_PAL1_INIT;
		for (x = 0; x < w; x++) {
			dacc = (dacc & ~mask) | ((SRC_GRAY << (7-dbit)) & mask);
			SRC_GRAY4_NEXT;
			DST_PAL1_NEXT;
		}
		DST_PAL1_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from gray4 source to gray2 paletteized destination
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
int blit_gray4_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_PAL2_INIT;
		for (x = 0; x < w; x++) {
			dacc = (dacc & ~mask) | ((SRC_GRAY << (6-dbit)) & mask);
			SRC_GRAY4_NEXT;
			DST_PAL2_NEXT;
		}
		DST_PAL2_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from gray4 source to gray4 paletteized destination
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
int blit_gray4_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_PAL4_INIT;
		for (x = 0; x < w; x++) {
			dacc = (dacc & ~mask) | ((SRC_GRAY << (4-dbit)) & mask);
			SRC_GRAY4_NEXT;
			DST_PAL4_NEXT;
		}
		DST_PAL4_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from gray4 source to 8bpp paletteized destination
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
int blit_gray4_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_PAL8_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_GRAY & mask;
			DST_PAL8_NEXT;
			SRC_GRAY4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray4 source to gray8 + alpha destination
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
int blit_gray4_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_GRAYA8_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_GRAY & mask;
			DST_GRAYA8_NEXT;
			SRC_GRAY4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray4 source to  + alpha destination
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
int blit_gray4_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_GRAYA16_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_GRAY & mask;
			DST_GRAYA16_NEXT;
			SRC_GRAY4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray4 source to rgb8 + alpha destination
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
int blit_gray4_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_RGBA8_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_GRAY & mask;
			DST_RGBA8_NEXT;
			SRC_GRAY4_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from gray4 source to rgb16 + alpha destination
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
int blit_gray4_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, sbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = gray4_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_GRAY4_INIT;
		DST_RGBA16_INIT;
		for (x = 0; x < w; x++) {
			dacc = SRC_GRAY & mask;
			DST_RGBA16_NEXT;
			SRC_GRAY4_NEXT;
		}
	}
	return 0;
}
