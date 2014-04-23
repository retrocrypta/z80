/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * ay8910.c	General Electrics AY8910 emulation
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_AY8910_H_INCLUDED_)
#define _AY8910_H_INCLUDED_

#include "system.h"
#include "timer.h"
#include "osd.h"

#define MAX_AY8910 2

typedef struct ifc_ay8910_s {
	int num;		/* total number of 8910 in the machine */
	int baseclock;
	int mixing_level[MAX_AY8910];
	uint8_t (*port_a_r[MAX_AY8910])(uint32_t);
	uint8_t (*port_b_r[MAX_AY8910])(uint32_t);
	void (*port_a_w[MAX_AY8910])(uint32_t,uint8_t);
	void (*port_b_w[MAX_AY8910])(uint32_t,uint8_t);
}	ifc_ay8910_t;

void ay8910_reset(uint32_t chip);

void ay8910_set_clock(uint32_t chip, uint32_t clk);
void ay8910_set_volume(uint32_t chip, uint32_t channel, uint32_t volume);

void ay8910_update_stream(void);

void ay8910_w(uint32_t chip, uint32_t offset, uint32_t data);
int ay8910_r(uint32_t chip);

int ay8910_sh_start(const ifc_ay8910_t *ifc);
void ay8910_sh_stop(void);
void ay8910_sh_reset(void);

/*********** MEMORY INTERFACE ***********/

uint8_t ay8910_read_port_0_r(uint32_t offset);
uint8_t ay8910_read_port_1_r(uint32_t offset);

void ay8910_control_port_0_w(uint32_t offset, uint8_t data);
void ay8910_control_port_1_w(uint32_t offset, uint8_t data);

void ay8910_write_port_0_w(uint32_t offset, uint8_t data);
void ay8910_write_port_1_w(uint32_t offset, uint8_t data);

#endif	/* !defined(_AY8910_H_INCLUDED_) */
