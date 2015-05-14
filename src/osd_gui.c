/*****************************************************************************
 *
 *  osd_gui.c	GUI / widget handling
 *
 * Copyright Juergen Buchmueller <pullmoll@t-online.de>
 * Copyright 2015 Andreas Müller <schnitzeltony@googlemail.com>
 *
 *****************************************************************************/
#include "osd_gui.h"

/**
 * @brief allocate a widget inside a bitmap
 *
 * @param bitmap pointer to the osd_bitmap_t
 * @param width width of the bitmap in pixels
 * @param height height of the bitmap in pixels
 * @param depth number of bits per pixel (1, 4, 8, 15, 16, 24, 32)
 * @result returns 0 on success, -1 on error
 */
/*int32_t osd_widget_alloc(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t w, int32_t h,
	int32_t r, int32_t g, int32_t b, widget_style_t style, int32_t id, const char *text)
{
	osd_widget_t *widget;

	if (NULL == bitmap)
		return -1;

	widget = calloc(1, sizeof(osd_widget_t));
	if (NULL == widget)
		return -1;

	widget->style = style;
	widget->state = ST_NONE;
	widget->active = 0;
	widget->id = id;
	widget->text = text ? strdup(text) : NULL;
	widget->rect.x = x;
	widget->rect.y = y;
	widget->rect.w = w;
	widget->rect.h = h;
	widget->cwidth = (w - 4) / FONT_W;

	widget->face = osd_color(bitmap,
		r,
		g,
		b);
	widget->hover = osd_color(bitmap,
		r,
		g,
		7 * b / 4 > 255 ? 255 : 7 * b / 4);
	widget->tl0 = osd_color(bitmap,
		r * 6 / 4 > 255 ? 255 : r * 6 / 4,
		g * 6 / 4 > 255 ? 255 : g * 6 / 4,
		b * 6 / 4 > 255 ? 255 : b * 6 / 4);
	widget->br0 = osd_color(bitmap,
		r * 2 / 4,
		g * 2 / 4,
		b * 2 / 4);
	widget->tl1 = osd_color(bitmap,
		r * 5 / 4 > 255 ? 255 : r * 5 / 4,
		g * 5 / 4 > 255 ? 255 : g * 5 / 4,
		b * 5 / 4 > 255 ? 255 : b * 5 / 4);
	widget->br1 = osd_color(bitmap,
		r * 1 / 4,
		g * 1 / 4,
		b * 1 / 4);

	widget->next = bitmap->widgets;
	bitmap->widgets = widget;
	return 0;
}*/

/**
 * @brief free a widget handle
 *
 * @param widget pointer to the osd_widget_t structure to free
 */
void osd_widget_free(osd_widget_t *widget)
{
	if (NULL == widget)
		return;
	if (NULL != widget->text)
		free(widget->text);
	free(widget);
}

/**
 * @brief return a widget inside a bitmap by its identifier
 *
 * @param bitmap pointer to the osd_bitmap_t
 * @param bid widget identifier
 * @result returns osd_widget_t* on success, NULL on error
 */
/*osd_widget_t *osd_widget_find(osd_bitmap_t *bitmap, int32_t id)
{
	osd_widget_t *widget;

	for (widget = bitmap->widgets; widget; widget = widget->next) {
		if (widget->id == id)
			return widget;
	}
	return NULL;
}*/

/**
 * @brief activate or deactivate a widget inside a bitmap
 *
 * @param bitmap pointer to the osd_bitmap_t
 * @param bid widget identifier
 * @result returns 0 on success, -1 on error
 */
/*int32_t osd_widget_active(osd_bitmap_t *bitmap, int32_t id, int32_t active)
{
	osd_widget_t *widget;

	for (widget = bitmap->widgets; widget; widget = widget->next) {
		if (widget->id == id) {
			if (active != widget->active) {
				widget->active = active;
//				osd_bitmap_dirty(bitmap, &widget->rect);
			}
			return 0;
		}
	}
	return -1;
}*/

/**
 * @brief change the text of a widget inside a bitmap
 *
 * @param bitmap pointer to the osd_bitmap_t
 * @param bid widget identifier
 * @result returns 0 on success, -1 on error
 */
/*int32_t osd_widget_text(osd_bitmap_t *bitmap, int32_t id, const char *fmt, ...)
{
	char buff[128];
	osd_widget_t *widget;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buff, sizeof(buff), fmt, ap);
	va_end(ap);
	for (widget = bitmap->widgets; widget; widget = widget->next) {
		if (widget->id == id) {
			if (NULL == widget->text || strcmp(widget->text, buff)) {
				if (widget->text)
					free(widget->text);
				widget->text = strdup(buff);
//				osd_bitmap_dirty(bitmap, &widget->rect);
			}
			return 0;
		}
	}
	return -1;
}*/

/**
 * @brief hit test a coordinate against the list of widgets of a bitmap
 *
 * @param bitmap pointer to the osd_bitmap_t
 * @param x x coordinate to check
 * @param y y coordinate to check
 * @param b widget status (mouse down)
 * @result returns widget id on success, 0 if hovering, -1 if nothing was hit
 */
/*int32_t osd_hittest(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t b)
{
	osd_widget_t *widget;
	int32_t rc = -1;
	int32_t st0, st1;

	x -= bitmap->x;
	y -= bitmap->y;
	for (widget = bitmap->widgets; widget; widget = widget->next) {
		if (x >= widget->rect.x && y >= widget->rect.y &&
			x < widget->rect.x + widget->rect.w &&
			y < widget->rect.y + widget->rect.h) {
			st0 = widget->state;
			st1 = (b & 1) ? ST_CLICK : ST_HOVER;
			if (st1 == ST_CLICK && st0 != ST_CLICK)
				rc = widget->id;
			widget->state = st1;
		} else {
			widget->state = ST_NONE;
		}
		osd_widget_update(bitmap, widget);
	}
	osd_bitmap_update(bitmap);
	return rc;
}*/




/**
 * @brief draw a widget with 3D edges
 *
 * The destination bitmap may be scaled
 *
 * @param bitmap pointer to the bitmap to set a pixel in
 * @param x left x coordinate of the rectangle
 * @param y top y coordinate of the rectangle
 * @param w width of the rectangle
 * @param h height of the rectangle
 * @param pixel pixel value
 * @param r red amount of the widget face
 * @param g green amount of the widget face
 * @param b blue amount of the widget face
 */
/*void osd_widget_update(osd_bitmap_t *bitmap, osd_widget_t *widget)
{
	int32_t x, y, w, h, len;
	uint32_t fg, tl0, br0, tl1, br1, black;
	uint32_t colors[2], i;
	uclock_t clk = uclock() / (UCLOCKS_PER_SEC / 4);

	x = widget->rect.x;
	y = widget->rect.y;
	w = widget->rect.w;
	h = widget->rect.h;

	black = osd_color(bitmap, 0, 0, 0);
	switch (widget->style) {
	case BT_STATIC:	// static text 
		fg = widget->face;
		br0 = widget->br0;
		tl0 = widget->tl0;

		osd_bitmap_framerect(bitmap, x, y, w, h, 1, br0, tl0);
		x += 1; y += 1; w -= 2; h -= 2;
		osd_bitmap_fillrect(bitmap, x, y, w, h, fg);

		if (NULL == widget->text)
			return;

		colors[0] = fg;
		colors[1] = black;
		w = strlen(widget->text) * FONT_W;	
		h = FONT_H;
		x = 2;
		y = (widget->rect.h - h) / 2;
		for (i = 0; i < strlen(widget->text); i++) {
			osd_font_blit(bitmap, font,
				x + widget->rect.x,
				y + widget->rect.y,
				widget->text[i],
				1, colors, FONT_W, FONT_H);
			x += FONT_W;
		}
		break;

	case BT_EDIT:	/// edit text
		fg = widget->active ? widget->tl1 : widget->state == ST_HOVER ?
			widget->hover : widget->face;
		br0 = widget->br0;
		tl0 = widget->tl0;

		osd_bitmap_framerect(bitmap, x, y, w, h, 1, br0, tl0);
		x += 1; y += 1; w -= 2; h -= 2;
		osd_bitmap_fillrect(bitmap, x, y, w, h, fg);

		if (widget->text) {
			len = strlen(widget->text);
			w = widget->cwidth;
			h = FONT_H;
			x = widget->rect.x + 2;
			y = widget->rect.y + (widget->rect.h - h) / 2;
			for (i = 0; i < w; i++) {
				int32_t offs = widget->offset + i;
				char ch = (offs < len) ? widget->text[offs] : ' ';
			
				colors[0] = fg;
				colors[1] = black;
				if (widget->active && (clk & 1) &&
					i == widget->cursor - widget->offset) {
					// invert character
					colors[0] = black;
					colors[1] = fg;
				}
				osd_font_blit(bitmap, font, x, y, ch, 2, colors, FONT_W, FONT_H);
				x += FONT_W;
			}
		}
		break;

	case BT_CHECK:	// check button
		fg = widget->state == ST_HOVER ? widget->hover : widget->face;
		tl0 = widget->tl0;
		br0 = widget->br0;
		if (widget->state == ST_CLICK) {
			uint32_t tmp;
			tmp = tl0;
			tl0 = br0;
			br0 = tmp;
		}

		osd_bitmap_framerect(bitmap, x, y, h, h, 0, black, black);
		x += 1; y += 1; h -= 2;
		osd_bitmap_framerect(bitmap, x, y, h, h, 1, tl0, br0);
		x += 1; y += 1; h -= 2;
		osd_bitmap_fillrect(bitmap, x, y, h, h, fg);
		if (widget->active) {
			x += 2; y += 1; h -= 3;
			if (widget->state == ST_CLICK) {
				x += 1;
				y += 1;
			}
			osd_bitmap_hline(bitmap, x + 0, y + 4, 2, black);
			osd_bitmap_hline(bitmap, x + 1, y + 5, 2, black);
			osd_bitmap_hline(bitmap, x + 1, y + 6, 2, black);
			osd_bitmap_hline(bitmap, x + 2, y + 7, 2, black);
			osd_bitmap_hline(bitmap, x + 2, y + 6, 2, black);
			osd_bitmap_hline(bitmap, x + 3, y + 5, 2, black);
			osd_bitmap_hline(bitmap, x + 3, y + 4, 2, black);
			osd_bitmap_hline(bitmap, x + 4, y + 3, 2, black);
			osd_bitmap_hline(bitmap, x + 5, y + 2, 2, black);
		}

		if (NULL == widget->text)
			return;

		colors[0] = fg;
		colors[1] = black;
		w = strlen(widget->text) * FONT_W;	
		h = FONT_H;
		x = widget->rect.h + 2;
		y = (widget->rect.h - h) / 2;
		for (i = 0; i < strlen(widget->text); i++) {
			osd_font_blit(bitmap, font,
				x + widget->rect.x,
				y + widget->rect.y,
				widget->text[i],
				1, colors, FONT_W, FONT_H);
			x += FONT_W;
		}
		break;

	case BT_PUSH:	// push button
		fg = widget->state == ST_HOVER ? widget->hover : widget->face;
		tl0 = widget->tl0;
		br0 = widget->br0;
		tl1 = widget->tl1;
		br1 = widget->br1;

		if (widget->state == ST_CLICK) {
			uint32_t tmp;
			tmp = tl0; tl0 = br0; br0 = tmp;
			tmp = tl1; tl1 = br1; br1 = tmp;
		}

		osd_bitmap_framerect(bitmap, x, y, w, h, 0, tl1, br1);
		x += 1; y += 1; w -= 2; h -= 2;
		osd_bitmap_framerect(bitmap, x, y, w, h, 1, tl0, br0);
		x += 1; y += 1; w -= 2; h -= 2;
		osd_bitmap_fillrect(bitmap, x, y, w, h, fg);

		if (NULL == widget->text)
			return;

		colors[0] = fg;
		colors[1] = black;
		w = strlen(widget->text) * FONT_W;	
		h = FONT_H;
		x = (widget->rect.w - w) / 2;
		y = (widget->rect.h - h) / 2;
		if (widget->state == ST_CLICK) {
			x++;
			y++;
		}
		for (i = 0; i < strlen(widget->text); i++) {
			osd_font_blit(bitmap, font,
				x + widget->rect.x,
				y + widget->rect.y,
				widget->text[i],
				1, colors, FONT_W, FONT_H);
			x += FONT_W;
		}
		break;
	}
}*/
