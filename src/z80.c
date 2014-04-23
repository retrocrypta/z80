/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * cpu->c	Z80 CPU emulator
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#include "z80.h"

/** @brief 32 bit program counter */
#define	dPC	cpu->pc.dword.d0
/** @brief 32 bit stack pointer */
#define	dSP	cpu->sp.dword.d0
/** @brief 32 bit BC register pair */
#define	dBC	cpu->bc.dword.d0
/** @brief 32 bit DE register pair */
#define	dDE	cpu->de.dword.d0
/** @brief 32 bit HL register pair */
#define	dHL	cpu->hl.dword.d0
/** @brief 32 bit AF register pair */
#define	dAF	cpu->af.dword.d0
/** @brief 32 bit BC' register pair */
#define	dBC2	cpu->bc2.dword.d0
/** @brief 32 bit DE' register pair */
#define	dDE2	cpu->de2.dword.d0
/** @brief 32 bit HL' register pair */
#define	dHL2	cpu->hl2.dword.d0
/** @brief 32 bit AF' register pair */
#define	dAF2	cpu->af2.dword.d0
/** @brief 32 bit IX register pair */
#define	dIX	cpu->ix.dword.d0
/** @brief 32 bit IY register pair */
#define	dIY	cpu->iy.dword.d0
/** @brief 32 bit effective address */
#define	dEA	cpu->ea.dword.d0
/** @brief 32 bit memory pointer */
#define	dMP	cpu->mp.dword.d0

/** @brief program counter */
#define	PC	cpu->pc.word.w0
/** @brief stack pointer */
#define	SP	cpu->sp.word.w0
/** @brief BC register pair */
#define	BC	cpu->bc.word.w0
/** @brief DE register pair */
#define	DE	cpu->de.word.w0
/** @brief HL register pair */
#define	HL	cpu->hl.word.w0
/** @brief AF register pair */
#define	AF	cpu->af.word.w0
/** @brief BC' register pair */
#define	BC2	cpu->bc2.word.w0
/** @brief DE' register pair */
#define	DE2	cpu->de2.word.w0
/** @brief HL' register pair */
#define	HL2	cpu->hl2.word.w0
/** @brief AF' register pair */
#define	AF2	cpu->af2.word.w0
/** @brief IX register pair */
#define	IX	cpu->ix.word.w0
/** @brief IY register pair */
#define	IY	cpu->iy.word.w0
/** @brief effective address */
#define	EA	cpu->ea.word.w0
/** @brief memory pointer */
#define	MP	cpu->mp.word.w0

/** @brief B register */
#define	B	cpu->bc.byte.b1
/** @brief C register */
#define	C	cpu->bc.byte.b0
/** @brief D register */
#define	D	cpu->de.byte.b1
/** @brief E register */
#define	E	cpu->de.byte.b0
/** @brief H register */
#define	H	cpu->hl.byte.b1
/** @brief L register */
#define	L	cpu->hl.byte.b0
/** @brief accumulator */
#define	A	cpu->af.byte.b1
/** @brief flags */
#define	F	cpu->af.byte.b0
/** @brief stack pointer hi */
#define	SPH	cpu->sp.byte.b1
/** @brief stack pointer lo */
#define	SPL	cpu->sp.byte.b0
/** @brief program counter hi */
#define	PCH	cpu->pc.byte.b1
/** @brief program counter lo */
#define	PCL	cpu->pc.byte.b0
/** @brief IX hi */
#define	HX	cpu->ix.byte.b1
/** @brief IX lo */
#define	LX	cpu->ix.byte.b0
/** @brief IY hi */
#define	HY	cpu->iy.byte.b1
/** @brief IY lo */
#define	LY	cpu->iy.byte.b0
/** @brief effective address hi */
#define	EAH	cpu->ea.byte.b1
/** @brief effective address lo */
#define	EAL	cpu->ea.byte.b0
/** @brief memory pointer hi */
#define	MPH	cpu->mp.byte.b1
/** @brief memory pointer lo */
#define	MPL	cpu->mp.byte.b0

/** @brief carry flag mask */
#define	CF	(1<<0)
/** @brief negate flag mask */
#define	NF	(1<<1)
/** @brief parity/overflow flag mask */
#define	PF	(1<<2)
/** @brief X flag mask */
#define	XF	(1<<3)
/** @brief half carry flag mask */
#define	HF	(1<<4)
/** @brief Y flag mask */
#define	YF	(1<<5)
/** @brief zero flag mask */
#define	ZF	(1<<6)
/** @brief sign flag mask */
#define	SF	(1<<7)

typedef enum {
	REG_BC, REG_DE, REG_HL, REG_AF, REG_IX, REG_IY, REG_PC
}	push_pop_t;

#define	change_pc(addr)

/**************************** prototypes ****************************/
static __inline uint8_t RD_OP(z80_cpu_t *cpu);
static __inline uint8_t RD_ARGB(z80_cpu_t *cpu);
static __inline uint16_t RD_ARGW(z80_cpu_t *cpu);
static __inline uint8_t INC(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t DEC(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t ADD(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t ADC(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t SUB(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t SBC(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t AND(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t XOR(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t OR(z80_cpu_t *cpu, uint8_t val);
static __inline void CP(z80_cpu_t *cpu, uint8_t val);
static __inline uint16_t ADD16(z80_cpu_t *cpu, uint32_t reg, uint32_t val);
static __inline uint16_t SBC16(z80_cpu_t *cpu, uint32_t reg, uint32_t val);
static __inline uint16_t ADC16(z80_cpu_t *cpu, uint32_t reg, uint32_t val);
static __inline uint8_t RLC(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t RRC(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t RL(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t RR(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t SLA(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t SRA(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t SLL(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t SRL(z80_cpu_t *cpu, uint8_t val);
static __inline uint8_t BIT(z80_cpu_t *cpu, uint8_t b, uint8_t val);
static __inline uint8_t BIT_HL(z80_cpu_t *cpu, uint8_t b, uint8_t val);
static __inline uint8_t BIT_XY(z80_cpu_t *cpu, uint8_t b, uint8_t val);
static __inline uint8_t RES(z80_cpu_t *cpu, uint8_t b, uint8_t val);
static __inline uint8_t SET(z80_cpu_t *cpu, uint8_t b, uint8_t val);
static __inline void LDI(z80_cpu_t *cpu);
static __inline void CPI(z80_cpu_t *cpu);
static __inline void INI(z80_cpu_t *cpu);
static __inline void OUTI(z80_cpu_t *cpu);
static __inline void LDD(z80_cpu_t *cpu);
static __inline void CPD(z80_cpu_t *cpu);
static __inline void IND(z80_cpu_t *cpu);
static __inline void OUTD(z80_cpu_t *cpu);
static __inline void PUSH(z80_cpu_t *cpu, push_pop_t which);
static __inline void POP(z80_cpu_t *cpu, push_pop_t which);

/** @brief memory */
uint8_t mem[MEMSIZE];

/** @brief Z80 CPU context */
z80_cpu_t z80;

/** @brief Z80 CPU cycle count */
int z80_cc;

/** @brief Z80 CPU stolen (by DMA) cycles */
int z80_dma;

static const uint8_t cc_op[0x100] = {
 4,10, 7, 6, 4, 4, 7, 4, 4,11, 7, 6, 4, 4, 7, 4,
 8,10, 7, 6, 4, 4, 7, 4,12,11, 7, 6, 4, 4, 7, 4,
 7,10,16, 6, 4, 4, 7, 4, 7,11,16, 6, 4, 4, 7, 4,
 7,10,13, 6,11,11,10, 4, 7,11,13, 6, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 7, 7, 7, 7, 7, 7, 4, 7, 4, 4, 4, 4, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 5,10,10,10,10,11, 7,11, 5,10,10, 0,10,17, 7,11,
 5,10,10,11,10,11, 7,11, 5, 4,10,11,10, 0, 7,11,
 5,10,10,19,10,11, 7,11, 5, 4,10, 4,10, 0, 7,11,
 5,10,10, 4,10,11, 7,11, 5, 6,10, 4,10, 0, 7,11};

static const uint8_t cc_cb[0x100] = {
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8};

static const uint8_t cc_ed[0x100] = {
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
12,12,15,20, 8,14, 8, 9,12,12,15,20, 8,14, 8, 9,
12,12,15,20, 8,14, 8, 9,12,12,15,20, 8,14, 8, 9,
12,12,15,20, 8,14, 8,18,12,12,15,20, 8,14, 8,18,
12,12,15,20, 8,14, 8, 8,12,12,15,20, 8,14, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,
16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};

static const uint8_t cc_xy[0x100] = {
 4, 4, 4, 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4,
 4, 4, 4, 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4,
 4,14,20,10, 9, 9, 9, 4, 4,15,20,10, 9, 9, 9, 4,
 4, 4, 4, 4,23,23,19, 4, 4,15, 4, 4, 4, 4, 4, 4,
 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
 9, 9, 9, 9, 9, 9,19, 9, 9, 9, 9, 9, 9, 9,19, 9,
19,19,19,19,19,19, 4,19, 4, 4, 4, 4, 9, 9,19, 4,
 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4,
 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
 4,14, 4,23, 4,15, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4,
 4, 4, 4, 4, 4, 4, 4, 4, 4,10, 4, 4, 4, 4, 4, 4};

static const uint8_t cc_xy_cb[0x100] = {
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23};

/* extra cycles if jr/jp/call taken and 'interrupt latency' on rst 0-7 */
static const uint8_t cc_ex[0x100] = {
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* DJNZ */
 5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,	/* JR NZ/JR Z */
 5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,	/* JR NC/JR C */
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0,	/* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2};

/** @brief read an opcode byte from memory */
static __inline uint8_t RD_OP(z80_cpu_t *cpu)
{
	uint8_t data = RD_MEM(dPC);
	PC++;
	return data;
}

/** @brief read an argument byte from memory */
static __inline uint8_t RD_ARGB(z80_cpu_t *cpu)
{
	uint8_t data = RD_MEM(dPC);
	PC++;
	return data;
}

/** @brief read an argument word from memory */
static __inline uint16_t RD_ARGW(z80_cpu_t *cpu)
{
	uint16_t data = RD_MEM(dPC);
	PC++;
	data = data + 256 * RD_MEM(dPC);
	PC++;
	return data;
}

#define	F_SZ(val) (0 == val ? ZF : (val & 0x80 ? SF : 0)) | (val & (YF | XF))

/** @brief sign and zero flags (used by ADD, ADC, SUB, SBC and CP) */
static const uint8_t flags_sz[256] = {
	F_SZ(0x00), F_SZ(0x01), F_SZ(0x02), F_SZ(0x03),
	F_SZ(0x04), F_SZ(0x05), F_SZ(0x06), F_SZ(0x07),
	F_SZ(0x08), F_SZ(0x09), F_SZ(0x0a), F_SZ(0x0b),
	F_SZ(0x0c), F_SZ(0x0d), F_SZ(0x0e), F_SZ(0x0f),
	F_SZ(0x10), F_SZ(0x11), F_SZ(0x12), F_SZ(0x13),
	F_SZ(0x14), F_SZ(0x15), F_SZ(0x16), F_SZ(0x17),
	F_SZ(0x18), F_SZ(0x19), F_SZ(0x1a), F_SZ(0x1b),
	F_SZ(0x1c), F_SZ(0x1d), F_SZ(0x1e), F_SZ(0x1f),
	F_SZ(0x20), F_SZ(0x21), F_SZ(0x22), F_SZ(0x23),
	F_SZ(0x24), F_SZ(0x25), F_SZ(0x26), F_SZ(0x27),
	F_SZ(0x28), F_SZ(0x29), F_SZ(0x2a), F_SZ(0x2b),
	F_SZ(0x2c), F_SZ(0x2d), F_SZ(0x2e), F_SZ(0x2f),
	F_SZ(0x30), F_SZ(0x31), F_SZ(0x32), F_SZ(0x33),
	F_SZ(0x34), F_SZ(0x35), F_SZ(0x36), F_SZ(0x37),
	F_SZ(0x38), F_SZ(0x39), F_SZ(0x3a), F_SZ(0x3b),
	F_SZ(0x3c), F_SZ(0x3d), F_SZ(0x3e), F_SZ(0x3f),
	F_SZ(0x40), F_SZ(0x41), F_SZ(0x42), F_SZ(0x43),
	F_SZ(0x44), F_SZ(0x45), F_SZ(0x46), F_SZ(0x47),
	F_SZ(0x48), F_SZ(0x49), F_SZ(0x4a), F_SZ(0x4b),
	F_SZ(0x4c), F_SZ(0x4d), F_SZ(0x4e), F_SZ(0x4f),
	F_SZ(0x50), F_SZ(0x51), F_SZ(0x52), F_SZ(0x53),
	F_SZ(0x54), F_SZ(0x55), F_SZ(0x56), F_SZ(0x57),
	F_SZ(0x58), F_SZ(0x59), F_SZ(0x5a), F_SZ(0x5b),
	F_SZ(0x5c), F_SZ(0x5d), F_SZ(0x5e), F_SZ(0x5f),
	F_SZ(0x60), F_SZ(0x61), F_SZ(0x62), F_SZ(0x63),
	F_SZ(0x64), F_SZ(0x65), F_SZ(0x66), F_SZ(0x67),
	F_SZ(0x68), F_SZ(0x69), F_SZ(0x6a), F_SZ(0x6b),
	F_SZ(0x6c), F_SZ(0x6d), F_SZ(0x6e), F_SZ(0x6f),
	F_SZ(0x70), F_SZ(0x71), F_SZ(0x72), F_SZ(0x73),
	F_SZ(0x74), F_SZ(0x75), F_SZ(0x76), F_SZ(0x77),
	F_SZ(0x78), F_SZ(0x79), F_SZ(0x7a), F_SZ(0x7b),
	F_SZ(0x7c), F_SZ(0x7d), F_SZ(0x7e), F_SZ(0x7f),
	F_SZ(0x80), F_SZ(0x81), F_SZ(0x82), F_SZ(0x83),
	F_SZ(0x84), F_SZ(0x85), F_SZ(0x86), F_SZ(0x87),
	F_SZ(0x88), F_SZ(0x89), F_SZ(0x8a), F_SZ(0x8b),
	F_SZ(0x8c), F_SZ(0x8d), F_SZ(0x8e), F_SZ(0x8f),
	F_SZ(0x90), F_SZ(0x91), F_SZ(0x92), F_SZ(0x93),
	F_SZ(0x94), F_SZ(0x95), F_SZ(0x96), F_SZ(0x97),
	F_SZ(0x98), F_SZ(0x99), F_SZ(0x9a), F_SZ(0x9b),
	F_SZ(0x9c), F_SZ(0x9d), F_SZ(0x9e), F_SZ(0x9f),
	F_SZ(0xa0), F_SZ(0xa1), F_SZ(0xa2), F_SZ(0xa3),
	F_SZ(0xa4), F_SZ(0xa5), F_SZ(0xa6), F_SZ(0xa7),
	F_SZ(0xa8), F_SZ(0xa9), F_SZ(0xaa), F_SZ(0xab),
	F_SZ(0xac), F_SZ(0xad), F_SZ(0xae), F_SZ(0xaf),
	F_SZ(0xb0), F_SZ(0xb1), F_SZ(0xb2), F_SZ(0xb3),
	F_SZ(0xb4), F_SZ(0xb5), F_SZ(0xb6), F_SZ(0xb7),
	F_SZ(0xb8), F_SZ(0xb9), F_SZ(0xba), F_SZ(0xbb),
	F_SZ(0xbc), F_SZ(0xbd), F_SZ(0xbe), F_SZ(0xbf),
	F_SZ(0xc0), F_SZ(0xc1), F_SZ(0xc2), F_SZ(0xc3),
	F_SZ(0xc4), F_SZ(0xc5), F_SZ(0xc6), F_SZ(0xc7),
	F_SZ(0xc8), F_SZ(0xc9), F_SZ(0xca), F_SZ(0xcb),
	F_SZ(0xcc), F_SZ(0xcd), F_SZ(0xce), F_SZ(0xcf),
	F_SZ(0xd0), F_SZ(0xd1), F_SZ(0xd2), F_SZ(0xd3),
	F_SZ(0xd4), F_SZ(0xd5), F_SZ(0xd6), F_SZ(0xd7),
	F_SZ(0xd8), F_SZ(0xd9), F_SZ(0xda), F_SZ(0xdb),
	F_SZ(0xdc), F_SZ(0xdd), F_SZ(0xde), F_SZ(0xdf),
	F_SZ(0xe0), F_SZ(0xe1), F_SZ(0xe2), F_SZ(0xe3),
	F_SZ(0xe4), F_SZ(0xe5), F_SZ(0xe6), F_SZ(0xe7),
	F_SZ(0xe8), F_SZ(0xe9), F_SZ(0xea), F_SZ(0xeb),
	F_SZ(0xec), F_SZ(0xed), F_SZ(0xee), F_SZ(0xef),
	F_SZ(0xf0), F_SZ(0xf1), F_SZ(0xf2), F_SZ(0xf3),
	F_SZ(0xf4), F_SZ(0xf5), F_SZ(0xf6), F_SZ(0xf7),
	F_SZ(0xf8), F_SZ(0xf9), F_SZ(0xfa), F_SZ(0xfb),
	F_SZ(0xfc), F_SZ(0xfd), F_SZ(0xfe), F_SZ(0xff)
};

#define	F_SZ_BIT(val) (val == 0 ? ZF | PF | HF : (val & 0x80) ? SF | HF : HF)

/** @brief sign and zero flags (used by BIT) */
static const uint8_t flags_sz_bit[256] = {
	F_SZ_BIT(0x00), F_SZ_BIT(0x01), F_SZ_BIT(0x02), F_SZ_BIT(0x03),
	F_SZ_BIT(0x04), F_SZ_BIT(0x05), F_SZ_BIT(0x06), F_SZ_BIT(0x07),
	F_SZ_BIT(0x08), F_SZ_BIT(0x09), F_SZ_BIT(0x0a), F_SZ_BIT(0x0b),
	F_SZ_BIT(0x0c), F_SZ_BIT(0x0d), F_SZ_BIT(0x0e), F_SZ_BIT(0x0f),
	F_SZ_BIT(0x10), F_SZ_BIT(0x11), F_SZ_BIT(0x12), F_SZ_BIT(0x13),
	F_SZ_BIT(0x14), F_SZ_BIT(0x15), F_SZ_BIT(0x16), F_SZ_BIT(0x17),
	F_SZ_BIT(0x18), F_SZ_BIT(0x19), F_SZ_BIT(0x1a), F_SZ_BIT(0x1b),
	F_SZ_BIT(0x1c), F_SZ_BIT(0x1d), F_SZ_BIT(0x1e), F_SZ_BIT(0x1f),
	F_SZ_BIT(0x20), F_SZ_BIT(0x21), F_SZ_BIT(0x22), F_SZ_BIT(0x23),
	F_SZ_BIT(0x24), F_SZ_BIT(0x25), F_SZ_BIT(0x26), F_SZ_BIT(0x27),
	F_SZ_BIT(0x28), F_SZ_BIT(0x29), F_SZ_BIT(0x2a), F_SZ_BIT(0x2b),
	F_SZ_BIT(0x2c), F_SZ_BIT(0x2d), F_SZ_BIT(0x2e), F_SZ_BIT(0x2f),
	F_SZ_BIT(0x30), F_SZ_BIT(0x31), F_SZ_BIT(0x32), F_SZ_BIT(0x33),
	F_SZ_BIT(0x34), F_SZ_BIT(0x35), F_SZ_BIT(0x36), F_SZ_BIT(0x37),
	F_SZ_BIT(0x38), F_SZ_BIT(0x39), F_SZ_BIT(0x3a), F_SZ_BIT(0x3b),
	F_SZ_BIT(0x3c), F_SZ_BIT(0x3d), F_SZ_BIT(0x3e), F_SZ_BIT(0x3f),
	F_SZ_BIT(0x40), F_SZ_BIT(0x41), F_SZ_BIT(0x42), F_SZ_BIT(0x43),
	F_SZ_BIT(0x44), F_SZ_BIT(0x45), F_SZ_BIT(0x46), F_SZ_BIT(0x47),
	F_SZ_BIT(0x48), F_SZ_BIT(0x49), F_SZ_BIT(0x4a), F_SZ_BIT(0x4b),
	F_SZ_BIT(0x4c), F_SZ_BIT(0x4d), F_SZ_BIT(0x4e), F_SZ_BIT(0x4f),
	F_SZ_BIT(0x50), F_SZ_BIT(0x51), F_SZ_BIT(0x52), F_SZ_BIT(0x53),
	F_SZ_BIT(0x54), F_SZ_BIT(0x55), F_SZ_BIT(0x56), F_SZ_BIT(0x57),
	F_SZ_BIT(0x58), F_SZ_BIT(0x59), F_SZ_BIT(0x5a), F_SZ_BIT(0x5b),
	F_SZ_BIT(0x5c), F_SZ_BIT(0x5d), F_SZ_BIT(0x5e), F_SZ_BIT(0x5f),
	F_SZ_BIT(0x60), F_SZ_BIT(0x61), F_SZ_BIT(0x62), F_SZ_BIT(0x63),
	F_SZ_BIT(0x64), F_SZ_BIT(0x65), F_SZ_BIT(0x66), F_SZ_BIT(0x67),
	F_SZ_BIT(0x68), F_SZ_BIT(0x69), F_SZ_BIT(0x6a), F_SZ_BIT(0x6b),
	F_SZ_BIT(0x6c), F_SZ_BIT(0x6d), F_SZ_BIT(0x6e), F_SZ_BIT(0x6f),
	F_SZ_BIT(0x70), F_SZ_BIT(0x71), F_SZ_BIT(0x72), F_SZ_BIT(0x73),
	F_SZ_BIT(0x74), F_SZ_BIT(0x75), F_SZ_BIT(0x76), F_SZ_BIT(0x77),
	F_SZ_BIT(0x78), F_SZ_BIT(0x79), F_SZ_BIT(0x7a), F_SZ_BIT(0x7b),
	F_SZ_BIT(0x7c), F_SZ_BIT(0x7d), F_SZ_BIT(0x7e), F_SZ_BIT(0x7f),
	F_SZ_BIT(0x80), F_SZ_BIT(0x81), F_SZ_BIT(0x82), F_SZ_BIT(0x83),
	F_SZ_BIT(0x84), F_SZ_BIT(0x85), F_SZ_BIT(0x86), F_SZ_BIT(0x87),
	F_SZ_BIT(0x88), F_SZ_BIT(0x89), F_SZ_BIT(0x8a), F_SZ_BIT(0x8b),
	F_SZ_BIT(0x8c), F_SZ_BIT(0x8d), F_SZ_BIT(0x8e), F_SZ_BIT(0x8f),
	F_SZ_BIT(0x90), F_SZ_BIT(0x91), F_SZ_BIT(0x92), F_SZ_BIT(0x93),
	F_SZ_BIT(0x94), F_SZ_BIT(0x95), F_SZ_BIT(0x96), F_SZ_BIT(0x97),
	F_SZ_BIT(0x98), F_SZ_BIT(0x99), F_SZ_BIT(0x9a), F_SZ_BIT(0x9b),
	F_SZ_BIT(0x9c), F_SZ_BIT(0x9d), F_SZ_BIT(0x9e), F_SZ_BIT(0x9f),
	F_SZ_BIT(0xa0), F_SZ_BIT(0xa1), F_SZ_BIT(0xa2), F_SZ_BIT(0xa3),
	F_SZ_BIT(0xa4), F_SZ_BIT(0xa5), F_SZ_BIT(0xa6), F_SZ_BIT(0xa7),
	F_SZ_BIT(0xa8), F_SZ_BIT(0xa9), F_SZ_BIT(0xaa), F_SZ_BIT(0xab),
	F_SZ_BIT(0xac), F_SZ_BIT(0xad), F_SZ_BIT(0xae), F_SZ_BIT(0xaf),
	F_SZ_BIT(0xb0), F_SZ_BIT(0xb1), F_SZ_BIT(0xb2), F_SZ_BIT(0xb3),
	F_SZ_BIT(0xb4), F_SZ_BIT(0xb5), F_SZ_BIT(0xb6), F_SZ_BIT(0xb7),
	F_SZ_BIT(0xb8), F_SZ_BIT(0xb9), F_SZ_BIT(0xba), F_SZ_BIT(0xbb),
	F_SZ_BIT(0xbc), F_SZ_BIT(0xbd), F_SZ_BIT(0xbe), F_SZ_BIT(0xbf),
	F_SZ_BIT(0xc0), F_SZ_BIT(0xc1), F_SZ_BIT(0xc2), F_SZ_BIT(0xc3),
	F_SZ_BIT(0xc4), F_SZ_BIT(0xc5), F_SZ_BIT(0xc6), F_SZ_BIT(0xc7),
	F_SZ_BIT(0xc8), F_SZ_BIT(0xc9), F_SZ_BIT(0xca), F_SZ_BIT(0xcb),
	F_SZ_BIT(0xcc), F_SZ_BIT(0xcd), F_SZ_BIT(0xce), F_SZ_BIT(0xcf),
	F_SZ_BIT(0xd0), F_SZ_BIT(0xd1), F_SZ_BIT(0xd2), F_SZ_BIT(0xd3),
	F_SZ_BIT(0xd4), F_SZ_BIT(0xd5), F_SZ_BIT(0xd6), F_SZ_BIT(0xd7),
	F_SZ_BIT(0xd8), F_SZ_BIT(0xd9), F_SZ_BIT(0xda), F_SZ_BIT(0xdb),
	F_SZ_BIT(0xdc), F_SZ_BIT(0xdd), F_SZ_BIT(0xde), F_SZ_BIT(0xdf),
	F_SZ_BIT(0xe0), F_SZ_BIT(0xe1), F_SZ_BIT(0xe2), F_SZ_BIT(0xe3),
	F_SZ_BIT(0xe4), F_SZ_BIT(0xe5), F_SZ_BIT(0xe6), F_SZ_BIT(0xe7),
	F_SZ_BIT(0xe8), F_SZ_BIT(0xe9), F_SZ_BIT(0xea), F_SZ_BIT(0xeb),
	F_SZ_BIT(0xec), F_SZ_BIT(0xed), F_SZ_BIT(0xee), F_SZ_BIT(0xef),
	F_SZ_BIT(0xf0), F_SZ_BIT(0xf1), F_SZ_BIT(0xf2), F_SZ_BIT(0xf3),
	F_SZ_BIT(0xf4), F_SZ_BIT(0xf5), F_SZ_BIT(0xf6), F_SZ_BIT(0xf7),
	F_SZ_BIT(0xf8), F_SZ_BIT(0xf9), F_SZ_BIT(0xfa), F_SZ_BIT(0xfb),
	F_SZ_BIT(0xfc), F_SZ_BIT(0xfd), F_SZ_BIT(0xfe), F_SZ_BIT(0xff)
};

#define	F_PARITY(n) ((((n>>7)^(n>>6)^(n>>5)^(n>>4)^(n>>3)^(n>>2)^(n>>1)^n)&1) ? 0 : PF)


/** @brief parity flags (used by INI/IND/INIR/INDR) */
static const uint8_t flags_p[256] = {
	F_PARITY(0x00), F_PARITY(0x01), F_PARITY(0x02), F_PARITY(0x03),
	F_PARITY(0x04), F_PARITY(0x05), F_PARITY(0x06), F_PARITY(0x07),
	F_PARITY(0x08), F_PARITY(0x09), F_PARITY(0x0a), F_PARITY(0x0b),
	F_PARITY(0x0c), F_PARITY(0x0d), F_PARITY(0x0e), F_PARITY(0x0f),
	F_PARITY(0x10), F_PARITY(0x11), F_PARITY(0x12), F_PARITY(0x13),
	F_PARITY(0x14), F_PARITY(0x15), F_PARITY(0x16), F_PARITY(0x17),
	F_PARITY(0x18), F_PARITY(0x19), F_PARITY(0x1a), F_PARITY(0x1b),
	F_PARITY(0x1c), F_PARITY(0x1d), F_PARITY(0x1e), F_PARITY(0x1f),
	F_PARITY(0x20), F_PARITY(0x21), F_PARITY(0x22), F_PARITY(0x23),
	F_PARITY(0x24), F_PARITY(0x25), F_PARITY(0x26), F_PARITY(0x27),
	F_PARITY(0x28), F_PARITY(0x29), F_PARITY(0x2a), F_PARITY(0x2b),
	F_PARITY(0x2c), F_PARITY(0x2d), F_PARITY(0x2e), F_PARITY(0x2f),
	F_PARITY(0x30), F_PARITY(0x31), F_PARITY(0x32), F_PARITY(0x33),
	F_PARITY(0x34), F_PARITY(0x35), F_PARITY(0x36), F_PARITY(0x37),
	F_PARITY(0x38), F_PARITY(0x39), F_PARITY(0x3a), F_PARITY(0x3b),
	F_PARITY(0x3c), F_PARITY(0x3d), F_PARITY(0x3e), F_PARITY(0x3f),
	F_PARITY(0x40), F_PARITY(0x41), F_PARITY(0x42), F_PARITY(0x43),
	F_PARITY(0x44), F_PARITY(0x45), F_PARITY(0x46), F_PARITY(0x47),
	F_PARITY(0x48), F_PARITY(0x49), F_PARITY(0x4a), F_PARITY(0x4b),
	F_PARITY(0x4c), F_PARITY(0x4d), F_PARITY(0x4e), F_PARITY(0x4f),
	F_PARITY(0x50), F_PARITY(0x51), F_PARITY(0x52), F_PARITY(0x53),
	F_PARITY(0x54), F_PARITY(0x55), F_PARITY(0x56), F_PARITY(0x57),
	F_PARITY(0x58), F_PARITY(0x59), F_PARITY(0x5a), F_PARITY(0x5b),
	F_PARITY(0x5c), F_PARITY(0x5d), F_PARITY(0x5e), F_PARITY(0x5f),
	F_PARITY(0x60), F_PARITY(0x61), F_PARITY(0x62), F_PARITY(0x63),
	F_PARITY(0x64), F_PARITY(0x65), F_PARITY(0x66), F_PARITY(0x67),
	F_PARITY(0x68), F_PARITY(0x69), F_PARITY(0x6a), F_PARITY(0x6b),
	F_PARITY(0x6c), F_PARITY(0x6d), F_PARITY(0x6e), F_PARITY(0x6f),
	F_PARITY(0x70), F_PARITY(0x71), F_PARITY(0x72), F_PARITY(0x73),
	F_PARITY(0x74), F_PARITY(0x75), F_PARITY(0x76), F_PARITY(0x77),
	F_PARITY(0x78), F_PARITY(0x79), F_PARITY(0x7a), F_PARITY(0x7b),
	F_PARITY(0x7c), F_PARITY(0x7d), F_PARITY(0x7e), F_PARITY(0x7f),
	F_PARITY(0x80), F_PARITY(0x81), F_PARITY(0x82), F_PARITY(0x83),
	F_PARITY(0x84), F_PARITY(0x85), F_PARITY(0x86), F_PARITY(0x87),
	F_PARITY(0x88), F_PARITY(0x89), F_PARITY(0x8a), F_PARITY(0x8b),
	F_PARITY(0x8c), F_PARITY(0x8d), F_PARITY(0x8e), F_PARITY(0x8f),
	F_PARITY(0x90), F_PARITY(0x91), F_PARITY(0x92), F_PARITY(0x93),
	F_PARITY(0x94), F_PARITY(0x95), F_PARITY(0x96), F_PARITY(0x97),
	F_PARITY(0x98), F_PARITY(0x99), F_PARITY(0x9a), F_PARITY(0x9b),
	F_PARITY(0x9c), F_PARITY(0x9d), F_PARITY(0x9e), F_PARITY(0x9f),
	F_PARITY(0xa0), F_PARITY(0xa1), F_PARITY(0xa2), F_PARITY(0xa3),
	F_PARITY(0xa4), F_PARITY(0xa5), F_PARITY(0xa6), F_PARITY(0xa7),
	F_PARITY(0xa8), F_PARITY(0xa9), F_PARITY(0xaa), F_PARITY(0xab),
	F_PARITY(0xac), F_PARITY(0xad), F_PARITY(0xae), F_PARITY(0xaf),
	F_PARITY(0xb0), F_PARITY(0xb1), F_PARITY(0xb2), F_PARITY(0xb3),
	F_PARITY(0xb4), F_PARITY(0xb5), F_PARITY(0xb6), F_PARITY(0xb7),
	F_PARITY(0xb8), F_PARITY(0xb9), F_PARITY(0xba), F_PARITY(0xbb),
	F_PARITY(0xbc), F_PARITY(0xbd), F_PARITY(0xbe), F_PARITY(0xbf),
	F_PARITY(0xc0), F_PARITY(0xc1), F_PARITY(0xc2), F_PARITY(0xc3),
	F_PARITY(0xc4), F_PARITY(0xc5), F_PARITY(0xc6), F_PARITY(0xc7),
	F_PARITY(0xc8), F_PARITY(0xc9), F_PARITY(0xca), F_PARITY(0xcb),
	F_PARITY(0xcc), F_PARITY(0xcd), F_PARITY(0xce), F_PARITY(0xcf),
	F_PARITY(0xd0), F_PARITY(0xd1), F_PARITY(0xd2), F_PARITY(0xd3),
	F_PARITY(0xd4), F_PARITY(0xd5), F_PARITY(0xd6), F_PARITY(0xd7),
	F_PARITY(0xd8), F_PARITY(0xd9), F_PARITY(0xda), F_PARITY(0xdb),
	F_PARITY(0xdc), F_PARITY(0xdd), F_PARITY(0xde), F_PARITY(0xdf),
	F_PARITY(0xe0), F_PARITY(0xe1), F_PARITY(0xe2), F_PARITY(0xe3),
	F_PARITY(0xe4), F_PARITY(0xe5), F_PARITY(0xe6), F_PARITY(0xe7),
	F_PARITY(0xe8), F_PARITY(0xe9), F_PARITY(0xea), F_PARITY(0xeb),
	F_PARITY(0xec), F_PARITY(0xed), F_PARITY(0xee), F_PARITY(0xef),
	F_PARITY(0xf0), F_PARITY(0xf1), F_PARITY(0xf2), F_PARITY(0xf3),
	F_PARITY(0xf4), F_PARITY(0xf5), F_PARITY(0xf6), F_PARITY(0xf7),
	F_PARITY(0xf8), F_PARITY(0xf9), F_PARITY(0xfa), F_PARITY(0xfb),
	F_PARITY(0xfc), F_PARITY(0xfd), F_PARITY(0xfe), F_PARITY(0xff)
};

#define	F_SZP(val) (F_SZ(val) | F_PARITY(val))

/** @brief sign, zero and parity flags (used by XOR, OR) */
static const uint8_t flags_szp[256] = {
	F_SZP(0x00), F_SZP(0x01), F_SZP(0x02), F_SZP(0x03),
	F_SZP(0x04), F_SZP(0x05), F_SZP(0x06), F_SZP(0x07),
	F_SZP(0x08), F_SZP(0x09), F_SZP(0x0a), F_SZP(0x0b),
	F_SZP(0x0c), F_SZP(0x0d), F_SZP(0x0e), F_SZP(0x0f),
	F_SZP(0x10), F_SZP(0x11), F_SZP(0x12), F_SZP(0x13),
	F_SZP(0x14), F_SZP(0x15), F_SZP(0x16), F_SZP(0x17),
	F_SZP(0x18), F_SZP(0x19), F_SZP(0x1a), F_SZP(0x1b),
	F_SZP(0x1c), F_SZP(0x1d), F_SZP(0x1e), F_SZP(0x1f),
	F_SZP(0x20), F_SZP(0x21), F_SZP(0x22), F_SZP(0x23),
	F_SZP(0x24), F_SZP(0x25), F_SZP(0x26), F_SZP(0x27),
	F_SZP(0x28), F_SZP(0x29), F_SZP(0x2a), F_SZP(0x2b),
	F_SZP(0x2c), F_SZP(0x2d), F_SZP(0x2e), F_SZP(0x2f),
	F_SZP(0x30), F_SZP(0x31), F_SZP(0x32), F_SZP(0x33),
	F_SZP(0x34), F_SZP(0x35), F_SZP(0x36), F_SZP(0x37),
	F_SZP(0x38), F_SZP(0x39), F_SZP(0x3a), F_SZP(0x3b),
	F_SZP(0x3c), F_SZP(0x3d), F_SZP(0x3e), F_SZP(0x3f),
	F_SZP(0x40), F_SZP(0x41), F_SZP(0x42), F_SZP(0x43),
	F_SZP(0x44), F_SZP(0x45), F_SZP(0x46), F_SZP(0x47),
	F_SZP(0x48), F_SZP(0x49), F_SZP(0x4a), F_SZP(0x4b),
	F_SZP(0x4c), F_SZP(0x4d), F_SZP(0x4e), F_SZP(0x4f),
	F_SZP(0x50), F_SZP(0x51), F_SZP(0x52), F_SZP(0x53),
	F_SZP(0x54), F_SZP(0x55), F_SZP(0x56), F_SZP(0x57),
	F_SZP(0x58), F_SZP(0x59), F_SZP(0x5a), F_SZP(0x5b),
	F_SZP(0x5c), F_SZP(0x5d), F_SZP(0x5e), F_SZP(0x5f),
	F_SZP(0x60), F_SZP(0x61), F_SZP(0x62), F_SZP(0x63),
	F_SZP(0x64), F_SZP(0x65), F_SZP(0x66), F_SZP(0x67),
	F_SZP(0x68), F_SZP(0x69), F_SZP(0x6a), F_SZP(0x6b),
	F_SZP(0x6c), F_SZP(0x6d), F_SZP(0x6e), F_SZP(0x6f),
	F_SZP(0x70), F_SZP(0x71), F_SZP(0x72), F_SZP(0x73),
	F_SZP(0x74), F_SZP(0x75), F_SZP(0x76), F_SZP(0x77),
	F_SZP(0x78), F_SZP(0x79), F_SZP(0x7a), F_SZP(0x7b),
	F_SZP(0x7c), F_SZP(0x7d), F_SZP(0x7e), F_SZP(0x7f),
	F_SZP(0x80), F_SZP(0x81), F_SZP(0x82), F_SZP(0x83),
	F_SZP(0x84), F_SZP(0x85), F_SZP(0x86), F_SZP(0x87),
	F_SZP(0x88), F_SZP(0x89), F_SZP(0x8a), F_SZP(0x8b),
	F_SZP(0x8c), F_SZP(0x8d), F_SZP(0x8e), F_SZP(0x8f),
	F_SZP(0x90), F_SZP(0x91), F_SZP(0x92), F_SZP(0x93),
	F_SZP(0x94), F_SZP(0x95), F_SZP(0x96), F_SZP(0x97),
	F_SZP(0x98), F_SZP(0x99), F_SZP(0x9a), F_SZP(0x9b),
	F_SZP(0x9c), F_SZP(0x9d), F_SZP(0x9e), F_SZP(0x9f),
	F_SZP(0xa0), F_SZP(0xa1), F_SZP(0xa2), F_SZP(0xa3),
	F_SZP(0xa4), F_SZP(0xa5), F_SZP(0xa6), F_SZP(0xa7),
	F_SZP(0xa8), F_SZP(0xa9), F_SZP(0xaa), F_SZP(0xab),
	F_SZP(0xac), F_SZP(0xad), F_SZP(0xae), F_SZP(0xaf),
	F_SZP(0xb0), F_SZP(0xb1), F_SZP(0xb2), F_SZP(0xb3),
	F_SZP(0xb4), F_SZP(0xb5), F_SZP(0xb6), F_SZP(0xb7),
	F_SZP(0xb8), F_SZP(0xb9), F_SZP(0xba), F_SZP(0xbb),
	F_SZP(0xbc), F_SZP(0xbd), F_SZP(0xbe), F_SZP(0xbf),
	F_SZP(0xc0), F_SZP(0xc1), F_SZP(0xc2), F_SZP(0xc3),
	F_SZP(0xc4), F_SZP(0xc5), F_SZP(0xc6), F_SZP(0xc7),
	F_SZP(0xc8), F_SZP(0xc9), F_SZP(0xca), F_SZP(0xcb),
	F_SZP(0xcc), F_SZP(0xcd), F_SZP(0xce), F_SZP(0xcf),
	F_SZP(0xd0), F_SZP(0xd1), F_SZP(0xd2), F_SZP(0xd3),
	F_SZP(0xd4), F_SZP(0xd5), F_SZP(0xd6), F_SZP(0xd7),
	F_SZP(0xd8), F_SZP(0xd9), F_SZP(0xda), F_SZP(0xdb),
	F_SZP(0xdc), F_SZP(0xdd), F_SZP(0xde), F_SZP(0xdf),
	F_SZP(0xe0), F_SZP(0xe1), F_SZP(0xe2), F_SZP(0xe3),
	F_SZP(0xe4), F_SZP(0xe5), F_SZP(0xe6), F_SZP(0xe7),
	F_SZP(0xe8), F_SZP(0xe9), F_SZP(0xea), F_SZP(0xeb),
	F_SZP(0xec), F_SZP(0xed), F_SZP(0xee), F_SZP(0xef),
	F_SZP(0xf0), F_SZP(0xf1), F_SZP(0xf2), F_SZP(0xf3),
	F_SZP(0xf4), F_SZP(0xf5), F_SZP(0xf6), F_SZP(0xf7),
	F_SZP(0xf8), F_SZP(0xf9), F_SZP(0xfa), F_SZP(0xfb),
	F_SZP(0xfc), F_SZP(0xfd), F_SZP(0xfe), F_SZP(0xff)
};

#define	F_SZPH(val) (F_SZP(val) | HF)

/** @brief sign, zero and parity flags; half carry flag set (used by AND) */
static const uint8_t flags_szph[256] = {
	F_SZPH(0x00), F_SZPH(0x01), F_SZPH(0x02), F_SZPH(0x03),
	F_SZPH(0x04), F_SZPH(0x05), F_SZPH(0x06), F_SZPH(0x07),
	F_SZPH(0x08), F_SZPH(0x09), F_SZPH(0x0a), F_SZPH(0x0b),
	F_SZPH(0x0c), F_SZPH(0x0d), F_SZPH(0x0e), F_SZPH(0x0f),
	F_SZPH(0x10), F_SZPH(0x11), F_SZPH(0x12), F_SZPH(0x13),
	F_SZPH(0x14), F_SZPH(0x15), F_SZPH(0x16), F_SZPH(0x17),
	F_SZPH(0x18), F_SZPH(0x19), F_SZPH(0x1a), F_SZPH(0x1b),
	F_SZPH(0x1c), F_SZPH(0x1d), F_SZPH(0x1e), F_SZPH(0x1f),
	F_SZPH(0x20), F_SZPH(0x21), F_SZPH(0x22), F_SZPH(0x23),
	F_SZPH(0x24), F_SZPH(0x25), F_SZPH(0x26), F_SZPH(0x27),
	F_SZPH(0x28), F_SZPH(0x29), F_SZPH(0x2a), F_SZPH(0x2b),
	F_SZPH(0x2c), F_SZPH(0x2d), F_SZPH(0x2e), F_SZPH(0x2f),
	F_SZPH(0x30), F_SZPH(0x31), F_SZPH(0x32), F_SZPH(0x33),
	F_SZPH(0x34), F_SZPH(0x35), F_SZPH(0x36), F_SZPH(0x37),
	F_SZPH(0x38), F_SZPH(0x39), F_SZPH(0x3a), F_SZPH(0x3b),
	F_SZPH(0x3c), F_SZPH(0x3d), F_SZPH(0x3e), F_SZPH(0x3f),
	F_SZPH(0x40), F_SZPH(0x41), F_SZPH(0x42), F_SZPH(0x43),
	F_SZPH(0x44), F_SZPH(0x45), F_SZPH(0x46), F_SZPH(0x47),
	F_SZPH(0x48), F_SZPH(0x49), F_SZPH(0x4a), F_SZPH(0x4b),
	F_SZPH(0x4c), F_SZPH(0x4d), F_SZPH(0x4e), F_SZPH(0x4f),
	F_SZPH(0x50), F_SZPH(0x51), F_SZPH(0x52), F_SZPH(0x53),
	F_SZPH(0x54), F_SZPH(0x55), F_SZPH(0x56), F_SZPH(0x57),
	F_SZPH(0x58), F_SZPH(0x59), F_SZPH(0x5a), F_SZPH(0x5b),
	F_SZPH(0x5c), F_SZPH(0x5d), F_SZPH(0x5e), F_SZPH(0x5f),
	F_SZPH(0x60), F_SZPH(0x61), F_SZPH(0x62), F_SZPH(0x63),
	F_SZPH(0x64), F_SZPH(0x65), F_SZPH(0x66), F_SZPH(0x67),
	F_SZPH(0x68), F_SZPH(0x69), F_SZPH(0x6a), F_SZPH(0x6b),
	F_SZPH(0x6c), F_SZPH(0x6d), F_SZPH(0x6e), F_SZPH(0x6f),
	F_SZPH(0x70), F_SZPH(0x71), F_SZPH(0x72), F_SZPH(0x73),
	F_SZPH(0x74), F_SZPH(0x75), F_SZPH(0x76), F_SZPH(0x77),
	F_SZPH(0x78), F_SZPH(0x79), F_SZPH(0x7a), F_SZPH(0x7b),
	F_SZPH(0x7c), F_SZPH(0x7d), F_SZPH(0x7e), F_SZPH(0x7f),
	F_SZPH(0x80), F_SZPH(0x81), F_SZPH(0x82), F_SZPH(0x83),
	F_SZPH(0x84), F_SZPH(0x85), F_SZPH(0x86), F_SZPH(0x87),
	F_SZPH(0x88), F_SZPH(0x89), F_SZPH(0x8a), F_SZPH(0x8b),
	F_SZPH(0x8c), F_SZPH(0x8d), F_SZPH(0x8e), F_SZPH(0x8f),
	F_SZPH(0x90), F_SZPH(0x91), F_SZPH(0x92), F_SZPH(0x93),
	F_SZPH(0x94), F_SZPH(0x95), F_SZPH(0x96), F_SZPH(0x97),
	F_SZPH(0x98), F_SZPH(0x99), F_SZPH(0x9a), F_SZPH(0x9b),
	F_SZPH(0x9c), F_SZPH(0x9d), F_SZPH(0x9e), F_SZPH(0x9f),
	F_SZPH(0xa0), F_SZPH(0xa1), F_SZPH(0xa2), F_SZPH(0xa3),
	F_SZPH(0xa4), F_SZPH(0xa5), F_SZPH(0xa6), F_SZPH(0xa7),
	F_SZPH(0xa8), F_SZPH(0xa9), F_SZPH(0xaa), F_SZPH(0xab),
	F_SZPH(0xac), F_SZPH(0xad), F_SZPH(0xae), F_SZPH(0xaf),
	F_SZPH(0xb0), F_SZPH(0xb1), F_SZPH(0xb2), F_SZPH(0xb3),
	F_SZPH(0xb4), F_SZPH(0xb5), F_SZPH(0xb6), F_SZPH(0xb7),
	F_SZPH(0xb8), F_SZPH(0xb9), F_SZPH(0xba), F_SZPH(0xbb),
	F_SZPH(0xbc), F_SZPH(0xbd), F_SZPH(0xbe), F_SZPH(0xbf),
	F_SZPH(0xc0), F_SZPH(0xc1), F_SZPH(0xc2), F_SZPH(0xc3),
	F_SZPH(0xc4), F_SZPH(0xc5), F_SZPH(0xc6), F_SZPH(0xc7),
	F_SZPH(0xc8), F_SZPH(0xc9), F_SZPH(0xca), F_SZPH(0xcb),
	F_SZPH(0xcc), F_SZPH(0xcd), F_SZPH(0xce), F_SZPH(0xcf),
	F_SZPH(0xd0), F_SZPH(0xd1), F_SZPH(0xd2), F_SZPH(0xd3),
	F_SZPH(0xd4), F_SZPH(0xd5), F_SZPH(0xd6), F_SZPH(0xd7),
	F_SZPH(0xd8), F_SZPH(0xd9), F_SZPH(0xda), F_SZPH(0xdb),
	F_SZPH(0xdc), F_SZPH(0xdd), F_SZPH(0xde), F_SZPH(0xdf),
	F_SZPH(0xe0), F_SZPH(0xe1), F_SZPH(0xe2), F_SZPH(0xe3),
	F_SZPH(0xe4), F_SZPH(0xe5), F_SZPH(0xe6), F_SZPH(0xe7),
	F_SZPH(0xe8), F_SZPH(0xe9), F_SZPH(0xea), F_SZPH(0xeb),
	F_SZPH(0xec), F_SZPH(0xed), F_SZPH(0xee), F_SZPH(0xef),
	F_SZPH(0xf0), F_SZPH(0xf1), F_SZPH(0xf2), F_SZPH(0xf3),
	F_SZPH(0xf4), F_SZPH(0xf5), F_SZPH(0xf6), F_SZPH(0xf7),
	F_SZPH(0xf8), F_SZPH(0xf9), F_SZPH(0xfa), F_SZPH(0xfb),
	F_SZPH(0xfc), F_SZPH(0xfd), F_SZPH(0xfe), F_SZPH(0xff)
};

#define	F_SZHV_INC(val)	(F_SZ(val) | (val == 0x80 ? PF : 0) | ((val & 0xf) == 0x0 ? HF : 0))

/** @brief increment r8 flags */
static const uint8_t flags_szhf_inc[256] = {
	F_SZHV_INC(0x00), F_SZHV_INC(0x01), F_SZHV_INC(0x02), F_SZHV_INC(0x03),
	F_SZHV_INC(0x04), F_SZHV_INC(0x05), F_SZHV_INC(0x06), F_SZHV_INC(0x07),
	F_SZHV_INC(0x08), F_SZHV_INC(0x09), F_SZHV_INC(0x0a), F_SZHV_INC(0x0b),
	F_SZHV_INC(0x0c), F_SZHV_INC(0x0d), F_SZHV_INC(0x0e), F_SZHV_INC(0x0f),
	F_SZHV_INC(0x10), F_SZHV_INC(0x11), F_SZHV_INC(0x12), F_SZHV_INC(0x13),
	F_SZHV_INC(0x14), F_SZHV_INC(0x15), F_SZHV_INC(0x16), F_SZHV_INC(0x17),
	F_SZHV_INC(0x18), F_SZHV_INC(0x19), F_SZHV_INC(0x1a), F_SZHV_INC(0x1b),
	F_SZHV_INC(0x1c), F_SZHV_INC(0x1d), F_SZHV_INC(0x1e), F_SZHV_INC(0x1f),
	F_SZHV_INC(0x20), F_SZHV_INC(0x21), F_SZHV_INC(0x22), F_SZHV_INC(0x23),
	F_SZHV_INC(0x24), F_SZHV_INC(0x25), F_SZHV_INC(0x26), F_SZHV_INC(0x27),
	F_SZHV_INC(0x28), F_SZHV_INC(0x29), F_SZHV_INC(0x2a), F_SZHV_INC(0x2b),
	F_SZHV_INC(0x2c), F_SZHV_INC(0x2d), F_SZHV_INC(0x2e), F_SZHV_INC(0x2f),
	F_SZHV_INC(0x30), F_SZHV_INC(0x31), F_SZHV_INC(0x32), F_SZHV_INC(0x33),
	F_SZHV_INC(0x34), F_SZHV_INC(0x35), F_SZHV_INC(0x36), F_SZHV_INC(0x37),
	F_SZHV_INC(0x38), F_SZHV_INC(0x39), F_SZHV_INC(0x3a), F_SZHV_INC(0x3b),
	F_SZHV_INC(0x3c), F_SZHV_INC(0x3d), F_SZHV_INC(0x3e), F_SZHV_INC(0x3f),
	F_SZHV_INC(0x40), F_SZHV_INC(0x41), F_SZHV_INC(0x42), F_SZHV_INC(0x43),
	F_SZHV_INC(0x44), F_SZHV_INC(0x45), F_SZHV_INC(0x46), F_SZHV_INC(0x47),
	F_SZHV_INC(0x48), F_SZHV_INC(0x49), F_SZHV_INC(0x4a), F_SZHV_INC(0x4b),
	F_SZHV_INC(0x4c), F_SZHV_INC(0x4d), F_SZHV_INC(0x4e), F_SZHV_INC(0x4f),
	F_SZHV_INC(0x50), F_SZHV_INC(0x51), F_SZHV_INC(0x52), F_SZHV_INC(0x53),
	F_SZHV_INC(0x54), F_SZHV_INC(0x55), F_SZHV_INC(0x56), F_SZHV_INC(0x57),
	F_SZHV_INC(0x58), F_SZHV_INC(0x59), F_SZHV_INC(0x5a), F_SZHV_INC(0x5b),
	F_SZHV_INC(0x5c), F_SZHV_INC(0x5d), F_SZHV_INC(0x5e), F_SZHV_INC(0x5f),
	F_SZHV_INC(0x60), F_SZHV_INC(0x61), F_SZHV_INC(0x62), F_SZHV_INC(0x63),
	F_SZHV_INC(0x64), F_SZHV_INC(0x65), F_SZHV_INC(0x66), F_SZHV_INC(0x67),
	F_SZHV_INC(0x68), F_SZHV_INC(0x69), F_SZHV_INC(0x6a), F_SZHV_INC(0x6b),
	F_SZHV_INC(0x6c), F_SZHV_INC(0x6d), F_SZHV_INC(0x6e), F_SZHV_INC(0x6f),
	F_SZHV_INC(0x70), F_SZHV_INC(0x71), F_SZHV_INC(0x72), F_SZHV_INC(0x73),
	F_SZHV_INC(0x74), F_SZHV_INC(0x75), F_SZHV_INC(0x76), F_SZHV_INC(0x77),
	F_SZHV_INC(0x78), F_SZHV_INC(0x79), F_SZHV_INC(0x7a), F_SZHV_INC(0x7b),
	F_SZHV_INC(0x7c), F_SZHV_INC(0x7d), F_SZHV_INC(0x7e), F_SZHV_INC(0x7f),
	F_SZHV_INC(0x80), F_SZHV_INC(0x81), F_SZHV_INC(0x82), F_SZHV_INC(0x83),
	F_SZHV_INC(0x84), F_SZHV_INC(0x85), F_SZHV_INC(0x86), F_SZHV_INC(0x87),
	F_SZHV_INC(0x88), F_SZHV_INC(0x89), F_SZHV_INC(0x8a), F_SZHV_INC(0x8b),
	F_SZHV_INC(0x8c), F_SZHV_INC(0x8d), F_SZHV_INC(0x8e), F_SZHV_INC(0x8f),
	F_SZHV_INC(0x90), F_SZHV_INC(0x91), F_SZHV_INC(0x92), F_SZHV_INC(0x93),
	F_SZHV_INC(0x94), F_SZHV_INC(0x95), F_SZHV_INC(0x96), F_SZHV_INC(0x97),
	F_SZHV_INC(0x98), F_SZHV_INC(0x99), F_SZHV_INC(0x9a), F_SZHV_INC(0x9b),
	F_SZHV_INC(0x9c), F_SZHV_INC(0x9d), F_SZHV_INC(0x9e), F_SZHV_INC(0x9f),
	F_SZHV_INC(0xa0), F_SZHV_INC(0xa1), F_SZHV_INC(0xa2), F_SZHV_INC(0xa3),
	F_SZHV_INC(0xa4), F_SZHV_INC(0xa5), F_SZHV_INC(0xa6), F_SZHV_INC(0xa7),
	F_SZHV_INC(0xa8), F_SZHV_INC(0xa9), F_SZHV_INC(0xaa), F_SZHV_INC(0xab),
	F_SZHV_INC(0xac), F_SZHV_INC(0xad), F_SZHV_INC(0xae), F_SZHV_INC(0xaf),
	F_SZHV_INC(0xb0), F_SZHV_INC(0xb1), F_SZHV_INC(0xb2), F_SZHV_INC(0xb3),
	F_SZHV_INC(0xb4), F_SZHV_INC(0xb5), F_SZHV_INC(0xb6), F_SZHV_INC(0xb7),
	F_SZHV_INC(0xb8), F_SZHV_INC(0xb9), F_SZHV_INC(0xba), F_SZHV_INC(0xbb),
	F_SZHV_INC(0xbc), F_SZHV_INC(0xbd), F_SZHV_INC(0xbe), F_SZHV_INC(0xbf),
	F_SZHV_INC(0xc0), F_SZHV_INC(0xc1), F_SZHV_INC(0xc2), F_SZHV_INC(0xc3),
	F_SZHV_INC(0xc4), F_SZHV_INC(0xc5), F_SZHV_INC(0xc6), F_SZHV_INC(0xc7),
	F_SZHV_INC(0xc8), F_SZHV_INC(0xc9), F_SZHV_INC(0xca), F_SZHV_INC(0xcb),
	F_SZHV_INC(0xcc), F_SZHV_INC(0xcd), F_SZHV_INC(0xce), F_SZHV_INC(0xcf),
	F_SZHV_INC(0xd0), F_SZHV_INC(0xd1), F_SZHV_INC(0xd2), F_SZHV_INC(0xd3),
	F_SZHV_INC(0xd4), F_SZHV_INC(0xd5), F_SZHV_INC(0xd6), F_SZHV_INC(0xd7),
	F_SZHV_INC(0xd8), F_SZHV_INC(0xd9), F_SZHV_INC(0xda), F_SZHV_INC(0xdb),
	F_SZHV_INC(0xdc), F_SZHV_INC(0xdd), F_SZHV_INC(0xde), F_SZHV_INC(0xdf),
	F_SZHV_INC(0xe0), F_SZHV_INC(0xe1), F_SZHV_INC(0xe2), F_SZHV_INC(0xe3),
	F_SZHV_INC(0xe4), F_SZHV_INC(0xe5), F_SZHV_INC(0xe6), F_SZHV_INC(0xe7),
	F_SZHV_INC(0xe8), F_SZHV_INC(0xe9), F_SZHV_INC(0xea), F_SZHV_INC(0xeb),
	F_SZHV_INC(0xec), F_SZHV_INC(0xed), F_SZHV_INC(0xee), F_SZHV_INC(0xef),
	F_SZHV_INC(0xf0), F_SZHV_INC(0xf1), F_SZHV_INC(0xf2), F_SZHV_INC(0xf3),
	F_SZHV_INC(0xf4), F_SZHV_INC(0xf5), F_SZHV_INC(0xf6), F_SZHV_INC(0xf7),
	F_SZHV_INC(0xf8), F_SZHV_INC(0xf9), F_SZHV_INC(0xfa), F_SZHV_INC(0xfb),
	F_SZHV_INC(0xfc), F_SZHV_INC(0xfd), F_SZHV_INC(0xfe), F_SZHV_INC(0xff)
};

#define	F_SZHV_DEC(val)	(F_SZ(val) | (val == 0x7f ? PF : 0) | ((val & 0xf) == 0xf ? HF : 0))

/** @brief decrement r8 flags */
static const uint8_t flags_szhf_dec[256] = {
	F_SZHV_DEC(0x00), F_SZHV_DEC(0x01), F_SZHV_DEC(0x02), F_SZHV_DEC(0x03),
	F_SZHV_DEC(0x04), F_SZHV_DEC(0x05), F_SZHV_DEC(0x06), F_SZHV_DEC(0x07),
	F_SZHV_DEC(0x08), F_SZHV_DEC(0x09), F_SZHV_DEC(0x0a), F_SZHV_DEC(0x0b),
	F_SZHV_DEC(0x0c), F_SZHV_DEC(0x0d), F_SZHV_DEC(0x0e), F_SZHV_DEC(0x0f),
	F_SZHV_DEC(0x10), F_SZHV_DEC(0x11), F_SZHV_DEC(0x12), F_SZHV_DEC(0x13),
	F_SZHV_DEC(0x14), F_SZHV_DEC(0x15), F_SZHV_DEC(0x16), F_SZHV_DEC(0x17),
	F_SZHV_DEC(0x18), F_SZHV_DEC(0x19), F_SZHV_DEC(0x1a), F_SZHV_DEC(0x1b),
	F_SZHV_DEC(0x1c), F_SZHV_DEC(0x1d), F_SZHV_DEC(0x1e), F_SZHV_DEC(0x1f),
	F_SZHV_DEC(0x20), F_SZHV_DEC(0x21), F_SZHV_DEC(0x22), F_SZHV_DEC(0x23),
	F_SZHV_DEC(0x24), F_SZHV_DEC(0x25), F_SZHV_DEC(0x26), F_SZHV_DEC(0x27),
	F_SZHV_DEC(0x28), F_SZHV_DEC(0x29), F_SZHV_DEC(0x2a), F_SZHV_DEC(0x2b),
	F_SZHV_DEC(0x2c), F_SZHV_DEC(0x2d), F_SZHV_DEC(0x2e), F_SZHV_DEC(0x2f),
	F_SZHV_DEC(0x30), F_SZHV_DEC(0x31), F_SZHV_DEC(0x32), F_SZHV_DEC(0x33),
	F_SZHV_DEC(0x34), F_SZHV_DEC(0x35), F_SZHV_DEC(0x36), F_SZHV_DEC(0x37),
	F_SZHV_DEC(0x38), F_SZHV_DEC(0x39), F_SZHV_DEC(0x3a), F_SZHV_DEC(0x3b),
	F_SZHV_DEC(0x3c), F_SZHV_DEC(0x3d), F_SZHV_DEC(0x3e), F_SZHV_DEC(0x3f),
	F_SZHV_DEC(0x40), F_SZHV_DEC(0x41), F_SZHV_DEC(0x42), F_SZHV_DEC(0x43),
	F_SZHV_DEC(0x44), F_SZHV_DEC(0x45), F_SZHV_DEC(0x46), F_SZHV_DEC(0x47),
	F_SZHV_DEC(0x48), F_SZHV_DEC(0x49), F_SZHV_DEC(0x4a), F_SZHV_DEC(0x4b),
	F_SZHV_DEC(0x4c), F_SZHV_DEC(0x4d), F_SZHV_DEC(0x4e), F_SZHV_DEC(0x4f),
	F_SZHV_DEC(0x50), F_SZHV_DEC(0x51), F_SZHV_DEC(0x52), F_SZHV_DEC(0x53),
	F_SZHV_DEC(0x54), F_SZHV_DEC(0x55), F_SZHV_DEC(0x56), F_SZHV_DEC(0x57),
	F_SZHV_DEC(0x58), F_SZHV_DEC(0x59), F_SZHV_DEC(0x5a), F_SZHV_DEC(0x5b),
	F_SZHV_DEC(0x5c), F_SZHV_DEC(0x5d), F_SZHV_DEC(0x5e), F_SZHV_DEC(0x5f),
	F_SZHV_DEC(0x60), F_SZHV_DEC(0x61), F_SZHV_DEC(0x62), F_SZHV_DEC(0x63),
	F_SZHV_DEC(0x64), F_SZHV_DEC(0x65), F_SZHV_DEC(0x66), F_SZHV_DEC(0x67),
	F_SZHV_DEC(0x68), F_SZHV_DEC(0x69), F_SZHV_DEC(0x6a), F_SZHV_DEC(0x6b),
	F_SZHV_DEC(0x6c), F_SZHV_DEC(0x6d), F_SZHV_DEC(0x6e), F_SZHV_DEC(0x6f),
	F_SZHV_DEC(0x70), F_SZHV_DEC(0x71), F_SZHV_DEC(0x72), F_SZHV_DEC(0x73),
	F_SZHV_DEC(0x74), F_SZHV_DEC(0x75), F_SZHV_DEC(0x76), F_SZHV_DEC(0x77),
	F_SZHV_DEC(0x78), F_SZHV_DEC(0x79), F_SZHV_DEC(0x7a), F_SZHV_DEC(0x7b),
	F_SZHV_DEC(0x7c), F_SZHV_DEC(0x7d), F_SZHV_DEC(0x7e), F_SZHV_DEC(0x7f),
	F_SZHV_DEC(0x80), F_SZHV_DEC(0x81), F_SZHV_DEC(0x82), F_SZHV_DEC(0x83),
	F_SZHV_DEC(0x84), F_SZHV_DEC(0x85), F_SZHV_DEC(0x86), F_SZHV_DEC(0x87),
	F_SZHV_DEC(0x88), F_SZHV_DEC(0x89), F_SZHV_DEC(0x8a), F_SZHV_DEC(0x8b),
	F_SZHV_DEC(0x8c), F_SZHV_DEC(0x8d), F_SZHV_DEC(0x8e), F_SZHV_DEC(0x8f),
	F_SZHV_DEC(0x90), F_SZHV_DEC(0x91), F_SZHV_DEC(0x92), F_SZHV_DEC(0x93),
	F_SZHV_DEC(0x94), F_SZHV_DEC(0x95), F_SZHV_DEC(0x96), F_SZHV_DEC(0x97),
	F_SZHV_DEC(0x98), F_SZHV_DEC(0x99), F_SZHV_DEC(0x9a), F_SZHV_DEC(0x9b),
	F_SZHV_DEC(0x9c), F_SZHV_DEC(0x9d), F_SZHV_DEC(0x9e), F_SZHV_DEC(0x9f),
	F_SZHV_DEC(0xa0), F_SZHV_DEC(0xa1), F_SZHV_DEC(0xa2), F_SZHV_DEC(0xa3),
	F_SZHV_DEC(0xa4), F_SZHV_DEC(0xa5), F_SZHV_DEC(0xa6), F_SZHV_DEC(0xa7),
	F_SZHV_DEC(0xa8), F_SZHV_DEC(0xa9), F_SZHV_DEC(0xaa), F_SZHV_DEC(0xab),
	F_SZHV_DEC(0xac), F_SZHV_DEC(0xad), F_SZHV_DEC(0xae), F_SZHV_DEC(0xaf),
	F_SZHV_DEC(0xb0), F_SZHV_DEC(0xb1), F_SZHV_DEC(0xb2), F_SZHV_DEC(0xb3),
	F_SZHV_DEC(0xb4), F_SZHV_DEC(0xb5), F_SZHV_DEC(0xb6), F_SZHV_DEC(0xb7),
	F_SZHV_DEC(0xb8), F_SZHV_DEC(0xb9), F_SZHV_DEC(0xba), F_SZHV_DEC(0xbb),
	F_SZHV_DEC(0xbc), F_SZHV_DEC(0xbd), F_SZHV_DEC(0xbe), F_SZHV_DEC(0xbf),
	F_SZHV_DEC(0xc0), F_SZHV_DEC(0xc1), F_SZHV_DEC(0xc2), F_SZHV_DEC(0xc3),
	F_SZHV_DEC(0xc4), F_SZHV_DEC(0xc5), F_SZHV_DEC(0xc6), F_SZHV_DEC(0xc7),
	F_SZHV_DEC(0xc8), F_SZHV_DEC(0xc9), F_SZHV_DEC(0xca), F_SZHV_DEC(0xcb),
	F_SZHV_DEC(0xcc), F_SZHV_DEC(0xcd), F_SZHV_DEC(0xce), F_SZHV_DEC(0xcf),
	F_SZHV_DEC(0xd0), F_SZHV_DEC(0xd1), F_SZHV_DEC(0xd2), F_SZHV_DEC(0xd3),
	F_SZHV_DEC(0xd4), F_SZHV_DEC(0xd5), F_SZHV_DEC(0xd6), F_SZHV_DEC(0xd7),
	F_SZHV_DEC(0xd8), F_SZHV_DEC(0xd9), F_SZHV_DEC(0xda), F_SZHV_DEC(0xdb),
	F_SZHV_DEC(0xdc), F_SZHV_DEC(0xdd), F_SZHV_DEC(0xde), F_SZHV_DEC(0xdf),
	F_SZHV_DEC(0xe0), F_SZHV_DEC(0xe1), F_SZHV_DEC(0xe2), F_SZHV_DEC(0xe3),
	F_SZHV_DEC(0xe4), F_SZHV_DEC(0xe5), F_SZHV_DEC(0xe6), F_SZHV_DEC(0xe7),
	F_SZHV_DEC(0xe8), F_SZHV_DEC(0xe9), F_SZHV_DEC(0xea), F_SZHV_DEC(0xeb),
	F_SZHV_DEC(0xec), F_SZHV_DEC(0xed), F_SZHV_DEC(0xee), F_SZHV_DEC(0xef),
	F_SZHV_DEC(0xf0), F_SZHV_DEC(0xf1), F_SZHV_DEC(0xf2), F_SZHV_DEC(0xf3),
	F_SZHV_DEC(0xf4), F_SZHV_DEC(0xf5), F_SZHV_DEC(0xf6), F_SZHV_DEC(0xf7),
	F_SZHV_DEC(0xf8), F_SZHV_DEC(0xf9), F_SZHV_DEC(0xfa), F_SZHV_DEC(0xfb),
	F_SZHV_DEC(0xfc), F_SZHV_DEC(0xfd), F_SZHV_DEC(0xfe), F_SZHV_DEC(0xff)
};

/** @brief increment 8 bit value */
static __inline uint8_t INC(z80_cpu_t *cpu, uint8_t val)
{
	uint8_t res = val + 1;
	F = (F & CF) | flags_szhf_inc[res];
	return res;
}

/** @brief decrement 8 bit value */
static __inline uint8_t DEC(z80_cpu_t *cpu, uint8_t val)
{
	uint8_t res = val - 1;
	F = (F & CF) | flags_szhf_dec[res];
	return res;
}

/** @brief add to accumulator */
static __inline uint8_t ADD(z80_cpu_t *cpu, uint8_t val)
{
	uint32_t res = A + val;
	F = flags_sz[(uint8_t)res] | ((res >> 8) & CF) |
		((A ^ res ^ val) & HF) |
		(((val ^ A ^ 0x80) & (val ^ res) & 0x80) >> 5);
	return (uint8_t)res;
}

/** @brief add with carry to accumulator */
static __inline uint8_t ADC(z80_cpu_t *cpu, uint8_t val)
{
	uint32_t res = A + val + (F & CF);
	F = flags_sz[(uint8_t)res] | ((res >> 8) & CF) |
		((A ^ res ^ val) & HF) |
		(((val ^ A ^ 0x80) & (val ^ res) & 0x80) >> 5);
	return (uint8_t)res;
}

/** @brief subtract from accumulator */
static __inline uint8_t SUB(z80_cpu_t *cpu, uint8_t val)
{
	uint32_t res = A - val;
	F = flags_sz[(uint8_t)res] | ((res >> 8) & CF) | NF |
		((A ^ res ^ val) & HF) |
		(((val ^ A) & (A ^ res) & 0x80) >> 5);
	return (uint8_t)res;
}

/** @brief subtract with carry from accumulator */
static __inline uint8_t SBC(z80_cpu_t *cpu, uint8_t val)
{
	uint32_t res = A - val - (F & CF);
	F = flags_sz[(uint8_t)res] | ((res >> 8) & CF) | NF |
		((A ^ res ^ val) & HF) |
		(((val ^ A) & (A ^ res) & 0x80) >> 5);
	return (uint8_t)res;
}

/** @brief and to accumulator */
static __inline uint8_t AND(z80_cpu_t *cpu, uint8_t val)
{
	uint8_t res = A & val;
	F = flags_szph[res];
	return res;
}

/** @brief xor to accumulator */
static __inline uint8_t XOR(z80_cpu_t *cpu, uint8_t val)
{
	uint8_t res = A ^ val;
	F = flags_szp[res];
	return res;
}

/** @brief or to accumulator */
static __inline uint8_t OR(z80_cpu_t *cpu, uint8_t val)
{
	uint8_t res = A | val;
	F = flags_szp[res];
	return res;
}

/** @brief compare accumulator with value */
static __inline void CP(z80_cpu_t *cpu, uint8_t val)
{
	uint32_t res = A - val;
	F = flags_sz[(uint8_t)res] | ((res >> 8) & CF) | NF |
		((A ^ res ^ val) & HF) |
		(((val ^ A) & (A ^ res) & 0x80) >> 5);
}

static __inline uint16_t ADD16(z80_cpu_t *cpu, uint32_t reg, uint32_t val)
{
	uint32_t res = reg + val;
	MP = reg + 1;
	F = (F & (SF | ZF | PF)) |
		((res >> 16) & CF) |
		((res >> 8) & (YF | XF)) |
		(((reg ^ res ^ val) >> 8) & HF);
	return (uint16_t)res;
}

static __inline uint16_t SBC16(z80_cpu_t *cpu, uint32_t reg, uint32_t val)
{
	uint32_t res = reg - val - (F & CF);
	MP = reg + 1;
	F = (((reg ^ res ^ val) >> 8) & HF) | NF |
		((res >> 16) & CF) |
		((res >> 8) & (SF | YF | XF)) |
		((res & 0xffff) ? 0 : ZF) |
		(((val ^ reg) & (reg ^ res) & 0x8000) ? PF : 0);
	return (uint16_t)res;
}

static __inline uint16_t ADC16(z80_cpu_t *cpu, uint32_t reg, uint32_t val)
{
	uint32_t res = reg + val + (F & CF);
	MP = reg + 1;
	F = (((reg ^ res ^ val) >> 8) & HF) |
		((res >> 16) & CF) |
		((res >> 8) & (SF | YF | XF)) |
		((res & 0xffff) ? 0 : ZF) |
		(((val ^ reg ^ 0x8000) & (val ^ res) & 0x8000) ? PF : 0);
	return (uint16_t)res;
}

/** @brief rotate left circular */
static __inline uint8_t RLC(z80_cpu_t *cpu, uint8_t val)
{
	uint8_t res = (val << 1) | (val >> 7);
	uint8_t cf = (val & 0x80) ? CF : 0;
	F = flags_szp[res] | cf;
	return res;
}

/** @brief rotate right circular */
static __inline uint8_t RRC(z80_cpu_t *cpu, uint8_t val)
{
	uint8_t res = (val >> 1) | (val << 7);
	uint8_t cf = (val & 0x01) ? CF : 0;
	F = flags_szp[res] | cf;
	return res;
}

/** @brief rotate left (through carry) */
static __inline uint8_t RL(z80_cpu_t *cpu, uint8_t val)
{
	uint8_t res = (val << 1) | (F & CF);
	uint8_t cf = (val & 0x80) ? CF : 0;
	F = flags_szp[res] | cf;
	return res;
}

/** @brief rotate right (through carry) */
static __inline uint8_t RR(z80_cpu_t *cpu, uint8_t val)
{
	uint8_t res = (val >> 1) | (F << 7);
	uint8_t cf = (val & 0x01) ? CF : 0;
	F = flags_szp[res] | cf;
	return res;
}

/** @brief shift left arithmetical */
static __inline uint8_t SLA(z80_cpu_t *cpu, uint8_t val)
{
	uint8_t res = val << 1;
	uint8_t cf = (val & 0x80) ? CF : 0;
	F = flags_szp[res] | cf;
	return res;
}

/** @brief shift right arithmetical */
static __inline uint8_t SRA(z80_cpu_t *cpu, uint8_t val)
{
	uint8_t res = (val & 0x80) | (val >> 1);
	uint8_t cf = (val & 0x01) ? CF : 0;
	F = flags_szp[res] | cf;
	return res;
}

/** @brief shift left logical (inverted) */
static __inline uint8_t SLL(z80_cpu_t *cpu, uint8_t val)
{
	uint8_t res = (val << 1) | 0x01;
	uint8_t cf = (val & 0x80) ? CF : 0;
	F = flags_szp[res] | cf;
	return res;
}

/** @brief shift right logical */
static __inline uint8_t SRL(z80_cpu_t *cpu, uint8_t val)
{
	uint32_t res = val;
	uint32_t cf = (res & 0x01) ? CF : 0;
	res = res >> 1;
	F = flags_szp[(uint8_t)res] | cf;
	return (uint8_t)res;
}

/** @brief test bit number b in val */
static __inline uint8_t BIT(z80_cpu_t *cpu, uint8_t b, uint8_t val)
{
	uint8_t res = val & (1 << b);
	F = (F & CF) | flags_sz_bit[res] | (res & (YF | XF));
	return res;
}

/** @brief test bit number b in memory (HL) */
static __inline uint8_t BIT_HL(z80_cpu_t *cpu, uint8_t b, uint8_t val)
{
	uint8_t res = val & (1 << b);
	F = (F & CF) | flags_sz_bit[res] | (MPH & (YF | XF));
	return res;
}

/** @brief test bit number b in memory (IX/Y+rel) */
static __inline uint8_t BIT_XY(z80_cpu_t *cpu, uint8_t b, uint8_t val)
{
	uint8_t res = val & (1 << b);
	F = (F & CF) | flags_sz_bit[res] | (EAH & (YF | XF));
	return res;
}

/** @brief reset bit number b in val */
static __inline uint8_t RES(z80_cpu_t *cpu, uint8_t b, uint8_t val)
{
	return val & ~(1 << b);
}

/** @brief set bit number b in val */
static __inline uint8_t SET(z80_cpu_t *cpu, uint8_t b, uint8_t val)
{
	return val | (1 << b);
}

/** @brief load (DE) <- (HL), decrement BC, increment HL and DE */
static __inline void LDI(z80_cpu_t *cpu)
{
	uint8_t io = RD_MEM(dHL);
	WR_MEM(dDE, io);
	F = F & (SF | ZF | CF);
	if ((A + io) & 0x02)
		F |= YF; /* bit 1 -> flag 5 */
	if ((A + io) & 0x08)
		F |= XF; /* bit 3 -> flag 3 */
	BC--;
	DE++;
	HL++;
	if (0 != BC)
		F |= PF;
}

/** @brief compare A == (HL), decrement BC, increment HL */
static __inline void CPI(z80_cpu_t *cpu)
{
	uint8_t val = RD_MEM(dHL);
	uint8_t res = A - val;
	MP++;
	HL++;
	BC--;
	F = (F & CF) |
		(flags_sz[res] & ~(YF | XF)) |
		((A ^ val ^ res) & HF) | NF;
	if (F & HF)
		res -= 1;
	if (res & 0x02)
		F |= YF; /* bit 1 -> flag 5 */
	if (res & 0x08)
		F |= XF; /* bit 3 -> flag 3 */
	if (0 != BC)
		F |= PF;
}

/** @brief read (HL) <- port (C), decrement B, increment HL */
static __inline void INI(z80_cpu_t *cpu)
{
	uint32_t tmp;
	uint8_t io = RD_IO(dBC);
	MP = BC + 1;
	B--;
	WR_MEM(dHL, io);
	HL++;
	F = flags_sz[B];
	tmp = (uint32_t)(uint8_t)(C + 1) + io;
	if (io & SF)
		F |= NF;
	if (tmp & 0x100)
		F |= HF | CF;
	F |= flags_p[(tmp & 0x07) ^ B];
}

/** @brief write port (C) <- (HL), decrement B, increment HL */
static __inline void OUTI(z80_cpu_t *cpu)
{
	uint32_t tmp;
	uint8_t io = RD_MEM(dHL);
	B--;
	MP = BC + 1;
	WR_IO(dBC, io);
	HL++;
	F = flags_sz[B];
	tmp = (uint32_t)L + io;
	if (io & SF)
		F |= NF;
	if (tmp & 0x100)
		F |= HF | CF;
	F |= flags_p[(tmp & 0x07) ^ B];
}

/** @brief load (DE) <- (HL), decrement BC, decrement HL and DE */
static __inline void LDD(z80_cpu_t *cpu)
{
	uint8_t io = RD_MEM(dHL);
	WR_MEM(dDE, io);
	F = F & (SF | ZF | CF);
	if ((A + io) & 0x02)
		F |= YF; /* bit 1 -> flag 5 */
	if ((A + io) & 0x08)
		F |= XF; /* bit 3 -> flag 3 */
	BC--;
	DE--;
	HL--;
	if (0 != BC)
		F |= PF;
}

/** @brief compare A == (HL), decrement BC, decrement HL */
static __inline void CPD(z80_cpu_t *cpu)
{
	uint8_t val = RD_MEM(dHL);
	uint8_t res = A - val;
	MP--;
	HL--;
	BC--;
	F = (F & CF) |
		(flags_sz[res] & ~(YF | XF)) |
		((A ^ val ^ res) & HF) | NF;
	if (F & HF)
		res -= 1;
	if (res & 0x02)
		F |= YF; /* bit 1 -> flag 5 */
	if (res & 0x08)
		F |= XF; /* bit 3 -> flag 3 */
	if (0 != BC)
		F |= PF;
}

/** @brief read (HL) <- port (C), decrement B, decrement HL */
static __inline void IND(z80_cpu_t *cpu)
{
	uint32_t tmp;
	uint8_t io = RD_IO(dBC);
	MP = BC - 1;
	B--;
	WR_MEM(dHL, io);
	HL--;
	F = flags_sz[B];
	tmp = (uint32_t)(uint8_t)(C - 1) + io;
	if (io & SF)
		F |= NF;
	if (tmp & 0x100)
		F |= HF | CF;
	F |= flags_p[(tmp & 0x07) ^ B];
}

/** @brief write port (C) <- (HL), decrement B, decrement HL */
static __inline void OUTD(z80_cpu_t *cpu)
{
	uint32_t tmp;
	uint8_t io = RD_MEM(dHL);
	B--;
	MP = BC - 1;
	WR_IO(dBC, io);
	HL--;
	F = flags_sz[B];
	tmp = (uint32_t)L + io;
	if (io & SF)
		F |= NF;
	if (tmp & 0x100)
		F |= HF | CF;
	F |= flags_p[(tmp & 0x07) ^ B];
}

/** @brief push a register pair to the stack */
static __inline void PUSH(z80_cpu_t *cpu, push_pop_t which)
{
	switch (which) {
	case REG_BC:
		SP--;
		WR_MEM(dSP, B);
		SP--;
		WR_MEM(dSP, C);
		break;
	case REG_DE:
		SP--;
		WR_MEM(dSP, D);
		SP--;
		WR_MEM(dSP, E);
		break;
	case REG_HL:
		SP--;
		WR_MEM(dSP, H);
		SP--;
		WR_MEM(dSP, L);
		break;
	case REG_AF:
		SP--;
		WR_MEM(dSP, A);
		SP--;
		WR_MEM(dSP, F);
		break;
	case REG_IX:
		SP--;
		WR_MEM(dSP, HX);
		SP--;
		WR_MEM(dSP, LX);
		break;
	case REG_IY:
		SP--;
		WR_MEM(dSP, HY);
		SP--;
		WR_MEM(dSP, LY);
		break;
	case REG_PC:
		SP--;
		WR_MEM(dSP, PCH);
		SP--;
		WR_MEM(dSP, PCL);
		break;
	}
}

/** @brief pop a register pair off the stack */
static __inline void POP(z80_cpu_t *cpu, push_pop_t which)
{
	switch (which) {
	case REG_BC:
		C = RD_MEM(dSP);
		SP++;
		B = RD_MEM(dSP);
		SP++;
		break;
	case REG_DE:
		E = RD_MEM(dSP);
		SP++;
		D = RD_MEM(dSP);
		SP++;
		break;
	case REG_HL:
		L = RD_MEM(dSP);
		SP++;
		H = RD_MEM(dSP);
		SP++;
		break;
	case REG_AF:
		F = RD_MEM(dSP);
		SP++;
		A = RD_MEM(dSP);
		SP++;
		break;
	case REG_IX:
		LX = RD_MEM(dSP);
		SP++;
		HX = RD_MEM(dSP);
		SP++;
		break;
	case REG_IY:
		LY = RD_MEM(dSP);
		SP++;
		HY = RD_MEM(dSP);
		SP++;
		break;
	case REG_PC:
		PCL = RD_MEM(dSP);
		SP++;
		PCH = RD_MEM(dSP);
		SP++;
		MP = PC;
		break;
	}
}

int z80_interrupt(z80_cpu_t *cpu, int type)
{
	cpu->irq = type;
	return 0;
}

uint32_t z80_get_reg(z80_cpu_t *cpu, uint32_t id)
{
	switch (id) {
	case Z80_PC:
		return PC;
	case Z80_SP:
		return SP;
	case Z80_AF:
		return AF;
	case Z80_BC:
		return BC;
	case Z80_DE:
		return DE;
	case Z80_HL:
		return HL;
	case Z80_IX:
		return IX;
	case Z80_IY:
		return IY;
	case Z80_EA:
		return EA;
	case Z80_MP:
		return MP;
	case Z80_AF2:
		return AF2;
	case Z80_BC2:
		return BC2;
	case Z80_DE2:
		return DE2;
	case Z80_HL2:
		return HL2;
	case Z80_R:
		return (cpu->r & 0x7f) | cpu->r7;
	case Z80_I:
		return cpu->iv;
	case Z80_IM:
		return cpu->im;
	case Z80_IFF1:
		return cpu->iff & 1;
	case Z80_IFF2:
		return (cpu->iff >> 1) & 1;
	case Z80_IRQ:
		return cpu->irq;
	}
	return (uint32_t)-1;
}

int z80_execute(z80_cpu_t *cpu)
{
	uint8_t op;
	uint8_t m;

	z80_cc = z80_dma;
	z80_dma = 0;

fetch_xx:
	switch (cpu->irq) {
	case 0:	/* no interrupt pending */
		break;
	case 1:	/* NMI */
		if (0x76 == mem[PC])
			PC++;
		PUSH(cpu, REG_PC);
		PC = 0x0066;
		cpu->iff = cpu->iff & 2;	/* reser IFF1, keep IFF2 */
		change_pc(dPC);
		cpu->irq = 0;
		break;
	case 2:	/* IRQ */
		if (0 == (cpu->iff & 1))
			break;
		if (0x76 == mem[PC])
			PC++;
		PUSH(cpu, REG_PC);
		PC = 0x0038;
		cpu->iff = 0;			/* reset IFF1 and IFF2 */
		change_pc(dPC);
		cpu->irq = 0;
		break;
	}
	op = RD_OP(cpu);
	cpu->r += 1;

decode_xx:
	switch (op) {
	case 0x00:	/* NOP			*/
		{
			z80_cc += cc_op[0x00];
		}
		break;

	case 0x01:	/* LD	BC,nnnn		*/
		{
			z80_cc += cc_op[0x01];
			BC = RD_ARGW(cpu);
		}
		break;

	case 0x02:	/* LD	(BC),A		*/
		{
			z80_cc += cc_op[0x02];
			WR_MEM(dBC, A);
		}
		break;

	case 0x03:	/* INC	BC		*/
		{
			z80_cc += cc_op[0x03];
			BC++;
		}
		break;

	case 0x04:	/* INC	B		*/
		{
			z80_cc += cc_op[0x04];
			B = INC(cpu, B);
		}
		break;

	case 0x05:	/* DEC	B		*/
		{
			z80_cc += cc_op[0x05];
			B = DEC(cpu, B);
		}
		break;

	case 0x06:	/* LD	B,nn		*/
		{
			z80_cc += cc_op[0x06];
			B = RD_ARGB(cpu);
		}
		break;

	case 0x07:	/* RLCA			*/
		{
			z80_cc += cc_op[0x07];
			A = (A << 1) | (A >> 7);
			F = (F & (SF | ZF | PF)) | (A & (YF | XF | CF));
		}
		break;

	case 0x08:	/* EX	AF,AF'		*/
		{
			z80_cc += cc_op[0x08];
			dAF ^= dAF2;
			dAF2 ^= dAF;
			dAF ^= dAF2;
		}
		break;

	case 0x09:	/* ADD	HL,BC		*/
		{
			z80_cc += cc_op[0x09];
			HL = ADD16(cpu, dHL, dBC);
		}
		break;

	case 0x0a:	/* LD	A,(BC)		*/
		{
			z80_cc += cc_op[0x0a];
			A = RD_MEM(dBC);
		}
		break;

	case 0x0b:	/* DEC	BC		*/
		{
			z80_cc += cc_op[0x0b];
			BC--;
		}
		break;

	case 0x0c:	/* INC	C		*/
		{
			z80_cc += cc_op[0x0c];
			C = INC(cpu, C);
		}
		break;

	case 0x0d:	/* DEC	C		*/
		{
			z80_cc += cc_op[0x0d];
			C = DEC(cpu, C);
		}
		break;

	case 0x0e:	/* LD	C,nn		*/
		{
			z80_cc += cc_op[0x0e];
			C = RD_ARGB(cpu);
		}
		break;

	case 0x0f:	/* RRCA			*/
		{
			z80_cc += cc_op[0x0f];
			F = (F & (SF | ZF | PF)) | (A & CF);
			A = (A >> 1) | (A << 7);
			F |= (A & (YF | XF));
		}
		break;

	case 0x10:	/* DJNZ	rel8		*/
		{
			int8_t rel = RD_ARGB(cpu);
			z80_cc += cc_op[0x10];
			if (--B != 0) {
				z80_cc += cc_ex[0x10];
				PC += rel;
				MP = PC;
			}
		}
		break;

	case 0x11:	/* LD	DE,nnnn		*/
		{
			z80_cc += cc_op[0x11];
			DE = RD_ARGW(cpu);
		}
		break;

	case 0x12:	/* LD	(DE),A		*/
		{
			z80_cc += cc_op[0x12];
			WR_MEM(dDE, A);
		}
		break;

	case 0x13:	/* INC	DE		*/
		{
			z80_cc += cc_op[0x13];
			DE++;
		}
		break;

	case 0x14:	/* INC	D		*/
		{
			z80_cc += cc_op[0x14];
			D = INC(cpu, D);
		}
		break;

	case 0x15:	/* DEC	D		*/
		{
			z80_cc += cc_op[0x15];
			D = DEC(cpu, D);
		}
		break;

	case 0x16:	/* LD	D,nn		*/
		{
			z80_cc += cc_op[0x16];
			D = RD_ARGB(cpu);
		}
		break;

	case 0x17:	/* RLA			*/
		{
			uint8_t res = (A << 1) | (F & CF);
			uint8_t cf = (A & 0x80) ? CF : 0;
			z80_cc += cc_op[0x17];
			A = res;
			F = (F & (SF | ZF | PF)) | (res & (YF | XF)) | cf;
		}
		break;

	case 0x18:	/* JR	rel8		*/
		{
			int8_t rel = RD_ARGB(cpu);
			z80_cc += cc_op[0x18];
			PC += rel;
			MP = PC;
		}
		break;

	case 0x19:	/* ADD	HL,DE		*/
		{
			z80_cc += cc_op[0x19];
			HL = ADD16(cpu, dHL, dDE);
		}
		break;

	case 0x1a:	/* LD	A,(DE)		*/
		{
			z80_cc += cc_op[0x1a];
			A = RD_MEM(dDE);
		}
		break;

	case 0x1b:	/* DEC	DE		*/
		{
			z80_cc += cc_op[0x1b];
			DE--;
		}
		break;

	case 0x1c:	/* INC	E		*/
		{
			z80_cc += cc_op[0x1c];
			E = INC(cpu, E);
		}
		break;

	case 0x1d:	/* DEC	E		*/
		{
			z80_cc += cc_op[0x1d];
			E = DEC(cpu, E);
		}
		break;

	case 0x1e:	/* LD	E,nn		*/
		{
			z80_cc += cc_op[0x1e];
			E = RD_ARGB(cpu);
		}
		break;

	case 0x1f:	/* RRA			*/
		{
			uint8_t res = (A >> 1) | ((F & CF) << 7);
			uint8_t cf = (A & 0x01) ? CF : 0;
			z80_cc += cc_op[0x1f];
			A = res;
			F = (F & (SF | ZF | PF)) | (res & (YF | XF)) | cf;
		}
		break;

	case 0x20:	/* JR	NZ,rel8		*/
		{
			int8_t rel = RD_ARGB(cpu);
			z80_cc += cc_op[0x20];
			if (0 == (F & ZF)) {
				z80_cc += cc_ex[0x20];
				PC += rel;
				MP = PC;
			}
		}
		break;

	case 0x21:	/* LD	HL,nnnn		*/
		{
			z80_cc += cc_op[0x21];
			HL = RD_ARGW(cpu);
		}
		break;

	case 0x22:	/* LD	(nnnn),HL	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0x22];
			WR_MEM(dMP, L);
			MP++;
			WR_MEM(dMP, H);
		}
		break;

	case 0x23:	/* INC	HL		*/
		{
			z80_cc += cc_op[0x23];
			HL++;
		}
		break;

	case 0x24:	/* INC	H		*/
		{
			z80_cc += cc_op[0x24];
			H = INC(cpu, H);
		}
		break;

	case 0x25:	/* DEC	H		*/
		{
			z80_cc += cc_op[0x25];
			H = DEC(cpu, H);
		}
		break;

	case 0x26:	/* LD	H,nn		*/
		{
			z80_cc += cc_op[0x26];
			H = RD_ARGB(cpu);
		}
		break;

	case 0x27:	/* DAA			*/
		{
			uint8_t cf = F & CF;
			uint8_t nf = F & NF;
			uint8_t hf = F & HF;
			uint8_t lo = A % 16;
			uint8_t hi = A / 16;
			uint8_t diff;

			z80_cc += cc_op[0x27];
			if (cf) {
				diff = (lo <= 9 && !hf) ? 0x60 : 0x66;
			} else {
				if (lo >= 10) {
					diff = hi <= 8 ? 0x06 : 0x66;
				} else {
					if (hi >= 10) {
						diff = hf ? 0x66 : 0x60;
					} else {
						diff = hf ? 0x06 : 0x00;
					}
				}
			}
			if (nf)
				A -= diff;
			else
				A += diff;

			F = flags_szp[A] | (F & NF);
			if (cf || (lo <= 9 ? hi >= 10 : hi >= 9))
				F |= CF;
			if (nf ? hf && lo <= 5 : lo >= 10)
				F |= HF;
		}
		break;

	case 0x28:	/* JR	Z,rel8		*/
		{
			int8_t rel = RD_ARGB(cpu);
			z80_cc += cc_op[0x28];
			if (0 != (F & ZF)) {
				z80_cc += cc_ex[0x28];
				PC += rel;
				MP = PC;
			}
		}
		break;

	case 0x29:	/* ADD	HL,HL		*/
		{
			z80_cc += cc_op[0x29];
			HL = ADD16(cpu, dHL, dHL);
		}
		break;

	case 0x2a:	/* LD	HL,(nnnn)	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0x2a];
			L = RD_MEM(dMP);
			MP++;
			H = RD_MEM(dMP);
		}
		break;

	case 0x2b:	/* DEC	HL		*/
		{
			z80_cc += cc_op[0x2b];
			HL--;
		}
		break;

	case 0x2c:	/* INC	L		*/
		{
			z80_cc += cc_op[0x2c];
			L = INC(cpu, L);
		}
		break;

	case 0x2d:	/* DEC	L		*/
		{
			z80_cc += cc_op[0x2d];
			L = DEC(cpu, L);
		}
		break;

	case 0x2e:	/* LD	L,nn		*/
		{
			z80_cc += cc_op[0x2e];
			L = RD_ARGB(cpu);
		}
		break;

	case 0x2f:	/* CPL			*/
		{
			z80_cc += cc_op[0x2f];
			A ^= 0xff;
			F = (F & (SF | ZF | PF | CF)) |
				HF | NF | (A & (YF | XF));
		}
		break;

	case 0x30:	/* JR	NC,rel8		*/
		{
			int8_t rel = RD_ARGB(cpu);
			z80_cc += cc_op[0x30];
			if (0 == (F & CF)) {
				z80_cc += cc_ex[0x30];
				PC += rel;
				MP = PC;
			}
		}
		break;

	case 0x31:	/* LD	SP,nnnn		*/
		{
			z80_cc += cc_op[0x31];
			SP = RD_ARGW(cpu);
		}
		break;

	case 0x32:	/* LD	(nnnn),A	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0x32];
			WR_MEM(dMP, A);
			MP++;
			MPH = A;
		}
		break;

	case 0x33:	/* INC	SP		*/
		{
			z80_cc += cc_op[0x33];
			SP++;
		}
		break;

	case 0x34:	/* INC	(HL)		*/
		{
			z80_cc += cc_op[0x34];
			WR_MEM(dHL, INC(cpu, RD_MEM(dHL)));
		}
		break;

	case 0x35:	/* DEC	(HL)		*/
		{
			z80_cc += cc_op[0x35];
			WR_MEM(dHL, DEC(cpu, RD_MEM(dHL)));
		}
		break;

	case 0x36:	/* LD	(HL),nn		*/
		{
			z80_cc += cc_op[0x36];
			WR_MEM(dHL, RD_ARGB(cpu));
		}
		break;

	case 0x37:	/* SCF			*/
		{
			z80_cc += cc_op[0x37];
			F = (F & (SF | ZF | PF)) | CF | (A & (YF | XF));
		}
		break;

	case 0x38:	/* JR	C,rel8		*/
		{
			int8_t rel = RD_ARGB(cpu);
			z80_cc += cc_op[0x38];
			if (0 != (F & CF)) {
				z80_cc += cc_ex[0x38];
				PC += rel;
				MP = PC;
			}
		}
		break;

	case 0x39:	/* ADD	HL,SP		*/
		{
			z80_cc += cc_op[0x39];
			HL = ADD16(cpu, dHL, dSP);
		}
		break;

	case 0x3a:	/* LD	A,(nnnn)	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0x3a];
			A = RD_MEM(MP);
			MP++;
		}
		break;

	case 0x3b:	/* DEC	SP		*/
		{
			z80_cc += cc_op[0x3b];
			SP--;
		}
		break;

	case 0x3c:	/* INC	A		*/
		{
			z80_cc += cc_op[0x3c];
			A = INC(cpu, A);
		}
		break;

	case 0x3d:	/* DEC	A		*/
		{
			z80_cc += cc_op[0x3d];
			A = DEC(cpu, A);
		}
		break;

	case 0x3e:	/* LD	A,nn		*/
		{
			z80_cc += cc_op[0x3e];
			A = RD_ARGB(cpu);
		}
		break;

	case 0x3f:	/* CCF			*/
		{
			z80_cc += cc_op[0x3f];
			F = ((F & ( SF | ZF | PF | CF)) |
				((F & CF) << 4) | (A & (YF | XF))) ^ CF;
		}
		break;

	case 0x40:	/* LD	B,B		*/
		{
			z80_cc += cc_op[0x40];
			B = B;
		}
		break;

	case 0x41:	/* LD	B,C		*/
		{
			z80_cc += cc_op[0x41];
			B = C;
		}
		break;

	case 0x42:	/* LD	B,D		*/
		{
			z80_cc += cc_op[0x42];
			B = D;
		}
		break;

	case 0x43:	/* LD	B,E		*/
		{
			z80_cc += cc_op[0x43];
			B = E;
		}
		break;

	case 0x44:	/* LD	B,H		*/
		{
			z80_cc += cc_op[0x44];
			B = H;
		}
		break;

	case 0x45:	/* LD	B,L		*/
		{
			z80_cc += cc_op[0x45];
			B = L;
		}
		break;

	case 0x46:	/* LD	B,(HL)		*/
		{
			z80_cc += cc_op[0x46];
			B = RD_MEM(dHL);
		}
		break;

	case 0x47:	/* LD	B,A		*/
		{
			z80_cc += cc_op[0x47];
			B = A;
		}
		break;

	case 0x48:	/* LD	C,B		*/
		{
			z80_cc += cc_op[0x48];
			C = B;
		}
		break;

	case 0x49:	/* LD	C,C		*/
		{
			z80_cc += cc_op[0x49];
			C = C;
		}
		break;

	case 0x4a:	/* LD	C,D		*/
		{
			z80_cc += cc_op[0x4a];
			C = D;
		}
		break;

	case 0x4b:	/* LD	C,E		*/
		{
			z80_cc += cc_op[0x4b];
			C = E;
		}
		break;

	case 0x4c:	/* LD	C,H		*/
		{
			z80_cc += cc_op[0x4c];
			C = H;
		}
		break;

	case 0x4d:	/* LD	C,L		*/
		{
			z80_cc += cc_op[0x4d];
			C = L;
		}
		break;

	case 0x4e:	/* LD	C,(HL)		*/
		{
			z80_cc += cc_op[0x4e];
			C = RD_MEM(dHL);
		}
		break;

	case 0x4f:	/* LD	C,A		*/
		{
			z80_cc += cc_op[0x4f];
			C = A;
		}
		break;

	case 0x50:	/* LD	D,B		*/
		{
			z80_cc += cc_op[0x50];
			D = B;
		}
		break;

	case 0x51:	/* LD	D,C		*/
		{
			z80_cc += cc_op[0x51];
			D = C;
		}
		break;

	case 0x52:	/* LD	D,D		*/
		{
			z80_cc += cc_op[0x52];
			D = D;
		}
		break;

	case 0x53:	/* LD	D,E		*/
		{
			z80_cc += cc_op[0x53];
			D = E;
		}
		break;

	case 0x54:	/* LD	D,H		*/
		{
			z80_cc += cc_op[0x54];
			D = H;
		}
		break;

	case 0x55:	/* LD	D,L		*/
		{
			z80_cc += cc_op[0x55];
			D = L;
		}
		break;

	case 0x56:	/* LD	D,(HL)		*/
		{
			z80_cc += cc_op[0x56];
			D = RD_MEM(dHL);
		}
		break;

	case 0x57:	/* LD	D,A		*/
		{
			z80_cc += cc_op[0x57];
			D = A;
		}
		break;

	case 0x58:	/* LD	E,B		*/
		{
			z80_cc += cc_op[0x58];
			E = B;
		}
		break;

	case 0x59:	/* LD	E,C		*/
		{
			z80_cc += cc_op[0x59];
			E = C;
		}
		break;

	case 0x5a:	/* LD	E,D		*/
		{
			z80_cc += cc_op[0x5a];
			E = D;
		}
		break;

	case 0x5b:	/* LD	E,E		*/
		{
			z80_cc += cc_op[0x5b];
			E = E;
		}
		break;

	case 0x5c:	/* LD	E,H		*/
		{
			z80_cc += cc_op[0x5c];
			E = H;
		}
		break;

	case 0x5d:	/* LD	E,L		*/
		{
			z80_cc += cc_op[0x5d];
			E = L;
		}
		break;

	case 0x5e:	/* LD	E,(HL)		*/
		{
			z80_cc += cc_op[0x5e];
			E = RD_MEM(dHL);
		}
		break;

	case 0x5f:	/* LD	E,A		*/
		{
			z80_cc += cc_op[0x5f];
			E = A;
		}
		break;

	case 0x60:	/* LD	H,B		*/
		{
			z80_cc += cc_op[0x60];
			H = B;
		}
		break;

	case 0x61:	/* LD	H,C		*/
		{
			z80_cc += cc_op[0x61];
			H = C;
		}
		break;

	case 0x62:	/* LD	H,D		*/
		{
			z80_cc += cc_op[0x62];
			H = D;
		}
		break;

	case 0x63:	/* LD	H,E		*/
		{
			z80_cc += cc_op[0x63];
			H = E;
		}
		break;

	case 0x64:	/* LD	H,H		*/
		{
			z80_cc += cc_op[0x64];
			H = H;
		}
		break;

	case 0x65:	/* LD	H,L		*/
		{
			z80_cc += cc_op[0x65];
			H = L;
		}
		break;

	case 0x66:	/* LD	H,(HL)		*/
		{
			z80_cc += cc_op[0x66];
			H = RD_MEM(dHL);
		}
		break;

	case 0x67:	/* LD	H,A		*/
		{
			z80_cc += cc_op[0x67];
			H = A;
		}
		break;

	case 0x68:	/* LD	L,B		*/
		{
			z80_cc += cc_op[0x68];
			L = B;
		}
		break;

	case 0x69:	/* LD	L,C		*/
		{
			z80_cc += cc_op[0x69];
			L = C;
		}
		break;

	case 0x6a:	/* LD	L,D		*/
		{
			z80_cc += cc_op[0x6a];
			L = D;
		}
		break;

	case 0x6b:	/* LD	L,E		*/
		{
			z80_cc += cc_op[0x6b];
			L = E;
		}
		break;

	case 0x6c:	/* LD	L,H		*/
		{
			z80_cc += cc_op[0x6c];
			L = H;
		}
		break;

	case 0x6d:	/* LD	L,L		*/
		{
			z80_cc += cc_op[0x6d];
			L = L;
		}
		break;

	case 0x6e:	/* LD	L,(HL)		*/
		{
			z80_cc += cc_op[0x6e];
			L = RD_MEM(dHL);
		}
		break;

	case 0x6f:	/* LD	L,A		*/
		{
			z80_cc += cc_op[0x6f];
			L = A;
		}
		break;

	case 0x70:	/* LD	(HL),B		*/
		{
			z80_cc += cc_op[0x70];
			WR_MEM(dHL, B);
		}
		break;

	case 0x71:	/* LD	(HL),C		*/
		{
			z80_cc += cc_op[0x71];
			WR_MEM(dHL, C);
		}
		break;

	case 0x72:	/* LD	(HL),D		*/
		{
			z80_cc += cc_op[0x72];
			WR_MEM(dHL, D);
		}
		break;

	case 0x73:	/* LD	(HL),E		*/
		{
			z80_cc += cc_op[0x73];
			WR_MEM(dHL, E);
		}
		break;

	case 0x74:	/* LD	(HL),H		*/
		{
			z80_cc += cc_op[0x74];
			WR_MEM(dHL, H);
		}
		break;

	case 0x75:	/* LD	(HL),L		*/
		{
			z80_cc += cc_op[0x75];
			WR_MEM(dHL, L);
		}
		break;

	case 0x76:	/* HALT			*/
		{
			z80_cc += cc_op[0x76];
			PC--;
		}
		break;

	case 0x77:	/* LD	(HL),A		*/
		{
			z80_cc += cc_op[0x77];
			WR_MEM(dHL, A);
		}
		break;

	case 0x78:	/* LD	A,B		*/
		{
			z80_cc += cc_op[0x78];
			A = B;
		}
		break;

	case 0x79:	/* LD	A,C		*/
		{
			z80_cc += cc_op[0x79];
			A = C;
		}
		break;

	case 0x7a:	/* LD	A,D		*/
		{
			z80_cc += cc_op[0x7a];
			A = D;
		}
		break;

	case 0x7b:	/* LD	A,E		*/
		{
			z80_cc += cc_op[0x7b];
			A = E;
		}
		break;

	case 0x7c:	/* LD	A,H		*/
		{
			z80_cc += cc_op[0x7c];
			A = H;
		}
		break;

	case 0x7d:	/* LD	A,L		*/
		{
			z80_cc += cc_op[0x7d];
			A = L;
		}
		break;

	case 0x7e:	/* LD	A,(HL)		*/
		{
			z80_cc += cc_op[0x7e];
			A = RD_MEM(dHL);
		}
		break;

	case 0x7f:	/* LD	A,A		*/
		{
			z80_cc += cc_op[0x7f];
			A = A;
		}
		break;

	case 0x80:	/* ADD	A,B		*/
		{
			z80_cc += cc_op[0x80];
			A = ADD(cpu, B);
		}
		break;

	case 0x81:	/* ADD	A,C		*/
		{
			z80_cc += cc_op[0x81];
			A = ADD(cpu, C);
		}
		break;

	case 0x82:	/* ADD	A,D		*/
		{
			z80_cc += cc_op[0x82];
			A = ADD(cpu, D);
		}
		break;

	case 0x83:	/* ADD	A,E		*/
		{
			z80_cc += cc_op[0x83];
			A = ADD(cpu, E);
		}
		break;

	case 0x84:	/* ADD	A,H		*/
		{
			z80_cc += cc_op[0x84];
			A = ADD(cpu, H);
		}
		break;

	case 0x85:	/* ADD	A,L		*/
		{
			z80_cc += cc_op[0x85];
			A = ADD(cpu, L);
		}
		break;

	case 0x86:	/* ADD	A,(HL)		*/
		{
			z80_cc += cc_op[0x86];
			A = ADD(cpu, RD_MEM(dHL));
		}
		break;

	case 0x87:	/* ADD	A,A		*/
		{
			z80_cc += cc_op[0x87];
			A = ADD(cpu, A);
		}
		break;

	case 0x88:	/* ADC	A,B		*/
		{
			z80_cc += cc_op[0x88];
			A = ADC(cpu, B);
		}
		break;

	case 0x89:	/* ADC	A,C		*/
		{
			z80_cc += cc_op[0x89];
			A = ADC(cpu, C);
		}
		break;

	case 0x8a:	/* ADC	A,D		*/
		{
			z80_cc += cc_op[0x8a];
			A = ADC(cpu, D);
		}
		break;

	case 0x8b:	/* ADC	A,E		*/
		{
			z80_cc += cc_op[0x8b];
			A = ADC(cpu, E);
		}
		break;

	case 0x8c:	/* ADC	A,H		*/
		{
			z80_cc += cc_op[0x8c];
			A = ADC(cpu, H);
		}
		break;

	case 0x8d:	/* ADC	A,L		*/
		{
			z80_cc += cc_op[0x8d];
			A = ADC(cpu, L);
		}
		break;

	case 0x8e:	/* ADC	A,(HL)		*/
		{
			z80_cc += cc_op[0x8e];
			A = ADC(cpu, RD_MEM(dHL));
		}
		break;

	case 0x8f:	/* ADC	A,A		*/
		{
			z80_cc += cc_op[0x8f];
			A = ADC(cpu, A);
		}
		break;

	case 0x90:	/* SUB	B		*/
		{
			z80_cc += cc_op[0x90];
			A = SUB(cpu, B);
		}
		break;

	case 0x91:	/* SUB	C		*/
		{
			z80_cc += cc_op[0x91];
			A = SUB(cpu, C);
		}
		break;

	case 0x92:	/* SUB	D		*/
		{
			z80_cc += cc_op[0x92];
			A = SUB(cpu, D);
		}
		break;

	case 0x93:	/* SUB	E		*/
		{
			z80_cc += cc_op[0x93];
			A = SUB(cpu, E);
		}
		break;

	case 0x94:	/* SUB	H		*/
		{
			z80_cc += cc_op[0x94];
			A = SUB(cpu, H);
		}
		break;

	case 0x95:	/* SUB	L		*/
		{
			z80_cc += cc_op[0x95];
			A = SUB(cpu, L);
		}
		break;

	case 0x96:	/* SUB	(HL)		*/
		{
			z80_cc += cc_op[0x96];
			A = SUB(cpu, RD_MEM(dHL));
		}
		break;

	case 0x97:	/* SUB	A		*/
		{
			z80_cc += cc_op[0x97];
			A = SUB(cpu, A);
		}
		break;

	case 0x98:	/* SBC	A,B		*/
		{
			z80_cc += cc_op[0x98];
			A = SBC(cpu, B);
		}
		break;

	case 0x99:	/* SBC	A,C		*/
		{
			z80_cc += cc_op[0x99];
			A = SBC(cpu, C);
		}
		break;

	case 0x9a:	/* SBC	A,D		*/
		{
			z80_cc += cc_op[0x9a];
			A = SBC(cpu, D);
		}
		break;

	case 0x9b:	/* SBC	A,E		*/
		{
			z80_cc += cc_op[0x9b];
			A = SBC(cpu, E);
		}
		break;

	case 0x9c:	/* SBC	A,H		*/
		{
			z80_cc += cc_op[0x9c];
			A = SBC(cpu, H);
		}
		break;

	case 0x9d:	/* SBC	A,L		*/
		{
			z80_cc += cc_op[0x9d];
			A = SBC(cpu, L);
		}
		break;

	case 0x9e:	/* SBC	A,(HL)		*/
		{
			z80_cc += cc_op[0x9e];
			A = SBC(cpu, RD_MEM(dHL));
		}
		break;

	case 0x9f:	/* SBC	A,A		*/
		{
			z80_cc += cc_op[0x9f];
			A = SBC(cpu, A);
		}
		break;

	case 0xa0:	/* AND	B		*/
		{
			z80_cc += cc_op[0xa0];
			A = AND(cpu, B);
		}
		break;

	case 0xa1:	/* AND	C		*/
		{
			z80_cc += cc_op[0xa1];
			A = AND(cpu, C);
		}
		break;

	case 0xa2:	/* AND	D		*/
		{
			z80_cc += cc_op[0xa2];
			A = AND(cpu, D);
		}
		break;

	case 0xa3:	/* AND	E		*/
		{
			z80_cc += cc_op[0xa3];
			A = AND(cpu, E);
		}
		break;

	case 0xa4:	/* AND	H		*/
		{
			z80_cc += cc_op[0xa4];
			A = AND(cpu, H);
		}
		break;

	case 0xa5:	/* AND	L		*/
		{
			z80_cc += cc_op[0xa5];
			A = AND(cpu, L);
		}
		break;

	case 0xa6:	/* AND	(HL)		*/
		{
			z80_cc += cc_op[0xa6];
			A = AND(cpu, RD_MEM(dHL));
		}
		break;

	case 0xa7:	/* AND	A		*/
		{
			z80_cc += cc_op[0xa7];
			A = AND(cpu, A);
		}
		break;

	case 0xa8:	/* XOR	B		*/
		{
			z80_cc += cc_op[0xa8];
			A = XOR(cpu, B);
		}
		break;

	case 0xa9:	/* XOR	C		*/
		{
			z80_cc += cc_op[0xa9];
			A = XOR(cpu, C);
		}
		break;

	case 0xaa:	/* XOR	D		*/
		{
			z80_cc += cc_op[0xaa];
			A = XOR(cpu, D);
		}
		break;

	case 0xab:	/* XOR	E		*/
		{
			z80_cc += cc_op[0xab];
			A = XOR(cpu, E);
		}
		break;

	case 0xac:	/* XOR	H		*/
		{
			z80_cc += cc_op[0xac];
			A = XOR(cpu, H);
		}
		break;

	case 0xad:	/* XOR	L		*/
		{
			z80_cc += cc_op[0xad];
			A = XOR(cpu, L);
		}
		break;

	case 0xae:	/* XOR	(HL)		*/
		{
			z80_cc += cc_op[0xae];
			A = XOR(cpu, RD_MEM(dHL));
		}
		break;

	case 0xaf:	/* XOR	A		*/
		{
			z80_cc += cc_op[0xaf];
			A = XOR(cpu, A);
		}
		break;

	case 0xb0:	/* OR	B		*/
		{
			z80_cc += cc_op[0xb0];
			A = OR(cpu, B);
		}
		break;

	case 0xb1:	/* OR	C		*/
		{
			z80_cc += cc_op[0xb1];
			A = OR(cpu, C);
		}
		break;

	case 0xb2:	/* OR	D		*/
		{
			z80_cc += cc_op[0xb2];
			A = OR(cpu, D);
		}
		break;

	case 0xb3:	/* OR	E		*/
		{
			z80_cc += cc_op[0xb3];
			A = OR(cpu, E);
		}
		break;

	case 0xb4:	/* OR	H		*/
		{
			z80_cc += cc_op[0xb4];
			A = OR(cpu, H);
		}
		break;

	case 0xb5:	/* OR	L		*/
		{
			z80_cc += cc_op[0xb5];
			A = OR(cpu, L);
		}
		break;

	case 0xb6:	/* OR	(HL)		*/
		{
			z80_cc += cc_op[0xb6];
			A = OR(cpu, RD_MEM(dHL));
		}
		break;

	case 0xb7:	/* OR	A		*/
		{
			z80_cc += cc_op[0xb7];
			A = OR(cpu, A);
		}
		break;

	case 0xb8:	/* CP	B		*/
		{
			z80_cc += cc_op[0xb8];
			CP(cpu, B);
		}
		break;

	case 0xb9:	/* CP	C		*/
		{
			z80_cc += cc_op[0xb9];
			CP(cpu, C);
		}
		break;

	case 0xba:	/* CP	D		*/
		{
			z80_cc += cc_op[0xba];
			CP(cpu, D);
		}
		break;

	case 0xbb:	/* CP	E		*/
		{
			z80_cc += cc_op[0xbb];
			CP(cpu, E);
		}
		break;

	case 0xbc:	/* CP	H		*/
		{
			z80_cc += cc_op[0xbc];
			CP(cpu, H);
		}
		break;

	case 0xbd:	/* CP	L		*/
		{
			z80_cc += cc_op[0xbd];
			CP(cpu, L);
		}
		break;

	case 0xbe:	/* CP	(HL)		*/
		{
			z80_cc += cc_op[0xbe];
			CP(cpu, RD_MEM(dHL));
		}
		break;

	case 0xbf:	/* CP	A		*/
		{
			z80_cc += cc_op[0xbf];
			CP(cpu, A);
		}
		break;

	case 0xc0:	/* RET	NZ		*/
		{
			z80_cc += cc_op[0xc0];
			if (0 == (F & ZF)) {
				z80_cc += cc_ex[0xc0];
				POP(cpu, REG_PC);
				change_pc(dPC);
			}
		}
		break;

	case 0xc1:	/* POP	BC		*/
		{
			z80_cc += cc_op[0xc1];
			POP(cpu, REG_BC);
		}
		break;

	case 0xc2:	/* JP	NZ,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xc2];
			if (0 == (F & ZF)) {
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xc3:	/* JP	nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xc3];
			PC = MP;
			change_pc(dPC);
		}
		break;

	case 0xc4:	/* CALL	NZ,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xc4];
			if (0 == (F & ZF)) {
				z80_cc += cc_ex[0xc4];
				PUSH(cpu, REG_PC);
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xc5:	/* PUSH	BC		*/
		{
			z80_cc += cc_op[0xc5];
			PUSH(cpu, REG_BC);
		}
		break;

	case 0xc6:	/* ADD	A,nn		*/
		{
			z80_cc += cc_op[0xc6];
			A = ADD(cpu, RD_ARGB(cpu));
		}
		break;

	case 0xc7:	/* RST	00H		*/
		{
			z80_cc += cc_op[0xc7];
			PUSH(cpu, REG_PC);
			MP = PC = 0x00;
			change_pc(dPC);
		}
		break;

	case 0xc8:	/* RET	Z		*/
		{
			z80_cc += cc_op[0xc8];
			if (0 != (F & ZF)) {
				z80_cc += cc_ex[0xc8];
				POP(cpu, REG_PC);
				change_pc(dPC);
			}
		}
		break;

	case 0xc9:	/* RET			*/
		{
			z80_cc += cc_op[0xc9];
			POP(cpu, REG_PC);
			change_pc(dPC);
		}
		break;

	case 0xca:	/* JP	Z,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xca];
			if (0 != (F & ZF)) {
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xcb:	/* prefix CB xx 	*/
		{
			z80_cc += cc_op[0xcb];
			goto fetch_cb_xx;
		}
		break;

	case 0xcc:	/* CALL	Z,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xcc];
			if (0 != (F & ZF)) {
				z80_cc += cc_ex[0xcc];
				PUSH(cpu, REG_PC);
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xcd:	/* CALL	nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xcd];
			PUSH(cpu, REG_PC);
			PC = MP;
			change_pc(dPC);
		}
		break;

	case 0xce:	/* ADC	A,nn		*/
		{
			z80_cc += cc_op[0xce];
			A = ADC(cpu, RD_ARGB(cpu));
		}
		break;

	case 0xcf:	/* RST	08H		*/
		{
			z80_cc += cc_op[0xcf];
			PUSH(cpu, REG_PC);
			MP = PC = 0x08;
			change_pc(dPC);
		}
		break;

	case 0xd0:	/* RET	NC		*/
		{
			z80_cc += cc_op[0xd0];
			if (0 == (F & CF)) {
				z80_cc += cc_ex[0xd0];
				POP(cpu, REG_PC);
				change_pc(dPC);
			}
		}
		break;

	case 0xd1:	/* POP	DE		*/
		{
			z80_cc += cc_op[0xd1];
			POP(cpu, REG_DE);
		}
		break;

	case 0xd2:	/* JP	NC,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xd2];
			if (0 == (F & CF)) {
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xd3:	/* OUT	(nn),A		*/
		{
			MP = RD_ARGB(cpu);
			z80_cc += cc_op[0xd3];
			MP |= (dAF & 0xff00);
			WR_IO(dMP, A);
			MP++;
			MPH = A;
		}
		break;

	case 0xd4:	/* CALL	NC,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xd4];
			if (0 == (F & CF)) {
				z80_cc += cc_ex[0xd4];
				PUSH(cpu, REG_PC);
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xd5:	/* PUSH	DE		*/
		{
			z80_cc += cc_op[0xd5];
			PUSH(cpu, REG_DE);
		}
		break;

	case 0xd6:	/* SUB	nn		*/
		{
			z80_cc += cc_op[0xd6];
			A = SUB(cpu, RD_ARGB(cpu));
		}
		break;

	case 0xd7:	/* RST	10H		*/
		{
			z80_cc += cc_op[0xd7];
			PUSH(cpu, REG_PC);
			MP = PC = 0x10;
			change_pc(dPC);
		}
		break;

	case 0xd8:	/* RET	C		*/
		{
			z80_cc += cc_op[0xd8];
			if (0 != (F & CF)) {
				z80_cc += cc_ex[0xd8];
				POP(cpu, REG_PC);
				change_pc(dPC);
			}
		}
		break;

	case 0xd9:	/* EXX			*/
		{
			z80_cc += cc_op[0xd9];
			dBC2 ^= dBC; dBC ^= dBC2; dBC2 ^= dBC;
			dDE2 ^= dDE; dDE ^= dDE2; dDE2 ^= dDE;
			dHL2 ^= dHL; dHL ^= dHL2; dHL2 ^= dHL;
		}
		break;

	case 0xda:	/* JP	C,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xda];
			if (0 != (F & CF)) {
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xdb:	/* IN	A,(nn)		*/
		{
			MP = RD_ARGB(cpu);
			z80_cc += cc_op[0xdb];
			MP |= (dAF & 0xff00);
			A = RD_IO(dMP);
			MP++;
		}
		break;

	case 0xdc:	/* CALL	C,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xdc];
			if (0 != (F & CF)) {
				z80_cc += cc_ex[0xdc];
				PUSH(cpu, REG_PC);
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xdd:	/* prefix DD xx (IX)	*/
		{
			z80_cc += cc_op[0xdd];
			goto fetch_dd_xx;
		}
		break;

	case 0xde:	/* SBC	A,nn		*/
		{
			z80_cc += cc_op[0xde];
			A = SBC(cpu, RD_ARGB(cpu));
		}
		break;

	case 0xdf:	/* RST	18H		*/
		{
			z80_cc += cc_op[0xdf];
			PUSH(cpu, REG_PC);
			MP = PC = 0x18;
			change_pc(dPC);
		}
		break;

	case 0xe0:	/* RET	PO		*/
		{
			z80_cc += cc_op[0xe0];
			if (0 == (F & PF)) {
				z80_cc += cc_ex[0xe0];
				POP(cpu, REG_PC);
				change_pc(dPC);
			}
		}
		break;

	case 0xe1:	/* POP	HL		*/
		{
			z80_cc += cc_op[0xe1];
			POP(cpu, REG_HL);
		}
		break;

	case 0xe2:	/* JP	PO,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xe2];
			if (0 == (F & PF)) {
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xe3:	/* EX	(SP),HL		*/
		{
			z80_cc += cc_op[0xe3];
			MPL = RD_MEM(dSP);
			WR_MEM(dSP, L);
			SP++;
			L = MPL;
			MPH = RD_MEM(dSP);
			WR_MEM(dSP, H);
			SP--;
			H = MPH;
		}
		break;

	case 0xe4:	/* CALL	PO,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xe4];
			if (0 == (F & PF)) {
				z80_cc += cc_ex[0xe4];
				PUSH(cpu, REG_PC);
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xe5:	/* PUSH	HL		*/
		{
			z80_cc += cc_op[0xe5];
			PUSH(cpu, REG_HL);
		}
		break;

	case 0xe6:	/* AND	nn		*/
		{
			z80_cc += cc_op[0xe6];
			A = AND(cpu, RD_ARGB(cpu));
		}
		break;


	case 0xe7:	/* RST	20H		*/
		{
			z80_cc += cc_op[0xe7];
			PUSH(cpu, REG_PC);
			MP = PC = 0x20;
			change_pc(dPC);
		}
		break;

	case 0xe8:	/* RET	PE		*/
		{
			z80_cc += cc_op[0xe8];
			if (0 != (F & PF)) {
				z80_cc += cc_ex[0xe8];
				POP(cpu, REG_PC);
				change_pc(dPC);
			}
		}
		break;

	case 0xe9:	/* JP	(HL)		*/
		{
			z80_cc += cc_op[0xe9];
			PC = HL;
			change_pc(dPC);
		}
		break;

	case 0xea:	/* JP	PE,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xea];
			if (0 != (F & PF)) {
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xeb:	/* EX	DE,HL		*/
		{
			uint16_t tmp = DE;
			z80_cc += cc_op[0xeb];
			DE = HL;
			HL = tmp;
		}
		break;

	case 0xec:	/* CALL	PE,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xec];
			if (0 != (F & PF)) {
				z80_cc += cc_ex[0xec];
				PUSH(cpu, REG_PC);
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xed:	/* prefix ED xx		*/
		{
			z80_cc += cc_op[0xed];
			goto fetch_ed_xx;
		}
		break;

	case 0xee:	/* XOR	nn		*/
		{
			z80_cc += cc_op[0xee];
			A = XOR(cpu, RD_ARGB(cpu));
		}
		break;

	case 0xef:	/* RST	28H		*/
		{
			z80_cc += cc_op[0xef];
			PUSH(cpu, REG_PC);
			MP = PC = 0x28;
			change_pc(dPC);
		}
		break;

	case 0xf0:	/* RET	P		*/
		{
			z80_cc += cc_op[0xf0];
			if (0 == (F & SF)) {
				z80_cc += cc_ex[0xf0];
				POP(cpu, REG_PC);
				change_pc(dPC);
			}
		}
		break;

	case 0xf1:	/* POP	AF		*/
		{
			z80_cc += cc_op[0xf1];
			POP(cpu, REG_AF);
		}
		break;

	case 0xf2:	/* JP	P,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xf2];
			if (0 == (F & SF)) {
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xf3:	/* DI			*/
		{
			z80_cc += cc_op[0xf3];
			cpu->iff = 0;
		}
		break;

	case 0xf4:	/* CALL	P,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xf4];
			if (0 == (F & SF)) {
				z80_cc += cc_ex[0xf4];
				PUSH(cpu, REG_PC);
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xf5:	/* PUSH	AF		*/
		{
			z80_cc += cc_op[0xf5];
			PUSH(cpu, REG_AF);
		}
		break;

	case 0xf6:	/* OR	nn		*/
		{
			z80_cc += cc_op[0xf6];
			A = OR(cpu, RD_ARGB(cpu));
		}
		break;

	case 0xf7:	/* RST	30H		*/
		{
			z80_cc += cc_op[0xf7];
			PUSH(cpu, REG_PC);
			MP = PC = 0x30;
			change_pc(dPC);
		}
		break;

	case 0xf8:	/* RET	M		*/
		{
			z80_cc += cc_op[0xf8];
			if (0 != (F & SF)) {
				z80_cc += cc_ex[0xf8];
				POP(cpu, REG_PC);
				change_pc(dPC);
			}
		}
		break;

	case 0xf9:	/* LD	SP,HL */
		{
			z80_cc += cc_op[0xf9];
			SP = HL;
		}
		break;

	case 0xfa:	/* JP	M,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xfa];
			if (0 != (F & SF)) {
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xfb:	/* EI			*/
		{
			z80_cc += cc_op[0xfb];
			cpu->iff = 3;
		}
		break;

	case 0xfc:	/* CALL	M,nnnn		*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_op[0xfc];
			if (0 != (F & SF)) {
				z80_cc += 7;
				PUSH(cpu, REG_PC);
				PC = MP;
				change_pc(dPC);
			}
		}
		break;

	case 0xfd:	/* prefix FD xx (IY)	*/
		{
			z80_cc += cc_op[0xfd];
			goto fetch_fd_xx;
		}
		break;

	case 0xfe:	/* CP	nn		*/
		{
			z80_cc += cc_op[0xfe];
			CP(cpu, RD_ARGB(cpu));
		}
		break;

	case 0xff:	/* RST	38H		*/
		{
			z80_cc += cc_op[0xff];
			PUSH(cpu, REG_PC);
			MP = PC = 0x38;
			change_pc(dPC);
		}
		break;
	}
	if (z80_cc < cycles)
		goto fetch_xx;
	return z80_cc;

fetch_cb_xx:
	op = RD_OP(cpu);
	switch (op) {
	case 0x00:	/* RLC	B		*/
		{
			z80_cc += cc_cb[0x00];
			B = RLC(cpu, B);
		}
		break;

	case 0x01:	/* RLC	C		*/
		{
			z80_cc += cc_cb[0x01];
			C = RLC(cpu, C);
		}
		break;

	case 0x02:	/* RLC	D		*/
		{
			z80_cc += cc_cb[0x02];
			D = RLC(cpu, D);
		}
		break;

	case 0x03:	/* RLC	E		*/
		{
			z80_cc += cc_cb[0x03];
			E = RLC(cpu, E);
		}
		break;

	case 0x04:	/* RLC	H		*/
		{
			z80_cc += cc_cb[0x04];
			H = RLC(cpu, H);
		}
		break;

	case 0x05:	/* RLC	L		*/
		{
			z80_cc += cc_cb[0x05];
			L = RLC(cpu, L);
		}
		break;

	case 0x06:	/* RLC	(HL)		*/
		{
			z80_cc += cc_cb[0x06];
			WR_MEM(dHL, RLC(cpu, RD_MEM(dHL)));
		}
		break;

	case 0x07:	/* RLC	A		*/
		{
			z80_cc += cc_cb[0x07];
			A = RLC(cpu, A);
		}
		break;

	case 0x08:	/* RRC	B		*/
		{
			z80_cc += cc_cb[0x08];
			B = RRC(cpu, B);
		}
		break;

	case 0x09:	/* RRC	C		*/
		{
			z80_cc += cc_cb[0x09];
			C = RRC(cpu, C);
		}
		break;

	case 0x0a:	/* RRC	D		*/
		{
			z80_cc += cc_cb[0x0a];
			D = RRC(cpu, D);
		}
		break;

	case 0x0b:	/* RRC	E		*/
		{
			z80_cc += cc_cb[0x0b];
			E = RRC(cpu, E);
		}
		break;

	case 0x0c:	/* RRC	H		*/
		{
			z80_cc += cc_cb[0x0c];
			H = RRC(cpu, H);
		}
		break;

	case 0x0d:	/* RRC	L		*/
		{
			z80_cc += cc_cb[0x0d];
			L = RRC(cpu, L);
		}
		break;

	case 0x0e:	/* RRC	(HL)		*/
		{
			z80_cc += cc_cb[0x0e];
			WR_MEM(dHL, RRC(cpu, RD_MEM(dHL)));
		}
		break;

	case 0x0f:	/* RRC	A		*/
		{
			z80_cc += cc_cb[0x0f];
			A = RRC(cpu, A);
		}
		break;

	case 0x10:	/* RL	B		*/
		{
			z80_cc += cc_cb[0x10];
			B = RL(cpu, B);
		}
		break;

	case 0x11:	/* RL	C		*/
		{
			z80_cc += cc_cb[0x11];
			C = RL(cpu, C);
		}
		break;

	case 0x12:	/* RL	D		*/
		{
			z80_cc += cc_cb[0x12];
			D = RL(cpu, D);
		}
		break;

	case 0x13:	/* RL	E		*/
		{
			z80_cc += cc_cb[0x13];
			E = RL(cpu, E);
		}
		break;

	case 0x14:	/* RL	H		*/
		{
			z80_cc += cc_cb[0x14];
			H = RL(cpu, H);
		}
		break;

	case 0x15:	/* RL	L		*/
		{
			z80_cc += cc_cb[0x15];
			L = RL(cpu, L);
		}
		break;

	case 0x16:	/* RL	(HL)		*/
		{
			z80_cc += cc_cb[0x16];
			WR_MEM(dHL, RL(cpu, RD_MEM(dHL)));
		}
		break;

	case 0x17:	/* RL	A		*/
		{
			z80_cc += cc_cb[0x17];
			A = RL(cpu, A);
		}
		break;

	case 0x18:	/* RR	B		*/
		{
			z80_cc += cc_cb[0x18];
			B = RR(cpu, B);
		}
		break;

	case 0x19:	/* RR	C		*/
		{
			z80_cc += cc_cb[0x19];
			C = RR(cpu, C);
		}
		break;

	case 0x1a:	/* RR	D		*/
		{
			z80_cc += cc_cb[0x1a];
			D = RR(cpu, D);
		}
		break;

	case 0x1b:	/* RR	E		*/
		{
			z80_cc += cc_cb[0x1b];
			E = RR(cpu, E);
		}
		break;

	case 0x1c:	/* RR	H		*/
		{
			z80_cc += cc_cb[0x1c];
			H = RR(cpu, H);
		}
		break;

	case 0x1d:	/* RR	L		*/
		{
			z80_cc += cc_cb[0x1d];
			L = RR(cpu, L);
		}
		break;

	case 0x1e:	/* RR	(HL)		*/
		{
			z80_cc += cc_cb[0x1e];
			WR_MEM(dHL, RR(cpu, RD_MEM(dHL)));
		}
		break;

	case 0x1f:	/* RR	A		*/
		{
			z80_cc += cc_cb[0x1f];
			A = RR(cpu, A);
		}
		break;

	case 0x20:	/* SLA	B		*/
		{
			z80_cc += cc_cb[0x20];
			B = SLA(cpu, B);
		}
		break;

	case 0x21:	/* SLA	C		*/
		{
			z80_cc += cc_cb[0x21];
			C = SLA(cpu, C);
		}
		break;

	case 0x22:	/* SLA	D		*/
		{
			z80_cc += cc_cb[0x22];
			D = SLA(cpu, D);
		}
		break;

	case 0x23:	/* SLA	E		*/
		{
			z80_cc += cc_cb[0x23];
			E = SLA(cpu, E);
		}
		break;

	case 0x24:	/* SLA	H		*/
		{
			z80_cc += cc_cb[0x24];
			H = SLA(cpu, H);
		}
		break;

	case 0x25:	/* SLA	L		*/
		{
			z80_cc += cc_cb[0x25];
			L = SLA(cpu, L);
		}
		break;

	case 0x26:	/* SLA	(HL)		*/
		{
			z80_cc += cc_cb[0x26];
			WR_MEM(dHL, SLA(cpu, RD_MEM(dHL)));
		}
		break;

	case 0x27:	/* SLA	A		*/
		{
			z80_cc += cc_cb[0x27];
			A = SLA(cpu, A);
		}
		break;

	case 0x28:	/* SRA	B		*/
		{
			z80_cc += cc_cb[0x28];
			B = SRA(cpu, B);
		}
		break;

	case 0x29:	/* SRA	C		*/
		{
			z80_cc += cc_cb[0x29];
			C = SRA(cpu, C);
		}
		break;

	case 0x2a:	/* SRA	D		*/
		{
			z80_cc += cc_cb[0x2a];
			D = SRA(cpu, D);
		}
		break;

	case 0x2b:	/* SRA	E		*/
		{
			z80_cc += cc_cb[0x2b];
			E = SRA(cpu, E);
		}
		break;

	case 0x2c:	/* SRA	H		*/
		{
			z80_cc += cc_cb[0x2c];
			H = SRA(cpu, H);
		}
		break;

	case 0x2d:	/* SRA	L		*/
		{
			z80_cc += cc_cb[0x2d];
			L = SRA(cpu, L);
		}
		break;

	case 0x2e:	/* SRA	(HL)		*/
		{
			z80_cc += cc_cb[0x2e];
			WR_MEM(dHL, SRA(cpu, RD_MEM(dHL)));
		}
		break;

	case 0x2f:	/* SRA	A		*/
		{
			z80_cc += cc_cb[0x2f];
			A = SRA(cpu, A);
		}
		break;

	case 0x30:	/* SLL	B		*/
		{
			z80_cc += cc_cb[0x30];
			B = SLL(cpu, B);
		}
		break;

	case 0x31:	/* SLL	C		*/
		{
			z80_cc += cc_cb[0x31];
			C = SLL(cpu, C);
		}
		break;

	case 0x32:	/* SLL	D		*/
		{
			z80_cc += cc_cb[0x32];
			D = SLL(cpu, D);
		}
		break;

	case 0x33:	/* SLL	E		*/
		{
			z80_cc += cc_cb[0x33];
			E = SLL(cpu, E);
		}
		break;

	case 0x34:	/* SLL	H		*/
		{
			z80_cc += cc_cb[0x34];
			H = SLL(cpu, H);
		}
		break;

	case 0x35:	/* SLL	L		*/
		{
			z80_cc += cc_cb[0x35];
			L = SLL(cpu, L);
		}
		break;

	case 0x36:	/* SLL	(HL)		*/
		{
			z80_cc += cc_cb[0x36];
			WR_MEM(dHL, SLL(cpu, RD_MEM(dHL)));
		}
		break;

	case 0x37:	/* SLL	A		*/
		{
			z80_cc += cc_cb[0x37];
			A = SLL(cpu, A);
		}
		break;

	case 0x38:	/* SRL	B		*/
		{
			z80_cc += cc_cb[0x38];
			B = SRL(cpu, B);
		}
		break;

	case 0x39:	/* SRL	C		*/
		{
			z80_cc += cc_cb[0x39];
			C = SRL(cpu, C);
		}
		break;

	case 0x3a:	/* SRL	D		*/
		{
			z80_cc += cc_cb[0x3a];
			D = SRL(cpu, D);
		}
		break;

	case 0x3b:	/* SRL	E		*/
		{
			z80_cc += cc_cb[0x3b];
			E = SRL(cpu, E);
		}
		break;

	case 0x3c:	/* SRL	H		*/
		{
			z80_cc += cc_cb[0x3c];
			H = SRL(cpu, H);
		}
		break;

	case 0x3d:	/* SRL	L		*/
		{
			z80_cc += cc_cb[0x3d];
			L = SRL(cpu, L);
		}
		break;

	case 0x3e:	/* SRL	(HL)		*/
		{
			z80_cc += cc_cb[0x3e];
			WR_MEM(dHL, SRL(cpu, RD_MEM(dHL)));
		}
		break;

	case 0x3f:	/* SRL	A		*/
		{
			z80_cc += cc_cb[0x3f];
			A = SRL(cpu, A);
		}
		break;

	case 0x40:	/* BIT	0,B		*/
		{
			z80_cc += cc_cb[0x40];
			BIT(cpu, 0, B);
		}
		break;

	case 0x41:	/* BIT	0,C		*/
		{
			z80_cc += cc_cb[0x41];
			BIT(cpu, 0, C);
		}
		break;

	case 0x42:	/* BIT	0,D		*/
		{
			z80_cc += cc_cb[0x42];
			BIT(cpu, 0, D);
		}
		break;

	case 0x43:	/* BIT	0,E		*/
		{
			z80_cc += cc_cb[0x43];
			BIT(cpu, 0, E);
		}
		break;

	case 0x44:	/* BIT	0,H		*/
		{
			z80_cc += cc_cb[0x44];
			BIT(cpu, 0, H);
		}
		break;

	case 0x45:	/* BIT	0,L		*/
		{
			z80_cc += cc_cb[0x45];
			BIT(cpu, 0, L);
		}
		break;

	case 0x46:	/* BIT	0,(HL)		*/
		{
			z80_cc += cc_cb[0x46];
			BIT_HL(cpu, 0, RD_MEM(dHL));
		}
		break;

	case 0x47:	/* BIT	0,A		*/
		{
			z80_cc += cc_cb[0x47];
			BIT(cpu, 0, A);
		}
		break;

	case 0x48:	/* BIT	1,B		*/
		{
			z80_cc += cc_cb[0x48];
			BIT(cpu, 1, B);
		}
		break;

	case 0x49:	/* BIT	1,C		*/
		{
			z80_cc += cc_cb[0x49];
			BIT(cpu, 1, C);
		}
		break;

	case 0x4a:	/* BIT	1,D		*/
		{
			z80_cc += cc_cb[0x4a];
			BIT(cpu, 1, D);
		}
		break;

	case 0x4b:	/* BIT	1,E		*/
		{
			z80_cc += cc_cb[0x4b];
			BIT(cpu, 1, E);
		}
		break;

	case 0x4c:	/* BIT	1,H		*/
		{
			z80_cc += cc_cb[0x4c];
			BIT(cpu, 1, H);
		}
		break;

	case 0x4d:	/* BIT	1,L		*/
		{
			z80_cc += cc_cb[0x4d];
			BIT(cpu, 1, L);
		}
		break;

	case 0x4e:	/* BIT	1,(HL)		*/
		{
			z80_cc += cc_cb[0x4e];
			BIT_HL(cpu, 1, RD_MEM(dHL));
		}
		break;

	case 0x4f:	/* BIT	1,A		*/
		{
			z80_cc += cc_cb[0x4f];
			BIT(cpu, 1, A);
		}
		break;

	case 0x50:	/* BIT	2,B		*/
		{
			z80_cc += cc_cb[0x50];
			BIT(cpu, 2, B);
		}
		break;

	case 0x51:	/* BIT	2,C		*/
		{
			z80_cc += cc_cb[0x51];
			BIT(cpu, 2, C);
		}
		break;

	case 0x52:	/* BIT	2,D		*/
		{
			z80_cc += cc_cb[0x52];
			BIT(cpu, 2, D);
		}
		break;

	case 0x53:	/* BIT	2,E		*/
		{
			z80_cc += cc_cb[0x53];
			BIT(cpu, 2, E);
		}
		break;

	case 0x54:	/* BIT	2,H		*/
		{
			z80_cc += cc_cb[0x54];
			BIT(cpu, 2, H);
		}
		break;

	case 0x55:	/* BIT	2,L		*/
		{
			z80_cc += cc_cb[0x55];
			BIT(cpu, 2, L);
		}
		break;

	case 0x56:	/* BIT	2,(HL)		*/
		{
			z80_cc += cc_cb[0x56];
			BIT_HL(cpu, 2, RD_MEM(dHL));
		}
		break;

	case 0x57:	/* BIT	2,A		*/
		{
			z80_cc += cc_cb[0x57];
			BIT(cpu, 2, A);
		}
		break;

	case 0x58:	/* BIT	3,B		*/
		{
			z80_cc += cc_cb[0x58];
			BIT(cpu, 3, B);
		}
		break;

	case 0x59:	/* BIT	3,C		*/
		{
			z80_cc += cc_cb[0x59];
			BIT(cpu, 3, C);
		}
		break;

	case 0x5a:	/* BIT	3,D		*/
		{
			z80_cc += cc_cb[0x5a];
			BIT(cpu, 3, D);
		}
		break;

	case 0x5b:	/* BIT	3,E		*/
		{
			z80_cc += cc_cb[0x5b];
			BIT(cpu, 3, E);
		}
		break;

	case 0x5c:	/* BIT	3,H		*/
		{
			z80_cc += cc_cb[0x5c];
			BIT(cpu, 3, H);
		}
		break;

	case 0x5d:	/* BIT	3,L		*/
		{
			z80_cc += cc_cb[0x5d];
			BIT(cpu, 3, L);
		}
		break;

	case 0x5e:	/* BIT	3,(HL)		*/
		{
			z80_cc += cc_cb[0x5e];
			BIT_HL(cpu, 3, RD_MEM(dHL));
		}
		break;

	case 0x5f:	/* BIT	3,A		*/
		{
			z80_cc += cc_cb[0x5f];
			BIT(cpu, 3, A);
		}
		break;

	case 0x60:	/* BIT	4,B		*/
		{
			z80_cc += cc_cb[0x60];
			BIT(cpu, 4, B);
		}
		break;

	case 0x61:	/* BIT	4,C		*/
		{
			z80_cc += cc_cb[0x61];
			BIT(cpu, 4, C);
		}
		break;

	case 0x62:	/* BIT	4,D		*/
		{
			z80_cc += cc_cb[0x62];
			BIT(cpu, 4, D);
		}
		break;

	case 0x63:	/* BIT	4,E		*/
		{
			z80_cc += cc_cb[0x63];
			BIT(cpu, 4, E);
		}
		break;

	case 0x64:	/* BIT	4,H		*/
		{
			z80_cc += cc_cb[0x64];
			BIT(cpu, 4, H);
		}
		break;

	case 0x65:	/* BIT	4,L		*/
		{
			z80_cc += cc_cb[0x65];
			BIT(cpu, 4, L);
		}
		break;

	case 0x66:	/* BIT	4,(HL)		*/
		{
			z80_cc += cc_cb[0x66];
			BIT_HL(cpu, 4, RD_MEM(dHL));
		}
		break;

	case 0x67:	/* BIT	4,A		*/
		{
			z80_cc += cc_cb[0x67];
			BIT(cpu, 4, A);
		}
		break;

	case 0x68:	/* BIT	5,B		*/
		{
			z80_cc += cc_cb[0x68];
			BIT(cpu, 5, B);
		}
		break;

	case 0x69:	/* BIT	5,C		*/
		{
			z80_cc += cc_cb[0x69];
			BIT(cpu, 5, C);
		}
		break;

	case 0x6a:	/* BIT	5,D		*/
		{
			z80_cc += cc_cb[0x6a];
			BIT(cpu, 5, D);
		}
		break;

	case 0x6b:	/* BIT	5,E		*/
		{
			z80_cc += cc_cb[0x6b];
			BIT(cpu, 5, E);
		}
		break;

	case 0x6c:	/* BIT	5,H		*/
		{
			z80_cc += cc_cb[0x6c];
			BIT(cpu, 5, H);
		}
		break;

	case 0x6d:	/* BIT	5,L		*/
		{
			z80_cc += cc_cb[0x6d];
			BIT(cpu, 5, L);
		}
		break;

	case 0x6e:	/* BIT	5,(HL)		*/
		{
			z80_cc += cc_cb[0x6e];
			BIT_HL(cpu, 5, RD_MEM(dHL));
		}
		break;

	case 0x6f:	/* BIT	5,A		*/
		{
			z80_cc += cc_cb[0x6f];
			BIT(cpu, 5, A);
		}
		break;

	case 0x70:	/* BIT	6,B		*/
		{
			z80_cc += cc_cb[0x70];
			BIT(cpu, 6, B);
		}
		break;

	case 0x71:	/* BIT	6,C		*/
		{
			z80_cc += cc_cb[0x71];
			BIT(cpu, 6, C);
		}
		break;

	case 0x72:	/* BIT	6,D		*/
		{
			z80_cc += cc_cb[0x72];
			BIT(cpu, 6, D);
		}
		break;

	case 0x73:	/* BIT	6,E		*/
		{
			z80_cc += cc_cb[0x73];
			BIT(cpu, 6, E);
		}
		break;

	case 0x74:	/* BIT	6,H		*/
		{
			z80_cc += cc_cb[0x74];
			BIT(cpu, 6, H);
		}
		break;

	case 0x75:	/* BIT	6,L		*/
		{
			z80_cc += cc_cb[0x75];
			BIT(cpu, 6, L);
		}
		break;

	case 0x76:	/* BIT	6,(HL)		*/
		{
			z80_cc += cc_cb[0x76];
			BIT_HL(cpu, 6, RD_MEM(dHL));
		}
		break;

	case 0x77:	/* BIT	6,A		*/
		{
			z80_cc += cc_cb[0x77];
			BIT(cpu, 6, A);
		}
		break;

	case 0x78:	/* BIT	7,B		*/
		{
			z80_cc += cc_cb[0x78];
			BIT(cpu, 7, B);
		}
		break;

	case 0x79:	/* BIT	7,C		*/
		{
			z80_cc += cc_cb[0x79];
			BIT(cpu, 7, C);
		}
		break;

	case 0x7a:	/* BIT	7,D		*/
		{
			z80_cc += cc_cb[0x7a];
			BIT(cpu, 7, D);
		}
		break;

	case 0x7b:	/* BIT	7,E		*/
		{
			z80_cc += cc_cb[0x7b];
			BIT(cpu, 7, E);
		}
		break;

	case 0x7c:	/* BIT	7,H		*/
		{
			z80_cc += cc_cb[0x7c];
			BIT(cpu, 7, H);
		}
		break;

	case 0x7d:	/* BIT	7,L		*/
		{
			z80_cc += cc_cb[0x7d];
			BIT(cpu, 7, L);
		}
		break;

	case 0x7e:	/* BIT	7,(HL)		*/
		{
			z80_cc += cc_cb[0x7e];
			BIT_HL(cpu, 7, RD_MEM(dHL));
		}
		break;

	case 0x7f:	/* BIT	7,A		*/
		{
			z80_cc += cc_cb[0x7f];
			BIT(cpu, 7, A);
		}
		break;

	case 0x80:	/* RES	0,B		*/
		{
			z80_cc += cc_cb[0x80];
			B = RES(cpu, 0, B);
		}
		break;

	case 0x81:	/* RES	0,C		*/
		{
			z80_cc += cc_cb[0x81];
			C = RES(cpu, 0, C);
		}
		break;

	case 0x82:	/* RES	0,D		*/
		{
			z80_cc += cc_cb[0x82];
			D = RES(cpu, 0, D);
		}
		break;

	case 0x83:	/* RES	0,E		*/
		{
			z80_cc += cc_cb[0x83];
			E = RES(cpu, 0, E);
		}
		break;

	case 0x84:	/* RES	0,H		*/
		{
			z80_cc += cc_cb[0x84];
			H = RES(cpu, 0, H);
		}
		break;

	case 0x85:	/* RES	0,L		*/
		{
			z80_cc += cc_cb[0x85];
			L = RES(cpu, 0, L);
		}
		break;

	case 0x86:	/* RES	0,(HL)		*/
		{
			z80_cc += cc_cb[0x86];
			WR_MEM(dHL, RES(cpu, 0, RD_MEM(dHL)));
		}
		break;

	case 0x87:	/* RES	0,A		*/
		{
			z80_cc += cc_cb[0x87];
			A = RES(cpu, 0, A);
		}
		break;

	case 0x88:	/* RES	1,B		*/
		{
			z80_cc += cc_cb[0x88];
			B = RES(cpu, 1, B);
		}
		break;

	case 0x89:	/* RES	1,C		*/
		{
			z80_cc += cc_cb[0x89];
			C = RES(cpu, 1, C);
		}
		break;

	case 0x8a:	/* RES	1,D		*/
		{
			z80_cc += cc_cb[0x8a];
			D = RES(cpu, 1, D);
		}
		break;

	case 0x8b:	/* RES	1,E		*/
		{
			z80_cc += cc_cb[0x8b];
			E = RES(cpu, 1, E);
		}
		break;

	case 0x8c:	/* RES	1,H		*/
		{
			z80_cc += cc_cb[0x8c];
			H = RES(cpu, 1, H);
		}
		break;

	case 0x8d:	/* RES	1,L		*/
		{
			z80_cc += cc_cb[0x8d];
			L = RES(cpu, 1, L);
		}
		break;

	case 0x8e:	/* RES	1,(HL)		*/
		{
			z80_cc += cc_cb[0x8e];
			WR_MEM(dHL, RES(cpu, 1, RD_MEM(dHL)));
		}
		break;

	case 0x8f:	/* RES	1,A		*/
		{
			z80_cc += cc_cb[0x8f];
			A = RES(cpu, 1, A);
		}
		break;

	case 0x90:	/* RES	2,B		*/
		{
			z80_cc += cc_cb[0x90];
			B = RES(cpu, 2, B);
		}
		break;

	case 0x91:	/* RES	2,C		*/
		{
			z80_cc += cc_cb[0x91];
			C = RES(cpu, 2, C);
		}
		break;

	case 0x92:	/* RES	2,D		*/
		{
			z80_cc += cc_cb[0x92];
			D = RES(cpu, 2, D);
		}
		break;

	case 0x93:	/* RES	2,E		*/
		{
			z80_cc += cc_cb[0x93];
			E = RES(cpu, 2, E);
		}
		break;

	case 0x94:	/* RES	2,H		*/
		{
			z80_cc += cc_cb[0x94];
			H = RES(cpu, 2, H);
		}
		break;

	case 0x95:	/* RES	2,L		*/
		{
			z80_cc += cc_cb[0x95];
			L = RES(cpu, 2, L);
		}
		break;

	case 0x96:	/* RES	2,(HL)		*/
		{
			z80_cc += cc_cb[0x96];
			WR_MEM(dHL, RES(cpu, 2, RD_MEM(dHL)));
		}
		break;

	case 0x97:	/* RES	2,A		*/
		{
			z80_cc += cc_cb[0x97];
			A = RES(cpu, 2, A);
		}
		break;

	case 0x98:	/* RES	3,B		*/
		{
			z80_cc += cc_cb[0x98];
			B = RES(cpu, 3, B);
		}
		break;

	case 0x99:	/* RES	3,C		*/
		{
			z80_cc += cc_cb[0x99];
			C = RES(cpu, 3, C);
		}
		break;

	case 0x9a:	/* RES	3,D		*/
		{
			z80_cc += cc_cb[0x9a];
			D = RES(cpu, 3, D);
		}
		break;

	case 0x9b:	/* RES	3,E		*/
		{
			z80_cc += cc_cb[0x9b];
			E = RES(cpu, 3, E);
		}
		break;

	case 0x9c:	/* RES	3,H		*/
		{
			z80_cc += cc_cb[0x9c];
			H = RES(cpu, 3, H);
		}
		break;

	case 0x9d:	/* RES	3,L		*/
		{
			z80_cc += cc_cb[0x9d];
			L = RES(cpu, 3, L);
		}
		break;

	case 0x9e:	/* RES	3,(HL)		*/
		{
			z80_cc += cc_cb[0x9e];
			WR_MEM(dHL, RES(cpu, 3, RD_MEM(dHL)));
		}
		break;

	case 0x9f:	/* RES	3,A		*/
		{
			z80_cc += cc_cb[0x9f];
			A = RES(cpu, 3, A);
		}
		break;

	case 0xa0:	/* RES	4,B		*/
		{
			z80_cc += cc_cb[0xa0];
			B = RES(cpu, 4, B);
		}
		break;

	case 0xa1:	/* RES	4,C		*/
		{
			z80_cc += cc_cb[0xa1];
			C = RES(cpu, 4, C);
		}
		break;

	case 0xa2:	/* RES	4,D		*/
		{
			z80_cc += cc_cb[0xa2];
			D = RES(cpu, 4, D);
		}
		break;

	case 0xa3:	/* RES	4,E		*/
		{
			z80_cc += cc_cb[0xa3];
			E = RES(cpu, 4, E);
		}
		break;

	case 0xa4:	/* RES	4,H		*/
		{
			z80_cc += cc_cb[0xa4];
			H = RES(cpu, 4, H);
		}
		break;

	case 0xa5:	/* RES	4,L		*/
		{
			z80_cc += cc_cb[0xa5];
			L = RES(cpu, 4, L);
		}
		break;

	case 0xa6:	/* RES	4,(HL)		*/
		{
			z80_cc += cc_cb[0xa6];
			WR_MEM(dHL, RES(cpu, 4, RD_MEM(dHL)));
		}
		break;

	case 0xa7:	/* RES	4,A		*/
		{
			z80_cc += cc_cb[0xa7];
			A = RES(cpu, 4, A);
		}
		break;

	case 0xa8:	/* RES	5,B		*/
		{
			z80_cc += cc_cb[0xa8];
			B = RES(cpu, 5, B);
		}
		break;

	case 0xa9:	/* RES	5,C		*/
		{
			z80_cc += cc_cb[0xa9];
			C = RES(cpu, 5, C);
		}
		break;

	case 0xaa:	/* RES	5,D		*/
		{
			z80_cc += cc_cb[0xaa];
			D = RES(cpu, 5, D);
		}
		break;

	case 0xab:	/* RES	5,E		*/
		{
			z80_cc += cc_cb[0xab];
			E = RES(cpu, 5, E);
		}
		break;

	case 0xac:	/* RES	5,H		*/
		{
			z80_cc += cc_cb[0xac];
			H = RES(cpu, 5, H);
		}
		break;

	case 0xad:	/* RES	5,L		*/
		{
			z80_cc += cc_cb[0xad];
			L = RES(cpu, 5, L);
		}
		break;

	case 0xae:	/* RES	5,(HL)		*/
		{
			z80_cc += cc_cb[0xae];
			WR_MEM(dHL, RES(cpu, 5, RD_MEM(dHL)));
		}
		break;

	case 0xaf:	/* RES	5,A		*/
		{
			z80_cc += cc_cb[0xaf];
			A = RES(cpu, 5, A);
		}
		break;

	case 0xb0:	/* RES	6,B		*/
		{
			z80_cc += cc_cb[0xb0];
			B = RES(cpu, 6, B);
		}
		break;

	case 0xb1:	/* RES	6,C		*/
		{
			z80_cc += cc_cb[0xb1];
			C = RES(cpu, 6, C);
		}
		break;

	case 0xb2:	/* RES	6,D		*/
		{
			z80_cc += cc_cb[0xb2];
			D = RES(cpu, 6, D);
		}
		break;

	case 0xb3:	/* RES	6,E		*/
		{
			z80_cc += cc_cb[0xb3];
			E = RES(cpu, 6, E);
		}
		break;

	case 0xb4:	/* RES	6,H		*/
		{
			z80_cc += cc_cb[0xb4];
			H = RES(cpu, 6, H);
		}
		break;

	case 0xb5:	/* RES	6,L		*/
		{
			z80_cc += cc_cb[0xb5];
			L = RES(cpu, 6, L);
		}
		break;

	case 0xb6:	/* RES	6,(HL)		*/
		{
			z80_cc += cc_cb[0xb6];
			WR_MEM(dHL, RES(cpu, 6, RD_MEM(dHL)));
		}
		break;

	case 0xb7:	/* RES	6,A		*/
		{
			z80_cc += cc_cb[0xb7];
			A = RES(cpu, 6, A);
		}
		break;

	case 0xb8:	/* RES	7,B		*/
		{
			z80_cc += cc_cb[0xb8];
			B = RES(cpu, 7, B);
		}
		break;

	case 0xb9:	/* RES	7,C		*/
		{
			z80_cc += cc_cb[0xb9];
			C = RES(cpu, 7, C);
		}
		break;

	case 0xba:	/* RES	7,D		*/
		{
			z80_cc += cc_cb[0xba];
			D = RES(cpu, 7, D);
		}
		break;

	case 0xbb:	/* RES	7,E		*/
		{
			z80_cc += cc_cb[0xbb];
			E = RES(cpu, 7, E);
		}
		break;

	case 0xbc:	/* RES	7,H		*/
		{
			z80_cc += cc_cb[0xbc];
			H = RES(cpu, 7, H);
		}
		break;

	case 0xbd:	/* RES	7,L		*/
		{
			z80_cc += cc_cb[0xbd];
			L = RES(cpu, 7, L);
		}
		break;

	case 0xbe:	/* RES	7,(HL)		*/
		{
			z80_cc += cc_cb[0xbe];
			WR_MEM(dHL, RES(cpu, 7, RD_MEM(dHL)));
		}
		break;

	case 0xbf:	/* RES	7,A		*/
		{
			z80_cc += cc_cb[0xbf];
			A = RES(cpu, 7, A);
		}
		break;

	case 0xc0:	/* SET	0,B		*/
		{
			z80_cc += cc_cb[0xc0];
			B = SET(cpu, 0, B);
		}
		break;

	case 0xc1:	/* SET	0,C		*/
		{
			z80_cc += cc_cb[0xc1];
			C = SET(cpu, 0, C);
		}
		break;

	case 0xc2:	/* SET	0,D		*/
		{
			z80_cc += cc_cb[0xc2];
			D = SET(cpu, 0, D);
		}
		break;

	case 0xc3:	/* SET	0,E		*/
		{
			z80_cc += cc_cb[0xc3];
			E = SET(cpu, 0, E);
		}
		break;

	case 0xc4:	/* SET	0,H		*/
		{
			z80_cc += cc_cb[0xc4];
			H = SET(cpu, 0, H);
		}
		break;

	case 0xc5:	/* SET	0,L		*/
		{
			z80_cc += cc_cb[0xc5];
			L = SET(cpu, 0, L);
		}
		break;

	case 0xc6:	/* SET	0,(HL)		*/
		{
			z80_cc += cc_cb[0xc6];
			WR_MEM(dHL, SET(cpu, 0, RD_MEM(dHL)));
		}
		break;

	case 0xc7:	/* SET	0,A		*/
		{
			z80_cc += cc_cb[0xc7];
			A = SET(cpu, 0, A);
		}
		break;

	case 0xc8:	/* SET	1,B		*/
		{
			z80_cc += cc_cb[0xc8];
			B = SET(cpu, 1, B);
		}
		break;

	case 0xc9:	/* SET	1,C		*/
		{
			z80_cc += cc_cb[0xc9];
			C = SET(cpu, 1, C);
		}
		break;

	case 0xca:	/* SET	1,D		*/
		{
			z80_cc += cc_cb[0xca];
			D = SET(cpu, 1, D);
		}
		break;

	case 0xcb:	/* SET	1,E		*/
		{
			z80_cc += cc_cb[0xcb];
			E = SET(cpu, 1, E);
		}
		break;

	case 0xcc:	/* SET	1,H		*/
		{
			z80_cc += cc_cb[0xcc];
			H = SET(cpu, 1, H);
		}
		break;

	case 0xcd:	/* SET	1,L		*/
		{
			z80_cc += cc_cb[0xcd];
			L = SET(cpu, 1, L);
		}
		break;

	case 0xce:	/* SET	1,(HL)		*/
		{
			z80_cc += cc_cb[0xce];
			WR_MEM(dHL, SET(cpu, 1, RD_MEM(dHL)));
		}
		break;

	case 0xcf:	/* SET	1,A		*/
		{
			z80_cc += cc_cb[0xcf];
			A = SET(cpu, 1, A);
		}
		break;

	case 0xd0:	/* SET	2,B		*/
		{
			z80_cc += cc_cb[0xd0];
			B = SET(cpu, 2, B);
		}
		break;

	case 0xd1:	/* SET	2,C		*/
		{
			z80_cc += cc_cb[0xd1];
			C = SET(cpu, 2, C);
		}
		break;

	case 0xd2:	/* SET	2,D		*/
		{
			z80_cc += cc_cb[0xd2];
			D = SET(cpu, 2, D);
		}
		break;

	case 0xd3:	/* SET	2,E		*/
		{
			z80_cc += cc_cb[0xd3];
			E = SET(cpu, 2, E);
		}
		break;

	case 0xd4:	/* SET	2,H		*/
		{
			z80_cc += cc_cb[0xd4];
			H = SET(cpu, 2, H);
		}
		break;

	case 0xd5:	/* SET	2,L		*/
		{
			z80_cc += cc_cb[0xd5];
			L = SET(cpu, 2, L);
		}
		break;

	case 0xd6:	/* SET	2,(HL)		*/
		{
			z80_cc += cc_cb[0xd6];
			WR_MEM(dHL, SET(cpu, 2, RD_MEM(dHL)));
		}
		break;

	case 0xd7:	/* SET	2,A		*/
		{
			z80_cc += cc_cb[0xd7];
			A = SET(cpu, 2, A);
		}
		break;

	case 0xd8:	/* SET	3,B		*/
		{
			z80_cc += cc_cb[0xd8];
			B = SET(cpu, 3, B);
		}
		break;

	case 0xd9:	/* SET	3,C		*/
		{
			z80_cc += cc_cb[0xd9];
			C = SET(cpu, 3, C);
		}
		break;

	case 0xda:	/* SET	3,D		*/
		{
			z80_cc += cc_cb[0xda];
			D = SET(cpu, 3, D);
		}
		break;

	case 0xdb:	/* SET	3,E		*/
		{
			z80_cc += cc_cb[0xdb];
			E = SET(cpu, 3, E);
		}
		break;

	case 0xdc:	/* SET	3,H		*/
		{
			z80_cc += cc_cb[0xdc];
			H = SET(cpu, 3, H);
		}
		break;

	case 0xdd:	/* SET	3,L		*/
		{
			z80_cc += cc_cb[0xdd];
			L = SET(cpu, 3, L);
		}
		break;

	case 0xde:	/* SET	3,(HL)		*/
		{
			z80_cc += cc_cb[0xde];
			WR_MEM(dHL, SET(cpu, 3, RD_MEM(dHL)));
		}
		break;

	case 0xdf:	/* SET	3,A		*/
		{
			z80_cc += cc_cb[0xdf];
			A = SET(cpu, 3, A);
		}
		break;

	case 0xe0:	/* SET	4,B		*/
		{
			z80_cc += cc_cb[0xe0];
			B = SET(cpu, 4, B);
		}
		break;

	case 0xe1:	/* SET	4,C		*/
		{
			z80_cc += cc_cb[0xe1];
			C = SET(cpu, 4, C);
		}
		break;

	case 0xe2:	/* SET	4,D		*/
		{
			z80_cc += cc_cb[0xe2];
			D = SET(cpu, 4, D);
		}
		break;

	case 0xe3:	/* SET	4,E		*/
		{
			z80_cc += cc_cb[0xe3];
			E = SET(cpu, 4, E);
		}
		break;

	case 0xe4:	/* SET	4,H		*/
		{
			z80_cc += cc_cb[0xe4];
			H = SET(cpu, 4, H);
		}
		break;

	case 0xe5:	/* SET	4,L		*/
		{
			z80_cc += cc_cb[0xe5];
			L = SET(cpu, 4, L);
		}
		break;

	case 0xe6:	/* SET	4,(HL)		*/
		{
			z80_cc += cc_cb[0xe6];
			WR_MEM(dHL, SET(cpu, 4, RD_MEM(dHL)));
		}
		break;

	case 0xe7:	/* SET	4,A		*/
		{
			z80_cc += cc_cb[0xe7];
			A = SET(cpu, 4, A);
		}
		break;

	case 0xe8:	/* SET	5,B		*/
		{
			z80_cc += cc_cb[0xe8];
			B = SET(cpu, 5, B);
		}
		break;

	case 0xe9:	/* SET	5,C		*/
		{
			z80_cc += cc_cb[0xe9];
			C = SET(cpu, 5, C);
		}
		break;

	case 0xea:	/* SET	5,D		*/
		{
			z80_cc += cc_cb[0xea];
			D = SET(cpu, 5, D);
		}
		break;

	case 0xeb:	/* SET	5,E		*/
		{
			z80_cc += cc_cb[0xeb];
			E = SET(cpu, 5, E);
		}
		break;

	case 0xec:	/* SET	5,H		*/
		{
			z80_cc += cc_cb[0xec];
			H = SET(cpu, 5, H);
		}
		break;

	case 0xed:	/* SET	5,L		*/
		{
			z80_cc += cc_cb[0xed];
			L = SET(cpu, 5, L);
		}
		break;

	case 0xee:	/* SET	5,(HL)		*/
		{
			z80_cc += cc_cb[0xee];
			WR_MEM(dHL, SET(cpu, 5, RD_MEM(dHL)));
		}
		break;

	case 0xef:	/* SET	5,A		*/
		{
			z80_cc += cc_cb[0xef];
			A = SET(cpu, 5, A);
		}
		break;

	case 0xf0:	/* SET	6,B		*/
		{
			z80_cc += cc_cb[0xf0];
			B = SET(cpu, 6, B);
		}
		break;

	case 0xf1:	/* SET	6,C		*/
		{
			z80_cc += cc_cb[0xf1];
			C = SET(cpu, 6, C);
		}
		break;

	case 0xf2:	/* SET	6,D		*/
		{
			z80_cc += cc_cb[0xf2];
			D = SET(cpu, 6, D);
		}
		break;

	case 0xf3:	/* SET	6,E		*/
		{
			z80_cc += cc_cb[0xf3];
			E = SET(cpu, 6, E);
		}
		break;

	case 0xf4:	/* SET	6,H		*/
		{
			z80_cc += cc_cb[0xf4];
			H = SET(cpu, 6, H);
		}
		break;

	case 0xf5:	/* SET	6,L		*/
		{
			z80_cc += cc_cb[0xf5];
			L = SET(cpu, 6, L);
		}
		break;

	case 0xf6:	/* SET	6,(HL)		*/
		{
			z80_cc += cc_cb[0xf6];
			WR_MEM(dHL, SET(cpu, 6, RD_MEM(dHL)));
		}
		break;

	case 0xf7:	/* SET	6,A		*/
		{
			z80_cc += cc_cb[0xf7];
			A = SET(cpu, 6, A);
		}
		break;

	case 0xf8:	/* SET	7,B		*/
		{
			z80_cc += cc_cb[0xf8];
			B = SET(cpu, 7, B);
		}
		break;

	case 0xf9:	/* SET	7,C		*/
		{
			z80_cc += cc_cb[0xf9];
			C = SET(cpu, 7, C);
		}
		break;

	case 0xfa:	/* SET	7,D		*/
		{
			z80_cc += cc_cb[0xfa];
			D = SET(cpu, 7, D);
		}
		break;

	case 0xfb:	/* SET	7,E		*/
		{
			z80_cc += cc_cb[0xfb];
			E = SET(cpu, 7, E);
		}
		break;

	case 0xfc:	/* SET	7,H		*/
		{
			z80_cc += cc_cb[0xfc];
			H = SET(cpu, 7, H);
		}
		break;

	case 0xfd:	/* SET	7,L		*/
		{
			z80_cc += cc_cb[0xfd];
			L = SET(cpu, 7, L);
		}
		break;

	case 0xfe:	/* SET	7,(HL)		*/
		{
			z80_cc += cc_cb[0xfe];
			WR_MEM(dHL, SET(cpu, 7, RD_MEM(dHL)));
		}
		break;

	case 0xff:	/* SET	7,A		*/
		{
			z80_cc += cc_cb[0xff];
			A = SET(cpu, 7, A);
		}
		break;
	}
	if (z80_cc < cycles)
		goto fetch_xx;
	return z80_cc;

fetch_ed_xx:
	op = RD_OP(cpu);
	switch (op) {
	case 0x40:	/* IN	B,(C)		*/
		{
			z80_cc += cc_ed[0x40];
			B = RD_IO(dBC);
			F = (F & CF) | flags_szp[B];
		}
		break;

	case 0x48:	/* IN	C,(C)		*/
		{
			z80_cc += cc_ed[0x48];
			C = RD_IO(dBC);
			F = (F & CF) | flags_szp[C];
		}
		break;

	case 0x50:	/* IN	D,(C)	*/
		{
			z80_cc += cc_ed[0x50];
			D = RD_IO(dBC);
			F = (F & CF) | flags_szp[D];
		}
		break;

	case 0x58:	/* IN	E,(C)		*/
		{
			z80_cc += cc_ed[0x58];
			E = RD_IO(dBC);
			F = (F & CF) | flags_szp[E];
		}
		break;

	case 0x60:	/* IN	H,(C)		*/
		{
			z80_cc += cc_ed[0x60];
			H = RD_IO(dBC);
			F = (F & CF) | flags_szp[H];
		}
		break;

	case 0x68:	/* IN	L,(C)		*/
		{
			z80_cc += cc_ed[0x68];
			L = RD_IO(dBC);
			F = (F & CF) | flags_szp[L];
		}
		break;

	case 0x70:	/* IN	0,(C)		*/
		{
			uint8_t res = RD_IO(dBC);
			z80_cc += cc_ed[0x70];
			F = (F & CF) | flags_szp[res];
		}
		break;

	case 0x78:	/* IN	A,(C)		*/
		{
			z80_cc += cc_ed[0x78];
			A = RD_IO(dBC);
			MP = BC + 1;
			F = (F & CF) | flags_szp[A];
		}
		break;

	case 0x41:	/* OUT	(C),B		*/
		{
			z80_cc += cc_ed[0x41];
			WR_IO(dBC, B);
			MP = BC + 1;
		}
		break;

	case 0x49:	/* OUT	(C),C		*/
		{
			z80_cc += cc_ed[0x49];
			WR_IO(dBC, C);
		}
		break;

	case 0x51:	/* OUT	(C),D		*/
		{
			z80_cc += cc_ed[0x51];
			WR_IO(dBC, D);
		}
		break;

	case 0x59:	/* OUT	(C),E		*/
		{
			z80_cc += cc_ed[0x59];
			WR_IO(dBC, E);
		}
		break;

	case 0x61:	/* OUT	(C),H		*/
		{
			z80_cc += cc_ed[0x61];
			WR_IO(dBC, H);
		}
		break;

	case 0x69:	/* OUT	(C),L		*/
		{
			z80_cc += cc_ed[0x69];
			WR_IO(dBC, L);
		}
		break;

	case 0x71:	/* OUT	(C),0		*/
		{
			z80_cc += cc_ed[0x71];
			WR_IO(dBC, 0);
		}
		break;

	case 0x79:	/* OUT	(C),A		*/
		{
			z80_cc += cc_ed[0x79];
			WR_IO(dBC, A);
		}
		break;

	case 0x42:	/* SBC	HL,BC		*/
		{
			z80_cc += cc_ed[0x42];
			HL = SBC16(cpu, dHL, dBC);
		}
		break;

	case 0x4a:	/* ADC	HL,BC		*/
		{
			z80_cc += cc_ed[0x4a];
			HL = ADC16(cpu, dHL, dBC);
		}
		break;

	case 0x52:	/* SBC	HL,DE	*/
		{
			z80_cc += cc_ed[0x52];
			HL = SBC16(cpu, dHL, dDE);
		}
		break;

	case 0x5a:	/* ADC	HL,DE		*/
		{
			z80_cc += cc_ed[0x5a];
			HL = ADC16(cpu, dHL, dDE);
		}
		break;

	case 0x62:	/* SBC	HL,HL		*/
		{
			z80_cc += cc_ed[0x62];
			HL = SBC16(cpu, dHL, dHL);
		}
		break;

	case 0x6a:	/* ADC	HL,HL		*/
		{
			z80_cc += cc_ed[0x6a];
			HL = ADC16(cpu, dHL, dHL);
		}
		break;

	case 0x72:	/* SBC	HL,SP		*/
		{
			z80_cc += cc_ed[0x72];
			HL = SBC16(cpu, dHL, dSP);
		}
		break;

	case 0x7a:	/* ADC	HL,SP		*/
		{
			z80_cc += cc_ed[0x7a];
			HL = ADC16(cpu, dHL, dSP);
		}
		break;

	case 0x43:	/* LD	(nnnn),BC	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_ed[0x43];
			WR_MEM(dMP, C);
			MP++;
			WR_MEM(dMP, B);
		}
		break;

	case 0x4b:	/* LD	BC,(nnnn)	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_ed[0x4b];
			C = RD_MEM(dMP);
			MP++;
			B = RD_MEM(dMP);
		}
		break;

	case 0x53:	/* LD	(nnnn),DE	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_ed[0x53];
			WR_MEM(dMP, E);
			MP++;
			WR_MEM(dMP, D);
		}
		break;

	case 0x5b:	/* LD	DE,(nnnn)	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_ed[0x5b];
			E = RD_MEM(dMP);
			MP++;
			D = RD_MEM(dMP);
		}
		break;

	case 0x63:	/* LD	(nnnn),HL	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_ed[0x63];
			WR_MEM(dMP, L);
			MP++;
			WR_MEM(dMP, H);
		}
		break;

	case 0x6b:	/* LD	HL,(nnnn)	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_ed[0x6b];
			L = RD_MEM(dMP);
			MP++;
			H = RD_MEM(dMP);
		}
		break;

	case 0x73:	/* LD	(nnnn),SP	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_ed[0x73];
			WR_MEM(dMP, SPL);
			MP++;
			WR_MEM(dMP, SPH);
		}
		break;

	case 0x7b:	/* LD	SP,(nnnn)	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_ed[0x7b];
			SPL = RD_MEM(dMP);
			MP++;
			SPH = RD_MEM(dMP);
		}
		break;

	case 0x44:	/* NEG			*/
	case 0x4c:	/* NEG(*)		*/
	case 0x54:	/* NEG(*)		*/
	case 0x5c:	/* NEG(*)		*/
	case 0x64:	/* NEG(*)		*/
	case 0x6c:	/* NEG(*)		*/
	case 0x74:	/* NEG(*)		*/
	case 0x7c:	/* NEG(*)		*/
		{
			uint32_t val = A;
			uint32_t res = 0 - val;
			z80_cc += cc_ed[0x44];
			F = flags_sz[res & 0xff] |
				((res >> 8) & CF) | NF |
				((res ^ val) & HF) |
				((val & res & 0x80) >> 5);
			A = (uint8_t)res;
		}
		break;

	case 0x45:	/* RETN			*/
	case 0x55:	/* RETN(*)		*/
	case 0x65:	/* RETN(*)		*/
	case 0x75:	/* RETN(*)		*/
		{
			z80_cc += cc_ed[0x45];
			POP(cpu, REG_PC);
			change_pc(dPC);
			cpu->iff |= cpu->iff >> 1;
		}
		break;

	case 0x4d:	/* RETI			*/
	case 0x5d:	/* RETI(*)		*/
	case 0x6d:	/* RETI(*)		*/
	case 0x7d:	/* RETI(*)		*/
		{
			z80_cc += cc_ed[0x4d];
			POP(cpu, REG_PC);
			change_pc(dPC);
			cpu->iff |= cpu->iff >> 1;
			/* FIXME: daisy chain controller callback */
		}
		break;

	case 0x46:	/* IM	0		*/
	case 0x4e:	/* IM	0(*)		*/
	case 0x66:	/* IM	0(*)		*/
	case 0x6e:	/* IM	0(*)		*/
		{
			z80_cc += cc_ed[0x46];
			cpu->im = 0;
		}
		break;

	case 0x56:	/* IM	1		*/
	case 0x76:	/* IM	1(*)		*/
		{
			z80_cc += cc_ed[0x56];
			cpu->im = 1;
		}
		break;

	case 0x5e:	/* IM	2		*/
	case 0x7e:	/* IM	2(*)		*/
		{
			z80_cc += cc_ed[0x5e];
			cpu->im = 2;
		}
		break;

	case 0x47:	/* LD	I,A		*/
		{
			z80_cc += cc_ed[0x47];
			cpu->iv = A;
		}
		break;

	case 0x4f:	/* LD	R,A		*/
		{
			z80_cc += cc_ed[0x4f];
			cpu->r = A;
			cpu->r7 = A & 0x80;
		}
		break;

	case 0x57:	/* LD	A,I		*/
		{
			z80_cc += cc_ed[0x57];
			A = cpu->iv;
			F = (F & CF) | flags_sz[A] | ((cpu->iff & 2) ? PF : 0);
		}
		break;

	case 0x5f:	/* LD	A,R		*/
		{
			z80_cc += cc_ed[0x5f];
			A = (cpu->r & 0x7f) | cpu->r7;
			F = (F & CF) | flags_sz[A] | ((cpu->iff & 2) ? PF : 0);
		}
		break;

	case 0x67:	/* RRD	(HL)		*/
		{
			uint8_t n = RD_MEM(dHL);
			z80_cc += cc_ed[0x67];
			MP = HL + 1;
			WR_MEM(dHL, (n >> 4) | (A << 4));
			A = (A & 0xf0) | (n & 0x0f);
			F = (F & CF) | flags_szp[A];
		}
		break;

	case 0x6f:	/* RLD	(HL)		*/
		{
			uint8_t n = RD_MEM(dHL);
			z80_cc += cc_ed[0x6f];
			MP = HL + 1;
			WR_MEM(dHL, (n << 4) | (A & 0x0f));
			A = (A & 0xf0) | (n >> 4);
			F = (F & CF) | flags_szp[A];
		}
		break;

	case 0xa0:	/* LDI			*/
		{
			z80_cc += cc_ed[0xa0];
			LDI(cpu);
		}
		break;

	case 0xa1:	/* CPI			*/
		{
			z80_cc += cc_ed[0xa1];
			CPI(cpu);
		}
		break;

	case 0xa2:	/* INI			*/
		{
			z80_cc += cc_ed[0xa2];
			INI(cpu);
		}
		break;

	case 0xa3:	/* OUTI			*/
		{
			z80_cc += cc_ed[0xa3];
			OUTI(cpu);
		}
		break;

	case 0xa8:	/* LDD			*/
		{
			z80_cc += cc_ed[0xa8];
			LDD(cpu);
		}
		break;

	case 0xa9:	/* CPD			*/
		{
			z80_cc += cc_ed[0xa9];
			CPD(cpu);
		}
		break;

	case 0xaa:	/* IND			*/
		{
			z80_cc += cc_ed[0xaa];
			IND(cpu);
		}
		break;

	case 0xab:	/* OUTD			*/
		{
			z80_cc += cc_ed[0xab];
			OUTD(cpu);
		}
		break;

	case 0xb0:	/* LDIR			*/
		{
			z80_cc += cc_ed[0xb0];
			LDI(cpu);
			if (0 != BC) {
				z80_cc += cc_ex[0xb0];
				MP = PC - 1;
				PC -= 2;
			}
		}
		break;

	case 0xb1:	/* CPIR			*/
		{
			z80_cc += cc_ed[0xb1];
			CPI(cpu);
			if (0 != BC && 0 == (F & ZF)) {
				z80_cc += cc_ex[0xb1];
				MP = PC - 1;
				PC -= 2;
			}
		}
		break;

	case 0xb2:	/* INIR			*/
		{
			z80_cc += cc_ed[0xb2];
			INI(cpu);
			if (0 != B) {
				z80_cc += cc_ex[0xb2];
				PC -= 2;
			}
		}
		break;

	case 0xb3:	/* OTIR			*/
		{
			z80_cc += cc_ed[0xb3];
			OUTI(cpu);
			if (0 != B) {
				z80_cc += cc_ex[0xb3];
				PC -= 2;
			}
		}
		break;

	case 0xb8:	/* LDDR			*/
		{
			z80_cc += cc_ed[0xb8];
			LDD(cpu);
			if (0 != BC) {
				z80_cc += cc_ex[0xb8];
				MP = PC - 1;
				PC -= 2;
			}
		}
		break;

	case 0xb9:	/* CPDR			*/
		{
			z80_cc += cc_ed[0xb9];
			CPD(cpu);
			if (0 != BC && 0 == (F & ZF)) {
				z80_cc += cc_ex[0xb9];
				MP = PC - 1;
				PC -= 2;
			}
		}
		break;

	case 0xba:	/* INDR			*/
		{
			z80_cc += cc_ed[0xba];
			IND(cpu);
			if (0 != B) {
				z80_cc += cc_ex[0xba];
				PC -= 2;
			}
		}
		break;

	case 0xbb:	/* OTDR			*/
		{
			z80_cc += cc_ed[0xbb];
			OUTD(cpu);
			if (0 != B) {
				z80_cc += cc_ex[0xbb];
				PC -= 2;
			}
		}
		break;

	default:
		/* illegal ED xx opcode */
		z80_cc += cc_ed[op];
	}
	if (z80_cc < cycles)
		goto fetch_xx;
	return z80_cc;

fetch_dd_xx:
	op = RD_ARGB(cpu);
	switch (op) {

	case 0x09:	/* ADD	IX,BC		*/
		{
			z80_cc += cc_xy[0x09];
			IX = ADD16(cpu, dIX, dBC);
		}
		break;


	case 0x19:	/* ADD	IX,DE		*/
		{
			z80_cc += cc_xy[0x19];
			IX = ADD16(cpu, dIX, dDE);
		}
		break;

	case 0x21:	/* LD	IX,nnnn		*/
		{
			z80_cc += cc_xy[0x21];
			IX = RD_ARGW(cpu);
		}
		break;

	case 0x22:	/* LD	(nnnn),IX	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_xy[0x22];
			WR_MEM(dMP, LX);
			MP++;
			WR_MEM(dMP, HX);
		}
		break;

	case 0x23:	/* INC	IX		*/
		{
			z80_cc += cc_xy[0x23];
			IX++;
		}
		break;

	case 0x24:	/* INC	HX		*/
		{
			z80_cc += cc_xy[0x24];
			HX = INC(cpu, HX);
		}
		break;

	case 0x25:	/* DEC	HX		*/
		{
			z80_cc += cc_xy[0x25];
			HX = DEC(cpu, HX);
		}
		break;

	case 0x26:	/* LD	HX,nn		*/
		{
			z80_cc += cc_xy[0x26];
			HX = RD_ARGB(cpu);
		}
		break;

	case 0x29:	/* ADD	IX,IX		*/
		{
			z80_cc += cc_xy[0x29];
			IX = ADD16(cpu, dIX, dIX);
		}
		break;

	case 0x2a:	/* LD	IX,(nnnn)	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_xy[0x2a];
			LX = RD_MEM(dMP);
			MP++;
			HX = RD_MEM(dMP);
		}
		break;

	case 0x2b:	/* DEC	IX		*/
		{
			z80_cc += cc_xy[0x2b];
			IX--;
		}
		break;

	case 0x2c:	/* INC	LX		*/
		{
			z80_cc += cc_xy[0x2c];
			LX = INC(cpu, LX);
		}
		break;

	case 0x2d:	/* DEC	LX		*/
		{
			z80_cc += cc_xy[0x2d];
			LX = DEC(cpu, LX);
		}
		break;

	case 0x2e:	/* LD	LX,nn		*/
		{
			z80_cc += cc_xy[0x2e];
			LX = RD_ARGB(cpu);
		}
		break;

	case 0x34:	/* INC	(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x34];
			WR_MEM(dEA, INC(cpu, RD_MEM(dEA)));
		}
		break;

	case 0x35:	/* DEC	(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x35];
			WR_MEM(dEA, DEC(cpu, RD_MEM(dEA)));
		}
		break;

	case 0x36:	/* LD	(IX+rel),nn	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x36];
			WR_MEM(dEA, RD_ARGB(cpu));
		}
		break;

	case 0x39:	/* ADD	IX,SP		*/
		{
			z80_cc += cc_xy[0x39];
			IX = ADD16(cpu, dIX, dSP);
		}
		break;

	case 0x44:	/* LD	B,HX		*/
		{
			z80_cc += cc_xy[0x44];
			B = HX;
		}
		break;

	case 0x45:	/* LD	B,LX		*/
		{
			z80_cc += cc_xy[0x45];
			B = LX;
		}
		break;

	case 0x46:	/* LD	B,(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x46];
			B = RD_MEM(dEA);
		}
		break;

	case 0x4c:	/* LD	C,HX		*/
		{
			z80_cc += cc_xy[0x4c];
			C = HX;
		}
		break;

	case 0x4d:	/* LD	C,LX		*/
		{
			z80_cc += cc_xy[0x4d];
			C = LX;
		}
		break;

	case 0x4e:	/* LD	C,(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x4e];
			C = RD_MEM(dEA);
		}
		break;

	case 0x54:	/* LD	D,HX		*/
		{
			z80_cc += cc_xy[0x54];
			D = HX;
		}
		break;

	case 0x55:	/* LD	D,LX		*/
		{
			z80_cc += cc_xy[0x55];
			D = LX;
		}
		break;

	case 0x56:	/* LD	D,(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x56];
			D = RD_MEM(dEA);
		}
		break;

	case 0x5c:	/* LD	E,HX		*/
		{
			z80_cc += cc_xy[0x5c];
			E = HX;
		}
		break;

	case 0x5d:	/* LD	E,LX		*/
		{
			z80_cc += cc_xy[0x5d];
			E = LX;
		}
		break;

	case 0x5e:	/* LD	E,(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x5e];
			E = RD_MEM(dEA);
		}
		break;

	case 0x60:	/* LD	HX,B		*/
		{
			z80_cc += cc_xy[0x60];
			HX = B;
		}
		break;

	case 0x61:	/* LD	HX,C		*/
		{
			z80_cc += cc_xy[0x61];
			HX = C;
		}
		break;

	case 0x62:	/* LD	HX,D		*/
		{
			z80_cc += cc_xy[0x62];
			HX = D;
		}
		break;

	case 0x63:	/* LD	HX,E		*/
		{
			z80_cc += cc_xy[0x63];
			HX = E;
		}
		break;

	case 0x64:	/* LD	HX,HX		*/
		{
			z80_cc += cc_xy[0x64];
			HX = HX;
		}
		break;

	case 0x65:	/* LD	HX,LX		*/
		{
			z80_cc += cc_xy[0x65];
			HX = LX;
		}
		break;

	case 0x66:	/* LD	H,(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x66];
			H = RD_MEM(dEA);
		}
		break;

	case 0x67:	/* LD	HX,A		*/
		{
			z80_cc += cc_xy[0x67];
			HX = A;
		}
		break;

	case 0x68:	/* LD	LX,B		*/
		{
			z80_cc += cc_xy[0x68];
			LX = B;
		}
		break;

	case 0x69:	/* LD	LX,C		*/
		{
			z80_cc += cc_xy[0x69];
			LX = C;
		}
		break;

	case 0x6a:	/* LD	LX,D		*/
		{
			z80_cc += cc_xy[0x6a];
			LX = D;
		}
		break;

	case 0x6b:	/* LD	LX,E		*/
		{
			z80_cc += cc_xy[0x6b];
			LX = E;
		}
		break;

	case 0x6c:	/* LD	LX,HX		*/
		{
			z80_cc += cc_xy[0x6c];
			LX = HX;
		}
		break;

	case 0x6d:	/* LD	LX,LX		*/
		{
			z80_cc += cc_xy[0x6d];
			LX = LX;
		}
		break;

	case 0x6e:	/* LD	L,(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x6e];
			L = RD_MEM(dEA);
		}
		break;

	case 0x6f:	/* LD	LX,A		*/
		{
			z80_cc += cc_xy[0x6f];
			LX = A;
		}
		break;

	case 0x70:	/* LD	(IX+rel),B	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x70];
			WR_MEM(dEA, B);
		}
		break;

	case 0x71:	/* LD	(IX+rel),C	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x71];
			WR_MEM(dEA, C);
		}
		break;

	case 0x72:	/* LD	(IX+rel),D	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x72];
			WR_MEM(dEA, D);
		}
		break;

	case 0x73:	/* LD	(IX+rel),E	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x73];
			WR_MEM(dEA, E);
		}
		break;

	case 0x74:	/* LD	(IX+rel),H	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x74];
			WR_MEM(dEA, H);
		}
		break;

	case 0x75:	/* LD	(IX+rel),L	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x75];
			WR_MEM(dEA, L);
		}
		break;

	case 0x77:	/* LD	(IX+rel),A	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x77];
			WR_MEM(dEA, A);
		}
		break;

	case 0x7c:	/* LD	A,HX		*/
		{
			z80_cc += cc_xy[0x7c];
			A = HX;
		}
		break;

	case 0x7d:	/* LD	A,LX		*/
		{
			z80_cc += cc_xy[0x7d];
			A = LX;
		}
		break;

	case 0x7e:	/* LD	A,(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x7e];
			A = RD_MEM(dEA);
		}
		break;

	case 0x84:	/* ADD	A,HX		*/
		{
			z80_cc += cc_xy[0x84];
			A = ADD(cpu, HX);
		}
		break;

	case 0x85:	/* ADD	A,LX		*/
		{
			z80_cc += cc_xy[0x85];
			A = ADD(cpu, LX);
		}
		break;

	case 0x86:	/* ADD	A,(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x86];
			A = ADD(cpu, RD_MEM(dEA));
		}
		break;

	case 0x8c:	/* ADC	A,HX		*/
		{
			z80_cc += cc_xy[0x8c];
			A = ADC(cpu, HX);
		}
		break;

	case 0x8d:	/* ADC	A,LX		*/
		{
			z80_cc += cc_xy[0x8d];
			A = ADC(cpu, LX);
		}
		break;

	case 0x8e:	/* ADC	A,(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x8e];
			A = ADC(cpu, RD_MEM(dEA));
		}
		break;

	case 0x94:	/* SUB	HX		*/
		{
			z80_cc += cc_xy[0x94];
			A = SUB(cpu, HX);
		}
		break;

	case 0x95:	/* SUB	LX		*/
		{
			z80_cc += cc_xy[0x95];
			A = SUB(cpu, LX);
		}
		break;

	case 0x96:	/* SUB	(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x96];
			A = SUB(cpu, RD_MEM(dEA));
		}
		break;

	case 0x9c:	/* SBC	A,HX		*/
		{
			z80_cc += cc_xy[0x9c];
			A = SBC(cpu, HX);
		}
		break;

	case 0x9d:	/* SBC	A,LX		*/
		{
			z80_cc += cc_xy[0x9d];
			A = SBC(cpu, LX);
		}
		break;

	case 0x9e:	/* SBC	A,(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0x9e];
			A = SBC(cpu, RD_MEM(dEA));
		}
		break;

	case 0xa4:	/* AND	HX		*/
		{
			z80_cc += cc_xy[0xa4];
			A = AND(cpu, HX);
		}
		break;

	case 0xa5:	/* AND	LX		*/
		{
			z80_cc += cc_xy[0xa5];
			A = AND(cpu, LX);
		}
		break;

	case 0xa6:	/* AND	(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0xa6];
			A = AND(cpu, RD_MEM(dEA));
		}
		break;

	case 0xac:	/* XOR	HX		*/
		{
			z80_cc += cc_xy[0xac];
			A = XOR(cpu, HX);
		}
		break;

	case 0xad:	/* XOR	LX		*/
		{
			z80_cc += cc_xy[0xad];
			A = XOR(cpu, LX);
		}
		break;

	case 0xae:	/* XOR	(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0xae];
			A = XOR(cpu, RD_MEM(dEA));
		}
		break;

	case 0xb4:	/* OR	HX		*/
		{
			z80_cc += cc_xy[0xb4];
			A = OR(cpu, HX);
		}
		break;

	case 0xb5:	/* OR	LX		*/
		{
			z80_cc += cc_xy[0xb5];
			A = OR(cpu, LX);
		}
		break;

	case 0xb6:	/* OR	(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0xb6];
			A = OR(cpu, RD_MEM(dEA));
		}
		break;

	case 0xbc:	/* CP	HX		*/
		{
			z80_cc += cc_xy[0xbc];
			CP(cpu, HX);
		}
		break;

	case 0xbd:	/* CP	LX		*/
		{
			z80_cc += cc_xy[0xbd];
			CP(cpu, LX);
		}
		break;

	case 0xbe:	/* CP	(IX+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IX + rel;
			z80_cc += cc_xy[0xbe];
			CP(cpu, RD_MEM(dEA));
		}
		break;

	case 0xcb:	/* prefix DD CB xx 	*/
		{
			int8_t rel = RD_ARGB(cpu);
			z80_cc += cc_xy[0xcb];
			MP = EA = IX + rel;
			m = RD_MEM(dEA);	
			goto fetch_xy_cb_xx;
		}
		break;

	case 0xdd:	/* prefix DD xx (IX)	*/
		{
			z80_cc += cc_xy[0xdd];
			if (z80_cc < cycles) {
				goto fetch_dd_xx;
			}
			PC--;
		}
		break;

	case 0xe1:	/* POP	IX		*/
		{
			z80_cc += cc_xy[0xe1];
			POP(cpu, REG_IX);
		}
		break;

	case 0xe3:	/* EX	(SP),IX		*/
		{
			z80_cc += cc_xy[0xe3];
			MPL = RD_MEM(dSP);
			WR_MEM(dSP, LX);
			SP++;
			LX = MPL;
			MPH = RD_MEM(dSP);
			WR_MEM(dSP, HX);
			SP--;
			HX = MPH;
		}
		break;

	case 0xe5:	/* PUSH	IX		*/
		{
			z80_cc += cc_xy[0xe5];
			PUSH(cpu, REG_IX);
		}
		break;

	case 0xe9:	/* JP	(IX)		*/
		{
			z80_cc += cc_xy[0xe9];
			PC = IX;
			change_pc(dPC);
		}
		break;

	case 0xed:	/* prefix ED xx		*/
		{
			z80_cc += cc_xy[0xed];
			goto fetch_ed_xx;
		}
		break;

	case 0xf9:	/* LD	SP,IX */
		{
			z80_cc += cc_xy[0xf9];
			SP = IX;
		}
		break;

	case 0xfd:	/* prefix FD xx (IY)	*/
		{
			z80_cc += cc_xy[0xfd];
			if (z80_cc < cycles) {
				goto fetch_fd_xx;
			}
			PC--;
		}
		break;

	default:	/* ignore DD prefix */
		z80_cc += cc_xy[op];
		goto decode_xx;
	}
	if (z80_cc < cycles)
		goto fetch_xx;
	return z80_cc;


fetch_fd_xx:
	op = RD_ARGB(cpu);
	switch (op) {

	case 0x09:	/* ADD	IY,BC		*/
		{
			z80_cc += cc_xy[0x09];
			IY = ADD16(cpu, dIY, dBC);
		}
		break;


	case 0x19:	/* ADD	IY,DE		*/
		{
			z80_cc += cc_xy[0x19];
			IY = ADD16(cpu, dIY, dDE);
		}
		break;

	case 0x21:	/* LD	IY,nnnn		*/
		{
			z80_cc += cc_xy[0x21];
			IY = RD_ARGW(cpu);
		}
		break;

	case 0x22:	/* LD	(nnnn),IY	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_xy[0x22];
			WR_MEM(dMP, LY);
			MP++;
			WR_MEM(dMP, HY);
		}
		break;

	case 0x23:	/* INC	IY		*/
		{
			z80_cc += cc_xy[0x23];
			IY++;
		}
		break;

	case 0x24:	/* INC	HY		*/
		{
			z80_cc += cc_xy[0x24];
			HY = INC(cpu, HY);
		}
		break;

	case 0x25:	/* DEC	HY		*/
		{
			z80_cc += cc_xy[0x25];
			HY = DEC(cpu, HY);
		}
		break;

	case 0x26:	/* LD	HY,nn		*/
		{
			z80_cc += cc_xy[0x26];
			HY = RD_ARGB(cpu);
		}
		break;

	case 0x29:	/* ADD	IY,IY		*/
		{
			z80_cc += cc_xy[0x29];
			IY = ADD16(cpu, dIY, dIY);
		}
		break;

	case 0x2a:	/* LD	IY,(nnnn)	*/
		{
			MP = RD_ARGW(cpu);
			z80_cc += cc_xy[0x2a];
			LY = RD_MEM(dMP);
			MP++;
			HY = RD_MEM(dMP);
		}
		break;

	case 0x2b:	/* DEC	IY		*/
		{
			z80_cc += cc_xy[0x2b];
			IY--;
		}
		break;

	case 0x2c:	/* INC	LY		*/
		{
			z80_cc += cc_xy[0x2c];
			LY = INC(cpu, LY);
		}
		break;

	case 0x2d:	/* DEC	LY		*/
		{
			z80_cc += cc_xy[0x2d];
			LY = DEC(cpu, LY);
		}
		break;

	case 0x2e:	/* LD	LY,nn		*/
		{
			z80_cc += cc_xy[0x2e];
			LY = RD_ARGB(cpu);
		}
		break;

	case 0x34:	/* INC	(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x34];
			WR_MEM(dEA, INC(cpu, RD_MEM(dEA)));
		}
		break;

	case 0x35:	/* DEC	(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x35];
			WR_MEM(dEA, DEC(cpu, RD_MEM(dEA)));
		}
		break;

	case 0x36:	/* LD	(IY+rel),nn	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x36];
			WR_MEM(dEA, RD_ARGB(cpu));
		}
		break;

	case 0x39:	/* ADD	IY,SP		*/
		{
			z80_cc += cc_xy[0x39];
			IY = ADD16(cpu, dIY, dSP);
		}
		break;

	case 0x44:	/* LD	B,HY		*/
		{
			z80_cc += cc_xy[0x44];
			B = HY;
		}
		break;

	case 0x45:	/* LD	B,LY		*/
		{
			z80_cc += cc_xy[0x45];
			B = LY;
		}
		break;

	case 0x46:	/* LD	B,(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x46];
			B = RD_MEM(dEA);
		}
		break;

	case 0x4c:	/* LD	C,HY		*/
		{
			z80_cc += cc_xy[0x4c];
			C = HY;
		}
		break;

	case 0x4d:	/* LD	C,LY		*/
		{
			z80_cc += cc_xy[0x4d];
			C = LY;
		}
		break;

	case 0x4e:	/* LD	C,(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x4e];
			C = RD_MEM(dEA);
		}
		break;

	case 0x54:	/* LD	D,HY		*/
		{
			z80_cc += cc_xy[0x54];
			D = HY;
		}
		break;

	case 0x55:	/* LD	D,LY		*/
		{
			z80_cc += cc_xy[0x55];
			D = LY;
		}
		break;

	case 0x56:	/* LD	D,(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x56];
			D = RD_MEM(dEA);
		}
		break;

	case 0x5c:	/* LD	E,HY		*/
		{
			z80_cc += cc_xy[0x5c];
			E = HY;
		}
		break;

	case 0x5d:	/* LD	E,LY		*/
		{
			z80_cc += cc_xy[0x5d];
			E = LY;
		}
		break;

	case 0x5e:	/* LD	E,(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x5e];
			E = RD_MEM(dEA);
		}
		break;

	case 0x60:	/* LD	HY,B		*/
		{
			z80_cc += cc_xy[0x60];
			HY = B;
		}
		break;

	case 0x61:	/* LD	HY,C		*/
		{
			z80_cc += cc_xy[0x61];
			HY = C;
		}
		break;

	case 0x62:	/* LD	HY,D		*/
		{
			z80_cc += cc_xy[0x62];
			HY = D;
		}
		break;

	case 0x63:	/* LD	HY,E		*/
		{
			z80_cc += cc_xy[0x63];
			HY = E;
		}
		break;

	case 0x64:	/* LD	HY,HY		*/
		{
			z80_cc += cc_xy[0x64];
			HY = HY;
		}
		break;

	case 0x65:	/* LD	HY,LY		*/
		{
			z80_cc += cc_xy[0x65];
			HY = LY;
		}
		break;

	case 0x66:	/* LD	H,(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x66];
			H = RD_MEM(dEA);
		}
		break;

	case 0x67:	/* LD	HY,A		*/
		{
			z80_cc += cc_xy[0x67];
			HY = A;
		}
		break;

	case 0x68:	/* LD	LY,B		*/
		{
			z80_cc += cc_xy[0x68];
			LY = B;
		}
		break;

	case 0x69:	/* LD	LY,C		*/
		{
			z80_cc += cc_xy[0x69];
			LY = C;
		}
		break;

	case 0x6a:	/* LD	LY,D		*/
		{
			z80_cc += cc_xy[0x6a];
			LY = D;
		}
		break;

	case 0x6b:	/* LD	LY,E		*/
		{
			z80_cc += cc_xy[0x6b];
			LY = E;
		}
		break;

	case 0x6c:	/* LD	LY,HY		*/
		{
			z80_cc += cc_xy[0x6c];
			LY = HY;
		}
		break;

	case 0x6d:	/* LD	LY,LY		*/
		{
			z80_cc += cc_xy[0x6d];
			LY = LY;
		}
		break;

	case 0x6e:	/* LD	L,(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x6e];
			L = RD_MEM(dEA);
		}
		break;

	case 0x6f:	/* LD	LY,A		*/
		{
			z80_cc += cc_xy[0x6f];
			LY = A;
		}
		break;

	case 0x70:	/* LD	(IY+rel),B	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x70];
			WR_MEM(dEA, B);
		}
		break;

	case 0x71:	/* LD	(IY+rel),C	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x71];
			WR_MEM(dEA, C);
		}
		break;

	case 0x72:	/* LD	(IY+rel),D	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x72];
			WR_MEM(dEA, D);
		}
		break;

	case 0x73:	/* LD	(IY+rel),E	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x73];
			WR_MEM(dEA, E);
		}
		break;

	case 0x74:	/* LD	(IY+rel),H	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x74];
			WR_MEM(dEA, H);
		}
		break;

	case 0x75:	/* LD	(IY+rel),L	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x75];
			WR_MEM(dEA, L);
		}
		break;

	case 0x77:	/* LD	(IY+rel),A	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x77];
			WR_MEM(dEA, A);
		}
		break;

	case 0x7c:	/* LD	A,HY		*/
		{
			z80_cc += cc_xy[0x7c];
			A = HY;
		}
		break;

	case 0x7d:	/* LD	A,LY		*/
		{
			z80_cc += cc_xy[0x7d];
			A = LY;
		}
		break;

	case 0x7e:	/* LD	A,(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x7e];
			A = RD_MEM(dEA);
		}
		break;

	case 0x84:	/* ADD	A,HY		*/
		{
			z80_cc += cc_xy[0x84];
			A = ADD(cpu, HY);
		}
		break;

	case 0x85:	/* ADD	A,LY		*/
		{
			z80_cc += cc_xy[0x85];
			A = ADD(cpu, LY);
		}
		break;

	case 0x86:	/* ADD	A,(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x86];
			A = ADD(cpu, RD_MEM(dEA));
		}
		break;

	case 0x8c:	/* ADC	A,HY		*/
		{
			z80_cc += cc_xy[0x8c];
			A = ADC(cpu, HY);
		}
		break;

	case 0x8d:	/* ADC	A,LY		*/
		{
			z80_cc += cc_xy[0x8d];
			A = ADC(cpu, LY);
		}
		break;

	case 0x8e:	/* ADC	A,(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x8e];
			A = ADC(cpu, RD_MEM(dEA));
		}
		break;

	case 0x94:	/* SUB	HY		*/
		{
			z80_cc += cc_xy[0x94];
			A = SUB(cpu, HY);
		}
		break;

	case 0x95:	/* SUB	LY		*/
		{
			z80_cc += cc_xy[0x95];
			A = SUB(cpu, LY);
		}
		break;

	case 0x96:	/* SUB	(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x96];
			A = SUB(cpu, RD_MEM(dEA));
		}
		break;

	case 0x9c:	/* SBC	A,HY		*/
		{
			z80_cc += cc_xy[0x9c];
			A = SBC(cpu, HY);
		}
		break;

	case 0x9d:	/* SBC	A,LY		*/
		{
			z80_cc += cc_xy[0x9d];
			A = SBC(cpu, LY);
		}
		break;

	case 0x9e:	/* SBC	A,(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0x9e];
			A = SBC(cpu, RD_MEM(dEA));
		}
		break;

	case 0xa4:	/* AND	HY		*/
		{
			z80_cc += cc_xy[0xa4];
			A = AND(cpu, HY);
		}
		break;

	case 0xa5:	/* AND	LY		*/
		{
			z80_cc += cc_xy[0xa5];
			A = AND(cpu, LY);
		}
		break;

	case 0xa6:	/* AND	(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0xa6];
			A = AND(cpu, RD_MEM(dEA));
		}
		break;

	case 0xac:	/* XOR	HY		*/
		{
			z80_cc += cc_xy[0xac];
			A = XOR(cpu, HY);
		}
		break;

	case 0xad:	/* XOR	LY		*/
		{
			z80_cc += cc_xy[0xad];
			A = XOR(cpu, LY);
		}
		break;

	case 0xae:	/* XOR	(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0xae];
			A = XOR(cpu, RD_MEM(dEA));
		}
		break;

	case 0xb4:	/* OR	HY		*/
		{
			z80_cc += cc_xy[0xb4];
			A = OR(cpu, HY);
		}
		break;

	case 0xb5:	/* OR	LY		*/
		{
			z80_cc += cc_xy[0xb5];
			A = OR(cpu, LY);
		}
		break;

	case 0xb6:	/* OR	(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0xb6];
			A = OR(cpu, RD_MEM(dEA));
		}
		break;

	case 0xbc:	/* CP	HY		*/
		{
			z80_cc += cc_xy[0xbc];
			CP(cpu, HY);
		}
		break;

	case 0xbd:	/* CP	LY		*/
		{
			z80_cc += cc_xy[0xbd];
			CP(cpu, LY);
		}
		break;

	case 0xbe:	/* CP	(IY+rel)	*/
		{
			int8_t rel = RD_ARGB(cpu);
			MP = EA = IY + rel;
			z80_cc += cc_xy[0xbe];
			CP(cpu, RD_MEM(dEA));
		}
		break;

	case 0xcb:	/* prefix FD CB xx 	*/
		{
			int8_t rel = RD_ARGB(cpu);
			z80_cc += cc_xy[0xcb];
			MP = EA = IY + rel;
			m = RD_MEM(dEA);	
			goto fetch_xy_cb_xx;
		}
		break;

	case 0xdd:	/* prefix DD xx (IX)	*/
		{
			z80_cc += cc_xy[0xdd];
			if (z80_cc < cycles) {
				goto fetch_dd_xx;
			}
			PC--;
		}
		break;

	case 0xe1:	/* POP	IY		*/
		{
			z80_cc += cc_xy[0xe1];
			POP(cpu, REG_IY);
		}
		break;

	case 0xe3:	/* EX	(SP),IY		*/
		{
			z80_cc += cc_xy[0xe3];
			MPL = RD_MEM(dSP);
			WR_MEM(dSP, LY);
			SP++;
			LY = MPL;
			MPH = RD_MEM(dSP);
			WR_MEM(dSP, HY);
			SP--;
			HY = MPH;
		}
		break;

	case 0xe5:	/* PUSH	IY		*/
		{
			z80_cc += cc_xy[0xe5];
			PUSH(cpu, REG_IY);
		}
		break;

	case 0xe9:	/* JP	(IY)		*/
		{
			z80_cc += cc_xy[0xe9];
			PC = IY;
			change_pc(dPC);
		}
		break;

	case 0xed:	/* prefix ED xx		*/
		{
			z80_cc += cc_xy[0xed];
			goto fetch_ed_xx;
		}
		break;

	case 0xf9:	/* LD	SP,IY */
		{
			z80_cc += cc_xy[0xf9];
			SP = IX;
		}
		break;

	case 0xfd:	/* prefix FD xx (IY)	*/
		{
			z80_cc += cc_xy[0xfd];
			if (z80_cc < cycles) {
				goto fetch_fd_xx;
			}
			PC--;
		}
		break;

	default:	/* ignore FD prefix */
		z80_cc += cc_xy[op];
		goto decode_xx;
	}
	if (z80_cc < cycles)
		goto fetch_xx;
	return z80_cc;

fetch_xy_cb_xx:
	op = RD_OP(cpu);
	switch (op) {
	case 0x00:	/* RLC	B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x00];
			B = m = RLC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x01:	/* RLC	C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x01];
			C = m = RLC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x02:	/* RLC	D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x02];
			D = m = RLC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x03:	/* RLC	E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x03];
			E = m = RLC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x04:	/* RLC	H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x04];
			H = m = RLC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x05:	/* RLC	L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x05];
			L = m = RLC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x06:	/* RLC	(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x06];
			m = RLC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x07:	/* RLC	A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x07];
			A = m = RLC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x08:	/* RRC	B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x08];
			B = m = RRC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x09:	/* RRC	C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x09];
			C = m = RRC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x0a:	/* RRC	D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x0a];
			D = m = RRC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x0b:	/* RRC	E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x0b];
			E = m = RRC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x0c:	/* RRC	H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x0c];
			H = m = RRC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x0d:	/* RRC	L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x0d];
			L = m = RRC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x0e:	/* RRC	(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x0e];
			m = RRC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x0f:	/* RRC	A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x0f];
			A = m = RRC(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x10:	/* RL	B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x10];
			B = m = RL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x11:	/* RL	C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x11];
			C = m = RL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x12:	/* RL	D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x12];
			D = m = RL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x13:	/* RL	E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x13];
			E = m = RL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x14:	/* RL	H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x14];
			H = m = RL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x15:	/* RL	L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x15];
			L = m = RL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x16:	/* RL	(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x16];
			m = RL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x17:	/* RL	A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x17];
			A = m = RL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x18:	/* RR	B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x18];
			B = m = RR(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x19:	/* RR	C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x19];
			C = m = RR(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x1a:	/* RR	D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x1a];
			D = m = RR(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x1b:	/* RR	E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x1b];
			E = m = RR(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x1c:	/* RR	H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x1c];
			H = m = RR(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x1d:	/* RR	L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x1d];
			L = m = RR(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x1e:	/* RR	(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x1e];
			m = RR(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x1f:	/* RR	A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x1f];
			A = m = RR(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x20:	/* SLA	B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x20];
			B = m = SLA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x21:	/* SLA	C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x21];
			C = m = SLA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x22:	/* SLA	D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x22];
			D = m = SLA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x23:	/* SLA	E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x23];
			E = m = SLA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x24:	/* SLA	H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x24];
			H = m = SLA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x25:	/* SLA	L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x25];
			L = m = SLA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x26:	/* SLA	(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x26];
			m = SLA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x27:	/* SLA	A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x27];
			A = m = SLA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x28:	/* SRA	B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x28];
			B = m = SRA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x29:	/* SRA	C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x29];
			C = m = SRA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x2a:	/* SRA	D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x2a];
			D = m = SRA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x2b:	/* SRA	E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x2b];
			E = m = SRA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x2c:	/* SRA	H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x2c];
			H = m = SRA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x2d:	/* SRA	L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x2d];
			L = m = SRA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x2e:	/* SRA	(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x2e];
			m = SRA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x2f:	/* SRA	A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x2f];
			A = m = SRA(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x30:	/* SLL	B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x30];
			B = m = SLL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x31:	/* SLL	C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x31];
			C = m = SLL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x32:	/* SLL	D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x32];
			D = m = SLL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x33:	/* SLL	E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x33];
			E = m = SLL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x34:	/* SLL	H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x34];
			H = m = SLL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x35:	/* SLL	L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x35];
			L = m = SLL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x36:	/* SLL	(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x36];
			m = SLL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x37:	/* SLL	A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x37];
			A = m = SLL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x38:	/* SRL	B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x38];
			B = m = SRL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x39:	/* SRL	C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x39];
			C = m = SRL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x3a:	/* SRL	D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x3a];
			D = m = SRL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x3b:	/* SRL	E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x3b];
			E = m = SRL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x3c:	/* SRL	H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x3c];
			H = m = SRL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x3d:	/* SRL	L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x3d];
			L = m = SRL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x3e:	/* SRL	(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x3e];
			m = SRL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x3f:	/* SRL	A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x3f];
			A = m = SRL(cpu, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x40:	/* BIT	0,(IX/Y+rel)	*/
	case 0x41:	/* BIT	0,(IX/Y+rel)	*/
	case 0x42:	/* BIT	0,(IX/Y+rel)	*/
	case 0x43:	/* BIT	0,(IX/Y+rel)	*/
	case 0x44:	/* BIT	0,(IX/Y+rel)	*/
	case 0x45:	/* BIT	0,(IX/Y+rel)	*/
	case 0x46:	/* BIT	0,(IX/Y+rel)	*/
	case 0x47:	/* BIT	0,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x40];
			BIT_XY(cpu, 0, m);
		}
		break;

	case 0x48:	/* BIT	1,(IX/Y+rel)	*/
	case 0x49:	/* BIT	1,(IX/Y+rel)	*/
	case 0x4a:	/* BIT	1,(IX/Y+rel)	*/
	case 0x4b:	/* BIT	1,(IX/Y+rel)	*/
	case 0x4c:	/* BIT	1,(IX/Y+rel)	*/
	case 0x4d:	/* BIT	1,(IX/Y+rel)	*/
	case 0x4e:	/* BIT	1,(IX/Y+rel)	*/
	case 0x4f:	/* BIT	1,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x48];
			BIT_XY(cpu, 1, m);
		}
		break;

	case 0x50:	/* BIT	2,(IX/Y+rel)	*/
	case 0x51:	/* BIT	2,(IX/Y+rel)	*/
	case 0x52:	/* BIT	2,(IX/Y+rel)	*/
	case 0x53:	/* BIT	2,(IX/Y+rel)	*/
	case 0x54:	/* BIT	2,(IX/Y+rel)	*/
	case 0x55:	/* BIT	2,(IX/Y+rel)	*/
	case 0x56:	/* BIT	2,(IX/Y+rel)	*/
	case 0x57:	/* BIT	2,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x50];
			BIT_XY(cpu, 2, m);
		}
		break;

	case 0x58:	/* BIT	3,(IX/Y+rel)	*/
	case 0x59:	/* BIT	3,(IX/Y+rel)	*/
	case 0x5a:	/* BIT	3,(IX/Y+rel)	*/
	case 0x5b:	/* BIT	3,(IX/Y+rel)	*/
	case 0x5c:	/* BIT	3,(IX/Y+rel)	*/
	case 0x5d:	/* BIT	3,(IX/Y+rel)	*/
	case 0x5e:	/* BIT	3,(IX/Y+rel)	*/
	case 0x5f:	/* BIT	3,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x58];
			BIT_XY(cpu, 3, m);
		}
		break;

	case 0x60:	/* BIT	4,(IX/Y+rel)	*/
	case 0x61:	/* BIT	4,(IX/Y+rel)	*/
	case 0x62:	/* BIT	4,(IX/Y+rel)	*/
	case 0x63:	/* BIT	4,(IX/Y+rel)	*/
	case 0x64:	/* BIT	4,(IX/Y+rel)	*/
	case 0x65:	/* BIT	4,(IX/Y+rel)	*/
	case 0x66:	/* BIT	4,(IX/Y+rel)	*/
	case 0x67:	/* BIT	4,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x60];
			BIT_XY(cpu, 4, m);
		}
		break;

	case 0x68:	/* BIT	5,(IX/Y+rel)	*/
	case 0x69:	/* BIT	5,(IX/Y+rel)	*/
	case 0x6a:	/* BIT	5,(IX/Y+rel)	*/
	case 0x6b:	/* BIT	5,(IX/Y+rel)	*/
	case 0x6c:	/* BIT	5,(IX/Y+rel)	*/
	case 0x6d:	/* BIT	5,(IX/Y+rel)	*/
	case 0x6e:	/* BIT	5,(IX/Y+rel)	*/
	case 0x6f:	/* BIT	5,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x68];
			BIT_XY(cpu, 5, m);
		}
		break;

	case 0x70:	/* BIT	6,(IX/Y+rel)	*/
	case 0x71:	/* BIT	6,(IX/Y+rel)	*/
	case 0x72:	/* BIT	6,(IX/Y+rel)	*/
	case 0x73:	/* BIT	6,(IX/Y+rel)	*/
	case 0x74:	/* BIT	6,(IX/Y+rel)	*/
	case 0x75:	/* BIT	6,(IX/Y+rel)	*/
	case 0x76:	/* BIT	6,(IX/Y+rel)	*/
	case 0x77:	/* BIT	6,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x70];
			BIT_XY(cpu, 6, m);
		}
		break;

	case 0x78:	/* BIT	7,(IX/Y+rel)	*/
	case 0x79:	/* BIT	7,(IX/Y+rel)	*/
	case 0x7a:	/* BIT	7,(IX/Y+rel)	*/
	case 0x7b:	/* BIT	7,(IX/Y+rel)	*/
	case 0x7c:	/* BIT	7,(IX/Y+rel)	*/
	case 0x7d:	/* BIT	7,(IX/Y+rel)	*/
	case 0x7e:	/* BIT	7,(IX/Y+rel)	*/
	case 0x7f:	/* BIT	7,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x78];
			BIT_XY(cpu, 7, m);
		}
		break;

	case 0x80:	/* RES	0,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x80];
			B = m = RES(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x81:	/* RES	0,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x81];
			C = m = RES(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x82:	/* RES	0,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x82];
			D = m = RES(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x83:	/* RES	0,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x83];
			E = m = RES(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x84:	/* RES	0,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x84];
			H = m = RES(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x85:	/* RES	0,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x85];
			L = m = RES(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x86:	/* RES	0,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x86];
			m = RES(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x87:	/* RES	0,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x87];
			A = m = RES(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x88:	/* RES	1,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x88];
			B = m = RES(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x89:	/* RES	1,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x89];
			C = m = RES(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x8a:	/* RES	1,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x8a];
			D = m = RES(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x8b:	/* RES	1,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x8b];
			E = m = RES(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x8c:	/* RES	1,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x8c];
			H = m = RES(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x8d:	/* RES	1,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x8d];
			L = m = RES(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x8e:	/* RES	1,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x8e];
			m = RES(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x8f:	/* RES	1,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x8f];
			A = m = RES(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x90:	/* RES	2,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x90];
			B = m = RES(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x91:	/* RES	2,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x91];
			C = m = RES(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x92:	/* RES	2,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x92];
			D = m = RES(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x93:	/* RES	2,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x93];
			E = m = RES(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x94:	/* RES	2,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x94];
			H = m = RES(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x95:	/* RES	2,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x95];
			L = m = RES(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x96:	/* RES	2,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x96];
			m = RES(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x97:	/* RES	2,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x97];
			A = m = RES(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x98:	/* RES	3,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x98];
			B = m = RES(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x99:	/* RES	3,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x99];
			C = m = RES(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x9a:	/* RES	3,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x9a];
			D = m = RES(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x9b:	/* RES	3,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x9b];
			E = m = RES(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x9c:	/* RES	3,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x9c];
			H = m = RES(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x9d:	/* RES	3,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x9d];
			L = m = RES(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x9e:	/* RES	3,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x9e];
			m = RES(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0x9f:	/* RES	3,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0x9f];
			A = m = RES(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xa0:	/* RES	4,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xa0];
			B = m = RES(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xa1:	/* RES	4,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xa1];
			C = m = RES(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xa2:	/* RES	4,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xa2];
			D = m = RES(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xa3:	/* RES	4,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xa3];
			E = m = RES(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xa4:	/* RES	4,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xa4];
			H = m = RES(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xa5:	/* RES	4,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xa5];
			L = m = RES(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xa6:	/* RES	4,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xa6];
			m = RES(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xa7:	/* RES	4,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xa7];
			A = m = RES(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xa8:	/* RES	5,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xa8];
			B = m = RES(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xa9:	/* RES	5,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xa9];
			C = m = RES(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xaa:	/* RES	5,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xaa];
			D = m = RES(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xab:	/* RES	5,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xab];
			E = m = RES(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xac:	/* RES	5,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xac];
			H = m = RES(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xad:	/* RES	5,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xad];
			L = m = RES(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xae:	/* RES	5,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xae];
			m = RES(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xaf:	/* RES	5,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xaf];
			A = m = RES(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xb0:	/* RES	6,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xb0];
			B = m = RES(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xb1:	/* RES	6,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xb1];
			C = m = RES(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xb2:	/* RES	6,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xb2];
			D = m = RES(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xb3:	/* RES	6,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xb3];
			E = m = RES(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xb4:	/* RES	6,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xb4];
			H = m = RES(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xb5:	/* RES	6,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xb5];
			L = m = RES(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xb6:	/* RES	6,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xb6];
			m = RES(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xb7:	/* RES	6,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xb7];
			A = m = RES(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xb8:	/* RES	7,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xb8];
			B = m = RES(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xb9:	/* RES	7,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xb9];
			C = m = RES(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xba:	/* RES	7,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xba];
			D = m = RES(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xbb:	/* RES	7,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xbb];
			E = m = RES(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xbc:	/* RES	7,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xbc];
			H = m = RES(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xbd:	/* RES	7,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xbd];
			L = m = RES(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xbe:	/* RES	7,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xbe];
			m = RES(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xbf:	/* RES	7,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xbf];
			A = m = RES(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xc0:	/* SET	0,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xc0];
			B = m = SET(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xc1:	/* SET	0,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xc1];
			C = m = SET(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xc2:	/* SET	0,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xc2];
			D = m = SET(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xc3:	/* SET	0,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xc3];
			E = m = SET(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xc4:	/* SET	0,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xc4];
			H = m = SET(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xc5:	/* SET	0,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xc5];
			L = m = SET(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xc6:	/* SET	0,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xc6];
			m = SET(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xc7:	/* SET	0,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xc7];
			A = m = SET(cpu, 0, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xc8:	/* SET	1,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xc8];
			B = m = SET(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xc9:	/* SET	1,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xc9];
			C = m = SET(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xca:	/* SET	1,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xca];
			D = m = SET(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xcb:	/* SET	1,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xcb];
			E = m = SET(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xcc:	/* SET	1,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xcc];
			H = m = SET(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xcd:	/* SET	1,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xcd];
			L = m = SET(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xce:	/* SET	1,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xce];
			m = SET(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xcf:	/* SET	1,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xcf];
			A = m = SET(cpu, 1, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xd0:	/* SET	2,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xd0];
			B = m = SET(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xd1:	/* SET	2,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xd1];
			C = m = SET(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xd2:	/* SET	2,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xd2];
			D = m = SET(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xd3:	/* SET	2,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xd3];
			E = m = SET(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xd4:	/* SET	2,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xd4];
			H = m = SET(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xd5:	/* SET	2,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xd5];
			L = m = SET(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xd6:	/* SET	2,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xd6];
			m = SET(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xd7:	/* SET	2,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xd7];
			A = m = SET(cpu, 2, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xd8:	/* SET	3,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xd8];
			B = m = SET(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xd9:	/* SET	3,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xd9];
			C = m = SET(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xda:	/* SET	3,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xda];
			D = m = SET(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xdb:	/* SET	3,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xdb];
			E = m = SET(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xdc:	/* SET	3,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xdc];
			H = m = SET(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xdd:	/* SET	3,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xdd];
			L = m = SET(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xde:	/* SET	3,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xde];
			m = SET(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xdf:	/* SET	3,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xdf];
			A = m = SET(cpu, 3, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xe0:	/* SET	4,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xe0];
			B = m = SET(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xe1:	/* SET	4,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xe1];
			C = m = SET(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xe2:	/* SET	4,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xe2];
			D = m = SET(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xe3:	/* SET	4,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xe3];
			E = m = SET(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xe4:	/* SET	4,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xe4];
			H = m = SET(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xe5:	/* SET	4,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xe5];
			L = m = SET(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xe6:	/* SET	4,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xe6];
			m = SET(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xe7:	/* SET	4,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xe7];
			A = m = SET(cpu, 4, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xe8:	/* SET	5,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xe8];
			B = m = SET(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xe9:	/* SET	5,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xe9];
			C = m = SET(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xea:	/* SET	5,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xea];
			D = m = SET(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xeb:	/* SET	5,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xeb];
			E = m = SET(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xec:	/* SET	5,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xec];
			H = m = SET(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xed:	/* SET	5,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xed];
			L = m = SET(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xee:	/* SET	5,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xee];
			m = SET(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xef:	/* SET	5,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xef];
			A = m = SET(cpu, 5, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xf0:	/* SET	6,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xf0];
			B = m = SET(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xf1:	/* SET	6,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xf1];
			C = m = SET(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xf2:	/* SET	6,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xf2];
			D = m = SET(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xf3:	/* SET	6,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xf3];
			E = m = SET(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xf4:	/* SET	6,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xf4];
			H = m = SET(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xf5:	/* SET	6,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xf5];
			L = m = SET(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xf6:	/* SET	6,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xf6];
			m = SET(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xf7:	/* SET	6,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xf7];
			A = m = SET(cpu, 6, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xf8:	/* SET	7,B=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xf8];
			B = m = SET(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xf9:	/* SET	7,C=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xf9];
			C = m = SET(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xfa:	/* SET	7,D=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xfa];
			D = m = SET(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xfb:	/* SET	7,E=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xfb];
			E = m = SET(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xfc:	/* SET	7,H=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xfc];
			H = m = SET(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xfd:	/* SET	7,L=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xfd];
			L = m = SET(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xfe:	/* SET	7,(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xfe];
			m = SET(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;

	case 0xff:	/* SET	7,A=(IX/Y+rel)	*/
		{
			z80_cc += cc_xy_cb[0xff];
			A = m = SET(cpu, 7, m);
			WR_MEM(dEA, m);
		}
		break;
	}

	if (z80_cc < cycles)
		goto fetch_xx;
	return z80_cc;
}

void z80_dump_state(z80_cpu_t *cpu)
{
	char dasm[80];
	uint32_t pc = PC;
	uint32_t len = z80_dasm(dasm, pc, &mem[pc], &mem[pc]);
	uint32_t n;
	len = len & 0xffff;
	printf("%4.7fs BC:%04x DE:%04x HL:%04x A:%02x %c%c%c%c%c%c%c%c (HL):%02x SP:%04x PC:%04x ",
		tmr_time_to_double(time_now()), BC, DE, HL, A,
		F & SF ? 'S' : '-',
		F & ZF ? 'Z' : '-',
		F & YF ? 'Y' : '-',
		F & HF ? 'H' : '-',
		F & XF ? 'X' : '-',
		F & PF ? 'P' : '-',
		F & NF ? 'N' : '-',
		F & CF ? 'C' : '-',
		mem[HL], SP, PC);
	for (n = 0; n < 4; n++) {
		if (n < len)
			printf(" %02x", mem[pc+n]);
		else
			printf("   ");
	}
	printf("  %s\n", dasm);
}

void z80_reset(z80_cpu_t *cpu)
{
	memset(cpu, 0, sizeof(*cpu));
	F = ZF;
	IX = 0xffff;
	IY = 0xffff;
}
