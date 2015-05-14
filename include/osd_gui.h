/***************************************************************************************
 *
 * osd_gui.h	Operating System gui handling
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 * Copyright 2015 by Andreas Müller <schnitzeltony@googlemail.com>
 *
 ***************************************************************************************/
#if !defined(_OSD_GUI_H_INCLUDED_)
#define	_OSD_GUI_H_INCLUDED_

#include "system.h"
#include "osd_bitmap.h"

typedef enum {
	BT_STATIC,
	BT_EDIT,
	BT_PUSH,
	BT_CHECK
}	widget_style_t;

typedef enum {
	ST_NONE,
	ST_HOVER,
	ST_CLICK
}	widget_state_t;

/** @brief Defines the osd_widget_t type */
typedef struct osd_widget_s {
	/** @brief next widget */
	struct osd_widget_s *next;
	/** @brief widget rectangle */
	SDL_Rect rect;
	/** @brief widget style */
	widget_style_t style;
	/** @brief widget state */
	widget_state_t state;
	/** @brief widget active (BT_CHECK or BT_EDIT) */
	int32_t active;
	/** @brief offset into text (BT_EDIT) */
	int32_t offset;
	/** @brief cursor location in tex (BT_EDIT) */
	int32_t cursor;
	/** @brief width in character positions */
	int32_t cwidth;
	/** @brief widget identifier */
	int32_t id;
	/** @brief widget text */
	char *text;
	/** @brief color of widget face */
	uint32_t face;
	/** @brief color of widget face - hovering */
	uint32_t hover;
	/** @brief color of top + left borders - inner */
	uint32_t tl0;
	/** @brief color of bottom + right borders - inner */
	uint32_t br0;
	/** @brief color of top + left borders - outer */
	uint32_t tl1;
	/** @brief color of bottom + right borders - outer */
	uint32_t br1;
}	osd_widget_t;


extern int32_t osd_widget_alloc(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t w, int32_t h,
	int32_t r, int32_t g, int32_t b, widget_style_t style, int32_t id, const char *text);
extern void osd_widget_free(osd_widget_t *widget);
extern int32_t osd_widget_check(osd_bitmap_t *bitmap, int32_t id, int32_t check);
extern int32_t osd_widget_text(osd_bitmap_t *bitmap, int32_t id, const char *fmt, ...);
extern void osd_widget_update(osd_bitmap_t *bitmap, osd_widget_t *widget);


#endif	/* !defined(_OSD_GUI_H_INCLUDED_) */
