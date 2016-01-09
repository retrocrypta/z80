/****************************************************************************
 *
 *	Colour Genie cassette emulation.
 *
 * $Id: cas.c,v 1.3 2005/12/21 02:40:54 pullmoll Exp $
 ****************************************************************************/
#include "cgenie/cas.h"

#define	CAS_DEBUG	0

#define CAS_HEADER "Colour Genie - Virtual Tape File"
#define	BUFF_SIZE	65536

#define	TIMING_CONST	5500

#if	CAS_DEBUG
#define	LL	3
#else
#define	LL	7
#endif

typedef struct cgenie_cas_s {
	char name[12+1];

	/** @brief current cassette file handle for input */
	struct img_s *get_img;

	/** @brief current cassette file handle for output */
	struct img_s *put_img;

	/** @brief buffer for the first eight bytes at write (to extract a filename) */
	uint8_t *buff;

	size_t offs;
	size_t size;

	/* file offset within cassette file */
	int count;

	/* number of sync and data bits that were written */
	int put_bitcnt;

	/* number of sync and data bits to read */
	int get_bitcnt;

	/* sync and data bits mask */
	uint32_t shiftreg;

	/* time for the next bit at read */
	int64_t bit_time;

	/* flag if writing to cassette detected the sync header A5 already */
	int in_sync;

	/* time at last output port change */
	tmr_time_t put_time;

	/* time at last input port read */
	tmr_time_t get_time;

	uint8_t get_bit;

}	cgenie_cas_t;

static cgenie_cas_t cas;

/* a prototype to be called from cgenie_stop_machine */
static void cas_put_close(void);


int cgenie_cas_init(void)
{
	memset(&cas, 0, sizeof(cas));
	cas.buff = malloc(BUFF_SIZE);
	if (NULL == cas.buff)
		return -1;

	return 0;
}

void cgenie_cas_stop(void)
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
	if (cas.count < 9) {
		cas.name[0] = '\0';
		cas.buff[cas.count++] = value;
		if (cas.count == 9) {
			/* BASIC cassette ? */
			if (cas.buff[1] != 0x55 || cas.buff[8] != 0x3c)
				sprintf(cas.name, "basic%c.cas", cas.buff[1]);
			else
			/* SYSTEM cassette ? */
			if (cas.buff[1] == 0x55 && cas.buff[8] == 0x3c)
				sprintf(cas.name, "%-6.6s.cas", cas.buff + 2);
			else
				strcpy(cas.name, "unknown.cas");
			cas.put_img = img_fopen(cas.name, IMG_TYPE_CAS, "wb");
		}
	} else {
		cas.count++;
	}
}

/*******************************************************************
 * cas_put_close
 * Flush output buffer, and close virtual cassette output file.
 *******************************************************************/
static void cas_put_close(void)
{
	if (NULL != cas.put_img) {
		img_fclose(cas.put_img);
		cas.put_img = NULL;
	}
	cas.count = 0;
}

/*******************************************************************
 * cas_put_bit
 * port FF cassette status bit changed. Figure out what to do with it
 *******************************************************************/
static void cas_put_bit(void)
{
	tmr_time_t now = time_now();
	tmr_time_t diff = now - cas.put_time;
	tmr_time_t limit = TIMING_CONST * mem[0x4310] + 4 * mem[0x4311];

	/* remember the cycle count of this write */
	cas.put_time = now;
	LOG((LL,"CGENIE","diff:%lld limit:%lld\n", diff, limit));

	/* overrun since last write ? */
	if (diff >= 30000000ull) {
		/* reset cassette output */
		cas_put_close();
		cas.put_bitcnt = 0;
		cas.shiftreg = 0x0000;
		cas.in_sync = 0;
		return;
	}


	/* change within time for a 1 bit ? */
	if (diff < limit) {
		cas.shiftreg = (cas.shiftreg << 1) | 1;
		switch (cas.in_sync) {
			case 0: /* look for sync AA */
				if (0xcccc == (cas.shiftreg & 0xffff)) {
					LOG((LL,"CGENIE","cassette sync1 written"));
					cas.in_sync = 1;
				}
				break;
			case 1: /* look for sync 66 */
				if (0x3c3c == (cas.shiftreg & 0xffff)) {
					LOG((LL,"CGENIE","cassette sync2 written"));
					cas.put_bitcnt = 16;
					cas.in_sync = 2;
				}
				break;
			case 2: /* count 1 bit */
				cas.put_bitcnt += 1;
				break;
		}
	} else {
		/* no change within time indicates a 0 bit */
		cas.shiftreg <<= 2;
		switch (cas.in_sync) {
			case 0: /* look for sync AA */
				if (0xcccc == (cas.shiftreg & 0xffff)) {
					LOG((LL,"CGENIE","cassette sync1 written"));
					cas.in_sync = 1;
				}
				break;
			case 1: /* look for sync 66 */
				if (0x3c3c == (cas.shiftreg & 0xffff)) {
					LOG((LL,"CGENIE","cassette sync2 written"));
					cas.put_bitcnt = 16;
					cas.in_sync = 2;
				}
				break;
			case 2: /* count 2 bits */
				cas.put_bitcnt += 2;
				break;
		}
	}

	LOG((LL,"CGENIE","cassette %4d %4d %d bits %04X\n",
		diff, limit, cas.in_sync, cas.shiftreg & 0xffff));

	/* collected 8 sync plus 8 data bits ? */
	if (cas.put_bitcnt >= 16) {
		/* extract data bits to value */
		uint8_t data = 
			((cas.shiftreg >> (15-7)) & 0x80) |
			((cas.shiftreg >> (13-6)) & 0x40) |
			((cas.shiftreg >> (11-5)) & 0x20) |
			((cas.shiftreg >> ( 9-4)) & 0x10) |
			((cas.shiftreg >> ( 7-3)) & 0x08) |
			((cas.shiftreg >> ( 5-2)) & 0x04) |
			((cas.shiftreg >> ( 3-1)) & 0x02) |
			((cas.shiftreg >> ( 1-0)) & 0x01);
		cas.put_bitcnt -= 16;
		cas.shiftreg = 0x0000;
		cas_put_byte(data);
	}
}

/*******************************************************************
 * cas_get_byte
 * read next byte from input cassette image file.
 * the first 32 bytes are faked to be sync header AA.
 *******************************************************************/
static void cas_get_byte(void)
{
	uint8_t data;

	if (NULL == cas.get_img)
		return;

	if (cas.count < 256) {
		data = 0xaa;
		LOG((LL,"CGENIE","cassette read 0x%02x (sync1)\n", data));
	} else if (cas.offs < cas.size) {
		data = cas.buff[cas.offs];
		cas.offs++;
		LOG((LL,"CGENIE","cassette read 0x%02x '%s' (0x%x/0x%x)\n",
			data, cas.name, cas.offs, cas.size));
	} else {
		/* reading should have stopped... */
		data = 0x00;
		LOG((LL,"CGENIE","cassette read 0x%02x '%s' (beyond end)\n",
			data, cas.name));
	}

	cas.shiftreg = 0xaaaa;
	if (data & 0x80)
		cas.shiftreg ^= 0x4000;
	if (data & 0x40)
		cas.shiftreg ^= 0x1000;
	if (data & 0x20)
		cas.shiftreg ^= 0x0400;
	if (data & 0x10)
		cas.shiftreg ^= 0x0100;
	if (data & 0x08)
		cas.shiftreg ^= 0x0040;
	if (data & 0x04)
		cas.shiftreg ^= 0x0010;
	if (data & 0x02)
		cas.shiftreg ^= 0x0004;
	if (data & 0x01)
		cas.shiftreg ^= 0x0001;
	cas.get_bitcnt = 16;
	cas.count++;
}

/*******************************************************************
 * cas_get_open
 * Open a virtual cassette image file for input. Look for a
 * special header and skip leading description, if present.
 * The filename is taken from BASIC input buffer at 41E8 ff.
 *******************************************************************/
static void cas_get_open(void)
{
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

	LOG((LL,"CGENIE","cas_get_open '%s'\n", cas.name));
	cas.get_img = img_fopen(cas.name, IMG_TYPE_CAS, "rb");
	if (NULL == cas.get_img)
		return;

	LOG((LL,"CGENIE","cassette load from file '%s'\n", cas.name));

	cas.size = img_fread(cas.get_img, cas.buff, BUFF_SIZE);
	if (0 == strncmp((char *)cas.buff, CAS_HEADER, sizeof(CAS_HEADER) - 1)) {
		LOG((LL,"CGENIE","Virtual tape file header found\n"));
		cas.offs = strlen((char *)cas.buff) + 1;
	} else {
		/* seek back to start of cassette */
		cas.offs = 0;
	}
}


/*******************************************************************
 * cas_get_bit
 * Port FF cassette status read. Check for cassette input toggle
 *******************************************************************/
static void cas_get_bit(void)
{
	tmr_time_t now = time_now();
	tmr_time_t limit = TIMING_CONST * mem[0x4312];
	tmr_time_t diff = now - cas.get_time;

	/* remember the cycle count of this read */
	cas.get_time = now;

	LOG((LL,"CGENIE","bit_time:%lld diff:%lld limit:%lld\n",
		cas.bit_time, diff, limit));

	/* overrun since last read ? */
	if (diff >= 30000000ull) {
		if (NULL != cas.get_img) {
			img_fclose(cas.get_img);
			cas.get_img = NULL;
			LOG((LL,"CGENIE","cassette file closed\n"));
		}
		cas.get_bitcnt = 0;
		cas.shiftreg = 0x0000;
		cas.bit_time = 0;
		return;
	}

	/* count down now */
	cas.bit_time -= diff;

	/* time for the next sync or data bit ? */
	if (cas.bit_time <= 0) {
		/* approx time for a bit */
		cas.bit_time += limit;
		/* need to read get new data ? */
		cas.get_bitcnt--;
		if (cas.get_bitcnt <= 0) {
			cas_get_open();
			cas_get_byte();
		}
		/* shift next sync or data bit to bit 16 */
		cas.shiftreg <<= 1;
		if (cas.shiftreg & 0x10000)
			cas.get_bit ^= 1;
	}
}

void cgenie_cas_w(uint32_t offset, uint8_t data)
{
	cas_put_bit();
}

uint8_t cgenie_cas_r(uint32_t offset)
{
	cas_get_bit();

	return cas.get_bit;
}
