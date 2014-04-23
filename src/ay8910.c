/* ed:set tabstop=8 noexpandtab: */
/**************************************************************************
 *
 *  ay8910.c	Emulation of the AY-3-8910 sound chip.
 *
 *  Based on various code snippets by Ville Hallik, Michael Cuddy,
 *  Tatsuyuki Satoh, Fabrice Frances, Nicola Salmoria.
 *
 ****************************************************************************/
#include "ay8910.h"

#define	PSG_DEBUG 0

#if	PSG_DEBUG
#define	LL	3
#else
#define	LL	7
#endif

#define MAX_OUTPUT 0x7fff

#define STEP 0x8000

static uint32_t num = 0;

typedef struct chip_ay8910_s {
	int32_t channel;
	int32_t sample_rate;
	uint8_t (*port_a_r)(uint32_t);
	uint8_t (*port_b_r)(uint32_t);
	void (*port_a_w)(uint32_t,uint8_t);
	void (*port_b_w)(uint32_t,uint8_t);
	int32_t latch;
	uint8_t regs[16];
	int32_t last_enable;
	uint32_t update_step;
	int32_t period_a;
	int32_t period_b;
	int32_t period_c;
	int32_t period_n;
	int32_t period_e;
	int32_t count_a;
	int32_t count_b;
	int32_t count_c;
	int32_t count_n;
	int32_t count_e;
	uint32_t vol_a;
	uint32_t vol_b;
	uint32_t vol_c;
	uint32_t vol_e;
	uint8_t envelope_a;
	uint8_t envelope_b;
	uint8_t envelope_c;
	uint8_t output_a;
	uint8_t output_b;
	uint8_t output_c;
	uint8_t output_n;
	int8_t count_env;
	uint8_t hold;
	uint8_t alternate;
	uint8_t attack;
	uint8_t holding;
	int32_t prng;
	uint32_t volume_table[32];
	int32_t audio_samples;
	int16_t *audio_stream;
	int32_t audio_pos;
}	chip_ay8910_t;

/* register id's */
enum {
	REG_AFINE,
	REG_ACOARSE,
	REG_BFINE,
	REG_BCOARSE,
	REG_CFINE,
	REG_CCOARSE,
	REG_NOISEPER,
	REG_ENABLE,
	REG_AVOL,
	REG_BVOL,
	REG_CVOL,
	REG_EFINE,
	REG_ECOARSE,
	REG_ESHAPE,
	REG_PORTA,
	REG_PORTB,
	REG_COUNT
}	AY8910_REG;

#define AY_AFINE	ay->regs[REG_AFINE]
#define AY_ACOARSE	ay->regs[REG_ACOARSE]
#define AY_BFINE	ay->regs[REG_BFINE]
#define AY_BCOARSE	ay->regs[REG_BCOARSE]
#define AY_CFINE	ay->regs[REG_CFINE]
#define AY_CCOARSE	ay->regs[REG_CCOARSE]
#define AY_NOISEPER	ay->regs[REG_NOISEPER]
#define AY_ENABLE	ay->regs[REG_ENABLE]
#define AY_AVOL		ay->regs[REG_AVOL]
#define AY_BVOL		ay->regs[REG_BVOL]
#define AY_CVOL		ay->regs[REG_CVOL]
#define AY_EFINE	ay->regs[REG_EFINE]
#define AY_ECOARSE	ay->regs[REG_ECOARSE]
#define AY_ESHAPE	ay->regs[REG_ESHAPE]
#define AY_PORTA	ay->regs[REG_PORTA]
#define AY_PORTB	ay->regs[REG_PORTB]

static chip_ay8910_t chips[MAX_AY8910];		/* array of ay's */

static void ay8910_update(uint32_t chip, int16_t *buffer, uint32_t length);

void _ay8910_reg_w(uint32_t chip, uint32_t reg, uint32_t data)
{
	chip_ay8910_t *ay = &chips[chip];
	int old;


	ay->regs[reg] = data;

	/*
	 * A note about the period of tones, noise and envelope:
	 * for speed reasons, we count down from the period to 0,
	 * but careful studies of the chip output prove that it
	 * instead counts up from 0 until the counter becomes
	 * greater or equal to the period.
	 * This is an important difference when the program is
	 * rapidly changing the period to modulate the sound.
	 * To compensate for the difference, when the period is
	 * changed we adjust our internal counter.
	 * Also, note that period = 0 is the same as period = 1.
	 * This is mentioned in the YM2203 data sheets.
	 * However, this does NOT apply to the Envelope
	 * period. In that case, period = 0 is half as period = 1.
	 */
	switch (reg) {
	case REG_AFINE:
	case REG_ACOARSE:
		AY_ACOARSE &= 0x0f;
		old = ay->period_a;
		ay->period_a = (AY_AFINE + 256 * AY_ACOARSE) * ay->update_step;
		if (0 == ay->period_a)
			ay->period_a = ay->update_step;
		ay->count_a += ay->period_a - old;
		if (ay->count_a <= 0)
			ay->count_a = 1;
		LOG((LL,"AY8910","channel a period 0x%x, count 0x%x\n",
			ay->period_a, ay->count_a));
		break;

	case REG_BFINE:
	case REG_BCOARSE:
		AY_BCOARSE &= 0x0f;
		old = ay->period_b;
		ay->period_b = (AY_BFINE + 256 * AY_BCOARSE) * ay->update_step;
		if (0 == ay->period_b)
			ay->period_b = ay->update_step;
		ay->count_b += ay->period_b - old;
		if (ay->count_b <= 0)
			ay->count_b = 1;
		LOG((LL,"AY8910","channel b period 0x%x, count 0x%x\n",
			ay->period_b, ay->count_b));
		break;

	case REG_CFINE:
	case REG_CCOARSE:
		AY_CCOARSE &= 0x0f;
		old = ay->period_c;
		ay->period_c = (AY_CFINE + 256 * AY_CCOARSE) * ay->update_step;
		if (0 == ay->period_c)
			ay->period_c = ay->update_step;
		ay->count_c += ay->period_c - old;
		if (ay->count_c <= 0)
			ay->count_c = 1;
		LOG((LL,"AY8910","channel c period 0x%x, count 0x%x\n",
			ay->period_c, ay->count_c));
		break;

	case REG_NOISEPER:
		AY_NOISEPER &= 31;
		old = ay->period_n;
		ay->period_n = AY_NOISEPER * ay->update_step;
		if (0 == ay->period_n)
			ay->period_n = ay->update_step;
		ay->count_n += ay->period_n - old;
		if (ay->count_n <= 0)
			ay->count_n = 1;
		LOG((LL,"AY8910","noise period 0x%x, count 0x%x\n",
			ay->period_n, ay->count_n));
		break;

	case REG_ENABLE:
		if (-1 == ay->last_enable ||
		    (ay->last_enable & 0x40) != (AY_ENABLE & 0x40)) {
			/* write out 0xff if port set to input */
			if (NULL != ay->port_a_w)
				(*ay->port_a_w)(0, (AY_ENABLE & 0x40) ?
					AY_PORTA : 0xff);
		}

		if (-1 == ay->last_enable ||
		    (ay->last_enable & 0x80) != (AY_ENABLE & 0x80)) {
			/* write out 0xff if port set to input */
			if (NULL != ay->port_b_w)
				(*ay->port_b_w)(0, (AY_ENABLE & 0x80) ?
					AY_PORTB : 0xff);
		}

		ay->last_enable = AY_ENABLE;
		break;

	case REG_AVOL:
		AY_AVOL &= 31;
		ay->envelope_a = AY_AVOL & 0x10;
		if (ay->envelope_a) {
			ay->vol_a = ay->vol_e;
		} else {
			ay->vol_a = ay->volume_table[AY_AVOL ? AY_AVOL*2+1 : 0];
		}
		break;

	case REG_BVOL:
		AY_BVOL &= 31;
		ay->envelope_b = AY_BVOL & 0x10;
		if (ay->envelope_b) {
			ay->vol_b = ay->vol_e;
		} else {
			ay->vol_b = ay->volume_table[AY_BVOL ? AY_BVOL*2+1 : 0];
		}
		break;

	case REG_CVOL:
		AY_CVOL &= 31;
		ay->envelope_c = AY_CVOL & 0x10;
		if (ay->envelope_c) {
			ay->vol_c = ay->vol_e;
		} else {
			ay->vol_c = ay->volume_table[AY_CVOL ? AY_CVOL*2+1 : 0];
		}
		break;
	case REG_EFINE:
	case REG_ECOARSE:
		old = ay->period_e;
		ay->period_e = (AY_EFINE + 256 * AY_ECOARSE) * ay->update_step;
		if (0 == ay->period_e)
			ay->period_e = ay->update_step / 2;
		ay->count_e += ay->period_e - old;
		if (ay->count_e <= 0)
			ay->count_e = 1;
		LOG((LL,"AY8910","envelope period 0x%x, count 0x%x\n",
			ay->period_e, ay->count_e));
		break;

	case REG_ESHAPE:
		/* envelope shapes:
		 * C AtAlH
		 * 0 0 x x  \___
		 *
		 * 0 1 x x  /___
		 *
		 * 1 0 0 0  \\\\
		 *
		 * 1 0 0 1  \___
		 *
		 * 1 0 1 0  \/\/
		 *           
		 * 1 0 1 1  \~~~
		 *
		 * 1 1 0 0  ////
		 *
		 * 1 1 0 1  /~~~
		 *
		 * 1 1 1 0  /\/\
		 *
		 * 1 1 1 1  /___
		 * The envelope counter on the AY-3-8910 has 16 steps.
		 * On the YM2149 it has twice the steps, happening twice
		 * as fast. Since the end result is just a smoother curve,
		 * we always use the YM2149 behaviour.
		 */
		AY_ESHAPE &= 0x0f;
		ay->attack = (AY_ESHAPE & 0x04) ? 31 : 0;
		if (0 == (AY_ESHAPE & 0x08)) {
			/* if Continue = 0, map the shape to the
			 * equivalent one which has Continue = 1
			 */
			ay->hold = 1;
			ay->alternate = ay->attack;
		} else {
			ay->hold = AY_ESHAPE & 0x01;
			ay->alternate = AY_ESHAPE & 0x02;
		}
		ay->count_e = ay->period_e;
		ay->count_env = 31;
		ay->holding = 0;
		ay->vol_e = ay->volume_table[ay->count_env ^ ay->attack];
		if (ay->envelope_a)
			ay->vol_a = ay->vol_e;
		if (ay->envelope_b)
			ay->vol_b = ay->vol_e;
		if (ay->envelope_c)
			ay->vol_c = ay->vol_e;
		break;

	case REG_PORTA:
		if (0 == (AY_ENABLE & 0x40)) {
			LOG((1,"AY8910","warning: write to 8910 #%d port A set as input - ignored\n",
				chip));
			break;
		}
		if (NULL == ay->port_a_w) {
			LOG((1,"AY8910","warning - write %02x to 8910 #%d port A\n",
				AY_PORTA, chip));
			break;
		}
		(*ay->port_a_w)(0, AY_PORTA);
		break;

	case REG_PORTB:
		if (0 == (AY_ENABLE & 0x80)) {
			LOG((1,"AY8910","warning: write to 8910 #%d port B set as input - ignored\n",
				chip));
			break;
		}
		if (NULL == ay->port_b_w) {
			LOG((1,"AY8910","warning - write %02x to 8910 #%d port B\n",
				AY_PORTB, chip));
			break;
		}
		(*ay->port_b_w)(0, AY_PORTB);
		break;
	}
}


/* write a register on ay8910 chip number 'n' */
void ay8910_reg_w(uint32_t chip, uint32_t reg, uint32_t data)
{
	chip_ay8910_t *ay = &chips[chip];

	if (reg >= REG_COUNT)
		return;
	if (reg < REG_PORTA) {
		if (reg == REG_ESHAPE || data != ay->regs[reg]) {
			/* update the output buffer before changing the register */
			tmr_t *frame_timer = sys_get_frame_timer();
			uint32_t pos = (uint32_t)(tmr_elapsed(frame_timer) * ay->audio_samples / frame_timer->restart);
			uint32_t length;
			length = pos - ay->audio_pos;
			if (pos > ay->audio_pos && pos <= ay->audio_samples) {
				ay8910_update(chip, ay->audio_stream + ay->audio_pos, length);
				ay->audio_pos = pos;
			}
		}
	}

	_ay8910_reg_w(chip, reg, data);
}



uint32_t ay8910_reg_r(uint32_t chip, uint32_t reg)
{
	chip_ay8910_t *ay = &chips[chip];


	if (reg >= REG_COUNT)
		return 0;

	switch (reg) {
	case REG_PORTA:
		if (0 != (AY_ENABLE & 0x40)) {
			LOG((LL,"AY8910","warning: read from 8910 #%d port A set as output\n",
				chip));
		}
		/*
		 * even if the port is set as output, we still need to
		 * return the external data. Some games, like kidniki,
		 * need this to work.
		 */
		if (NULL != ay->port_a_r) {
			AY_PORTA = (*ay->port_a_r)(0);
		} else {
			LOG((1,"AY8910","warning - read 8910 #%d port A\n", chip));
		}
		break;

	case REG_PORTB:
		if (0 != (AY_ENABLE & 0x80)) {
			LOG((LL,"AY8910","warning: read from 8910 #%d port B set as output\n",
				chip));
		}
		if (NULL != ay->port_b_r) {
			AY_PORTB = (*ay->port_b_r)(0);
		} else {
			LOG((1,"AY8910","warning - read 8910 #%d port B\n", chip));
		}
		break;
	}
	return ay->regs[reg];
}


void ay8910_w(uint32_t chip, uint32_t offset, uint32_t data)
{
	chip_ay8910_t *ay = &chips[chip];

	if (offset & 1) {
		/* data port */
		ay8910_reg_w(chip, ay->latch, data);
	} else {
		/* register port */
		ay->latch = data & 0x0f;
	}
}

int ay8910_r(uint32_t chip)
{
	chip_ay8910_t *ay = &chips[chip];

	return ay8910_reg_r(chip, ay->latch);
}

static void ay8910_update(uint32_t chip, int16_t *buffer, uint32_t length)
{
	chip_ay8910_t *ay = &chips[chip];
	int outl = length;
	int outn;
	int sum;

	/*
	 * The 8910 has three outputs, each output is the mix of one of
	 * the three tone generators and of the (single) noise generator.
	 * The two are mixed BEFORE going into the DAC.
	 * The formula to mix each channel is:
	 * (ToneOn | ToneDisable) & (NoiseOn | NoiseDisable).
	 * Note that this means that if both tone and noise are disabled,
	 * the output is 1, not 0, and can be modulated changing the volume.
	 */


	/*
	 * If the channels are disabled, set their output to 1, and increase
	 * the counter, if necessary, so they will not be inverted during
	 * this update.
	 * Setting the output to 1 is necessary because a disabled channel
	 * is locked into the ON state (see above); and it has no effect
	 * if the volume is 0.
	 * If the volume is 0, increase the counter, but don't touch
	 * the output.
	 */
	if (AY_ENABLE & 0x01) {
		if (ay->count_a <= outl * STEP)
			ay->count_a += outl * STEP;
		ay->output_a = 1;
	} else if (0 == AY_AVOL) {
		/* Note that I do count += length, NOT count = length + 1.
		 * You might think it's the same since the volume is 0,
		 * but doing the latter could cause interferencies when
		 * the program is rapidly modulating the volume.
		 */
		if (ay->count_a <= outl * STEP)
			ay->count_a += outl * STEP;
	}

	if (AY_ENABLE & 0x02) {
		if (ay->count_b <= outl * STEP)
			ay->count_b += outl * STEP;
		ay->output_b = 1;
	} else if (0 == AY_BVOL) {
		if (ay->count_b <= outl * STEP)
			ay->count_b += outl * STEP;
	}

	if (AY_ENABLE & 0x04) {
		if (ay->count_c <= outl * STEP)
			ay->count_c += outl * STEP;
		ay->output_c = 1;
	} else if (0 == AY_CVOL) {
		if (ay->count_c <= outl * STEP)
			ay->count_c += outl * STEP;
	}

	/*
	 * For the noise channel we must not touch output_n -
	 * it's also not necessary since we use outn.
	 */
	if (0x38 == (AY_ENABLE & 0x38))	/* all off */
		if (ay->count_n <= outl * STEP)
			ay->count_n += outl * STEP;

	outn = ay->output_n | AY_ENABLE;

	/* buffering loop */
	while (length > 0) {
		/* vola, volb and volc keep track of how long
		 * each square wave stays in the 1 position
		 * during the sample period.
		 */
		int vola = 0;
		int volb = 0;
		int volc = 0;
		int left = STEP;

		do {
			int nextevent = (ay->count_n < left) ? ay->count_n : left;

			if (outn & 0x08) {
				if (0 != ay->output_a)
					vola += ay->count_a;
				ay->count_a -= nextevent;

				/*
				 * period_a is the half period of the square
				 * wave. Here, in each loop I add period_a
				 * twice, so that at the end of the loop the
				 * square wave is in the same status (0 or 1)
				 * it was at the start.
				 * vola is also incremented by period_a,
				 * since the wave has been 1 exactly half
				 * of the time, regardless of the initial
				 * position.
				 * If we exit the loop in the middle,
				 * output_a has to be inverted and vola
				 * incremented only if the exit status of
				 * the square wave is 1.
				 */
				while (ay->count_a <= 0) {
					ay->count_a += ay->period_a;
					if (ay->count_a > 0) {
						ay->output_a ^= 1;
						if (0 != ay->output_a)
							vola += ay->period_a;
						break;
					}
					ay->count_a += ay->period_a;
					vola += ay->period_a;
				}
				if (0 != ay->output_a)
					vola -= ay->count_a;
			} else {
				ay->count_a -= nextevent;
				while (ay->count_a <= 0) {
					ay->count_a += ay->period_a;
					if (ay->count_a > 0) {
						ay->output_a ^= 1;
						break;
					}
					ay->count_a += ay->period_a;
				}
			}

			if (outn & 0x10) {
				if (0 != ay->output_b)
					volb += ay->count_b;
				ay->count_b -= nextevent;
				while (ay->count_b <= 0) {
					ay->count_b += ay->period_b;
					if (ay->count_b > 0) {
						ay->output_b ^= 1;
						if (0 != ay->output_b)
							volb += ay->period_b;
						break;
					}
					ay->count_b += ay->period_b;
					volb += ay->period_b;
				}
				if (0 != ay->output_b)
					volb -= ay->count_b;
			} else {
				ay->count_b -= nextevent;
				while (ay->count_b <= 0) {
					ay->count_b += ay->period_b;
					if (ay->count_b > 0) {
						ay->output_b ^= 1;
						break;
					}
					ay->count_b += ay->period_b;
				}
			}

			if (outn & 0x20) {
				if (0 != ay->output_c)
					volc += ay->count_c;
				ay->count_c -= nextevent;
				while (ay->count_c <= 0) {
					ay->count_c += ay->period_c;
					if (ay->count_c > 0) {
						ay->output_c ^= 1;
						if (0 != ay->output_c)
							volc += ay->period_c;
						break;
					}
					ay->count_c += ay->period_c;
					volc += ay->period_c;
				}
				if (0 != ay->output_c)
					volc -= ay->count_c;
			} else {
				ay->count_c -= nextevent;
				while (ay->count_c <= 0) {
					ay->count_c += ay->period_c;
					if (ay->count_c > 0) {
						ay->output_c ^= 1;
						break;
					}
					ay->count_c += ay->period_c;
				}
			}

			ay->count_n -= nextevent;
			if (ay->count_n <= 0) {
				/* Is noise output going to change? */
				if ((ay->prng + 1) & 2) {
					/* (bit0^bit1) */
					ay->output_n = ~ay->output_n;
					outn = ay->output_n | AY_ENABLE;
				}

				/*
				 * The Pseudo Random Number Generator of the
				 * 8910 is a 17-bit shift register. The input
				 * to the shift register is bit0 XOR bit3 
				 * (bit0 is the output). This was verified
				 * on AY-3-8910 and YM2149 chips.
				 */

				/*
				 * The following is a fast way to compute
				 * bit17 = bit0^bit3. Instead of doing all
				 * the logic operations, we only check
				 * bit0, relying on the fact that after
				 * three shifts of the register, what now
				 * is bit3 will become bit0, and will 
				 * invert, if necessary, bit14, which
				 * previously was bit17.
				 */

				 /* This version is called the "Galois configuration". */
				if (0 != (ay->prng & 1))
					ay->prng ^= 0x24000;
				ay->prng >>= 1;
				ay->count_n += ay->period_n;
			}

			left -= nextevent;
		} while (left > 0);

		/* update envelope */
		if (0 == ay->holding) {
			ay->count_e -= STEP;
			if (ay->count_e <= 0) {
				do {
					ay->count_env--;
					ay->count_e += ay->period_e;
				} while (ay->count_e <= 0);

				/* check envelope current position */
				if (ay->count_env < 0) {
					if (0 != ay->hold) {
						if (0 != ay->alternate)
							ay->attack ^= 31;
						ay->holding = 1;
						ay->count_env = 0;
					} else {
						/* if count_env has looped
						 * an odd number of times
						 * (usually 1), invert the
						 * output.
						 */
						if (0 != ay->alternate && 0 != (ay->count_env & 0x20))
 							ay->attack ^= 31;
						ay->count_env &= 31;
					}
				}

				ay->vol_e = ay->volume_table[ay->count_env ^ ay->attack];
				/* reload volumes */
				if (ay->envelope_a)
					ay->vol_a = ay->vol_e;
				if (ay->envelope_b)
					ay->vol_b = ay->vol_e;
				if (ay->envelope_c)
					ay->vol_c = ay->vol_e;
			}
		}

		sum = (vola * ay->vol_a + volb * ay->vol_b + volc * ay->vol_c) / STEP;
		if (sum < -32767)
			*buffer = -32767;
		else if (sum > 32767)
			*buffer = 32767;
		else
			*buffer = (int16_t)sum;
		buffer++;
		length--;
	}
}


void ay8910_update_stream(void)
{
	uint32_t chip;
	uint32_t length;

	/* XXX: this doesn't work for > 1 chip (need a mixer) */
	for (chip = 0; chip < num; chip++) {
		chip_ay8910_t *ay = &chips[chip];
		length = ay->audio_samples - ay->audio_pos;
		ay8910_update(chip, ay->audio_stream + ay->audio_pos, length);
		osd_update_audio_stream(ay->audio_stream);
		ay->audio_pos = 0;
	}
}

void ay8910_set_clock(uint32_t chip, uint32_t clk)
{
	chip_ay8910_t *ay = &chips[chip];

	/*
	 * The step clock for the tone and noise generators is the chip clock
	 * divided by 8; for the envelope generator of the AY-3-8910,
	 * it is half that much (clock/16), but the envelope of the YM2149
	 * goes twice as fast, therefore again clock/8.
	 * Here we calculate the number of steps which happen during one
	 * sample at the given sample rate.
	 * Number of events = sample rate / (clock/8).
	 * STEP is a multiplier used to turn the fraction into a
	 * fixed point number.
	 */
	ay->update_step = ((double)STEP * ay->sample_rate * 8 + clk/2) / clk;
	LOG((LL,"AY8910","update step: %d\n", ay->update_step));
}


static void build_mixer_table(uint32_t chip)
{
	chip_ay8910_t *ay = &chips[chip];
	int i;
	double out;

	/*
	 * Calculate the volume->voltage conversion table
	 * The AY-3-8910 has 16 levels, in a logarithmic scale (3dB per step)
	 * The YM2149 still has 16 levels for the tone generators, but 32 for
	 * the envelope generator (1.5dB per step).
	 */
	out = MAX_OUTPUT;
	for (i = 31; i > 0; i--) {
		/* round to nearest */
		ay->volume_table[i] = out + 0.5;
		out /= 1.188502227;	/* = 10 ^ (1.5/20) = 1.5dB */
	}
	ay->volume_table[0] = 0;
}

void ay8910_reset(uint32_t chip)
{
	int i;
	chip_ay8910_t *ay = &chips[chip];

	ay->latch = 0;
	ay->prng = 1;
	ay->output_a = 0;
	ay->output_b = 0;
	ay->output_c = 0;
	ay->output_n = 0xff;
	ay->last_enable = -1;	/* force a write */

	/*
	 * ay8910_reg_w() (without the leading underscore) uses
	 * the timer system; we cannot call it at this time
	 * because the timer system has not been initialized.
	 */
	for (i = 0; i < REG_PORTA; i++)
		_ay8910_reg_w(chip, i, 0);
}

void ay8910_sh_reset(void)
{
	uint32_t i;

	for (i = 0; i < num; i++)
		ay8910_reset(i);
}

static int ay8910_init(uint32_t chip, int clock, int sample_rate,
	uint8_t (*port_a_r)(uint32_t), uint8_t (*port_b_r)(uint32_t),
	void (*port_a_w)(uint32_t,uint8_t), void (*port_b_w)(uint32_t,uint8_t))
{
	chip_ay8910_t *ay = &chips[chip];

	memset(ay,0,sizeof(chip_ay8910_t));
	ay->sample_rate = sample_rate;
	ay->port_a_r = port_a_r;
	ay->port_b_r = port_b_r;
	ay->port_a_w = port_a_w;
	ay->port_b_w = port_b_w;

	ay8910_set_clock(chip, clock);
	ay8910_reset(chip);

	ay->audio_samples = osd_start_audio_stream(0);
	if (ay->audio_samples < 0)
		return -1;
	ay->audio_stream = calloc(ay->audio_samples + 2, sizeof(int16_t));
	ay->audio_pos = 0;

	return 0;
}


int ay8910_sh_start(const ifc_ay8910_t *ifc)
{
	uint32_t chip;

	num = ifc->num;
	for (chip = 0; chip < num; chip++) {
		int rc = ay8910_init(chip, ifc->baseclock, osd_get_sample_rate(),
			ifc->port_a_r[chip], ifc->port_b_r[chip],
			ifc->port_a_w[chip], ifc->port_b_w[chip]);
		if (0 != rc)
			return rc;
		build_mixer_table(chip);
	}

	return 0;
}

void ay8910_sh_stop(void)
{
	uint32_t chip;
	for (chip = 0; chip < num; chip++) {
		chip_ay8910_t *ay = &chips[chip];
		if (NULL != ay->audio_stream) {
			free(ay->audio_stream);
			ay->audio_stream = NULL;
			ay->audio_samples = 0;
			ay->audio_pos = 0;
		}
	}
	osd_stop_audio_stream();
	num = 0;
}

/* AY8910 interface */
uint8_t ay8910_read_port_0_r(uint32_t offset) { return ay8910_r(0); }
uint8_t ay8910_read_port_1_r(uint32_t offset) { return ay8910_r(1); }

void ay8910_control_port_0_w(uint32_t offset, uint8_t data) { ay8910_w(0,0,data); }
void ay8910_control_port_1_w(uint32_t offset, uint8_t data) { ay8910_w(1,0,data); }

void ay8910_write_port_0_w(uint32_t offset, uint8_t data) { ay8910_w(0,1,data); }
void ay8910_write_port_1_w(uint32_t offset, uint8_t data) { ay8910_w(1,1,data); }
