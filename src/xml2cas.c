/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * xml2cas.c	Colour Genie and TRS-80 XML to cassette converter
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <expat.h>
#include "sha1.h"
#include "unicode.h"

#define	DEBUG_UNICODE	0
#define	DEBUG_BLOCKS	0

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

typedef struct {
	uint8_t type;
	uint8_t csum;
	uint16_t addr;
	uint16_t size;
	uint16_t line;
	uint8_t *data;
	sha1_digest_t digest;
}	block_t;

#define	CAS_TRS80_SYNC		0xa5
#define	CAS_TRS80_BASIC_HEADER	0xd3
#define	CAS_EG2000_SYNC		0x66
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

block_t *blocks = NULL;
uint32_t nblocks = 0;
uint32_t nalloc = 0;

sha1_state_t cas_sha1;
sha1_digest_t cas_digest;

sha1_digest_t xml_digest;

int comments = 0;

int verbose = 0;

char *cdata = NULL;


/** @brief XML element start <cassette ... */
static void xml_start_cassette(const XML_Char **atts)
{
	const XML_Char **att;
	int rc;

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
	cas_machine = MACH_EG2000;
	cas_sync = CAS_EG2000_SYNC;
	cas_addr = 0;
	cas_line = 0;
	cas_entry = 0;
	cas_prefix = CAS_SYSTEM_HEADER;
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

	for (att = atts; *att; att += 2) {
		if (!strcasecmp(att[0], "sha1")) {
			rc = sha1_strhex(&xml_digest, att[1]);
			continue;
		}
		fprintf(stderr, "ignored %s attribute %s=%s...\n",
			"cassette", att[0], att[1]);
	}
}

/** @brief XML element start <machine ... */
static void xml_start_machine(const XML_Char **atts)
{
	const XML_Char **att;

	for (att = atts; *att; att += 2) {
		fprintf(stderr, "ignored %s attribute %s=%s...\n",
			"machine", att[0], att[1]);
	}
}

/** @brief XML element start <format ... */
static void xml_start_format(const XML_Char **atts)
{
	const XML_Char **att;

	for (att = atts; *att; att += 2) {
		fprintf(stderr, "ignored %s attribute %s=%s...\n",
			"format", att[0], att[1]);
	}
}

/** @brief XML element start <name ... */
static void xml_start_name(const XML_Char **atts)
{
	const XML_Char **att;

	for (att = atts; *att; att += 2) {
		fprintf(stderr, "ignored %s attribute %s=%s...\n",
			"name", att[0], att[1]);
	}
}

/** @brief XML element start <author ... */
static void xml_start_author(const XML_Char **atts)
{
	const XML_Char **att;

	for (att = atts; *att; att += 2) {
		fprintf(stderr, "ignored %s attribute %s=%s...\n",
			"author", att[0], att[1]);
	}
}

/** @brief XML element start <copyright ... */
static void xml_start_copyright(const XML_Char **atts)
{
	const XML_Char **att;

	for (att = atts; *att; att += 2) {
		fprintf(stderr, "ignored %s attribute %s=%s...\n",
			"copyright", att[0], att[1]);
	}
}

/** @brief XML element start <description ... */
static void xml_start_description(const XML_Char **atts)
{
	const XML_Char **att;

	for (att = atts; *att; att += 2) {
		fprintf(stderr, "ignored %s attribute %s=%s...\n",
			"description", att[0], att[1]);
	}
}

/** @brief XML element start <sync ... */
static void xml_start_sync(const XML_Char **atts)
{
	const XML_Char **att;

	for (att = atts; *att; att += 2) {
		fprintf(stderr, "ignored %s attribute %s=%s...\n",
			"sync", att[0], att[1]);
	}
}

/** @brief XML element start <filename ... */
static void xml_start_filename(const XML_Char **atts)
{
	const XML_Char **att;

	for (att = atts; *att; att += 2) {
		fprintf(stderr, "ignored %s attribute %s=%s...\n",
			"filename", att[0], att[1]);
	}
}

/** @brief XML element start <blocks ... */
static void xml_start_blocks(const XML_Char **atts)
{
	const XML_Char **att;

	for (att = atts; *att; att += 2) {
		if (!strcasecmp(att[0], "count")) {
			nalloc = strtoull(att[1], NULL, 0);
			blocks = calloc(nalloc, sizeof(block_t));
			if (NULL == blocks) {
				perror("xml_start_blocks: calloc()");
				exit(1);
			}
			continue;
		}
		fprintf(stderr, "ignored %s attribute %s=%s...\n",
			"blocks", att[0], att[1]);
	}
}

/** @brief XML element start <block ... */
static void xml_start_block(const XML_Char **atts)
{
	const XML_Char **att;
	block_t blk;

	memset(&blk, 0, sizeof(blk));
	nblocks = 0;
	for (att = atts; *att; att += 2) {
		if (!strcasecmp(att[0], "number")) {
			nblocks = strtoull(att[1], NULL, 0);
			continue;
		}
		if (!strcasecmp(att[0], "type")) {
			if (!strcasecmp(att[1], "basic")) {
				blk.type = T_BASIC;
				continue;
			}
			if (!strcasecmp(att[1], "system")) {
				blk.type = T_SYSTEM;
				continue;
			}
			if (!strcasecmp(att[1], "entry")) {
				blk.type = T_ENTRY;
				continue;
			}
			if (!strcasecmp(att[1], "raw")) {
				blk.type = T_RAW;
				continue;
			}
			fprintf(stderr, "ignored %s attribute %s=%s...\n",
				"blocks", att[0], att[1]);
			continue;
		}
		if (!strcasecmp(att[0], "address")) {
			blk.addr = strtoull(att[1], NULL, 0);
			continue;
		}
		if (!strcasecmp(att[0], "size")) {
			blk.size = strtoull(att[1], NULL, 0);
			continue;
		}
		if (!strcasecmp(att[0], "csum")) {
			blk.csum = strtoull(att[1], NULL, 0);
			continue;
		}
		if (!strcasecmp(att[0], "line")) {
			blk.line = strtoull(att[1], NULL, 0);
			continue;
		}
		if (!strcasecmp(att[0], "sha1")) {
			sha1_strhex(&blk.digest, att[1]);
			continue;
		}
		fprintf(stderr, "ignored %s attribute %s=%s...\n",
			"blocks", att[0], att[1]);
	}
#if	DEBUG_BLOCKS
	printf("block number: %u\n", nblocks);
	printf("        type: %u\n", blk.type);
	printf("     address: %u (%#x)\n", blk.addr, blk.addr);
	printf("        size: %u (%#x)\n", blk.size, blk.size);
	printf("        csum: %u (%#x)\n", blk.csum, blk.csum);
	printf("        line: %u (%#x)\n", blk.line, blk.line);
	printf("      digest: %s\n", sha1_hexstr(&blk.digest));
#endif
	if (nblocks >= nalloc) {
		perror("n > nalloc");
		exit(1);
	}
	memcpy(&blocks[nblocks], &blk, sizeof(blk));
	nblocks += 1;
}

/** @brief XML element start <image ... */
static void xml_start_image(const XML_Char **atts)
{
	const XML_Char **att;
	uint32_t size;

	for (att = atts; *att; att += 2) {
		if (!strcasecmp(att[0], "size")) {
			size = strtoull(att[1], NULL, 0);
			continue;
		}
		fprintf(stderr, "ignored %s attribute %s=%s...\n",
			"image", att[0], att[1]);
	}
}


/** @brief XML element start <decoded ... */
static void xml_start_decoded(const XML_Char **atts)
{
	const XML_Char **att;

	for (att = atts; *att; att += 2) {
		fprintf(stderr, "ignored %s attribute %s=%s...\n",
			"decoded", att[0], att[1]);
	}
}

/** @brief XML element start */
static void xml_start(void *user_data, const XML_Char *name, const XML_Char **atts)
{
	cdata = NULL;

	if (!strcasecmp(name, "cassette")) {
		xml_start_cassette(atts);
		return;
	}
	if (!strcasecmp(name, "machine")) {
		xml_start_machine(atts);
		return;
	}
	if (!strcasecmp(name, "format")) {
		xml_start_format(atts);
		return;
	}
	if (!strcasecmp(name, "name")) {
		xml_start_name(atts);
		return;
	}
	if (!strcasecmp(name, "author")) {
		xml_start_author(atts);
		return;
	}
	if (!strcasecmp(name, "copyright")) {
		xml_start_copyright(atts);
		return;
	}
	if (!strcasecmp(name, "description")) {
		xml_start_description(atts);
		return;
	}
	if (!strcasecmp(name, "sync")) {
		xml_start_sync(atts);
		return;
	}
	if (!strcasecmp(name, "filename")) {
		xml_start_filename(atts);
		return;
	}
	if (!strcasecmp(name, "blocks")) {
		xml_start_blocks(atts);
		return;
	}
	if (!strcasecmp(name, "block")) {
		xml_start_block(atts);
		return;
	}
	if (!strcasecmp(name, "image")) {
		xml_start_image(atts);
		return;
	}
	if (!strcasecmp(name, "decoded")) {
		xml_start_decoded(atts);
		return;
	}
}

/** @brief XML element end */
static void xml_end(void *user_data, const XML_Char *name)
{
	if (!strcasecmp(name, "cassette")) {
		if (cdata) {
			free(cdata);
			cdata = NULL;
		}
		return;
	}
	if (!strcasecmp(name, "machine")) {
		if (cdata) {
			if (!strcasecmp(cdata, "eg2000")) {
				cas_machine = MACH_EG2000;
			}
			if (!strcasecmp(cdata, "trs80")) {
				cas_machine = MACH_TRS80;
			}
			free(cdata);
			cdata = NULL;
		}
		return;
	}
	if (!strcasecmp(name, "format")) {
		if (cdata) {
			if (!strcasecmp(cdata, "basic")) {
				cas_basic = 1;
			}
			if (!strcasecmp(cdata, "system")) {
				cas_basic = 0;
			}
			free(cdata);
			cdata = NULL;
		}
		return;
	}
	if (!strcasecmp(name, "name")) {
		if (NULL != cas_name)
			free(cas_name);
		cas_name = NULL;
		if (cdata) {
			cas_name = utf8_to_iso8859_1(cdata, 0);
			free(cdata);
			cdata = NULL;
		}
		return;
	}
	if (!strcasecmp(name, "author")) {
		if (NULL != cas_author)
			free(cas_author);
		cas_author = NULL;
		if (cdata) {
			cas_author = utf8_to_iso8859_1(cdata, 0);
			free(cdata);
			cdata = NULL;
		}
		return;
	}
	if (!strcasecmp(name, "copyright")) {
		if (NULL != cas_copyright)
			free(cas_copyright);
		cas_copyright = NULL;
		if (cdata) {
			cas_copyright = utf8_to_iso8859_1(cdata, 0);
			free(cdata);
			cdata = NULL;
		}
		return;
	}
	if (!strcasecmp(name, "description")) {
		if (NULL != cas_description)
			free(cas_description);
		cas_description = NULL;
		if (cdata) {
			cas_description = utf8_to_iso8859_1(cdata, 0);
			free(cdata);
			cdata = NULL;
		}
		return;
	}
	if (!strcasecmp(name, "sync")) {
		cas_sync = cas_machine == MACH_TRS80 ?
			CAS_TRS80_SYNC : CAS_EG2000_SYNC;
		if (cdata) {
			cas_sync = strtoull(cdata, NULL, 0);
			free(cdata);
			cdata = NULL;
		}
		return;
	}
	if (!strcasecmp(name, "filename")) {
		if (cdata) {
			char *name = utf8_to_iso8859_1(cdata, 0);
			snprintf(cas_filename, sizeof(cas_filename), "%s", name);
			free(name);
			free(cdata);
			cdata = NULL;
		}
		return;
	}
	if (!strcasecmp(name, "blocks")) {
		if (cdata) {
			free(cdata);
			cdata = NULL;
		}
		return;
	}
	if (!strcasecmp(name, "block")) {
		if (cdata) {
			block_t *b = &blocks[nblocks - 1];
			sha1_state_t sha1;
			sha1_digest_t digest;

			sha1_init(&sha1);
			if (b->size) {
				char *src;
				uint32_t n;

				b->data = malloc(b->size);
				if (NULL == b->data) {
					perror("xml_end: block malloc()");
					exit(1);
				}
				for (n = 0, src = cdata; n < b->size; n++) {
					b->data[n] = 16 * xdigit(*src);
					if ('\0' != *src)
						src++;
					b->data[n] += xdigit(*src);
					if ('\0' != *src)
						src++;
				}
				sha1_append(&sha1, b->data, b->size);
			}
			sha1_finish(&sha1, &digest);
			if (memcmp(b->digest.b, digest.b, sizeof(digest.b))) {
				fprintf(stderr, "SHA1 digest mismatch: %s\n",
					sha1_hexstr(&digest));
			}
			free(cdata);
			cdata = NULL;
		}
		return;
	}
	if (!strcasecmp(name, "image")) {
		if (cdata) {
			/* ignore image data */
			free(cdata);
			cdata = NULL;
		}
		return;
	}
	if (!strcasecmp(name, "decoded")) {
		if (cdata) {
			/* ignore decoded data */
			free(cdata);
			cdata = NULL;
		}
		return;
	}
	if (cdata) {
		free(cdata);
		cdata = NULL;
	}
}


/** @brief XML element data */
static void xml_cdata(void *user_data, const XML_Char *data, int len)
{
	int old = 0;

	if (cdata)
		old = strlen(cdata);
	cdata = realloc(cdata, old + len + 1);
	if (NULL == cdata) {
		perror("xml_cdata: realloc()");
		exit(1);
	}
	memcpy(cdata + old, data, len);
	cdata[old + len] = '\0';
}

int cas_read_xml(const char *filename)
{
	FILE *fp;
	XML_Parser xml;
	char buff[4096];
	int size, rc;

	printf("===> %s\n", filename);
	fp = fopen(filename, "r");
	if (NULL == fp) {
		perror("cas_read_xml: fopen()");
		return -1;
	}

	xml = XML_ParserCreate(NULL);
	if (NULL == xml) {
		perror("cas_read_xml: XML_ParserCreate()");
		exit(1);
	}
	XML_SetUserData(xml, NULL);
	XML_SetElementHandler(xml, xml_start, xml_end);
	XML_SetCharacterDataHandler(xml, xml_cdata);

	while (!feof(fp)) {
		size = fread(buff, 1, sizeof(buff), fp);
		rc = XML_Parse(xml, buff, size, 0);
		if (rc == XML_STATUS_ERROR) {
			fprintf(stderr, "XML_Parse(%p,%p,%d,%d) failed in line:%d column:%d\n",
				xml, buff, size, 0,
				(int)XML_GetCurrentLineNumber(xml),
				(int)XML_GetCurrentColumnNumber(xml));
			XML_ParserFree(xml);
			return -1;
		}
	}
	fclose(fp);
	rc = XML_Parse(xml, buff, 0, 1);
	XML_ParserFree(xml);

	return 0;
}

int cas_write_image(const char *filename)
{
	char casname[FILENAME_MAX], *ext;
	uint8_t buff[8];
	FILE *fp;
	uint32_t i, n;

	snprintf(casname, sizeof(casname), "%s", filename);
	ext = strrchr(casname, '.');
	if (ext && !strcasecmp(ext, ".xml")) {
		sprintf(ext, ".cas");
	} else {
		snprintf(casname, sizeof(casname), "%s.xml", filename);
	}

	fp = fopen(casname, "wb");
	if (NULL == fp)
		return -1;
	if (cas_machine == MACH_EG2000 && comments) {
		fprintf(fp, "Colour Genie - Virtual Tape File\r\n");
		fprintf(fp, "Name       : %s\r\n", cas_name);
		fprintf(fp, "Author     : %s\r\n", cas_author);
		fprintf(fp, "Copyright  : %s\r\n", cas_copyright);
		fprintf(fp, "Description: %s\r\n", cas_description);
		fprintf(fp, "%c%c", 26, 0); /* Ctrl-Z (text EOF) */
	}

	sha1_init(&cas_sha1);
	if (cas_basic) {
		switch (cas_machine) {
		case MACH_TRS80:
			buff[0] = cas_sync;
			buff[1] = CAS_TRS80_BASIC_HEADER;
			buff[2] = CAS_TRS80_BASIC_HEADER;
			buff[3] = CAS_TRS80_BASIC_HEADER;
			buff[4] = cas_filename[0];
			sha1_append(&cas_sha1, buff, 5);
			fwrite(buff, 1, 5, fp);
			break;
		case MACH_EG2000:
			buff[0] = cas_sync;
			buff[1] = cas_filename[0];
			sha1_append(&cas_sha1, buff, 2);
			fwrite(buff, 1, 2, fp);
			break;
		}
		for (i = 0; i < nblocks; i++) {
			block_t *b = &blocks[i];
			switch (b->type) {
			case T_BASIC:
				if (b->addr) {
					buff[0] = b->addr % 256;
					buff[1] = b->addr / 256;
					buff[2] = b->line % 256;
					buff[3] = b->line / 256;
					sha1_append(&cas_sha1, buff, 4);
					fwrite(buff, 1, 4, fp);
					sha1_append(&cas_sha1, b->data, b->size);
					fwrite(b->data, 1, b->size, fp);
				} else {
					buff[0] = b->addr % 256;
					buff[1] = b->addr / 256;
					sha1_append(&cas_sha1, buff, 2);
					fwrite(buff, 1, 2, fp);
				}
				break;
			}
		}
	} else {
		buff[0] = cas_sync;
		buff[1] = CAS_SYSTEM_HEADER;
		for (i = 0; i < 6; i++)
			buff[i+2] = cas_filename[i];
		sha1_append(&cas_sha1, buff, 8);
		fwrite(buff, 1, 8, fp);
		for (i = 0; i < nblocks; i++) {
			block_t *b = &blocks[i];
			switch (b->type) {
			case T_SYSTEM:
				buff[0] = CAS_SYSTEM_DATA;
				buff[1] = b->size % 256;
				buff[2] = b->addr % 256;
				buff[3] = b->addr / 256;
				sha1_append(&cas_sha1, buff, 4);
				fwrite(buff, 1, 4, fp);
				cas_csum = buff[2];
				cas_csum += buff[3];
				sha1_append(&cas_sha1, b->data, b->size);
				fwrite(b->data, 1, b->size, fp);
				for (n = 0; n < b->size; n++)
					cas_csum += b->data[n];
				buff[0] = b->csum;
				if (cas_csum != b->csum) {
					fprintf(stderr, "SYSTEM_CSUM: block #%u checksum error (found:0x%02x calc:0x%02x)\n",
						i, b->csum, cas_csum);
				}
				sha1_append(&cas_sha1, buff, 1);
				fwrite(buff, 1, 1, fp);
				break;
			case T_ENTRY:
				buff[0] = CAS_SYSTEM_ENTRY;
				buff[1] = b->addr % 256;
				buff[2] = b->addr / 256;
				sha1_append(&cas_sha1, buff, 3);
				fwrite(buff, 1, 3, fp);
				break;
			case T_RAW:
				fwrite(b->data, 1, b->size, fp);
			}
		}
	}
	sha1_finish(&cas_sha1, &cas_digest);
	fclose(fp);

	if (memcmp(cas_digest.b, &xml_digest.b, sizeof(cas_digest.b))) {
		fprintf(stderr, "SHA1 digest mismatch: %s\n",
			sha1_hexstr(&cas_digest));
	}

	return 0;
}

int main(int argc, char **argv)
{
	int i, rc;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-v")) {
			verbose++;
			continue;
		}
		if (!strcmp(argv[i], "-c")) {
			comments++;
			continue;
		}
		rc = cas_read_xml(argv[i]);
		if (rc < 0) {
			fprintf(stderr, "cas_read_xml(\"%s\") returned %d\n",
				argv[i], rc);
			continue;
		}
		rc = cas_write_image(argv[i]);
		if (rc < 0)
			break;
	}
	return 0;
}
