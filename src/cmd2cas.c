/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * cmd2cas.c	Colour Genie command file to cassette converter
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#include "system.h"

#define	CAS_SYNC		0x66
#define	CAS_SYSTEM_HEADER	0x55
#define	CAS_SYSTEM_DATA		0x3c
#define	CAS_SYSTEM_ENTRY	0x78

typedef enum {
	ST_BLOCK_TYPE,
	ST_DATA_SIZE,
	ST_DATA_LSB,
	ST_DATA_MSB,
	ST_DATA,
	ST_ENTRY_SIZE,
	ST_ENTRY_LSB,
	ST_ENTRY_MSB,
	ST_COMMENT_SIZE,
	ST_COMMENT
}	state_t;

/**
 * @brief offset loader machine code
 *
 * This is the offset loader appended to the original machine code
 * by the COLOFF program. If this piece of code is found at the
 * entry address, the original cassette image can be restored from
 * the list of destination and size entries following this code.
 */
static uint8_t lmoffset[] = {
	0xf3,			/* di                  ; disable interrupts  */
	0x21,0xfb,0xc9,		/* ld   hl,$c9fb       ; EI, RET             */
	0x22,0x12,0x40,		/* ld   ($4012),hl     ; store to $4012 ..   */
	0x21,0x3e,0x00,		/* ld   hl,$003e       ; LD A,00h            */
	0x22,0x33,0x40,		/* ld   ($4033),hl     ; store to $4033 ..   */
	0x3e,0xc9,		/* ld   a,$c9          ; RET                 */
	0x32,0x35,0x40,		/* ld   ($4035),a      ; store to $4035      */
	0x32,0xe2,0x41,		/* ld   ($41e2),a      ; and to $41e2        */
	0x06,0x1c,		/* ld   b,$1c          ; 28 DOS commands     */
	0x21,0x52,0x41,		/* ld   hl,$4152       ; jump table at $4152 */
	0x36,0xc3,		/* ld   (hl),$c3       ; JP $013b            */
	0x23,			/* inc  hl             ;                     */
	0x36,0x3b,		/* ld   (hl),$3b       ; address low         */
	0x23,			/* inc  hl             ;                     */
	0x36,0x01,		/* ld   (hl),$01       ; address high        */
	0x23,			/* inc  hl             ;                     */
	0x10,0xf5,		/* djnz $-11           ; next jump vector    */
	0x06,0x15,		/* ld   b,$15          ; 21 DOS functions    */
	0x36,0xc9,		/* ld   (hl),$c9       ; RET                 */
	0x23,			/* inc  hl             ; skip 3 bytes        */
	0x23,			/* inc  hl             ;                     */
	0x23,			/* inc  hl             ;                     */
	0x10,0xf9,		/* djnz $-7            ; next return vector  */
	0xcd,0x0b,0x00,		/* call $000b          ; get address to HL   */
	0x11,0x21,0x00,		/* ld   de,$0021       ; add $0021           */
	0x19,			/* add  hl,de          ; address of relocate */
	0xe5,			/* push hl             ; save address        */
	0x21,0x00,0x60,		/* ld   hl,$6000       ; source              */
	0xe3,			/* ex   (sp),hl        ; swap source/table   */
	0x5e,			/* ld   e,(hl)         ; get destination lo  */
	0x23,			/* inc  hl             ;                     */
	0x56,			/* ld   d,(hl)         ; get destination hi  */
	0x23,			/* inc  hl             ;                     */
	0x4e,			/* ld   c,(hl)         ; get count lo        */
	0x23,			/* inc  hl             ;                     */
	0x46,			/* ld   b,(hl)         ; get count hi        */
	0x23,			/* inc  hl             ;                     */
	0x79,			/* ld   a,c            ; check count for     */
	0xb0,			/* or   b              ; $0000               */
	0x28,0x05,		/* jr   z,$+7          ; yes, we're done     */
	0xe3,			/* ex   (sp),hl        ; swap table/source   */
	0xed,0xb0,		/* ldir                ; copy memory block   */
	0x18,0xee,		/* jr   $-18           ; next table entry    */
	0xe1,			/* pop  hl             ; pop address         */
	0xcd,0xe2,0x41,		/* call $41e2          ; call vector         */
	0xc3			/* jp   $XXXX          ; start program       */
	/* relocate with the following format follows:                       */
        /* <addr_lo> <addr_hi> <count_lo> <count_hi>                         */
};

/**
 * @brief mover machine code
 *
 * This is the mover appended to the original machine code.
 */
static uint8_t mover[] = {
	0xf3,			/* di                  ; disable interrupts  */
	0x21,0x00,0x60,		/* ld   hl,$6000       ; source address      */
	0x11,0x00,0x48,		/* ld   de,$4800       ; destination address */
	0x01,0x00,0x40,		/* ld   bc,$4000       ; size of the move    */
	0xed,0xb0,		/* ldir                ; block move          */
	0xc3			/* jp   $XXXX          ; start program       */
};

int load_file(const char *filename)
{
	char casname[FILENAME_MAX], *name, *ext;
	uint8_t *mem;
	FILE *fcmd, *fcas;
	int ch;
	state_t state;
	uint8_t size;
	uint8_t csum;
	uint16_t addr;
	uint16_t dst;
	uint16_t src;
	uint16_t entry;
	uint16_t count;
	uint16_t i;

	mem = calloc(65536, sizeof(uint8_t));
	if (NULL == mem) {
		perror("load_file: calloc()");
		return -1;
	}

	size = 0;
	addr = 0;
	count = 0;
	state = ST_BLOCK_TYPE;
	fcmd = fopen(filename, "rb");
	if (NULL == fcmd) {
		perror(filename);
		free(mem);
		return -1;
	}
	while (-1 != (ch = fgetc(fcmd))) {
		switch (state) {
		case ST_BLOCK_TYPE:
			switch (ch) {
			case 0x01:	/* data block */
				state = ST_DATA_SIZE;
				break;
			case 0x02:	/* entry block */
				state = ST_ENTRY_SIZE;
				break;
			default:	/* comment block */
				state = ST_COMMENT_SIZE;
			}
			break;

		case ST_DATA_SIZE:
			size = ch;
			state = ST_DATA_LSB;
			break;

		case ST_DATA_LSB:
			addr = ch;
			state = ST_DATA_MSB;
			size--;
			break;

		case ST_DATA_MSB:
			addr = addr + 256 * ch;
			state = ST_DATA;
			size--;
			count = (0 == size) ? 256 : size;
			break;

		case ST_DATA:
			mem[addr++] = ch;
			if (0 == --count) {
				state = ST_BLOCK_TYPE;
			}
			break;

		case ST_ENTRY_SIZE:
			size = ch;
			state = ST_ENTRY_LSB;
			break;

		case ST_ENTRY_LSB:
			addr = ch;
			state = ST_ENTRY_MSB;
			size--;
			break;

		case ST_ENTRY_MSB:
			addr = addr + 256 * ch;
			size--;
			if (0 != size) {
				fprintf(stderr, "ENTRY: invalid size (%#x)\n", size);
			}
			state = ST_BLOCK_TYPE;
			break;

		case ST_COMMENT_SIZE:
			size = ch;
			state = ST_COMMENT;
			count = (0 == size) ? 256 : size;
			break;

		case ST_COMMENT:
			count--;
			if (0 == count) {
				state = ST_BLOCK_TYPE;
			}
			break;
		}
	}
	fclose(fcmd);

	snprintf(casname, sizeof(casname), "%s", filename);
	ext = strrchr(casname, '.');
	if (!strcasecmp(ext, ".cmd")) {
		/* replace .cmd with .cas */
		sprintf(ext, ".cas");
	} else {
		/* otherwise append .cas */
		snprintf(casname, sizeof(casname), "%s.cas", filename);
	}

	if (0 == memcmp(&mem[addr], lmoffset, sizeof(lmoffset))) {
		printf("LMOFFSET loader at address 0x%x\n", addr);

		fcas = fopen(casname, "wb");
		name = strrchr(casname, '/');
		if (NULL == name)
			name = casname;
		else
			name++;

		/* write cassette header */
		fprintf(fcas, "%c", CAS_SYNC);
		fprintf(fcas, "%c", CAS_SYSTEM_HEADER);
		for (i = 0; i < 6; i++) {
			if (*name != '\0' && *name != '.') {
				fprintf(fcas, "%c", toupper((uint8_t)*name++));
			} else {
				fprintf(fcas, "%c", 32);
			}
		}

		/* skip over program */
		addr += sizeof(lmoffset);
		/* read the original code entry address */
		entry = mem[addr] + 256 * mem[addr+1];
		/* addr = offset of the relocation table */
		addr += 2;
		/* source seems to always be $6000 */
		src = 0x6000;
		for (;;) {
			dst = mem[addr] + 256 * mem[addr+1];
			addr += 2;
			count = mem[addr] + 256 * mem[addr+1];
			addr += 2;
			if (0 == count)
				break;
			fprintf(fcas, "%c%c%c%c",
				CAS_SYSTEM_DATA, count % 256,
				dst % 256, dst / 256);
			/* checksum contains address lo and hi */
			csum = dst % 256 + (dst / 256);
			for (i = 0; i < count; i++) {
				fprintf(fcas, "%c", mem[src + i]);
				csum += mem[src + i];
			}
			fprintf(fcas, "%c", csum);
			src += count;
		}
		fprintf(fcas, "%c%c%c",
			CAS_SYSTEM_ENTRY, entry % 256, entry / 256);
		fclose(fcas);
	} else if (mem[addr+0] == mover[0] &&
		mem[addr+1] == mover[1] &&
		mem[addr+4] == mover[4] &&
		mem[addr+7] == mover[7] &&
		mem[addr+10] == mover[10] &&
		mem[addr+11] == mover[11] &&
		mem[addr+12] == mover[12]) {

		printf("MOVER at address 0x%x\n", addr);
		fcas = fopen(casname, "wb");
		name = strrchr(casname, '/');
		if (NULL == name)
			name = casname;
		else
			name++;

		/* write cassette header */
		fprintf(fcas, "%c", CAS_SYNC);
		fprintf(fcas, "%c", CAS_SYSTEM_HEADER);
		for (i = 0; i < 6; i++) {
			if (*name != '\0' && *name != '.') {
				fprintf(fcas, "%c", toupper((uint8_t)*name++));
			} else {
				fprintf(fcas, "%c", 32);
			}
		}

		/* source address */
		src = mem[addr+2] + 256 * mem[addr+3];
		/* destination address */
		dst = mem[addr+5] + 256 * mem[addr+6];
		/* size of the block move */
		count = mem[addr+8] + 256 * mem[addr+9];
		/* entry point */
		entry = mem[addr+13] + 256 * mem[addr+14];
		while (count > 0) {
			size = count > 255 ? 0x00 : count;
			fprintf(fcas, "%c%c%c%c",
				CAS_SYSTEM_DATA, size,
				dst % 256, dst / 256);
			/* checksum contains address lo and hi */
			csum = (dst % 256) + (dst / 256);
			for (i = 0; i < (size ? size : 256); i++) {
				fprintf(fcas, "%c", mem[src + i]);
				csum += mem[src + i];
			}
			fprintf(fcas, "%c", csum);
			src += size ? size : 256;
			dst += size ? size : 256;
			count -= size ? size : 256;
		}
		fprintf(fcas, "%c%c%c",
			CAS_SYSTEM_ENTRY, entry % 256, entry / 256);
		fclose(fcas);
	} else {

		printf("Converting to cassette format\n");
		fcas = fopen(casname, "wb");
		name = strrchr(casname, '/');
		if (NULL == name)
			name = casname;
		else
			name++;

		/* write cassette header */
		fprintf(fcas, "%c", CAS_SYNC);
		fprintf(fcas, "%c", CAS_SYSTEM_HEADER);
		for (i = 0; i < 6; i++) {
			if (*name != '\0' && *name != '.') {
				fprintf(fcas, "%c", toupper((uint8_t)*name++));
			} else {
				fprintf(fcas, "%c", 32);
			}
		}

		size = 0;
		csum = 0;
		addr = 0;
		count = 0;
		state = ST_BLOCK_TYPE;
		fcmd = fopen(filename, "rb");
		if (NULL == fcmd) {
			perror(filename);
			free(mem);
			return -1;
		}
		while (-1 != (ch = fgetc(fcmd))) {
			switch (state) {
			case ST_BLOCK_TYPE:
				switch (ch) {
				case 0x01:	/* data block */
					state = ST_DATA_SIZE;
					break;
				case 0x02:	/* entry block */
					state = ST_ENTRY_SIZE;
					break;
				default:	/* comment block */
					state = ST_COMMENT_SIZE;
				}
				break;

			case ST_DATA_SIZE:
				size = ch;
				state = ST_DATA_LSB;
				break;

			case ST_DATA_LSB:
				addr = ch;
				state = ST_DATA_MSB;
				size--;
				break;

			case ST_DATA_MSB:
				addr = addr + 256 * ch;
				state = ST_DATA;
				size--;
				fprintf(fcas, "%c%c%c%c",
					CAS_SYSTEM_DATA, size,
					addr % 256, addr / 256);
				/* checksum contains address lo and hi */
				csum = (addr % 256) + (addr / 256);
				count = (0 == size) ? 256 : size;
				break;

			case ST_DATA:
				fprintf(fcas, "%c", ch);
				csum += ch;
				if (0 == --count) {
					state = ST_BLOCK_TYPE;
					fprintf(fcas, "%c", csum);
				}
				break;

			case ST_ENTRY_SIZE:
				size = ch;
				state = ST_ENTRY_LSB;
				break;

			case ST_ENTRY_LSB:
				addr = ch;
				state = ST_ENTRY_MSB;
				size--;
				break;

			case ST_ENTRY_MSB:
				addr = addr + 256 * ch;
				size--;
				if (0 != size) {
					fprintf(stderr, "ENTRY: invalid size (%#x)\n", size);
				}
				fprintf(fcas, "%c%c%c",
					CAS_SYSTEM_ENTRY,
					addr % 256, addr / 256);
				state = ST_BLOCK_TYPE;
				break;

			case ST_COMMENT_SIZE:
				/* FIXME: add comments to header? */
				size = ch;
				state = ST_COMMENT;
				count = (0 == size) ? 256 : size;
				break;

			case ST_COMMENT:
				count--;
				if (0 == count) {
					state = ST_BLOCK_TYPE;
				}
				break;
			}
		}
		fclose(fcmd);
		fclose(fcas);
	}
	free(mem);
	return 0;
}

int main(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			continue;
		}
		load_file(argv[i]);
	}

	return 0;
}
