#include "nes.h"
#include "emu6502.h"
#include <raylib.h>
#include <stdint.h>
#include <sys/types.h>

void	init_nes(t_nes *nes)
{
	nes->cpu.parent_device = nes;
	nes->ppu.nes = nes;
	nes->ppu.nmi_pin = &nes->cpu.nmi_pending;
	nes->settings.fps = 60;
}

uint8_t nes_step(t_nes *nes)
{
	uint8_t cycles = cpu_step(&nes->cpu);
	nes->cpu.catchup_cycles += cycles;
	ppu_tick_for(&nes->ppu, cycles * 3);
	apu_tick_for(&nes->apu, cycles);

	return cycles;
}

uint64_t nes_run_for(t_nes *nes, uint64_t n_cycles)
{
	uint64_t cycles = 0;

	while (cycles < n_cycles)
		cycles += nes_step(nes);

	return cycles - n_cycles;
}

// void	game_loop(t_nes *nes)
// {
//
// }

uint8_t	cpu_ppu_reg_read(struct pt_entry *entry, void *arg, uint16_t addr)
{
	t_cpu	*cpu = arg;
	t_ppu	*ppu = &((t_nes *)cpu->parent_device)->ppu;
	PPUReg	reg = addr & 0x7;
	uint8_t	data = 0;
	// printf("mapped: %04X reg: %02X\n", addr, reg);

	switch (reg) {
		case (PPUCTRL): // 0x2000
			printf("\e[31;1mError\e[m: %s: Invalid read\n", get_ppureg_str(reg));
			exit(1);
			break;
		case (PPUMASK): // 0x2001
			printf("\e[31;1mError\e[m: %s: Invalid read\n", get_ppureg_str(reg));
			exit(1);
			break;
		case (PPUSTATUS): // 0x2002
			data = ppu->registers[PPUSTATUS];
			// printf("before: 0x%02X\n", ppu->registers[PPUSTATUS]);
			SET_BIT(ppu->registers[PPUSTATUS], VBLANK_ENABLE, 0);
			// printf("after:  0x%02X\n", ppu->registers[PPUSTATUS]);
			// printf("\e[32;1mPPUSTATUS Read\e[m: scanline %3d cycle %3d\n", ppu->scanline, ppu->cycle);
			ppu->w = 0;
			break;
		case (OAMADDR): // 0x2003
			printf("\e[31;1mError\e[m: %s: Invalid read\n", get_ppureg_str(reg));
			exit(1);
			break;
		case (OAMDATA): // 0x2004
			data = ppu->oam[ppu->oam_addr];
			break;
		case (PPUSCROLL): // 0x2005
			break;
		case (PPUADDR): // 0x2006
			break;
		case (PPUDATA): // 0x2007
			data = ppu->readbuf;
			ppu->readbuf= ppu_read(ppu, ppu->v & 0x3FFF);
			if ((ppu->v & 0x3FFF) >= 0x3F00)
			{
				data = ppu->readbuf;
			}
			// printf("PPUDATA read: 0x%02X ppu->v: 0x%04X\n", data, ppu->v);
			ppu->v += (ppu->registers[PPUCTRL] & BIT_2) ? 32 : 1;
			break;
	}

	return data;
	(void)entry;
}

void	cpu_ppu_reg_write(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val)
{
	t_cpu	*cpu = arg;
	t_ppu	*ppu = &((t_nes *)cpu->parent_device)->ppu;
	PPUReg	reg = addr & 0x7;

	switch (reg) {
		case (PPUCTRL): // 0x2000
			SET_BIT(ppu->t, BIT_10, val & BIT_0);
			SET_BIT(ppu->t, BIT_11, val & BIT_1);
			entry->memory[reg] = val;
			return ;
		case (PPUMASK): // 0x2001
			break;
		case (PPUSTATUS): // 0x2002
			printf("\e[31;1mError\e[m: %s: Invalid write\n", get_ppureg_str(reg));
			exit(1);
			break;
		case (OAMADDR): // 0x2003
			ppu->oam_addr = val;
			break;
		case (OAMDATA): // 0x2004
			ppu->oam[ppu->oam_addr++] = val;
			break;
		case (PPUSCROLL): // 0x2005
			if (ppu->w == 0)
			{
				// Sets Fine X to low 3 bits of val;
				ppu->x = val & 0x07;
				// Sets Coarse X (bits 0-4 of t) to high 5 bits of val
				ppu->t = (ppu->t & ~0x1F) | (val >> 3);
				ppu->w = 1;
			}
			else
			{
				// Sets Fine Y (bits 12-14 of t) to low 3 bits of val
				ppu->t = (ppu->t & ~(0x07 << 12)) | ((val & 0x07) << 12);
				// Sets Coarse Y (bits 5-9 of t) to high 5 bits of val
				ppu->t = (ppu->t & ~(0x1F << 5)) | ((val & 0xF8) << 2);
				ppu->w = 0;
			}
			// printf("PPUSCROLL write: 0x%02X ppu->t: 0x%04X\n", val, ppu->t);
			break;
		case (PPUADDR): // 0x2006
			if (ppu->w == 0)
			{
				ppu->w = 1;
				// Sets bits 8-13 of t to low 6 bits of val, bit 14 cleared
				ppu->t = (ppu->t & 0x80FF) | ((val & 0x3F) << 8);
			}
			else
			{
				ppu->w = 0;
				// Sets low byte of t to to val
				ppu->t = (ppu->t & 0xFF00) | val;
				ppu->v = ppu->t;
			}
			// printf("PPUADDR write: 0x%02X ppu->t: 0x%04X\n", val, ppu->t);
			break;
		case (PPUDATA): // 0x2007
			ppu_write(ppu, ppu->v & 0x3FFF, val);
			// printf("PPUDATA write: 0x%02X ppu->v: 0x%04X\n", val, ppu->v);
			ppu->v += (ppu->registers[PPUCTRL] & BIT_2) ? 32 : 1;
			break;
	}
	entry->memory[reg] = val;
}

uint8_t cpu_io_page_read(struct pt_entry *entry, void *arg, uint16_t addr)
{
	t_cpu	*cpu = arg;
	t_nes	*nes = (t_nes *)cpu->parent_device;
	// t_ppu	*ppu = &((t_nes *)cpu->parent_device)->ppu;
	uint8_t	out = (addr >> 8) & 0xFF;

	addr &= 0xFFF;
	if (addr > 0x17)
		return out;
	
	IOReg reg = addr & 0xFFF;
	switch (reg) {
		case (SND_CHN):
			return out;
		case (JOY1):
			if (nes->joy_strobe)
				nes->joy_shift[0] = nes->joy_state[0];

			out = nes->joy_shift[0] & 0x01;

			if (!nes->joy_strobe)
				nes->joy_shift[0] >>= 1;
			return out;
		case (JOY2):
			if (nes->joy_strobe)
				nes->joy_shift[1] = nes->joy_state[1];

			out = nes->joy_shift[1] & 0x01;

			if (!nes->joy_strobe)
				nes->joy_shift[1] >>= 1;
			return out;
		default:
			return out;
	}
	(void)entry;
}

void cpu_io_page_write(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val)
{
	t_cpu		*cpu = arg;
	t_nes		*nes = (t_nes *)cpu->parent_device;
	t_ppu		*ppu = &nes->ppu;
	t_apu		*apu = &nes->apu;
	uint16_t	dma_src = 0;

	addr &= 0xFFF;
	if (addr >= IOREG_MAX)
		return ;

	if (addr <= DMC_LEN)
	{
		handle_apu_writes(apu, addr, val);
		return ;
	}
	// IOReg reg = addr & 0xFF;
	switch (addr) {
		case (OAMDMA): // 0x4014
			// cpu->cycle_events |= CYCLE_DMA;
			dma_src = val << 8;
			// cpu->catchup_cycles += 1;
			// ppu_catchup(nes);
			ppu_tick_for(ppu, 3);
			if (ppu->oam_addr != 0)
				printf("\e[31;1mDMA initiated\e[m OAMADDR: 0x%02X scanline: %d cycle: %d\n", ppu->oam_addr, ppu->scanline, ppu->cycle);
			for (int i = 0; i < 256; i++)
			{
				// cpu->catchup_cycles += 2;
				ppu->oam[ppu->oam_addr++] = read_byte(cpu, dma_src + (uint8_t)i);
				ppu_tick_for(ppu, 6);
				// ppu_catchup(nes);
			}
			return;
		case (SND_CHN): // 0x4015
			return;
		case (JOY1): // 0x4016
			nes->joy_strobe = (val & 0x01);
			if (nes->joy_strobe)
			{
				nes->joy_shift[0] = nes->joy_state[0];
				nes->joy_shift[1] = nes->joy_state[1];
			}
			return;
		case (JOY2): // 0x4017
			return;
		default:
			// Unreachable
			return;
	}
	(void)entry;
}

void	apply_nes_mmap(t_nes *nes)
{
	t_cpu *cpu = &nes->cpu;

	// Map work cpu work ram
	map_memory(cpu->pagetable, 0, 2, nes->ram, NULL, passthrough_write);
	map_memory(cpu->pagetable, 0x800, 2, nes->ram, NULL, passthrough_write);
	map_memory(cpu->pagetable, 0x1000, 2, nes->ram, NULL, passthrough_write);
	map_memory(cpu->pagetable, 0x1800, 2, nes->ram, NULL, passthrough_write);
	map_memory(cpu->pagetable, 0x4000, 1, NULL, cpu_io_page_read, cpu_io_page_write);

	// Map ppu ctrl registers
	map_memory(cpu->pagetable, 0x2000, 8, nes->ppu.registers, cpu_ppu_reg_read, cpu_ppu_reg_write);

	// Map ppu to vram based on mirroring
	map_ppu_nametables(&nes->ppu, nes->ppu.mirroring);
	map_ppu_pattern_tables(nes, nes->cart);
}
