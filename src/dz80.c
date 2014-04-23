/* ed:set tabstop=8 noexpandtab: */
/**************************************************************************
 *
 * dz80.c	Quick and dirty Z80 disassembler frontend
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 **************************************************************************/
#include "z80dasm.h"
#include "dasm.h"

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

int offset;
int cmd_format;

dasm_t *dasm = NULL;
uint32_t ndasm = 0;
uint32_t nalloc = 0;

#define	COMMENT_COLUMN	48
#define	MAX_COLUMN	76

int load_defs(const char *filename)
{
	char line[1024], *eol, *src;
	FILE *fp;
	dasm_t *d;
	int lno;

	fp = fopen(filename, "r");
	if (NULL == fp) {
		perror(filename);
		return -1;
	}
	lno = 0;
	while (!feof(fp)) {
		if (NULL == fgets(line, sizeof(line), fp))
			break;

		lno++;
		eol = strchr(line, '\n');
		if (NULL != eol)
			*eol = '\0';

		if ('#' == line[0] || '\0' == line[0])
			continue;			/* comment or empty line */

		if ('*' == line[0]) {
			/* continuation of multi-line comment */
			int old, len;

			d = &dasm[ndasm - 1];
			src = strchr(line, ';');
			if (NULL == src) {
				fprintf(stderr, "error on line #%d (%s): missing semicolon\n",
					lno, filename);
				continue;
			}
			old = strlen(d->comment);
			len = old + 1 + strlen(src) + 1;
			d->comment = realloc(d->comment, len);
			if (NULL == d->comment) {
				perror("realloc()");
				exit(1);
			}
			snprintf(d->comment + old, len - old, "\n%s", src);
			continue;
		}
		if (ndasm >= nalloc) {
			nalloc = nalloc ? nalloc * 2 : 32;
			dasm = realloc(dasm, nalloc * sizeof(dasm_t));
			if (NULL == dasm) {
				perror("load_defs: realloc()");
				return -1;
			}
		}
		d = &dasm[ndasm];
		d->addr = strtoull(line, &src, 16);
		while (*src == '\t' || *src == ' ')
			src++;
		switch (*src) {
		case 'c': case 'C':
			d->type = DASM_CODE;
			break;
		case 'e': case 'E':
			d->type = DASM_ENTRY;
			break;
		case 'b': case 'B':
			d->type = DASM_BYTES;
			break;
		case 'w': case 'W':
			d->type = DASM_WORDS;
			break;
		case 'a': case 'A':
			d->type = DASM_ASCII;
			break;
		case 's': case 'S':
			d->type = DASM_SPACE;
			break;
		}
		while (*src != '\0' && *src != '\t' && *src != ' ')
			src++;
		while (*src == '\t' || *src == ' ')
			src++;
		d->comment = strdup(src);
		ndasm++;
	}
	return 0;
}

const dasm_t *get_dasm(uint32_t addr)
{
	const dasm_t *d0 = dasm;
	const dasm_t *d;

	if (NULL == d0)
		return d0;
	d = d0;
	for (d = d0; addr > d->addr; d++)
		;
	if (addr == d->addr)
		return d;
	if (d > d0)
		d--;
	return d;
}

uint32_t get_length(uint32_t addr)
{
	const dasm_t *d0 = dasm;
	const dasm_t *d;
	uint32_t len;

	if (NULL == dasm)
		return 0;
	for (d = d0; addr > d->addr; d++)
		;
	if (addr != d->addr)
		return 0;
	while (addr == d->addr)
		d++;
	len = d->addr - addr;
	return len;
}

const char *ascii(uint8_t *src, uint32_t len)
{
	static char *buff;
	uint32_t size = 65536;
	uint32_t i;
	uint8_t b;
	char *dst;
	int str;
	char *comma;

	if (NULL != buff)
		free(buff);
	buff = calloc(size, sizeof(char));

	str = 0;
	comma = "";
	for (i = 0, dst = buff; i < len; i++) {
		b = src[i];
		if (b < 32 || b > 126) {
			if (str) {
				dst += snprintf(dst, size - (uint32_t)(dst - buff), "\"");
				comma = ",";
				str = 0;
			}
			if ((b & 127) < 32 || (b & 127) > 126) {
				dst += snprintf(dst, size - (uint32_t)(dst - buff), "%s%u", comma, b);
			} else {
				dst += snprintf(dst, size - (uint32_t)(dst - buff), "%s%u+'%c'", comma, 128, b & 127);
			}
			comma = ",";
		} else {
			if (!str) {
				dst += snprintf(dst, size - (uint32_t)(dst - buff), "%s\"", comma);
				str = 1;
			}
			*dst++ = b;
			*dst = '\0';
		}
	}
	if (str) {
		dst += snprintf(dst, size - (uint32_t)(dst - buff), "\"");
		str = 0;
	}
	return buff;
}

void comment_head(uint32_t start)
{
	const dasm_t *d;
	d = get_dasm(start);
	if (NULL == d)
		return;
	while (start == d->addr) {
		if (d->comment[0] == ';')
			printf("\n%s\n\n", d->comment);
		d++;
	}
}

void comment_line(uint32_t start, char *line, char *dst, size_t size)
{
	const dasm_t *d;
	char *space;
	int offs;

	d = get_dasm(start);
	if (NULL == d) {
		return;
		printf("%s\n", line);
		return;
	}
	while (start == d->addr) {
		if (d->comment[0] == ';') {
			d++;
			continue;
		}
		while (dst < &line[COMMENT_COLUMN])
			*dst++ = ' ';
		dst += snprintf(dst, size - (size_t)(dst-line), "; %s", d->comment);
		d++;
	}
	for (;;) {
		if ((size_t)(dst-line) < MAX_COLUMN) {
			printf("%s\n", line);
			return;
		}
		for (offs = MAX_COLUMN - 8; offs < size; offs++)
			if (line[offs] == ' ')
				break;
		line[offs] = '\0';
		space = line + offs;
		printf("%s\n", line);
		for (offs = 0, dst = line; offs < COMMENT_COLUMN; offs++)
			*dst++ = ' ';
		dst += snprintf(dst, size - (size_t)(dst-line), "; %s", space + 1);
	}
}

int load_file(const char *filename)
{
	char line[1024];
	char parm[80];
	char *dst;
	uint8_t *mem;
	FILE *fp;
	int ch;
	state_t state;
	const dasm_t *d;
	uint32_t start;
	uint8_t size;
	uint16_t addr;
	uint32_t min;
	uint32_t max;
	uint32_t count;
	uint32_t len;
	uint32_t ent;
	uint32_t app;
	uint32_t i;

	mem = calloc(65536 + 4, sizeof(uint8_t));
	if (NULL == mem) {
		perror("load_file: calloc()");
		return -1;
	}

	fp = fopen(filename, "rb");
	if (NULL == fp) {
		perror(filename);
		free(mem);
		return -1;
	}

	min = (uint16_t)-1;
	max = (uint16_t)0;
	size = 0;
	addr = 0;
	count = 0;
	if (cmd_format) {
		state = ST_BLOCK_TYPE;
		while (-1 != (ch = fgetc(fp))) {
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
				if (addr < min)
					min = addr;
				if (addr > max)
					max = addr;
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
	} else {
		min = offset;
		max = min + fread(mem + min, 1, 0x10000 - min, fp);
	}
	fclose(fp);

	while (min < max) {
		start = min;
		ent = get_length(min);

		comment_head(start);

		d = get_dasm(start);
		dst = line;
		dst += sprintf(dst, "  %05X ", min);
		if (NULL == d) {
			len = z80_dasm(parm, min, &mem[min], &mem[min]) & 0xffff;
		} else {
			switch (d->type) {
			case DASM_BYTES:
				snprintf(parm, sizeof(parm), "DEFB    %03XH", mem[min]);
				len = 1;
				break;
			case DASM_WORDS:
				snprintf(parm, sizeof(parm), "DEFW    %05XH", mem[min] + 256 * mem[min+1]);
				len = 2;
				break;
			case DASM_ASCII:
				len = get_length(start);
				if (start + len > max)
					len = max - start;
				if (0 == len)
					len = 1;
				snprintf(parm, sizeof(parm), "DEFB    %s", ascii(&mem[start], len));
				while (len > 7) {
					for (i = 0; i < 7; i++)
						dst += snprintf(dst, sizeof(line)-(int)(dst-line), "%02X", mem[min++]);
					dst += snprintf(dst, sizeof(line)-(int)(dst-line), "  %s", parm);
					d = get_dasm(min);
					while (min == d->addr) {
						if (d->comment[0] != ';') {
							while (dst < &line[COMMENT_COLUMN])
								*dst++ = ' ';
							dst += snprintf(dst, sizeof(line)-(int)(dst-line), "; %s", d->comment);
						}
						d++;
					}
					parm[0] = '\0';
					len -= 7;
					printf("%s\n", line);
					dst = line;
					dst += snprintf(dst, sizeof(line)-(int)(dst-line), "  %05X ", min);
				}
				break;
			case DASM_SPACE:
				len = get_length(start);
				if (start + len > max)
					len = max - start;
				if (0 == len)
					len = 1;
				snprintf(parm, sizeof(parm), "DEFS    %u,%03XH", len, mem[min]);
				while (len > 7) {
					for (i = 0; i < 7; i++)
						dst += snprintf(dst, sizeof(line)-(int)(dst-line), "%02X", mem[min++]);
					dst += snprintf(dst, sizeof(line)-(int)(dst-line), "  %s", parm);
					d = get_dasm(min);
					while (min == d->addr) {
						if (d->comment[0] != ';') {
							while (dst < &line[COMMENT_COLUMN])
								*dst++ = ' ';
							dst += snprintf(dst, sizeof(line)-(int)(dst-line), "; %s", d->comment);
						}
						d++;
					}
					parm[0] = '\0';
					len -= 7;
					printf("%s\n", line);
					dst = line;
					dst += snprintf(dst, sizeof(line)-(int)(dst-line), "  %05X ", min);
				}
				break;
			case DASM_CODE:
			case DASM_ENTRY:
				len = z80_dasm(parm, min, &mem[min], &mem[min]) & 0xffff;
				break;
			case DASM_NONE:
			default:
				snprintf(parm, sizeof(parm), "DEFB    %03XH", mem[min]);
				len = 1;			
			}
		}
		for (i = 0; i < 7; i++) {
			if (i < len) {
				dst += snprintf(dst, sizeof(line)-(int)(dst-line), "%02X", mem[min++]);
			} else {
				dst += snprintf(dst, sizeof(line)-(int)(dst-line), "  ");
			}
		}
		dst += snprintf(dst, sizeof(line)-(int)(dst-line), "  %s", parm);
		comment_line(start, line, dst, sizeof(line));

		/* entry within previous opcodes? */
		while (0 != ent && ent != len) {
			d = get_dasm(start + ent);
			if (start + ent != d->addr || d->type != DASM_ENTRY)
				break;
			min = start + ent;
			comment_head(min);
			app = z80_dasm(parm, min, &mem[min], &mem[min]) & 0xffff;
			dst = line;
			dst += sprintf(dst, "* %05X ", min);
			for (i = 0; i < 7; i++) {
				if (i < ent) {
					dst += snprintf(dst, sizeof(line)-(int)(dst-line), "  ");
				} else if (i < ent + app) {
					dst += snprintf(dst, sizeof(line)-(int)(dst-line), "%02X", mem[min++]);
				} else {
					dst += snprintf(dst, sizeof(line)-(int)(dst-line), "  ");
				}
			}
			dst += snprintf(dst, sizeof(line)-(int)(dst-line), "  %s", parm);
			comment_line(start + ent, line, dst, sizeof(line));
			ent += app;
		}
	}

	free(mem);
	return 0;
}

int main(int argc, char **argv)
{
	int i;

	cmd_format = 0;
	offset = 0;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'c':
				cmd_format = 1;
				break;
			case 'd':
				if (i + 1 < argc) {
					i++;
					load_defs(argv[i]);
				}
				break;
			case 'o':
				i++;
				offset = strtoul(argv[i], NULL, 0);
				break;
			}
			continue;
		}
		load_file(argv[i]);
	}

	return 0;
}
