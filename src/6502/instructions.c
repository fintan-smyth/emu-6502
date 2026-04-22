#include "emu6502.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

static const t_instruct codes[256] = {
	[0x00] = { .instruction = BRK, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 7 },
	[0x01] = { .instruction = ORA, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 6 },
	[0x02] = { .instruction = HLT, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x03] = { .instruction = SLO, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 8 },
	[0x04] = { .instruction = SKB, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0x05] = { .instruction = ORA, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0x06] = { .instruction = ASL, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 5 },
	[0x07] = { .instruction = SLO, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 5 },
	[0x08] = { .instruction = PHP, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 3 },
	[0x09] = { .instruction = ORA, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0x0A] = { .instruction = ASL, .n_bytes = 1, .addrmode = ACCUMULATOR, .cycles = 2 },
	[0x0B] = { .instruction = ANC, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0x0C] = { .instruction = SKW, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0x0D] = { .instruction = ORA, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0x0E] = { .instruction = ASL, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 6 },
	[0x0F] = { .instruction = SLO, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 6 },
	[0x10] = { .instruction = BPL, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 2 | CYCLE_BRANCHTAKEN},
	[0x11] = { .instruction = ORA, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 5 | CYCLE_PAGECROSS },
	[0x12] = { .instruction = HLT, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x13] = { .instruction = SLO, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 8 },
	[0x14] = { .instruction = SKB, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0x15] = { .instruction = ORA, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0x16] = { .instruction = ASL, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 6 },
	[0x17] = { .instruction = SLO, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 6 },
	[0x18] = { .instruction = CLC, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0x19] = { .instruction = ORA, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 4 | CYCLE_PAGECROSS },
	[0x1A] = { .instruction = NOP, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0x1B] = { .instruction = SLO, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 7 },
	[0x1C] = { .instruction = SKW, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0x1D] = { .instruction = ORA, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0x1E] = { .instruction = ASL, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 7 },
	[0x1F] = { .instruction = SLO, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 7 },
	[0x20] = { .instruction = JSR, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 6 },
	[0x21] = { .instruction = AND, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 6 },
	[0x22] = { .instruction = HLT, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x23] = { .instruction = RLA, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 8 },
	[0x24] = { .instruction = BIT, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0x25] = { .instruction = AND, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0x26] = { .instruction = ROL, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 5 },
	[0x27] = { .instruction = RLA, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 5 },
	[0x28] = { .instruction = PLP, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 4 },
	[0x29] = { .instruction = AND, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0x2A] = { .instruction = ROL, .n_bytes = 1, .addrmode = ACCUMULATOR, .cycles = 2 },
	[0x2B] = { .instruction = ANC, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0x2C] = { .instruction = BIT, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0x2D] = { .instruction = AND, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0x2E] = { .instruction = ROL, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles =  6 },
	[0x2F] = { .instruction = RLA, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 6 },
	[0x30] = { .instruction = BMI, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 2 | CYCLE_BRANCHTAKEN },
	[0x31] = { .instruction = AND, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 5 | CYCLE_PAGECROSS },
	[0x32] = { .instruction = HLT, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x33] = { .instruction = RLA, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 8 },
	[0x34] = { .instruction = SKB, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0x35] = { .instruction = AND, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0x36] = { .instruction = ROL, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 6 },
	[0x37] = { .instruction = RLA, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 6 },
	[0x38] = { .instruction = SEC, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0x39] = { .instruction = AND, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 4 | CYCLE_PAGECROSS },
	[0x3A] = { .instruction = NOP, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0x3B] = { .instruction = RLA, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 7 },
	[0x3C] = { .instruction = SKW, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0x3D] = { .instruction = AND, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0x3E] = { .instruction = ROL, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 7 },
	[0x3F] = { .instruction = RLA, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 7 },
	[0x40] = { .instruction = RTI, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 6 },
	[0x41] = { .instruction = EOR, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 6 },
	[0x42] = { .instruction = HLT, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x43] = { .instruction = SRE, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 8 },
	[0x44] = { .instruction = SKB, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0x45] = { .instruction = EOR, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0x46] = { .instruction = LSR, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 5 },
	[0x47] = { .instruction = SRE, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 5 },
	[0x48] = { .instruction = PHA, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 3 },
	[0x49] = { .instruction = EOR, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0x4A] = { .instruction = LSR, .n_bytes = 1, .addrmode = ACCUMULATOR, .cycles = 2 },
	[0x4B] = { .instruction = ASR, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0x4C] = { .instruction = JMP, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 3 },
	[0x4D] = { .instruction = EOR, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0x4E] = { .instruction = LSR, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 6 },
	[0x4F] = { .instruction = SRE, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 6 },
	[0x50] = { .instruction = BVC, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 2 | CYCLE_BRANCHTAKEN },
	[0x51] = { .instruction = EOR, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 5 | CYCLE_PAGECROSS},
	[0x52] = { .instruction = HLT, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x53] = { .instruction = SRE, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 8 },
	[0x54] = { .instruction = SKB, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0x55] = { .instruction = EOR, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0x56] = { .instruction = LSR, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 6 },
	[0x57] = { .instruction = SRE, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 6 },
	[0x58] = { .instruction = CLI, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0x59] = { .instruction = EOR, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 4 | CYCLE_PAGECROSS },
	[0x5A] = { .instruction = NOP, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0x5B] = { .instruction = SRE, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 7 },
	[0x5C] = { .instruction = SKW, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0x5D] = { .instruction = EOR, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0x5E] = { .instruction = LSR, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 7 },
	[0x5F] = { .instruction = SRE, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 7 },
	[0x60] = { .instruction = RTS, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 6 },
	[0x61] = { .instruction = ADC, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 6 },
	[0x62] = { .instruction = HLT, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x63] = { .instruction = RRA, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 8 },
	[0x64] = { .instruction = SKB, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0x65] = { .instruction = ADC, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0x66] = { .instruction = ROR, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 5 },
	[0x67] = { .instruction = RRA, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 5 },
	[0x68] = { .instruction = PLA, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 4 },
	[0x69] = { .instruction = ADC, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0x6A] = { .instruction = ROR, .n_bytes = 1, .addrmode = ACCUMULATOR, .cycles = 2 },
	[0x6B] = { .instruction = ARR, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0x6C] = { .instruction = JMP, .n_bytes = 3, .addrmode = ABSOLUTE_INDIRECT, .cycles = 5 },
	[0x6D] = { .instruction = ADC, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0x6E] = { .instruction = ROR, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 6 },
	[0x6F] = { .instruction = RRA, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 6 },
	[0x70] = { .instruction = BVS, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 2 | CYCLE_BRANCHTAKEN },
	[0x71] = { .instruction = ADC, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 5 | CYCLE_PAGECROSS },
	[0x72] = { .instruction = HLT, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x73] = { .instruction = RRA, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 8 },
	[0x74] = { .instruction = SKB, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0x75] = { .instruction = ADC, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0x76] = { .instruction = ROR, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 6 },
	[0x77] = { .instruction = RRA, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 6 },
	[0x78] = { .instruction = SEI, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0x79] = { .instruction = ADC, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 4 | CYCLE_PAGECROSS },
	[0x7A] = { .instruction = NOP, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0x7B] = { .instruction = RRA, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 7 },
	[0x7C] = { .instruction = SKW, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0x7D] = { .instruction = ADC, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0x7E] = { .instruction = ROR, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 7 },
	[0x7F] = { .instruction = RRA, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 7 },
	[0x80] = { .instruction = SKB, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0x81] = { .instruction = STA, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 6 },
	[0x82] = { .instruction = SKB, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0x83] = { .instruction = SAX, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 6 },
	[0x84] = { .instruction = STY, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0x85] = { .instruction = STA, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0x86] = { .instruction = STX, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0x87] = { .instruction = SAX, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0x88] = { .instruction = DEY, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0x89] = { .instruction = SKB, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0x8A] = { .instruction = TXA, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0x8B] = { .instruction = XAA, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0x8C] = { .instruction = STY, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0x8D] = { .instruction = STA, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0x8E] = { .instruction = STX, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0x8F] = { .instruction = SAX, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0x90] = { .instruction = BCC, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 2 | CYCLE_BRANCHTAKEN },
	[0x91] = { .instruction = STA, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 6 },
	[0x92] = { .instruction = HLT, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0x93] = { .instruction = SHA, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 6 },
	[0x94] = { .instruction = STY, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0x95] = { .instruction = STA, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0x96] = { .instruction = STX, .n_bytes = 2, .addrmode = ZEROPAGE_Y, .cycles = 4 },
	[0x97] = { .instruction = SAX, .n_bytes = 2, .addrmode = ZEROPAGE_Y, .cycles = 4 },
	[0x98] = { .instruction = TYA, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0x99] = { .instruction = STA, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 5 },
	[0x9A] = { .instruction = TXS, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0x9B] = { .instruction = SHS, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 5 },
	[0x9C] = { .instruction = SHY, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 5 },
	[0x9D] = { .instruction = STA, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 5 },
	[0x9E] = { .instruction = SHX, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 5 },
	[0x9F] = { .instruction = SHA, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 5 },
	[0xA0] = { .instruction = LDY, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0xA1] = { .instruction = LDA, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 6 },
	[0xA2] = { .instruction = LDX, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0xA3] = { .instruction = LAX, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 6 },
	[0xA4] = { .instruction = LDY, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0xA5] = { .instruction = LDA, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0xA6] = { .instruction = LDX, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0xA7] = { .instruction = LAX, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0xA8] = { .instruction = TAY, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0xA9] = { .instruction = LDA, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0xAA] = { .instruction = TAX, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0xAB] = { .instruction = OAL, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0xAC] = { .instruction = LDY, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0xAD] = { .instruction = LDA, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0xAE] = { .instruction = LDX, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0xAF] = { .instruction = LAX, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0xB0] = { .instruction = BCS, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 2 | CYCLE_BRANCHTAKEN },
	[0xB1] = { .instruction = LDA, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 5 | CYCLE_PAGECROSS },
	[0xB2] = { .instruction = HLT, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0xB3] = { .instruction = LAX, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 5 | CYCLE_PAGECROSS },
	[0xB4] = { .instruction = LDY, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0xB5] = { .instruction = LDA, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0xB6] = { .instruction = LDX, .n_bytes = 2, .addrmode = ZEROPAGE_Y, .cycles = 4 },
	[0xB7] = { .instruction = LAX, .n_bytes = 2, .addrmode = ZEROPAGE_Y, .cycles = 4 },
	[0xB8] = { .instruction = CLV, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0xB9] = { .instruction = LDA, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 4 | CYCLE_PAGECROSS },
	[0xBA] = { .instruction = TSX, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0xBB] = { .instruction = LAS, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 4 | CYCLE_PAGECROSS },
	[0xBC] = { .instruction = LDY, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0xBD] = { .instruction = LDA, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0xBE] = { .instruction = LDX, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 4 | CYCLE_PAGECROSS },
	[0xBF] = { .instruction = LAX, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 4 | CYCLE_PAGECROSS },
	[0xC0] = { .instruction = CPY, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0xC1] = { .instruction = CMP, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 6 },
	[0xC2] = { .instruction = SKB, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0xC3] = { .instruction = DCP, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 8 },
	[0xC4] = { .instruction = CPY, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0xC5] = { .instruction = CMP, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0xC6] = { .instruction = DEC, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 5 },
	[0xC7] = { .instruction = DCP, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 5 },
	[0xC8] = { .instruction = INY, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0xC9] = { .instruction = CMP, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0xCA] = { .instruction = DEX, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0xCB] = { .instruction = SBX, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0xCC] = { .instruction = CPY, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0xCD] = { .instruction = CMP, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0xCE] = { .instruction = DEC, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 6 },
	[0xCF] = { .instruction = DCP, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 6 },
	[0xD0] = { .instruction = BNE, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 2 | CYCLE_BRANCHTAKEN },
	[0xD1] = { .instruction = CMP, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 5 | CYCLE_PAGECROSS },
	[0xD2] = { .instruction = HLT, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0xD3] = { .instruction = DCP, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 8 },
	[0xD4] = { .instruction = SKB, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0xD5] = { .instruction = CMP, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0xD6] = { .instruction = DEC, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 6 },
	[0xD7] = { .instruction = DCP, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 6 },
	[0xD8] = { .instruction = CLD, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0xD9] = { .instruction = CMP, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 4 | CYCLE_PAGECROSS },
	[0xDA] = { .instruction = NOP, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0xDB] = { .instruction = DCP, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 7 },
	[0xDC] = { .instruction = SKW, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0xDD] = { .instruction = CMP, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0xDE] = { .instruction = DEC, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 7 },
	[0xDF] = { .instruction = DCP, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 7 },
	[0xE0] = { .instruction = CPX, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0xE1] = { .instruction = SBC, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 6 },
	[0xE2] = { .instruction = SKB, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0xE3] = { .instruction = ISC, .n_bytes = 2, .addrmode = ZEROPAGE_X_INDIRECT, .cycles = 8 },
	[0xE4] = { .instruction = CPX, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0xE5] = { .instruction = SBC, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 3 },
	[0xE6] = { .instruction = INC, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 5 },
	[0xE7] = { .instruction = ISC, .n_bytes = 2, .addrmode = ZEROPAGE, .cycles = 5 },
	[0xE8] = { .instruction = INX, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0xE9] = { .instruction = SBC, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0xEA] = { .instruction = NOP, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0xEB] = { .instruction = SBC, .n_bytes = 2, .addrmode = IMMEDIATE, .cycles = 2 },
	[0xEC] = { .instruction = CPX, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0xED] = { .instruction = SBC, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 4 },
	[0xEE] = { .instruction = INC, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 6 },
	[0xEF] = { .instruction = ISC, .n_bytes = 3, .addrmode = ABSOLUTE, .cycles = 6 },
	[0xF0] = { .instruction = BEQ, .n_bytes = 2, .addrmode = RELATIVE, .cycles = 2 | CYCLE_BRANCHTAKEN },
	[0xF1] = { .instruction = SBC, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 5 | CYCLE_PAGECROSS },
	[0xF2] = { .instruction = HLT, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 255 },
	[0xF3] = { .instruction = ISC, .n_bytes = 2, .addrmode = ZEROPAGE_Y_INDIRECT, .cycles = 8 },
	[0xF4] = { .instruction = SKB, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0xF5] = { .instruction = SBC, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 4 },
	[0xF6] = { .instruction = INC, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 6 },
	[0xF7] = { .instruction = ISC, .n_bytes = 2, .addrmode = ZEROPAGE_X, .cycles = 6 },
	[0xF8] = { .instruction = SED, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0xF9] = { .instruction = SBC, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 4 | CYCLE_PAGECROSS },
	[0xFA] = { .instruction = NOP, .n_bytes = 1, .addrmode = IMPLIED, .cycles = 2 },
	[0xFB] = { .instruction = ISC, .n_bytes = 3, .addrmode = ABSOLUTE_Y, .cycles = 7 },
	[0xFC] = { .instruction = SKW, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0xFD] = { .instruction = SBC, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 4 | CYCLE_PAGECROSS },
	[0xFE] = { .instruction = INC, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 7 },
	[0xFF] = { .instruction = ISC, .n_bytes = 3, .addrmode = ABSOLUTE_X, .cycles = 7 },
};

const t_instruct *get_instruction(uint8_t opcode)
{
	return &codes[opcode];
}

static inline void exec_LDA_core(t_cpu *cpu, uint8_t op)
{
	cpu->a = op;

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
}

static void exec_LDA(t_cpu *cpu, const t_instruct *instr)
{
	exec_LDA_core(cpu, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static inline void exec_LDX_core(t_cpu *cpu, uint8_t op)
{
	cpu->x = op;

	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);
}

static void exec_LDX(t_cpu *cpu, const t_instruct *instr)
{
	exec_LDX_core(cpu, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static inline void exec_LDY_core(t_cpu *cpu, uint8_t op)
{
	cpu->y = op;

	SET_BIT(cpu->status, FLAG_N, cpu->y & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->y == 0);
}

static void exec_LDY(t_cpu *cpu, const t_instruct *instr)
{
	exec_LDY_core(cpu, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static void exec_STA(t_cpu *cpu, const t_instruct *instr)
{
	cpu->addrbus = get_addr(cpu, instr->addrmode);
	write_byte(cpu, cpu->addrbus, cpu->a);
	cpu->pc += instr->n_bytes;
}

static void exec_STX(t_cpu *cpu, const t_instruct *instr)
{
	cpu->addrbus = get_addr(cpu, instr->addrmode);
	write_byte(cpu, cpu->addrbus, cpu->x);
	cpu->pc += instr->n_bytes;
}

static void exec_STY(t_cpu *cpu, const t_instruct *instr)
{
	cpu->addrbus = get_addr(cpu, instr->addrmode);
	write_byte(cpu, cpu->addrbus, cpu->y);
	cpu->pc += instr->n_bytes;
}

static void exec_TAX(t_cpu *cpu, const t_instruct *instr)
{
	cpu->x = cpu->a;

	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);
	cpu->pc += instr->n_bytes;
}

static void exec_TAY(t_cpu *cpu, const t_instruct *instr)
{
	cpu->y = cpu->a;

	SET_BIT(cpu->status, FLAG_N, cpu->y & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->y == 0);
	cpu->pc += instr->n_bytes;
}

static void exec_TSX(t_cpu *cpu, const t_instruct *instr)
{
	cpu->x = cpu->sp;

	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);
	cpu->pc += instr->n_bytes;
}

static void exec_TXA(t_cpu *cpu, const t_instruct *instr)
{
	cpu->a = cpu->x;

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	cpu->pc += instr->n_bytes;
}

static void exec_TXS(t_cpu *cpu, const t_instruct *instr)
{
	cpu->sp = cpu->x;
	cpu->pc += instr->n_bytes;
}

static void exec_TYA(t_cpu *cpu, const t_instruct *instr)
{
	cpu->a = cpu->y;

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	cpu->pc += instr->n_bytes;
}

static void exec_PHA(t_cpu *cpu, const t_instruct *instr)
{
	push_stack(cpu, cpu->a);
	cpu->pc += instr->n_bytes;
}

static void exec_PHP(t_cpu *cpu, const t_instruct *instr)
{
	push_stack(cpu, cpu->status | FLAG_E | FLAG_B);
	cpu->pc += instr->n_bytes;
}

static void exec_PLA(t_cpu *cpu, const t_instruct *instr)
{
	cpu->a = pop_stack(cpu);
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	cpu->pc += instr->n_bytes;
}

static void exec_PLP(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status = (pop_stack(cpu) | FLAG_E) & ~FLAG_B;
	cpu->pc += instr->n_bytes;
}

static inline uint8_t exec_ASL_core(t_cpu *cpu, uint8_t op)
{
	bool to_carry = (op & BIT_7);
	SET_BIT(cpu->status, FLAG_C, to_carry);

	op <<= 1;
	SET_BIT(cpu->status, FLAG_N, op & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, op == 0);

	return op;
}

static void exec_ASL(t_cpu *cpu, const t_instruct *instr)
{
	if (instr->addrmode != ACCUMULATOR)
	{
		uint16_t addr = get_addr(cpu, instr->addrmode);
		uint8_t result = exec_ASL_core(cpu, read_byte(cpu, addr));
		write_byte(cpu, addr, result);
	}
	else
		cpu->a = exec_ASL_core(cpu, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static inline uint8_t exec_LSR_core(t_cpu *cpu, uint8_t op)
{
	bool to_carry = (op & BIT_0);
	SET_BIT(cpu->status, FLAG_C, to_carry);

	op >>= 1;
	SET_BIT(cpu->status, FLAG_N, op & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, op == 0);

	return op;
}

static void exec_LSR(t_cpu *cpu, const t_instruct *instr)
{
	if (instr->addrmode != ACCUMULATOR)
	{
		uint16_t addr = get_addr(cpu, instr->addrmode);
		uint8_t result = exec_LSR_core(cpu, read_byte(cpu, addr));
		write_byte(cpu, addr, result);
	}
	else
		cpu->a = exec_LSR_core(cpu, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static inline uint8_t exec_ROL_core(t_cpu *cpu, uint8_t op)
{
	bool to_carry = (op & BIT_7);
	op <<= 1;
	SET_BIT(op, BIT_0, cpu->status & FLAG_C);

	SET_BIT(cpu->status, FLAG_C, to_carry);
	SET_BIT(cpu->status, FLAG_N, op & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, op == 0);

	return op;
}

static void exec_ROL(t_cpu *cpu, const t_instruct *instr)
{
	if (instr->addrmode != ACCUMULATOR)
	{
		uint16_t addr = get_addr(cpu, instr->addrmode);
		uint8_t result = exec_ROL_core(cpu, read_byte(cpu, addr));
		write_byte(cpu, addr, result);
	}
	else
		cpu->a = exec_ROL_core(cpu, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static inline uint8_t exec_ROR_core(t_cpu *cpu, uint8_t op)
{
	bool to_carry = (op & BIT_0);
	op >>= 1;
	SET_BIT(op, BIT_7, cpu->status & FLAG_C);

	SET_BIT(cpu->status, FLAG_C, to_carry);
	SET_BIT(cpu->status, FLAG_N, op & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, op == 0);

	return op;
}

static void exec_ROR(t_cpu *cpu, const t_instruct *instr)
{
	if (instr->addrmode != ACCUMULATOR)
	{
		uint16_t addr = get_addr(cpu, instr->addrmode);
		uint8_t result = exec_ROR_core(cpu, read_byte(cpu, addr));
		write_byte(cpu, addr, result);
	}
	else
		cpu->a = exec_ROR_core(cpu, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static inline void exec_AND_core(t_cpu *cpu, uint8_t op)
{
	cpu->a &= op;

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
}

static void exec_AND(t_cpu *cpu, const t_instruct *instr)
{
	exec_AND_core(cpu, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static inline void exec_BIT_core(t_cpu *cpu, uint8_t op)
{
	uint8_t result = cpu->a & op;
	SET_BIT(cpu->status, FLAG_N, op & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_V, op & BIT_6);
	SET_BIT(cpu->status, FLAG_Z, result == 0);
}

static void exec_BIT(t_cpu *cpu, const t_instruct *instr)
{
	exec_BIT_core(cpu, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static inline void exec_EOR_core(t_cpu *cpu, uint8_t op)
{
	cpu->a ^= op;

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
}

static void exec_EOR(t_cpu *cpu, const t_instruct *instr)
{
	exec_EOR_core(cpu, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static inline void exec_ORA_core(t_cpu *cpu, uint8_t op)
{
	cpu->a |= op;

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
}

static void exec_ORA(t_cpu *cpu, const t_instruct *instr)
{
	exec_ORA_core(cpu, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static inline void exec_ADC_core(t_cpu *cpu, uint8_t op)
{
	uint16_t sum = cpu->a + op + (cpu->status & FLAG_C);

	bool overflow = ~(op ^ cpu->a) & (sum ^ cpu->a) & SIGN_BIT;
	SET_BIT(cpu->status, FLAG_V, overflow);
	SET_BIT(cpu->status, FLAG_N, sum & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, (sum & 0xFF) == 0);

#ifndef NES_MODE
	if (cpu->status & FLAG_D)
	{
		if ((sum & 0x0F) > 9 || ((cpu->a ^ op ^ sum) & 0x10))
			sum += 0x06;

		SET_BIT(cpu->status, FLAG_C, sum > 0x9F || (sum & 0x100));
		if (cpu->status & FLAG_C)
			sum += 0x60;
	}
#endif
	SET_BIT(cpu->status, FLAG_C, sum > 0xFF);

	cpu->a = (uint8_t)sum;
}

static void exec_ADC(t_cpu *cpu, const t_instruct *instr)
{
	exec_ADC_core(cpu, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static inline uint8_t exec_compare_core(t_cpu *cpu, uint8_t reg, uint8_t op)
{
	uint8_t result = reg - op;
	SET_BIT(cpu->status, FLAG_C, op <= reg);
	SET_BIT(cpu->status, FLAG_Z, op == reg);
	SET_BIT(cpu->status, FLAG_N, result & SIGN_BIT);
	return result;
}

static void exec_CMP(t_cpu *cpu, const t_instruct *instr)
{
	exec_compare_core(cpu, cpu->a, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static void exec_CPX(t_cpu *cpu, const t_instruct *instr)
{
	exec_compare_core(cpu, cpu->x, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static void exec_CPY(t_cpu *cpu, const t_instruct *instr)
{
	exec_compare_core(cpu, cpu->y, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static inline void exec_SBC_core(t_cpu *cpu, uint8_t op)
{
	uint8_t unflipped = op;
	op = ~op;
	uint16_t sum = cpu->a + op + (cpu->status & FLAG_C);

	bool overflow = ~(op ^ cpu->a) & (sum ^ cpu->a) & SIGN_BIT;
	SET_BIT(cpu->status, FLAG_V, overflow);
	SET_BIT(cpu->status, FLAG_N, sum & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, (sum & 0xFF) == 0);

#ifndef NES_MODE
	if (cpu->status & FLAG_D)
	{
		if ((cpu->a ^ unflipped ^ sum) & 0x10)
			sum -= 0x06;

		if (sum < 0x100)
			sum -= 0x60;
	}
#endif

	SET_BIT(cpu->status, FLAG_C, sum > 0xFF);

	cpu->a = (uint8_t)sum;
}

static void exec_SBC(t_cpu *cpu, const t_instruct *instr)
{
	exec_SBC_core(cpu, get_operand(cpu, instr->addrmode));
	cpu->pc += instr->n_bytes;
}

static inline uint8_t exec_decrement_core(t_cpu *cpu, uint8_t op)
{
	op--;
	SET_BIT(cpu->status, FLAG_N, op & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, op == 0);
	return op;
}

static void exec_DEC(t_cpu *cpu, const t_instruct *instr)
{
	uint16_t addr = get_addr(cpu, instr->addrmode);
	uint8_t result = exec_decrement_core(cpu, read_byte(cpu, addr));
	write_byte(cpu, addr, result);
	cpu->pc += instr->n_bytes;
}

static void exec_DEX(t_cpu *cpu, const t_instruct *instr)
{
	cpu->x = exec_decrement_core(cpu, cpu->x);
	cpu->pc += instr->n_bytes;
}

static void exec_DEY(t_cpu *cpu, const t_instruct *instr)
{
	cpu->y = exec_decrement_core(cpu, cpu->y);
	cpu->pc += instr->n_bytes;
}

static inline uint8_t exec_increment_core(t_cpu *cpu, uint8_t op)
{
	op++;
	SET_BIT(cpu->status, FLAG_N, op & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, op == 0);
	return op;
}

static void exec_INC(t_cpu *cpu, const t_instruct *instr)
{
	uint16_t addr = get_addr(cpu, instr->addrmode);
	uint8_t result = exec_increment_core(cpu, read_byte(cpu, addr));
	write_byte(cpu, addr, result);
	cpu->pc += instr->n_bytes;
}

static void exec_INX(t_cpu *cpu, const t_instruct *instr)
{
	cpu->x = exec_increment_core(cpu, cpu->x);
	cpu->pc += instr->n_bytes;
}

static void exec_INY(t_cpu *cpu, const t_instruct *instr)
{
	cpu->y = exec_increment_core(cpu, cpu->y);
	cpu->pc += instr->n_bytes;
}

static void exec_BRK(t_cpu *cpu, const t_instruct *instr)
{
	cpu->pc += 2;
	push_stack(cpu, (cpu->pc >> 8) & 0xFF);
	push_stack(cpu, cpu->pc & 0xFF);
	push_stack(cpu, cpu->status | FLAG_E | FLAG_B);
	cpu->status |= FLAG_I;
	cpu->pc = read_word(cpu, 0xFFFE);
	(void)instr;
}

static void exec_JMP(t_cpu *cpu, const t_instruct *instr)
{
	cpu->addrbus = get_addr(cpu, instr->addrmode);
	cpu->pc = cpu->addrbus;
}

static void exec_JSR(t_cpu *cpu, const t_instruct *instr)
{
	cpu->addrbus = get_addr(cpu, instr->addrmode);

	cpu->pc += instr->n_bytes - 1;
	push_stack(cpu, (cpu->pc >> 8) & 0xFF);
	push_stack(cpu, cpu->pc & 0xFF);
	cpu->pc = cpu->addrbus;
}

static void exec_RTI(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status = (pop_stack(cpu) | FLAG_E) & ~FLAG_B;
	cpu->pc = pop_stack(cpu);
	cpu->pc |= (pop_stack(cpu) << 8);
	(void)instr;
}

static void exec_RTS(t_cpu *cpu, const t_instruct *instr)
{
	cpu->pc = pop_stack(cpu);
	cpu->pc |= (pop_stack(cpu) << 8);
	cpu->pc++;
	(void)instr;
}

static inline void exec_branch_core(t_cpu *cpu, const t_instruct *instr, bool branch)
{
	cpu->addrbus = get_addr(cpu, instr->addrmode);

	if (branch)
	{
		cpu->cycle_events |= CYCLE_BRANCHTAKEN;
		// print_instr(cpu->memory, cpu->pc);
		// printf("%04X -> %04X\n", cpu->pc, cpu->addrbus + instr->n_bytes);
		// if (cpu->cycle_events & CYCLE_PAGECROSS)
		// 	printf("PAGE CROSSED\n");
		// printf("----------\n");
		cpu->pc = cpu->addrbus;
	}

	cpu->pc += instr->n_bytes;
}

static void exec_BCC(t_cpu *cpu, const t_instruct *instr)
{
	exec_branch_core(cpu, instr, !(cpu->status & FLAG_C));
}

static void exec_BCS(t_cpu *cpu, const t_instruct *instr)
{
	exec_branch_core(cpu, instr, (cpu->status & FLAG_C));
}

static void exec_BEQ(t_cpu *cpu, const t_instruct *instr)
{
	exec_branch_core(cpu, instr, (cpu->status & FLAG_Z));
}

static void exec_BMI(t_cpu *cpu, const t_instruct *instr)
{
	exec_branch_core(cpu, instr, (cpu->status & FLAG_N));
}

static void exec_BNE(t_cpu *cpu, const t_instruct *instr)
{
	exec_branch_core(cpu, instr, !(cpu->status & FLAG_Z));
}

static void exec_BPL(t_cpu *cpu, const t_instruct *instr)
{
	exec_branch_core(cpu, instr, !(cpu->status & FLAG_N));
}

static void exec_BVC(t_cpu *cpu, const t_instruct *instr)
{
	exec_branch_core(cpu, instr, !(cpu->status & FLAG_V));
}

static void exec_BVS(t_cpu *cpu, const t_instruct *instr)
{
	exec_branch_core(cpu, instr, (cpu->status & FLAG_V));
}

static void exec_CLC(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status &= ~FLAG_C;
	cpu->pc += instr->n_bytes;
}

static void exec_CLD(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status &= ~FLAG_D;
	cpu->pc += instr->n_bytes;
}

static void exec_CLI(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status &= ~FLAG_I;
	cpu->pc += instr->n_bytes;
}

static void exec_CLV(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status &= ~FLAG_V;
	cpu->pc += instr->n_bytes;
}

static void exec_SEC(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status |= FLAG_C;
	cpu->pc += instr->n_bytes;
}

static void exec_SED(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status |= FLAG_D;
	cpu->pc += instr->n_bytes;
}

static void exec_SEI(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status |= FLAG_I;
	cpu->pc += instr->n_bytes;
}

static void exec_NOP(t_cpu *cpu, const t_instruct *instr)
{
	cpu->pc += instr->n_bytes;
	return ;
}

static void exec_HLT(t_cpu *cpu, const t_instruct *instr)
{
	printf("HLT encountered: exiting...\n");
	dprintf(cpu->logfd, "lookup table size: %lu\n", sizeof(codes));
	(void)cpu;
	(void)instr;
	exit(4);
}

static void exec_SKB(t_cpu *cpu, const t_instruct *instr)
{
	(void)get_operand(cpu, instr->addrmode);
	cpu->pc += instr->n_bytes;
	return ;
}

static void exec_SKW(t_cpu *cpu, const t_instruct *instr)
{
	(void)get_operand(cpu, instr->addrmode);
	cpu->pc += instr->n_bytes;
	return ;
}

static void exec_SLO(t_cpu *cpu, const t_instruct *instr)
{
	uint16_t addr = get_addr(cpu, instr->addrmode);
	uint8_t result = exec_ASL_core(cpu, read_byte(cpu, addr));
	write_byte(cpu, addr, result);
	exec_ORA_core(cpu, result);
	cpu->pc += instr->n_bytes;
}

static void exec_RLA(t_cpu *cpu, const t_instruct *instr)
{
	uint16_t addr = get_addr(cpu, instr->addrmode);
	uint8_t result = exec_ROL_core(cpu, read_byte(cpu, addr));
	write_byte(cpu, addr, result);
	exec_AND_core(cpu, result);
	cpu->pc += instr->n_bytes;
}

static void exec_SRE(t_cpu *cpu, const t_instruct *instr)
{
	uint16_t addr = get_addr(cpu, instr->addrmode);
	uint8_t result = exec_LSR_core(cpu, read_byte(cpu, addr));
	write_byte(cpu, addr, result);
	exec_EOR_core(cpu, result);
	cpu->pc += instr->n_bytes;
}

static void exec_RRA(t_cpu *cpu, const t_instruct *instr)
{
	uint16_t addr = get_addr(cpu, instr->addrmode);
	uint8_t result = exec_ROR_core(cpu, read_byte(cpu, addr));
	write_byte(cpu, addr, result);
	exec_ADC_core(cpu, result);
	cpu->pc += instr->n_bytes;
}

static void exec_SAX(t_cpu *cpu, const t_instruct *instr)
{
	uint8_t op = cpu->x & cpu->a;
	cpu->addrbus = get_addr(cpu, instr->addrmode);
	write_byte(cpu, cpu->addrbus, op);
	cpu->pc += instr->n_bytes;
}

static void exec_SHA(t_cpu *cpu, const t_instruct *instr)
{
	cpu->addrbus = get_addr(cpu, instr->addrmode);
	uint8_t op = cpu->x & cpu->a & (((cpu->addrbus >> 8) & 0xFF) + 1);
	write_byte(cpu, cpu->addrbus, op);
	cpu->pc += instr->n_bytes;
}

static void exec_SHX(t_cpu *cpu, const t_instruct *instr)
{
	cpu->addrbus = get_addr(cpu, instr->addrmode);
	uint8_t op = cpu->x & (((cpu->addrbus >> 8) & 0xFF) + 1);
	write_byte(cpu, cpu->addrbus, op);
	cpu->pc += instr->n_bytes;
}

static void exec_SHY(t_cpu *cpu, const t_instruct *instr)
{
	cpu->addrbus = get_addr(cpu, instr->addrmode);
	uint8_t op = cpu->y & (((cpu->addrbus >> 8) & 0xFF) + 1);
	write_byte(cpu, cpu->addrbus, op);
	cpu->pc += instr->n_bytes;
}

static void exec_LAX(t_cpu *cpu, const t_instruct *instr)
{
	uint8_t op = get_operand(cpu, instr->addrmode);
	exec_LDA_core(cpu, op);
	exec_LDX_core(cpu, op);
	cpu->pc += instr->n_bytes;
}

static void exec_DCP(t_cpu *cpu, const t_instruct *instr)
{
	uint16_t addr = get_addr(cpu, instr->addrmode);
	uint8_t result = exec_decrement_core(cpu, read_byte(cpu, addr));
	write_byte(cpu, addr, result);
	exec_compare_core(cpu, cpu->a, result);
	cpu->pc += instr->n_bytes;
}

static void exec_ARR(t_cpu *cpu, const t_instruct *instr)
{
	uint8_t result = cpu->a & get_operand(cpu, instr->addrmode);
	result >>= 1;
	SET_BIT(result, BIT_7, cpu->status & FLAG_C);

	SET_BIT(cpu->status, FLAG_N, result & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, result == 0);
	SET_BIT(cpu->status, FLAG_C, result & BIT_6);

	// Set overflow flag if bit 6 is different from bit 7
	SET_BIT(cpu->status, FLAG_V, ((result >> 6) ^ (result >> 5)) & 1);

	cpu->a = result;
	cpu->pc += instr->n_bytes;
}

static void exec_XAA(t_cpu *cpu, const t_instruct *instr)
{
	// idealise version used to pass simple NES tests
	cpu->a = cpu->x;
	cpu->a &= get_operand(cpu, instr->addrmode);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	cpu->pc += instr->n_bytes;
}

static void exec_SHS(t_cpu *cpu, const t_instruct *instr)
{
	cpu->pc += instr->n_bytes;
}

static void exec_OAL(t_cpu *cpu, const t_instruct *instr)
{
	// Behaves like LAX on NES
	uint8_t op = get_operand(cpu, instr->addrmode);
	exec_LDA_core(cpu, op);
	exec_LDX_core(cpu, op);
	cpu->pc += instr->n_bytes;
}

static void exec_LAS(t_cpu *cpu, const t_instruct *instr)
{
	cpu->sp &= get_operand(cpu, instr->addrmode);
	cpu->a = cpu->sp;
	cpu->x = cpu->sp;
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	cpu->pc += instr->n_bytes;
}

static void exec_SBX(t_cpu *cpu, const t_instruct *instr)
{
	uint8_t reg = cpu->a & cpu->x;
	uint8_t op = get_operand(cpu, instr->addrmode);
	cpu->x = exec_compare_core(cpu, reg, op);
	cpu->pc += instr->n_bytes;
}

static void exec_ISC(t_cpu *cpu, const t_instruct *instr)
{
	uint16_t addr = get_addr(cpu, instr->addrmode);
	uint8_t result = exec_increment_core(cpu, read_byte(cpu, addr));

	write_byte(cpu, addr, result);
	exec_SBC_core(cpu, result);
	cpu->pc += instr->n_bytes;
}

static void exec_ASR(t_cpu *cpu, const t_instruct *instr)
{
	cpu->a &= get_operand(cpu, instr->addrmode);
	cpu->a = exec_LSR_core(cpu, cpu->a);
	cpu->pc += instr->n_bytes;
}

static void exec_ANC(t_cpu *cpu, const t_instruct *instr)
{
	exec_AND_core(cpu, get_operand(cpu, instr->addrmode));
	SET_BIT(cpu->status, FLAG_C, cpu->a & BIT_7);
	cpu->pc += instr->n_bytes;
}

static void (*const instr_funcs[])(t_cpu *, const t_instruct *) = {
	[LDA] = exec_LDA,
	[LDX] = exec_LDX,
	[LDY] = exec_LDY,
	[STA] = exec_STA,
	[STX] = exec_STX,
	[STY] = exec_STY,
	[TAX] = exec_TAX,
	[TAY] = exec_TAY,
	[TSX] = exec_TSX,
	[TXA] = exec_TXA,
	[TXS] = exec_TXS,
	[TYA] = exec_TYA,
	[PHA] = exec_PHA,
	[PHP] = exec_PHP,
	[PLA] = exec_PLA,
	[PLP] = exec_PLP,
	[ASL] = exec_ASL,
	[LSR] = exec_LSR,
	[ROL] = exec_ROL,
	[ROR] = exec_ROR,
	[AND] = exec_AND,
	[BIT] = exec_BIT,
	[EOR] = exec_EOR,
	[ORA] = exec_ORA,
	[ADC] = exec_ADC,
	[CMP] = exec_CMP,
	[CPX] = exec_CPX,
	[CPY] = exec_CPY,
	[SBC] = exec_SBC,
	[DEC] = exec_DEC,
	[DEX] = exec_DEX,
	[DEY] = exec_DEY,
	[INC] = exec_INC,
	[INX] = exec_INX,
	[INY] = exec_INY,
	[BRK] = exec_BRK,
	[JMP] = exec_JMP,
	[JSR] = exec_JSR,
	[RTI] = exec_RTI,
	[RTS] = exec_RTS,
	[BCC] = exec_BCC,
	[BCS] = exec_BCS,
	[BEQ] = exec_BEQ,
	[BMI] = exec_BMI,
	[BNE] = exec_BNE,
	[BPL] = exec_BPL,
	[BVC] = exec_BVC,
	[BVS] = exec_BVS,
	[CLC] = exec_CLC,
	[CLD] = exec_CLD,
	[CLI] = exec_CLI,
	[CLV] = exec_CLV,
	[SEC] = exec_SEC,
	[SED] = exec_SED,
	[SEI] = exec_SEI,
	[NOP] = exec_NOP,
	[HLT] = exec_HLT,
	[SKB] = exec_SKB,
	[SKW] = exec_SKW,
	[SLO] = exec_SLO,
	[RLA] = exec_RLA,
	[SRE] = exec_SRE,
	[RRA] = exec_RRA,
	[SAX] = exec_SAX,
	[SHA] = exec_SHA,
	[SHX] = exec_SHX,
	[SHY] = exec_SHY,
	[LAX] = exec_LAX,
	[DCP] = exec_DCP,
	[ARR] = exec_ARR,
	[XAA] = exec_XAA,
	[SHS] = exec_SHS,
	[OAL] = exec_OAL,
	[LAS] = exec_LAS,
	[SBX] = exec_SBX,
	[ISC] = exec_ISC,
	[ASR] = exec_ASR,
	[ANC] = exec_ANC,
};

void	handle_cycles(t_cpu *cpu, const t_instruct *instr)
{
	uint8_t cycles = 0;
	cycles += instr->cycles & 0x0F;

	switch (instr->cycles & 0xF0)
	{
		case (CYCLE_PAGECROSS):
			if (cpu->cycle_events & CYCLE_PAGECROSS)
				cycles++;
			break;

		case (CYCLE_BRANCHTAKEN):
			if (cpu->cycle_events & CYCLE_BRANCHTAKEN)
			{
				cycles++;
				if (cpu->cycle_events & CYCLE_PAGECROSS)
					cycles++;
			}
			break;

		default:
			break ;
	}
	cpu->cycles += cycles;
}

void	execute_instr(t_cpu *cpu, const t_instruct *instr)
{
	#ifdef NES_MODE
		log_instr(cpu->logfd, cpu, instr);
	#endif

	cpu->cycle_events = 0;

	instr_funcs[instr->instruction](cpu, instr);
	handle_cycles(cpu, instr);
}
