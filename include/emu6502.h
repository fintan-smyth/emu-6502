//     ______                _____ __________ ___      ____   ___
//    / ____/___ ___  __  __/ ___// ____/ __ \__ \    / __ \ <  /
//   / __/ / __ `__ \/ / / / __ \/___ \/ / / /_/ /   / / / / / /
//  / /___/ / / / / / /_/ / /_/ /___/ / /_/ / __/   / /_/ / / /
// /_____/_/ /_/ /_/\__,_/\____/_____/\____/____/   \____(_)_/

#ifndef _EMU6502_H_
# define _EMU6502_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "fslib.h"

#define NES_MODE

#define FLAG_C 1
#define FLAG_Z 2
#define FLAG_I 4
#define FLAG_D 8
#define FLAG_B 16
#define FLAG_E 32
#define FLAG_V 64
#define FLAG_N 128

#define SHIFT_C 0
#define SHIFT_Z 1
#define SHIFT_I 2
#define SHIFT_D 3
#define SHIFT_B 4
#define SHIFT_E 5
#define SHIFT_V 6
#define SHIFT_N 7

#define BIT_0 1
#define BIT_1 2
#define BIT_2 4
#define BIT_3 8
#define BIT_4 16
#define BIT_5 32
#define BIT_6 64
#define BIT_7 128
#define BIT_8 256

#define SIGN_BIT 128

#define CYCLE_PAGECROSS 64
#define CYCLE_BRANCHTAKEN 128
#define CYCLE_MASK 0x0F

#define SET_BIT(var, mask, expr) \
    ((var) = ((var) & ~(mask)) | ((!!(expr)) ? (mask) : 0))

#define COL_BLACK 0
#define COL_RED 1
#define COL_GREEN 2
#define COL_YELLOW 3
#define COL_BLUE 4
#define COL_MAGENTA 5
#define COL_CYAN 6
#define COL_WHITE 7


typedef enum e_addr_mode
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
}	AddrMode;

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
	HLT,
	SKB,
	SKW,
	SLO,
	RLA,
	SRE,
	RRA,
	SAX,
	SHA,
	SHX,
	SHY,
	LAX,
	DCP,
	ARR,
	XAA,
	SHS,
	OAL,
	LAS,
	SBX,
	ISC,
	ASR,
	ANC,
	ISCTRUCTIONS_MAX,
};

typedef struct t_opcode
{
	uint8_t	instruction;
	uint8_t	n_bytes;
	uint8_t	addrmode;
	uint8_t cycles;
}	t_instruct;

struct pt_entry
{
	uint8_t	*memory;
	uint8_t	(*read_handler)(struct pt_entry *entry, void *, uint16_t);
	void	(*write_handler)(struct pt_entry *entry, void *, uint16_t, uint8_t);
};

typedef struct t_cpu
{
	uint8_t 	a;
	uint8_t 	x;
	uint8_t 	y;
	uint8_t 	sp;
	uint8_t		status;
	uint16_t	pc;
	uint8_t		dbus;
	uint16_t	addrbus;
	uint8_t		*memory;
	size_t		memsize;
	size_t		cycles;
	uint8_t		cycle_events;
	int			logfd;
	struct pt_entry	pagetable[0x40];
}	t_cpu;

uint8_t		read_byte(t_cpu *cpu, size_t addr);
uint16_t	read_word(t_cpu *cpu, size_t addr);
uint16_t	read_word_zp(t_cpu *cpu, size_t addr);
void		write_byte(t_cpu *cpu, size_t addr, uint8_t value);
void		passthrough_write(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val);
void		map_memory(struct pt_entry *pagetable, uint16_t addr, uint8_t pages,
				 uint8_t *memory, void *read_handler, void *write_handler);
void		setup_default_pagetable(t_cpu *cpu);
void		push_stack(t_cpu *cpu, uint8_t val);
uint8_t 	pop_stack(t_cpu *cpu);

uint16_t	get_addr(t_cpu *cpu, AddrMode mode);
uint8_t		get_operand(t_cpu *cpu, AddrMode mode);

const t_instruct	*get_instruction(uint8_t opcode);
const char			*get_instruct_str(enum instructions instr);
const char			*get_addrmode_str(AddrMode mode);

void	execute_instr(t_cpu *cpu, const t_instruct *instr);
void	print_instr(t_cpu *cpu, uint16_t addr);
void 	print_registers(t_cpu *cpu);
void	print_debug_view(t_cpu *cpu, uint16_t pc);
void	log_instr(int fd, t_cpu *cpu, const t_instruct *instr);

#endif
