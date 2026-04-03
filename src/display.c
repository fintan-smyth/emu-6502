#include "emu6502.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

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

	printf("\e[31;1m%s\e[m : ", get_instruct_str(instr->instruction));
	for (int i = 0; i < instr->n_bytes; i++)
	  printf("\e[34;1m%02X ", mem[addr + i]);
	printf("\t\e[32;1m%s\e[m\n",get_addrmode_str(instr->addrmode));
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
