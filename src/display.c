#include "emu6502.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

static const int instr_cols[INSTRUCTIONS_MAX] = {
	[LDA] = COL_RED,
	[LDX] = COL_RED,
	[LDY] = COL_RED,
	[STA] = COL_RED,
	[STX] = COL_RED,
	[STY] = COL_RED,
	[TAX] = COL_CYAN,
	[TAY] = COL_CYAN,
	[TSX] = COL_CYAN,
	[TXA] = COL_CYAN,
	[TXS] = COL_CYAN,
	[TYA] = COL_CYAN,
	[PHA] = COL_MAGENTA,
	[PHP] = COL_MAGENTA,
	[PLA] = COL_MAGENTA,
	[PLP] = COL_MAGENTA,
	[ASL] = COL_BLUE,
	[LSR] = COL_BLUE,
	[ROL] = COL_BLUE,
	[ROR] = COL_BLUE,
	[AND] = COL_BLUE,
	[BIT] = COL_BLUE,
	[EOR] = COL_BLUE,
	[ORA] = COL_BLUE,
	[ADC] = COL_GREEN,
	[CMP] = COL_GREEN,
	[CPX] = COL_GREEN,
	[CPY] = COL_GREEN,
	[SBC] = COL_GREEN,
	[DEC] = COL_CYAN,
	[DEX] = COL_CYAN,
	[DEY] = COL_CYAN,
	[INC] = COL_CYAN,
	[INX] = COL_CYAN,
	[INY] = COL_CYAN,
	[BRK] = COL_YELLOW,
	[JMP] = COL_YELLOW,
	[JSR] = COL_YELLOW,
	[RTI] = COL_YELLOW,
	[RTS] = COL_YELLOW,
	[BCC] = COL_YELLOW,
	[BCS] = COL_YELLOW,
	[BEQ] = COL_YELLOW,
	[BMI] = COL_YELLOW,
	[BNE] = COL_YELLOW,
	[BPL] = COL_YELLOW,
	[BVC] = COL_YELLOW,
	[BVS] = COL_YELLOW,
	[CLC] = COL_WHITE,
	[CLD] = COL_WHITE,
	[CLI] = COL_WHITE,
	[CLV] = COL_WHITE,
	[SEC] = COL_WHITE,
	[SED] = COL_WHITE,
	[SEI] = COL_WHITE,
	[NOP] = COL_WHITE,
};

void	colour_instr(uint8_t opcode)
{
	const t_instruct *instr = get_instruction(opcode);
	if (instr->n_bytes == 0)
		printf("\e[m");
	else
		printf("\e[3%d;1m", instr_cols[instr->instruction]);
}

const char *get_instruct_str(enum instructions instr)
{
	switch (instr) {
		case (LDA):
			return "LDA";
		case (LDX):
			return "LDX";
		case (LDY):
			return "LDY";
		case (STA):
			return "STA";
		case (STX):
			return "STX";
		case (STY):
			return "STY";
		case (TAX):
			return "TAX";
		case (TAY):
			return "TAY";
		case (TSX):
			return "TSX";
		case (TXA):
			return "TXA";
		case (TXS):
			return "TXS";
		case (TYA):
			return "TYA";
		case (PHA):
			return "PHA";
		case (PHP):
			return "PHP";
		case (PLA):
			return "PLA";
		case (PLP):
			return "PLP";
		case (ASL):
			return "ASL";
		case (LSR):
			return "LSR";
		case (ROL):
			return "ROL";
		case (ROR):
			return "ROR";
		case (AND):
			return "AND";
		case (BIT):
			return "BIT";
		case (EOR):
			return "EOR";
		case (ORA):
			return "ORA";
		case (ADC):
			return "ADC";
		case (CMP):
			return "CMP";
		case (CPX):
			return "CPX";
		case (CPY):
			return "CPY";
		case (SBC):
			return "SBC";
		case (DEC):
			return "DEC";
		case (DEX):
			return "DEX";
		case (DEY):
			return "DEY";
		case (INC):
			return "INC";
		case (INX):
			return "INX";
		case (INY):
			return "INY";
		case (BRK):
			return "BRK";
		case (JMP):
			return "JMP";
		case (JSR):
			return "JSR";
		case (RTI):
			return "RTI";
		case (RTS):
			return "RTS";
		case (BCC):
			return "BCC";
		case (BCS):
			return "BCS";
		case (BEQ):
			return "BEQ";
		case (BMI):
			return "BMI";
		case (BNE):
			return "BNE";
		case (BPL):
			return "BPL";
		case (BVC):
			return "BVC";
		case (BVS):
			return "BVS";
		case (CLC):
			return "CLC";
		case (CLD):
			return "CLD";
		case (CLI):
			return "CLI";
		case (CLV):
			return "CLV";
		case (SEC):
			return "SEC";
		case (SED):
			return "SED";
		case (SEI):
			return "SEI";
		case (NOP):
			return "NOP";
		case (INSTRUCTIONS_MAX):
			return "INSTRUCTIONS_MAX";
	}
	return NULL;
}

const char *get_addrmode_str(AddrMode mode)
{
	switch (mode) {
		case (IMPLIED):
			return "IMPLIED";
		case (ACCUMULATOR):
			return "ACCUMULATOR";
		case (IMMEDIATE):
			return "IMMEDIATE";
		case (RELATIVE):
			return "RELATIVE";
		case (ZEROPAGE):
			return "ZEROPAGE";
		case (ZEROPAGE_X):
			return "ZEROPAGE_X";
		case (ZEROPAGE_Y):
			return "ZEROPAGE_Y";
		case (ZEROPAGE_X_INDIRECT):
			return "ZEROPAGE_X_INDIRECT";
		case (ZEROPAGE_Y_INDIRECT):
			return "ZEROPAGE_Y_INDIRECT";
		case (ABSOLUTE):
			return "ABSOLUTE";
		case (ABSOLUTE_X):
			return "ABSOLUTE_X";
		case (ABSOLUTE_Y):
			return "ABSOLUTE_Y";
		case (ABSOLUTE_INDIRECT):
			return "ABSOLUTE_INDIRECT";
	}
	return NULL;
}

void	print_instr(uint8_t *mem, uint16_t addr)
{
	const t_instruct *instr = get_instruction(mem[addr]);

	colour_instr(mem[addr]);
	printf("%s\e[m : ", get_instruct_str(instr->instruction));
	for (int i = 0; i < instr->n_bytes; i++)
	  printf("\e[34;1m%02X ", mem[addr + i]);
	printf("\t\e[32;1m%s\e[m\n", get_addrmode_str(instr->addrmode));
}

void print_status_register(uint8_t status)
{
	const char *status_str = "NVEBDIZC";

	printf("[");
	for (int i = 0; i < 8; i++)
	{
		if (status & (1 << (7 - i)))
			printf("\e[47;30;1m");
		printf("%c\e[m", status_str[i]);
	}
	printf("]\n");
}

void print_registers(t_cpu *cpu)
{
	printf("\e[33;1m### REGISTERS ###\e[m\n");
	printf("\e[35;1mPC\e[m: 0x%04X\t", cpu->pc);
	printf("\e[33;1mX\e[m:  0x%02X\t", cpu->x);
	printf("\e[32;1mA\e[m:  0x%02X\n", cpu->a);
	printf("\e[34;1mSP\e[m: 0x%02X\t", cpu->sp);
	printf("\e[33;1mY\e[m:  0x%02X       ", cpu->y);
	print_status_register(cpu->status);

	printf("\n");
}

void	print_stack(t_cpu *cpu)
{
	printf("\e[31;1mSTK\e[m:\e[34;1m");
	for (uint8_t i = 0xFF; i > cpu->sp; i--)
		printf(" %02X", read_byte(cpu, 0x100 | i));
	printf("\e[m\n");
}

void	print_zeropage(t_cpu *cpu)
{
	printf("   \e[34;4;1m 00");
	for (uint16_t i = 1; i < 0x20; i++)
	{
		printf("%s%s", i % 4 == 0 ? " " : "",
						i % 16 == 0 ? " " : ""
		);
		printf(" %02X", i);
	}
	printf("\e[m\n\e[34;1m00|");
	for (uint16_t i = 0;;)
	{
		uint8_t op = read_byte(cpu, i++);
		colour_instr(op);
		printf(" %02X", op);
		printf("%s%s", i % 4 == 0 ? " " : "",
						i % 16 == 0 ? " " : ""
		);
		if (i == 0x100)
			break;
		if (i % 32 == 0)
			printf("\n\e[34;1m%02X|", i);
	}
	printf("\e[m\n");
}

void print_debug_view(t_cpu *cpu, uint16_t pc)
{
	print_instr(cpu->memory, pc);
	printf("\n");
	print_registers(cpu);
	print_stack(cpu);
	print_zeropage(cpu);
}
