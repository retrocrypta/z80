/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * trs80.c	Tandy TRS-80 emulation
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 * Copyright 2015 Andreas Müller <schnitzeltony@googlemail.com>
 *
 ***************************************************************************************/
#include "z80.h"
#include "z80dasm.h"
#include "timer.h"
#include "trs80/kbd.h"
#include "trs80/cas.h"
#include "trs80/fdc.h"
#include "wd179x.h"
#include "osd.h"
#include "osd_bitmap.h"
#include "osd_font.h"

#define	FONT_W	6
#define	FONT_H	15

#define	SCREENW	64
#define	SCREENH	16

/** @brief filename of file containing the ROM image */
static const char trs80_rom[] = "trs80.rom";

/** @brief filename of file containing the character generator */
static const char trs80_chr[] = "trs80.chr";

/** @brief size of the ROM image */
#define	MAIN_ROM_SIZE	0x3000

/** @brief base address of the video RAM */
#define	VIDEO_RAM_BASE	0x3c00

/** @brief size of the video RAM */
#define	VIDEO_RAM_SIZE	0x0400

/** @brief character generator */
static uint8_t chargen[256 * FONT_H];

/** @brief rendered font */
static osd_font_t *font;

/** @brief left offset of first pixel */
static int32_t screen_x = 0;

/** @brief top offset of first pixel */
static int32_t screen_y = 0;

/** @brief TRS-80 port FF */
static uint8_t port_ff;

/** @brief video dirty flags */
static uint32_t video_ram_dirty[VIDEO_RAM_SIZE/32];

/** @brief marks all video locations dirty */
static uint32_t dirty_all;

/** @brief audio stream buffer */
static int16_t *audio_stream;

/** @brief audio samples per frame */
static int32_t audio_samples;

/** @brief current position in audio stream buffer */
static int32_t audio_pos;

/** @brief current value to fill in audio stream buffer */
static int16_t audio_value;

/** @brief frame timer (50 Hz) */
static tmr_t *frame_timer;

/** @brief clock timer (40 Hz) */
static tmr_t *clock_timer;

typedef enum {
	C_BLACK,
	C_WHITE,
}	trs80_colour_t;

/** @brief colors for background and foreground */
static uint32_t colors[2];

/** @brief non zero if the emulation stops */
int stop;

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
		trs80_fdc_stop();
		trs80_cas_stop();
		z80_reset(cpu);
		trs80_cas_init();
		trs80_fdc_init();
		break;
	}
}

/** @brief return the system name */
const char *sys_get_name(void)
{
	return "trs80";
}

/** @brief return the system frame timer */
void *sys_get_frame_timer(void)
{
	return frame_timer;
}

/*void sys_cpu_panel_init(void *bitmap)
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
}*/

/*void sys_cpu_panel_update(void *bitmap)
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
}*/

/** @brief mark a video RAM location dirty */
static __inline void set_video_ram_dirty(uint32_t offset, uint32_t mask)
{
	uint32_t o = offset % VIDEO_RAM_SIZE;
	uint32_t b = o % 32;
	video_ram_dirty[o / 32] |= mask << b;
}

/** @brief mark a video RAM location clean */
static __inline void res_video_ram_dirty(uint32_t offset, uint32_t mask)
{
	uint32_t o = offset % VIDEO_RAM_SIZE;
	uint32_t b = o % 32;
	video_ram_dirty[o / 32] &= ~(mask << b);
}

/** @brief get a video RAM location dirty flag */
static __inline int get_video_ram_dirty(uint32_t offset, uint32_t mask)
{
	uint32_t o = offset % VIDEO_RAM_SIZE;
	uint32_t b = o % 32;
	return ((dirty_all | video_ram_dirty[o / 32]) & (mask << b)) ? 1 : 0;
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
	uint8_t *keymap = trs80_kbd_map();
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

/** @brief read nothing */
static uint8_t rd_nop(uint32_t offset)
{
	uint8_t data = 0xff;
	return data;
}

/** @brief read from memory mapped I/O registers */
static uint8_t rd_fdc(uint32_t offset)
{
	uint8_t data = z80.mp.byte.b0;

	switch (offset) {
	case 0x37e0:
	case 0x37e1:
	case 0x37e2:
	case 0x37e3:
		data = trs80_irq_status_r(offset);
		break;
	case 0x37ec:
	case 0x37ed:
	case 0x37ee:
	case 0x37ef:
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

/** @brief write to video RAM memory address */
static void wr_vid(uint32_t offset, uint8_t data)
{
	if (data == mem[offset])
		return;
	mem[offset] = data;
	set_video_ram_dirty(offset, 1);
}

/** @brief write to nowhere (memory mapped keyboard) */
static void wr_nop(uint32_t offset, uint8_t data)
{
	/* no op */
}

/** @brief write to memory mapped I/O range */
static void wr_fdc(uint32_t offset, uint8_t data)
{
	switch (offset) {
	case 0x37e0:
	case 0x37e1:
	case 0x37e2:
	case 0x37e3:
		trs80_motors_w(offset, data);
		break;
	case 0x37ec:
	case 0x37ed:
	case 0x37ee:
	case 0x37ef:
		wd179x_0_w(offset, data);
		break;
	}
}

/**
 * @brief Voltage levels at the audio output
 *
 * The two bits of audio output select between 4 voltage
 * levels at the output. Assuming +/-1 Vss for +/-32767.
 *
 */
static int16_t audio_levels[4] = {
	15073,		/* 0.46 V */
	-16384,
	27852,		/* 0.85 V */
	-16384
};

static void trs80_audio_w(uint8_t data)
{
	int32_t pos = (int32_t)(tmr_elapsed(frame_timer) * audio_samples / frame_timer->restart);
	if (pos < 0 || pos > audio_samples) {
		printf("pos: %#x/%#x\n", pos, audio_samples);
		return;
	}
	while (audio_pos < pos && audio_pos < audio_samples)
		audio_stream[audio_pos++] = audio_value;
	audio_value = audio_levels[data & 3];
}

static uint8_t rd_port(uint32_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0xff) {
	case 0xff:
		port_ff = (port_ff & ~0x10) | trs80_cas_r(offset);
		data = (port_ff << 3) & 0xc0;
//		trs80_audio_w(data >> 7);
		break;
	}
	return data;
}

static void wr_port(uint32_t offset, uint8_t data)
{
	uint8_t changes;

	switch (offset & 0xff) {
	case 0xff:
		changes = port_ff ^ data;
		port_ff = data;
		if (changes & 0x03) {
			trs80_cas_w(offset, data);
			trs80_audio_w(data);
		}
		if (changes & 0x08) {
			/* FIXME: character width */
		}
		break;
	}
}

uint8_t (*rd_mem[L1SIZE])(uint32_t offset) = {
	rd_rom,	rd_rom,	rd_rom,	rd_rom,	/* 0000-0fff */
	rd_rom,	rd_rom,	rd_rom,	rd_rom,	/* 1000-1fff */
	rd_rom,	rd_rom,	rd_rom,	rd_rom,	/* 2000-2fff */
	rd_nop,	rd_fdc,	rd_kbd,	rd_ram,	/* 3000-3fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* 4000-4fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* 5000-5fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* 6000-6fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* 7000-7fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* 8000-8fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* 9000-9fff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* a000-afff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* b000-bfff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* c000-cfff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* d000-dfff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram,	/* e000-efff */
	rd_ram,	rd_ram,	rd_ram,	rd_ram	/* f000-ffff */
};

void (*wr_mem[L1SIZE])(uint32_t offset, uint8_t data) = {
	wr_rom,	wr_rom,	wr_rom,	wr_rom,	/* 0000-0fff */
	wr_rom,	wr_rom,	wr_rom,	wr_rom,	/* 1000-1fff */
	wr_rom,	wr_rom,	wr_rom,	wr_rom,	/* 2000-2fff */
	wr_fdc,	wr_fdc,	wr_nop,	wr_vid,	/* 3000-3fff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* 4000-4fff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* 5000-5fff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* 6000-6fff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* 7000-7fff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* 8000-8fff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* 9000-9fff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* a000-afff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* b000-bfff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* c000-cfff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* d000-dfff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram,	/* e000-efff */
	wr_ram,	wr_ram,	wr_ram,	wr_ram	/* f000-ffff */
};

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

void trs80_frame(uint32_t param)
{
	uint32_t offset;
	uint8_t data;
	int32_t dx, dy;

	while (audio_pos++ < audio_samples)
		audio_stream[audio_pos] = audio_value;
	osd_update_audio_stream(audio_stream);
	audio_pos = 0;
	audio_stream[audio_pos] = audio_value;

	osd_display_frequency((uint64_t)50.0 * cycles_this_frame);
	cycles_this_frame = 0;

	for (offset = 0; offset < VIDEO_RAM_SIZE; offset++) {
		if (0 == get_video_ram_dirty(offset, 1))
			continue;
		data = mem[VIDEO_RAM_BASE + offset];

		if (data < 0x20)
			data |= 0x40;

		dx = (offset % SCREENW) * FONT_W + screen_x;
		dy = (offset / SCREENW) * FONT_H + screen_y;
		osd_font_blit(font, frame, colors, dx, dy, data, 1, 1);
	}
	memset(video_ram_dirty, 0, sizeof(video_ram_dirty));
	dirty_all = 0;

	stop = osd_update(osd_skip_next_frame());
}

static void trs80_update_colors()
{
	uint8_t r, g, b;

	colors[C_BLACK] = oss_bitmap_get_pix_val(frame, 0  ,   0,   0, 255);
	r =  48; g = 48; b = 48;
	osd_align_colors_to_mode(&r, &g, &b);
	colors[C_WHITE] = oss_bitmap_get_pix_val(frame, r, g, b, 255);
}

int trs80_screen(void)
{
	char filename[FILENAME_MAX];
	int32_t ch, w, h, x, y, dx, dy, y0;
	FILE *fp;

	memset(chargen, 0, sizeof(chargen));
	snprintf(filename, sizeof(filename), "%s/%s",
		sys_get_name(), trs80_chr);
	fp = fopen(filename, "rb");
	if (NULL == fp) {
		fprintf(stderr, "fopen(\"%s\",\"%s\") failed\n",
			filename, "rb");
		return -1;
	}

	/* generate character / graphics font */
	for (ch = 0; ch < 128; ch++) {
#if	1
		switch (ch) {
		case ',': case ';': case 'g': case 'p':
		case 'q': case 'y': case '_':
			y0 = 4;
			break;
		default:
			y0 = 2;
		}
#else
		y0 = 2;
#endif
		fread(&chargen[ch * FONT_H + y0], 1, 8, fp);
	}
	fclose(fp);

	for (ch = 128; ch < 256; ch++) {
		for (y = 0; y < FONT_H; y++) {
			uint8_t b = 0x00;
			if ((ch & 0x01) && y < 1*FONT_H/3)
				b |= ((1<<(FONT_W/2))-1) << (FONT_W/2);
			if ((ch & 0x02) && y < 1*FONT_H/3)
				b |= ((1<<(FONT_W/2))-1);
			if ((ch & 0x04) && y >= 1*FONT_H/3 && y < 2*FONT_H/3)
				b |= ((1<<(FONT_W/2))-1) << (FONT_W/2);
			if ((ch & 0x08) && y >= 1*FONT_H/3 && y < 2*FONT_H/3)
				b |= ((1<<(FONT_W/2))-1);
			if ((ch & 0x10) && y >= 2*FONT_H/3)
				b |= ((1<<(FONT_W/2))-1) << (FONT_W/2);
			if ((ch & 0x20) && y >= 2*FONT_H/3)
				b |= ((1<<(FONT_W/2))-1);
			chargen[ch * FONT_H + y] = b;
		}
	}

	osd_font_alloc(&font, 256, FONT_W, FONT_H, FONT_DEPTH_1BPP);
	osd_render_font(font,
		chargen, /* bitmap data */
		0,       /* first code */
		256,     /* count */
		1);      /* top aligned */

	w = SCREENW * FONT_W;
	h = SCREENH * FONT_H;
	if (osd_open_display(w, h, "TRS-80", trs80_update_colors) < 0) {
		fprintf(stderr, "osd_open_display() failed\n");
		return -1;
	}
	trs80_update_colors();

	return 0;
}

int trs80_audio(void)
{
	osd_set_refresh_rate(50.0);
	audio_samples = osd_start_audio_stream(0);
	if (audio_samples <= 0)
		return -1;
	audio_stream = calloc(audio_samples, sizeof(*audio_stream));
	if (NULL == audio_stream)
		return -1;
	return 0;
}

int trs80_memory(void)
{
	char filename[FILENAME_MAX];
	FILE *fp;

	memset(mem, 0, sizeof(mem));

	snprintf(filename, sizeof(filename), "%s/%s",
		sys_get_name(), trs80_rom);
	fp = fopen(filename, "rb");
	if (NULL == fp) {
		printf("%s not found\n", filename);
		return -1;
	}
	if (MAIN_ROM_SIZE != fread(mem, 1, MAIN_ROM_SIZE, fp)) {
		printf("%s too small (expected %#x)\n", filename, MAIN_ROM_SIZE);
		return -1;
	}
	fclose(fp);
	return 0;
}


void trs80_clock(uint32_t param)
{
	trs80_timer_interrupt();
}

int main(int argc, char **argv)
{
	z80_cpu_t *cpu = &z80;
	int dumpmem;
	int i;

	if (osd_init(NULL, trs80_key_dn, trs80_key_up, argc, argv))
		return 1;
	for (i = 1, dumpmem = 0; i < argc; i++)
		if (!strcmp(argv[i], "-d"))
			dumpmem = 1;

	if (trs80_screen() < 0) {
		printf("Screen init: failed\n");
		return 1;
	}

	if (trs80_audio() < 0) {
		printf("Audio init: failed\n");
		return 2;
	}

	if (trs80_memory() < 0) {
		printf("ROM loading: failed\n");
		return 3;
	}
	z80_reset(cpu);
	trs80_cas_init();
	trs80_fdc_init();
	clock_timer = tmr_alloc(trs80_clock, tmr_double_to_time(TIME_IN_HZ(40)),
		0, tmr_double_to_time(TIME_IN_HZ(40)));
	frame_timer = tmr_alloc(trs80_frame, tmr_double_to_time(TIME_IN_HZ(50)),
		0, tmr_double_to_time(TIME_IN_HZ(50)));

	while (!stop)
		tmr_run_cpu(cpu, 1747200.0);

	if (dumpmem) {
		FILE *fp = fopen("trs80.mem", "wb");
		fwrite(mem, 1, MEMSIZE, fp);
		fclose(fp);
	}

	trs80_fdc_stop();
	trs80_cas_stop();
	osd_exit();
	return 0;
}
