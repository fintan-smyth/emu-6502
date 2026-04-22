#include "emu6502.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

void	map_memory(struct pt_entry *pagetable, uint16_t addr, uint8_t pages,
				uint8_t *memory, void *read_handler, void *write_handler)
{
	uint8_t pageno = addr >> 10;
	for (uint8_t i = 0; i < pages; i++)
	{
		struct pt_entry *entry = &pagetable[pageno + i];

		entry->memory = (memory == NULL) ? NULL : &memory[i * 0x400];
		entry->read_handler = read_handler;
		entry->write_handler = write_handler;
	}
}

uint8_t	read_byte(t_cpu *cpu, size_t addr)
{
	uint8_t	pageno = addr >> 10;
	struct pt_entry *entry = &cpu->pagetable[pageno];

	if (entry->memory)
		return entry->memory[addr & 0x03FF];

	return entry->read_handler(entry, cpu, addr);
}

uint16_t	read_word(t_cpu *cpu, size_t addr)
{
	uint16_t word = 0;
	uint8_t lo = read_byte(cpu, addr);
	uint8_t hi = read_byte(cpu, addr + 1);

	word = (hi << 8) | lo;

	return word;
}

uint16_t	read_word_zp(t_cpu *cpu, size_t addr)
{
	uint16_t word = 0;
	uint8_t lo = read_byte(cpu, addr);
	uint8_t hi = read_byte(cpu, (addr + 1) & 0xFF);

	word = (hi << 8) | lo;

	return word;
}

void	write_byte(t_cpu *cpu, size_t addr, uint8_t value)
{
	uint8_t	pageno = addr >> 10;
	struct pt_entry *entry = &cpu->pagetable[pageno];

	if (entry->write_handler)
		entry->write_handler(entry, cpu, addr, value);
}

void	passthrough_write(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val)
{
	entry->memory[addr & 0x3FF] = val;
	(void)arg;
}

void	setup_default_pagetable(t_cpu *cpu)
{
	map_memory(cpu->pagetable, 0x00, 64, cpu->memory, NULL, passthrough_write);
}

uint16_t	get_addr(t_cpu *cpu, AddrMode mode)
{
	uint16_t	addrbus = 0x0;
	uint16_t	ptr;
	int8_t		rel;
	switch (mode) {
		case (RELATIVE):
			rel = read_byte(cpu, cpu->pc + 1);
			ptr = cpu->pc + 2;
			addrbus = cpu->pc + rel;
			if ((ptr & 0xFF00) != ((addrbus + 2) & 0xFF00))
				cpu->cycle_events |= CYCLE_PAGECROSS;
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
			addrbus = read_byte(cpu, cpu->pc + 1);
			addrbus = read_word_zp(cpu, addrbus);
			if ((addrbus & 0xFF) + cpu->y > 0xFF)
				cpu->cycle_events |= CYCLE_PAGECROSS;
			addrbus += cpu->y;
			break;
		case (ABSOLUTE):
			addrbus = read_word(cpu, cpu->pc + 1);
			break;
		case (ABSOLUTE_X):
			addrbus = read_word(cpu, cpu->pc + 1);
			if ((addrbus & 0xFF) + cpu->x > 0xFF)
				cpu->cycle_events |= CYCLE_PAGECROSS;
			addrbus += cpu->x;
			break;
		case (ABSOLUTE_Y):
			addrbus = read_word(cpu, cpu->pc + 1);
			if ((addrbus & 0xFF) + cpu->y > 0xFF)
				cpu->cycle_events |= CYCLE_PAGECROSS;
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
			op = cpu->a;
			break;
		case (IMMEDIATE):
			op = read_byte(cpu, cpu->pc + 1);
			break;
		default:
			op = read_byte(cpu, get_addr(cpu, mode));
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
