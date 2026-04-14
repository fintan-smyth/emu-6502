#include "emu6502.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

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
	uint16_t ptr;
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
			ptr = read_word(cpu, cpu->pc + 1);
			if ((ptr & 0xFF) == 0xFF)
			{
				cpu->addrbus = read_byte(cpu, ptr);
				cpu->addrbus |= read_byte(cpu, ptr & 0xFF00);
			}
			else
				cpu->addrbus = read_word(cpu, ptr);
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
