/******************************************************************************
 *
 *	PAL8 source
 *
 ******************************************************************************/
#define	SRC_PAL8_INIT do {			\
	soff = (sy + y) * sstride + sx;		\
} while (0)

#define	SRC_PAL8_NEXT do {			\
	soff++;					\
} while (0)

#undef	SRC_IDX

#define	SRC_IDX src[soff]

/**
 * @brief bit block transfer from pal8 source to gray1 destination
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
int blit_pal8_to_gray1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_GRAY1_INIT;
		for (x = 0; x < w; x++) {
			sacc = colors[SRC_IDX];
			sacc = CCIR_709(MSB_R(sacc), MSB_G(sacc), MSB_B(sacc)) >> 7;
			sacc = sacc << (7 - dbit);
			dacc = (dacc & ~mask) | (sacc & mask);
			SRC_PAL8_NEXT;
			DST_GRAY1_NEXT;
		}
		DST_GRAY1_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from pal8 source to gray2 destination
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
int blit_pal8_to_gray2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_GRAY2_INIT;
		for (x = 0; x < w; x++) {
			sacc = colors[SRC_IDX];
			sacc = CCIR_709(MSB_R(sacc), MSB_G(sacc), MSB_B(sacc)) >> 6;
			sacc = sacc << (6 - dbit);
			dacc = (dacc & ~mask) | (sacc & mask);
			SRC_PAL8_NEXT;
			DST_GRAY2_NEXT;
		}
		DST_GRAY2_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from pal8 source to gray4 destination
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
int blit_pal8_to_gray4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y, dbit;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_GRAY4_INIT;
		for (x = 0; x < w; x++) {
			sacc = colors[SRC_IDX];
			sacc = CCIR_709(MSB_R(sacc), MSB_G(sacc), MSB_B(sacc)) >> 4;
			sacc = sacc << (4 - dbit);
			dacc = (dacc & ~mask) | (sacc & mask);
			SRC_PAL8_NEXT;
			DST_GRAY4_NEXT;
		}
		DST_GRAY4_LAST;
	}
	return 0;
}


/**
 * @brief bit block transfer from pal8 source to gray8 destination
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
int blit_pal8_to_gray8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_GRAY8_INIT;
		for (x = 0; x < w; x++) {
			sacc = colors[SRC_IDX];
			sacc = CCIR_709(MSB_R(sacc), MSB_G(sacc), MSB_B(sacc));
			dacc = sacc & mask;
			SRC_PAL8_NEXT;
			DST_GRAY8_NEXT;
		}
	}
	return 0;
}



/**
 * @brief bit block transfer from pal8 source to gray16 destination
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
int blit_pal8_to_gray16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_GRAY16_INIT;
		for (x = 0; x < w; x++) {
			sacc = colors[SRC_IDX];
			sacc = CCIR_709(MSB_R(sacc), MSB_G(sacc), MSB_B(sacc)) << 8;
			dacc = sacc & mask;
			SRC_PAL8_NEXT;
			DST_GRAY16_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from pal8 source to rgb8 destination
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
int blit_pal8_to_rgb8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_RGB8_INIT;
		for (x = 0; x < w; x++) {
			sacc = colors[SRC_IDX];
			dacc = sacc & mask;
			SRC_PAL8_NEXT;
			DST_RGB8_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from pal8 source to rgb16 destination
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
int blit_pal8_to_rgb16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_RGB16_INIT;
		for (x = 0; x < w; x++) {
			sacc = colors[SRC_IDX];
			dacc = sacc & mask;
			SRC_PAL8_NEXT;
			DST_RGB16_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from pal8 source to gray1 paletteized destination
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
int blit_pal8_to_pal1(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t))
{
	int x, y, dbit, n;
	uint32_t soff, doff, sacc, dacc, mask;
	uint32_t lut[256];

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (n = 0; n < 256; n++)
		lut[n] = find_palette(cookie, 2, colors[n]);
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_PAL1_INIT;
		for (x = 0; x < w; x++) {
			sacc = lut[SRC_IDX] << (7 - dbit);
			dacc = (dacc & ~mask) | (sacc & mask);
			SRC_PAL8_NEXT;
			DST_PAL1_NEXT;
		}
		DST_PAL1_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from pal8 source to gray2 paletteized destination
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
int blit_pal8_to_pal2(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t))
{
	int x, y, dbit, n;
	uint32_t soff, doff, sacc, dacc, mask;
	uint32_t lut[256];

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (n = 0; n < 256; n++)
		lut[n] = find_palette(cookie, 4, colors[n]);
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_PAL2_INIT;
		for (x = 0; x < w; x++) {
			sacc = lut[SRC_IDX] << (6 - dbit);
			dacc = (dacc & ~mask) | (sacc & mask);
			SRC_PAL8_NEXT;
			DST_PAL2_NEXT;
		}
		DST_PAL2_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from gray1 source to gray4 paletteized destination
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
int blit_pal8_to_pal4(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t))
{
	int x, y, dbit, n;
	uint32_t soff, doff, sacc, dacc, mask;
	uint32_t lut[256];

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (n = 0; n < 256; n++)
		lut[n] = find_palette(cookie, 16, colors[n]);
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_PAL4_INIT;
		for (x = 0; x < w; x++) {
			sacc = lut[SRC_IDX] << (4 - dbit);
			dacc = (dacc & ~mask) | (sacc & mask);
			SRC_PAL8_NEXT;
			DST_PAL4_NEXT;
		}
		DST_PAL4_LAST;
	}
	return 0;
}

/**
 * @brief bit block transfer from pal8 source to 8bpp paletteized destination
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
int blit_pal8_to_pal8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha,
	void *cookie, uint32_t (*find_palette)(void *, uint32_t, uint32_t))
{
	int x, y, n;
	uint32_t soff, doff, sacc, dacc, mask;
	uint32_t lut[256];

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (n = 0; n < 256; n++)
		lut[n] = find_palette(cookie, 256, colors[n]);
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_PAL8_INIT;
		for (x = 0; x < w; x++) {
			sacc = lut[SRC_IDX];
			dacc = sacc & mask;
			SRC_PAL8_NEXT;
			DST_PAL8_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from pal8 source to gray8 + alpha destination
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
int blit_pal8_to_graya8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_GRAYA8_INIT;
		for (x = 0; x < w; x++) {
			sacc = colors[SRC_IDX];
			sacc = CCIR_709(MSB_R(sacc), MSB_G(sacc), MSB_B(sacc));
			dacc = sacc & mask;
			SRC_PAL8_NEXT;
			DST_GRAYA8_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from pal8 source to gray16 + alpha destination
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
int blit_pal8_to_graya16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_GRAYA16_INIT;
		for (x = 0; x < w; x++) {
			sacc = colors[SRC_IDX];
			sacc = CCIR_709(MSB_R(sacc), MSB_G(sacc), MSB_B(sacc)) << 8;
			dacc = sacc & mask;
			SRC_PAL8_NEXT;
			DST_GRAYA16_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from pal8 source to rgb8 + alpha destination
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
int blit_pal8_to_rgba8(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_RGBA8_INIT;
		for (x = 0; x < w; x++) {
			sacc = colors[SRC_IDX];
			dacc = sacc & mask;
			SRC_PAL8_NEXT;
			DST_RGBA8_NEXT;
		}
	}
	return 0;
}

/**
 * @brief bit block transfer from pal8 source to rgb16 + alpha destination
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
int blit_pal8_to_rgba16(
	uint8_t *dst, int dx, int dy, uint32_t dstride,
	uint8_t *src, int sx, int sy, uint32_t sstride,
	int w, int h, uint32_t *colors, int alpha)
{
	int x, y;
	uint32_t soff, doff, sacc, dacc, mask;

	if (NULL == colors)
		colors = pal8_colors;
	CLIPPING;
	for (y = 0; y < h; y++) {
		SRC_PAL8_INIT;
		DST_RGBA16_INIT;
		for (x = 0; x < w; x++) {
			sacc = colors[SRC_IDX];
			dacc = sacc & mask;
			SRC_PAL8_NEXT;
			DST_RGBA16_NEXT;
		}
	}
	return 0;
}
