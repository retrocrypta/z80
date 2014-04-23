/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * system.h	System includes and global macros
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_SYS_H_INCLUDED_)
#define _SYS_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <SDL.h>
#include <SDL_video.h>
#include <SDL_endian.h>

#if	DEBUG
#define	LOG(x) logprintf x
extern void logprintf(int ll, const char *tag, const char *fmt, ...);
#else
#define	LOG(x)
#endif

#if	(HAVE_FSEEKO==0)
extern int fseeko(FILE *fp, off_t offs, int whence);
#endif

typedef enum {
	SYS_IRQ,
	SYS_NMI,
	SYS_RST
}	reset_t;

extern void sys_reset(reset_t how);
extern const char *sys_get_name(void);
extern void *sys_get_frame_timer(void);
extern void sys_set_full_refresh(void);
extern void sys_cpu_panel_init(void *);
extern void sys_cpu_panel_update(void *);

/** @brief invalid value (unsigned int) */
#define	U32INVALID	((uint32_t)-1)

/** @brief address spaces */
typedef enum {
	ADDRESS_SPACE_PROGRAM,
	ADDRESS_SPACE_DATA,
	ADDRESS_SPACE_IO,
	ADDRESS_SPACES
}	address_space_t;

/** @brief max. number of concurrent CPUs */
#define	MAX_CPU		4

/** @brief max. number of timers */
#define	MAX_TMR		32


/** @brief address range for memory */
#define	MEMSIZE		0x10000

/** @brief address range for I/O */
#define	IOSIZE		0x10000

/** @brief lookup table size for memory and I/O */
#define	L1SIZE		64

/** @brief lookup table shift for memory and I/O */
#define	L1SHIFT		10

extern uint8_t mem[MEMSIZE];

/** @brief array of function pointers: read from memory */
extern uint8_t (*rd_mem[L1SIZE])(uint32_t addr);

/** @brief array of function pointers: write to memory */
extern void (*wr_mem[L1SIZE])(uint32_t addr, uint8_t data);

/** @brief read from memory */
#define	program_read_byte(addr) rd_mem[(addr)>>L1SHIFT](addr)

/** @brief write to memory */
#define	program_write_byte(addr,data) do { \
	wr_mem[(addr)>>L1SHIFT](addr,data); \
} while (0)

/** @brief array of function pointers: read from memory */
extern uint8_t (*rd_io[L1SIZE])(uint32_t addr);

/** @brief array of function pointers: write to memory */
extern void (*wr_io[L1SIZE])(uint32_t addr, uint8_t data);

/** @brief read from memory */
#define	io_read_byte(addr) rd_io[(addr)>>L1SHIFT](addr)

/** @brief write to memory */
#define	io_write_byte(addr,data) do { \
	wr_io[(addr)>>L1SHIFT](addr,data); \
} while (0)

/** @brief XXX: fixme */
#define	mem_set_context(cpunum)

/** @brief XXX: fixme */
#define	mem_set_opbase(opbase)

#endif	/* !defined(_SYS_H_INCLUDED_) */
