/* ed:set tabstop=8 noexpandtab: */
/**************************************************************************
 *
 * cas2xml.c	Colour Genie and TRS-80 cassette to XML converter
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sha1.h"

/**************************************************************************
 *
 * Cassette format for a TRS-80 BASIC tape
 *
 *  byte #      name value meaning
 * ========================================================================
 *      ??       nul  0x00 silence (e.g. 256 bytes)
 *       0      sync  0xa5 synchronization
 *     1-3     basic  0xd3 3 header bytes
 *       4  filename    ?? 1 character filename (usually A to Z)
 * ----------------------L-I-N-E---N-U-M-B-E-R---1-------------------------
 *       2  next_lsb    ?? next line address less significant byte
 *       3  next_msb    ?? next line address more significant byte 
 *       4  line_lsb    ?? line number less significant byte
 *       5  line_msb    ?? line number more significant byte
 *    6-??    tokens    ?? tokenized BASIC statements
 *      ??       nul    ?? 0x00 byte terminates the line
 * ----------------------L-I-N-E---N-U-M-B-E-R---2-------------------------
 *      ??  next_lsb    ?? next line address less significant byte
 *      ??  next_msb    ?? next line address more significant byte 
 *      ??  line_lsb    ?? line number less significant byte
 *      ??  line_msb    ?? line number more significant byte
 *   ??-??    tokens    ?? tokenized BASIC statements
 *      ??       nul    ?? 0x00 byte terminates the line
 * ----------------------L-I-N-E---N-U-M-B-E-R---4-------------------------
 *                         ...
 * ----------------------E-N-D---B-L-O-C-K---------------------------------
 *      ??  next_lsb    ?? 0x00
 *      ??  next_msb    ?? 0x00
 * ========================================================================
 *
 * + The filename is a single character, usually uppercase A to Z
 * + The next line address is adjusted when loading a BASIC program
 *   to a different base address (depending on memory configuration).
 * + Line numbers are usually stricly incremental in the range between
 *   1 and 65534. 65535 is a special value for the immediate command
 *   input and should not appear as a line number.
 * + There is _no_ checksum whatsoever in BASIC tape format
 * + Any data byte following after the end block is ignored
 *
 **************************************************************************/

/**************************************************************************
 *
 * Cassette format for a TRS-80 SYSTEM tape
 *
 *  byte #      name value meaning
 * ========================================================================
 *      ??       nul  0x00 silence (e.g. 256 bytes)
 *       0      sync  0xa5 synchronization
 *       1    header  0x55 system tape marker
 *     2-7  filename    ?? 6 character filename (usually alphanumeric)
 * ----------------------D-A-T-A-B-L-O-C-K-1-------------------------------
 *       8  datamark  0x3c data block marker
 *       9     count    ?? block length in bytes (0x00 means 0x100)
 *      10  addr_lsb    ?? load address less significant byte
 *      11  addr_msb    ?? load address more significant byte
 *   12-??      data    ?? 1 to 256 bytes of data
 *      ??      csum    ?? 1 byte checksum of addr_lsb, addr_msb and data
 * ----------------------D-A-T-A-B-L-O-C-K-2-------------------------------
 *      ??  datamark  0x3c data block marker
 *      ??     count    ?? block length in bytes (0x00 means 0x100)
 *      ??  addr_lsb    ?? load address less significant byte
 *      ??  addr_msb    ?? load address more significant byte
 *   ??-??      data    ?? 1 to 256 bytes of data
 *      ??      csum    ?? 1 byte checksum of addr_lsb, addr_msb and data
 * ----------------------D-A-T-A-B-L-O-C-K-3-------------------------------
 *                         ...
 * ----------------------E-N-T-R-Y-B-L-O-C-K-------------------------------
 *      ??  datamark  0x78 data block marker
 *      ??  addr_lsb    ?? entry address less significant byte
 *      ??  addr_msb    ?? entry address more significant byte
 * ========================================================================
 *
 * + The filename is padded to 6 characters with blanks (0x20)
 * + At least one data block should be there.
 * + There is just one entry block and any data beyond it is not loaded
 * + Some images write to special vectors (e.g. 0x41e2) and have an
 *   entry address of 0x0000; the warm boot will then jump through the
 *   modified vector to the start of the machine code program.
 *
 **************************************************************************/

/**************************************************************************
 *
 * Cassette format for a Colour Genie EG2000 BASIC tape
 *
 *  byte #      name value meaning
 * ========================================================================
 *       0      sync  0x66 synchronization
 *       1  filename    ?? 1 character filename (usually A to Z)
 * ----------------------L-I-N-E---N-U-M-B-E-R---1-------------------------
 *       2  next_lsb    ?? next line address less significant byte
 *       3  next_msb    ?? next line address more significant byte 
 *       4  line_lsb    ?? line number less significant byte
 *       5  line_msb    ?? line number more significant byte
 *    6-??    tokens    ?? tokenized BASIC statements
 *      ??       nul    ?? 0x00 byte terminates the line
 * ----------------------L-I-N-E---N-U-M-B-E-R---2-------------------------
 *      ??  next_lsb    ?? next line address less significant byte
 *      ??  next_msb    ?? next line address more significant byte 
 *      ??  line_lsb    ?? line number less significant byte
 *      ??  line_msb    ?? line number more significant byte
 *   ??-??    tokens    ?? tokenized BASIC statements
 *      ??       nul    ?? 0x00 byte terminates the line
 * ----------------------L-I-N-E---N-U-M-B-E-R---4-------------------------
 *                         ...
 * ----------------------E-N-D---B-L-O-C-K---------------------------------
 *      ??  next_lsb    ?? 0x00
 *      ??  next_msb    ?? 0x00
 * ========================================================================
 *
 * + The filename is a single character, usually uppercase A to Z
 * + The next line address is adjusted when loading a BASIC program
 *   to a different base address (depending on memory configuration).
 * + Line numbers are usually stricly incremental in the range between
 *   1 and 65534. 65535 is a special value for the immediate command
 *   input and should not appear as a line number.
 * + There is _no_ checksum whatsoever in BASIC tape format
 * + Any data byte following after the end block is ignored
 *
 **************************************************************************/

/**************************************************************************
 *
 * Cassette format for a Colour Genie EG2000 SYSTEM tape
 *
 *  byte #      name value meaning
 * ========================================================================
 *       0      sync  0x66 synchronization
 *       1    header  0x55 system tape marker
 *     2-7  filename    ?? 6 character filename (usually alphanumeric)
 * ----------------------D-A-T-A-B-L-O-C-K-1-------------------------------
 *       8  datamark  0x3c data block marker
 *       9     count    ?? block length in bytes (0x00 means 0x100)
 *      10  addr_lsb    ?? load address less significant byte
 *      11  addr_msb    ?? load address more significant byte
 *   12-??      data    ?? 1 to 256 bytes of data
 *      ??      csum    ?? 1 byte checksum of addr_lsb, addr_msb and data
 * ----------------------D-A-T-A-B-L-O-C-K-2-------------------------------
 *      ??  datamark  0x3c data block marker
 *      ??     count    ?? block length in bytes (0x00 means 0x100)
 *      ??  addr_lsb    ?? load address less significant byte
 *      ??  addr_msb    ?? load address more significant byte
 *   ??-??      data    ?? 1 to 256 bytes of data
 *      ??      csum    ?? 1 byte checksum of addr_lsb, addr_msb and data
 * ----------------------D-A-T-A-B-L-O-C-K-3-------------------------------
 *                         ...
 * ----------------------E-N-T-R-Y-B-L-O-C-K-------------------------------
 *      ??  datamark  0x78 data block marker
 *      ??  addr_lsb    ?? entry address less significant byte
 *      ??  addr_msb    ?? entry address more significant byte
 * ========================================================================
 *
 * + The filename is padded to 6 characters with blanks (0x20)
 * + At least one data block should be there.
 * + There is just one entry block and any data beyond it is not loaded
 * + Some images write to special vectors (e.g. 0x41e2) and have an
 *   entry address of 0x0000; the warm boot will then jump through the
 *   modified vector to the start of the machine code program.
 *
 **************************************************************************/


/** @brief machine type */
typedef enum {
	MACH_TRS80,
	MACH_EG2000
}	machine_t;

/** @brief block type */
typedef enum {
	T_BASIC,
	T_SYSTEM,
	T_ENTRY,
	T_RAW
}	block_type_t;

/** @brief decoder status */
typedef enum {
	ST_COMMENT,
	ST_EOF,
	ST_SILENCE,
	ST_NUL,
	ST_HEADER,
	ST_SYSTEM_BLOCKTYPE,
	ST_SYSTEM_COUNT,
	ST_SYSTEM_ADDR_LSB,
	ST_SYSTEM_ADDR_MSB,
	ST_SYSTEM_DATA,
	ST_SYSTEM_CSUM,
	ST_SYSTEM_ENTRY_LSB,
	ST_SYSTEM_ENTRY_MSB,
	ST_BASIC_ADDR_LSB,
	ST_BASIC_ADDR_MSB,
	ST_BASIC_LINE_LSB,
	ST_BASIC_LINE_MSB,
	ST_BASIC_DATA,
	ST_AFTER_ENTRY,
	ST_IGNORE
}	status_t;

typedef struct {
	uint8_t type;
	uint8_t csum;
	uint16_t addr;
	uint16_t size;
	uint16_t line;
	uint8_t *data;
}	block_t;

#define	CAS_TRS80_SYNC		0xa5
#define	CAS_TRS80_BASIC_HEADER	0xd3
#define	CAS_CGENIE_SYNC		0x66
#define	CAS_SYSTEM_HEADER	0x55
#define	CAS_SYSTEM_DATA		0x3c
#define	CAS_SYSTEM_ENTRY	0x78

machine_t cas_machine;
uint8_t cas_sync = 0;
char *cas_name = NULL;
char *cas_author = NULL;
char *cas_copyright = NULL;
char *cas_description = NULL;
char cas_filename[6+1] = "";

uint16_t cas_addr = 0;
uint16_t cas_line = 0;
uint16_t cas_entry = 0;
uint16_t cas_size = 0;
uint8_t cas_prefix = 0;
uint8_t cas_csum = 0;
uint8_t cas_basic = 0;

sha1_state_t cas_sha1;
sha1_digest_t cas_digest;

block_t *blocks = NULL;
uint32_t nalloc = 0;
uint32_t nblocks = 0;

int verbose = 0;

/** @brief TRS-80 and Colour genie shared tokens */
static const char *token[128+26+1] = {
	"END","FOR","RESET","SET","CLS","CMD","RANDOM","NEXT",
	"DATA","INPUT","DIM","READ","LET","GOTO","RUN","IF",
	"RESTORE","GOSUB","RETURN","REM","STOP","ELSE","TRON","TROFF",
	"DEFSTR","DEFINT","DEFSNG","DEFDBL","LINE","EDIT","ERROR","RESUME",
	"OUT","ON","OPEN","FIELD","GET","PUT","CLOSE","LOAD",
	"MERGE","NAME","KILL","LSET","RSET","SAVE","SYSTEM","LPRINT",
	"DEF","POKE","PRINT","CONT","LIST","LLIST","DELETE","AUTO",
	"CLEAR","CLOAD","CSAVE","NEW","TAB(","TO","FN", "USING",
	"VARPTR","USR","ERL","ERR","STRING$","INSTR","CHECK","TIME$",
	"MEM","INKEY$","THEN","NOT","STEP","+","-","*",
	"/","[","AND","OR",">","=","<","SGN",
	"INT","ABS","FRE","INP","POS","SQR","RND","LOG",
	"EXP","COS","SIN","TAN","ATN","PEEK","CVI","CVS",
	"CVD","EOF","LOC","LOF","MKI$","MKS$","MKD$","CINT",
	"CSNG","CDBL","FIX","LEN","STR$","VAL","ASC","CHR$",
	"LEFT$","RIGHT$","MID$","'","\\374","\\375","\\376","\\377",
	/* Colour Genie specific tokens (after 0xff) */
	"COLOUR","FCOLOU",/* (sic!) */
	"KEYPAD","JOY","PLOT","FGR","LGR","FCLS",
	"PLAY","CIRCLE","SCALE","SHAPE","NSHAPE","XSHAPE","PAINT","CPOINT",
	"NPLOT","SOUND","CHAR","RENUM","SWAP","FKEY","CALL","VERIFY",
	"BGRD","NBGRD", NULL
};

/**
 * @brief decode a tokenized memory range into plain text
 *
 * @brief src pointer to the buffer conatining tokenized text
 * @brief maxsize maximum number of bytes to detokenize (NUL-byte marks the end, too)
 */
const char *detokenize(uint8_t *src, int maxsize)
{
	static char buff[8192];
	char *dst = buff;
	int string = 0;

	while (0x00 != *src && maxsize-- > 0) {
		if (string) {
			if ('"' == *src)
				string = 0;
			switch (*src) {
			case '&':
				dst += sprintf(dst, "&amp;");
				break;
			case '<':
				dst += sprintf(dst, "&lt;");
				break;
			case '>':
				dst += sprintf(dst, "&gt;");
				break;
			default:
				if (*src >= 32 && *src < 127)
					*dst++ = *src;
				else
					dst += sprintf(dst, "\\%03o", *src);
				break;
			}
		} else {
			if (0xff == *src) {
				src++;
				if (*src < 0x80 + 26)
					dst += sprintf(dst, "%s", token[*src]);
				else
					dst += sprintf(dst, "\\377\\%03o", *src);
			} else if (*src > 0x7f) {
				if (0 == strcmp(token[*src & 0x7f], "&"))
					dst += sprintf(dst, "&amp;");
				else if (0 == strcmp(token[*src & 0x7f], "<"))
					dst += sprintf(dst, "&lt;");
				else if (0 == strcmp(token[*src & 0x7f], ">"))
					dst += sprintf(dst, "&gt;");
				else
					dst += sprintf(dst, "%s", token[*src & 0x7f]);
			} else {
				switch (*src) {
				case '&':
					dst += sprintf(dst, "&amp;");
					break;
				case '<':
					dst += sprintf(dst, "&lt;");
					break;
				case '>':
					dst += sprintf(dst, "&gt;");
					break;
				default:
					if (*src >= 32 && *src < 127)
						*dst++ = *src;
					else
						dst += sprintf(dst, "\\%03o", *src);
					break;
				}
				if ('"' == *src)
					string = 1;
			}
		}
		src++;
	}
	*dst = '\0';
	return buff;
}

block_t *blocks_alloc(void)
{
	if (nalloc <= nblocks) {
		nalloc = nalloc ? nalloc * 2 : 4;
		blocks = realloc(blocks, nalloc * sizeof(block_t));
		if (NULL == blocks) {
			perror("realloc()");
			exit(1);
		}
		memset(&blocks[nblocks], 0, (nalloc - nblocks) * sizeof(block_t));
	}
	return &blocks[nblocks];
}

int cas_read_image(const char *filename)
{
	uint8_t buff[1024], *src;
	FILE *fp;
	block_t *b;
	status_t status;
	int in, pos, count;
	uint8_t ch;

	printf("===> %s\n", filename);

	fp = fopen(filename, "rb");
	if (NULL == fp)
		return -1;

	memset(buff, 0, sizeof(buff));
	fread(buff, 1, sizeof(buff), fp);
	sha1_init(&cas_sha1);

	src = memchr((char *)buff, CAS_TRS80_SYNC, sizeof(buff));
	if (0 == strncmp((char *)buff, "Colour Genie - Virtual Tape File", 32)) {
		fseek(fp, 32, SEEK_SET);
		status = ST_COMMENT;
		cas_machine = MACH_EG2000;
	} else if (NULL != src) {
		count = (int)(src - buff);
		for (pos = 0; pos < count; pos++)
			if (0x00 != buff[pos])
				break;
		if (pos < count) {
			/* probably Colour Genie SYSTEM or BASIC including an 0xA5 token */
			fseek(fp, 0, SEEK_SET);
			status = ST_NUL;
			cas_machine = MACH_EG2000;
		} else {
			/* TRS-80 silence header + sync byte */
			fseek(fp, count, SEEK_SET);
			status = ST_SILENCE;
			cas_machine = MACH_TRS80;
		}
	} else {
		/* read from the beginning */
		fseek(fp, 0, SEEK_SET);
		status = ST_NUL;
		cas_machine = MACH_EG2000;
	}

	pos = 0;
	count = 0;
	b = NULL;

	while (-1 != (in = fgetc(fp))) {
		ch = (uint8_t)in;
		switch (status) {
		case ST_COMMENT:
			switch (ch) {
			case 13:	/* carriage return */
				pos = 0;
				break;
			case 10:	/* line feed */
				buff[pos] = '\0';
				if (!strncmp((char *)buff, "Name       :", 12)) {
					cas_name = strdup((char *)buff + 13);
					break;
				}
				if (!strncmp((char *)buff, "Author     :", 12)) {
					cas_author = strdup((char *)buff + 13);
					break;
				}
				if (!strncmp((char *)buff, "Copyright  :", 12)) {
					cas_copyright = strdup((char *)buff + 13);
					break;
				}
				if (!strncmp((char *)buff, "Description:", 12)) {
					cas_description = strdup((char *)buff + 13);
					break;
				}
				src = buff;
				while (*src == ' ')
					src++;
				if (cas_description) {
					int append = strlen(cas_description) + 2 + strlen((char *)src) + 1;
					cas_description = realloc(cas_description, append);
					if (NULL == cas_description) {
						perror("realloc()");
						exit(1);
					}
					strcat(cas_description, "\r\n");
					strcat(cas_description, (char *)src);
				}
				pos = 0;
				break;
			case 26:	/* Ctrl-Z (text EOF) */
				status = ST_EOF;
				break;
			default:
				buff[pos++] = ch;
				if (pos == sizeof(buff))
					pos--;
			}
			break;

		case ST_EOF:
			switch (ch) {
			case 0x00:
				status = ST_NUL;
				break;
			default:
				fprintf(stderr, "EOF: unexpected %d (0x%x)\n", ch, ch);
				break;
			}
			break;

		case ST_SILENCE:
			if (0x00 == ch)
				break;
			status = ST_NUL;
			/* FALLTHROUGH */

		case ST_NUL:
			switch (ch) {
			case CAS_TRS80_SYNC:
				cas_sync = ch;
				sha1_append(&cas_sha1, &ch, 1);
				status = ST_HEADER;
				pos = 0;
				count = 8;
				break;
			case CAS_CGENIE_SYNC:
				cas_sync = ch;
				sha1_append(&cas_sha1, &ch, 1);
				status = ST_HEADER;
				pos = 0;
				count = 8;
				break;
			default:
				fprintf(stderr, "NUL: unexpected %d (0x%x)\n", ch, ch);
				break;
			}
			break;

		case ST_HEADER:
			buff[pos] = ch;
			pos++;
			count--;
			if (count)
				break;
			if (verbose > 1)
				printf("HEADER: %02x %02x %02x %02x %02x %02x %02x %02x\n",
					(uint8_t)buff[0], (uint8_t)buff[1],
					(uint8_t)buff[2], (uint8_t)buff[3],
					(uint8_t)buff[4], (uint8_t)buff[5],
					(uint8_t)buff[6], (uint8_t)buff[7]);
			/* look for TRS-80 or Colour Genie SYSTEM tape format */
			if (buff[0] == CAS_SYSTEM_HEADER && buff[7] == CAS_SYSTEM_DATA) {
				snprintf(cas_filename, sizeof(cas_filename), "%.6s", buff + 1);
				if (verbose)
					printf("SYSTEM tape: '%s'\n", cas_filename);
				status = ST_SYSTEM_COUNT;
				sha1_append(&cas_sha1, buff, 8);
				cas_prefix = buff[0];
				cas_basic = 0;
			} else if (buff[0] == CAS_TRS80_BASIC_HEADER &&
				buff[1] == CAS_TRS80_BASIC_HEADER &&
				buff[2] == CAS_TRS80_BASIC_HEADER) {
				snprintf(cas_filename, sizeof(cas_filename), "%c", buff[3]);
				if (verbose)
					printf("TRS-80 BASIC tape: '%s'\n", cas_filename);
				fseek(fp, -4, SEEK_CUR);
				status = ST_BASIC_ADDR_LSB;
				sha1_append(&cas_sha1, buff, 1);
				cas_prefix = 0;
				cas_basic = 1;
			} else {
				snprintf(cas_filename, sizeof(cas_filename), "%c", buff[0]);
				if (verbose)
					printf("Colour Genie BASIC tape: '%s'\n", cas_filename);
				fseek(fp, -7, SEEK_CUR);
				status = ST_BASIC_ADDR_LSB;
				sha1_append(&cas_sha1, buff, 1);
				cas_prefix = 0;
				cas_basic = 1;
			}
			break;

		case ST_SYSTEM_BLOCKTYPE:
			sha1_append(&cas_sha1, &ch, 1);
			switch (ch) {
			case CAS_SYSTEM_DATA:
				status = ST_SYSTEM_COUNT;
				break;
			case CAS_SYSTEM_ENTRY:
				status = ST_SYSTEM_ENTRY_LSB;
				break;
			default:
				fprintf(stderr, "SYSTEM_BLOCKTYPE: unexpected %d (0x%x)\n", ch, ch);
				break;
			}
			break;

		case ST_SYSTEM_COUNT:
			sha1_append(&cas_sha1, &ch, 1);
			count = ch;
			if (0 == count)
				count = 256;
			cas_size = count;
			status = ST_SYSTEM_ADDR_LSB;
			break;

		case ST_SYSTEM_ADDR_LSB:
			sha1_append(&cas_sha1, &ch, 1);
			cas_addr = ch;
			cas_csum = ch;
			status = ST_SYSTEM_ADDR_MSB;
			break;

		case ST_SYSTEM_ADDR_MSB:
			sha1_append(&cas_sha1, &ch, 1);
			cas_addr = cas_addr + 256 * ch;
			cas_csum = cas_csum + ch;
			status = ST_SYSTEM_DATA;
			if (verbose > 1)
				printf("SYSTEM data block: %u (0x%x) at 0x%04x\n",
					count, count, cas_addr);
			b = blocks_alloc();
			b->type = T_SYSTEM;
			b->addr = cas_addr;
			b->size = cas_size;
			b->data = malloc(cas_size);
			pos = 0;
			break;

		case ST_SYSTEM_DATA:
			sha1_append(&cas_sha1, &ch, 1);
			b->data[pos++] = ch;
			cas_csum = cas_csum + ch;
			if (0 == --count) {
				status = ST_SYSTEM_CSUM;
			}
			break;

		case ST_SYSTEM_CSUM:
			sha1_append(&cas_sha1, &ch, 1);
			if (ch != cas_csum) {
				fprintf(stderr, "SYSTEM_CSUM: block #%u checksum error (found:0x%02x calc:0x%02x)\n",
					nblocks, ch, cas_csum);
				cas_csum = ch;
			}
			status = ST_SYSTEM_BLOCKTYPE;
			b->csum = cas_csum;
			b = NULL;
			nblocks++;
			break;

		case ST_SYSTEM_ENTRY_LSB:
			sha1_append(&cas_sha1, &ch, 1);
			cas_entry = ch;
			status = ST_SYSTEM_ENTRY_MSB;
			break;

		case ST_SYSTEM_ENTRY_MSB:
			sha1_append(&cas_sha1, &ch, 1);
			cas_entry = cas_entry + 256 * ch;
			b = blocks_alloc();
			b->type = T_ENTRY;
			b->addr = cas_entry;
			b->size = 0;
			b = NULL;
			nblocks++;
			if (verbose)
				printf("SYSTEM entry point: %u (0x%04x)\n",
					cas_entry, cas_entry);
			status = ST_AFTER_ENTRY;
			b = blocks_alloc();
			b->type = T_RAW;
			b->size = 0;
			b->data = malloc(1024);
			break;

		case ST_BASIC_ADDR_LSB:
			sha1_append(&cas_sha1, &ch, 1);
			cas_addr = ch;
			status = ST_BASIC_ADDR_MSB;
			break;

		case ST_BASIC_ADDR_MSB:
			sha1_append(&cas_sha1, &ch, 1);
			cas_addr = cas_addr + 256 * ch;
			if (0 == cas_addr) {
				if (verbose > 1)
					printf("BASIC end: 0x%x\n", pos);
				status = ST_IGNORE;
				b = blocks_alloc();
				b->type = T_RAW;
				b->size = 0;
				b->data = malloc(1024);
			} else {
				pos = 0;
				status = ST_BASIC_LINE_LSB;
			}
			break;

		case ST_BASIC_LINE_LSB:
			sha1_append(&cas_sha1, &ch, 1);
			cas_line = ch;
			status = ST_BASIC_LINE_MSB;
			break;

		case ST_BASIC_LINE_MSB:
			sha1_append(&cas_sha1, &ch, 1);
			cas_line = cas_line + 256 * ch;
			status = ST_BASIC_DATA;
			pos = 0;
			b = blocks_alloc();
			b->type = T_BASIC;
			b->line = cas_line;
			b->addr = cas_addr;
			b->size = 1024;
			b->data = malloc(1024);
			if (verbose > 1)
				printf("BASIC line: #%u at 0x%x\n", cas_line, cas_addr);
			break;

		case ST_BASIC_DATA:
			sha1_append(&cas_sha1, &ch, 1);
			b->data[pos] = ch;
			if (++pos == 1024)
				pos = 1023;
			if (0 == ch) {
				cas_size = pos;
				b->size = cas_size;
				b->data = realloc(b->data, cas_size);
				b = NULL;
				nblocks++;
				status = ST_BASIC_ADDR_LSB;
			}
			break;

		case ST_AFTER_ENTRY:
			if (ch == CAS_SYSTEM_DATA) {
				/* another data block after the system entry */
				sha1_append(&cas_sha1, &ch, 1);
				status = ST_SYSTEM_COUNT;
				break;
			}
			if (ch == CAS_SYSTEM_ENTRY) {
				/* another entry block after the system entry */
				sha1_append(&cas_sha1, &ch, 1);
				status = ST_SYSTEM_ENTRY_LSB;
				break;
			}
			status = ST_IGNORE;
			/* FALLTHROUGH */

		case ST_IGNORE:
			/* do not append ch to image SHA1 */
			b = blocks_alloc();
			if (0 == (b->size % 1024)) {
				b->data = realloc(b->data, b->size + 1024);
				if (NULL == b->data) {
					perror("realloc()");
					exit(1);
				}
			}
			b->data[b->size] = ch;
			b->size += 1;
		}
	}
	if (NULL != b) {
		if (0 == b->size) {
			free(b->data);
			b->data = NULL;
		} else {
			/* last (extra data) block */
			nblocks++;
		}
	}
	sha1_finish(&cas_sha1, &cas_digest);
	fclose(fp);
	return 0;
}

int cas_write_xml(const char *filename)
{
	char xmlname[FILENAME_MAX], *ext;
	FILE *fp;
	uint32_t i, n;
	uint32_t size;

	snprintf(xmlname, sizeof(xmlname), "%s", filename);
	ext = strrchr(xmlname, '.');
	if (!strcasecmp(ext, ".cas")) {
		/* replace existing .cas extension with .xml */
		sprintf(ext, ".xml");
	} else {
		/* or otherwise append .xml to the filename */
		snprintf(xmlname, sizeof(xmlname), "%s.xml", filename);
	}
	fp = fopen(xmlname, "w");
	if (NULL == fp)
		return -1;	

	fprintf(fp, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");

	fprintf(fp, "<!DOCTYPE cassette [\n");
	fprintf(fp, "	<!ATTLIST cassette sha1 CDATA #REQUIRED>\n");
	fprintf(fp, "	<!ELEMENT cassette (machine, format, name?, author?,"
		" copyright?, description?, sync, prefix?, blocks, image?, decoded?)>\n");
	fprintf(fp, "		<!ELEMENT machine (trs80|eg2000)>\n");
	fprintf(fp, "		<!ELEMENT format (basic|system)>\n");
	fprintf(fp, "		<!ELEMENT name (#PCDATA)>\n");
	fprintf(fp, "		<!ELEMENT author (#PCDATA)>\n");
	fprintf(fp, "		<!ELEMENT copyright (#PCDATA)>\n");
	fprintf(fp, "		<!ELEMENT description (#PCDATA)>\n");
	fprintf(fp, "		<!ELEMENT sync (#PCDATA)>\n");
	fprintf(fp, "		<!ELEMENT prefix (#PCDATA)>\n");
	fprintf(fp, "		<!ELEMENT blocks (block*)>\n");
	fprintf(fp, "			<!ATTLIST blocks count CDATA #REQUIRED>\n");
	fprintf(fp, "			<!ELEMENT block (#PCDATA)>\n");
	fprintf(fp, "				<!ATTLIST block number CDATA #REQUIRED>\n");
	fprintf(fp, "				<!ATTLIST block type (basic|system|entry|raw) #REQUIRED>\n");
	fprintf(fp, "				<!ATTLIST block address CDATA #REQUIRED>\n");
	fprintf(fp, "				<!ATTLIST block size CDATA #IMPLIED>\n");
	fprintf(fp, "				<!ATTLIST block line CDATA #IMPLIED>\n");
	fprintf(fp, "				<!ATTLIST block csum CDATA #IMPLIED>\n");
	fprintf(fp, "				<!ATTLIST block sha1 CDATA #IMPLIED>\n");
	fprintf(fp, "		<!ELEMENT image (#PCDATA)>\n");
	fprintf(fp, "			<!ATTLIST image size CDATA #REQUIRED>\n");
	fprintf(fp, "			<!ATTLIST image sha1 CDATA #IMPLIED>\n");
	fprintf(fp, "		<!ELEMENT decoded (#PCDATA)>\n");
	fprintf(fp, "]>\n");

	fprintf(fp, "<cassette sha1=\"%s\">\n", sha1_hexstr(&cas_digest));
	fprintf(fp, "\t<machine>%s</machine>\n", cas_machine == MACH_EG2000 ? "eg2000" : "trs80");
	fprintf(fp, "\t<format>%s</format>\n", cas_basic ? "basic" : "system");
	fprintf(fp, "\t<name>%s</name>\n", cas_name ? cas_name : "");
	fprintf(fp, "\t<author>%s</author>\n", cas_author ? cas_author : "");
	fprintf(fp, "\t<copyright>%s</copyright>\n", cas_copyright ? cas_copyright : "");
	fprintf(fp, "\t<description>%s</description>\n", cas_description ? cas_description : "");
	fprintf(fp, "\t<sync>0x%x</sync>\n", cas_sync);
	if (0 == cas_basic)
		fprintf(fp, "\t<prefix>0x%x</prefix>\n", CAS_SYSTEM_HEADER);
	fprintf(fp, "\t<filename>%s</filename>\n", cas_filename);
	fprintf(fp, "\t<blocks count=\"%u\">\n", nblocks);
	for (i = 0; i < nblocks; i++) {
		block_t *b = &blocks[i];
		sha1_state_t sha1;
		sha1_digest_t digest;

		sha1_init(&sha1);
		if (b->size && b->data)
			sha1_append(&sha1, b->data, b->size);
		sha1_finish(&sha1, &digest);
		fprintf(fp, "\t\t<block number=\"%u\"", i);
		switch (b->type) {
		case T_BASIC:
			fprintf(fp, " type=\"%s\" address=\"0x%04x\" size=\"0x%x\" line=\"%u\" sha1=\"%s\">",
				"basic", b->addr, b->size, b->line, sha1_hexstr(&digest));
			break;
		case T_SYSTEM:
			fprintf(fp, " type=\"%s\" address=\"0x%04x\" size=\"0x%x\" csum=\"0x%02x\" sha1=\"%s\">",
				"system", b->addr, b->size, b->csum, sha1_hexstr(&digest));
			break;
		case T_ENTRY:
			fprintf(fp, " type=\"%s\" address=\"0x%04x\">",
				"entry", b->addr);
			break;
		case T_RAW:
			fprintf(fp, " type=\"%s\" size=\"0x%x\" sha1=\"%s\">",
				"raw", b->size, sha1_hexstr(&digest));
			break;
		}
		if (b->data) {
			for (n = 0; n < b->size; n++)
				fprintf(fp, "%02x", b->data[n]);
		}
		fprintf(fp, "</block>\n");
	}

	fprintf(fp, "\t</blocks>\n");

	if (cas_basic) {
		size =	1 + 1;		/* sync + filename */
		for (i = 0; i < nblocks; i++) {
			block_t *b = &blocks[i];
			switch (b->type) {
			case T_BASIC:
				if (b->addr)
					size += 2 + 2 + b->size;	/* next address, line number, data <00> */
				else
					size += 2;			/* next address NULL */
				break;
			}
		}
	} else {
		size =	1 + 1 + 6;	/* CAS_SYNC + CAS_SYSTEM_HEADER + 6 character filename */
		for (i = 0; i < nblocks; i++) {
			block_t *b = &blocks[i];
			switch (b->type) {
			case T_SYSTEM:
				size += 1 + 1 + 2 + b->size + 1;/* block type, length, address, data, csum */
				break;
			case T_ENTRY:
				size += 1 + 2;			/* block type, address */
				break;
			case T_RAW:
				size += b->size;		/* raw data */
			}
		}
	}

	fprintf(fp, "\t<image size=\"0x%x\">", size);
	if (cas_basic) {
		fprintf(fp,"%02x", cas_sync);
		fprintf(fp,"%02x", cas_filename[0]);
		for (i = 0; i < nblocks; i++) {
			block_t *b = &blocks[i];
			switch (b->type) {
			case T_BASIC:
				if (b->addr) {
					fprintf(fp, "%02x", b->addr % 256);
					fprintf(fp, "%02x", b->addr / 256);
					fprintf(fp, "%02x", b->line % 256);
					fprintf(fp, "%02x", b->line / 256);
					for (n = 0; n < b->size; n++)
						fprintf(fp, "%02x", b->data[n]);
				} else {
					fprintf(fp, "0000");
				}
				break;
			}
		}
	} else {
		fprintf(fp,"%02x", cas_sync);
		fprintf(fp,"%02x", CAS_SYSTEM_HEADER);
		for (n = 0; n < 6; n++)
			fprintf(fp,"%02x", cas_filename[n]);
		for (i = 0; i < nblocks; i++) {
			block_t *b = &blocks[i];
			switch (b->type) {
			case T_SYSTEM:
				fprintf(fp, "%02x", CAS_SYSTEM_DATA);
				fprintf(fp, "%02x", (uint8_t)b->size);
				fprintf(fp, "%02x", b->addr % 256);
				fprintf(fp, "%02x", b->addr / 256);
				for (n = 0; n < b->size; n++)
					fprintf(fp, "%02x", b->data[n]);
				fprintf(fp, "%02x", b->csum);
				break;
			case T_ENTRY:
				fprintf(fp, "%02x", CAS_SYSTEM_ENTRY);
				fprintf(fp, "%02x", b->addr % 256);
				fprintf(fp, "%02x", b->addr / 256);
				break;
			case T_RAW:
				size += b->size;		/* raw data */
				for (n = 0; n < b->size; n++)
					fprintf(fp, "%02x", b->data[n]);
			}
		}
	}
	fprintf(fp, "</image>\n");

	if (cas_basic) {
		fprintf(fp, "\t<decoded>\n");
		for (i = 0; i < nblocks; i++) {
			block_t *b = &blocks[i];
			switch (b->type) {
			case T_BASIC:
				if (0 == b->addr)
					break;
				fprintf(fp, "%u %s\n", b->line, detokenize(b->data, b->size));
				break;
			}
		}
		fprintf(fp, "\t</decoded>\n");
	}

	fprintf(fp, "</cassette>\n");
	fclose(fp);

	return 0;
}

int main(int argc, char **argv)
{
	sha1_state_t sha1;
	sha1_digest_t digest;
	int i, rc;

	sha1_init(&sha1);
	sha1_append(&sha1, "abc", 3);
	sha1_finish(&sha1, &digest);
	if (strcmp(sha1_hexstr(&digest), "a9993e364706816aba3e25717850c26c9cd0d89d")) {
		fprintf(stderr, "Internal SHA1 test #1 failed (%s)\n", sha1_hexstr(&digest));
		return 1;
	}

	sha1_init(&sha1);
	sha1_append(&sha1, "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56);
	sha1_finish(&sha1, &digest);
	if (strcmp(sha1_hexstr(&digest), "84983e441c3bd26ebaae4aa1f95129e5e54670f1")) {
		fprintf(stderr, "Internal SHA1 test #2 failed (%s)\n", sha1_hexstr(&digest));
		return 1;
	}

	sha1_init(&sha1);
	for (i = 0; i < 1000000; i++)
		sha1_append(&sha1, "a", 1);
	sha1_finish(&sha1, &digest);
	if (strcmp(sha1_hexstr(&digest), "34aa973cd4c4daa4f61eeb2bdbad27316534016f")) {
		fprintf(stderr, "Internal SHA1 test #3 failed (%s)\n", sha1_hexstr(&digest));
		return 1;
	}

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-v")) {
			verbose++;
			continue;
		}
		if (NULL != cas_name) {
			free(cas_name);
			cas_name = NULL;
		}
		if (NULL != cas_author) {
			free(cas_author);
			cas_author = NULL;
		}
		if (NULL != cas_copyright) {
			free(cas_copyright);
			cas_copyright = NULL;
		}
		if (NULL != cas_description) {
			free(cas_description);
			cas_description = NULL;
		}
		memset(cas_filename, 0, sizeof(cas_filename));
		cas_sync = 0;
		cas_addr = 0;
		cas_line = 0;
		cas_entry = 0;
		cas_prefix = 0;
		cas_csum = 0;
		if (blocks) {
			while (nblocks > 0) {
				block_t *b = &blocks[--nblocks];
				if (b->data)
					free(b->data);
			}
			free(blocks);
			blocks = NULL;
		}
		nblocks = nalloc = 0;

		rc = cas_read_image(argv[i]);
		if (rc < 0)
			continue;
		rc = cas_write_xml(argv[i]);
		if (rc < 0)
			break;
	}
	return 0;
}
