/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * z80.h	Z80 CPU emulator typedefs and externs
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_Z80_H_INCLUDED_)
#define	_Z80_H_INCLUDED_

#include <SDL_endian.h>
#include "system.h"
#include "timer.h"
#include "z80dasm.h"

#if	(SDL_BYTEORDER==SDL_LIL_ENDIAN)
typedef union {
	uint64_t qword;
	struct { uint32_t d0, d1; } dword;
	struct { uint16_t w0, w1, w2, w3; } word;
	struct { uint8_t b0, b1, b2, b3, b4, b5, b6, b7; } byte;
}	bwdq_t;
#else
typedef union {
	uint64_t qword;
	struct { uint32_t d1, d0; } dword;
	struct { uint16_t w3, w2, w1, w0; } word;
	struct { uint8_t b7, b6, b5, b4, b3, b2, b1, b0; } byte;
}	bwdq_t;
#endif

typedef enum {
	Z80_PC, Z80_SP, Z80_AF, Z80_BC, Z80_DE, Z80_HL, Z80_IX, Z80_IY,
	Z80_EA, Z80_MP, Z80_AF2, Z80_BC2, Z80_DE2, Z80_HL2, Z80_R, Z80_I,
	Z80_IM, Z80_IFF1, Z80_IFF2, Z80_IRQ
}	z80_reg_t;


typedef struct {
	/** @brief program counter */
	bwdq_t	pc;
	/** @brief stack pointer */
	bwdq_t	sp;
	/** @brief Accu and Flags register pair */
	bwdq_t	af;
	/** @brief BC register pair */
	bwdq_t	bc;
	/** @brief DE register pair */
	bwdq_t	de;
	/** @brief HL register pair */
	bwdq_t	hl;
	/** @brief IX register pair */
	bwdq_t	ix;
	/** @brief IY register pair */
	bwdq_t	iy;
	/** @brief effective address register */
	bwdq_t	ea;
	/** @brief memory pointer register */
	bwdq_t	mp;
	/** @brief AF' register pair */
	bwdq_t	af2;
	/** @brief BC' register pair */
	bwdq_t	bc2;
	/** @brief DE' register pair */
	bwdq_t	de2;
	/** @brief HL' register pair */
	bwdq_t	hl2;
	/** @brief refresh register */
	uint8_t r;
	/** @brief refresh register bit 7 saved on LD R,A */
	uint8_t	r7;
	/** @brief interrupt vector register */
	uint8_t	iv;
	/** @brief interrupt mode register */
	uint8_t	im;
	/** @brief interrupt flip-flops */
	uint8_t	iff;
	/** @brief pending interrupt request */
	uint8_t	irq;
}	z80_cpu_t;

#ifdef	__cplusplus
extern "C" {
#endif

/** @brief Z80 CPU context */
extern z80_cpu_t z80;

/** @brief current cycle count (incrementing from 0 to cycle count per frame) */
extern int z80_cc;

/** @brief DMA cycle count (stolen from Z80 CPU) */
extern int z80_dma;

/** @brief shorthand name for program space read byte */
#define	RD_MEM(addr) program_read_byte(addr)

/** @brief shorthand name for program space write byte */
#define	WR_MEM(addr,val) program_write_byte(addr,val)

/** @brief shorthand name for I/O space read byte */
#define	RD_IO(addr) io_read_byte(addr)

/** @brief shorthand name for I/O space write byte */
#define	WR_IO(addr,val) io_write_byte(addr,val)

/** @brief get a Z80 CPU register */
extern uint32_t z80_get_reg(z80_cpu_t *cpu, uint32_t id);

/** @brief execute a number of cycles */
extern int z80_execute(z80_cpu_t *cpu);

/** @brief dump the current state of (some) CPU registers and disassemble from PC */
extern void z80_dump_state(z80_cpu_t *cpu);

/** @brief reset the Z80 CPU context */
extern void z80_reset(z80_cpu_t *cpu);

/** @brief interrupt the Z80 CPU context */
extern int z80_interrupt(z80_cpu_t *cpu, int type);

#ifdef	__cplusplus
}
#endif

#endif	/* !defined(_Z80_H_INCLUDED_) */
