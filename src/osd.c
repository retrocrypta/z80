/* ed:set tabstop=8 noexpandtab: */
/*****************************************************************************
 *
 *  osd.c	Operating System Dependencies for audio/video/keyboard I/O
 *
 * Copyright Juergen Buchmueller <pullmoll@t-online.de>
 *
 *****************************************************************************/
#include "osd.h"

osd_bitmap_t *frame = NULL;

static int32_t throttle = 1;
static int32_t fullscreen = 0;
static int32_t frameskip = 0;
static int32_t autoframeskip = 1;
static int32_t max_autoframeskip = 8;
static int32_t start_video = 0;
static int32_t scale = 2;

static SDL_Rect *dirty;
static uint32_t dirty_count = 0;
static uint32_t dirty_alloc = 0;

static int32_t display_w;
static int32_t display_h;

static int32_t mousex;
static int32_t mousey;
static int32_t mouseb;

static SDL_AudioSpec *audio_spec_desired;
static SDL_AudioSpec *audio_spec_obtained;
static uint32_t sample_rate = 48000;
static double refresh_rate = 50.0;

#define	SBUFF_SIZE	16384
typedef struct sbuff_s {
	ssize_t size;
	ssize_t head;
	ssize_t tail;
	int16_t buffer[SBUFF_SIZE];
}	sbuff_t;

static sbuff_t *sbuff;

static char osd_title[256];

static SDL_Surface *screen = NULL;
static SDL_Cursor *cursor = NULL;

static int (*resize_callback)(int32_t, int32_t) = NULL;

static void (*keydn_callback)(void *cookie, osd_key_t *) = NULL;
static void (*keyup_callback)(void *cookie, osd_key_t *) = NULL;
static void *cookie_callback = NULL;

static void (*keydn_osd_local)(void *cookie, osd_key_t *) = NULL;
static void (*keyup_osd_local)(void *cookie, osd_key_t *) = NULL;
static void *cookie_local = NULL;

static osd_bitmap_t *font;

static osd_bitmap_t *ctrl_panel;
static int32_t ctrl_panel_on = 0;

static osd_bitmap_t *cpu_panel;
static int32_t cpu_panel_on = 0;

static size_t xngsize;
static mng_t *mng;

#define	FONT_W	6
#define	FONT_H	10

typedef enum {
	WID_NONE,
	WID_RESET,
	WID_THROTTLE,
	WID_1X1,
	WID_2X2,
	WID_3X3,
	WID_4X4,
	WID_SNAPSHOT,
	WID_VIDEO,
	WID_CPU_PANEL,
	WID_CASSETTE,
	WID_FLOPPY,
	WID_FREQUENCY
}	widget_id_ctrl_t;

typedef struct {
	osd_bitmap_t *bitmap;
	widget_id_ctrl_t id;
}	cookie_local_t;

/****************************************
 * forward declarations
 ****************************************/
static void keydn_edit(void *cookie, osd_key_t *key);
static void keyup_edit(void *cookie, osd_key_t *key);


/** @brief hardcoded 6x10 character generator */
static const uint8_t chargen_6x10[] = {
	0x00,  0x00,0xa8,0x00,0x88,0x00,0x88,0x00,0xa8,0x00,0x00,
	0x20,  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x21,  0x00,0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x00,0x00,
	0x22,  0x00,0x50,0x50,0x50,0x00,0x00,0x00,0x00,0x00,0x00,
	0x23,  0x00,0x50,0x50,0xf8,0x50,0xf8,0x50,0x50,0x00,0x00,
	0x24,  0x00,0x20,0x70,0xa0,0x70,0x28,0x70,0x20,0x00,0x00,
	0x25,  0x00,0x48,0xa8,0x50,0x20,0x50,0xa8,0x90,0x00,0x00,
	0x26,  0x00,0x40,0xa0,0xa0,0x40,0xa8,0x90,0x68,0x00,0x00,
	0x27,  0x00,0x20,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,
	0x28,  0x00,0x10,0x20,0x40,0x40,0x40,0x20,0x10,0x00,0x00,
	0x29,  0x00,0x40,0x20,0x10,0x10,0x10,0x20,0x40,0x00,0x00,
	0x2a,  0x00,0x00,0x88,0x50,0xf8,0x50,0x88,0x00,0x00,0x00,
	0x2b,  0x00,0x00,0x20,0x20,0xf8,0x20,0x20,0x00,0x00,0x00,
	0x2c,  0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x20,0x40,0x00,
	0x2d,  0x00,0x00,0x00,0x00,0xf8,0x00,0x00,0x00,0x00,0x00,
	0x2e,  0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x70,0x20,0x00,
	0x2f,  0x00,0x08,0x08,0x10,0x20,0x40,0x80,0x80,0x00,0x00,
	0x30,  0x00,0x20,0x50,0x88,0x88,0x88,0x50,0x20,0x00,0x00,
	0x31,  0x00,0x20,0x60,0xa0,0x20,0x20,0x20,0xf8,0x00,0x00,
	0x32,  0x00,0x70,0x88,0x08,0x30,0x40,0x80,0xf8,0x00,0x00,
	0x33,  0x00,0xf8,0x08,0x10,0x30,0x08,0x88,0x70,0x00,0x00,
	0x34,  0x00,0x10,0x30,0x50,0x90,0xf8,0x10,0x10,0x00,0x00,
	0x35,  0x00,0xf8,0x80,0xb0,0xc8,0x08,0x88,0x70,0x00,0x00,
	0x36,  0x00,0x30,0x40,0x80,0xb0,0xc8,0x88,0x70,0x00,0x00,
	0x37,  0x00,0xf8,0x08,0x10,0x10,0x20,0x40,0x40,0x00,0x00,
	0x38,  0x00,0x70,0x88,0x88,0x70,0x88,0x88,0x70,0x00,0x00,
	0x39,  0x00,0x70,0x88,0x98,0x68,0x08,0x10,0x60,0x00,0x00,
	0x3a,  0x00,0x00,0x20,0x70,0x20,0x00,0x20,0x70,0x20,0x00,
	0x3b,  0x00,0x00,0x20,0x70,0x20,0x00,0x30,0x20,0x40,0x00,
	0x3c,  0x00,0x08,0x10,0x20,0x40,0x20,0x10,0x08,0x00,0x00,
	0x3d,  0x00,0x00,0x00,0xf8,0x00,0xf8,0x00,0x00,0x00,0x00,
	0x3e,  0x00,0x40,0x20,0x10,0x08,0x10,0x20,0x40,0x00,0x00,
	0x3f,  0x00,0x70,0x88,0x10,0x20,0x20,0x00,0x20,0x00,0x00,
	0x40,  0x00,0x70,0x88,0x98,0xa8,0xb0,0x80,0x70,0x00,0x00,
	0x41,  0x00,0x20,0x50,0x88,0x88,0xf8,0x88,0x88,0x00,0x00,
	0x42,  0x00,0xf0,0x48,0x48,0x70,0x48,0x48,0xf0,0x00,0x00,
	0x43,  0x00,0x70,0x88,0x80,0x80,0x80,0x88,0x70,0x00,0x00,
	0x44,  0x00,0xf0,0x48,0x48,0x48,0x48,0x48,0xf0,0x00,0x00,
	0x45,  0x00,0xf8,0x80,0x80,0xf0,0x80,0x80,0xf8,0x00,0x00,
	0x46,  0x00,0xf8,0x80,0x80,0xf0,0x80,0x80,0x80,0x00,0x00,
	0x47,  0x00,0x70,0x88,0x80,0x80,0x98,0x88,0x70,0x00,0x00,
	0x48,  0x00,0x88,0x88,0x88,0xf8,0x88,0x88,0x88,0x00,0x00,
	0x49,  0x00,0x70,0x20,0x20,0x20,0x20,0x20,0x70,0x00,0x00,
	0x4a,  0x00,0x38,0x10,0x10,0x10,0x10,0x90,0x60,0x00,0x00,
	0x4b,  0x00,0x88,0x90,0xa0,0xc0,0xa0,0x90,0x88,0x00,0x00,
	0x4c,  0x00,0x80,0x80,0x80,0x80,0x80,0x80,0xf8,0x00,0x00,
	0x4d,  0x00,0x88,0x88,0xd8,0xa8,0x88,0x88,0x88,0x00,0x00,
	0x4e,  0x00,0x88,0x88,0xc8,0xa8,0x98,0x88,0x88,0x00,0x00,
	0x4f,  0x00,0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x00,
	0x50,  0x00,0xf0,0x88,0x88,0xf0,0x80,0x80,0x80,0x00,0x00,
	0x51,  0x00,0x70,0x88,0x88,0x88,0x88,0xa8,0x70,0x08,0x00,
	0x52,  0x00,0xf0,0x88,0x88,0xf0,0xa0,0x90,0x88,0x00,0x00,
	0x53,  0x00,0x70,0x88,0x80,0x70,0x08,0x88,0x70,0x00,0x00,
	0x54,  0x00,0xf8,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,
	0x55,  0x00,0x88,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x00,
	0x56,  0x00,0x88,0x88,0x88,0x50,0x50,0x50,0x20,0x00,0x00,
	0x57,  0x00,0x88,0x88,0x88,0xa8,0xa8,0xd8,0x88,0x00,0x00,
	0x58,  0x00,0x88,0x88,0x50,0x20,0x50,0x88,0x88,0x00,0x00,
	0x59,  0x00,0x88,0x88,0x50,0x20,0x20,0x20,0x20,0x00,0x00,
	0x5a,  0x00,0xf8,0x08,0x10,0x20,0x40,0x80,0xf8,0x00,0x00,
	0x5b,  0x00,0x70,0x40,0x40,0x40,0x40,0x40,0x70,0x00,0x00,
	0x5c,  0x00,0x80,0x80,0x40,0x20,0x10,0x08,0x08,0x00,0x00,
	0x5d,  0x00,0x70,0x10,0x10,0x10,0x10,0x10,0x70,0x00,0x00,
	0x5e,  0x00,0x20,0x50,0x88,0x00,0x00,0x00,0x00,0x00,0x00,
	0x5f,  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0x00,
	0x60,  0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x61,  0x00,0x00,0x00,0x70,0x08,0x78,0x88,0x78,0x00,0x00,
	0x62,  0x00,0x80,0x80,0xb0,0xc8,0x88,0xc8,0xb0,0x00,0x00,
	0x63,  0x00,0x00,0x00,0x70,0x88,0x80,0x88,0x70,0x00,0x00,
	0x64,  0x00,0x08,0x08,0x68,0x98,0x88,0x98,0x68,0x00,0x00,
	0x65,  0x00,0x00,0x00,0x70,0x88,0xf8,0x80,0x70,0x00,0x00,
	0x66,  0x00,0x30,0x48,0x40,0xf0,0x40,0x40,0x40,0x00,0x00,
	0x67,  0x00,0x00,0x00,0x78,0x88,0x88,0x78,0x08,0x88,0x70,
	0x68,  0x00,0x80,0x80,0xb0,0xc8,0x88,0x88,0x88,0x00,0x00,
	0x69,  0x00,0x20,0x00,0x60,0x20,0x20,0x20,0x70,0x00,0x00,
	0x6a,  0x00,0x08,0x00,0x18,0x08,0x08,0x08,0x48,0x48,0x30,
	0x6b,  0x00,0x80,0x80,0x88,0x90,0xe0,0x90,0x88,0x00,0x00,
	0x6c,  0x00,0x60,0x20,0x20,0x20,0x20,0x20,0x70,0x00,0x00,
	0x6d,  0x00,0x00,0x00,0xd0,0xa8,0xa8,0xa8,0x88,0x00,0x00,
	0x6e,  0x00,0x00,0x00,0xb0,0xc8,0x88,0x88,0x88,0x00,0x00,
	0x6f,  0x00,0x00,0x00,0x70,0x88,0x88,0x88,0x70,0x00,0x00,
	0x70,  0x00,0x00,0x00,0xb0,0xc8,0x88,0xc8,0xb0,0x80,0x80,
	0x71,  0x00,0x00,0x00,0x68,0x98,0x88,0x98,0x68,0x08,0x08,
	0x72,  0x00,0x00,0x00,0xb0,0xc8,0x80,0x80,0x80,0x00,0x00,
	0x73,  0x00,0x00,0x00,0x70,0x80,0x70,0x08,0xf0,0x00,0x00,
	0x74,  0x00,0x40,0x40,0xf0,0x40,0x40,0x48,0x30,0x00,0x00,
	0x75,  0x00,0x00,0x00,0x88,0x88,0x88,0x98,0x68,0x00,0x00,
	0x76,  0x00,0x00,0x00,0x88,0x88,0x50,0x50,0x20,0x00,0x00,
	0x77,  0x00,0x00,0x00,0x88,0x88,0xa8,0xa8,0x50,0x00,0x00,
	0x78,  0x00,0x00,0x00,0x88,0x50,0x20,0x50,0x88,0x00,0x00,
	0x79,  0x00,0x00,0x00,0x88,0x88,0x98,0x68,0x08,0x88,0x70,
	0x7a,  0x00,0x00,0x00,0xf8,0x10,0x20,0x40,0xf8,0x00,0x00,
	0x7b,  0x00,0x18,0x20,0x10,0x60,0x10,0x20,0x18,0x00,0x00,
	0x7c,  0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,
	0x7d,  0x00,0x60,0x10,0x20,0x18,0x20,0x10,0x60,0x00,0x00,
	0x7e,  0x00,0x48,0xa8,0x90,0x00,0x00,0x00,0x00,0x00,0x00,
	0xa0,  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xa1,  0x00,0x20,0x00,0x20,0x20,0x20,0x20,0x20,0x00,0x00,
	0xa2,  0x00,0x00,0x20,0x78,0xa0,0xa0,0xa0,0x78,0x20,0x00,
	0xa3,  0x00,0x30,0x48,0x40,0xe0,0x40,0x48,0xb0,0x00,0x00,
	0xa4,  0x00,0x00,0x00,0x88,0x70,0x50,0x70,0x88,0x00,0x00,
	0xa5,  0x00,0x88,0x88,0x50,0x20,0xf8,0x20,0x20,0x20,0x00,
	0xa6,  0x00,0x20,0x20,0x20,0x00,0x20,0x20,0x20,0x00,0x00,
	0xa7,  0x00,0x70,0x80,0xe0,0x90,0x48,0x38,0x08,0x70,0x00,
	0xa8,  0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xa9,  0x00,0x70,0x88,0xa8,0xc8,0xa8,0x88,0x70,0x00,0x00,
	0xaa,  0x00,0x38,0x48,0x58,0x28,0x00,0x78,0x00,0x00,0x00,
	0xab,  0x00,0x00,0x00,0x24,0x48,0x90,0x48,0x24,0x00,0x00,
	0xac,  0x00,0x00,0x00,0x00,0x78,0x08,0x00,0x00,0x00,0x00,
	0xad,  0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,
	0xae,  0x00,0x70,0x88,0xe8,0xc8,0xc8,0x88,0x70,0x00,0x00,
	0xaf,  0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xb0,  0x00,0x20,0x50,0x20,0x00,0x00,0x00,0x00,0x00,0x00,
	0xb1,  0x00,0x00,0x20,0x20,0xf8,0x20,0x20,0xf8,0x00,0x00,
	0xb2,  0x30,0x48,0x10,0x20,0x78,0x00,0x00,0x00,0x00,0x00,
	0xb3,  0x70,0x08,0x30,0x08,0x70,0x00,0x00,0x00,0x00,0x00,
	0xb4,  0x10,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xb5,  0x00,0x00,0x00,0x88,0x88,0x88,0xc8,0xb0,0x80,0x00,
	0xb6,  0x00,0x78,0xe8,0xe8,0x68,0x28,0x28,0x28,0x00,0x00,
	0xb7,  0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,
	0xb8,  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x20,
	0xb9,  0x20,0x60,0x20,0x20,0x70,0x00,0x00,0x00,0x00,0x00,
	0xba,  0x00,0x30,0x48,0x48,0x30,0x00,0x78,0x00,0x00,0x00,
	0xbb,  0x00,0x00,0x00,0x90,0x48,0x24,0x48,0x90,0x00,0x00,
	0xbc,  0x40,0xc0,0x40,0x40,0xe4,0x0c,0x14,0x3c,0x04,0x00,
	0xbd,  0x40,0xc0,0x40,0x40,0xe8,0x14,0x04,0x08,0x1c,0x00,
	0xbe,  0xc0,0x20,0x40,0x20,0xc8,0x18,0x28,0x78,0x08,0x00,
	0xbf,  0x00,0x20,0x00,0x20,0x20,0x40,0x88,0x70,0x00,0x00,
	0xc0,  0x40,0x20,0x70,0x88,0x88,0xf8,0x88,0x88,0x00,0x00,
	0xc1,  0x10,0x20,0x70,0x88,0x88,0xf8,0x88,0x88,0x00,0x00,
	0xc2,  0x20,0x50,0x70,0x88,0x88,0xf8,0x88,0x88,0x00,0x00,
	0xc3,  0x48,0xb0,0x70,0x88,0x88,0xf8,0x88,0x88,0x00,0x00,
	0xc4,  0x50,0x00,0x70,0x88,0x88,0xf8,0x88,0x88,0x00,0x00,
	0xc5,  0x20,0x50,0x70,0x88,0x88,0xf8,0x88,0x88,0x00,0x00,
	0xc6,  0x00,0x3c,0x50,0x90,0x9c,0xf0,0x90,0x9c,0x00,0x00,
	0xc7,  0x00,0x70,0x88,0x80,0x80,0x80,0x88,0x70,0x20,0x40,
	0xc8,  0x40,0xf8,0x80,0x80,0xf0,0x80,0x80,0xf8,0x00,0x00,
	0xc9,  0x10,0xf8,0x80,0x80,0xf0,0x80,0x80,0xf8,0x00,0x00,
	0xca,  0x20,0xf8,0x80,0x80,0xf0,0x80,0x80,0xf8,0x00,0x00,
	0xcb,  0x50,0xf8,0x80,0x80,0xf0,0x80,0x80,0xf8,0x00,0x00,
	0xcc,  0x40,0x20,0x70,0x20,0x20,0x20,0x20,0x70,0x00,0x00,
	0xcd,  0x10,0x20,0x70,0x20,0x20,0x20,0x20,0x70,0x00,0x00,
	0xce,  0x20,0x50,0x70,0x20,0x20,0x20,0x20,0x70,0x00,0x00,
	0xcf,  0x50,0x00,0x70,0x20,0x20,0x20,0x20,0x70,0x00,0x00,
	0xd0,  0x00,0xf0,0x48,0x48,0xe8,0x48,0x48,0xf0,0x00,0x00,
	0xd1,  0x28,0x50,0x88,0xc8,0xa8,0x98,0x88,0x88,0x00,0x00,
	0xd2,  0x40,0x20,0x70,0x88,0x88,0x88,0x88,0x70,0x00,0x00,
	0xd3,  0x10,0x20,0x70,0x88,0x88,0x88,0x88,0x70,0x00,0x00,
	0xd4,  0x20,0x50,0x70,0x88,0x88,0x88,0x88,0x70,0x00,0x00,
	0xd5,  0x28,0x50,0x70,0x88,0x88,0x88,0x88,0x70,0x00,0x00,
	0xd6,  0x50,0x00,0x70,0x88,0x88,0x88,0x88,0x70,0x00,0x00,
	0xd7,  0x00,0x00,0x00,0x88,0x50,0x20,0x50,0x88,0x00,0x00,
	0xd8,  0x00,0x70,0x98,0x98,0xa8,0xc8,0xc8,0x70,0x00,0x00,
	0xd9,  0x40,0x20,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x00,
	0xda,  0x10,0x20,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x00,
	0xdb,  0x20,0x50,0x00,0x88,0x88,0x88,0x88,0x70,0x00,0x00,
	0xdc,  0x50,0x00,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x00,
	0xdd,  0x10,0x20,0x88,0x88,0x50,0x20,0x20,0x20,0x00,0x00,
	0xde,  0x00,0x80,0xf0,0x88,0xf0,0x80,0x80,0x80,0x00,0x00,
	0xdf,  0x00,0x70,0x88,0x90,0xa0,0x90,0x88,0xb0,0x00,0x00,
	0xe0,  0x40,0x20,0x00,0x70,0x08,0x78,0x88,0x78,0x00,0x00,
	0xe1,  0x10,0x20,0x00,0x70,0x08,0x78,0x88,0x78,0x00,0x00,
	0xe2,  0x20,0x50,0x00,0x70,0x08,0x78,0x88,0x78,0x00,0x00,
	0xe3,  0x28,0x50,0x00,0x70,0x08,0x78,0x88,0x78,0x00,0x00,
	0xe4,  0x00,0x50,0x00,0x70,0x08,0x78,0x88,0x78,0x00,0x00,
	0xe5,  0x20,0x50,0x20,0x70,0x08,0x78,0x88,0x78,0x00,0x00,
	0xe6,  0x00,0x00,0x00,0x78,0x14,0x7c,0x90,0x7c,0x00,0x00,
	0xe7,  0x00,0x00,0x00,0x70,0x88,0x80,0x88,0x70,0x20,0x40,
	0xe8,  0x40,0x20,0x00,0x70,0x88,0xf8,0x80,0x70,0x00,0x00,
	0xe9,  0x10,0x20,0x00,0x70,0x88,0xf8,0x80,0x70,0x00,0x00,
	0xea,  0x20,0x50,0x00,0x70,0x88,0xf8,0x80,0x70,0x00,0x00,
	0xeb,  0x00,0x50,0x00,0x70,0x88,0xf8,0x80,0x70,0x00,0x00,
	0xec,  0x40,0x20,0x00,0x60,0x20,0x20,0x20,0x70,0x00,0x00,
	0xed,  0x20,0x40,0x00,0x60,0x20,0x20,0x20,0x70,0x00,0x00,
	0xee,  0x20,0x50,0x00,0x60,0x20,0x20,0x20,0x70,0x00,0x00,
	0xef,  0x00,0x50,0x00,0x60,0x20,0x20,0x20,0x70,0x00,0x00,
	0xf0,  0x00,0xc0,0x30,0x70,0x88,0x88,0x88,0x70,0x00,0x00,
	0xf1,  0x28,0x50,0x00,0xb0,0xc8,0x88,0x88,0x88,0x00,0x00,
	0xf2,  0x40,0x20,0x00,0x70,0x88,0x88,0x88,0x70,0x00,0x00,
	0xf3,  0x10,0x20,0x00,0x70,0x88,0x88,0x88,0x70,0x00,0x00,
	0xf4,  0x20,0x50,0x00,0x70,0x88,0x88,0x88,0x70,0x00,0x00,
	0xf5,  0x28,0x50,0x00,0x70,0x88,0x88,0x88,0x70,0x00,0x00,
	0xf6,  0x00,0x50,0x00,0x70,0x88,0x88,0x88,0x70,0x00,0x00,
	0xf7,  0x00,0x00,0x20,0x00,0xf8,0x00,0x20,0x00,0x00,0x00,
	0xf8,  0x00,0x00,0x00,0x78,0x98,0xa8,0xc8,0xf0,0x00,0x00,
	0xf9,  0x40,0x20,0x00,0x88,0x88,0x88,0x98,0x68,0x00,0x00,
	0xfa,  0x10,0x20,0x00,0x88,0x88,0x88,0x98,0x68,0x00,0x00,
	0xfb,  0x20,0x50,0x00,0x88,0x88,0x88,0x98,0x68,0x00,0x00,
	0xfc,  0x00,0x50,0x00,0x88,0x88,0x88,0x98,0x68,0x00,0x00,
	0xfd,  0x00,0x10,0x20,0x88,0x88,0x98,0x68,0x08,0x88,0x70,
	0xfe,  0x00,0x00,0x80,0xf0,0x88,0x88,0x88,0xf0,0x80,0x80,
	0xff,  0x00,0x50,0x00,0x88,0x88,0x98,0x68,0x08,0x88,0x70
};

#define	FRAMESKIP_LEVELS 12

#define	UCLOCKS_PER_SEC 1000000

typedef long uclock_t;

uclock_t uclock(void)
{

	static uclock_t init_sec = 0;
	struct timeval tv;
	gettimeofday(&tv, 0);
	if (0 == init_sec)
		init_sec = tv.tv_sec;
	return (uclock_t)UCLOCKS_PER_SEC * (tv.tv_sec - init_sec) +
		tv.tv_usec;
}

/**
 * @brief add a dirty rectangle to the screen surface
 * @param dst destination rectangle
 */
static __inline void osd_screen_dirty(SDL_Rect *dst)
{
	SDL_Rect *r;
	uint32_t n;

	if (NULL == screen)
		return;

	/* try to combine the dirty rectangle with an existing one */
	for (n = 0; n < dirty_count; n++) {
		r = &dirty[n];
		if (dst->y == r->y && dst->h == r->h) {
			if (r->x + r->w == dst->x) {
				r->w = r->w + dst->w;
				if (r->x + r->w > screen->w)
					r->w = screen->w - r->x;
				return;
			}
			if (dst->x + dst->w == r->x) {
				r->x = dst->x;
				r->w = r->w + dst->w;
				if (r->x + r->w > screen->w)
					r->w = screen->w - r->x;
				return;
			}
		}
		if (dst->x == r->x && dst->w == r->w) {
			if (r->y + r->h == dst->y) {
				r->h = r->h + dst->h;
				if (r->y + r->h > screen->h)
					r->h = screen->h - r->y;
				return;
			}
			if (dst->y + dst->h == r->y) {
				r->y = dst->y;
				r->h = r->h + dst->h;
				if (r->y + r->h > screen->h)
					r->h = screen->h - r->y;
				return;
			}
		}
	}
	if (dirty_count >= dirty_alloc) {
		dirty_alloc = dirty_alloc ? dirty_alloc * 2 : 32;
		dirty = realloc(dirty, dirty_alloc * sizeof(SDL_Rect));
		if (NULL == dirty)
			osd_die("realloc() dirty rects");
	}
	memcpy(&dirty[dirty_count], dst, sizeof(*dst));
	r = &dirty[dirty_count];
	if (r->x < 0) {
		r->w += r->x;
		r->x = 0;
	}
	if (r->y < 0) {
		r->h += r->y;
		r->y = 0;
	}
	if (r->x + r->w > screen->w) {
		r->w = screen->w - r->x;
	}
	if (r->y + r->h > screen->h) {
		r->h = screen->h - r->y;
	}
	if (r->w <= 0 || r->h <= 0)
		return;
	if (r->x > screen->w || r->y > screen->h)
		return;
	dirty_count++;
}


/**
 * @brief update dirty rectangles from a bitmap to the screen
 *
 * @param bitmap bitmap handle to add the dirty rectangle to
 * @param dst destination rectangle
 */
static __inline void osd_bitmap_update(osd_bitmap_t *bitmap)
{
	SDL_Rect dst;
	uint32_t n;

	if (NULL == bitmap)
		return;

	for (n = 0; n < bitmap->dirty_count; n++) {
		dst = bitmap->dirty[n];
		osd_blit(NULL, bitmap, bitmap->x + dst.x, bitmap->y + dst.y, dst.w, dst.h, dst.x, dst.y);
	}
	bitmap->dirty_count = 0;
}

/**
 * @brief add a dirty rectangle to a bitmap's surface
 *
 * @param bitmap bitmap handle to add the dirty rectangle to
 * @param dst destination rectangle
 */
static __inline void osd_bitmap_dirty(osd_bitmap_t *bitmap, SDL_Rect *dst)
{
	SDL_Rect *r;
	uint32_t n;

	if (NULL == bitmap) {
		osd_screen_dirty(dst);
		return;
	}

	/* try to combine the dirty rectangle with an existing one */
	for (n = 0; n < bitmap->dirty_count; n++) {
		r = &bitmap->dirty[n];
		if (dst->y == r->y && dst->h == r->h) {
			if (r->x + r->w == dst->x) {
				r->w = r->w + dst->w;
				return;
			}
			if (dst->x + dst->w == r->x) {
				r->x = dst->x;
				r->w = r->w + dst->w;
				return;
			}
		}
		if (dst->x == r->x && dst->w == r->w) {
			if (r->y + r->h == dst->y) {
				r->h = r->h + dst->h;
				return;
			}
			if (dst->y + dst->h == r->y) {
				r->y = dst->y;
				r->h = r->h + dst->h;
				return;
			}
		}
	}
	if (bitmap->dirty_count >= bitmap->dirty_alloc) {
		bitmap->dirty_alloc = bitmap->dirty_alloc ? bitmap->dirty_alloc * 2 : 32;
		bitmap->dirty = realloc(bitmap->dirty, bitmap->dirty_alloc * sizeof(SDL_Rect));
		if (NULL == bitmap->dirty)
			osd_die("realloc() dirty rects");
	}
	
	memcpy(&bitmap->dirty[bitmap->dirty_count], dst, sizeof(*dst));
	r = &bitmap->dirty[bitmap->dirty_count];
	if (r->x < 0) {
		r->w += r->x;
		r->x = 0;
	}
	if (r->y < 0) {
		r->h += r->y;
		r->y = 0;
	}
	if (r->x + r->w > bitmap->w) {
		r->w = bitmap->w - r->x;
	}
	if (r->y + r->h > bitmap->h) {
		r->h = bitmap->h - r->y;
	}
	if (r->w <= 0 || r->h <= 0)
		return;
	if (r->x > bitmap->w || r->y > bitmap->h)
		return;

	bitmap->dirty_count++;
}

/**
 * @brief exit after printing some message to stderr
 * @param fmt format string and optional parameters following
 */
void osd_die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
}

/**
 * @brief allocate a bitmap handle
 *
 * @param bitmap pointer to the osd_bitmap_t structure to fill
 * @param width width of the bitmap in pixels
 * @param height height of the bitmap in pixels
 * @param depth number of bits per pixel (1, 4, 8, 15, 16, 24, 32)
 * @result returns 0 on success, -1 on error
 */
int32_t osd_bitmap_alloc(osd_bitmap_t **pbitmap, int32_t width, int32_t height, int32_t depth)
{
	osd_bitmap_t *bitmap;
	SDL_Surface *dst_surface = NULL;
	SDL_Color palette[2];
	uint32_t rmask, gmask, bmask, amask, flags;


	bitmap = calloc(1, sizeof(osd_bitmap_t));
	if (NULL == bitmap)
		osd_die("calloc(%d,%d) failed\n", 1, sizeof(osd_bitmap_t));

	if (-1 == width)
		width = screen->w;
	if (-1 == height)
		height = screen->h;
	if (-1 == depth)
		depth = screen->format->BitsPerPixel;

	flags = SDL_SWSURFACE | SDL_ASYNCBLIT | SDL_SRCCOLORKEY;
	rmask = 0;
	gmask = 0;
	bmask = 0;
	amask = 0;
	switch (depth) {
	case 15:
#if	(SDL_BYTEORDER==SDL_BIG_ENDIAN)
		rmask = 0x7c00;
		gmask = 0x03e0;
		bmask = 0x001f;
#else
		rmask = 0x001f;
		gmask = 0x03e0;
		bmask = 0x7c00;
#endif
		break;
	case 16:
#if	(SDL_BYTEORDER==SDL_BIG_ENDIAN)
		rmask = 0xf800;
		gmask = 0x07e0;
		bmask = 0x001f;
#else
		rmask = 0x001f;
		gmask = 0x07e0;
		bmask = 0xf800;
#endif
		break;
	case 24:
#if	(SDL_BYTEORDER==SDL_BIG_ENDIAN)
		rmask = 0x00ff0000;
		gmask = 0x0000ff00;
		bmask = 0x000000ff;
#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
#endif
		break;
	case 32:
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
		break;
	}
	dst_surface = SDL_CreateRGBSurface(flags, width, height, depth, rmask, gmask, bmask, amask);

	if (NULL == dst_surface)
		return -1;

	memset(bitmap, 0, sizeof(*bitmap));
	bitmap->w = width;
	bitmap->h = height;
	bitmap->bpp = depth;
	bitmap->xscale = 1;
	bitmap->yscale = 1;
	bitmap->_private = dst_surface;

	/* set default palette colors black + white */
	osd_rgb_to_sdl_color(&palette[0], 0x00, 0x00, 0x00);
	osd_rgb_to_sdl_color(&palette[1], 0xff, 0xff, 0xff);
	SDL_SetColors(dst_surface, palette, 0, 2);
	SDL_FillRect(dst_surface, NULL, SDL_MapRGB(dst_surface->format, 0, 0, 0));

	*pbitmap = bitmap;
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
	SDL_Surface *dst_surface;
	osd_widget_t *widget, *next;
	if (NULL == pbitmap)
		return;
	if (NULL == *pbitmap)
		return;
	bitmap = *pbitmap;
	widget = bitmap->widgets;
	while (NULL != widget) {
		next = widget->next;
		osd_widget_free(widget);
		widget = next;
	}
	dst_surface = (SDL_Surface *)bitmap->_private;
	if (NULL != dst_surface) {
		SDL_FreeSurface(dst_surface);
		bitmap->_private = NULL;
	}
	if (NULL != bitmap->dirty) {
		free(bitmap->dirty);
		bitmap->dirty = NULL;
	}
	bitmap->dirty_count = 0;
	bitmap->dirty_alloc = 0;
	free(bitmap);
	*pbitmap = NULL;
}

/**
 * @brief allocate a widget inside a bitmap
 *
 * @param bitmap pointer to the osd_bitmap_t
 * @param width width of the bitmap in pixels
 * @param height height of the bitmap in pixels
 * @param depth number of bits per pixel (1, 4, 8, 15, 16, 24, 32)
 * @result returns 0 on success, -1 on error
 */
int32_t osd_widget_alloc(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t w, int32_t h,
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
}

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
osd_widget_t *osd_widget_find(osd_bitmap_t *bitmap, int32_t id)
{
	osd_widget_t *widget;

	for (widget = bitmap->widgets; widget; widget = widget->next) {
		if (widget->id == id)
			return widget;
	}
	return NULL;
}

/**
 * @brief activate or deactivate a widget inside a bitmap
 *
 * @param bitmap pointer to the osd_bitmap_t
 * @param bid widget identifier
 * @result returns 0 on success, -1 on error
 */
int32_t osd_widget_active(osd_bitmap_t *bitmap, int32_t id, int32_t active)
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
}

/**
 * @brief change the text of a widget inside a bitmap
 *
 * @param bitmap pointer to the osd_bitmap_t
 * @param bid widget identifier
 * @result returns 0 on success, -1 on error
 */
int32_t osd_widget_text(osd_bitmap_t *bitmap, int32_t id, const char *fmt, ...)
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
}

/**
 * @brief hit test a coordinate against the list of widgets of a bitmap
 *
 * @param bitmap pointer to the osd_bitmap_t
 * @param x x coordinate to check
 * @param y y coordinate to check
 * @param b widget status (mouse down)
 * @result returns widget id on success, 0 if hovering, -1 if nothing was hit
 */
int32_t osd_hittest(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t b)
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
}

/**
 * @brief return a surface specific color value for an red, green, blue triple
 *
 * @param bitmap pointer to the osd_bitmap_t to determine the color format from
 * @param r red amount
 * @param g green amount
 * @param b blue amount
 * @result returns a pixel value for the bitmap
 */
uint32_t osd_color(osd_bitmap_t *bitmap, int32_t r, int32_t g, int32_t b)
{
	SDL_Surface *dst_surface = NULL;
	if (NULL != bitmap)
		dst_surface = (SDL_Surface *)bitmap->_private;
	if (NULL == dst_surface)
		dst_surface = screen;

	if (NULL == dst_surface)
		return 0;
	
	return SDL_MapRGB(dst_surface->format, r, g, b);
}

/**
 * @brief return a surface specific color value for an red, green, blue, alpha 4-tuple
 *
 * @param bitmap pointer to the osd_bitmap_t to determine the color format from
 * @param r red amount
 * @param g green amount
 * @param b blue amount
 * @param a alpha amount
 * @result returns a pixel value for the bitmap
 */
uint32_t osd_color_alpha(osd_bitmap_t *bitmap, int32_t r, int32_t g, int32_t b, int32_t a)
{
	SDL_Surface *dst_surface = NULL;
	if (NULL != bitmap)
		dst_surface = (SDL_Surface *)bitmap->_private;
	if (NULL == dst_surface)
		dst_surface = screen;

	if (NULL == dst_surface)
		return 0;
	
	return SDL_MapRGBA(dst_surface->format, r, g, b, a);
}

/**
 * @brief return set the clipping region for a bitmap
 *
 * @param x leftmost pixel coordinate of clipping region
 * @param y topmost pixel coordinate of clipping region
 * @param w width of clipping region
 * @param h height of clipping region
 */
void osd_set_clip(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t w, int32_t h)
{
	SDL_Surface *dst_surface = NULL;
	SDL_Rect dst;
	if (NULL != bitmap)
		dst_surface = (SDL_Surface *)bitmap->_private;
	if (NULL == dst_surface) {
		dst_surface = screen;
	}
	if (0 == w && 0 == h) {
		SDL_SetClipRect(dst_surface, NULL);
		return;
	}
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;
	SDL_SetClipRect(dst_surface, &dst);
}

/**
 * @brief set a pixel color in a bitmap
 *
 * The destination bitmap may be scaled
 *
 * @param bitmap pointer to the bitmap to set a pixel in (NULL = screen surface)
 * @param x x coordinate of the pixel
 * @param y y coordinate of the pixel
 * @param pixel pixel value
 */
void osd_putpixel(osd_bitmap_t *bitmap, int32_t x, int32_t y, uint32_t pixel)
{
	SDL_Surface *dst_surface = NULL;
	SDL_Rect dst;

	if (NULL != bitmap)
		dst_surface = (SDL_Surface *)bitmap->_private;
	if (NULL == dst_surface) {
		dst_surface = screen;
		dst.x = x;
		dst.y = y;
		dst.w = 1;
		dst.h = 1;
		SDL_FillRect(dst_surface, &dst, pixel);
		osd_screen_dirty(&dst);
		return;
	}
	dst.x = x * bitmap->xscale;
	dst.y = y * bitmap->yscale;
	dst.w = bitmap->xscale;
	dst.h = bitmap->yscale;
	SDL_FillRect(dst_surface, &dst, pixel);
	osd_bitmap_dirty(bitmap, &dst);
}

/**
 * @brief return a pixel's color from a bitmap
 *
 * @param bitmap pointer to the bitmap to read a pixel from (NULL = screen surface)
 * @param x x coordinate of the pixel
 * @param y y coordinate of the pixel
 * @result return pixel value
 */
uint32_t osd_getpixel(osd_bitmap_t *bitmap, int32_t x, int32_t y)
{
	SDL_Surface *dst_surface = NULL;
	uint8_t *p;

	if (NULL != bitmap)
		dst_surface = (SDL_Surface *)bitmap->_private;
	if (NULL == dst_surface) {
		dst_surface = screen;
	} else {
		x = x * bitmap->xscale;
		y = y * bitmap->yscale;
	}

	if (x < 0 || y < 0 || x > dst_surface->w || y >= dst_surface->h)
		return 0;

	p = (uint8_t *)dst_surface->pixels + y * dst_surface->pitch +
		x * dst_surface->format->BytesPerPixel;

	switch (dst_surface->format->BytesPerPixel) {
	case 1:
		switch (dst_surface->format->BitsPerPixel) {
		case 1:
			p = (uint8_t *)dst_surface->pixels + y * dst_surface->pitch +
				(x / 8) * dst_surface->format->BytesPerPixel;
			return (*p >> (7 - (x % 8))) & 1;
		case 8:
			return *p;
		}
	case 2:
#if	(SDL_BYTEORDER==SDL_BIG_ENDIAN)
		return 256 * p[0] + p[1];
#else
		return p[0] + 256 * p[1];
#endif
	case 3:
#if	(SDL_BYTEORDER==SDL_BIG_ENDIAN)
		return 65536 * p[0] + 256 * p[1] + p[2];
#else
		return p[0] + 256 * p[1] + 65536 * p[2];
#endif
	case 4:
#if	(SDL_BYTEORDER==SDL_BIG_ENDIAN)
		return 16777216 * p[0] + 65536 * p[1] + 256 * p[2] + p[3];
#else
		return p[0] + 256 * p[1] + 65536 * p[2] + 16777216 * p[3];
#endif
	}
	return 0;
}

/**
 * @brief draw a vertical line of pixels of one color in a bitmap
 *
 * The destination bitmap may be scaled
 *
 * @param bitmap pointer to the bitmap to set a pixel in (NULL = screen surface)
 * @param x x coordinate of the vertical line
 * @param y y coordinate of the vertical line
 * @param h height of the vertical line
 * @param pixel pixel value
 */
void osd_vline(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t h, uint32_t pixel)
{
	SDL_Surface *dst_surface = NULL;
	SDL_Rect dst;

	if (NULL != bitmap)
		dst_surface = (SDL_Surface *)bitmap->_private;
	if (NULL == dst_surface) {
		dst_surface = screen;
		dst.x = x;
		dst.y = y;
		dst.w = 1;
		dst.h = h;
		SDL_FillRect(dst_surface, &dst, pixel);
		osd_screen_dirty(&dst);
		return;
	}
	dst.x = x * bitmap->xscale;
	dst.y = y * bitmap->yscale;
	dst.w = bitmap->xscale;
	dst.h = h * bitmap->yscale;
	SDL_FillRect(dst_surface, &dst, pixel);
	osd_bitmap_dirty(bitmap, &dst);
}

/**
 * @brief draw a horizontal line of pixels of one color in a bitmap
 *
 * The destination bitmap may be scaled
 *
 * @param bitmap pointer to the bitmap to set a pixel in (NULL = screen surface)
 * @param x x coordinate of the vertical line
 * @param y y coordinate of the vertical line
 * @param w width of the vertical line
 * @param pixel pixel value
 */
void osd_hline(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t w, uint32_t pixel)
{
	SDL_Surface *dst_surface = NULL;
	SDL_Rect dst;

	if (NULL != bitmap)
		dst_surface = (SDL_Surface *)bitmap->_private;
	if (NULL == dst_surface) {
		dst_surface = screen;
		dst.x = x;
		dst.y = y;
		dst.w = w;
		dst.h = 1;
		SDL_FillRect(dst_surface, &dst, pixel);
		osd_screen_dirty(&dst);
		return;
	}
	dst.x = x * bitmap->xscale;
	dst.y = y * bitmap->yscale;
	dst.w = w * bitmap->xscale;
	dst.h = bitmap->yscale;
	SDL_FillRect(dst_surface, &dst, pixel);
	osd_bitmap_dirty(bitmap, &dst);
}

/**
 * @brief frame a rectangular shape of pixels in a bitmap
 *
 * The destination bitmap may be scaled
 *
 * @param bitmap pointer to the bitmap to set a pixel in (NULL = screen surface)
 * @param x left x coordinate of the rectangle
 * @param y top y coordinate of the rectangle
 * @param w width of the rectangle
 * @param h height of the rectangle
 * @param tl pixel value for the top and left lines
 * @param br pixel value for the bottom and right lines
 */
void osd_framerect(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t w, int32_t h,
	int32_t corners, uint32_t tl, uint32_t br)
{
	if (corners) {
		osd_hline(bitmap, x, y, w, tl);
		osd_vline(bitmap, x, y, h, tl);
		osd_hline(bitmap, x, y + h - 1, w, br);
		osd_vline(bitmap, x + w - 1, y, h, br);
	} else {
		osd_hline(bitmap, x + 1, y, w - 2, tl);
		osd_vline(bitmap, x, y + 1, h - 2, tl);
		osd_hline(bitmap, x + 1, y + h - 1, w - 2, br);
		osd_vline(bitmap, x + w - 1, y + 1, h - 2, br);
	}
}

/**
 * @brief fill a rectangular shape of pixels in a bitmap
 *
 * The destination bitmap may be scaled
 *
 * @param bitmap pointer to the bitmap to set a pixel in (NULL = screen surface)
 * @param x left x coordinate of the rectangle
 * @param y top y coordinate of the rectangle
 * @param w width of the rectangle
 * @param h height of the rectangle
 * @param pixel pixel value
 */
void osd_fillrect(osd_bitmap_t *bitmap, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t pixel)
{
	SDL_Surface *dst_surface = NULL;
	SDL_Rect dst;

	if (NULL != bitmap)
		dst_surface = (SDL_Surface *)bitmap->_private;
	if (NULL == dst_surface) {
		dst_surface = screen;
		dst.x = x;
		dst.y = y;
		dst.w = w;
		dst.h = h;
		SDL_FillRect(dst_surface, &dst, pixel);
		osd_screen_dirty(&dst);
	} else {
		dst.x = x * bitmap->xscale;
		dst.y = y * bitmap->yscale;
		dst.w = w * bitmap->xscale;
		dst.h = h * bitmap->yscale;
		SDL_FillRect(dst_surface, &dst, pixel);
		osd_bitmap_dirty(bitmap, &dst);
	}
}

/**
 * @brief draw a widget with 3D edges
 *
 * The destination bitmap may be scaled
 *
 * @param bitmap pointer to the bitmap to set a pixel in (NULL = screen surface)
 * @param x left x coordinate of the rectangle
 * @param y top y coordinate of the rectangle
 * @param w width of the rectangle
 * @param h height of the rectangle
 * @param pixel pixel value
 * @param r red amount of the widget face
 * @param g green amount of the widget face
 * @param b blue amount of the widget face
 */
void osd_widget_update(osd_bitmap_t *bitmap, osd_widget_t *widget)
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
	case BT_STATIC:	/* static text */
		fg = widget->face;
		br0 = widget->br0;
		tl0 = widget->tl0;

		osd_framerect(bitmap, x, y, w, h, 1, br0, tl0);
		x += 1; y += 1; w -= 2; h -= 2;
		osd_fillrect(bitmap, x, y, w, h, fg);

		if (NULL == widget->text)
			return;

		colors[0] = fg;
		colors[1] = black;
		w = strlen(widget->text) * FONT_W;	
		h = FONT_H;
		x = 2;
		y = (widget->rect.h - h) / 2;
		for (i = 0; i < strlen(widget->text); i++) {
			osd_pattern(bitmap, font,
				x + widget->rect.x,
				y + widget->rect.y,
				widget->text[i],
				1, colors, FONT_W, FONT_H);
			x += FONT_W;
		}
		break;

	case BT_EDIT:	/* edit text */
		fg = widget->active ? widget->tl1 : widget->state == ST_HOVER ?
			widget->hover : widget->face;
		br0 = widget->br0;
		tl0 = widget->tl0;

		osd_framerect(bitmap, x, y, w, h, 1, br0, tl0);
		x += 1; y += 1; w -= 2; h -= 2;
		osd_fillrect(bitmap, x, y, w, h, fg);

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
					/* invert character */
					colors[0] = black;
					colors[1] = fg;
				}
				osd_pattern(bitmap, font, x, y, ch, 2, colors, FONT_W, FONT_H);
				x += FONT_W;
			}
		}
		break;

	case BT_CHECK:	/* check button */
		fg = widget->state == ST_HOVER ? widget->hover : widget->face;
		tl0 = widget->tl0;
		br0 = widget->br0;
		if (widget->state == ST_CLICK) {
			uint32_t tmp;
			tmp = tl0;
			tl0 = br0;
			br0 = tmp;
		}

		osd_framerect(bitmap, x, y, h, h, 0, black, black);
		x += 1; y += 1; h -= 2;
		osd_framerect(bitmap, x, y, h, h, 1, tl0, br0);
		x += 1; y += 1; h -= 2;
		osd_fillrect(bitmap, x, y, h, h, fg);
		if (widget->active) {
			x += 2; y += 1; h -= 3;
			if (widget->state == ST_CLICK) {
				x += 1;
				y += 1;
			}
			osd_hline(bitmap, x + 0, y + 4, 2, black);
			osd_hline(bitmap, x + 1, y + 5, 2, black);
			osd_hline(bitmap, x + 1, y + 6, 2, black);
			osd_hline(bitmap, x + 2, y + 7, 2, black);
			osd_hline(bitmap, x + 2, y + 6, 2, black);
			osd_hline(bitmap, x + 3, y + 5, 2, black);
			osd_hline(bitmap, x + 3, y + 4, 2, black);
			osd_hline(bitmap, x + 4, y + 3, 2, black);
			osd_hline(bitmap, x + 5, y + 2, 2, black);
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
			osd_pattern(bitmap, font,
				x + widget->rect.x,
				y + widget->rect.y,
				widget->text[i],
				1, colors, FONT_W, FONT_H);
			x += FONT_W;
		}
		break;

	case BT_PUSH:	/* push button */
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

		osd_framerect(bitmap, x, y, w, h, 0, tl1, br1);
		x += 1; y += 1; w -= 2; h -= 2;
		osd_framerect(bitmap, x, y, w, h, 1, tl0, br0);
		x += 1; y += 1; w -= 2; h -= 2;
		osd_fillrect(bitmap, x, y, w, h, fg);

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
			osd_pattern(bitmap, font,
				x + widget->rect.x,
				y + widget->rect.y,
				widget->text[i],
				1, colors, FONT_W, FONT_H);
			x += FONT_W;
		}
		break;
	}
}

/**
 * @brief set colors for a bitmap
 *
 * @param bitmap pointer to the bitmap to set a pixel in (NULL = screen surface)
 * @param colors a list of osd_colors to set
 * @param ncolors number of colors in the list
 */
void osd_set_colors(osd_bitmap_t *bitmap, uint32_t *colors, uint32_t ncolors)
{
	SDL_Surface *surface = NULL;
	SDL_Color palette[256];
	uint32_t i;

	if (NULL != bitmap)
		surface = (SDL_Surface *)bitmap->_private;
	if (NULL == surface)
		surface = screen;
	for (i = 0; i < ncolors; i++)
		osd_u32_to_sdl_color(&palette[i], colors[i]);
	SDL_SetColors(surface, palette, 0, ncolors);
}

/**
 * @brief bit block transfer
 *
 * @param dst pointer to the destination bitmap
 * @param src pointer to the source bitmap
 * @param sx source bitmap x coordinate
 * @param sy source bitmap y coordinate
 * @param w blit width in pixels
 * @param h blit height in pixels
 * @param dx destination bitmap x coordinate
 * @param dy destination bitmap y coordinate
 */
void osd_blit(osd_bitmap_t *dst, osd_bitmap_t *src, int32_t sx, int32_t sy, int32_t w, int32_t h, int32_t dx, int32_t dy)
{
	SDL_Surface *src_surface = NULL;
	SDL_Surface *dst_surface = NULL;
	SDL_Rect src_rect, dst_rect;

	if (NULL != src)
		src_surface = (SDL_Surface *)src->_private;
	if (NULL == src_surface) {
		src_surface = screen;
		/* source rectangle */
		src_rect.x = sx;
		src_rect.y = sy;
		src_rect.w = w;
		src_rect.h = h;
	} else {
		/* source rectangle */
		src_rect.x = sx * src->xscale;
		src_rect.y = sy * src->yscale;
		src_rect.w = w * src->xscale;
		src_rect.h = h * src->yscale;
	}
	if (NULL != dst)
		dst_surface = (SDL_Surface *)dst->_private;
	if (NULL == dst_surface) {
		dst_surface = screen;
		/* destination rectangle */
		dst_rect.x = dx;
		dst_rect.y = dy;
		dst_rect.w = w;
		dst_rect.h = h;
	} else {
		/* destination rectangle */
		dst_rect.x = dx * dst->xscale;
		dst_rect.y = dy * dst->yscale;
		dst_rect.w = w * dst->xscale;
		dst_rect.h = h * dst->yscale;
	}

	SDL_SetColorKey(src_surface, 0, 0);
	SDL_BlitSurface(src_surface, &src_rect, dst_surface, &dst_rect);
	if (dst_surface == screen)
		osd_screen_dirty(&dst_rect);
	else
		osd_bitmap_dirty(dst, &dst_rect);
}

/**
 * @brief return scaling factor for the video display
 * @param fmt format string and optional parameters following
 */
int32_t osd_get_scale(void)
{
	return scale;
}

static const char *humanize(size_t size)
{
	static char buff[32];
	size_t rem;
	if (size < 1024) {
		snprintf(buff, sizeof(buff), "%u", (unsigned)size);
		return buff;
	}
	size = (size + 1023) / 1024;
	if (size < 1024) {
		snprintf(buff, sizeof(buff), "%uKB", (unsigned)size);
		return buff;
	}
	rem = 100 * (size % 1024) / 1024;;
	size = size / 1024;
	snprintf(buff, sizeof(buff), "%u.%02uMB", (unsigned)size, (unsigned)rem);
	return buff;
}

static int write_fp(void *cookie, uint8_t *data, int size)
{
	int rc = 0;
	if (size != fwrite(data, 1, size, (FILE *)cookie))
		rc = -1;
	xngsize += size;
	return rc;
}

/**
 * @brief start MNG screenshot recording
 */
int32_t osd_mng_start(void)
{
	SDL_Surface *surface;
	static char comment[256];
	static char author[256];
	static char filename[FILENAME_MAX];
	FILE *fp;
	int i;

	if (NULL != mng)
		osd_mng_stop();

	snprintf(filename, sizeof(filename), "%s/screen.mng",
		sys_get_name());
	fp = fopen(filename, "wb");
	if (NULL == fp) {
		fprintf(stderr, "Cannot create osd_mng file '%s'\n", filename);
		return -1;
	}
	surface = (SDL_Surface *)frame->_private;
	mng = mng_create(frame->w, frame->h, refresh_rate, fp, write_fp);
	if (NULL == mng) {
		fprintf(stderr, "Cannot create MNG stream (%s)\n", strerror(errno));
		return -1;
	}
	for (i = 0; i < surface->format->palette->ncolors; i++) {
		SDL_Color *p = &surface->format->palette->colors[i];
		mng_set_palette(mng, i, PNG_RGB(p->r, p->g, p->b));
	}
	snprintf(comment, sizeof(comment), "%s screen MNG", sys_get_name());
	mng->comment = comment;
	snprintf(author, sizeof(author), "Emulator osd.c");
	mng->author = author;
	xngsize = 0;
	return 0;
}

/**
 * @brief stop MNG screenshot recording
 */
int32_t osd_mng_stop(void)
{
	char title[256];
	FILE *fp;
	off_t pos;
	int frames;

	if (NULL == mng)
		return -1;

	fp = xng_get_cookie(mng);
	if (NULL == fp)
		return -1;

	frames = mng_get_fcount(mng);

	/* remember where we are and seek to the file pos where the MHDR is */
	pos = ftell(fp);
	fseek(fp, 8, SEEK_SET);

	/* re-write the MHDR chunk with the (now) known counts */
	mng_write_MHDR(mng);

	/* seek to where we were and finish the mng */
	fseek(fp, pos, SEEK_SET);
	mng_finish(mng);

	mng = NULL;
	fclose(fp);

	snprintf(title, sizeof(title), "%s - %s [%d frames; %s]",
			osd_title, "finished", frames, humanize(xngsize));
	SDL_WM_SetCaption(title, osd_title);
	return 0;
}

static void osd_mng_frame_write(SDL_Surface *surface, int frame, int x, int y, int w, int h)
{
	png_t *png;
	int i, px, py;

	/* append the frame */
	if (SDL_MUSTLOCK(surface))
		SDL_LockSurface(surface);
	switch (surface->format->BytesPerPixel) {
	case 1:
		switch (surface->format->BitsPerPixel) {
		case 1:	/* monochrome surface */
			png = mng_append_png(mng, frame, x, y, w, h, COLOR_RGBTRIPLE, 8);
			if (NULL == png)
				osd_die("mng_append_png() failed (%s)\n", strerror(errno));
			png_blit_from_gray1(png, 0, 0, x, y, w, h,
				surface->pixels, surface->pitch,
				(uint32_t*)surface->format->palette->colors, 0xff);
			break;
		case 2:	/* 4 grays surface */
			png = mng_append_png(mng, frame, x, y, w, h, COLOR_RGBTRIPLE, 8);
			if (NULL == png)
				osd_die("mng_append_png() failed (%s)\n", strerror(errno));
			png_blit_from_gray2(png, 0, 0, x, y, w, h,
				surface->pixels, surface->pitch,
				(uint32_t*)surface->format->palette->colors, 0xff);
			break;
		case 4:	/* 16 grays surface */
			png = mng_append_png(mng, frame, x, y, w, h, COLOR_RGBTRIPLE, 8);
			if (NULL == png)
				osd_die("mng_append_png() failed (%s)\n", strerror(errno));
			png_blit_from_gray4(png, 0, 0, x, y, w, h,
				surface->pixels, surface->pitch,
				(uint32_t*)surface->format->palette->colors, 0xff);
			break;
		case 8:	/* paletteized surface */
			png = mng_append_png(mng, frame, x, y, w, h, COLOR_PALETTE, 8);
			if (NULL == png)
				osd_die("mng_append_png() failed (%s)\n", strerror(errno));
			for (i = 0; i < surface->format->palette->ncolors; i++) {
				SDL_Color *p = &surface->format->palette->colors[i];
				png_set_palette(png, i, PNG_RGB(p->r, p->g, p->b));
			}
			png_blit_from_pal8(png, 0, 0, x, y, w, h,
				surface->pixels, surface->pitch,
				(uint32_t*)surface->format->palette->colors, 0xff);
			break;
		}
		break;
	case 3:	/* 8:8:8 RGB surface */
		png = mng_append_png(mng, frame, x, y, w, h, COLOR_RGBTRIPLE, 8);
		if (NULL == png)
			osd_die("mng_append_png() failed (%s)\n", strerror(errno));
		png_blit_from_rgb8(png, 0, 0, x, y, w, h,
			surface->pixels, surface->pitch, NULL, 0xff);
		break;
	case 4:	/* 8:8:8:8 RGB(A) surface */
		png = mng_append_png(mng, frame, x, y, w, h, COLOR_RGBALPHA, 8);
		if (NULL == png)
			osd_die("mng_append_png() failed (%s)\n", strerror(errno));
		png_blit_from_rgba8(png, 0, 0, x, y, w, h,
			surface->pixels, surface->pitch, NULL, 0xff);
		break;
	default:
		png = mng_append_png(mng, frame, x, y, w, h, COLOR_RGBTRIPLE, 8);
		if (NULL == png)
			osd_die("mng_append_png() failed (%s)\n", strerror(errno));
		/* slow copy */
		for (py = 0; py < h; py++) {
			for (px = 0; px < w; px++) {
				uint32_t color = osd_getpixel(NULL, x + px, y + py);
				png_put_pixel(png, x, y, color & 0xffffff, 0xff);
			}
		}
	}
	if (SDL_MUSTLOCK(surface))
		SDL_UnlockSurface(surface);
}

/**
 * @brief write a screen frame to the MNG stream
 */
int32_t osd_mng_frame(void)
{
	SDL_Surface *surface;
	char title[256];
	int n, frames;

	/* recording is off, or simulation is paused */
	if (NULL == mng)
		return -1;
	if (NULL == frame)
		return -1;
	surface = (SDL_Surface *)frame->_private;
	if (NULL == surface)
		return -1;

	if (0 == dirty_count) {
		osd_mng_frame_write(surface, 1, 0, 0, 1, 1);
	} else {
		for (n = 0; n < dirty_count; n++) {
			SDL_Rect *r = &dirty[n];
			osd_mng_frame_write(surface,
				n + 1 == dirty_count ? 1 : 0,
				r->x, r->y, r->w, r->h);
		}
	}

	/* write the progress info to the border */
	frames = mng_get_fcount(mng);
	snprintf(title, sizeof(title), "%s - %s [%d frames; %s]",
		osd_title, "recording", frames, humanize(xngsize));
	SDL_WM_SetCaption(title, osd_title);
	return 0;
}

int32_t osd_save_snapshot(void)
{
	SDL_Surface *surface;
	static uint32_t no;
	char filename[FILENAME_MAX];
	struct stat st;
	FILE *fp = NULL;
	png_t *png = NULL;
	int32_t x, y, w, h, i, rc;

	if (NULL == frame)
		return -1;
	surface = (SDL_Surface *)frame->_private;
	if (NULL == surface)
		return -1;
	do {
		no++;
		snprintf(filename, sizeof(filename), "%s/snap%06x.png",
			sys_get_name(), no);
		rc = stat(filename, &st);
	} while (0 == rc);

	fp = fopen(filename, "wb");
	if (NULL == fp) {
		return -1;
	}
	w = surface->w;
	h = surface->h;

	if (SDL_MUSTLOCK(surface))
		SDL_LockSurface(surface);
	switch (surface->format->BytesPerPixel) {
	case 1:
		switch (surface->format->BitsPerPixel) {
		case 1:	/* monochrome surface */
			png = png_create(w, h, COLOR_GRAYSCALE, 1, fp, write_fp);
			if (NULL == png)
				osd_die("png_create() failed (%s)\n", strerror(errno));
			png_blit_from_gray1(png, 0, 0, 0, 0, w, h,
				surface->pixels, surface->pitch,
				(uint32_t*)surface->format->palette->colors, 0xff);
			break;
		case 2:	/* 4 grays surface */
			png = png_create(w, h, COLOR_GRAYSCALE, 1, fp, write_fp);
			if (NULL == png)
				osd_die("png_create() failed (%s)\n", strerror(errno));
			png_blit_from_gray2(png, 0, 0, 0, 0, w, h,
				surface->pixels, surface->pitch,
				(uint32_t*)surface->format->palette->colors, 0xff);
			break;
		case 4:	/* 16 grays surface */
			png = png_create(w, h, COLOR_GRAYSCALE, 1, fp, write_fp);
			if (NULL == png)
				osd_die("png_create() failed (%s)\n", strerror(errno));
			png_blit_from_gray4(png, 0, 0, 0, 0, w, h,
				surface->pixels, surface->pitch,
				(uint32_t*)surface->format->palette->colors, 0xff);
			break;
		case 8:	/* 8 bit paletteized surface */
			png = png_create(w, h, COLOR_PALETTE, 8, fp, write_fp);
			if (NULL == png)
				osd_die("png_create() failed (%s)\n", strerror(errno));
			for (i = 0; i < surface->format->palette->ncolors; i++) {
				SDL_Color *p = &surface->format->palette->colors[i];
				png_set_palette(png, i, PNG_RGB(p->r,p->g,p->b));
			}
			png_blit_from_pal8(png, 0, 0, 0, 0, w, h,
				surface->pixels, surface->pitch,
				(uint32_t*)surface->format->palette->colors, 0xff);
			break;
		}
		break;
	case 3:	/* 8:8:8 RGB surface */
		png = png_create(w, h, COLOR_RGBTRIPLE, 8, fp, write_fp);
		if (NULL == png)
			osd_die("png_create() failed (%s)\n", strerror(errno));
		png_blit_from_rgb8(png, 0, 0, 0, 0, w, h,
			surface->pixels, surface->pitch, NULL, 0xff);
		break;
	case 4:	/* 8:8:8:8 RGB(A) surface */
		png = png_create(w, h, COLOR_RGBALPHA, 8, fp, write_fp);
		if (NULL == png)
			osd_die("png_create() failed (%s)\n", strerror(errno));
		png_blit_from_rgba8(png, 0, 0, 0, 0, w, h,
			surface->pixels, surface->pitch, NULL, 0xff);
		break;
	default:
		/* slow copy */
		png = png_create(w, h, COLOR_RGBTRIPLE, 8, fp, write_fp);
		if (NULL == png)
			osd_die("png_create() failed (%s)\n", strerror(errno));
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				uint32_t px = osd_getpixel(NULL, x, y);
				png_put_pixel(png, x, y, px & 0xffffff, 0xff);
			}
		}
	}
	if (SDL_MUSTLOCK(surface))
		SDL_UnlockSurface(surface);
	png_finish(png);
	fclose(fp);
	no++;

	return 0;
}

/**
 * @brief create a cursor pixel and mask array from a string
 *
 * Pixel values are defined using one of 4 characters:
 *  .  pixel is clear, mask is clear (no change)
 *  x  pixel is set, mask is set (solid pixel)
 *  #  pixel is clear, mask is set (transparency)
 *  o  pixel is set, mask is clear (xor)
 *
 * @param pix pointer to pixel array
 * @param msk pointer to mask array
 * @param w width in pixels
 * @param h height in pixels
 * @param src string defining the cursor
 */
static int32_t make_cursor(unsigned char *pix, unsigned char *msk,
	int32_t w, int32_t h, const char *src)
{
	int32_t x, y;

	for (y = 0; y < h; y++) {
		*pix = 0;
		*msk = 0;
		for (x = 0; x < w; x++) {
			switch (*src++) {
			case '.':	/* both 0 */
				break;
			case 'x':
				*pix |= 0x80 >> (x % 8);
				*msk |= 0x80 >> (x % 8);
				break;
			case '#':
				*msk |= 0x80 >> (x % 8);
				break;
			case 'o':
				*pix |= 0x80 >> (x % 8);
				break;
			}
			if (7 == (x & 7)) {
				pix++;
				*pix = 0;
				msk++;
				*msk = 0;
			}
		}
		if (w & 7) {
			pix++;
			msk++;
		}
	}
	return 0;
}

int32_t osd_get_display(int32_t *w, int32_t *h)
{
	if (NULL == screen)
		return -1;
	*w = screen->w;
	*h = screen->h;
	return 0;
}

void osd_set_display(int32_t w, int32_t h)
{
	display_w = w;
	display_h = h;
}

int32_t osd_open_display(int32_t width, int32_t height, const char *title)
{
	uint8_t bits[128];
	uint8_t mask[128];
	uint32_t bg, rmask, gmask, bmask, amask;
	uint32_t flags;
	uint32_t i, x, y, w, h;
	const uint8_t *src;

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

	flags = SDL_HWSURFACE | SDL_ASYNCBLIT | SDL_RESIZABLE | (fullscreen ? SDL_FULLSCREEN : 0);
	screen = SDL_SetVideoMode(width, height, 8, flags);
	if (NULL == screen) {
		osd_die("SDL_SetVideoMode(%d,%d,%d,0x%x) failed\n",
			width, height, 8, flags);
	}

	osd_bitmap_alloc(&frame, width, height, 8);

	osd_bitmap_alloc(&font, 16 * FONT_W, 16 * FONT_H, 8);
	for (i = 0, src = chargen_6x10; i < 256; i++) {
		if (i != *src)
			continue;
		src++;
		osd_render_font(font, src, i, 1, FONT_W, FONT_H, FONT_W, FONT_H, 1, 1, NULL);
		src += FONT_H;
	}

	osd_bitmap_alloc(&ctrl_panel, width, 20, 32);
	ctrl_panel->x = 0;
	ctrl_panel->y = 0;
	osd_fillrect(ctrl_panel, 0, 0, ctrl_panel->w, ctrl_panel->h,
		osd_color(ctrl_panel, 0xdf, 0xdf, 0xdf));

	osd_bitmap_alloc(&cpu_panel, width, 28, 32);
	cpu_panel->x = 0;
	cpu_panel->y = screen->h - cpu_panel->h;
	osd_fillrect(cpu_panel, 0, 0, cpu_panel->w, cpu_panel->h,
		osd_color(cpu_panel, 0xdf, 0xdf, 0xdf));

	x = 4;
	y = 2;
	w = 6;
	h = 16;
	osd_widget_alloc(ctrl_panel, x, y, 7*w, h, 0xef, 0x3f, 0x3f, BT_PUSH, WID_RESET, "Reset");
	x += 7*w + 2;
	osd_widget_alloc(ctrl_panel, x, y, 4*w, h, 0xbf, 0xbf, 0xbf, BT_PUSH, WID_1X1, "1x1");
	x += 4*w + 2;
	osd_widget_alloc(ctrl_panel, x, y, 4*w, h, 0xbf, 0xbf, 0xbf, BT_PUSH, WID_2X2, "2x2");
	x += 4*w + 2;
	osd_widget_alloc(ctrl_panel, x, y, 4*w, h, 0xbf, 0xbf, 0xbf, BT_PUSH, WID_3X3, "3x3");
	x += 4*w + 2;
	osd_widget_alloc(ctrl_panel, x, y, 4*w, h, 0xbf, 0xbf, 0xbf, BT_PUSH, WID_4X4, "4x4");
	x += 4*w + 2;
	osd_widget_alloc(ctrl_panel, x, y, 11*w, h, 0xbf, 0xbf, 0xbf, BT_PUSH, WID_SNAPSHOT, "Snapshot");
	x += 11*w + 2;
	osd_widget_alloc(ctrl_panel, x, y, 11*w, h, 0xbf, 0xbf, 0xbf, BT_CHECK, WID_THROTTLE, "Throttle");
	x += 11*w + 2;
	osd_widget_alloc(ctrl_panel, x, y, 11*w, h, 0xbf, 0xbf, 0xbf, BT_CHECK, WID_VIDEO, "Video");
	x += 11*w + 2;
	osd_widget_alloc(ctrl_panel, x, y, 11*w, h, 0xbf, 0xbf, 0xbf, BT_CHECK, WID_CPU_PANEL, "CPU panel");
	x += 11*w + 2;
#if	0
	osd_widget_alloc(ctrl_panel, x, y, 11*w, h, 0xbf, 0xbf, 0xbf, BT_PUSH, WID_CASSETTE, "Cassette");
	x += 11*w + 2;
	osd_widget_alloc(ctrl_panel, x, y, 11*w, h, 0xbf, 0xbf, 0xbf, BT_PUSH, WID_FLOPPY, "Floppy");
	x += 11*w + 2;
#endif
	if (width > x) {
		x = width - 30*w;
		osd_widget_alloc(ctrl_panel, x, y, 12*w, h, 0xf0, 0xf0, 0xf0, BT_EDIT, WID_CASSETTE, "");

		x = width - 13*w;
		osd_widget_alloc(ctrl_panel, x, y, 12*w, h, 0xbf, 0xbf, 0xbf, BT_STATIC, WID_FREQUENCY, "1 MHz");
	}

	osd_widget_active(ctrl_panel, WID_THROTTLE, throttle);
	osd_widget_active(ctrl_panel, WID_VIDEO, start_video);
	osd_widget_active(ctrl_panel, WID_CPU_PANEL, cpu_panel_on);

	sys_cpu_panel_init(cpu_panel);
	osd_bitmap_update(cpu_panel);

	/* fill rect */
	bg = SDL_MapRGB(screen->format, 0, 0, 0);
	SDL_FillRect(screen, NULL, bg);
	SDL_UpdateRect(screen, 0, 0, width, height);

	if (NULL != title)
		snprintf(osd_title, sizeof(osd_title), "%s", title);
	SDL_WM_SetCaption(osd_title, osd_title);

	make_cursor(bits, mask, 12, 16,
		"xx.........." \
		"x#x........." \
		"x##x........" \
		"x###x......." \
		"x####x......" \
		"x#####x....." \
		"x######x...." \
		"x#######x..." \
		"x########x.." \
		"x#####xxxxx." \
		"x##x##x....." \
		"x#x.x##x...." \
		"xx..x##x...." \
		"x....x##x..." \
		".....x##x..." \
		"......xx....");

	cursor = SDL_CreateCursor(bits, mask, 12, 16, 0, 0);
	SDL_SetCursor(cursor);
	SDL_ShowCursor(1);

	return 0;
}

int32_t osd_close_display(void)
{
	if (NULL != screen) {
		SDL_FreeSurface(screen);
		screen = NULL;
	}
	if (NULL != cursor) {
		SDL_FreeCursor(cursor);
		cursor = NULL;
	}
	osd_bitmap_free(&font);
	osd_bitmap_free(&frame);
	osd_bitmap_free(&ctrl_panel);
	osd_bitmap_free(&cpu_panel);
	if (NULL != dirty) {
		free(dirty);
		dirty = NULL;
	}
	dirty_count = 0;
	dirty_alloc = 0;
	return 0;
}

static __inline int get_index_1bpp(const void *data, uint32_t stride, int x, int y)
{
	const uint8_t *pb = data + stride * y + x / 8;
	return (*pb >> (7 - (x % 8))) & 1;
}

static int32_t osd_render_font_1bpp(osd_bitmap_t *bitmap, const void *font, uint32_t code, uint32_t count,
	uint32_t fw, uint32_t fh, uint32_t bw, uint32_t bh, uint32_t fs, uint32_t *colors)
{
	SDL_Surface *dst_surface = NULL;
	SDL_Rect dst_rect;
	SDL_Color palette[2];
	const uint8_t *src = (const uint8_t *)font;
	uint32_t col[2], px;
	uint32_t i, x, y, x0, y0, fx, fy;

	if (NULL != bitmap)
		dst_surface = (SDL_Surface *)bitmap->_private;
	if (NULL == dst_surface)
		osd_die("osd_render_font_1bpp() _private is NULL\n");

	if (colors) {
		for (i = 0; i < 2; i++)
			osd_u32_to_sdl_color(&palette[i], colors[i]);
	} else {
		/* set palette colors black + white */
		osd_rgb_to_sdl_color(&palette[0], 0x00, 0x00, 0x00);
		osd_rgb_to_sdl_color(&palette[1], 0xff, 0xff, 0xff);
	}

	SDL_SetColors(dst_surface, palette, 0, 2);

	for (i = 0; i < 2; i++)
		col[i] = SDL_MapRGB(dst_surface->format,
			palette[i].r, palette[i].g, palette[i].b);

	while (count-- > 0) {
		x0 = (code % 16) * bw * bitmap->xscale;
		y0 = (code / 16) * bh * bitmap->yscale;
		for (y = 0; y < bh; y++) {
			fy = y * fh / bh;
			dst_rect.y = y0 + y * bitmap->yscale;
			dst_rect.w = bitmap->xscale;
			dst_rect.h = bitmap->yscale;
			for (x = 0; x < bw; x++) {
				fx = x * fw / bw;
				px = col[get_index_1bpp(src, fs, fx, fy)];
				dst_rect.x = x0 + x * bitmap->xscale;
				SDL_FillRect(dst_surface, &dst_rect, px);
			}
		}
		src += fh * fs;
		code++;
	}

	return 0;
}

static __inline int get_index_2bpp(const void *data, uint32_t stride, int x, int y)
{
	const uint8_t *pb = (const uint8_t *)data + stride * y + x / 4;
	return (*pb >> (3 - (x % 4))) & 3;
}

static int32_t osd_render_font_2bpp(osd_bitmap_t *bitmap, const void *font, uint32_t code, uint32_t count,
	uint32_t fw, uint32_t fh, uint32_t bw, uint32_t bh, uint32_t fs, uint32_t *colors)
{
	SDL_Surface *dst_surface = NULL;
	SDL_Rect dst_rect;
	SDL_Color palette[4];
	const uint8_t *src = (const uint8_t *)font;
	uint32_t col[4], px;
	uint32_t i, x, y, x0, y0, fx, fy;

	if (NULL != bitmap)
		dst_surface = (SDL_Surface *)bitmap->_private;
	if (NULL == dst_surface)
		osd_die("osd_render_font_2bpp() _private is NULL\n");

	if (colors) {
		for (i = 0; i < 4; i++)
			osd_u32_to_sdl_color(&palette[i], colors[i]);
	} else {
		/* set palette colors 4 shades of gray */
		osd_rgb_to_sdl_color(&palette[0], 0x00, 0x00, 0x00);
		osd_rgb_to_sdl_color(&palette[1], 0x55, 0x55, 0x55);
		osd_rgb_to_sdl_color(&palette[2], 0xaa, 0xaa, 0xaa);
		osd_rgb_to_sdl_color(&palette[3], 0xff, 0xff, 0xff);
	}

	SDL_SetColors(dst_surface, palette, 0, 4);

	for (i = 0; i < 4; i++)
		col[i] = SDL_MapRGB(dst_surface->format,
			palette[i].r, palette[i].g, palette[i].b);

	while (count-- > 0) {
		x0 = (code % 16) * bw * bitmap->xscale;
		y0 = (code / 16) * bh * bitmap->yscale;
		for (y = 0; y < bh; y++) {
			fy = y * fh / bh;
			dst_rect.y = y0 + y * bitmap->yscale;
			dst_rect.w = bitmap->xscale;
			dst_rect.h = bitmap->yscale;
			for (x = 0; x < bw; x++) {
				fx = x * fw / bw;
				px = col[get_index_2bpp(src, fs, fx, fy)];
				dst_rect.x = x0 + x * bitmap->xscale;
				SDL_FillRect(dst_surface, &dst_rect, px);
			}
		}
		src += fh * fs;
		code++;
	}

	return 0;
}

static __inline int get_index_4bpp(const void *data, uint32_t stride, int x, int y)
{
	const uint8_t *pb = (const uint8_t *)data + stride * y + x / 2;
	return (*pb >> (1 - (x % 2))) & 15;
}

static int32_t osd_render_font_4bpp(osd_bitmap_t *bitmap, const void *font, uint32_t code, uint32_t count,
	uint32_t fw, uint32_t fh, uint32_t bw, uint32_t bh, uint32_t fs, uint32_t *colors)
{
	SDL_Surface *dst_surface = NULL;
	SDL_Rect dst_rect;
	SDL_Color palette[16];
	const uint8_t *src = (const uint8_t *)font;
	uint32_t col[16], px;
	uint32_t i, x, y, x0, y0, fx, fy;

	if (NULL != bitmap)
		dst_surface = (SDL_Surface *)bitmap->_private;
	if (NULL == dst_surface)
		osd_die("osd_render_font_4bpp() _private is NULL\n");

	if (colors) {
		for (i = 0; i < 16; i++)
			osd_u32_to_sdl_color(&palette[i], colors[i]);
	} else {
		/* set palette colors 16 shades of gray */
		for (i = 0; i < 16; i++)
			osd_rgb_to_sdl_color(&palette[i], 0x11*i, 0x11*i, 0x11*i);
	}

	SDL_SetColors(dst_surface, palette, 0, 16);

	for (i = 0; i < 16; i++)
		col[i] = SDL_MapRGB(dst_surface->format,
			palette[i].r, palette[i].g, palette[i].b);

	while (count-- > 0) {
		x0 = (code % 16) * bw * bitmap->xscale;
		y0 = (code / 16) * bh * bitmap->yscale;
		for (y = 0; y < bh; y++) {
			fy = y * fh / bh;
			dst_rect.y = y0 + y * bitmap->yscale;
			dst_rect.w = bitmap->xscale;
			dst_rect.h = bitmap->yscale;
			for (x = 0; x < bw; x++) {
				fx = x * fw / bw;
				px = col[get_index_4bpp(src, fs, fx, fy)];
				dst_rect.x = x0 + x * bitmap->xscale;
				SDL_FillRect(dst_surface, &dst_rect, px);
			}
		}
		src += fh * fs;
		code++;
	}

	return 0;
}

int32_t osd_render_font(osd_bitmap_t *bitmap, const void *font,
	uint32_t code, uint32_t count,
	uint32_t fw, uint32_t fh, uint32_t bw, uint32_t bh, uint32_t fs,
	uint32_t bpp, uint32_t *colors)
{
	switch (bpp) {
	case 1:	return osd_render_font_1bpp(bitmap, font, code, count, fw, fh, bw, bh, fs, colors);
	case 2:	return osd_render_font_2bpp(bitmap, font, code, count, fw, fh, bw, bh, fs, colors);
	case 4:	return osd_render_font_4bpp(bitmap, font, code, count, fw, fh, bw, bh, fs, colors);
	}
	return -1;
}

void osd_pattern(osd_bitmap_t *dst, osd_bitmap_t *src, int32_t x, int32_t y, uint32_t code,
	uint32_t ncolors, uint32_t *colors, uint32_t pw, uint32_t ph)
{
	SDL_Surface *src_surface = NULL;
	SDL_Surface *dst_surface = NULL;
	SDL_Color palette[256];
	SDL_Rect src_rect, dst_rect;
	uint32_t i;

	if (NULL != dst)
		dst_surface = (SDL_Surface *)dst->_private;
	if (NULL == dst_surface)
		dst_surface = screen;
	if (NULL != src)
		src_surface = (SDL_Surface *)src->_private;
	if (NULL == src_surface)
		osd_die("osd_pattern() src->_private is NULL\n");

	pw = pw * src->xscale;
	ph = ph * src->yscale;

	/* source rect is derived from code */
	src_rect.x = (code % 16) * pw;
	src_rect.y = (code / 16) * ph;
	src_rect.w = pw;
	src_rect.h = ph;
	if (dst_surface == screen) {
		/* destination rect screen */
		dst_rect.x = x;
		dst_rect.y = y;
		dst_rect.w = pw;
		dst_rect.h = ph;
	} else {
		/* destination rect bitmap */
		dst_rect.x = x * dst->xscale;
		dst_rect.y = y * dst->yscale;
		dst_rect.w = pw;
		dst_rect.h = ph;
	}

	if (ncolors > 0) {
		if (1 == ncolors) {
			osd_u32_to_sdl_color(&palette[0], 0x00000000);
			osd_u32_to_sdl_color(&palette[1], colors[1]);
			SDL_SetColors(src_surface, palette, 0, 2);
			SDL_SetColorKey(src_surface, SDL_SRCCOLORKEY,
				SDL_MapRGB(src_surface->format, palette[0].r, palette[0].g, palette[0].b));
		} else {
			for (i = 0; i < ncolors; i++)
				osd_u32_to_sdl_color(&palette[i], colors[i]);
			SDL_SetColors(src_surface, palette, 0, ncolors);
			SDL_SetColorKey(src_surface, 0, 0);
		}
	}
	SDL_BlitSurface(src_surface, &src_rect, dst_surface, &dst_rect);
	if (dst_surface == screen)
		osd_screen_dirty(&dst_rect);
	else
		osd_bitmap_dirty(dst, &dst_rect);
}

int32_t osd_keys(SDL_KeyboardEvent *key)
{
	/* RCTRL is for OSD keys */
	if (0 == (key->keysym.mod & KMOD_RCTRL))
		return 0;

	if (key->keysym.sym == SDLK_RETURN) {
		if (SDL_WM_ToggleFullScreen(screen))
			fullscreen ^= 1;
		SDL_ShowCursor(fullscreen ^ 1);
		return 1;
	}

	/* OSD+DELETE = toggle throttle */
	if (key->keysym.sym == SDLK_DELETE ||
		key->keysym.sym == SDLK_KP_PERIOD) {
		throttle ^= 1;
		osd_widget_active(ctrl_panel, WID_THROTTLE, throttle);
		return 1;
	}

	/* OSD+INSERT = grab/release input */
	if (key->keysym.sym == SDLK_INSERT ||
		key->keysym.sym == SDLK_KP0) {
		if (SDL_GRAB_ON == SDL_WM_GrabInput(SDL_GRAB_QUERY)) {
			SDL_WM_GrabInput(SDL_GRAB_OFF);
		} else {
			SDL_WM_GrabInput(SDL_GRAB_ON);
		}
		return 1;
	}

	/* OSD+SYSREQ = toggle CPU panel */
	if (key->keysym.sym == SDLK_HOME ||
		key->keysym.sym == SDLK_KP7) {
		cpu_panel_on ^= 1;
		osd_widget_active(ctrl_panel, WID_CPU_PANEL, cpu_panel_on);
		return 1;
	}


	/* OSD+END = exit the hard way */
	if (key->keysym.sym == SDLK_END ||
		key->keysym.sym == SDLK_KP1) {
		exit(1);
		return 1;
	}
	return 0;
}

int32_t osd_update(int32_t skip_this_frame)
{
	SDL_Rect dst;
	SDL_Event ev;
	osd_key_t key;
	int32_t rc;
	int32_t w, h;


	/* record video right from the start? */
	if (start_video) {
		start_video = 0;
		osd_mng_start();
	}
	osd_bitmap_update(frame);
	if (NULL != mng)
		osd_mng_frame();

	if (ctrl_panel_on)
		osd_hittest(ctrl_panel, mousex, mousey, mouseb);

	if (0 == skip_this_frame) {
		if (ctrl_panel_on) {
			osd_blit(NULL, ctrl_panel, 0, 0, ctrl_panel->w, ctrl_panel->h, ctrl_panel->x, ctrl_panel->y);
			dst.x = ctrl_panel->x;
			dst.y = ctrl_panel->y;
			dst.w = ctrl_panel->w;
			dst.h = ctrl_panel->h;
			osd_bitmap_dirty(frame, &dst);
		}
		if (cpu_panel_on) {
			sys_cpu_panel_update(cpu_panel);
			osd_hittest(cpu_panel, mousex, mousey, mouseb);
			osd_blit(NULL, cpu_panel, 0, 0, cpu_panel->w, cpu_panel->h, cpu_panel->x, cpu_panel->y);
			dst.x = cpu_panel->x;
			dst.y = cpu_panel->y;
			dst.w = cpu_panel->w;
			dst.h = cpu_panel->h;
			osd_bitmap_dirty(frame, &dst);
		}
		if (dirty_count > 0) {
			/* update rectangles */
			if (NULL != screen)
				SDL_UpdateRects(screen, dirty_count, dirty);
			dirty_count = 0;
		}
	}

	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_VIDEORESIZE:
			osd_close_display();
			osd_open_display(ev.resize.w, ev.resize.h, NULL);
			if (resize_callback)
				(*resize_callback)(ev.resize.w, ev.resize.h);
			break;

		case SDL_KEYDOWN:
			osd_keys(&ev.key);
			ev.key.keysym.mod &= ~KMOD_RSHIFT;
			key.flags = OSD_KEY_SCANCODE | OSD_KEY_SYM | OSD_KEY_MOD | OSD_KEY_UNICODE;
			key.scancode = ev.key.keysym.scancode;
			key.sym = ev.key.keysym.sym;
			key.mod = ev.key.keysym.mod;
			key.unicode = ev.key.keysym.unicode;
			if (keydn_osd_local) {
				(*keydn_osd_local)(cookie_local, &key);
			} else if (keydn_callback) {
				(*keydn_callback)(cookie_callback, &key);
			}
			break;

		case SDL_KEYUP:
			ev.key.keysym.mod &= ~KMOD_RSHIFT;
			key.flags = OSD_KEY_SCANCODE | OSD_KEY_SYM | OSD_KEY_MOD;
			key.scancode = ev.key.keysym.scancode;
			key.sym = ev.key.keysym.sym;
			key.mod = ev.key.keysym.mod;
			key.unicode = 0;
			if (keyup_osd_local) {
				(*keyup_osd_local)(cookie_local, &key);
			} else if (keyup_callback) {
				(*keyup_callback)(cookie_callback, &key);
			}
			break;

		case SDL_MOUSEMOTION:
			mousex = ev.motion.x;
			mousey = ev.motion.y;
			mouseb = (mouseb & ~1) | (ev.motion.state & 1);
			break;

		case SDL_MOUSEBUTTONDOWN:
			mouseb |= 1 | (1 << ev.button.button);
			if (3 == ev.button.button)
				ctrl_panel_on ^= 1;
			if (!ctrl_panel_on)
				break;

			rc = osd_hittest(ctrl_panel, mousex, mousey, mouseb);
			switch (rc) {
			case WID_RESET:
				sys_reset(SYS_RST);
				break;
			case WID_THROTTLE:
				throttle ^= 1;
				osd_widget_active(ctrl_panel, WID_THROTTLE, throttle);
				break;
			case WID_1X1:
				w = display_w;
				h = display_h;
				osd_close_display();
				osd_open_display(w, h, NULL);
				if (resize_callback)
					(*resize_callback)(w, h);
				break;
			case WID_2X2:
				w = 2 * display_w;
				h = 2 * display_h;
				osd_close_display();
				osd_open_display(w, h, NULL);
				if (resize_callback)
					(*resize_callback)(w, h);
				break;
			case WID_3X3:
				w = 3 * display_w;
				h = 3 * display_h;
				osd_close_display();
				osd_open_display(w, h, NULL);
				if (resize_callback)
					(*resize_callback)(w, h);
				break;
			case WID_4X4:
				w = 4 * display_w;
				h = 4 * display_h;
				osd_close_display();
				osd_open_display(w, h, NULL);
				if (resize_callback)
					(*resize_callback)(w, h);
				break;
			case WID_SNAPSHOT:
				osd_save_snapshot();
				break;
			case WID_VIDEO:
				if (NULL != mng) {
					osd_mng_stop();
					osd_widget_active(ctrl_panel, WID_VIDEO, 0);
				} else {
					osd_mng_start();
					osd_widget_active(ctrl_panel, WID_VIDEO, 1);
				}
				break;
			case WID_CPU_PANEL:
				cpu_panel_on ^= 1;
				osd_widget_active(ctrl_panel, WID_CPU_PANEL, cpu_panel_on);
				break;

			case WID_CASSETTE:
				if (NULL == keydn_osd_local) {
					static cookie_local_t cookie;

					cookie.bitmap = ctrl_panel;
					cookie.id = WID_CASSETTE;
					keydn_osd_local = keydn_edit;
					keyup_osd_local = keyup_edit;
					cookie_local = (void *)&cookie;
					osd_widget_active(ctrl_panel, WID_CASSETTE, 1);
				}
				break;
			}
			break;

		case SDL_MOUSEBUTTONUP:
			mouseb &= ~(1 | (1 << ev.button.button));
			if (ctrl_panel_on)
				osd_hittest(ctrl_panel, mousex, mousey, mouseb);
			break;

		case SDL_QUIT:
			return -1;
		}
	}

	return 0;
}

static char *insert_char(char *src, int32_t offs, int32_t ch)
{
	int32_t len = src ? strlen(src) : 0;
	char *dst;

	if (0 == len) {
		dst = calloc(2, sizeof(char));
		dst[0] = ch;
		return dst;
	}
	if (offs < len) {
		dst = realloc(src, len + 2);
		memmove(src + offs + 1, src + offs, len - offs);
		dst[offs] = ch;
		return dst;
	}
	dst = realloc(src, len + 2);
	dst[len + 0] = ch;
	dst[len + 1] = '\0';
	return dst;
}

static char *delete_char(char *src, int32_t offs)
{
	int32_t len = src ? strlen(src) : 0;

	if (0 == len)
		return src;
	if (offs < len) {
		memmove(src + offs, src + offs + 1, len - offs);
		return src;
	}
	return src;
}

static void keydn_edit(void *cookie, osd_key_t *key)
{
	cookie_local_t *c = (cookie_local_t *)cookie;
	osd_widget_t *e;
	int32_t len, w;

	if (NULL == c)
		return;
	e = osd_widget_find(c->bitmap, c->id);
	len = e->text ? strlen(e->text) : 0;
	w = e->cwidth;

	switch (key->unicode) {
	case 13:
		keydn_osd_local = NULL;
		keyup_osd_local = NULL;
		cookie_local = NULL;
		osd_widget_active(c->bitmap, c->id, 0);
		return;
	default:
		if (key->unicode < ' ')
			break;
		e->text = insert_char(e->text, e->cursor, key->unicode);
		e->cursor++;
		break;
	}
	switch (key->sym) {
	case SDLK_HOME:
		e->cursor = 0;
		e->offset = 0;
		break;

	case SDLK_END:
		e->cursor = len;
		break;

	case SDLK_LEFT:
		if (e->cursor > 0)
			e->cursor--;
		break;

	case SDLK_RIGHT:
		if (e->cursor < len)
			e->cursor++;
		break;

	case SDLK_BACKSPACE:
		if (0 == e->cursor)
			break;
		e->cursor--;
		e->text = delete_char(e->text, e->cursor);
		break;

	case SDLK_DELETE:
		e->text = delete_char(e->text, e->cursor);
		break;
	}
	if (e->cursor < e->offset)
		e->offset = e->cursor;
	if (e->cursor - e->offset >= w - 1)
		e->offset = e->cursor - w + 1;
}

static void keyup_edit(void *cookie, osd_key_t *key)
{
}

uint8_t *osd_get_key_state(int32_t *numkeys)
{
	return SDL_GetKeyState(numkeys);
}

const char *osd_key_name(osd_key_t *key)
{
	switch (key->sym) {
	case SDLK_BACKSPACE:		return "BACKSPACE";
	case SDLK_TAB:			return "TAB";
	case SDLK_CLEAR:		return "CLEAR";
	case SDLK_RETURN:		return "RETURN";
	case SDLK_PAUSE:		return "PAUSE";
	case SDLK_ESCAPE:		return "ESCAPE";
	case SDLK_SPACE:		return "SPACE";
	case SDLK_EXCLAIM:		return "EXCLAIM";
	case SDLK_QUOTEDBL:		return "QUOTEDBL";
	case SDLK_HASH:			return "HASH";
	case SDLK_DOLLAR:		return "DOLLAR";
	case SDLK_AMPERSAND:		return "AMPERSAND";
	case SDLK_QUOTE:		return "QUOTE";
	case SDLK_LEFTPAREN:		return "LEFTPAREN";
	case SDLK_RIGHTPAREN:		return "RIGHTPAREN";
	case SDLK_ASTERISK:		return "ASTERISK";
	case SDLK_PLUS:			return "PLUS";
	case SDLK_COMMA:		return "COMMA";
	case SDLK_MINUS:		return "MINUS";
	case SDLK_PERIOD:		return "PERIOD";
	case SDLK_SLASH:		return "SLASH";
	case SDLK_0:			return "0";
	case SDLK_1:			return "1";
	case SDLK_2:			return "2";
	case SDLK_3:			return "3";
	case SDLK_4:			return "4";
	case SDLK_5:			return "5";
	case SDLK_6:			return "6";
	case SDLK_7:			return "7";
	case SDLK_8:			return "8";
	case SDLK_9:			return "9";
	case SDLK_COLON:		return "COLON";
	case SDLK_SEMICOLON:		return "SEMICOLON";
	case SDLK_LESS:			return "LESS";
	case SDLK_EQUALS:		return "EQUALS";
	case SDLK_GREATER:		return "GREATER";
	case SDLK_QUESTION:		return "QUESTION";
	case SDLK_AT:			return "AT";
	/* Skip uppercase letters */
	case SDLK_LEFTBRACKET:		return "LEFTBRACKET";
	case SDLK_BACKSLASH:		return "BACKSLASH";
	case SDLK_RIGHTBRACKET:		return "RIGHTBRACKET";
	case SDLK_CARET:		return "CARET";
	case SDLK_UNDERSCORE:		return "UNDERSCORE";
	case SDLK_BACKQUOTE:		return "BACKQUOTE";
	case SDLK_a:			return "a";
	case SDLK_b:			return "b";
	case SDLK_c:			return "c";
	case SDLK_d:			return "d";
	case SDLK_e:			return "e";
	case SDLK_f:			return "f";
	case SDLK_g:			return "g";
	case SDLK_h:			return "h";
	case SDLK_i:			return "i";
	case SDLK_j:			return "j";
	case SDLK_k:			return "k";
	case SDLK_l:			return "l";
	case SDLK_m:			return "m";
	case SDLK_n:			return "n";
	case SDLK_o:			return "o";
	case SDLK_p:			return "p";
	case SDLK_q:			return "q";
	case SDLK_r:			return "r";
	case SDLK_s:			return "s";
	case SDLK_t:			return "t";
	case SDLK_u:			return "u";
	case SDLK_v:			return "v";
	case SDLK_w:			return "w";
	case SDLK_x:			return "x";
	case SDLK_y:			return "y";
	case SDLK_z:			return "z";
	case SDLK_DELETE:		return "DELETE";
	/* End of ASCII mapped keysyms */

	/* International keyboard syms */
	case SDLK_WORLD_0:		return "WORLD_0";
	case SDLK_WORLD_1:		return "WORLD_1";
	case SDLK_WORLD_2:		return "WORLD_2";
	case SDLK_WORLD_3:		return "WORLD_3";
	case SDLK_WORLD_4:		return "WORLD_4";
	case SDLK_WORLD_5:		return "WORLD_5";
	case SDLK_WORLD_6:		return "WORLD_6";
	case SDLK_WORLD_7:		return "WORLD_7";
	case SDLK_WORLD_8:		return "WORLD_8";
	case SDLK_WORLD_9:		return "WORLD_9";
	case SDLK_WORLD_10:		return "WORLD_10";
	case SDLK_WORLD_11:		return "WORLD_11";
	case SDLK_WORLD_12:		return "WORLD_12";
	case SDLK_WORLD_13:		return "WORLD_13";
	case SDLK_WORLD_14:		return "WORLD_14";
	case SDLK_WORLD_15:		return "WORLD_15";
	case SDLK_WORLD_16:		return "WORLD_16";
	case SDLK_WORLD_17:		return "WORLD_17";
	case SDLK_WORLD_18:		return "WORLD_18";
	case SDLK_WORLD_19:		return "WORLD_19";
	case SDLK_WORLD_20:		return "WORLD_20";
	case SDLK_WORLD_21:		return "WORLD_21";
	case SDLK_WORLD_22:		return "WORLD_22";
	case SDLK_WORLD_23:		return "WORLD_23";
	case SDLK_WORLD_24:		return "WORLD_24";
	case SDLK_WORLD_25:		return "WORLD_25";
	case SDLK_WORLD_26:		return "WORLD_26";
	case SDLK_WORLD_27:		return "WORLD_27";
	case SDLK_WORLD_28:		return "WORLD_28";
	case SDLK_WORLD_29:		return "WORLD_29";
	case SDLK_WORLD_30:		return "WORLD_30";
	case SDLK_WORLD_31:		return "WORLD_31";
	case SDLK_WORLD_32:		return "WORLD_32";
	case SDLK_WORLD_33:		return "WORLD_33";
	case SDLK_WORLD_34:		return "WORLD_34";
	case SDLK_WORLD_35:		return "WORLD_35";
	case SDLK_WORLD_36:		return "WORLD_36";
	case SDLK_WORLD_37:		return "WORLD_37";
	case SDLK_WORLD_38:		return "WORLD_38";
	case SDLK_WORLD_39:		return "WORLD_39";
	case SDLK_WORLD_40:		return "WORLD_40";
	case SDLK_WORLD_41:		return "WORLD_41";
	case SDLK_WORLD_42:		return "WORLD_42";
	case SDLK_WORLD_43:		return "WORLD_43";
	case SDLK_WORLD_44:		return "WORLD_44";
	case SDLK_WORLD_45:		return "WORLD_45";
	case SDLK_WORLD_46:		return "WORLD_46";
	case SDLK_WORLD_47:		return "WORLD_47";
	case SDLK_WORLD_48:		return "WORLD_48";
	case SDLK_WORLD_49:		return "WORLD_49";
	case SDLK_WORLD_50:		return "WORLD_50";
	case SDLK_WORLD_51:		return "WORLD_51";
	case SDLK_WORLD_52:		return "WORLD_52";
	case SDLK_WORLD_53:		return "WORLD_53";
	case SDLK_WORLD_54:		return "WORLD_54";
	case SDLK_WORLD_55:		return "WORLD_55";
	case SDLK_WORLD_56:		return "WORLD_56";
	case SDLK_WORLD_57:		return "WORLD_57";
	case SDLK_WORLD_58:		return "WORLD_58";
	case SDLK_WORLD_59:		return "WORLD_59";
	case SDLK_WORLD_60:		return "WORLD_60";
	case SDLK_WORLD_61:		return "WORLD_61";
	case SDLK_WORLD_62:		return "WORLD_62";
	case SDLK_WORLD_63:		return "WORLD_63";
	case SDLK_WORLD_64:		return "WORLD_64";
	case SDLK_WORLD_65:		return "WORLD_65";
	case SDLK_WORLD_66:		return "WORLD_66";
	case SDLK_WORLD_67:		return "WORLD_67";
	case SDLK_WORLD_68:		return "WORLD_68";
	case SDLK_WORLD_69:		return "WORLD_69";
	case SDLK_WORLD_70:		return "WORLD_70";
	case SDLK_WORLD_71:		return "WORLD_71";
	case SDLK_WORLD_72:		return "WORLD_72";
	case SDLK_WORLD_73:		return "WORLD_73";
	case SDLK_WORLD_74:		return "WORLD_74";
	case SDLK_WORLD_75:		return "WORLD_75";
	case SDLK_WORLD_76:		return "WORLD_76";
	case SDLK_WORLD_77:		return "WORLD_77";
	case SDLK_WORLD_78:		return "WORLD_78";
	case SDLK_WORLD_79:		return "WORLD_79";
	case SDLK_WORLD_80:		return "WORLD_80";
	case SDLK_WORLD_81:		return "WORLD_81";
	case SDLK_WORLD_82:		return "WORLD_82";
	case SDLK_WORLD_83:		return "WORLD_83";
	case SDLK_WORLD_84:		return "WORLD_84";
	case SDLK_WORLD_85:		return "WORLD_85";
	case SDLK_WORLD_86:		return "WORLD_86";
	case SDLK_WORLD_87:		return "WORLD_87";
	case SDLK_WORLD_88:		return "WORLD_88";
	case SDLK_WORLD_89:		return "WORLD_89";
	case SDLK_WORLD_90:		return "WORLD_90";
	case SDLK_WORLD_91:		return "WORLD_91";
	case SDLK_WORLD_92:		return "WORLD_92";
	case SDLK_WORLD_93:		return "WORLD_93";
	case SDLK_WORLD_94:		return "WORLD_94";
	case SDLK_WORLD_95:		return "WORLD_95";

	/* Numeric keypad */
	case SDLK_KP0:			return "KP0";
	case SDLK_KP1:			return "KP1";
	case SDLK_KP2:			return "KP2";
	case SDLK_KP3:			return "KP3";
	case SDLK_KP4:			return "KP4";
	case SDLK_KP5:			return "KP5";
	case SDLK_KP6:			return "KP6";
	case SDLK_KP7:			return "KP7";
	case SDLK_KP8:			return "KP8";
	case SDLK_KP9:			return "KP9";
	case SDLK_KP_PERIOD:		return "KP_PERIOD";
	case SDLK_KP_DIVIDE:		return "KP_DIVIDE";
	case SDLK_KP_MULTIPLY:		return "KP_MULTIPLY";
	case SDLK_KP_MINUS:		return "KP_MINUS";
	case SDLK_KP_PLUS:		return "KP_PLUS";
	case SDLK_KP_ENTER:		return "KP_ENTER";
	case SDLK_KP_EQUALS:		return "KP_EQUALS";

	/* Arrows + Home/End pad */
	case SDLK_UP:			return "UP";
	case SDLK_DOWN:			return "DOWN";
	case SDLK_RIGHT:		return "RIGHT";
	case SDLK_LEFT:			return "LEFT";
	case SDLK_INSERT:		return "INSERT";
	case SDLK_HOME:			return "HOME";
	case SDLK_END:			return "END";
	case SDLK_PAGEUP:		return "PAGEUP";
	case SDLK_PAGEDOWN:		return "PAGEDOWN";

	/* Function keys */
	case SDLK_F1:			return "F1";
	case SDLK_F2:			return "F2";
	case SDLK_F3:			return "F3";
	case SDLK_F4:			return "F4";
	case SDLK_F5:			return "F5";
	case SDLK_F6:			return "F6";
	case SDLK_F7:			return "F7";
	case SDLK_F8:			return "F8";
	case SDLK_F9:			return "F9";
	case SDLK_F10:			return "F10";
	case SDLK_F11:			return "F11";
	case SDLK_F12:			return "F12";
	case SDLK_F13:			return "F13";
	case SDLK_F14:			return "F14";
	case SDLK_F15:			return "F15";

	/* Key state modifier keys */
	case SDLK_NUMLOCK:		return "NUMLOCK";
	case SDLK_CAPSLOCK:		return "CAPSLOCK";
	case SDLK_SCROLLOCK:		return "SCROLLOCK";
	case SDLK_RSHIFT:		return "RSHIFT";
	case SDLK_LSHIFT:		return "LSHIFT";
	case SDLK_RCTRL:		return "RCTRL";
	case SDLK_LCTRL:		return "LCTRL";
	case SDLK_RALT:			return "RALT";
	case SDLK_LALT:			return "LALT";
	case SDLK_RMETA:		return "RMETA";
	case SDLK_LMETA:		return "LMETA";
	case SDLK_LSUPER:		return "LSUPER";
	case SDLK_RSUPER:		return "RSUPER";
	case SDLK_MODE:			return "MODE";
	case SDLK_COMPOSE:		return "COMPOSE";

	/* Miscellaneous function keys */
	case SDLK_HELP:			return "HELP";
	case SDLK_PRINT:		return "PRINT";
	case SDLK_SYSREQ:		return "SYSREQ";
	case SDLK_BREAK:		return "BREAK";
	case SDLK_MENU:			return "MENU";
	case SDLK_POWER:		return "POWER";
	case SDLK_EURO:			return "EURO";
	case SDLK_UNDO:			return "UNDO";
	}
	return "UNKNOWN";
}

int32_t osd_mousex(void)
{
	return mousex;
}

int32_t osd_mousey(void)
{
	return mousey;
}

int32_t osd_mouseb(void)
{
	return mouseb;
}

uint32_t osd_get_sample_rate(void)
{
	return sample_rate;
}

void osd_set_sample_rate(uint32_t rate)
{
	sample_rate = rate;
}

void osd_set_refresh_rate(double rate)
{
	refresh_rate = rate;
}

uint32_t osd_update_audio_stream(int16_t *stream)
{
	sbuff_t *sb = (sbuff_t *)sbuff;
	int16_t *src, *dst;
	int32_t size, copy;

	if (NULL == sb)
		return 0;

	SDL_LockAudio();
	src = (int16_t *)stream;
	dst = sb->buffer + sb->head;
	size = sample_rate / refresh_rate;
	copy = sb->size - sb->head;
	sb->head += size;
	if (copy > size)
		copy = size;
	if (sb->head >= sb->size)
		sb->head -= sb->size;
	if (copy > 0) {
		memcpy(dst, src, copy * sizeof(*dst));
		src += copy;
		copy = size - copy;
	}
	if (copy > 0) {
		dst = sb->buffer;
		memcpy(dst, src, copy * sizeof(*dst));
	}
	SDL_UnlockAudio();

	return size;
}

void osd_flush_audio_stream(void *userdata, uint8_t *stream, int32_t len)
{
	sbuff_t *sb = (sbuff_t *)userdata;
	int16_t *src, *dst;
	int32_t size, copy;

	if (NULL == sb)
		return;

	src = sb->buffer + sb->tail;
	dst = (int16_t *)stream;
	size = len / sizeof(*dst);
	copy = sb->size - sb->tail;
	sb->tail += size;
	if (copy > size)
		copy = size;
	if (sb->tail >= sb->size)
		sb->tail -= sb->size;
	if (copy > 0) {
		memcpy(dst, src, copy * sizeof(*dst));
		dst += copy;
		copy = size - copy;
	}
	if (copy > 0) {
		src = sb->buffer;
		memcpy(dst, src, copy * sizeof(*dst));
	}

}

void osd_stop_audio_stream(void)
{
	SDL_PauseAudio(1);

	SDL_LockAudio();
	if (NULL != sbuff) {
		free(sbuff);
		sbuff = NULL;
	}

	if (NULL != audio_spec_obtained) {
		free(audio_spec_obtained);
		audio_spec_obtained = NULL;
	}
	SDL_UnlockAudio();
}

int32_t osd_start_audio_stream(int32_t stereo)
{
	int32_t samples;
	int32_t rc;

	rc = SDL_InitSubSystem(SDL_INIT_AUDIO);
	if (0 != rc) {
		fprintf(stderr, "osd_start_audio_stream: SDL_InitSubSystem(SDL_INIT_AUDIO) failed (%d)\n", rc);
		return -1;
	}

	sbuff = (sbuff_t *)calloc(1, sizeof(sbuff_t));
	if (NULL == sbuff) {
		fprintf(stderr, "osd_start_audio_stream: memory problem (%s)\n",
			strerror(errno));
		return -1;
	}

	audio_spec_desired = (SDL_AudioSpec *)calloc(1, sizeof(SDL_AudioSpec));
	if (NULL == audio_spec_desired) {
		fprintf(stderr, "osd_start_audio_stream: memory problem (%s)\n",
			strerror(errno));
		free(sbuff);
		return -1;
	}
	audio_spec_obtained = (SDL_AudioSpec *)calloc(1, sizeof(SDL_AudioSpec));
	if (NULL == audio_spec_obtained) {
		fprintf(stderr, "osd_start_audio_stream: memory problem (%s)\n",
			strerror(errno));
		free(sbuff);
		return -1;
	}

	audio_spec_desired->freq = sample_rate;
	audio_spec_desired->format = AUDIO_S16SYS;
	audio_spec_desired->channels = stereo ? 2 : 1;
	audio_spec_desired->samples = SBUFF_SIZE / 16;
	audio_spec_desired->callback = osd_flush_audio_stream;
	audio_spec_desired->userdata = sbuff;
	rc = SDL_OpenAudio(audio_spec_desired, audio_spec_obtained);

	if (rc) {
		/* fail */
		free(audio_spec_desired);
		audio_spec_desired = NULL;
		free(audio_spec_obtained);
		audio_spec_obtained = NULL;
		free(sbuff);
		sbuff = NULL;
		fprintf(stderr, "osd_start_audio_stream: SDL_OpenAudio failed (%s)\n",
			strerror(errno));
		return 0;
	}

	/* obtained sample rate */
	sample_rate = audio_spec_obtained->freq;
	samples = sample_rate / refresh_rate;
	sbuff->size = SBUFF_SIZE;
	sbuff->head = 0;
	sbuff->tail = SBUFF_SIZE / 8;

#if	0
	printf("--- rates\n");
	printf("sample_rate:   %d\n", sample_rate);
	printf("refresh_rate:  %g\n", refresh_rate);
	printf("--- audio_spec_obtained\n");
	printf("freq:          %d\n", audio_spec_obtained->freq);
	printf("format:        0x%x\n", audio_spec_obtained->format);
	printf("channels:      %d\n", audio_spec_obtained->channels);
	printf("samples:       %d\n", audio_spec_obtained->samples);
	printf("size:          %d\n", audio_spec_obtained->size);
	printf("callback:      %p\n", audio_spec_obtained->callback);
	printf("userdata:      %p\n", audio_spec_obtained->userdata);

	printf("--- sbuff\n");
	printf("size:          %d\n", sbuff->size);
	printf("head:          %d\n", sbuff->head);
	printf("tail:          %d\n", sbuff->tail);
	printf("--- samples per frame\n");
	printf("samples:       %d\n", samples);
#endif
	SDL_PauseAudio(0);

	return samples;
}

static int32_t frameskip_counter = 0;
static const int32_t skiptable[FRAMESKIP_LEVELS][FRAMESKIP_LEVELS] = {
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,1 },
	{ 0,0,0,0,0,1,0,0,0,0,0,1 },
	{ 0,0,0,1,0,0,0,1,0,0,0,1 },
	{ 0,0,1,0,0,1,0,0,1,0,0,1 },
	{ 0,1,0,0,1,0,1,0,0,1,0,1 },
	{ 0,1,0,1,0,1,0,1,0,1,0,1 },
	{ 0,1,0,1,1,0,1,0,1,1,0,1 },
	{ 0,1,1,0,1,1,0,1,1,0,1,1 },
	{ 0,1,1,1,0,1,1,1,0,1,1,1 },
	{ 0,1,1,1,1,1,0,1,1,1,1,1 },
	{ 0,1,1,1,1,1,1,1,1,1,1,1 }
};

static const int32_t waittable[FRAMESKIP_LEVELS][FRAMESKIP_LEVELS] = {
	{ 1,1,1,1,1,1,1,1,1,1,1,1 },
	{ 2,1,1,1,1,1,1,1,1,1,1,0 },
	{ 2,1,1,1,1,0,2,1,1,1,1,0 },
	{ 2,1,1,0,2,1,1,0,2,1,1,0 },
	{ 2,1,0,2,1,0,2,1,0,2,1,0 },
	{ 2,0,2,1,0,2,0,2,1,0,2,0 },
	{ 2,0,2,0,2,0,2,0,2,0,2,0 },
	{ 2,0,2,0,0,3,0,2,0,0,3,0 },
	{ 3,0,0,3,0,0,3,0,0,3,0,0 },
	{ 4,0,0,0,4,0,0,0,4,0,0,0 },
	{ 6,0,0,0,0,0,6,0,0,0,0,0 },
	{12,0,0,0,0,0,0,0,0,0,0,0 }
};

static uclock_t prev_frames[FRAMESKIP_LEVELS] = {0,0,0,0,0,0,0,0,0,0,0,0};
static uclock_t prev = 0;
static int32_t speed = 100;

int32_t should_sleep_idle(void)
{
	return 1;
}

int32_t osd_skip_next_frame(void)
{
	uclock_t curr;
	int32_t i;

	if (skiptable[frameskip][frameskip_counter]) {
		frameskip_counter = (frameskip_counter + 1) % FRAMESKIP_LEVELS;
		return skiptable[frameskip][frameskip_counter];
	}

	/* now wait until it's time to update the screen */
	if (throttle) {
		uclock_t target, target2;

		/* wait until enough time has passed since last frame... */
		target = prev + waittable[frameskip][frameskip_counter] *
			UCLOCKS_PER_SEC / refresh_rate;

		/* ... OR since FRAMESKIP_LEVELS frames ago. This way,
		 * if a frame takes longer than the allotted time,
		 * we can compensate in the following frames.
		 */
		target2 = prev_frames[frameskip_counter] +
			FRAMESKIP_LEVELS * UCLOCKS_PER_SEC / refresh_rate;

		if (target - target2 > 0)
			target = target2;
       		
		curr = uclock();
       		
		/*
		 * If we need to sleep more then half a second,
		 * we've somehow got totally out of sync. So
		 * if this happens we reset all counters
		 */
		if ((target - curr) > (UCLOCKS_PER_SEC / 2)) {
			for (i = 0; i < FRAMESKIP_LEVELS; i++)
				prev_frames[i] = curr;
		} else {
			while ((curr - target) < 0) {
				curr = uclock();
				if ((target - curr) > (UCLOCKS_PER_SEC/100) &&
					should_sleep_idle())
					usleep(100);
			}
		}

	} else {
		curr = uclock();
	}

	if (frameskip_counter == 0 &&
		0 != (curr - prev_frames[frameskip_counter])) {
			int32_t divdr;

			divdr = refresh_rate *
				(curr - prev_frames[frameskip_counter]) /
				(100 * FRAMESKIP_LEVELS);
			speed = (UCLOCKS_PER_SEC + divdr/2) / divdr;
	}

	prev = curr;
	for (i = 0;i < waittable[frameskip][frameskip_counter];i++)
		prev_frames[(frameskip_counter + FRAMESKIP_LEVELS - i) %
			FRAMESKIP_LEVELS] = curr;

	if (throttle && autoframeskip && frameskip_counter == 0) {
		static int32_t frameskipadjust;

		if (speed >= 100) {
			frameskipadjust++;
			if (frameskipadjust > 1) {
				frameskipadjust = 0;
				if (frameskip > 0)
					frameskip--;
			}
		} else {
			if (speed < 80) {
				frameskipadjust -= (90 - speed) / 5;
			} else {
				/*
				 * Don't push frameskip too far,
				 * if we are close to 100% speed
				 */
				if (frameskip < 8)
					frameskipadjust--;
			}

			while (frameskipadjust <= -2) {
				frameskipadjust += 2;
				if (frameskip < max_autoframeskip)
					frameskip++;
			}
		}
	}
	
	frameskip_counter = (frameskip_counter + 1) % FRAMESKIP_LEVELS;
	return skiptable[frameskip][frameskip_counter];
}

void osd_display_frequency(uint64_t frq)
{
	uint32_t mhz = (uint32_t)(frq / 1000000ull);
	uint32_t khz = (uint32_t)((frq / 1000ull) % 1000ull);
	if (NULL != ctrl_panel)
		osd_widget_text(ctrl_panel, WID_FREQUENCY, "%u.%03u MHz", mhz, khz);
}

void osd_help(int argc, char **argv)
{
	char *program, *slash;
	slash = strrchr(argv[0], '/');
	if (NULL == slash)
		slash = strrchr(argv[0], '\\');
	if (NULL == slash)
		program = argv[0];
	else
		program = slash + 1;
	printf("usage: %s [options]\n", program);
	printf("options can be one or more of:\n");
	printf("-h|--help      display this help\n");
	printf("-f|--fast      disable throttling to original speed\n");
	printf("-v|--video     record MNG video right from the start\n");
	printf("-s|--scale n   scale video display to n times 1:1\n");
}

int32_t osd_init(int (*resize)(int32_t,int32_t),
	void *cookie,
	void (*keydn)(void *cookie, osd_key_t *),
	void (*keyup)(void *cookie, osd_key_t *),
	int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			osd_help(argc, argv);
			return -1;
		}
		if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--fast")) {
			throttle = 0;
			continue;
		}
		if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--video")) {
			start_video = 1;
			continue;
		}
		if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--scale")) {
			if (i + 1 >= argc) {
				fprintf(stderr, "missing parameter for scale\n");
				continue;
			}
			i++;
			scale = strtoull(argv[i], NULL, 0);
			if (scale < 1)
				scale = 1;
			if (scale > 5)
				scale = 5;
			continue;
		}
	}

	resize_callback = resize;
	cookie_callback = cookie;
	keydn_callback = keydn;
	keyup_callback = keyup;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE);
	atexit(SDL_Quit);

	SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
	SDL_EventState(SDL_KEYUP, SDL_ENABLE);
	SDL_EventState(SDL_QUIT, SDL_ENABLE);
	SDL_EnableKeyRepeat(250,30);
	SDL_EnableUNICODE(1);

#if	0
	SDL_WM_GrabInput(SDL_GRAB_ON);
#endif
	return 0;
}

void osd_exit(void)
{
	osd_close_display();
	osd_mng_stop();
	SDL_Quit();
}
