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

uint16_t	read_word_zp(t_cpu *cpu, size_t addr)
{
	uint16_t word = 0;
	uint8_t lo = cpu->memory[addr];
	uint8_t hi = cpu->memory[(addr + 1) & 0xFF];

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

uint16_t	get_addr(t_cpu *cpu, AddrMode mode)
{
	uint16_t addrbus = 0x0;
	uint16_t ptr;
	switch (mode) {
		case (RELATIVE):
			addrbus = cpu->pc + (int8_t)read_byte(cpu, cpu->pc + 1);
			break;
		case (ZEROPAGE):
			addrbus = read_byte(cpu, cpu->pc + 1);
			break;
		case (ZEROPAGE_X):
			addrbus = read_byte(cpu, cpu->pc + 1);
			addrbus = (addrbus + cpu->x) & 0xFF;
			break;
		case (ZEROPAGE_Y):
			addrbus = read_byte(cpu, cpu->pc + 1);
			addrbus = (addrbus + cpu->y) & 0xFF;
			break;
		case (ZEROPAGE_X_INDIRECT):
			addrbus = read_byte(cpu, cpu->pc + 1);
			addrbus = (addrbus + cpu->x) & 0xFF;
			addrbus = read_word_zp(cpu, addrbus);
			break;
		case (ZEROPAGE_Y_INDIRECT):
			// addrbus = handle_zeropage_y_indirect(cpu);
			addrbus = read_byte(cpu, cpu->pc + 1);
			addrbus = read_word_zp(cpu, addrbus);
			addrbus += cpu->y;
			break;
		case (ABSOLUTE):
			addrbus = read_word(cpu, cpu->pc + 1);
			break;
		case (ABSOLUTE_X):
			addrbus = read_word(cpu, cpu->pc + 1);
			addrbus += cpu->x;
			break;
		case (ABSOLUTE_Y):
			addrbus = read_word(cpu, cpu->pc + 1);
			addrbus += cpu->y;
			break;
		case (ABSOLUTE_INDIRECT):
			ptr = read_word(cpu, cpu->pc + 1);
			if ((ptr & 0xFF) == 0xFF)
			{
				addrbus = read_byte(cpu, ptr);
				addrbus |= read_byte(cpu, ptr & 0xFF00) << 8;
			}
			else
				addrbus = read_word(cpu, ptr);
			break;
		default:
			break;
	}
	return addrbus;
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
			op = read_byte(cpu, get_addr(cpu, mode));
			break;
	}

	return op;
}

uint8_t	*get_operand_addr(t_cpu *cpu, AddrMode mode)
{
	switch (mode) {
		case (IMPLIED):
			exit(3);
			break;
		case (ACCUMULATOR):
			return &cpu->a;
			break;
		case (IMMEDIATE):
			return &cpu->memory[cpu->pc + 1];
			break;
		default:
			return &cpu->memory[get_addr(cpu, mode)];
			break;
	}
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
