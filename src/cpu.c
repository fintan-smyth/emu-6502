#include "emu6502.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

static const t_instruct codes[256] = {
	[0x00] = { .instruction = BRK, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x01] = { .instruction = ORA, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 255 },
	[0x02] = { },
	[0x03] = { },
	[0x04] = { },
	[0x05] = { .instruction = ORA, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0x06] = { .instruction = ASL, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0x07] = { },
	[0x08] = { .instruction = PHP, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x09] = { .instruction = ORA, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 255 },
	[0x0A] = { .instruction = ASL, .n_bytes = 1, .addrmode = ACCUMULATOR, .cycles = 255 },
	[0x0B] = {  },
	[0x0C] = {  },
	[0x0D] = { .instruction = ORA, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x0E] = { .instruction = ASL, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x0F] = {  },
	[0x10] = { .instruction = BPL, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 255 },
	[0x11] = { .instruction = ORA, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 255 },
	[0x12] = {  },
	[0x13] = {  },
	[0x14] = {  },
	[0x15] = { .instruction = ORA, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0x16] = { .instruction = ASL, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0x17] = {  },
	[0x18] = { .instruction = CLC, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x19] = { .instruction = ORA, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 255 },
	[0x1A] = {  },
	[0x1B] = {  },
	[0x1C] = {  },
	[0x1D] = { .instruction = ORA, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0x1E] = { .instruction = ASL, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0x1F] = {  },
	[0x20] = { .instruction = JSR, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x21] = { .instruction = AND, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 255 },
	[0x22] = {  },
	[0x23] = {  },
	[0x24] = { .instruction = BIT, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0x25] = { .instruction = AND, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0x26] = { .instruction = ROL, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0x27] = {  },
	[0x28] = { .instruction = PLP, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x29] = { .instruction = AND, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 255 },
	[0x2A] = { .instruction = ROL, .n_bytes = 1, .addrmode = ACCUMULATOR, .cycles = 255 },
	[0x2B] = {  },
	[0x2C] = { .instruction = BIT, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x2D] = { .instruction = AND, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x2E] = { .instruction = ROL, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x2F] = {  },
	[0x30] = { .instruction = BMI, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 255 },
	[0x31] = { .instruction = AND, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 255 },
	[0x32] = {  },
	[0x33] = {  },
	[0x34] = {  },
	[0x35] = { .instruction = AND, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0x36] = { .instruction = ROL, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0x37] = {  },
	[0x38] = { .instruction = SEC, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x39] = { .instruction = AND, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 255 },
	[0x3A] = {  },
	[0x3B] = {  },
	[0x3C] = {  },
	[0x3D] = { .instruction = AND, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0x3E] = { .instruction = ROL, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0x3F] = {  },
	[0x40] = { .instruction = RTI, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x41] = { .instruction = EOR, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 255 },
	[0x42] = {  },
	[0x43] = {  },
	[0x44] = {  },
	[0x45] = { .instruction = EOR, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0x46] = { .instruction = LSR, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0x47] = {  },
	[0x48] = { .instruction = PHA, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x49] = { .instruction = EOR, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 255 },
	[0x4A] = { .instruction = LSR, .n_bytes = 1, .addrmode = ACCUMULATOR, .cycles = 255 },
	[0x4B] = {  },
	[0x4C] = { .instruction = JMP, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x4D] = { .instruction = EOR, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x4E] = { .instruction = LSR, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x4F] = {  },
	[0x50] = { .instruction = BVC, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 255 },
	[0x51] = { .instruction = EOR, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 255 },
	[0x52] = {  },
	[0x53] = {  },
	[0x54] = {  },
	[0x55] = { .instruction = EOR, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0x56] = { .instruction = LSR, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0x57] = {  },
	[0x58] = { .instruction = CLI, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x59] = { .instruction = EOR, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 255 },
	[0x5A] = {  },
	[0x5B] = {  },
	[0x5C] = {  },
	[0x5D] = { .instruction = EOR, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0x5E] = { .instruction = LSR, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0x5F] = {  },
	[0x60] = { .instruction = RTS, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x61] = { .instruction = ADC, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 255 },
	[0x62] = {  },
	[0x63] = {  },
	[0x64] = {  },
	[0x65] = { .instruction = ADC, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0x66] = { .instruction = ROR, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0x67] = {  },
	[0x68] = { .instruction = PLA, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x69] = { .instruction = ADC, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 255 },
	[0x6A] = { .instruction = ROR, .n_bytes = 1, .addrmode = ACCUMULATOR, .cycles = 255 },
	[0x6B] = {  },
	[0x6C] = { .instruction = JMP, .n_bytes = 3, .addrmode = ABSOLUTE_INDIRECT, .cycles = 255 },
	[0x6D] = { .instruction = ADC, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x6E] = { .instruction = ROR, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x6F] = {  },
	[0x70] = { .instruction = BVS, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 255 },
	[0x71] = { .instruction = ADC, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 255 },
	[0x72] = {  },
	[0x73] = {  },
	[0x74] = {  },
	[0x75] = { .instruction = ADC, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0x76] = { .instruction = ROR, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0x77] = {  },
	[0x78] = { .instruction = SEI, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x79] = { .instruction = ADC, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 255 },
	[0x7A] = {  },
	[0x7B] = {  },
	[0x7C] = {  },
	[0x7D] = { .instruction = ADC, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0x7E] = { .instruction = ROR, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0x7F] = {  },
	[0x80] = {  },
	[0x81] = { .instruction = STA, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 255 },
	[0x82] = {  },
	[0x83] = {  },
	[0x84] = { .instruction = STY, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0x85] = { .instruction = STA, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0x86] = { .instruction = STX, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0x87] = {  },
	[0x88] = { .instruction = DEY, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x89] = {  },
	[0x8A] = { .instruction = TXA, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x8B] = {  },
	[0x8C] = { .instruction = STY, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x8D] = { .instruction = STA, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x8E] = { .instruction = STX, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0x8F] = {  },
	[0x90] = { .instruction = BCC, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 255 },
	[0x91] = { .instruction = STA, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 255 },
	[0x92] = {  },
	[0x93] = {  },
	[0x94] = { .instruction = STY, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0x95] = { .instruction = STA, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0x96] = { .instruction = STX, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0x97] = {  },
	[0x98] = { .instruction = TYA, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x99] = { .instruction = STA, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 255 },
	[0x9A] = { .instruction = TXS, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x9B] = {  },
	[0x9C] = {  },
	[0x9D] = { .instruction = STA, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0x9E] = {  },
	[0x9F] = {  },
	[0xA0] = { .instruction = LDY, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 255 },
	[0xA1] = { .instruction = LDA, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 255 },
	[0xA2] = { .instruction = LDX, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 255 },
	[0xA3] = {  },
	[0xA4] = { .instruction = LDY, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0xA5] = { .instruction = LDY, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0xA6] = { .instruction = LDX, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0xA7] = {  },
	[0xA8] = { .instruction = TAY, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0xA9] = { .instruction = LDA, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 255 },
	[0xAA] = { .instruction = TAX, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0xAB] = {  },
	[0xAC] = { .instruction = LDY, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0xAD] = { .instruction = LDA, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0xAE] = { .instruction = LDX, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0xAF] = {  },
	[0xB0] = { .instruction = BCS, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 255 },
	[0xB1] = { .instruction = LDA, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 255 },
	[0xB2] = {  },
	[0xB3] = {  },
	[0xB4] = { .instruction = LDY, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0xB5] = { .instruction = LDY, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0xB6] = { .instruction = LDX, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0xB7] = {  },
	[0xB8] = { .instruction = CLV, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0xB9] = { .instruction = LDA, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 255 },
	[0xBA] = { .instruction = TSX, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0xBB] = {  },
	[0xBC] = { .instruction = LDY, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0xBD] = { .instruction = LDA, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0xBE] = { .instruction = LDX, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 255 },
	[0xBF] = {  },
	[0xC0] = { .instruction = CPY, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 255 },
	[0xC1] = { .instruction = CMP, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 255 },
	[0xC2] = {  },
	[0xC3] = {  },
	[0xC4] = { .instruction = CPY, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0xC5] = { .instruction = CPY, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0xC6] = { .instruction = DEC, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0xC7] = {  },
	[0xC8] = { .instruction = INY, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0xC9] = { .instruction = CMP, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 255 },
	[0xCA] = { .instruction = DEX, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0xCB] = {  },
	[0xCC] = { .instruction = CPY, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0xCD] = { .instruction = CMP, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0xCE] = { .instruction = DEC, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0xCF] = {  },
	[0xD0] = { .instruction = BNE, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 255 },
	[0xD1] = { .instruction = CMP, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 255 },
	[0xD2] = {  },
	[0xD3] = {  },
	[0xD4] = {  },
	[0xD5] = { .instruction = CPY, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0xD6] = { .instruction = DEC, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0xD7] = {  },
	[0xD8] = { .instruction = CLD, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0xD9] = { .instruction = CMP, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 255 },
	[0xDA] = {  },
	[0xDB] = {  },
	[0xDC] = { .instruction = CPX, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0xDD] = { .instruction = CMP, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0xDE] = { .instruction = DEC, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0xDF] = {  },
	[0xE0] = { .instruction = CPX, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 255 },
	[0xE1] = { .instruction = SBC, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 255 },
	[0xE2] = {  },
	[0xE3] = {  },
	[0xE4] = { .instruction = CPX, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0xE5] = { .instruction = SBC, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0xE6] = { .instruction = INC, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 255 },
	[0xE7] = {  },
	[0xE8] = { .instruction = INX, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0xE9] = { .instruction = SBC, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 255 },
	[0xEA] = { .instruction = NOP, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0xEB] = {  },
	[0xEC] = {  },
	[0xED] = { .instruction = SBC, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0xEE] = { .instruction = INC, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 255 },
	[0xEF] = {  },
	[0xF0] = { .instruction = BEQ, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 255 },
	[0xF1] = { .instruction = SBC, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 255 },
	[0xF2] = {  },
	[0xF3] = {  },
	[0xF4] = {  },
	[0xF5] = { .instruction = SBC, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0xF6] = { .instruction = INC, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 255 },
	[0xF7] = {  },
	[0xF8] = { .instruction = SED, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0xF9] = { .instruction = SBC, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 255 },
	[0xFA] = {  },
	[0xFB] = {  },
	[0xFC] = {  },
	[0xFD] = { .instruction = SBC, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0xFE] = { .instruction = INC, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 255 },
	[0xFF] = {  },
};

uint8_t	read_byte(t_cpu *cpu, size_t addr)
{
	return cpu->memory[addr];
}
uint16_t	read_word(t_cpu *cpu, size_t addr)
{
	uint16_t word = 0;
	uint8_t lo = cpu->memory[addr];
	uint8_t hi = cpu->memory[addr + 1];

	word = (hi << 8) | lo;

	return word;
}

void	write_byte(t_cpu *cpu, size_t addr, uint8_t value)
{
	cpu->memory[addr] = value;
}

u_int16_t	handle_zeropage_y_indirect(t_cpu *cpu)
{
	uint16_t	addr = 0;
	uint16_t	zp_addr = read_byte(cpu, cpu->pc + 1);			// Get zero-page addr
	uint8_t		zp_content = read_byte(cpu, zp_addr);			// Get zero-page addr contents
	uint8_t		zp_next_content = read_byte(cpu, zp_addr + 1);	// Get zero-page addr + 1 contents

	addr = zp_content + cpu->y;									// Add zp contents to Y register
	if (addr > 0xFF)											// Check for carry
	{
		zp_next_content++;
		addr &= 0xFF;
	}
	addr |= (zp_next_content << 8);								// Combine high-order bits;
	
	return addr;
}

void	get_addr(t_cpu *cpu, AddrMode mode)
{
	cpu->addrbus = 0;
	switch (mode) {
		case (RELATIVE):
			cpu->addrbus = cpu->pc + (int8_t)read_byte(cpu, cpu->pc + 1);
			return;
		case (ZEROPAGE):
			cpu->addrbus = read_byte(cpu, cpu->pc + 1);
			return;
		case (ZEROPAGE_X):
			cpu->addrbus = read_byte(cpu, cpu->pc + 1);
			cpu->addrbus = (cpu->addrbus + cpu->x) & 0xFF;
			return;
		case (ZEROPAGE_Y):
			cpu->addrbus = read_byte(cpu, cpu->pc + 1);
			cpu->addrbus = (cpu->addrbus + cpu->y) & 0xFF;
			return;
		case (ZEROPAGE_X_INDIRECT):
			cpu->addrbus = read_byte(cpu, cpu->pc + 1);
			cpu->addrbus = (cpu->addrbus + cpu->x) & 0xFF;
			cpu->addrbus = read_word(cpu, cpu->addrbus);
			return;
		case (ZEROPAGE_Y_INDIRECT):
			// cpu->addrbus = handle_zeropage_y_indirect(cpu);
			cpu->addrbus = read_byte(cpu, cpu->pc + 1);
			cpu->addrbus = read_word(cpu, cpu->addrbus);
			cpu->addrbus += cpu->y;
			return;
		case (ABSOLUTE):
			cpu->addrbus = read_word(cpu, cpu->pc + 1);
			return;
		case (ABSOLUTE_X):
			cpu->addrbus = read_word(cpu, cpu->pc + 1);
			cpu->addrbus += cpu->x;
			return;
		case (ABSOLUTE_Y):
			cpu->addrbus = read_word(cpu, cpu->pc + 1);
			cpu->addrbus += cpu->y;
			return;
		case (ABSOLUTE_INDIRECT):
			cpu->addrbus = read_word(cpu, cpu->pc + 1);
			cpu->addrbus = read_word(cpu, cpu->addrbus);
			return;
		default:
			return;
	}
}

uint8_t	get_operand(t_cpu *cpu, AddrMode mode)
{
	uint8_t op = 0;
	switch (mode) {
		case (IMPLIED):
			exit(1);
			break;
		case (ACCUMULATOR):
			exit(2);
			break;
		case (IMMEDIATE):
			op = cpu->memory[cpu->pc + 1];
			break;
		default:
			get_addr(cpu, mode);
			op = read_byte(cpu, cpu->addrbus);
			break;
	}

	return op;
}

const t_instruct *get_instruction(uint8_t opcode)
{
	return &codes[opcode];
}

void	push_stack(t_cpu *cpu, uint8_t val)
{
	uint16_t addr = 0x100 | cpu->sp--;
	write_byte(cpu, addr, val);
}

uint8_t pop_stack(t_cpu *cpu)
{
	uint16_t addr = 0x100 | ++cpu->sp;
	return read_byte(cpu, addr);
}
