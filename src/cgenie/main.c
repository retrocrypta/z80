/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * main.c	Colour Genie EG2000 emulation
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#include "z80.h"
#include "z80dasm.h"
#include "timer.h"
#include "cgenie/kbd.h"
#include "cgenie/cas.h"
#include "cgenie/fdc.h"
#include "wd179x.h"
#include "ay8910.h"
#include "mc6845.h"

#define	LOAD_DOSEXT	0

#define	FONT_W	8
#define	FONT_H	8

#define	SCREENW	50
#define	SCREENH	32

/** @brief filename of file containing the ROM image */
const char cgenie_rom[] = "cgenie.rom";

/** @brief base address of the ROM image */
#define	MAIN_ROM_BASE	0x0000

/** @brief size of the ROM image */
#define	MAIN_ROM_SIZE	0x4000

/** @brief filename of file containing the DOS ROM image */
const char cgdos_rom[] = "cgdos.rom";

/** @brief base of the DOS ROM image */
#define	DOS_ROM_BASE	0xc000

/** @brief size of the DOS ROM image */
#define	DOS_ROM_SIZE	0x2000

/** @brief filename of file containing the DOS extension ROM image */
const char dosext_rom[] = "dosext.rom";

/** @brief filename of file containing the character generator */
const char cgenie_chr[] = "cgenie.chr";

/** @brief base of the extension ROM image */
#define	EXT_ROM_BASE	0xe000

/** @brief size of the extension ROM image */
#define	EXT_ROM_SIZE	0x1000

/** @brief Video RAM dirty flags */
static uint8_t *video_ram_dirty;

/** @brief Colour RAM dirty flags */
static uint32_t *colour_ram_dirty;

/** @brief Font RAM dirty flags */
static uint32_t *font_ram_dirty;

/** @brief redraw entire screen */
static uint32_t dirty_all;

/** @brief video memory (16K at 0x4000) */
#define	VIDEO_RAM_BASE	0x4000
#define	VIDEO_RAM_SIZE	0x4000

/** @brief colour memory (1K at 0xf000) */
#define	COLOUR_RAM_BASE	0xf000
#define	COLOUR_RAM_SIZE	0x0400

/** @brief character genertor memory (1K at 0xf400) */
#define	FONT_RAM_BASE	0xf400
#define	FONT_RAM_SIZE	0x0400

/** @brief rendered font */
static osd_bitmap_t *font;

typedef enum {
	C_GRAY,
	C_CYAN,
	C_RED,
	C_WHITE,
	C_YELLOW,
	C_GREEN,
	C_ORANGE,
	C_LIGHTYELLOW,
	C_BLUE,
	C_LIGHTBLUE,
	C_PINK,
	C_PURPLE,
	C_LIGHTGRAY,
	C_LIGHTCYAN,
	C_MAGENTA,
	C_BRIGHTWHITE,
	C_BACKGROUND
}	cgenie_colour_t;

/** @brief text mode palette colors */
static uint32_t pal_txt[16+1];

/** @brief graphics mode palette colors */
static uint32_t pal_gfx[4];

/** @brief non-zero if in graphics mod */
static uint32_t gfx_mode;

/** @brief character generator base address (0x00 or 0x80) */
static uint32_t font_base[4];

/** @brief character generator raw data */
static uint8_t chargen[256 * 8];

/** @brief font glyph width */
static int32_t font_w = 8;

/** @brief font glyph height */
static int32_t font_h = 8;

/** @brief screen character columns */
static uint32_t screen_w = 40;

/** @brief screen character rows */
static uint32_t screen_h = 25;

/** @brief character pixel rows */
static uint32_t char_h = 8;

/** @brief horizontal sync position */
static uint32_t hsync = 4;

/** @brief vertical sync position */
static uint32_t vsync = 3;

/** @brief screen character columns (changed) */
static uint32_t screen_w_changed;

/** @brief screen character rows (changed) */
static uint32_t screen_h_changed;

/** @brief character pixel rows (changed) */
static uint32_t char_h_changed;

/** @brief horizontal sync position (changed) */
static uint32_t hsync_changed;

/** @brief vertical sync position (changed) */
static uint32_t vsync_changed;

/** @brief frame buffer width */
static int32_t frame_w = 8 * SCREENW;

/** @brief frame buffer height */
static int32_t frame_h = 8 * SCREENH;

/** @brief screen x offset */
static int32_t screen_x = 8 * 3;

/** @brief screen y offset */
static int32_t screen_y = 8 * 2;

#define	PORT_FF_CAS	(1<<0)
#define	PORT_FF_BGRD	(1<<2)
#define	PORT_FF_CHAR_HI	(1<<3)
#define	PORT_FF_CHAR_LO	(1<<4)
#define	PORT_FF_FGR	(1<<5)
#define	PORT_FF_BGRD_0	(1<<6)
#define	PORT_FF_BGRD_1	(1<<7)
#define	PORT_FF_BGRD_ALL (PORT_FF_BGRD|PORT_FF_BGRD_0|PORT_FF_BGRD_1)

/** @brief port 0xff value */
static uint8_t port_ff = 0xff;

/** @brief frame timer (50 Hz) */
static tmr_t *frame_timer;

/** @brief clock timer (40 Hz) */
static tmr_t *clock_timer;

/** @brief scanline timer (16525 Hz) */
static tmr_t *scan_timer;

/** @brief non zero if the emulation stops */
int stop;

/** @brief beam positions when an access conflict occured */
#define	CONFLICT_MAX	256
uint32_t conflict_pos[CONFLICT_MAX];

/** @brief number of conflicts this frame */
uint32_t conflict_cnt;

typedef enum {
	CPU_NONE,
	CPU_BC,
	CPU_DE,
	CPU_HL,
	CPU_AF,
	CPU_IX,
	CPU_IY,
	CPU_SP,
	CPU_PC,
	CPU_BC2,
	CPU_DE2,
	CPU_HL2,
	CPU_AF2,
	CPU_R,
	CPU_I
}	button_id_cpu_t;

/***************************************************************************
 * forward declarations
 ***************************************************************************/
static __inline void set_video_ram_dirty(uint32_t offset);
static __inline void res_video_ram_dirty(uint32_t offset);
static __inline int get_video_ram_dirty(uint32_t offset);
static __inline void set_colour_ram_dirty(uint32_t offset, uint32_t mask);
static __inline void res_colour_ram_dirty(uint32_t offset, uint32_t mask);
static __inline int get_colour_ram_dirty(uint32_t offset, uint32_t mask);
static __inline void set_font_ram_dirty(uint32_t offset, uint32_t mask);
static __inline void res_font_ram_dirty(uint32_t offset, uint32_t mask);
static __inline int get_font_ram_dirty(uint32_t offset, uint32_t mask);
static void video_bgd(uint8_t r, uint8_t g, uint8_t b);
static void video_font_base(uint32_t which, uint32_t base);
static void video_mode(uint32_t enable);
static uint8_t rd_rom(uint32_t offset);
static uint8_t rd_ram(uint32_t offset);
static uint8_t rd_kbd(uint32_t offset);
static uint8_t rd_col(uint32_t offset);
static uint8_t rd_chr(uint32_t offset);
static uint8_t rd_fdc(uint32_t offset);
static void wr_rom(uint32_t offset, uint8_t data);
static void wr_ram(uint32_t offset, uint8_t data);
static void wr_nop(uint32_t offset, uint8_t data);
static void wr_col(uint32_t offset, uint8_t data);
static void wr_chr(uint32_t offset, uint8_t data);
static void wr_vid(uint32_t offset, uint8_t data);
static void wr_fdc(uint32_t offset, uint8_t data);
static uint8_t rd_port(uint32_t offset);
static void wr_port(uint32_t offset, uint8_t data);
static void cgenie_cursor(uint32_t chip, mc6845_cursor_t *cursor);
static void cgenie_video_addr(uint32_t chip, uint32_t frame_base_old, uint32_t frame_base_new);
static void video_text(void);
static void video_graphics(void);
static int cgenie_resize(int32_t w, int32_t h);
static void cgenie_frame(uint32_t param);
static int cgenie_screen(void);
static int cgenie_memory(void);
static uint8_t cgenie_port_a_r(uint32_t offset);
static uint8_t cgenie_port_b_r(uint32_t offset);
static void cgenie_port_a_w(uint32_t offset, uint8_t data);
static void cgenie_port_b_w(uint32_t offset, uint8_t data);
static int cgenie_audio(void);
static void cgenie_clock(uint32_t param);
static void cgenie_scan(uint32_t param);

/** @brief memory read handlers */
uint8_t (*rd_mem[L1SIZE])(uint32_t offset) = {
	rd_rom,	rd_rom,	rd_rom,	rd_rom,	/* 0000-0fff */
	rd_rom,	rd_rom,	rd_rom,	rd_rom,	/* 1000-1fff */
	rd_rom,	rd_rom,	rd_rom,	rd_rom,	/* 2000-2fff */
	rd_rom,	rd_rom,	rd_rom,	rd_rom,	/* 3000-3fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* 4000-4fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* 5000-5fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* 6000-6fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* 7000-7fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* 8000-8fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* 9000-9fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* a000-afff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* b000-bfff */
	rd_rom,	rd_rom,	rd_rom,	rd_rom,	/* c000-cfff */
	rd_rom,	rd_rom,	rd_rom,	rd_rom,	/* d000-dfff */
	rd_rom,	rd_rom,	rd_rom,	rd_rom,	/* e000-efff */
	rd_col,	rd_chr,	rd_kbd,	rd_fdc	/* f000-ffff */
};

/** @brief memory write handlers */
void (*wr_mem[L1SIZE])(uint32_t offset, uint8_t data) = {
	wr_rom,	wr_rom,	wr_rom,	wr_rom,	/* 0000-0fff */
	wr_rom,	wr_rom,	wr_rom,	wr_rom,	/* 1000-1fff */
	wr_rom,	wr_rom,	wr_rom,	wr_rom,	/* 2000-2fff */
	wr_rom,	wr_rom,	wr_rom,	wr_rom,	/* 3000-3fff */
	wr_vid,	wr_vid,	wr_vid,	wr_vid,	/* 4000-4fff */
	wr_vid,	wr_vid,	wr_vid,	wr_vid,	/* 5000-5fff */
	wr_vid,	wr_vid,	wr_vid,	wr_vid,	/* 6000-6fff */
	wr_vid,	wr_vid,	wr_vid,	wr_vid,	/* 7000-7fff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* 8000-8fff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* 9000-9fff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* a000-afff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* b000-bfff */
	wr_rom,	wr_rom,	wr_rom,	wr_rom,	/* c000-cfff */
	wr_rom,	wr_rom,	wr_rom,	wr_rom,	/* d000-dfff */
	wr_rom,	wr_rom,	wr_rom,	wr_rom,	/* e000-efff */
	wr_col,	wr_chr,	wr_nop,	wr_fdc	/* f000-ffff */
};

/** @brief I/O space read handlers */
uint8_t (*rd_io[L1SIZE])(uint32_t offset) = {
	rd_port,rd_port,rd_port,rd_port,/* 0000-0fff */
	rd_port,rd_port,rd_port,rd_port,/* 1000-1fff */
	rd_port,rd_port,rd_port,rd_port,/* 2000-2fff */
	rd_port,rd_port,rd_port,rd_port,/* 3000-3fff */
	rd_port,rd_port,rd_port,rd_port,/* 4000-4fff */
	rd_port,rd_port,rd_port,rd_port,/* 5000-5fff */
	rd_port,rd_port,rd_port,rd_port,/* 6000-6fff */
	rd_port,rd_port,rd_port,rd_port,/* 7000-7fff */
	rd_port,rd_port,rd_port,rd_port,/* 8000-8fff */
	rd_port,rd_port,rd_port,rd_port,/* 9000-9fff */
	rd_port,rd_port,rd_port,rd_port,/* a000-afff */
	rd_port,rd_port,rd_port,rd_port,/* b000-bfff */
	rd_port,rd_port,rd_port,rd_port,/* c000-cfff */
	rd_port,rd_port,rd_port,rd_port,/* d000-dfff */
	rd_port,rd_port,rd_port,rd_port,/* e000-efff */
	rd_port,rd_port,rd_port,rd_port,/* f000-ffff */
};

/** @brief I/O space write handlers */
void (*wr_io[L1SIZE])(uint32_t offset, uint8_t data) = {
	wr_port,wr_port,wr_port,wr_port,/* 0000-0fff */
	wr_port,wr_port,wr_port,wr_port,/* 1000-1fff */
	wr_port,wr_port,wr_port,wr_port,/* 2000-2fff */
	wr_port,wr_port,wr_port,wr_port,/* 3000-3fff */
	wr_port,wr_port,wr_port,wr_port,/* 4000-4fff */
	wr_port,wr_port,wr_port,wr_port,/* 5000-5fff */
	wr_port,wr_port,wr_port,wr_port,/* 6000-6fff */
	wr_port,wr_port,wr_port,wr_port,/* 7000-7fff */
	wr_port,wr_port,wr_port,wr_port,/* 8000-8fff */
	wr_port,wr_port,wr_port,wr_port,/* 9000-9fff */
	wr_port,wr_port,wr_port,wr_port,/* a000-afff */
	wr_port,wr_port,wr_port,wr_port,/* b000-bfff */
	wr_port,wr_port,wr_port,wr_port,/* c000-cfff */
	wr_port,wr_port,wr_port,wr_port,/* d000-dfff */
	wr_port,wr_port,wr_port,wr_port,/* e000-efff */
	wr_port,wr_port,wr_port,wr_port,/* f000-ffff */
};

/** @brief MC6845 CRTC interface structure */
static ifc6845_t mc6845 = {
	M6845_TYPE_GENUINE,
	14380000,
	cgenie_cursor,
	cgenie_video_addr
};

/** @brief AY8910 audio chip interface structure */
static ifc_ay8910_t ay8910 = {
	1,			/* number of chips */
	2000000.0,		/* baseclock 1 MHz */
	{0xffff},		/* mixing level */
	{cgenie_port_a_r},	/* port A read handler */
	{cgenie_port_b_r},	/* port B read handler */
	{cgenie_port_a_w},	/* port A write handler */
	{cgenie_port_b_w}	/* port B write handler */
};

/** @brief reset the system */
void sys_reset(reset_t how)
{
	z80_cpu_t *cpu = &z80;
	switch (how) {
	case SYS_IRQ:
		z80_interrupt(cpu, 2);
		break;
	case SYS_NMI:
		z80_interrupt(cpu, 1);
		break;
	case SYS_RST:
		cgenie_fdc_stop();
		cgenie_cas_stop();
		z80_reset(cpu);
		ay8910_reset(0);
		cgenie_cas_init();
		cgenie_fdc_init();
		break;
	}
}

const char *sys_get_name(void)
{
	return "cgenie";
}

void *sys_get_frame_timer(void)
{
	return frame_timer;
}

void sys_set_full_refresh(void)
{
	dirty_all = (uint32_t)-1;
}

void sys_cpu_panel_init(void *bitmap)
{
	osd_bitmap_t *cpu_panel = (osd_bitmap_t *)bitmap;
	int32_t x;
	int32_t y;

	x = 2;
	y = 2;
	osd_widget_alloc(cpu_panel, x, y, 52, 12, 0xbf, 0xbf, 0xbf, BT_STATIC, CPU_BC,  "BC: 0000");
	x += 54;
	osd_widget_alloc(cpu_panel, x, y, 52, 12, 0xbf, 0xbf, 0xbf, BT_STATIC, CPU_DE,  "DE: 0000");
	x += 54;
	osd_widget_alloc(cpu_panel, x, y, 52, 12, 0xbf, 0xbf, 0xbf, BT_STATIC, CPU_HL,  "HL: 0000");
	x += 54;
	osd_widget_alloc(cpu_panel, x, y, 52, 12, 0xbf, 0xbf, 0xbf, BT_STATIC, CPU_AF,  "AF: 0000");
	x += 54;
	osd_widget_alloc(cpu_panel, x, y, 52, 12, 0xbf, 0xbf, 0xbf, BT_STATIC, CPU_IX,  "IX: 0000");
	x += 54;
	osd_widget_alloc(cpu_panel, x, y, 52, 12, 0xbf, 0xbf, 0xbf, BT_STATIC, CPU_IY,  "IY: 0000");
	x += 54;

	x = 2;
	y = 14;
	osd_widget_alloc(cpu_panel, x, y, 52, 12, 0xbf, 0xbf, 0xbf, BT_STATIC, CPU_BC2, "BC' 0000");
	x += 54;
	osd_widget_alloc(cpu_panel, x, y, 52, 12, 0xbf, 0xbf, 0xbf, BT_STATIC, CPU_DE2, "DE' 0000");
	x += 54;
	osd_widget_alloc(cpu_panel, x, y, 52, 12, 0xbf, 0xbf, 0xbf, BT_STATIC, CPU_HL2, "HL' 0000");
	x += 54;
	osd_widget_alloc(cpu_panel, x, y, 52, 12, 0xbf, 0xbf, 0xbf, BT_STATIC, CPU_AF2, "AF' 0000");
	x += 54;
	osd_widget_alloc(cpu_panel, x, y, 52, 12, 0xbf, 0xbf, 0xbf, BT_STATIC, CPU_SP,  "SP: 0000");
	x += 54;
	osd_widget_alloc(cpu_panel, x, y, 52, 12, 0xbf, 0xbf, 0xbf, BT_STATIC, CPU_PC,  "PC: 0000");
	x += 54;
}

void sys_cpu_panel_update(void *bitmap)
{
	osd_bitmap_t *cpu_panel = (osd_bitmap_t *)bitmap;
	osd_widget_text(cpu_panel, CPU_BC,  "BC: %04x", z80_get_reg(&z80, Z80_BC));
	osd_widget_text(cpu_panel, CPU_DE,  "DE: %04x", z80_get_reg(&z80, Z80_DE));
	osd_widget_text(cpu_panel, CPU_HL,  "HL: %04x", z80_get_reg(&z80, Z80_HL));
	osd_widget_text(cpu_panel, CPU_AF,  "AF: %04x", z80_get_reg(&z80, Z80_AF));
	osd_widget_text(cpu_panel, CPU_IX,  "IX: %04x", z80_get_reg(&z80, Z80_IX));
	osd_widget_text(cpu_panel, CPU_IY,  "IY: %04x", z80_get_reg(&z80, Z80_IY));
	osd_widget_text(cpu_panel, CPU_SP,  "SP: %04x", z80_get_reg(&z80, Z80_SP));
	osd_widget_text(cpu_panel, CPU_PC,  "PC: %04x", z80_get_reg(&z80, Z80_PC));
	osd_widget_text(cpu_panel, CPU_BC2, "BC' %04x", z80_get_reg(&z80, Z80_BC2));
	osd_widget_text(cpu_panel, CPU_DE2, "DE' %04x", z80_get_reg(&z80, Z80_DE2));
	osd_widget_text(cpu_panel, CPU_HL2, "HL' %04x", z80_get_reg(&z80, Z80_HL2));
	osd_widget_text(cpu_panel, CPU_AF2, "AF' %04x", z80_get_reg(&z80, Z80_AF2));
}

/** @brief mark a video RAM location dirty */
static __inline void set_video_ram_dirty(uint32_t offset)
{
	uint32_t o = offset % VIDEO_RAM_SIZE;
	video_ram_dirty[o] = 1;
}

/** @brief mark a video RAM location clean */
static __inline void res_video_ram_dirty(uint32_t offset)
{
	uint32_t o = offset % VIDEO_RAM_SIZE;
	video_ram_dirty[o] = 0;
}

/** @brief get a video RAM location dirty flag */
static __inline int get_video_ram_dirty(uint32_t offset)
{
	uint32_t o = offset % VIDEO_RAM_SIZE;
	return (dirty_all || video_ram_dirty[o]);
}

/** @brief mark a colour RAM location dirty */
static __inline void set_colour_ram_dirty(uint32_t offset, uint32_t mask)
{
	uint32_t o = offset % COLOUR_RAM_SIZE;
	uint32_t b = o % 32;
	colour_ram_dirty[o / 32] |= mask << b;
}

/** @brief mark a colour RAM location clean */
static __inline void res_colour_ram_dirty(uint32_t offset, uint32_t mask)
{
	uint32_t o = offset % COLOUR_RAM_SIZE;
	uint32_t b = o % 32;
	colour_ram_dirty[o / 32] &= ~(mask << b);
}

/** @brief get a colour RAM location dirty flag */
static __inline int get_colour_ram_dirty(uint32_t offset, uint32_t mask)
{
	uint32_t o = offset % COLOUR_RAM_SIZE;
	uint32_t b = o % 32;
	return ((dirty_all | colour_ram_dirty[o / 32]) & (mask << b)) ? 1 : 0;
}

/** @brief mark a font RAM location dirty */
static __inline void set_font_ram_dirty(uint32_t offset, uint32_t mask)
{
	uint32_t o = offset % 128;
	uint32_t b = o % 32;
	font_ram_dirty[o / 32] |= mask << b;
}

/** @brief mark a font RAM location clean */
static __inline void res_font_ram_dirty(uint32_t offset, uint32_t mask)
{
	uint32_t o = offset % 128;
	uint32_t b = o % 32;
	font_ram_dirty[o / 32] &= ~(mask << b);
}

/** @brief get a font RAM location dirty flag */
static __inline int get_font_ram_dirty(uint32_t offset, uint32_t mask)
{
	uint32_t o = offset % 128;
	uint32_t b = o % 32;
	return ((dirty_all | font_ram_dirty[o / 32]) & (mask << b)) ? 1 : 0;
}

/** @brief simulate a bus conflict when accessing the character generator RAM */
static void video_conflict(void)
{
	uint32_t chars = screen_w * screen_h * char_h * 8;
	uint32_t pos = (uint32_t)(tmr_elapsed(frame_timer) * chars / frame_timer->restart);
	if (conflict_cnt < CONFLICT_MAX) {
		conflict_pos[conflict_cnt] = pos;
		conflict_cnt++;
	}
}

/** @brief set the video background colour */
static void video_bgd(uint8_t r, uint8_t g, uint8_t b)
{
	pal_gfx[0] = pal_txt[C_BACKGROUND] = osd_rgb(r, g, b);
	/* mark everything dirty */
	dirty_all = (uint32_t)-1;
}

/** @brief change a font base (character code ranges 2:0x80-0xbf 3:0xc0-0xff) */
static void video_font_base(uint32_t which, uint32_t base)
{
	uint32_t i, offs, size;
	if (base == font_base[which])
		return;
	font_base[which] = base;

	/* mark the changed characters dirty */
	offs = mc6845_get_start(0);
	size = mc6845_get_char_lines(0) * mc6845_get_char_columns(0);

	/* calculate the character code range that changed */
	which *= 64;
	for (i = 0; i < size; i++, offs = (offs + 1) % VIDEO_RAM_SIZE) {
		if (which == (mem[VIDEO_RAM_BASE + offs] & 0xc0))
			set_video_ram_dirty(offs);
	}
}

/** @brief change the video mode (enable 1:graphics 0:text) */
static void video_mode(uint32_t enable)
{
	if (enable == gfx_mode)
		return;
	gfx_mode = enable;
	/* mark everything dirty */
	dirty_all = (uint32_t)-1;
}

/** @brief read from ROM address */
static uint8_t rd_rom(uint32_t offset)
{
	return mem[offset];
}

/** @brief read from RAM address */
static uint8_t rd_ram(uint32_t offset)
{
	return mem[offset];
}

/** @brief read from keyboard (memory mapped 8x8 matrix) */
static uint8_t rd_kbd(uint32_t offset)
{
	uint8_t data = 0xff;
	uint8_t *keymap = cgenie_kbd_map();
	if (offset & 0x001)
		data &= keymap[0];
	if (offset & 0x002)
		data &= keymap[1];
	if (offset & 0x004)
		data &= keymap[2];
	if (offset & 0x008)
		data &= keymap[3];
	if (offset & 0x010)
		data &= keymap[4];
	if (offset & 0x020)
		data &= keymap[5];
	if (offset & 0x040)
		data &= keymap[6];
	if (offset & 0x080)
		data &= keymap[7];
	return data;
}

/** @brief read colour RAM (bits 7-4 are floating) */
static uint8_t rd_col(uint32_t offset)
{
	uint8_t data = mem[offset];
	data |= z80.mp.byte.b1 & 0xf0;
	return data;
}

/** @brief read character generator RAM  */
static uint8_t rd_chr(uint32_t offset)
{
	uint8_t data = mem[offset];
	video_conflict();
	return data;
}

/** @brief read from memory mapped I/O registers */
static uint8_t rd_fdc(uint32_t offset)
{
	uint8_t data = z80.mp.byte.b0;

	switch (offset) {
	case 0xffe0:
	case 0xffe1:
	case 0xffe2:
	case 0xffe3:
		data = cgenie_irq_status_r(offset);
		break;
	case 0xffec:
	case 0xffed:
	case 0xffee:
	case 0xffef:
		data = wd179x_0_r(offset);
		break;
	}
	return data;
}

/** @brief write to ROM memory address */
static void wr_rom(uint32_t offset, uint8_t data)
{
	/* no op */
}

/** @brief write to RAM memory address */
static void wr_ram(uint32_t offset, uint8_t data)
{
	mem[offset] = data;
}


/** @brief write to nowhere (memory mapped keyboard) */
static void wr_nop(uint32_t offset, uint8_t data)
{
	/* no op */
}

/** @brief write to colour RAM */
static void wr_col(uint32_t offset, uint8_t data)
{
	data %= 16;		/* only the lower 4 bits are used */
	if (data == mem[offset])
		return;
	mem[offset] = data;
	set_colour_ram_dirty(offset, 1);
}

/** @brief write to character generator RAM */
static void wr_chr(uint32_t offset, uint8_t data)
{
	video_conflict();
	if (data == mem[offset])
		return;
	mem[offset] = data;
	set_font_ram_dirty(offset/8,1);
}

/** @brief write to video RAM address */
static void wr_vid(uint32_t offset, uint8_t data)
{
	if (data == mem[offset])
		return;
	mem[offset] = data;
	set_video_ram_dirty(offset);
}

/** @brief write to memory mapped I/O range (floppy disc motors and controller) */
static void wr_fdc(uint32_t offset, uint8_t data)
{
	switch (offset) {
	case 0xffe0:
	case 0xffe1:
	case 0xffe2:
	case 0xffe3:
		cgenie_motors_w(offset, data);
		break;
	case 0xffec:
	case 0xffed:
	case 0xffee:
	case 0xffef:
		wd179x_0_w(offset, data);
		break;
	}
}

/** @brief read from the I/O range */
static uint8_t rd_port(uint32_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0xff) {
	case 0xf8:
		break;
	case 0xf9:
		/* AY8910 */
		ay8910_read_port_0_r(offset);
		break;
	case 0xfa:
	case 0xfb:
		/* MC6845 */
		data = mc6845_0_r(offset);
		break;
	case 0xff:
		data = (port_ff & ~PORT_FF_CAS) | cgenie_cas_r(offset);
		break;
	}
	return data;
}

/** @brief write to the I/O range */
static void wr_port(uint32_t offset, uint8_t data)
{
	uint8_t changed;

	switch (offset & 0xff) {
	case 0xf8:
		/* AY8910 control */
		ay8910_control_port_0_w(offset, data);
		break;
	case 0xf9:
		/* AY8910 data */
		ay8910_write_port_0_w(offset, data);
		break;
	case 0xfa:
	case 0xfb:
		/* MC6845 CRTC */
		mc6845_0_w(offset, data);
		break;
	case 0xff:
		changed = port_ff ^ data;

		if (changed & PORT_FF_CAS) {
			/* cassette output signal change */
			cgenie_cas_w(0, data);
		}

		/* background color enable/select change */
		if (changed & PORT_FF_BGRD_ALL) {
			uint8_t r, g, b;
			if (data & PORT_FF_BGRD) {
				/* BGD0 */
				r = 112;
				g = 0;
				b = 112;
			} else {
				switch (data & (PORT_FF_BGRD_0 | PORT_FF_BGRD_1)) {
				case PORT_FF_BGRD_0:
					r = 112;
					g = 40;
					b = 32;
					break;
				case PORT_FF_BGRD_1:
					r = 40;
					g = 112;
					b = 32;
					break;
				case PORT_FF_BGRD_0 | PORT_FF_BGRD_1:
					r = 72;
					g = 72;
					b = 72;
					break;
				default:
					r = 0;
					g = 0;
					b = 0;
					break;
				}
			}
			video_bgd(r, g, b); /* background */
		}
		/* text/graphics mode change */
		if (changed & PORT_FF_FGR) {
			video_mode(data & PORT_FF_FGR ? 1 : 0);
		}

		/* character generator lo (0x80-0xbf) change */
		if (changed & PORT_FF_CHAR_LO) {
			video_font_base(2, (data & PORT_FF_CHAR_LO) ? 0x00 : 0x80);
		}

		/* character generator hi (0xc0-0xff) change */
		if (changed & PORT_FF_CHAR_HI) {
			video_font_base(3, (data & PORT_FF_CHAR_HI) ? 0x00 : 0x80);
		}

		port_ff = data;
		break;
	}
}

static void cgenie_cursor(uint32_t chip, mc6845_cursor_t *cursor)
{
	(void)chip;
	if (cursor->pos < VIDEO_RAM_SIZE)
		set_video_ram_dirty(cursor->pos);
}

static void cgenie_video_addr(uint32_t chip, uint32_t frame_base_old, uint32_t frame_base_new)
{
	(void)chip;
	uint32_t i, size;

	size = mc6845_get_char_lines(0) * mc6845_get_char_columns(0);
	/* if the video base offset changed, mark dirty changes */
	if (frame_base_new != frame_base_old) {
		for (i = 0; i < size; i++) {
			uint32_t o0 = (frame_base_old + i) % VIDEO_RAM_SIZE;
			uint32_t o1 = (frame_base_new + i) % VIDEO_RAM_SIZE;
			if (mem[VIDEO_RAM_BASE + o0] == mem[VIDEO_RAM_BASE + o1])
				continue;
			set_video_ram_dirty(o1);
		}
	}
}

static void video_text(void)
{
	uint32_t offs = mc6845_get_start(0);
	uint32_t x, y, x0, y0, ct, ch, px;
	uint32_t data;
	uint32_t attr;
	uint32_t pal2[2];
	mc6845_cursor_t cursor;

	mc6845_get_cursor(0, &cursor);
	if (cursor.top >= char_h)
		cursor.on = 0;
	if (cursor.bottom >= char_h)
		cursor.bottom = char_h - 1;

	pal2[0] = pal_txt[C_BACKGROUND];
	for (y = 0; y < screen_h; y++) {
		y0 = screen_y + y * font_h;
		for (x = 0; x < screen_w; x++, offs = (offs + 1) % VIDEO_RAM_SIZE) {
			if (0 == get_video_ram_dirty(offs) &&
				0 == get_colour_ram_dirty(offs,1))
				continue;
			x0 = screen_x + x * font_w;
			res_video_ram_dirty(offs);
			data = mem[VIDEO_RAM_BASE + offs];
			data = data + font_base[data / 64];
			attr = mem[COLOUR_RAM_BASE + offs % COLOUR_RAM_SIZE] % 16;
			pal2[1] = pal_txt[attr];
			osd_pattern(frame, font, x0, y0, data, 2, pal2, font_w, font_h);
			if (0 == cursor.on)
				continue;
			if (offs != cursor.pos)
				continue;
			set_video_ram_dirty(offs);
			ct = cursor.top * font_h / char_h;
			ch = (cursor.bottom + 1 - cursor.top) * font_h / char_h;
			px = osd_color(frame, osd_get_r(pal2[1]), osd_get_g(pal2[1]), osd_get_b(pal2[1]));
			osd_fillrect(frame, x0, y0 + ct, font_w, ch, px);
		}
	}
}

static void video_graphics(void)
{
	uint32_t offs = mc6845_get_start(0);
	uint32_t x, y, x0, y0;
	uint32_t w = font_w / 4;
	uint32_t h = char_h * font_h / FONT_H;
	uint32_t pal[4];
	uint32_t data, i;

	for (i = 0; i < 4; i++) {
		data = pal_gfx[i];
		pal[i] = osd_color(frame, osd_get_r(data), osd_get_g(data), osd_get_b(data));
	}

	for (y = 0; y < screen_h * h; y += h) {
		y0 = screen_y + y;
		for (x = 0; x < screen_w; x++, offs = (offs + 1) % VIDEO_RAM_SIZE) {
			if (0 == get_video_ram_dirty(offs))
				continue;
			res_video_ram_dirty(offs);
			x0 = screen_x + x * font_w;
			data = mem[VIDEO_RAM_BASE + offs];
			osd_fillrect(frame, x0 + 0*w, y0, w, h, pal[(data >> 6) & 3]);
			osd_fillrect(frame, x0 + 1*w, y0, w, h, pal[(data >> 4) & 3]);
			osd_fillrect(frame, x0 + 2*w, y0, w, h, pal[(data >> 2) & 3]);
			osd_fillrect(frame, x0 + 3*w, y0, w, h, pal[(data >> 0) & 3]);
		}
	}
}

static int cgenie_resize(int32_t w, int32_t h)
{
	int32_t hs = hsync == 71 ? -14 : hsync - 14;
	int32_t vs = vsync - 5;
	int32_t fw;
	int32_t fh;
	uint32_t bg;

	bg = osd_color(frame,
		osd_get_r(pal_txt[C_BACKGROUND]),
		osd_get_g(pal_txt[C_BACKGROUND]),
		osd_get_b(pal_txt[C_BACKGROUND]));
	osd_fillrect(frame, screen_x, screen_y,
		screen_w * font_w,
		screen_h * char_h * font_h / FONT_H, bg);
	frame_w = w;
	frame_h = h;
	fw = w / SCREENW;
	fh = h / SCREENH;
	screen_x = hs * fw;
	screen_y = vs * char_h * fh / FONT_H;

	if (fw != font_w || fh != font_h) {
		font_w = fw;
		font_h = fh;

		osd_bitmap_free(&font);
		osd_bitmap_alloc(&font, font_w * 16, font_h * 24, 8);
		osd_set_colors(font, pal_txt, 17);
		osd_render_font(font,
			chargen,	/* bitmap data */
			0,		/* first code */
			256 + 128,	/* count */
			FONT_W,		/* font width */
			FONT_H,		/* font height */
			font_w,		/* scaled font width */
			font_h,		/* scaled font height */
			1,		/* line skew */
			1,		/* bits per pixel */
			NULL);		/* palette */
	}
	osd_set_colors(frame, pal_txt, 17);
	osd_set_colors(NULL, pal_txt, 17);

	dirty_all = (uint32_t)-1;
	return 0;
}

#define CHANGE(n) do { \
	if (0 == changecnt) \
		changecnt = n; \
} while (0)

static void cgenie_frame(uint32_t param)
{
	static uint32_t changecnt;
	uint32_t ch, i, n, size, frame_base;

	ay8910_update_stream();

	osd_display_frequency((uint64_t)50.0 * cycles_this_frame);
	cycles_this_frame = 0;

	if (screen_h_changed != mc6845_get_char_lines(0)) {
		screen_h_changed = mc6845_get_char_lines(0);
		if (0 == screen_h_changed)
			screen_h_changed = 1;
		CHANGE(1);
	}
	if (screen_w_changed != mc6845_get_char_columns(0)) {
		screen_w_changed = mc6845_get_char_columns(0);
		if (0 == screen_w_changed)
			screen_w_changed = 1;
		CHANGE(1);
	}
	if (char_h_changed != mc6845_get_char_height(0)) {
		char_h_changed = mc6845_get_char_height(0);
		if (0 == char_h_changed)
			char_h_changed = 1;
		CHANGE(1);
	}
	if (hsync_changed != mc6845_get_horz_sync(0)) {
		hsync_changed = mc6845_get_horz_sync(0);
		if (0 == hsync_changed)
			hsync_changed = 1;
		CHANGE(1);
	}
	if (vsync_changed != mc6845_get_vert_sync(0)) {
		vsync_changed = mc6845_get_vert_sync(0);
		if (0 == vsync_changed)
			vsync_changed = 1;
		CHANGE(1);
	}
	if (changecnt > 0) {
		if (--changecnt == 0) {
#if	0
			printf("w:%d (%d)  h:%d (%d)  ch:%d (%d)  hs:%d (%d)  vs:%d (%d)\n",
				screen_w_changed, screen_w,
				screen_h_changed, screen_h,
				char_h_changed, char_h,
				hsync_changed, hsync,
				vsync_changed, vsync);
#endif
			screen_h = screen_h_changed;
			screen_w = screen_w_changed;
			char_h = char_h_changed;
			hsync = hsync_changed;
			vsync = vsync_changed;
			cgenie_resize(frame_w, frame_h);
		}
	}

	frame_base = mc6845_get_start(0);
	size = mc6845_get_char_lines(0) * mc6845_get_char_columns(0);

	/* render modified character generator codes */
	for (ch = 0; ch < 128; ch++) {
		if (0 == get_font_ram_dirty(ch, 1))
			continue;
		res_font_ram_dirty(ch, 1);
		osd_render_font(font,
			&mem[FONT_RAM_BASE + FONT_H * ch],
			256 + ch,	/* first code */
			1,		/* count */
			FONT_W,		/* font width */
			FONT_H,		/* font height */
			font_w,		/* scaled font width */
			font_h,		/* scaled font height */
			1,		/* skew */
			1,		/* bits per pixel */
			NULL);		/* palette */
		for (i = 0; i < size; i++) {
			uint32_t o1 = (frame_base + i) % VIDEO_RAM_SIZE;
			if (128 + ch != mem[VIDEO_RAM_BASE + o1])
				continue;
			set_video_ram_dirty(o1);
		}
	}

	if (dirty_all) {
		uint32_t bg = osd_color(frame,
			osd_get_r(pal_gfx[0]),
			osd_get_g(pal_gfx[0]),
			osd_get_b(pal_gfx[0]));
		osd_fillrect(frame, 0, 0, frame_w, frame_h, bg);
	}

	switch (gfx_mode) {
	case 0:
		video_text();
		break;
	case 1:
		video_graphics();
		break;
	}
	/* clear all the color dirty flags */
	memset(colour_ram_dirty, 0, COLOUR_RAM_SIZE/8);
	dirty_all = 0;

	/* add flicker caused by accessing the character generator RAM */
	if (screen_w > 0 && char_h > 0 && conflict_cnt > 0) {
		uint32_t white = osd_color(frame, 255, 255, 255);
		for (n = 0; n < conflict_cnt; n++) {
			uint32_t pos = conflict_pos[n];
			uint32_t x = (pos / 8) % screen_w;
			uint32_t y = (pos / 8) / screen_w / char_h;
			uint32_t px = pos % 8;
			uint32_t py = (pos / 8 / screen_w) % char_h;
			uint32_t addr = mc6845_get_start(0) + y * screen_w + x;
			uint32_t x0 = screen_x + x * font_w + font_w * px / 8;
			uint32_t y0 = screen_y + y * font_h + font_h * py / char_h;
			uint32_t w = font_w / 8;
			uint32_t h = font_h / char_h;
			if (y >= screen_h)
				continue;
			osd_fillrect(NULL, x0, y0, w, h, white);
			osd_fillrect(frame, x0, y0, w, h, white);
			set_video_ram_dirty(addr);
		}
	}
	conflict_cnt = 0;

	stop = osd_update(osd_skip_next_frame());
}

static int cgenie_screen(void)
{
	char filename[FILENAME_MAX];
	FILE *fp;
	int rc;

	snprintf(filename, sizeof(filename), "%s/%s",
		sys_get_name(), cgenie_chr);
	fp = fopen(filename, "rb");
	if (NULL == fp) {
		fprintf(stderr, "fopen(\"%s\",\"%s\") failed\n",
			filename, "rb");
		return -1;
	}
	fread(chargen, 1, sizeof(chargen), fp);
	fclose(fp);

	video_ram_dirty = malloc(VIDEO_RAM_SIZE);
	if (NULL == video_ram_dirty)
		return -1;

	colour_ram_dirty = malloc(COLOUR_RAM_SIZE / 8);
	if (NULL == colour_ram_dirty)
		return -1;

	font_ram_dirty = malloc(FONT_RAM_SIZE / 8);
	if (NULL == font_ram_dirty)
		return -1;

	font = calloc(1, sizeof(osd_bitmap_t));
	if (NULL == font)
		return -1;

	mc6845_init(1, &mc6845);
	frame_w = SCREENW * font_w;
	frame_h = SCREENH * font_h;
	osd_set_display(frame_w, frame_h);
	frame_w *= osd_get_scale();
	frame_h *= osd_get_scale();
	rc = osd_open_display(frame_w, frame_h, "Colour Genie EG2000");
	if (0 != rc) {
		fprintf(stderr, "osd_open_display(%d,%d,...) failed\n",
			frame_w, frame_h);
		return -1;
	}
	font_w = FONT_W * osd_get_scale();
	font_h = FONT_H * osd_get_scale();
	osd_bitmap_alloc(&font, font_w * 16, font_h * 24, 8);

	pal_txt[C_GRAY       ] = osd_rgb(15*4,15*4,15*4);
	pal_txt[C_CYAN       ] = osd_rgb(0*4,48*4,48*4);
	pal_txt[C_RED        ] = osd_rgb(60*4, 0*4, 0*4);
	pal_txt[C_WHITE      ] = osd_rgb(47*4,47*4,47*4);
	pal_txt[C_YELLOW     ] = osd_rgb(55*4,55*4, 0*4);
	pal_txt[C_GREEN      ] = osd_rgb(0*4,56*4, 0*4);
	pal_txt[C_ORANGE     ] = osd_rgb(42*4,32*4, 0*4);
	pal_txt[C_LIGHTYELLOW] = osd_rgb(63*4,63*4, 0*4);
	pal_txt[C_BLUE       ] = osd_rgb( 0*4, 0*4,48*4);
	pal_txt[C_LIGHTBLUE  ] = osd_rgb( 0*4,24*4,63*4);
	pal_txt[C_PINK       ] = osd_rgb(60*4, 0*4,38*4);
	pal_txt[C_PURPLE     ] = osd_rgb(38*4, 0*4,60*4);
	pal_txt[C_LIGHTGRAY  ] = osd_rgb(31*4,31*4,31*4);
	pal_txt[C_LIGHTCYAN  ] = osd_rgb( 0*4,63*4,63*4);
	pal_txt[C_MAGENTA    ] = osd_rgb(58*4, 0*4,58*4);
	pal_txt[C_BRIGHTWHITE] = osd_rgb(63*4,63*4,63*4);
	pal_txt[C_BACKGROUND ] = osd_rgb(   0,   0,   0);
	osd_set_colors(font, pal_txt, 17);
	osd_set_colors(frame, pal_txt, 17);
	osd_set_colors(NULL, pal_txt, 17);

	pal_gfx[0] = pal_txt[C_BACKGROUND ];
	pal_gfx[1] = pal_txt[C_BLUE       ];
	pal_gfx[2] = pal_txt[C_ORANGE     ];
	pal_gfx[3] = pal_txt[C_GREEN      ];

	font_base[0] = 0x00;
	font_base[1] = 0x00;
	font_base[2] = 0x00;
	font_base[3] = 0x00;

	osd_render_font(font,
		chargen,	/* bitmap data */
		0,		/* first code */
		256,		/* count */
		FONT_W,		/* font width */
		FONT_H,		/* font height */
		font_w,		/* scaled font width */
		font_h,		/* scaled font height */
		1,		/* line skew */
		1,		/* bits per pixel */
		NULL);		/* palette */

	return 0;
}

static int cgenie_memory(void)
{
	char filename[FILENAME_MAX];
	FILE *fp;

	snprintf(filename, sizeof(filename), "%s/%s",
		sys_get_name(), cgenie_rom);
	fp = fopen(filename, "rb");
	if (NULL == fp) {
		printf("%s not found\n", filename);
		return -1;
	}
	if (MAIN_ROM_SIZE != fread(&mem[MAIN_ROM_BASE], 1, MAIN_ROM_SIZE, fp)) {
		printf("%s too small (expected 0x%x)\n", filename, MAIN_ROM_SIZE);
		return -1;
	}
	fclose(fp);

	snprintf(filename, sizeof(filename), "%s/%s",
		sys_get_name(), cgdos_rom);
	fp = fopen(filename, "rb");
	if (NULL == fp) {
		printf("%s not found\n", filename);
		return -1;
	}
	if (DOS_ROM_SIZE != fread(&mem[DOS_ROM_BASE], 1, DOS_ROM_SIZE, fp)) {
		printf("%s too small (expected 0x%x)\n", filename, DOS_ROM_SIZE);
		return -1;
	}
	fclose(fp);

#if	LOAD_DOSEXT
	snprintf(filename, sizeof(filename), "%s/%s",
		sys_get_name(), dosext_rom);
	fp = fopen(filename, "rb");
	if (NULL == fp) {
		printf("%s not found\n", filename);
		return -1;
	}
	if (EXT_ROM_SIZE != fread(&mem[EXT_ROM_BASE], 1, EXT_ROM_SIZE, fp)) {
		printf("%s too small (expected 0x%x)\n", filename, EXT_ROM_SIZE);
		return -1;
	}
	fclose(fp);
#endif

	return 0;
}

/** @brief read from AY8910 port A */
static uint8_t cgenie_port_a_r(uint32_t offset)
{
	return 0xff;
}

/** @brief read from AY8910 port B */
static uint8_t cgenie_port_b_r(uint32_t offset)
{
	return 0xff;
}

/** @brief write to AY8910 port A */
static void cgenie_port_a_w(uint32_t offset, uint8_t data)
{
	(void)offset;
	(void)data;
}

/** @brief write to AY8910 port B */
static void cgenie_port_b_w(uint32_t offset, uint8_t data)
{
	(void)offset;
	(void)data;
}

static int cgenie_audio(void)
{
	osd_set_refresh_rate(50.0);
	return ay8910_sh_start(&ay8910);
}

static void cgenie_clock(uint32_t param)
{
	cgenie_timer_interrupt();
}

static void cgenie_scan(uint32_t param)
{
	/* steal cycles for the video DMA (screen width RAM accesses) */
	z80_dma += screen_w;
}

int main(int argc, char **argv)
{
	z80_cpu_t *cpu = &z80;
	int dumpmem;
	int i;

	for (i = 1, dumpmem = 0; i < argc; i++)
		if (!strcmp(argv[i], "-d"))
			dumpmem = 1;

	if (osd_init(cgenie_resize, NULL, cgenie_key_dn, cgenie_key_up, argc, argv))
		return 1;

	if (cgenie_screen() < 0) {
		printf("Screen init: failed\n");
		return 1;
	}
	if (cgenie_audio() < 0) {
		printf("Audio init: failed\n");
		return 2;
	}
	if (cgenie_memory() < 0) {
		printf("ROM loading: failed\n");
		return 3;
	}

	z80_reset(cpu);
	cgenie_cas_init();
	cgenie_fdc_init();
	frame_timer = tmr_alloc(cgenie_frame, tmr_double_to_time(TIME_IN_MSEC(20)),
		0, tmr_double_to_time(TIME_IN_MSEC(20)));
	clock_timer = tmr_alloc(cgenie_clock, tmr_double_to_time(TIME_IN_MSEC(25)),
		0, tmr_double_to_time(TIME_IN_MSEC(25)));
	scan_timer = tmr_alloc(cgenie_scan, tmr_double_to_time(TIME_IN_HZ(16525)),
		0, tmr_double_to_time(TIME_IN_HZ(16525)));

	while (!stop)
		tmr_run_cpu(cpu, 2216800.0);

	if (dumpmem) {
		FILE *fp = fopen("cgenie.mem", "wb");
		fwrite(mem, 1, MEMSIZE, fp);
		fclose(fp);
	}

	cgenie_fdc_stop();
	cgenie_cas_stop();
	osd_exit();
	return 0;
}
