/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * osd.h	Operating System Dependencies
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 * Copyright 2015 by Andreas Müller <schnitzeltony@googlemail.com>
 *
 ***************************************************************************************/
#if !defined(_OSD_H_INCLUDED_)
#define	_OSD_H_INCLUDED_

/* FIXME: split up into core (definition) and platform (implementation) parts */
#include "system.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* VIDEO interface */
extern int32_t osd_open_display(int32_t w, int32_t h, const char *title, void (*update_vmode)(void));
extern int32_t osd_close_display(void);
extern int32_t osd_update(int32_t skip_this_frame);
extern void osd_align_colors_to_mode(uint8_t *r, uint8_t *g, uint8_t *b);

/* AUDIO interface */
extern uint32_t osd_get_sample_rate(void);
extern void osd_set_sample_rate(uint32_t sample_rate);
extern void osd_set_refresh_rate(double refresh_rate);

extern int32_t osd_start_audio_stream(int32_t stereo);
extern void osd_stop_audio_stream(void);
extern uint32_t osd_update_audio_stream(int16_t *buffer);

/* KEYBOARD interface */
/* Note: KMOD_RSHIFT is used as OSD key, too */
#define	KMOD_SHIFT (KMOD_LSHIFT|KMOD_RSHIFT)
#define	KMOD_CTRL (KMOD_LCTRL|KMOD_RCTRL)
#define	KMOD_ALT (KMOD_LALT|KMOD_RALT)
#define	KMOD_META (KMOD_LMETA|KMOD_RMETA)

#define	OSD_KEY_SCANCODE	(1<<0)
#define	OSD_KEY_SYM		(1<<1)
#define	OSD_KEY_MOD		(1<<2)
#define	OSD_KEY_UNICODE		(1<<3)

typedef struct osd_key_s {
	uint32_t flags;
	uint32_t scancode;
	uint32_t sym;
	uint32_t mod;
}	osd_key_t;

extern const char *osd_key_name(osd_key_t *key);

/* MOUSE interface */
extern int32_t osd_mousex(void);
extern int32_t osd_mousey(void);
extern int32_t osd_mouseb(void);

/* TIMER interface */
extern int32_t osd_skip_next_frame(void);
extern void osd_display_frequency(uint64_t frq);

#define osd_profiling_ticks osd_cycles

/* OS helpers */
extern void osd_die(const char *fmt, ...);
extern int32_t osd_init(void *cookie,
	void (*keydn)(void *cookie, osd_key_t *),
	void (*keyup)(void *cookie, osd_key_t *),
	int argc, char **argv);
extern void osd_exit(void);


#if defined(__cplusplus)
}
#endif

#endif	/* !defined(_OSD_H_INCLUDED_) */
