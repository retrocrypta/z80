/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * osd.h	Operating System Dependencies
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_OSD_H_INCLUDED_)
#define	_OSD_H_INCLUDED_

/* FIXME: split up into core (definition) and platform (implementation) parts */
#include "system.h"
#include "png.h"
#include "mng.h"

/* Note: KMOD_RSHIFT is used as OSD key, too */
#define	KMOD_SHIFT (KMOD_LSHIFT|KMOD_RSHIFT)
#define	KMOD_CTRL (KMOD_LCTRL|KMOD_RCTRL)
#define	KMOD_ALT (KMOD_LALT|KMOD_RALT)
#define	KMOD_META (KMOD_LMETA|KMOD_RMETA)

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

typedef struct {
	uint8_t r, g, b, a;
}	rgba_t;

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

/** @brief Defines the osd_bitmap_t type */
typedef struct osd_bitmap_s {
	/** @brief x coordinate on screen */
	int32_t x;
	/** @brief y coordinate on screen */
	int32_t y;
	/** @brief width */
	int32_t w;
	/** @brief height */
	int32_t h;
	/** @brief bits per pixel */
	int32_t bpp;
	/** @brief x scaling factor */
	int32_t xscale;
	/** @brief y scaling factor */
	int32_t yscale;
	/** @brief number of dirty rectangles */
	uint32_t dirty_count;
	/** @brief maximum number of dirty rectangles */
	uint32_t dirty_alloc;
	/** @brief dirty rectangles */
	SDL_Rect *dirty;
	/** @brief bitmap widgets */
	osd_widget_t *widgets;
	/** @brief OS dependent bitmap data */
	void *_private;
}	osd_bitmap_t;

#define	OSD_KEY_SCANCODE	(1<<0)
#define	OSD_KEY_SYM		(1<<1)
#define	OSD_KEY_MOD		(1<<2)
#define	OSD_KEY_UNICODE		(1<<3)
typedef struct osd_key_s {
	uint32_t flags;
	uint32_t scancode;
	uint32_t sym;
	uint32_t mod;
	uint32_t unicode;
}	osd_key_t;

#if defined(__cplusplus)
extern "C" {
#endif

/** @brief convert red, green, blue into an OSD uint32_t color */
#define osd_rgb(r,g,b) ((r)|((uint32_t)(g) << 8)|((uint32_t)(b) << 16))

/** @brief extract red from OSD uint32_t color */
#define	osd_get_r(color) ((uint8_t)((color) >> 0))

/** @brief extract green from OSD uint32_t color */
#define	osd_get_g(color) ((uint8_t)((color) >> 8))

/** @brief extract blue from OSD uint32_t color */
#define	osd_get_b(color) ((uint8_t)((color) >> 16))

/** @brief extract alpha from OSD uint32_t color */
#define	osd_get_a(color) ((uint8_t)((color) >> 24))

/** @brief helper macro to set the red, green, blue fields of an SDL_Color */
#define	osd_rgb_to_sdl_color(p,red,green,blue) do { \
	(p)->r = red; \
	(p)->g = green; \
	(p)->b = blue; \
	(p)->unused = 0; \
} while (0)

/** @brief helper macro to set the red, green, blue fields of an SDL_Color from a OSD color */
#define	osd_u32_to_sdl_color(p,color) do { \
	(p)->r = osd_get_r(color); \
	(p)->g = osd_get_g(color); \
	(p)->b = osd_get_b(color); \
	(p)->unused = 0; \
} while (0)

extern osd_bitmap_t *frame;

/* OS helpers */
extern void osd_die(const char *fmt, ...);
extern int32_t osd_init(int (*resize)(int32_t,int32_t),
	void *cookie,
	void (*keydn)(void *cookie, osd_key_t *),
	void (*keyup)(void *cookie, osd_key_t *),
	int argc, char **argv);
extern void osd_exit(void);

/* VIDEO primitives */
extern int32_t osd_bitmap_alloc(osd_bitmap_t **pbitmap, int32_t width, int32_t height, int32_t depth);
extern void osd_bitmap_free(osd_bitmap_t **pbitmap);

extern int32_t osd_widget_alloc(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t w, int32_t h,
	int32_t r, int32_t g, int32_t b, widget_style_t style, int32_t id, const char *text);
extern void osd_widget_free(osd_widget_t *widget);
extern int32_t osd_widget_check(osd_bitmap_t *bitmap, int32_t id, int32_t check);
extern int32_t osd_widget_text(osd_bitmap_t *bitmap, int32_t id, const char *fmt, ...);
extern void osd_widget_update(osd_bitmap_t *bitmap, osd_widget_t *widget);

extern uint32_t osd_color(osd_bitmap_t *bitmap, int32_t r, int32_t g, int32_t b);
extern uint32_t osd_color_alpha(osd_bitmap_t *bitmap, int32_t r, int32_t g, int32_t b, int32_t a);
extern void osd_set_clip(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t w, int32_t h);
extern void osd_putpixel(osd_bitmap_t *bitmap, int32_t x, int32_t y, uint32_t pixel);
extern uint32_t osd_getpixel(osd_bitmap_t *bitmap, int32_t x, int32_t y);
extern void osd_hline(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t w, uint32_t pixel);
extern void osd_vline(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t h, uint32_t pixel);
extern void osd_framerect(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t w, int32_t h,
	int32_t corners, uint32_t tl, uint32_t br);
extern void osd_fillrect(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t pixel);
extern void osd_blit(osd_bitmap_t *dst, osd_bitmap_t *src,
	int32_t sx, int32_t sy, int32_t w, int32_t h, int32_t dx, int32_t dy);

/* VIDEO interface */
extern int32_t osd_get_scale(void);
extern void osd_set_display(int32_t w, int32_t h);
extern int32_t osd_open_display(int32_t w, int32_t h, const char *title);
extern int32_t osd_get_display(int32_t *w, int32_t *h);
extern int32_t osd_close_display(void);
extern void osd_set_colors(osd_bitmap_t *dst, uint32_t *colors, uint32_t ncolors);
extern int32_t osd_render_font(osd_bitmap_t *bitmap, const void *font,
	uint32_t code, uint32_t count, uint32_t fw, uint32_t fh,
	uint32_t bw, uint32_t bh, uint32_t fs, uint32_t bpp, uint32_t *colors);
extern void osd_pattern(osd_bitmap_t *dst, osd_bitmap_t *src,
	int32_t x, int32_t y, uint32_t code, uint32_t ncolors, uint32_t *colors,
	uint32_t pw, uint32_t ph);
extern int32_t osd_update(int32_t skip_this_frame);
extern int32_t osd_mng_start(void);
extern int32_t osd_mng_stop(void);
extern int32_t osd_mng_frame(void);
extern int32_t osd_save_snapshot(void);

/* AUDIO interface */
extern uint32_t osd_get_sample_rate(void);
extern void osd_set_sample_rate(uint32_t sample_rate);
extern void osd_set_refresh_rate(double refresh_rate);

extern int32_t osd_start_audio_stream(int32_t stereo);
extern void osd_stop_audio_stream(void);
extern uint32_t osd_update_audio_stream(int16_t *buffer);

/* KEYBOARD interface */
extern const char *osd_key_name(osd_key_t *key);
extern uint8_t *osd_get_key_state(int32_t *numkeys);

/* MOUSE interface */
extern int32_t osd_mousex(void);
extern int32_t osd_mousey(void);
extern int32_t osd_mouseb(void);

/* TIMER interface */
extern int32_t osd_skip_next_frame(void);
extern void osd_display_frequency(uint64_t frq);

#define osd_profiling_ticks osd_cycles

#if defined(__cplusplus)
}
#endif

#endif	/* !defined(_OSD_H_INCLUDED_) */
