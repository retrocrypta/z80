/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * z80dasm.h	Z80 CPU disassembler typedefs and externs
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_Z80DASM_H_INCLUDED_)
#define	_Z80DASM_H_INCLUDED_

#include "system.h"

#define	DASMFLAG_STEP_OVER		0x100000
#define	DASMFLAG_STEP_OUT		0x200000
#define	DASMFLAG_SUPPORTED		0x800000

#ifdef __cplusplus
extern "C" {
#endif

/** @brief disassemble opcode at PC and return number of bytes it takes */
extern uint32_t z80_dasm(char *buffer, uint32_t pc, const uint8_t *oprom, const uint8_t *opram);

#ifdef __cplusplus
}
#endif

#endif	/* !defined(_Z80DASM_H_INCLUDED_) */
