//     ______                _____ __________ ___      ____   ___
//    / ____/___ ___  __  __/ ___// ____/ __ \__ \    / __ \ <  /
//   / __/ / __ `__ \/ / / / __ \/___ \/ / / /_/ /   / / / / / /
//  / /___/ / / / / / /_/ / /_/ /___/ / /_/ / __/   / /_/ / / /
// /_____/_/ /_/ /_/\__,_/\____/_____/\____/____/   \____(_)_/

#ifndef _EMU6502_H_
# define _EMU6502_H_

#include <stdint.h>
#include <stdio.h>

#define C_FLAG 1,
#define Z_FLAG 2,
#define I_FLAG 4,
#define D_FLAG 8,
#define B_FLAG 16,
#define E_FLAG 32,
#define V_FLAG 64,
#define N_FLAG 128,

enum e_addr_mode
{
	IMPLIED = 0,
	ACCUMULATOR,
	IMMEDIATE,
	RELATIVE,
	ZEROPAGE,
	ZEROPAGE_X,
	ZEROPAGE_Y,
	ZEROPAGE_X_INDIRECT,
	ZEROPAGE_Y_INDIRECT,
	ABSOLUTE,
	ABSOLUTE_X,
	ABSOLUTE_Y,
	ABSOLUTE_INDIRECT,
};

enum instructions
{
	LDA = 0,
	LDX,
	LDY,
	STA,
	STX,
	STY,
	TAX,
	TAY,
	TSX,
	TXA,
	TXS,
	TYA,
	PHA,
	PHP,
	PLA,
	PLP,
	ASL,
	LSR,
	ROL,
	ROR,
	AND,
	BIT,
	EOR,
	ORA,
	ADC,
	CMP,
	CPX,
	CPY,
	SBC,
	DEC,
	DEX,
	DEY,
	INC,
	INX,
	INY,
	BRK,
	JMP,
	JSR,
	RTI,
	RTS,
	BCC,
	BCS,
	BEQ,
	BMI,
	BNE,
	BPL,
	BVC,
	BVS,
	CLC,
	CLD,
	CLI,
	CLV,
	SEC,
	SED,
	SEI,
	NOP,
	INSTRUCTIONS_MAX,
};

typedef struct t_opcode
{
	uint8_t	instruction;
	uint8_t	n_bytes;
	uint8_t	addrmode;
	uint8_t cycles;
}	t_opcode;

typedef struct t_cpu
{
	uint8_t 	a;
	uint8_t 	x;
	uint8_t 	y;
	uint8_t 	sp;
	uint8_t		status;
	uint16_t	pc;
	uint8_t		memory[0x10000];
}	t_cpu;

#endif
