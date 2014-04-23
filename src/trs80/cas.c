/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * cas.c	TRS-80 cassette emulation.
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#include "trs80/cas.h"

#define	CAS_DEBUG	1

#define	BUFF_SIZE	65536

#if	CAS_DEBUG
#define	LL	3
#else
#define	LL	7
#endif

typedef struct trs80_cas_s {
	char name[12+1];

	/** @brief current cassette file handle for input */
	struct img_s *get_img;

	/** @brief current cassette file handle for output */
	struct img_s *put_img;

	/** @brief buffer for the first eight bytes at write (to extract a filename) */
	uint8_t *buff;

	/** @brief current offset into buff */
	size_t offs;

	/** @brief size of buff */
	size_t size;

	/** @brief byte count within cassette file */
	int count;

	/** @brief number of sync and data bits that were written */
	int put_bitcnt;

	/** @brief sync and data bits mask */
	uint32_t put_shiftreg;

	/** @brief number of sync and data bits to read */
	int get_bitcnt;

	/** @brief sync and data bits mask */
	uint32_t get_shiftreg;

	/** @brief time for the next bit at read */
	int64_t bit_time;

	/** @brief flag if writing to cassette detected the sync header A5 already */
	int in_sync;

	/** @brief time at last input port read */
	tmr_time_t get_time;

	/** @brief time at last output port change */
	tmr_time_t put_time;

	/** @brief value of the input bit */
	uint8_t get_bit;

}	trs80_cas_t;

static trs80_cas_t cas;

/* a prototype to be called from trs80_stop_machine */
static void cas_put_close(void);


int trs80_cas_init(void)
{
	memset(&cas, 0, sizeof(cas));
	cas.buff = malloc(BUFF_SIZE);
	if (NULL == cas.buff)
		return -1;

	cas.size = BUFF_SIZE;
	return 0;
}

void trs80_cas_stop(void)
{
	cas_put_close();
}

/*******************************************************************
 * cas_put_byte
 * write next data byte to virtual cassette. After collecting the
 * first nine bytes try to extract the kind of data and filename.
 *******************************************************************/
static void cas_put_byte(uint8_t value)
{
	int i;

	cas.buff[cas.offs++] = value;
	if (++cas.count != 9)
		return;
	if (cas.buff[1] == 0xd3) {
		/* BASIC cassette: A5 D3 D3 D3 xx ... */
		sprintf(cas.name, "%c.cas", cas.buff[4]);
	} else if (cas.buff[1] == 0x55 && cas.buff[8] == 0x3c) {
		/* SYSTEM cassette: A5 55 xx xx xx xx xx xx 3c ... */
		sprintf(cas.name, "%-6.6s.cas", cas.buff + 2);
	} else {
		strcpy(cas.name, "unknown.cas");
	}
	for (i = 0; i < strlen(cas.name); i++)
		cas.name[i] = tolower((uint8_t)cas.name[i]);
	cas.put_img = img_fopen(cas.name, IMG_TYPE_CAS, "wb");
	if (NULL == cas.put_img)
		return;
	/* insert 256 bytes of zeroes before the data */
	memmove(&cas.buff[0x100], &cas.buff[0], 9);
	memset(&cas.buff[0], 0, 0x100);
	cas.offs = 0x109;
}

/*******************************************************************
 * cas_put_close
 * Flush output buffer, and close virtual cassette output file.
 *******************************************************************/
static void cas_put_close(void)
{
	if (NULL != cas.put_img) {
		if (cas.put_bitcnt > 0) {
			uint8_t data;
			while (cas.put_bitcnt < 17) {
				cas.put_shiftreg <<= 1;
				cas.put_bitcnt++;
			}
			/* extract data bits to value */
			data = 	((cas.put_shiftreg >> (14-7)) & 0x80) |
				((cas.put_shiftreg >> (12-6)) & 0x40) |
				((cas.put_shiftreg >> (10-5)) & 0x20) |
				((cas.put_shiftreg >> ( 8-4)) & 0x10) |
				((cas.put_shiftreg >> ( 6-3)) & 0x08) |
				((cas.put_shiftreg >> ( 4-2)) & 0x04) |
				((cas.put_shiftreg >> ( 2-1)) & 0x02) |
				((cas.put_shiftreg >> ( 0-0)) & 0x01);
			LOG((LL,"CAS","image data (%02d) 0x%02x\n",
				cas.put_bitcnt, data));
			cas_put_byte(data);
		}
		img_fwrite(cas.put_img, cas.buff, cas.offs);
		img_fclose(cas.put_img);
		cas.put_img = NULL;
	}
	cas.put_bitcnt = 0;
	cas.put_shiftreg = 0x0000;
	cas.in_sync = 0;
	cas.count = 0;
	cas.offs = 0;
}

/*******************************************************************
 * cas_put_bit
 * port FF cassette status bit changed. Figure out what to do with it
 *******************************************************************/
static void cas_put_bit(uint8_t data)
{
	tmr_time_t now = time_now();
	tmr_time_t diff = now - cas.put_time;
	tmr_time_t limit = 480000;

	cas.get_bit = 0x00;

	/* remember the cycle count of this write */
	cas.put_time = now;

	/* overrun since last write ? */
	if (diff >= 1600000) {
		/* reset cassette output */
		cas_put_close();
		cas.in_sync = 0;
		return;
	}

	if (0x01 != (data & 0x03))
		return;

	cas.put_shiftreg = (cas.put_shiftreg << 1) | 1;
	LOG((LL,"CAS","image %7lld %-2d %-2d bits 0x%04X\n",
		diff, cas.in_sync, cas.put_bitcnt, cas.put_shiftreg & 0xffff));
	switch (cas.in_sync) {
	case 0: /* look for silence 00 */
		if (0xaaaa == (cas.put_shiftreg & 0xffff)) {
			LOG((LL,"CAS","image sync1 written\n"));
			cas.in_sync = 1;
		}
		break;
	case 1: /* look for sync A5 */
		if (0xeebb == (cas.put_shiftreg & 0xffff)) {
			LOG((LL,"CAS","image sync2 written (#1)\n"));
			cas.in_sync = 2;
			cas_put_byte(0xa5);
			cas.put_bitcnt = 0;
			cas.put_shiftreg = 0;
		}
		break;
	case 2: /* count 1 bit */
		cas.put_bitcnt += 1;
		break;
	}

	/* change within time for a 1 bit ? */
	if (diff >= limit) {
		cas.put_shiftreg = cas.put_shiftreg << 1;
		LOG((LL,"CAS","image %4d %-2d %-2d bits 0x%04X\n",
			diff, cas.in_sync, cas.put_bitcnt, cas.put_shiftreg & 0xffff));
		/* no change within time indicates a 0 bit */
		switch (cas.in_sync) {
		case 0: /* look for silence 00 */
			if (0xaaaa == (cas.put_shiftreg & 0xffff)) {
				LOG((LL,"CAS","image sync1 written\n"));
				cas.in_sync = 1;
			}
			break;
		case 1: /* look for sync A5 */
			if (0xeebb == (cas.put_shiftreg & 0xffff)) {
				LOG((LL,"CAS","image sync2 written (#2)\n"));
				cas.in_sync = 2;
			}
			break;
		case 2: /* count 1 bit */
			cas.put_bitcnt += 1;
			break;
		}
	}

	/* collected 8 sync plus 8 data bits ? */
	if (cas.put_bitcnt > 16) {
		/* extract data bits to value */
		uint8_t data = ((cas.put_shiftreg >> (14-7)) & 0x80) |
			((cas.put_shiftreg >> (12-6)) & 0x40) |
			((cas.put_shiftreg >> (10-5)) & 0x20) |
			((cas.put_shiftreg >> ( 8-4)) & 0x10) |
			((cas.put_shiftreg >> ( 6-3)) & 0x08) |
			((cas.put_shiftreg >> ( 4-2)) & 0x04) |
			((cas.put_shiftreg >> ( 2-1)) & 0x02) |
			((cas.put_shiftreg >> ( 0-0)) & 0x01);
		LOG((LL,"CAS","image data (%02d) 0x%02x\n",
			cas.put_bitcnt, data));
		cas.put_bitcnt -= 16;
		cas.put_shiftreg = 0x0000;
		cas_put_byte(data);
	}
}

/*******************************************************************
 * cas_get_open
 * Open a virtual tape image file for input. Look for a special
 * header and skip leading description, if present.
 * The filename is taken from BASIC input buffer at 41E8 ff.
 *******************************************************************/
static void cas_get_open(void)
{
#if	0
	char *p;
	int i;

	if (NULL != cas.get_img)
		return;

	cas.count = 0;

	/* extract name from input buffer */
	sprintf(cas.name, "%-6.6s", mem + 0x41e8);
	p = strchr(cas.name, ' ');
	if (NULL != p)
		*p = '\0';
	if (cas.name[0] == ' ')
		return;
	for (i = 0; i < strlen(cas.name); i++)
		cas.name[i] = tolower((uint8_t)cas.name[i]);
	strcat(cas.name, ".cas");
#else
	if (NULL != cas.get_img)
		return;
	cas.count = 0;
	sprintf(cas.name, "bable.cas");
#endif

	cas.get_img = img_fopen(cas.name, IMG_TYPE_CAS, "rb");
	LOG((LL,"CAS","cas_get_open '%s' (0x%p)\n", cas.name, cas.get_img));
	if (NULL == cas.get_img)
		return;

	LOG((LL,"CAS","image load '%s'\n", cas.name));

	cas.size = img_fread(cas.get_img, cas.buff, BUFF_SIZE);
	cas.offs = 0;
}

/*******************************************************************
 * cas_get_byte
 * read next byte from input tape image file.
 * the first 32 bytes are faked to be sync header AA.
 *******************************************************************/
static void cas_get_byte(void)
{
	uint8_t data;

	if (NULL == cas.get_img)
		cas_get_open();

	if (cas.offs < cas.size) {
		data = cas.buff[cas.offs];
		cas.offs++;
		if (0 == cas.in_sync && 0xa5 == data)
			cas.in_sync = 1;
		LOG((LL,"CAS","image read 0x%02x '%s' (0x%x/0x%x)\n",
			data, cas.name, cas.offs, cas.size));
	} else {
		/* reading should have stopped... */
		data = 0x00;
		LOG((LL,"CAS","image read 0x%02x '%s' (beyond end)\n",
			data, cas.name));
	}

	cas.get_shiftreg = 0xaaaa;
	if (data & 0x80)
		cas.get_shiftreg ^= 0x4000;
	if (data & 0x40)
		cas.get_shiftreg ^= 0x1000;
	if (data & 0x20)
		cas.get_shiftreg ^= 0x0400;
	if (data & 0x10)
		cas.get_shiftreg ^= 0x0100;
	if (data & 0x08)
		cas.get_shiftreg ^= 0x0040;
	if (data & 0x04)
		cas.get_shiftreg ^= 0x0010;
	if (data & 0x02)
		cas.get_shiftreg ^= 0x0004;
	if (data & 0x01)
		cas.get_shiftreg ^= 0x0001;
	cas.get_bitcnt = 16;
	cas.count++;
	LOG((LL,"CAS","image bits %2d %04X\n",
		cas.get_bitcnt, cas.get_shiftreg));
}


/*******************************************************************
 * cas_get_bit
 * Port FF tape status read. Check for cassette input toggle
 *******************************************************************/
static void cas_get_bit(void)
{
	tmr_time_t now = time_now();
	tmr_time_t diff = now - cas.get_time;
	tmr_time_t limit = 480000;

	/* remember the cycle count of this read */
	cas.get_time = now;

	/* overrun since last read ? */
	if (diff >= 1600000) {
		if (NULL != cas.get_img) {
			img_fclose(cas.get_img);
			cas.get_img = NULL;
			LOG((LL,"CAS","image file closed (diff:%lld)\n", diff));
		}
		cas.get_bitcnt = 0;
		cas.get_shiftreg = 0x0000;
		cas.bit_time = 0;
		return;
	}

	/* count down time */
	cas.bit_time -= diff;

	/* time for the next sync or data bit ? */
	if (cas.bit_time <= 0) {
		/* approx time for a bit */
		cas.bit_time += limit;
		if (--cas.get_bitcnt <= 0) {
			/* need to read get new data */
			cas_get_byte();
			cas.bit_time = 0;
		}
		LOG((LL,"CAS","image %7lld %+9lld %-2d bits 0x%04X\n",
			diff, cas.bit_time, cas.in_sync, cas.get_shiftreg & 0xffff));
		/* shift next sync or data bit to bit 16 */
		cas.get_shiftreg <<= 1;
		cas.get_bit = (uint8_t)((cas.get_shiftreg >> 12) & 0x10);
	}
}

void trs80_cas_w(uint32_t offset, uint8_t data)
{
	cas_put_bit(data);
}

uint8_t trs80_cas_r(uint32_t offset)
{
	cas_get_bit();

	return cas.get_bit;
}
