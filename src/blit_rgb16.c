/******************************************************************************
 *
 *	RGB16 source
 *
 ******************************************************************************/
#define	SRC_RGB16_INIT do {			\
	soff = (sy + y) * sstride + sx * 6;	\
} while (0)

#define	SRC_RGB16_NEXT do {			\
	soff += 6;				\
} while (0)

#undef	SRC_R
#undef	SRC_G
#undef	SRC_B

#define	SRC_R src[soff + 0]
#define	SRC_G src[soff + 2]
#define	SRC_B src[soff + 4]

/**
 * @brief bit block transfer from rgb16 source to gray1 destination
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
int blit_rgb16_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_GRAY1_INIT;
		for (x = 0; x < w; x++) {
			sacc = CCIR_709(SRC_R, SRC_G, SRC_B) >> 7;
			SRC_RGB16_NEXT;
			sacc = sacc << (7 - dbit);
			dacc = (dacc & ~mask) | (sacc & mask);
			DST_GRAY1_NEXT;
		}
		DST_GRAY1_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from rgb16 source to gray2 destination
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
int blit_rgb16_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_GRAY2_INIT;
		for (x = 0; x < w; x++) {
			sacc = CCIR_709(SRC_R, SRC_G, SRC_B) >> 6;
			SRC_RGB16_NEXT;
			sacc = sacc << (6 - dbit);
			dacc = (dacc & ~mask) | (sacc & mask);
			DST_GRAY2_NEXT;
		}
		DST_GRAY2_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from rgb16 source to gray4 destination
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
int blit_rgb16_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_GRAY4_INIT;
		for (x = 0; x < w; x++) {
			sacc = CCIR_709(SRC_R, SRC_G, SRC_B) >> 4;
			SRC_RGB16_NEXT;
			sacc = sacc << (4 - dbit);
			dacc = (dacc & ~mask) | (sacc & mask);
			DST_GRAY4_NEXT;
		}
		DST_GRAY4_LAST;
	}
	return 0;
}


/**
 * @brief bit block transfer from rgb16 source to gray8 destination
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
int blit_rgb16_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_GRAY8_INIT;
		for (x = 0; x < w; x++) {
			sacc = CCIR_709(SRC_R, SRC_G, SRC_B);
			SRC_RGB16_NEXT;
			dacc = sacc & mask;
			DST_GRAY8_NEXT;
		}
	}
	return 0;
}



/**
 * @brief bit block transfer from rgb16 source to gray16 destination
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
int blit_rgb16_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_GRAY16_INIT;
		for (x = 0; x < w; x++) {
			sacc = CCIR_709(SRC_R, SRC_G, SRC_B) << 8;
			SRC_RGB16_NEXT;
			dacc = sacc & mask;
			DST_GRAY16_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from rgb16 source to rgb8 destination
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
int blit_rgb16_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_RGB8_INIT;
		for (x = 0; x < w; x++) {
			sacc = RGB(SRC_R, SRC_G, SRC_B);
			SRC_RGB16_NEXT;
			dacc = sacc & mask;
			DST_RGB8_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from rgb16 source to rgb16 destination
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
int blit_rgb16_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_RGB16_INIT;
		for (x = 0; x < w; x++) {
			sacc = RGB(SRC_R, SRC_G, SRC_B);
			SRC_RGB16_NEXT;
			dacc = sacc & mask;
			DST_RGB16_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from rgb16 source to gray1 paletteized destination
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
int blit_rgb16_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t))
{
	int x, y, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_PAL1_INIT;
		for (x = 0; x < w; x++) {
			sacc = RGB(SRC_R, SRC_G, SRC_B);
			SRC_RGB16_NEXT;
			sacc = find_palette(cookie, 2, sacc) << (7 - dbit);
			dacc = (dacc & ~mask) | (sacc & mask);
			DST_PAL1_NEXT;
		}
		DST_PAL1_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from rgb16 source to gray2 paletteized destination
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
int blit_rgb16_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t))
{
	int x, y, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_PAL2_INIT;
		for (x = 0; x < w; x++) {
			sacc = RGB(SRC_R, SRC_G, SRC_B);
			SRC_RGB16_NEXT;
			sacc = find_palette(cookie, 4, sacc) << (6 - dbit);
			dacc = (dacc & ~mask) | (sacc & mask);
			DST_PAL2_NEXT;
		}
		DST_PAL2_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from rgb16 source to gray4 paletteized destination
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
int blit_rgb16_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t))
{
	int x, y, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_PAL4_INIT;
		for (x = 0; x < w; x++) {
			sacc = RGB(SRC_R, SRC_G, SRC_B);
			SRC_RGB16_NEXT;
			sacc = find_palette(cookie, 16, sacc) << (4 - dbit);
			dacc = (dacc & ~mask) | (sacc & mask);
			DST_PAL4_NEXT;
		}
		DST_PAL4_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from rgb16 source to 8bpp paletteized destination
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
int blit_rgb16_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t))
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_PAL8_INIT;
		for (x = 0; x < w; x++) {
			sacc = RGB(SRC_R, SRC_G, SRC_B);
			SRC_RGB16_NEXT;
			sacc = find_palette(cookie, 256, sacc);
			dacc = sacc & mask;
			DST_PAL8_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from rgb16 source to gray8 + alpha destination
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
int blit_rgb16_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_GRAYA8_INIT;
		for (x = 0; x < w; x++) {
			sacc = CCIR_709(SRC_R, SRC_G, SRC_B);
			SRC_RGB16_NEXT;
			dacc = sacc & mask;
			DST_GRAYA8_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from rgb16 source to gray16 + alpha destination
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
int blit_rgb16_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_GRAYA16_INIT;
		for (x = 0; x < w; x++) {
			sacc = CCIR_709(SRC_R, SRC_G, SRC_B) << 8;
			SRC_RGB16_NEXT;
			dacc = sacc & mask;
			DST_GRAYA16_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from rgb16 source to rgb8 + alpha destination
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
int blit_rgb16_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_RGBA8_INIT;
		for (x = 0; x < w; x++) {
			sacc = RGB(SRC_R, SRC_G, SRC_B);
			SRC_RGB16_NEXT;
			dacc = sacc & mask;
			DST_RGBA8_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from rgb16 source to rgb16 + alpha destination
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
int blit_rgb16_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	/* clipping to destination dimensions */
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_RGB16_INIT;
		DST_RGBA16_INIT;
		for (x = 0; x < w; x++) {
			sacc = RGB(SRC_R, SRC_G, SRC_B);
			SRC_RGB16_NEXT;
			dacc = sacc & mask;
			DST_RGBA16_NEXT;
		}
	}
	return 0;
}
